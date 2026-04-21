###############################################################################
# (c) Copyright, Real-Time Innovations 2024-2024, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_PSL_CFLAGS
   -DOSPSL_OS_DEF_H=rti_me_psl/ospsl/ospsl_os_stubs.h
)

SET(RTI_PSL_SRC
   stubMutex.c 
   stubProcess.c 
   stubSemaphore.c 
   stubSystem.c 
   stubThread.c 
   stubHeap.c
   stubStdio.c
)

SET(RTI_PSL_HDR
)

SET(RTI_PSL_SUBDIR
)
