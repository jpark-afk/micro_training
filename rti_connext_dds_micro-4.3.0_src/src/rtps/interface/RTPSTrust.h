/*
 * FILE: RTPSTrust.h - RTPS Transform functions
 *
 * Copyright (c) 2018-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef RTPSTransform_h
#define RTPSTransform_h

#include "RTPSInterface.h"

#define RTPS_MAX_TRANSFORM_REDA_BUFFERS (4)

extern RTI_BOOL
RTPS_Interface_trust_initialize_intf(struct RTPS_Interface *rtps_intf,
                                    struct RTPS_Interface *ext_intf);

extern void
RTPS_Interface_trust_finalize_intf(struct RTPS_Interface *rtps_intf);

extern RTI_BOOL
RTPS_Interface_trust_initialize_ext_intf(
                        struct RTPS_Interface *rtps_intf,
                        const struct RTPS_InterfaceProperty *const property);

#ifndef RTI_CERT
extern void
RTPS_Interface_trust_finalize_ext_intf(struct RTPS_Interface *rtps_intf);
#endif

extern RTI_BOOL
RTPS_Interface_trust_transform_outgoing_buffer(
        struct RTPS_Interface *rtps_intf,
        NETIO_Packet_T *packet, struct RTPS_HEADER_EXT **hdrext);

extern RTI_BOOL
RTPS_Interface_trust_transform_incoming_buffer(
    struct RTPS_Interface *rtps_intf,
    NETIO_Packet_T *packet,
    NETIO_PacketState_T *saved_packet_state);
#endif
