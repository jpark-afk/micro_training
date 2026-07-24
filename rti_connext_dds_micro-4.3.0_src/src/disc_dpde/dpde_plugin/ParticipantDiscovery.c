/*
 * FILE: ParticipantDiscovery.c - Participant discovery API
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
 * 19mar2014,tk MICRO-74   Support endpoint specific transport
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief Participant discovery API
 */
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "DiscoveryPlugin.h"
#include "ParticipantListener.h"
#include "ParticipantDiscovery.h"
#include "PublicationDiscovery.h"
#include "SubscriptionDiscovery.h"

/*** SOURCE_BEGIN ***/

RTI_PRIVATE RTI_BOOL
DPDE_ParticipantDiscovery_get_datareader_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataReaderQos *reader_qos /* out */)
{
    struct DDS_DomainParticipantQos *dp_qos = NULL;

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    reader_qos->protocol.rtps_object_id = RTPS_OBJECT_ID_READER_SDP_PARTICIPANT;
    reader_qos->history.depth = 1;
    reader_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    reader_qos->resource_limits.max_samples = 1;
    reader_qos->resource_limits.max_instances =
                            reader_qos->resource_limits.max_samples;
    reader_qos->resource_limits.max_samples_per_instance = 1;
    reader_qos->liveliness.kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
    reader_qos->liveliness.lease_duration.sec = DDS_DURATION_INFINITE_SEC;
    reader_qos->liveliness.lease_duration.nanosec = DDS_DURATION_INFINITE_NSEC;
    reader_qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    reader_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    reader_qos->reader_resource_limits.instance_replacement =
                            DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS;

    /* All remote participants look like the same reader (same instance) */
    reader_qos->reader_resource_limits.max_remote_writers = 1;

    /* We need to be able to send to all locators across all
     * remote participants.
     */
    reader_qos->reader_resource_limits.max_routes_per_writer =
            dp_qos->resource_limits.remote_participant_allocation *
            plugin->properties.max_locators_per_discovered_participant;

    reader_qos->type_support.plugin_data = (void*)participant;

    reader_qos->management.is_anonymous = DDS_BOOLEAN_TRUE;
    reader_qos->management.is_hidden = DDS_BOOLEAN_TRUE;
    reader_qos->management.is_announced = DDS_BOOLEAN_FALSE;
    reader_qos->transport_priority.value = dp_qos->discovery.metatraffic_transport_priority;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DPDE_ParticipantDiscovery_get_datawriter_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataWriterQos *writer_qos /* out */)
{
    struct DDS_DomainParticipantQos *dp_qos = NULL;
    DDS_Long peer_length = 0;

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    peer_length = DDS_StringSeq_get_length(&dp_qos->discovery.initial_peers);

    /* Builtin endpoints have well-known object IDs */
    writer_qos->protocol.rtps_object_id = RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT;
    /* Disable flow control and fragmentation for data(p) */
    writer_qos->publish_mode.kind = DDS_SYNCHRONOUS_PUBLISH_MODE_QOS;
    writer_qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    writer_qos->history.depth = 1;
    writer_qos->resource_limits.max_instances = 1;
    writer_qos->resource_limits.max_samples = 1;
    writer_qos->resource_limits.max_samples_per_instance = 1;
    writer_qos->writer_resource_limits.max_remote_readers = 1;

    /* This resource-limit determines the maximum number of addresses
     * the participant will ping for other participants. This is a challenging
     * limit because it has a noticeable effect on memory usage.
     *
     * The challenge is that this limit is set before any of the peers are
     * parsed, and before any additional peers are added with add_peer, thus
     * it is not possible to know how many are needed.
     *
     * The calculation here uses the following logic:
     * - Each remote participant must listen to at least one address
     * - Each peer address can correspond to a maximum of
     *   DPDE_MAX_ANON_PARTICIPANT participants
     */
    if (plugin->properties.max_participant_locators == DDS_LENGTH_AUTO)
    {
        writer_qos->writer_resource_limits.max_routes_per_reader =
                dp_qos->resource_limits.remote_participant_allocation +
                (DPDE_MAX_ANON_PARTICIPANT * (peer_length + 1));
    }
    else
    {
        writer_qos->writer_resource_limits.max_routes_per_reader =
                plugin->properties.max_participant_locators;
    }

    writer_qos->liveliness.kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
    writer_qos->liveliness.lease_duration.sec = DDS_DURATION_INFINITE_SEC;
    writer_qos->liveliness.lease_duration.nanosec = DDS_DURATION_INFINITE_NSEC;
    writer_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    writer_qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    writer_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    writer_qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples = 0;
    writer_qos->management.is_anonymous = DDS_BOOLEAN_TRUE;
    writer_qos->management.is_hidden = DDS_BOOLEAN_TRUE;
    writer_qos->management.is_announced = DDS_BOOLEAN_FALSE;
    writer_qos->type_support.plugin_data = (void*)participant;
    writer_qos->transport_priority.value = dp_qos->discovery.metatraffic_transport_priority;

    return RTI_TRUE;
}

DDS_ReturnCode_t
DPDE_ParticipantDiscovery_write_announcement(
        struct DPDE_DiscoveryPlugin *dpde_plugin,
        struct DDS_ParticipantBuiltinTopicData *participant_data)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    retcode = DDS_DataWriter_write(dpde_plugin->participant_writer,
                                   participant_data,
                                   &DDS_HANDLE_NIL);
    
#if OSAPI_ENABLE_LOG
    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_ANNOUNCE_WRITE(OSAPI_LOGKIND_ERROR,retcode)
    }
#endif

    return retcode;
}


/* Callback called when it is time to assert the local participant.
 *
 * The core participant does not know anything about how and where it is
 * announced. It is up to the plugin to handle this based on the confguration
 * of the discovery plugin
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
DPDE_ParticipantDiscovery_assert_participant(struct OSAPI_TimeoutUserData *tdata)
{
    struct DPDE_DiscoveryPlugin *dpde_plugin =
                                (struct DPDE_DiscoveryPlugin *)tdata->field[0];
    void *local_participant_data = tdata->field[1];
    struct DDS_Duration_t next_duration;

    if (DPDE_ParticipantDiscovery_write_announcement(dpde_plugin,
                                    local_participant_data) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ANNOUNCEMENT(OSAPI_LOGKIND_ERROR)
    }

    /* If we are sending the initial data(p), we use the initial announcement
     * period to reschedule the event, otherwise use the regular announcement
     * interval
     */
    if (dpde_plugin->initial_participant_announcements_counter > 1)
    {
        next_duration = dpde_plugin->properties.initial_participant_announcement_period;
    }
    else
    {
        next_duration = dpde_plugin->properties.participant_liveliness_assert_period;
    }

    if (dpde_plugin->initial_participant_announcements_counter > 0)
    {
        dpde_plugin->initial_participant_announcements_counter--;
    }

    if (RTI_TRUE != OSAPI_Timer_update_timeout(
            DDS_DomainParticipant_get_timer(
                    DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                    &dpde_plugin->announcement_event,
                    next_duration.sec,
                    (RTI_INT32)next_duration.nanosec))

    {
        DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(OSAPI_LOGKIND_ERROR)
    }

    return OSAPI_TIMEOUT_OP_MANUAL;
}

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_ParticipantDiscovery_schedule_fast_assertions(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_Boolean new_event)
{
    struct DPDE_DiscoveryPlugin *const dpde_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    struct OSAPI_TimeoutUserData storage;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_Duration_t next_duration;
    UNUSED_ARG(participant);

    OSAPI_TRACE_DDS("[DPDE] Schedule participant assertions",RTI_TRUE)

    if (new_event)
    {
        /* Advance the sequence number the first time */
        retcode = DDS_DataWriter_advance_sn(dpde_plugin->participant_writer);
        if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_ADVANCE_SN(OSAPI_LOGKIND_ERROR)
            return retcode;
        }
    }

    /* Always send out the first participant announcements */
    if (DPDE_ParticipantDiscovery_write_announcement(
                    dpde_plugin,
                    dpde_plugin->local_participant_data) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ANNOUNCEMENT(OSAPI_LOGKIND_ERROR);
        return DDS_RETCODE_ERROR;
    }

    storage.count[0] = 0;
    storage.count[1] = 0;
    storage.field[0] = (void *)dpde_plugin;
    storage.field[1] = (void *)dpde_plugin->local_participant_data;

    /* The number of initial announcements already sent is stored in the
     * initial_participant_announcements_counter.  Since we have already
     * sent one announcement, we schedule sending the rest of the
     * initial announcements.
     */
    if (dpde_plugin->properties.initial_participant_announcements > 1)
    {
        dpde_plugin->initial_participant_announcements_counter =
                dpde_plugin->properties.initial_participant_announcements - 1;
        next_duration = dpde_plugin->properties.initial_participant_announcement_period;
    }
    else
    {
        dpde_plugin->initial_participant_announcements_counter = 0;
        next_duration = dpde_plugin->properties.participant_liveliness_assert_period;
    }

    if (new_event)
    {
        if (RTI_TRUE !=
            OSAPI_Timer_create_timeout(
              DDS_DomainParticipant_get_timer(
                 DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                 &dpde_plugin->announcement_event,
                 next_duration.sec,
                 (RTI_INT32)next_duration.nanosec,
                 OSAPI_TIMER_PERIODIC,
                 DPDE_ParticipantDiscovery_assert_participant,
                 &storage))
        {
            DPDE_LOG_SCHEDULE_FAST_ASSERTION(OSAPI_LOGKIND_ERROR)
            goto finally;
        }
        dpde_plugin->timer_created = RTI_TRUE;
    }
    else
    {
        /* Confirm that the announcement has been posted */
        if (dpde_plugin->announcement_event.epoch > 0)
        {
            if (RTI_TRUE !=
                OSAPI_Timer_update_timeout(
                   DDS_DomainParticipant_get_timer(
                      DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                      &dpde_plugin->announcement_event,
                      next_duration.sec,
                      (RTI_INT32)next_duration.nanosec))
            {
                DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(OSAPI_LOGKIND_ERROR)
                goto finally;
            }
        }
    }

    retcode = DDS_RETCODE_OK;

finally:
    return retcode;
}

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_ParticipantDiscovery_after_local_participant_created(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data)
{
    struct DPDE_DiscoveryPlugin *disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    struct DDS_DataWriterQos writer_qos = DDS_DataWriterQos_INITIALIZER;
    struct DDS_DataReaderQos reader_qos = DDS_DataReaderQos_INITIALIZER;
    struct DDS_DataReaderListener participant_builtin_listener;
    struct DDS_TopicQos topic_qos = DDS_TopicQos_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Long peer_no = 0,
             peer_length = 0;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address to_address = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator;

    NETIO_Address_init(&src_writer,NETIO_ADDRESS_KIND_INTRA);
    src_writer.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT);

    disc_plugin->part_type_plugin = DDS_ParticipantBuiltinTopicDataTypePlugin_get();

    if (DDS_RETCODE_OK != DDS_DomainParticipant_register_type(participant,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,
                                disc_plugin->part_type_plugin))
    {
        DPDE_LOG_REGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                               DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    topic_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    disc_plugin->participant_topic = DDS_DomainParticipant_create_topic(participant,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_NAME,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,
                                &topic_qos, NULL,
                                DDS_STATUS_MASK_NONE);
    if (NULL == disc_plugin->participant_topic)
    {
        DPDE_LOG_CREATE_TOPIC(OSAPI_LOGKIND_ERROR,
                              DDS_PARTICIPANT_BUILTIN_TOPIC_NAME,
                              DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    if (!DPDE_ParticipantDiscovery_get_datawriter_qos(
            disc_plugin, participant, &writer_qos))
    {
        goto finally;
    }

    disc_plugin->participant_writer = DDS_Publisher_create_datawriter(
           disc_plugin->publisher,disc_plugin->participant_topic,
           &writer_qos,NULL,DDS_STATUS_MASK_NONE);

    if (disc_plugin->participant_writer == NULL)
    {
        DPDE_LOG_CREATE_WRITER(OSAPI_LOGKIND_ERROR,
          DDS_TopicDescription_get_name(
                  DDS_Topic_as_topicdescription(
                          disc_plugin->participant_topic)))
        goto finally;
    }

    DPDE_ParticipantBuiltinDataReaderListener_initialize(
                                (struct NDDS_Discovery_Plugin *)disc_plugin,
                                &participant_builtin_listener);

    if (!DPDE_ParticipantDiscovery_get_datareader_qos(
            disc_plugin, participant, &reader_qos))
    {
        goto finally;
    }

    disc_plugin->participant_reader = DDS_Subscriber_create_datareader(
                                         disc_plugin->subscriber,
             DDS_Topic_as_topicdescription(disc_plugin->participant_topic),
                                         &reader_qos,
                                         &participant_builtin_listener,
                                         DDS_STATUS_MASK_ALL);

    if (disc_plugin->participant_reader == NULL)
    {
        DPDE_LOG_CREATE_READER(OSAPI_LOGKIND_ERROR,
                DDS_TopicDescription_get_name(
                                DDS_Topic_as_topicdescription(
                                        disc_plugin->participant_topic)))
        goto finally;
    }

    peer_length = DDS_LocatorSeq_get_length(
                        &local_participant_data->metatraffic_unicast_locators);
    for (peer_no = 0; peer_no < peer_length; peer_no++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                &local_participant_data->metatraffic_unicast_locators,peer_no);

        to_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataReader_add_anonymous_route(disc_plugin->participant_reader,
                &src_writer,
                &to_address))
        {
            goto finally;
        }
    }

    peer_length = DDS_LocatorSeq_get_length(
                        &local_participant_data->metatraffic_multicast_locators);
    for (peer_no = 0; peer_no < peer_length; peer_no++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                &local_participant_data->metatraffic_multicast_locators,peer_no);

        to_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataReader_add_anonymous_route(disc_plugin->participant_reader,
                &src_writer,
                &to_address))
        {
            goto finally;
        }
    }

    /* Set discovery-specific information in the participant's builtin topic
     * data.
     */
    local_participant_data->liveliness_lease_duration =
        disc_plugin->properties.participant_liveliness_lease_duration;

    if (!DDS_DomainParticipant_is_mtu_greater_than_builtindata(
        participant,
        disc_plugin->participant_writer,
        local_participant_data))
    {
        goto finally;
    }

    retcode = DDS_RETCODE_OK;

finally:

    return retcode;
}

RTI_BOOL
DPDE_ParticipantDiscovery_add_remote_participant_routes(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *const data)
{
    RTI_BOOL retval = RTI_FALSE;
    struct NETIO_Address dst_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator = NULL;
    RTI_INT32 i = 0,
              j = 0;
    struct DPDE_DiscoveryPlugin * const dpde_plugin =
            (struct DPDE_DiscoveryPlugin *const)discovery_plugin;

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.port = 0;
    dst_reader.value.rtps_guid.object_id =
            NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    j = DDS_LocatorSeq_get_length(&data->metatraffic_unicast_locators);

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                                    &data->metatraffic_unicast_locators,i);
        dst_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataWriter_add_anonymous_route(
                dpde_plugin->participant_writer,&dst_reader,&dst_address))
        {
            DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    j = DDS_LocatorSeq_get_length(&data->metatraffic_multicast_locators);

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                                  &data->metatraffic_multicast_locators,i);
        dst_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataWriter_add_anonymous_route(
                dpde_plugin->participant_writer,&dst_reader,&dst_address))
        {
            DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        /* try to clean up any route that might have been added */
        if (!DPDE_ParticipantDiscovery_delete_remote_participant_routes(
                discovery_plugin, participant, data))
        {
            DPDE_LOG_DELETE_REMOTE_PARTICIPANT_ROUTES_FAILED(
                                                OSAPI_LOGKIND_ERROR)
        }
    }

    return retval;
}


RTI_BOOL
DPDE_ParticipantDiscovery_delete_remote_participant_routes(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *const data)
{
    struct NETIO_Address dst_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator = NULL;
    RTI_INT32 i = 0,
              j = 0;
    struct DPDE_DiscoveryPlugin * const dpde_plugin =
            (struct DPDE_DiscoveryPlugin *const)discovery_plugin;

    UNUSED_ARG(participant);

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.port = 0;
    dst_reader.value.rtps_guid.object_id =
            NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    j = DDS_LocatorSeq_get_length(&data->metatraffic_unicast_locators);

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                                    &data->metatraffic_unicast_locators,i);
        dst_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataWriter_delete_anonymous_route(
                dpde_plugin->participant_writer,&dst_reader,&dst_address))
        {
            DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_WARNING)
            /* we still try to remove all routes */
        }
    }

    j = DDS_LocatorSeq_get_length(&data->metatraffic_multicast_locators);

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(
                                  &data->metatraffic_multicast_locators,i);
        dst_address = *(struct NETIO_Address*)a_locator;

        if (!DDS_DataWriter_delete_anonymous_route(
                dpde_plugin->participant_writer,&dst_reader,&dst_address))
        {
            DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_WARNING)
            /* we still try to remove all routes */
        }
    }

    return RTI_TRUE;
}

void
DPDE_ParticipantDiscovery_assert_remote_participant(
        struct DPDE_DiscoveryPlugin *dpde_plugin,
        DDS_DomainParticipant *participant,
        struct DDS_ParticipantBuiltinTopicData *data,
        struct DDS_SampleInfo *info)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_BuiltinTopicKey_t local_dp_key = DDS_BUILTINTOPICKEY_UNKNOWN;
    DDS_InstanceHandle_t ih;
    DDS_RemoteParticipantStatusMask status;
    UNUSED_ARG(info);

    /* Ignore ourselves */
    ih = DDS_Entity_get_instance_handle(
            DDS_DomainParticipant_as_entity(participant));
    DDS_BuiltinTopicKey_from_instance_handle(&local_dp_key, &ih);
    if (DDS_BuiltinTopicKey_compare(&local_dp_key,&data->key) == 0)
    {
        return;
    }

    OSAPI_TRACE_DDS("[DPDE] Assert participant",RTI_FALSE);
    OSAPI_TRACE_GUID("key",&data->key,RTI_TRUE)

    if (DDS_RETCODE_OK != NDDS_DomainParticipant_assert_remote_participant(
                                                participant, data, &status))
    {
        DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
        return;
    }

    if (status & DDS_REMOTE_PARTICIPANT_STATUS_ENABLED)
    {
        retcode =
                NDDS_DomainParticipant_refresh_remote_participant_liveliness(
                    participant, &data->key);
        if (DDS_RETCODE_OK != retcode)
        {
            DPDE_LOG_REFRESH_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR,retcode);
            if (!DPDE_DiscoveryPlugin_remove_builtin_peers(
                    dpde_plugin, &data->key))
            {
            }
        }
    }
}
