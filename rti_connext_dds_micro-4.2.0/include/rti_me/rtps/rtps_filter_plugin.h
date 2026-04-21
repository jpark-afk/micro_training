/*
 * FILE: rtps_filter_plugin.h - RTPS Filter Plugin definitions
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef rtps_filter_plugin_h
#define rtps_filter_plugin_h

#include "rtps/rtps_rtps.h"
#include "rt/rt_rt.h"
#include "netio/netio_address.h"
#include "netio/netio_interface.h"
#include "cdr/cdr_cdr_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Base class for an RTPS filter plugin
 */
struct RTPS_FilterPlugin
{
    struct RT_ComponentProperty _parent;
};

/*ci
 * \brief An opaque writer filter for an RTPS filter plugin
 */
struct RTPS_FilterPluginWriterFilter;

/*ci
 * \brief An opaque RTPS interface
 */
struct RTPS_Interface;

/*ci
 * \brief An opaque RTPS peer entry
 */
struct RTPS_PeerEntry;

typedef RTI_BOOL
(*RTPS_FilterPlugin_for_each_peerFunc)(
        struct RTPS_Interface *intf,
        struct RTPS_PeerEntry *peer_entry,
        struct REDA_SequenceNumber *last_sn);

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*RTPS_FilterPlugin_add_routeFunc)(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address,
        struct RTPS_Interface *rtps_intf,
        struct RTPS_PeerEntry *peer_entry,
        const struct REDA_SequenceNumber *last_sent_sn))

typedef RTI_BOOL
(*RTPS_FilterPlugin_delete_routeFunc)(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address);

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*RTPS_FilterPlugin_apply_filterFunc)(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Address *dest_address,
        NETIO_Packet_T *packet,
        RTPS_FilterPlugin_for_each_peerFunc send_gap_func,
        RTI_BOOL *drop_sample_out))

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*RTPS_FilterPlugin_for_each_reliable_peerFunc)(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        RTPS_FilterPlugin_for_each_peerFunc for_each_func))

struct RTPS_FilterPluginI
{
    struct RT_ComponentI _parent;

    RTPS_FilterPlugin_add_routeFunc add_route;

    RTPS_FilterPlugin_delete_routeFunc delete_route;

    RTPS_FilterPlugin_apply_filterFunc apply_filter;

    RTPS_FilterPlugin_for_each_reliable_peerFunc for_each_reliable_peer;
};

#define RTPS_FilterPlugin_add_route(plugin_, writer_filter_, reader_guid_, dest_address_, rtps_intf_, route_entry_, last_sent_sn_) \
    ((struct RTPS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        add_route((plugin_), (writer_filter_), (reader_guid_), (dest_address_), (rtps_intf_), (route_entry_), (last_sent_sn_))

#define RTPS_FilterPlugin_delete_route(plugin_, writer_filter_, reader_guid_, dest_address_) \
    ((struct RTPS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        delete_route((plugin_), (writer_filter_), (reader_guid_), (dest_address_))

#define RTPS_FilterPlugin_apply_filter(plugin_, writer_filter_, dest_address_, packet_, send_gap_func_, drop_sample_out_) \
    ((struct RTPS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        apply_filter((plugin_), (writer_filter_), (dest_address_), (packet_), (send_gap_func_), (drop_sample_out_))

#define RTPS_FilterPlugin_for_each_reliable_peer(plugin_, writer_filter_, for_each_func_) \
    ((struct RTPS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        for_each_reliable_peer((plugin_), (writer_filter_), (for_each_func_))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* rtps_filter_plugin_h */
