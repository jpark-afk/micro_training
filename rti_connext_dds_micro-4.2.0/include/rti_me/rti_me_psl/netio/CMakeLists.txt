###############################################################################
# Copyright (c) 2024-2025 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_MODULE_PUBLIC_HDR
    netio_udp.h
    netio_udp_cpp.hxx
)

IF (NOT RTIME_EXCLUDE_SHMEM OR RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
        netio_shmem_segment.h
    ) 
ENDIF()
