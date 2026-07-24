################################################################################
# Copyright (c) 2018-2025  Real-Time Innovations, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC 
    Type.c 
    PluginHelper.c 
    TypePlugin.c 
    TypePluginDefault.c 
)

IF (NOT RTIME_DDS_EXCLUDE_XTYPES OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC 
         TypeCode.c
    )
ENDIF()

SET(RTI_SUBMODULE_HDR 
    Type.h 
    PluginHelper.h
)


INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
