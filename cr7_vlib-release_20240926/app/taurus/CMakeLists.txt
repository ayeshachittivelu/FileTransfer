cmake_minimum_required(VERSION 3.16)
project(taurus)

if (R_TAURUS_ENABLE_ETHER)
    file(GLOB
        TAURUS_ETHER_SOURCES
            "src/perserv/ethserv/src/r_ether_sv_main.c"
            "driver/eth/cio/src/r_ether_drv_main.c"
    )
endif()

if (R_TAURUS_ENABLE_CAN)
    file(GLOB
        TAURUS_CANFD_SOURCES
            "src/perserv/canserv/src/r_can_sv_main.c"
            "driver/can/cio/src/r_can_drv_main.c"
    )
endif()

if (R_TAURUS_ENABLE_RVGC)
    file(GLOB
        TAURUS_RVGC_SOURCES
            "src/perserv/rvgcserv/src/r_rvgc_sv_main.c"
            "driver/rvgc/src/r_rvgc_drv_main.c"
    )
endif()

if (R_TAURUS_ENABLE_CAMERA)
    file(GLOB
        TAURUS_CAMERA_SOURCES
            "src/perserv/cameraserv/src/r_camera_sv_main.c"
            "driver/camera/r_camera_drv_main.c"
    )
endif()

if (R_TAURUS_ENABLE_COMCH)
    file(GLOB
      TAURUS_COMCH_SOURCES
      "src/perserv/comchserv/src/r_comch_sv_main.c"
      "driver/comch/src/r_comch_drv_main.c"
    )
endif()

add_library(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    STATIC
        src/r_taurus_main.c
        src/comserv/r_comsv_main.c
        comal/src/rpmsg/r_rte_comal_rpmsg_main.c
        comal/src/rpmsg/taurus_rpmsg.c
        comal/src/rpmsg/taurus_virtqueue.c
        platform/ros_cr7_r-carx3/src/r_pf_ros_cr7_r-carx3.c
        ${TAURUS_ETHER_SOURCES}
        ${TAURUS_CANFD_SOURCES}
        ${TAURUS_RVGC_SOURCES}
        ${TAURUS_CAMERA_SOURCES}
        ${TAURUS_COMCH_SOURCES}
)

target_include_directories(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    PUBLIC
        lib
        comal/lib
        comal/src/rpmsg
    PRIVATE
        src
        src/comserv
        src/perserv/lib
        driver/include
        driver/can/lib
        driver/rvgc/lib
        driver/eth/lib
        driver/comch/lib
        comal/lib
        bridge/lib
        platform/lib
        src/perserv/lib
        src/perserv/canserv/lib
        src/perserv/rvgcserv/lib
        src/perserv/ethserv/lib
        src/perserv/comchserv/lib
        ${CMAKE_VLIB_ROOT}/os/osal/include
)

target_link_libraries(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    common
    os-def
    um_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    um_ciolib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)
