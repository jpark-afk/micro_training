/*
 * FILE: ParticipantBuiltinTopicDataPlugin.c -
 *                                      ParticipantBuiltinTopicDataPlugin API
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
 *
 * All rights reserved.
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
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef disc_dpde_dpdediscovery_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "ParticipantBuiltinTopicDataPlugin.h"
#include "DiscoveryPlugin.h"
#include "BuiltinCdr.h"

/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
                                        DDS_UnsignedLong size,
                                        struct DPDE_DiscoveryPlugin *disc_plugin)
{
    DDS_UnsignedLong orig_sz = size;
    RTI_INT32 i;
    struct DDS_Property *a_property;
    RTI_SIZE_T s_len;

    /*
     * Call get max size serialized on each parameter header and field
     * according to the type.
     * We assume there is a logical reset before serializing each value.
     * getParameterHeaderMaxSizeSerialize() take care of 4-byte alignment.
     * Sentinel parameter size is not included in the calculation.
     */
    size += DDS_CdrQosPolicy_getKeyMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getEntityNameQosPolicyMaxSizeSerialized(size);

    /* Builtin endpoint mask */
    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_unsigned_long(0);

    /* promisciuity endpoint mask */
    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_unsigned_long(0);

    /*  RTPS Protocol Version */
    size += DDS_CdrQosPolicy_getProtocolVersionMaxSerializedSize(size);

    /* RTPS Vendor Id */
    size += DDS_CdrQosPolicy_getVendorIdMaxSerializedSize(size);

    /* User Unicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
        size += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* User Multicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
        size += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Meta Unicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
        size += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Meta Multicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
        size += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Lease duration */
    size += DDS_CdrQosPolicy_getLeaseDurationMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getProductVersionMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getChecksumPropertyMaxSerializedSize(size);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    /* NOTE: Participant properties are sent once when the participant is
     * created. Thus, it is ok to calculate exact space needed to serialize
     * only current/existing properties.
     */
    if (disc_plugin)
    {
        for (i = 0; i < DDS_PropertySeq_get_length(disc_plugin->dds_properties); ++i)
        {
            a_property = DDS_PropertySeq_get_reference(disc_plugin->dds_properties,i);
            if (a_property == NULL)
            {
                continue;
            }

            s_len = REDA_String_length(a_property->name) + 1;
            size += CDR_get_max_size_serialized_string(size, s_len);

            size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

            s_len = REDA_String_length(a_property->value) + 1;
            size += CDR_get_max_size_serialized_string(size, s_len);

            size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);
        }
    }

    /* sentinel parameter header */
    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);

    return (size - orig_sz);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_UINT32
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(
                                struct NDDS_Type_Plugin *plugin,
                                RTI_UINT32 current_alignment,
                                void *param)
{
    struct DPDE_DiscoveryPlugin *disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)param;
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);

    return DDS_CdrQosPolicy_get_header_max_size_serialized(0) +
            DPDE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
            0,disc_plugin);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_serialize(
                                                    struct CDR_Stream_t *stream,
                                                    const void *data,
                                                    void *param)
{
    DDS_UnsignedShort zeroLength = 0;
    DDS_UnsignedShort sentinel;
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                                (struct DDS_ParticipantBuiltinTopicData *)data;
    struct DPDE_DiscoveryPlugin *disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)param;

    UNUSED_ARG(param);

    if (!DDS_CdrQosPolicy_serializeKey(stream, &topic_data->key,
                                       RTPS_PID_PARTICIPANT_GUID))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeEntityNameQosPolicy(
        stream, &topic_data->participant_name, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_Cdr_serializeFourByteParameter(
            stream, &topic_data->dds_builtin_endpoints,
            RTPS_PID_BUILTIN_ENDPOINT_MASK))
    {
        DPDE_LOG_SERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeProtocolVersion(
            stream, &topic_data->rtps_protocol_version, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeVendorId(
            stream, &topic_data->rtps_vendor_id, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeLocator(
            stream, &topic_data->default_unicast_locators,
            RTPS_PID_DEFAULT_UNICAST_LOCATOR6))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeLocator(
            stream, &topic_data->default_multicast_locators,
            RTPS_PID_MULTICAST_LOCATOR6))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeLocator(
            stream, &topic_data->metatraffic_unicast_locators,
            RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeLocator(
            stream, &topic_data->metatraffic_multicast_locators,
            RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeLeaseDuration(
            stream, &topic_data->liveliness_lease_duration, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeProductVersion(
            stream, &topic_data->product_version, NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_CdrQosPolicy_serializeChecksumProperty(
            stream, &topic_data->checksum, NULL))
    {
        return RTI_FALSE;
    }

    if (disc_plugin && !DDS_CdrQosPolicy_serializeNonPrimitiveParameter(stream,
            disc_plugin->dds_properties,
            (CDR_Stream_SerializeFunction)
            CDR_Stream_serialize_property_sequence,
            RTPS_PID_PROPERTY_LIST, RTI_FALSE,
            RTI_TRUE))
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
    UNUSED_ARG(topic_data);
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
    struct DPDE_DiscoveryPlugin *disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)param;

    UNUSED_ARG(param_length);
    *ok = RTI_TRUE;

    switch (param_id)
    {
        case RTPS_PID_PARTICIPANT_GUID:
            *ok = DDS_CdrQosPolicy_deserializeKey(
                stream, &topic_data->key, NULL);
            break;
        case RTPS_PID_BUILTIN_ENDPOINT_MASK:
            if (!CDR_Stream_deserialize_unsigned_long(
                    stream, &topic_data->dds_builtin_endpoints))
            {
                DPDE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
                *ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_PROTOCOL_VERSION:
            *ok = DDS_CdrQosPolicy_deserializeProtocolVersion(
                    stream, &topic_data->rtps_protocol_version, NULL);
            break;
        case RTPS_PID_VENDOR_ID:
            *ok = DDS_CdrQosPolicy_deserializeVendorId(
                    stream, &topic_data->rtps_vendor_id, NULL);
            break;
        case RTPS_PID_DEFAULT_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserializeLocator(
                    stream, &topic_data->default_unicast_locators,param_id,disc_plugin);
            break;
        case RTPS_PID_MULTICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserializeLocator(
                    stream, &topic_data->default_multicast_locators,param_id,disc_plugin);
            break;
        case RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserializeLocator(
                    stream, &topic_data->metatraffic_unicast_locators,param_id,disc_plugin);
            break;
        case RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserializeLocator(
                    stream, &topic_data->metatraffic_multicast_locators,param_id,disc_plugin);
            break;
        case RTPS_PID_LEASE_DURATION:
            *ok = DDS_CdrQosPolicy_deserializeLeaseDuration(
                    stream, &topic_data->liveliness_lease_duration, NULL);
            break;
        case DISC_RTPS_PID_PRODUCT_VERSION:
            *ok = DDS_CdrQosPolicy_deserializeProductVersion(
                    stream, &topic_data->product_version, NULL);
            break;
        case DISC_RTPS_PID_ENTITY_NAME:
            *ok = DDS_CdrQosPolicy_deserializeEntityNameQosPolicy(
                    stream, &topic_data->participant_name, NULL);
            break;
        case RTPS_PID_PROPERTY_LIST:
            /* Micro sends DDS_ParticipantBuiltinTopicData containing
               RTPS_PID_PROPERTY_LIST, but does not use any of its 
               current properties.  Thus, ignore this parameter quietly. */
            break;
        case RTPS_PID_CHECKSUM_PROPERTY:
            if (CDR_Stream_is_vendor_rti(stream))
            {
                *ok = DDS_CdrQosPolicy_deserializeChecksumProperty(
                                stream, &topic_data->checksum, NULL);
            }
            else
            {
                *ok = RTI_FALSE;
                DPDE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)param_id,
                                                 (RTI_INT32)param_length)
            }
            break;
        default:
            /* Unknown parameter is either ignored or treated as
               incompatible QoS */
            if ((param_id & RTPS_PID_INCOMPATIBLE_MASK) != 0)
            {
                *ok = RTI_FALSE;
                DPDE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)param_id,
                                                 (RTI_INT32)param_length)
            }
#if OSAPI_ENABLE_LOG
            else
            {
                DPDE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_INFO,
                                                 (RTI_INT32)param_id,
                                                 (RTI_INT32)param_length)
            }
#endif
            return *ok;
    }

    return *ok;
}

MUST_CHECK_RETURN RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_create_sample(
                                                struct NDDS_Type_Plugin* plugin,
                                                void **sample,
                                                void *param)
{
    struct DDS_ParticipantBuiltinTopicData *participant_builtin_data = NULL;
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(param);

    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_allocate_struct(sample, struct DDS_ParticipantBuiltinTopicData);

    if (*sample == NULL)
    {
        return RTI_FALSE;
    }

    participant_builtin_data = (*sample);

    if (!DDS_ParticipantBuiltinTopicData_initialize(participant_builtin_data))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(
                        &participant_builtin_data->default_unicast_locators,
                        DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }
    if (!DDS_LocatorSeq_set_maximum(
                        &participant_builtin_data->metatraffic_unicast_locators,
                        DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }
    if (!DDS_LocatorSeq_set_maximum(
                        &participant_builtin_data->metatraffic_multicast_locators,
                        DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }
    if (!DDS_LocatorSeq_set_maximum(
                        &participant_builtin_data->default_multicast_locators,
                        DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }

    participant_builtin_data->dds_builtin_endpoints =
        DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
        DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);
#ifdef RTI_CERT
    UNUSED_ARG(sample);
#else
    DDS_ParticipantBuiltinTopicData_finalize(
                            (struct DDS_ParticipantBuiltinTopicData*)sample);
    OSAPI_Heap_free_struct(sample);
#endif /* RTI_CERT */
    return RTI_TRUE;
}


MUST_CHECK_RETURN RTI_PRIVATE NDDS_TypePluginKeyKind
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_key_kind(
                                            struct NDDS_Type_Plugin* plugin,
                                            void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);
    return NDDS_TYPEPLUGIN_GUID_KEY;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_copy_sample(
                                                struct NDDS_Type_Plugin *type,
                                                void *dst,
                                                const void *src,
                                                void *param)
{
    UNUSED_ARG(type);
    UNUSED_ARG(param);

    return (DDS_ParticipantBuiltinTopicData_copy(
            (struct DDS_ParticipantBuiltinTopicData *)dst,
            (struct DDS_ParticipantBuiltinTopicData *)src));
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_deserialize(
                                                struct CDR_Stream_t *stream,
                                                void *sample,
                                                void *param)
{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                        (struct DDS_ParticipantBuiltinTopicData *)sample;
    RTI_BOOL retval;

    UNUSED_ARG(param);

    /* This is a new sample: empty out all sequences */
    DDS_LocatorSeq_set_length(&topic_data->default_unicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->metatraffic_unicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->metatraffic_multicast_locators, 0);
    DDS_LocatorSeq_set_length(&topic_data->default_multicast_locators, 0);

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

    retval =  DDS_Cdr_deserializeParameterSequence(topic_data, stream,
              (DDS_Cdr_SetDefaultParameterValuesFunction)
              DPDE_BuiltinTopicParticipantDataPluginSupport_set_dflt_pv,
              (DDS_Cdr_DeserializeParameterValueFunction)
              DPDE_ParticipantBuiltinTopicData_deserialize_pv,param);


    return retval;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_serializeKey(
                                        struct CDR_Stream_t *stream,
                                        const void *sample_key,
                                        void *param)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(param);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_deserializeKey(
                                        struct CDR_Stream_t *stream,
                                        UserDataKeyHolder_t sample_key,
                                        void *param)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(param);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_instanceToKeyHash(
                                            struct NDDS_Type_Plugin *plugin,
                                            struct CDR_Stream_t *stream,
                                            DDS_KeyHash_t *keyHash,
                                            const void *instance,
                                            void *param)
{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                        (struct DDS_ParticipantBuiltinTopicData *)instance;
    DDS_InstanceHandle_t ih;

    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(param);

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid*)&topic_data->key);
    OSAPI_Memory_copy(keyHash->value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);

    return RTI_TRUE;

}

MUST_CHECK_RETURN RTI_PRIVATE  RTI_UINT32
DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment,
        void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);
    UNUSED_ARG(param);

    return RTPS_KEY_HASH_MAX_LENGTH;
}

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NDDS_Type_Plugin DPDE_fv_ParticipantBuiltinTopicDataTypePlugin =
{
    {0,0},
    NULL,
    NULL,
    NDDS_TYPEPLUGIN_GUID_KEY,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_serialize,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_deserialize,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_max_size,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_serializeKey,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_deserializeKey,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_create_sample,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_delete_sample,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_copy_sample,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_get_key_kind,
    DPDE_ParticipantBuiltinTopicDataTypePlugin_instanceToKeyHash,
    NULL,
    NULL,
    NULL,
    NULL
};

MUST_CHECK_RETURN struct NDDS_Type_Plugin*
DPDE_ParticipantBuiltinTopicDataTypePlugin_get(void)
{
    return &DPDE_fv_ParticipantBuiltinTopicDataTypePlugin;
}
