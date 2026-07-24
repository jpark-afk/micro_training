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
 
SET(RTI_SUBMODULE_SRC DiscoveryPlugin.c
                      ParticipantDiscovery.c
                      ParticipantListener.c
                      PublicationDiscovery.c
                      PublicationListener.c
                      SubscriptionDiscovery.c
                      SubscriptionListener.c
                      DPDEVersion.c)

SET(RTI_SUBMODULE_HDR DiscoveryPlugin.h
                      ParticipantDiscovery.h
                      ParticipantListener.h
                      PublicationDiscovery.h
                      PublicationListener.h
                      SubscriptionDiscovery.h
                      SubscriptionListener.h)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
