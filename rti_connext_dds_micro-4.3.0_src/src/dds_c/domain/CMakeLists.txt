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
    DomainFactoryQos.c
    DomainFactory.c
    DomainParticipantChecksum.c
    DomainParticipantQos.c
    DomainParticipant.c
    DomainParticipantTrust.c
    DomainParticipantEvent.c
    RemoteEndpoint.c
    RemoteEntity.c
    RemoteParticipant.c
    RemotePublication.c
    RemoteSubscription.c
    DomainQosPolicy.c
    FlowControl.c
    DiscoveryQueue.c
)

SET(RTI_SUBMODULE_HDR
    DomainFactory.h
    DomainFactoryQos.h
    DomainParticipant.h
    DomainParticipantTrust.h
    DomainParticipantChecksum.h
    DomainParticipantEvent.h
    DomainParticipantQos.h
    RemoteEndpoint.h
    RemoteEntity.h
    RemoteParticipant.h
    RemotePublication.h
    RemoteSubscription.h
    DomainQosPolicy.h
    FlowControl.h
    DiscoveryQueue.h
)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
        DomainParticipantFilter.c
    )
    LIST(APPEND RTI_SUBMODULE_HDR
        DomainParticipantFilter.h
    )
ENDIF()

IF (RTIME_DDS_ENABLE_FLOW_CONTROL OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
         LeakyBucketFlowController.c
    )

    LIST(APPEND RTI_SUBMODULE_HDR
        LeakyBucketFlowController.h
    )
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
