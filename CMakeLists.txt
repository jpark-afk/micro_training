
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

function(SET_LIB_ORDER INPUT_LIST_VAR)
    # The target library order
    SET(TARGET_LIB_ORDER
        rti_me_appgen${RTI_LIB_SUFFIX}
        rti_me_cpp${RTI_LIB_SUFFIX}
        rti_me_netiosdm${RTI_LIB_SUFFIX}
        rti_me_netioshmem${RTI_LIB_SUFFIX}
        rti_me_netiozcopy${RTI_LIB_SUFFIX}
        rti_me_rhsm${RTI_LIB_SUFFIX}
        rti_me_whsm${RTI_LIB_SUFFIX}
        rti_me_discdpse${RTI_LIB_SUFFIX}
        rti_me_discdpde${RTI_LIB_SUFFIX}
        rti_me_netiopsl_cpp${RTI_LIB_SUFFIX}
        rti_me_netiopsl${RTI_LIB_SUFFIX}
        rti_me_ddsfilter${RTI_LIB_SUFFIX}
        rti_me_ddsxtypes${RTI_LIB_SUFFIX}
        rti_me${RTI_LIB_SUFFIX}
        rti_me_ospsl${RTI_LIB_SUFFIX}
        rti_me_ddspsk${RTI_LIB_SUFFIX}
        rti_me_pskpsl${RTI_LIB_SUFFIX}
    )

    SET(INPUT_LIST ${${INPUT_LIST_VAR}})

    # Remove duplicates
    LIST(LENGTH INPUT_LIST LIST_LENGTH)
    IF(LIST_LENGTH GREATER 0)
        LIST(REMOVE_DUPLICATES INPUT_LIST)
    ENDIF()

    # Reorder the items based on the target order
    SET(REORDERED_LIBS)
    FOREACH(item IN LISTS TARGET_LIB_ORDER)
        IF(item IN_LIST INPUT_LIST)
            LIST(APPEND REORDERED_LIBS ${item})
        ENDIF()
    ENDFOREACH()

    SET(${INPUT_LIST_VAR} ${REORDERED_LIBS} PARENT_SCOPE)
endfunction()

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

IF (NOT (DEFINED RTIME_SECURE))
    SET(RTIME_SECURE FALSE)
ENDIF()

IF (NOT (DEFINED RTIME_LIGHTWEIGHT_SECURE))
    SET(RTIME_LIGHTWEIGHT_SECURE FALSE)
ENDIF()

IF (NOT (DEFINED RTI_SHARED_LIB))
    SET(RTI_SHARED_LIB FALSE)
ENDIF()

MESSAGE(STATUS "[RTI] RTI_SHARED_LIB .............: ${RTI_SHARED_LIB}")
MESSAGE(STATUS "[RTI] RTIME_LIGHTWEIGHT_SECURE ...: ${RTIME_LIGHTWEIGHT_SECURE}")
MESSAGE(STATUS "[RTI] RTIME_SECURE ...............: ${RTIME_SECURE}")
MESSAGE(STATUS "[RTI] EXCLUDE_SHMEM ..............: ${EXCLUDE_SHMEM}")
MESSAGE(STATUS "[RTI] EXCLUDE_ZCV2 ...............: ${EXCLUDE_ZCV2}")

SET(RTI_LIB_PREFIX librti_me_)
SET(RTI_LIB_SUFFIX)

IF (NOT RTI_SHARED_LIB)
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "z")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "zd")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ELSE()
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "d")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ENDIF()

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
    SET(RTI_LIB_PREFIX rti_me_)
    SET(PLATFORM_LIBS netapi32.lib advapi32.lib user32.lib winmm.lib WS2_32.lib)
    ADD_DEFINITIONS(-DRTI_WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

    IF (RTI_USE_NMAKE OR (${CMAKE_GENERATOR} MATCHES "NMake"))
        string(APPEND CMAKE_EXE_LINKER_FLAGS " /defaultlib:libc.lib /defaultlib:msvcrt.lib /defaultlib:Winmm.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrtd.lib")
    ELSE()
        IF (RTI_DEBUG_BUILD)
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

SET(RTI_SECURE_INC)
SET(RTI_SECURE_LINK_DIRECTORES)

IF (NOT RTI_SHARED_LIB)
    SET(RTI_LIB_EXT ${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE()
    SET(RTI_LIB_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF()

IF ((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND RTI_SHARED_LIB)
    ADD_DEFINITIONS(-DRTIME_DLL_VARIABLE)
ENDIF()

IF (${RTIME_SECURE} OR ${RTIME_LIGHTWEIGHT_SECURE})
   IF(NOT DEFINED OPENSSLHOME)
        IF(DEFINED ENV{OPENSSLHOME})
            SET(OPENSSLHOME $ENV{OPENSSLHOME})
        ELSE()
            IF(RTI_DEBUG_BUILD)
                SET(_cfg_type "debug")
            ELSE()
                SET(_cfg_type "release")
            ENDIF()

            IF(RTI_SHARED_LIB)
                SET(_cfg_link "shared")
            ELSE()
                SET(_cfg_link "static")
            ENDIF()

            SET(OPENSSLHOME
                "${RTIMEHOME}/third_party/openssl-3.5.0/${RTIME_TARGET_NAME}/${_cfg_type}-${_cfg_link}"
            )
        ENDIF()
    ENDIF()

    MESSAGE(STATUS "[RTI] OPENSSLHOME ................: ${OPENSSLHOME}")

    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        SET(SSL_LIBS libcrypto)
        SET(PLATFORM_LIBS crypt32.lib ${PLATFORM_LIBS})
    ELSE()
        SET(SSL_LIBS crypto)

        IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            LIST(APPEND SSL_LIBS dl)
        ENDIF()
    ENDIF()

    SET(RTI_SECURE_LINK_DIRECTORES ${OPENSSLHOME}/lib)
    SET(RTI_SECURE_INC ${OPENSSLHOME}/include)
ENDIF()

IF (${RTIME_SECURE})
    SET(RTI_SECURE_LIB rti_me_seccore${RTI_LIB_SUFFIX})
    LIST(APPEND RTI_SECURE_INC ${XML_INC})

    IF (SYSLIBXML2)
        IF (LIBXML2HOME)
            SET(XML_INC ${LIBXML2HOME}/lib)
        ENDIF()
    SET(XML_LIBS xml2)
    ENDIF()
ENDIF()

# Find ZCV2, SHMEM, and xtypes libraries in the library folder and add them
# to our library list. This is for convenience. In the future this will
# be based on the example generated and not on the presence of the libraries.
# CPP requires Shared memory. ZCv2 requries shared memory.
STRING(REGEX MATCH "^[^-]+" ARCH_NAME ${RTIME_TARGET_NAME})
SET(LIB_PATH "${RTIMEHOME}/lib/${ARCH_NAME}")
SET(PSL_LIB_PATH "${RTIMEHOME}/lib/${RTIME_TARGET_NAME}")

# Add the xtypes library if supported in case the IDL was compiled with
# -interpreted 1 (default).
IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddsxtypes${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")
    SET(MICRO_C_LIBS
        rti_me_ddsxtypes${RTI_LIB_SUFFIX}
        rti_me${RTI_LIB_SUFFIX}
    )
ELSE()
    SET(MICRO_C_LIBS
        rti_me${RTI_LIB_SUFFIX}
    )
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddsfilter${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")
    LIST(APPEND MICRO_C_LIBS "rti_me_ddsfilter${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netioshmem${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_SHMEM)
    LIST(APPEND MICRO_C_LIBS "rti_me_netioshmem${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netiosdm${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_SHMEM)
    LIST(APPEND MICRO_C_LIBS "rti_me_netiosdm${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netiozcopy${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_ZCV2)
    LIST(APPEND MICRO_C_LIBS "rti_me_netiozcopy${RTI_LIB_SUFFIX}")
    ADD_DEFINITIONS(-DRTIME_HAVE_ZCOPYV2=1)
ENDIF()

IF (${RTIME_LIGHTWEIGHT_SECURE})
    IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddspsk${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
        AND EXISTS "${PSL_LIB_PATH}/${RTI_LIB_PREFIX}pskpsl${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")

        LIST(APPEND MICRO_C_LIBS "rti_me_ddspsk${RTI_LIB_SUFFIX}")
        LIST(APPEND MICRO_C_LIBS "rti_me_pskpsl${RTI_LIB_SUFFIX}")
    ELSE()
        MESSAGE(FATAL_ERROR "PSK libraries were not found.")
    ENDIF()
ENDIF()

IF (NOT RTIME_CERT)
    IF (DEFINED RTIME_MAG_FILES)
        LIST(APPEND MICRO_C_LIBS rti_me_appgen${RTI_LIB_SUFFIX})
    ENDIF()


    LIST(APPEND MICRO_C_LIBS
        rti_me_rhsm${RTI_LIB_SUFFIX}
        rti_me_whsm${RTI_LIB_SUFFIX}
        rti_me_discdpse${RTI_LIB_SUFFIX}
        rti_me_discdpde${RTI_LIB_SUFFIX}
            )
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
    LIST(APPEND MICRO_C_LIBS rti_me_netiopsl${RTI_LIB_SUFFIX})
    LIST(APPEND MICRO_C_LIBS rti_me_ospsl${RTI_LIB_SUFFIX})
    LINK_DIRECTORIES(${RTIMEHOME}/lib/${RTIME_PIL_ARCH})
    GET_PROPERTY(cppdef DIRECTORY PROPERTY COMPILE_DEFINITIONS)
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

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf
                              ${CMAKE_SOURCE_DIR}/security/openssl_publisher.cnf
                              ${CMAKE_SOURCE_DIR}/security/openssl_subscriber.cnf
                              ${CMAKE_SOURCE_DIR}/security/xml/governance.xml
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.xml
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.xml
                       COMMAND "${CMAKE_COMMAND}" -E copy_directory "${RTIMEHOME}/rtiddsgen/resource/rtiddsgen/security" "security"
                       COMMENT "Copy secure configuration files"
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
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

SET_LIB_ORDER(MICRO_C_LIBS)

ADD_EXECUTABLE(HelloWorld_publisher
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_publisher.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
                                             ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

ADD_EXECUTABLE(HelloWorld_subscriber
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld_subscriber.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldApplication.h
                                             ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

TARGET_LINK_LIBRARIES(HelloWorld_publisher
        ${MICRO_C_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)

TARGET_LINK_LIBRARIES(HelloWorld_subscriber
        ${MICRO_C_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)
