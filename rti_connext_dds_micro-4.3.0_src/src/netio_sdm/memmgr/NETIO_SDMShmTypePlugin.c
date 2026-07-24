/*
 * FILE: NETIO_SDMShmTypePlugin.c - Shared memory type-plugin
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
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
#include "netio_sdm/netio_sdm.h"
#include "netio_sdm/netio_sdm_type_plugin.h"
#include "NETIO_SDMDataReaderMemMgr.h"
#include "NETIO_SDMDataWriterMemMgr.h"

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous)
{
    /* Called on DataWriter side only */
    struct NETIO_TypeShmPlugin *plugin = (struct NETIO_TypeShmPlugin*)mp->allocator_plugin;
    struct SDM_SampleHeader *sample_hdr = (struct SDM_SampleHeader*)in_address;
    RTI_BOOL success = RTI_FALSE;
    sample_hdr--;

    if (revert_to_previous)
    {
        sample_hdr->state = sample_hdr->old_state;
        success = RTI_TRUE;
        goto done;
    }

    /* We are about to commit it to writer history */
    if (new_state == TYPEPLUGIN_SAMPLE_STATE_COMMITTED)
    {
        if (sample_hdr->state == SDM_MEMBUFFERSTATE_ALLOCATED)
        {
            /* This function is called only when calling DataWriter write
             * If the user called get_loan, the the sample's state will be
             * ALLOCATED. If so, the sample is already in the "in_use_list"
             */
            sample_hdr->old_state = sample_hdr->state;
            sample_hdr->state = SDM_MEMBUFFERSTATE_SERIALIZED;
        }
        else if (sample_hdr->state == SDM_MEMBUFFERSTATE_REMOVED)
        {
            /* User is writing a sample that they cached via on_sample_removed
             * So we need to move it from the free_list to in_use_list;
             */
            sample_hdr->old_state = sample_hdr->state;
            sample_hdr->state = SDM_MEMBUFFERSTATE_SERIALIZED;

            if (!DataWriterShmMgr_move_buffer_from_free_list_to_in_use(
                    plugin->data_writer_shm_mgr,
                    in_address))
            {
                SDM_LOG_SAMPLE_STATE(OSAPI_LOGKIND_ERROR);
                goto done;
            }
        }
        else
        {
            SDM_LOG_SAMPLE_STATE(OSAPI_LOGKIND_ERROR);
            goto done;
        }
    }
    else
    {
        /* Currently not expecting to set any other state other than comitted */
        SDM_LOG_SAMPLE_STATE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    success = RTI_TRUE;
done:

    return success;
}

RTI_BOOL
NETIO_TypeShmPlugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out)
{
    struct SDM_SampleHeader *sample_hdr = (struct SDM_SampleHeader*)in_address;
    UNUSED_ARG(mp);

    /* Subtract to access the header preceeding the actual sample */
    sample_hdr--;

    if ((sample_hdr->state == SDM_MEMBUFFERSTATE_ALLOCATED) ||
        (sample_hdr->state == SDM_MEMBUFFERSTATE_REMOVED))
    {
        *state_out = TYPEPLUGIN_SAMPLE_STATE_UNCOMMITTED;
    }
    else if (sample_hdr->state == SDM_MEMBUFFERSTATE_SERIALIZED)
    {
        *state_out = TYPEPLUGIN_SAMPLE_STATE_COMMITTED;
    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_TypeShmPlugin_get_reference(
        struct DDS_TypeMemoryPlugin *mp,
        const void *in_address,
        void *reference)
{
    struct SDM_SampleHeader *dataref = (struct SDM_SampleHeader*)reference;
    UNUSED_ARG(mp);

    /* At this point, this type plugin is the owner of */

    if (!SDM_MemPool_get_buffer_header_from_sample(dataref, in_address))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

void*
NETIO_TypeShmPlugin_get_address(
        struct DDS_TypeMemoryPlugin *mp,
        void *reference)
{
    struct NETIO_TypeShmPlugin *self = (struct NETIO_TypeShmPlugin*)mp;
    struct SDM_MemSegment *segment = NULL;
    struct SDM_SampleHeader *in_state = (struct SDM_SampleHeader*)reference;
    void *address = NULL;

    segment = DataReaderShmMgr_find_segment(
            self->data_reader_shm_mgr,
            in_state->key,
            in_state->shm_segment_epoch);

    if (segment == NULL)
    {
        SDM_LOG_MEMMGR_OUT_OF_SEGMENTS(OSAPI_LOGKIND_ERROR);
        return NULL;
    }

    address = SDM_Segment_get_data_from_reference(segment, in_state);
    if (address == (void*)RTI_SDM_INVALID_SAMPLE_ADDRESS)
    {
        /* In Pro the RTI_SDM_INVALID_SAMPLE_ADDRESS is used when it is
         * not possible to map in a segment due to running out of space.
         * However, checking if a segment returns NULL is already a good
         * indicator that something went wrong.
         */
    }

    return address;
}

RTI_BOOL
NETIO_TypeShmPlugin_return_address(
        struct DDS_TypeMemoryPlugin *mp,
        void *address)
{
    /* Assumption is that this will only be called on the DataReader Side
     */
    RTI_BOOL decrement_success = RTI_FALSE;
    struct NETIO_TypeShmPlugin *self = (struct NETIO_TypeShmPlugin*)mp;

    if (self->data_reader_shm_mgr != NULL)
    {
        decrement_success = DataReaderShmMgr_decrement_reference(
                self->data_reader_shm_mgr,
                address);
    }

    return decrement_success;
}

RTI_BOOL
NETIO_TypeShmPlugin_is_owner(struct DDS_TypeMemoryPlugin *mp,const void *sample)
{
    struct NETIO_TypeShmPlugin *self = (struct NETIO_TypeShmPlugin*)mp;

    if (self->data_writer_shm_mgr != NULL)
    {
        return DataWriterShmMgr_is_buffer_from_here(
                self->data_writer_shm_mgr,
                sample) != NULL;
    }
    else
    {
        return DataReaderShmMgr_is_buffer_from_here(
                self->data_reader_shm_mgr,
                sample);
    }
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmPlugin_create_sample(
        struct DDS_TypePlugin *tp,
        void **sample)
{
    void *new_sample;
    struct RTIXCdrStream tmp_stream;
    struct NETIO_TypeShmPlugin *plugin =
            (struct NETIO_TypeShmPlugin*)tp->allocator_plugin;


    new_sample = DataWriterShmMgr_allocate_buffer(plugin->data_writer_shm_mgr);

    if (new_sample == NULL)
    {
#if OSAPI_ENABLE_LOG
        RTI_INT32 last_error_code = OSAPI_Log_get_last_error_code();
        if (last_error_code == SDM_LOG_DW_OUT_OF_AVAILABLE_SEGMENTS_EC)
        {
            DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR);
        }
#endif /*OSAPI_ENABLE_LOG*/
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
    RTIXCdrStream_set(&tmp_stream, new_sample, 4);
    RTIXCdrStream_serializeAndSetCdrEncapsulation(
            &tmp_stream,
            DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE);

    if (plugin->initialize_sample && tp->_intf->initialize_sample)
    {
        if (!tp->_intf->initialize_sample(tp,new_sample))
        {
            return RTI_FALSE;
        }
    }

    *sample = new_sample;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmPlugin_delete_sample(
        struct DDS_TypePlugin *tp,
        void *sample)
{
    struct NETIO_TypeShmPlugin *plugin =
                OSAPI_Compiler_reinterpret_cast(struct NETIO_TypeShmPlugin*,
                                                tp->memory_plugin);

    struct SDM_SampleHeader *buffer_state =
                OSAPI_Compiler_reinterpret_cast(
                            struct SDM_SampleHeader*,
                            ((char*)sample - sizeof(struct SDM_SampleHeader)));

    if (!DataWriterShmMgr_is_buffer_from_here(
            plugin->data_writer_shm_mgr,
            sample))
    {
        return RTI_FALSE;
    }

    if (buffer_state->state == SDM_MEMBUFFERSTATE_SERIALIZED)
    {
        /* If the DataWriter is returning a sample after serializing it,
         * then we mark it as removed
         */
        return DataWriterShmMgr_free_buffer(
                plugin->data_writer_shm_mgr,
                sample,
                RTI_TRUE);
    }
    else
    {
        /* Buffer state must be ALLOCATED or REMOVED.*/
        return DataWriterShmMgr_free_buffer(
                plugin->data_writer_shm_mgr,
                sample,
                RTI_FALSE);
    }
}

RTI_BOOL
NETIO_TypeShmPlugin_serialize_inline_qos(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeMemoryPlugin *plugin,
        struct CDR_Stream_t *stream,
        const void* const sample,
        DDS_InstanceHandle_t *destination)
{
    /* This has to be read from the buffer when complete */
    struct REDA_SequenceNumber epoch = SDM_MemPool_get_related_epoch(sample);
    DDS_UnsignedShort pid_id;
    DDS_UnsignedShort pid_length;

    UNUSED_ARG(tp);
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(destination);

    pid_id = RTPS_PID_SAMPLE_EPOCH;
    pid_length = 8;

    if (!CDR_Stream_serialize_unsigned_short(stream, &pid_id))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_short(stream, &pid_length))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_long(stream, &epoch.high))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_long(stream, &epoch.low))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_TypeShmPlugin_is_sample_consistent(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeMemoryPlugin *plugin,
        DDS_Boolean *is_data_consistent,
        const void *sample,
        const struct DDS_SampleInfo *sample_info)
{
    /* Need to check against buffer header and related epoch in sample_info */
    struct REDA_SequenceNumber epoch = SDM_MemPool_get_related_epoch(sample);
    UNUSED_ARG(tp);
    UNUSED_ARG(plugin);

    if (REDA_SequenceNumber_is_zero(
            (const struct REDA_SequenceNumber*)&sample_info->related_epoch))
    {
        /* The epoch has not been sent as part of the inline QoS
         * The necessary conditions have not been met so the check fails.
         * This results in PRECONDITION_NOT_MET to the user
         */
        return RTI_FALSE;
    }

    if (REDA_SequenceNumber_compare(
            &epoch,
            (const struct
            REDA_SequenceNumber*)&sample_info->related_epoch.high) == 0 )
    {
        *is_data_consistent = DDS_BOOLEAN_TRUE;
    }
    else
    {
        *is_data_consistent = DDS_BOOLEAN_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
NETIO_TypeShmPlugin_locator_kind_exists(DDS_TypePluginEndpoint *endpoint,
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
            /* we want the default locators instead*/
            uc = DDS_DataWriter_get_resolved_locator_ref((DDS_DataWriter*)endpoint);
        }

        len = DDS_LocatorSeq_get_length(uc);
        for (i = 0; i < len; i++)
        {
            locator = DDS_LocatorSeq_get_reference(uc, i);
            if (locator->kind == NETIO_ADDRESS_KIND_SHMEM)
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
            /* we want the default locators instead*/
            uc = DDS_DataReader_get_resolved_locator_ref((DDS_DataReader*)endpoint);
        }

        len = DDS_LocatorSeq_get_length(uc);
        for (i = 0; i < len; i++)
        {
            locator = DDS_LocatorSeq_get_reference(uc, i);
            if (locator->kind == NETIO_ADDRESS_KIND_SHMEM)
            {
               retval = RTI_TRUE;
               goto done;
            }
        }

    }
done:
    return retval;

}

struct DDS_TypeMemoryPlugin*
NETIO_TypeShmPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos_deprecated)
{
    struct NETIO_TypeShmPlugin *plugin = (struct NETIO_TypeShmPlugin*)tp;
    DDS_ReturnCode_t retcode;
    DDS_Long writer_loaned_sample_allocation;
    UNUSED_ARG(qos_deprecated);

    OSAPI_Heap_allocate_struct(&plugin, struct NETIO_TypeShmPlugin);
    if (plugin == NULL)
    {
        goto failure;
    }

    plugin->data_reader_shm_mgr = NULL;
    plugin->data_writer_shm_mgr = NULL;
    plugin->initialize_sample = DDS_BOOLEAN_FALSE;
    plugin->dp_user_data = NULL;
    plugin->mode = endpoint_mode;
    plugin->domain_participant = participant;

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        /* only create the plugin if a locator of type SHMEM exists */
        if (!NETIO_TypeShmPlugin_locator_kind_exists(endpoint,endpoint_mode))
        {
            goto failure;
        }
        struct DataWriterShmMgrProperty p = SDM_DataWriterShmMgrProp_INITALIZER;
        struct SDM_UserData *sdm_user_data = NULL;
        DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;

        handle = DDS_Entity_get_instance_handle((DDS_Entity*)endpoint);
        retcode = DDS_DomainParticipant_get_user_data(
                participant,
                DDS_PARTICIPANT_USER_DATA_ZERO_COPY_SHARED_DATA,
                (void**)&sdm_user_data);

        if (retcode != DDS_RETCODE_OK)
        {
            goto failure;
        }

        /* If this is the first writer being created in a participant, create
         * the bitmap which will server for managing which keys can be used
         * to create shared memory segments
         */
        if (sdm_user_data == NULL)
        {
            sdm_user_data = SDM_UserData_new(
                    dp_qos->resource_limits.shmem_ref_transfer_mode_max_segments);

            retcode = DDS_DomainParticipant_set_user_data(
                    participant,
                    DDS_PARTICIPANT_USER_DATA_ZERO_COPY_SHARED_DATA,
                    sdm_user_data);
            if (retcode != DDS_RETCODE_OK)
            {
                goto failure;
            }
        }

        SDM_UserData_increment_ref_count(sdm_user_data);
        p.key_bitmap = SDM_UserData_get_key_bitmap(sdm_user_data);
        p.domain_id = DDS_DomainParticipant_get_domain_id(participant);
        p.participant_index = DDS_DomainParticipant_get_participant_id(participant);
        p.participant_gain = (RTI_INT32)dp_qos->resource_limits.shmem_ref_transfer_mode_max_segments;

        /* Only flat memory is supported. For the plain language binding the
         * sizeof(T) is sufficient. For FLAT_DATA buffers we would need to
         * call the interpreter.
         */
        p.user_sample_buffer_max_size = DDS_TypePlugin_get_sample_size(tp);
        writer_loaned_sample_allocation =
                    DDS_DataWriter_get_writer_loaned_sample_allocation(
                                                    (DDS_DataWriter*)endpoint);

        if (writer_loaned_sample_allocation >= 1)
        {
            /* If the user specified a value, then that value is used as is.
             */
            p.max_user_sample_buffers = (RTI_UINT32)writer_loaned_sample_allocation;
        }
        else
        {
            DDS_Long max_samples;

            max_samples = DDS_DataWriter_get_max_samples((DDS_DataWriter*)endpoint);

            /* We need at least 1 additional buffer to be able to push samples
             * out of the queue.
             */
            p.max_user_sample_buffers = (RTI_UINT32)max_samples + 1;
        }

        OSAPI_Memory_copy(&p.datawriter_owner_guid, handle.octet,
                          sizeof(p.datawriter_owner_guid));

        plugin->data_writer_shm_mgr = DataWriterShmMgr_new(&p);
        if (plugin->data_writer_shm_mgr == NULL)
        {
            SDM_UserData_decrement_ref_count(sdm_user_data);
            goto failure;
        }

        {
            plugin->initialize_sample =
                        DDS_DataWriter_get_initialize_writer_loaned_sample(
                                (DDS_DataWriter*)endpoint);
            plugin->dp_user_data = sdm_user_data;
        }

    }
    else /* READER */
    {
        struct DataReaderShmMgrProperty prop;
        DDS_Long max_remote_writers;
        DDS_Long shmem_ref_transfer_mode_attached_segment_allocation;

        max_remote_writers = DDS_DataReader_get_max_remote_writers(
                                (DDS_DataReader*)endpoint);

        shmem_ref_transfer_mode_attached_segment_allocation =
            DDS_DataReader_get_shmem_ref_transfer_mode_attached_segment_allocation(
                        (DDS_DataReader*)endpoint);

        /* only create the plugin if a locator of type SHMEM exists */
        if (!NETIO_TypeShmPlugin_locator_kind_exists(endpoint,endpoint_mode))
        {
            goto failure;
        }

        if (shmem_ref_transfer_mode_attached_segment_allocation >= 1)
        {
            prop.max_remote_segments =
                            shmem_ref_transfer_mode_attached_segment_allocation;
        }
        else
        {
            prop.max_remote_segments = max_remote_writers;
        }

        plugin->data_reader_shm_mgr = DataReaderShmMgr_new(&prop);
        if (plugin->data_reader_shm_mgr == NULL)
        {
            goto failure;
        }
    }

    return &plugin->_parent;

failure:

    if (plugin != NULL)
    {
        OSAPI_Heap_free(plugin);
    }

    return NULL;
}

void
NETIO_TypeShmPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct NETIO_TypeShmPlugin *plugin = (struct NETIO_TypeShmPlugin*)mp;
    UNUSED_ARG(p);

    if (plugin->mode == DDS_TYPEPLUGIN_MODE_READER)
    {
        if (plugin->data_reader_shm_mgr != NULL)
        {
            DataReaderShmMgr_delete(plugin->data_reader_shm_mgr);
        }
    }
    else if (plugin->mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        if (plugin->data_writer_shm_mgr != NULL)
        {
            RTI_INT32 ref_count = 0;

            /* First delete the type plugin */
            DataWriterShmMgr_delete(plugin->data_writer_shm_mgr);

            SDM_UserData_decrement_ref_count(plugin->dp_user_data);
            ref_count = SDM_UserData_get_ref_count(plugin->dp_user_data);

            if (ref_count == 0)
            {
                SDM_UserData_delete(plugin->dp_user_data);
                DDS_DomainParticipant_set_user_data(
                        plugin->domain_participant,
                        DDS_PARTICIPANT_USER_DATA_ZERO_COPY_SHARED_DATA,
                        NULL);
            }
        }
    }

    OSAPI_Heap_free_struct(mp);
}

RTI_BOOL
NETIO_TypeShmPlugin_add_peer(
        struct DDS_TypeMemoryPlugin *mp,
        DDS_InstanceHandle_t *peer)
{
    UNUSED_ARG(mp);
    UNUSED_ARG(peer);
    return RTI_TRUE;
}

RTI_BOOL
NETIO_TypeShmPlugin_remove_peer(
        struct DDS_TypeMemoryPlugin *mp,
        DDS_InstanceHandle_t *peer)
{
    struct NETIO_TypeShmPlugin *plugin = (struct NETIO_TypeShmPlugin*)mp;

    DDS_Octet oid = peer->octet[12];
    RTI_BOOL remove_peer_success = RTI_FALSE;

    /* RTPS 2.2 (Table 9.1)
     *
     * Writer (with Key) 0x02
     * Writer (no Key) 0x03
     * Reader (no Key) 0x04
     * Reader (with Key) 0x07
     */

    if (oid <= 0x03)
    {
        remove_peer_success =
                DataReaderShmMgr_unmap_all_segments_with_publication_id(
                plugin->data_reader_shm_mgr,
                (struct NETIO_Guid*)&peer->octet);
    }
    else if (oid <= 0x07)
    {
        remove_peer_success = RTI_TRUE;
    }
    else
    {
        remove_peer_success = RTI_FALSE;
    }

    return remove_peer_success;
}

void*
NETIO_TypeShmStreamPlugin_get_buffer(struct DDS_TypePlugin *tp)
{
    struct NETIO_TypeShmStreamPlugin *self =
            (struct NETIO_TypeShmStreamPlugin*)tp->wire_plugin;
    struct DDS_TypePluginBuffer *tbuf;

    tbuf = DDS_TypePlugin_init_inline_buffer(
            &self->_parent,
            self->buffer_pool,
            self->size);

    return tbuf;
}

void
NETIO_TypeShmStreamPlugin_return_buffer(
        struct DDS_TypePlugin *tp,
        void *buffer)
{
    struct NETIO_TypeShmStreamPlugin *self =
            (struct NETIO_TypeShmStreamPlugin*)tp->wire_plugin;

    REDA_BufferPool_return_buffer(self->buffer_pool,buffer);
}

struct DDS_TypePluginSampleHolder*
NETIO_TypeShmStreamPlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream)
{
    struct NETIO_TypeShmStreamPlugin *self =
            (struct NETIO_TypeShmStreamPlugin*)tp->wire_plugin;
    struct SDM_SampleHeader in_state;
    struct DDS_TypePluginSampleHolder *out_sample = NULL;
    void *sample;

    if (!CDR_Stream_deserialize_primitive_array(stream,
                                                &in_state.key,
                                                SIZE_OF_SHM_REFERENCE,
                                                CDR_OCTET_TYPE))
    {
        SDM_LOG_DESERIALIZE(OSAPI_LOGKIND_ERROR);
        return NULL;
    }

    sample = DDS_TypeMemoryPlugin_get_address(self->_parent.memory_plugin,
                                              &in_state);
    if (sample == NULL)
    {
        return NULL;
    }

    out_sample = REDA_BufferPool_get_buffer(self->buffer_pool);
    if (out_sample == NULL)
    {
        return NULL;
    }
    out_sample->sample = sample;
    out_sample->owner = &self->_parent;

    return out_sample;
}

void
NETIO_TypeShmStreamPlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample)
{
    struct NETIO_TypeShmStreamPlugin *self =
            (struct NETIO_TypeShmStreamPlugin*)sample->owner;
    UNUSED_ARG(tp);

    DDS_TypePlugin_return_address(tp,sample->sample);

    REDA_BufferPool_return_buffer(self->buffer_pool,sample);
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmStreamPlugin_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination)
{
    struct NETIO_TypeShmStreamPlugin *self =
            (struct NETIO_TypeShmStreamPlugin*)plugin->wire_plugin;
    struct SDM_SampleHeader dataref;
    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if (!DDS_TypeMemoryPlugin_get_reference(
            self->_parent.memory_plugin,
            data,
            &dataref))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_primitive_array(
            stream,
            &dataref.key,
            SIZE_OF_SHM_REFERENCE,
            CDR_OCTET_TYPE))
    {
        SDM_LOG_SERIALIZE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmStreamPlugin_serialize_flat_data(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if (!CDR_Stream_serialize_header(
            stream,
            DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_NATIVE,
            RTI_CDR_ENCAPSULATION_OPTIONS_NONE))
    {
        SDM_LOG_SERIALIZE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return NETIO_TypeShmStreamPlugin_serialize(plugin, stream, data, destination);
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmStreamPlugin_deserialize_flat_data(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(stream);
    UNUSED_ARG(source);

    /* At this point, we already deserialized the encapsulation header
     * as well as the
     * The stream is at its end so the "remaining space" is 0
     */
    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_TypeShmStreamPlugin_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(stream);
    UNUSED_ARG(source);

    /* Sample is already pointing to the correct sample */

    return RTI_TRUE;
}

struct DDS_TypeEncapsulationPlugin*
NETIO_TypeShmStreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos_deprecated,
        struct DDS_TypeMemoryPlugin *mp)
{
    struct NETIO_TypeShmStreamPlugin *plugin = NULL;
    struct REDA_BufferPoolProperty bp = REDA_BufferPoolProperty_INITIALIZER;

    UNUSED_ARG(participant);
    UNUSED_ARG(dp_qos);
    UNUSED_ARG(endpoint);
    UNUSED_ARG(qos_deprecated);

    OSAPI_Heap_allocate_struct(&plugin, struct NETIO_TypeShmStreamPlugin);
    if (plugin == NULL)
    {
        return NULL;
    }

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        plugin->size = (RTI_UINT32)sizeof(struct DDS_TypePluginBuffer) +
                RTI_CDR_ENCAPSULATION_HEADER_SIZE +
                SIZE_OF_SHM_REFERENCE +
                (RTI_UINT32)tp->property.head_padding +
                (RTI_UINT32)tp->property.tail_padding;

        bp.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
        bp.buffer_size = plugin->size;
        bp.max_buffers = (RTI_SIZE_T)DDS_DataWriter_get_max_samples(
                                        (DDS_DataWriter*)endpoint);
        plugin->buffer_pool = REDA_BufferPool_new("shmref",&bp,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL);
    }
    else
    {
        bp.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
        bp.buffer_size = sizeof(struct DDS_TypePluginSampleHolder);

        bp.max_buffers = (RTI_SIZE_T)DDS_DataReader_get_max_samples(
                                            (DDS_DataReader*)endpoint);

        plugin->buffer_pool = REDA_BufferPool_new("shmdref",&bp,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL);
    }

    if (plugin->buffer_pool == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(plugin);
#endif
        return NULL;
    }

    plugin->_parent.memory_plugin = mp;

    return &plugin->_parent;
}

RTI_UINT32
NETIO_TypeShmStreamPlugin_get_serialized_sample_size(struct DDS_TypePlugin *tp,
                                    struct DDS_TypeEncapsulationPlugin *ep,
                                    RTI_UINT32 alignment)
{
    UNUSED_ARG(tp);
    UNUSED_ARG(ep);
    UNUSED_ARG(alignment);

    return SIZE_OF_SHM_REFERENCE;
}

void
NETIO_TypeShmStreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp)
{
    struct NETIO_TypeShmStreamPlugin *plugin = (struct
            NETIO_TypeShmStreamPlugin*)mp;
    UNUSED_ARG(p);

    if (plugin->buffer_pool != NULL)
    {
        if (!REDA_BufferPool_delete(plugin->buffer_pool))
        {
            /* Exception */
        }
    }

    OSAPI_Heap_free_struct(mp);
}
