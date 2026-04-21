/*
 * FILE: netio_sdm_type_plugin.h
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_sdm_type_plugin_h
#define netio_sdm_type_plugin_h

#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "netio_sdm/netio_sdm.h"

#ifdef __cplusplus
extern "C" {
#endif

struct NETIO_TypeShmPlugin
{
    struct DDS_TypeMemoryPlugin _parent;

    struct DataWriterShmMgr *data_writer_shm_mgr;

    struct DataReaderShmMgr *data_reader_shm_mgr;

    struct SDM_UserData *dp_user_data;

    DDS_DomainParticipant *domain_participant;

    DDS_Boolean initialize_sample;

    DDS_TypePluginMode_T mode;
};

#ifndef RTI_SDM_INVALID_SAMPLE_ADDRESS
#define RTI_SDM_INVALID_SAMPLE_ADDRESS 0x1
#endif

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_get_reference(
        struct DDS_TypeMemoryPlugin *mp,
        const void *in_address,
        void *reference);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous);

NETIO_SDMDllExport void*
NETIO_TypeShmPlugin_get_address(
        struct DDS_TypeMemoryPlugin *mp,
        void *reference);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_return_address(
        struct DDS_TypeMemoryPlugin *mp,
        void *address);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_is_owner(
        struct DDS_TypeMemoryPlugin *mp,
        const
        void *sample);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_create_sample(
        struct DDS_TypePlugin *tp,
        void **sample);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_delete_sample(
        struct DDS_TypePlugin *tp,
        void *sample);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_serialize_inline_qos(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeMemoryPlugin *plugin,
        struct CDR_Stream_t *stream,
        const void* const sample,
        DDS_InstanceHandle_t *destination);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_is_sample_consistent(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeMemoryPlugin *plugin,
        DDS_Boolean *is_data_consistent,
        const void *sample,
        const struct DDS_SampleInfo *sample_info);

NETIO_SDMDllExport struct DDS_TypeMemoryPlugin*
NETIO_TypeShmPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos);

NETIO_SDMDllExport void
NETIO_TypeShmPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_add_peer(
        struct DDS_TypeMemoryPlugin *mp,
        DDS_InstanceHandle_t *peer);

NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmPlugin_remove_peer(
        struct DDS_TypeMemoryPlugin *mp,
        DDS_InstanceHandle_t *peer);

struct NETIO_TypeShmStreamPlugin
{
    struct DDS_TypeEncapsulationPlugin _parent;

    REDA_BufferPool_T buffer_pool;

    RTI_UINT32 size;
};

NETIO_SDMDllExport void*
NETIO_TypeShmStreamPlugin_get_buffer(struct DDS_TypePlugin *tp);

NETIO_SDMDllExport void
NETIO_TypeShmStreamPlugin_return_buffer(
        struct DDS_TypePlugin *tp,
        void *buffer);

NETIO_SDMDllExport struct DDS_TypePluginSampleHolder*
NETIO_TypeShmStreamPlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream);

NETIO_SDMDllExport void
NETIO_TypeShmStreamPlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmStreamPlugin_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmStreamPlugin_serialize_flat_data(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmStreamPlugin_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
NETIO_TypeShmStreamPlugin_deserialize_flat_data(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source);

NETIO_SDMDllExport struct DDS_TypeEncapsulationPlugin*
NETIO_TypeShmStreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp);

NETIO_SDMDllExport void
NETIO_TypeShmStreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp);

NETIO_SDMDllExport RTI_UINT32
NETIO_TypeShmStreamPlugin_get_serialized_sample_size(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* netio_sdm_type_plugin_h */
