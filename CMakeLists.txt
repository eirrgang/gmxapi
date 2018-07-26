cmake_minimum_required(VERSION 3.4.3)

# CMake modules/macros are in a subdirectory to keep this file cleaner
# This needs to be set before project() in order to pick up toolchain files
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# skbuild sets OS X deployment target to 10.6 for compatibility, but we want to require 10.9 and C++11 compatibility.
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE STRING "OS X deployment target below 10.9 does not use C++11 standard library" FORCE)

# Sets the PROJECT_VERSION variable, as well...
project(gmxpy VERSION 0.0.7)

# Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0054 NEW)
set(CMAKE_CXX_STANDARD 11)
include(Ccache.cmake)

# Since a user may have multiple virtual environments with different Python interpreters, it is generally confusing to
# have a package for a virtual environment installed in the user's default user site-packages directory. For setup.py
# installs, use the `--user` flag for `pip` or `setup.py`. When using CMake directly, use GMXAPI_USER_INSTALL=ON to
# override the default site-packages install directory with the user-specific location.

unset(PYTHONINTERP_FOUND)
unset(PYTHONLIBS_FOUND)
# This will get cleaner with FindPython in CMake 3.12 and higher. We're a ways from requiring that, but
# we can add logic for it soon to be ready.
unset(PYTHON_LIBRARY)
unset(PYTHON_INCLUDE_DIR)
find_package(PythonInterp)
if (PYTHONINTERP_FOUND)
    message(STATUS "Found Python interpreter: ${PYTHON_EXECUTABLE}")
    add_subdirectory(pybind11)
    find_package(PythonLibs)
    if (PYTHONLIBS_FOUND)
        message(STATUS "Python headers in ${PYTHON_INCLUDE_DIRS}")
        message(STATUS "Python libraries in ${PYTHON_LIBRARIES}")
        if(SKBUILD)
            message(STATUS "The project is built using scikit-build")
            # for hints on Python environment: find_package(PythonExtensions REQUIRED)
            #     https://scikit-build.readthedocs.io/en/latest/cmake-modules/PythonExtensions.html
            find_package(PythonExtensions REQUIRED)
            message(STATUS "scikit-build has found python installation at ${PYTHON_PREFIX}")
            # for testing: find_package(NumPy REQUIRED)
            #     https://scikit-build.readthedocs.io/en/latest/cmake-modules/NumPy.html
            message(STATUS "scikit-build has set CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
        else()
            if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
                # If the user is not in a virtual environment and is not a privileged user and has not specified an install location
                # for the Python module (GMXAPI_INSTALL_PATH), this option causes the automatic install location to query the user
                # site-packages directory instead of using the default site-packages directory for the interpreter.
                option(GMXAPI_USER_INSTALL "Override the default site-packages directory with the user-specific Python packages directory. \
                       (Do not use with virtual environments.) \
                       Has no effect if GMXAPI_INSTALL_PATH is defined or cached. \
                       Use -UGMXAPI_INSTALL_PATH to force recalculation." OFF)

                if (GMXAPI_USER_INSTALL)
                    execute_process(COMMAND ${PYTHON_EXECUTABLE} "-m" "site" "--user-site"
                                    OUTPUT_VARIABLE GMXAPI_DEFAULT_SITE_PACKAGES
                                    OUTPUT_STRIP_TRAILING_WHITESPACE)
                    message(STATUS "Python user site-packages directory is ${GMXAPI_DEFAULT_SITE_PACKAGES}")
                else()
                    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
                                    "import sys; import os; print(os.path.abspath(os.path.join(sys.prefix, 'lib', 'python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}', 'site-packages')))"
                                    OUTPUT_VARIABLE GMXAPI_DEFAULT_SITE_PACKAGES
                                    OUTPUT_STRIP_TRAILING_WHITESPACE)
                    message(STATUS "Python site-packages directory is ${GMXAPI_DEFAULT_SITE_PACKAGES}")
                endif(GMXAPI_USER_INSTALL)
                set(CMAKE_INSTALL_PREFIX ${GMXAPI_DEFAULT_SITE_PACKAGES} CACHE PATH
                    "Path to Python module install location (site-packages). For an automatically determined install location based on \
                    the Python installation, leave undefined or explicitly undefined with -UGMXAPI_INSTALL_PATH and, optionally, set \
                    GMXAPI_USER_INSTALL on or off to specify the installation's site-packages directory or the 'user' site-packages \
                    directory."
                    FORCE)
            endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        endif(SKBUILD)
    else(PYTHONLIBS_FOUND)
        message(FATAL_ERROR "Found Python interpreter ${PYTHON_EXECUTABLE} but this Python installation does not have
        developer tools."
                "Set PYTHON_EXECUTABLE to the Python interpreter that was installed with a working Python.h header file.")
    endif(PYTHONLIBS_FOUND)
else()
    message(FATAL_ERROR "Could not find Python interpreter. Set CMake flag -DPYTHON_EXECUTABLE=/path/to/python to
    hint.")
endif(PYTHONINTERP_FOUND)

add_subdirectory(src/gmx)

add_subdirectory(docs)
