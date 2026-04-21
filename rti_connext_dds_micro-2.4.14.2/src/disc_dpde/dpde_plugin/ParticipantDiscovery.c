/*
 * FILE: ParticipantDiscovery.c - Participant discovery API
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
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "BuiltinCdr.h"
#include "DiscoveryPlugin.h"
#include "ParticipantBuiltinTopicDataPlugin.h"
#include "ParticipantListener.h"
#include "ParticipantDiscovery.h"
#include "PublicationDiscovery.h"
#include "SubscriptionDiscovery.h"

/*** SOURCE_BEGIN ***/

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


    if (DDS_RETCODE_OK != DDS_DataWriter_write(dpde_plugin->participant_writer,
                                               local_participant_data,
                                               &DDS_HANDLE_NIL))
    {
        DPDE_LOG_ANNOUNCEMENT(OSAPI_LOGKIND_ERROR)
    }

    /* If we are sending the initial three, we use the initial announcement
     * period to reschedule the event, otherwise use the regular announcement
     * interval
     */
    if (tdata->count[0] > 0)
    {
        tdata->count[0]--;
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    if (RTI_TRUE != OSAPI_Timer_update_timeout(
            DDS_DomainParticipant_get_timer(
                    DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                    &dpde_plugin->announcement_event,
                    dpde_plugin->properties.participant_liveliness_assert_period.sec,
                    (RTI_INT32)dpde_plugin->properties.participant_liveliness_assert_period.nanosec))

    {
        DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(OSAPI_LOGKIND_ERROR)
    }

    return OSAPI_TIMEOUT_OP_MANUAL;
}

MUST_CHECK_RETURN DDS_ReturnCode_t
DPDE_ParticipantDiscovery_schedule_fast_assertions(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *local_participant_data,
        DDS_Boolean new_event)
{
    struct DPDE_DiscoveryPlugin *const dpde_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
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

    /* Send out initial participant announcement */
    retcode = DDS_DataWriter_write(dpde_plugin->participant_writer,
                         (void *)local_participant_data, &DDS_HANDLE_NIL);

    if (retcode != DDS_RETCODE_OK)
    {
        DPDE_LOG_ANNOUNCE_WRITE(OSAPI_LOGKIND_ERROR,retcode)
        return retcode;
    }
    storage.count[0] = 0;
    storage.count[1] = 0;
    storage.field[0] = (void *)dpde_plugin;
    storage.field[1] = (void *)local_participant_data;

    /* The number of initial announcements already sent is stored in the 
     * storage.count[0] variable passed to the event.  Since we have already
     * sent one announcement, we schedule sending the rest of the
     * initial announcements.
     */
    storage.count[0] =
            (RTI_UINT32)dpde_plugin->properties.initial_participant_announcements - 1U;

    if (new_event)
    {
        if (RTI_TRUE !=
            OSAPI_Timer_create_timeout(
              DDS_DomainParticipant_get_timer(
                 DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                 &dpde_plugin->announcement_event,
                 dpde_plugin->properties.initial_participant_announcement_period.sec,
                 (RTI_INT32)dpde_plugin->properties.initial_participant_announcement_period.nanosec,
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
                      dpde_plugin->properties.participant_liveliness_assert_period.sec,
                      (RTI_INT32)dpde_plugin->properties.participant_liveliness_assert_period.nanosec))
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
    struct NDDS_Type_Plugin *type_plugin;
    struct DDS_DataReaderListener participant_builtin_listener;
    struct DDS_DomainParticipantQos *dp_qos = NULL;
    struct DDS_TopicQos topic_qos = DDS_TopicQos_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Long peer_no,peer_length;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address to_address = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator;

    NETIO_Address_init(&src_writer,NETIO_ADDRESS_KIND_INTRA);
    src_writer.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT);

    type_plugin = DPDE_ParticipantBuiltinTopicDataTypePlugin_get();
    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    if (DDS_RETCODE_OK != DDS_DomainParticipant_register_type(participant,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,
                                (struct NDDS_Type_Plugin *)type_plugin))
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

    peer_length = DDS_StringSeq_get_length(&dp_qos->discovery.initial_peers);

    /* Builtin endpoints have well-known object IDs */
    writer_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT;
    writer_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    writer_qos.history.depth = 1;
    writer_qos.resource_limits.max_instances = 1;
    writer_qos.resource_limits.max_samples = 1;
    writer_qos.resource_limits.max_samples_per_instance = 1;
    writer_qos.writer_resource_limits.max_remote_readers = 1;

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
    if (disc_plugin->properties.max_participant_locators == DDS_LENGTH_AUTO)
    {
        writer_qos.writer_resource_limits.max_routes_per_reader =
                dp_qos->resource_limits.remote_participant_allocation +
                (DPDE_MAX_ANON_PARTICIPANT * (peer_length + 1));
    }
    else
    {
        writer_qos.writer_resource_limits.max_routes_per_reader =
                disc_plugin->properties.max_participant_locators;
    }

    writer_qos.liveliness.kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
    writer_qos.liveliness.lease_duration.sec = DDS_DURATION_INFINITE_SEC;
    writer_qos.liveliness.lease_duration.nanosec = DDS_DURATION_INFINITE_NSEC;
    writer_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    writer_qos.durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    writer_qos.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    writer_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples = 0;
    writer_qos.management.is_anonymous = DDS_BOOLEAN_TRUE;
    writer_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    writer_qos.type_support.plugin_data = (void*)disc_plugin;

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

    reader_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_READER_SDP_PARTICIPANT;
    reader_qos.history.depth = 1;
    reader_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    reader_qos.resource_limits.max_samples = 1;
    reader_qos.resource_limits.max_instances = reader_qos.resource_limits.max_samples;
    reader_qos.resource_limits.max_samples_per_instance = 1;
    reader_qos.liveliness.kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
    reader_qos.liveliness.lease_duration.sec = DDS_DURATION_INFINITE_SEC;
    reader_qos.liveliness.lease_duration.nanosec = DDS_DURATION_INFINITE_NSEC;
    reader_qos.durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    reader_qos.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    reader_qos.reader_resource_limits.instance_replacement = DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS;

    /* All remote participants look like the same reader (same instance) */
    reader_qos.reader_resource_limits.max_remote_writers = 1;

    /* We need to be able to send to all locators across all
     * remote participants.
     */
    reader_qos.reader_resource_limits.max_routes_per_writer =
            dp_qos->resource_limits.remote_participant_allocation *
            disc_plugin->properties.max_locators_per_discovered_participant;

    reader_qos.type_support.plugin_data = (void*)disc_plugin;

    reader_qos.management.is_anonymous = DDS_BOOLEAN_TRUE;
    reader_qos.management.is_hidden = DDS_BOOLEAN_TRUE;

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

    retcode = DDS_RETCODE_OK;

finally:

    return retcode;
}

void
DPDE_ParticipantDiscovery_assert_remote_participant(
                                struct DPDE_DiscoveryPlugin *dpde_plugin,
                                DDS_DomainParticipant *participant,
                                struct DDS_ParticipantBuiltinTopicData *data,
                                struct DDS_SampleInfo *info)
{
    DDS_Long i,j;
    struct RTPS_Guid guid;
    DDS_InstanceHandle_t ih;
    DDS_Boolean is_new;
    struct NETIO_Address dst_address;
    struct NETIO_Address dst_reader;
    struct DDS_Locator *a_locator;
    DDS_ReturnCode_t retcode;
    struct DDS_DataReaderQos *dr_qos;
    struct DDS_DataWriterQos *dw_qos;

    UNUSED_ARG(info);

    /* Ignore ourselves
     */
    ih = DDS_Entity_get_instance_handle(
                                DDS_DomainParticipant_as_entity(participant));
    DDS_InstanceHandle_to_rtps(&guid, &ih);
    if (RTPS_Guid_equals(&guid,
            (struct RTPS_Guid *)&data->key))
    {
        return;
    }

    OSAPI_TRACE_DDS("[DPDE] Assert participant",RTI_FALSE);
    OSAPI_TRACE_GUID("key",&data->key,RTI_TRUE)

    retcode = NDDS_DomainParticipant_assert_remote_participant(
                  participant,data,&is_new);

    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (is_new)
    {
        OSAPI_TRACE_DDS("[DPDE] new participant detected",RTI_FALSE);
        OSAPI_TRACE_GUID("key",&data->key,RTI_FALSE)
        OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

        retcode = NDDS_DomainParticipant_enable_remote_participant_guid(
                        participant, data);

        if (DDS_RETCODE_OK != retcode)
        {
            DPDE_LOG_ENABLE_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR,retcode)
            goto done;
        }

        NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
        dst_reader.port = 0;
        dst_reader.value.rtps_guid.object_id =
                NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

        j = DDS_LocatorSeq_get_length(&data->metatraffic_unicast_locators);

        retcode = DDS_RETCODE_ERROR;

        for (i = 0; i < j; i++)
        {
            a_locator = DDS_LocatorSeq_get_reference(
                                        &data->metatraffic_unicast_locators,i);

            OSAPI_Memory_copy(&dst_address,a_locator,sizeof(struct DDS_Locator));

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

            OSAPI_Memory_copy(&dst_address,a_locator,sizeof(struct DDS_Locator));

            if (!DDS_DataWriter_add_anonymous_route(
                    dpde_plugin->participant_writer,&dst_reader,&dst_address))
            {
                DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }

        if (dpde_plugin->timer_created)
        {
            if (DPDE_ParticipantDiscovery_schedule_fast_assertions(
                    (struct NDDS_Discovery_Plugin *)dpde_plugin,
                    participant,
                    dpde_plugin->participant_builtin_data,
                    DDS_BOOLEAN_FALSE) != DDS_RETCODE_OK)
            {
                DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }

        dr_qos = DDS_DataReader_get_qos_ref(dpde_plugin->publication_reader);
        dw_qos = DDS_DataWriter_get_qos_ref(dpde_plugin->publication_writer);

        if (DDS_DataReader_add_peer(dpde_plugin->publication_reader,&data->key,
                dw_qos,RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                              RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION);
            goto done;
        }

        if (DDS_DataReader_add_peer(dpde_plugin->subscription_reader,&data->key,
                dw_qos,RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                              RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION);
            if (!DDS_DataReader_remove_peer(dpde_plugin->publication_reader,
                                        &data->key,
                                        RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
            {
            }
            goto done;
        }

        if (DDS_DataWriter_add_peer(dpde_plugin->publication_writer,&data->key,
                dr_qos,RTPS_OBJECT_ID_READER_SDP_PUBLICATION) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                              RTPS_OBJECT_ID_READER_SDP_PUBLICATION);
            if (!DDS_DataReader_remove_peer(dpde_plugin->publication_reader,
                                        &data->key,
                                       RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
            {
            }
            if (!DDS_DataReader_remove_peer(dpde_plugin->subscription_reader,
                                        &data->key,
                                       RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION))
            {
            }

            goto done;
        }

        if (DDS_DataWriter_add_peer(dpde_plugin->subscription_writer,&data->key,
                dr_qos,RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                              RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION);
            if (!DDS_DataReader_remove_peer(dpde_plugin->publication_reader,
                                        &data->key,
                                       RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
            {
            }
            if (!DDS_DataReader_remove_peer(dpde_plugin->subscription_reader,
                                        &data->key,
                                        RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION))
            {
            }
            if (!DDS_DataWriter_remove_peer(dpde_plugin->publication_writer,
                                        &data->key,
                                        RTPS_OBJECT_ID_READER_SDP_PUBLICATION))
            {
            }

            goto done;
        }
    }

    retcode = NDDS_DomainParticipant_refresh_remote_participant_liveliness(
            participant, &data->key);

    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_REFRESH_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR,retcode);
        if (!DDS_DataReader_remove_peer(dpde_plugin->publication_reader,&data->key,
                                   RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
        {
        }
        if (!DDS_DataReader_remove_peer(dpde_plugin->subscription_reader,&data->key,
                                   RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION))
        {
        }
        if (!DDS_DataWriter_remove_peer(dpde_plugin->publication_writer,&data->key,
                                   RTPS_OBJECT_ID_READER_SDP_PUBLICATION))
        {
        }
        if (!DDS_DataWriter_remove_peer(dpde_plugin->subscription_writer,&data->key,
                                   RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION))
        {
        }

        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (is_new && (retcode != DDS_RETCODE_OK))
    {
#ifdef RTI_CERT
        retcode = NDDS_DomainParticipant_reset_remote_participant(participant,&data->key);
        IGNORE_RETVAL(retcode);
#else
        if (NDDS_DomainParticipant_remove_remote_participant(participant,&data->key) != DDS_RETCODE_OK)
        {
            DPDE_LOG_REMOVE_PEER(OSAPI_LOGKIND_ERROR,RTPS_OBJECT_ID_PARTICIPANT);
        }
#endif
    }

    return;
}
