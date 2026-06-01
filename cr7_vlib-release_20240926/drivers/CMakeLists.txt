cmake_minimum_required(VERSION 3.16)
project(drivers)

if(ETHER_ENABLE)
    # ether sources
    file(GLOB
        ETH_GENERIC_SOURCES "ether/src/generic/*.c"
        ETH_COMMON_SOURCES "ether/src/modules_common/src/*.c"
        ETH_H3_SOURCES "ether/src/modules_h3/src/*.c"
        ETH_NETIF_SOURCES "ether/netif/src/*.c"
        ETH_RAVB_SOURCES "ether/ravb/src/*.c"
        # ETH_STUB_SOURCES "ether/stub/src/*.c"
    )
endif()

# Create drivers lib file
add_library(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    STATIC
        du/src/r_du_api.c
        du/src/r_du_plane.c
        du/src/r_du_regs.c
        du/src/r_du_timings.c
        delay/src/r_delay_api.c
        vsp2/src/r_vsp2_api.c
        vsp2/src/r_vsp2_brs.c
        vsp2/src/r_vsp2_bru.c
        vsp2/src/r_vsp2_dl.c
        vsp2/src/r_vsp2_dpr.c
        vsp2/src/r_vsp2_isr.c
        vsp2/src/r_vsp2_lif.c
        vsp2/src/r_vsp2_regs.c
        vsp2/src/r_vsp2_rpf.c
        vsp2/src/r_vsp2_vspd_api.c
        vsp2/src/r_vsp2_vspi_api.c
        vsp2/src/r_vsp2_wpf.c
        lvds/src/r_lvds_api.c
        lvds/src/r_lvds_regs.c
        i2c/src/r_5p49v6901_api.c
        i2c/src/r_i2c_api.c
        i2c/src/r_i2c_regs.c
        prr/src/r_prr_api.c
        print/src/r_print_api.c
        tick/src/r_tick_api.c
        tick/src/rcarx3-ros/r_tick_sys.c
        cpg/src/r_cpg_api.c
        gpio/src/r_gpio_api.c
        doc/src/r_doc_api.c
        doc/src/r_doc_regs.c
        pfc/src/r_pinctrl_api.c
        sysc/src/r_sysc_api.c
        vin/src/r_vin_api.c
        csi2/src/r_csi2_api.c
        dmac/src/r_rtdmac_api.c
        dmac/src/r_audmac_api.c
        dmac/src/rdmac_api.c
        dmac/src/rdmac_ctrl.c
        dmac/src/rdmac_int.c
        dmac/src/rdmac_reg.c
        imr/src/basic_function/r_imrdrv_basic_api.c
        imr/src/basic_function/r_imrdrv_basic_mainctl.c
        imr/src/basic_function/gen3e/r_imrdrv_basic_dev.c
        imr/src/optional_function/r_imrdrv_opt_api.c
        imr/src/osal_wrapper/r_imrdrv_osal_io.c
        imr/src/osal_wrapper/r_imrdrv_osal_mutex.c
        imr/src/state_management/r_imrdrv_state_manage.c
        dcu/src/r_dcu_api.c
        ${ETH_GENERIC_SOURCES}
        ${ETH_COMMON_SOURCES}
        ${ETH_H3_SOURCES}
        emmc/emmc_cmd.c
        emmc/emmc_init.c
        emmc/emmc_interrupt.c
        emmc/emmc_mount.c
        emmc/emmc_read.c
        emmc/emmc_utility.c
        emmc/micro_delay.c
        emmc/common.c
        emmc/emmc_write.c
        emmc/emmc_erase.c
        ${ETH_NETIF_SOURCES}
        ${ETH_RAVB_SOURCES}
        ${ETH_STUB_SOURCES}
)

target_include_directories(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    PUBLIC
        lib
        du/lib
        delay/lib
        prr/lib
        du/lib
        vsp2/lib
        lvds/lib
        i2c/lib
        lib
        prr/lib
        print/lib
        tick/lib
        cpg/lib
        gpio/lib
        doc/lib
        pfc/lib
        sysc/lib
        vin/lib
        io/lib
        csi2/lib
        dmac/lib
        imr/include
        dcu/lib
        audio/lib
        ether/lib
        ether/src/generic
        ether/src/modules_common/include
        ether/src/modules_h3/include
        emmc/
        ether/netif/lib
        ether/ravb/lib
        ether/stub/lib
    PRIVATE
        ${CMAKE_VLIB_ROOT}/os/osal/include
        imr/src/include
        imr/src/include/basic_function
        imr/src/include/basic_function/gen3e
        # TODO: handle SOC variants differently
        imr/src/include/basic_function/gen3e/h3
        imr/src/include/state_management
        imr/src/include/osal_wrapper
        dcu/src
)

target_link_libraries(${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    PRIVATE
        hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        drv_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
        common
        os-def
)

# Create HDMI lib file
file(GLOB
    HDMI_SOURCES "hdmi/src/*.c"
)

if(HDMI_SOURCES) # Build HDMI with source if present
    add_library(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        STATIC
            ${HDMI_SOURCES}
    )

    target_include_directories(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        PUBLIC
            hdmi/lib
            hdmi/src
    )

    target_link_libraries(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        PRIVATE
            cio-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
            drivers-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    )

    # default prefix is "lib"
    set_target_properties(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU} PROPERTIES PREFIX "")

else() # Link existing library if no source code
    add_library(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        STATIC IMPORTED GLOBAL
    )

    target_include_directories(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdmi/lib>
    )

    set_target_properties(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}
        PROPERTIES
            PREFIX ""
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/hdmi/lib/${RCAR_TARGET_OS}/hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU}.a
    )

    # default prefix is "lib"
    set_target_properties(hdmi-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${RCAR_FPU} PROPERTIES PREFIX "")

endif()

