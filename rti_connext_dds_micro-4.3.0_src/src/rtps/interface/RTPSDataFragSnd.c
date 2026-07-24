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
#include "RTPSTrust.h"

#define RTPS_FRAGMENTS_PER_HB_FRAG 8

RTI_PRIVATE NETIO_FlowControllerFlowState_T
RTPS_Sender_send_task_run(void *param,RTI_INT32 bits_recvd,RTI_INT32 *bits_used);

/*ci
 * \brief Check if the MTU is exceeded for a specific route
 *
 * \param[in] route_entry The route entry
 * \param[in] bytes       Number of bytes to check against MTU
 *
 * \return RTI_TRUE if the MTU is exceeded, RTI_FALSE otherwise.
 */

RTI_BOOL
RTPS_Sender_is_mtu_exceeded(struct RTPS_RouteEntry *route_entry,
                            RTI_SIZE_T bytes)
{
    return route_entry->mtu < bytes;
}

RTI_BOOL
RTPS_Sender_is_queue_empty(struct RTPS_Interface *intf)
{
    return REDA_CircularList_is_empty(&RTPS_WriterFragment_tx_list(
                                        RTPS_Interface_as_writer(intf)));
}

/*ci
 * \brief Check if a SN has been scheduled in the fragmentation table
 *
 * \param[in] intf       RTPS Interface
 * \param[in] sample_id  SN
 *
 * \return RTI_TRUE if the packet has been scheduled, RTI_FALSE otherwise
 */
RTI_BOOL
RTPS_Sender_is_packet_scheduled(struct RTPS_Interface *intf,
                                RTPS_SampleId_T *sample_id)
{
    struct RTPS_TxFragmentRecord *entry = NULL;
    struct RTPS_TxFragmentRecordKey key;
    DB_ReturnCode_T dbrc;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return RTI_FALSE;
    }

    key.sn = *sample_id;
    key.route_entry = NULL;
    key.peer_entry = NULL;

    dbrc = DB_Table_select_match(RTPS_WriterFragment_tx_frag_table(writer),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&entry,
                                 (DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Return or create and return the packet state for a specific SN
 *
 * \param[in] intf          RTPS interface
 * \param[in] packet        Packet to create state for
 * \param[in] assert_state  Create a new entry if it does not exist
 *
 * \return Packet State for packet, NULL if it doesn't exist or failure to
 *         create a new state.
 */
RTI_PRIVATE struct RTPS_TxFragmentRecord*
RTPS_Sender_get_packet_state(struct RTPS_Interface *intf,
                             NETIO_Packet_T *packet,
                             RTI_BOOL assert_state)
{
    struct RTPS_TxFragmentRecord *packet_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct RTPS_TxFragmentRecordKey key;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    key.sn = packet->info.sn;
    key.route_entry = NULL;
    key.peer_entry = NULL;

    dbrc = DB_Table_select_match(RTPS_WriterFragment_tx_frag_table(writer),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&packet_entry,
                                 &key);

    if (dbrc == DB_RETCODE_OK)
    {
        return packet_entry;
    }

    if (!assert_state)
    {
        return NULL;
    }

    packet_entry = NULL;
    dbrc = DB_Table_create_record(RTPS_WriterFragment_tx_frag_table(writer),
                                  (DB_Record_T*)&packet_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        return NULL;
    }

    packet_entry->sn = packet->info.sn;
    packet_entry->route_entry = NULL;
    packet_entry->peer_entry = NULL;
    packet_entry->kind = RTPS_TXFRAGMENT_STATE_PACKET_KIND;
    packet_entry->state.packet.head_pbuf = packet->head_pbuf;
    packet_entry->state.packet.tail_pbuf = packet->tail_pbuf;

    packet_entry->state.packet.timestamp = packet->info.timestamp;
    packet_entry->state.packet.rtps_flags = packet->info.rtps_flags;

    packet_entry->state.packet.length = NETIO_Packet_get_payload_length(packet);
    packet_entry->state.packet.ref_count = 0;
    packet_entry->state.packet.in_progress = RTI_FALSE;
    packet_entry->state.packet.next_route = NULL;

    /* If the packet has an inline Qos, subtract the inline qos length.
     * The inline qos is always in its own pbuf
     */
    if (packet_entry->state.packet.rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS)
    {
        packet_entry->state.packet.length -=
                            NETIO_PacketBuffer_get_length(packet->head_pbuf);
    }

    packet_entry->state.packet.last_fragment = 0;
    packet_entry->state.packet.packet_ref = packet->ref;

    dbrc = DB_Table_insert_record(RTPS_WriterFragment_tx_frag_table(writer),
                                  (DB_Record_T)packet_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),packet_entry);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_RECORD(OSAPI_LOGKIND_ERROR)
        }
#else
            IGNORE_RETVAL(dbrc);
#endif
        return NULL;
    }

    OSAPI_Trace_write("Created state entry for %S",
                      &packet->info.sn,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    return packet_entry;
}

RTI_PRIVATE RTI_BOOL
RTPS_Sender_set_packet_state(struct RTPS_Interface *intf,
                              const struct REDA_SequenceNumber *const sn,
                              NETIO_EventKind_T state)
{
    struct NETIO_Event evt;

    if (intf->upstr_intf == NULL)
    {
        return RTI_FALSE;
    }

    evt.kind = state;
    evt.value.sn = *sn;

    if (!NETIO_Interface_post_event(intf->upstr_intf,&intf->_parent,&evt))
    {
        /* IN_PROGRESS is the only event that can return false */
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTI_PRIVATE void
RTPS_QueuedPacketNode_set_from_packet(
        RTPS_QueuedPacketNode *node,
        NETIO_Packet_T *packet,
        RTPS_Entity_T *reader_entity,
        RTPS_SampleId_T *gap_sn_start,
        RTPS_SampleId_T *gap_sn_end,
        RTPS_SendFlags_T send_flags,
        struct RTPS_PeerEntry *peer_entry)
{
    node->node_bitmap = 0x00u;
    node->max_length = packet->max_length;
    node->head_pos = packet->head_pos;
    node->tail_pos = packet->tail_pos;
    node->head_pbuf = packet->head_pbuf;
    node->tail_pbuf = packet->tail_pbuf;
    node->ref = packet->ref;
    node->dests = packet->dests;
    node->sn = packet->info.sn;
    node->timestamp = packet->info.timestamp;
    node->rtps_flags = packet->info.rtps_flags;
    node->encapsulation = packet->info.encapsulation;
    node->peer_entry = peer_entry;
    node->send_flags = send_flags;

    if (reader_entity != NULL)
    {
        node->reader_entity = *reader_entity;
    }
    else
    {
        node->node_bitmap |= RTPS_QUEUED_PACKET_RDR_ENTITY_NULL_FLAG;
    }

    if (gap_sn_start != NULL)
    {
        node->gap_sn_start = *gap_sn_start;
    }
    else
    {
        node->node_bitmap |= RTPS_QUEUED_PACKET_SN_START_NULL_FLAG;
    }

    if (gap_sn_end != NULL)
    {
        node->gap_sn_end = *gap_sn_end;
    }
    else
    {
        node->node_bitmap |= RTPS_QUEUED_PACKET_SN_END_NULL_FLAG;
    }

    if (packet->info.valid_data)
    {
        node->node_bitmap |= RTPS_QUEUED_PACKET_VALID_DATA_FLAG;
    }
}

RTI_PRIVATE void
RTPS_Packet_set_from_queued_packet_node(NETIO_Packet_T *packet,
                                        RTPS_QueuedPacketNode *node,
                                        RTPS_Entity_T **reader_entity,
                                        RTPS_SampleId_T **gap_sn_end,
                                        RTPS_SampleId_T **gap_sn_start)
{
    NETIO_Packet_T packet_init = NETIO_Packet_INITIALIZER;
    *packet = packet_init;
    packet->max_length = node->max_length;
    packet->head_pos = node->head_pos;
    packet->tail_pos = node->tail_pos;
    packet->head_pbuf = node->head_pbuf;
    packet->tail_pbuf = node->tail_pbuf;
    packet->ref = node->ref;
    packet->dests = node->dests;
    packet->info.sn = node->sn;
    packet->info.timestamp = node->timestamp;
    packet->info.valid_data = node->node_bitmap & RTPS_QUEUED_PACKET_VALID_DATA_FLAG;
    packet->info.rtps_flags = node->rtps_flags;
    packet->info.encapsulation = node->encapsulation;

    if (!(node->node_bitmap & RTPS_QUEUED_PACKET_RDR_ENTITY_NULL_FLAG))
    {
        *reader_entity = &node->reader_entity;
    }

    if (!(node->node_bitmap & RTPS_QUEUED_PACKET_SN_START_NULL_FLAG))
    {
        *gap_sn_start = &node->gap_sn_start;
    }

    if (!(node->node_bitmap & RTPS_QUEUED_PACKET_SN_END_NULL_FLAG))
    {
        *gap_sn_end = &node->gap_sn_end;
    }
}

/*ci
 * \brief Schedule the flow controller to process packets from an RTPS interface
 *
 * \param[in] intf The RTPS interface
 *
 * \return RTI_TRUE if the flow controller was successfully scheduled, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
RTPS_Sender_schedule_flow(struct RTPS_Interface *intf)
{
    struct NETIO_FlowProperty tp = NETIO_FlowProperty_INITIALIZER;
    RTI_BOOL ok = RTI_FALSE;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (RTPS_WriterFragment_sched_handle(writer) == NULL)
    {
        tp.min_bits_required = (RTI_INT32)((RTPS_WriterFragment_fragment_size_bytes(writer) - RTPS_PROTOCOL_OVERHEAD) * 8);

        RTPS_WriterFragment_sched_handle(writer) = NETIO_FlowController_add_flow(
                                        RTPS_WriterFragment_netio_fc(writer),
                                        &intf->_parent.local_address.value.guid,
                                        RTPS_Sender_send_task_run,
                                        NULL,
                                        intf,&tp);

        if (RTPS_WriterFragment_sched_handle(writer) == NULL)
        {
            goto done;
        }
    }
    else if (!NETIO_FlowController_reschedule_flow(
                RTPS_WriterFragment_netio_fc(writer),RTPS_WriterFragment_sched_handle(writer)))
    {
        goto done;
    }

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_BOOL
RTPS_Sender_queue_packet(struct RTPS_Interface *intf,
                         NETIO_Packet_T *packet,
                         struct RTPS_PeerEntry *peer_entry,
                         RTPS_SendFlags_T send_flags,
                         RTPS_Entity_T *reader_entity,
                         RTPS_SampleId_T *gap_sn_start,
                         RTPS_SampleId_T *gap_sn_end)
{
    RTI_BOOL retval = RTI_FALSE;
    RTPS_QueuedPacketNode *record = NULL;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!OSAPI_Mutex_take(RTPS_WriterFragment_packet_queue_lock(writer)))
    {
        RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR,"packet_queue_lock")
        return RTI_FALSE;
    }
    record = (RTPS_QueuedPacketNode *)
                REDA_BufferPool_get_buffer(RTPS_WriterFragment_packet_queue_pool(writer));
    if (record == NULL)
    {
        goto done;
    }

    /* if the packet is NULL it is potentially a liveliness */
    if (packet == NULL)
    {
        if (send_flags & RTPS_SEND_LIVE_HB_FLAG)
        {
            record->node_bitmap = RTPS_QUEUED_PACKET_LIVELINESS_FLAG;
        }
        else
        {
            goto done;
        }
    }
    else
    {
        RTPS_QueuedPacketNode_set_from_packet(
            record,
            packet,
            reader_entity,
            gap_sn_start,
            gap_sn_end,
            send_flags,
            peer_entry);
    }

    REDA_CircularListNode_init(&record->_parent);

    /* Packets from the built-in writer or Liveliness or periodic HB
     * are added to the front of the queue for priority processing
     */
    if (RTPS_Interface_is_builtin(intf))
    {
        REDA_CircularList_prepend(&RTPS_WriterFragment_packet_queue(writer), &record->_parent);
    }
    else
    {
        REDA_CircularList_append(&RTPS_WriterFragment_packet_queue(writer), &record->_parent);
    }

    if (!RTPS_Sender_schedule_flow(intf))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    if (!OSAPI_Mutex_give(RTPS_WriterFragment_packet_queue_lock(writer)))
    {
        RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR,"packet_queue_lock")
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief This function schedules a request for fragments.
 *
 * \details
 *
 * A response to a NACK_FRAG is similar to that of an ACKNACK except that only
 * a subset of the sample is resent. The subset if based on the bitmap received
 * in the NACK_FRAG. A NACK_FRAG can contain at most 256 fragments.
 *
 * An entry in the fragment table also has a bitmap. If the bitmap has a
 * lead of 0 it is not used and the packet is sent as is. However, if the
 * lead is 0 it means that a specific fragment have been requested of this
 * packet and those are served first. The bitmap is truncated to the
 * highest available fragment.
 *
 * The advantage with this approach is that no additional resources are
 * required. if a NACK_FRAG is received that falls outside of a current
 * bitmap, the current bitmap is merged with the new one reset. However,
 * it means that some fragments may not be resent until the current bitmap
 * has been resent.
 *
 * Open question: The route entries are not directed, they are always sent to
 * a peer. It is an open question if the key should include the peer_entry
 * in case it is a directed send. If this is not the case if there are
 * multiple matched readers on the same route they will simply discard the
 * messages.
 */
RTI_BOOL
RTPS_Sender_schedule_packet(struct RTPS_Interface *intf,
                            NETIO_Packet_T *packet,
                            struct RTPS_Bitmap *bitmap,
                            struct RTPS_RouteEntry *route_entry,
                            struct RTPS_PeerEntry *peer_entry,
                            RTI_BOOL schedule_flow)
{
    struct RTPS_TxFragmentRecord *tx_entry = NULL;
    struct RTPS_TxFragmentRecord *search_entry = NULL;
    struct RTPS_TxFragmentRecord *packet_entry = NULL;
    struct RTPS_TxFragmentRecordKey key;
    DB_ReturnCode_T dbrc;
    RTI_BOOL ok = RTI_FALSE;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    /* Check if there is an existing entry for the {SN,Route} already. */

    key.sn = packet->info.sn;
    key.route_entry = route_entry;
    key.peer_entry = peer_entry;

    dbrc = DB_Table_select_match(RTPS_WriterFragment_tx_frag_table(writer),
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&tx_entry,&key);

    /* There is already an entry for this record. This could happen if
     * a NACK is received while the first sample is sent.
     */
    if (dbrc == DB_RETCODE_OK)
    {

        /* A non-NULL bitmap indicates that specific fragments have been
         * requested by the peer. Replace the existing bitmap if the new
         * bitmap has newer information.
         */
        if ((tx_entry->state.route.bitmap.lead.low > 0) && (bitmap != NULL))
        {
            OSAPI_Trace_write("Bitmap already in progress for SN=%S, bitmap=%B",
                              &packet->info.sn,bitmap,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);
            if (bitmap->lead.low >= tx_entry->state.route.bitmap.lead.low)
            {
                tx_entry->state.route.bitmap = *bitmap;
            }
            tx_entry->state.route.is_completed = RTI_FALSE;
            ok =  RTI_TRUE;
            goto done;
        }

        OSAPI_Trace_write("Packet %S was aleady scheduled with no bitmap, leave as is",
                          &packet->info.sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        ok =  RTI_TRUE;
        goto done;
    }

    /* No entry for {SN,route} existed. First check if {SN,NULL} exists,
     * otherwise create it first. The {SN,NULL} entry holds information
     * that is common to all {SN,route} entries.
     */
    packet_entry = RTPS_Sender_get_packet_state(intf,packet,RTI_TRUE);
    if (packet_entry == NULL)
    {
        OSAPI_Trace_write("Failed to create packet state for %S",
                          &packet->info.sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        goto done;
    }

    /* Create a new entry in the fragmentation table
     */
    tx_entry = NULL;
    dbrc = DB_Table_create_record(RTPS_WriterFragment_tx_frag_table(writer),
                                  (DB_Record_T*)&tx_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    REDA_CircularListNode_init(&tx_entry->_parent);

    /* Keys */
    tx_entry->sn = packet->info.sn;

    OSAPI_Trace_write("Adding route to %G using address %A @ %p",
                      &route_entry->destination,
                      &route_entry->intf_address,
                      tx_entry,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    tx_entry->route_entry = route_entry;
    tx_entry->peer_entry = peer_entry;
    tx_entry->encapsulation = RTPS_RouteEntry_get_encapsulation(route_entry);

    /* Shared state */
    tx_entry->state.route.packet_state = &packet_entry->state.packet;

    /* offset is the offset into the user payload, not including the
     * inline qos
     */
    tx_entry->state.route.offset = 0;
    tx_entry->state.route.packet_state->ref_count++;

    tx_entry->state.route.fragment_last = 0;

    tx_entry->state.route.is_completed = RTI_FALSE;

    if (bitmap != NULL)
    {
        tx_entry->state.route.nack_frag_resend = RTI_TRUE;

        OSAPI_Trace_write("Scheduled packet %S for delivery with %B, length=%^d",
                          &tx_entry->sn,bitmap,
                          &packet_entry->state.packet.length,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);

        tx_entry->state.route.bitmap = *bitmap;
        OSAPI_TRACE_PRINTF1("Created entry for NACK_FRAG for delivery %B",bitmap)

        /* Reset next_route so that the resend is processed before any initial sends */
        tx_entry->state.route.packet_state->next_route = NULL;
    }
    else
    {
        tx_entry->state.route.nack_frag_resend = RTI_FALSE;

        OSAPI_Trace_write("Scheduled packet %S for delivery length=%^d",
                          &tx_entry->sn,
                          &packet_entry->state.packet.length,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        OSAPI_Memory_zero(&tx_entry->state.route.bitmap,
                          sizeof(tx_entry->state.route.bitmap));
    }

    dbrc = DB_Table_insert_record(RTPS_WriterFragment_tx_frag_table(writer),(DB_Record_T)tx_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),tx_entry);
 #if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_RECORD(OSAPI_LOGKIND_ERROR)
        }
#else
            IGNORE_RETVAL(dbrc);
#endif
        goto done;
    }

    if (REDA_CircularList_is_empty(&RTPS_WriterFragment_tx_list(writer)))
    {
        REDA_CircularList_append(&RTPS_WriterFragment_tx_list(writer),&tx_entry->_parent);
    }
    else if (RTPS_Interface_is_writer(intf) && RTPS_Interface_is_builtin(intf))
    {
        /* For builtin writer add to head of the list for priority sending */
        REDA_CircularList_prepend(&RTPS_WriterFragment_tx_list(writer),&tx_entry->_parent);
    }
    else
    {
        /* Packets are sent in increasing order of SN and peer. The first
         * time any Sn is sent to all it will be added to the list. Any
         * subsequent sends of the same SN is repairs. These have priority
         * and are added first.
         */
        search_entry = (struct RTPS_TxFragmentRecord*)
                                REDA_CircularList_get_first(&RTPS_WriterFragment_tx_list(writer));
        while (!REDA_CircularList_node_at_head(&RTPS_WriterFragment_tx_list(writer),search_entry))
        {
            if (REDA_SequenceNumber_compare(&search_entry->sn,&tx_entry->sn) >= 0)
            {
                break;
            }
            else
            {
                search_entry = (struct RTPS_TxFragmentRecord*)
                                        REDA_CircularListNode_get_next(
                                           &search_entry->_parent);
            }
        }

        if (REDA_CircularList_node_at_head(&RTPS_WriterFragment_tx_list(writer),search_entry))
        {
            REDA_CircularList_append(&RTPS_WriterFragment_tx_list(writer),&tx_entry->_parent);
        }
        else
        {
            REDA_CircularList_link_node_after(search_entry->_parent._prev,
                                              &tx_entry->_parent);
        }
    }

    if (schedule_flow && (!RTPS_Sender_schedule_flow(intf)))
    {
        goto done;
    }

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_PRIVATE RTI_BOOL
RTPS_Sender_is_entry_in_same_group(struct RTPS_TxFragmentRecord *entry1,
                                   struct RTPS_TxFragmentRecord *entry2)
{
    return (!REDA_SequenceNumber_compare(&entry1->sn,&entry2->sn) &&
            (((entry1->peer_entry == NULL) &&
              (entry2->peer_entry == NULL)) ||
             ((entry1->peer_entry != NULL) &&
              (entry2->peer_entry != NULL))));
}

RTI_PRIVATE RTI_UINT32
RTPS_Sender_next_fragment_size(struct RTPS_Interface *intf,
                               struct RTPS_TxFragmentRecord *tx_entry,
                               struct RTPS_RouteEntry *route_entry)
{
    RTI_UINT32 fs_bytes;
    RTI_UINT32 bytes_left;
    RTI_UINT32 bytes_to_send;

    UNUSED_ARG(route_entry);

    /* Calculate the number of bytes to include in this fragment:
     *
     * fs_bytes  - The maximum number of bytes in a fragment.
     * mtu_bytes - The maximum number of bytes allowed on the transport.
     * offet     - Current offset into payload starting at 0.
     * length    - The total number of bytes to send.
     */
    fs_bytes = (RTI_UINT32)(RTPS_WriterFragment_fragment_size_bytes(RTPS_Interface_as_writer(intf)) - RTPS_PROTOCOL_OVERHEAD);

    bytes_left = tx_entry->state.route.packet_state->length -
                 tx_entry->state.route.offset;

    if (bytes_left > fs_bytes)
    {
        bytes_to_send = fs_bytes;
    }
    else
    {
        bytes_to_send = bytes_left;
    }

    return bytes_to_send;
}

RTI_PRIVATE NETIO_Packet_T*
RTPS_Sender_get_data_payload(struct RTPS_Interface *intf,
                             struct RTPS_TxFragmentRecord *tx_entry,
                             NETIO_Packet_T *packet,
                             union RTPS_MESSAGES *fragment)
{
    RTI_UINT32 bytes_to_send;

    UNUSED_ARG(intf);

    OSAPI_Memory_zero(fragment,sizeof(struct RTPS_DATA));

    fragment->data.hdr.kind = RTPS_DATA_KIND;

    /* NOTE: When the packet is scheduled the optional inline qos is
     * subtracted from the length as it is not considered part of the
     * fragmented data.
     */
    bytes_to_send = tx_entry->state.route.packet_state->length;

    if ((tx_entry->state.route.packet_state->length > 0) &&
        (tx_entry->state.route.packet_state->rtps_flags & NETIO_RTPS_FLAGS_DATA))
    {
        packet->info.valid_data = RTI_TRUE;
    }
    else
    {
        packet->info.valid_data = RTI_FALSE;
    }

    if (tx_entry->state.route.packet_state->rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS)
    {
        /* NOTE: If the packet has inline Qos, add the length here.
         */
        packet->info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;

        bytes_to_send += NETIO_PacketBuffer_get_length(
                               tx_entry->state.route.packet_state->head_pbuf);
    }
    else
    {
        packet->info.rtps_flags &= ~NETIO_RTPS_FLAGS_INLINEQOS;
    }

    /* Bytes to send is always <= 65535 */
    fragment->data.hdr.length = (RTI_UINT16)bytes_to_send;

    tx_entry->state.route.is_completed = RTI_TRUE;

    OSAPI_Trace_write("Completed sending DATA sn=%S",
                      &tx_entry->sn,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);


    packet->head_pbuf = tx_entry->state.route.packet_state->head_pbuf;
    packet->tail_pbuf = tx_entry->state.route.packet_state->tail_pbuf;

    OSAPI_Trace_write("Created packet head=%p,tail=%p length=%^d,bytes_to_send=%^d",
                      packet->head_pbuf,packet->tail_pbuf,
                      &tx_entry->state.route.packet_state->length,
                      &bytes_to_send,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    return packet;
}

NETIO_Packet_T*
RTPS_Sender_get_next_payload(struct RTPS_Interface *intf,
                             struct RTPS_TxFragmentRecord *tx_entry,
                             RTI_UINT32 max_bits,
                             union RTPS_MESSAGES *fragment,
                             RTI_BOOL *last_fragment)
{
    NETIO_Packet_T *packet = intf->packet;
    struct NETIO_PacketBuffer *frag_pbuf;
    struct NETIO_PacketBuffer *data_pbuf;
    RTI_UINT16 fs_bytes;
    RTI_UINT32 bytes_left;
    RTI_UINT16 fc_per_max_bits;
    RTI_UINT16 fc_per_mtu;
    RTI_UINT32 max_bytes_per_mtu;
    RTI_UINT32 bytes_to_send = 0;
    RTI_UINT32 fragment_offset = 0;
    struct RTPS_Bitmap tmp_bitmap;
    struct REDA_SequenceNumber fn_nack_low;
    struct REDA_SequenceNumber fn_nack_high;
    RTI_BOOL resend = RTI_FALSE;
    RTI_UINT32 fragment_count;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!RTPS_Interface_initialize_packet(intf))
    {
        return NULL;
    }

    /* The original packet is saved in a fragments head_pbuf and tail_pbuf
     */
    packet->info.rtps_flags = tx_entry->state.route.packet_state->rtps_flags;
    packet->info.sn = tx_entry->sn;
    packet->info.timestamp = tx_entry->state.route.packet_state->timestamp;
    packet->info.encapsulation = tx_entry->encapsulation;

    if (tx_entry->state.route.packet_state->length <= tx_entry->route_entry->mtu)
    {
        *last_fragment = RTI_TRUE;
        /* This is a regular DATA payload */
        return RTPS_Sender_get_data_payload(intf,tx_entry,packet,fragment);
    }

    *last_fragment = RTI_FALSE;

    OSAPI_Memory_zero(fragment,sizeof(struct RTPS_DATA_FRAG));

    /* All fragments have DATA */
    packet->info.valid_data = RTI_TRUE;
    fragment->data_frag.hdr.kind = RTPS_DATA_FRAG_KIND;

    /* The payload is split into one more more fragments */

    fs_bytes = (RTI_UINT16)(RTPS_WriterFragment_fragment_size_bytes(RTPS_Interface_as_writer(intf)) - RTPS_PROTOCOL_OVERHEAD);
    fc_per_mtu = (RTI_UINT16)(tx_entry->route_entry->mtu / fs_bytes);
    fc_per_max_bits = (RTI_UINT16)(max_bits / (((RTI_UINT32)fs_bytes) * 8U));
    if (fc_per_max_bits < fc_per_mtu)
    {
        /* Limit the number of fragments which could fit in the MTU to fit in
         * the currently available number of bits.
         */
        fc_per_mtu = fc_per_max_bits;
    }
    if (fc_per_mtu == 0)
    {
        /* RTPS_Sender_send_fragments already verified that the next fragment
         * (full or last partial) fits in max_bits. fc_per_max_bits == 0 only
         * occurs on the last partial fragment of a sample; clamp to 1 so the
         * last-fragment branch below emits a single-fragment DATA_FRAG.
         */
        fc_per_mtu = 1;
    }

    max_bytes_per_mtu = (RTI_UINT32)fs_bytes * (RTI_UINT32)fc_per_mtu;

    fragment->data_frag.sample_size = tx_entry->state.route.packet_state->length;
    fragment->data_frag.fragment_size = fs_bytes;

    fragment_count = fragment->data_frag.sample_size /
                     fragment->data_frag.fragment_size;
    if (fragment->data_frag.sample_size % fragment->data_frag.fragment_size)
    {
        ++fragment_count;
    }

    /* If the highest fragment count is not set, the full send may have
     * fully completed before the NACK_FRAG was received. Set the highest
     * fragment number to the full count.
     */
    if ((tx_entry->state.route.packet_state->last_fragment == 0) &&
            (REDA_SequenceNumber_compare(&tx_entry->sn,
                            &RTPS_Writer_last_completed_sn(writer)) <= 0))
    {
        tx_entry->state.route.packet_state->last_fragment = fragment_count;
    }

    if (tx_entry->state.route.bitmap.lead.low > 0)
    {
        if (!RTPS_Bitmap_get_first_bit(&tx_entry->state.route.bitmap,
                                       &fn_nack_low,RTI_TRUE))
        {
            /* Nothing more to resend. This condition is not expected because
             * the entry should have been removed if the bitmap was empty.
             */
            OSAPI_Trace_write("bitmap for %S = %B, no entries",
                              &packet->info.sn,&tx_entry->state.route.bitmap,
                              NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);
            return NULL;
        }
        else
        {
            tmp_bitmap = tx_entry->state.route.bitmap;

            /* fn_nack_low is now the next lowest fragment number to be resent
             * shift the tmp_bitmap to start at fn_nack_low + 1.
             */
            if (!RTPS_Bitmap_shift(&tmp_bitmap,&fn_nack_low))
            {
                OSAPI_TRACE_PRINTF1("failed to shift bitmap to new lead %S",
                                    &fn_nack_low)
            }
            OSAPI_Trace_write("Found unacked fragment in bitmap (%B), New lead in tmp_bitmap = %B for NACK_FRAG of SN=%S",
                              &tx_entry->state.route.bitmap,&tmp_bitmap,&packet->info.sn,
                              NULL,NULL,NULL,NULL,NULL,NULL,NULL);

            /* Find the maximum contiguous range of fragments to resend
             * starting at fn_nack_low.
             */
            if (!RTPS_Bitmap_get_first_bit(&tmp_bitmap,&fn_nack_high,RTI_FALSE))
            {
                /* Did not find a zero, resend all fragments bitmap */
                fn_nack_high.low  = fn_nack_low.low + ((RTI_UINT32)tx_entry->state.route.bitmap.bit_count - 1U);

                OSAPI_Trace_write("did not find a zero, setting highest "
                                    "acknack to %^d + %d\n",
                                    &fn_nack_low.low,
                                    OSAPI_TRACE_INT_AS_PTR(tx_entry->state.route.bitmap.bit_count - 1),
                                    NULL,NULL,NULL,NULL,
                                    NULL,NULL,NULL,NULL);
            }
            else
            {
                /* Found a zero, highest fragment to resend is one less */
                --fn_nack_high.low;

                OSAPI_Trace_write("found a zero, setting highest acknack to %S\n",
                                    &fn_nack_high,
                                    NULL,NULL,NULL,NULL,
                                    NULL,NULL,NULL,NULL,NULL);
            }

            /* Check that we don't resend fragments which have not yet been sent the first time */
            if (fn_nack_low.low > tx_entry->state.route.packet_state->last_fragment)
            {
                OSAPI_Trace_write("Resend %d fragments for SN=%S, out of range [%S - %S], fragment count = %^d\n",
                                  OSAPI_TRACE_INT_AS_PTR(fn_nack_high.low - fn_nack_low.low + 1),
                                  &packet->info.sn,
                                  &fn_nack_low,&fn_nack_high,&fragment_count,
                                  NULL,NULL,NULL,NULL,NULL);

                tx_entry->state.route.bitmap.lead.low = 0;

                if (tx_entry->state.route.nack_frag_resend)
                {
                    REDA_CircularList_unlink_node(&tx_entry->_parent);

                    /* Resending of fragments complete. Delete the route entry
                     * from the tx table.
                     */
                    RTPS_Sender_delete_txtable_entry(intf,tx_entry);
                }

                return NULL;
            }
            else if (fn_nack_high.low > tx_entry->state.route.packet_state->last_fragment)
            {
                fn_nack_high.low = tx_entry->state.route.packet_state->last_fragment;
            }

            if ((fn_nack_high.low - fn_nack_low.low + 1) > fc_per_mtu)
            {
                /* Justification: (fc_per_mtu - 1) underflow warning.
                 * fc_per_mtu is guaranteed to be at least 1. This is because:
                 * 1. <mtu> is validated during Domain Participant creation to
                 *     ensure it's large enough for at least one fragment.
                 * 2. <fs_bytes> (fragment_size_bytes) is validated in
                 *     RTPS_Interface_add_route to be greater than
                 *     RTPS_PROTOCOL_OVERHEAD, thus always 1 or greater.
                 */
                /* coverity[overflow_const] */
                fn_nack_high.low = fn_nack_low.low + ((RTI_UINT32)fc_per_mtu - 1);
            }

            OSAPI_TRACE_PRINTF3("Resend %d fragments, range is [%S - %S]\n",
                                OSAPI_TRACE_INT_AS_PTR(fn_nack_high.low - fn_nack_low.low + 1),
                                &fn_nack_low,&fn_nack_high);

            OSAPI_Trace_write("Resend %d fragments for SN=%S, range is [%S - %S]\n",
                              OSAPI_TRACE_INT_AS_PTR(fn_nack_high.low - fn_nack_low.low + 1),
                              &packet->info.sn,
                              &fn_nack_low,&fn_nack_high,NULL,
                              NULL,NULL,NULL,NULL,NULL);

            /* Justification: (fc_per_mtu - 1) underflow warning.
             * fc_per_mtu is guaranteed to be at least 1 becuase it is
             * always shifted by 1 in RTPS_Bitmap_shift_value
             */
            /* coverity[overflow_const] */
            fragment_offset = (RTI_UINT32)((RTPS_WriterFragment_fragment_size_bytes(RTPS_Interface_as_writer(intf)) - RTPS_PROTOCOL_OVERHEAD) * (fn_nack_low.low - 1U));
            fragment->data_frag.fragment_start = fn_nack_low.low;
            fragment->data_frag.fragment_count = (RTI_UINT16)(fn_nack_high.low - fn_nack_low.low + 1U);

            if (fn_nack_high.low < fragment_count)
            {
                /* Multiple, equal size fragments */
                bytes_to_send = (RTI_UINT32)fs_bytes * (RTI_UINT32)fragment->data_frag.fragment_count;
            }
            else
            {
                /* Last fragment might be partial, calculate bytes to send
                 * based on total length.
                 */

                /* Justification: (fragment->data_frag.fragment_start - 1)
                 * underflow warning.
                 *
                 * fragment_start is assigned from fn_nack_low.low which must
                 * be at least 1.
                 */
                /* coverity[overflow_const] */
                bytes_to_send = tx_entry->state.route.packet_state->length -
                                  ((fragment->data_frag.fragment_start - 1) *
                                    fragment->data_frag.fragment_size);
            }

            /* Update bitmap to next lead */
            fn_nack_high.low++;
            if (!RTPS_Bitmap_shift(&tx_entry->state.route.bitmap,&fn_nack_high))
            {
                OSAPI_Trace_write("Nothing to resend for for SN=%S",
                                  &packet->info.sn,NULL,NULL,NULL,NULL,
                                  NULL,NULL,NULL,NULL,NULL);

                OSAPI_TRACE_PRINTF0("nothing more to resend, clear bitmap\n");
                tx_entry->state.route.bitmap.lead.low = 0;
            }

            /* If the bitmap is empty, this is the last fragment being resent for now */
            if (!RTPS_Bitmap_get_first_bit(&tx_entry->state.route.bitmap,
                                            &fn_nack_low,RTI_TRUE))
            {
                *last_fragment = RTI_TRUE;

                /* This is the last fragment of this route entry. Only mark the
                 * entry as complete. It will be removed when the fragment has been
                 * sent.
                 */
                tx_entry->state.route.is_completed = RTI_TRUE;
            }

            resend = RTI_TRUE;
        }
    }

    if (!resend)
    {
        /* Calculate the number of bytes to include in this fragment:
         *
         * fs_bytes  - The maximum number of bytes in a fragment.
         * mtu_bytes - The maximum number of bytes allowed on the transport.
         * offet     - Current offset into payload starting at 0.
         * length    - The total number of bytes to send.
         */

        bytes_left = tx_entry->state.route.packet_state->length -
                     tx_entry->state.route.offset;

        if (bytes_left > max_bytes_per_mtu)
        {
            /* More than 1 fragment left */
            bytes_to_send = (RTI_UINT32)fs_bytes * (RTI_UINT32)fc_per_mtu;
            fragment->data_frag.fragment_start = (tx_entry->state.route.offset / fs_bytes) + 1;
            fragment->data_frag.fragment_count = fc_per_mtu;
        }
        else
        {
            /* Last fragment, may not be the full fragment */
            bytes_to_send = bytes_left;
            fragment->data_frag.fragment_start = (tx_entry->state.route.offset / fs_bytes) + 1;
            fragment->data_frag.fragment_count = (RTI_UINT16)(bytes_to_send / (RTI_UINT32)fs_bytes);

            if (bytes_to_send % fs_bytes)
            {
                ++fragment->data_frag.fragment_count;
            }

            *last_fragment = RTI_TRUE;

            /* This is the last fragment of this route entry. Only mark the
             * entry as complete. It will be removed when the fragment has been
             * sent.
             */
            tx_entry->state.route.is_completed = RTI_TRUE;
            OSAPI_Trace_write("Completed sending sn=%S",
                              &tx_entry->sn,NULL,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);
        }

        fragment_offset = tx_entry->state.route.offset;
        tx_entry->state.route.offset += bytes_to_send;
    }

    if ((fragment->data_frag.fragment_start == 1) &&
        (tx_entry->state.route.packet_state->rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS))
    {
        packet->info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
    }
    else
    {
        packet->info.rtps_flags &= ~NETIO_RTPS_FLAGS_INLINEQOS;
    }

    frag_pbuf = &RTPS_WriterFragment_frag_pbuf(writer)[0];
    frag_pbuf->_next = NULL;

    if ((fragment->data_frag.fragment_start == 1) &&
        (tx_entry->state.route.packet_state->head_pbuf->_next != NULL))
    {
        frag_pbuf->buffer = tx_entry->state.route.packet_state->head_pbuf->buffer;
        frag_pbuf->head_pos = tx_entry->state.route.packet_state->head_pbuf->head_pos;
        frag_pbuf->tail_pos = tx_entry->state.route.packet_state->head_pbuf->tail_pos;
        /* The fragment buffer cannot be larger than 64K */
        fragment->data_frag.hdr.length = (RTI_UINT16)NETIO_PacketBuffer_get_length(frag_pbuf);
        frag_pbuf->_next = &RTPS_WriterFragment_frag_pbuf(writer)[1];
        frag_pbuf = &RTPS_WriterFragment_frag_pbuf(writer)[1];
        frag_pbuf->_next = NULL;
        frag_pbuf->buffer = tx_entry->state.route.packet_state->head_pbuf->_next->buffer;
        data_pbuf = tx_entry->state.route.packet_state->head_pbuf->_next;
    }
    else if (tx_entry->state.route.packet_state->head_pbuf->_next != NULL)
    {
        frag_pbuf->buffer = tx_entry->state.route.packet_state->head_pbuf->_next->buffer;
        fragment->data_frag.hdr.length = 0;
        data_pbuf = tx_entry->state.route.packet_state->head_pbuf->_next;
    }
    else
    {
        frag_pbuf->buffer = tx_entry->state.route.packet_state->head_pbuf->buffer;
        fragment->data_frag.hdr.length = 0;
        data_pbuf = tx_entry->state.route.packet_state->head_pbuf;
    }

    frag_pbuf->head_pos = data_pbuf->head_pos + fragment_offset;
    frag_pbuf->tail_pos = frag_pbuf->head_pos + bytes_to_send;

    fragment->data_frag.hdr.length = (RTI_UINT16)((RTI_SIZE_T)fragment->data_frag.hdr.length +
                                     NETIO_PacketBuffer_get_length(frag_pbuf));

    {
        RTI_UINT32 align = (4U - ((RTI_UINT32)fragment->data_frag.hdr.length & 0x3U)) & 0x3U;

        fragment->data_frag.hdr.length = (RTI_UINT16)((RTI_UINT32)fragment->data_frag.hdr.length + align);
        frag_pbuf->tail_pos += align;
    }

    packet->head_pbuf = &RTPS_WriterFragment_frag_pbuf(writer)[0];
    packet->tail_pbuf = frag_pbuf;
    packet->tail_pbuf->_next = NULL;

    /* Cache the highest available fragment number to be used in HEART_BEAT_FRAG. */
    if (tx_entry->state.route.nack_frag_resend && *last_fragment)
    {
        /* Only if we are resending and this is the last fragment in the resent
         * sequence do we want to instead set fragment_last to the last fragment
         * that has been sent on this route as a part of the initial send. This
         * is an optimization to help readers which are behind by more fragments
         * than can be NACKed at once catch back up with the writer.
         */
        struct RTPS_TxFragmentRecordKey key;
        struct RTPS_TxFragmentRecord *entry = NULL;
        DB_ReturnCode_T dbrc;

        key.sn = tx_entry->sn;
        key.route_entry = tx_entry->route_entry;
        key.peer_entry = NULL;

        dbrc = DB_Table_select_match(RTPS_WriterFragment_tx_frag_table(writer),
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T*)&entry,
                                     (DB_Key_T)&key);
        if (dbrc == DB_RETCODE_OK)
        {
            /* The inital send is still in-progress */
            tx_entry->state.route.fragment_last = entry->state.route.fragment_last;
        }
        else if (dbrc == DB_RETCODE_NO_DATA)
        {
            /* The inital send was completed */
            tx_entry->state.route.fragment_last = fragment_count;
        }
        else
        {
            /* Error, could not determine the last fragment number */
            tx_entry->state.route.fragment_last = fragment->data_frag.fragment_start + fragment->data_frag.fragment_count - 1;
        }
    }
    else
    {
        /* Normally the fragment_last in a HEARTBEAT_FRAG should be equal to
         * the fragment_last in the packet so that the reader does not NACK
         * fragments which we have not yet sent or are in the process of sending.
         */
        tx_entry->state.route.fragment_last = fragment->data_frag.fragment_start + fragment->data_frag.fragment_count - 1;
    }

    /* Update the highest fragment number ever sent */
    if (!tx_entry->state.route.nack_frag_resend &&
        (tx_entry->state.route.fragment_last > tx_entry->state.route.packet_state->last_fragment))
    {
        tx_entry->state.route.packet_state->last_fragment = tx_entry->state.route.fragment_last;
    }

    return packet;
}

RTI_UINT32
RTPS_Sender_send_fragments(struct RTPS_Interface *intf,RTI_UINT32 max_bits)
{
    struct RTPS_TxFragmentRecord *tx_entry = NULL;
    struct NETIO_Packet *packet = NULL;
    union RTPS_MESSAGES data_frag;
    struct NETIO_Address dest_addr;
    RTI_UINT32 next_fragment_size = 0;
    RTI_BOOL last_fragment = RTI_FALSE;
    RTI_UINT32 send_flags = 0;
    RTPS_Entity_T reader_entity = RTPS_ENTITY_UNKNOWN;
    NETIO_PacketState_T saved_packet_state;
    RTI_UINT32 bits_sent = 0;
    struct RTPS_TxFragmentRecord *next_entry = NULL;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    /* Do not return 0 unless there are no entries to be sent or if there are not
     * enough bits left to send the next fragment. Otherwise, if an error occurs
     * with sending an entry, remove the entry and try the next one.
     */
    while (tx_entry == NULL)
    {
        if (REDA_CircularList_is_empty(&RTPS_WriterFragment_tx_list(writer)))
        {
            return 0;
        }

        /* Get the next tx_entry to be sent */
        tx_entry = (struct RTPS_TxFragmentRecord *)REDA_CircularList_get_first(&RTPS_WriterFragment_tx_list(writer));
        if (tx_entry->state.route.packet_state->next_route != NULL)
        {
            tx_entry = tx_entry->state.route.packet_state->next_route;
        }

        /* Check if there are enough bits left to send the next fragment */
        next_fragment_size = RTPS_Sender_next_fragment_size(intf,
                                            tx_entry,tx_entry->route_entry);
        if ((next_fragment_size  * 8 ) > max_bits)
        {
            return 0;
        }

        /* Mark the packet as in progress if it is not already */
        if (!tx_entry->state.route.packet_state->in_progress)
        {
            if (!RTPS_Sender_set_packet_state(intf,
                                              &tx_entry->sn,
                                              NETIO_EVENTKIND_PACKET_IN_PROGRESS))
            {
                /* This is expected if the packet has been evicted from
                 * the writer's history before it could be sent.
                 */
                RTPS_Sender_delete_txtable_entry(intf, tx_entry);
                tx_entry = NULL;
                continue;
            }
            tx_entry->state.route.packet_state->in_progress = RTI_TRUE;
        }

        packet = RTPS_Sender_get_next_payload(intf,tx_entry,max_bits,
                                                &data_frag,&last_fragment);
        if (packet == NULL)
        {
            RTPS_Sender_delete_txtable_entry(intf,tx_entry);
            tx_entry = NULL;
            continue;
        }

        OSAPI_Trace_write("Send fragments for %S, start=%^d,count=%d,size=%d,"
                        "sample_size=%^d,hdr.len=%d",
                        &packet->info.sn,
                        &data_frag.data_frag.fragment_start,
                        OSAPI_TRACE_INT_AS_PTR(data_frag.data_frag.fragment_count),
                        OSAPI_TRACE_INT_AS_PTR(data_frag.data_frag.fragment_size),
                        &data_frag.data_frag.sample_size,
                        OSAPI_TRACE_INT_AS_PTR(data_frag.data_frag.hdr.length),
                        NULL,NULL,NULL,NULL);

        NETIO_Address_set_guid(&dest_addr,0,&tx_entry->route_entry->destination);

        if (tx_entry->peer_entry != NULL)
        {
            reader_entity = tx_entry->peer_entry->addr.entity;
        }

        if (data_frag.data_frag.hdr.kind == RTPS_DATA_FRAG_KIND)
        {
            send_flags = RTPS_SEND_DATA_FRAG_FLAG;

            if (last_fragment)
            {
                if (!tx_entry->state.route.nack_frag_resend)
                {
                    /* Indicate that this is the last fragment of the sequence. This
                     * will be used to check if a piggyback HB should be included with
                     * the last fragment in the initial send of a sample.
                     */
                    send_flags |= RTPS_SEND_LAST_DATA_FRAG_FLAG;
                }
                else if ((data_frag.data_frag.hdr.kind == RTPS_DATA_FRAG_KIND)
                            && RTPS_Interface_is_reliable(intf)
                            && (RTPS_WriterReliable_active_reliable_reader_count(writer)
                                + RTPS_WriterReliable_inactive_reliable_reader_count(writer)) > 0)
                {
                    /* Always include a HB_FRAG with the last fragment of a resent
                     * sequence to allow the reader to NACK any fragments which
                     * are still missing.
                     */
                    send_flags |= RTPS_SEND_HB_FRAG_FLAG;
                }
            }

            /* Check if a periodic piggyback HB_FRAG should be included */
            if (!tx_entry->state.route.nack_frag_resend
                && RTPS_Interface_is_reliable(intf)
                && (RTPS_WriterReliable_samples_per_hb(writer) > 0))
            {
                RTI_UINT32 fragments_per_sample, fragments_per_hb;

                fragments_per_sample = data_frag.data_frag.sample_size
                                            / data_frag.data_frag.fragment_size;
                if (data_frag.data_frag.sample_size % data_frag.data_frag.fragment_size)
                {
                    fragments_per_sample++;
                }
                fragments_per_hb = fragments_per_sample
                                        * (RTI_UINT32)RTPS_WriterReliable_samples_per_hb(writer);

                /* Send piggyback HB_FRAGs at a fixed rate of RTPS_FRAGMENTS_PER_HB_FRAG
                 * unless full HBs are being included at a rate equal or greater.
                 */
                if (fragments_per_hb > RTPS_FRAGMENTS_PER_HB_FRAG)
                {
                    tx_entry->route_entry->piggyback_fragment_count++;

                    if (tx_entry->route_entry->piggyback_fragment_count >= RTPS_FRAGMENTS_PER_HB_FRAG)
                    {
                        /* Do not send piggyback HB_FRAG is no reliable readers are matched */
                        if ((RTPS_WriterReliable_active_reliable_reader_count(writer)
                            + RTPS_WriterReliable_inactive_reliable_reader_count(writer)) > 0)
                        {
                            send_flags |= RTPS_SEND_HB_FRAG_FLAG;
                        }

                        tx_entry->route_entry->piggyback_fragment_count = 0;
                    }
                }
            }
        }
        else
        {
            /* This is a regular DATA message, not a fragment being sent
             * with the flow controller.
             */
            send_flags = RTPS_SEND_DATA_NON_FRAG_FLAG;
        }

        if (tx_entry->state.route.nack_frag_resend)
        {
            /* Prevent automatic inclusion of piggyback HBs with resends */
            send_flags |= RTPS_SEND_RESEND_DATA_FLAG;
        }

        /* Save original packet state because create_rtps_msg will
        * modify the head and tail pbufs in the packet.
        */
        NETIO_Packet_save_state(packet, &saved_packet_state);

        RTPS_WriterFragment_tx_entry_cur(writer) = tx_entry;
        if (!RTPS_Interface_create_rtps_msg(intf,
                                            &dest_addr,
                                            tx_entry->route_entry,
                                            tx_entry->peer_entry,
                                            packet,
                                            send_flags,
                                            &reader_entity,
                                            NULL,NULL,NULL,0,
                                            &data_frag))
        {
            /* coverity[uninit_use] */
            /* coverity[misra_c_2012_rule_9_1_violation] */
            RTPS_Interface_restore_packet(
                    intf,
                    packet,
                    &saved_packet_state);
            RTPS_WriterFragment_tx_entry_cur(writer) = NULL;

            RTPS_Sender_delete_txtable_entry(intf,tx_entry);
            tx_entry = NULL;
            continue;
        }
        RTPS_WriterFragment_tx_entry_cur(writer) = NULL;
    }

    if (!RTPS_Sender_send_pdu(intf,tx_entry->route_entry,packet))
    {
    }
    bits_sent = next_fragment_size * 8;
    if ((bits_sent == 0) &&
        (packet->info.rtps_flags & (NETIO_RTPS_FLAGS_UNREGISTER | NETIO_RTPS_FLAGS_DISPOSE)))
    {
        /* check if this is a dispose or unregister message that
         * would result in bits sent to be zero. We set it to 1
         * so that the flow controller sees that some bits were
         * sent and does not treat this as a zero bit send
         * condition.
         */
        bits_sent = 1;
    }

    /* Write down the next entry in the same group so that routes
     * can be given bits in a round-robin fashion.
     */
    next_entry = (struct RTPS_TxFragmentRecord*)REDA_CircularListNode_get_next(
                                                    &tx_entry->_parent);
    if (!REDA_CircularList_node_at_head(&RTPS_WriterFragment_tx_list(writer), &next_entry->_parent)
            && RTPS_Sender_is_entry_in_same_group(next_entry, tx_entry))
    {
        tx_entry->state.route.packet_state->next_route = next_entry;
    }
    else
    {
        tx_entry->state.route.packet_state->next_route = NULL;
    }

    /* The entry was completed, remove it */
    if (tx_entry->state.route.is_completed)
    {
        OSAPI_Trace_write("Route completed, removing  %G using address %A for %s, ref_count=%^d",
                  &tx_entry->route_entry->destination,
                  &tx_entry->route_entry->intf_address,
                  intf->session_name,
                  &tx_entry->state.route.packet_state->ref_count,
                  NULL,NULL,NULL,NULL,NULL,NULL);

        RTPS_Sender_delete_txtable_entry(intf,tx_entry);
    }

    /* restore the packet */
    /* Note: this function is called for both transformed and non-transformed packet.
     * If the packet was transformed this function will also return the
     * loan to the transformed buffer pool
     */

    /* saved_packet_state.pbuf_tail is not initialized before use.
    * The value is saved with NETIO_Packet_save_state prior to reaching
    * restore. Restore can only be reached after save_state is called */
    /* coverity[uninit_use] */
    /* coverity[misra_c_2012_rule_9_1_violation] */
    RTPS_Interface_restore_packet(
            intf,
            packet,
            &saved_packet_state);

    return bits_sent;
}

/*ci
 * \brief Gets a queued packet node and schedules it for delivery
 * \param[in] intf RTPS Interface
 * \param[in] record The node from the queued packets queue to be scheduled
 * \param[in] packet packet to be used to schedule

 \ return RTI_TRUE on success and RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
RTPS_Sender_dequeue_and_schedule(
        struct RTPS_Interface *intf,
        RTPS_QueuedPacketNode *record,
        NETIO_Packet_T *packet)
{
    RTPS_Entity_T *reader_entity = NULL;
    RTPS_SampleId_T *gap_sn_end = NULL;
    RTPS_SampleId_T *gap_sn_start = NULL;
    RTI_BOOL bretval = RTI_FALSE;

    if (record->node_bitmap & RTPS_QUEUED_PACKET_LIVELINESS_FLAG)
    {
        /* this is the liveliness packet.
         * This does not need to be scheduled instead directly
         * send them to the all the peers.*/
        if (!OSAPI_Mutex_take(intf->network_lock))
        {
            RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR, "network_lock")
            return RTI_FALSE;
        }

        bretval = RTPS_Interface_send_liveliness(intf);
#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            RTPS_LOG_SEND_LIVELINESS(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(bretval);
#endif

        if (!OSAPI_Mutex_give(intf->network_lock))
        {
            RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR,"network_lock")
            return RTI_FALSE;
        }
        return RTI_TRUE;
    }
    RTPS_Packet_set_from_queued_packet_node(
            packet,
            record,
            &reader_entity,
            &gap_sn_end,
            &gap_sn_start);

    OSAPI_Trace_write("deqeuued packet: SN=%S head_pbuf=%p/%p/%^d/%^d,tail_buf=%p/%p/%^d/%^d",
                        &record->sn,
                        record->head_pbuf,record->head_pbuf->buffer,
                        &record->head_pos,&record->head_pbuf->tail_pos,
                        record->tail_pbuf,record->tail_pbuf->buffer,
                        &record->tail_pos,&record->tail_pbuf->tail_pos,NULL);

    if (!RTPS_Sender_route_packet(
                intf,
                packet,
                record->peer_entry,
                record->send_flags,
                reader_entity,
                gap_sn_start,
                gap_sn_end,
                NULL,
                0,
                NULL,
                RTPS_SENDMODE_SCHEDULE_PACKET))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}



RTI_PRIVATE NETIO_FlowControllerFlowState_T
RTPS_Sender_send_task_run(void *param,
                          RTI_INT32 bits_recvd,
                          RTI_INT32 *bits_used)
{
    struct RTPS_Interface *intf = param;
    RTI_UINT32 bits_sent;
    NETIO_FlowControllerFlowState_T retstate  = NETIO_FLOW_CONTROLLER_FLOW_STATE_READY;
    NETIO_Packet_T packet = NETIO_Packet_INITIALIZER;
    RTPS_QueuedPacketNode *record = NULL;
    RTI_BOOL bretval = RTI_FALSE;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);
    *bits_used = 0;

   /* we try to deque all the packet from the queue and then we try to
    * send packets downstream based on the bits recvd/used below
    */
    do
    {
        if (!OSAPI_Mutex_take(RTPS_WriterFragment_packet_queue_lock(writer)))
        {
            RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
            return retstate;
        }
        if (!REDA_CircularList_is_empty(&RTPS_WriterFragment_packet_queue(writer)))
        {

            record = (RTPS_QueuedPacketNode *)
                        REDA_CircularList_get_first(
                            &RTPS_WriterFragment_packet_queue(writer));
            REDA_CircularList_unlink_node(&record->_parent);
        }
        else
        {
            if (!OSAPI_Mutex_give(RTPS_WriterFragment_packet_queue_lock(writer)))
            {
                RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
                return retstate;
            }
            break;
        }
        if (!OSAPI_Mutex_give(RTPS_WriterFragment_packet_queue_lock(writer)))
        {
            RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
            return retstate;
        }

        /* Coverity gives a warning that record contains Non-atomic updates of
         * a concurrently shared value. This is not true as explained
         * in MICRO-7316 */

        /* will take and release the n/w lock */
        /* coverity[use : FALSE] */
        bretval = RTPS_Sender_dequeue_and_schedule(intf, record, &packet);

        if (!OSAPI_Mutex_take(RTPS_WriterFragment_packet_queue_lock(writer)))
        {
            RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
            return retstate;
        }
        /* A deep copy of the record has been made in the RTPS_Sender_dequeue_and_schedule
         * function. The original record is now returned to the pool. Details in MICRO-7316.
         */
        /* coverity[use : FALSE] */
        REDA_BufferPool_return_buffer(RTPS_WriterFragment_packet_queue_pool(writer),record);
        if (!OSAPI_Mutex_give(RTPS_WriterFragment_packet_queue_lock(writer)))
        {
            RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
            return retstate;
        }
        if (!bretval)
        {
            break;
        }
    } while (1);

    do
    {
        bits_sent = 0;
        if (!OSAPI_Mutex_take(intf->network_lock))
        {
            RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR, "network_lock")
            return retstate;
        }
        if (REDA_CircularList_is_empty(&RTPS_WriterFragment_tx_list(writer)))
        {
            retstate = NETIO_FLOW_CONTROLLER_FLOW_STATE_COMPLETE;
        }
        else
        {
            retstate = NETIO_FLOW_CONTROLLER_FLOW_STATE_READY;
            bits_sent =
                RTPS_Sender_send_fragments(intf, (RTI_UINT32)(bits_recvd - *bits_used));
            *bits_used += (RTI_INT32)bits_sent;
        }
        if(!OSAPI_Mutex_give(intf->network_lock))
        {
            RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR, "network_lock")
            break;
        }
    } while ((*bits_used < bits_recvd) && (bits_sent != 0));

    return retstate;
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
RTI_INT32
RTPS_Sender_compare_tx_frag(RTI_INT32 flags,const DB_Record_T op1, void *key)
{
    RTI_INT32 diff;
    struct RTPS_TxFragmentRecord *lval = (struct RTPS_TxFragmentRecord*)op1;
    struct RTPS_TxFragmentRecord *rval = (struct RTPS_TxFragmentRecord *)key;
    UNUSED_ARG(flags);

    diff = REDA_SequenceNumber_compare(&lval->sn,&rval->sn);
    if (diff)
    {
        return diff;
    }

    /* Sort by peers in decending order */
    if (lval->peer_entry > rval->peer_entry)
    {
        return -1;
    }

    if (lval->peer_entry < rval->peer_entry)
    {
        return 1;
    }

    if (lval->route_entry > rval->route_entry)
    {
        return 1;
    }

    if (lval->route_entry < rval->route_entry)
    {
        return -1;
    }

    return 0;
}

void
RTPS_Sender_delete_txtable_peer_entry(struct RTPS_Interface *intf,
                                      struct RTPS_PeerEntry *peer_entry,
                                      struct RTPS_RouteEntry *route_entry)
{
    DB_Cursor_T cursor = NULL;
    DB_Cursor_T cursor2 = NULL;
    struct RTPS_TxFragmentRecord *tx_entry;
    struct RTPS_TxFragmentRecord *removed;
    DB_ReturnCode_T dbrc;
    struct RTPS_RouteEntry *route_entry2 = NULL;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    dbrc = DB_Table_select_all(RTPS_WriterFragment_tx_frag_table(writer),
                               DB_TABLE_DEFAULT_INDEX,
                               &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_Trace_write("Failed to select all routes",
                            NULL,NULL,NULL,NULL,NULL,
                            NULL,NULL,NULL,NULL,NULL);
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&tx_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        /* If the entry to remove is an exact match then remove this entry
         */
        if ((tx_entry->route_entry == route_entry) &&
            (tx_entry->peer_entry == peer_entry) &&
            (tx_entry->peer_entry != NULL))
        {
            OSAPI_Trace_write("Delete exact match from send table",
                              NULL,NULL,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);

            DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);

            RTPS_Sender_delete_txtable_entry(intf,tx_entry);

            dbrc = DB_Table_select_all(RTPS_WriterFragment_tx_frag_table(writer),
                                       DB_TABLE_DEFAULT_INDEX,
                                       &cursor);
            if (dbrc != DB_RETCODE_OK)
            {
                OSAPI_Trace_write("Failed to select all routes",
                                  NULL,NULL,NULL,NULL,NULL,
                                  NULL,NULL,NULL,NULL,NULL);
                return;
            }
        }
        else if (tx_entry->route_entry == route_entry)
        {
            /* This entry is shared with multiple destinations. Determine if
             * there is another entry to replace it with. If there is then
             * update the route entry pointer, otherwise delete the entry
             */
            dbrc = DB_Table_select_all(intf->_parent._rtable,
                                       DB_TABLE_DEFAULT_INDEX,
                                       &cursor2);
            if (dbrc != DB_RETCODE_OK)
            {
                OSAPI_Trace_write("Failed to select all routes",
                                NULL,NULL,NULL,NULL,NULL,
                                NULL,NULL,NULL,NULL,NULL);
            }
            else
            {
                RTI_BOOL replaced = RTI_FALSE;

                dbrc = DB_Cursor_get_next(cursor2,(DB_Record_T*)(&route_entry2));
                while (dbrc == DB_RETCODE_OK)
                {
                    /* A route can only be replaced by another selected route
                     * because a route that was selected for one reader may
                     * not be the selected route for another reader and should
                     * not replace a removed route.
                     */
                    if ((route_entry2->intf == tx_entry->route_entry->intf) &&
                        (!RTPS_RouteEntryAddress_compare(&route_entry2->intf_address,
                                                &tx_entry->route_entry->intf_address)) &&
                        RTPS_RouteEntry_is_selected_group(route_entry2))
                    {
                        DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);

                        /* Remove the record and add it back so it is
                         * sorted again
                         */
                        removed = NULL;
                        dbrc = DB_Table_remove_record(RTPS_WriterFragment_tx_frag_table(writer),
                                                    (DB_Record_T*)&removed,
                                                    (DB_Key_T)tx_entry);
                        if (dbrc != DB_RETCODE_OK)
                        {
                            OSAPI_Trace_write("Failed to remove tx_entry",
                                              NULL,NULL,NULL,NULL,NULL,
                                              NULL,NULL,NULL,NULL,NULL);
                        }
                        else
                        {
                            tx_entry->route_entry = route_entry2;
                            dbrc = DB_Table_insert_record(RTPS_WriterFragment_tx_frag_table(writer),
                                                        (DB_Record_T*)tx_entry);
                            if (dbrc != DB_RETCODE_OK)
                            {
                                OSAPI_Trace_write("Failed to reinsert tx_entry",
                                                  NULL,NULL,NULL,NULL,NULL,
                                                  NULL,NULL,NULL,NULL,NULL);
                            }
                            else
                            {
                                OSAPI_Trace_write("Replaced tx_entry",
                                                  NULL,NULL,NULL,NULL,NULL,
                                                  NULL,NULL,NULL,NULL,NULL);
                            }
                        }

                        dbrc = DB_Table_select_all(RTPS_WriterFragment_tx_frag_table(writer),
                                                   DB_TABLE_DEFAULT_INDEX,
                                                   &cursor);
                        if (dbrc != DB_RETCODE_OK)
                        {
                            OSAPI_Trace_write("Failed to select all routes",
                                              NULL,NULL,NULL,NULL,NULL,
                                              NULL,NULL,NULL,NULL,NULL);
                        }
                        replaced = RTI_TRUE;
                        break;
                    } /* if matched entry */
                    dbrc = DB_Cursor_get_next(cursor2,(DB_Record_T*)&route_entry2);
                }
                DB_Cursor_finish(intf->_parent._rtable,cursor2);

                if (!replaced)
                {
                    DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);

                    RTPS_Sender_delete_txtable_entry(intf,tx_entry);

                    dbrc = DB_Table_select_all(RTPS_WriterFragment_tx_frag_table(writer),
                                               DB_TABLE_DEFAULT_INDEX,
                                               &cursor);
                    if (dbrc != DB_RETCODE_OK)
                    {
                        OSAPI_Trace_write("Failed to select all routes",
                                            NULL,NULL,NULL,NULL,NULL,
                                            NULL,NULL,NULL,NULL,NULL);
                        return;
                    }
                }
            }
        } /* else if */
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&tx_entry);
    }

    DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);
}

void
RTPS_Sender_delete_txtable_entry(struct RTPS_Interface *intf,
                                 struct RTPS_TxFragmentRecord *key)
{
    struct RTPS_TxFragmentRecord *removed = NULL;
    struct RTPS_TxFragmentRecord *an_entry = NULL;
    struct RTPS_TxFragmentRecord state_key;
    DB_ReturnCode_T dbrc;
    NETIO_Packet_T packet = NETIO_Packet_INITIALIZER;
    RTI_BOOL bretval = RTI_FALSE;
    DB_Cursor_T cursor = NULL;
    RTI_BOOL delete_state = RTI_FALSE;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    OSAPI_TRACE_PRINTF1("delete tx entry for SN %S\n",&key->sn)

    cursor = NULL;

    if (key->route_entry != NULL)
    {
        removed = NULL;
        dbrc = DB_Table_remove_record(RTPS_WriterFragment_tx_frag_table(writer),
                                    (DB_Record_T*)&removed,
                                    (DB_Key_T)key);
        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_Trace_write("Failed to removed packet entry %S",
                              &key->sn,NULL,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);
            return;
        }

        OSAPI_Trace_write("Removed route to %G using address %A for %s, ref_count=%^d",
                          &removed->route_entry->destination,
                          &removed->route_entry->intf_address,
                          intf->session_name,
                          &removed->state.route.packet_state->ref_count,
                          NULL,NULL,NULL,NULL,NULL,NULL);

        REDA_CircularList_unlink_node(&removed->_parent);

        removed->state.route.packet_state->ref_count -= 1;

        dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),
                                    (DB_Record_T)removed);
        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_Trace_write("Failed to delete packet entry %S",
                              &key->sn,NULL,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);
            return;
        }

        if (removed->state.route.packet_state->ref_count == 0)
        {
            delete_state = RTI_TRUE;
        }
        else if (removed->state.route.packet_state->next_route == removed)
        {
            removed->state.route.packet_state->next_route = NULL;
        }

        /* Update the last completed SN if this was an inital send of SN */
        if ((removed->peer_entry == NULL)
            && REDA_SequenceNumber_compare(&RTPS_Writer_last_completed_sn(writer),
                                           &key->sn) < 0)
        {
            if (!REDA_CircularList_is_empty(&RTPS_WriterFragment_tx_list(writer)))
            {
                /* Only update the last_completed_sn if that SN has finished being
                 * sent on all routes.
                 */
                struct RTPS_TxFragmentRecord *entry = NULL;

                entry = (struct RTPS_TxFragmentRecord*)
                            REDA_CircularList_get_first(
                                    &RTPS_WriterFragment_tx_list(writer));

                /* tx_list entries are sorted by sequence number, so if the next
                 * entry has a sequence number greater than the one we are removing
                 * then we know that we have finished sending all entries with this SN.
                 */
                if (REDA_SequenceNumber_compare(&key->sn,&entry->sn) < 0)
                {
                    RTPS_Writer_last_completed_sn(writer) = key->sn;
                }
            }
            else
            {
                RTPS_Writer_last_completed_sn(writer) = key->sn;
            }
        }
    }
    else
    {
        /* Delete all entries */
        dbrc = DB_Table_select_from(RTPS_WriterFragment_tx_frag_table(writer),
                                    DB_TABLE_DEFAULT_INDEX,
                                    &cursor,(DB_Key_T)key);

        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T)&an_entry);
        while (dbrc == DB_RETCODE_OK)
        {
            if (REDA_SequenceNumber_compare(&an_entry->sn,&key->sn) > 0)
            {
                /* Different sequence number, done */
                break;
            }

            if (an_entry->route_entry != NULL)
            {
                removed = NULL;
                dbrc = DB_Table_remove_record(RTPS_WriterFragment_tx_frag_table(writer),
                                              (DB_Record_T*)&removed,
                                              (DB_Key_T)an_entry);
                if (dbrc != DB_RETCODE_OK)
                {

                }

                REDA_CircularList_unlink_node(&removed->_parent);

                removed->state.route.packet_state->ref_count -= 1;
                if (removed->state.route.packet_state->ref_count == 0)
                {
                    delete_state = RTI_TRUE;
                }
                else if (removed->state.route.packet_state->next_route == removed)
                {
                    removed->state.route.packet_state->next_route = NULL;
                }

                dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),removed);

                if (dbrc != DB_RETCODE_OK)
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                }
            }

            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T)&an_entry);
        }

        DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);
    }

    if (delete_state)
    {
        removed = NULL;
        state_key = *key;
        state_key.route_entry = NULL;
        state_key.peer_entry = NULL;

        dbrc = DB_Table_remove_record(RTPS_WriterFragment_tx_frag_table(writer),
                                      (DB_Record_T*)&removed,
                                      (DB_Key_T)&state_key);

        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }

        if (removed->state.packet.in_progress)
        {
            /* Restore sufficient state for upstream to return the loan */
            packet.info.sn = removed->sn;
            packet.info.rtps_flags = removed->state.packet.rtps_flags;
            packet.ref = removed->state.packet.packet_ref;

            if (intf->upstr_intf != NULL)
            {
                bretval = NETIO_Interface_return_loan(intf->upstr_intf,
                                                      NULL,
                                                      &packet,
                                                      &packet.info.sn);
                IGNORE_RETVAL(bretval);
            }
        }

        dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),removed);

        if (dbrc != DB_RETCODE_OK)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }

        OSAPI_Trace_write("Removed state record for %S",
                          &key->sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
    }

    OSAPI_TRACE_PRINTF1("deleted tx entry for SN %S",&packet.info.sn)
}

/*ci
 * \brief Helper function to remove a given packet id from the
 *        packet_queue and return it to the buffer pool. If the
 *        packet_id is NULL all of the packets from the packet queue.
 *
 * \param[in] intf RTPS Interface
 * \param[in] packet_id sequence number of the packet to remove.
 *                      NULL packet_id means remove all the packets.
 */
RTI_PRIVATE void
RTPS_Sender_dequeue_and_return_packet(struct RTPS_Interface *intf,
                              NETIO_PacketId_T *packet_id)
{
    RTPS_QueuedPacketNode *record = NULL;
    RTPS_QueuedPacketNode *record_next = NULL;
    RTI_BOOL remove_all = (packet_id == NULL) ? RTI_TRUE : RTI_FALSE;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!OSAPI_Mutex_take(RTPS_WriterFragment_packet_queue_lock(writer)))
    {
        RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
        return;
    }
    if (REDA_CircularList_is_empty(&RTPS_WriterFragment_packet_queue(writer)))
    {
        /* Nothing to do */
        goto done;
    }
    record = (RTPS_QueuedPacketNode *)
                    REDA_CircularList_get_first(&RTPS_WriterFragment_packet_queue(writer));

    while (!REDA_CircularList_node_at_head(&RTPS_WriterFragment_packet_queue(writer),record))
    {
        record_next = (RTPS_QueuedPacketNode *)
                            REDA_CircularListNode_get_next(&record->_parent);

        if (remove_all == RTI_FALSE)
        {
            if (REDA_SequenceNumber_compare(
                        packet_id,
                        &record->sn) == 0)
            {
                REDA_CircularList_unlink_node(&record->_parent);
                REDA_BufferPool_return_buffer(RTPS_WriterFragment_packet_queue_pool(writer),record);
            }
        }
        else
        {
            REDA_CircularList_unlink_node(&record->_parent);
            REDA_BufferPool_return_buffer(RTPS_WriterFragment_packet_queue_pool(writer),record);
        }
        record = record_next;
    }

done:
    if (!OSAPI_Mutex_give(RTPS_WriterFragment_packet_queue_lock(writer)))
    {
        RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR, "packet_queue_lock")
    }
    return;
}

void
RTPS_Sender_deschedule_packet(struct RTPS_Interface *intf,
                              NETIO_PacketId_T *packet_id)
{

    /* check if the packet is in the packet queue and remove */
    if (RTPS_Interface_is_fragmented(intf))
    {
        RTPS_Sender_dequeue_and_return_packet(intf, packet_id);
    }
    return;
}

/*ci
 * \brief Append a HEARTBEAT_FRAG if a fragmented sample is being sent.
 *
 * \details
 *
 * A HEARTBEAT_FRAG is only piggy-backed as a regular HB will be periodically
 * sent.
 */
void
RTPS_Sender_add_heartbeat_frag_ext(struct RTPS_Interface *intf,
                                   RTPS_Entity_T reader_entity,
                                   RTPS_Entity_T writer_entity)
{
    struct RTPS_HEARTBEAT_FRAG *hbf = NULL;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);
    struct RTPS_TxFragmentRecord *tx_entry = RTPS_WriterFragment_tx_entry_cur(writer);

    if ((tx_entry == NULL) || (tx_entry->state.route.fragment_last == 0))
    {
        return;
    }

    hbf = (struct RTPS_HEARTBEAT_FRAG*)NETIO_PacketBuffer_get_tail(&intf->tail_pbuf);
    if (hbf == NULL)
    {
        return;
    }

    if (!NETIO_PacketBuffer_adjust_tail(&intf->tail_pbuf,
                                        (sizeof(struct RTPS_HEARTBEAT_FRAG))))
    {
        return;
    }

    RTPS_Interface_set_submessage_header(&hbf->hdr,RTPS_HEARTBEAT_FRAG_KIND,
                                         RTPS_FLAGS_NONE,
                                         sizeof(struct RTPS_HEARTBEAT_FRAG));

    hbf->reader = reader_entity;
    hbf->writer = writer_entity;

    /* Sequence # of sample being sent. All SN < than this receives a regular
     * HEARTBEAT
     */
    hbf->writer_sn = tx_entry->sn;

    /* The highest available fragment number for this tx_entry */
    hbf->last_fn = tx_entry->state.route.fragment_last;

    hbf->count = ++RTPS_WriterReliable_hb_epoch(writer);
}

/*ci
 * \brief Process a NACK_FRAG
 *
 * \details
 *
 * When this function is called the state of the stream is:
 *
 * writer_sn (64 bit) - The sequence number the NACK_FRAG is for
 * fragment_set (bit) - Bitmap for missing fragments
 */
void
RTPS_Sender_process_nack_frag_ext(struct RTPS_Interface *intf,
                                  struct RTPS_PeerEntry *peer_entry,
                                  RTPS_SampleId_T *writer_sn,
                                  struct RTPS_Bitmap *bitmap)
{
    NETIO_Packet_T *packet = NULL;
    struct RTPS_ResendContext ctxt = RTPS_ResentContext_INITIALIZER;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(intf);

    if (!RTPS_Interface_is_fragmented(intf))
    {
        return;
    }

    OSAPI_Trace_write("Received nack_frag for SN=%S bitmap=%B %G",
                        writer_sn,bitmap,&peer_entry->addr,NULL,NULL,
                        NULL,NULL,NULL,NULL,NULL);

    ctxt.pull_mode = RTI_FALSE;
    ctxt.writer_entity = intf->_parent.local_address.value.guid.entity;
    ctxt.reader_entity = peer_entry->addr.entity;
    ctxt.req_sn = *writer_sn;

    OSAPI_Trace_write("Request packet %S upstream for retransmission",
                      &ctxt.req_sn,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    /* The SN may or may not already be in the process of being resent. It
     * will be sorted in out in RTPS_Sender_schedule_packet if it is.
     */
    if (!RTPS_Writer_request_and_resend_packet(intf,
                                               writer,
                                               RTPS_PeerEntry_as_remote_reader(peer_entry),
                                               peer_entry,
                                               &packet,
                                               &ctxt,
                                               NULL,
                                               bitmap))
    {
        OSAPI_TRACE_PRINTF0("failed to resend packet w/fragments\n")
    }
}


RTI_BOOL
RTPS_Sender_intialize_datafrag(struct RTPS_Interface *rtps_intf,
                       const struct RTPS_InterfaceProperty *const property)
{
    DB_ReturnCode_T db_rc;
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[RTPS_TABLE_NAME_MAX_LEN];
    union RT_ComponentFactoryId id;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(rtps_intf);

    if (!RTPS_Interface_is_fragmented(rtps_intf))
    {
        return RTI_TRUE;
    }

    RTPS_WriterFragment_netio_fc(writer) = property->netio_fc;
    RTPS_WriterFragment_fragment_size_bytes(writer) = property->fragment_size_bytes;
    RTPS_WriterFragment_tx_frag_table(writer) = NULL;
    REDA_CircularList_init(&RTPS_WriterFragment_tx_list(writer));
    RTPS_WriterFragment_tx_entry_cur(writer) = NULL;
    RTPS_WriterFragment_packet_queue_lock(writer) = OSAPI_Mutex_new();
    if (RTPS_WriterFragment_packet_queue_lock(writer) == NULL)
    {
        RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    REDA_CircularList_init(&RTPS_WriterFragment_packet_queue(writer));
    RTPS_WriterFragment_sched_handle(writer) = NULL;

    OSAPI_Trace_write("Initialize tx fragment table, fragment size = %^d,peer_count=%^d,max_fragmented_samples=%^d\n",
                      &property->fragment_size_bytes,&property->max_peer_count,
                      &property->max_fragmented_samples,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);


    /* The fragmentation table */
    id._value = rtps_intf->factory->_parent._id._value;

    NETIO_Interface_Table_name_from_id(tbl_name,&id,'f',
                   (RTI_INT32)property->intf_address.value.rtps_guid.object_id);

    /* Max_routes is calculated to be max_peers * routes_per_peer.
     * Needs one additional state sample per fragmented samples hence
     * + property->max_fragmented_samples
     */

    tbl_property.max_records = (property->_parent.max_routes + 1) *
                               (RTI_UINT32)property->max_fragmented_samples;
    tbl_property.max_cursors = 2;
    db_rc = DB_Database_create_table(&RTPS_WriterFragment_tx_frag_table(writer),
                                     property->_parent._parent.db,
                                     tbl_name,
                                     sizeof(struct RTPS_TxFragmentRecord),
                                     RTPS_Sender_compare_tx_frag,
                                     &tbl_property);

    if (db_rc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    pool_property.buffer_size = RTI_SIZEOF(RTPS_QueuedPacketNode);
    pool_property.max_buffers = (property->_parent.max_routes) *
                                (RTI_UINT32)property->max_fragmented_samples;

    RTPS_WriterFragment_packet_queue_pool(writer) = REDA_BufferPool_new(
            "send_q_pool",
            &pool_property,
            NULL,
            NULL,
            NULL,
            NULL);

    if (RTPS_WriterFragment_packet_queue_pool(writer) == NULL)
    {
        RTPS_Sender_finalize_datafrag(rtps_intf, RTI_FALSE);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


/*ci brief
 * Finalize the data frag. Can be called with or without holding the nw lock. In the first case
 * the lock is release so that the flow controller can be progress and remove the flow.
 */
extern void
RTPS_Sender_finalize_datafrag(struct RTPS_Interface *rtps_intf, RTI_BOOL with_nw_lock)
{
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_TxFragmentRecord *entry;
    struct RTPS_Writer *writer = RTPS_Interface_as_writer(rtps_intf);
#ifndef RTI_CERT
    RTI_BOOL bretval;
#endif /* RTI_CERT */

    if (!RTPS_Interface_is_fragmented(rtps_intf))
    {
        return;
    }

    /* release the nw lock so that the remove flow can progress and remove flows.*/
    if (with_nw_lock)
    {
        if(!OSAPI_Mutex_give(rtps_intf->network_lock))
        {
            RTPS_LOG_UNLOCK(OSAPI_LOGKIND_ERROR,"network_lock")
            return;
        }
    }

    if (RTPS_WriterFragment_sched_handle(writer) != NULL)
    {
        if (!NETIO_FlowController_remove_flow(
                                RTPS_WriterFragment_netio_fc(writer),
                                RTPS_WriterFragment_sched_handle(writer)))
        {

        }
        RTPS_WriterFragment_sched_handle(writer) = NULL;
    }

    if (with_nw_lock)
    {
        if (!OSAPI_Mutex_take(rtps_intf->network_lock))
        {
            RTPS_LOG_LOCK(OSAPI_LOGKIND_ERROR,"network_lock")
            return;
        }
    }

    dbrc = DB_Table_select_all_default(RTPS_WriterFragment_tx_frag_table(writer),&cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }
    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&entry);
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(RTPS_WriterFragment_tx_frag_table(writer),
                                          (DB_Record_T)entry);
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(RTPS_WriterFragment_tx_frag_table(writer),cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        return;
    }
#ifndef RTI_CERT
    dbrc = DB_Database_delete_table(rtps_intf->db,RTPS_WriterFragment_tx_frag_table(writer));

    if (dbrc != DB_RETCODE_OK)
    {
        RTPS_LOG_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
        return;
    }
#endif /* RTI_CERT */

    RTPS_Sender_dequeue_and_return_packet(rtps_intf, NULL);

    if (RTPS_WriterFragment_packet_queue_pool(writer) != NULL)
    {
#ifndef RTI_CERT
        bretval = REDA_BufferPool_delete(RTPS_WriterFragment_packet_queue_pool(writer));
#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        RTPS_LOG_BUFFERPOOL_DELETE(OSAPI_LOGKIND_WARNING)
    }
#else
        IGNORE_RETVAL(bretval);
#endif /* OSAPI_ENABLE_LOG */
#endif /* RTI_CERT */
        RTPS_WriterFragment_packet_queue_pool(writer) = NULL;
    }

    if (RTPS_WriterFragment_packet_queue_lock(writer) != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Mutex_delete(RTPS_WriterFragment_packet_queue_lock(writer));
#endif
        RTPS_WriterFragment_packet_queue_lock(writer) = NULL;
    }
}
