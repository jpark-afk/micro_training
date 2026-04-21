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
SET(RTI_PSL_CFLAGS
)

SET(RTI_PSL_SRC
    sysvShmSemMutex.c
)

SET(RTI_PSL_HDR
)

SET(RTI_PSL_SUBDIR
)

IF (RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    LIST(APPEND RTI_PSL_SRC
        posixRobustShmMutex.c
        posixShmMutex.c
        posixShmSegment.c
        sysvShmSegment.c
    )

    LIST(APPEND RTI_PSL_HDR
        posixRobustShmMutex.h
        posixShmMutex.h
    )
    RETURN()
ENDIF()

IF (RTIME_HAVE_POSIX_ROBUST_MUTEX)
    LIST(APPEND RTI_PSL_CFLAGS
        -DRTI_HAVE_POSIX_ROBUST_MUTEX=1
    )

    # posixShmMutex is needed for signalling semaphores
    LIST(APPEND RTI_PSL_SRC
        posixRobustShmMutex.c
        posixShmMutex.c
    )

    LIST(APPEND RTI_PSL_HDR
        posixRobustShmMutex.h
        posixShmMutex.h
    )
ELSEIF(RTIME_HAVE_POSIX_MUTEX)
    LIST(APPEND RTI_PSL_CFLAGS
        -DRTI_HAVE_POSIX_MUTEX=1
    )
    LIST(APPEND RTI_PSL_SRC
        posixShmMutex.c
    )

    LIST(APPEND RTI_PSL_HDR
        posixShmMutex.h
    )
ENDIF()

IF (RTIME_HAVE_POSIX_SHMEM)
    LIST(APPEND RTI_PSL_SRC
         posixShmSegment.c
    )
ELSE()
    LIST(APPEND RTI_PSL_SRC
         sysvShmSegment.c
    )
ENDIF()
