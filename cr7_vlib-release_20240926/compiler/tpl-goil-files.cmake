## Common file ##

# This file is included by applications using Trampoline OS
#  It is used to generate the goil and AutoSAR source files

### PRE-REQUISITES
## apps / wrappers must set the following when including
# PROJECT_NAME
# RCAR_LINKER_SCRIPT

set(GOIL "${CMAKE_VLIB_ROOT}/os/trampoline/goil/makefile-unix/goil")

add_custom_target(
    ${PROJECT_NAME}_goil_files
)

# first, build & copy the goil compiler to our CMake build directory
add_custom_command(TARGET
    ${PROJECT_NAME}_goil_files
        PRE_BUILD
            COMMAND ./build.py
            COMMAND cp goil ${CMAKE_CURRENT_BINARY_DIR}
            WORKING_DIRECTORY "${CMAKE_VLIB_ROOT}/os/trampoline/goil/makefile-unix"
)

# then, cp the .oil file and run the goil command
# after, also copy the "autosar" files
#   TODO: only run goil when necessary
add_custom_command(TARGET ${PROJECT_NAME}_goil_files
    POST_BUILD
        COMMAND cp ${CMAKE_VLIB_ROOT}/app/${PROJECT_NAME}/target/${RCAR_TARGET_OS}_salvatorxs/autosar/trampoline.oil ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${GOIL} --target=cortex-r/armv7/RCar --templates=${CMAKE_VLIB_ROOT}/os/trampoline/goil/templates/ trampoline.oil
        COMMAND cp ${CMAKE_VLIB_ROOT}/app/${PROJECT_NAME}/target/${RCAR_TARGET_OS}_salvatorxs/autosar/autosar_osal.h ${CMAKE_CURRENT_BINARY_DIR}/trampoline/.
        COMMAND cp ${CMAKE_VLIB_ROOT}/app/${PROJECT_NAME}/target/${RCAR_TARGET_OS}_salvatorxs/autosar/autosar_main.c ${CMAKE_CURRENT_BINARY_DIR}/trampoline/.
        COMMAND cp ${CMAKE_VLIB_ROOT}/app/${PROJECT_NAME}/target/${RCAR_TARGET_OS}_salvatorxs/autosar/autosar_threads.c ${CMAKE_CURRENT_BINARY_DIR}/trampoline/.
)