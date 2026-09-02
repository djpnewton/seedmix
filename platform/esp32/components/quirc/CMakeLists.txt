# quirc (QR decoder) as an ESP-IDF component.
#
# Sources live in external/quirc/lib (vendored by scripts/ensure_deps.sh).

set(QUIRC_DIR "${COMPONENT_DIR}/../../../..//external/quirc/lib")

idf_component_register(SRCS "")

add_library(quirc STATIC
    "${QUIRC_DIR}/decode.c"
    "${QUIRC_DIR}/identify.c"
    "${QUIRC_DIR}/quirc.c"
    "${QUIRC_DIR}/version_db.c"
)

target_include_directories(quirc PUBLIC "${QUIRC_DIR}")
# identify.c uses sqrt()/fabs() from libm.
target_link_libraries(quirc PUBLIC m)

target_link_libraries(${COMPONENT_LIB} INTERFACE quirc)
