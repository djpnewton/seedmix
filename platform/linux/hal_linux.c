/**
 * @file platform/linux/hal_linux.c
 * @brief Linux HAL implementation
 *
 * Random source is /dev/urandom.  Camera entropy is captured from a
 * Video4Linux2 device (default /dev/video0, override with HAL_CAMERA_DEV).
 */

#include "hal.h"
#include "util/error.h"
#include "util/log.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

/* -- Random ----------------------------------------------------------- */
const char* hal_get_random_source() { return "/dev/urandom"; }

void hal_get_random(uint8_t* buf, size_t len) {
    ASSERT_OR_DIE(buf && len > 0, "hal_get_random: invalid buffer");

    FILE* f = fopen("/dev/urandom", "rb");
    ASSERT_OR_DIE(f, "hal_get_random: failed to open /dev/urandom");
    size_t n = fread(buf, 1, len, f);
    ASSERT_OR_DIE(n == len, "hal_get_random: short read from /dev/urandom");
    fclose(f);
}

/* -- Camera ----------------------------------------------------------- */
static int xioctl(int fd, unsigned long request, void* arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

static const char* camera_device(void) {
    const char* dev = getenv("HAL_CAMERA_DEV");
    return (dev && *dev) ? dev : "/dev/video0";
}

bool hal_camera_available(void) {
    int fd = open(camera_device(), O_RDWR);
    if (fd < 0) return false;

    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        close(fd);
        return false;
    }
    close(fd);
    return (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0;
}

/* Bookkeeping for one mmap'd V4L2 buffer. */
struct v4l2_mmap_buf {
    void*  start;
    size_t length;
};

/** Map a V4L2 pixel format to the HAL enum. */
static hal_camera_pixfmt_t map_pixfmt(uint32_t v4l2_fmt) {
    switch (v4l2_fmt) {
    case V4L2_PIX_FMT_GREY:
        return HAL_CAMERA_FMT_GRAY8;
    case V4L2_PIX_FMT_YUYV:
        return HAL_CAMERA_FMT_YUYV;
    case V4L2_PIX_FMT_RGB565:
        return HAL_CAMERA_FMT_RGB565;
    case V4L2_PIX_FMT_MJPEG:
        return HAL_CAMERA_FMT_JPEG;
    default:
        return HAL_CAMERA_FMT_UNKNOWN;
    }
}

/**
 * @brief Capture one frame from the V4L2 device into @p out.
 * @return true on success; false on any capture failure.
 */
static bool v4l2_capture_frame(hal_camera_frame_t* out) {
    const char* dev = camera_device();

    uint8_t*              frame     = NULL;
    size_t                frame_len = 0;
    struct v4l2_mmap_buf* bufs      = NULL;
    uint32_t              n_mapped  = 0;

    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        LOG_ERROR("failed to open %s: %s", dev, strerror(errno));
        return false;
    }

    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1 || !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        !(cap.capabilities & V4L2_CAP_STREAMING)) {
        LOG_ERROR("%s is not a streaming capture device", dev);
        close(fd);
        return false;
    }

    /* Prefer YUYV; fall back to MJPEG if the device doesn't support it. */
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
            LOG_ERROR("failed to set capture format on %s", dev);
            close(fd);
            return false;
        }
    }

    LOG_INFO("camera %s format %ux%u fourcc '%c%c%c%c' stride=%u sizeimage=%u", dev,
             fmt.fmt.pix.width, fmt.fmt.pix.height, (char)(fmt.fmt.pix.pixelformat & 0xff),
             (char)((fmt.fmt.pix.pixelformat >> 8) & 0xff),
             (char)((fmt.fmt.pix.pixelformat >> 16) & 0xff),
             (char)((fmt.fmt.pix.pixelformat >> 24) & 0xff), fmt.fmt.pix.bytesperline,
             fmt.fmt.pix.sizeimage);

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1 || req.count < 2) {
        LOG_ERROR("failed to request buffers on %s", dev);
        close(fd);
        return false;
    }

    bufs = calloc(req.count, sizeof(*bufs));
    if (!bufs) {
        LOG_ERROR("out of memory");
        close(fd);
        return false;
    }

    for (uint32_t i = 0; i < req.count; i++) {
        struct v4l2_buffer vb;
        memset(&vb, 0, sizeof(vb));
        vb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vb.memory = V4L2_MEMORY_MMAP;
        vb.index  = i;
        if (xioctl(fd, VIDIOC_QUERYBUF, &vb) == -1) goto cleanup;

        bufs[i].length = vb.length;
        bufs[i].start  = mmap(NULL, vb.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, vb.m.offset);
        if (bufs[i].start == MAP_FAILED) goto cleanup;
        n_mapped++;
    }

    for (uint32_t i = 0; i < req.count; i++) {
        struct v4l2_buffer vb;
        memset(&vb, 0, sizeof(vb));
        vb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vb.memory = V4L2_MEMORY_MMAP;
        vb.index  = i;
        if (xioctl(fd, VIDIOC_QBUF, &vb) == -1) goto cleanup;
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) == -1) goto cleanup;

    /* Dequeue a single frame, waiting up to ~10s for the device. */
    for (unsigned attempt = 0; attempt < 5 && !frame; attempt++) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv = {2, 0};

        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r <= 0) continue; /* timeout or interrupted */

        struct v4l2_buffer vb;
        memset(&vb, 0, sizeof(vb));
        vb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vb.memory = V4L2_MEMORY_MMAP;
        if (xioctl(fd, VIDIOC_DQBUF, &vb) == -1) continue;

        frame_len = vb.bytesused;
        frame     = malloc(frame_len);
        if (!frame) {
            xioctl(fd, VIDIOC_QBUF, &vb);
            break;
        }
        memcpy(frame, bufs[vb.index].start, frame_len);
        xioctl(fd, VIDIOC_QBUF, &vb);
    }

    xioctl(fd, VIDIOC_STREAMOFF, &type);

cleanup:
    for (uint32_t i = 0; i < n_mapped; i++) {
        munmap(bufs[i].start, bufs[i].length);
    }
    free(bufs);
    close(fd);

    if (!frame) {
        LOG_ERROR("failed to capture a frame from %s", dev);
        return false;
    }

    out->data           = frame;
    out->size           = frame_len;
    out->width          = fmt.fmt.pix.width;
    out->height         = fmt.fmt.pix.height;
    out->bytes_per_line = fmt.fmt.pix.bytesperline;
    out->pixfmt         = map_pixfmt(fmt.fmt.pix.pixelformat);
    return true;
}

bool hal_camera_capture(hal_camera_frame_t* frame) {
    ASSERT_OR_DIE(frame, "hal_camera_capture: null frame");
    memset(frame, 0, sizeof(*frame));
    return v4l2_capture_frame(frame);
}

void hal_camera_frame_free(hal_camera_frame_t* frame) {
    if (!frame) return;
    if (frame->data) {
        memset(frame->data, 0, frame->size);
        free(frame->data);
    }
    memset(frame, 0, sizeof(*frame));
}
