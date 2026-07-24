/*
 * FILE: PublicationDiscovery.h - Publication discovery API
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
 * \brief Publication discovery API
 */
#ifndef PublicationDiscovery_h
#define PublicationDiscovery_h

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_PublicationDiscovery_on_before_local_datawriter_created(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dw_key,
        DDS_Boolean reserved);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_PublicationDiscovery_after_local_participant_created(
            struct NDDS_Discovery_Plugin *const discovery_plugin,
            DDS_DomainParticipant *const participant,
            struct DDS_ParticipantBuiltinTopicData *local_participant_data);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_PublicationDiscovery_after_local_data_writer_enabled(
                        struct NDDS_Discovery_Plugin *const discovery_plugin,
                        DDS_DomainParticipant *const participant,
                        DDS_DataWriter *const datawriter,
                        const struct DDS_DataWriterQos *const qos);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_PublicationDiscovery_after_local_data_writer_deleted(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *writer
        /*const struct DDS_BuiltinTopicKey_t *local_datawriter_key*/);

MUST_CHECK_RETURN RTI_BOOL
DPDE_PublicationDiscovery_get_datareader_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataReaderQos *reader_qos /* out */);

MUST_CHECK_RETURN RTI_BOOL
DPDE_PublicationDiscovery_get_datawriter_qos(
        struct DPDE_DiscoveryPlugin *const plugin,
        DDS_DomainParticipant *participant,
        struct DDS_DataWriterQos *writer_qos /* out */);

void
DPDE_PublicationDiscovery_on_publication_data_return_loan(
                        struct NDDS_Discovery_Plugin *const discovery_plugin,
                        DDS_DomainParticipant *const participant,
                        struct DDS_PublicationBuiltinTopicData *data,
                        struct DDS_SampleInfo *info);
#endif

