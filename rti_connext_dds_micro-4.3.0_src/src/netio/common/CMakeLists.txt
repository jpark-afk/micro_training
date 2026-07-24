###############################################################################
#
# Copyright (c) 2018-2026 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC
    NETIOAddress.c
    NETIORouteResolver.c
    NETIOBindResolver.c
    NETIOInterface.c
    NETIOPacket.c
    NETIORtps.c
)

SET(RTI_SUBMODULE_HDR
    NETIORouteResolver.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

