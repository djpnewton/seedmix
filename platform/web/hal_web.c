/**
 * @file platform/web/hal_web.c
 * @brief Web (Emscripten) HAL implementation.
 *
 * Entropy comes from the Web Crypto API (crypto.getRandomValues), the only
 * CSPRNG available in the browser.  Camera uses getUserMedia: the
 * webcam stream is drawn into a hidden <canvas> and each grab copies a
 * grayscale frame into the C heap.
 */

#include "hal.h"
#include "util/error.h"
#include "util/log.h"
#include "util/utils.h"

#include <emscripten/emscripten.h>
#include <stdlib.h>
#include <string.h>

/* -- Random ----------------------------------------------------------- */
const char* hal_get_random_source(void) { return "WebCrypto getRandomValues"; }

void hal_get_random(uint8_t* buf, size_t len) {
    ASSERT_OR_DIE(buf && len > 0, "hal_get_random: invalid buffer");

    // crypto.getRandomValues caps a single call at 65536 bytes
    while (len > 0) {
        size_t n = len > 65536 ? 65536 : len;
        EM_ASM({ crypto.getRandomValues(new Uint8Array(HEAPU8.buffer, $0, $1)); }, buf, n);
        buf += n;
        len -= n;
    }
}

/* -- Camera ----------------------------------------------------------- */
/* Only one camera session exists at a time in the app, so the JS side is a
 * singleton (Module.__seedmixCam).  getUserMedia is asynchronous, so
 * hal_camera_open() returns immediately and hal_camera_grab() reports "no
 * frame yet" until the stream is ready (the app polls it from an LVGL timer).
 *
 * Note: a browser with no camera still exposes getUserMedia, so
 * hal_camera_available() can only test for the API, not for actual hardware.
 */

struct hal_camera {
    uint32_t width;
    uint32_t height;
};

bool hal_camera_available(void) {
    return (bool)EM_ASM_INT({
        return (window.isSecureContext && typeof navigator != 'undefined' &&
                navigator.mediaDevices && typeof navigator.mediaDevices.getUserMedia == 'function')
                   ? 1
                   : 0;
    });
}

hal_camera_t* hal_camera_open(void) {
    hal_camera_t* cam = (hal_camera_t*)calloc(1, sizeof(*cam));
    if (!cam) {
        LOG_ERROR("out of memory");
        return NULL;
    }

    EM_ASM({
        var c = ({
            video : null,
            canvas : null,
            ctx : null,
            stream : null,
            ready : false,
            width : 0,
            height : 0
        });

        c.video = document.createElement('video');
        c.video.setAttribute('playsinline', '');
        c.video.setAttribute('autoplay', '');
        c.video.muted         = true;
        c.video.style.display = 'none';
        document.body.appendChild(c.video);

        c.canvas = document.createElement('canvas');
        c.ctx    = c.canvas.getContext('2d', {willReadFrequently : true});

        Module.__seedmixCam = c;

        navigator.mediaDevices
            .getUserMedia({
                video : {facingMode : 'user', width : {ideal : 640}, height : {ideal : 480}},
                audio : false
            })
            .then(function(stream) {
                c.stream          = stream;
                c.video.srcObject = stream;
                c.video.addEventListener(
                    'loadedmetadata', function() {
                        c.width  = c.video.videoWidth;
                        c.height = c.video.videoHeight;
                        if (c.width && c.height) {
                            c.canvas.width  = c.width;
                            c.canvas.height = c.height;
                            c.ready         = true;
                        }
                    });
                return c.video.play();
            })
            .catch(function(err) {
                console.error('seedmix: getUserMedia failed: ' + err);
                c.ready = false;
            });
    });

    LOG_INFO("camera: requesting webcam access");
    return cam;
}

bool hal_camera_grab(hal_camera_t* cam, hal_camera_frame_t* out) {
    (void)cam;
    if (!out) return false;
    memset(out, 0, sizeof(*out));

    if (!EM_ASM_INT({ return Module.__seedmixCam && Module.__seedmixCam.ready ? 1 : 0; })) {
        return false;
    }

    uint32_t w = (uint32_t)EM_ASM_INT({ return Module.__seedmixCam.width; });
    uint32_t h = (uint32_t)EM_ASM_INT({ return Module.__seedmixCam.height; });
    if (!w || !h) return false;

    uint8_t* buf = (uint8_t*)malloc((size_t)w * h);
    if (!buf) {
        LOG_ERROR("out of memory");
        return false;
    }

    // Draw the latest video frame and copy its luma into `buf`
    EM_ASM(
        {
            var c = Module.__seedmixCam;
            c.ctx.drawImage(c.video, 0, 0, c.width, c.height);
            var d    = c.ctx.getImageData(0, 0, c.width, c.height).data;
            var n    = c.width * c.height;
            var gray = new Uint8Array(n);
            for (var i = 0; i < n; i++) {
                var r   = d[i * 4];
                var g   = d[i * 4 + 1];
                var b   = d[i * 4 + 2];
                gray[i] = ((r * 299 + g * 587 + b * 114 + 500) / 1000) | 0;
            }
            HEAPU8.set(gray, $0);
        },
        buf);

    out->data           = buf;
    out->size           = (size_t)w * h;
    out->width          = w;
    out->height         = h;
    out->bytes_per_line = w;
    out->pixfmt         = HAL_CAMERA_FMT_GRAY8;
    return true;
}

void hal_camera_close(hal_camera_t* cam) {
    if (!cam) return;
    EM_ASM({
        var c = Module.__seedmixCam;
        if (c) {
            if (c.stream) {
                var tracks = c.stream.getTracks();
                for (var i = 0; i < tracks.length; i++) tracks[i].stop();
            }
            if (c.video && c.video.parentNode) c.video.parentNode.removeChild(c.video);
            Module.__seedmixCam = null;
        }
    });
    free(cam);
}

void hal_camera_frame_free(hal_camera_frame_t* frame) {
    if (!frame) return;
    if (frame->data) {
        secure_memzero(frame->data, frame->size);
        free(frame->data);
    }
    memset(frame, 0, sizeof(*frame));
}

/* -- Touch / pointer input ------------------------------------------- */
bool hal_touch_available(void) {
    // The SDL backend always registers a mouse pointer
    return true;
}
