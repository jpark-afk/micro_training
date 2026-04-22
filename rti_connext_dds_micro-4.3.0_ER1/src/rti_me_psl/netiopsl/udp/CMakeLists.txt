###############################################################################
# Copyright (c) 2018-2024 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_PSL_CFLAGS
)

SET(RTI_PSL_SRC
    UDPInterface.c
)

SET(RTI_PSL_HDR
    UDPInterface.h
)

SET(RTI_PSL_SUBDIR
)

IF (RTIME_UDP_ENABLE_TRANSFORMS OR RTI_CMAKE_SOURCE)
    LIST(APPEND RTI_PSL_SRC
        UDPTransform.c
    )
    LIST(APPEND RTI_PSL_HDR
        UDPTransform.h
    )
ENDIF()

IF (DEFINED RTIME_INCLUDE_AUTOSAR OR RTI_CMAKE_SOURCE)
    message(STATUS "[RTI] Including AUTOSAR UDP PSL")
    LIST(APPEND RTI_PSL_SRC
        autosar/autosarSocket.c
    )
    LIST(APPEND RTI_PSL_HDR
        autosar/autosarSocket.h
    )
ENDIF()
