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

SET(RTI_SUBMODULE_SRC
    Duration.c
    BuiltinTopicKey.c
    Entity.c
    GUID.c
    QosPolicy.c
    RtpsWellKnownPorts.c
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
    DDSCVersion.c
    UserDataQosPolicy.c
    PropertyQosPolicy.c
    EncapsulationSeq.c
    Locator.c
    Transport.c
    Cookie.c
    Sequence.c
    Buffer.c
    UserDataManager.c
    StringManager.c
    PartitionQosPolicy.c
)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
        ContentFilterQosPolicy.c
        ContentFilterProperty.c
    )
ENDIF()

SET(RTI_SUBMODULE_HDR
    BuiltinTopicKey.h
    Duration.h
    Entity.h
    InstanceHandle.h
    QosPolicy.h
    RtpsWellKnownPorts.h
    Conditions.h
    Waitset.h
    UserDataQosPolicy.h
    Locator.h
    Transport.h
    UserDataManager.h
    StringManager.h
    PartitionQosPolicy.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
