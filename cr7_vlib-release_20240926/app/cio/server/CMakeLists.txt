cmake_minimum_required(VERSION 3.16)
project(cio)

# Conditional checks for source code compiling
if (ETHER_ENABLE)
    file(GLOB
        CIO_ETHER_SOURCES "../driver/r-car_ether/src/*.c"
    )
endif()

if (R_USE_CANFD)
    file(GLOB
        CIO_CANFD_SOURCES "../driver/r-car_canfd/src/*.c"
    )
endif()

if (R_USE_VIN)
    file(GLOB
        CIO_VIN_SOURCES "../driver/r-car_vin/src/*.c"
    )
endif()

if (R_USE_WM)
    file(GLOB
        CIO_WM_SOURCES "../driver/r-car_wm/src/*.c"
    )
endif()

# Create CIO library
add_library(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    STATIC
        src/r_cio_main.c
        src/r_cio_server.c
        ../bsp/src/r_cio_bsp_utility.c
        ../bsp/salvatorxs_cr7/src/r_cio_bsp_board.c
        ../bsp/salvatorxs_cr7/src/r_cio_bsp_r-carx3_cr7_can.c
        ../bsp/salvatorxs_cr7/src/r_cio_bsp_r-carx3_cr7_vin.c
        ../bsp/salvatorxs_cr7/src/r_cio_bsp_r-carx3_cr7.c
        ../bsp/salvatorxs_cr7/src/bsp_wm/r_cio_bsp_r-carx3_cr7_vout.c
        ../driver/dummy/src/r_cio_dummy_main.c
        ${CIO_WM_SOURCES}
        ${CIO_VIN_SOURCES}
        ${CIO_ETHER_SOURCES}
        ${CIO_CANFD_SOURCES}
)

target_include_directories(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    PUBLIC
        lib
        ../driver/r-car_canfd/lib
        ../driver/r-car_can/lib
        ../driver/r-car_wm/lib
        ../driver/r-car_vin/lib
        ../driver/r-car_ether/lib
    PRIVATE
        src
        ../bridge/lib
        ../bsp/lib
        ../bsp/src
        ../driver/dummy/lib
        ../driver/dummy/src
        ../bsp/salvatorxs_cr7/src
        ${CMAKE_VLIB_ROOT}/os/osal/include
)

target_link_libraries(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drv_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drivers-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)
