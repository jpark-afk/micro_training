###############################################################################
# Copyright (c) 2016-2025 Real-Time Innovations, Inc. All rights reserved.
#
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
IF (${CMAKE_VERSION} VERSION_LESS "3.17")
    SET(HAS_RM FALSE)
ELSE()
    SET(HAS_RM TRUE)
ENDIF()

#===============================================================================
#                                  SETUP
#===============================================================================

# Determine if this is buildable source or the development tree
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

FUNCTION(RTI_MESSAGE_VERBOSE msg)
    MESSAGE(VERBOSE "[RTI] ${msg}")
ENDFUNCTION(RTI_MESSAGE_VERBOSE)

RTI_MESSAGE_STATUS("Determine source-tree type")

IF (NOT DEFINED RTIME_FIND_JAVA)
    IF (NOT DEFINED ENV{JREHOME} AND NOT DEFINED ENV{JDKHOME})
        SET(RTIME_FIND_JAVA true)
    ELSE()
        SET(RTIME_FIND_JAVA false)
    ENDIF()
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
    IF (IS_DIRECTORY "${RTIME_SOURCE_ROOT}/resource.1.0" OR
        IS_SYMLINK "${RTIME_SOURCE_ROOT}/resource.1.0")
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
    ELSEIF(IS_DIRECTORY "${RTIME_SOURCE_ROOT}/resource" OR
           IS_SYMLINK "${RTIME_SOURCE_ROOT}/resource.1.0")
        SET(RTIME_DEVTREE FALSE)
        RTI_MESSAGE_STATUS("Source-bundle detected")
        SET(RTIME_RESOURCE_ROOT ${RTIME_SOURCE_ROOT}/resource)
    ELSE()
        RTI_MESSAGE_FATAL("Could not determine if this is "
                          "buildable source or a development tree")
    ENDIF()
ENDIF()

IF (NOT IS_DIRECTORY ${RTIME_RESOURCE_ROOT}/cmake AND
    NOT IS_SYMLINK "${RTIME_RESOURCE_ROOT}/cmake")
    RTI_MESSAGE_FATAL("${RTIME_RESOURCE_ROOT} is not a valid resource directory")
ENDIF()

RTI_MESSAGE_STATUS("Resource directory is ${RTIME_RESOURCE_ROOT}")

IF (NOT DEFINED RTIME_TARGET_NAME)
    GET_FILENAME_COMPONENT(RTIME_TARGET_NAME ${CMAKE_CURRENT_BINARY_DIR} NAME)
    RTI_MESSAGE_WARNING("RTIME_TARGET_NAME not set, using ${RTIME_TARGET_NAME}")
ENDIF()

SET(RTIME_TARGET_NAME ${RTIME_TARGET_NAME} CACHE STRING "RTIME target name")

# For the RTI tree, get the GIT branch
IF (${RTIME_DEVTREE} AND (NOT RTIME_SHIPPED_DEVTREE))
    IF (NOT DEFINED RTI_SHARED_NO_COPY)
        SET(RTI_SHARED_NO_COPY false)
    ENDIF()
    IF (NOT DEFINED RTI_BUILD_BRANCH OR NOT DEFINED RTI_COMMIT_ID)
        FIND_PACKAGE(Git)
        IF (Git_NOT_FOUND)
            MESSAGE(FATAL_ERROR "Did not find a git executable")
        ENDIF()
    ENDIF()
    IF (NOT DEFINED RTI_BUILD_BRANCH)
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

    IF (NOT DEFINED RTI_COMMIT_ID)
        EXECUTE_PROCESS(COMMAND  ${GIT_EXECUTABLE} rev-parse --short HEAD
                        OUTPUT_VARIABLE tmp_commit_id
                        RESULT_VARIABLE result_code)

        if (NOT ${result_code} EQUAL 0)
            RTI_MESSAGE_FATAL("failed to get Git commit-id: [${GIT_EXECUTABLE} rev-parse --short HEAD] returned ${result_code}")
        endif()
        STRING(REPLACE "\n" "" RTI_COMMIT_ID ${tmp_commit_id})
    ENDIF()

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

    IF (NOT DEFINED RTIME_MAG_ROOT)
        SET(RTIME_MAG_ROOT ${RTIME_ROOT}/build/download/connext-mag)
    ENDIF()

    IF (NOT DEFINED RTIME_BUNDLE_ROOT)
       SET(RTIME_BUNDLE_ROOT ${RTIME_ROOT}/build/${RTI_BUILD_BRANCH}/tests)
    ENDIF()

    IF (NOT DEFINED RTIME_QUACK_TEST_ROOT)
       SET(RTIME_QUACK_TEST_ROOT ${RTIME_ROOT}/build/${RTI_BUILD_BRANCH}/quack)
    ENDIF()

    IF (NOT DEFINED RTIME_THIRDPARTY_ROOT)
        SET(RTIME_THIRDPARTY_ROOT ${RTIME_ROOT}/build/download/thirdparty)
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

    IF (NOT DEFINED RTIME_MAG_ROOT)
        SET(RTIME_MAG_ROOT ${RTIME_SOURCE_ROOT}/build/download/connext-mag)
    ENDIF()

    IF (NOT DEFINED RTIME_BUNDLE_ROOT)
       SET(RTIME_BUNDLE_ROOT ${RTIME_ROOT}/tests)
    ENDIF()

    IF (NOT DEFINED RTIME_QUACK_TEST_ROOT)
       SET(RTIME_QUACK_TEST_ROOT ${RTIME_ROOT}/tests)
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

IF (${CMAKE_HOST_SYSTEM} MATCHES "Windows")
    SET(USERNAME $ENV{username})
ELSEIF(${CMAKE_HOST_SYSTEM} MATCHES "Darwin")
    SET(USERNAME $ENV{USER})
ELSEIF(${CMAKE_HOST_SYSTEM} MATCHES "Linux")
    SET(USERNAME $ENV{USER})
ENDIF()

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

#== Unittests (only for development tree)
IF (${RTIME_DEVTREE} AND (NOT DEFINED RTI_BUILD_UNITTESTS) AND (NOT DEFINED RTIME_SHIPPED_DEVTREE) AND (NOT DEFINED RTIME_CERT))
    SET(RTI_BUILD_UNITTESTS TRUE)
ELSEIF((NOT ${RTIME_DEVTREE}) OR (DEFINED RTIME_SHIPPED_DEVTREE) OR (DEFINED RTIME_CERT AND ${RTIME_CERT}))
    SET(RTI_BUILD_UNITTESTS FALSE)
ENDIF()
IF (RTI_BUILD_UNITTESTS)
    ADD_DEFINITIONS(-DRTI_BUILD_UNITTESTS=1)
ENDIF()

IF (${RTIME_DEVTREE} AND (NOT DEFINED RTI_EXCLUDE_TESTSUITE) AND (NOT DEFINED RTIME_SHIPPED_DEVTREE) AND (NOT DEFINED RTIME_CERT))
    SET(RTI_EXCLUDE_TESTSUITE FALSE)
    ELSEIF((NOT ${RTIME_DEVTREE}) OR (DEFINED RTIME_SHIPPED_DEVTREE) OR (DEFINED RTIME_CERT AND ${RTIME_CERT}))
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
    SET(RTI_STAGE_MAG_HOME "${RTI_STAGE_MAG_ROOT}/unlicensed/rti_connext_dds-${RTI_STAGE_MAG_VERSION}")
ENDIF()

RTI_MESSAGE_STATUS("USER ...............................: ${USERNAME}")

SET(RTIME_PRINT_CONFIG TRUE)
# Load the options again to print the final settings
INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_options.txt)

string(TOUPPER ${CMAKE_BUILD_TYPE} RTIME_BUILD_TYPE)

RTI_MESSAGE_STATUS("RTIME_BUILD_TYPE ...................: ${RTIME_BUILD_TYPE}")
RTI_MESSAGE_STATUS("RTIME_TARGET      ..................: ${RTIME_TARGET}")
RTI_MESSAGE_STATUS("RTIME_TOOLCHAIN_FILE ...............: ${CMAKE_TOOLCHAIN_FILE}")
RTI_MESSAGE_STATUS("RTIME_TARGET_NAME ..................: ${RTIME_TARGET_NAME}")
RTI_MESSAGE_STATUS("RTIME_BUILD_ROOT ...................: ${RTIME_BUILD_ROOT}")
RTI_MESSAGE_STATUS("RTIME_BUNDLE_ROOT ..................: ${RTIME_BUNDLE_ROOT}")
RTI_MESSAGE_STATUS("RTIME_QUACK_TEST_ROOT ..............: ${RTIME_QUACK_TEST_ROOT}")
RTI_MESSAGE_STATUS("RTIME_RESOURCE_ROOT ................: ${RTIME_RESOURCE_ROOT}")
RTI_MESSAGE_STATUS("RTIME_SOURCE_ROOT ..................: ${RTIME_SOURCE_ROOT}")
RTI_MESSAGE_STATUS("RTIME_MODULE_EXPORT_ROOT ...........: ${RTIME_MODULE_EXPORT_ROOT}")
RTI_MESSAGE_STATUS("RTIME_MODULE_EXPORT_INCLUDE_ROOT ...: ${RTIME_MODULE_EXPORT_INCLUDE_ROOT}")
RTI_MESSAGE_STATUS("Build directory ....................: ${RTIME_CMAKE_ROOT}")
IF (DEFINED RTIME_TARGET_PSL)
RTI_MESSAGE_STATUS("Platform Support Library  ..........: ${RTIME_TARGET_PSL}")
ELSEIF(DEFINED RTIME_PLATFORM_PSL)
RTI_MESSAGE_STATUS("Platform Support Library  ..........: ${RTIME_PLATFORM_PSL}")
ENDIF()
RTI_MESSAGE_STATUS("CMAKE_C_FLAGS ......................: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${RTIME_BUILD_TYPE}}")
RTI_MESSAGE_STATUS("CMAKE_CXX_FLAGS ....................: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${RTIME_BUILD_TYPE}}")

IF ((DEFINED RTI_BUILD_BRANCH) AND (NOT ${RTI_BUILD_BRANCH} STREQUAL ""))
    RTI_MESSAGE_STATUS("RTI_BUILD_BRANCH ...................: ${RTI_BUILD_BRANCH}")
    RTI_MESSAGE_STATUS("RTI_COMMIT_ID ......................: ${RTI_COMMIT_ID}")
ENDIF()
IF (${RTIME_DEVTREE})
    IF (RTI_USE_STAGE)
        RTI_MESSAGE_STATUS("RTI_STAGE_ROOT .....................: ${RTI_STAGE_ROOT}")
        RTI_MESSAGE_STATUS("RTI_STAGE_HOME .....................: ${RTI_STAGE_HOME}")
        RTI_MESSAGE_STATUS("RTI_SHARED_HOME ....................: ${RTI_SHARED_HOME}")
        RTI_MESSAGE_STATUS("RTI_STAGE_VERSION ..................: ${RTI_STAGE_VERSION}")
        IF (NDDSMAGHOME)
            RTI_MESSAGE_WARNING("NDDSMAGHOME set by user. Will ignore default")
            RTI_MESSAGE_STATUS("NDDSMAGHOME.........................: ${NDDSMAGHOME}")
        ENDIF()
    ELSE()
        RTI_MESSAGE_STATUS("RTI_SHARED_HOME ....................: ${RTI_SHARED_HOME}")
        RTI_MESSAGE_STATUS("NDDSGENHOME ........................: ${NDDSGENHOME}")
        RTI_MESSAGE_STATUS("NDDSMAGHOME.........................: ${NDDSMAGHOME}")
    ENDIF()
    RTI_MESSAGE_STATUS("RTIDDSPRO ..........................: ${CONNEXT_ARTIFACTORY_ROOT}")
    RTI_MESSAGE_STATUS("RTIDDSGEN ..........................: ${RTIDDSGEN}")
    RTI_MESSAGE_STATUS("RTIDDSMAG ..........................: ${RTIDDSMAG}")

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
SET(RTI_TARGET_BUNDLE_ROOT ${RTIME_BUNDLE_ROOT}/${RTIME_TARGET_NAME}/${RTI_BUNDLE_TYPE})
SET(RTI_TARGET_BUNDLE_ROOT ${RTIME_BUNDLE_ROOT}/${RTIME_TARGET_NAME}/${RTI_BUNDLE_TYPE})

IF (${RTIME_DEVTREE})
    IF (RTIME_UTEST_QUACK_ENABLE)
        process_resource_bundle_module(${RTIME_SOURCE_ROOT}/resource.1.0
                                       ${RTI_TARGET_BUNDLE_ROOT} static "")
        LIST(APPEND TEST_BUNDLE_RESOURCE_FILE ${return_value})
        ADD_CUSTOM_TARGET(bundle_common
                          DEPENDS ${TEST_BUNDLE_RESOURCE_FILE})
    ENDIF()
ENDIF()

# With the buildable source, only include the top-level include directory
IF (NOT ${RTIME_DEVTREE})
    INCLUDE_DIRECTORIES(
            ${RTIME_SOURCE_ROOT}/include/rti_me
    )
ENDIF()

SET(MODULE_GENERATE)
SET(MODULE_LIST)
SET(RTI_TEST_TARGETS)

if (${RTIME_DEVTREE})
    INCLUDE_MODULE(rtiddsgen.2.0)
    INCLUDE_MODULE(rtiddsmag.1.0)
    INCLUDE_MODULE(rtiddspro.1.0)
    ADD_SUBDIRECTORY(thirdparty.1.0)
 endif()

IF (NOT RTIME_PIL)
    IF (${RTIME_DEVTREE} AND (${RTI_BUILD_UNITTESTS} OR NOT RTIME_EXCLUDE_QTS))
        INCLUDE_MODULE(test.1.0)
    ENDIF()
ELSE()
    INCLUDE_DIRECTORIES("test.1.0/include")
ENDIF()

SET(MODULE_GENERATE)
SET(MODULE_LIST)
SET(RTI_TEST_TARGETS)

# The check for db.2.0 is to test if the full source is available or not.
# if not available, it is assumed that only the PSL source is available.
IF (EXISTS "${RTIME_SOURCE_ROOT}/db.2.0" OR
    EXISTS "${RTIME_SOURCE_ROOT}/src/db" OR
    IS_SYMLINK "${RTIME_SOURCE_ROOT}/db.2.0" OR
    IS_SYMLINK "${RTIME_SOURCE_ROOT}/src/db")
SET(RTIME_FULL_SOURCE TRUE)
# The API for the OSAPI module is needed to build the PSL layer.
INCLUDE_PSL(rti_me_psl.2.0 OSAPI osapi.2.0)
INCLUDE_MODULE(osapi.2.0)
INCLUDE_MODULE(reda.2.0)
INCLUDE_MODULE(db.2.0)
INCLUDE_MODULE(rt.1.0)
INCLUDE_MODULE(cdr.1.0)
INCLUDE_MODULE(netio.2.0)
INCLUDE_MODULE(rtps.2.0)
INCLUDE_MODULE(netio_dgram.1.0)
IF (NOT RTIME_EXCLUDE_SHMEM)
    INCLUDE_MODULE_API(netio_shmem.1.0)
ENDIF()
IF (RTIME_INCLUDE_ZCOPY_V2)
    INCLUDE_MODULE_API(netio_zcopy.1.0)
ENDIF()
IF (NOT RTIME_PSL AND NOT RTIME_PIL)
        INCLUDE_PSL(rti_me_psl.2.0 NETIO)
ENDIF()
IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE) AND ${RTI_BUILD_UNITTESTS})
        INCLUDE_MODULE(netio_tintf.1.0)
    ENDIF()
ENDIF()
INCLUDE_MODULE_API(xcdr.1.0)
INCLUDE_MODULE(dds_c.2.0)
IF ((RTIME_PSL OR RTIME_PIL) AND NOT RTIME_BUILD_SINGLE_LIB)
    INCLUDE_PSL(rti_me_psl.2.0 NETIO)
ENDIF()

IF (NOT RTIME_EXCLUDE_SHMEM)
    INCLUDE_MODULE(netio_shmem.1.0)
    IF (NOT RTIME_DDS_EXCLUDE_XTYPES)
        INCLUDE_MODULE(netio_sdm.1.0)
    ENDIF()
ENDIF()

INCLUDE_MODULE_API(dds_security.1.0 ONLY_IF_EXISTS)

#Always build PSK-PSL (integrated and PSL) except when building PIL
IF (NOT RTIME_PIL)
    INCLUDE_PSL(rti_me_psl.2.0 PSK)
ENDIF()
INCLUDE_MODULE(dds_psk.1.0 ONLY_IF_EXISTS)
INCLUDE_MODULE(rh_sm.1.0)
INCLUDE_MODULE(wh_sm.1.0)
INCLUDE_MODULE(xcdr.1.0)
INCLUDE_MODULE(dds_filter.1.0)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE) AND ${RTI_BUILD_UNITTESTS})
        INCLUDE_MODULE(netio_proxy.1.0)
    ENDIF()
ENDIF()

IF (NOT RTIME_EXCLUDE_DPSE)
   INCLUDE_MODULE(disc_dpse.1.0)
    SET(LAST_MODULE disc_dpse)
ENDIF()

IF ((RTIME_PSL OR RTIME_PIL) AND RTIME_BUILD_SINGLE_LIB)
    INCLUDE_PSL(rti_me_psl.2.0 NETIO)
ENDIF()

IF (NOT RTIME_EXCLUDE_DPDE)
    INCLUDE_MODULE(disc_dpde.1.0)
    SET(LAST_MODULE disc_dpde)
ENDIF()

IF (RTIME_INCLUDE_ZCOPY_V2 OR RTI_MAKEDOC)
    INCLUDE_MODULE(netio_zcopy.1.0)
ENDIF()

IF (NOT RTIME_EXCLUDE_DPDE AND NOT RTIME_EXCLUDE_CPP AND NOT (RTIME_TARGET_NAME MATCHES ".*VSTAR.*"))
    INCLUDE_PSL(rti_me_psl.2.0 NETIOCPP)
    INCLUDE_MODULE(dds_cpp.2.0)
ENDIF()

INCLUDE_MODULE(dds_security.1.0 ONLY_IF_EXISTS)

IF (NOT RTIME_EXCLUDE_APPGEN)
    INCLUDE_MODULE(app_gen.1.0)
ENDIF()

IF (${RTIME_TRUST_INCLUDE_BUILTIN})
    INCLUDE_MODULE(sec_core.2.0)
ENDIF()

IF (RTIME_BUILD_SINGLE_LIB)
    IF (${RTIME_DEVTREE}  AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        INCLUDE_MODULE(netio_tintf.1.0)
        INCLUDE_MODULE(netio_proxy.1.0)
    ENDIF()
ENDIF()

IF (${RTIME_DEVTREE} AND ${RTI_BUILD_UNITTESTS} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    INCLUDE_MODULE(testsuite_c.1.0)
ENDIF()

IF ((NOT RTIME_EXCLUDE_DPDE) AND (NOT RTIME_EXCLUDE_DPSE) AND (NOT RTI_DEOS) AND (NOT RTIME_EXCLUDE_DT))
    IF (${RTIME_DEVTREE} AND ${RTI_BUILD_UNITTESTS} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
        INCLUDE_APPLICATION(dt.1.0)
        IF (NOT RTI_EXCLUDE_TESTSUITE)
            INCLUDE_APPLICATION(testsuite.1.0)
            IF (NOT RTIME_EXCLUDE_CPP AND (NOT RTIME_FLATDATA_DISABLE))
                INCLUDE_APPLICATION(testsuite_cpp.1.0)
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

IF (${RTIME_DEVTREE} AND (NOT RTIME_EXCLUDE_QTS) AND (NOT DEFINED RTIME_SHIPPED_DEVTREE) AND ${RTI_BUILD_UNITTESTS})
    INCLUDE_MODULE(testsuite_qts.1.0)
ENDIF()

IF (EXISTS ${RTIME_RESOURCE_ROOT}/cmake/cmake_sloc.cmake)
    INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_sloc.cmake)
ENDIF()

IF (EXISTS ${RTIME_RESOURCE_ROOT}/cmake/cmake_libinfo.cmake)
    INCLUDE(${RTIME_RESOURCE_ROOT}/cmake/cmake_libinfo.cmake)
ENDIF()

IF (${RTIME_DEVTREE} AND (NOT DEFINED RTIME_SHIPPED_DEVTREE))
    ADD_CUSTOM_TARGET(generate
                      DEPENDS rtiddsgen rtiddsmag rtiddspro ${MODULE_GENERATE})
    ADD_CUSTOM_TARGET(generate_tools
                      DEPENDS rtiddsgen rtiddsmag)
ELSEIF(RTIME_SHIP_SOURCE)
    ADD_CUSTOM_TARGET(generate
                      DEPENDS ${MODULE_GENERATE})
ENDIF()


IF (RTIME_DEVTREE)
    SET(DOC_TARGETS_HTML)
    SET(DOC_TARGETS_PDF)
    SET(DOC_TARGETS_PRODUCT)
    SET(EXAMPLE_TARGETS)

    INCLUDE(rti_me.2.0/CMakeLists.txt)
    INCLUDE(rti_me_security_sdk.1.0/CMakeLists.txt)
    ADD_CUSTOM_TARGET(doc_mg_html
        COMMAND make -C ${RTIME_SOURCE_ROOT}/rti_me.2.0/doc/MG html
                BUILDDIR=${RTIME_BUILD_ROOT}/rti_me.2.0/doc/mg
        COMMAND ${CMAKE_COMMAND} -E remove_directory
                ${RTIME_BUILD_ROOT}/rti_me.2.0/doc/mg/doctrees
        WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
    )
    LIST(APPEND DOC_TARGETS_HTML doc_mg_html)
    ADD_CUSTOM_TARGET(doc_html
        DEPENDS ${DOC_TARGETS_HTML}
    )
    ADD_CUSTOM_TARGET(doc_pdf
        DEPENDS ${DOC_TARGETS_PDF}
    )
    ADD_CUSTOM_TARGET(doc
        DEPENDS doc_html doc_pdf
    )
    ADD_CUSTOM_TARGET(examples DEPENDS ${EXAMPLE_TARGETS})
ENDIF()

ELSE()
    INCLUDE_PSL(rti_me_psl.2.0 OSAPI osapi.2.0)
    INCLUDE_PSL(rti_me_psl.2.0 NETIO)
    INCLUDE_PSL(rti_me_psl.2.0 NETIOCPP)
    INCLUDE_MODULE_API(dds_security.1.0 ONLY_IF_EXISTS)
    INCLUDE_PSL(rti_me_psl.2.0 PSK)
ENDIF()
