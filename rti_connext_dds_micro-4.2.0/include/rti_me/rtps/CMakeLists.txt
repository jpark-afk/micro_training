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

SET(RTI_MODULE_PUBLIC_HDR
    rtps_config.h
    rtps_dll.h
    rtps_log.h
    rtps_rtps.h
    rtps_rtps_impl.h
    rtps_checksum.h
    rtps_trust_plugin.h
)

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
        rtps_filter_plugin.h
    )
ENDIF()
