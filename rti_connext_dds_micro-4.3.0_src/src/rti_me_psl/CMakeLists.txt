################################################################################
#
# Copyright (c) 2018-2026 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Modification History
# --------------------
# 09sep2020,tk MICRO-2420 / PR#27261 Exclude unused header files
#
################################################################################

SET(RTI_MODULE_NAME rti_me_psl)

SET(RTIME_NETIOPSL_SUBMODULES)
SET(RTIME_OSPSL_SUBMODULES common)

IF (NOT (RTI_CMAKE_SOURCE OR TEST_BUNDLE))
    IF (NOT RTIME_PIL)
        IF (RTIME_TARGET_PSL)
            SET(RTIME_RESOLVED_PSL ${RTIME_TARGET_PSL})
        ELSEIF(DEFINED RTIME_PLATFORM_PSL)
            SET(RTIME_RESOLVED_PSL ${RTIME_PLATFORM_PSL})
        ELSE()
            RTI_MESSAGE_FATAL("PSL is not defined for ${RTI_TARGET_NAME}")
        ENDIF()
    ELSE()
        # AI-MOD-BEGIN: honor explicit PSL routing for RTIME_PIL targets
        IF (RTIME_PIL_USE_TARGET_PSL AND RTIME_TARGET_PSL)
            SET(RTIME_RESOLVED_PSL ${RTIME_TARGET_PSL})
        ELSE()
            SET(RTIME_RESOLVED_PSL ospsl::stubs ospsl::libc/stubs netiopsl::stubs)
            IF (NOT RTIME_EXCLUDE_SHMEM)
                LIST(APPEND RTIME_RESOLVED_PSL netiopsl::shmem/stubs)
            ENDIF()
            IF (RTIME_INCLUDE_ZCOPY_V2)
                LIST(APPEND RTIME_RESOLVED_PSL netiopsl::shmem/zcopy/stubs)
            endif()
        ENDIF()
        # AI-MOD-END
    ENDIF()
ENDIF()

FOREACH(a ${RTIME_RESOLVED_PSL})
    IF ("${a}" MATCHES "netiopsl::(.*)")
        LIST(APPEND RTIME_NETIOPSL_SUBMODULES ${CMAKE_MATCH_1})
    ELSEIF ("${a}" MATCHES "pskpsl::(.*)")
        LIST(APPEND RTIME_PSKPSL_SUBMODULES ${CMAKE_MATCH_1})
    ELSEIF ("${a}" MATCHES "ospsl::(.*)")
        LIST(APPEND RTIME_OSPSL_SUBMODULES ${CMAKE_MATCH_1})
    ELSEIF ("${a}" MATCHES "enable::(.*)")
        SET(${CMAKE_MATCH_1} TRUE)
    ELSEIF ("${a}" MATCHES "disable::(.*)")
        SET(${CMAKE_MATCH_1} FALSE)
    ELSE()
        message(FATAL_ERROR "No specification for ${RTIME_TARGET_NAME} found")
    ENDIF()
ENDFOREACH()

# PSK PSL is not considered a OBJECT MODULE.
# This is done because the rtime_add_psl_library()
# only builds libraries if they are not a a object module
IF(RTIME_PSL_PSK)
    SET(RTI_OBJECT_MODULE FALSE)
ELSE()
    IF (RTIME_PIL)
        SET(RTI_MODULE_DEPENDS osapi)
    ELSEIF (NOT RTIME_PSL)
        SET(RTI_OBJECT_MODULE TRUE)
    ENDIF()
ENDIF()

SET(RTI_SUBMODULES)

if (RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    SET(RTIME_PSL_OSAPI TRUE)
    SET(RTIME_PSL_NETIO TRUE)
    SET(RTIME_PSL_NETIOCPP TRUE)
    SET(RTIME_PSL_PSK TRUE)
ENDIF()

IF (RTIME_PSL_OSAPI)
    IF (RTIME_FULL_SOURCE)
        IF (NOT RTI_OBJECT_MODULE)
            SET(RTI_MODULE_EXPORTS osapi)
        ELSE()
            SET(RTI_MODULE_DEPENDS osapi)
        ENDIF()
    ENDIF()
    LIST(APPEND RTI_SUBMODULES ospsl common)
ENDIF()

IF (RTIME_PSL_NETIO)
    SET(RTI_MODULE_EXPORTS)
    SET(RTI_MODULE_DEPENDS)
    IF (NOT RTIME_EXCLUDE_SHMEM)
        LIST(APPEND RTI_MODULE_DEPENDS netio_shmem)
    ENDIF()
    IF (RTIME_INCLUDE_ZCOPY_V2)
        LIST(APPEND RTI_MODULE_DEPENDS netio_zcopy)
    ENDIF()
    IF ((NOT RTIME_PSL AND NOT RTIME_PIL) AND RTIME_FULL_SOURCE)
        # Build integrated library and full source is available
        LIST(APPEND RTI_MODULE_DEPENDS osapi)
    ELSE()
        # For split libraries, always depend only on OSPSL
        SET(RTI_MODULE_DEPENDS rti_me_psl::ospsl)
    ENDIF()
    SET(RTI_PIL_DEPENDS librti_me )
    LIST(APPEND RTI_SUBMODULES netiopsl)
    LIST(APPEND RTI_LIBRARY_DEPENDS librti_me ospsl)
ENDIF()

IF (RTIME_PSL_NETIOCPP AND NOT RTIME_CERT)
    SET(RTI_MODULE_EXPORTS)
    SET(RTI_PIL_DEPENDS librti_me)
    SET(RTI_MODULE_DEPENDS rti_me_psl::netiopsl)
    IF (RTIME_FULL_SOURCE)
        LIST(APPEND RTI_MODULE_DEPENDS osapi)
    ENDIF()
    LIST(APPEND RTI_SUBMODULES netiopsl_cpp)
    LIST(APPEND RTI_LIBRARY_DEPENDS netiopsl librti_me)
ENDIF()

IF (RTIME_PSL_PSK)
    SET(RTI_MODULE_EXPORTS)
    SET(RTI_MODULE_DEPENDS)
    LIST(APPEND RTI_SUBMODULES pskpsl)
    IF((NOT RTIME_PSL AND NOT RTIME_PIL))
        LIST(APPEND RTI_LIBRARY_DEPENDS librti_me)
    ELSE()
        LIST(APPEND RTI_LIBRARY_DEPENDS librti_me ospsl)
    ENDIF()
ENDIF()

SET(RTI_SHIP true)
IF (RTIME_PSL)
    SET(RTI_MODULE_EXPORT_HEADER TRUE)
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
