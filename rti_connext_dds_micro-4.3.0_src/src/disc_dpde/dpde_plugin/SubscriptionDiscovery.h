/*
 * FILE: SubscriptionDiscovery.h - Subscription discovery API
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
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief Subscription discovery API
 */
#ifndef SubscriptionDiscovery_h
#define SubscriptionDiscovery_h

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_on_before_local_datareader_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dr_key,
        DDS_Boolean reservation);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_participant_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_data_reader_deleted(
                    struct NDDS_Discovery_Plugin *const discovery_plugin,
                    DDS_DomainParticipant *const participant,
                    DDS_DataReader *reader
                    /*const struct DDS_BuiltinTopicKey_t *local_datareader_key*/);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_SubscriptionDiscovery_after_local_data_reader_enabled(
                        struct NDDS_Discovery_Plugin *const discovery_plugin,
                        DDS_DomainParticipant *const participant,
                        DDS_DataReader *const data_reader,
                        const struct DDS_DataReaderQos *const qos);

MUST_CHECK_RETURN RTI_BOOL
DPDE_SubscriptionDiscovery_get_datareader_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataReaderQos *reader_qos /* out */);

MUST_CHECK_RETURN RTI_BOOL
DPDE_SubscriptionDiscovery_get_datawriter_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataWriterQos *writer_qos /* out */);

void
DPDE_SubscriptionDiscovery_on_subscription_data_return_loan(
                    struct NDDS_Discovery_Plugin *const discovery_plugin,
                    DDS_DomainParticipant *const participant,
                    struct DDS_SubscriptionBuiltinTopicData *data,
                    struct DDS_SampleInfo *info);

#endif

