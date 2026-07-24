/*
 * FILE: NETIOPacket.c - Packet functionality
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 30may2013,eh MICRO-378: set_head_tail bounds
 * 17may2013,eh MICRO-385: remove unused fns/fields of packet
 * 05apr2013,eh Fix MICRO-378: get_head/tail
 * 27apr2012,tk Written
 *
 * *** API: GREEN
 * *** CODE: GREEN
 */

/*ce
 * \file
 * \brief NETIO Packet implementation
 *
 * \details
 * This file implements the NETIO_Packet data-type. Functions to manipulate the
 * contents of the packet is found here.
 *
 * \addtogroup NETIO_PacketClass
 * @{
 */

#include "osapi/osapi_config.h"

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif

/*** SOURCE_BEGIN ***/

void
NETIO_Packet_save_positions(NETIO_Packet_T *const packet)
{
    OSAPI_PRECONDITION(packet == NULL,return,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    packet->saved_head_pos = packet->head_pos;
    packet->saved_tail_pos = packet->tail_pos;
}

void
NETIO_Packet_restore_positions(NETIO_Packet_T *const packet)
{
    OSAPI_PRECONDITION(packet == NULL,return,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    packet->head_pos = packet->saved_head_pos;
    packet->tail_pos = packet->saved_tail_pos;
}

RTI_BOOL
NETIO_Packet_set_head(NETIO_Packet_T *const packet,RTI_INT32 delta)
{
    OSAPI_PRECONDITION(packet == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    /* verify delta does not go beyond, or wrap around, head and tail */
    if (((delta > 0) &&
         ((RTI_SIZE_T)delta > (packet->tail_pos - packet->head_pos))) ||
        ((delta < 0) &&
         (((RTI_SIZE_T)(0 - delta)) > packet->head_pos)))
    {
           NETIO_LOG_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR,delta)
           return RTI_FALSE;
    }

    /* bounds checking above, safe cast */
    if (delta >= 0)
    {
        packet->head_pos += (RTI_UINT32)delta;
    }
    else
    {
        packet->head_pos -= (RTI_UINT32)(0-delta);
    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_Packet_set_tail(NETIO_Packet_T *const packet,RTI_INT32 delta)
{
    OSAPI_PRECONDITION(packet == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    /* verify delta does not go beyond, or wrap around, head and tail */
    if (((delta > 0) &&
         ((RTI_SIZE_T)delta > (packet->max_length - packet->tail_pos))) ||
        ((delta < 0) &&
         (((RTI_SIZE_T)(0 - delta)) > (packet->tail_pos - packet->head_pos))))
    {
        NETIO_LOG_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* bounds checking above, safe cast */
    if (delta >= 0)
    {
        packet->tail_pos += (RTI_UINT32)delta;
    }
    else
    {
        packet->tail_pos -= (RTI_UINT32)(0-delta);
    }

    return RTI_TRUE;
}

void*
NETIO_Packet_get_head(const NETIO_Packet_T *const packet)
{
    OSAPI_PRECONDITION_ALWAYS(packet == NULL,return NULL,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return (void*)&packet->buffer[packet->head_pos];
}

void*
NETIO_Packet_get_tail(const NETIO_Packet_T *const packet)
{
    OSAPI_PRECONDITION(packet == NULL,return NULL,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return (void*)&packet->buffer[packet->tail_pos];
}

RTI_SIZE_T
NETIO_Packet_get_payload_length(const NETIO_Packet_T *const packet)
{
    struct NETIO_PacketBuffer *pbuf;
    RTI_SIZE_T length = 0;
    RTI_SIZE_T pbuf_length;

    OSAPI_PRECONDITION_ALWAYS(packet == NULL,return RTI_SIZE_INVALID,
            OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    if (packet->head_pbuf != NULL)
    {
        pbuf = packet->head_pbuf;
        while (pbuf)
        {
            pbuf_length = pbuf->tail_pos - pbuf->head_pos;

            if (((RTI_SIZE_MAX - 1) - length) < pbuf_length)
            {
                NETIO_LOG_PACKET_OVERFLOW(OSAPI_LOGKIND_ERROR)
                return RTI_SIZE_INVALID;
            }

            length += pbuf_length;
            pbuf = pbuf->_next;
        }
    }
    else
    {
        length = packet->tail_pos - packet->head_pos;
    }

    return length;
}

RTI_BOOL
NETIO_Packet_initialize(NETIO_Packet_T *const packet,
                       void *init_buffer,
                       RTI_SIZE_T init_length,
                       RTI_SIZE_T trailer_length,
                       struct NETIO_AddressSeq *dest_seq)
{
    NETIO_Packet_T packet_init = NETIO_Packet_INITIALIZER;

    OSAPI_PRECONDITION(packet == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    *packet = packet_init;

    if (trailer_length > init_length)
    {
        NETIO_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (init_buffer)
    {
        packet->buffer = init_buffer;
        packet->max_length = init_length;
    }

    /* 0 indexed */
    packet->head_pos =  init_length - trailer_length;
    packet->tail_pos =  packet->head_pos;

    packet->dests = dest_seq;

    return RTI_TRUE;
}

NETIODllExport RTI_BOOL
NETIO_Packet_set_buffer(NETIO_Packet_T *packet,
                        void *const buffer,
                        RTI_SIZE_T max_buffer_length,
                        RTI_SIZE_T head_pos,
                        RTI_SIZE_T tail_pos)
{
    OSAPI_PRECONDITION((packet == NULL || buffer == NULL),return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    if (head_pos > tail_pos)
    {
        NETIO_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
    packet->buffer = buffer;
    packet->max_length = max_buffer_length;

    /* 0 indexed */
    packet->head_pos =  head_pos;
    packet->tail_pos =  tail_pos;
    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_Packet_initialize_from(NETIO_Packet_T *const out_packet,
                             const NETIO_Packet_T *const in_packet,
                             void *const buffer,
                             RTI_SIZE_T buffer_size,
                             RTI_SIZE_T head_pos,
                             RTI_SIZE_T tail_pos)
{
    OSAPI_PRECONDITION((out_packet == NULL) || (in_packet == NULL)
                       || (buffer == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("out_packet",out_packet,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("in_packet",in_packet,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    if (head_pos > tail_pos)
    {
        NETIO_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    *out_packet = *in_packet;

    out_packet->buffer = buffer;
    out_packet->max_length = buffer_size;

    /* 0 indexed */
    out_packet->head_pos =  head_pos;
    out_packet->tail_pos =  tail_pos;

    out_packet->head_pbuf = NULL;
    out_packet->tail_pbuf = NULL;

    return RTI_TRUE;
}
#endif

#ifndef RTI_CERT
RTI_BOOL
NETIO_Packet_finalize(NETIO_Packet_T *const packet)
{
    PRECOND_ARG(packet)
    OSAPI_PRECONDITION(packet == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

struct NETIO_PacketInfo*
NETIO_Packet_get_info(NETIO_Packet_T *const packet)
{
    OSAPI_PRECONDITION(packet == NULL,return NULL,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return &packet->info;
}

#ifndef RTI_CERT
void
NETIO_Packet_set_source(NETIO_Packet_T *const packet,
                        const struct NETIO_Address *const src)
{
    OSAPI_PRECONDITION(packet == NULL,return,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    packet->source = *src;
}
#endif

void
NETIO_Packet_save_positions_to(const NETIO_Packet_T *const packet,
                               RTI_SIZE_T *const head,
                               RTI_SIZE_T *const tail)
{
    OSAPI_PRECONDITION((packet == NULL) || (head == NULL)
                           || (tail == NULL),return,
                       OSAPI_Log_entry_add_pointer("packet",packet,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("head",head,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("tail",tail,RTI_TRUE);)

    *head = packet->head_pos;
    *tail = packet->tail_pos;
}

void
NETIO_Packet_restore_positions_from(NETIO_Packet_T *const packet,
                                    RTI_SIZE_T head,
                                    RTI_SIZE_T tail)
{
    OSAPI_PRECONDITION(packet == NULL,return,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    packet->head_pos = head;
    packet->tail_pos = tail;
}

RTI_BOOL
NETIO_Packet_set_payload(NETIO_Packet_T *const packet,
                         void *buffer,
                         RTI_INT32 buffer_length)
{
    OSAPI_PRECONDITION_ALWAYS((packet == NULL) ||
                              (buffer == NULL) ||
                              (buffer_length <= 0),
                              return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("packet",packet,RTI_FALSE);
        OSAPI_Log_entry_add_int("buffer_length",buffer_length,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    if (!NETIO_Packet_initialize(packet,buffer,
                                 (RTI_SIZE_T)buffer_length, 0, NULL))
    {
        return RTI_FALSE;
    }

    if (!NETIO_Packet_set_head(packet, 0 - buffer_length))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_set_pbuf(NETIO_Packet_T *packet,
                      struct NETIO_PacketBuffer *pbuf_head,
                      struct NETIO_PacketBuffer *pbuf_tail)
{
    packet->head_pbuf = pbuf_head;
    packet->tail_pbuf = pbuf_tail;

    return RTI_TRUE;
}

NETIODllExport void
NETIO_Packet_save_state(const NETIO_Packet_T *const packet,
                        NETIO_PacketState_T *state)
{
    state->shallow_copy = *packet;
    if (packet->head_pbuf != NULL)
    {
        state->pbuf_head = *packet->head_pbuf;
    }
    if (packet->tail_pbuf != NULL)
    {
        state->pbuf_tail = *packet->tail_pbuf;
    }
}

NETIODllExport void
NETIO_Packet_restore_state(NETIO_Packet_T *packet,
                           const NETIO_PacketState_T *const state)
{
    *packet = state->shallow_copy;
    if (packet->head_pbuf != NULL)
    {
        *packet->head_pbuf = state->pbuf_head;
    }
    if (packet->tail_pbuf != NULL)
    {
        *packet->tail_pbuf = state->pbuf_tail;
    }
}

NETIODllExport void*
NETIO_Packet_get_buffer(NETIO_Packet_T *packet)
{
    OSAPI_PRECONDITION(packet == NULL,return NULL,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return packet->buffer;
}

/******************************************************************************/

RTI_BOOL
NETIO_PacketBuffer_set(struct NETIO_PacketBuffer *pbuf,
                       char *buffer,RTI_SIZE_T max_length,
                       RTI_UINT32 head_pos,RTI_UINT32 tail_pos)
{
    pbuf->_next = NULL;
    pbuf->buffer = buffer;

    if (max_length > 0xfffffffe)
    {
        return RTI_FALSE;
    }

    pbuf->head_pos =  head_pos;
    pbuf->tail_pos =  tail_pos;
    pbuf->max_length = max_length;

    return RTI_TRUE;
}

void
NETIO_PacketBuffer_reset(struct NETIO_PacketBuffer *pbuf)
{
    pbuf->_next = NULL;
    pbuf->head_pos =  0;
    pbuf->tail_pos =  0;
}

RTI_BOOL
NETIO_PacketBuffer_link(struct NETIO_PacketBuffer *pbuf_before,
                        struct NETIO_PacketBuffer *pbuf_after)
{
    pbuf_before->_next = pbuf_after;

    return RTI_TRUE;
}

RTI_BOOL
NETIO_PacketBuffer_unlink(struct NETIO_PacketBuffer *pbuf_before,
                          struct NETIO_PacketBuffer *pbuf_after)
{

    pbuf_before->_next = pbuf_after->_next;
    pbuf_after->_next = NULL;

    return RTI_TRUE;
}

struct NETIO_PacketBuffer*
NETIO_PacketBuffer_get_next(struct NETIO_PacketBuffer *pbuf)
{
    return pbuf->_next;
}

struct NETIO_PacketBuffer*
NETIO_PacketBuffer_get_next_non_empty(struct NETIO_PacketBuffer *pbuf)
{
    pbuf = pbuf->_next;

    while ((pbuf != NULL) && (pbuf->head_pos == pbuf->tail_pos))
    {
        pbuf = pbuf->_next;
    }

    return pbuf;
}

void*
NETIO_PacketBuffer_get_head(struct NETIO_PacketBuffer *pbuf)
{
    return &pbuf->buffer[pbuf->head_pos];
}

void*
NETIO_PacketBuffer_get_tail(struct NETIO_PacketBuffer *pbuf)
{
    if (pbuf->tail_pos > (pbuf->max_length - 1))
    {
        return NULL;
    }

    return &pbuf->buffer[pbuf->tail_pos];
}

RTI_SIZE_T
NETIO_PacketBuffer_get_length(struct NETIO_PacketBuffer *pbuf)
{
    return (pbuf->tail_pos - pbuf->head_pos);
}

RTI_BOOL
NETIO_PacketBuffer_adjust_head(struct NETIO_PacketBuffer *pbuf,RTI_INT32 delta)
{
    /* verify delta does not go beyond, or wrap around, head and tail */
    if (((delta > 0) &&
         ((RTI_SIZE_T)delta > (pbuf->tail_pos - pbuf->head_pos))) ||
        ((delta < 0) &&
         (((RTI_SIZE_T)(0 - delta)) > pbuf->head_pos)))
    {
           NETIO_LOG_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR,delta)
           return RTI_FALSE;
    }

    if (delta >= 0)
    {
        pbuf->head_pos += (RTI_UINT32)delta;
    }
    else
    {
        pbuf->head_pos -= (RTI_UINT32)(0-delta);
    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_PacketBuffer_adjust_tail(struct NETIO_PacketBuffer *pbuf,RTI_INT32 delta)
{
    /* verify delta does not go beyond, or wrap around, head and tail */
    if (((delta > 0) &&
         ((RTI_SIZE_T)delta > (pbuf->max_length - pbuf->tail_pos))) ||
        ((delta < 0) &&
         (((RTI_SIZE_T)(0 - delta)) > (pbuf->tail_pos - pbuf->head_pos))))
    {
        NETIO_LOG_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (delta >= 0)
    {
        pbuf->tail_pos += (RTI_UINT32)delta;
    }
    else
    {
        pbuf->tail_pos -= (RTI_UINT32)(0-delta);

    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_Packet_is_ndds_ping(const NETIO_Packet_T *packet)
{
    const char *packet_buffer = NETIO_Packet_get_head(packet);
    RTI_INT32 rx_len = (RTI_INT32)NETIO_Packet_get_payload_length(packet);

    if (rx_len == NETIO_CONNEXT_PING_MSG_SIZE)
    {
        /* The message format is "RTPSaabbNDDSPING", where aa is a 2 digit
         * version number and bb is a 2 digit vendor ID. Ignore the version
         * and vendor ID. There are no constants for these numbers as they
         * are only used here.
         */
        if ((OSAPI_Memory_compare("RTPS",&packet_buffer[0],4) == 0)
             && (OSAPI_Memory_compare("NDDSPING",&packet_buffer[8],8) == 0))
        {
            OSAPI_TRACE_NET("received NDDS ping",RTI_TRUE)
            return RTI_TRUE;
        }
    }
    else if (rx_len == NETIO_INFO_TS_PING_MSG_SIZE)
    {
        if (OSAPI_Memory_compare(NETIO_INFO_TS_PING_MSG,
                                 NETIO_Packet_get_head(packet),
                                 NETIO_INFO_TS_PING_MSG_SIZE) == 0)
        {
            OSAPI_TRACE_NET("received INFO_TS ping",RTI_TRUE)
            return RTI_TRUE;
        }
    }

    return RTI_FALSE;
}

/*ci @} */

