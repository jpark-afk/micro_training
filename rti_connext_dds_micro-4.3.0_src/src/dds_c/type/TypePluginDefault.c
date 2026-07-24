/*
 * FILE: TypePluginDefault.c - Default type-plugin
 *
 * Copyright (c) 2017-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "DataWriterImpl.h"
#include "DataReaderImpl.h"
#include "DomainParticipant.h"

struct DDS_TypeMemoryPlugin*
DDS_TypePluginDefaultHeap_create(struct DDS_TypePlugin *tp,
                                DDS_DomainParticipant *participant,
                                struct DDS_DomainParticipantQos *dp_qos,
                                DDS_TypePluginMode_T endpoint_mode,
                                DDS_TypePluginEndpoint *endpoint,
                                DDS_TypePluginEndpointQos *qos_deprecated,
                                DDS_TypePlugin_initialize_pool_sample_T init_sample,
                                DDS_TypePlugin_initialize_pool_sample_T finalize_sample)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault *)tp;
    struct REDA_BufferPoolProperty bufp = REDA_BufferPoolProperty_INITIALIZER;
    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);
    UNUSED_ARG(qos_deprecated);

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_READER)
    {
        bufp.buffer_size = sizeof(struct DDS_TypePluginSampleHolder);
        if (tp->property.max_buffers == 0)
        {
            /* max_samples cannot be negative */
            bufp.max_buffers = (RTI_SIZE_T)DDS_DataReader_get_max_samples(
                                                    (DDS_DataReader*)endpoint);
        }
        else
        {
            bufp.max_buffers = (RTI_SIZE_T)tp->property.max_buffers;
        }
        plugin->pool = REDA_BufferPool_new("cdr_samples",
                                      &bufp,
                                      init_sample,tp,
                                      finalize_sample,tp);
    }

    return &plugin->heap_plugin._parent;
}

void*
DDS_TypePluginDefaultCdr_get_buffer(struct DDS_TypePlugin *tp)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)tp;

    return DDS_TypePlugin_init_inline_buffer(&plugin->cdr_plugin._parent,
                                             plugin->pool,
                                             plugin->cdr_plugin.size);
}

void
DDS_TypePluginDefaultCdr_return_buffer(struct DDS_TypePlugin *tp,void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)tp;

    REDA_BufferPool_return_buffer(plugin->pool,buffer);
}

struct DDS_TypePluginSampleHolder*
DDS_TypePluginDefaultCdr_get_sample(struct DDS_TypePlugin *tp,
                                    struct CDR_Stream_t *stream)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)tp;
    struct DDS_TypePluginSampleHolder *sh;
    UNUSED_ARG(stream);

    sh = REDA_BufferPool_get_buffer(plugin->pool);
    if (sh == NULL)
    {
        return NULL;
    }

    sh->owner = &plugin->cdr_plugin._parent;

    return sh;
}

void
DDS_TypePluginDefaultCdr_return_sample(struct DDS_TypePlugin *tp,
                                      struct DDS_TypePluginSampleHolder *sample)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)tp;

    REDA_BufferPool_return_buffer(plugin->pool,sample);
}

struct DDS_TypeEncapsulationPlugin*
DDS_TypePluginDefaultCdr_create(struct DDS_TypePlugin *tp,
                               DDS_DomainParticipant *participant,
                               struct DDS_DomainParticipantQos *dp_qos,
                               DDS_TypePluginMode_T endpoint_mode,
                               DDS_TypePluginEndpoint *endpoint,
                               DDS_TypePluginEndpointQos *qos,
                               struct DDS_TypeMemoryPlugin *mp,
                               RTI_UINT32 buf_size)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault *)tp;
    struct REDA_BufferPoolProperty bufp = REDA_BufferPoolProperty_INITIALIZER;
    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(qos);
    UNUSED_ARG(endpoint);

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        /* Round up the buffer size to be aligned to 4 bytes */
        RTI_UINT32 aligned_buf_size = OSAPI_Heap_align_size_up(buf_size,
                                        RTI_ENCAPSULATION_MAX_PADDING_BYTES);

        /* The buf_size does not include the encapsulation header */
        bufp.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_TypePluginBuffer) +
                            RTI_CDR_ENCAPSULATION_HEADER_SIZE +
                            aligned_buf_size +
                            (RTI_SIZE_T)tp->property.head_padding +
                            (RTI_SIZE_T)tp->property.tail_padding;

        if (tp->property.max_buffers == 0)
        {
            bufp.max_buffers = (RTI_SIZE_T)DDS_DataWriter_get_max_samples(
                                                (DDS_DataWriter*)endpoint);
        }
         else
        {
            bufp.max_buffers = (RTI_SIZE_T)tp->property.max_buffers;
        }

        plugin->pool = REDA_BufferPool_new("cdr_buffers",
                                          &bufp,NULL,NULL,NULL,NULL);

        /* The padding does not exceed 2GB */
        plugin->cdr_plugin.size = bufp.buffer_size;
    }

    plugin->cdr_plugin._parent.memory_plugin = mp;

    return &plugin->cdr_plugin._parent;
}

struct DDS_TypePlugin*
DDS_TypePluginDefault_create(struct DDS_TypePluginI *intf,
                            DDS_DomainParticipant *participant,
                            struct DDS_DomainParticipantQos *dp_qos,
                            DDS_TypePluginMode_T endpoint_mode,
                            DDS_TypePluginEndpoint *endpoint,
                            DDS_TypePluginEndpointQos *qos,
                            struct DDS_TypePluginProperty *const property)
{
    struct DDS_TypePluginDefault *plugin = NULL;
    struct DDS_TypePlugin *retval = NULL;
    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);
    UNUSED_ARG(endpoint_mode);
    UNUSED_ARG(qos);

    OSAPI_Heap_allocate_struct(&plugin,struct DDS_TypePluginDefault);
    if (plugin == NULL)
    {
        goto done;
    }

    plugin->pool = NULL;
    retval = &plugin->_parent;
    REDA_CircularListNode_init(&plugin->heap_plugin._parent._node);
    REDA_CircularListNode_init(&plugin->cdr_plugin._parent._node);
    REDA_CircularList_init(&retval->wire_plugins);
    REDA_CircularList_init(&retval->memory_plugins);
    retval->property = *property;
    plugin->_parent.dp = participant;

    retval->_intf = intf;
    plugin->cdr_plugin._parent._intf = intf->wire_intf[0];
    plugin->heap_plugin._parent._intf = intf->memory_intf[0];

done:
    return retval;
}

DDS_Boolean
DDS_TypePluginDefault_delete(struct DDS_TypePlugin *plugin)
{
#ifndef RTI_CERT
    struct DDS_TypePluginDefault *self = (struct DDS_TypePluginDefault*)plugin;

    if (self->pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->pool))
        {
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free_struct(plugin);
#else
    UNUSED_ARG(plugin);
#endif

    return RTI_TRUE;
}
