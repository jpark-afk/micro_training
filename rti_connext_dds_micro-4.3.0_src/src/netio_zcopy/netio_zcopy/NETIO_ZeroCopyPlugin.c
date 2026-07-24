/*
 * FILE: NETIO_ZCopy_PluginHelper.c - Zero Copy Plugin
 *
 * (c) Copyright 2024-2026 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "dds_c/dds_c_infrastructure.h"
#include "dds_c/dds_c_publication.h"
#include "dds_c/dds_c_subscription.h"
#include "dds_c/dds_c_log.h"
#include "osapi/osapi_string.h"
#include "netio_zcopy/netio_zcopy_plugin.h"
#include "netio_zcopy/netio_zcopy_sharedq.h"
#include "netio_zcopy/netio_zcopy_log.h"


/*** SOURCE_BEGIN ***/

RTI_PRIVATE RTI_BOOL
ZCOPY_ShmV2_locator_kind_exists(DDS_TypePluginEndpoint *endpoint,
                                DDS_TypePluginMode_T endpoint_mode)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_Locator *locator;
    const struct DDS_LocatorSeq *uc = NULL;
    RTI_INT32 len, i;

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER )
    {
        uc = DDS_DataWriter_get_unicast_locator_ref((DDS_DataWriter*)endpoint);
        if (DDS_LocatorSeq_get_length(uc) == 0)
        {
            uc = DDS_DataWriter_get_resolved_locator_ref((DDS_DataWriter*)endpoint);
        }

        len = DDS_LocatorSeq_get_length(uc);
        for (i = 0; i < len; i++)
        {
            locator = DDS_LocatorSeq_get_reference(uc, i);
            if (locator->kind == NETIO_ADDRESS_KIND_NOTIF)
            {
               retval = RTI_TRUE;
               goto done;
            }
        }
    }
    else if (endpoint_mode == DDS_TYPEPLUGIN_MODE_READER)
    {
        uc = DDS_DataReader_get_unicast_locator_ref((DDS_DataReader*)endpoint);
        if (DDS_LocatorSeq_get_length(uc) == 0)
        {
            uc = DDS_DataReader_get_resolved_locator_ref((DDS_DataReader*)endpoint);
        }
        len = DDS_LocatorSeq_get_length(uc);
        for (i = 0; i < len; i++)
        {
            locator = DDS_LocatorSeq_get_reference(uc, i);
            if (locator->kind == NETIO_ADDRESS_KIND_NOTIF)
            {
               retval = RTI_TRUE;
               goto done;
            }
        }
    }
done:
    return retval;

}

/* memory plugin */
struct DDS_TypeMemoryPlugin*
ZCOPY_ShmV2_memory_plugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos)
{
    NETIO_ZCOPY_SharedQWriter *sq_writer = NULL;
    DDS_DataWriter *writer = NULL;
    struct ZCOPY_ShmV2Plugin *plugin = NULL;
    struct ZCOPY_Guid owner_key;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    RTI_UINT32 sample_size = 0;
    RTI_BOOL init_sample = RTI_FALSE;
    RTI_UINT32 allocated_samples = 0;
    RTI_BOOL notif_locator_available = RTI_FALSE;

#ifndef RTI_CERT
    RTI_BOOL rtn;
#endif
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(qos);

    notif_locator_available = ZCOPY_ShmV2_locator_kind_exists(endpoint,endpoint_mode);
    if (!notif_locator_available)
    {
        return NULL;
    }
    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        DDS_Long max_remote_readers;
        DDS_Long writer_loaned_sample_allocation;
        DDS_UnsignedShort zcv2_version;
        SQ_VersionNumber sq_version = SQ_VERSION_NUMBER_NONE;

        writer = (DDS_DataWriter*)endpoint;
        /* get max sample size from the plugin */
        sample_size = DDS_TypePlugin_get_sample_size(tp);

        /*Get instance handle and create an ZCOPY_Guid*/
        instance_handle = DDS_Entity_get_instance_handle((DDS_Entity *)writer);
        OSAPI_Memory_copy(owner_key.value, instance_handle.octet, 16);

        /*Get domain ID*/
        DDS_Long domain_id = DDS_DomainParticipant_get_domain_id(participant);

        writer_loaned_sample_allocation =
                DDS_DataWriter_get_writer_loaned_sample_allocation(
                                        (DDS_DataWriter*)endpoint);

        if (writer_loaned_sample_allocation >= 1)
        {
            /* If the user specified a value, then that value is used as is.*/
            allocated_samples = (RTI_UINT32)writer_loaned_sample_allocation;
        }
        else
        {
            DDS_Long max_samples;

            max_samples = DDS_DataWriter_get_max_samples((DDS_DataWriter*)endpoint);

            allocated_samples = (RTI_UINT32)max_samples + 1U;
        }

        max_remote_readers = DDS_DataWriter_get_max_remote_readers((DDS_DataWriter*)endpoint);

        zcv2_version = DDS_DataWriter_get_zcv2_protocol_version((DDS_DataWriter*)endpoint);
        if (zcv2_version != DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_DEFAULT)
        {
            sq_version = zcv2_version;
        }
        else
        {
            sq_version = SQ_VERSION_NUMBER_NONE;
        }
        if (!NETIO_ZCOPY_SharedQWriter_create(
                    &owner_key,
                    (RTI_UINT32)domain_id,
                    allocated_samples,
                    sample_size,
                    (RTI_UINT32)max_remote_readers,
                    sq_version,
                    &sq_writer))
        {
            goto done;
        }
        if (tp->_intf->initialize_sample)
        {
            init_sample = DDS_DataWriter_get_initialize_writer_loaned_sample(
                                                (DDS_DataWriter*)endpoint);
        }
    }
    OSAPI_Heap_allocate_struct(&plugin, struct ZCOPY_ShmV2Plugin);
    if (plugin == NULL)
    {
        goto done;
    }
    plugin->sq_writer = sq_writer;
    plugin->initialize_sample = init_sample;

done:
    if (plugin == NULL)
    {
#ifndef RTI_CERT
        if (sq_writer != NULL)
        {
            rtn = NETIO_ZCOPY_SharedQWriter_destroy(sq_writer);
            /* call has failed anyway */
            UNUSED_ARG(rtn);
        }
#endif
        return NULL;

    }
    return &plugin->_parent;
}

 void
ZCOPY_ShmV2_memory_plugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct ZCOPY_ShmV2Plugin *plugin = (struct ZCOPY_ShmV2Plugin*)mp;
    RTI_BOOL rtn;

    UNUSED_ARG(p);
#ifndef RTI_CERT
    if (plugin != NULL)
    {
        if (plugin->sq_writer != NULL)
        {
            rtn = NETIO_ZCOPY_SharedQWriter_destroy(plugin->sq_writer);
#if OSAPI_ENABLE_LOG
            if (!rtn)
            {
                DDSC_LOG_MEMORY_MANAGER_DELETE(OSAPI_LOGKIND_ERROR)
            }
#else
            UNUSED_ARG(rtn);
#endif
        }
        OSAPI_Heap_free_struct(plugin);
    }
#else
    UNUSED_ARG(rtn);
    UNUSED_ARG(plugin);
#endif
   return;
}


RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_loan(
        struct DDS_TypePlugin *tp,
        void **sample)
{
    struct ZCOPY_ShmV2Plugin *plugin =
            (struct ZCOPY_ShmV2Plugin*)tp->allocator_plugin;
    SQ_Index sample_id_loaned;
    SQ_Length sample_size;
    SQ_ReturnCode_t return_code;
    RTI_BOOL result = RTI_FALSE;

    return_code = NETIO_ZCOPY_SharedQWriter_get_loan(
            plugin->sq_writer,
            &sample_size,
            (SQ_MemPtr *)sample,
            &sample_id_loaned);

    if (return_code == SQ_RETCODE_OK)
    {
       result = RTI_TRUE;
    }
#if OSAPI_ENABLE_LOG
    else if (return_code == SQ_RETCODE_OUT_OF_RESOURCES)
    {
        DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR)
    }
#endif

    if (plugin->initialize_sample)
    {
        if (!tp->_intf->initialize_sample(tp,*sample))
        {
            return RTI_FALSE;
        }
    }

    return result;
}

RTI_BOOL
ZCOPY_ShmV2_memory_plugin_discard_loan(
        struct DDS_TypePlugin *tp,
        void *sample)
{
    struct ZCOPY_ShmV2Plugin *plugin =
            (struct ZCOPY_ShmV2Plugin*)tp->allocator_plugin;
    SQ_WriterSampleId sample_id;

    if (!NETIO_ZCOPY_SharedQWriter_lookup_loaned_sample_id(
                plugin->sq_writer, (SQ_MemPtr)sample, &sample_id))
    {
        DDSC_LOG_MEMORY_MANAGER_NOT_OWNER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
    if (!NETIO_ZCOPY_SharedQWriter_discard_loan(plugin->sq_writer, sample_id))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}


RTI_BOOL
ZCOPY_ShmV2_memory_plugin_is_owner(
        struct DDS_TypeMemoryPlugin *mp,
        const void *sample)
{
    struct ZCOPY_ShmV2Plugin *plugin = (struct ZCOPY_ShmV2Plugin *)mp;
    SQ_WriterSampleId sample_id;
    /* can be called from the reader as well */
    if (plugin->sq_writer != NULL)
    {
        /* If lookup loan succeeds the plugin is the owner */
        if (!NETIO_ZCOPY_SharedQWriter_lookup_loaned_sample_id(
                    plugin->sq_writer, (SQ_MemPtr)sample, &sample_id))
        {
            return RTI_FALSE;
        }
    }
    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_sample_handle(struct DDS_TypeMemoryPlugin *mp,
                                        const void *address,
                                        void *reference)
{
    struct ZCOPY_ShmV2Plugin *plugin = (struct ZCOPY_ShmV2Plugin *)mp;
    RTI_UINT32 *sample_handle = (RTI_UINT32 *)reference;
    if (plugin->sq_writer != NULL)
    {
        if (!NETIO_ZCOPY_SharedQWriter_lookup_loaned_sample_id(
                    plugin->sq_writer, address, sample_handle))
        {
            return RTI_FALSE;
        }
    }
    return RTI_TRUE;
}


/* Not needed but this can be implemented so that a
 * user does not use a pointer which it previously loaned
 */
 RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out)
{
    UNUSED_ARG(mp);
    UNUSED_ARG(in_address);
    *state_out = TYPEPLUGIN_SAMPLE_STATE_FREE;
    return RTI_TRUE;
}

/* Function is a no op for this type plugin */
 RTI_BOOL
ZCOPY_ShmV2_memory_plugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous)
{
    UNUSED_ARG(mp);
    UNUSED_ARG(in_address);
    UNUSED_ARG(new_state);
    UNUSED_ARG(revert_to_previous);
    return RTI_TRUE;
}


struct DDS_TypePluginSampleHolder*
ZCOPY_ShmV2_WirePlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream)
{
    struct DDS_TypePluginSampleHolder *retval = NULL;
    UNUSED_ARG(stream);
    struct ZCOPY_ShmV2_WirePlugin *plugin = (struct ZCOPY_ShmV2_WirePlugin*)(tp->wire_plugin);
    if (plugin->opaque_samples != NULL)
    {
        retval = REDA_BufferPool_get_buffer(plugin->opaque_samples);
        if (retval != NULL)
        {
            retval->owner = &plugin->_parent;
        }
    }
    return retval;
}

void
ZCOPY_ShmV2_WirePlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample)
{
    struct ZCOPY_ShmV2_WirePlugin *plugin = (struct ZCOPY_ShmV2_WirePlugin*)tp->wire_plugin;
    if (plugin->opaque_samples != NULL)
    {
        REDA_BufferPool_return_buffer(plugin->opaque_samples, sample);
    }
    return;
}


MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_ShmV2_opaque_initialize(void *init_config, void *buffer)
{
    UNUSED_ARG(init_config);
    struct DDS_TypePluginSampleHolder *sample = (struct DDS_TypePluginSampleHolder *)buffer;

    sample->sample= OSAPI_Heap_allocate(1, sizeof(struct NETIO_OpaqueInfo));
    if (sample->sample == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Finalized an opaque sample
 *
 * \details
 * This method is called by the bufferpool when the opaque pool is deleted. This
 * function deletes the opaque sample.
 *
 * \param[in] finalize_config Parameter that was passed to REDA_BufferPool_new()
 * \param[in] buffer          The buffer element to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_ShmV2_opaque_finalize(void *finalize_config, void *buffer)
{
    UNUSED_ARG(finalize_config);
    struct DDS_TypePluginSampleHolder *sample = (struct DDS_TypePluginSampleHolder*)buffer;

    if (sample->sample != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free(sample->sample);
#endif
    }

    return RTI_TRUE;
}



 struct DDS_TypeEncapsulationPlugin*
ZCOPY_ShmV2_WirePlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct ZCOPY_ShmV2_WirePlugin *plugin = NULL;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    UNUSED_ARG(tp);
    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(qos);

    OSAPI_Heap_allocate_struct(&plugin, struct ZCOPY_ShmV2_WirePlugin);
    if (plugin == NULL)
    {
        return NULL;
    }

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_READER)
    {
        pool_property.buffer_size = sizeof(struct DDS_TypePluginSampleHolder);

        pool_property.max_buffers = (RTI_SIZE_T)DDS_DataReader_get_max_samples(
                                                    (DDS_DataReader*)endpoint);

        /* Add one for keep last purposes */
        pool_property.max_buffers++;
        pool_property.flags |= REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
        plugin->opaque_samples = REDA_BufferPool_new(
            "opaque_samples", &pool_property,
            ZCOPY_ShmV2_opaque_initialize, NULL,
            ZCOPY_ShmV2_opaque_finalize, NULL);
    }

    plugin->_parent.memory_plugin = mp;
    return &plugin->_parent;
}

void
ZCOPY_ShmV2_WirePlugin_delete(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *wp)
{
    struct ZCOPY_ShmV2_WirePlugin *plugin = (struct ZCOPY_ShmV2_WirePlugin*)wp;
    RTI_BOOL rtn;

    UNUSED_ARG(tp);
#ifndef RTI_CERT
    if (plugin != NULL)
    {
        if (plugin->opaque_samples != NULL)
        {
            rtn = REDA_BufferPool_delete(plugin->opaque_samples);
#if OSAPI_ENABLE_LOG
            if (!rtn)
            {
                ZCOPY_LOG_DELETE_POOL(OSAPI_LOGKIND_ERROR)
            }
#else
            UNUSED_ARG(rtn);
#endif
        }
        OSAPI_Heap_free_struct(plugin);
    }
#else
    UNUSED_ARG(plugin);
    UNUSED_ARG(rtn);
#endif

    return;
}

RTI_UINT32
ZCOPY_ShmV2_get_serialized_sample_size(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment)
{
    UNUSED_ARG(tp);
    UNUSED_ARG(ep);
    UNUSED_ARG(alignment);
    /* This type plugin does not serialize.
     * It is better to return 0 since the maximum
     * returned across all type plugin is considered
     */
    return 0;
}
