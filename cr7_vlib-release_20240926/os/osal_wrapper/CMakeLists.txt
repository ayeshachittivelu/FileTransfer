cmake_minimum_required(VERSION 3.16)
# PARENT_PROJECT_NAME - AutoSAR only parameter
set(PARENT_PROJECT_NAME ${PROJECT_NAME})
project(osallib)

# Must enable ASM for this project
enable_language(C ASM)

# OS specific source files
if(RCAR_TARGET_OS STREQUAL "freertos")
    set(LIBRARY_NAME ${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS})
    set_property(SOURCE src/target/freertos/r_os_asm_cache.s PROPERTY LANGUAGE ASM)
    set(OS_SOURCES
        src/target/${RCAR_TARGET_OS}/r_os_asm_cache.s
    )
    set(OS_LIB ${RCAR_TARGET_OS})
elseif(RCAR_TARGET_OS STREQUAL "trampoline")
    set(LIBRARY_NAME ${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}-${PARENT_PROJECT_NAME})
    set(OS_SOURCES
        src/target/${RCAR_TARGET_OS}/r_osal_base.c
        src/target/${RCAR_TARGET_OS}/r_osal_dummy.c
    )
    set(OS_LIB ${RCAR_TARGET_OS}-${PARENT_PROJECT_NAME})
endif()

add_library(${LIBRARY_NAME}
    STATIC
        ${CMAKE_VLIB_ROOT}/os/osal_dev_cfg/src/target/${RCAR_TARGET_OS}/r_osal_dev_cfg_tbl.c
        ${CMAKE_VLIB_ROOT}/os/osal_dev_cfg/src/target/${RCAR_TARGET_OS}/r_osal_dev_cfg.c
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/src/target/${RCAR_TARGET_OS}/r_osal_usr_cfg.c
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/src/target/${RCAR_TARGET_OS}/r_osal_resource_info.c
        ${OS_SOURCES}
        src/target/${RCAR_TARGET_OS}/r_os_cache.c
        src/target/${RCAR_TARGET_OS}/r_osal_common.c
        src/target/${RCAR_TARGET_OS}/r_osal_interrupt.c
        src/target/${RCAR_TARGET_OS}/r_osal_io.c
        src/target/${RCAR_TARGET_OS}/r_osal_memory.c
        src/target/${RCAR_TARGET_OS}/r_osal_message.c
        src/target/${RCAR_TARGET_OS}/r_osal_power.c
        src/target/${RCAR_TARGET_OS}/r_osal_soc.c
        src/target/${RCAR_TARGET_OS}/r_osal_thread.c
        src/target/${RCAR_TARGET_OS}/r_osal_threadsync.c
        src/target/${RCAR_TARGET_OS}/r_osal_timeclock.c
        src/target/${RCAR_TARGET_OS}/r_osal_version.c
        ${CMAKE_VLIB_ROOT}/middleware/libraries/memory_allocator/src/r_meml_allocator.c
        src/target/${RCAR_TARGET_OS}/pma/r_pma_hwdepend.c
        src/target/${RCAR_TARGET_OS}/pma/r_pma_if.c
        src/target/${RCAR_TARGET_OS}/pma/r_pma_statemachine.c
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${CMAKE_VLIB_ROOT}/os/osal/include
        ${CMAKE_VLIB_ROOT}/os/osal/include/rcar-xos/osal
        ${CMAKE_VLIB_ROOT}/os/osal_dev_cfg/include
        ${CMAKE_VLIB_ROOT}/os/osal_dev_cfg/src/target/${RCAR_TARGET_OS}/include
        ${CMAKE_VLIB_ROOT}/os/osal_dev_cfg/src/target/${RCAR_TARGET_OS}/include/config
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/include
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/src/target/${RCAR_TARGET_OS}/include
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/src/target/${RCAR_TARGET_OS}/include/config/${RCAR_TARGET_OS}
        src/include/target/${RCAR_TARGET_OS}
        ${CMAKE_VLIB_ROOT}/middleware/libraries/memory_allocator/include
        src/include/${RCAR_TARGET_OS}/pma
        ${CMAKE_VLIB_ROOT}/drivers/print/lib
    PRIVATE
        ${CMAKE_VLIB_ROOT}/drivers/tick/lib
        ${CMAKE_VLIB_ROOT}/drivers/cpg/lib
        ${CMAKE_VLIB_ROOT}/drivers/io/lib
        ${CMAKE_VLIB_ROOT}/drivers/sysc/lib
        ${CMAKE_VLIB_ROOT}/drivers/prr/lib
        src/include
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/src/include/config/${RCAR_TARGET_OS}
        ${CMAKE_VLIB_ROOT}/os/osal_usr_cfg/include/rcar-xos/osal_usr_cfg
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${OS_LIB}
)

# default prefix is "lib"
set_target_properties(${LIBRARY_NAME} PROPERTIES PREFIX "")
