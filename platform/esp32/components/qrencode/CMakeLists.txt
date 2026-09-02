# libqrencode as an ESP-IDF component.
#
# Sources live in external/libqrencode (vendored by scripts/ensure_deps.sh).
# The component registers no sources of its own; it builds the upstream
# static library and links it into the component's interface.

set(QRENCODE_DIR "${COMPONENT_DIR}/../../../..//external/libqrencode")

idf_component_register(SRCS "")

add_library(qrencode STATIC
    "${QRENCODE_DIR}/qrencode.c"
    "${QRENCODE_DIR}/qrinput.c"
    "${QRENCODE_DIR}/bitstream.c"
    "${QRENCODE_DIR}/qrspec.c"
    "${QRENCODE_DIR}/rsecc.c"
    "${QRENCODE_DIR}/split.c"
    "${QRENCODE_DIR}/mask.c"
    "${QRENCODE_DIR}/mqrspec.c"
    "${QRENCODE_DIR}/mmask.c"
)

target_include_directories(qrencode PUBLIC "${QRENCODE_DIR}")
target_compile_definitions(qrencode PRIVATE
    STATIC_IN_RELEASE=static
    MAJOR_VERSION=4
    MINOR_VERSION=1
    MICRO_VERSION=1
    VERSION=\"4.1.1\"
)

target_link_libraries(${COMPONENT_LIB} INTERFACE qrencode)
