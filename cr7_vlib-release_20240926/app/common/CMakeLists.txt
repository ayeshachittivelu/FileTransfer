cmake_minimum_required(VERSION 3.16)
project(common)

# Build logo library
add_library(${PROJECT_NAME}
    src/display.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        inc
        ${CMAKE_VLIB_ROOT}/os/osal/include
        ${CMAKE_VLIB_ROOT}/os/osal_wrapper/src/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        um_ciolib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
        um_wmlib-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)