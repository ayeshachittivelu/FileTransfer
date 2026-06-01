cmake_minimum_required(VERSION 3.16)
project(canfd_sample)

set(RCAR_WRAPPER_APP "")
set(RCAR_LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/target/${RCAR_TARGET_OS}_salvatorxs/${RCAR_TARGET_OS}.ld")
set(RCAR_STANDALONE_PATH src/standalone)

# Build audio-lib
add_library(${PROJECT_NAME}-lib
    src/canfd_sample.c
)

target_include_directories(${PROJECT_NAME}-lib
    PUBLIC
        lib
    PRIVATE
        src
)

add_compile_definitions(
    R_TARGET_BOARD_ERGUOTOU
    # USE_CANFD_CR
    # USE_CANFD_CH00
)

target_link_libraries(${PROJECT_NAME}-lib
    PRIVATE
        common
)

# build and link final executable
set(RCAR_ADDITIONAL_LIBS
    ${PROJECT_NAME}-lib
    taurus-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_COMPILER}-${RCAR_TARGET_OS}
)

include(${CMAKE_VLIB_ROOT}/compiler/build-executable.cmake)