/*
 * FILE: RTPSTrust.c - RTPS Transform functions
 *
 * Copyright (c) 2018-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"
#ifndef rtps_config_h
#include "rtps/rtps_config.h"
#endif



#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef db_log_h
#include "db/db_log.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
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
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef rtps_log_h
#include "rtps/rtps_log.h"
#endif

#ifndef RTPSInterface_pkg_h
#include "RTPSInterface.h"
#endif

#include "RTPSTrust.h"

#define RTPS_InterfaceTrust_get_service_plugin(rtps_intf_)\
    RTPS_InterfaceTransform_service_plugin(rtps_intf)

RTI_BOOL
RTPS_Interface_trust_initialize_intf(struct RTPS_Interface *rtps_intf,
                                     struct RTPS_Interface *ext_intf)
{

    if (!RTPS_Interface_is_trust_enabled(ext_intf))
    {
        return RTI_TRUE;
    }

    /* The external interface contains the shared state and is allocated first.
     * Creation of the external interface should already have failed.
     */
    rtps_intf->transform_state = ext_intf->transform_state;
    RTPS_Interface_set_trust_enabled(rtps_intf);

    if (RTPS_Interface_is_aad_enabled(ext_intf))
    {
        RTPS_Interface_set_aad_enabled(rtps_intf);
    }

    return RTI_TRUE;
}

 /*ci @brief Finalize the trust transform state for an RTPS interface.
  *
  * This function cleans up the trust transform state associated with an RTPS
  * interface. If trust is not enabled for the interface, the function returns
  * without performing any operations.
  *
  * @param[in,out] rtps_intf The RTPS interface whose trust state should be finalized.
  *                          The transform_state member will be freed and set to NULL.
  *
  * @note This function only frees the transform_state structure itself, not the
  *       shared resources like the buffer pool or service plugin, which are
  *       managed by the external interface.
  */
void
RTPS_Interface_trust_finalize_intf(struct RTPS_Interface *rtps_intf)
{
    rtps_intf->transform_state = NULL;
}

RTI_BOOL
RTPS_Interface_trust_initialize_ext_intf(
                struct RTPS_Interface *rtps_intf,
                                        const struct RTPS_InterfaceProperty *const property)
{
    RTI_BOOL retval = RTI_FALSE;
    struct REDA_BufferPoolProperty p_prop = REDA_BufferPoolProperty_INITIALIZER;

    if (property->trust_property.enabled == RTI_FALSE)
    {
        /* no need to initialize the trust */
        return RTI_TRUE;
    }

    OSAPI_Heap_allocate_struct(&rtps_intf->transform_state,
                               struct RTPS_TransformState);
    if (rtps_intf->transform_state == NULL)
    {
        RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    RTPS_Interface_set_trust_enabled(rtps_intf);
    if (property->trust_property.aad_enabled)
    {
        RTPS_Interface_set_aad_enabled(rtps_intf);
    }

    RTPS_InterfaceTransform_transform_buffer_size(rtps_intf) = property->trust_property.transform_buffer_size;
    RTPS_InterfaceTransform_service_plugin(rtps_intf) = property->trust_property.service_plugin;

    p_prop.buffer_size = property->trust_property.transform_buffer_size;
    /* currently only 2 buffers are allocated They will be used both in the send and receive path */
    p_prop.max_buffers = property->trust_property.transform_buffer_count;
    RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) = REDA_BufferPool_new("transform_buf",
                                            &p_prop,NULL,NULL,NULL,NULL);

    if (RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) == NULL)
    {
        RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    retval = RTI_TRUE;

cleanup:
    if (retval == RTI_FALSE)
    {
        if (RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) != NULL)
        {
#ifndef RTI_CERT
            retval &= REDA_BufferPool_delete(RTPS_InterfaceTransform_transform_buf_pool(rtps_intf));
#endif
            RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) = NULL;
        }

        OSAPI_Heap_free_struct(rtps_intf->transform_state);
    }
    return retval;
}


#ifndef RTI_CERT
void
RTPS_Interface_trust_finalize_ext_intf(struct RTPS_Interface *rtps_intf)
{
    RTI_BOOL retval = RTI_FALSE;

    if (!RTPS_Interface_is_trust_enabled(rtps_intf))
    {
        return;
    }

    if (RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) != NULL)
    {
        retval = REDA_BufferPool_delete(RTPS_InterfaceTransform_transform_buf_pool(rtps_intf));
        IGNORE_RETVAL(retval);
        RTPS_InterfaceTransform_transform_buf_pool(rtps_intf) = NULL;
    }

    OSAPI_Heap_free_struct(rtps_intf->transform_state);
}
#endif



RTI_BOOL
RTPS_Interface_trust_transform_outgoing_buffer(
        struct RTPS_Interface *rtps_intf,
        NETIO_Packet_T *packet,
        struct RTPS_HEADER_EXT **hdrext)
{
    RTI_BOOL retval = RTI_TRUE;
    RTI_BOOL bretval = RTI_FALSE;
    void* transformed_buffer = NULL;
    RTI_UINT32 transformed_buf_len = 0;
    struct REDA_Buffer transformed_reda_buffer;

    if (!RTPS_Interface_is_trust_enabled(rtps_intf))
    {
        goto done;
    }

    /* RTPS owns the buffers for transformation */
    transformed_buffer = REDA_BufferPool_get_buffer(
                         RTPS_InterfaceTransform_transform_buf_pool(rtps_intf));
    if (transformed_buffer == NULL)
    {
        retval = RTI_FALSE;
        goto done;
    }

    /* set the transformation buffer. The max size of the buffer
     *  included to allow the service plugin to not overflow the buffer
     */
    REDA_Buffer_set(&transformed_reda_buffer,
                    transformed_buffer,
                    RTPS_InterfaceTransform_transform_buffer_size(rtps_intf));

    if (!RTPS_TrustPlugin_outgoing_transform(
            RTPS_InterfaceTrust_get_service_plugin(rtps_intf),
            packet,
            &transformed_reda_buffer))
    {
        RTPS_LOG_TRANSFORM_RTPS(OSAPI_LOGKIND_ERROR)
        retval = RTI_FALSE;
        goto done;
    }

    /* The trust plugin will set the transformed buffer length */
    transformed_buf_len = transformed_reda_buffer.length;

    /*
     * Modify the packet such that it uses the tranformed buffer and does not use pbuf
     */
    bretval = NETIO_Packet_set_buffer(packet,
                    transformed_reda_buffer.pointer,
                    RTPS_InterfaceTransform_transform_buffer_size(rtps_intf),
                    0, /* head pos */
                    transformed_buf_len);

    bretval &= NETIO_Packet_set_pbuf(packet,
                                     NULL,
                                     NULL);
    packet->info.rtps_flags |= NETIO_RTPS_FLAGS_TRUST_LOAN;
    IGNORE_RETVAL(bretval);

    /* If the value at Header Extension was not NULL, it was previously pointing to the header
     * extension in the non-transformed packet.Since the packet has changed, let's modify the
     * header extension to point to the new location. It is assumed that the transformation
     * has kept the header extension unmodified in the transformed buffer and is located right
     * after the RTPS header.
     */
    if (hdrext != NULL && *hdrext != NULL)
    {
        /* set the header extension to point to the new location */
        *hdrext = OSAPI_Compiler_reinterpret_cast(struct RTPS_HEADER_EXT *, ((char *)transformed_buffer +
                sizeof(struct RTPS_Header)));
    }

    retval = RTI_TRUE;

done:
        if (!retval)
        {
            if(transformed_buffer != NULL)
            {
                /* return the buffer to the pool */
                REDA_BufferPool_return_buffer(
                        RTPS_InterfaceTransform_transform_buf_pool(rtps_intf),
                        transformed_buffer);
            }
            packet->info.rtps_flags &= ~NETIO_RTPS_FLAGS_TRUST_LOAN;
        }

        return retval;
}


RTI_BOOL
RTPS_Interface_trust_transform_incoming_buffer(
    struct RTPS_Interface *rtps_intf,
    NETIO_Packet_T *packet,
    NETIO_PacketState_T *saved_packet_state)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_BOOL bretval = RTI_FALSE;
    union RTPS_MESSAGES *rtps_msg = NULL;
    union RTPS_MESSAGES tmp_msg;
    char *msg_ptr = NULL;
    RTI_BOOL byte_swap;
    void* transformed_buffer = NULL;
    struct REDA_Buffer transformed_reda_buffer;
    RTI_UINT32 submessage_length = 0;

    /* RTPS owns the buffers for transformation */
    transformed_buffer = REDA_BufferPool_get_buffer(
                         RTPS_InterfaceTransform_transform_buf_pool(rtps_intf));
    if (transformed_buffer == NULL)
    {
        goto done;
    }
    /* set the transformation buffer. The max size of the buffer
     * included to allow the service plugin to not overflow the buffer
     */
    REDA_Buffer_set(&transformed_reda_buffer,
        transformed_buffer,
        RTPS_InterfaceTransform_transform_buffer_size(rtps_intf));

    if (!RTPS_TrustPlugin_incoming_transform(
            RTPS_InterfaceTrust_get_service_plugin(rtps_intf),
            packet,
            &transformed_reda_buffer))
    {

        RTPS_LOG_TRANSFORM_RTPS(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* save the pcket state*/
    NETIO_Packet_save_state(packet, saved_packet_state);

    /* set the packet such that the packet now points to the transformed buffer */
    /* the head pos is  set such that its right after the rtps header and rtps header extension if present */
    bretval = NETIO_Packet_set_buffer(packet,
                      transformed_reda_buffer.pointer,
                      RTPS_InterfaceTransform_transform_buffer_size(rtps_intf),
                      0, /* head pos */
                      transformed_reda_buffer.length);
    IGNORE_RETVAL(bretval);
    packet->info.rtps_flags |= NETIO_RTPS_FLAGS_TRUST_LOAN;

    rtps_msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    /* confirm this is the RTPS header */
    if (rtps_msg->header.rtps != VALID_RTPS_HEADER)
    {
        goto done;
    }

    /* RTPS header is fixed size, it is safe to move past it*/
    if (!NETIO_Packet_set_head(packet, sizeof(struct RTPS_Header)))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
        goto done;
    }

    rtps_msg = NETIO_Packet_get_head(packet);

    tmp_msg.submsg.kind = rtps_msg->submsg.kind;
    /* if we have a header extension get the length and set the head position of the packet */
    if (tmp_msg.submsg.kind == RTPS_HEADER_EXTN_KIND)
    {
        tmp_msg.submsg.flags = rtps_msg->submsg.flags;
        byte_swap = RTPSInterface_byte_swap(tmp_msg.submsg.flags);
        msg_ptr = (char *)&rtps_msg->submsg.length;

        CDR_deserialize_unsigned_short(&msg_ptr, &tmp_msg.submsg.length,
                                       byte_swap);
        /* set the head position to the start of the next submessage */
        submessage_length = tmp_msg.submsg.length + RTPS_SUBMESSAGE_HEADER_LENGTH;
        if (submessage_length >= INT_MAX)
        {
            goto done;
        }
        if (!NETIO_Packet_set_head(packet, (RTI_INT32)submessage_length))
        {
            /* this should not happen */
            RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
            goto done;
        }
    }
    retval = RTI_TRUE;
done:
    if (!retval)
    {
        if (packet->info.rtps_flags & NETIO_RTPS_FLAGS_TRUST_LOAN)
        {
            /* return the loan for transformation */
            packet->info.rtps_flags &= ~NETIO_RTPS_FLAGS_TRUST_LOAN;
            NETIO_Packet_restore_state(packet, saved_packet_state);
        }
        if (transformed_buffer != NULL)
        {
            /* return the buffer to the pool */
            REDA_BufferPool_return_buffer(
                    RTPS_InterfaceTransform_transform_buf_pool(rtps_intf),
                    transformed_buffer);
        }
    }

    return retval;
}
