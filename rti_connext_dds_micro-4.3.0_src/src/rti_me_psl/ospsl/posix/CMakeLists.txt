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
        -DOSPSL_OS_DEF_H=rti_me_psl/ospsl/ospsl_os_posix.h
)

SET(RTI_PSL_SRC
        posixMutex.c
        posixProcess.c
        posixSemaphore.c
        posixSystem.c
        posixThread.c
)

SET(RTI_PSL_HDR
        posixSystem.h
        posixThread.h
)

SET(RTI_PSL_SUBDIR
)
