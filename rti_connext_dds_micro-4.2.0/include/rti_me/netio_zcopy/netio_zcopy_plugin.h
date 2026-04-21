/*
 * FILE: NETIO_ZCopy_Plugin.h - Zero Copy Plugin Helper Declarations
 *
 * Copyright (c) 2023-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef netio_zcopy_plugin_h
#define netio_zcopy_plugin_h

#include "dds_c/dds_c_type.h"
#include "netio_zcopy_sharedq.h"
#include "netio_zcopy_sharedq_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ZCOPY_ShmV2Plugin
{
    struct DDS_TypeMemoryPlugin _parent;

    NETIO_ZCOPY_SharedQWriter *sq_writer;

    RTI_BOOL initialize_sample;
};

extern struct DDS_TypeMemoryPlugin*
ZCOPY_ShmV2_memory_plugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos);


extern void
ZCOPY_ShmV2_memory_plugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_loan(
        struct DDS_TypePlugin *tp,
        void **sample);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_discard_loan(
        struct DDS_TypePlugin *tp,
        void *sample);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_is_owner(
        struct DDS_TypeMemoryPlugin *mp,
        const void *sample);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_sample_handle(
        struct DDS_TypeMemoryPlugin *mp,
        const void *address,
        void *reference);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out);

extern RTI_BOOL
ZCOPY_ShmV2_memory_plugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous);

struct ZCOPY_ShmV2_WirePlugin
{
    struct DDS_TypeEncapsulationPlugin _parent;

    REDA_BufferPool_T opaque_samples;
};


extern void*
ZCOPY_ShmV2_get_buffer(struct DDS_TypePlugin *tp);

extern RTI_UINT32
ZCOPY_ShmV2_get_serialized_sample_size(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment);

extern struct DDS_TypeEncapsulationPlugin*
ZCOPY_ShmV2_WirePlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp);

extern void
ZCOPY_ShmV2_WirePlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp);

extern struct DDS_TypePluginSampleHolder*
ZCOPY_ShmV2_WirePlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream);

extern void
ZCOPY_ShmV2_WirePlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample);


#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_zcopy_plugin_h */
