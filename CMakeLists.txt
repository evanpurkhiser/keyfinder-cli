cmake_minimum_required(VERSION 3.10)
project(keyfinder-cli)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    keyfinder_cli.cpp
)

find_package(PkgConfig REQUIRED)

# FFMPEG
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswresample
    libavutil
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    PkgConfig::LIBAV
)

# libkeyfinder
find_package(KeyFinder REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE KeyFinder::keyfinder)

# Installation
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME})
if(UNIX)
    install(FILES ${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# Packaging: `cpack -G DEB` produces a .deb of the installed tree.
# CPACK_DEBIAN_PACKAGE_SHLIBDEPS=ON makes dpkg-shlibdeps derive the
# Depends: line from the binary's dynamic library references, so the
# package gets the correct libavformatN / libfftw3-3 etc. for the
# build environment without us having to hand-curate per-distro deps.
set(CPACK_PACKAGE_NAME "keyfinder-cli")
set(CPACK_PACKAGE_VENDOR "keyfinder-cli upstream")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Small command line tool that detects the musical key of an audio file")
set(CPACK_PACKAGE_HOMEPAGE_URL
    "https://github.com/EvanPurkhiser/keyfinder-cli")
set(CPACK_PACKAGE_CONTACT
    "keyfinder-cli upstream <https://github.com/EvanPurkhiser/keyfinder-cli/issues>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# DEB-specific.
set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Allow CI to override the version (so a tag like v1.2.0-prebuilt-1
# carries through into the .deb filename + control file).
if(NOT CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_VERSION "1.2.0")
endif()

include(CPack)
