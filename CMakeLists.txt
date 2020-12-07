cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(PySubnetTree C CXX)
include(cmake/CommonCMakeConfig.cmake)

########################################################################
## Dependency Configuration

include(FindRequiredPackage)

FindRequiredPackage(SWIG)
list(APPEND Python_ADDITIONAL_VERSIONS 3)
FindRequiredPackage(PythonInterp)
FindRequiredPackage(PythonDev)

if ( PYTHON_VERSION_STRING AND PYTHON_VERSION_STRING VERSION_LESS 3.5  )
    message("Found python version: ${PYTHON_VERSION_STRING}")
    message(FATAL_ERROR "Python 3.5 or greater is required.")
endif ()

if ( SWIG_VERSION AND SWIG_VERSION VERSION_LESS 1.3.30 )
    message("Found swig version: ${SWIG_VERSION}")
    message(FATAL_ERROR "Swig versions less than 1.3.30 are incompatible with "
                        "Python versions greater than or equal to 2.5, "
                        "upgrading your swig installation is recommended.")
endif ()

if (MISSING_PREREQS)
    foreach (prereq ${MISSING_PREREQ_DESCS})
        message(SEND_ERROR ${prereq})
    endforeach ()
    message(FATAL_ERROR "Configuration aborted due to missing prerequisites")
endif ()

include_directories(BEFORE ${PYTHON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

########################################################################
## Build Python Extension

if ( ${CMAKE_VERSION} VERSION_EQUAL 3.13 OR
     ${CMAKE_VERSION} VERSION_GREATER 3.13 )
    cmake_policy(SET CMP0078 NEW)
endif ()

if ( ${CMAKE_VERSION} VERSION_EQUAL 3.14 OR
     ${CMAKE_VERSION} VERSION_GREATER 3.14 )
    cmake_policy(SET CMP0086 NEW)
endif ()

include(UseSWIG)

set_source_files_properties(SubnetTree.i PROPERTIES CPLUSPLUS true)
set(SWIG_MODULE_SubnetTree_EXTRA_DEPS include/SubnetTree.h)

if ( CMAKE_VERSION VERSION_LESS 3.8.0 )
    swig_add_module(SubnetTree python SubnetTree.i SubnetTree.cc patricia.c)
else ()
    swig_add_library(SubnetTree
                     LANGUAGE python
                     SOURCES SubnetTree.i SubnetTree.cc patricia.c)
endif ()

swig_link_libraries(SubnetTree ${PYTHON_LIBRARIES})
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

if ( ${CMAKE_VERSION} VERSION_EQUAL 3.13 OR
     ${CMAKE_VERSION} VERSION_GREATER 3.13 )
    install(TARGETS SubnetTree
            DESTINATION ${PY_MOD_INSTALL_DIR})
else ()
    install(TARGETS ${SWIG_MODULE_SubnetTree_REAL_NAME}
            DESTINATION ${PY_MOD_INSTALL_DIR})
endif ()

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
