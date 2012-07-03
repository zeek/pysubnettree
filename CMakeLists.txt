project(PySubnetTree C CXX)
cmake_minimum_required(VERSION 2.6.3 FATAL_ERROR)
include(cmake/CommonCMakeConfig.cmake)

########################################################################
## Dependency Configuration

include(FindRequiredPackage)

FindRequiredPackage(SWIG)
FindRequiredPackage(PythonInterp)
FindRequiredPackage(PythonDev)

if ( SWIG_VERSION AND PYTHON_VERSION_STRING AND
     SWIG_VERSION VERSION_LESS 1.3.30 AND
     PYTHON_VERSION_STRING VERSION_GREATER 2.4 )
    message("Found swig version: ${SWIG_VERSION}")
    message("Found python version: ${PYTHON_VERSION_STRING}")
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

########################################################################
## Build Python Extension

include(UseSWIG)

set_source_files_properties(SubnetTree.i PROPERTIES CPLUSPLUS true)
set(SWIG_MODULE_SubnetTree_EXTRA_DEPS SubnetTree.h)
swig_add_module(SubnetTree python SubnetTree.i SubnetTree.cc patricia.c)
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

include(UserChangedWarning)
