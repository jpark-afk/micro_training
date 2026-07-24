################################################################################
# 
# Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC 
    BuiltinIpcCdr.c
    DDS_IpcLiveliness.c
    DDS_ParticipantMessageDataSupport.c
    DDS_ParticipantMessageDataPlugin.c
    DDS_ParticipantMessageData.c
)

SET(RTI_SUBMODULE_HDR 
    BuiltinIpcCdr.h
    DDS_IpcLiveliness.h
    DDS_ParticipantMessageDataSupport.h
    DDS_ParticipantMessageDataPlugin.h
    DDS_ParticipantMessageData.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
