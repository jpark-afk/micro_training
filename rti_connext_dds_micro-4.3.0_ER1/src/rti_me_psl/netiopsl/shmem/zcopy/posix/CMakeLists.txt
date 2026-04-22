################################################################################
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
IF (RTIME_INCLUDE_ZCOPY_V2 OR RTI_CMAKE_SOURCE)
    SET(RTI_PSL_SRC
        posixShmMonitor.c
        posixShmSegment.c
    )

    SET(RTI_PSL_HDR
        posixShmMonitor.h
        posixShmSegment.h
    )
ENDIF()

