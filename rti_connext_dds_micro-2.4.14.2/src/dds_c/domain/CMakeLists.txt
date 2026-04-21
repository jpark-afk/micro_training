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

SET(RTI_SUBMODULE_SRC DomainFactoryQos.c DomainFactory.c 
                      DomainParticipantQos.c DomainParticipant.c 
                      DomainParticipantEvent.c RemoteEndpoint.c 
                      RemoteEntity.c RemoteParticipant.c 
                      RemotePublication.c RemoteSubscription.c 
                      DomainQosPolicy.c DomainParticipantChecksum.c)
                      
SET(RTI_SUBMODULE_HDR DomainFactory.h DomainFactoryQos.h 
                      DomainParticipant.h DomainParticipantEvent.h 
                      DomainParticipantQos.h RemoteEndpoint.h 
                      RemoteEntity.h RemoteParticipant.h 
                      RemotePublication.h RemoteSubscription.h 
                      DomainQosPolicy.h DomainParticipantChecksum.h)
                                                       
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
