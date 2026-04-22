###############################################################################
# Copyright (c) 2017-2025 Real-Time Innovations, Inc.  All rights reserved.    	
# Permission to modify and use for internal purposes granted.   	
# This software is provided "as is", without warranty, express or implied.
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

FUNCTION(GET_DIRECTORY dir path)
    IF (${path} STREQUAL ".")
        SET(result ".")
    ELSEIF(${path} MATCHES "(.*)/.*$")
        SET(result ${CMAKE_MATCH_1})
    ENDIF()
    SET(${dir} ${result} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(RTI_MESSAGE_STATUS msg)
    MESSAGE(STATUS "[RTI] ${msg}")
ENDFUNCTION(RTI_MESSAGE_STATUS)

IF (NOT DEFINED RTIMEHOME)
    IF (DEFINED ENV{RTIMEHOME})
        SET(RTIMEHOME $ENV{RTIMEHOME})
    ELSE()
        MESSAGE(FATAL_ERROR "RTIMEHOME must be defined or set as an environment variable.")
    ENDIF()
ENDIF()
MESSAGE(STATUS "[RTI] RTIMEHOME ..................: ${RTIMEHOME}")

IF (NOT DEFINED RTIME_TARGET_NAME)
    IF (DEFINED ENV{RTIME_TARGET_NAME})
        SET(RTIME_TARGET_NAME $ENV{RTIME_TARGET_NAME})
    ELSE()
        MESSAGE(FATAL_ERROR "RTIME_TARGET_NAME must be defined or set as an environment variable.")
    ENDIF()
ENDIF()
MESSAGE(STATUS "[RTI] RTIME_TARGET_NAME ..........: ${RTIME_TARGET_NAME}")

IF (NOT (DEFINED RTI_SHARED_LIB))
    SET(RTI_SHARED_LIB FALSE)
ENDIF()
MESSAGE(STATUS "[RTI] RTI_SHARED_LIB .............: ${RTI_SHARED_LIB}")

ADD_DEFINITIONS(-DUDP_ENABLE_TRANSFORMS=1 -DRTIME_UDP_ENABLE_TRANSFORMS=1)

LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(HelloWorld C)
SET(SOURCE_EXTENSION c)
set(RTIME_SECURE TRUE)

RTI_MESSAGE_STATUS("Looking for OpenSSL")
IF(DEFINED OPENSSLHOME)
    RTI_MESSAGE_STATUS("OPENSSLHOME defined using openssl from ${OPENSSLHOME}")
    SET(RTIME_OPENSSL_INCLUDE_DIR ${OPENSSLHOME}/release/include CACHE STRING "")
    SET(RTIME_OPENSSL_LIBRARY_DIR "${OPENSSLHOME}/release/lib" CACHE STRING  "")
    SET(RTIME_HAVE_OPENSSL true CACHE INTERNAL "")
    IF (NOT ${OPENSSL_VERSION} VERSION_GREATER "1.0")
        RTI_MESSAGE_STATUS("RTI Connext Micro requires OpenSSL comptabile with version 1.1.1")
    ENDIF()
ELSE()
    RTI_MESSAGE_STATUS(" ..........: find OpenSSL")
    FIND_PACKAGE(OpenSSL)
    IF (OPENSSL_FOUND)
        IF (NOT ${OPENSSL_VERSION} VERSION_GREATER "1.0")
            RTI_MESSAGE_STATUS("RTI Connext Micro requires OpenSSL comptabile with version 1.1.1, looking for RTIME_OPENSSL_INCLUDE_DIR and RTIME_OPENSSL_LIBRARY_DIR")
        ELSE()
            RTI_MESSAGE_STATUS("Using locally installed OpenSSL")
            GET_DIRECTORY(dir ${OPENSSL_CRYPTO_LIBRARY})
            SET(RTIME_OPENSSL_LIBRARY_DIR ${dir} CACHE STRING "")
            SET(RTIME_OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR} CACHE STRING  "")
            SET(RTIME_HAVE_OPENSSL true CACHE INTERNAL "")
        ENDIF()
    ENDIF()
ENDIF()

IF (NOT ${RTIME_HAVE_OPENSSL})
    RTI_MESSAGE_FATAL("Did not find openssl")
ENDIF()

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    SET(PLATFORM_LIBS dl m pthread rt)
    ADD_DEFINITIONS(-DRTI_LINUX)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    SET(PLATFORM_LIBS dl m pthread )
    ADD_DEFINITIONS(-DRTI_DARWIN)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(PLATFORM_LIBS netapi32.lib advapi32.lib user32.lib winmm.lib WS2_32.lib)
    ADD_DEFINITIONS(-DRTI_WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    IF (RTI_USE_NMAKE OR (${CMAKE_GENERATOR} MATCHES "NMake"))
        string(APPEND CMAKE_EXE_LINKER_FLAGS " /defaultlib:libc.lib /defaultlib:msvcrt.lib /defaultlib:Winmm.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrtd.lib")
    ELSE()
        IF (${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug")
            string(APPEND CMAKE_EXE_LINKER_FLAGS " /NODEFAULTLIB:MSVCRT.lib")
        ENDIF()
        add_compile_options(
            $<$<CONFIG:>:/MD>
            $<$<CONFIG:Debug>:/MDd>
            $<$<CONFIG:Release>:/MD>
        )
    ENDIF()
ENDIF()

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LIST(INSERT PLATFORM_LIBS 0 libssl${RTI_LIB_SUFFIX_SSL} libcrypto${RTI_LIB_SUFFIX_SSL})
ELSE()
    LIST(INSERT PLATFORM_LIBS 0 ssl${RTI_LIB_SUFFIX_SSL} crypto${RTI_LIB_SUFFIX_SSL})
ENDIF()

SET(RTIDDSGEN ${RTIMEHOME}/rtiddsgen/scripts/rtiddsgen)
SET(RTIDDSMAG ${RTIMEHOME}/rtiddsmag/scripts/rtiddsmag)

SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

SET(IDL_SOURCE_C HelloWorld.idl)

SET(IDL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(XML_PATH ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})

IF (CMAKE_GENERATOR MATCHES "Visual Studio")
    SET(COMMAND_PREFIX call)
ELSE()
    SET(COMMAND_PREFIX)
ENDIF()

IF (RTIME_CERT)
    ADD_DEFINITIONS(-DRTI_CERT)
ENDIF()

IF (RTIME_LINK_ZCV2)
    ADD_DEFINITIONS(-DRTIME_HAVE_ZCOPYV2=1)
ENDIF()

SET(RTI_LIB_PREFIX rti_me)
SET(RTI_LIB_SUFFIX)
SET(RTI_LIB_SUFFIX_SSL)

IF (NOT RTI_SHARED_LIB)
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "z")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "zd")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
    SET(RTI_LIB_SUFFIX_SSL "z")
ELSE()
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "d")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()

    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        ADD_DEFINITIONS(-DRTIME_DLL_VARIABLE)
    ENDIF()
ENDIF()

SET(MICRO_RTIME_LIB rti_me_ddsxtypes${RTI_LIB_SUFFIX} rti_me${RTI_LIB_SUFFIX})
SET(MICRO_C_LIBS)

IF (NOT RTIME_CERT)
    IF (DEFINED RTIME_MAG_FILES)
        SET(MICRO_APPGEN_LIB rti_me_appgen${RTI_LIB_SUFFIX})
    ENDIF()

    SET(MICRO_C_LIBS ${MICRO_APPGEN_LIB}

    )

    IF (RTIME_LINK_SHMEM_LIBS)
        LIST(APPEND MICRO_C_LIBS
            rti_me_netiosdm${RTI_LIB_SUFFIX}
            rti_me_netioshmem${RTI_LIB_SUFFIX}
        )
    ENDIF()

    IF (RTIME_LINK_ZCV2)
        LIST(APPEND MICRO_C_LIBS
            rti_me_netiosdm${RTI_LIB_SUFFIX}
            rti_me_netioshmem${RTI_LIB_SUFFIX}
            rti_me_netiozcopy${RTI_LIB_SUFFIX}
        )
    ENDIF()

    LIST(APPEND MICRO_C_LIBS
        rti_me_rhsm${RTI_LIB_SUFFIX}
        rti_me_whsm${RTI_LIB_SUFFIX}
        rti_me_discdpse${RTI_LIB_SUFFIX}
        rti_me_discdpde${RTI_LIB_SUFFIX}
            )
ENDIF()

LIST(LENGTH MICRO_C_LIBS list_length)
IF (list_length GREATER 0)
    LIST(REMOVE_DUPLICATES MICRO_C_LIBS)
ENDIF()

INCLUDE_DIRECTORIES(
    ${RTIMEHOME}/include
    ${RTIMEHOME}/include/rti_me
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${RTIME_OPENSSL_INCLUDE_DIR} 
)

LINK_DIRECTORIES(
    ${RTIMEHOME}/lib/${RTIME_TARGET_NAME}
    ${RTIME_OPENSSL_LIBRARY_DIR}
)

STRING(REGEX MATCH "(.*)-(.*)" pil_match ${RTIME_TARGET_NAME})
IF (pil_match)
    SET(RTIME_PIL_ARCH ${CMAKE_MATCH_1})
    SET(MICRO_NETIOPSL_LIBS rti_me_netiopsl${RTI_LIB_SUFFIX})
    SET(MICRO_OSPSL_LIBS rti_me_ospsl${RTI_LIB_SUFFIX})
    LINK_DIRECTORIES(${RTIMEHOME}/lib/${RTIME_PIL_ARCH})
    GET_PROPERTY(cppdef DIRECTORY PROPERTY COMPILE_DEFINITIONS)
ELSE()
    SET(MICRO_NETIOPSL_LIBS)
    SET(MICRO_OSPSL_LIBS)
ENDIF()

FOREACH(idl ${IDL_SOURCE_C})
    GET_FILENAME_COMPONENT(filename ${idl} NAME)
    STRING(REGEX REPLACE "\\.idl" "" basename ${filename})
    LIST(APPEND IDL_GEN_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.${SOURCE_EXTENSION})

    LIST(APPEND IDL_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.h)

    IF (RTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE)
        IF (NOT EXISTS ${idl})
            MESSAGE(FATAL_ERROR "${idl} does not exist.")
        ELSEIF(IS_DIRECTORY ${idl})
            MESSAGE(FATAL_ERROR "${idl} is not a file.")
        ENDIF()

        SET(IDL_GEN_FILE_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.${SOURCE_EXTENSION})

        SET(IDL_GEN_FILE_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.h)

        ADD_CUSTOM_COMMAND(OUTPUT ${IDL_GEN_FILE_C} ${IDL_GEN_FILE_H}
                           COMMAND ${COMMAND_PREFIX} ${RTIDDSGEN} -language C
                                   -micro -verbosity 1 -update typefiles
                                   -d ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${IDL_PATH}/${idl}
                           DEPENDS ${IDL_PATH}/${idl}
                           COMMENT "Regeneration type support plugin from ${idl}"
                          )
        SET_SOURCE_FILES_PROPERTIES(${IDLIDL_GEN_FILE_C_GEN_C} ${IDL_GEN_FILE_H} PROPERTIES GENERATED 1)
    ENDIF()
ENDFOREACH()

IF (${RTIME_SECURE})
    ADD_CUSTOM_TARGET(security_material ALL
                      DEPENDS ${CMAKE_SOURCE_DIR}/security/ca/ca.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher_key.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber_key.pem
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.p7s
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.p7s
                              ${CMAKE_SOURCE_DIR}/security/xml/governance.p7s
                     )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem ${CMAKE_SOURCE_DIR}/security/ca/ca.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca
                       COMMAND "${CMAKE_COMMAND}" -E touch ca/index.txt
                       COMMAND echo 01 > ca/serial
                       COMMAND openssl genrsa -out ca/ca_key.pem 2048
                       COMMAND openssl req -new -key ca/ca_key.pem -out ca/ca.csr -config openssl_ca.cnf
                       COMMAND openssl x509 -req -days 365 -in ca/ca.csr -signkey ca/ca_key.pem -out ca/ca.pem
                       COMMENT "Regenerating Certificate Authority"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher.pem ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher_key.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca/certs
                       COMMAND openssl genrsa -out ca/certs/publisher_key.pem 2048
                       COMMAND openssl req -new -key ca/certs/publisher_key.pem -out ca/certs/publisher.csr -config openssl_publisher.cnf
                       COMMAND openssl ca -batch -config openssl_ca.cnf -in ca/certs/publisher.csr -out ca/certs/publisher.pem
                       COMMENT "Regenerating Publisher identity"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf ${CMAKE_SOURCE_DIR}/security/openssl_publisher.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber.pem ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber_key.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca/certs
                       COMMAND openssl genrsa -out ca/certs/subscriber_key.pem 2048
                       COMMAND openssl req -new -key ca/certs/subscriber_key.pem -out ca/certs/subscriber.csr -config openssl_subscriber.cnf
                       COMMAND openssl ca -batch -config openssl_ca.cnf -in ca/certs/subscriber.csr -out ca/certs/subscriber.pem
                       COMMENT "Regenerating Subscriber identity"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf ${CMAKE_SOURCE_DIR}/security/openssl_subscriber.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.p7s
                       COMMAND openssl cms -in xml/permissions_publisher.xml -out xml/permissions_publisher.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Publisher Permissions file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.p7s
                       COMMAND openssl cms -in xml/permissions_subscriber.xml -out xml/permissions_subscriber.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Subscriber Permissions file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/governance.p7s
                       COMMAND openssl cms -in xml/governance.xml -out xml/governance.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Governance file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/governance.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )
ENDIF()

ADD_EXECUTABLE(HelloWorld_publisher
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_publisher.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldEncryption.c
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldEncryption.h
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldUdpTransform.c
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldUdpTransform.h
                ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

ADD_EXECUTABLE(HelloWorld_subscriber
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_subscriber.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldEncryption.c
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldEncryption.h
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldUdpTransform.c
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldUdpTransform.h
                ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

TARGET_LINK_LIBRARIES(HelloWorld_publisher
        ${MICRO_C_LIBS}
        ${MICRO_NETIOPSL_LIBS}
        ${MICRO_RTIME_LIB}
        ${MICRO_OSPSL_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)

TARGET_LINK_LIBRARIES(HelloWorld_subscriber
        ${MICRO_C_LIBS}
        ${MICRO_NETIOPSL_LIBS}
        ${MICRO_RTIME_LIB}
        ${MICRO_OSPSL_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)
