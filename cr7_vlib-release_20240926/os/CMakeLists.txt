add_library(os-def INTERFACE)

if(RCAR_TARGET_OS STREQUAL "freertos")
    add_subdirectory(freertos)
    add_subdirectory(osal_wrapper)
    target_compile_definitions(os-def INTERFACE USE_FREERTOS)
elseif(RCAR_TARGET_OS STREQUAL "trampoline")
    # AutoSAR build chain is handled differently
    target_compile_definitions(os-def INTERFACE USE_AUTOSAR)
else()
    message(FATAL_ERROR "RCAR_TARGET_OS must be specified\n
        Supported: freertos, trampoline")
endif()
