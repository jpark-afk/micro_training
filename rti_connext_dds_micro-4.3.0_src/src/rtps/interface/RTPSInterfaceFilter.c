/*
 * FILE: RTPSInterfaceFilter.c - RTPSInterface filter related functions implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief RTPSInterface filter related functions implementation
 */

#include "rtps/rtps_filter_plugin.h"
#include "rtps/rtps_log.h"
#include "cdr/cdr_stream.h"
#include "cdr/cdr_serialize.h"
#include "RTPSInterfaceFilter.h"

/*** SOURCE_BEGIN ***/

RTI_BOOL
RTPS_Interface_add_filtered_route(
        struct RTPS_Interface *intf,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address,
        struct RTPS_PeerEntry *peer_entry)
{
    struct RTPS_RemoteReader *reader = RTPS_PeerEntry_as_remote_reader(peer_entry);
    RTPS_SampleId_T last_sent_sn = RTPS_Interface_as_writer(intf)->last_sn;

    OSAPI_PRECONDITION((intf == NULL)
                        || (reader_guid == NULL)
                        || (dest_address == NULL)
                        || !RTPS_Interface_is_filtering_enabled(intf),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("intf", intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_TRUE);)

    if (RTPS_RemoteReader_is_reliable(reader))
    {
        last_sent_sn = RTPS_RemoteReaderReliable_last_sent_sn(RTPS_PeerEntry_as_remote_reader(peer_entry));
    }

    return RTPS_FilterPlugin_add_route(RTPS_Interface_as_writer(intf)->filter_plugin,
                                       RTPS_Interface_as_writer(intf)->writer_filter,
                                       reader_guid,
                                       dest_address,
                                       intf,
                                       peer_entry,
                                       &last_sent_sn);
}

RTI_BOOL
RTPS_Interface_delete_filtered_route(
        struct RTPS_Interface *intf,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address)
{
    OSAPI_PRECONDITION((intf == NULL) || (reader_guid == NULL) || (dest_address == NULL)
                            || !RTPS_Interface_is_filtering_enabled(intf),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("intf", intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_TRUE);)

    return RTPS_FilterPlugin_delete_route(RTPS_Interface_as_writer(intf)->filter_plugin,
                                          RTPS_Interface_as_writer(intf)->writer_filter,
                                          reader_guid,
                                          dest_address);
}

RTI_PRIVATE RTI_BOOL
RTPS_Interface_send_filter_gap(
        struct RTPS_Interface *rtps_intf,
        struct RTPS_PeerEntry *peer_entry,
        struct REDA_SequenceNumber *last_sent_sn)
{
    NETIO_Packet_T packet = NETIO_Packet_INITIALIZER;
    RTPS_SendFlags_T send_flags = RTPS_SEND_GAP_FLAG;
    struct REDA_SequenceNumber gap_sn_start;
    struct REDA_SequenceNumber *gap_sn_end;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(rtps_intf);

    OSAPI_PRECONDITION((rtps_intf == NULL) || (peer_entry == NULL) || (last_sent_sn == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("intf", rtps_intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("peer_entry", peer_entry, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("last_sent_sn", last_sent_sn, RTI_TRUE);)

    gap_sn_start = *last_sent_sn;
    REDA_SequenceNumber_plusplus(&gap_sn_start);

    gap_sn_end = &writer->last_sn;

    if (!RTPS_Sender_route_packet(rtps_intf, &packet,
                                  peer_entry, send_flags, NULL,
                                  &gap_sn_start, gap_sn_end,
                                  NULL, 0, NULL, RTPS_SENDMODE_QUEUE_PACKET))
    {
        RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
RTPS_Interface_apply_filter(
        struct RTPS_Interface *intf,
        struct NETIO_Address *dest_address,
        NETIO_Packet_T *packet,
        RTI_BOOL *drop_sample_out)
{
    OSAPI_PRECONDITION((intf == NULL) || (dest_address == NULL) || (packet == NULL)
                            || (drop_sample_out == NULL) || !RTPS_Interface_is_filtering_enabled(intf),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("intf", intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("packet", packet, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("drop_sample_out", drop_sample_out, RTI_TRUE);)

    return RTPS_FilterPlugin_apply_filter(RTPS_Interface_as_writer(intf)->filter_plugin,
                                          RTPS_Interface_as_writer(intf)->writer_filter,
                                          dest_address,
                                          packet,
                                          RTPS_Interface_send_filter_gap,
                                          drop_sample_out);
}

RTI_PRIVATE RTI_BOOL
RTPS_Interface_for_each_reliable_peer(
        struct RTPS_Interface *rtps_intf,
        struct RTPS_PeerEntry *peer_entry,
        struct REDA_SequenceNumber *last_sent_sn)
{
    NETIO_Packet_T packet = NETIO_Packet_INITIALIZER;
    RTPS_SendFlags_T send_flags = RTPS_SEND_HB_FLAG;
    struct REDA_SequenceNumber *last_written_sn;
    struct REDA_SequenceNumber *last_acked_sn;
    struct NETIO_Address peer_addr;
    RTI_BOOL retval;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(rtps_intf);

    OSAPI_PRECONDITION((rtps_intf == NULL) || (peer_entry == NULL) || (last_sent_sn == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("rtps_intf", rtps_intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("peer_entry", peer_entry, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("last_sent_sn", last_sent_sn, RTI_TRUE);)

    last_written_sn = &writer->last_sn;
    last_acked_sn = &RTPS_RemoteReaderReliable_last_acked_sn(RTPS_PeerEntry_as_remote_reader(peer_entry));

    /* Write down the last_sent_sn to this reader so that a periodic heartbeat
     * can include the correct sn that the reader should have received.
     */
    if ((REDA_SequenceNumber_compare(last_sent_sn, &RTPS_RemoteReaderReliable_last_sent_sn(RTPS_PeerEntry_as_remote_reader(peer_entry))) > 0))
    {
        RTPS_RemoteReaderReliable_last_sent_sn(RTPS_PeerEntry_as_remote_reader(peer_entry)) = *last_sent_sn;
    }

    if (REDA_SequenceNumber_compare(last_sent_sn, last_written_sn) < 0)
    {
        /* The reader was not sent the last sample so remove it from the send window */
        RTPS_Window_remove(&RTPS_RemoteReaderReliable_window(RTPS_PeerEntry_as_remote_reader(peer_entry)), NULL, last_written_sn);

        /* The last_sent_sn can be immediately marked as acknowledged
         * by the reader. When the reader acknowledges the next non-filtered
         * sample this SN will not be acked again because it will not be in
         * the send window.
         */
        if (rtps_intf->upstr_intf != NULL)
        {
            NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);
            retval = NETIO_Interface_acknack(rtps_intf->upstr_intf,
                                             &peer_addr,
                                             last_written_sn,
                                             RTI_FALSE);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                RTPS_LOG_ACK(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(retval);
#endif /* OSAPI_ENABLE_LOG */
        }

        /* Piggyback heartbeats help ensure that any outstanding samples are
         * acknowledged within the send window. However, if samples are being
         * filtered then the piggyback hearts are not being sent. Therefore,
         * we need to check if a piggyback heartbeat would have been included
         * with this sample if it had not been filtered and send a regular
         * heartbeat instead.
         *
         * It is only necessary to send a heartbeat if the reader has outstanding
         * samples. If the reader does not have any outstanding samples, then it
         * is fine for the last_acked_sn to be outside of the send window.
         */
        if ((RTPS_WriterReliable_samples_per_hb(writer) > 0)
             && (REDA_SequenceNumber_compare(last_acked_sn, last_sent_sn) < 0))
        {
            /* The RTPS Interface tracks piggyback heartbeats per route, but
             * but we cannot use that value here because we are sending heartbeat
             * to a peer, not a route.
             */
            if (last_written_sn->low % (RTI_UINT32)RTPS_WriterReliable_samples_per_hb(writer) == 0)
            {
                if (!RTPS_Sender_route_packet(rtps_intf, &packet,
                                              peer_entry, send_flags, NULL,
                                              NULL, NULL,
                                              NULL, 0, NULL, RTPS_SENDMODE_QUEUE_PACKET))
                {
                    RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
                    return RTI_FALSE;
                }
            }
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
RTPS_Interface_update_reliable_filtered_peers(struct RTPS_Interface *intf)
{
    OSAPI_PRECONDITION((intf == NULL) || !RTPS_Interface_is_filtering_enabled(intf),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("intf", intf, RTI_TRUE);)

    return RTPS_FilterPlugin_for_each_reliable_peer(RTPS_Interface_as_writer(intf)->filter_plugin,
                                                    RTPS_Interface_as_writer(intf)->writer_filter,
                                                    RTPS_Interface_for_each_reliable_peer);
}
