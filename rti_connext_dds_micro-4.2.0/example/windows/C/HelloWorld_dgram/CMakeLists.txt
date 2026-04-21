###############################################################################
# Copyright (c) 2025-2025 Real-Time Innovations, Inc.  All rights reserved.    	
# Permission to modify and use for internal purposes granted.   	
# This software is provided "as is", without warranty, express or implied.
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

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

ADD_DEFINITIONS()
LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(HelloWorld C)
SET(SOURCE_EXTENSION c)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    SET(PLATFORM_LIBS m pthread rt)
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
    #Visual studio needs to run commands using 'call'
    #if there is more than one dependency
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

SET(RTI_SECURE_INC)
SET(RTI_SECURE_LINK_DIRECTORES)

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
    ${RTI_SECURE_INC}
)

LINK_DIRECTORIES(
    ${RTIMEHOME}/lib/${RTIME_TARGET_NAME} 
    ${RTI_SECURE_LINK_DIRECTORES}
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

IF (DEFINED RTIME_MAG_FILES)
    FOREACH(xml ${RTIME_MAG_FILES})
        IF (NOT EXISTS ${xml})
            MESSAGE(FATAL_ERROR "${xml} does not exist.")
        ELSEIF(IS_DIRECTORY ${xml})
            MESSAGE(FATAL_ERROR "${xml} is not a file.")
        ENDIF()

        GET_FILENAME_COMPONENT(filename ${xml} NAME)
        STRING(REGEX REPLACE "\\.xml" "" basename ${filename})
        GET_FILENAME_COMPONENT(basename ${xml} NAME_WE)

        STRING(REGEX REPLACE "\\.xml" "Qos.xml" xml_qos ${xml})
        IF (EXISTS ${xml_qos})
            IF(NOT IS_DIRECTORY ${xml_qos})
                SET(REFERENCED_OPT "YES")
            ENDIF()
        ENDIF()

        LIST(APPEND APP_GEN_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Appgen.c)

        LIST(APPEND APP_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Appgen.h)


        IF (DEFINED REFERENCED_OPT)
            ADD_CUSTOM_COMMAND(OUTPUT ${APP_GEN_C} ${APP_GEN_H}
                               COMMAND ${COMMAND_PREFIX} ${RTIDDSMAG} -verbosity 1
                                       -language C
                                       -replace
                                       -d ${CMAKE_CURRENT_SOURCE_DIR}
                                       -idlFile ${IDL_PATH}/${IDL_SOURCE_C}
                                       -referencedFile ${XML_PATH}/${basename}Qos.xml
                                       ${XML_PATH}/${xml}
                               DEPENDS ${XML_PATH}/${xml} ${XML_PATH}/${basename}Qos.xml
                               COMMENT "Regeneration of application support from ${xml}"
            )
        ELSE()
            ADD_CUSTOM_COMMAND(OUTPUT ${APP_GEN_C} ${APP_GEN_H}
                               COMMAND ${COMMAND_PREFIX} ${RTIDDSMAG} -verbosity 1
                                       -language C
                                       -replace
                                       -d ${CMAKE_CURRENT_SOURCE_DIR}
                                       -idlFile ${IDL_PATH}/${IDL_SOURCE_C}
                                       ${XML_PATH}/${xml}
                               DEPENDS ${XML_PATH}/${xml}
                               COMMENT "Regeneration of application support from ${xml}"
            )
        ENDIF()
    ENDFOREACH()
ENDIF()

ADD_EXECUTABLE(HelloWorld_publisher
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_publisher.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_dgram_udpv4.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
                                             ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

ADD_EXECUTABLE(HelloWorld_subscriber
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_subscriber.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_dgram_udpv4.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
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
