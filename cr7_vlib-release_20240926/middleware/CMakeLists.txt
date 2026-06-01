add_subdirectory(WindowMng)
add_subdirectory(drw2d)
add_subdirectory(davehd)

if(RIVP_SUPPORTED)
    add_subdirectory(rivp)
endif()

if(LWIP_SUPPORTED)
    add_subdirectory(lwip)
endif()