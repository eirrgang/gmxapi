add_library(gmxapicompat INTERFACE)

#INTERFACE_LIBRARY targets may only have whitelisted properties.  The
#  property "POSITION_INDEPENDENT_CODE" is not allowed.
#set_target_properties(gmxapicompat PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(gmxapicompat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(gmxapi_VERSION_MAJOR EQUAL 0)
    if(gmxapi_VERSION_MINOR EQUAL 0)
        if(gmxapi_VERSION_PATCH EQUAL 7)
            target_compile_definitions(gmxapicompat INTERFACE GROMACS2019)
        elseif(gmxapi_VERSION_PATCH EQUAL 8)
            # We do not yet have compatibility differences to resolve for
            # gmxapi 0.0.8 developement spec
            target_compile_definitions(gmxapicompat INTERFACE GROMACS2019)
        endif()
    endif()
endif()

add_subdirectory(test)
