/*
 * FILE: DPSEDiscoveryPlugin.c - DPSE Discovery plugin API
 *
 * Copyright (c) 2011-2026 Real-Time Innovations, Inc.
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
 * 11sep2015,tk MICRO-1499/PR#16514 Fixed issues when resetting a remote
 *                                  participant that prevented it from being
 *                                  rediscovered.
 * 28jul2015,tk MICRO-1471/PR#15633 Added robustness check that deadline and
 *                                  liveliness durations are normalized
 * 27jul2015,tk Due to change in dispose API on writer side, always unregister
 * 20jul2015,tk MICRO-1445/PR#15492 Do not add 1 to the
 *                                  matching_reader_writer_pair_allocation as
 *                                  since will allow one addition matching to
 *                                  occur. This is to meet the HLR
 * 14jul2015,tk MICRO-1420/PR#15357 Removed unnecessary participant resource
 *                                  allocations.
 * 09jul2015,tk MICRO-1392/PR#15232 Added missing consistency check
 * 09jul2015,tk MICRO-1394/PR#15242 Use initial_announcement_count instead of
 *                                  timer storage to switch between initial and
 *                                  regular announcement period
 * 02jul2015,tk MICRO-1292/PR#14955 Removed misleading comment from the factory
 * 02jul2015,tk MICRO-1292/PR#14955 Removed misleading comment from the factory
 *                                  initialize() method
 * 29jun2015,tk MICRO-1367/PR#15181 Install the nil listener on the publisher
 *                                  /subscriber to prevent propagation to the
 *                                  participant.
 * 11jun2015,tk MICRO-1292/PR#14955 Added consistency checks
 * 11jun2015,tk MICRO-1264/PR#14864 Added robustness check on key kind
 * 28may2015,tk MICRO-1263/PR#14863 Re-factored for clarity and consistency
 * 28may2015,tk MICRO-1261/PR#14861 Added robustness checks to
 *                                  RemoteParticipant_assert
 *                                  RemotePublication_assert
 *                                  RemoteSubscription_assert
 * 05jan2014,tk Updated log-codes
 * 14nov2014,eh MICRO-974 Set dds_builtin_endpoints for participant announcer
 *              and detector
 * 31jul2014,tk MICRO-172/PR#1064 - Removed superfluous REDA_Indexer fields
 * 05may2014,as MICRO-270 Always enable precondition
 *              checks for public API operations
 * 07mar2014,tk MICRO-735: Send properties for tools
 * 10sep2013,tk MICRO-696: Fixed deletion of plugin if deleted before
 *                         participant is enabled. This is tested in
 *                         testsuite.
 * 07aug2013,tk MICRO-486: multicast discovery
 *              MICRO-250: multicast user-data
 * 06feb2013,eh Temp fix for MICRO-294: announce to 6 participants\
 * 30apr2011,tk Written
 */
/*ce
 * \file
 * \brief DPSE Discovery plugin API
 *
 * \details
 * This file implements the discovery plugin methods for the DPSE discovery
 * plugin. It is primarily concerned with the life-cycle and responding to
 * listener callbacks from a participant.
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#include "DPSEDiscoveryPlugin.h"
#include "DPSECdr.h"
#include "DPSEParticipantBuiltinTopicData.h"
#include "DPSEParticipantDiscovery.h"
#include "DPSEParticipantListener.h"

/* ------------------------------------------------------------------------ */
/*                   DPSE Discovery plugin                                  */
/* ------------------------------------------------------------------------ */

struct DPSE_Discovery_Factory
{
    struct RT_ComponentFactory _parent;
    struct DPSE_DiscoveryPluginProperty property;
};

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DPSE_DiscoveryPluginProperty_initialize(struct DPSE_DiscoveryPluginProperty *self)
{
    struct DPSE_DiscoveryPluginProperty v =
                                    DPSE_DiscoveryPluginProperty_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = v;

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Assert a remote participant into a participant
 *
 * \details
 * Assert a remote participant into the participant. It is legal to assert a
 * remote participant multiple times. Note that asserting a remote participant
 * does not automatically enable it. Either \ref
 * NDDS_DomainParticipant_enable_remote_participant_name or \ref
 * NDDS_DomainParticipant_enable_remote_participant_guid must be called for
 * that. Matching of a remote participant's endpoints with local endpoints
 * does not occur until the remote participant is enabled.
 *
 * NOTE: NDDS_DomainParticipant_assert_remote_participant can indicate whether
 *       the participant exists or not, this function ignores that result.
 *
 * \param[in]    participant The participant to assert the remote participant in
 * \param[in]    data        Discovery data for the remote participant
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
RTI_PRIVATE DDS_ReturnCode_t
DPSE_RemoteParticipant_assert_builtin(
                            DDS_DomainParticipant *const participant,
                            struct DDS_ParticipantBuiltinTopicData *const data)
{
    DDS_RemoteParticipantStatusMask status = 0;

    OSAPI_PRECONDITION(participant == NULL,
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("participant",
                               participant,RTI_TRUE);)

    return NDDS_DomainParticipant_assert_remote_participant(participant, data,
                                                            &status);
}

DDS_ReturnCode_t
DPSE_RemoteParticipant_assert(DDS_DomainParticipant *const participant,
                              const char *rem_participant_name)
{
    struct DDS_ParticipantBuiltinTopicData remote_part_data =
                                  DDS_ParticipantBuiltinTopicData_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS((participant == NULL) ||
                           (rem_participant_name == NULL) ||
                           (REDA_String_length(rem_participant_name) >
                            DDS_ENTITYNAME_QOS_NAME_MAX),
                            return DDS_RETCODE_BAD_PARAMETER,
                 OSAPI_Log_entry_add_pointer("participant",
                                             participant,RTI_FALSE);
                 OSAPI_Log_entry_add_pointer("rem_participant_name",
                                             rem_participant_name,RTI_TRUE);)

    OSAPI_Memory_copy(remote_part_data.participant_name.name,
                      rem_participant_name,
                      REDA_String_length(rem_participant_name)+1);

    return DPSE_RemoteParticipant_assert_builtin(participant,
                                                 &remote_part_data);
}

DDS_ReturnCode_t
DPSE_RemotePublication_assert(DDS_DomainParticipant *const participant,
                      const char *const rem_participant_name,
                      const struct DDS_PublicationBuiltinTopicData *const data,
                      NDDS_TypePluginKeyKind key_kind)
{
    OSAPI_PRECONDITION_ALWAYS((participant == NULL) ||
                           (rem_participant_name == NULL) ||
                           (data == NULL) ||
                           (data->topic_name == NULL) ||
                           (data->type_name == NULL)  ||
                           ((key_kind != NDDS_TYPEPLUGIN_NO_KEY) &&
                            (key_kind != NDDS_TYPEPLUGIN_USER_KEY) &&
                            (key_kind != NDDS_TYPEPLUGIN_GUID_KEY)) ||
                           (REDA_String_length(rem_participant_name) >
                            DDS_ENTITYNAME_QOS_NAME_MAX) ||
                           (REDA_String_length(data->topic_name) >
                            RTPS_PATHNAME_LEN_MAX) ||
                           (REDA_String_length(data->type_name) >
                            RTPS_PATHNAME_LEN_MAX),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("rem_participant_name",rem_participant_name,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("data",data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("data->topic_name",
            data != NULL ? data->topic_name : NULL,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("data->type_name",
            data != NULL ? data->type_name : NULL,RTI_TRUE);)

    return NDDS_DomainParticipant_assert_remote_publication(
                      participant,rem_participant_name,data,key_kind);

}

DDS_ReturnCode_t
DPSE_RemoteSubscription_assert(DDS_DomainParticipant *const participant,
                const char *const rem_participant_name,
                const struct DDS_SubscriptionBuiltinTopicData *const data,
                NDDS_TypePluginKeyKind key_kind)
{
    OSAPI_PRECONDITION_ALWAYS((participant == NULL) ||
                           (rem_participant_name == NULL) ||
                           (data == NULL) ||
                           (data->topic_name == NULL) ||
                           (data->type_name == NULL)  ||
                           ((key_kind != NDDS_TYPEPLUGIN_NO_KEY) &&
                            (key_kind != NDDS_TYPEPLUGIN_USER_KEY) &&
                            (key_kind != NDDS_TYPEPLUGIN_GUID_KEY)) ||
                           (REDA_String_length(rem_participant_name) >
                            DDS_ENTITYNAME_QOS_NAME_MAX) ||
                           (REDA_String_length(data->topic_name) >
                            RTPS_PATHNAME_LEN_MAX) ||
                           (REDA_String_length(data->type_name) >
                            RTPS_PATHNAME_LEN_MAX),
                            return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("rem_participant_name",rem_participant_name,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("data",data,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("data->topic_name",
                            data != NULL ? data->topic_name : NULL,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("data->type_name",
                            data != NULL ? data->type_name : NULL,RTI_TRUE);)

    return NDDS_DomainParticipant_assert_remote_subscription(participant,
                                                             rem_participant_name,
                                                             data,
                                                             key_kind);
}

/*ci
 * \brief  REDA_Indexer_T compare function for participant names
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just key
 * \param[in] key A         An existing indexed record or a key
 *
 * \return zero if record = key,
 *         negative integer if record < key,
 *         positive integer if record > key
 */
RTI_PRIVATE RTI_INT32
DPSE_DiscoveryPlugin_compare_name(const void *const record,
                                  RTI_BOOL key_is_record,
                                  const void *const key)
{
    UNUSED_ARG(key_is_record);

    return REDA_String_compare(record,key);
}

/*ci
 * \brief  REDA_Indexer_T compare function for participant names
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just key
 * \param[in] key A         An existing indexed record or a key
 *
 * \return zero if record = key,
 *         negative integer if record < key,
 *         positive integer if record > key
 */
RTI_PRIVATE RTI_INT32
DPSE_DiscoveryPlugin_compare_key(const void *const record,
                                  RTI_BOOL key_is_record,
                                  const void *const key)
{
    UNUSED_ARG(key_is_record);

    return DDS_BuiltinTopicKey_compare(record,key);
}

/*ci
 * \brief Called by the participant before it is created and initialized.
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_before_local_participant_created.
 * When a participant creates an instance of the discovery plugin it calls
 * this function before allocating any resources. The plugin is allowed
 * to modify the participant qos to take its own resource needs into
 * account.
 *
 * \param[in]    discovery_plugin     The DPSE plugin
 * \param[in]    participant          The participant that created the plugin
 * \param[inout] dp_qos               The participant's qos policy
 * \param[inout] participant_data_out The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_before_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant * const participant,
        struct DDS_DomainParticipantQos *dp_qos,
        struct DDS_ParticipantBuiltinTopicData *participant_data_out)
{
    struct DPSE_DiscoveryPlugin *disc_plugin =
                (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    struct REDA_IndexerProperty iprop = REDA_IndexerProperty_INITIALIZER;
    struct REDA_BufferPoolProperty bprop;

    UNUSED_ARG(participant);

    dp_qos->resource_limits.local_writer_allocation =
        dp_qos->resource_limits.local_writer_allocation + 1;
    dp_qos->resource_limits.local_reader_allocation =
        dp_qos->resource_limits.local_reader_allocation + 1;
    dp_qos->resource_limits.local_publisher_allocation =
        dp_qos->resource_limits.local_publisher_allocation + 1;
    dp_qos->resource_limits.local_subscriber_allocation =
        dp_qos->resource_limits.local_subscriber_allocation + 1;
    dp_qos->resource_limits.local_topic_allocation =
        dp_qos->resource_limits.local_topic_allocation + 1;
    dp_qos->resource_limits.local_type_allocation =
        dp_qos->resource_limits.local_type_allocation + 1;

    /* The DPSE plug-in has 1 built-in reader/writer pair and
     * additional matching resources must be allocated. Add 1 match for
     * writer->reader. The resource is used for the anonymous reader.
     */
    dp_qos->resource_limits.matching_writer_reader_pair_allocation += 1;

    /* NOTE: RTI's implementation of RTPS does not use the
     *       matching_reader_writer_pair_allocation. In order
     *       to be 100% accurate with the HLR for Cert, this resource
     *       limit is not incremented.
     */

    /* NOTE: Micro only supports one participant plugin. Thus, assign
     *       the supported built-in endpoints instead of ORing.
     */
    participant_data_out->dds_builtin_endpoints =
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    bprop.buffer_size = sizeof(struct DPSE_AssertedParticipant);
    bprop.flags = 0;
    bprop.max_buffers = (RTI_SIZE_T)dp_qos->resource_limits.remote_participant_allocation;
    disc_plugin->asserted_participants =
                               REDA_BufferPool_new("asserted_participants",
                                                   &bprop,NULL,NULL,NULL,NULL);

    if (disc_plugin->asserted_participants == NULL)
    {
        DPSE_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DPSE_PARTICIPANTMAMES_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    iprop.max_entries = dp_qos->resource_limits.remote_participant_allocation;
    disc_plugin->name_index = REDA_Indexer_new(
                                    DPSE_DiscoveryPlugin_compare_name,&iprop);
    if (disc_plugin->name_index == NULL)
    {
        DPSE_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                 DPSE_PARTICIPANTNAMEINDEX_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    disc_plugin->key_index = REDA_Indexer_new(
                                    DPSE_DiscoveryPlugin_compare_key,&iprop);
    if (disc_plugin->key_index == NULL)
    {
        DPSE_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                 DPSE_ENABLEDPARTICIPANTINDEX_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }


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

    disc_plugin->ignore_unknown_peers = dp_qos->discovery.accept_unknown_peers ?
                                                DDS_BOOLEAN_FALSE : DDS_BOOLEAN_TRUE;

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Called by the participant after it is fully initialized
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_participant_created.
 * When a participant is created it is ready for creating user entities. In
 * this callback the DPSE plugin creates the discovery endpoints. The qos
 * for the discovery endpoints are pre-defined by the DDS specification.
 *
 * \param[in] discovery_plugin       The DPSE plugin
 * \param[in] participant            The participant that created the plugin
 * \param[in] local_participant_data The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const local_participant_data)
{
    struct DPSE_DiscoveryPlugin *disc_plugin =
        (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    struct DDS_PublisherQos publisher_qos = DDS_PublisherQos_INITIALIZER;
    struct DDS_SubscriberQos subscriber_qos = DDS_SubscriberQos_INITIALIZER;
    struct DDS_DataWriterQos writer_qos = DDS_DataWriterQos_INITIALIZER;
    struct DDS_DataReaderQos reader_qos = DDS_DataReaderQos_INITIALIZER;
    struct DDS_TypePluginI *type_plugin;
    struct DDS_DataReaderListener participant_builtin_listener;
    struct DDS_TopicQos topic_qos = DDS_TopicQos_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos;
    struct NETIO_Address src_writer;
    struct NETIO_Address to_address;
    struct DDS_Locator *a_locator;
    DDS_Long peer_no,peer_length;

    NETIO_Address_init(&src_writer,NETIO_ADDRESS_KIND_INTRA);
    src_writer.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT);

    disc_plugin->participant = participant;

    /* Loan the timer from the participant. Fail if one cannot be loaned
     */
    disc_plugin->loaned_timer = DDS_DomainParticipant_get_timer(participant);

    if (disc_plugin->loaned_timer == NULL)
    {
        DPSE_LOG_GET_TIMER(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    disc_plugin->participant_builtin_data = local_participant_data;

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    /* Create endpoints that are not visible by users, such as delete
     * contained entities.
     */
    publisher_qos.management.is_hidden = DDS_BOOLEAN_TRUE;

    disc_plugin->participant_publisher = DDS_DomainParticipant_create_publisher(
            participant,&publisher_qos,NULL,DDS_STATUS_MASK_ALL);

    if (disc_plugin->participant_publisher == NULL)
    {
        DPSE_LOG_CREATE_DISCOVERY_PUBLISHER(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    subscriber_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    disc_plugin->participant_subscriber = DDS_DomainParticipant_create_subscriber(
            participant,&subscriber_qos,NULL,DDS_STATUS_MASK_ALL);

    if (disc_plugin->participant_subscriber == NULL)
    {
        DPSE_LOG_CREATE_DISCOVERY_SUBSCRIBER(OSAPI_LOGKIND_ERROR)
        goto finally;
    }

    type_plugin = DPSE_ParticipantBuiltinTopicDataTypePlugin_get();

    if (DDS_RETCODE_OK != DDS_DomainParticipant_register_type(
                                    participant,
                                    DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,
                                    type_plugin))
    {
        DPSE_LOG_REGISTER_TYPE(OSAPI_LOGKIND_ERROR,
                               DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    topic_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    disc_plugin->participant_topic = DDS_DomainParticipant_create_topic(
                    participant,DDS_PARTICIPANT_BUILTIN_TOPIC_NAME,
                    DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME,&topic_qos, NULL,
                    DDS_STATUS_MASK_NONE);

    if (NULL == disc_plugin->participant_topic)
    {
        DPSE_LOG_CREATE_TOPIC(OSAPI_LOGKIND_ERROR,
                              DDS_PARTICIPANT_BUILTIN_TOPIC_NAME,
                              DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME)
        goto finally;
    }

    /* Builtin endpoints have well-known object IDs */
    peer_length = DDS_StringSeq_get_length(&dp_qos->discovery.initial_peers);

    writer_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT;
    /* Disable flow control and fragmentation for data(p) */
    writer_qos.publish_mode.kind = DDS_SYNCHRONOUS_PUBLISH_MODE_QOS;
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
                (DPSE_MAX_ANON_PARTICIPANT * (peer_length + 1));
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
    writer_qos.management.is_announced = DDS_BOOLEAN_FALSE;
    writer_qos.type_support.plugin_data = (void*)disc_plugin;
    writer_qos.transport_priority.value = dp_qos->discovery.metatraffic_transport_priority;

    disc_plugin->participant_writer = DDS_Publisher_create_datawriter(
                                        disc_plugin->participant_publisher,
                                        disc_plugin->participant_topic,
                                        &writer_qos,
                                        NULL,
                                        DDS_STATUS_MASK_NONE);
    if (disc_plugin->participant_writer == NULL)
    {
        DPSE_LOG_CREATE_WRITER(OSAPI_LOGKIND_ERROR,
                    DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                        disc_plugin->participant_topic)))
        goto finally;
    }

    DPSE_ParticipantBuiltinDataReaderListener_initialize(
                            (struct NDDS_Discovery_Plugin*)disc_plugin,
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
    reader_qos.transport_priority.value = dp_qos->discovery.metatraffic_transport_priority;

    /* All remote participants looks like the same reader */
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
    reader_qos.management.is_announced = DDS_BOOLEAN_FALSE;

    disc_plugin->participant_reader =
        DDS_Subscriber_create_datareader(disc_plugin->participant_subscriber,
                DDS_Topic_as_topicdescription(disc_plugin->participant_topic),
                 &reader_qos,&participant_builtin_listener,
                 DDS_DATA_AVAILABLE_STATUS);
    if (disc_plugin->participant_reader == NULL)
    {
        DPSE_LOG_CREATE_READER(OSAPI_LOGKIND_ERROR,
           DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(disc_plugin->participant_topic)))
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
                                                &src_writer,&to_address))
        {
            DPSE_LOG_ADD_ANONYMOUS_ROUTE(OSAPI_LOGKIND_ERROR)
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
                                                &src_writer,&to_address))
        {
            DPSE_LOG_ADD_ANONYMOUS_ROUTE(OSAPI_LOGKIND_ERROR)
            goto finally;
        }
    }

    /* Set discovery-specific information in the participant's builtin topic
     * data.
     */
    local_participant_data->liveliness_lease_duration =
            disc_plugin->properties.participant_liveliness_lease_duration;

    if (!DDS_DomainParticipant_is_mtu_greater_than_builtindata(
            participant, disc_plugin->participant_writer,
            local_participant_data))
    {
        goto finally;
    }

    return DDS_BOOLEAN_TRUE;

finally:
    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Called by the participant when it is enabled by the user
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_participant_enabled.
 * When a participant is enabled it is ready to announce discovery data. In
 * this callback the DPSE plugin enables its own discovery endpoints and
 * starts announcing the participant.
 *
 * \param[in] discovery_plugin       The DPSE plugin
 * \param[in] participant            The participant that created the plugin
 * \param[in] local_participant_data The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_participant_enabled(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const local_participant_data)
{
    struct DPSE_DiscoveryPlugin *disc_plugin =
        (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    UNUSED_ARG(participant);

    if (DDS_Entity_enable(DDS_Topic_as_entity(disc_plugin->participant_topic)) != DDS_RETCODE_OK)
    {
        DPSE_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DPSE_TOPIC_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_enable(DDS_Publisher_as_entity(disc_plugin->participant_publisher)) != DDS_RETCODE_OK)
    {
        DPSE_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DPSE_PUBLISHER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    {
        DDS_InstanceHandle_t dp_inst = DDS_HANDLE_NIL;

        dp_inst = DDS_DataWriter_register_instance(
                                        disc_plugin->participant_writer,
                                        disc_plugin->participant_builtin_data);

        if (DDS_InstanceHandle_is_nil(&dp_inst))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    /* Schedule the fast assertions before the subscriber is enabled to
     * ensure that the timer is created.
     */
    if (DPSE_ParticipantDiscovery_schedule_fast_assertions(
                    discovery_plugin, local_participant_data, DDS_BOOLEAN_TRUE)
            != DDS_RETCODE_OK)
    {
        DPSE_LOG_SCHEDULE_FAST_ASSERTION(OSAPI_LOGKIND_ERROR)
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_enable(DDS_Subscriber_as_entity(disc_plugin->participant_subscriber)) != DDS_RETCODE_OK)
    {
        DPSE_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DPSE_SUBSCRIBER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Dispose of a participant
 *
 * \param[in] disc_plugin           DPSE plugin
 * \param[in] participant           The participant being disposed
 * \param[in] local_participant_key The disposed participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_announce_local_participant_deletion(
                    struct NDDS_Discovery_Plugin *disc_plugin,
                    DDS_DomainParticipant *const participant,
                    struct DDS_BuiltinTopicKey_t *const local_participant_key)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DPSE_DiscoveryPlugin *plugin =
                                    (struct DPSE_DiscoveryPlugin *)disc_plugin;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(local_participant_key);

    if (!DDS_Entity_is_enabled(DDS_DataWriter_as_entity(
                                                  plugin->participant_writer)))
    {
        return DDS_BOOLEAN_TRUE;
    }

    ih = DDS_Entity_get_instance_handle(
                            DDS_DomainParticipant_as_entity(participant));
    ih.is_valid = DDS_BOOLEAN_TRUE;

    retcode = DDS_DataWriter_unregister_instance(plugin->participant_writer,
                                     plugin->participant_builtin_data,
                                     &ih);

    if (DDS_RETCODE_OK != retcode)
    {
        DPSE_LOG_DISPOSE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:
    return retval;
}
#endif /* !RTI_CERT */


#ifndef RTI_CERT
/*ci
 * \brief Implementation of NDDS_Discovery_Plugin_on_before_local_participant_deleted
 *
 * \details
 * When a participant is deleted the plugin must delete all its internal
 * resources. It also disposes of the participant.
 *
 * \param[in] discovery_plugin      The DPSE plugin
 * \param[in] participant           The participant being deleted
 * \param[in] local_participant_key The participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_before_local_participant_deleted(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const local_participant_key)
{
    struct DPSE_DiscoveryPlugin *dpse_plugin =
        (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    DDS_Boolean retcode = DDS_BOOLEAN_FALSE;
    DDS_Long peer_no,peer_length;
    struct DDS_Locator *a_locator;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address to_address = NETIO_Address_INITIALIZER;
    UNUSED_ARG(local_participant_key);

    OSAPI_TRACE_DDS("[DPSE] shutdown started",RTI_TRUE)

    OSAPI_TRACE_DDS("[DPSE] delete meta unicast locators",RTI_TRUE)

    if (dpse_plugin->participant_reader != NULL)
    {
        peer_length = DDS_LocatorSeq_get_length(
                &dpse_plugin->participant_builtin_data->metatraffic_unicast_locators);

        for (peer_no = 0; peer_no < peer_length; peer_no++)
        {
            a_locator = DDS_LocatorSeq_get_reference(
                    &dpse_plugin->participant_builtin_data->metatraffic_unicast_locators,
                    peer_no);

            to_address = *(struct NETIO_Address*)a_locator;
            if (!DDS_DataReader_delete_anonymous_route(dpse_plugin->participant_reader,
                    &src_writer,
                    &to_address))
            {
                DPSE_LOG_DELETE_ANONYMOUS_ROUTE(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }

        OSAPI_TRACE_DDS("[DPSE] delete meta multicast locators",RTI_TRUE)

        peer_length = DDS_LocatorSeq_get_length(
                &dpse_plugin->participant_builtin_data->metatraffic_multicast_locators);
        for (peer_no = 0; peer_no < peer_length; peer_no++)
        {
            a_locator = DDS_LocatorSeq_get_reference(
                    &dpse_plugin->participant_builtin_data->metatraffic_multicast_locators,
                    peer_no);

            to_address = *(struct NETIO_Address*)a_locator;
            if (!DDS_DataReader_delete_anonymous_route(dpse_plugin->participant_reader,
                    &src_writer,
                    &to_address))
            {
                DPSE_LOG_DELETE_ANONYMOUS_ROUTE(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
    }

    /* In an error condition, it's possible that the participant was never
     * fully-created, so the subscriber is NULL.
     */
    if (dpse_plugin->participant_subscriber != NULL)
    {
        /* In an error condition, it's possible that the participant was never
         * fully-created, so the reader is NULL.
         */
        if (dpse_plugin->participant_reader != NULL)
        {
            if (DDS_RETCODE_OK !=
                DDS_Subscriber_delete_datareader(dpse_plugin->participant_subscriber,
                                                 dpse_plugin->participant_reader))
            {
                DPSE_LOG_DELETE_READER(OSAPI_LOGKIND_ERROR)
                goto done;
            }
            dpse_plugin->participant_reader = NULL;
        }

        if (DDS_RETCODE_OK !=
            DDS_DomainParticipant_delete_subscriber(participant,
                                        dpse_plugin->participant_subscriber))
        {
            DPSE_LOG_DELETE_SUBSCRIBER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        dpse_plugin->participant_subscriber = NULL;
    }


    /* Delete the timer. Since the datareader is deleted we do not expect
     * on more data to be received (this using the datawriter).
     */
    if (dpse_plugin->timer_is_created)
    {
        if (!OSAPI_Timer_delete_timeout(dpse_plugin->loaned_timer,
                                        &dpse_plugin->announcement_event))
        {
            DPSE_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                           DPSE_PARTICIPANTANNOUCEMENT_OBJECT)
            goto done;
        }
    }

    /* It's possible that in an error condition, the participant was not
     * allocated, but the participant is being deleted due to the error.
     */
    if (dpse_plugin->participant_publisher != NULL)
    {

        /* It's possible that in an error condition, the data writer was
         * not allocated, and the participant is being deleted due to the error.
         */
        if (dpse_plugin->participant_writer != NULL)
        {
            if (DDS_RETCODE_OK !=
                DDS_Publisher_delete_datawriter(dpse_plugin->participant_publisher,
                                                dpse_plugin->participant_writer))
            {
                DPSE_LOG_DELETE_WRITER(OSAPI_LOGKIND_ERROR)
                goto done;
            }
            dpse_plugin->participant_writer = NULL;
        }

        if (DDS_RETCODE_OK != DDS_DomainParticipant_delete_publisher(participant,
                                        dpse_plugin->participant_publisher))
        {
            DPSE_LOG_DELETE_PUBLISHER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        dpse_plugin->participant_publisher = NULL;
    }

    if (dpse_plugin->participant_topic != NULL)
    {
        if (DDS_RETCODE_OK != DDS_DomainParticipant_delete_topic(
                participant,dpse_plugin->participant_topic))
        {
            DPSE_LOG_DELETE_TOPIC(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    (void)DDS_DomainParticipant_unregister_type(
                                participant,
                                DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME);

    retcode = DDS_BOOLEAN_TRUE;

done:
    return retcode;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check if resources are available for a new DDS DataWriter
 *
 * \details
 *
 * This function is not needed in DPSE.
 *
 * \param[in] discovery_plugin The discovery plugin instance
 * \param[in] participant      The participant creating the DDS DataWriter
 * \param[in] dw_key           The assigned key for the DDS DataWriter
 * \param[in] reservation      DDS_BOOLEAN_TRUE if this is a reservation,
 *                             DDS_BOOLEAN_FALSE if this is releasing a
 *                             reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
RTI_PRIVATE DDS_ReturnCode_t
DPSE_DiscoveryPlugin_on_before_local_datawriter_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dw_key,
        DDS_Boolean reservation)
{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(dw_key);
    UNUSED_ARG(reservation);

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Called by the participant when a local datawriter is enabled
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_datawriter_enabled.
 * The participant does not take care about which plugin supports what, it
 * only calls the interface. Since static discovery does not implement
 * datareader and datawriter discovery this function only returns
 * DDS_BOOLEAN_TRUE.
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] data_writer      The datawriter being enabled
 * \param[in] qos              The datawriters Qos
 *
 * \return DDS_BOOLEAN_TRUE
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_data_writer_enabled(
                struct NDDS_Discovery_Plugin *const discovery_plugin,
                DDS_DomainParticipant *const participant,
                DDS_DataWriter *const data_writer,
                const struct DDS_DataWriterQos *const qos)
{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(data_writer);
    UNUSED_ARG(qos);

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Called by the participant when a local datawriter is deleted
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_datawriter_deleted.
 * The participant does not care about which plugin supports what, it
 * only calls the interface. Since static discovery does not implement
 * datareader and datawriter discovery this function only returns
 * DDS_BOOLEAN_TRUE.
 *
 * \param[in] discovery_plugin     The DPSE plugin
 * \param[in] participant          The participant
 * \param[in] local_datawriter_key The key of the deleted datawriter
 *
 * \return DDS_BOOLEAN_TRUE
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_data_writer_deleted(
                struct NDDS_Discovery_Plugin *const discovery_plugin,
                DDS_DomainParticipant *const participant,
                DDS_DataWriter *writer
                /* const struct DDS_BuiltinTopicKey_t *const local_datawriter_key */)
{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(writer);

    return DDS_BOOLEAN_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check if resources are available for a new DDS DataReader.
 *
 * \details
 *
 * This function is not needed in DPSE
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
RTI_PRIVATE DDS_ReturnCode_t
DPSE_DiscoveryPlugin_on_before_local_datareader_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dr_key,
        DDS_Boolean reservation)

{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(dr_key);
    UNUSED_ARG(reservation);

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Called by the participant when a local datareader is enabled
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_datareader_enabled.
 * The participant does not care about which plugin supports what, it
 * only calls the interface. Since static discovery does not implement
 * datareader and datawriter discovery this function only returns
 * DDS_BOOLEAN_TRUE.
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] data_reader      The datareader being enabled
 * \param[in] qos              The datareader Qos
 *
 * \return DDS_BOOLEAN_TRUE
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_data_reader_enabled(
                struct NDDS_Discovery_Plugin *const discovery_plugin,
                DDS_DomainParticipant *const participant,
                DDS_DataReader *const data_reader,
                const struct DDS_DataReaderQos *const qos)
{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(data_reader);
    UNUSED_ARG(qos);

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Called by the participant when a local datareader is deleted
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_after_local_datareader_deleted.
 * The participant does not care about which plugin supports what, it
 * only calls the interface. Since static discovery does not implement
 * datareader and datawriter discovery this function only returns
 * DDS_BOOLEAN_TRUE.
 *
 * \param[in] discovery_plugin     The DPSE plugin
 * \param[in] participant          The participant
 * \param[in] local_datareader_key The key of the deleted datareader
 *
 * \return DDS_BOOLEAN_TRUE
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_after_local_data_reader_deleted(
                struct NDDS_Discovery_Plugin *const discovery_plugin,
                DDS_DomainParticipant* const participant,
                DDS_DataReader *reader
                /*struct DDS_BuiltinTopicKey_t *const local_datareader_key*/)
{
    UNUSED_ARG(discovery_plugin);
    UNUSED_ARG(participant);
    UNUSED_ARG(reader);

    return DDS_BOOLEAN_TRUE;
}
#endif /* !RTI_CERT */
/*ci
 *
 * \brief Reset a remote participantin a DDS DomainParticipant
 *
 * \details
 *
 * When a remote participant is reset (either disposed of or liveliness has
 * expired) it is removed from the DPSE local database and then reset in the
 * DomainParticipant. If it is an unknown participant we ignore it (for example
 * it was asserted by some other means and DPSE do not know about it).
 *
 * \param[in] dpse_plugin  The DPSE plugin
 * \param[in] participant  The participant that created the plugin
 * \param[in] key          The key of the remote participant
 * \param[in] delete_keys  If TRUE also delete all the instances the
 *                         participant reader has detected.
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DPSE_Plugin_reset_remote_participant(
        struct DPSE_DiscoveryPlugin *const dpse_plugin,
        DDS_DomainParticipant *const participant,
        const DDS_BuiltinTopicKey_t *const key,
        DDS_Boolean delete_keys)
{
    struct DPSE_AssertedParticipant *rp;
    DDS_BuiltinTopicKey_t writer_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_InstanceHandle_t publication_handle = DDS_HANDLE_NIL;

    UNUSED_ARG(participant);

    if (delete_keys)
    {
        /* Remove the remote participant writer.
         * NOTE: The participant writer is anonymous and from the DDS reader point
         * of view the remote participant writer is always unknown. Thus,
         * only set the object_id for the well-known built-in participant writer.
         */
        writer_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT;
        DDS_InstanceHandle_from_rtps(&publication_handle,
                                     (struct RTPS_Guid*)&writer_key);

        DDS_DataReader_liveliness_lost(dpse_plugin->participant_reader,
                                       &publication_handle);
    }

    /* Remove from the array of discovered remote participants so it can
     * be rediscovered.
     *
     * NOTE: When a participant is reset it is _not_ removed from the name_index.
     * Doing so will cause the remote participant to be ignored if it is later
     * re-discovered.
     */
    rp = (struct DPSE_AssertedParticipant*)
                    REDA_Indexer_remove_entry(dpse_plugin->key_index,key);

    /* If this is an unknown participant there is nothing to do */
    if (rp == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (DDS_DataWriter_remove_participant_routes(
        dpse_plugin->participant_writer,key) != DDS_RETCODE_OK)
    {
        DPSE_LOG_DELETE_ANONYMOUS_ROUTE(OSAPI_LOGKIND_ERROR)
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Called by the participant when a remote participant should be removed,
 *  e.g. if it has failed to maintain liveliness, or to complete authentication.
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_on_before_remote_participant_deleted
 *
 * The participant calls this interface when it detects that a remote
 * participant should be unmatched, e.g. if it does not maintain its
 * lease_duration commitment, or if it fails to authenticate.
 *
 * \param[in] discovery_plugin        The DPSE plugin
 * \param[in] participant             The participant that created the plugin
 * \param[in] remote_participant_data The remote participant data
 * \param[in] remote_participant_status The remote participant's status
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_Plugin_before_remote_participant_removed(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const remote_participant_data,
        const DDS_RemoteParticipantStatusMask status)
{
    struct DPSE_DiscoveryPlugin *const dpse_plugin =
                        (struct DPSE_DiscoveryPlugin *)discovery_plugin;

    UNUSED_ARG(status);
    return DPSE_Plugin_reset_remote_participant(dpse_plugin,participant,
                                                &remote_participant_data->key,
                                                DDS_BOOLEAN_TRUE);
}

/*ci
 * \brief Add a peer address to the DPSE participant announcer
 *
 * \details
 * Implementation of NDDS_Discovery_Plugin_add_peer. This API enables peer
 * addresses to be dynamically added to the DPSE participant announcer.
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] add_peer         A peer address string
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DPSE_DiscoveryPlugin_add_peer(
                    struct NDDS_Discovery_Plugin *const discovery_plugin,
                    DDS_DomainParticipant *const participant,
                    const char *add_peer)
{
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    struct DPSE_DiscoveryPlugin *const dpse_plugin =
                        (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    UNUSED_ARG(participant);

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.value.rtps_guid.object_id =
                        NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    if (!DDS_DataWriter_add_anonymous_peer(dpse_plugin->participant_writer,
                                           &dst_reader,add_peer))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Called by the participant when a remote participant is asserted
 * as part of discovery.
 *
 * \details
*  Implementation of NDDS_Discovery_Plugin_assert_remote_participant.
 * When a remote participant is added to a participant the discovery plugin
 * is notified. The DPSE plugin keeps track of which participants have
 * been statically asserted. If a participant is discovered, but has not
 * been statically asserted it is ignored.
 *
 * \param[in] disc_plugin       The DPSE plugin
 * \param[in] participant       The participant that created the plugin
 * \param[in] participant_name  The name of the remote participant
 */
RTI_PRIVATE void
DPSE_DiscoveryPlugin_on_assert_remote_participant(
        struct NDDS_Discovery_Plugin *const disc_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *data)
{
    struct DPSE_DiscoveryPlugin *dpse =
                                (struct DPSE_DiscoveryPlugin *)disc_plugin;
    RTI_BOOL bretval;
    struct DPSE_AssertedParticipant *rp;
    DDS_BuiltinTopicKey_t nil_key = DDS_BUILTINTOPICKEY_UNKNOWN;

    UNUSED_ARG(participant);

    rp = REDA_Indexer_find_entry(dpse->name_index,
                                 data->participant_name.name);
    if (rp != NULL)
    {
        /* The participant may be asserted multiple times when reset, so this
         * is not an error condition.
         */
        return;
    }

    rp = (struct DPSE_AssertedParticipant*)
                        REDA_BufferPool_get_buffer(dpse->asserted_participants);
    if (rp == NULL)
    {
#if OSAPI_ENABLE_LOG
        DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
#endif
        return;
    }

    /* Since this information is coming from the participant the assumption is
     * that all the information is valid and legal. Still enforce string bounds
     * before storing it locally.
     */
    if (!REDA_String_copy(rp->name,
                          DDS_ENTITYNAME_QOS_NAME_MAX,
                          data->participant_name.name))
    {
#if OSAPI_ENABLE_LOG
        DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
#endif
        REDA_BufferPool_return_buffer(dpse->asserted_participants,rp);
        return;
    }

    rp->key = nil_key;

    bretval = REDA_Indexer_add_entry(dpse->name_index,(void*)rp->name);
    if (!bretval)
    {
        DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
        REDA_BufferPool_return_buffer(dpse->asserted_participants,rp);
    }
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a DPSE plugin instance
 *
 * \param[in] plugin DPSE plugin instance to delete
 *
 * \sa \ref DPSE_DiscoveryPlugin_create
 */
RTI_PRIVATE void
DPSE_DiscoveryPlugin_delete(struct DPSE_DiscoveryPlugin *plugin)
{

    RTI_BOOL bretval;
    RTI_INT32 i;
    struct DPSE_AssertedParticipant *rp;

    if (plugin->name_index != NULL)
    {
        for (i = 0; i < REDA_Indexer_get_count(plugin->name_index); i++)
        {
            rp = (struct DPSE_AssertedParticipant*)
                                   REDA_Indexer_get_entry(plugin->name_index,i);

            /* name comes after the key in the structure, subtract the key size
             * to get the beginning of the struct
             */
            rp = DPSE_AssertedParticipant_from_name_ptr(rp);
            REDA_BufferPool_return_buffer(plugin->asserted_participants,rp);
        }

        bretval = REDA_Indexer_delete(plugin->name_index);
#if OSAPI_ENABLE_LOG
        if (!bretval)

        {
            DPSE_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                   DPSE_PARTICIPANTNAMEINDEX_OBJECT)
        }
#else
        IGNORE_RETVAL(bretval);
#endif
    }

    if (plugin->key_index != NULL)
    {
        bretval = REDA_Indexer_delete(plugin->key_index);
#if OSAPI_ENABLE_LOG
        if (!bretval)

        {
            DPSE_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                   DPSE_ENABLEDPARTICIPANTINDEX_OBJECT)
        }
#else
        IGNORE_RETVAL(bretval);
#endif
    }

    if (plugin->asserted_participants != NULL)
    {
        bretval = REDA_BufferPool_delete(plugin->asserted_participants);
#if OSAPI_ENABLE_LOG
        if (!bretval)

        {
            DPSE_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                   DPSE_PARTICIPANTPOOL_OBJECT)
        }
#else
        IGNORE_RETVAL(bretval);
#endif

        OSAPI_Heap_free_struct(plugin);
    }
}
#endif

LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NDDS_DiscoveryI DPSE_DiscoveryPlugin_fv_Intf;

/*ci
 * \brief Create a new DPSE plugin instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new DPSE instance
 * \param[in] listener  The listener for the new DPSE plugin instance
 *
 * \return Pointer to new DPSE plugin instance on success, NULL on failure
 *
 * \sa \ref DPSE_DiscoveryPlugin_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct DPSE_DiscoveryPlugin*
DPSE_DiscoveryPlugin_create(struct DPSE_Discovery_Factory *factory,
                        const struct NDDS_Discovery_Property *const property,
                        const struct NDDS_Discovery_Listener *const listener)
{
    /* Variable declarations */
    struct DPSE_DiscoveryPlugin *new_plugin = NULL;
    DDS_Boolean error = DDS_BOOLEAN_TRUE;
    struct DPSE_DiscoveryPluginProperty *plugin_properties = &factory->property;

    OSAPI_PRECONDITION(property == NULL,
                   goto finally,
                   OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    /* --- Create the plugin --- */
    OSAPI_Heap_allocate_struct(&new_plugin, struct DPSE_DiscoveryPlugin);
    if (NULL == new_plugin)
    {
        DPSE_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                 DPSE_DISCOVERYPLUGIN_OBJECT)
        goto finally;
    }

    RT_Component_initialize(&new_plugin->_parent._parent,
                           &DPSE_DiscoveryPlugin_fv_Intf._parent,
                           0,
                           (property ? &property->_parent : NULL),
                           (listener ? &listener->_parent : NULL));

    /* --- Set properties --- */
    /* 19sep2014,as: DPSE_DiscoveryPluginProperties_copy was removed
     * from our API as part of the clean-up of support methods exposed
     * for each value type. The operation performed only a shallow copy
     * of the data structure, which has been added to this function.
     */
    new_plugin->properties = *plugin_properties;

    /* --- Fill in SystemTime structures --- */
    /* Fill in Time structures with the periods set by the user in properties
     */
    new_plugin->participant_liveliness_assert_period.sec =
            new_plugin->properties.participant_liveliness_assert_period.sec;
    new_plugin->participant_liveliness_assert_period.nanosec =
            new_plugin->properties.participant_liveliness_assert_period.nanosec;

    new_plugin->initial_participant_announcement_period.sec =
        new_plugin->properties.initial_participant_announcement_period.sec;
    new_plugin->initial_participant_announcement_period.nanosec =
        new_plugin->properties.initial_participant_announcement_period.nanosec;

    new_plugin->timer_is_created = RTI_FALSE;
    new_plugin->ignore_unknown_peers = RTI_FALSE;

    error = DDS_BOOLEAN_FALSE;

finally:

    if (DDS_BOOLEAN_TRUE == error)
    {
        new_plugin = NULL;
    }

    return new_plugin;
}

/*ci
 * \brief The DPSE discovery plugin interface implementation
 */
RTI_PRIVATE DDS_ReturnCode_t
DPSE_DiscoveryPlugin_write_announcement(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const participant_data)
{
    struct DPSE_DiscoveryPlugin *const dpse_plugin =
            (struct DPSE_DiscoveryPlugin *)discovery_plugin;

    UNUSED_ARG(participant);

    return DDS_DataWriter_write(dpse_plugin->participant_writer,
                                   (void *)participant_data,
                                   &DDS_HANDLE_NIL);
}

#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NDDS_DiscoveryI DPSE_DiscoveryPlugin_fv_Intf =
{
    RT_COMPONENTI_BASE,
    DPSE_Plugin_before_remote_participant_removed,
    DPSE_DiscoveryPlugin_before_local_participant_created,
    DPSE_DiscoveryPlugin_after_local_participant_created,
    DPSE_DiscoveryPlugin_after_local_participant_enabled,
    DPSE_DiscoveryPlugin_before_local_participant_deleted,
    DPSE_DiscoveryPlugin_after_local_data_writer_enabled,
    DPSE_DiscoveryPlugin_after_local_data_writer_deleted,
    DPSE_DiscoveryPlugin_after_local_data_reader_enabled,
    DPSE_DiscoveryPlugin_after_local_data_reader_deleted,
    DPSE_DiscoveryPlugin_add_peer,
    DPSE_DiscoveryPlugin_on_assert_remote_participant,
    DPSE_DiscoveryPlugin_on_before_local_datareader_created,
    DPSE_DiscoveryPlugin_on_before_local_datawriter_created,
    DPSE_DiscoveryPlugin_announce_local_participant_deletion,
    NULL,
    NULL,
    DPSE_DiscoveryPlugin_write_announcement
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NDDS_DiscoveryI DPSE_DiscoveryPlugin_fv_Intf =
{
    RT_COMPONENTI_BASE,
    DPSE_Plugin_before_remote_participant_removed,
    DPSE_DiscoveryPlugin_before_local_participant_created,
    DPSE_DiscoveryPlugin_after_local_participant_created,
    DPSE_DiscoveryPlugin_after_local_participant_enabled,
    NULL, /* DPSE_DiscoveryPlugin_before_local_participant_deleted, */
    DPSE_DiscoveryPlugin_after_local_data_writer_enabled,
    NULL, /* DPSE_DiscoveryPlugin_after_local_data_writer_deleted, */
    DPSE_DiscoveryPlugin_after_local_data_reader_enabled,
    NULL, /* DPSE_DiscoveryPlugin_after_local_data_reader_deleted, */
    DPSE_DiscoveryPlugin_add_peer,
    DPSE_DiscoveryPlugin_on_assert_remote_participant,
    DPSE_DiscoveryPlugin_on_before_local_datareader_created,
    DPSE_DiscoveryPlugin_on_before_local_datawriter_created,
    NULL,
    NULL,
    NULL,
    DPSE_DiscoveryPlugin_write_announcement
};
#endif /* !RTI_CERT */

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

#ifndef RTI_CERT
/*ci
 * \brief Delete a DPSE discovery plugin
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This method deletes
 * a DPSE discovery-plugin. It is never called directly, only via
 * a factory interface type.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref DPSE_DiscoveryFactory_create_component
 */
RTI_PRIVATE void
DPSE_DiscoveryFactory_delete_component(struct RT_ComponentFactory *factory,
                                       RT_Component_T *component)
{
    struct DPSE_DiscoveryPlugin *self =
                            (struct DPSE_DiscoveryPlugin *)component;
    UNUSED_ARG(factory);

    DPSE_DiscoveryPlugin_delete(self);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Creates a new DPSE discovery plugin.
 *
 * \details
 * Implementation of the RT ComponentFactory create_component method. This
 * method is never called directly, only via a factory interface type.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 *
 * \sa \ref DPSE_DiscoveryFactory_delete_component
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
DPSE_DiscoveryFactory_create_component(struct RT_ComponentFactory *factory,
                                        struct RT_ComponentProperty *property,
                                        struct RT_ComponentListener *listener)
{
    struct DPSE_DiscoveryPlugin *retval = NULL;

    retval = DPSE_DiscoveryPlugin_create(
                    (struct DPSE_Discovery_Factory*)factory,
                    (const struct NDDS_Discovery_Property *)property,
                    (const struct NDDS_Discovery_Listener *)listener);

    if (retval == NULL)
    {
        return NULL;
    }

    return &retval->_parent._parent;
}

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DPSE_DiscoveryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                  struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
DPSE_DiscoveryFactory_finalize(struct RT_ComponentFactory *factory,
                                struct RT_ComponentFactoryProperty **property,
                                struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct RT_ComponentFactoryI DPSE_Discovery_Factory_fv_Intf =
{
    RTI_DPSEDISCOVERY_INTERFACE_ID,
    DPSE_DiscoveryFactory_initialize,
    DPSE_DiscoveryFactory_finalize,
    DPSE_DiscoveryFactory_create_component,
    DPSE_DiscoveryFactory_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct RT_ComponentFactoryI DPSE_Discovery_Factory_fv_Intf =
{
    RTI_DPSEDISCOVERY_INTERFACE_ID,
    DPSE_DiscoveryFactory_initialize,
    NULL, /* DPSE_DiscoveryFactory_finalize, */
    DPSE_DiscoveryFactory_create_component,
    NULL, /* DPSE_DiscoveryFactory_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief DPSE plugin factory
 *
 * \details
 * The DPSE plugin class is implemented as a singleton, there are
 * no shared resources between plugins.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct DPSE_Discovery_Factory DPSE_Discovery_Factory_fv_Factory =
{
    {
            &DPSE_Discovery_Factory_fv_Intf,
            NULL,
            {{{0,0}}}
    },
    DPSE_DiscoveryPluginProperty_INITIALIZER
};

/*ci
 * \brief Initialize the DPSE discovery plugin factory
 *
 * \details
 * The DPSE component specific implementation of the RT ComponentFactory
 * initialize method. This method is called by RT when the factory is
 * registered.
 *
 * \param[in] property The properties registered with the DPSE
 *                     interface factory
 * \param[in] listener The listener registered with the DPSE
 *                     interface factory
 *
 * \return A fully initialized factory on success, NULL on failure
 *
 * \sa \ref DPSE_DiscoveryFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DPSE_DiscoveryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                  struct RT_ComponentFactoryListener *listener)
{
    struct DPSE_DiscoveryPluginProperty *dpse_prop;

    UNUSED_ARG(listener);

    DPSE_Discovery_Factory_fv_Factory._parent._factory =
                                    &DPSE_Discovery_Factory_fv_Factory._parent;

    if (property != NULL)
    {
        DPSE_Discovery_Factory_fv_Factory.property =
                            *((struct DPSE_DiscoveryPluginProperty *)property);
    }

    dpse_prop = &DPSE_Discovery_Factory_fv_Factory.property;

    if (!DDS_Duration_is_normalized(&dpse_prop->participant_liveliness_assert_period))
    {
        return NULL;
    }

    if (!DDS_Duration_is_normalized(&dpse_prop->participant_liveliness_lease_duration))
    {
        return NULL;
    }

    if (!DDS_Duration_is_normalized(&dpse_prop->initial_participant_announcement_period))
    {
        return NULL;
    }

    /* check the DPSE property for consistency */
    if (dpse_prop->initial_participant_announcements < 0)
    {
        return NULL;
    }

    /* check that initial period is [0nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpse_prop->initial_participant_announcement_period,
                             &DDS_DURATION_ZERO) < 0)
    {
        /* initial_participant_announcement_period < 0 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpse_prop->initial_participant_announcement_period,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* initial_participant_announcement_period > 1 year */
        return NULL;
    }


    /* check that regular period is [1nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpse_prop->participant_liveliness_assert_period,
                             &DDS_DURATION_NANOSEC) < 0)
    {
        /* participant_liveliness_assert_period < 1 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpse_prop->participant_liveliness_assert_period,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* participant_liveliness_assert_period > 1 year */
        return NULL;
    }

    /* check that lease duration is [1nsec,1year] (1 year = 365 days) */
    if (DDS_Duration_compare(&dpse_prop->participant_liveliness_lease_duration,
                             &DDS_DURATION_NANOSEC) < 0)
    {
        /* participant_liveliness_lease_duration < 1 nanosec */
        return NULL;
    }

    if (DDS_Duration_compare(&dpse_prop->participant_liveliness_lease_duration,
                             &DDS_DURATION_YEAR) > 0)
    {
        /* participant_liveliness_lease_duration > 1 year */
        return NULL;
    }

    /* If initial announcements are requested, but timeout is 0 then
     * return error
     */
    if ((dpse_prop->initial_participant_announcements > 0) &&
        DDS_Duration_is_zero(&dpse_prop->initial_participant_announcement_period))
    {
        return NULL;
    }

    /* If the lease duration is < regular liveliness period return error */
    if (DDS_Duration_compare(&dpse_prop->participant_liveliness_lease_duration,
                         &dpse_prop->participant_liveliness_assert_period) < 0)
    {
        return NULL;
    }

    if ((dpse_prop->max_participant_locators < 1) &&
        (dpse_prop->max_participant_locators != DDS_LENGTH_AUTO))
    {
        return NULL;
    }

    if (dpse_prop->max_locators_per_discovered_participant < 1)
    {
        return NULL;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if ((dpse_prop->participant_message_reader_reliability_kind !=
                                          DDS_RELIABLE_RELIABILITY_QOS) &&
        (dpse_prop->participant_message_reader_reliability_kind !=
                                          DDS_BEST_EFFORT_RELIABILITY_QOS))
    {
        return NULL;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    return &DPSE_Discovery_Factory_fv_Factory._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the datareader DPSE component factory
 *
 * \details
 * The DPSE specific implementation of the RT ComponentFactory
 * finalize method. This method is called by RT when the factory is
 * unregistered.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref DPSE_DiscoveryFactory_initialize
 */
RTI_PRIVATE void
DPSE_DiscoveryFactory_finalize(struct RT_ComponentFactory *factory,
                                struct RT_ComponentFactoryProperty **property,
                                struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
DPSE_DiscoveryFactory_get_interface(void)
{
    return &DPSE_Discovery_Factory_fv_Intf;
}

/*ci @} */
