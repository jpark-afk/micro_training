###############################################################################
# (c) Copyright, Real-Time Innovations 2019-2021
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC autosarHeap.c
                      autosarMutex.c
                      autosarProcess.c
                      autosarSemaphore.c
                      autosarSystem.c
                      autosarThread.c
                      autosarString.c
)

IF (NOT RTIME_CERT)
    LIST(APPEND RTI_SUBMODULE_SRC
        autosarLog.c
    )
ENDIF()

SET(RTI_SUBMODULE_HDR autosarHeap.h
                      autosarMutex.h
                      autosarSemaphore.h)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
