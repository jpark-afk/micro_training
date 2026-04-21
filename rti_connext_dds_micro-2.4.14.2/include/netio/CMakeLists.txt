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
    netio_config.h
    netio_dll.h
    netio_log.h
    netio_common.h
    netio_address.h
    netio_route.h
    netio_interface.h
    netio_loopback.h
    netio_rtps.h
)

IF (NOT RTIME_UDP_EXCLUDE_BUILTIN)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR netio_udp.h)
ENDIF()
