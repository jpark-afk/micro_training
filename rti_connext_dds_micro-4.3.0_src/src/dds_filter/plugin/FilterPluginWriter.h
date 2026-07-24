/*
 * FILE: FilterPluginWriter.h - Filter plugin Writer definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef FilterPluginWriter_h
#define FilterPluginWriter_h

#include "dds_c/dds_c_filter_plugin.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct RTPS_FilterPluginWriterFilter
{
    /*ci \brief The registered type name of the writer's topic
     */
    const char *type_name;

    /*ci \brief The type code of the writer's topic
     */
    struct DDS_TypeCode *type_code;

    DB_Table_T reader_table;

    DB_Table_T route_table;

    char *inline_qos_buf;

    struct NETIO_PacketBuffer inline_qos_pbuf;

    DDS_UnsignedLong max_remote_reader_filters;

    DDS_UnsignedLong cur_remote_reader_filters;

    /*ci \brief Maximum width of the per-reader filter-result bitmap
     */
    DDS_UnsignedLong max_window_size;

    /*ci \brief The last sequence number evaluated by the filter for all readers
     */
    struct REDA_SequenceNumber last_evaluated_sn;
};

struct DDS_ReaderFilterEntry
{
    /*ci \brief The RTPS interface used to reach to the reader
     */
    struct RTPS_Interface *rtps_intf;

    /*ci \brief The RTPS peer entry for the reader
     */
    struct RTPS_PeerEntry *rtps_peer_entry;

    /*ci \brief The compiled filter for the reader
     */
    struct DDS_ContentFilterCompiledFilter *compiled_filter;

    /*ci \brief The guid of the reader
     */
    struct NETIO_Guid reader_guid;

    /*ci \brief The last SequenceNumber sent to the reader
     */
    struct REDA_SequenceNumber last_sent_sn;

    /*ci \brief Whether or not the reader is reliable
     */
    DDS_Boolean reliable;

    /*ci \brief Whether or not this is a local reader
     */
    DDS_Boolean is_local;

    /*ci
     * \brief The result of applying this reader's filter
     *
     * \details This is a bitmap for which a bit is set for each SN for which the
     *          filter was evaluated and the sample passed the filter. If a SN is
     *          between the lead and the last evaluated SN inclusive, it means
     *          that the filter was evaluated for that SN. Otherwise, there is
     *          no result stored for that SN.
     */
    struct RTPS_Bitmap filter_result;

    /*ci \brief The number of route entries which are associated with this reader
     */
    RTI_UINT32 route_count;
};

struct DDS_RouteFilterEntry
{
    struct NETIO_Address dest_address;

    struct DDS_ReaderFilterEntry *reader;
};

extern DDS_Boolean
DDS_FilterPluginImpl_writer_attach(
        struct DDS_FilterPlugin *plugin,
        DDS_DataWriter *writer,
        DDS_UnsignedLong max_remote_reader_filters,
        DDS_UnsignedLong max_remote_readers,
        DDS_UnsignedLong max_routes_per_reader,
        DDS_UnsignedLong max_samples,
        struct RTPS_FilterPluginWriterFilter **writer_filter_out);

#ifndef RTI_CERT
extern void
DDS_FilterPluginImpl_writer_detach(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter);
#endif /* !RTI_CERT */

extern DDS_Boolean
DDS_FilterPluginImpl_writer_add_remote_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        const struct DDS_ContentFilterProperty *filter_property,
        DDS_Boolean reliable);

extern DDS_Boolean
DDS_FilterPluginImpl_writer_add_local_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        struct DDS_ContentFilterCompiledFilter *reader_filter,
        DDS_Boolean reliable);

extern void
DDS_FilterPluginImpl_writer_remove_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key);

extern DDS_Boolean
DDS_FilterPluginImpl_writer_evaluate(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean unregister_or_dispose,
        const void *sample);

extern DDS_Boolean
DDS_FilterPluginImpl_writer_apply_reader_filter(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct NETIO_Guid *reader_guid,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean *sample_dropped_out);

extern RTI_BOOL
DDS_FilterPluginImpl_writer_add_route(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address,
        struct RTPS_Interface *rtps_intf,
        struct RTPS_PeerEntry *peer_entry,
        const struct REDA_SequenceNumber *last_sent_sn);

extern RTI_BOOL
DDS_FilterPluginImpl_writer_delete_route(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address);

extern RTI_BOOL
DDS_FilterPluginImpl_writer_apply_filter(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Address *dest_address,
        NETIO_Packet_T *packet,
        RTPS_FilterPlugin_for_each_peerFunc send_gap_func,
        RTI_BOOL *drop_sample_out);

extern RTI_BOOL
DDS_FilterPluginImpl_for_each_reliable_peer(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        RTPS_FilterPlugin_for_each_peerFunc for_each_func);

#ifdef __cplusplus
}
#endif

#endif /* FilterPluginWriter_h */
