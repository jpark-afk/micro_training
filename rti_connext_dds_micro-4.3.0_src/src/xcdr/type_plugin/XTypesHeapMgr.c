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
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"
#include "reda/reda_log.h"

#include "xcdr/xcdr_heapmgr.h"

RTI_BOOL
DDS_XTypesHeapPlugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous_state)
{
    struct DDS_XTypesHeapPlugin* plugin = (struct DDS_XTypesHeapPlugin*)mp->allocator_plugin;
    RTI_BOOL success = RTI_FALSE;

    if (revert_to_previous_state)
    {
        success = XCDR_HeapMgr_revert_sample_state(plugin->heap_manager, in_address);
        goto done;
    }

    if (new_state == TYPEPLUGIN_SAMPLE_STATE_COMMITTED)
    {
        if (!XCDR_HeapMgr_set_buffer_in_use(plugin->heap_manager, in_address))
        {
            DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR(OSAPI_LOGKIND_ERROR);
            goto done;
        }
    }
    else
    {
        /* Currently not expecting to set any other state other than comitted */
        DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    success = RTI_TRUE;

done:

    return success;
}

RTI_BOOL
DDS_XTypesHeapPlugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out)
{
    struct DDS_XTypesHeapPlugin* plugin = (struct DDS_XTypesHeapPlugin*)mp->allocator_plugin;
    RTI_BOOL is_in_use;
    RTI_BOOL success = RTI_FALSE;

    if (XCDR_HeapMgr_is_buffer_in_use(plugin->heap_manager, in_address, &is_in_use))
    {
        if (is_in_use)
        {
            *state_out = TYPEPLUGIN_SAMPLE_STATE_COMMITTED;
        }
        else
        {
            *state_out = TYPEPLUGIN_SAMPLE_STATE_UNCOMMITTED;
        }
    }
    else
    {
        DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    success = RTI_TRUE;

done:

    return success;
}

RTI_BOOL
DDS_XTypesHeapPlugin_get_reference(
        struct DDS_TypeMemoryPlugin *mp,
        const void *in_address,
        void *reference)
{
    UNUSED_ARG(mp);
    UNUSED_ARG(in_address);
    UNUSED_ARG(reference);

    /* This should never be called */
    return RTI_FALSE;
}

RTI_BOOL
DDS_XTypesHeapPlugin_is_owner(struct DDS_TypeMemoryPlugin *mp, const void *sample)
{
    struct DDS_XTypesHeapPlugin* plugin = (struct DDS_XTypesHeapPlugin*) mp;
    RTI_BOOL is_owner = RTI_FALSE;

    if (plugin->endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
       is_owner = XCDR_HeapMgr_is_owner(plugin->heap_manager, sample);
    }
    else /* XTypes Heap Plugin on Reader */
    {
        /* For a heap manager, this function should only be called on the writer side
         * Since the XTypes Heap Manager requires is_owner for DataWriter side,
         * we need to make sure that for DataReaders, this always return false.
         * The Zero Copy plugin calls TypePlugin_is_owner on the reader side
         * which ends up calling this function.
         */
        is_owner = RTI_FALSE;
    }

    return is_owner;
}

MUST_CHECK_RETURN RTI_BOOL
DDS_XTypesHeapPlugin_create_sample(
        struct DDS_TypePlugin *tp,
        void **sample)
{
    struct DDS_XTypesHeapPlugin* heap_plugin;
    heap_plugin = (struct DDS_XTypesHeapPlugin*) tp->allocator_plugin;

    if (!heap_plugin->is_managing_flat_data_samples)
    {
        *sample = NULL;
        return RTI_FALSE;
    }

    if (heap_plugin->endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        char* loaned_sample = XCDR_HeapMgr_allocate_buffer(heap_plugin->heap_manager);
        struct RTIXCdrStream tmp_stream;

        if (loaned_sample == NULL)
        {
#if OSAPI_ENABLE_LOG
            RTI_INT32 last_error_code = OSAPI_Log_get_last_error_code();

            if (last_error_code == REDA_LOG_HEAP_MGR_GET_BUFFER_EC)
            {
                DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR);
            }
#endif /*OSAPI_ENABLE_LOG*/
            *sample = NULL;

            return RTI_FALSE;
        }

        /* At this point, the assumption is that memory returned by DataWriterShmMgr
         * is unallocated and contains garbage.
         *
         * Therefore, we set the first 4 bytes to the native final encapsulation
         * because if this memory manager is managing FLAT_DATA, then the first
         * 4 bytes (the encapsulation header) must be initialized. FLAT_DATA
         * currently only supports FINAL and MUTABLE extensibility. However,
         * the FLAT_DATA MUTABLE API will set the encapsulation each time.
         * APPENDABLE is not support so setting the encapsulation to FINAL
         * is sufficient in all cases.
         *
         * When FLAT_DATA supports APPENDABLE extensibility, this need to change
         * to either FINAL or APPENDABLE extensibility.
         */
        RTIXCdrStream_init(&tmp_stream);
        RTIXCdrStream_set(&tmp_stream, loaned_sample, 4);
        RTIXCdrStream_serializeAndSetCdrEncapsulation(
                &tmp_stream,
                DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE);

        *sample = loaned_sample;

        if (heap_plugin->initialize_sample && tp->_intf->initialize_sample)
        {
            if (!tp->_intf->initialize_sample(tp, loaned_sample))
            {
                return RTI_FALSE;
            }
        }
    }
    else
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
DDS_XTypesHeapPlugin_delete_sample(
        struct DDS_TypePlugin *tp,
        void *sample)
{
    struct DDS_XTypesHeapPlugin* heap_plugin;
    heap_plugin = (struct DDS_XTypesHeapPlugin*) tp->allocator_plugin;
    return XCDR_HeapMgr_return_buffer(heap_plugin->heap_manager, sample);
}

struct DDS_TypeMemoryPlugin*
DDS_XTypesHeapPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos_deprecated,
        XCDR_HeapMgr_gen_init initialize_sample_writer_pool,
        XCDR_HeapMgr_gen_init finalize_sample_writer_pool,
        DDS_TypePlugin_initialize_pool_sample_T initialize_sample_reader_pool,
        DDS_TypePlugin_finalize_pool_sample_T finalize_sample_reader_pool,
        DDS_UnsignedLong top_level_type_size,
        RTI_BOOL is_flat_data)
{
    struct DDS_XTypesHeapPlugin *new_heap_plugin = NULL;
    struct XCDR_HeapMgrProperty property;
    struct REDA_BufferPoolProperty bufp = REDA_BufferPoolProperty_INITIALIZER;

    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);
    UNUSED_ARG(qos_deprecated);

    OSAPI_Heap_allocate_struct(&new_heap_plugin, struct DDS_XTypesHeapPlugin);
    if (new_heap_plugin == NULL)
    {
        return NULL;
    }

    new_heap_plugin->heap_manager = NULL;
    new_heap_plugin->sample_holder_pool = NULL;
    new_heap_plugin->endpoint_mode = endpoint_mode;
    new_heap_plugin->is_managing_flat_data_samples = is_flat_data;
    new_heap_plugin->initialize_sample = RTI_FALSE;

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        DDS_Long writer_loaned_sample_allocation;
        DDS_Long max_samples;
        DDS_Boolean initialize_writer_loaned_sample;

        writer_loaned_sample_allocation =
                      DDS_DataWriter_get_writer_loaned_sample_allocation(
                                                  (DDS_DataWriter*)endpoint);

        max_samples = DDS_DataWriter_get_max_samples((DDS_DataWriter*)endpoint);

        initialize_writer_loaned_sample =
                      DDS_DataWriter_get_initialize_writer_loaned_sample(
                                        (DDS_DataWriter*)endpoint);

        /* A type cannot have a negative size */
        property.buffer_size = (RTI_SIZE_T)top_level_type_size;

        property.initialize_func = initialize_sample_writer_pool;
        property.finalize_func = finalize_sample_writer_pool;

        if (writer_loaned_sample_allocation >= 1)
        {
            /* If the user specified a value, then that value is used as is.
             */
            property.max_buffers = (RTI_SIZE_T)writer_loaned_sample_allocation;
        }
        else
        {
            property.max_buffers = (RTI_SIZE_T)max_samples + 1U;
        }

        if (initialize_writer_loaned_sample)
        {
            new_heap_plugin->initialize_sample = RTI_TRUE;
        }
        new_heap_plugin->heap_manager = XCDR_HeapMgr_new(&property);
    }
    else
    {

        bufp.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_TypePluginSampleHolder);
        if (tp->property.max_buffers == 0)
        {
            bufp.max_buffers = (RTI_SIZE_T)DDS_DataReader_get_max_samples(
                                                    (DDS_DataReader*)endpoint);
        }
        else
        {
            bufp.max_buffers = (RTI_SIZE_T)tp->property.max_buffers;
        }
        new_heap_plugin->sample_holder_pool =
                        REDA_BufferPool_new("cdr_samples",
                                            &bufp,
                                            initialize_sample_reader_pool,tp,
                                            finalize_sample_reader_pool,tp);

        if (new_heap_plugin->sample_holder_pool == NULL)
        {
#ifndef RTI_CERT
            OSAPI_Heap_free_struct(new_heap_plugin);
#endif
            return NULL;
        }
    }

    return &new_heap_plugin->_parent;
}

void
DDS_XTypesHeapPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp)
{
#ifndef RTI_CERT
    struct DDS_XTypesHeapPlugin* plugin = (struct DDS_XTypesHeapPlugin*)mp;
    UNUSED_ARG(p);

    if (plugin->sample_holder_pool != NULL)
    {
        if (!REDA_BufferPool_delete(plugin->sample_holder_pool))
        {

        }
    }

    if (plugin->heap_manager != NULL)
    {
        if (!XCDR_HeapMgr_delete(plugin->heap_manager))
        {

        }
    }
    OSAPI_Heap_free(plugin);
#else
    UNUSED_ARG(p);
    UNUSED_ARG(mp);
#endif
}
