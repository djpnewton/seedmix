#[=======================================================================[
Vendor libqrencode (QR encoder) and quirc (QR decoder).

Both are cloned by scripts/ensure_deps.sh into external/.  After inclusion
the following targets are available:

  qrencode - static QR encoder library (auto mode/version, ECC L)
  quirc    - static QR decoder library
#]=======================================================================]

# libqrencode ships a CMakeLists; build just the library (its CLI tool needs
# libpng/iconv, which we don't want to make a build requirement).
set(WITH_TOOLS OFF CACHE BOOL "" FORCE)
set(WITH_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/libqrencode ${CMAKE_BINARY_DIR}/qrencode)

# libqrencode's CMakeLists doesn't export its include dir; do it here.
target_include_directories(qrencode INTERFACE ${CMAKE_SOURCE_DIR}/external/libqrencode)

# quirc has no CMakeLists; build the decoder library from its lib/ sources.
add_library(quirc STATIC
    ${CMAKE_SOURCE_DIR}/external/quirc/lib/decode.c
    ${CMAKE_SOURCE_DIR}/external/quirc/lib/identify.c
    ${CMAKE_SOURCE_DIR}/external/quirc/lib/quirc.c
    ${CMAKE_SOURCE_DIR}/external/quirc/lib/version_db.c
)
target_include_directories(quirc PUBLIC ${CMAKE_SOURCE_DIR}/external/quirc/lib)
target_link_libraries(quirc PUBLIC m)
