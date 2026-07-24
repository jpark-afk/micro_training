/*
 * FILE: DPSEDiscoveryPlugin.h - DPSE Discovery plugin API
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
 * 07mar2014,tk MICRO-735: Send properties for tools
 * 30apr2011,tk Written
 */
/*ce
 * \file
 * \brief DPSE Discovery plugin API
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef DPSEDiscoveryPlugin_h
#define DPSEDiscoveryPlugin_h

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
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#endif
#ifndef disc_dpse_log_h
#include "disc_dpse/disc_dpse_log.h"
#endif
#define DPSE_MAX_ANON_PARTICIPANT 6

/*ci
 * brief Implementation of the DPSE discovery plugin
 */
struct DPSE_DiscoveryPlugin
{
    /*ci
     * \brief Inherit from base-class
     */
    struct NDDS_Discovery_Plugin _parent;

    /*ci
     * \brief The local participant associated with this plugin
     */
    DDS_DomainParticipant *participant;

    /*ci
     * \brief The built-in publisher containing discovery endpoints
     */
    DDS_Publisher *participant_publisher;

    /*ci
     * \brief DDS Participant announcer
     */
    DDS_DataWriter *participant_writer;

    /*ci
     * \brief The built-in subscriber containing discovery endpoints
     */
    DDS_Subscriber *participant_subscriber;

    /*ci
     * \brief DDS Participant listener
     */
    DDS_DataReader *participant_reader;

    /*ci
     * \brief DDS Participant topic
     */
    DDS_Topic *participant_topic;

    /*ci
     * \brief The lease-duration committed to by the participant
     */
    struct OSAPI_SystemTime participant_liveliness_assert_period;

    /*ci
     * \brief The frequency for the announcements sent when the participant is
     *        first enabled
     */
    struct OSAPI_SystemTime initial_participant_announcement_period;

    /*ci
     *  \brief A copy of the properties the discovery plugin was created with
     */
    struct DPSE_DiscoveryPluginProperty properties;

    /*ci
     * \brief The asserted remote participant names. If a discovered
     *        participant is not listed as valid name it is ignored.
     */
    REDA_BufferPool_T asserted_participants;

    /*ci
     * \brief An index to search for remotely asserted participants with
     */
    REDA_Indexer_T *name_index;


    /*ci
     * \brief An index to search for enabled remote participants.
     */
    REDA_Indexer_T *key_index;

    /*ci
     * \brief The announcement event, which may get updated as we receive
     *        announcements from remote participants.
     */
    OSAPI_TimeoutHandle_T announcement_event;

    /*ci
     * \brief The participant announcement. This is owned by the participant,
     *  not the plugin. A pointer is stored for convenience. It is valid as
     *  long as the plugin exists.
     */
    struct DDS_ParticipantBuiltinTopicData *participant_builtin_data;

    /*ci
     * \brief Whether the internal annoucement timer has been created or not,
     *        in case the plugin is deleted before the time is started.
     */
    RTI_BOOL timer_is_created;

    /*ci
     *\brief Whether to ignore a participant not listed in the peer
     */
    DDS_Boolean ignore_unknown_peers;

    /*ci
     * \brief A timer object to create a liveliness refresh timer. This timer
     *        is owned by the participant that created this discovery plugin.
     */
    OSAPI_Timer_T loaned_timer;


    /*ci
     * \brief A counter used to keep track of how many initial announcements
     *        are sent. The counter is stored here as it changes value, but
     *        updating a timer does not update the user_data.
     */
    RTI_INT32 initial_announcement_count;
};

/*ci
 * \brief An entry for an asserted remote participant, used to keep local state
 *
 * \details
 * Each remote participant that is asserted has local state with the DPSE
 * plugin. The information is used to filter out participants that are
 * discovered but not asserted and to only enable a participant once.
 */
struct DPSE_AssertedParticipant
{
    /*ci
     * \brief The remote participant
     */
    DDS_BuiltinTopicKey_t key;

    /*ci
     * \brief The name of the remote participant
     */
    char name[DDS_ENTITYNAME_QOS_NAME_MAX + 1];
};


/*ci
 * \brief Convenience macro to convert a name ptr to a structure pointer
 */
#define DPSE_AssertedParticipant_from_name_ptr(ptr_) \
    OSAPI_Compiler_reinterpret_cast(\
        struct DPSE_AssertedParticipant*,\
        ((char*)(ptr_) - sizeof(DDS_BuiltinTopicKey_t)))


extern DDS_Boolean
DPSE_Plugin_reset_remote_participant(
        struct DPSE_DiscoveryPlugin *const dpse_plugin,
        DDS_DomainParticipant *const participant,
        const DDS_BuiltinTopicKey_t *const key,
        DDS_Boolean delete_keys);

#endif /* DPSEDiscoveryPlugin_h */

/*ci @} */

