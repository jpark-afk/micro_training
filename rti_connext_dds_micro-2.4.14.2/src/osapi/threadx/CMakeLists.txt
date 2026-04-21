###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2021
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

set(RTI_SHIP TRUE)

SET(RTI_SUBMODULE_SRC threadxHeap.c threadxMutex.c threadxProcess.c threadxSemaphore.c threadxSystem.c threadxThread.c threadxString.c)
SET(RTI_SUBMODULE_HDR )
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
