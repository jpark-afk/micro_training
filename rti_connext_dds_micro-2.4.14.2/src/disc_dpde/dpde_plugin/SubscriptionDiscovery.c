/*
 * FILE: SubscriptionDiscovery.c - Subscription discovery API
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
 * 23feb2015,eh MICRO-1081 Fix function/var names to conform to coding std
 * 26jan2015,tk MICRO-1028/PR#13473 Removed magic number 0xc0
 * 19mar2014,tk MICRO-74  Support endpoint specific transport
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief Subscription discovery API
 */
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpde_dpdediscovery_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "DiscoveryPlugin.h"
#include "SubscriptionBuiltinTopicDataPlugin.h"
#include "SubscriptionDiscovery.h"
#include "SubscriptionListener.h"

/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data)
{
    struct DPDE_DiscoveryPlugin *disc_plugin =
        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    struct DDS_DataWriterQos writer_qos = DDS_DataWriterQos_INITIALIZER;
    struct DDS_DataReaderQos reader_qos = DDS_DataReaderQos_INITIALIZER;
    struct DDS_DataReaderListener pub_builtin_listener;
    struct DDS_DataWriterListener sub_builtin_listener;
    struct DDS_TopicQos topic_qos = DDS_TopicQos_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_DomainParticipantQos *dp_qos;
    UNUSED_ARG(local_participant_data);

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    disc_plugin->sub_type_plugin = DPDE_SubscriptionBuiltinTopicDataTypePlugin_get();

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipant_register_type(participant,
                DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME,
                (struct NDDS_Type_Plugin *)disc_plugin->sub_type_plugin))
    {
        DPDE_LOG_REGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    /* Builtin endpoints have well-known# object IDs */
    topic_qos.management.is_hidden = DDS_BOOLEAN_TRUE;

    disc_plugin->subscription_topic = DDS_DomainParticipant_create_topic(participant,
                                    DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME,
                                    DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME,
                                    &topic_qos,
                                    NULL,
                                    DDS_STATUS_MASK_NONE);

    if (NULL == disc_plugin->subscription_topic)
    {
        DPDE_LOG_CREATE_TOPIC(OSAPI_LOGKIND_ERROR,
                              DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME,
                              DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    writer_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION;
    writer_qos.resource_limits.max_samples = dp_qos->resource_limits.local_reader_allocation - 3;
    writer_qos.resource_limits.max_instances = dp_qos->resource_limits.local_reader_allocation - 3;
    writer_qos.resource_limits.max_samples_per_instance = 1;
    writer_qos.history.depth = 1;
    writer_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    writer_qos.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    writer_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;

    if (disc_plugin->properties.builtin_writer_heartbeats_per_max_samples == DDS_LENGTH_UNLIMITED)
    {
        writer_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                writer_qos.resource_limits.max_samples;
    }
    else
    {
        writer_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                disc_plugin->properties.builtin_writer_heartbeats_per_max_samples;
    }

    writer_qos.protocol.rtps_reliable_writer.heartbeat_period =
            disc_plugin->properties.builtin_writer_heartbeat_period;

    writer_qos.protocol.rtps_reliable_writer.max_heartbeat_retries =
            disc_plugin->properties.builtin_writer_max_heartbeat_retries;


    writer_qos.protocol.serialize_on_write = disc_plugin->properties.cache_serialized_samples;
    writer_qos.deadline.period.sec = DDS_DURATION_INFINITE_SEC;
    writer_qos.deadline.period.nanosec = DDS_DURATION_INFINITE_NSEC;
    writer_qos.management.is_anonymous = DDS_BOOLEAN_FALSE;
    writer_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    writer_qos.management.disable_unregister_dispose_for_unpublished_instance = DDS_BOOLEAN_TRUE;
    writer_qos.writer_resource_limits.max_remote_readers =
                        dp_qos->resource_limits.remote_participant_allocation;
    writer_qos.writer_resource_limits.max_routes_per_reader =
                disc_plugin->properties.max_locators_per_discovered_participant;

    DPDE_SubscriptionBuiltinDataWriterListener_initialize(
                                    disc_plugin, &sub_builtin_listener);

    disc_plugin->subscription_writer =
        DDS_Publisher_create_datawriter(disc_plugin->publisher,
                disc_plugin->subscription_topic,
                                        &writer_qos,
                                        &sub_builtin_listener,
                                        DDS_PUBLICATION_MATCHED_STATUS);

    if (disc_plugin->subscription_writer == NULL)
    {
        DPDE_LOG_CREATE_WRITER(OSAPI_LOGKIND_ERROR,
                DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(
                                disc_plugin->subscription_topic)))
        goto finally;
    }

    DPDE_SubscriptionBuiltinDataReaderListener_initialize(
                                    disc_plugin,
                                    &pub_builtin_listener);

    reader_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION;
    reader_qos.resource_limits.max_samples =
                disc_plugin->properties.max_samples_per_builtin_endpoint_reader;
    reader_qos.resource_limits.max_samples_per_instance = 1;
    reader_qos.history.depth = 1;
    reader_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    reader_qos.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    reader_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.reader_resource_limits.max_remote_writers_per_instance = 1;
    reader_qos.reader_resource_limits.max_remote_writers =
                dp_qos->resource_limits.remote_participant_allocation;
    reader_qos.reader_resource_limits.max_routes_per_writer =
                disc_plugin->properties.max_locators_per_discovered_participant;
    reader_qos.reader_resource_limits.instance_replacement =
                                DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS;

    if (DDS_MAX_UNLIMITED == disc_plugin->properties.max_samples_per_remote_builtin_endpoint_writer)
    {
        reader_qos.reader_resource_limits.max_samples_per_remote_writer =
                disc_plugin->properties.max_samples_per_builtin_endpoint_reader;
    }
    else
    {
        /* max_samples_per_remote_builtin_writer must be [1,256] based
         * on the DPDE_DiscoveryFactory_initialize
         */
        reader_qos.reader_resource_limits.max_samples_per_remote_writer =
                disc_plugin->properties.max_samples_per_remote_builtin_endpoint_writer;

        reader_qos.resource_limits.max_samples =
                (dp_qos->resource_limits.remote_participant_allocation *
                 reader_qos.reader_resource_limits.max_samples_per_remote_writer);
    }

    /* Make sure that there is sufficient instances to avoid instances
     * from being replaced while committing samples from a remote writer
     * (MICRO-2195)
     */
    if (reader_qos.reader_resource_limits.max_samples_per_remote_writer >
        dp_qos->resource_limits.remote_writer_allocation)
    {
        reader_qos.resource_limits.max_instances =
                            dp_qos->resource_limits.remote_writer_allocation;
    }
    else
    {
        reader_qos.resource_limits.max_instances =
                reader_qos.reader_resource_limits.max_samples_per_remote_writer;
    }


    reader_qos.deadline.period.sec = DDS_DURATION_INFINITE_SEC;
    reader_qos.deadline.period.nanosec = DDS_DURATION_INFINITE_NSEC;
    reader_qos.type_support.plugin_data = (void*)disc_plugin;
    reader_qos.management.is_anonymous = DDS_BOOLEAN_FALSE;
    reader_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    reader_qos.protocol.rtps_reliable_reader.nack_period =
                        disc_plugin->properties.builtin_endpoint_reader_nack_period;


    disc_plugin->subscription_reader =
        DDS_Subscriber_create_datareader(disc_plugin->subscriber,
                                         DDS_Topic_as_topicdescription(
                                               disc_plugin->subscription_topic),
                                         &reader_qos, &pub_builtin_listener,
                                         DDS_STATUS_MASK_ALL);

    if (disc_plugin->subscription_reader == NULL)
    {
        DPDE_LOG_CREATE_READER(OSAPI_LOGKIND_ERROR,
                DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(
                                        disc_plugin->subscription_topic)))
        goto finally;
    }

    retcode = DDS_RETCODE_OK;

finally:

    return retcode;
}

/*ci
 * \brief Check if resources are available for a new DDS DataReader
 *
 * \details
 *
 * If the reservation argument is DDS_BOOLEAN_TRUE the discovery plugin
 * asserts the assigned key for the new DDS DataReader to determine if
 * resources are available. If the assertion succeeds there are sufficient
 * resources. Note that register_instance() cannot be used since that
 * requires an instance and the DDS DataReader has not been created yet.
 *
 * If the reservation argument is DDS_BOOLEAN_FALSE the plugin releases
 * a previous reservation. This is used for example when the creation of a
 * datawriter fails.
 *
 * \param[in] discovery_plugin The discovery plugin instance
 * \param[in] participant      The participant creating the DDS DataReader
 * \param[in] dr_key           The assigned key for the DDS DataReader
 * \param[in] reservation      DDS_BOOLEAN_TRUE if this is a reservation,
 *                             DDS_BOOLEAN_FALSE if this is releasing a
 *                             reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_on_before_local_datareader_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dr_key,
        DDS_Boolean reservation)
{
    struct DPDE_DiscoveryPlugin *const dpde_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    DDS_InstanceHandle_t ih;
    DDS_ReturnCode_t retval;

    UNUSED_ARG(participant);

    if (DDS_BuiltinTopicKey_is_builtin(dr_key))
    {
        return DDS_RETCODE_OK;
    }

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid *)dr_key);

    if (reservation)
    {
        retval = DDS_DataWriter_assert_instance(dpde_plugin->subscription_writer,&ih);
    }
    else
    {
        retval =  DDS_DataWriter_remove_instance(dpde_plugin->subscription_writer,&ih);
    }

    return retval;
}

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_data_reader_deleted(
                    struct NDDS_Discovery_Plugin *const discovery_plugin,
                    DDS_DomainParticipant *const participant,
                    const struct DDS_BuiltinTopicKey_t *local_datareader_key)
{
    struct DPDE_DiscoveryPlugin *const dpde_plugin =
                    (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    DDS_InstanceHandle_t ih;
    DDS_ReturnCode_t retcode;
    UNUSED_ARG(participant);

    /* Do not dispose builtin entities */
    if (DDS_BuiltinTopicKey_is_builtin(local_datareader_key))
    {
        return DDS_RETCODE_OK;
    }

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid *)local_datareader_key);

    /* If the writer is not enabled it is not possible to unregister
     * the instance. In that case just delete the key (assuming
     * it was previously registered in on_before_local_datawriter_created).
     */
    if (!DDS_Entity_is_enabled(
            DDS_DataWriter_as_entity(dpde_plugin->subscription_writer)))
    {
        retcode = DDS_DataWriter_remove_instance(
                                        dpde_plugin->subscription_writer,&ih);
    }
    else
    {
        retcode = DDS_DataWriter_unregister_instance(
                                dpde_plugin->subscription_writer, NULL, &ih);
    }

    if (retcode != DDS_RETCODE_OK)
    {
        DPDE_LOG_DISPOSE_SUBSCRIPTION(OSAPI_LOGKIND_ERROR,retcode);
    }

    return retcode;
}

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_data_reader_enabled(
                        struct NDDS_Discovery_Plugin *const discovery_plugin,
                        DDS_DomainParticipant *const participant,
                        DDS_DataReader *const datareader,
                        const struct DDS_DataReaderQos *const qos)
{
    struct DPDE_DiscoveryPlugin *const dpde_plugin =
                    (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    UNUSED_ARG(participant);
    UNUSED_ARG(qos);

    /* Do not publish anything from our built-in endpoints */
    if ((datareader == dpde_plugin->participant_reader) ||
        (datareader == dpde_plugin->publication_reader) ||
        (datareader == dpde_plugin->subscription_reader))
    {
        return DDS_RETCODE_OK;
    }

    return DDS_DataWriter_write(dpde_plugin->subscription_writer,
                                (void *)datareader, &DDS_HANDLE_NIL);
}
