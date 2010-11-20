########################################################################
## CMake Configuration

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Prohibit in-source builds.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./configure to choose a build directory and "
                        "initialize the build configuration.")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                @ONLY)

add_custom_target(uninstall COMMAND
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

########################################################################
## Project/Build Configuration

project(PySubnetTree)

if (ENABLE_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else ()
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

########################################################################
## Dependency Configuration

include(FindRequiredPackage)

FindRequiredPackage(SWIG)
FindRequiredPackage(PythonInterp)
FindRequiredPackage(PythonLibs)

if (MISSING_PREREQS)
    foreach (prereq ${MISSING_PREREQ_DESCS})
        message(SEND_ERROR ${prereq})
    endforeach ()
    message(FATAL_ERROR "Configuration aborted due to missing prerequisites")
endif ()

include_directories(BEFORE ${PYTHON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

########################################################################
## Build Python Extension

include(UseSWIG)

set_source_files_properties(SubnetTree.i PROPERTIES CPLUSPLUS true)
swig_add_module(SubnetTree python SubnetTree.i SubnetTree.cc patricia.c)
swig_link_libraries(SubnetTree ${PYTHON_LIBRARIES})

########################################################################
## Install Files

# create the byte-compiled module for installation
set(compileArg "import py_compile; py_compile.compile('SubnetTree.py')")
install(CODE
    "execute_process(COMMAND ${PYTHON_EXECUTABLE} -c \"${compileArg}\"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})"
)

if (NOT PY_MOD_INSTALL_DIR)
    # the configure wrapper was not used, default to "home" style installation
    set(PY_MOD_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/python)
endif ()

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/SubnetTree.py
            ${CMAKE_CURRENT_BINARY_DIR}/SubnetTree.pyc
        DESTINATION ${PY_MOD_INSTALL_DIR})
install(TARGETS ${SWIG_MODULE_SubnetTree_REAL_NAME}
        DESTINATION ${PY_MOD_INSTALL_DIR})

########################################################################
## Build Summary

if (CMAKE_BUILD_TYPE)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BuildType)
endif ()

message(
    "\n===============|  PySubnetTree Build Summary  |================="
    "\n"
    "\nInstall dir:       ${PY_MOD_INSTALL_DIR}"
    "\nDebug mode:        ${ENABLE_DEBUG}"
    "\n"
    "\nCC:                ${CMAKE_C_COMPILER}"
    "\nCFLAGS:            ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BuildType}}"
    "\nCXX:               ${CMAKE_CXX_COMPILER}"
    "\nCXXFLAGS:          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BuildType}}"
    "\nCPP:               ${CMAKE_CXX_COMPILER}"
    "\n"
    "\n================================================================\n"
)
