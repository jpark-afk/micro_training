###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2021
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC SubscriberQos.c Subscriber.c SubscriberEvent.c 
                      SubscriberImpl.c DataReaderHistory.c DataReaderQos.c 
                      DataReader.c DataReaderEvent.c DataReaderImpl.c 
                      DataReaderInterface.c DataReaderDiscovery.c)
                      
SET(RTI_SUBMODULE_HDR DataReaderDiscovery.h DataReaderEvent.h 
                      DataReaderHistory.h DataReaderImpl.h 
                      DataReaderInterface.h DataReaderQos.h 
                      SubscriberEvent.h SubscriberImpl.h SubscriberQos.h)
                                                       
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

