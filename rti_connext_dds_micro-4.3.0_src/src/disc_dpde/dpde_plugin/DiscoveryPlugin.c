/*
 * FILE: DiscoveryPlugin.c - DPDE Discovery Plugin API
 *
 * (c) Copyright 2011-2026 Real-Time Innovations,
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
 * 07mar2014,tk MICRO-735  Send properties for tools
 * 10sep2013,tk MICRO-696  Fixed deletion of plugin if deleted before
 *                         participant is enabled. This is tested in
 *                         testsuite.
 * 06feb2013,eh MICRO-294: temp increase to 6 participant announcements
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief DPDE Discovery Plugin API
 */
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpde_discovery_plugin_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif

#include "DiscoveryPlugin.h"
#include "ParticipantDiscovery.h"
#include "PublicationDiscovery.h"
#include "SubscriptionDiscovery.h"

/* ------------------------------------------------------------------------ */
/*                   DPDE Discovery plugin                                  */
/* ------------------------------------------------------------------------ */

struct DPDE_DiscoveryFactory
{
    struct RT_ComponentFactory _parent;
    struct DPDE_DiscoveryPluginProperty property;
};

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DPDE_DiscoveryPluginProperty_initialize(struct DPDE_DiscoveryPluginProperty *self)
{
    struct DPDE_DiscoveryPluginProperty v =
                                    DPDE_DiscoveryPluginProperty_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = v;

    return DDS_RETCODE_OK;
}

RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_before_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant* const participant,
        struct DDS_DomainParticipantQos *dp_qos,
        struct DDS_ParticipantBuiltinTopicData *participant_data_out)
{
    struct DPDE_DiscoveryPlugin *const disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    DDS_Long add_writers = 0,
             add_readers = 0,
             add_topics = 0,
             add_types = 0;
    DDS_UnsignedLong add_endp_mask = 0;


    UNUSED_ARG(participant);

    /* SDP endpoints (Participant, Publication, Subscription) */
    add_topics += 3;
    add_writers += 3;
    add_readers += 3;
    add_types += 3;
    add_endp_mask = DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER  |
                    DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR   |
                    DDS_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER  |
                    DDS_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR   |
                    DDS_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER |
                    DDS_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;


    dp_qos->resource_limits.local_writer_allocation =
        dp_qos->resource_limits.local_writer_allocation + add_writers;
    dp_qos->resource_limits.local_reader_allocation =
            dp_qos->resource_limits.local_reader_allocation + add_readers;
    dp_qos->resource_limits.local_topic_allocation =
        dp_qos->resource_limits.local_topic_allocation + add_topics;
    dp_qos->resource_limits.local_type_allocation =
        dp_qos->resource_limits.local_type_allocation + add_types;

    /* The DPDE plug-in has 3 built-in readers/writers pairs, so
     * additional matching resources are used:
     */
    dp_qos->resource_limits.matching_writer_reader_pair_allocation +=
            dp_qos->resource_limits.remote_participant_allocation *
                                                (add_writers + add_readers);

    participant_data_out->dds_builtin_endpoints |= add_endp_mask;


#if DDS_LIVELINESS_CHANNEL_ENABLED
    participant_data_out->participant_message_reader_reliability_kind =
        disc_plugin->properties.participant_message_reader_reliability_kind;
    participant_data_out->builtin_endpoint_qos_mask =
                                       DDS_BUILTIN_ENDPOINT_QOS_BIT_IS_VALID;
    if (participant_data_out->participant_message_reader_reliability_kind ==
                                               DDS_BEST_EFFORT_RELIABILITY_QOS)
    {
        participant_data_out->builtin_endpoint_qos_mask |=
        DDS_BUILTIN_ENDPOINT_QOS_BIT_BEST_EFFORT_PARTICIPANT_MESSAGE_DATA_READER;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    disc_plugin->ignore_unknown_peers =
            dp_qos->discovery.accept_unknown_peers ?
                        DDS_BOOLEAN_FALSE : DDS_BOOLEAN_TRUE;

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const local_participant_data)
{
    struct DPDE_DiscoveryPlugin *const disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct OSAPI_TimeoutHandle tim_hndl = OSAPI_TimeoutHandle_INITIALIZER;

    disc_plugin->participant = participant;
    disc_plugin->local_participant_data = local_participant_data;

    disc_plugin->announcement_event = tim_hndl;
    disc_plugin->publisher =
            DDS_DomainParticipant_get_builtin_publisher(participant);
    if (disc_plugin->publisher == NULL)
    {
        DPDE_LOG_CREATE_PUBLISHER(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    disc_plugin->subscriber =
            DDS_DomainParticipant_get_builtin_subscriber(participant);
    if (disc_plugin->subscriber == NULL)
    {
        DPDE_LOG_CREATE_SUBSCRIBER(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    retcode = DPDE_ParticipantDiscovery_after_local_participant_created(
                        discovery_plugin,
                        participant,
                        local_participant_data);

    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_CREATE_PARTICIPANT_DISCOVERY(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    retcode = DPDE_PublicationDiscovery_after_local_participant_created(
                        discovery_plugin,
                        participant,
                        local_participant_data);

    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_CREATE_PUBLICATION_DISCOVERY(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    retcode = DPDE_SubscriptionDiscovery_after_local_participant_created(
                        discovery_plugin,
                        participant,
                        local_participant_data);

    if (DDS_RETCODE_OK != retcode)
    {
        DPDE_LOG_CREATE_SUBSCRIPTION_DISCOVERY(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    retcode = DDS_RETCODE_OK;

finally:

    if (retcode != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*i dref_Discovery_Plugin_AfterLocalParticipantEnabledCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_participant_enabled(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const loc_dp_data)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    DDS_ReturnCode_t retcode;
    struct DPDE_DiscoveryPlugin *const disc_plugin =
                    (struct DPDE_DiscoveryPlugin *)discovery_plugin;

    if (!DDS_ParticipantBuiltinTopicData_is_equal(
            disc_plugin->local_participant_data, loc_dp_data))
    {
        goto done;
    }

    retcode = DDS_Entity_enable(
                DDS_Topic_as_entity(disc_plugin->participant_topic));
    if (DDS_RETCODE_OK != retcode)
    {
        return DDS_BOOLEAN_FALSE;
    }
    retcode = DDS_Entity_enable(
                DDS_Topic_as_entity(disc_plugin->publication_topic));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    retcode = DDS_Entity_enable(
                DDS_Topic_as_entity(disc_plugin->subscription_topic));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }

    retcode = DDS_Entity_enable(
                DDS_DataReader_as_entity(disc_plugin->participant_reader));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    retcode = DDS_Entity_enable(
                DDS_DataWriter_as_entity(disc_plugin->participant_writer));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }

    {
        DDS_InstanceHandle_t dp_inst = DDS_HANDLE_NIL;

        dp_inst = DDS_DataWriter_register_instance(
                                        disc_plugin->participant_writer,
                                        disc_plugin->local_participant_data);

        if (DDS_InstanceHandle_is_nil(&dp_inst))
        {
            goto done;
        }
    }

    retcode = DDS_Entity_enable(
                DDS_DataReader_as_entity(disc_plugin->publication_reader));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    retcode = DDS_Entity_enable(
                DDS_DataWriter_as_entity(disc_plugin->publication_writer));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }

    retcode = DDS_Entity_enable(
                DDS_DataReader_as_entity(disc_plugin->subscription_reader));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    retcode = DDS_Entity_enable(
                DDS_DataWriter_as_entity(disc_plugin->subscription_writer));
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }


    if (DDS_RETCODE_OK !=
            DPDE_ParticipantDiscovery_schedule_fast_assertions(
                    discovery_plugin, participant, DDS_BOOLEAN_TRUE))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:
    return retval;
}

#ifndef RTI_CERT
SHOULD_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
DPDE_DiscoveryPlugin_shutdown(struct DPDE_DiscoveryPlugin *dpde_plugin,
                              DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Long peer_no,peer_length;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address to_address = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator;
    struct DDS_TypePluginI *tplugin = NULL;

    OSAPI_TRACE_DDS("[DPDE] shutdown started",RTI_TRUE)

    NETIO_Address_init(&src_writer,NETIO_ADDRESS_KIND_INTRA);
    src_writer.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT);

    if (dpde_plugin->timer_created)
    {
        if (!OSAPI_Timer_delete_timeout(
                DDS_DomainParticipant_get_timer(
                        DDS_Subscriber_get_participant(dpde_plugin->subscriber)),
                        &dpde_plugin->announcement_event))
        {
            return retcode;
        }
    }

    OSAPI_TRACE_DDS("[DPDE] delete meta-traffic unicast peers",RTI_TRUE)

    if (dpde_plugin->participant_reader != NULL)
    {
        peer_length = DDS_LocatorSeq_get_length(
                &dpde_plugin->local_participant_data->metatraffic_unicast_locators);
        for (peer_no = 0; peer_no < peer_length; peer_no++)
        {
            a_locator = DDS_LocatorSeq_get_reference(
                    &dpde_plugin->local_participant_data->metatraffic_unicast_locators,
                    peer_no);

            to_address = *(struct NETIO_Address*)a_locator;
            if (!DDS_DataReader_delete_anonymous_route(dpde_plugin->participant_reader,
                    &src_writer,
                    &to_address))
            {
                goto done;
            }
        }

        OSAPI_TRACE_DDS("[DPDE] delete meta-traffic multicast peers",RTI_TRUE)

        peer_length = DDS_LocatorSeq_get_length(
                &dpde_plugin->local_participant_data->metatraffic_multicast_locators);
        for (peer_no = 0; peer_no < peer_length; peer_no++)
        {
            a_locator = DDS_LocatorSeq_get_reference(
                    &dpde_plugin->local_participant_data->metatraffic_multicast_locators,
                    peer_no);

            to_address = *(struct NETIO_Address*)a_locator;
            if (!DDS_DataReader_delete_anonymous_route(dpde_plugin->participant_reader,
                    &src_writer,
                    &to_address))
            {
                goto done;
            }
        }
    }

    /* Remove all the routes between the built-in endpoints
     * datareaders/datawriter
     */
    if (dpde_plugin->publication_reader != NULL)
    {
        if (DDS_DataReader_remove_all_peers(dpde_plugin->publication_reader,
                RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    if (dpde_plugin->subscription_reader != NULL)
    {
        if (DDS_DataReader_remove_all_peers(dpde_plugin->subscription_reader,
                RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    if (dpde_plugin->publication_writer != NULL)
    {
        if (DDS_DataWriter_remove_all_peers(dpde_plugin->publication_writer,
                  RTPS_OBJECT_ID_READER_SDP_PUBLICATION) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    if (dpde_plugin->subscription_writer != NULL)
    {
        if (DDS_DataWriter_remove_all_peers(dpde_plugin->subscription_writer,
                  RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    /* Cannot use delete contained entities */
    if (NULL != dpde_plugin->subscriber)
    {

        if (dpde_plugin->participant_reader != NULL)
        {
            OSAPI_TRACE_DDS("[DPDE] delete participant reader",RTI_TRUE)
            retcode = DDS_Subscriber_delete_datareader(
                                dpde_plugin->subscriber,
                                dpde_plugin->participant_reader);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->participant_reader = NULL;
        }

        if (dpde_plugin->publication_reader)
        {
            OSAPI_TRACE_DDS("[DPDE] delete publication reader",RTI_TRUE)

            retcode = DDS_Subscriber_delete_datareader(
                                dpde_plugin->subscriber,
                                dpde_plugin->publication_reader);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->publication_reader = NULL;
        }

        if (dpde_plugin->subscription_reader)
        {
            OSAPI_TRACE_DDS("[DPDE] delete subscription reader",RTI_TRUE)

            retcode = DDS_Subscriber_delete_datareader(dpde_plugin->subscriber,
                                            dpde_plugin->subscription_reader);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->subscription_reader = NULL;
        }

        /* Builtin Subscriber is now managed by DomainParticipant */
        dpde_plugin->subscriber = NULL;
    }

    if (NULL != dpde_plugin->publisher)
    {

        if (dpde_plugin->participant_writer)
        {
            OSAPI_TRACE_DDS("[DPDE] delete participant writer",RTI_TRUE)

            retcode = DDS_Publisher_delete_datawriter(dpde_plugin->publisher,
                                        dpde_plugin->participant_writer);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->participant_writer = NULL;
        }

        if (dpde_plugin->publication_writer != NULL)
        {
            OSAPI_TRACE_DDS("[DPDE] delete publication writer",RTI_TRUE)

            retcode = DDS_Publisher_delete_datawriter(dpde_plugin->publisher,
                                            dpde_plugin->publication_writer);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->publication_writer = NULL;
        }

        if (dpde_plugin->subscription_writer != NULL)
        {
            OSAPI_TRACE_DDS("[DPDE] delete subscription writer",RTI_TRUE)

            retcode = DDS_Publisher_delete_datawriter(dpde_plugin->publisher,
                                            dpde_plugin->subscription_writer);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
            dpde_plugin->subscription_writer = NULL;
        }

        /* Builtin Publisher is now managed by DomainParticipant */
        dpde_plugin->publisher = NULL;
    }

    if (dpde_plugin->participant_topic != NULL)
    {
        retcode = DDS_DomainParticipant_delete_topic(participant,
                dpde_plugin->participant_topic);
        if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_DELETE_SUBSCRIPTION_TOPIC(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        dpde_plugin->participant_topic = NULL;
    }

    if (dpde_plugin->publication_topic != NULL)
    {
        retcode = DDS_DomainParticipant_delete_topic(participant,
                dpde_plugin->publication_topic);
        if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_DELETE_PUBLICATION_TOPIC(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        dpde_plugin->publication_topic = NULL;
    }

    if (dpde_plugin->subscription_topic != NULL)
    {
        retcode = DDS_DomainParticipant_delete_topic(participant,
                dpde_plugin->subscription_topic);
        if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_DELETE_PARTICIPANT_TOPIC(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        dpde_plugin->subscription_topic = NULL;
    }

    if (dpde_plugin->part_type_plugin != NULL)
    {
        tplugin = DDS_DomainParticipant_unregister_type(
                        participant,
                        DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME);
        if (tplugin != dpde_plugin->part_type_plugin)
        {
             DPDE_LOG_UNREGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                                      DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME);
            goto done;
        }

        dpde_plugin->part_type_plugin = NULL;
    }

    if (dpde_plugin->pub_type_plugin != NULL)
    {
        tplugin = DDS_DomainParticipant_unregister_type(
                        participant,
                        DDS_PUBLICATION_BUILTIN_TOPIC_TYPE_NAME);
        if (tplugin != dpde_plugin->pub_type_plugin)
        {
            DPDE_LOG_UNREGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                                      DDS_PUBLICATION_BUILTIN_TOPIC_TYPE_NAME);
            goto done;
        }

        dpde_plugin->pub_type_plugin = NULL;
    }

    if (dpde_plugin->sub_type_plugin != NULL)
    {
        tplugin = DDS_DomainParticipant_unregister_type(
                        participant,
                        DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME);
        if (tplugin != dpde_plugin->sub_type_plugin)
        {
           DPDE_LOG_UNREGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                                      DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME);
            goto done;
        }

        dpde_plugin->sub_type_plugin = NULL;
    }

    retcode = DDS_RETCODE_OK;

    OSAPI_TRACE_DDS("[DPDE] shutdown complete",RTI_TRUE)

done:
    return retcode;
}
#endif /* !RTI_CERT */


RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_dispose_local_participant(
        struct NDDS_Discovery_Plugin *const plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const local_participant_key)
{

    struct DPDE_DiscoveryPlugin *const self =
                               (struct DPDE_DiscoveryPlugin *)plugin;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(local_participant_key);

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(
                                            self->participant_writer)))
    {
        ih = DDS_Entity_get_instance_handle(
                                DDS_DomainParticipant_as_entity(participant));
        ih.is_valid = DDS_BOOLEAN_TRUE;

        retcode = DDS_DataWriter_unregister_instance(self->participant_writer,
                                                     NULL, &ih);
        if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_DISPOSE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*i dref_Discovery_Plugin_BeforeLocalParticipantDeletedCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_before_local_participant_deleted(
        struct NDDS_Discovery_Plugin *const disc_plugin,
        DDS_DomainParticipant * const participant,
        struct DDS_BuiltinTopicKey_t *const local_participant_key)
{
#ifndef RTI_CERT
    struct DPDE_DiscoveryPlugin *const plugin =
                               (struct DPDE_DiscoveryPlugin *)disc_plugin;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    UNUSED_ARG(local_participant_key);

    retcode = DPDE_DiscoveryPlugin_shutdown(plugin,participant);

    if (retcode != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
#else
    UNUSED_ARG(disc_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(local_participant_key);

    return DDS_BOOLEAN_TRUE;
#endif
}

/* writer-related activities */
/*i dref_Discovery_Plugin_AfterLocalDataWriterEnabledCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_data_writer_enabled(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *const data_writer,
        const struct DDS_DataWriterQos *const qos)
{
    if (DPDE_PublicationDiscovery_after_local_data_writer_enabled(
            discovery_plugin, participant, data_writer, qos) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*i dref_Discovery_Plugin_AfterLocalDataWriterDeletedCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_data_writer_deleted(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *datawriter
        /* const struct DDS_BuiltinTopicKey_t *const local_datawriter_key */)
{
    if (DPDE_PublicationDiscovery_after_local_data_writer_deleted(
            discovery_plugin, participant, datawriter) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/* reader-related activities */
/*i dref_Discovery_Plugin_AfterLocalDataReaderEnabledCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_data_reader_enabled(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataReader *const data_reader,
        const struct DDS_DataReaderQos *const qos)
{
    if (DPDE_SubscriptionDiscovery_after_local_data_reader_enabled(
            discovery_plugin, participant, data_reader, qos) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*i dref_Discovery_Plugin_AfterLocalDataReaderDeletedCallback
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_after_local_data_reader_deleted(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant* const participant,
        DDS_DataReader *datareader
        /* struct DDS_BuiltinTopicKey_t *const local_datareader_key */)
{
    if (DPDE_SubscriptionDiscovery_after_local_data_reader_deleted(
            discovery_plugin, participant, datareader) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Remove a remote participants endpoint peers
 *
 * \param[in] dpde_plugin The discovery plugin
 * \param[in] dp_key      The key of the remote participant
 */
DDS_Boolean
DPDE_DiscoveryPlugin_remove_builtin_peers(
                         struct DPDE_DiscoveryPlugin *const dpde_plugin,
                         const DDS_BuiltinTopicKey_t *const dp_key)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

#ifndef RTI_CERT

    if (DDS_DataReader_remove_peer(
            dpde_plugin->publication_reader,dp_key,
            RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                             RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION);
        goto done;
    }

    if (DDS_DataReader_remove_peer(
            dpde_plugin->subscription_reader,dp_key,
            RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                             RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION);
        goto done;
    }

    if (DDS_DataWriter_remove_peer(
            dpde_plugin->publication_writer,dp_key,
            RTPS_OBJECT_ID_READER_SDP_PUBLICATION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                             RTPS_OBJECT_ID_READER_SDP_PUBLICATION);
        goto done;
    }

    if (DDS_DataWriter_remove_peer(
            dpde_plugin->subscription_writer,dp_key,
            RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                             RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION);
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#else
    UNUSED_ARG(dpde_plugin);
    UNUSED_ARG(dp_key);

    retval = DDS_BOOLEAN_TRUE;
#endif

    return retval;
}

/*i \dref_Discovery_Plugin_on_before_remote_participant_deleted
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_before_remote_participant_deleted(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const remote_participant_data,
        const DDS_RemoteParticipantStatusMask status)
{
#ifdef RTI_CERT
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(remote_participant_data);
    UNUSED_ARG(status);
#else
    struct DPDE_DiscoveryPlugin *const disc_plugin =
                               (struct DPDE_DiscoveryPlugin *)discovery_plugin;
#endif

#ifndef RTI_CERT
    DDS_InstanceHandle_t publication_handle = DDS_HANDLE_NIL;
    DDS_BuiltinTopicKey_t key;

    /* Reset with the core */

    key = remote_participant_data->key;

    if (status & DDS_REMOTE_PARTICIPANT_STATUS_ENABLED)
    {
        key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                        RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION;
        DDS_InstanceHandle_from_rtps(&publication_handle,
                                     (struct RTPS_Guid *)&key);
        DDS_DataReader_liveliness_lost(disc_plugin->publication_reader,
                                                 &publication_handle);

        key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                        RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION;
        DDS_InstanceHandle_from_rtps(&publication_handle,
                                     (struct RTPS_Guid *)&key);
        DDS_DataReader_liveliness_lost(disc_plugin->subscription_reader,
                                                 &publication_handle);
    }


    key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                        RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT;
    DDS_InstanceHandle_from_rtps(&publication_handle,
                                 (struct RTPS_Guid *)&key);
    DDS_DataReader_liveliness_lost(disc_plugin->participant_reader,
                                             &publication_handle);

    if (!DPDE_ParticipantDiscovery_delete_remote_participant_routes(
                discovery_plugin, participant, remote_participant_data))
    {
        DPDE_LOG_DELETE_REMOTE_PARTICIPANT_ROUTES_FAILED(OSAPI_LOGKIND_ERROR)
        return DDS_BOOLEAN_FALSE;
    }

    if (status & DDS_REMOTE_PARTICIPANT_STATUS_ENABLED)
    {
        if (!DPDE_DiscoveryPlugin_remove_builtin_peers(
                                disc_plugin, &remote_participant_data->key))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

#endif /* !RTI_CERT */

    return DDS_BOOLEAN_TRUE;
}

/*i dref_Discovery_Plugin_add_peer
 */
RTI_PRIVATE DDS_Boolean
DPDE_DiscoveryPlugin_add_peer(
                    struct NDDS_Discovery_Plugin *const discovery_plugin,
                    DDS_DomainParticipant *const participant,
                    const char *add_peer)
{
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    struct DPDE_DiscoveryPlugin *const disc_plugin =
                        (struct DPDE_DiscoveryPlugin *)discovery_plugin;
    UNUSED_ARG(participant);

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    if (!DDS_DataWriter_add_anonymous_peer(disc_plugin->participant_writer,
                                           &dst_reader,add_peer))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_is_enabled(DDS_Publisher_as_entity(disc_plugin->publisher)))
    {
        if (DPDE_ParticipantDiscovery_write_announcement(
                disc_plugin, disc_plugin->local_participant_data) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ANNOUNCEMENT(OSAPI_LOGKIND_ERROR)
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE void
DPDE_DiscoveryPlugin_on_assert_remote_participant(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *const data)
{
    struct DPDE_DiscoveryPlugin * const dpde_plugin =
            (struct DPDE_DiscoveryPlugin *const)discovery_plugin;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_DataReaderQos dr_pub_qos = DDS_DataReaderQos_INITIALIZER;
    struct DDS_DataWriterQos dw_pub_qos = DDS_DataWriterQos_INITIALIZER;
    RTI_BOOL add_peer_pub_writer = RTI_FALSE,
             add_peer_pub_reader = RTI_FALSE,
             add_peer_sub_reader = RTI_FALSE;

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

    retcode = DDS_RETCODE_ERROR;

    if (dpde_plugin->timer_created)
    {
        if (!DPDE_ParticipantDiscovery_add_remote_participant_routes(
                (struct NDDS_Discovery_Plugin *)dpde_plugin,
                participant,
                data))
        {
            DPDE_LOG_ADD_REMOTE_PARTICIPANT_ROUTES_FAILED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        if (DPDE_ParticipantDiscovery_schedule_fast_assertions(
                (struct NDDS_Discovery_Plugin *)dpde_plugin,
                participant,
                DDS_BOOLEAN_FALSE) != DDS_RETCODE_OK)
        {
            DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    /* NOTE:
     * These are shallow copies, do not finalize
     */
    DDS_DataReader_get_request_offered_qos(dpde_plugin->publication_reader,
                                           &dr_pub_qos);
    DDS_DataWriter_get_request_offered_qos(dpde_plugin->publication_writer,
                                           &dw_pub_qos);

    if (DDS_DataReader_add_peer(dpde_plugin->publication_reader,&data->key,
            &dw_pub_qos,RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                          RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION);
        goto done;
    }
    add_peer_pub_reader = RTI_TRUE;

    if (DDS_DataReader_add_peer(dpde_plugin->subscription_reader,&data->key,
            &dw_pub_qos,RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                          RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION);
        goto done;
    }
    add_peer_sub_reader = RTI_TRUE;

    if (DDS_DataWriter_add_peer(dpde_plugin->publication_writer,&data->key,
            &dr_pub_qos,RTPS_OBJECT_ID_READER_SDP_PUBLICATION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                          RTPS_OBJECT_ID_READER_SDP_PUBLICATION);
        goto done;
    }
    add_peer_pub_writer = RTI_TRUE;

    if (DDS_DataWriter_add_peer(dpde_plugin->subscription_writer,&data->key,
            &dr_pub_qos,RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION) != DDS_RETCODE_OK)
    {
        DPDE_LOG_ADD_PEER(OSAPI_LOGKIND_ERROR,
                          RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION);

        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK)
    {
        if (add_peer_pub_reader &&
            !DDS_DataReader_remove_peer(dpde_plugin->publication_reader,
                                    &data->key,
                                   RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
        {
        }
        if (add_peer_sub_reader &&
            !DDS_DataReader_remove_peer(dpde_plugin->subscription_reader,
                                    &data->key,
                                    RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION))
        {
        }
        if (add_peer_pub_writer &&
            !DDS_DataWriter_remove_peer(dpde_plugin->publication_writer,
                                    &data->key,
                                    RTPS_OBJECT_ID_READER_SDP_PUBLICATION))
        {
        }

#ifdef RTI_CERT
        retcode = NDDS_DomainParticipant_reset_remote_participant(
                                                participant,&data->key);
        IGNORE_RETVAL(retcode);
#else
        if (NDDS_DomainParticipant_remove_remote_participant(
                            participant, &data->key) != DDS_RETCODE_OK)
        {
            DPDE_LOG_REMOVE_PEER(
                    OSAPI_LOGKIND_ERROR,RTPS_OBJECT_ID_PARTICIPANT);
        }
#endif
    }

    return;
}

/**************************** COMPONENT INTERFACE *****************************/

LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NDDS_DiscoveryI DPDE_DiscoveryPlugin_fv_Intf;

RTI_PRIVATE void
DPDE_DiscoveryPlugin_delete(struct DPDE_DiscoveryPlugin *plugin)
{
#ifndef RTI_CERT
    OSAPI_Heap_free_struct(plugin);
#else
    UNUSED_ARG(plugin);
#endif
}

MUST_CHECK_RETURN RTI_PRIVATE struct DPDE_DiscoveryPlugin*
DPDE_DiscoveryPlugin_create(
                        struct DPDE_DiscoveryFactory *factory,
                        const struct NDDS_Discovery_Property *const property,
                        const struct NDDS_Discovery_Listener *const listener)
{
    /* Variable declarations */
    struct DPDE_DiscoveryPlugin *new_plugin = NULL;
    DDS_Boolean error = DDS_BOOLEAN_TRUE;
    struct DPDE_DiscoveryPluginProperty *plugin_properties = &factory->property;

    /* --- Create the plugin --- */
    OSAPI_Heap_allocate_struct(&new_plugin, struct DPDE_DiscoveryPlugin);

    if (NULL == new_plugin)
    {
        DPDE_LOG_ALLOCATE_DPDE(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    new_plugin->timer_created = RTI_FALSE;

    RT_Component_initialize(&new_plugin->_parent._parent,
                           &DPDE_DiscoveryPlugin_fv_Intf._parent,
                           0,
                           (property ? &property->_parent : NULL),
                           (listener ? &listener->_parent : NULL));

    new_plugin->part_type_plugin = NULL;
    new_plugin->participant_reader = NULL;
    new_plugin->participant_writer = NULL;
    new_plugin->pub_type_plugin = NULL;
    new_plugin->publication_reader = NULL;
    new_plugin->publication_writer = NULL;
    new_plugin->publisher = NULL;
    new_plugin->sub_type_plugin = NULL;
    new_plugin->subscriber = NULL;
    new_plugin->subscription_reader = NULL;
    new_plugin->subscription_writer = NULL;


    /* --- Set properties --- */
    /* 19sep2014,as: DPDE_DiscoveryPluginProperties_copy was removed
     * from our API as part of the clean-up of support methods exposed
     * for each value type. The operation performed only a shallow copy
     * of the data structure, which has been added to this function.
     */
    new_plugin->properties = *plugin_properties;

    /* --- Fill in SystemTime structures --- */
    /* Fill in Time structures with the periods set by the user in properties
     */

    new_plugin->participant_liveliness_assert_period.sec =
        (RTI_INT64)new_plugin->properties.participant_liveliness_assert_period.sec;
    new_plugin->participant_liveliness_assert_period.nanosec =
        new_plugin->properties.participant_liveliness_assert_period.nanosec;

    new_plugin->initial_participant_announcement_period.sec =
        (RTI_INT64)new_plugin->properties.initial_participant_announcement_period.sec;
    new_plugin->initial_participant_announcement_period.nanosec =
        new_plugin->properties.initial_participant_announcement_period.nanosec;

    error = DDS_BOOLEAN_FALSE;

finally:

    if (error)
    {
        new_plugin = NULL;
    }

    return new_plugin;
}

RTI_PRIVATE DDS_ReturnCode_t
DPDE_DiscoveryPlugin_write_announcement(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const participant_data)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    UNUSED_ARG(participant);

    retcode = DPDE_ParticipantDiscovery_write_announcement(
            (struct DPDE_DiscoveryPlugin *)discovery_plugin,
            participant_data);

    return retcode;
}

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NDDS_DiscoveryI DPDE_DiscoveryPlugin_fv_Intf =
{
    RT_COMPONENTI_BASE,
    DPDE_DiscoveryPlugin_before_remote_participant_deleted,
    DPDE_DiscoveryPlugin_before_local_participant_created,
    DPDE_DiscoveryPlugin_after_local_participant_created,
    DPDE_DiscoveryPlugin_after_local_participant_enabled,
    DPDE_DiscoveryPlugin_before_local_participant_deleted,
    DPDE_DiscoveryPlugin_after_local_data_writer_enabled,
    DPDE_DiscoveryPlugin_after_local_data_writer_deleted,
    DPDE_DiscoveryPlugin_after_local_data_reader_enabled,
    DPDE_DiscoveryPlugin_after_local_data_reader_deleted,
    DPDE_DiscoveryPlugin_add_peer,
    DPDE_DiscoveryPlugin_on_assert_remote_participant,
    DPDE_SubscriptionDiscovery_on_before_local_datareader_created,
    DPDE_PublicationDiscovery_on_before_local_datawriter_created,
    DPDE_DiscoveryPlugin_dispose_local_participant,
    DPDE_PublicationDiscovery_on_publication_data_return_loan,
    DPDE_SubscriptionDiscovery_on_subscription_data_return_loan,
    DPDE_DiscoveryPlugin_write_announcement
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
DPDE_DiscoveryFactory_create_component(struct RT_ComponentFactory *factory,
                                       struct RT_ComponentProperty *property,
                                       struct RT_ComponentListener *listener)
{
    struct DPDE_DiscoveryPlugin *retval = NULL;

    retval = DPDE_DiscoveryPlugin_create(
                        (struct DPDE_DiscoveryFactory*)factory,
                        (const struct NDDS_Discovery_Property *)property,
                        (const struct NDDS_Discovery_Listener *)listener);

    return &retval->_parent._parent;
}

RTI_PRIVATE void
DPDE_DiscoveryFactory_delete_component(struct RT_ComponentFactory *factory,
                                       RT_Component_T *component)
{
    struct DPDE_DiscoveryPlugin *self = (struct DPDE_DiscoveryPlugin*)component;
    UNUSED_ARG(factory);

    DPDE_DiscoveryPlugin_delete(self);
}

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DPDE_DiscoveryFactory_initialize(struct RT_ComponentFactoryProperty*property,
                                 struct RT_ComponentFactoryListener *listener);

RTI_PRIVATE void
DPDE_DiscoveryFactory_finalize(struct RT_ComponentFactory *factory,
                               struct RT_ComponentFactoryProperty **property,
                               struct RT_ComponentFactoryListener **listener);

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI DPDE_DiscoveryFactory_fv_Intf =
{
    DPDE_DISCOVERY_INTERFACE_ID,
    DPDE_DiscoveryFactory_initialize,
    DPDE_DiscoveryFactory_finalize,
    DPDE_DiscoveryFactory_create_component,
    DPDE_DiscoveryFactory_delete_component,
    NULL,
    NULL
};

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct DPDE_DiscoveryFactory DPDE_DiscoveryFactory_fv_Factory =
{
  {
     &DPDE_DiscoveryFactory_fv_Intf,
     NULL,
     {{{0,0}}}
  },
  DPDE_DiscoveryPluginProperty_INITIALIZER
};

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DPDE_DiscoveryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                 struct RT_ComponentFactoryListener *listener)
{
    struct DPDE_DiscoveryPluginProperty *dpde_prop;
    UNUSED_ARG(listener);

    DPDE_DiscoveryFactory_fv_Factory._parent._factory =
                                    &DPDE_DiscoveryFactory_fv_Factory._parent;

    if (property != NULL)
    {
        DPDE_DiscoveryFactory_fv_Factory.property =
                    *((struct DPDE_DiscoveryPluginProperty *)property);
    }

    dpde_prop = &DPDE_DiscoveryFactory_fv_Factory.property;

    if (!DDS_Duration_is_normalized(&dpde_prop->participant_liveliness_assert_period))
    {
        return NULL;
    }

    if (!DDS_Duration_is_normalized(&dpde_prop->participant_liveliness_lease_duration))
    {
        return NULL;
    }

    if (!DDS_Duration_is_normalized(&dpde_prop->initial_participant_announcement_period))
    {
        return NULL;
    }

    /* check the DPDE property for consistency */
    if (dpde_prop->initial_participant_announcements < 0)
    {
        return NULL;
    }

    /* check that initial period is [0nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpde_prop->initial_participant_announcement_period,
                             &DDS_DURATION_ZERO) < 0)
    {
        /* initial_participant_announcement_period < 0 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpde_prop->initial_participant_announcement_period,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* initial_participant_announcement_period > 1 year */
        return NULL;
    }


    /* check that regular period is [1nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpde_prop->participant_liveliness_assert_period,
                             &DDS_DURATION_NANOSEC) < 0)
    {
        /* participant_liveliness_assert_period < 1 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpde_prop->participant_liveliness_assert_period,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* participant_liveliness_assert_period > 1 year */
        return NULL;
    }

    /* check that lease duration is [1nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpde_prop->participant_liveliness_lease_duration,
                             &DDS_DURATION_NANOSEC) < 0)
    {
        /* participant_liveliness_lease_duration < 1 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpde_prop->participant_liveliness_lease_duration,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* participant_liveliness_lease_duration > 1 year */
        return NULL;
    }

    /* If initial announcements are requested, but timeout is 0 then
     * return error
     */
    if ((dpde_prop->initial_participant_announcements > 0) &&
        DDS_Duration_is_zero(&dpde_prop->initial_participant_announcement_period))
    {
        return NULL;
    }

    /* If the lease duration is < regular liveliness period return error */
    if (DDS_Duration_compare(&dpde_prop->participant_liveliness_lease_duration,
                         &dpde_prop->participant_liveliness_assert_period) < 0)
    {
        return NULL;
    }

    if ((dpde_prop->max_participant_locators < 1) &&
        (dpde_prop->max_participant_locators != DDS_LENGTH_AUTO))
    {
        return NULL;
    }

    if (dpde_prop->max_locators_per_discovered_participant < 1)
    {
        return NULL;
    }

    if ((dpde_prop->max_samples_per_builtin_endpoint_reader < 1) ||
        (dpde_prop->max_samples_per_builtin_endpoint_reader > 100000000))
    {
        return NULL;
    }

    if ((dpde_prop->max_samples_per_remote_builtin_endpoint_writer != DDS_MAX_UNLIMITED) &&
        ((dpde_prop->max_samples_per_remote_builtin_endpoint_writer < 1) ||
         (dpde_prop->max_samples_per_remote_builtin_endpoint_writer > 256)))
    {
        return NULL;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if ((dpde_prop->participant_message_reader_reliability_kind !=
                                          DDS_RELIABLE_RELIABILITY_QOS) &&
        (dpde_prop->participant_message_reader_reliability_kind !=
                                          DDS_BEST_EFFORT_RELIABILITY_QOS))
    {
        return NULL;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    return &DPDE_DiscoveryFactory_fv_Factory._parent;
}

RTI_PRIVATE void
DPDE_DiscoveryFactory_finalize(struct RT_ComponentFactory *factory,
                               struct RT_ComponentFactoryProperty **property,
                               struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}

MUST_CHECK_RETURN struct RT_ComponentFactoryI*
DPDE_DiscoveryFactory_get_interface(void)
{
    return &DPDE_DiscoveryFactory_fv_Intf;
}
