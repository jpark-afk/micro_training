/*
 * FILE: DiscoveryPlugin.h - DPDE Discovery Plugin API
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
 * 06feb2013,eh MICRO-294  temp increase to 6 participant announcements
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief DPDE Discovery Plugin API
 */
#ifndef DiscoveryPlugin_h
#define DiscoveryPlugin_h

#include "osapi/osapi_types.h"

#define DPDE_MAX_ANON_PARTICIPANT 6

struct DPDE_DiscoveryPlugin
{
    /* --- Parent plugin --- */
    struct NDDS_Discovery_Plugin _parent;

    DDS_DomainParticipant *participant;

    /* --- Builtin --- */
    DDS_Publisher *publisher;
    DDS_DataWriter *participant_writer;
    DDS_DataWriter *publication_writer;
    DDS_DataWriter *subscription_writer;

    DDS_Subscriber *subscriber;
    DDS_DataReader *participant_reader;
    DDS_DataReader *publication_reader;
    DDS_DataReader *subscription_reader;

    DDS_Topic *participant_topic;
    DDS_Topic *publication_topic;
    DDS_Topic *subscription_topic;

    /*DDS_InstanceHandle_t *instance_handle; */

    struct OSAPI_SystemTime participant_liveliness_assert_period;
    struct OSAPI_SystemTime initial_participant_announcement_period;

    /*ci \brief Counter to count how many initial annoucemnts have been sent.
     * Reset on each call to DPDE_ParticipantDiscovery_schedule_fast_assertions.
     */
    DDS_Long initial_participant_announcements_counter;

    struct DPDE_DiscoveryPluginProperty properties;

    /* The announcement event, which may get updated as we receive announcements
     * from remote participants.
     */
    OSAPI_TimeoutHandle_T announcement_event;

    /* This is owned by the participant, not the plugin.  A pointer is
     * stored for convenience
     */
    struct DDS_ParticipantBuiltinTopicData *local_participant_data;

    struct DDS_TypePluginI *part_type_plugin;

    struct DDS_TypePluginI *pub_type_plugin;

    struct DDS_TypePluginI *sub_type_plugin;

    RTI_BOOL timer_created;

    /*ci
     *\brief Whether to ignore a participant not listed in the peer
     */
    DDS_Boolean ignore_unknown_peers;
};

extern DDS_Boolean
DPDE_DiscoveryPluginProperty_is_equal(
            const struct DPDE_DiscoveryPluginProperty* self,
            const struct DPDE_DiscoveryPluginProperty* from);

extern DDS_Boolean
DPDE_DiscoveryPlugin_remove_builtin_peers(
                         struct DPDE_DiscoveryPlugin *const dpde_plugin,
                         const DDS_BuiltinTopicKey_t *const dp_key);

#endif

