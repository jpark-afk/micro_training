/*
 * FILE: PublicationBuiltinTopicDataPlugin.c -
 *                          PublicationBuiltinTopicDataPlugin API
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
 * 20may2014,eh  MICRO-799: Unknown RTPS parameter as incompatible Qos
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief PublicationBuiltinTopicDataPlugin API
 */
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef disc_dpde_discovery_plugin_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "BuiltinCdr.h"
#include "PublicationBuiltinTopicDataPlugin.h"

/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPDE_PublicationBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
                                        DDS_UnsignedLong size)
{
    DDS_UnsignedLong orig_sz = size;
    RTI_INT32 i;

    /* Call get max size serialized on each parameter header and field
     * according to the type.
     * We assume there is a logical reset before serializing each value.
     * getParameterHeaderMaxSizeSerialize() take care of 4-byte alignment.
     * Sentinel parameter size is not included in the calculation.
     */
    size += DDS_CdrQosPolicy_getKeyMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getKeyMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getTopicNameMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getTypeNameMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getDeadlineMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getOwnershipMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getOwnershipStrengthMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getReliabilityMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getLivelinessMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getDurabilityMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getDestinationOrderMaxSerializedSize(size);

    size += DDS_CdrQosPolicy_getPresentationMaxSerializedSize(size);

    /* uni-cast locator */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
        size += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Add size for the sentinel parameter header */
    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);

    return (size - orig_sz);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_UINT32
DPDE_PublicationBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(
                                            struct NDDS_Type_Plugin *plugin,
                                            RTI_UINT32 current_alignment,
                                            void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);
    UNUSED_ARG(param);

    return DDS_CdrQosPolicy_get_header_max_size_serialized(0) +
           DPDE_PublicationBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(0);

}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_serialize(struct CDR_Stream_t *stream,
                                                     const void *data,
                                                     void *param)
{
    DDS_UnsignedShort zeroLength = 0;
    DDS_UnsignedShort sentinel;
    DDS_DataWriter *datawriter = (DDS_DataWriter*)data;
    DDS_DomainParticipant *participant;
    struct DDS_DataWriterQos *qos;
    struct RTPS_Guid guid;
    DDS_InstanceHandle_t instance_handle;
    struct DDS_BuiltinTopicKey_t key;
    const char *s_ptr;

    UNUSED_ARG(param);

    participant = DDS_Publisher_get_participant(DDS_DataWriter_get_publisher(datawriter));
    qos = DDS_DataWriter_get_qos_ref(datawriter);


    /*---------------------- Serialize the endpoint guid ---------------------*/
    instance_handle = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_InstanceHandle_to_rtps(&guid, &instance_handle);

    key.value[0] = guid.prefix.host_id;
    key.value[1] = guid.prefix.app_id ;
    key.value[2] = guid.prefix.instance_id ;
    key.value[3] = guid.object_id;

    if (!DDS_CdrQosPolicy_serializeKey(stream,&key,RTPS_PID_ENDPOINT_GUID))
    {
        return RTI_FALSE;
    }

    /*---------------------- Serialize the participant guid ------------------*/
    instance_handle = DDS_Entity_get_instance_handle(
                                DDS_DomainParticipant_as_entity(participant));
    DDS_InstanceHandle_to_rtps(&guid, &instance_handle);

    key.value[0] = guid.prefix.host_id;
    key.value[1] = guid.prefix.app_id ;
    key.value[2] = guid.prefix.instance_id ;
    key.value[3] = guid.object_id;

    if (!DDS_CdrQosPolicy_serializeKey(stream,&key,RTPS_PID_PARTICIPANT_GUID))
    {
        return RTI_FALSE;
    }

    /*---------------------- Serialize the topic name ------------------------*/
    s_ptr = DDS_TopicDescription_get_name(
                                DDS_Topic_as_topicdescription(
                                        DDS_DataWriter_get_topic(datawriter)));
    if (!DDS_CdrQosPolicy_serializeTopicName(stream,(const char *)s_ptr,NULL))
    {
        return RTI_FALSE;
    }

    /*---------------------- Serialize the type name -------------------------*/
    s_ptr = DDS_TopicDescription_get_type_name(
                                DDS_Topic_as_topicdescription(
                                        DDS_DataWriter_get_topic(datawriter)));

    if (!DDS_CdrQosPolicy_serializeTypeName(stream,(const char *)s_ptr,NULL))
    {
        return RTI_FALSE;
    }

    /*------------------------ Serialize  deadline ---------------------------*/
    if (!DDS_CdrQosPolicy_serializeDeadline(stream,&qos->deadline, NULL))
    {
        return RTI_FALSE;
    }

    /*------------------------ Serialize ownership ---------------------------*/
    if (!DDS_CdrQosPolicy_serializeOwnership(stream,&qos->ownership, NULL))
    {
        return RTI_FALSE;
    }

    /*--------------------- Serialize ownership_strength ---------------------*/
    if (!DDS_CdrQosPolicy_serializeOwnershipStrength(stream,
                                                &qos->ownership_strength, NULL))
    {
        return RTI_FALSE;
    }

    /*----------------------- Serialize reliability --------------------------*/

    if (!DDS_CdrQosPolicy_serializeReliability(stream,&qos->reliability, NULL))
    {
        return RTI_FALSE;
    }

    /*----------------------- Serialize liveliness ---------------------------*/

    if (!DDS_CdrQosPolicy_serializeLiveliness(stream,&qos->liveliness, NULL))
    {
        return RTI_FALSE;
    }

    /*------------------------ Serialize  durability -------------------------*/
    if (!DDS_CdrQosPolicy_serializeDurability(stream,&qos->durability, NULL))
    {
        return RTI_FALSE;
    }

    /*----------------- Serialize  destination order -------------------------*/
    if (!DDS_CdrQosPolicy_serializeDestinationOrder(stream,&qos->destination_order,
                                                    NULL))
    {
        return RTI_FALSE;
    }

    /*----------------- Serialize  presentation ------------------------------*/
    if (!DDS_CdrQosPolicy_serializePresentation(stream,
                                     &DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT,
                                     NULL))
    {
        return RTI_FALSE;
    }

    /*----------------------- Serialize send queue size (deprecated) ---------*/

    /* Always send -1 for infinite */
    if (!DDS_CdrQosPolicy_serializeSendQueueSize(stream,DDS_LENGTH_UNLIMITED, NULL))
    {
        return RTI_FALSE;
    }

    /*----------------------- Serialize unicast locators ---------------------*/
    if (!DDS_CdrQosPolicy_serializeLocator(stream,
                                           qos->data->unicast_locator,
                                           RTPS_PID_UNICAST_LOCATOR6))
    {
        return RTI_FALSE;
    }

    /*------------------------ Serialize sentinel ----------------------------*/

    sentinel = RTPS_PID_SENTINEL;
    if (RTI_TRUE != CDR_Stream_serialize_unsigned_short(stream,&sentinel))
    {
        return RTI_FALSE;
    }

    if (RTI_TRUE != CDR_Stream_serialize_unsigned_short(stream, &zeroLength))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


/*
  Returns RTI_TRUE if there is a match in parameterId. Still need to set the
  ok to indicate if value is deserialized correctly.
  
*/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_deserialize_pv(RTI_BOOL *ok,
                            struct DDS_PublicationBuiltinTopicData* topic_data,
                            struct CDR_Stream_t *stream,
                            DDS_UnsignedShort param_id,
                            DDS_UnsignedShort param_length,
                            void *param)
{
    struct DPDE_DiscoveryPlugin *plugin = (struct DPDE_DiscoveryPlugin *)param;

    *ok = RTI_TRUE;

    /* check each field to see if the paramterId matches */
    switch (param_id)
    {
        case RTPS_PID_PARTICIPANT_GUID:
            *ok = DDS_CdrQosPolicy_deserializeKey(
                stream, &topic_data->participant_key, NULL);
            break;
        case RTPS_PID_ENDPOINT_GUID:
            *ok = DDS_CdrQosPolicy_deserializeKey(
                stream, &topic_data->key, NULL);
            break;
        case RTPS_PID_TOPIC_NAME:
            *ok = DDS_CdrQosPolicy_deserializeTopicName(
                stream, topic_data->topic_name, NULL);
            break;
        case RTPS_PID_TYPE_NAME:
            *ok = DDS_CdrQosPolicy_deserializeTypeName(
                stream, topic_data->type_name, NULL);
            break;
        case RTPS_PID_DEADLINE:
            *ok = DDS_CdrQosPolicy_deserializeDeadline(
                stream, &topic_data->deadline, NULL);
            break;
        case RTPS_PID_OWNERSHIP:
            *ok = DDS_CdrQosPolicy_deserializeOwnership(
                stream, &topic_data->ownership, NULL);
            break;
        case RTPS_PID_OWNERSHIP_STRENGTH:
            *ok = DDS_CdrQosPolicy_deserializeOwnershipStrength(
                    stream, &topic_data->ownership_strength, NULL);
            break;
        case RTPS_PID_RELIABILITY:
            *ok = DDS_CdrQosPolicy_deserializeReliability(
                    stream, &topic_data->reliability, NULL);
            break;
        case RTPS_PID_LIVELINESS:
            *ok = DDS_CdrQosPolicy_deserializeLiveliness(
                    stream, &topic_data->liveliness, NULL);
            break;
        case RTPS_PID_DURABILITY:
            *ok = DDS_CdrQosPolicy_deserializeDurability(
                    stream, &topic_data->durability, NULL);
            break;
        case RTPS_PID_DESTINATION_ORDER:
            *ok = DDS_CdrQosPolicy_deserializeDestinationOrder(
                stream, &topic_data->destination_order, NULL);
            break;
        case RTPS_PID_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserializeLocator(
                    stream, &topic_data->unicast_locator, param_id,plugin);
            break;
        case RTPS_PID_SEND_QUEUE_SIZE_DEPRECATED:
            if (!CDR_Stream_increment_current_position(stream,param_length))
            {
            }
            break;
        case RTPS_PID_PRESENTATION:
            {
                /* DR use the default PRESENTATION QoS. Any PRESENTATION QoS
                 * in the DW is compatible
                 */
                struct DDS_PresentationQosPolicy presentation =
                                        DDS_PRESENTATION_QOS_POLICY_DEFAULT;

                *ok = DDS_CdrQosPolicy_deserializePresentation(
                                                    stream,
                                                    &presentation,
                                                    plugin);
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
DPDE_PublicationBuiltinTopicDataTypePlugin_create_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void **sample,
                                            void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);
    OSAPI_Heap_allocate_struct(sample, struct DDS_PublicationBuiltinTopicData);

    if (*sample == NULL)
    {
        return RTI_FALSE;
    }

    return DDS_PublicationBuiltinTopicData_initialize(
                        (struct DDS_PublicationBuiltinTopicData*)(*sample));
}

MUST_CHECK_RETURN RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);
#ifdef RTI_CERT
    UNUSED_ARG(sample);
#else
    DDS_PublicationBuiltinTopicData_finalize(
                            (struct DDS_PublicationBuiltinTopicData *)sample);
    OSAPI_Heap_free_struct(sample);
#endif /* RTI_CERT */
    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE NDDS_TypePluginKeyKind
DPDE_PublicationBuiltinTopicDataTypePlugin_get_key_kind(
        struct NDDS_Type_Plugin *plugin,
        void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);
    return NDDS_TYPEPLUGIN_GUID_KEY;
}

MUST_CHECK_RETURN RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_copy_sample(
        struct NDDS_Type_Plugin *type,
        void *dst,
        const void *src,
        void *param)
{
    UNUSED_ARG(type);
    UNUSED_ARG(param);

    return DDS_PublicationBuiltinTopicData_copy(
            (struct DDS_PublicationBuiltinTopicData*)dst,
            (struct DDS_PublicationBuiltinTopicData*)src);
}

RTI_PRIVATE void
DPDE_PublicationBuiltinTopicPublicationDataTypeSupport_set_dflt_pv(
        struct  DDS_PublicationBuiltinTopicData *topic_data)
{
    UNUSED_ARG(topic_data);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_deserialize(
        struct CDR_Stream_t *stream, void *sample, void *param)
{
    struct DDS_PublicationBuiltinTopicData *topic_data =
        (struct DDS_PublicationBuiltinTopicData *)sample;

    struct DDS_PublicationBuiltinTopicData dflt_data =
                    DDS_PublicationBuiltinTopicData_INITIALIZER;
    UNUSED_ARG(param);

    DDS_LocatorSeq_set_length(&topic_data->unicast_locator, 0);

    topic_data->deadline = dflt_data.deadline;
    topic_data->durability = dflt_data.durability;
    topic_data->liveliness = dflt_data.liveliness;
    topic_data->ownership = dflt_data.ownership;
    topic_data->reliability = dflt_data.reliability;
    topic_data->ownership_strength = dflt_data.ownership_strength;

    return DDS_Cdr_deserializeParameterSequence(
       topic_data, stream,
       (DDS_Cdr_SetDefaultParameterValuesFunction)
       DPDE_PublicationBuiltinTopicPublicationDataTypeSupport_set_dflt_pv,
       (DDS_Cdr_DeserializeParameterValueFunction)
       DPDE_PublicationBuiltinTopicDataTypePlugin_deserialize_pv,param);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_serializeKey(
                                            struct CDR_Stream_t *stream,
                                            const void* sample_key,
                                            void *param)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(param);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_deserializeKey(
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
DPDE_PublicationBuiltinTopicDataTypePlugin_instance_to_keyhash(
                                        struct NDDS_Type_Plugin *plugin,
                                        struct CDR_Stream_t *stream,
                                        DDS_KeyHash_t *keyHash,
                                        const void *instance,
                                        void *param)
{
    struct DDS_PublicationBuiltinTopicData *topic_data =
                        (struct DDS_PublicationBuiltinTopicData*)instance;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(param);

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid*)&topic_data->key);
    OSAPI_Memory_copy(keyHash->value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE  RTI_UINT32
DPDE_PublicationBuiltinTopicDataTypePlugin_get_serialized_key_size(
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
RTI_PRIVATE struct NDDS_Type_Plugin DPDE_fv_PublicationBuiltinTopicDataTypePlugin =
{
    {0,0},
    NULL,
    NULL,
    NDDS_TYPEPLUGIN_GUID_KEY,
    DPDE_PublicationBuiltinTopicDataTypePlugin_serialize,
    DPDE_PublicationBuiltinTopicDataTypePlugin_deserialize,
    DPDE_PublicationBuiltinTopicDataTypePlugin_get_serialized_sample_max_size,
    DPDE_PublicationBuiltinTopicDataTypePlugin_serializeKey,
    DPDE_PublicationBuiltinTopicDataTypePlugin_deserializeKey,
    DPDE_PublicationBuiltinTopicDataTypePlugin_get_serialized_key_size,
    DPDE_PublicationBuiltinTopicDataTypePlugin_create_sample,
    DPDE_PublicationBuiltinTopicDataTypePlugin_delete_sample,
    DPDE_PublicationBuiltinTopicDataTypePlugin_copy_sample,
    DPDE_PublicationBuiltinTopicDataTypePlugin_get_key_kind,
    DPDE_PublicationBuiltinTopicDataTypePlugin_instance_to_keyhash,
    NULL,
    NULL,
    NULL,
    NULL
};

struct NDDS_Type_Plugin*
DPDE_PublicationBuiltinTopicDataTypePlugin_get(void)
{
    return &DPDE_fv_PublicationBuiltinTopicDataTypePlugin;
}
