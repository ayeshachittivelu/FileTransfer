## Common file ##

# This file is included to generate executable files
### PRE-REQUISITES
## apps / wrappers must set the following when including
# PROJECT_NAME
#   - name of the project
# RCAR_WRAPPER_APP (optional)
#   - name of application if building a wrapper
# RCAR_LINKER_SCRIPT
#   - PAth to linker file for the application
# RCAR_STANDALONE_PATH
#   - This is the path to the folder containing main.c and resource_table_linux.c
# RCAR_ADDITIONAL_SOURCES (optional)
#   - Additional sources files needed by the application
# RCAR_ADDITIONAL_LIBS (optional)
#   - Additional library files needed by the application
# RCAR_INCLUDE_DIRECTORIES (optional)
#   - Additional include directories needed by the application


set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=${PROJECT_NAME}.map -lg -lnosys -MD -mfloat-abi=${RCAR_FLOAT_ABI} -mfpu=${RCAR_FPU} -ffreestanding --specs=nosys.specs  ${CMAKE_EXE_LINKER_FLAGS} -T ${RCAR_LINKER_SCRIPT}")

# AutoSAR specific
#  Build goil, AutoSAR files, OSAL wrapper and OS libraries
if(RCAR_TARGET_OS STREQUAL "trampoline")
    include(${CMAKE_VLIB_ROOT}/compiler/tpl-goil-files.cmake)
    add_subdirectory(${CMAKE_VLIB_ROOT}/os/osal_wrapper ${CMAKE_BINARY_DIR}/app/${PROJECT_NAME}/os/osal_wrapper)
    add_subdirectory(${CMAKE_VLIB_ROOT}/os/trampoline ${CMAKE_BINARY_DIR}/app/${PROJECT_NAME}/os/trampoline)
    set(OS_LIB ${RCAR_TARGET_OS}-${PROJECT_NAME})
elseif(RCAR_TARGET_OS STREQUAL "freertos")
    set(OS_LIB ${RCAR_TARGET_OS})
endif()

# Adds an executable
add_executable(${PROJECT_NAME}
    ${RCAR_STANDALONE_PATH}/main.c
    ${RCAR_STANDALONE_PATH}/resource_table_linux.c
    ${RCAR_ADDITIONAL_SOURCES}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        SUFFIX ".out"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LINK_DEPENDS ${RCAR_LINKER_SCRIPT}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${RCAR_INCLUDE_DIRECTORIES}
        ${CMAKE_VLIB_ROOT}/os/osal/include
)

add_library(OS_LIBS-${PROJECT_NAME} INTERFACE)
target_link_libraries(OS_LIBS-${PROJECT_NAME}
    INTERFACE
        ${RCAR_WRAPPER_APP}
        ${OS_LIB}
        osallib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${OS_LIB}
)

target_link_libraries(${PROJECT_NAME}
    OS_LIBS-${PROJECT_NAME}
    ${RCAR_ADDITIONAL_LIBS}
    davehd-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drw2d-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    cio-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drv_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
    drivers-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)

# create the srec with custom command
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} --srec-forceS3 -O srec $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.srec
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.bin
)
