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
    RTPSInterface.c
    RTPSHdrExt.c
    RTPSDataFragSnd.c
    RTPSDataFragRcv.c
    RTPSTrust.c
)

SET(RTI_SUBMODULE_HDR
    RTPSInterface.h
    RTPSHdrExt.h
    RTPSDataFrag.h
    RTPSTrust.h
)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
        RTPSInterfaceFilter.c
    )
    LIST(APPEND RTI_SUBMODULE_HDR
        RTPSInterfaceFilter.h
    )
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
