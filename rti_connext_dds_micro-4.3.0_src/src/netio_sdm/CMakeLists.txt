###############################################################################
# Copyright (c) 2018-2024 Real-Time Innovations, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
IF (RTIME_EXCLUDE_SHMEM)
    return()
ENDIF()

SET(RTI_MODULE_NAME netio_sdm)
SET(RTI_SUBMODULES sdm memmgr)
SET(RTI_MODULE_IMPORTS xcdr netio_shmem dds_c)
SET(RTI_MODULE_EXPORT netiosdm)

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
