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

SET(RTI_SHARED_SUBMODULE flat_data
)

SET(RTI_SHARED_SRC
    FlatDataLog.c
    FlatSample.c
)

IF (UPDATE_PATCHED)
    LIST(APPEND RTI_SHARED_SRC
    )
    SET(RTI_SHARED_SRC_PATCHED)
ELSE()
    SET(RTI_SHARED_SRC_PATCHED
    )
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
