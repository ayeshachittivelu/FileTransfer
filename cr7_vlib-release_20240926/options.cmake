include(CMakeDependentOption)

# Optional features for customizing build
# - Syntax explanation
#   option(<variable> "<help_text>" [value])
#   cmake_dependent_option(<option> "<help_text>" <value> <depends> <force>)
#  See CMake documentation: https://cmake.org/cmake/help/v3.15/
#
#  Using the following syntax, the options influence whether or not the macro is defined for compilation
#   $<$<BOOL:${<option name>}>:<macro>

# Ethernet specific options
option(ETHER_ENABLE "Enables MCAL Ethernet driver" OFF)
option(USE_ETHER_CA_ENABLE "Enables Ethernet driver exclusively for A5x" OFF)
option(USE_ETHER_CR "Enables Ethernet driver exclusively for R7" OFF)

# CIO specific options
# option(R_USE_CAN "Enables CIO CAN driver" OFF)
option(R_USE_CANFD "Enables CIO CAN-FD driver" ON)
cmake_dependent_option(R_USE_ETHER "Enables CIO Ethernet driver" OFF "ETHER_ENABLE" OFF)
option(R_USE_WM "Enables CIO Window Manager driver" ON)
option(R_USE_VIN "Enables CIO VIN driver" ON)
option(R_USE_AUDIO "Enables CIO Audio driver" OFF)

# Taurus options
cmake_dependent_option(R_TAURUS_ENABLE_RVGC "Enables Taurus RVGC feature (shared graphics)" ON "R_USE_WM" OFF)
cmake_dependent_option(R_TAURUS_ENABLE_CAMERA "Enables Taurus CAMERA feature (shared video input)" OFF "R_USE_VIN" OFF)
cmake_dependent_option(R_TAURUS_ENABLE_CAN "Enables Taurus CAN feature (shared CAN)" OFF "R_USE_CANFD" OFF)
cmake_dependent_option(R_TAURUS_ENABLE_ETHER "Enables Taurus Ethernet feature (shared Ethernet)" OFF "R_USE_ETHER;ETHER_ENABLE" OFF)
option(R_TAURUS_ENABLE_COMCH "Enables communication channel example from rich os to RTOS" ON)

# Display related options
cmake_dependent_option(USE_WIDE_HD_AUO "Enables support for AUO LVDS display (Renesas internal)" ON "R_USE_WM" OFF)
cmake_dependent_option(USE_HDMI "Enables HDMI display support" ON "R_USE_WM" OFF)
cmake_dependent_option(USE_HDMI0 "Use HDMI0 (logo only)" OFF "USE_HDMI" OFF)
cmake_dependent_option(USE_LVDS "Use LVDS" ON "R_USE_WM" OFF)

# Debug logging options
option(R_MEML_ENABLE_DEBUG "Enables MMngr debug prints" OFF)
option(R_RPMSG_ENABLE_DEBUG "Enables RPMsg debug prints" OFF)
option(R_DAVEHD_ERROR "Enables error logging for Dave-HD driver" ON)
option(R_DAVEHD_DEBUG "Enables info / debug loggin for Dave-HD driver" OFF)

# Other options
option(PRINT_CPU_IDLE "Enables periodic console output of CPU idle %" OFF)
option(LWIP_SUPPORTED "Enable LWIP" OFF)
option(RIVP_SUPPORTED "Enable RIVP" OFF)
option(DRW2D_ON_DHD "Enables DRW2D on D/AVE-HD GPU" ON)
cmake_dependent_option(DRW2D_ON_CPU "Enables DRW2D on CPU" OFF "NOT DRW2D_ON_DHD" OFF)

add_compile_definitions(
    RCAR_SALVATOR_XS=0
    R_TARGET_BOARD_SALVATORXS
    RCAR_EBISU=1
    RCAR_BOARD=1
    RENESAS_RCAR
    ${RCAR_TARGET_GUEST}
    RESOURCE_TABLE_BASE=0x70400000

    # The below definitions uses a special syntax for defining definitions
    #   If the corresponding option is set to OFF, the definition will not be used during compilation
    $<$<BOOL:${R_TAURUS_ENABLE_RVGC}>:R_TAURUS_ENABLE_RVGC>
    $<$<BOOL:${R_TAURUS_ENABLE_CAMERA}>:R_TAURUS_ENABLE_CAMERA>
    $<$<BOOL:${R_TAURUS_ENABLE_CAN}>:R_TAURUS_ENABLE_CAN>
    $<$<BOOL:${R_TAURUS_ENABLE_ETHER}>:R_TAURUS_ENABLE_ETHER>
    $<$<BOOL:${R_TAURUS_ENABLE_COMCH}>:R_TAURUS_ENABLE_COMCH>
    

    $<$<BOOL:${ETHER_ENABLE}>:ETHER_ENABLE>
    $<$<BOOL:${USE_ETHER_CA_ENABLE}>:USE_ETHER_CA_ENABLE>
    $<$<BOOL:${USE_ETHER_CR}>:USE_ETHER_CR>

    # $<$<BOOL:${R_USE_CAN}>:R_USE_CAN>
    $<$<BOOL:${R_USE_CANFD}>:R_USE_CANFD>
    $<$<BOOL:${R_USE_ETHER}>:R_USE_ETHER>
    $<$<BOOL:${R_USE_WM}>:R_USE_WM>
    $<$<BOOL:${R_USE_VIN}>:R_USE_VIN>
    $<$<BOOL:${R_USE_AUDIO}>:R_USE_AUDIO>
    $<$<BOOL:${R_MEML_ENABLE_DEBUG}>:R_MEML_ENABLE_DEBUG>

    $<$<BOOL:${USE_WIDE_HD_AUO}>:USE_WIDE_HD_AUO>
    $<$<BOOL:${USE_HDMI}>:USE_HDMI>
    $<$<BOOL:${USE_HDMI0}>:USE_HDMI0>
    $<$<BOOL:${USE_LVDS}>:USE_LVDS>

    $<$<BOOL:${R_DAVEHD_ERROR}>:R_DAVEHD_ERROR>
    $<$<BOOL:${R_DAVEHD_DEBUG}>:R_DAVEHD_DEBUG>

    $<$<BOOL:${DRW2D_ON_DHD}>:DRW2D_ON_DHD>
    $<$<BOOL:${DRW2D_ON_CPU}>:DRW2D_ON_CPU>

    $<$<BOOL:${PRINT_CPU_IDLE}>:PRINT_CPU_IDLE>
    $<$<BOOL:${LWIP_SUPPORTED}>:LWIP_SUPPORTED>
    $<$<BOOL:${RIVP_SUPPORTED}>:RIVP_SUPPORTED>
)
