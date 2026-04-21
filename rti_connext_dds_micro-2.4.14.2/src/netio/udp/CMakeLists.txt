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
    UDPInterface.c 
)

SET(RTI_SUBMODULE_HDR 
    UDPInterface.h 
)

IF (RTIME_UDP_ENABLE_TRANSFORMS OR (RTIME_SHIP_SOURCE AND (NOT RTIME_CERT)))
    LIST(APPEND RTI_SUBMODULE_SRC UDPTransform.c)
    LIST(APPEND RTI_SUBMODULE_HDR UDPTransform.h)
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
