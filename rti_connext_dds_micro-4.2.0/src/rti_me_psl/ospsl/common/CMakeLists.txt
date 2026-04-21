################################################################################
#
# Copyright (c) 2025-2025 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
SET(RTI_PSL_CFLAGS
)

IF (RTIME_INCLUDE_LOG OR RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    SET(RTI_PSL_SRC
        LogWrapper.c
    )
ENDIF()

SET(RTI_PSL_HDR
)

SET(RTI_PSL_SUBDIR
)
