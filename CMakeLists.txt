cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(PySubnetTree C CXX)
include(cmake/CommonCMakeConfig.cmake)

########################################################################
## Dependency Configuration

find_package(SWIG 1.3.30 REQUIRED)
list(APPEND Python_ADDITIONAL_VERSIONS 3)
find_package(Python 3.5 REQUIRED COMPONENTS Interpreter Development)

include_directories(BEFORE ${Python_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

########################################################################
## Build Python Extension

cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

include(UseSWIG)

set_source_files_properties(SubnetTree.i PROPERTIES CPLUSPLUS true)
set(SWIG_MODULE_SubnetTree_EXTRA_DEPS include/SubnetTree.h)

swig_add_library(SubnetTree
                 LANGUAGE python
                 SOURCES SubnetTree.i SubnetTree.cc patricia.c)

swig_link_libraries(SubnetTree ${Python_LIBRARIES})
set_source_files_properties(${swig_generated_file_fullname} SubnetTree.cc
                            PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)

########################################################################
## Install Files

if (NOT PY_MOD_INSTALL_DIR)
    # the configure wrapper was not used, default to "home" style installation
    set(PY_MOD_INSTALL_DIR lib/python)
endif ()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SubnetTree.py
        DESTINATION ${PY_MOD_INSTALL_DIR})

install(TARGETS SubnetTree
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

include(UserChangedWarning)
