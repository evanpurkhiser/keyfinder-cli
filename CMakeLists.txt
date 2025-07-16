cmake_minimum_required(VERSION 3.10)
project(keyfinder-cli VERSION 1.1.2)

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
install(FILES ${PROJECT_NAME}.1 TYPE MAN)
