/*
 * FILE: DDS_ParticipantMessageDataPlugin.c
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */


/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from DDS_ParticipantMessageData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#include "DDS_ParticipantMessageData.h"
#include "reda/reda_bufferpool.h"
#include "DDS_ParticipantMessageDataPlugin.h"

/*** SOURCE_BEGIN ***/
#ifndef UNUSED_ARG
#define UNUSED_ARG(x) (void)(x)
#endif

/* --------------------------------------------------------------------------
(De)Serialize functions:
* -------------------------------------------------------------------------- */
RTI_UINT32
DDS_GuidPrefix_t_get_serialized_sample_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;
    UNUSED_ARG(plugin);

    current_alignment += CDR_get_max_size_serialized_primitive_array(
        current_alignment, (12), CDR_OCTET_TYPE);

    return  current_alignment - initial_alignment;
}
RTI_BOOL
DDS_GuidPrefix_t_cdr_serialize(struct DDS_TypePlugin *plugin,
struct CDR_Stream_t *stream,
const void *void_sample,
DDS_InstanceHandle_t *destination)
{
    DDS_GuidPrefix_t *sample = (DDS_GuidPrefix_t *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if (!CDR_Stream_serialize_primitive_array(
        stream,
        (void*)*sample,
        (12),
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_GuidPrefix_t_cdr_deserialize(struct DDS_TypePlugin *plugin,
void *void_sample,
struct CDR_Stream_t *stream,
DDS_InstanceHandle_t *source)
{
    DDS_GuidPrefix_t *sample = (DDS_GuidPrefix_t*)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(source);

    if (!CDR_Stream_deserialize_primitive_array(
        stream,
        (void*)*sample,
        (12),
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

/* --------------------------------------------------------------------------
(De)Serialize functions:
* -------------------------------------------------------------------------- */
RTI_UINT32
DDS_OctetArray4_get_serialized_sample_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;
    UNUSED_ARG(plugin);

    current_alignment += CDR_get_max_size_serialized_primitive_array(
        current_alignment, (4), CDR_OCTET_TYPE);

    return  current_alignment - initial_alignment;
}
RTI_BOOL
DDS_OctetArray4_cdr_serialize(struct DDS_TypePlugin *plugin,
struct CDR_Stream_t *stream,
const void *void_sample,
DDS_InstanceHandle_t *destination)
{
    DDS_OctetArray4 *sample = (DDS_OctetArray4 *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if (!CDR_Stream_serialize_primitive_array(
        stream,
        (void*)*sample,
        (4),
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_OctetArray4_cdr_deserialize(struct DDS_TypePlugin *plugin,
void *void_sample,
struct CDR_Stream_t *stream,
DDS_InstanceHandle_t *source)
{
    DDS_OctetArray4 *sample = (DDS_OctetArray4*)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(source);

    if (!CDR_Stream_deserialize_primitive_array(
        stream,
        (void*)*sample,
        (4),
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

/* --------------------------------------------------------------------------
(De)Serialize functions:
* -------------------------------------------------------------------------- */
RTI_UINT32
DDS_Liveliness_ParticipantMessageDataKey_get_serialized_sample_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;
    UNUSED_ARG(plugin);

    current_alignment += DDS_GuidPrefix_t_get_serialized_sample_size(
        plugin,current_alignment);

    current_alignment += DDS_OctetArray4_get_serialized_sample_size(
        plugin,current_alignment);

    return  current_alignment - initial_alignment;
}
RTI_BOOL
DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize(struct DDS_TypePlugin *plugin,
struct CDR_Stream_t *stream,
const void *void_sample,
DDS_InstanceHandle_t *destination)
{
    DDS_Liveliness_ParticipantMessageDataKey *sample = (DDS_Liveliness_ParticipantMessageDataKey *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if(!DDS_GuidPrefix_t_cdr_serialize(
        plugin,
        stream,
        &sample->participant_guid_prefix,
        NULL)) {
        return RTI_FALSE;
    }
    if(!DDS_OctetArray4_cdr_serialize(
        plugin,
        stream,
        &sample->kind,
        NULL)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize(struct DDS_TypePlugin *plugin,
void *void_sample,
struct CDR_Stream_t *stream,
DDS_InstanceHandle_t *source)
{
    DDS_Liveliness_ParticipantMessageDataKey *sample = (DDS_Liveliness_ParticipantMessageDataKey*)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(source);

    if(!DDS_GuidPrefix_t_cdr_deserialize(
        plugin,
        &sample->participant_guid_prefix,
        stream,NULL))
    {
        return RTI_FALSE;
    }
    if(!DDS_OctetArray4_cdr_deserialize(
        plugin,
        &sample->kind,
        stream,NULL))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

/* --------------------------------------------------------------------------
(De)Serialize functions:
* -------------------------------------------------------------------------- */
RTI_UINT32
DDS_Liveliness_ParticipantMessageData_get_serialized_sample_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;
    UNUSED_ARG(plugin);

    current_alignment += DDS_Liveliness_ParticipantMessageDataKey_get_serialized_sample_size(
        plugin,current_alignment);

    current_alignment += CDR_get_max_size_serialized_primitive_sequence(
        current_alignment, (1), CDR_OCTET_TYPE);

    return  current_alignment - initial_alignment;
}
RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_serialize(struct DDS_TypePlugin *plugin,
struct CDR_Stream_t *stream,
const void *void_sample,
DDS_InstanceHandle_t *destination)
{
    DDS_Liveliness_ParticipantMessageData *sample = (DDS_Liveliness_ParticipantMessageData *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    if(!DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize(
        plugin,
        stream,
        &sample->key,
        NULL)) {
        return RTI_FALSE;
    }
    if (!CDR_Stream_serialize_primitive_sequence(
        stream,
        (const struct REDA_Sequence*) &sample->data,
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_deserialize(struct DDS_TypePlugin *plugin,
void *void_sample,
struct CDR_Stream_t *stream,
DDS_InstanceHandle_t *source)
{
    DDS_Liveliness_ParticipantMessageData *sample = (DDS_Liveliness_ParticipantMessageData*)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(source);

    if(!DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize(
        plugin,
        &sample->key,
        stream,NULL))
    {
        return RTI_FALSE;
    }
    if (!CDR_Stream_deserialize_primitive_sequence(
        stream,
        (struct REDA_Sequence*) &sample->data,
        CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

/* --------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------- */

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_serialize_key(struct DDS_TypePlugin *plugin,
struct CDR_Stream_t *stream,
const void *void_sample,
DDS_InstanceHandle_t *destination)
{
    const DDS_Liveliness_ParticipantMessageData *sample = (DDS_Liveliness_ParticipantMessageData *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);
    if(!DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize_key(
        plugin,
        stream,
        &sample->key,
        NULL)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_deserialize_key(struct DDS_TypePlugin *plugin,
void *void_sample,
struct CDR_Stream_t *stream,
DDS_InstanceHandle_t *source)
{
    DDS_Liveliness_ParticipantMessageData *sample = (DDS_Liveliness_ParticipantMessageData *)void_sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(source);
    if(!DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize_key(
        plugin,
        &sample->key,
        stream,NULL))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_Liveliness_ParticipantMessageData_get_serialized_key_size(
    struct DDS_TypePlugin *plugin,
    RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;

    UNUSED_ARG(plugin);
    current_alignment +=  DDS_Liveliness_ParticipantMessageDataKey_get_serialized_key_size(
        plugin, current_alignment);

    return current_alignment - initial_alignment;
}

/* --------------------------------------------------------------------------
*  Sample Support functions:
* -------------------------------------------------------------------------- */
RTI_BOOL
DDS_Liveliness_ParticipantMessageDataPlugin_create_sample(struct DDS_TypePlugin *plugin,
void **sample)
{
    UNUSED_ARG(plugin);

    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    *sample = (void *) DDS_Liveliness_ParticipantMessageData_create();

    return (*sample != NULL);
}

#ifndef RTI_CERT
RTI_BOOL
DDS_Liveliness_ParticipantMessageDataPlugin_delete_sample(struct DDS_TypePlugin *plugin,
void *sample)
{
    UNUSED_ARG(plugin);

    /* DDS_Liveliness_ParticipantMessageData_delete() is a void function
    *  which expects (sample != NULL). Since
    * DDS_Liveliness_ParticipantMessageDataPlugin_delete_sample
    * is an internal function, sample is assumed to be a valid pointer
    */
    DDS_Liveliness_ParticipantMessageData_delete((DDS_Liveliness_ParticipantMessageData *) sample);

    return RTI_TRUE;
}
#endif

RTI_BOOL
DDS_Liveliness_ParticipantMessageDataPlugin_copy_sample(struct DDS_TypePlugin *plugin,
void *dst,
const void *src)
{
    UNUSED_ARG(plugin);

    return DDS_Liveliness_ParticipantMessageData_copy(
        (DDS_Liveliness_ParticipantMessageData*)dst,
        (const DDS_Liveliness_ParticipantMessageData*)src);
}

/* --------------------------------------------------------------------------
*  Type DDS_Liveliness_ParticipantMessageData Plugin Instantiation
* -------------------------------------------------------------------------- */

NDDSCDREncapsulation DDS_Liveliness_ParticipantMessageDataEncapsulationKind[] =
{
    {
        DDS_ENCAPSULATION_ID_CDR_LE,
        DDS_ENCAPSULATION_ID_CDR_BE,
        0
    }
};

RTI_PRIVATE RTI_UINT32
DDS_Liveliness_ParticipantMessageDataCdrPlugin_get_serialized_sample_size(
    struct DDS_TypePlugin *plugin,
    struct DDS_TypeEncapsulationPlugin *ep,
    RTI_UINT32 current_alignment)
{
    UNUSED_ARG(ep);

    return DDS_Liveliness_ParticipantMessageData_get_serialized_sample_size(plugin,
    current_alignment);
}

RTI_PRIVATE struct DDS_TypeEncapsulationPlugin*
DDS_Liveliness_ParticipantMessageDataCdrPlugin_create(struct DDS_TypePlugin *tp,
DDS_DomainParticipant *participant,
struct DDS_DomainParticipantQos *dp_qos,
DDS_TypePluginMode_T endpoint_mode,
DDS_TypePluginEndpoint *endpoint,
DDS_TypePluginEndpointQos *qos,
struct DDS_TypeMemoryPlugin *mp)
{
    RTI_UINT32 size =
      DDS_Liveliness_ParticipantMessageData_get_serialized_sample_size(tp,0);

    return DDS_TypePluginDefaultCdr_create(tp,participant,dp_qos,
                                        endpoint_mode,endpoint,qos,mp,size);
}

RTI_PRIVATE void
DDS_Liveliness_ParticipantMessageDataCdrPlugin_delete(struct DDS_TypePlugin *p,
struct DDS_TypeEncapsulationPlugin *ep)
{
    UNUSED_ARG(p);
    UNUSED_ARG(ep);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_initialize(void *init_config, void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)init_config;
    void *sample;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!DDS_Liveliness_ParticipantMessageDataPlugin_create_sample(&plugin->_parent,&sample))
    {
        return RTI_FALSE;
    }

    sh->sample = sample;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_Liveliness_ParticipantMessageData_cdr_finalize(void *finalize_config, void *buffer)
{
#ifndef RTI_CERT
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)finalize_config;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!DDS_Liveliness_ParticipantMessageDataPlugin_delete_sample(&plugin->_parent,sh->sample))
    {
        return RTI_FALSE;
    }
#else
    UNUSED_ARG(finalize_config);
    UNUSED_ARG(buffer);
#endif
    return RTI_TRUE;
}

RTI_PRIVATE struct DDS_TypeMemoryPlugin*
DDS_Liveliness_ParticipantMessageDataHeapPlugin_create(struct DDS_TypePlugin *tp,
DDS_DomainParticipant *participant,
struct DDS_DomainParticipantQos *dp_qos,
DDS_TypePluginMode_T endpoint_mode,
DDS_TypePluginEndpoint *endpoint,
DDS_TypePluginEndpointQos *qos)
{
    return DDS_TypePluginDefaultHeap_create(tp,participant,dp_qos,
    endpoint_mode,endpoint,qos,
    DDS_Liveliness_ParticipantMessageData_cdr_initialize,
    DDS_Liveliness_ParticipantMessageData_cdr_finalize);
}

RTI_PRIVATE void
DDS_Liveliness_ParticipantMessageDataHeapPlugin_delete(struct DDS_TypePlugin *p,
struct DDS_TypeMemoryPlugin *mp)
{
    UNUSED_ARG(p);
    UNUSED_ARG(mp);
}

RTI_PRIVATE struct DDS_TypeEncapsulationI DDS_Liveliness_ParticipantMessageData_fv_CdrPluginI =
{
    DDS_XCDR_DATA_REPRESENTATION,
    NULL,
    DDS_Liveliness_ParticipantMessageDataEncapsulationKind,
    RTI_MEMORY_TYPE_HEAP,
    RTI_MEMORY_MANAGER_HEAP,
    NULL,
    NULL,
    DDS_TypePluginDefaultCdr_get_buffer,
    DDS_TypePluginDefaultCdr_return_buffer,
    DDS_TypePluginDefaultCdr_get_sample,
    DDS_TypePluginDefaultCdr_return_sample,
    DDS_Liveliness_ParticipantMessageData_cdr_serialize,
    DDS_Liveliness_ParticipantMessageData_cdr_deserialize,
    DDS_Liveliness_ParticipantMessageDataCdrPlugin_get_serialized_sample_size,
    DDS_Liveliness_ParticipantMessageDataCdrPlugin_create,
    DDS_Liveliness_ParticipantMessageDataCdrPlugin_delete
};

/*    &NETIO_gv_TypeShmStreamPluginI, */

RTI_PRIVATE struct DDS_TypeEncapsulationI *DDS_Liveliness_ParticipantMessageData_fv_WirePlugins[] =
{
    &DDS_Liveliness_ParticipantMessageData_fv_CdrPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypeMemoryI DDS_Liveliness_ParticipantMessageData_fv_HeapPluginI =
{
    RTI_MEMORY_MANAGER_HEAP,
    RTI_MEMORY_TYPE_HEAP,
    DDS_Liveliness_ParticipantMessageDataPlugin_create_sample,
    #ifndef RTI_CERT
    DDS_Liveliness_ParticipantMessageDataPlugin_delete_sample,
    #else
    NULL,
    #endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DDS_Liveliness_ParticipantMessageDataHeapPlugin_create,
    DDS_Liveliness_ParticipantMessageDataHeapPlugin_delete
};

RTI_PRIVATE struct DDS_TypeMemoryI *DDS_Liveliness_ParticipantMessageData_fv_MemoryPlugins[] =
{
    &DDS_Liveliness_ParticipantMessageData_fv_HeapPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypePlugin*
DDS_Liveliness_ParticipantMessageDataTypePlugin_create_plugin(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_TypePluginMode_T endpoint_mode,
    DDS_TypePluginEndpoint *endpoint,
    DDS_TypePluginEndpointQos *qos,
    struct DDS_TypePluginProperty *const property);
RTI_PRIVATE RTI_BOOL
DDS_Liveliness_ParticipantMessageDataTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin);

RTI_PRIVATE struct DDS_TypePluginI DDS_Liveliness_ParticipantMessageData_fv_TypePluginI =
{
    /**************************************************************************
    *                   Type information functions
    **************************************************************************/

    NULL,                       /* DDS_TypeCode_t* */
    NDDS_TYPEPLUGIN_USER_KEY,   /* NDDS_TypePluginKeyKind */
    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,
    NULL,
    RTI_MEMORY_TYPE_HEAP,
    PluginHelper_instance_to_keyhash,
    DDS_Liveliness_ParticipantMessageDataPlugin_copy_sample,
    NULL,
    DDS_Liveliness_ParticipantMessageData_cdr_serialize_key,
    DDS_Liveliness_ParticipantMessageData_cdr_deserialize_key,
    DDS_Liveliness_ParticipantMessageData_get_serialized_key_size,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DDS_Liveliness_ParticipantMessageData_fv_MemoryPlugins,
    DDS_Liveliness_ParticipantMessageData_fv_WirePlugins,

    /**************************************************************************
    *       Helper APIs to create language binding wrapper Functions
    **************************************************************************/

    NULL, NULL, NULL, NULL,  /* endpoint wrappers not used in C */
    /**************************************************************************
     *       Life-cycle
     **************************************************************************/
    DDS_Liveliness_ParticipantMessageDataTypePlugin_create_plugin,
    DDS_Liveliness_ParticipantMessageDataTypePlugin_delete_plugin,
    NULL,
    NULL
    DDS_TypePluginI_XTYPES_INITIALIZER
};

/* --------------------------------------------------------------------------
*  Type DDS_Liveliness_ParticipantMessageData Plugin Methods
* -------------------------------------------------------------------------- */

struct DDS_TypePluginI*
DDS_Liveliness_ParticipantMessageDataTypePlugin_get(void)
{
    return &DDS_Liveliness_ParticipantMessageData_fv_TypePluginI;
}

RTI_PRIVATE struct DDS_TypePlugin*
DDS_Liveliness_ParticipantMessageDataTypePlugin_create_plugin(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_TypePluginMode_T endpoint_mode,
    DDS_TypePluginEndpoint *endpoint,
    DDS_TypePluginEndpointQos *qos,
    struct DDS_TypePluginProperty *const property)
{
    return DDS_TypePluginDefault_create(&DDS_Liveliness_ParticipantMessageData_fv_TypePluginI,
    participant,dp_qos,
    endpoint_mode,endpoint,qos,
    property);
}

RTI_PRIVATE RTI_BOOL
DDS_Liveliness_ParticipantMessageDataTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin)
{
    return DDS_TypePluginDefault_delete(plugin);
}

struct DDS_TypePlugin*
DDS_Liveliness_ParticipantMessageDataWriterTypePlugin_create(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_DataWriter *writer,
    struct DDS_DataWriterQos *qos,
    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
        &DDS_Liveliness_ParticipantMessageData_fv_TypePluginI,
        participant,dp_qos,
        DDS_TYPEPLUGIN_MODE_WRITER,
        (DDS_TypePluginEndpoint*)writer,
        (DDS_TypePluginEndpointQos*)qos,
        property);
}

struct DDS_TypePlugin*
DDS_Liveliness_ParticipantMessageDataReaderTypePlugin_create(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_DataReader *reader,
    struct DDS_DataReaderQos *qos,
    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
        &DDS_Liveliness_ParticipantMessageData_fv_TypePluginI,
        participant,dp_qos,
        DDS_TYPEPLUGIN_MODE_READER,
        (DDS_TypePluginEndpoint*)reader,
        (DDS_TypePluginEndpointQos*)qos,
        property);
}

const char*
DDS_Liveliness_ParticipantMessageDataTypePlugin_get_default_type_name(void)
{
    return DDS_Liveliness_ParticipantMessageDataTYPENAME;
}

