/*
 * FILE: NETIOPacket.c - Packet functionality
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
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
    OSAPI_PRECONDITION(packet == NULL,return NULL,
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
    OSAPI_PRECONDITION(packet == NULL,return RTI_SIZE_INVALID,
                        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return packet->tail_pos - packet->head_pos;
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

/*ci @} */

