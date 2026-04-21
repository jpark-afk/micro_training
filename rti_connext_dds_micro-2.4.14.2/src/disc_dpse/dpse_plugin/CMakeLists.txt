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
    DPSECdr.c 
    DPSEDiscoveryPlugin.c 
    DPSEParticipantBuiltinTopicData.c 
    DPSEParticipantListener.c 
    DPSEParticipantDiscovery.c
)

IF (NOT RTIME_CERT)
    LIST(APPEND RTI_SUBMODULE_SRC DPSEVersion.c)
ENDIF()
                      
SET(RTI_SUBMODULE_HDR DPSECdr.h 
                      DPSEDiscoveryPlugin.h 
                      DPSEParticipantBuiltinTopicData.h 
                      DPSEParticipantListener.h 
                      DPSEParticipantDiscovery.h)
                      
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

