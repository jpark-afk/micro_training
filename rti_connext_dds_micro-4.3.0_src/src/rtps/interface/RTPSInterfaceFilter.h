/*
 * FILE: RTPSInterfaceFilter.h - RTPSInterface filter related functions declarations
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief RTPSInterface filter related functions declarations
 */

#ifndef RTPSInterfaceFilter_h
#define RTPSInterfaceFilter_h

#include "RTPSInterface.h"

#define RTPS_Interface_is_filtering_enabled(intf_) \
    (RTPS_Interface_is_writer(intf_) \
     && (RTPS_Interface_as_writer(intf_)->writer_filter != NULL))

#ifdef __cplusplus
extern "C"
{
#endif

extern RTI_BOOL
RTPS_Interface_add_filtered_route(
        struct RTPS_Interface *intf,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address,
        struct RTPS_PeerEntry *peer_entry);

extern RTI_BOOL
RTPS_Interface_delete_filtered_route(
        struct RTPS_Interface *intf,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address);

extern RTI_BOOL
RTPS_Interface_apply_filter(
        struct RTPS_Interface *intf,
        struct NETIO_Address *dest_address,
        NETIO_Packet_T *packet,
        RTI_BOOL *drop_sample_out);

extern RTI_BOOL
RTPS_Interface_update_reliable_filtered_peers(struct RTPS_Interface *intf);

#ifdef __cplusplus
}
#endif

#endif /* RTPSInterfaceFilter_h */
