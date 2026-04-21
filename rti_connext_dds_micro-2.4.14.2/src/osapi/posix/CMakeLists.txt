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

SET(RTI_SUBMODULE_SRC 
        posixHeap.c 
        posixMutex.c 
        posixProcess.c  
        posixSemaphore.c 
        posixSystem.c 
        posixThread.c 
        posixString.c
)
SET(RTI_SUBMODULE_HDR posixSystem.h)
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
