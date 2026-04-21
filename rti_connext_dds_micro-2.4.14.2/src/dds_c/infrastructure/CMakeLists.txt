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
    Duration.c 
    BuiltinTopicKey.c 
    Entity.c 
    GUID.c 
    QosPolicy.c 
    RtpsWellKnownPorts.c 
    BuiltinTopicData.c 
    LocatorSeq.c 
    VendorId.c 
    ProductVersion.c 
    ProtocolVersion.c 
    InstanceHandle.c 
    InstanceHandleSeq.c 
    SampleInfoSeq.c 
    Waitset.c 
    Conditions.c 
    QosPolicyCountSeq.c 
)

SET(RTI_SUBMODULE_HDR 
    BuiltinTopicKey.h 
    Duration.h 
    Entity.h 
    InstanceHandle.h 
    QosPolicy.h 
    RtpsWellKnownPorts.h 
    Conditions.h 
    Waitset.h
)

# Exclude empty files for CERT

IF (NOT RTIME_CERT)
    LIST(APPEND RTI_SUBMODULE_SRC DDSCVersion.c StringManager.c)
    LIST(APPEND RTI_SUBMODULE_HDR StringManager.h)
ENDIF()

                                    
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

