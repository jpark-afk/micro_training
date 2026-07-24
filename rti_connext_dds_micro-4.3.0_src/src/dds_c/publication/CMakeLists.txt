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

SET(RTI_SUBMODULE_SRC PublisherQos.c PublisherImpl.c Publisher.c
                      PublisherEvent.c DataWriterQos.c DataWriterImpl.c
                      DataWriter.c DataWriterEvent.c DataWriterInterface.c
                      DataWriterDiscovery.c DataWriterProperty.c)

SET(RTI_SUBMODULE_HDR DataWriterDiscovery.h DataWriterEvent.h
                      DataWriterImpl.h DataWriterInterface.h DataWriterQos.h
                      PublisherEvent.h PublisherImpl.h PublisherQos.h
                      DataWriterProperty.h)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_SUBMODULE_SRC
        DataWriterFilter.c
    )
    LIST(APPEND RTI_SUBMODULE_HDR
        DataWriterFilter.h
    )
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

