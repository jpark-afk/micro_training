###############################################################################
# (c) Copyright, Real-Time Innovations 2025-2026
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
SET(RTI_PSL_CFLAGS
        -DOSPSL_OS_DEF_H=rti_me_psl/ospsl/ospsl_os_autosar.h
)

SET(RTI_PSL_SRC
        autosarHeap.c
        autosarMutex.c
        autosarProcess.c
        autosarSemaphore.c
        autosarSystem.c
        autosarThread.c
        autosarString.c
        autosarLog.c
)

SET(RTI_PSL_HDR
        autosarHeap.h
        autosarMutex.h
        autosarSemaphore.h
)
