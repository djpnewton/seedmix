#[=======================================================================[
Build libwally-core from the external/libwally-core submodule.

Uses ExternalProject to run autogen/configure/make and install into the
build tree.  After inclusion, the following are available:

  LIBWALLY_INCLUDE_DIR    - path to wally_core.h etc.
  LIBWALLY_LIBRARY        - path to libwallycore.a (static)
  libwally                - CMake IMPORTED target
#]=======================================================================]

include(ExternalProject)

set(LIBWALLY_SOURCE_DIR  ${CMAKE_SOURCE_DIR}/external/libwally-core)
set(LIBWALLY_INSTALL_DIR ${CMAKE_BINARY_DIR}/libwally-install)
set(LIBWALLY_INCLUDE_DIR ${LIBWALLY_INSTALL_DIR}/include)
set(LIBWALLY_LIBRARY     ${LIBWALLY_INSTALL_DIR}/lib/libwallycore.a)
set(LIBSECP256K1_LIBRARY ${LIBWALLY_INSTALL_DIR}/lib/libsecp256k1.a)

if(NOT EXISTS ${LIBWALLY_SOURCE_DIR}/tools/autogen.sh)
    message(FATAL_ERROR
        "libwally-core submodule not found at ${LIBWALLY_SOURCE_DIR}\n"
        "Run:  git submodule update --init")
endif()

ExternalProject_Add(libwally_ext
    SOURCE_DIR        ${LIBWALLY_SOURCE_DIR}
    CONFIGURE_COMMAND cd ${LIBWALLY_SOURCE_DIR}
                COMMAND ./tools/autogen.sh
                COMMAND ./configure
                    --prefix=${LIBWALLY_INSTALL_DIR}
                    --enable-static
                    --disable-shared
    BUILD_COMMAND     $(MAKE) -j
    INSTALL_COMMAND   $(MAKE) install
    BUILD_IN_SOURCE   1
    BUILD_BYPRODUCTS  ${LIBWALLY_LIBRARY}
    USES_TERMINAL_CONFIGURE OFF
    USES_TERMINAL_BUILD     OFF
    USES_TERMINAL_INSTALL   OFF
)

# Ensure the install directories exist at configure time so the
# IMPORTED target's properties resolve
file(MAKE_DIRECTORY ${LIBWALLY_INCLUDE_DIR})
file(MAKE_DIRECTORY ${LIBWALLY_INSTALL_DIR}/lib)

# Create an IMPORTED target so we can link with `libwally` + `libsecp256k1`
add_library(libwally STATIC IMPORTED GLOBAL)
add_dependencies(libwally libwally_ext)
set_target_properties(libwally PROPERTIES
    IMPORTED_LOCATION ${LIBWALLY_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${LIBWALLY_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES ${LIBSECP256K1_LIBRARY}
)

message(STATUS "libwally will be built from submodule -> ${LIBWALLY_INSTALL_DIR}")
