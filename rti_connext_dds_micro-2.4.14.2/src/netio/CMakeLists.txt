###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2018
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_MODULE_NAME netio)
SET(RTI_OBJECT_MODULE TRUE)

SET(RTI_SUBMODULES common loopback)

IF (NOT RTIME_UDP_EXCLUDE_BUILTIN) 
    LIST(APPEND RTI_SUBMODULES udp)
ENDIF()

IF (((DEFINED _RTIME_OSAPI_PLATFORM) AND (${_RTIME_OSAPI_PLATFORM} STREQUAL "autosar")) OR
    (NOT DEFINED _RTIME_OSAPI_PLATFORM))
    LIST(APPEND RTI_SUBMODULES autosar)
ENDIF()

IF (((DEFINED _RTIME_OSAPI_PLATFORM) AND (${_RTIME_OSAPI_PLATFORM} STREQUAL "threadx")) OR
    (NOT DEFINED _RTIME_OSAPI_PLATFORM))
    LIST(APPEND RTI_SUBMODULES threadx)
ENDIF()

SET(RTI_MODULE_IMPORTS rt db cdr reda osapi)

SET(RTI_SECONDARY_TEST_DIRS udp)

SET(RTI_SHIP true)
SET(RTI_SHIP_TEST true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
