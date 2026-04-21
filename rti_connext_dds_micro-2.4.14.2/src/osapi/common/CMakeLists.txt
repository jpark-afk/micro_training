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
    Thread.c 
    Timer.c 
    Heap.c
    System.c 
    Time.c 
    Process.c
    Mutex.c
    Log.c    
)
                
SET(RTI_SUBMODULE_HDR Log.h System.h Thread.h Mutex.h)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

