cmake_minimum_required(VERSION 3.16)

# include toolchain file
include(${CMAKE_TOOLCHAIN_FILE})

project(crx-platform)

set(CMAKE_VLIB_ROOT "${PROJECT_SOURCE_DIR}")

include(${CMAKE_VLIB_ROOT}/options.cmake)

add_subdirectory(app)
add_subdirectory(drivers)
add_subdirectory(middleware)
add_subdirectory(os)
