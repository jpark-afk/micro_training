###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2016
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# Top level CMakeLists.txt for building Micro
#
# Modification History
# --------------------
# 03may16 tk Written
################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

SET(HAS_TARGET_INCLUDE_DIR TRUE)
SET(HAS_STRING_TIMESTAMP TRUE)

#===============================================================================
#                                  SETUP
#===============================================================================


#== Determine if this is buildable source or the development tree
# For buildable source, the assumption is that the location of the
# CMakeLists.txt file is <some-path>/source/unix or <some-path>/source/windows
# For the development tree, the assumption is that the location if the
# CMakeLists.txt is rti/ndds_lite

FUNCTION(RTI_MESSAGE_STATUS msg)
    MESSAGE(STATUS "[RTI] ${msg}")
ENDFUNCTION(RTI_MESSAGE_STATUS)

FUNCTION(RTI_MESSAGE_FATAL msg)
    MESSAGE(FATAL_ERROR "[RTI] ${msg}")
ENDFUNCTION(RTI_MESSAGE_FATAL)

FUNCTION(RTI_MESSAGE_WARNING msg)
    MESSAGE(WARNING "[RTI] ${msg}")
ENDFUNCTION(RTI_MESSAGE_WARNING)

IF (${RTI_BUILD_UNITTESTS})
    enable_testing()
ENDIF()

RTI_MESSAGE_STATUS("Determine source-tree type")

IF (NOT DEFINED RTIME_FIND_JAVA)
    SET(RTIME_FIND_JAVA true)
ENDIF()

IF (DEFINED TARGET_ONLY)
    IF (${TARGET_ONLY})
        RTI_MESSAGE_STATUS("TARGET_ONLY defined")
        SET(RTI_SHARED_NO_COPY true)
        SET(RTIME_FIND_JAVA false)
    ENDIF()
ENDIF()

# This function is used instead of GET_FILENAME_COMPONENT since
# GET_FILENAME_COMPONENT is not supported in 2.8.4
FUNCTION(GET_DIRECTORY dir path)
    IF (${path} STREQUAL ".")
        SET(result ".")
    ELSEIF(${path} MATCHES "(.*)/.*$")
        SET(result ${CMAKE_MATCH_1})
    ENDIF()
    SET(${dir} ${result} PARENT_SCOPE)
ENDFUNCTION()

GET_DIRECTORY(RTIME_SOURCE_ROOT ${CMAKE_CURRENT_LIST_FILE})
GET_DIRECTORY(RTIME_ROOT ${RTIME_SOURCE_ROOT})

IF (NOT DEFINED RTIME_RESOURCE_ROOT)
    IF (IS_DIRECTORY ${RTIME_SOURCE_ROOT}/resource.1.0)
            SET(RTIME_DEVTREE TRUE)
            IF (NOT IS_DIRECTORY "${RTIME_SOURCE_ROOT}/../ndds_lite")
                SET(RTIME_SHIPPED_DEVTREE TRUE)
                RTI_MESSAGE_STATUS("Shipped development tree detected")
                IF ((IS_DIRECTORY "${RTIME_SOURCE_ROOT}/rti_me_cert.1.0")
                    OR RTIME_CERT_SOURCE)
                    RTI_MESSAGE_STATUS("Cert source detected")
                    IF (NOT DEFINED RTIME_CERT_SOURCE)
                        SET(RTIME_CERT_SOURCE TRUE)
                    ENDIF()
                    IF (NOT DEFINED RTIME_BUILD_SINGLE_LIB)
                        SET(RTIME_BUILD_SINGLE_LIB TRUE)
                    ENDIF()
                    IF (NOT DEFINED RTIME_DISABLE_BUILDID)
                        SET(RTIME_DISABLE_BUILDID TRUE)
                    ELSEIF(NOT RTIME_DISABLE_BUILDID)
                        RTI_MESSAGE_FATAL("RTIME_DISABLE_BUILDID must be TRUE for Cert builds")
                    ENDIF()
                    IF (NOT DEFINED RTIME_CERT)
                        SET(RTIME_CERT TRUE)
                    ENDIF()
                ENDIF()
            ELSE()
                RTI_MESSAGE_STATUS("Development tree detected")
            ENDIF()
            SET(RTIME_RESOURCE_ROOT ${RTIME_SOURCE_ROOT}/resource.1.0)
    ELSEIF(IS_DIRECTORY ${RTIME_SOURCE_ROOT}/resource)
            SET(RTIME_DEVTREE FALSE)
            RTI_MESSAGE_STATUS("Source-bundle detected")
            SET(RTIME_RESOURCE_ROOT ${RTIME_SOURCE_ROOT}/resource)
    ELSE()
        RTI_MESSAGE_FATAL("Could not determine if this is "
                          "buildable source or a development tree")
    ENDIF()
ENDIF()

IF (NOT IS_DIRECTORY ${RTIME_RESOURCE_ROOT}/cmake)
    RTI_MESSAGE_FATAL("${RTIME_RESOURCE_ROOT} is not a valid resource directory")
ENDIF()

RTI_MESSAGE_STATUS("Resource directory is ${RTIME_RESOURCE_ROOT}")

IF (NOT DEFINED RTIME_TARGET_NAME)
    GET_FILENAME_COMPONENT(RTIME_TARGET_NAME ${CMAKE_CURRENT_BINARY_DIR} NAME)
    RTI_MESSAGE_WARNING("RTIME_TARGET_NAME not set, using ${RTIME_TARGET_NAME}")
ENDIF()

SET(RTIME_TARGET_NAME ${RTIME_TARGET_NAME} CACHE STRING "RTIME target name")

IF (${RTIME_DEVTREE})
    ADD_DEFINITIONS(-DDEVTREE_BUILD=1)
ENDIF()

# For the RTI tree, get the GIT branch
IF (${RTIME_DEVTREE} AND (NOT RTIME_SHIPPED_DEVTREE))
    IF (NOT DEFINED RTI_SHARED_NO_COPY)
        SET(RTI_SHARED_NO_COPY false)
    ENDIF()
    IF (NOT DEFINED RTI_BUILD_BRANCH)
        FIND_PACKAGE(Git)
        IF (Git_NOT_FOUND)
            MESSAGE(FATAL_ERROR "Did not find a git executable")
        ENDIF()
        # Get the branch name
        EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} branch
                        OUTPUT_VARIABLE git_output
                        RESULT_VARIABLE result_code)
        SEPARATE_ARGUMENTS(git_output_list UNIX_COMMAND ${git_output})
        LIST(FIND git_output_list "*" index)
        MATH(EXPR index "${index} + 1")
        LIST(GET git_output_list ${index} RTI_BUILD_BRANCH)
    ENDIF()
    SET(tmp_branch ${RTI_BUILD_BRANCH})
    STRING(REPLACE "\\" "/" RTI_BUILD_BRANCH ${tmp_branch})
    RTI_MESSAGE_STATUS("current branch is [${RTI_BUILD_BRANCH}]")

    IF (NOT DEFINED RTIME_CMAKE_ROOT)
        # rti/build/<branch>/cmake/${architecture}
        SET(RTIME_CMAKE_ROOT ${RTIME_ROOT}/build/${RTI_BUILD_BRANCH}/cmake/cmake/${CMAKE_BUILD_TYPE}/${RTIME_TARGET_NAME})
    ENDIF()

    # rti/build/<branch>/
    IF (NOT DEFINED RTIME_BUILD_ROOT)
       SET(RTIME_BUILD_ROOT ${RTIME_ROOT}/build/${RTI_BUILD_BRANCH}/cmake)
    ENDIF()

    IF (NOT DEFINED RTIME_PRO_ROOT)
        SET(RTIME_PRO_ROOT ${RTIME_ROOT}/build/download/connext-pro)
    ENDIF()
ELSE()
    IF (NOT DEFINED RTIME_CMAKE_ROOT)
        # <source>/build/cmake
        SET(RTIME_CMAKE_ROOT ${RTIME_SOURCE_ROOT}/build/cmake/${CMAKE_BUILD_TYPE}/${RTIME_TARGET_NAME})
    ENDIF()

    IF (NOT DEFINED RTIME_BUILD_ROOT)
        # <source>/build
        SET(RTIME_BUILD_ROOT ${RTIME_SOURCE_ROOT}/build)
    ENDIF()

    IF (NOT DEFINED RTIME_PRO_ROOT)
        SET(RTIME_PRO_ROOT ${RTIME_SOURCE_ROOT}/build/download/connext-pro)
    ENDIF()
ENDIF()

#===============================================================================
# Define the CMake project
#===============================================================================
SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

IF (${RTIME_DEVTREE} AND (NOT RTIME_SHIPPED_DEVTREE))
    STRING(REPLACE "/" "-" branch_name ${RTI_BUILD_BRANCH})
    PROJECT(rtime-${branch_name}-${RTIME_TARGET_NAME} C CXX)
ELSE()
    PROJECT(rti_me-${RTIME_TARGET_NAME} C CXX)
ENDIF()

#INCLUDE(${CMAKE_TOOLCHAIN_FILE})

IF (${CMAKE_HOST_SYSTEM} MATCHES "Windows")
    SET(USERNAME $ENV{username})
ELSEIF(${CMAKE_HOST_SYSTEM} MATCHES "Darwin")
    SET(USERNAME $ENV{USER})
ELSEIF(${CMAKE_HOST_SYSTEM} MATCHES "Linux")
    SET(USERNAME $ENV{USER})
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

SET(CMAKE_XCODE_ATTRIBUTE_SYMROOT ${RTIME_CMAKE_ROOT})

SET(RESOURCEHOME ${RTIME_RESOURCE_ROOT})

INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_defaults.txt)
INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_library.txt)

#===============================================================================
#                                  OPTIONS
#===============================================================================

IF (DEFINED RTIME_TARGET)
    IF (${RTIME_TARGET} STREQUAL "self")
        INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/architectures/self.tc)
    ENDIF()
ELSE()
    INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/architectures/self.tc)
ENDIF()

IF (NOT ${RTIME_DEVTREE})
    IF (NOT IS_DIRECTORY "src/sec_core")
        SET(RTIME_TRUST_INCLUDE_BUILTIN false)
    ENDIF()
    IF (NOT IS_DIRECTORY "src/dds_security")
        SET(RTIME_TRUST_INCLUDE_INTERFACES false)
    ENDIF()
ENDIF()

# This needs to be included after the tool-chain file since it relies
# on flags there to set the correct options.
INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_options.txt)

# Add a definition to use the build id generated in a file instead of a
# hardcoded value.

IF (NOT RTIME_DISABLE_BUILDID)
	ADD_DEFINITIONS(-DRTIME_ENABLE_BUILDID=1)
ENDIF()

#===============================================================================
#                    TESTS (only for development tree)
#===============================================================================

IF (RTIME_CERT)
    SET(RTI_BUILD_UNITTESTS FALSE)
ENDIF()

#== Unittests (only for development tree)
IF (NOT DEFINED RTI_BUILD_UNITTESTS)
    IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        SET(RTI_BUILD_UNITTESTS TRUE)
    ELSEIF((NOT ${RTIME_DEVTREE}) OR DEFINED RTIME_SHIPPED_DEVTREE)
        SET(RTI_BUILD_UNITTESTS FALSE)
    ENDIF()
ENDIF()

IF (${RTIME_DEVTREE} AND (NOT DEFINED RTI_EXCLUDE_TESTSUITE) AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    SET(RTI_EXCLUDE_TESTSUITE FALSE)
ELSEIF((NOT ${RTIME_DEVTREE}) OR (DEFINED RTIME_SHIPPED_DEVTREE))
    SET(RTI_EXCLUDE_TESTSUITE TRUE)
ENDIF()

#===============================================================================
#                               CUSTOMIZATION
#===============================================================================

IF (EXISTS ${RTIME_SOURCE_ROOT}/personal.cmake)
    INCLUDE(${RTIME_SOURCE_ROOT}/personal.cmake)
ENDIF()

IF (RTI_USE_STAGE)
    SET(RTI_STAGE_HOME "${RTI_STAGE_ROOT}/unlicensed/rti_connext_dds-${RTI_STAGE_VERSION}")
    SET(RTI_SHARED_HOME "${RTI_STAGE_ROOT}/unlicensed/rti_connext_dds-${RTI_STAGE_VERSION}/connextdds-${RTI_STAGE_VERSION}-src")
ENDIF()

RTI_MESSAGE_STATUS("USER ...............................: ${USERNAME}")

SET(RTIME_PRINT_CONFIG TRUE)
# Load the options again to print the final settings
INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_options.txt)

RTI_MESSAGE_STATUS("RTIME_TARGET .......................: ${RTIME_TARGET}")
RTI_MESSAGE_STATUS("RTIME_TARGET_NAME ..................: ${RTIME_TARGET_NAME}")
RTI_MESSAGE_STATUS("RTIME_BUILD_ROOT ...................: ${RTIME_BUILD_ROOT}")
RTI_MESSAGE_STATUS("RTIME_RESOURCE_ROOT ................: ${RTIME_RESOURCE_ROOT}")
RTI_MESSAGE_STATUS("RTIME_SOURCE_ROOT ..................: ${RTIME_SOURCE_ROOT}")
RTI_MESSAGE_STATUS("RTIME_MODULE_EXPORT_ROOT ...........: ${RTIME_MODULE_EXPORT_ROOT}")
RTI_MESSAGE_STATUS("RTIME_MODULE_EXPORT_INCLUDE_ROOT ...: ${RTIME_MODULE_EXPORT_INCLUDE_ROOT}")
RTI_MESSAGE_STATUS("Build directory ....................: ${RTIME_CMAKE_ROOT}")

IF ((DEFINED RTI_BUILD_BRANCH) AND (NOT ${RTI_BUILD_BRANCH} STREQUAL ""))
    RTI_MESSAGE_STATUS("RTI_BUILD_BRANCH ...................: ${RTI_BUILD_BRANCH}")
ENDIF()

IF ((DEFINED RTIME_ENABLE_FACE_COMPLIANCE) AND (NOT ${RTIME_ENABLE_FACE_COMPLIANCE} STREQUAL ""))
    RTI_MESSAGE_STATUS("RTIME_ENABLE_FACE_COMPLIANCE .......: ${RTIME_ENABLE_FACE_COMPLIANCE}")
ENDIF()

IF (${RTIME_DEVTREE})
    IF (RTI_USE_STAGE)
        RTI_MESSAGE_STATUS("RTI_STAGE_ROOT .....................: ${RTI_STAGE_ROOT}")
        RTI_MESSAGE_STATUS("RTI_STAGE_HOME .....................: ${RTI_STAGE_HOME}")
        RTI_MESSAGE_STATUS("RTI_STAGE_VERSION ..................: ${RTI_STAGE_VERSION}")
    ELSE()
        IF (NOT DEFINED NDDSGENHOME)
                #RTI_MESSAGE_FATAL("NDDSGENHOME is not set")
        ENDIF()
        RTI_MESSAGE_STATUS("NDDSGENHOME ........................: ${NDDSGENHOME}")
    ENDIF()
    RTI_MESSAGE_STATUS("RTIDDSGEN ..........................: ${RTIDDSGEN}")

    IF (${RTI_BUILD_UNITTESTS} AND ${RTIME_FIND_JAVA})

        IF (POLICY CMP0054)
            CMAKE_POLICY(PUSH)
            CMAKE_POLICY(SET CMP0054 OLD)
        ENDIF()
        FIND_PACKAGE(Java COMPONENTS Runtime)
        IF (POLICY CMP0054)
            CMAKE_POLICY(POP)
        ENDIF()

        IF (NOT JAVA_FOUND)
            RTI_MESSAGE_FATAL("Unable to locate a Java run-time environment")
        ENDIF()

        GET_DIRECTORY(JRE_DIR ${Java_JAVA_EXECUTABLE})
        GET_DIRECTORY(JRE_DIR ${JRE_DIR})

        RTI_MESSAGE_STATUS("JRE is located in ..................: ${JRE_DIR}")

        SET(RTIDDSGEN_OPT "-jre" "${JRE_DIR}")
    ENDIF()
ENDIF()


#===============================================================================
#                                  MODULES
#===============================================================================
# With the buildable source, include the top-level include directory
# the autosar include directory and the top-level test include directory.
IF (NOT ${RTIME_DEVTREE})
    INCLUDE_DIRECTORIES(${RTIME_SOURCE_ROOT}/include)
    IF (RTIME_INCLUDE_AUTOSAR)
        INCLUDE_DIRECTORIES(${RTIME_SOURCE_ROOT}/include/autosar)
    ENDIF()
    IF (${RTI_BUILD_UNITTESTS})
        INCLUDE_DIRECTORIES(${RTIME_SOURCE_ROOT}/test/include)
    ENDIF()
ENDIF()

IF (${RTI_BUILD_UNITTESTS} AND (NOT RTIME_CERT_SOURCE))
    INCLUDE_MODULE(test.1.0)
ENDIF()

SET(MODULE_GENERATE)
SET(MODULE_LIST)

IF (RTIME_INCLUDE_AUTOSAR)
    INCLUDE_MODULE(autosar.1.0)
ENDIF()

IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    INCLUDE_MODULE(rtiddsgen.2.0)
ENDIF()

INCLUDE_MODULE(osapi.2.0)
INCLUDE_MODULE(reda.2.0)
INCLUDE_MODULE(db.2.0)
INCLUDE_MODULE(rt.1.0)
INCLUDE_MODULE(cdr.1.0)
INCLUDE_MODULE(netio.2.0)
INCLUDE_MODULE(rtps.2.0)

INCLUDE_MODULE(dds_c.2.0)

SET(LAST_MODULE wh_sm)

IF (RTIME_EXCLUDE_DPDE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPDE=0)
ELSE()
    ADD_DEFINITIONS(-DRTIME_HAVE_DPDE=1)
ENDIF()

IF (RTIME_EXCLUDE_DPSE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPSE=0)
ELSE()
    ADD_DEFINITIONS(-DRTIME_HAVE_DPSE=1)
ENDIF()

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (${RTIME_DEVTREE}  AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        INCLUDE_MODULE(netio_tintf.1.0)
        INCLUDE_MODULE(netio_proxy.1.0)
    ENDIF()
ENDIF()

INCLUDE_MODULE(rh_sm.1.0)
INCLUDE_MODULE(wh_sm.1.0)

IF (NOT RTIME_EXCLUDE_DPSE)
    INCLUDE_MODULE(disc_dpse.1.0)
    SET(LAST_MODULE disc_dpse)
ENDIF()
IF (NOT RTIME_EXCLUDE_DPDE)
    INCLUDE_MODULE(disc_dpde.1.0)
    SET(LAST_MODULE disc_dpde)
ENDIF()

IF (NOT RTIME_EXCLUDE_CPP)
    IF (NOT RTIME_TARGET_NAME MATCHES ".*VSTAR.*")
        INCLUDE_MODULE(dds_cpp.2.0)
    ENDIF()
ENDIF()

IF (RTIME_BUILD_SINGLE_LIB)
    IF (${RTIME_DEVTREE}  AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        INCLUDE_MODULE(netio_tintf.1.0)
        INCLUDE_MODULE(netio_proxy.1.0)
    ENDIF()
ENDIF()

IF ((NOT RTIME_EXCLUDE_DPDE) AND (NOT RTIME_EXCLUDE_DPSE) AND (NOT RTI_DEOS) AND (NOT RTIME_EXCLUDE_DT))
    IF (${RTIME_DEVTREE} AND ${RTI_BUILD_UNITTESTS} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        INCLUDE_MODULE(dt.1.0)
        IF (NOT ${RTI_EXCLUDE_TESTSUITE})
           INCLUDE_MODULE(testsuite.1.0)
           IF (NOT RTIME_EXCLUDE_CPP)
               INCLUDE_MODULE(testsuite_cpp.1.0)
           ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

IF (${RTIME_DEVTREE} AND RTIME_BUILD_SINGLE_LIB AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    # This is a work-around when building a single library
    FOREACH(target ${RTI_TEST_TARGETS})
        IF (${target} MATCHES "disc_")
            TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE
                       "${CMAKE_CURRENT_SOURCE_DIR}/netio_tintf.1.0/include")
            GET_TARGET_PROPERTY(targetlib ${target} LINK_LIBRARIES)
            LIST(INSERT targetlib 0 rti_me_netiotintf${RTI_LIB_SUFFIX_STATIC})
            SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_LIBRARIES "${targetlib}")
        ELSEIF(${target} MATCHES "dds_cpp")
            TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE
                       "${CMAKE_CURRENT_SOURCE_DIR}/netio_tintf.1.0/include")
            GET_TARGET_PROPERTY(targetlib ${target} LINK_LIBRARIES)
            LIST(INSERT targetlib 0 rti_me_cpp${RTI_LIB_SUFFIX_STATIC})
            SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_LIBRARIES "${targetlib}")
        ELSEIF(${target} MATCHES "test")
            TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE
                       "${CMAKE_CURRENT_SOURCE_DIR}/test.1.0/include")
            GET_TARGET_PROPERTY(targetlib ${target} LINK_LIBRARIES)
            LIST(INSERT targetlib 0 rti_me_test${RTI_LIB_SUFFIX_STATIC})
            SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_LIBRARIES "${targetlib}")
        ENDIF()
    ENDFOREACH()
    ADD_CUSTOM_TARGET(rtimetests ALL DEPENDS ${RTI_TEST_TARGETS})
ENDIF()

IF (EXISTS ${RTIME_RESOURCE_ROOT}/cmake/cmake_sloc.cmake)
    INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_sloc.cmake)
ENDIF()

IF (EXISTS ${RTIME_RESOURCE_ROOT}/cmake/cmake_libinfo.cmake)
    INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_libinfo.cmake)
ENDIF()


IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    ADD_CUSTOM_TARGET(generate
                      DEPENDS rtiddsgen ${MODULE_GENERATE})

    ADD_CUSTOM_TARGET(generate_tools
                      DEPENDS rtiddsgen )
ELSEIF(RTIME_SHIP_SOURCE)
    ADD_CUSTOM_TARGET(generate
                      DEPENDS ${MODULE_GENERATE})
ENDIF()
