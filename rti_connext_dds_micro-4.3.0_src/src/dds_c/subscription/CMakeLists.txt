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

SET(RTI_SUBMODULE_SRC SubscriberQos.c Subscriber.c SubscriberEvent.c 
                      SubscriberImpl.c DataReaderHistory.c DataReaderQos.c 
                      DataReader.c DataReaderEvent.c DataReaderImpl.c 
                      DataReaderInterface.c DataReaderDiscovery.c)
                      
SET(RTI_SUBMODULE_HDR DataReaderDiscovery.h DataReaderEvent.h 
                      DataReaderHistory.h DataReaderImpl.h 
                      DataReaderInterface.h DataReaderQos.h 
                      SubscriberEvent.h SubscriberImpl.h SubscriberQos.h)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
        DataReaderFilter.c
    )
    LIST(APPEND RTI_SUBMODULE_HDR
        DataReaderFilter.h
    )
ENDIF()

SET(DLL_WIN32APPLICATIONINCLUDES -DRTI_rtps_DLL_VARIABLE 
                                 -DRTI_dds_c_DLL_VARIABLE 
                                 -DRTI_dds_c_DLL_EXPORT 
                                 -DRTI_cdr_DLL_VARIABLE 
                                 -DRTI_netio_DLL_VARIABLE 
                                 -DRTI_reda_DLL_VARIABLE 
                                 -DRTI_db_DLL_VARIABLE 
                                 -DRTI_rt_DLL_VARIABLE 
                                 -DRTI_osapi_DLL_VARIABLE 
                                 -DRTI_DLL_VARIABLE 
                                 -DRTIME_DLL_EXPORT)
                                 
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

