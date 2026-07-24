###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2025
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC
    Timer.c
    System.c
    Time.c
    Process.c
    Log.c
    Heap.c
    Hash.c
    Md5.c
    Task.c
    Thread.c
    String.c
    Semaphore.c
)

SET(RTI_SUBMODULE_HDR
    Log.h
    Thread.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
