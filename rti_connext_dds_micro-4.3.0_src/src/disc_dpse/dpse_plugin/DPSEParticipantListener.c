/*
 * FILE: DPSEParticipantListener.c - DPSE Participant Listener
 *
 * (c) Copyright 2008-2024 Real-Time Innovations,
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
 * 11sep2015,tk  MICRO-1499/PR#16514 Fixed issues when resetting a remote
 *                                   participant that prevented it from being
 *                                   rediscovered.
 * 28may2015,tk  MICRO-1263/PR#14863 Refactored for clarity and consistency
 * 28may2015,tk  MICRO-1262/PR#14862 Return samples before reading next batch
 * 05jan2014,tk  Updated log-codes
 * 11jun2014,tk  Filter out participant which have not been asserted
 * 03jun2008,rmw Created.
 */
/*ce
 * \file
 * \brief DPSE Participant Listener
 *
 * \details
 * This file implements the datareader listeners for the participant
 * announcement datareader.
 */
/*ci \addtogroup DPSEModule
 * @{
 */
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

#include "DPSECdr.h"
#include "DPSEParticipantBuiltinTopicData.h"
#include "DPSEParticipantDiscovery.h"
#include "DPSEParticipantListener.h"
#include "DPSEDiscoveryPlugin.h"

/*** SOURCE_BEGIN ***/

/* ------------------------------------------------------------------------ */
/*                Participant Discovery Listener                            */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Handle on_before_commit events
 *
 * \details
 * The DPSE plugin uses the on_before_commit event on the datareader to
 * filter out discovery data for participant that have not been asserted.
 *
 * \param[in] listener_data Opaque pointer passed to the datareader
 * \param[in] reader        The datareader the event occurred on
 * \param[in] sample        The sample which are about to be committed
 * \param[in] sample_info   Partially filled sample_info with only
 *                          reception_sequence_number being valid
 * \param[inout] dropped    Set to DDS_BOOLEAN_FALSE to keep the sample, and
 *                          DDS_BOOLEAN_TRUE to drop the sample
 *
 * \return DDS_BOOLEAN_FALSE if the function succeeded, DDS_BOOLEAN_FALSE
 *         it not
 */
RTI_PRIVATE DDS_Boolean
DPSE_ParticipantBuiltinDataReaderListener_on_before_sample_commit(
                                void *listener_data,
                                DDS_DataReader *reader,
                                const void *const sample,
                                const struct DDS_SampleInfo *const sample_info,
                                DDS_Boolean *dropped)
{
    struct DPSE_DiscoveryPlugin *dpse =
                            (struct DPSE_DiscoveryPlugin *)listener_data;
    struct DDS_ParticipantBuiltinTopicData *data =
                            (struct DDS_ParticipantBuiltinTopicData*)sample;
    struct NETIO_Address dst_reader;

    UNUSED_ARG(sample_info);
    UNUSED_ARG(reader);

    *dropped = DDS_BOOLEAN_FALSE;

    if (!DDS_DomainParticipant_is_checksum_compatible(dpse->participant,data))
    {
        DPSE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED(OSAPI_LOGKIND_WARNING)
        OSAPI_TRACE_DDS("[DPDE] ignored participant with incompatible CRC",RTI_FALSE)
        OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

        *dropped = DDS_BOOLEAN_TRUE;
        return DDS_BOOLEAN_TRUE;
    }

    /* First check if the participant was even asserted locally, if not drop
     * the sample.
     */
    if (REDA_Indexer_find_entry(dpse->name_index,
                                data->participant_name.name) == NULL)
    {
        *dropped = RTI_TRUE;

        OSAPI_TRACE_DDS("[DPSE] ignored participant not locally asserted",RTI_FALSE)
        OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

        return DDS_BOOLEAN_TRUE;
    }

    if (!dpse->ignore_unknown_peers)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* Next, if ignore_unknown_peers is true then check if the peer is in the
     * participant writer's route table. If not, drop the sample. Need to check
     * both unicast and multicast locators.
     */
    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.value.rtps_guid.object_id =
            NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    if (DDS_DataWriter_lookup_route(dpse->participant_writer,&dst_reader,
                                    &data->metatraffic_multicast_locators))
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (DDS_DataWriter_lookup_route(dpse->participant_writer,&dst_reader,
                                    &data->metatraffic_unicast_locators))
    {
        return DDS_BOOLEAN_TRUE;
    }

    *dropped = DDS_BOOLEAN_TRUE;

    OSAPI_TRACE_DDS("[DPSE] ignored participant not locally asserted",RTI_FALSE)
    OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Add anonymous route to a locator sequence
 *
 * \param[in] dpse_plugin The DPSE plugin
 * \param[in] locator_seq A sequence of locators to add routes to
 */
RTI_PRIVATE void
DPSE_ParticipantBuiltinDataReaderListener_add_route(
                            struct DPSE_DiscoveryPlugin *dpse_plugin,
                            struct DDS_LocatorSeq const *locator_seq)
{
    RTI_INT32 i,j;
    struct DDS_Locator *a_locator = NULL;
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_address = NETIO_Address_INITIALIZER;
    DDS_Boolean bretval;

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.port = 0;
    dst_reader.value.rtps_guid.object_id =
                            NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator_seq,i);
        dst_address = *(struct NETIO_Address*)a_locator;

        bretval = DDS_DataWriter_add_anonymous_route(
                                dpse_plugin->participant_writer,
                                &dst_reader,&dst_address);
#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            DPSE_LOG_ADD_ANONYMOUS_ROUTE(OSAPI_LOGKIND_WARNING)
        }
#else
        IGNORE_RETVAL(bretval);
#endif
    }
}

/*ci
 * \brief Handle on_data_available event on the participant datareader
 *
 * \details
 * This listener handles participant announcements from asserted datareaders.
 * The  on_before_commit handler is used to drop samples not destined for
 * this participant (including itself).
 * \p
 * If a new participant is asserted it is added, otherwise the lease_duration
 * timer is refreshed for that participant.
 *
 * \param[in] listener_data Opaque pointer passed to the datareader
 * \param[in] reader        The datareader the event occurred on
 */
RTI_PRIVATE void
DPSE_ParticipantBuiltinDataReaderListener_on_data_available(
                                                        void *listener_data,
                                                        DDS_DataReader *reader)
{
    struct DPSE_DiscoveryPlugin *dpse_plugin =
                            (struct DPSE_DiscoveryPlugin *)listener_data;
    DDS_ReturnCode_t retcode;
    DDS_Long k,l;
    DDS_DomainParticipant *participant = NULL;
    struct DDS_ParticipantBuiltinTopicDataSeq data_seq =
                            DDS_ParticipantBuiltinTopicDataSeq_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq  = DDS_SEQUENCE_INITIALIZER;
    struct DDS_ParticipantBuiltinTopicData *data = NULL;
    struct DDS_SampleInfo *info = NULL;
    struct DDS_BuiltinTopicKey_t key;
    DDS_Boolean bretval;
    struct DPSE_AssertedParticipant *rp;
    char *rp_by_name;

    participant = DDS_Subscriber_get_participant(
                                    dpse_plugin->participant_subscriber);

    do
    {
        retcode = DDS_DataReader_take(reader,
                (struct DDS_UntypedSampleSeq*)&data_seq,&info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_VIEW_STATE,DDS_ANY_SAMPLE_STATE,DDS_ANY_INSTANCE_STATE);

#if OSAPI_ENABLE_LOG
        if (retcode == DDS_RETCODE_NO_DATA)
        {
            break;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            DPSE_LOG_PARTICIPANT_TAKE(OSAPI_LOGKIND_ERROR,retcode)
            break;
        }
#else
        if (retcode != DDS_RETCODE_OK)
        {
            break;
        }
#endif

        l = DDS_ParticipantBuiltinTopicDataSeq_get_length(&data_seq);
        for (k = 0; k < l; ++k)
        {
            data = DDS_ParticipantBuiltinTopicDataSeq_get_reference(&data_seq,k);
            info = DDS_SampleInfoSeq_get_reference(&info_seq, k);

            if (info->valid_data)
            {
                DDS_RemoteParticipantStatusMask status =
                                        DDS_REMOTE_PARTICIPANT_STATUS_DEFAULT;

                /* on_before_sample_commit() drops all samples from
                 * participants that have not explicitly asserted. The
                 * participant that created the plugin cannot be asserted
                 * so at this point it must be a remote participant.
                 */
                OSAPI_TRACE_DDS("[DPSE] Detected participant",RTI_FALSE)
                OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

                /* Ensure the participant is asserted. For static discovery this
                 * may seem redundant. However, in case of rediscovery its is
                 * necessary to reset the current perticipant by name. A
                 * call to NDDS_DomainParticipant_assert_remote_participant
                 * handles that.
                 */
                if (DDS_DomainParticipant_is_discovery_by_name_enabled(participant))
                {
                    if (NDDS_DomainParticipant_assert_remote_participant(
                                participant,data,&status) != DDS_RETCODE_OK)
                    {
                        DPSE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR,
                                                           retcode)
                        continue;
                    }
                }

                if (REDA_Indexer_find_entry(dpse_plugin->key_index,
                                            &data->key) != NULL)
                {
                    /* Refresh the liveliness of a pre-existing
                     * remote participant
                     */
                    retcode =
                        NDDS_DomainParticipant_refresh_remote_participant_liveliness(
                           participant, &data->key);
#if OSAPI_ENABLE_LOG
                    if (DDS_RETCODE_OK != retcode)
                    {
                        DPSE_LOG_REFRESH_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
                    }
#else
                    IGNORE_RETVAL(retcode);
#endif
                    continue;
                }

                /* A participant previously asserted, but not
                 * yet enabled, must have been discovered.
                 */
                OSAPI_TRACE_DDS("[DPSE] Detected new participant",RTI_FALSE)
                OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE)

                /* --- Enable the participant --- */
                retcode = NDDS_DomainParticipant_enable_remote_participant_name(
                                                    participant, data);

                if (DDS_RETCODE_OK != retcode)
                {
                    DPSE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_WARNING,
                                                       retcode)
                    continue;
                }

                /* The participant was successfully enabled, add it.
                */
                DPSE_ParticipantBuiltinDataReaderListener_add_route(
                        dpse_plugin,&data->metatraffic_unicast_locators);

                DPSE_ParticipantBuiltinDataReaderListener_add_route(
                        dpse_plugin,&data->metatraffic_multicast_locators);

                retcode = DPSE_ParticipantDiscovery_schedule_fast_assertions(
                        (struct NDDS_Discovery_Plugin*)dpse_plugin,
                        dpse_plugin->participant_builtin_data,
                        DDS_BOOLEAN_FALSE);

                /* The above function already logs error as this function
                 * does not return error
                 */
                IGNORE_RETVAL(retcode);

                rp_by_name = (char*)REDA_Indexer_find_entry(
                                                 dpse_plugin->name_index,
                                                 data->participant_name.name);

                /* Get the structure pointer, avoid "increases required alignment
                 * warning".
                 */
                {
                    void *rp_void_ptr = DPSE_AssertedParticipant_from_name_ptr(rp_by_name);
                    OSAPI_Memory_copy(&rp,&rp_void_ptr,sizeof(void*));
                }

                rp->key = data->key;

                bretval = REDA_Indexer_add_entry(dpse_plugin->key_index,rp) ?
                                                 DDS_BOOLEAN_TRUE :
                                                 DDS_BOOLEAN_FALSE;
#if OSAPI_ENABLE_LOG
                if (!bretval)
                {
                    DPSE_LOG_ASSERT_REMOTE_PARTICIPANT(OSAPI_LOGKIND_WARNING,
                                                       retcode)
                }
#else
                IGNORE_RETVAL(bretval);
#endif
                continue;
            }

            /* At this point it has been determined this is not a valid
             * data sample. Check for a state change on the remote participant.
             */
            if (info->instance_state == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
                info->instance_state == DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
            {
                DDS_InstanceHandle_to_rtps((struct RTPS_Guid *)&key,
                                           &info->instance_handle);

                /* Do not delete the keys explicitly, they will be deleted when
                 * this calls returns to the reader-history
                 */
                retcode = NDDS_DomainParticipant_reset_remote_participant(
                                                        participant,&key);
#if OSAPI_ENABLE_LOG
                if (retcode != DDS_RETCODE_OK)
                {
                    DPSE_LOG_RESET_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR)
                }
#else
                IGNORE_RETVAL(retcode);
#endif
            }
#if OSAPI_ENABLE_LOG
            else
            {
                DPSE_LOG_INVALID_DISCOVERY_SAMPLE(OSAPI_LOGKIND_ERROR,
                                                  info->instance_state)
            }
#endif
            /* Done Processing all the sample in the sequence
             * (for data_seq length)
             */
        }

        retcode = DDS_DataReader_return_loan(reader,
                                (struct DDS_UntypedSampleSeq*)&data_seq,&info_seq);

#if OSAPI_ENABLE_LOG
        if (DDS_RETCODE_OK != retcode)
        {
            DPSE_LOG_RETURN_DISCOVERY_SAMPLE(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(retcode);
#endif
    } while (retcode == DDS_RETCODE_OK);
}

/*ci
 * \brief Initialize the datareader listener for the ParticipantBuiltinTopicData
 *        datareader.
 *
 * \param[in] plugin    The DPSE plugin
 * \param[in] listener  The listener to initialize
 *
 */
void
DPSE_ParticipantBuiltinDataReaderListener_initialize(
                                       struct NDDS_Discovery_Plugin *plugin,
                                       struct DDS_DataReaderListener *listener)
{
    struct DDS_DataReaderListener DEFAULT_LISTENER =
                                        DDS_DataReaderListener_INITIALIZER;

    *listener = DEFAULT_LISTENER;

    listener->as_listener.listener_data = plugin;

    listener->on_data_available =
            DPSE_ParticipantBuiltinDataReaderListener_on_data_available;

    listener->on_before_sample_commit =
            DPSE_ParticipantBuiltinDataReaderListener_on_before_sample_commit;
}

/*ci @} */
