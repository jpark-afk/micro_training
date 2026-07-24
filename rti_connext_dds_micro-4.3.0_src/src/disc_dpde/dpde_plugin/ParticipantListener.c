/*
 * FILE: ParticipantListener.c - Participant listener API
 *
 * (c) Copyright 2011-2024 Real-Time Innovations,
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
 * \brief Participant listener API
 */
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
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

#include "DiscoveryPlugin.h"
#include "ParticipantDiscovery.h"
#include "ParticipantListener.h"
#include "PublicationDiscovery.h"
#include "SubscriptionDiscovery.h"

/*** SOURCE_BEGIN ***/

/* ------------------------------------------------------------------------ */
/*                Participant Discovery Listener                            */
/* ------------------------------------------------------------------------ */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_requested_deadline_missed(
                        void *listener_data,
                        DDS_DataReader* reader,
                        const struct DDS_RequestedDeadlineMissedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ce \dref_ParticipantBuiltinDataReaderListener_LivelinessChangedCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_liveliness_changed(
                    void *listener_data,
                    DDS_DataReader *reader,
                    const struct DDS_LivelinessChangedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ce \dref_ParticipantBuiltinDataReaderListener_RequestedIncompatibleQosCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_requested_incompatible_qos(
                    void *listener_data,
                    DDS_DataReader *reader,
                    const struct DDS_RequestedIncompatibleQosStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ce \dref_ParticipantBuiltinDataReaderListener_SampleRejectedCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_sample_rejected(
                    void *listener_data,
                    DDS_DataReader *reader,
                    const struct DDS_SampleRejectedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ci
 * \brief Handle on_before_commit events
 *
 * \details
 * The DPDE plugin uses the on_before_commit event on the datareader to
 * filter out discovery data for participant that have not been asserted.
 *
 * \param[in] listener_data Opaque pointer passed to the datareader
 * \param[in] reader        The datareader the event occurred on
 * \param[in] sample        The sample which are about to be committed
 * \param[in] sample_info   Partially filled sample_info with only
 *                          reception_sequence_number being valid
 * \param[out] dropped      Set to DDS_BOOLEAN_FALSE to keep the sample, and
 *                          DDS_BOOLEAN_TRUE to drop the sample
 * \return DDS_BOOLEAN_FALSE if the function succeeded, DDS_BOOLEAN_FALSE
 *         it not
 */
RTI_PRIVATE DDS_Boolean
DPDE_ParticipantBuiltinDataReaderListener_on_before_sample_commit(
                                void *listener_data,
                                DDS_DataReader *reader,
                                const void *const sample,
                                const struct DDS_SampleInfo *const sample_info,
                                DDS_Boolean *dropped)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    struct DPDE_DiscoveryPlugin *dpde =
                            (struct DPDE_DiscoveryPlugin *)listener_data;
    struct DDS_ParticipantBuiltinTopicData *data =
                            (struct DDS_ParticipantBuiltinTopicData*)sample;
    struct NETIO_Address dst_reader;
    DDS_DataWriter *writer = NULL;
    DDS_UnsignedLong dst_reader_id = 0;

    UNUSED_ARG(sample_info);

    if (!DDS_DomainParticipant_is_checksum_compatible(dpde->participant,data))
    {
        DPDE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED(OSAPI_LOGKIND_WARNING)
        OSAPI_TRACE_DDS("[DPDE] ignored participant with incompatible CRC",RTI_FALSE);
        OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE);

        *dropped = DDS_BOOLEAN_TRUE;
        result = DDS_BOOLEAN_TRUE;
        goto done;
    }

    if (reader == dpde->participant_reader)
    {
        writer = dpde->participant_writer;
        dst_reader_id = RTPS_OBJECT_ID_READER_SDP_PARTICIPANT;
    }
    else
    {
        goto done;
    }

    *dropped = DDS_BOOLEAN_FALSE;

    if (!dpde->ignore_unknown_peers)
    {
        result = DDS_BOOLEAN_TRUE;
        goto done;
    }

    /* Next, if ignore_unknown_peers is true then check if the peer is in the
     * participant writer's route table. If not, drop the sample. Need to check
     * both unicast and multicast locators.
     */
    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.value.rtps_guid.object_id = NETIO_htonl(dst_reader_id);

    if (DDS_DataWriter_lookup_route(
            writer, &dst_reader, &data->metatraffic_multicast_locators) ||
       DDS_DataWriter_lookup_route(
            writer, &dst_reader, &data->metatraffic_unicast_locators))
    {
        result = DDS_BOOLEAN_TRUE;
        goto done;
    }

    *dropped = DDS_BOOLEAN_TRUE;
    result = DDS_BOOLEAN_TRUE;

    OSAPI_TRACE_DDS("[DPDE] ignored participant not in local peer list",RTI_FALSE);
    OSAPI_TRACE_STRING("name",data->participant_name.name,RTI_TRUE);

done:

    return result;
}

/*ce \dref_ParticipantBuiltinDataReaderListener_DataAvailableCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_data_available(
                                                        void *listener_data,
                                                        DDS_DataReader *reader)
{
    DDS_ReturnCode_t retcode;
    struct DPDE_DiscoveryPlugin *dpde_plugin =
                                (struct DPDE_DiscoveryPlugin *)listener_data;
    DDS_Long i,l;
    DDS_DomainParticipant *participant = NULL;
    struct DDS_ParticipantBuiltinTopicDataSeq data_seq =
                            DDS_ParticipantBuiltinTopicDataSeq_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq  = DDS_SEQUENCE_INITIALIZER;
    struct DDS_ParticipantBuiltinTopicData *data = NULL;
    struct DDS_SampleInfo *info = NULL;
    struct DDS_BuiltinTopicKey_t key;

    participant = DDS_Subscriber_get_participant(dpde_plugin->subscriber);

    do
    {
        retcode = DDS_DataReader_take(reader,
                (struct DDS_UntypedSampleSeq*)&data_seq,&info_seq, DDS_LENGTH_UNLIMITED,
                DDS_ANY_VIEW_STATE,DDS_ANY_SAMPLE_STATE,DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            break;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            DPDE_LOG_TAKE_PARTICIPANT_SAMPLE(OSAPI_LOGKIND_ERROR,retcode)
            break;
        }

        l = DDS_ParticipantBuiltinTopicDataSeq_get_length(&data_seq);
        for (i = 0; i < l; ++i)
        {
            data = (struct DDS_ParticipantBuiltinTopicData *)
                               DDS_UntypedSampleSeq_get_reference(
                                    (struct DDS_UntypedSampleSeq*)&data_seq,i);
            info = DDS_SampleInfoSeq_get_reference(&info_seq,i);

            DDS_InstanceHandle_to_rtps((struct RTPS_Guid *)&key,
                                               &info->instance_handle);

            if (info->valid_data)
            {
                DPDE_ParticipantDiscovery_assert_remote_participant(
                        dpde_plugin, participant, data, info);
            }
            else
            {
                if (info->instance_state == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE
                    || info->instance_state == DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
                {
#ifndef RTI_CERT
                    OSAPI_TRACE_DDS("[DPDE] participant disposed/not-alive",RTI_FALSE)
                    OSAPI_TRACE_GUID("key",&key,RTI_TRUE)

                    retcode = NDDS_DomainParticipant_remove_remote_participant(
                                                    participant, &key);
                    if (retcode != DDS_RETCODE_OK)
                    {
                        DPDE_LOG_REMOVE_REMOTE_PARTICIPANT(OSAPI_LOGKIND_ERROR,
                                                           retcode)
                    }
#endif /* !RTI_CERT */
                }
                else
                {
                    DPDE_LOG_INVALID_PARTICIPANT_SAMPLE(OSAPI_LOGKIND_ERROR,info->instance_state)
                }
            }
        }

        retcode = DDS_DataReader_return_loan(reader,
                       (struct DDS_UntypedSampleSeq*)&data_seq,&info_seq);

    } while (retcode == DDS_RETCODE_OK);

    if ((DDS_RETCODE_OK != retcode) && (retcode != DDS_RETCODE_NO_DATA))
    {
        DPDE_LOG_RETURN_PARTICIPANT_SAMPLE(OSAPI_LOGKIND_ERROR,retcode)
    }
}

/*ce \dref_ParticipantBuiltinDataReaderListener_SubscriptionMatchedCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_subscription_matched(
                        void *listener_data,
                        DDS_DataReader *reader,
                        const struct DDS_SubscriptionMatchedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ce \dref_ParticipantBuiltinDataReaderListener_SampleLostCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_sample_lost(
                        void *listener_data,
                        DDS_DataReader *reader,
                        const struct DDS_SampleLostStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

/*ce \dref_ParticipantBuiltinDataReaderListener_InstanceReplacedCallback
 */
RTI_PRIVATE void
DPDE_ParticipantBuiltinDataReaderListener_on_instance_replaced(
   void *listener_data,
   DDS_DataReader *reader,
   const struct DDS_DataReaderInstanceReplacedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(reader);
    UNUSED_ARG(status);
}

void
DPDE_ParticipantBuiltinDataReaderListener_initialize(
                        struct NDDS_Discovery_Plugin *plugin,
                        struct DDS_DataReaderListener *listener)
{
    struct DDS_DataReaderListener DEFAULT_LISTENER =
                                        DDS_DataReaderListener_INITIALIZER;

    *listener = DEFAULT_LISTENER;

    listener->as_listener.listener_data = plugin;

    listener->on_requested_deadline_missed =
        DPDE_ParticipantBuiltinDataReaderListener_on_requested_deadline_missed;

    listener->on_requested_incompatible_qos =
        DPDE_ParticipantBuiltinDataReaderListener_on_requested_incompatible_qos;

    listener->on_sample_rejected =
        DPDE_ParticipantBuiltinDataReaderListener_on_sample_rejected;

    listener->on_liveliness_changed =
        DPDE_ParticipantBuiltinDataReaderListener_on_liveliness_changed;

    listener->on_data_available =
        DPDE_ParticipantBuiltinDataReaderListener_on_data_available;

    listener->on_subscription_matched =
        DPDE_ParticipantBuiltinDataReaderListener_on_subscription_matched;

    listener->on_sample_lost =
        DPDE_ParticipantBuiltinDataReaderListener_on_sample_lost;

    listener->on_instance_replaced =
        DPDE_ParticipantBuiltinDataReaderListener_on_instance_replaced;

    listener->on_before_sample_commit =
        DPDE_ParticipantBuiltinDataReaderListener_on_before_sample_commit;
}
