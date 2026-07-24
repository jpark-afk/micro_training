###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
IF (RTIME_EXCLUDE_SHMEM)
    return()
ENDIF()

SET(RTI_MODULE_NAME 
    netio_shmem
)

SET(RTI_SUBMODULES 
    transport 
    shmem 
    concurrentqueue
)

IF (RTIME_PIL)
    SET(RTI_MODULE_IMPORTS 
        rti_me_psl::netiopsl 
        dds_c
    )
ELSE()
    SET(RTI_MODULE_IMPORTS dds_c)
ENDIF()

SET(RTI_MODULE_EXPORT netioshmem)

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
