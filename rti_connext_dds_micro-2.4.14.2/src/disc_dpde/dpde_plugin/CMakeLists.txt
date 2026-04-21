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
 
SET(RTI_SUBMODULE_SRC BuiltinCdr.c 
                      DiscoveryPlugin.c
                      ParticipantBuiltinTopicDataPlugin.c 
                      ParticipantDiscovery.c
                      ParticipantListener.c
                      PublicationBuiltinTopicDataPlugin.c
                      PublicationDiscovery.c
                      PublicationListener.c
                      SubscriptionDiscovery.c
                      SubscriptionListener.c
                      SubscriptionBuiltinTopicDataPlugin.c
)
                      
SET(RTI_SUBMODULE_HDR BuiltinCdr.h 
                      DiscoveryPlugin.h 
                      ParticipantBuiltinTopicDataPlugin.h 
                      ParticipantDiscovery.h 
                      ParticipantListener.h 
                      PublicationBuiltinTopicDataPlugin.h 
                      PublicationDiscovery.h 
                      PublicationListener.h 
                      SubscriptionBuiltinTopicDataPlugin.h 
                      SubscriptionDiscovery.h SubscriptionListener.h
)

IF (NOT RTIME_CERT)
    LIST(APPEND RTI_SUBMODULE_SRC DPDEVersion.c)
ENDIF()
                 
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
