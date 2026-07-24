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

SET(RTI_PSL_SRC
)

SET(RTI_PSL_HDR
)

SET(RTI_PSL_SUBDIR
)

IF ( (NOT RTIME_EXCLUDE_SHMEM) OR RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    LIST(APPEND RTI_PSL_SRC
        commonShmSegment.c
    )
ENDIF()
