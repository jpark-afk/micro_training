/*
 * FILE: SubscriptionBuiltinTopicDataPlugin.c -
 *                          SubscriptionBuiltinTopicDataPlugin API
 *
 * Copyright (c) 2011-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 25Mar2015,as MICRO-908  Removed unused PluginHelper functions
 * 20may2014,eh MICRO-799  Unknown RTPS parameter as incompatible Qos
 * 19mar2014,tk MICRO-74   Support endpoint specific transport
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief SubscriptionBuiltinTopicDataPlugin API
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
#include "SubscriptionBuiltinTopicDataPlugin.h"
#include "QosPolicy.h"
#include "PartitionQosPolicy.h"
#include "DomainParticipant.h"
#include "BuiltinTopicData.h"
#include "UserDataQosPolicy.h"

#if DDS_FILTERING_ENABLED
#include "DomainParticipantFilter.h"
#include "DataReaderFilter.h"
#endif

/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN RTI_PRIVATE RTI_UINT32
DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(
            struct NDDS_Type_Plugin *plugin,
            void *sample,
            RTI_UINT32 current_alignment);

/* ------------------------------------------------------------------------ */
/*                   Builtin type plugin                                    */
/* ------------------------------------------------------------------------ */
MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
                                        struct NDDS_Type_Plugin *plugin,
                                        DDS_UnsignedLong size)
{
    DDS_UnsignedLong origSize = size;
    struct DDS_LocatorSeq locator = DDS_SEQUENCE_INITIALIZER;
    struct DDS_LocatorExSeq locator_ex = DDS_SEQUENCE_INITIALIZER;
    struct DDS_DataRepresentationQosPolicy policy = DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT;
    struct DDS_DomainParticipantQos *dp_qos =
            DDS_DomainParticipant_get_qos_ref(DDS_TypePlugin_get_participant(plugin));

    /* No need to allocate the sequence */
    locator._maximum = RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX;
    locator_ex._maximum = RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX;
    policy.value._maximum = 4;


    /* Call get max size serialized on each parameter header and field
     * according to the type.
     * We assume there is a logical reset before serializing each value.
     * getParameterHeaderMaxSizeSerialize() take care of 4-byte alignment.
     * Sentinel parameter size is not included in the calculation.
     */
    size += DDS_CdrQosPolicy_get_max_size_serialized_key(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_key(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_topic_name(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_type_name(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_partition(size,
        (RTI_UINT32)dp_qos->resource_limits.max_partition_cumulative_characters);

    size += DDS_CdrQosPolicy_get_max_size_serialized_deadline(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_ownership(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_reliability(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_liveliness(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_durability(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_destination_order(size);

    /*-- Exteneded Unicast --*/
    size += DDS_CdrQosPolicy_get_max_size_serialized_ex_locator(&locator_ex, size);

    /*-- Multicast --*/
    size += DDS_CdrQosPolicy_get_max_size_serialized_locator(&locator, size);

    size += DDS_CdrQosPolicy_get_max_serialized_size_data_representation(&policy, size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_latency_budget(size);

    /* NOTE: Protocol version and vendor id are not part of the standard
     * builtin topic, and shouldn't have to be, but Connext Pro relies on
     * the endpoint data to contain the vendor and protocol version.
     */
    /*  RTPS Protocol Version */
    size += DDS_CdrQosPolicy_get_max_size_serialized_protocol_version(size);

    /* RTPS Vendor Id */
    size += DDS_CdrQosPolicy_get_max_size_serialized_vendor_id(size);

    size += DDS_CdrQosPolicy_get_max_size_serialized_user_data(
                size, (RTI_UINT32)dp_qos->resource_limits.reader_user_data_max_length);

    size += DDS_CdrQosPolicy_get_max_size_serialized_group_data(
                size, (RTI_UINT32)dp_qos->resource_limits.subscriber_group_data_max_length);

    size += DDS_CdrQosPolicy_get_max_size_serialized_topic_data(
                size, (RTI_UINT32)dp_qos->resource_limits.topic_data_max_length);

#if DDS_FILTERING_ENABLED
    size += DDS_DomainParticipant_filter_property_get_max_serialized_size(
                DDS_TypePlugin_get_participant(plugin),
                size);
#endif

    /* Add size for the sentinel parameter header */
    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);

    return (size - origSize);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_UINT32
DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(
            struct NDDS_Type_Plugin *plugin,
            void *sample,
            RTI_UINT32 current_alignment)
{
    UNUSED_ARG(current_alignment);
    UNUSED_ARG(sample);

    return DDS_Cdr_get_header_max_size_serialized(0) +
           DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(plugin, 0);

}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_serialize(
        struct NDDS_Type_Plugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination)
{
    DDS_UnsignedShort zeroLength = 0;
    const unsigned short sentinel = RTPS_PID_SENTINEL;
    DDS_DataReader *datareader = (DDS_DataReader*)data;
    DDS_Subscriber *subscriber = NULL;
    DDS_DomainParticipant *participant = NULL;
    struct RTPS_Guid guid = RTPS_GUID_UNKNOWN;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    struct DDS_BuiltinTopicKey_t key = DDS_BUILTINTOPICKEY_UNKNOWN;
    const char *s_ptr;
    struct DDS_TypePlugin *reader_plugin;
    const struct DDS_VendorId vendor_id = {{RTI_CONNEXT_MICRO_VENDOR_ID_MAJOR,
                                            RTI_CONNEXT_MICRO_VENDOR_ID_MINOR}};
    const DDS_ProtocolVersion_t protocol_version = DDS_PROTOCOLVERSION;
    DDS_Topic *topic = NULL;

    UNUSED_ARG(plugin);
    UNUSED_ARG(destination);

    subscriber = DDS_DataReader_get_subscriber(datareader);
    participant = DDS_Subscriber_get_participant(subscriber);
    reader_plugin = DDS_DataReader_get_type_plugin(datareader);

    topic = DDS_DataReader_get_topic(datareader);

    /*---------------------- Serialize the endpoint guid ---------------------*/
    instance_handle = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datareader));
    DDS_InstanceHandle_to_rtps(&guid, &instance_handle);
    DDS_BuiltinTopicKey_from_instance_handle(&key,&instance_handle);
    key.value[0] = guid.prefix.host_id;
    key.value[1] = guid.prefix.app_id ;
    key.value[2] = guid.prefix.instance_id ;
    key.value[3] = guid.object_id;

    if (!DDS_CdrQosPolicy_serialize_key(stream,&key,RTPS_PID_ENDPOINT_GUID))
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

    if (!DDS_CdrQosPolicy_serialize_key(stream,&key,RTPS_PID_PARTICIPANT_GUID))
    {
        return RTI_FALSE;
    }

    /*---------------------- Serialize the topic name ------------------------*/
    s_ptr = DDS_TopicDescription_get_name(
                            DDS_DataReader_get_topicdescription(datareader));

    if (!DDS_CdrQosPolicy_serialize_topic_name(stream,(const char *)s_ptr, NULL))
    {
        return RTI_FALSE;
    }

    /*---------------------- Serialize the type name -------------------------*/
    s_ptr = DDS_TopicDescription_get_type_name(
                            DDS_DataReader_get_topicdescription(datareader));

    if (!DDS_CdrQosPolicy_serialize_type_name(stream,(const char *)s_ptr, NULL))
    {
        return RTI_FALSE;
    }

    /*---------- Serialize the protocol version name -------------------------*/

    /* None standard, required by Connext Pro */
    if (!DDS_CdrQosPolicy_serialize_protocol_version(stream,
                                                     &protocol_version, NULL))
    {
        return RTI_FALSE;
    }

    /*---------- Serialize the vendor id  -------------------------*/

    /* None standard, required by Connext Pro */
    if (!DDS_CdrQosPolicy_serialize_vendor_id(stream, &vendor_id, NULL))
    {
        return RTI_FALSE;
    }


    /*------------------------ Serialize  partition --------------------------*/
    if (!DDS_Subscriber_serialize(subscriber,stream))
    {
        return RTI_FALSE;
    }

    if (!DDS_DataReader_serialize(datareader,stream))
    {
        return RTI_FALSE;
    }

    /*----------------------- Serialize latency budget  ----------------------*/

    /* Serialize the data-representation supported. Due to implicit behavior
     * and allowing the application qos to be different than what is actually
     * used the data represenation oqs is not serialized as is. Instead check
     * with is actually enabled in the type-plugin and serialize based on this.
     */
    if (DDS_TypePlugin_is_representation_enabled(reader_plugin,DDS_XCDR2_DATA_REPRESENTATION) &&
        DDS_TypePlugin_is_representation_enabled(reader_plugin,DDS_XCDR_DATA_REPRESENTATION))
    {
        if (!DDS_CdrQosPolicy_serialize_data_representation(stream,
                                &DDS_DataRepresentationQosPolicy_gv_V2_V1))
        {
            return RTI_FALSE;
        }
    }
    else if (DDS_TypePlugin_is_representation_enabled(reader_plugin,DDS_XCDR2_DATA_REPRESENTATION))
    {
        if (!DDS_CdrQosPolicy_serialize_data_representation(stream,
                                &DDS_DataRepresentationQosPolicy_gv_V2))
        {
            return RTI_FALSE;
        }
    }
    else
    {
        /* Don't send V1 as this is the default */
    }

    /*------------------------ Serialize topic data --------------------------*/
    if (!DDS_Topic_serialize(topic,stream))
    {
        return RTI_FALSE;
    }

    /*------------------------ Serialize sentinel ----------------------------*/
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

/*
  Returns RTI_TRUE if there is a match in parameterId. Still need to set the
  ok to indicate if value is deserialized correctly.
*/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicData_deserialize_pv(RTI_BOOL *ok,
        struct DDS_SubscriptionBuiltinTopicData *topic_data,
        struct CDR_Stream_t *stream,
        DDS_UnsignedShort parameterId,
        DDS_UnsignedShort parameterLength,
        void *param)
{
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*)param;
    struct DDS_DomainParticipantQos *dp_qos =
                                        DDS_DomainParticipant_get_qos_ref(dp);

    UNUSED_ARG(parameterLength);

    /* check each field to see if the paramterId matches */
    switch (parameterId)
    {
        case RTPS_PID_PARTICIPANT_GUID:
            *ok = DDS_CdrQosPolicy_deserialize_key(
                stream, &topic_data->participant_key, NULL);
            break;
        case RTPS_PID_ENDPOINT_GUID:
            *ok = DDS_CdrQosPolicy_deserialize_key(
                stream, &topic_data->key, NULL);
            break;
        case RTPS_PID_TOPIC_NAME:
            *ok = DDS_CdrQosPolicy_deserialize_topic_name(
                stream, &topic_data->topic_name, dp->string_manager);
            break;
        case RTPS_PID_TYPE_NAME:
            *ok = DDS_CdrQosPolicy_deserialize_type_name(
                stream, &topic_data->type_name, dp->string_manager);
            break;
        case RTPS_PID_PARTITION:
            *ok = DDS_CdrQosPolicy_deserialize_partition(stream,
                    &topic_data->partition,
                    DDS_DomainParticipant_get_partition_string_manager(dp),
                    dp_qos->resource_limits.max_partition_cumulative_characters,
                    dp_qos->resource_limits.max_partition_string_size);
            break;
        case RTPS_PID_DEADLINE:
            *ok = DDS_CdrQosPolicy_deserialize_deadline(
                stream, &topic_data->deadline, NULL);
            break;
        case RTPS_PID_OWNERSHIP:
            *ok = DDS_CdrQosPolicy_deserialize_ownership(
                stream, &topic_data->ownership, NULL);
            break;
        case RTPS_PID_RELIABILITY:
            *ok = DDS_CdrQosPolicy_deserialize_reliability(
                stream, &topic_data->reliability, NULL);
            break;
        case RTPS_PID_LIVELINESS:
            *ok = DDS_CdrQosPolicy_deserialize_liveliness(
                stream, &topic_data->liveliness, NULL);
            break;
        case RTPS_PID_DURABILITY:
            *ok = DDS_CdrQosPolicy_deserialize_durability(
                stream, &topic_data->durability, NULL);
            break;
        case RTPS_PID_DESTINATION_ORDER:
            *ok = DDS_CdrQosPolicy_deserialize_destination_order(
                stream, &topic_data->destination_order, NULL);
            break;
        case RTPS_PID_UNICAST_LOCATOR6_EX:
            if (!CDR_Stream_is_vendor_rti(stream))
            {
                /* if vendor is not RTI, we ignore this PID else we fall through */
                *ok = RTI_TRUE;
                break;
            }
            else
            {
                *ok = DDS_CdrQosPolicy_deserialize_locator_ex_sequence(
                                    stream,
                                    &topic_data->unicast_locator,
                                    parameterId, dp);
                break;
            }
        /* The exteneded locators are only supported over the shmem
         * transport which is only unicast. Thus
         * RTPS_PID_MULTICAST_CAST_LOCATOR6_EX is ignored.
         */
        case RTPS_PID_UNICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_ex_sequence(
                    stream,
                    &topic_data->unicast_locator,
                    parameterId,dp);
            break;
        case RTPS_PID_MULTICAST_LOCATOR6:
            *ok = DDS_CdrQosPolicy_deserialize_locator_sequence(
                    stream, &topic_data->multicast_locator,parameterId, dp);
            break;
        case RTPS_PID_DATA_REPRESENTATION:
            *ok = DDS_CdrQosPolicy_deserialize_data_representation(
                                                    stream,
                                                    &topic_data->representation,
                                                    NULL);
            break;
        case RTPS_PID_LATENCY_BUDGET:
            *ok = DDS_CdrQosPolicy_deserialize_latency_budget(
                    stream, &topic_data->latency_budget, NULL);
            break;
        case RTPS_PID_USER_DATA:
            *ok = DDS_CdrQosPolicy_deserialize_user_data(
                            stream, &topic_data->user_data,
                            DDS_DomainParticipant_get_user_data_manager(dp),
                            DDS_USER_DATA_DATAREADER_TYPE);
            break;
        case RTPS_PID_GROUP_DATA:
            *ok = DDS_CdrQosPolicy_deserialize_group_data(
                            stream, &topic_data->group_data,
                            DDS_DomainParticipant_get_user_data_manager(dp),
                            DDS_USER_DATA_SUBSCRIBER_TYPE);
            break;
        case RTPS_PID_TOPIC_DATA:
            *ok = DDS_CdrQosPolicy_deserialize_topic_data(
                            stream, &topic_data->topic_data,
                            DDS_DomainParticipant_get_user_data_manager(dp),
                            DDS_USER_DATA_TOPIC_TYPE);
            break;
#if DDS_FILTERING_ENABLED
        case RTPS_PID_CONTENT_FILTER_PROPERTY:
            *ok = DDS_DomainParticipant_deserialize_content_filter_property(
                    dp, stream, &topic_data->content_filter);
            break;
#endif
        /* Don't show any warning/error for received PIDs which are known
         * but not needed
         */
        case RTPS_PID_PROTOCOL_VERSION:
        case RTPS_PID_VENDOR_ID:
            *ok = RTI_TRUE;
            break;

        case RTPS_PID_PRESENTATION:
                *ok = DDS_CdrQosPolicy_deserialize_presentation(
                                                    stream,
                                                    &topic_data->presentation,
                                                    NULL);
            break;

        default:
            /* Unknown parameter is either ignored or treated as
             * incompatible QoS
             */
            if ((parameterId & RTPS_PID_INCOMPATIBLE_MASK) != 0)
            {
                *ok = RTI_FALSE;
                DDSC_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)parameterId,
                                                 (RTI_INT32)parameterLength)
            }
#if OSAPI_ENABLE_LOG
            else
            {
                DDSC_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_INFO,
                                                 (RTI_INT32)parameterId,
                                                 (RTI_INT32)parameterLength)
            }
#endif
            return *ok;
    }

    return *ok;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_create_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void **sample)
{
    OSAPI_Heap_allocate_struct(sample, struct DDS_SubscriptionBuiltinTopicData);

    if (*sample == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_SubscriptionBuiltinTopicData_initialize_shallow(
            (struct DDS_SubscriptionBuiltinTopicData*)(*sample),
            DDS_TypePlugin_get_participant(plugin)))
    {
        OSAPI_Heap_free_struct(*sample);
        *sample = NULL;
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample)
{
#ifdef RTI_CERT
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
#else
    if (!DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(
            (struct DDS_SubscriptionBuiltinTopicData *)sample,
            DDS_TypePlugin_get_participant(plugin)))
    {
        return RTI_FALSE;
    }

    if (!DDS_SubscriptionBuiltinTopicData_finalize(
                            (struct DDS_SubscriptionBuiltinTopicData *)sample))
    {
        return RTI_FALSE;
    }
    OSAPI_Heap_free_struct((void*)sample);

#endif /* RTI_CERT */
    return RTI_TRUE;
}

RTI_PRIVATE void
DPDE_SubscriptionBuiltinTopicDataTypePlugin_return_sample(struct DDS_TypePlugin *tp,
                                      struct DDS_TypePluginSampleHolder *sample)
{
    DDS_DomainParticipant *participant = DDS_TypePlugin_get_participant(tp);
    struct DDS_SubscriptionBuiltinTopicData *data =
            (struct DDS_SubscriptionBuiltinTopicData *)sample->sample;
    DDS_Boolean retval = 0;

    retval &= DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(data, participant);

    /* return sample returns void*/
    IGNORE_RETVAL(retval);

    DDS_TypePluginDefaultCdr_return_sample(tp, sample);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_copy_sample(
                                                struct NDDS_Type_Plugin *plugin,
                                                void *dst,
                                                const void *src)
{
    UNUSED_ARG(plugin);

    return !DDS_SubscriptionBuiltinTopicData_copy(
                        (struct DDS_SubscriptionBuiltinTopicData*)dst,
                        (struct DDS_SubscriptionBuiltinTopicData*)src);
}

RTI_PRIVATE void
DPDE_SubscriptionBuiltinTopicDataPluginTypeSupport_set_dflt_pv(
        struct DDS_SubscriptionBuiltinTopicData *topic_data)
{
    struct DDS_SubscriptionBuiltinTopicData dflt_data =
                            DDS_SubscriptionBuiltinTopicData_INITIALIZER;

    topic_data->key = dflt_data.key;
    topic_data->participant_key = dflt_data.participant_key;
    topic_data->topic_name = dflt_data.topic_name;
    topic_data->type_name = dflt_data.type_name;
    topic_data->deadline = dflt_data.deadline;
    topic_data->ownership = dflt_data.ownership;
    topic_data->latency_budget = dflt_data.latency_budget;
    topic_data->reliability = dflt_data.reliability;
    topic_data->liveliness = dflt_data.liveliness;
    topic_data->durability = dflt_data.durability;
    topic_data->destination_order = dflt_data.destination_order;

    DDS_LocatorExSeq_set_length(&topic_data->unicast_locator, 0);
    DDS_LocatorSeq_set_length(&topic_data->multicast_locator, 0);

    topic_data->presentation = dflt_data.presentation;

    DDS_DataRepresentationIdSeq_set_length(&topic_data->representation.value, 0);
    DDS_StringSeq_set_length(&topic_data->partition.name, 0);
    DDS_OctetSeq_set_length(&topic_data->user_data.value, 0);
    DDS_OctetSeq_set_length(&topic_data->group_data.value, 0);
    DDS_OctetSeq_set_length(&topic_data->topic_data.value, 0);

#if DDS_FILTERING_ENABLED
    topic_data->content_filter.content_filtered_topic_name = dflt_data.content_filter.content_filtered_topic_name;
    topic_data->content_filter.related_topic_name = dflt_data.content_filter.related_topic_name;
    topic_data->content_filter.filter_class_name = dflt_data.content_filter.filter_class_name;
    topic_data->content_filter.filter_expression = dflt_data.content_filter.filter_expression;
    DDS_StringSeq_set_length(&topic_data->content_filter.expression_parameters, 0);
#endif
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_deserialize(
                                                struct NDDS_Type_Plugin *plugin,
                                                void *sample,
                                                struct CDR_Stream_t *stream,
                                                DDS_InstanceHandle_t *source)
{
    struct DDS_SubscriptionBuiltinTopicData *topic_data =
                            (struct DDS_SubscriptionBuiltinTopicData *)sample;
    UNUSED_ARG(source);

    return DDS_CdrStream_deserialize_parameter_sequence(
                topic_data, stream,
                (DDS_Cdr_SetDefaultParameterValuesFunction)
                DPDE_SubscriptionBuiltinTopicDataPluginTypeSupport_set_dflt_pv,
                (DDS_Cdr_DeserializeParameterValueFunction)
                DPDE_SubscriptionBuiltinTopicData_deserialize_pv,
                plugin->property.plugin_param);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_serializeKey(
                                                struct NDDS_Type_Plugin *plugin,
                                                struct CDR_Stream_t *stream,
                                                const void* sample_key,
                                                DDS_InstanceHandle_t *destination)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(destination);


    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_deserializeKey(
                                                struct NDDS_Type_Plugin *plugin,
                                                UserDataKeyHolder_t sample_key,
                                                struct CDR_Stream_t *stream,
                                                DDS_InstanceHandle_t *source)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(source);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_instanceToKeyHash(
                                        struct NDDS_Type_Plugin *plugin,
                                        struct CDR_Stream_t *stream,
                                        DDS_KeyHash_t *keyHash,
                                        const void *instance,
                                        DDS_EncapsulationId_t id)
{
    struct DDS_SubscriptionBuiltinTopicData *topic_data =
                        (struct DDS_SubscriptionBuiltinTopicData*)instance;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(id);

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid*)&topic_data->key);
    OSAPI_Memory_copy(keyHash->value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE  RTI_UINT32
DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_key_size(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);

    return RTPS_KEY_HASH_MAX_LENGTH;
}

RTI_PRIVATE struct DDS_TypePluginI DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI;

RTI_PRIVATE RTI_UINT32
DPDE_SubscriptionBuiltinTopicDataCdrPlugin_get_serialized_sample_size(
                                       struct DDS_TypePlugin *tp,
                                       struct DDS_TypeEncapsulationPlugin *wp,
                                       RTI_UINT32 alignment)
{
    UNUSED_ARG(wp);

    return DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(tp,NULL,alignment);
}

RTI_PRIVATE struct DDS_TypeEncapsulationPlugin*
DPDE_SubscriptionBuiltinTopicDataCdrPlugin_create(struct DDS_TypePlugin *tp,
                                                  DDS_DomainParticipant *participant,
                                                  struct DDS_DomainParticipantQos *dp_qos,
                                                  DDS_TypePluginMode_T endpoint_mode,
                                                  DDS_TypePluginEndpoint *endpoint,
                                                  DDS_TypePluginEndpointQos *qos,
                                                  struct DDS_TypeMemoryPlugin *mp)
{
    RTI_UINT32 size = DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(tp,NULL,0);

    return DDS_TypePluginDefaultCdr_create(tp,participant,dp_qos,
                                           endpoint_mode,endpoint,qos,mp,size);
}

RTI_PRIVATE void
DPDE_SubscriptionBuiltinTopicDataCdrPlugin_delete(struct DDS_TypePlugin *p,
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

    if (!DPDE_SubscriptionBuiltinTopicDataTypePlugin_create_sample(&plugin->_parent,&sample))
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

    if (!DPDE_SubscriptionBuiltinTopicDataTypePlugin_delete_sample(&plugin->_parent,sh->sample))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE struct DDS_TypeMemoryPlugin*
DPDE_SubscriptionBuiltinTopicDataHeapPlugin_create(struct DDS_TypePlugin *tp,
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
DPDE_SubscriptionBuiltinTopicDataHeapPlugin_delete(struct DDS_TypePlugin *p,
                                                   struct DDS_TypeMemoryPlugin *mp)
{
    UNUSED_ARG(p);
    UNUSED_ARG(mp);
}

RTI_PRIVATE struct DDS_TypePlugin*
DPDE_SubscriptionBuiltinTopicDataTypePlugin_create_plugin(
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypePluginProperty *const property)
{
    return DDS_TypePluginDefault_create(&DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI,
                                       participant,dp_qos,
                                       endpoint_mode,endpoint,qos,
                                       property);
}

RTI_PRIVATE SHOULD_CHECK_RETURN RTI_BOOL
DPDE_SubscriptionBuiltinTopicDataTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin)
{
    return DDS_TypePluginDefault_delete(plugin);
}

RTI_PRIVATE NDDSCDREncapsulation DPDE_fv_SubscriptionBuiltinTopicDataEncapsulationKind[] =
{
    {
        DDS_ENCAPSULATION_ID_PL_CDR_LE,
        DDS_ENCAPSULATION_ID_PL_CDR_BE,
        0
    }
};

RTI_PRIVATE struct DDS_TypeEncapsulationI DPDE_fv_SubscriptionBuiltinTopicDataCdrPluginI =
{
        DDS_XCDR_DATA_REPRESENTATION,
        NULL,
        DPDE_fv_SubscriptionBuiltinTopicDataEncapsulationKind,
        RTI_MEMORY_TYPE_HEAP,
        RTI_MEMORY_MANAGER_HEAP,
        NULL,
        NULL,
        DDS_TypePluginDefaultCdr_get_buffer,
        DDS_TypePluginDefaultCdr_return_buffer,
        DDS_TypePluginDefaultCdr_get_sample,
        DPDE_SubscriptionBuiltinTopicDataTypePlugin_return_sample,
        DPDE_SubscriptionBuiltinTopicDataTypePlugin_serialize,
        DPDE_SubscriptionBuiltinTopicDataTypePlugin_deserialize,
        DPDE_SubscriptionBuiltinTopicDataCdrPlugin_get_serialized_sample_size,
        DPDE_SubscriptionBuiltinTopicDataCdrPlugin_create, /* create_plugin */
        DPDE_SubscriptionBuiltinTopicDataCdrPlugin_delete /* delete_plugin */
};

RTI_PRIVATE struct DDS_TypeEncapsulationI *DPDE_fv_SubscriptionBuiltinTopicDataWirePlugins[] =
{
    &DPDE_fv_SubscriptionBuiltinTopicDataCdrPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypeMemoryI DPDE_fv_SubscriptionBuiltinTopicDataHeapPluginI =
{
        RTI_MEMORY_MANAGER_HEAP,
        RTI_MEMORY_TYPE_HEAP,
        DPDE_SubscriptionBuiltinTopicDataTypePlugin_create_sample,
        DPDE_SubscriptionBuiltinTopicDataTypePlugin_delete_sample,
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
        DPDE_SubscriptionBuiltinTopicDataHeapPlugin_create, /* create_plugin */
        DPDE_SubscriptionBuiltinTopicDataHeapPlugin_delete /* delete_plugin */
};

RTI_PRIVATE struct DDS_TypeMemoryI *DPDE_fv_SubscriptionBuiltinTopicDataMemoryPlugins[] =
{
    &DPDE_fv_SubscriptionBuiltinTopicDataHeapPluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypePluginI DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI =
{
    /**************************************************************************
     *                   Type information functions
     **************************************************************************/

    NULL,
    NDDS_TYPEPLUGIN_GUID_KEY,
    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,
    NULL,
    RTI_MEMORY_TYPE_HEAP,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_instanceToKeyHash,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_copy_sample,
    NULL,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_serializeKey,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_deserializeKey,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_get_serialized_key_size,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DPDE_fv_SubscriptionBuiltinTopicDataMemoryPlugins,
    DPDE_fv_SubscriptionBuiltinTopicDataWirePlugins,

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
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_create_plugin,
    DPDE_SubscriptionBuiltinTopicDataTypePlugin_delete_plugin,
    NULL,
    NULL
    DDS_TypePluginI_XTYPES_INITIALIZER
};

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant)
{
    return DDS_DomainParticipant_register_type(participant,
                                DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME,
                                &DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI);
}

struct DDS_TypePlugin*
DDS_SubscriptionBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
                        &DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI,
                        participant,dp_qos,
                        endpoint_mode,endpoint,qos,
                        property);
}

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePluginI*
DDS_SubscriptionBuiltinTopicDataTypePlugin_get(void)
{
    return &DPDE_fv_SubscriptionBuiltinTopicDataTypePluginI;
}
