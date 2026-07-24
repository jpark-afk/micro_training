/*
 * FILE: ParticipantBuiltinTopicDataPlugin.c -
 *                                      ParticipantBuiltinTopicDataPlugin API
 *
 * Copyright (c) 2011-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 30mar2015,as Replaced references to DDS_Boolean (and its values) with
 *              RTI_BOOL.
 * 25Mar2015,as MICRO-908: Removed unused PluginHelper functions
 * 26jun2014,eh MICRO-812: ignore deserialized RTPS_PID_PROPERTY_LIST
 * 20may2014,eh MICRO-700: Unknown RTPS parameter as incompatible Qos
 * 07mar2014,tk MICRO-735: Send properties for tools
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief ParticipantBuiltinTopicDataPlugin API
 */
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "BuiltinCdr.h"
#include "BuiltinTopicData.h"
#include "UserDataQosPolicy.h"
#include "DomainParticipantTrust.h"

#if DDS_LIVELINESS_CHANNEL_ENABLED
#include "BuiltinIpcCdr.h"
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */


/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
                                        struct NDDS_Type_Plugin *plugin,
                                        DDS_UnsignedLong size)
{
    DDS_UnsignedLong orig_sz = size;
    struct DDS_LocatorSeq locator = DDS_SEQUENCE_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos =
            DDS_DomainParticipant_get_qos_ref(DDS_TypePlugin_get_participant(plugin));

    /* No need to allocate the sequence */
    locator._maximum = RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX;

    /*
     * Call get max size serialized on each parameter header and field
     * according to the type.
     * We assume there is a logical reset before serializing each value.
     * getParameterHeaderMaxSizeSerialize() take care of 4-byte alignment.
     * Sentinel parameter size is not included in the calculation.
     */
    size += DDS_CdrQosPolicy_get_max_size_serialized_key(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_entity_name(size);

    /* Builtin endpoint mask */
    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_unsigned_long(0);

    /* promisciuity endpoint mask */
    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_unsigned_long(0);

    /*  RTPS Protocol Version */
    size += DDS_CdrQosPolicy_get_max_size_serialized_protocol_version(size);

    /* RTPS Vendor Id */
    size += DDS_CdrQosPolicy_get_max_size_serialized_vendor_id(size);

    /* default uni-cast locators */
    size += DDS_CdrQosPolicy_get_max_size_serialized_locator(&locator, size);

    /* default multicast locators
     * Twice for user multicast because both the old and new PID is sent
     */
    size += 2 * DDS_CdrQosPolicy_get_max_size_serialized_locator(&locator, size);

    /* default meta-traffic unicast locator */
    size += DDS_CdrQosPolicy_get_max_size_serialized_locator(&locator, size);

    /* default meta-traffic multicast locator */
    size += DDS_CdrQosPolicy_get_max_size_serialized_locator(&locator, size);

    /* Lease duration */
    size += DDS_CdrQosPolicy_get_max_size_serialized_lease_duration(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_product_version(size);

    size += DDS_CdrQosPolicy_getChecksumPropertyMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_property(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_user_data(
                size, (RTI_UINT32)dp_qos->resource_limits.participant_user_data_max_length);

#if DDS_LIVELINESS_CHANNEL_ENABLED
    size +=
    DDS_CdrQosPolicy_get_max_size_serialized_builtin_endpoint_qos(size);
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    /* trust specific paramters */
    size += DDS_DomainParticipant_get_max_serialized_trust_param_size(
            DDS_TypePlugin_get_participant(plugin), size);

    /* sentinel parameter header */
    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);

    return (size - orig_sz);
}

MUST_CHECK_RETURN RTI_UINT32
DDS_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_size(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            RTI_UINT32 current_alignment)
{
    UNUSED_ARG(current_alignment);
    UNUSED_ARG(sample);

    return DDS_Cdr_get_header_max_size_serialized(0) +
            DPDE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
            plugin, 0);
}

MUST_CHECK_RETURN RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_serialize(struct DDS_TypePlugin *plugin,
                                                     struct CDR_Stream_t *stream,
                                                     const void *data,
                                                     DDS_InstanceHandle_t *destination)
{
    DDS_UnsignedShort zeroLength = 0;
    DDS_UnsignedShort sentinel;
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                                (struct DDS_ParticipantBuiltinTopicData*)data;

    UNUSED_ARG(destination);
    UNUSED_ARG(plugin);

    if (!DDS_CdrQosPolicy_serialize_key(stream, &topic_data->key,
                                       RTPS_PID_PARTICIPANT_GUID))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_entity_name(
        stream, &topic_data->participant_name, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrStream_serialize_4_byte_parameter(
            stream, &topic_data->dds_builtin_endpoints,
            RTPS_PID_BUILTIN_ENDPOINT_MASK))
    {
        DDSC_LOG_SERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_protocol_version(
            stream, &topic_data->rtps_protocol_version, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_vendor_id(
            stream, &topic_data->rtps_vendor_id, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_locator_sequence(
            stream, &topic_data->default_unicast_locators,
            RTPS_PID_DEFAULT_UNICAST_LOCATOR6,
            RTPS_PID_DEFAULT_UNICAST_LOCATOR6,
            NULL,NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_locator_sequence(
                        stream, &topic_data->default_multicast_locators,
                        RTPS_PID_DEFAULT_MULTICAST_LOCATOR6,
                        RTPS_PID_DEFAULT_MULTICAST_LOCATOR6,
                        NULL,NULL))
    {
        return RTI_FALSE;
    }

   /* Micro 2 and Micro 3 (incorrectly) expects RTPS_PID_MULTICAST_LOCATOR6
    * for the default user traffic locators. Thus, it must still be sent.
    */
    if (!DDS_CdrQosPolicy_serialize_locator_sequence(
            stream, &topic_data->default_multicast_locators,
            RTPS_PID_MULTICAST_LOCATOR6,
            RTPS_PID_MULTICAST_LOCATOR6,
            NULL,NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_locator_sequence(
            stream, &topic_data->metatraffic_unicast_locators,
            RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6,
            RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6,
            NULL,NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_locator_sequence(
            stream, &topic_data->metatraffic_multicast_locators,
            RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6,
            RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6,
            NULL,NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_lease_duration(
            stream, &topic_data->liveliness_lease_duration, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_product_version(
            stream, &topic_data->product_version, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeChecksumProperty(
            stream, &topic_data->checksum, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_property(stream, &topic_data->property))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_user_data(stream, &topic_data->user_data))
    {
        return RTI_FALSE;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (!DDS_CdrQosPolicy_serialize_builtin_endpoint_qos(
        stream, &topic_data->builtin_endpoint_qos_mask, NULL))
    {
        return RTI_FALSE;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */


    if (!DDS_DomainParticipant_serialize_trust_param(DDS_TypePlugin_get_participant(plugin), stream))
    {
        return RTI_FALSE;
    }

    sentinel = RTPS_PID_SENTINEL;
    if (RTI_TRUE != CDR_Stream_serialize_unsigned_short(stream, &sentinel))
    {
        return RTI_FALSE;
    }

    if (RTI_TRUE != CDR_Stream_serialize_unsigned_short(stream, &zeroLength))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

RTI_PRIVATE void
DPDE_BuiltinTopicParticipantDataPluginSupport_set_dflt_pv(
                        struct DDS_ParticipantBuiltinTopicData *topic_data)
{
    struct DDS_ParticipantBuiltinTopicData dflt_data =
            DDS_ParticipantBuiltinTopicData_INITIALIZER;

    topic_data->key = dflt_data.key;
    topic_data->participant_name = dflt_data.participant_name;
    topic_data->dds_builtin_endpoints = dflt_data.dds_builtin_endpoints;
    topic_data->rtps_protocol_version = dflt_data.rtps_protocol_version;
    topic_data->rtps_vendor_id = dflt_data.rtps_vendor_id;

    DDS_LocatorSeq_set_length(&topic_data->default_unicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->metatraffic_unicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->metatraffic_multicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->default_multicast_locators, 0);

    topic_data->liveliness_lease_duration = dflt_data.liveliness_lease_duration;
    topic_data->product_version = dflt_data.product_version;

    DDS_PropertySeq_set_length(&topic_data->property.value, 0);
    DDS_OctetSeq_set_length(&topic_data->user_data.value, 0);

#if DDS_LIVELINESS_CHANNEL_ENABLED
    topic_data->participant_message_reader_reliability_kind =
            dflt_data.participant_message_reader_reliability_kind;
    topic_data->builtin_endpoint_qos_mask = dflt_data.builtin_endpoint_qos_mask;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
}

/*
  Returns RTI_TRUE if there is a match in parameterId. Still need to set the
  ok to indicate if value is deserialized correctly.
*/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicData_deserialize_pv(RTI_BOOL *ok,
                            struct DDS_ParticipantBuiltinTopicData *topic_data,
                            struct CDR_Stream_t *stream,
                            DDS_UnsignedShort param_id,
                            DDS_UnsignedShort param_length,
                            void *param)
{
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*)param;

    UNUSED_ARG(param_length);
    *ok = RTI_TRUE;

    switch (param_id)
    {
        case RTPS_PID_PARTICIPANT_GUID:
            *ok = DDS_CdrQosPolicy_deserialize_key(
                stream, &topic_data->key, NULL);
            break;
        case RTPS_PID_BUILTIN_ENDPOINT_MASK:
            if (!CDR_Stream_deserialize_unsigned_long(
                    stream, &topic_data->dds_builtin_endpoints))
            {
                DDSC_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
                *ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_PROTOCOL_VERSION:
            *ok = DDS_CdrQosPolicy_deserialize_protocol_version(
                    stream, &topic_data->rtps_protocol_version, NULL);
            break;
        case RTPS_PID_VENDOR_ID:
            *ok = DDS_CdrQosPolicy_deserialize_vendor_id(
                    stream, &topic_data->rtps_vendor_id, NULL);
            break;
        case RTPS_PID_DEFAULT_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                    stream, &topic_data->default_unicast_locators,param_id,param);
            break;
        case RTPS_PID_DEFAULT_MULTICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                    stream, &topic_data->default_multicast_locators,param_id,param);
            break;
        case RTPS_PID_MULTICAST_LOCATOR6:
            /* Micro always sends the vendor ID first, so the vendor ID is
             * available to be checked
             */
            if ((topic_data->rtps_vendor_id.vendorId[0] == RTPS_VENDOR_ID_MAJOR) &&
                (topic_data->rtps_vendor_id.vendorId[1] == RTPS_VENDOR_ID_MINOR))
            {
                *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                            stream, &topic_data->default_multicast_locators,
                            param_id,param);
            }
            break;
        case RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                    stream, &topic_data->metatraffic_unicast_locators,param_id,param);
            break;
        case RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                    stream, &topic_data->metatraffic_multicast_locators,param_id,param);
            break;
        case RTPS_PID_LEASE_DURATION:
            *ok = DDS_CdrQosPolicy_deserialize_lease_duration(
                    stream, &topic_data->liveliness_lease_duration, NULL);
            break;
        case DISC_RTPS_PID_PRODUCT_VERSION:
            *ok = DDS_CdrQosPolicy_deserialize_product_version(
                    stream, &topic_data->product_version, NULL);
            break;
        case DISC_RTPS_PID_ENTITY_NAME:
            *ok = DDS_CdrQosPolicy_deserialize_entity_name(
                    stream, &topic_data->participant_name, NULL);
            break;
        case RTPS_PID_USER_DATA:
            *ok = DDS_CdrQosPolicy_deserialize_user_data(
                        stream, &topic_data->user_data,
                        DDS_DomainParticipant_get_user_data_manager(dp),
                        DDS_USER_DATA_PARTICIPANT_TYPE);
            break;
        case RTPS_PID_PROPERTY_LIST:
            /* Micro sends DDS_ParticipantBuiltinTopicData containing
             * RTPS_PID_PROPERTY_LIST, but does not use any of its
             * current properties if security is not enabled.
             * Thus, ignore this parameter quietly.
             */
            break;
        case RTPS_PID_CHECKSUM_PROPERTY:
            if (CDR_Stream_is_vendor_rti(stream))
            {
                *ok = DDS_CdrQosPolicy_deserializeChecksumProperty(
                                stream, &topic_data->checksum, NULL);
            }
            else
            {
               *ok = RTI_TRUE;
            }
            break;
#if DDS_LIVELINESS_CHANNEL_ENABLED
        case RTPS_PID_BUILTIN_ENDPOINT_QOS:
            *ok = DDS_CdrQosPolicy_deserialize_builtin_endpoint_qos(
                    stream, &topic_data->builtin_endpoint_qos_mask, NULL);
            topic_data->builtin_endpoint_qos_mask |=
                                         DDS_BUILTIN_ENDPOINT_QOS_BIT_IS_VALID;
            break;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
        default:
            /* Unknown parameter is either ignored or treated as
               incompatible QoS */
            if ((param_id & RTPS_PID_INCOMPATIBLE_MASK) != 0)
            {
                *ok = RTI_FALSE;
                DDSC_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)param_id,
                                                 (RTI_INT32)param_length)
            }
#if OSAPI_ENABLE_LOG
            else
            {
                DDSC_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_INFO,
                                                 (RTI_INT32)param_id,
                                                 (RTI_INT32)param_length)
            }
#endif
            return *ok;
    }

    return *ok;
}

MUST_CHECK_RETURN RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_create_sample(
                                                struct NDDS_Type_Plugin* plugin,
                                                void **sample)
{
    struct DDS_ParticipantBuiltinTopicData *participant_builtin_data = NULL;

    OSAPI_Heap_allocate_struct(sample, struct DDS_ParticipantBuiltinTopicData);

    if (*sample == NULL)
    {
        return RTI_FALSE;
    }

    participant_builtin_data = (*sample);

    if (!DDS_ParticipantBuiltinTopicData_initialize_shallow(
            participant_builtin_data,
            DDS_DomainParticipant_get_qos_ref(DDS_TypePlugin_get_participant(plugin))))
    {
        OSAPI_Heap_free_struct(*sample);
        *sample = NULL;
        return RTI_FALSE;
    }

    participant_builtin_data->dds_builtin_endpoints =
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_delete_sample(
                                            struct DDS_TypePlugin *plugin,
                                            void *sample)
{
#ifdef RTI_CERT
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
#else

    if (!DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(
            (struct DDS_ParticipantBuiltinTopicData*)sample,
            DDS_TypePlugin_get_participant(plugin)))
    {
        return RTI_FALSE;
    }

    if (!DDS_ParticipantBuiltinTopicData_finalize(
                            (struct DDS_ParticipantBuiltinTopicData*)sample))
    {
        return RTI_FALSE;
    }
    OSAPI_Heap_free_struct((void*)sample);
#endif /* RTI_CERT */
    return RTI_TRUE;
}

RTI_PRIVATE void
DPDE_ParticipantBuiltinTopicDataTypePlugin_return_sample(struct DDS_TypePlugin *tp,
                                      struct DDS_TypePluginSampleHolder *sample)
{
    DDS_DomainParticipant *participant = DDS_TypePlugin_get_participant(tp);
    struct DDS_ParticipantBuiltinTopicData *data =
            (struct DDS_ParticipantBuiltinTopicData *)sample->sample;

    DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(data, participant);

    DDS_TypePluginDefaultCdr_return_sample(tp, sample);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_copy_sample(
                                                struct NDDS_Type_Plugin *type,
                                                void *dst,
                                                const void *src)
{
    UNUSED_ARG(type);
    return (DDS_ParticipantBuiltinTopicData_copy(
                            (struct DDS_ParticipantBuiltinTopicData *)dst,
                            (struct DDS_ParticipantBuiltinTopicData *)src));
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_deserialize(struct DDS_TypePlugin *plugin,
                                       void *sample,
                                       struct CDR_Stream_t *stream,
                                       DDS_InstanceHandle_t *source)

{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                        (struct DDS_ParticipantBuiltinTopicData *)sample;
    UNUSED_ARG(source);

    /* If the stream was protected with a legacy RTI CRC32 checksum
     * assume that the sender does not require a checksum and will
     * accept any checksum. However, if a checksum property is received it
     * takes precedence.
     */
    if (CDR_Stream_is_checksum_crc32(stream) &&
        CDR_Stream_is_vendor_rti(stream))
    {
        topic_data->checksum.computed_crc_kind = DDS_CHECKSUM_BUILTIN32;
        topic_data->checksum.allowed_crc_mask = DDS_CHECKSUM_NONE;
        topic_data->checksum.require_crc = RTI_FALSE;
    }
    else
    {
        OSAPI_Memory_zero(&topic_data->checksum,sizeof(topic_data->checksum));
    }

    return DDS_CdrStream_deserialize_parameter_sequence(topic_data, stream,
              (DDS_Cdr_SetDefaultParameterValuesFunction)
              DPDE_BuiltinTopicParticipantDataPluginSupport_set_dflt_pv,
              (DDS_Cdr_DeserializeParameterValueFunction)
              DPDE_ParticipantBuiltinTopicData_deserialize_pv,
              plugin->property.plugin_param);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_serializeKey(
                                        struct DDS_TypePlugin *plugin,
                                        struct CDR_Stream_t *stream,
                                        const void *sample_key,
                                        DDS_InstanceHandle_t *destination)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_deserializeKey(
                                        struct DDS_TypePlugin *plugin,
                                        UserDataKeyHolder_t sample_key,
                                        struct CDR_Stream_t *stream,
                                        DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(plugin);
    UNUSED_ARG(source);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_instanceToKeyHash(
                                            struct NDDS_Type_Plugin *plugin,
                                            struct CDR_Stream_t *stream,
                                            DDS_KeyHash_t *keyHash,
                                            const void *instance,
                                            DDS_EncapsulationId_t id)
{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                        (struct DDS_ParticipantBuiltinTopicData *)instance;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(id);

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid*)&topic_data->key);
    OSAPI_Memory_copy(keyHash->value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);

    return RTI_TRUE;

}

MUST_CHECK_RETURN RTI_PRIVATE  RTI_UINT32
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);

    return RTPS_KEY_HASH_MAX_LENGTH;
}

RTI_PRIVATE struct DDS_TypePluginI DPDE_fv_ParticipantBuiltinTopicDataTypePluginI;


RTI_PRIVATE struct DDS_TypeEncapsulationPlugin*
DPDE_ParticipantBuiltinTopicDataCdrPlugin_create(
                                       struct DDS_TypePlugin *tp,
                                       DDS_DomainParticipant *participant,
                                       struct DDS_DomainParticipantQos *dp_qos,
                                       DDS_TypePluginMode_T endpoint_mode,
                                       DDS_TypePluginEndpoint *endpoint,
                                       DDS_TypePluginEndpointQos *qos,
                                       struct DDS_TypeMemoryPlugin *mp)
{
    RTI_UINT32 size = DDS_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_size(tp,NULL,0);

    return DDS_TypePluginDefaultCdr_create(tp,participant,dp_qos,
                                           endpoint_mode,endpoint,qos,mp,size);
}

RTI_PRIVATE void
DPDE_ParticipantBuiltinTopicDataCdrPlugin_delete(struct DDS_TypePlugin *p,
                                         struct DDS_TypeEncapsulationPlugin *mp)
{
    UNUSED_ARG(p);
    UNUSED_ARG(mp);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReader_cdr_initialize(void *init_config, void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault *)init_config;
    void *sample;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!DDS_ParticipantBuiltinTopicDataTypePlugin_create_sample(&plugin->_parent,&sample))
    {
        return RTI_FALSE;
    }

    sh->sample = sample;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReader_cdr_finalize(void *finalize_config, void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault *)finalize_config;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!DDS_ParticipantBuiltinTopicDataTypePlugin_delete_sample(&plugin->_parent,sh->sample))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE struct DDS_TypeMemoryPlugin*
DPDE_ParticipantBuiltinTopicDataHeapPlugin_create(
                                 struct DDS_TypePlugin *tp,
                                 DDS_DomainParticipant *participant,
                                 struct DDS_DomainParticipantQos *dp_qos,
                                 DDS_TypePluginMode_T endpoint_mode,
                                 DDS_TypePluginEndpoint *endpoint,
                                 DDS_TypePluginEndpointQos *qos)
{
    return DDS_TypePluginDefaultHeap_create(tp,participant,dp_qos,
                                           endpoint_mode,endpoint,qos,
                                           DDS_DataReader_cdr_initialize,
                                           DDS_DataReader_cdr_finalize);
}

RTI_PRIVATE void
DPDE_ParticipantBuiltinTopicDataHeapPlugin_delete(struct DDS_TypePlugin *p,
                                                  struct DDS_TypeMemoryPlugin *mp)
{
    UNUSED_ARG(p);
    UNUSED_ARG(mp);
}

RTI_PRIVATE struct DDS_TypePlugin*
DPDE_ParticipantBuiltinTopicDataTypePlugin_create_plugin(
                        DDS_DomainParticipant *participant,
                        struct DDS_DomainParticipantQos *dp_qos,
                        DDS_TypePluginMode_T endpoint_mode,
                        DDS_TypePluginEndpoint *endpoint,
                        DDS_TypePluginEndpointQos *qos,
                        struct DDS_TypePluginProperty *const property)
{
    return DDS_TypePluginDefault_create(&DPDE_fv_ParticipantBuiltinTopicDataTypePluginI,
                                       participant,dp_qos,
                                       endpoint_mode,endpoint,qos,
                                       property);
}

RTI_PRIVATE SHOULD_CHECK_RETURN RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin)
{
    return DDS_TypePluginDefault_delete(plugin);
}

RTI_PRIVATE RTI_UINT32
DPDE_ParticipantBuiltinTopicDataTypeCdrPlugin_get_serialized_sample_size(
                                       struct DDS_TypePlugin *tp,
                                       struct DDS_TypeEncapsulationPlugin *wp,
                                       RTI_UINT32 alignment)
{
    UNUSED_ARG(wp);

    return DDS_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_size(tp,NULL,alignment);
}

RTI_PRIVATE NDDSCDREncapsulation DPDE_fv_ParticipantBuiltinTopicDataEncapsulationKind[] =
{
    {
        DDS_ENCAPSULATION_ID_PL_CDR_LE,
        DDS_ENCAPSULATION_ID_PL_CDR_BE,
        0
    }
};

RTI_PRIVATE struct DDS_TypeMemoryI DPDE_fv_ParticipantBuiltinTopicDataHeapPluginI =
{
    RTI_MEMORY_MANAGER_HEAP,
    RTI_MEMORY_TYPE_HEAP,
    DDS_ParticipantBuiltinTopicDataTypePlugin_create_sample,
    DDS_ParticipantBuiltinTopicDataTypePlugin_delete_sample,
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
    DPDE_ParticipantBuiltinTopicDataHeapPlugin_create, /* create reader */
    DPDE_ParticipantBuiltinTopicDataHeapPlugin_delete  /* delete_plugin */
};

RTI_PRIVATE struct DDS_TypeMemoryI *DPDE_fv_ParticipantBuiltinTopicDataMemoryPlugins[] =
{
    &DPDE_fv_ParticipantBuiltinTopicDataHeapPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypeEncapsulationI DPDE_fv_ParticipantBuiltinTopicDataCdrPluginI =
{
    DDS_XCDR_DATA_REPRESENTATION,
    NULL,
    DPDE_fv_ParticipantBuiltinTopicDataEncapsulationKind,
    RTI_MEMORY_TYPE_HEAP,
    RTI_MEMORY_MANAGER_HEAP,
    NULL,
    NULL,
    DDS_TypePluginDefaultCdr_get_buffer,
    DDS_TypePluginDefaultCdr_return_buffer,
    DDS_TypePluginDefaultCdr_get_sample,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_return_sample,
    DDS_ParticipantBuiltinTopicDataTypePlugin_serialize,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_deserialize,
    DPDE_ParticipantBuiltinTopicDataTypeCdrPlugin_get_serialized_sample_size,
    DPDE_ParticipantBuiltinTopicDataCdrPlugin_create, /* create_plugin */
    DPDE_ParticipantBuiltinTopicDataCdrPlugin_delete /* delete_plugin */
};

RTI_PRIVATE struct DDS_TypeEncapsulationI *DPDE_fv_ParticipantBuiltinTopicDataWirePlugins[] =
{
    &DPDE_fv_ParticipantBuiltinTopicDataCdrPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypePluginI DPDE_fv_ParticipantBuiltinTopicDataTypePluginI =
{
    /**************************************************************************
     *                   Type information functions
     **************************************************************************/

    NULL,
    NDDS_TYPEPLUGIN_GUID_KEY,
    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,
    NULL, /* Currently not used */
    RTI_MEMORY_TYPE_HEAP,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_instanceToKeyHash,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_copy_sample,
    NULL,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_serializeKey,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_deserializeKey,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DPDE_fv_ParticipantBuiltinTopicDataMemoryPlugins,
    DPDE_fv_ParticipantBuiltinTopicDataWirePlugins,

    /**************************************************************************
     *       Helper APIs to create language binding wrapper Functions
     **************************************************************************/

    NULL,
    NULL,
    NULL,
    NULL,

    /**************************************************************************
     *       Life-cycle
     **************************************************************************/
    DPDE_ParticipantBuiltinTopicDataTypePlugin_create_plugin,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_delete_plugin,
    NULL,
    NULL
    DDS_TypePluginI_XTYPES_INITIALIZER
};

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant)
{
    return DDS_DomainParticipant_register_type(participant,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,
                                &DPDE_fv_ParticipantBuiltinTopicDataTypePluginI);
}

struct DDS_TypePlugin*
DDS_ParticipantBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
                        &DPDE_fv_ParticipantBuiltinTopicDataTypePluginI,
                        participant,dp_qos,
                        endpoint_mode,endpoint,qos,
                        property);
}

MUST_CHECK_RETURN struct DDS_TypePluginI*
DDS_ParticipantBuiltinTopicDataTypePlugin_get(void)
{
    return &DPDE_fv_ParticipantBuiltinTopicDataTypePluginI;
}
