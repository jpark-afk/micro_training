/*
 * FILE: ParticipantDiscovery.h - Participant discovery API
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
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief Participant discovery API
 */
#ifndef ParticipantDiscovery_h
#define ParticipantDiscovery_h

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_ParticipantDiscovery_schedule_fast_assertions(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *local_participant_data,
        DDS_Boolean new_event);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_ParticipantDiscovery_after_local_participant_created(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data);

extern void
DPDE_ParticipantDiscovery_assert_remote_participant(
                                struct DPDE_DiscoveryPlugin *dpde_plugin,
                                DDS_DomainParticipant *participant,
                                struct DDS_ParticipantBuiltinTopicData *data,
                                struct DDS_SampleInfo *info);

#endif

