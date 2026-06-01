# Cross compiler
set(CROSS_COMPILE "arm-none-eabi-")

# Define as generic system
set(CMAKE_SYSTEM_NAME "Generic")

# This is so CMake can build a "Test library" successfully
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# System processor
set(CMAKE_SYSTEM_PROCESSOR "arm")

# System compiler 
set(CMAKE_SYSTEM_COMPILER "gnu")

# R-Car processor
set(RCAR_PROCESSOR "cortex-r7")

# Debug symbols
option(CMAKE_BUILD_TYPE "CMake build type" "Debug")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# C Compiler
set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")

# Floating point settings
if(NOT RCAR_SET_FPU STREQUAL "hard")
    set(RCAR_FLOAT_ABI "soft")
    set(RCAR_FPU "fpv4-sp-d16")
else()
    set(RCAR_FLOAT_ABI "hard")
    set(RCAR_FPU "vfpv3-d16")
endif()

# Debug build
set(CMAKE_C_FLAGS_Debug "-Wall -Wextra -MD -MP -MF -g -O0 -mfloat-abi=${RCAR_FLOAT_ABI}  -mfpu=${RCAR_FPU}  -mcpu=${RCAR_PROCESSOR} -ffreestanding -std=gnu99")

# Release build
set(CMAKE_C_FLAGS_Release "-MD -MP -MF -g -O3 -mfloat-abi=${RCAR_FLOAT_ABI}  -mfpu=${RCAR_FPU}  -mcpu=${RCAR_PROCESSOR} -ffreestanding -std=gnu99")

# Release with Debug Info
set(CMAKE_C_FLAGS_RelWithDebInfo ${CMAKE_C_FLAGS_Release})

# ASM flags
set(CMAKE_ASM_FLAGS "-mfloat-abi=${RCAR_FLOAT_ABI} -mfpu=${RCAR_FPU}  -mcpu=${RCAR_PROCESSOR}")

# Set C Flags based on build type
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}})
