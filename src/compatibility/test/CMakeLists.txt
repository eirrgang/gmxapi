add_executable(test_basicmapping basicmapping.cpp)


# RPATH management
#set(PYGMX_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}" CACHE PATH
#    "Set to override RPATH for Python C++ extension (as by setuptools)")
#mark_as_advanced(PYGMX_RPATH)
# don't skip the full RPATH for the build tree
set_target_properties(test_basicmapping PROPERTIES SKIP_BUILD_RPATH FALSE)

# (but later on when installing)
# If building with setuptools, CMake will not be performing the install
set_target_properties(test_basicmapping PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
#set_target_properties(test_basicmapping PROPERTIES INSTALL_RPATH ${PYGMX_RPATH})

target_link_libraries(test_basicmapping PRIVATE Gromacs::gmxapi)
#target_link_libraries(test_basicmapping PRIVATE gmxapicompat Gromacs::gmxapi)

if(UNIX AND NOT APPLE)
    set_target_properties(test_basicmapping PROPERTIES INSTALL_RPATH "$ORIGIN/data/gromacs/${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
    set_target_properties(test_basicmapping PROPERTIES INSTALL_RPATH "@loader_path/data/gromacs/${CMAKE_INSTALL_LIBDIR}")
endif()

set_target_properties(test_basicmapping PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
