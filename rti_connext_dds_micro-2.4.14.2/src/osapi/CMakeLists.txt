###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2020
#
# All rights reserved.
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

SET(RTI_MODULE_NAME osapi)

IF (RTIME_CERT OR RTIME_CERT_SOURCE)
    SET(RTI_SUBMODULES_ALL
        common
        posix
        autosar
        vxworks
    )
ELSE()
    SET(RTI_SUBMODULES_ALL
        common
        windows
        posix
        vxworks
        freertos
        threadx
        autosar
    )
ENDIF()

IF ((${RTIME_OSAPI_PLATFORM} MATCHES ${RTIME_OSAPI_AUTO}) OR RTIME_SHIP_SOURCE)
    SET(RTI_SUBMODULES ${RTI_SUBMODULES_ALL})
ELSE()
    SET(RTI_SUBMODULES common ${RTIME_OSAPI_PLATFORM})
ENDIF()

SET(RTI_OBJECT_MODULE TRUE)

SET(RTI_SHIP true)
SET(RTI_SHIP_TEST true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
