/*
 * FILE: RTPSDataFrag.c - RTPS Data Fragmentation
 *
 * Copyright (c) 2018-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "RTPSDataFrag.h"

RTI_PRIVATE void
RTPS_Receiver_delete_rxtable_entry(struct RTPS_Interface *intf,
                                   struct RTPS_RxFragmentRecord *entry)
{
    struct RTPS_RxFragmentRecord key;
    DB_ReturnCode_T dbrc;
    struct RTPS_RxFragmentData *frag_ptr;
    struct RTPS_RxFragmentData *next_frag;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    frag_ptr = entry->head_frag;

    while (frag_ptr != NULL)
    {
        next_frag = RTPS_RxFragmentNode_get_next(frag_ptr);
        REDA_BufferPool_return_buffer(RTPS_ReaderFragment_pbuf_pool(reader),frag_ptr);
        frag_ptr = next_frag;
    }

    key.writer = entry->writer;
    key.sn = entry->sn;
    entry = NULL;

    dbrc = DB_Table_remove_record(RTPS_ReaderFragment_data_frag_table(reader),
                                  (DB_Record_T*)&entry,
                                  (DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    OSAPI_TRACE_PRINTF2("Deleted fragmentation entry %S from writer %G",
                       &key.sn,&entry->writer);

    RTPS_ReaderFragment_pbuf_count(reader) += entry->reserved_pbuf_count;
    if (entry->zero_record != NULL)
    {
        entry->zero_record->reserved_pbuf_count -= entry->reserved_pbuf_count;
    }

    OSAPI_TRACE_PRINTF1("Added pbufs, total is %d\n",
                        OSAPI_TRACE_INT_AS_PTR(RTPS_ReaderFragment_pbuf_count(reader)))

    dbrc = DB_Table_delete_record(RTPS_ReaderFragment_data_frag_table(reader),
                                  (DB_Record_T)entry);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
}

/*ci
 * \brief Forward a fully assembled data sample for normal processing
 *
 * \details
 *
 * RTPS_Receiver_process_data(struct RTPS_Interface *intf,
 *                            struct RTPS_PeerEntry *peer_entry,
 *                            NETIO_Packet_T *packet,
 *                            RTI_UINT8 flags,
 *                            RTI_UINT32 data_len,
 *                            RTI_BOOL byte_swap);
 *
 * RTPS_Interface_receive(NETIO_Interface_T *netio_intf,
 *                        struct NETIO_Address *source,
 *                        struct NETIO_Address *dst,
 *                        NETIO_Packet_T *packet);
 *
 * As fragmented samples are completed they are forward to the RTPS
 * reader as a regular DATA sample for regular processing.
 */
RTI_PRIVATE void
RTPS_Receiver_forward_data(struct RTPS_Interface *intf,
                           struct RTPS_RxFragmentRecord *entry)
{
    struct RTPS_RxFragmentData *frag_ptr;
    struct RTPS_ExtBindEntry bind_key_low, bind_key_high;
    struct NETIO_GuidEntity OID_MAX = {{0xff,0xff,0xff,0xff}};
    struct NETIO_GuidEntity OID_ZERO = {{0,0,0,0}};
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T db_rc;
    struct RTPS_ExtBindEntry *bind_entry = NULL;
    struct NETIO_Packet packet;
    RTI_SIZE_T pkt_head, pkt_tail;
    RTI_SIZE_T data_len;

    /* Create a packet to process. The head of the packet must be at the
     * beginning of the inline Qos
     */
    if (!NETIO_Packet_initialize(&packet,NULL,0,0,NULL))
    {
        return;
    }

    bind_key_low.source = entry->writer;
    bind_key_low.destination = OID_ZERO;
    bind_key_high.source = entry->writer;
    bind_key_high.destination = OID_MAX;

    db_rc = DB_Table_select_range(intf->ext_intf->_parent._btable,
                                  DB_TABLE_DEFAULT_INDEX,
                                  &cursor,
                                  (DB_Key_T)&bind_key_low,
                                  (DB_Key_T)&bind_key_high);

    if (db_rc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    packet.head_pbuf = &entry->head_frag->pbuf;
    packet.buffer = NULL;
    packet.info.protocol_id = NETIO_PROTOCOL_RTPS;
    packet.info.timestamp = entry->timestamp;
    packet.info.sn = entry->sn;

    if ((entry->flags & RTPS_ENDIAN_FLAG) != 0)
    {
        packet.info.rtps_flags |= NETIO_RTPS_FLAGS_LITTLE_ENDIAN;
    }

    if ((entry->flags & RTPS_DATAFLAGS_Q) != 0)
    {
        packet.info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
    }

    /* The spec states that K=0 means that the Submessage contains the
     * serialized Data. K=1 means that the Submessage contains the Key.
     */
    if ((entry->flags & RTPS_DATAFRAG_FLAGS_K) != 0)
    {
        /* The RTPS_DATAFRAG_FLAGS_K is the same as a normal submessage data
         * flag RTPS_DATAFLAGS_D. Clear it do avoid confusion downstream.
         */
        entry->flags &= ~(RTI_UINT32)RTPS_DATAFRAG_FLAGS_K;
        entry->flags &= ~(RTI_UINT32)RTPS_DATAFLAGS_D;
        entry->flags |= RTPS_DATAFLAGS_K;
    }
    else
    {
        entry->flags |= RTPS_DATAFLAGS_D;
    }

    data_len = 0;
    frag_ptr = entry->head_frag;
    while (frag_ptr != NULL)
    {
        data_len += NETIO_PacketBuffer_get_length(&frag_ptr->pbuf);
        frag_ptr = RTPS_RxFragmentNode_get_next(frag_ptr);
        packet.tail_pbuf = &frag_ptr->pbuf;
    }

    NETIO_Packet_save_positions_to(&packet, &pkt_head, &pkt_tail);

    /* Forward submessage to each selected destination */
    db_rc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);

    while (db_rc == DB_RETCODE_OK)
    {
        if (!RTPS_Receiver_process_data(bind_entry->_rtps_intf,
                                        bind_entry->peer_ref,
                                        &packet,
                                        (RTI_UINT8)entry->flags,
                                        data_len,
                                        entry->byte_swap))
        {
        }

        NETIO_Packet_restore_positions_from(&packet, pkt_head, pkt_tail);

        db_rc = DB_Cursor_get_next(cursor, (DB_Record_T*)&bind_entry);
    }

    DB_Cursor_finish(intf->ext_intf->_parent._btable, cursor);
}

/*ci \brief Update the acknack response based on the content of the rxtable.
 *
 * \details
 *
 * Per the specification it is not allowed to ACK a fragmented sample until
 * all the fragments have been received. In addition, do not send a NACK for
 * a sample that has not been fully received. The reason is to be conservative
 * in case the peer writer does does not keep track of what is fragmented and
 * received and what is fragmented and not fully received. A simple application
 * may simply respond to a NACK partially received sample with all the
 * fragments.
 *
 *
 * An AckNack has the following form:
 *
 * base/bits:bitmap
 *
 * All samples <lowest available,base-1] are implicitly acknowledged. The
 * remaining samples are either acked (1) or nacked (0) in the bitmap.
 *
 * This function take a NACK generated by a reader and modifies it to remove
 * any holes in the bitmap.
 */
void
RTPS_Receiver_update_acknack_from_rxtable(struct RTPS_Interface *intf,
                                          struct NETIO_Guid *writer,
                                          struct RTPS_ACKNACK *acknack)
{
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord key;
    struct RTPS_RxFragmentRecord key_low;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    OSAPI_Trace_write("update acknack, lead = %S bits=%^d\n",
                      &acknack->bitmap.lead,&acknack->bitmap.bit_count,
                      NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    if (REDA_SequenceNumber_is_zero(&acknack->bitmap.lead))
    {
        return;
    }

	/* Reset the acknack bitcount for all fragmented samples in flight by
	 * searching for all SN [lead,last_sn received]
	 */
    key.writer = *writer;
    key.sn = acknack->bitmap.lead;
    cursor = NULL;

    OSAPI_Trace_write("check writer %G, sn=%B",writer,&acknack->bitmap,
                      NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

    dbrc = DB_Table_select_from(RTPS_ReaderFragment_data_frag_table(reader),
                                DB_TABLE_DEFAULT_INDEX,&cursor,&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_Trace_write("no entries in fragmentation table\n",
                          NULL,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
    }

    while (dbrc == DB_RETCODE_OK)
    {
        /* Stop when/if a new writer is reached */
        if (OSAPI_Memory_compare(&entry->writer,writer,
                                 sizeof(entry->writer)))
        {
            break;
        }

        /* If the first SN in the table is higher or equal to the lead it
         * means the main RTPS state machine is not up to date and there
         * are potentially holes in the receive window. The following scenarios
         * are possible:
         *
         * 1) acknack->bitmap.lead < entry->sn
         * 2) acknack->bitmap.lead == entry->sn
         * 4) acknack->bitmap.lead > entry->sn
         *
         * Case 1: This means the reader is missing one or more
         *         samples that is not yet in the fragmentation map.
         *         However, it is possible that _some_ samples are in the
         *         fragmentation table, based on the bit_count,  and
         *         should not be NACKed.
         *
         * Case 2: This means the reader is asking for at least
         *         one sample already in the fragmentation map (the first).
         *         Thus do not request the first sample again. However, there
         *         could be other samples in the fragmentation map already
         *         partially received that should not be requested again
         *         since NACK_FRAGs are sent for the fragmented samples.
         *         Positively ACK everything below lead and avoid re-sending
         *         samples already partially received.
         *
         * Case 3: This case could happen if the fragmentation table is not
         *         up to date. For example, a mixture of fragmented and
         *         non-fragmented samples could cause this scenario.
         *         However, this case is handled after this code where all
         *         samples < lead in the fragmentation table are removed.
         *
         * The logic excludes requested samples already in the fragmentation
         * table and passes requests for samples not in the bitmap on.
         * Note that the bitmap size or count is not changed, only the
         * state.
         */
        if (REDA_SequenceNumber_compare(&entry->sn,&acknack->bitmap.lead) >= 0)
        {
            RTI_INT32 diff;

            diff = RTPS_SequenceNumber_get_distance(&entry->sn,
                                                    &acknack->bitmap.lead);

            /* Since the lead is included in the bitmap add one to diff instead
             * of subtracting one from the bit_count since this also handles
             * the case where bit_count == 0 (positive ACKNACK) in which case
             * this will terminate the loop.
             */
            if ((diff + 1) > acknack->bitmap.bit_count)
            {
                /* The end of the bitmap has been reached, stop */
                break;
            }

            /* The fragmentation entry is within the bitmap range
             * clear the bit to avoid re-sending the hole sample.
             */
            if (!RTPS_Bitmap_set_bit(&acknack->bitmap,NULL,&entry->sn,RTI_FALSE))
            {
                /* Error */
                RTPS_LOG_BITMAP_SET_BIT(OSAPI_LOGKIND_ERROR);
            }
            else
            {
                /* Would only be used for logging */
            }
        }

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    }

    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);

    key.writer = *writer;
    key.sn = acknack->bitmap.lead;
    REDA_SequenceNumber_minusminus(&key.sn);

    /* Delete all SN < lead. Those samples will not be accepted. The record with
     * SN=0 is general state record for the writer, do not delete it. In the
     * special case were nothing has been received, do nothing.
     */
    key_low.writer = *writer;
    REDA_SequenceNumber_set_zero(&key_low.sn);
    REDA_SequenceNumber_plusplus(&key_low.sn);

    if (REDA_SequenceNumber_compare(&key_low.sn,&key.sn) > 0)
    {
        OSAPI_Trace_write("nothing has been received, lead=%S [%S,%S]\n",
                          &acknack->bitmap.lead,
                          &key_low.sn,&key.sn,
                          NULL,NULL,NULL,NULL,NULL,NULL,NULL);

        /* This can happen if nothing has been received yet */
        return;
    }

    /* delete from 1 - (lead - 1 ) */
    cursor = NULL;
    dbrc = DB_Table_select_range(RTPS_ReaderFragment_data_frag_table(reader),
                                DB_TABLE_DEFAULT_INDEX,&cursor,
                                &key_low,&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    while (dbrc == DB_RETCODE_OK)
    {
    	/* Do not delete the state  record */
        if (REDA_SequenceNumber_is_zero(&entry->sn))
        {
            break;
        }

		/* extra caution */
        if (entry->zero_record != NULL)
        {
            entry->zero_record->writer_lead = acknack->bitmap.lead;
        }

        OSAPI_Trace_write("delete stale entry for SN=%S (less than lead=%S)\n",
                            &entry->sn,&acknack->bitmap.lead,
                            NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        RTPS_Receiver_delete_rxtable_entry(intf,entry);
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    }
    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);
}

/*ci \brief Update the rx fragmentation table based on available samples.
 *
 * \details
 * When a HEARTBEAT is received remove all SNs that is no longer available
 * and incomplete. Each matched reader will update their local bitmaps.
 */
RTI_PRIVATE void
RTPS_Receiver_delete_unavailable_samples(struct RTPS_Interface *intf,
                                         struct NETIO_Guid *writer,
                                         RTPS_SampleId_T *sn_first,
                                         RTPS_SampleId_T *sn_last)
{
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord key_low;
    struct RTPS_RxFragmentRecord key_high;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    UNUSED_ARG(sn_last);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    OSAPI_TRACE_PRINTF2("delete unavailable samples [%S,%S]",sn_first,sn_last)

    key_low.writer = *writer;
    REDA_SequenceNumber_set_zero(&key_low.sn);

    key_high.writer = *writer;
    key_high.sn = *sn_first;
    REDA_SequenceNumber_minusminus(&key_high.sn);

    dbrc = DB_Table_select_range(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,&cursor,
                                 &key_low,&key_high);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (REDA_SequenceNumber_is_zero(&entry->sn))
        {
            OSAPI_TRACE_PRINTF1("Updated writer lead to %S",sn_first);

            entry->writer_lead = *sn_first;
        }
        else
        {
            OSAPI_TRACE_PRINTF1("delete entry for %S, no longer available\n",
                                &entry->sn)
            RTPS_Receiver_delete_rxtable_entry(intf,entry);
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    }

    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);

}

/*ci \brief Update the rx fragmentation table based on available samples.
 *
 * \details
 * When a HEARTBEAT is received remove all SNs that is no longer available
 * and incomplete. Each matched reader will update their local bitmaps.
 */
void
RTPS_Receiver_process_heartbeat_ext(struct RTPS_Interface *intf,
                                    struct RTPS_Interface *local_intf,
                                    struct RTPS_PeerEntry *peer_entry,
                                    RTPS_SampleId_T *sn_first,
                                    RTPS_SampleId_T *sn_last)
{
    UNUSED_ARG(local_intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    RTPS_Receiver_delete_unavailable_samples(intf,&peer_entry->addr,
                                             sn_first,sn_last);
}

/*ci \brief
 * When a GAP is received remove all SNs that is no longer available
 * and incomplete. Each matched reader will update their local bitmaps.
 */
void
RTPS_Receiver_process_gap_ext(struct RTPS_Interface *intf,
                              struct NETIO_Guid *writer,
                              RTPS_SampleId_T *sn_start,
                              RTPS_SampleId_T *sn_end,
                              struct RTPS_Bitmap *bitmap)
{

    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord key_low;
    struct RTPS_RxFragmentRecord key_high;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    UNUSED_ARG(bitmap);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    key_low.writer = *writer;
    key_low.sn = *sn_start;

    key_high.writer = *writer;
    key_high.sn = *sn_end;

    dbrc = DB_Table_select_range(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,&cursor,
                                 &key_low,&key_high);

    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    while (dbrc == DB_RETCODE_OK)
    {
        OSAPI_TRACE_PRINTF1("delete entry for %S, no longer available\n",
                            &entry->sn)
        RTPS_Receiver_delete_rxtable_entry(intf,entry);
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    }

    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);
}

/*ci
 * \brief Assign fields of NACK_FRAG
 *
 * \param[inout] nack_frag NACK_FRAG submessage to set
 * \param[in]    flags     NACK_FRAG flags
 * \param[in]    length    Submessage length
 * \param[in]    reader    Reader entity ID
 * \param[in]    writer    Writer entity ID
 * \param[in]    sn        The RTPS sn number fragments are nack'ed for
 * \param[in]    bitmap    Bitmap
 * \param[in]    count     Epoch count
 */
RTI_PRIVATE void
RTPS_Receiver_set_nack_frag(struct RTPS_NACK_FRAG *nack_frag,
                            RTI_UINT8 flags,
                            RTI_SIZE_T length,
                            RTPS_Entity_T reader,
                            RTPS_Entity_T writer,
                            struct REDA_SequenceNumber *sn,
                            struct RTPS_Bitmap *bitmap,
                            RTI_INT32 bit_count,
                            RTI_INT32 count)
{
    RTI_INT32 i, int_count, *epoch_ref = NULL;

    /* Epoch may start at the bitmap's first element. Will end up there if
     * bitmap has bitcount of zero.
     */
    epoch_ref = (RTI_INT32*)&(nack_frag->bits[0]);

    RTPS_Interface_set_submessage_header(&nack_frag->hdr, RTPS_NACK_FRAG_KIND,
                                         flags, length);

    nack_frag->reader = reader;
    nack_frag->writer = writer;
    nack_frag->sn = *sn;
    nack_frag->lead = bitmap->lead.low;

    /* Don't use the bitmap bit_count as it may be larger than what is
     * actually requested.
     */
    nack_frag->bit_count = bit_count;

    int_count = (nack_frag->bit_count + 31) / 32;

    for (i = 0; i < int_count; ++i)
    {
        nack_frag->bits[i] = bitmap->bits[i];
        epoch_ref = (RTI_INT32*)&(nack_frag->bits[i+1]);
    }

    *epoch_ref = count;
}

/*ci
 * \brief Send a NACK_FRAG for one fragmented sample.
 *
 * \param[in] intf  RTPS interface
 * \param[in] entry The fragmented entry to send a NACK_FRAG for
 *
 */
RTI_PRIVATE void
RTPS_Receiver_create_nack_frag(struct RTPS_Interface *intf,
                               struct RTPS_RxFragmentRecord *entry,
                               RTI_UINT32 last_fn,
                               union RTPS_MESSAGES *msg)
{
    RTI_SIZE_T payload_length;
    struct RTPS_Bitmap tmp_bitmap = entry->bitmap;
    struct REDA_SequenceNumber last_missing_fn = REDA_SEQUENCE_NUMBER_ZERO;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (entry->bitmap.lead.low > last_fn)
    {
        msg->nack_frag.bit_count = 0;
        return;
    }

    /* The bitmap has at least one missing fragment */

    /* Restrict the search to include last fragment number */
    tmp_bitmap.bit_count = (RTI_INT32)(last_fn - entry->bitmap.lead.low + 1U);

    if (tmp_bitmap.bit_count > RTPS_BITMAP_SIZE_MAX)
    {
        tmp_bitmap.bit_count = RTPS_BITMAP_SIZE_MAX;
    }

    if (RTPS_Bitmap_get_last_bit(&tmp_bitmap,&last_missing_fn,RTI_TRUE))
    {
        /* Limit bitmap to the last missing fragment */
        tmp_bitmap.bit_count = (RTI_INT32)(last_missing_fn.low - entry->bitmap.lead.low + 1U);
    }

    if (tmp_bitmap.bit_count > RTPS_BITMAP_SIZE_MAX)
    {
        tmp_bitmap.bit_count = RTPS_BITMAP_SIZE_MAX;
    }

    payload_length = RTPS_Interface_get_nack_frag_size(tmp_bitmap.bit_count);

    RTPS_ReaderReliable_nack_frag_epoch(reader)++;

    OSAPI_TRACE_PRINTF3("NACK_FRAG: bitmap=%B last=%^d bit_count=%^d",
                       &tmp_bitmap,
                       &last_fn,
                       &tmp_bitmap.bit_count);

    OSAPI_Trace_write("Send NACK_FRAG for sn=%S, last_sn=%^d bitmap (count=%^d)=%B",
                      &entry->sn,&last_fn,&tmp_bitmap.bit_count,&tmp_bitmap,
                      NULL,NULL,NULL,NULL,NULL,NULL);

    RTPS_Receiver_set_nack_frag(&msg->nack_frag, 0,
                                payload_length,
                                intf->_parent.local_address.value.guid.entity,
                                entry->writer.entity,
                                &entry->sn,
                                &tmp_bitmap,
                                tmp_bitmap.bit_count,
                                RTPS_ReaderReliable_nack_frag_epoch(reader));
}

RTI_PRIVATE void
RTPS_Receiver_send_nack_frag(struct RTPS_Interface *intf,
                             struct RTPS_PeerEntry *peer_entry,
                             struct RTPS_RxFragmentRecord *entry,
                             RTI_UINT32 last_fn)
{
    union RTPS_MESSAGES msg;

    RTPS_Receiver_create_nack_frag(intf,entry,last_fn,&msg);

    if (!RTPS_Interface_initialize_packet(intf))
    {
        return;
    }

    if (!RTPS_Sender_route_packet(intf, intf->packet,
                                  peer_entry,
                                  (RTPS_SendFlags_T)RTPS_SEND_NACK_FRAG_FLAG,
                                  NULL,NULL,NULL,&msg,1,NULL, RTPS_SENDMODE_QUEUE_PACKET))
    {
        RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
        return;
    }
}

void
RTPS_Receiver_get_nack_frags(struct RTPS_Interface *intf,
                             struct RTPS_PeerEntry *peer_entry,
                             union RTPS_MESSAGES *msg,
                             RTI_INT32 *msglen,
                             struct REDA_SequenceNumber *last_sn)
{
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord key;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    RTI_INT32 maxlen = *msglen;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    key.writer = peer_entry->addr;
    key.sn.low = 1;
    key.sn.high = 0;

    *msglen = 0;

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    dbrc = DB_Table_select_from(RTPS_ReaderFragment_data_frag_table(reader),
                                DB_TABLE_DEFAULT_INDEX,&cursor,&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }


    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    while (dbrc == DB_RETCODE_OK)
    {
        /* Stop when/if a new writer is reached */
        if (OSAPI_Memory_compare(&entry->writer,&peer_entry->addr,
                                 sizeof(entry->writer)))
        {
            break;
        }

        if (REDA_SequenceNumber_compare(&entry->sn,last_sn) > 0)
        {
            break;
        }

        if (*msglen == maxlen)
        {
            break;
        }

        RTPS_Receiver_create_nack_frag(intf,entry,entry->total_fragment_count,
                                       &msg[*msglen]);
        (*msglen)++;

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
    }

    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);
}

/*ci
 * \brief Fill the fragment bitmap
 *
 * \details
 * Fill the fragment bitmap with 1/0 based on the fragment data starting
 * at fragment numbers [be,ue]. If there is not sufficient fragments received
 * to fill up the bitmap it is fill in with 1s.
 */
RTI_PRIVATE void
RTPS_Receiver_fill_fragment_bitmap(struct RTPS_Bitmap *bitmap,
                                   struct RTPS_RxFragmentData *start_entry,
                                   RTI_UINT32 f_start,
                                   RTI_UINT32 f_end)
{
    RTI_UINT32 fn = 0;
    struct REDA_SequenceNumber Fs = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber Fe = REDA_SEQUENCE_NUMBER_ZERO;
    struct RTPS_RxFragmentData *next_entry = start_entry;

    fn = f_start;
    while ((next_entry != NULL) && (fn <= f_end) &&
           ((fn >= next_entry->first_fn) &&
            (fn <= next_entry->last_fn)))
    {
        /* The first fragment number to update within the range of the
         * fragment entry. If it is not within the entry there is nothing
         * to update.
         */
        Fs.low = fn;
        Fe.low = next_entry->last_fn;
        /* Only fill up to max bits */
        if (Fe.low > f_end)
        {
            Fe.low = f_end;
        }
        else
        {
            /* May have to go to the next entry */
            next_entry = RTPS_RxFragmentNode_get_next(next_entry);
        }

        OSAPI_TRACE_PRINTF2("fill 0s from %d - %d\n",
                            OSAPI_TRACE_INT_AS_PTR(Fs.low),
                            OSAPI_TRACE_INT_AS_PTR(Fe.low))

        /* Mark samples as received */
        if (!RTPS_Bitmap_fill(bitmap,&Fs,&Fe,RTI_FALSE))
        {
        }

        fn = Fe.low + 1;
    }

    if (fn <= f_end)
    {
        OSAPI_TRACE_PRINTF2("fill in 1s from %d - %d\n",
                            OSAPI_TRACE_INT_AS_PTR(fn),
                            OSAPI_TRACE_INT_AS_PTR(f_end))

        /* Not enough remaining fragments, fill rest with 1s to indicate
         * they have not been received.
         */
        Fs.low = fn;
        Fe.low = f_end;
        if (!RTPS_Bitmap_fill(bitmap,&Fs,&Fe,RTI_TRUE))
        {
        }
    }
}

/*ci
 * \brief Update the fragment bitmap based on reception of fragment
 *
 * \details
 * For each fragmented sample the receiver maintains a bitmap with
 * received fragments, at most 256. Fragments received outside the
 * range of the bitmap is still stored, but state is not maintained.
 * This does only means that if the distance between two fragments
 * is larger then MAX_OUTSTANDING_FRAGMENTS_PER_SAMPLE, only the first
 * is NACKed.
 *
 */
RTI_PRIVATE void
RTPS_Receiver_update_bitmap_from_fragment(struct RTPS_Bitmap *bitmap,
                                          struct RTPS_DATA_FRAG *data_frag,
                                          struct RTPS_RxFragmentData *frag_data,
                                          RTI_UINT32 *highest_sn)
{
    RTI_UINT32 bs = 0, be = 0;
    RTI_UINT32 us = 0, ue = 0;
    RTI_UINT32 fs = 0, fe = 0;
    RTI_UINT32 new_lead;
    struct REDA_SequenceNumber Us = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber Ue = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber new_lead_sn = REDA_SEQUENCE_NUMBER_ZERO;
    struct RTPS_RxFragmentData *next_entry;

    /* The delta cannot be more than signed 32 bit, calculate the highest
     * known fragment number.The bitmap is always
     * MAX_OUTSTANDING_FRAGMENTS_PER_SAMPLE.
     */
    bs = bitmap->lead.low;
    be = *highest_sn;

    /* The maximum number of fragments cannot exceed a signed 32 bit, calculate
     * the highest FN in the RTPS message.
     */
    fs = data_frag->fragment_start;
    fe = fs + data_frag->fragment_count - 1;

    OSAPI_TRACE_PRINTF4("bitmap update: bs=%^d be=%^d fs=%^d fe=%^d",
                        &bs,&be,&fs,&fe);

    if ((fe < bs) || (fs > be))
    {
        OSAPI_TRACE_PRINTF4("nothing to update in bitmap [fe=%d < bs=%d] || [fs=%d > be%d]\n",
                            OSAPI_TRACE_INT_AS_PTR(fe),
                            OSAPI_TRACE_INT_AS_PTR(bs),
                            OSAPI_TRACE_INT_AS_PTR(fs),
                            OSAPI_TRACE_INT_AS_PTR(be))
        return;
    }

    us = (fs < bs) ? bs : fs;
    ue = (fe > be) ? be : fe;

    Us.low = us;
    Ue.low = ue;

    OSAPI_TRACE_PRINTF2("update bitmap from [%d,%d]\n",
                        OSAPI_TRACE_INT_AS_PTR(Us.low),
                        OSAPI_TRACE_INT_AS_PTR(Ue.low))

    /* Fill the bitmap with 0s in the fragment range at the most 256 bits. Note
     * that this function truncates if the range falls outside the bitmap.
     */
    if (!RTPS_Bitmap_fill(bitmap,&Us,&Ue,RTI_FALSE))
    {
        OSAPI_TRACE_PRINTF0("failed to fill bitmap\n")
    }

    /* Based on the new bitmap, shift the bitmap to a new lead. If there
     * is a new lead it means that there are still unacked fragments in the
     * bitmap, but some previously not received samples are now received and
     * more bits can be shifted in from the right if there are fragment
     * numbers available while at the.
     */
    if (RTPS_Bitmap_get_first_bit(bitmap,&new_lead_sn,RTI_TRUE))
    {
        OSAPI_TRACE_PRINTF1("existing lead = %S\n",&new_lead_sn)

        /* There is at least one missing fragment in the new bitmap
         */
        if (!RTPS_Bitmap_shift_value(bitmap,&new_lead_sn,1))
        {
            OSAPI_TRACE_PRINTF0("failed to shift bitmap\n")
            return;
        }

        /* Only check from [be,be + diff] since anything < be already has
         * a state.
         */
        new_lead = new_lead_sn.low;

        *highest_sn = *highest_sn + (new_lead - bs);

        OSAPI_TRACE_PRINTF3("new lead: %d, fill from %d - %^d\n",
                            OSAPI_TRACE_INT_AS_PTR(new_lead),
                            OSAPI_TRACE_INT_AS_PTR(be),
                            highest_sn)

        RTPS_Receiver_fill_fragment_bitmap(bitmap,frag_data,be+1,*highest_sn);
    }
    else
    {
        OSAPI_TRACE_PRINTF0("find new lead\n")
        /* Did not find a 1 in the current bitmap, all fragments in bitmap
         * have been acknowledged. Start at fe + 1 to find the first new
         * missing fragment. Start at the next available fragment since
         * frag_data filled up the range in the fragment.
         */
        be = fe + 1;
        next_entry = RTPS_RxFragmentNode_get_next(frag_data);

        /* Skip already received ranges */
        while ((next_entry != NULL) &&
               ((next_entry->first_fn == be)))
        {
            be = next_entry->last_fn + 1;
            next_entry = RTPS_RxFragmentNode_get_next(next_entry);
        }

        /* Found the new lead, fill in the bitmap up to 256 bits */

        bitmap->lead.low = be;
        *highest_sn = be + MAX_OUTSTANDING_FRAGMENTS_PER_SAMPLE - 1;

        OSAPI_TRACE_PRINTF2("found new lead: %d - %^d\n",
                            OSAPI_TRACE_INT_AS_PTR(be),highest_sn);

        RTPS_Receiver_fill_fragment_bitmap(bitmap,next_entry,be,*highest_sn);
    }

    OSAPI_TRACE_PRINTF1("bitmap=%B",bitmap);

    OSAPI_Trace_write("bitmap=%B",bitmap,
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);
}

/*ci
 * \brief The submessage has already been  deserialized. The packet content
 *        is the parameter list (if it exists)
 */
void
RTPS_Receiver_process_heartbeat_frag_ext(struct RTPS_Interface *intf,
                                         struct RTPS_Interface *local_intf,
                                         struct RTPS_PeerEntry *peer_entry,
                                         RTPS_SampleId_T *writer_sn,
                                         RTI_UINT32 last_fn)
{
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord key;
    DB_ReturnCode_T dbrc;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    key.writer = peer_entry->addr;
    key.sn = *writer_sn;
    entry = NULL;

    dbrc = DB_Table_select_match(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&entry,
                                 (DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_PRINTF1("failed to look up SN=%^d for writer\n",
                            &writer_sn->low);
        return;
    }

    /* Check if any fragments are missing. If the fragment bitmap lead
     * is higher than the last_fn in the HEARTBEAT_FRAG it means the
     * receiver is up to date, nothing to do.
     */
    if (entry->bitmap.lead.low > last_fn)
    {
        return;
    }

    /* The last_fn >= than the bitmap lead and there are outstanding
     * fragments. Create a bitmap to request the missing fragments. Only
     * create a bitmap from [lead,last_fn].
     */
    RTPS_Receiver_send_nack_frag(local_intf,peer_entry,entry,last_fn);
}


/*ci \brief Get the length of the inline qos in a RTPS message.
 *
 * \details
 * The inline qos is a sequence of parameter ids and lengths, terminated by
 * RTPS_PID_SENTINEL. The length is returned in bytes, including the
 * RTPS_PID_SENTINEL. If the inline qos exceeds the maximum size specified
 * by max_inline_qos_size, -1 is returned.
 *
 * \param[in] msg_ptr Pointer to the start of the inline qos
 * \param[in] byte_swap Whether to byte swap the values
 * \param[in] max_inline_qos_size Maximum size of the inline qos
 *
 * \return Length of the inline qos or -1 if it exceeds max_inline_qos_size.
 */
RTI_PRIVATE RTI_INT32
RTPS_Receiver_get_inline_qos_length(char *msg_ptr,
                                    RTI_BOOL byte_swap,
                                    RTI_UINT32 max_inline_qos_size)
{
    RTI_UINT16 pid = 0;
    RTI_UINT16 pid_length = 0;
    char *start_msg_ptr = msg_ptr;
    RTI_UINT32 length = 0;

    do
    {
        CDR_deserialize_unsigned_short(&msg_ptr, &pid, byte_swap);
        CDR_deserialize_unsigned_short(&msg_ptr, &pid_length, byte_swap);
        if (pid != RTPS_PID_SENTINEL)
        {
            msg_ptr += pid_length;
            /* Safe to cast as pid_length is a 16 bit value. We will never exceed UINT_MAX */
            length = (RTI_UINT32)(msg_ptr - start_msg_ptr);
            if (length > max_inline_qos_size)
            {
                OSAPI_TRACE_PRINTF0("inline qos length exceeds max size")
                return -1;
            }
        }
    } while (pid != RTPS_PID_SENTINEL);

    /*safe to cast since max_inline_qos_size is being passed in as 64 */
    return (RTI_INT32)(msg_ptr - start_msg_ptr);
}

RTI_PRIVATE RTI_BOOL
RTPS_Receiver_add_data_frag(NETIO_Packet_T *packet,
                            struct RTPS_DATA_FRAG *data_frag,
                            struct RTPS_RxFragmentRecord *entry,
                            RTI_BOOL byte_swap,
                            RTI_UINT8 flags)
{
#define INLINE_QOS_OFFSET (64)
    struct RTPS_RxFragmentData *frag_data = NULL;
    RTI_UINT32 pbuf_idx,pbuf_off;
    RTI_UINT32 max_fn_pbuf,fn_count;
#if OSAPI_ENABLE_DEBUG_TRACE
    RTI_UINT32 last_fn;
#endif
    RTI_UINT32 copy_bytes,start_pos;
    char *dst;
    char *src;
    RTI_BOOL copy_inline = RTI_TRUE;
    RTI_BOOL first_frag_pbuf = RTI_FALSE;
    RTI_INT32 inline_bytes = 0;
    struct REDA_SequenceNumber fn;
    RTI_BOOL bit_val = RTI_FALSE;

    if (data_frag->fragment_start > entry->total_fragment_count)
    {
        OSAPI_Trace_write("Received out of bounds fragment (%d)",
                           OSAPI_TRACE_INT_AS_PTR(data_frag->fragment_start),
                           NULL,NULL,NULL,NULL,
                           NULL,NULL,NULL,NULL,NULL);

        return RTI_TRUE;
    }

    if ((data_frag->fragment_start + data_frag->fragment_count - 1)
            < entry->bitmap.lead.low)
    {
        OSAPI_Trace_write("Fragments for sn=%S in range [%d,%d] already received, ignore",
                           &data_frag->sn,
                           OSAPI_TRACE_INT_AS_PTR(data_frag->fragment_start),
                           OSAPI_TRACE_INT_AS_PTR((data_frag->fragment_start +  data_frag->fragment_count - 1)),
                           NULL,NULL,NULL,NULL,
                           NULL,NULL,NULL);

        return RTI_TRUE;
    }

    max_fn_pbuf = (entry->head_frag->pbuf.max_length / data_frag->fragment_size);
    if (entry->head_frag->pbuf.max_length % data_frag->sample_size)
    {
        max_fn_pbuf++;
    }

    /* Find the first pbuf to copy data into, use 0 based index so an
     * integer division is sufficient.
     */
    pbuf_idx =  (data_frag->fragment_start - 1) / max_fn_pbuf;
    pbuf_off = (data_frag->fragment_start - 1) % max_fn_pbuf;

#if OSAPI_ENABLE_DEBUG_TRACE
    last_fn = data_frag->fragment_start + data_frag->fragment_count - 1;

    OSAPI_TRACE_PRINTF5("Adding fragments [%^d - %^d] to sample %^d, pbuf index=%^d pbuf offset=%^d\n",
                        &data_frag->fragment_start,
                        &last_fn,
                        &entry->sn.low,
                        &pbuf_idx,&pbuf_off);

    OSAPI_Trace_write("Adding fragments [%^d - %^d] out of %^d to sample %^d, pbuf index=%^d pbuf offset=%^d\n",
                      &data_frag->fragment_start,
                      &last_fn,
                      &entry->total_fragment_count,
                      &entry->sn.low,
                      &pbuf_idx,&pbuf_off,NULL,
                      NULL,NULL,NULL);
#endif

    frag_data = entry->head_frag;
    first_frag_pbuf = RTI_TRUE;
    while (pbuf_idx > 0 && frag_data != NULL)
    {
        frag_data = (struct RTPS_RxFragmentData *)frag_data->pbuf._next;
        --pbuf_idx;
        first_frag_pbuf = RTI_FALSE;
    }

    if (frag_data == NULL)
    {
        OSAPI_Trace_write("No pbuf available to store fragment of sample %S\n",
                           &data_frag->sn,
                           NULL,NULL,NULL,NULL,
                           NULL,NULL,NULL,NULL, NULL);
        return RTI_FALSE;
    }

    fn_count = data_frag->fragment_count;
    fn.high = 0;
    fn.low = data_frag->fragment_start;

    src = (char*)NETIO_Packet_get_head(packet);
    start_pos = 0;

    if (first_frag_pbuf)
    {
        start_pos = (pbuf_off * data_frag->fragment_size) + INLINE_QOS_OFFSET;
    }
    else
    {
        start_pos = (pbuf_off * data_frag->fragment_size);
    }
    while (fn_count)
    {
        /* Check if the fragment has already been received */
        if (RTPS_Bitmap_get_bit(&entry->bitmap,&bit_val,&fn))
        {
            if (!bit_val)
            {
                REDA_SequenceNumber_plusplus(&fn);
                fn_count --;
                continue;
            }
        }

        /* If the remaining bytes is not an integral # of fragments (the
         * last can be less than the fragment size), calculate how much to
         * copy.
         */
        if (fn.low == entry->total_fragment_count)
        {
            copy_bytes = data_frag->sample_size -
                         ((entry->total_fragment_count - 1) *
                           data_frag->fragment_size);
        }
        else
        {
            copy_bytes = data_frag->fragment_size;
        }

        /* If the first fragment has not been received yet (lead == 1)
         * and there is an inline qos update the head position. Note
         * that this while loop resets copy_inline so the inline qos is
         * copied only once. lead is updated outside the while() loop
         * so once the inline qos has been received it is never copied
         * again.
         */
        if ((data_frag->fragment_start == 1) && copy_inline &&
            (entry->flags & RTPS_DATAFLAGS_Q) &&
            (entry->bitmap.lead.low == 1))
        {
            copy_inline = RTI_FALSE;
            inline_bytes = RTPS_Receiver_get_inline_qos_length(src,
                                                               byte_swap,
                                                               NETIO_Packet_get_payload_length(packet) < INLINE_QOS_OFFSET ?
                                                                            NETIO_Packet_get_payload_length(packet): INLINE_QOS_OFFSET);
            if (inline_bytes < 0)
            {
                return RTI_FALSE;
            }
            start_pos -= (RTI_UINT32)inline_bytes;
            copy_bytes += (RTI_UINT32)inline_bytes;

            frag_data->pbuf.head_pos = INLINE_QOS_OFFSET - (RTI_UINT32)inline_bytes;
        }
        else if (!(entry->flags & RTPS_DATAFLAGS_Q) && first_frag_pbuf)
        {
            frag_data->pbuf.head_pos = INLINE_QOS_OFFSET;
        }
        else if (flags & RTPS_DATAFLAGS_Q)
        {
            /* We are processing a fragment in which the Inline QoS is present
             * but ignored. Move the source pointer forward by the length of the
             * Inline QoS. This code assumes that the Inline QoS received is identical
             * to that received with the first fragment. We do not have a consistency
             * check for subsequent fragments.
             */
            inline_bytes = RTPS_Receiver_get_inline_qos_length(src,
                                                               byte_swap,
                                                               NETIO_Packet_get_payload_length(packet) < INLINE_QOS_OFFSET ?
                                                                            NETIO_Packet_get_payload_length(packet) : INLINE_QOS_OFFSET);
            if (inline_bytes < 0)
            {
                return RTI_FALSE;
            }
            src += inline_bytes;
        }

        dst = &frag_data->pbuf.buffer[start_pos];

        if ((frag_data->pbuf.max_length - start_pos) < copy_bytes)
        {
            OSAPI_Trace_write("Rejecting sample for sn=%S: received fragment "
                              "with more data then space allocated. Potentially a malformed fragment."
                              "Space available = %d and bytes to copy = %d Fragment start in this packet = %d \n",
                                &data_frag->sn,(frag_data->pbuf.max_length - start_pos),
                                copy_bytes,data_frag->fragment_start,NULL,NULL,
                                NULL,NULL,NULL,NULL);
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(dst,src,copy_bytes);

        src += copy_bytes;

        OSAPI_Trace_write("Copied 1 fragment (%^d bytes)\n",
                            &copy_bytes,NULL,
                            NULL,NULL,NULL,NULL,
                            NULL,NULL,NULL,NULL);

        if ((start_pos + copy_bytes) > frag_data->pbuf.tail_pos)
        {
            frag_data->pbuf.tail_pos = start_pos + copy_bytes;
        }
        start_pos += copy_bytes;
        REDA_SequenceNumber_plusplus(&fn);
        fn_count--;
    }

    RTPS_Receiver_update_bitmap_from_fragment(&entry->bitmap,
                                              data_frag,
                                              frag_data,
                                              &entry->highest_sn);
    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
RTPS_Receiver_process_data_frag(struct RTPS_Interface *intf,
                                struct RTPS_PeerEntry *peer_entry,
                                struct NETIO_Guid *writer,
                                NETIO_Packet_T *packet,
                                RTI_UINT8 flags,
                                RTI_UINT32 data_len,
                                struct RTPS_DATA_FRAG *data_frag,
                                RTI_BOOL byte_swap)
{
    struct RTPS_RxFragmentRecord key;
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_RxFragmentRecord *zero_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct RTPS_RxFragmentData *frag_data;
    struct RTPS_RxFragmentData *next_frag;
    RTI_INT32 total_count;
    struct REDA_SequenceNumber SN_ZERO = {0,0};
    struct REDA_SequenceNumber SN_ONE = {0,1};
    struct REDA_SequenceNumber SN_256 = {0,256};
    RTI_INT32 pbuf_cnt;
    RTI_BOOL bit_val;
    OSAPI_TRACE_ONLY_VARIABLE(data_len);
    RTI_BOOL return_buffers = RTI_FALSE;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    UNUSED_ARG(data_len);

    key.writer = *writer;
    key.sn = SN_ZERO;
    zero_entry = NULL;

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&zero_entry,
                                 (DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {

        OSAPI_TRACE_PRINTF1("unbound bound writer: %G",
                            &writer->entity.value[0])

        /* No reader bound to this writer, drop fragment */
        return RTI_TRUE;
    }

    if ((zero_entry->reliable_count == 0) &&
         REDA_SequenceNumber_compare(&zero_entry->writer_lead,&packet->info.sn) < 0)
    {
        /* Delete entries up to lead current lead */
        OSAPI_TRACE_PRINTF1("Updated writer lead based on packet to %S",&packet->info.sn);

        RTPS_Receiver_delete_unavailable_samples(intf,writer,
                                                 &packet->info.sn,
                                                 &packet->info.sn);


        zero_entry->writer_lead = packet->info.sn;
    }

    if (RTPS_Interface_is_reliable(intf))
    {
        if (!RTPS_Bitmap_get_bit(&RTPS_RemoteWriterReliable_bitmap(
                                RTPS_PeerEntry_as_remote_writer(peer_entry)),
                                &bit_val,&packet->info.sn))
        {
            bit_val = RTI_FALSE;
        }

        if (bit_val ||
           (REDA_SequenceNumber_compare(&zero_entry->writer_lead,
                                        &packet->info.sn) > 0))
        {
            /* The SN number to which this fragment belongs is no longer available.
             * drop the fragment.
             */

            OSAPI_Trace_write("fragment for unavailable SN=%S or already received (bitval = %^d), lowest is %S\n",
                              &packet->info.sn,&bit_val,
                              &zero_entry->writer_lead,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);

            return RTI_TRUE;
        }
    }
    else
    {
        /* Already received */
        if (REDA_SequenceNumber_compare(
                         &packet->info.sn,
                         &RTPS_PeerEntry_as_remote_writer(peer_entry)->highest_accepted_sn) <= 0)
        {
            return RTI_TRUE;
        }
    }

    /* At least one reader is bound to the writer and the fragment is within
     * the writers available SNs. Accept the incoming packet.
     */
    key.writer = *writer;
    key.sn = packet->info.sn;
    entry = NULL;

    dbrc = DB_Table_select_match(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&entry,
                                 (DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {
        if (!RTPS_Interface_is_anonymous(intf)
            && RTPS_Interface_is_reliable(intf))
        {
            RTI_BOOL received = RTI_FALSE;

            /* Check if this SN falls outside the writer's receive window when
             * reliability is enabled. If it is then drop the fragment to prevent
             * de fragmentation of samples that will immediately be dropped
             * later due to being outside the window.
             */
            if (!RTPS_Bitmap_set_bit(&RTPS_RemoteWriterReliable_bitmap(
                                     RTPS_PeerEntry_as_remote_writer(peer_entry)),
                                     &received, &packet->info.sn,RTI_TRUE))
            {
                OSAPI_TRACE_NET("rejected out-of-range DATA:",RTI_FALSE)
                OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)
                goto done;
            }

            if (received)
            {
                /* The sample has already been received, nothing to do */
                OSAPI_TRACE_NET("rejected already received DATA:",RTI_FALSE)
                OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)
                goto done;
            }

            /* Clear the bit as the sample has not been received yet */
            if (!RTPS_Bitmap_set_bit(&RTPS_RemoteWriterReliable_bitmap(
                                     RTPS_PeerEntry_as_remote_writer(peer_entry)),
                                     &received, &packet->info.sn,RTI_FALSE))
            {
                goto done;
            }
        }

        /* Only one buffer is required to reassemble a serialized buffer in
         * this implementation.
         */
        total_count = 1;

        if ((zero_entry->reserved_pbuf_count + (RTI_INT32)total_count) >
            RTPS_ReaderFragment_max_fragmented_samples_per_remote_writer(reader))
        {
            OSAPI_TRACE_PRINTF3("max_fragmented_samples_per_remote_writer "
                                "exceeded, %^d already reserved, %^d allowed,"
                                " %^d needed. Sample rejected\n",
                                &zero_entry->reserved_pbuf_count,
                                &RTPS_ReaderFragment_max_fragmented_samples_per_remote_writer(reader),
                                &total_count)
            goto done;
        }

        if (total_count > RTPS_ReaderFragment_pbuf_count(reader))
        {

            OSAPI_TRACE_PRINTF2("out of pbufs, need %^d but only %^d "
                                "available. Sample rejected\n",
                                &total_count,&RTPS_ReaderFragment_pbuf_count(reader))
            goto done;
        }

        entry = NULL;
        dbrc = DB_Table_create_record(RTPS_ReaderFragment_data_frag_table(reader),
                                      (DB_Record_T*)&entry);

        if (dbrc != DB_RETCODE_OK)
        {
            /* If we're out of resources return TRUE otherwise the rest
             * of the RTPS message is discarded.
             */
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        entry->sn = packet->info.sn;
        entry->writer = *writer;
        entry->reserved_pbuf_count = total_count;
        entry->total_fragment_count = data_frag->sample_size / data_frag->fragment_size;

        if (data_frag->sample_size % data_frag->fragment_size)
        {
            /* fragment_size is always the same from the same writer */
            ++entry->total_fragment_count;
        }

        entry->current_fragment_count = 0;
        entry->flags = 0;
        entry->byte_swap = byte_swap;
        entry->timestamp = packet->info.timestamp;
        entry->zero_record = zero_entry;

        entry->bitmap.lead.low = 1;
        entry->bitmap.bit_count = 256;
        entry->highest_sn = 256;

        /* Initialze the bitmap to all 1s, meaning these are not received */
        if (!RTPS_Bitmap_fill(&entry->bitmap,&SN_ONE,&SN_256,RTI_TRUE))
        {
        }

        /* Create the list of pbufs to receive data into */
        entry->head_frag = REDA_BufferPool_get_buffer(RTPS_ReaderFragment_pbuf_pool(reader));
        if (entry->head_frag == NULL)
        {
            dbrc = DB_Table_delete_record(RTPS_ReaderFragment_data_frag_table(reader),
                                                            (DB_Record_T)entry);
            IGNORE_RETVAL(dbrc);
            goto done;
        }

        frag_data = entry->head_frag;
        frag_data->pbuf.head_pos = 0;
        frag_data->pbuf.tail_pos = 0;

        for (pbuf_cnt = 1; pbuf_cnt < entry->reserved_pbuf_count; pbuf_cnt++)
        {
            frag_data->pbuf._next = REDA_BufferPool_get_buffer(RTPS_ReaderFragment_pbuf_pool(reader));
            if (frag_data->pbuf._next == NULL)
            {
                OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                dbrc = DB_Table_delete_record(RTPS_ReaderFragment_data_frag_table(reader),
                                                (DB_Record_T)entry);
                IGNORE_RETVAL(dbrc);
                return_buffers = RTI_TRUE;
                goto done;
            }
            frag_data = (struct RTPS_RxFragmentData *)frag_data->pbuf._next;

            frag_data->pbuf.head_pos = 0;
        }

        frag_data->pbuf._next = NULL;
        entry->tail_frag = frag_data;

        dbrc = DB_Table_insert_record(RTPS_ReaderFragment_data_frag_table(reader),(DB_Record_T)entry);

        if (dbrc != DB_RETCODE_OK)
        {
            /* If we're out of resources return TRUE otherwise the rest
             * of the RTPS message is discarded.
             */
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
            dbrc = DB_Table_delete_record(RTPS_ReaderFragment_data_frag_table(reader),
                                            (DB_Record_T)entry);
            IGNORE_RETVAL(dbrc);
            return_buffers = RTI_TRUE;
            goto done;
        }

        RTPS_ReaderFragment_pbuf_count(reader) -= total_count;
        /* entry points to maintenance record */
        zero_entry->reserved_pbuf_count += total_count;

        OSAPI_Trace_write("New fragmented SN added [SN=%S] %^d %^d %d %^d"
                            " data_len=%^d required pbufs=%^d\n",
                            &packet->info.sn,
                            &data_frag->fragment_start,
                            &data_frag->fragment_count,
                            OSAPI_TRACE_INT_AS_PTR(data_frag->fragment_size & 0x0000ffff),
                            &data_frag->sample_size,
                            &data_len,
                            &total_count,NULL,NULL,NULL);
    }

    /* We only use one pbuf, which is sized large enough to hold the full sample. If any of the fragments cannot
     * fit in the pbuf, then the sample is rejected and all fragments for that sample are deleted. Note that this also
     * means that if the fragment size specified by the sender is larger than the pbuf size, all fragments for that sn will be rejected.
     */
    if (entry->head_frag->pbuf.max_length < (data_frag->sample_size))
    {
        OSAPI_Trace_write("fragment size %d is larger than pbuf size %d, fragment rejected\n",
                                data_frag->fragment_size,entry->head_frag->pbuf.max_length,
                                NULL,NULL,NULL,NULL,
                                NULL,NULL,NULL,NULL);
        RTPS_Receiver_delete_rxtable_entry(intf,entry);
        return RTI_FALSE;
    }

    /* The first fragment must have a possible inline Qos, make sure the
     * Q bit is saved in case fragments are received out of order.
     */
    if (data_frag->fragment_start == 1)
    {
        entry->flags |= flags;
    }

    /* Either insert into an existing record or insert into the newly created
     * record.
     */
    if (!RTPS_Receiver_add_data_frag(packet,data_frag,entry,byte_swap,flags))
    {
        RTPS_Receiver_delete_rxtable_entry(intf,entry);
        return RTI_FALSE;
    }

    if (entry->bitmap.lead.low > entry->total_fragment_count)
    {
        OSAPI_Trace_write("received all the fragments for %S from %G, forward entry",
                          &packet->info.sn,
                          &entry->writer,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL);

        RTPS_Receiver_forward_data(intf,entry);
        RTPS_Receiver_delete_rxtable_entry(intf,entry);
    }

done:

    if (return_buffers)
    {
        frag_data = entry->head_frag;

        while (frag_data != NULL)
        {
            next_frag = RTPS_RxFragmentNode_get_next(frag_data);
            REDA_BufferPool_return_buffer(RTPS_ReaderFragment_pbuf_pool(reader),frag_data);
            frag_data = next_frag;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Compare function fragment index
 *
 * \param[in] record Peer entry already in index
 * \param[in] key_is_record Whether key is peer entry record
 * \param[in] key Key to compare
 *
 * \return Given the left record's address and the right key address, return 0
 * \return positive integer if record is greater than key,
 *         negative integer if record is less than key,
 *         zero if record is equal to key
 */
RTI_PRIVATE RTI_INT32
RTPS_Receiver_compare_datafrag(RTI_INT32 flags,const DB_Record_T op1,void *key)
{
    RTI_INT32 diff;
    struct RTPS_RxFragmentRecord *lval = (struct RTPS_RxFragmentRecord*)op1;
    struct RTPS_RxFragmentRecord *rval = (struct RTPS_RxFragmentRecord *)key;
    UNUSED_ARG(flags);

    diff = OSAPI_Memory_compare(&lval->writer,&rval->writer,
                                sizeof(struct NETIO_Guid));

    if (diff)
    {
        return diff;
    }

    return REDA_SequenceNumber_compare(&lval->sn,&rval->sn);
}

RTI_PRIVATE RTI_BOOL
RTPS_Recevier_initialize_datafrag_pbuf(void *param, void *buffer)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface*)param;
    struct RTPS_RxFragmentData *rxdata = (struct RTPS_RxFragmentData*)buffer;
    UNUSED_ARG(intf);

    rxdata->pbuf.buffer = (char*)buffer +
                           sizeof(struct RTPS_RxFragmentData);

    rxdata->first_fn = 0;
    rxdata->last_fn = 0;
    rxdata->pbuf.head_pos = 0;
    rxdata->pbuf.tail_pos = 0;
    rxdata->pbuf.max_length = RTPS_ReaderFragment_rcv_fragment_size_bytes(RTPS_Interface_as_reader(intf));
    rxdata->pbuf._next = NULL;

    return RTI_TRUE;
}

void
RTPS_Receiver_datafrag_writer_unbind(struct RTPS_Interface *intf,
                                     struct NETIO_Guid *guid,
                                     RTI_BOOL is_reliable)
{
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_RxFragmentRecord *f_entry = NULL;
    struct RTPS_RxFragmentRecord *zero_entry = NULL;
    struct RTPS_RxFragmentRecord key;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    key.writer = *guid;
    REDA_SequenceNumber_set_zero(&key.sn);

    dbrc = DB_Table_select_from(RTPS_ReaderFragment_data_frag_table(reader),
                                DB_TABLE_DEFAULT_INDEX,
                                &cursor,(DB_Key_T)&key);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    OSAPI_TRACE_PRINTF1("unbind writer %G",guid);

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&f_entry);

    /* This loop assumes that the zero entry is the first entry encountered
     * when processing the data_frag_table. This means that zero_entry is always
     * set before the first fragment is deleted in the else statement below.
     * It is also assumes that all fragments after the first entry are for the
     * same writer up until the last fragment for that writer.
     */
    while (dbrc == DB_RETCODE_OK)
    {
        if (OSAPI_Memory_compare(guid,&f_entry->writer,sizeof(struct NETIO_Guid)))
        {
            break;
        }

        if (REDA_SequenceNumber_is_zero(&f_entry->sn))
        {
            zero_entry = f_entry;
            OSAPI_TRACE_PRINTF3("unbind writer %G ref_count=%^d record=%p",
                                &zero_entry->writer,&zero_entry->ref_count,f_entry);

            zero_entry->ref_count--;
            if (is_reliable)
            {
                --zero_entry->reliable_count;
            }
        }
        else
        {
            if (zero_entry != NULL)
            {
                /* SN != 0, must be a fragment */
                if (zero_entry->ref_count == 0)
                {
                    OSAPI_TRACE_PRINTF2("delete entry for writer %G with SN=%S ",
                                        &zero_entry->writer,&f_entry->sn)

                    RTPS_Receiver_delete_rxtable_entry(intf,f_entry);
                }
                else
                {
                    OSAPI_TRACE_PRINTF3("writer %G still bound to reader(s) "
                                        "(%^d) count=%^d",
                                        &zero_entry->writer,&f_entry->sn,&zero_entry->ref_count)

                    /* There are still readers that receive data from this
                    * writer. Maintain the state.
                    */
                }
            }
        }

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&f_entry);
    }
    DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);

    if ((zero_entry != NULL) && (zero_entry->ref_count == 0))
    {
        /* There are no more references to the writer, remove the
         * state record.
         */
        key.writer = *guid;
        REDA_SequenceNumber_set_zero(&key.sn);
        f_entry = NULL;

        dbrc = DB_Table_remove_record(RTPS_ReaderFragment_data_frag_table(reader),
                                      (DB_Record_T*)&f_entry,
                                      (DB_Key_T)&key);

        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }
        else
        {
            dbrc = DB_Table_delete_record(RTPS_ReaderFragment_data_frag_table(reader),
                                          (DB_Record_T)f_entry);


            if (dbrc != DB_RETCODE_OK)
            {
                OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
            }
        }
    }
}

void
RTPS_Receiver_datafrag_writer_bind(struct RTPS_Interface *intf,
                                   struct NETIO_Guid *guid,
                                   RTI_BOOL is_reliable)
{
    DB_ReturnCode_T dbrc;
    struct RTPS_RxFragmentRecord key;
    struct RTPS_RxFragmentRecord *entry = NULL;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    REDA_SequenceNumber_set_zero(&key.sn);
    key.writer = *guid;

    dbrc = DB_Table_select_match(RTPS_ReaderFragment_data_frag_table(reader),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&entry,
                                 (DB_Key_T)&key);

    if (dbrc == DB_RETCODE_OK)
    {
        entry->ref_count++;

        if (is_reliable)
        {
            entry->reliable_count++;
        }

        return;
    }

    dbrc = DB_Table_create_record(RTPS_ReaderFragment_data_frag_table(reader),
                                  (DB_Record_T*)&entry);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    OSAPI_Memory_zero(entry,sizeof(struct RTPS_RxFragmentRecord));
    REDA_SequenceNumber_set_zero(&entry->sn);

    /* Don't know what the first SN will be, start with the first available
     * or until the first HB arrives.
     */
    REDA_SequenceNumber_set_zero(&entry->writer_lead);
    REDA_SequenceNumber_plusplus(&entry->writer_lead);
    entry->writer = *guid;
    entry->ref_count = 1;
    entry->reserved_pbuf_count = 0;
    entry->zero_record = NULL;

    if (is_reliable)
    {
        entry->reliable_count = 1;
    }

    dbrc = DB_Table_insert_record(RTPS_ReaderFragment_data_frag_table(reader),
                                  (DB_Record_T)entry);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR);
    }

    OSAPI_TRACE_PRINTF2("bound to writer: %G, ref_count=%^d\n",
                        &guid->prefix.value[0],&entry->ref_count)

    OSAPI_Trace_write("bound to writer: %G, ref_count=%^d\n",
                      &guid->prefix.value[0],&entry->ref_count,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

}

void
RTPS_Receiver_finalize_datafrag(struct RTPS_Interface *intf)
{
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_RxFragmentRecord *f_entry = NULL;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    if (RTPS_ReaderFragment_data_frag_table(reader) != NULL)
    {
        dbrc = DB_Table_select_all(RTPS_ReaderFragment_data_frag_table(reader),
                                   DB_TABLE_DEFAULT_INDEX,
                                   &cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            return;
        }

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&f_entry);
        while (dbrc == DB_RETCODE_OK)
        {
            RTPS_Receiver_delete_rxtable_entry(intf,f_entry);
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&f_entry);
        }

        DB_Cursor_finish(RTPS_ReaderFragment_data_frag_table(reader),cursor);

#ifndef RTI_CERT
        dbrc = DB_Database_delete_table(intf->db,RTPS_ReaderFragment_data_frag_table(reader));
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_TABLE(OSAPI_LOGKIND_ERROR, dbrc)
        }
#endif
    }

#ifndef RTI_CERT
    if ((RTPS_ReaderFragment_pbuf_pool(reader) != NULL) &&
         !REDA_BufferPool_delete(RTPS_ReaderFragment_pbuf_pool(reader)))
    {

    }
#endif

}

RTI_BOOL
RTPS_Receiver_initialize_datafrag(struct RTPS_Interface *intf,
                          const struct RTPS_InterfaceProperty *const property)
{
    DB_ReturnCode_T db_rc;
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[RTPS_TABLE_NAME_MAX_LEN];
    struct REDA_BufferPoolProperty p = REDA_BufferPoolProperty_INITIALIZER;
    union RT_ComponentFactoryId id;
    struct RTPS_Reader *reader = RTPS_Interface_as_reader(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return RTI_TRUE;
    }

    RTPS_ReaderFragment_data_frag_table(reader)= NULL;
    RTPS_ReaderFragment_max_fragmented_samples_per_remote_writer(reader) = property->max_fragmented_samples_per_remote_writer;
    RTPS_ReaderFragment_rcv_fragment_size_bytes(reader) = property->rcv_fragment_size_bytes;
    RTPS_ReaderFragment_max_fragmented_samples(reader) = property->max_fragmented_samples;

    /* max_fragmented_samples cannot be < 0 */
    tbl_property.max_records = (RTI_SIZE_T)property->max_fragmented_samples;

    id._value = intf->factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'x',
                   (RTI_INT32)property->intf_address.value.rtps_guid.object_id);

    db_rc = DB_Database_create_table(&RTPS_ReaderFragment_data_frag_table(reader),
                                     property->_parent._parent.db,
                                     tbl_name,
                                     (RTI_SIZE_T)sizeof(struct RTPS_RxFragmentRecord),
                                     RTPS_Receiver_compare_datafrag,
                                     &tbl_property);

    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_CREATE_REASSEMBLY_TABLE(OSAPI_LOGKIND_ERROR, db_rc)
        RTPS_Interface_clear_fragmented(intf);
        return RTI_FALSE;
    }

    p.buffer_size = (RTI_SIZE_T)sizeof(struct RTPS_RxFragmentData) +
                        ((property->rcv_fragment_size_bytes + 7U) & ~0x7U);

    p.max_buffers = (RTI_SIZE_T)property->max_fragment_buffers;

    RTPS_ReaderFragment_pbuf_pool(reader) = REDA_BufferPool_new("pbuf_pool",&p,
                                          RTPS_Recevier_initialize_datafrag_pbuf,
                                          intf,
                                          NULL,NULL);

    if (RTPS_ReaderFragment_pbuf_pool(reader) == NULL)
    {
#ifndef RTI_CERT
        db_rc = DB_Database_delete_table(property->_parent._parent.db,
                                 RTPS_ReaderFragment_data_frag_table(reader));
        IGNORE_RETVAL(db_rc);
#endif
        RTPS_Interface_clear_fragmented(intf);
        return RTI_FALSE;
    }

    RTPS_ReaderFragment_pbuf_count(reader) = property->max_fragment_buffers;

    OSAPI_TRACE_PRINTF4("Initialized fragment receiver with: "
                        "max_fragmented_samples=%^d max_fragment_buffers=%^d "
                        "max_fragmented_samples_per_remote_writer=%^d "
                        "fragment_size=%^d\n",
                        &property->max_fragmented_samples,
                        &RTPS_ReaderFragment_pbuf_count(reader),
                        &property->max_fragmented_samples_per_remote_writer,
                        &property->fragment_size_bytes);

    return RTI_TRUE;
}

