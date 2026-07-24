/*
 * FILE: XTypesV2TypePlugin.c
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef cdr_encapsulation_h
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "cdr/cdr_md5.h"
#include "Interpreter.h"
#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"
#include "xcdr/xcdr_dds_interpreter.h"

/*****************************************************************************
 *                            XCDR v1 specific functions
 *****************************************************************************/

struct DDS_TypeEncapsulationPlugin*
XCDRv1_StreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct XCDR_StreamPlugin *stream_plugin = NULL;
    struct REDA_BufferPoolProperty bp = REDA_BufferPoolProperty_INITIALIZER;
    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);
    UNUSED_ARG(qos);

    OSAPI_Heap_allocate_struct(&stream_plugin, struct XCDR_StreamPlugin);
    if (stream_plugin == NULL)
    {
        return NULL;
    }

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        RTI_UINT32 max_serialized_size;
        DDS_Long max_samples;

        max_samples = DDS_DataWriter_get_max_samples((DDS_DataWriter*)endpoint);
        max_serialized_size = XCDRv1_StreamPlugin_get_serialized_sample_size(tp, NULL, 0);
        max_serialized_size = OSAPI_Heap_align_size_up(max_serialized_size,
                                        RTI_ENCAPSULATION_MAX_PADDING_BYTES);

        stream_plugin->size =
                (RTI_UINT32)sizeof(struct DDS_TypePluginBuffer) +
                RTI_CDR_ENCAPSULATION_HEADER_SIZE +
                max_serialized_size +
                (RTI_UINT32)tp->property.head_padding +
                (RTI_UINT32)tp->property.tail_padding;

        bp.buffer_size = stream_plugin->size;
        bp.max_buffers = (RTI_SIZE_T)max_samples;
        stream_plugin->serialization_buffers_buf_pool = REDA_BufferPool_new(
                "xcdr",
                &bp,
                NULL,
                NULL,
                NULL,
                NULL);

        if (stream_plugin->serialization_buffers_buf_pool == NULL)
        {
#ifndef RTI_CERT
            OSAPI_Heap_free_struct(stream_plugin);
#endif
            return NULL;
        }
    }

    stream_plugin->_parent.memory_plugin = mp;

    return &stream_plugin->_parent;
}

void
XCDRv1_StreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp)
{
    struct XCDR_StreamPlugin *plugin = (struct XCDR_StreamPlugin*) mp;
    UNUSED_ARG(p);

    if (plugin->serialization_buffers_buf_pool != NULL)
    {
#ifndef RTI_CERT
        if (!REDA_BufferPool_delete(plugin->serialization_buffers_buf_pool))
        {
            /* Exception */
        }
#endif

    }
#ifndef RTI_CERT
    OSAPI_Heap_free_struct(mp);
#endif
}

RTI_UINT32
XCDRv1_StreamPlugin_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment)
{
    UNUSED_ARG(ep);
    return XCDR_Interpreter_get_serialized_sample_size(
            plugin,
            RTI_FALSE, /* Include encapsulation */
            RTI_FALSE, /* Is CDRv2 (false) */
            alignment);
}

/*****************************************************************************
 *                            XCDR v2 specific functions
 *****************************************************************************/

struct DDS_TypeEncapsulationPlugin*
XCDRv2_StreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct XCDR_StreamPlugin *stream_plugin = NULL;
    struct REDA_BufferPoolProperty bp = REDA_BufferPoolProperty_INITIALIZER;
    UNUSED_ARG(qos);

    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);

    OSAPI_Heap_allocate_struct(&stream_plugin, struct XCDR_StreamPlugin);
    if (stream_plugin == NULL)
    {
        return NULL;
    }

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        RTI_UINT32 max_serialized_size;
        DDS_Long max_samples;

        max_samples = DDS_DataWriter_get_max_samples((DDS_DataWriter*)endpoint);

        max_serialized_size = XCDRv2_StreamPlugin_get_serialized_sample_size(tp, NULL, 0);
        max_serialized_size = OSAPI_Heap_align_size_up(max_serialized_size,
                                        RTI_ENCAPSULATION_MAX_PADDING_BYTES);

        stream_plugin->size =
                (RTI_UINT32)sizeof(struct DDS_TypePluginBuffer) +
                RTI_CDR_ENCAPSULATION_HEADER_SIZE +
                max_serialized_size +
                (RTI_UINT32)tp->property.head_padding +
                (RTI_UINT32)tp->property.tail_padding;

        bp.buffer_size = stream_plugin->size;
        bp.max_buffers = (RTI_SIZE_T)max_samples;
        stream_plugin->serialization_buffers_buf_pool = REDA_BufferPool_new(
                "xcdr",
                &bp,
                NULL,
                NULL,
                NULL,
                NULL);

        if (stream_plugin->serialization_buffers_buf_pool == NULL)
        {
#ifndef RTI_CERT
            OSAPI_Heap_free_struct(stream_plugin);
#endif
            return NULL;
        }
    }

    stream_plugin->_parent.memory_plugin = mp;

    return &stream_plugin->_parent;
}

void
XCDRv2_StreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp)
{
#ifndef RTI_CERT
    struct XCDR_StreamPlugin *plugin = (struct XCDR_StreamPlugin*) mp;
    UNUSED_ARG(p);

    if (plugin->serialization_buffers_buf_pool != NULL)
    {
        if (!REDA_BufferPool_delete(plugin->serialization_buffers_buf_pool))
        {
            /* Exception */
        }
    }

    OSAPI_Heap_free_struct(mp);
#else
    UNUSED_ARG(mp);
    UNUSED_ARG(p);
#endif
}

RTI_UINT32
XCDRv2_StreamPlugin_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment)
{
    UNUSED_ARG(ep);

    return XCDR_Interpreter_get_serialized_sample_size(
            plugin,
            RTI_FALSE, /* include encapsulation */
            RTI_TRUE,  /* is CDRv2 (true) */
            alignment);
}

/*
 * DataWriter side
 * Gets the buffer to serialize a sample into
 */
void*
XCDR_GenericStreamPlugin_get_buffer(struct DDS_TypePlugin *tp)
{
    struct XCDR_StreamPlugin *self = (struct XCDR_StreamPlugin*) tp->wire_plugin;
    struct DDS_TypePluginBuffer *tbuf;

    tbuf = DDS_TypePlugin_init_inline_buffer(
            &self->_parent,
            self->serialization_buffers_buf_pool,
            self->size);

    return tbuf;
}

/*
 * DataWriter side
 */
void
XCDR_GenericStreamPlugin_return_buffer(struct DDS_TypePlugin *tp, void *buffer)
{
    struct XCDR_StreamPlugin *self = (struct XCDR_StreamPlugin*) tp->wire_plugin;
    REDA_BufferPool_return_buffer(self->serialization_buffers_buf_pool, buffer);
}

/*
 * DataReader side
 */
struct DDS_TypePluginSampleHolder*
XCDR_GenericStreamPlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream)
{
    struct DDS_TypePluginSampleHolder *sh;
    struct DDS_XTypesHeapPlugin *heap_plugin;
    struct XCDR_StreamPlugin *current_stream_plugin;

    UNUSED_ARG(stream);

    current_stream_plugin = (struct XCDR_StreamPlugin*) tp->wire_plugin;
    heap_plugin = (struct DDS_XTypesHeapPlugin *)
            current_stream_plugin->_parent.memory_plugin;

    sh = REDA_BufferPool_get_buffer(heap_plugin->sample_holder_pool);
    if (sh == NULL)
    {
        return NULL;
    }

    sh->owner = &current_stream_plugin->_parent;

    return sh;
}

void
XCDR_GenericStreamPlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample)
{
    struct DDS_XTypesHeapPlugin *heap_plugin;
    struct XCDR_StreamPlugin *current_stream_plugin;

    UNUSED_ARG(tp);

    current_stream_plugin = (struct XCDR_StreamPlugin*) sample->owner;
    heap_plugin = (struct DDS_XTypesHeapPlugin *)current_stream_plugin->_parent.memory_plugin;
    REDA_BufferPool_return_buffer(heap_plugin->sample_holder_pool, sample);
}

MUST_CHECK_RETURN RTI_BOOL
XCDR_GenericStreamPlugin_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination)
{
    UNUSED_ARG(destination);
    return XCDR_Interpreter_serialize(
            plugin,
            stream,
            data,
            RTI_FALSE,
            RTI_TRUE,
            plugin->current_encapsulation);
}

MUST_CHECK_RETURN RTI_BOOL
XCDR_GenericStreamPlugin_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(source);
    return XCDR_Interpreter_deserialize(
            plugin,
            sample,
            stream,
            RTI_FALSE,
            RTI_TRUE,
            plugin->current_encapsulation);
}

RTI_BOOL
XCDR_GenericTypePlugin_serialize_key(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        DDS_InstanceHandle_t *destination)
{
    UNUSED_ARG(destination);
    return XCDR_Interpreter_serialize_key(
            plugin,
            stream,
            sample,
            RTI_FALSE,
            RTI_TRUE,
            plugin->current_encapsulation);
}

RTI_BOOL
XCDR_GenericTypePlugin_deserialize_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(source);
    return XCDR_Interpreter_deserialize_key(
            plugin,
            sample,
            stream,
            RTI_FALSE,
            RTI_TRUE,
            plugin->current_encapsulation);
}

RTI_UINT32
XCDR_GenericTypelugin_get_serialized_key_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{

    /* We need to take the maximum size of all 4 because it is the type plugin
     * interface that has a function that determines the size of the
     * serialized key. The max size of a serialized key for non-keyhash
     * may be different that the max size of a serialized for a keyhash;
     */

    RTI_UINT32 max_key_size;
    RTI_UINT32 current_key_size = 0;

    /**************************
     * CDRv1 non-keyhash size *
     **************************/
    max_key_size = XCDR_Interpreter_get_serialized_key_size(
            plugin,
            RTI_FALSE,
            RTI_FALSE,
            current_alignment);

    if (max_key_size == 0)
    {
        DDSC_LOG_GET_SERIALIZED_KEY_SIZE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    /**************************
     * CDRv2 non-keyhash size *
     **************************/
    current_key_size = XCDR_Interpreter_get_serialized_key_size(
            plugin,
            RTI_FALSE,
            RTI_TRUE,
            current_alignment);
    if (current_key_size == 0)
    {
        DDSC_LOG_GET_SERIALIZED_KEY_SIZE(OSAPI_LOGKIND_ERROR);
        max_key_size = 0;
        goto done;
    }

    if (current_key_size > max_key_size)
    {
        max_key_size = current_key_size;
    }


    /**********************
     * CDRv1 keyhash size *
     **********************/
    current_key_size = XCDR_Interpreter_get_serialized_key_size_for_keyhash(
            plugin,
            RTI_FALSE,
            current_alignment);
    if (current_key_size == 0)
    {
        DDSC_LOG_GET_SERIALIZED_KEY_SIZE(OSAPI_LOGKIND_ERROR);
        max_key_size = 0;
        goto done;
    }

    if (current_key_size > max_key_size)
    {
        max_key_size = current_key_size;
    }


    /**********************
     * CDRv2 keyhash size *
     **********************/
    current_key_size = XCDR_Interpreter_get_serialized_key_size_for_keyhash(
            plugin,
            RTI_TRUE,
            current_alignment);

    if (current_key_size == 0)
    {
        DDSC_LOG_GET_SERIALIZED_KEY_SIZE(OSAPI_LOGKIND_ERROR);
        max_key_size = 0;
        goto done;
    }

    if (current_key_size > max_key_size)
    {
        max_key_size = current_key_size;
    }

    /* We ensure that the max_key size is at least 16. This is needed
     * for a use case with FLAT_DATA that when a dispose is received
     * along with the the serialized_key. In the receive path,
     * we calculate the keyhash and store it in the md5_buffer
     * that would normally contain the serialized key.
     */
    if (max_key_size < 16)
    {
        max_key_size = 16;
    }

done:

    return max_key_size;
}

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
XCDR_GenericTypePlugin_instance_to_keyhash(
        struct NDDS_Type_Plugin *plugin,
        struct CDR_Stream_t *md5stream,
        DDS_KeyHash_t *key_hash,
        const void *instance,
        DDS_EncapsulationId_t effective_id)
{
    RTI_UINT32 serialized_key_max_size;
    RTIXCdrBoolean is_cdr_v2;
    RTI_BOOL retval = RTI_FALSE;
#ifdef RTI_ENDIAN_LITTLE
    RTI_BOOL saved_byte_swap;
    RTI_BOOL is_byte_swapped = RTI_FALSE;
#endif

    /* validate parameters */
    if ((plugin == NULL) || (md5stream == NULL) ||
        (key_hash == NULL) || (instance == NULL))
    {
        goto done;
    }

    /* verify key support exists for this plugin */
    if (DDS_TypePlugin_get_key_kind(plugin) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        goto done;
    }

    is_cdr_v2 = RTIXCdrEncapsulationId_isCdrV2(effective_id);
    serialized_key_max_size = XCDR_Interpreter_get_serialized_key_size_for_keyhash(
            plugin,
            is_cdr_v2,
            0);

    if (serialized_key_max_size > md5stream->length)
    {
        goto done;
    }

    /* need to zero stream buffer because any padding bytes won't be set by
       serialization, but they will be included in the hash */
    OSAPI_Memory_zero(md5stream->buffer, serialized_key_max_size);

    /* keyhash is always calculated from Big Endian serialization,
       so if we are little endian we need to set byte swap */
#ifdef RTI_ENDIAN_LITTLE
    saved_byte_swap = md5stream->need_byte_swap;
    md5stream->need_byte_swap = RTI_TRUE;
    is_byte_swapped = RTI_TRUE;
#endif
    CDR_Stream_reset(md5stream);
    if (!XCDR_Interpreter_serialize_key_for_keyhash(
            plugin,
            md5stream,
            instance,
            effective_id))
    {
        goto done;
    }

    /* if the serialized key size is greater than the hash size,
       hash the serialized key, otherwise just use the serialized key */
    if (serialized_key_max_size > (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH))
    {
        NDDSCDR_Stream_compute_MD5(md5stream, key_hash->value);
    }
    else
    {
        /* sanity check to insure serialized key hasn't exceeded max size */
        if (CDR_Stream_get_current_position_offset(md5stream) >
            (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH))
        {
            goto done;
        }
        OSAPI_Memory_zero(key_hash->value,RTPS_KEY_HASH_MAX_LENGTH);
        OSAPI_Memory_copy(key_hash->value, md5stream->buffer,
                         CDR_Stream_get_current_position_offset(md5stream));
    }
    key_hash->length = RTPS_KEY_HASH_MAX_LENGTH;

    retval = RTI_TRUE;
done:

#ifdef RTI_ENDIAN_LITTLE
    if (is_byte_swapped)
    {
        md5stream->need_byte_swap = saved_byte_swap;
    }
#endif
    return retval;
}
