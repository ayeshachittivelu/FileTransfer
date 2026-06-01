cmake_minimum_required(VERSION 3.16)
project(um_ciolib)

add_library(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    STATIC
        src/r_cio_um_ioctl.c
        src/r_cio_um_main.c
        src/r_cio_um_read.c
        src/r_cio_um_status.c
        src/r_cio_um_write.c
)

target_include_directories(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    PUBLIC
        lib
        ../bridge/lib
    PRIVATE
        ${CMAKE_VLIB_ROOT}/os/osal/include
)

target_link_libraries(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drv_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drivers-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)
