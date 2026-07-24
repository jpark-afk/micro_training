/*
 * FILE: DataWriterEvent.c - DataWriter events functionality
 *
 * (c) Copyright 2008-2023 Real-Time Innovations,
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
 * 05dec2014,as MICRO-969 Additional fixes
 * 16sep2014,as MICRO-903/PR#11236 Incorrect handling of status
 *                                 events and listeners
 * 04mar2014,tk MICRO-84  Support on_sample_removed
 * 03feb2014,eh MICRO-714 Discard unacked samples in on_sample_removed
 * 27jan2014,eh MICRO-714 Add on_reliable_reader_activity_changed()
 * 11nov2013,as MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *                        and support for StatusConditions
 * 30apr2008,tk Created
 */
/*ci
 * \file
 * \brief DataWriter events functionality
 *
 * \details
 * This file implements functionality to manage various events related to a
 * DDS datawriter, such as liveliness, deadlines, samples returned to pool etc.
 */
/*ci
 * \addtogroup DDSPublicationModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
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

#include "Entity.h"
#include "DataWriterImpl.h"
#include "DataWriterQos.h"
#include "DataWriterEvent.h"
#include "DataWriterInterface.h"
#include "PublisherEvent.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Notify a datawriter listener of an incompatible qos event
 *
 * \param[in] self  The datawriter the event occurred on
 * \param[in] key   The key of the incompatible datareader
 */
void
DDS_DataWriterEvent_on_incompatible_qos(struct DDS_DataWriterImpl *self,
                                        const DDS_BuiltinTopicKey_t *key)
{
    struct DDS_OfferedIncompatibleQosStatus status;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    UNUSED_ARG(key);

    ++self->off_incompatible_qos_status.total_count;
    ++self->off_incompatible_qos_status.total_count_change;

    status = self->off_incompatible_qos_status;

    if (self->mask & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)
    {
        if (self->listener.on_offered_incompatible_qos != NULL)
        {
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
            DDS_OfferedIncompatibleQosStatus_reset(
                        &self->off_incompatible_qos_status);
            self->listener.on_offered_incompatible_qos(
                    self->listener.as_listener.listener_data,
                    self, &status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_PublisherEvent_on_offered_incompatible_qos(
                                        self->publisher,self,&status);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS, event_consumed);
}

/*ci
 * \brief Notify a datawriter listener of an publication match or unmatch event
 *
 * \details
 *
 * When a datawriter matches a datareader (compatible topic, type, and QoS)
 * the application is notified with a callback. In addition, when the datawriter
 * and datareader no longer matches, for example on datareader deletion or
 * an incompatible qos due to a change after a match, the application is
 * also notified.
 *
 * \param[in] self          The datawriter the event occurred on
 * \param[in] key           The key of the matched/unmatched datareader
 * \param[in] route_existed Whether this match already existed or not
 * \param[in] matched       Whether this was a match or unmatch event
 */
void
DDS_DataWriterEvent_on_publication_matched(struct DDS_DataWriterImpl *self,
                                           const DDS_BuiltinTopicKey_t *key,
                                           DDS_Boolean route_existed,
                                           DDS_Boolean matched)
{
    struct DDS_PublicationMatchedStatus pms;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if ((route_existed) && matched)
    {
        return;
    }

    if (!route_existed && !matched)
    {
        /* Nothing to do. Probably an impossible case,
         * but we check for it and return (works as a
         * precondition for the rest of the code).
         */
        return;
    }

    if (!route_existed && matched)
    {
        ++self->publication_matched_status.current_count;
        ++self->publication_matched_status.current_count_change;
        ++self->publication_matched_status.total_count;
        ++self->publication_matched_status.total_count_change;
    }
    else if (route_existed && !matched)
    {
        --self->publication_matched_status.current_count;
        --self->publication_matched_status.current_count_change;
    }

    DDS_InstanceHandle_from_rtps(
            &self->publication_matched_status.last_subscription_handle,
            (struct RTPS_Guid*)key);

    pms = self->publication_matched_status;

    if (self->mask & DDS_PUBLICATION_MATCHED_STATUS)
    {
        if (self->listener.on_publication_matched != NULL)
        {
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_PUBLICATION_MATCHED_STATUS);
            DDS_PublicationMatchedStatus_reset(
                    &self->publication_matched_status);
            self->listener.on_publication_matched(
                    self->listener.as_listener.listener_data,self,&pms);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_PublisherEvent_on_publication_matched(
                                             self->publisher,self,&pms);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_PUBLICATION_MATCHED_STATUS, event_consumed);
}

#ifdef ENABLE_QOS_DEADLINE
/*ci
 * \brief Notify a datawriter listener of a deadline missed event
 *
 * \param[in] self  The datawriter the event occurred on
 * \param[in] ih    The instance the deadline was missed on
 */
RTI_PRIVATE void
DDS_DataWriterEvent_deadline_missed(struct DDS_DataWriterImpl *self,
                                    DDS_InstanceHandle_t *ih)
{
    struct DDS_OfferedDeadlineMissedStatus dms;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    ++self->off_deadline_missed_status.total_count;
    ++self->off_deadline_missed_status.total_count_change;
    self->off_deadline_missed_status.last_instance_handle = *ih;

    dms = self->off_deadline_missed_status;

    if (self->mask & DDS_OFFERED_DEADLINE_MISSED_STATUS)
    {
        if (self->listener.on_offered_deadline_missed != NULL)
        {
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_OFFERED_DEADLINE_MISSED_STATUS);
            DDS_OfferedDeadlineMissedStatus_reset(
                    &self->off_deadline_missed_status);
            self->listener.on_offered_deadline_missed(
                    self->listener.as_listener.listener_data,
                    self,
                    &dms);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_PublisherEvent_on_offered_deadline_missed(
                self->publisher,self,&dms);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_OFFERED_DEADLINE_MISSED_STATUS, event_consumed);
}
#endif

/*ci
 * \brief Timeout handler to detect that the datawriter lost liveliness
 *
 * \details
 * The liveliness timer is restarted every time activity is detected on the
 * datawriter. If the timer expires this function is called and the liveliness
 * listener is called (if installed).
 *
 * \param[in] storage Timeout data passed to the event handler when the
 *                    timer was created.
 *
 * \return Always OSAPI_TIMEOUT_OP_AUTOMATIC, restarting the timer
 */
OSAPI_TimeoutOp_t
DDS_DataWriterEvent_on_liveliness(struct OSAPI_TimeoutUserData *storage)
{
    struct DDS_DataWriterImpl *self;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    struct DDS_LivelinessLostStatus lls;

    self = (struct DDS_DataWriterImpl *)storage->field[0];

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    ++self->liveliness_lost_status.total_count;
    ++self->liveliness_lost_status.total_count_change;

    lls = self->liveliness_lost_status;

    /* delete timeout until there is activity in the DW again */
    self->writer_state = WRITERSTATE_NOT_ALIVE;
    if (!OSAPI_Timer_delete_timeout(self->config->timer,
                                    &self->liveliness_event))
    {
        DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT)
    }

    if (self->mask & DDS_LIVELINESS_LOST_STATUS)
    {
        if (self->listener.on_liveliness_lost != NULL)
        {
            DDS_EntityImpl_disable_status(&self->as_entity,
                                          DDS_LIVELINESS_LOST_STATUS);
            DDS_LivelinessLostStatus_reset(&self->liveliness_lost_status);
            self->listener.on_liveliness_lost(
                            self->listener.as_listener.listener_data,self,&lls);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_PublisherEvent_on_liveliness_lost(
                                       self->publisher, self,&lls);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_LIVELINESS_LOST_STATUS, event_consumed);

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}

/*ci
 * \brief Handle sample removed events from the writer history
 *
 * \details
 * The DDS writer history interface includes a notification for when
 * a sample is removed from the history cache. Because the history cache
 * does not store actual data, just pointers to the data, it is up to the
 * user of this history cache callback to appropriately release any resources
 * used by the history sample.
 *
 * If the sample is forcefully removed the downstream interface is also
 * notified to cancel any more attempts to deliver the sample.
 *
 * \param[in] config    Configuration data passed to the history cache
 * \param[in] key       The instance the history sample belonged to
 * \param[in] sample    The removed sample
 * \param[in] sn        The sequence number for the removed entry
 * \param[in] kind      The reason for the sample being removed
 * \param[in] ack_count The number of outstanding acknowledgments (only
 *                      applicable for reliable communication)
 */
void
DDS_DataWriterEvent_on_sample_removed(void *config,
                                      DDS_InstanceHandle_t *key,
                                      struct DDSHST_WriterSample *sample,
                                      struct REDA_SequenceNumber *sn,
                                      DDSHST_WriterSampleRemovedKind_T kind,
                                      DDS_Long ack_count)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)config;
    struct DDS_DataWriterSample *cdr_sample =
                                    (struct DDS_DataWriterSample *)sample;
    struct DDS_ReliableSampleUnacknowledgedStatus unack_status;
    UNUSED_ARG(key);
    RTI_BOOL brc;

    if (kind != DDSHST_WRITER_SAMPLE_REMOVED_NORMAL)
    {
        if (datawriter->listener.on_reliable_sample_unacknowledged != NULL)
        {
            unack_status.sequence_number = *(struct DDS_SequenceNumber_t*)sn;
            unack_status.unacknowledged_count = ack_count;
            unack_status.instance_handle = *key;

            datawriter->listener.on_reliable_sample_unacknowledged(
                    datawriter->listener.as_listener.listener_data,
                    datawriter,&unack_status);
        }
    }

    if (!OSAPI_Mutex_take(datawriter->write_lock))
    {
        OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR,-1)
        return;
    }

    if (datawriter->rtps_intf != NULL)
    {
        /* When this function is called all samples downstream is returned
         * in the same path. Thus, when this function returns, all loans on
         * this sample are returned and the sample can be returned.
         */
        if (!NETIO_Interface_xmit_remove(datawriter->dw_intf, NULL,sn))
        {
            DDSC_LOG_NETIO_FORCED_REMOVE(OSAPI_LOGKIND_WARNING)
        }
    }


    if (cdr_sample == NULL)
    {
        brc = OSAPI_Mutex_give(datawriter->write_lock); 
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,-1)
        }
#else   
        IGNORE_RETVAL(brc);
#endif
        return;
    }

    /* Indicate that the sample is actually removed. This may free up
     * additional resources.
     */
    cdr_sample->rtps_flags |= NETIO_RTPS_FLAGS_REMOVED;

    OSAPI_Trace_write("return sample and cdr buffer for %S on topic %s",
                      sn,
                      DDS_TopicDescription_get_name(
                              DDS_Topic_as_topicdescription(datawriter->topic)),
                              NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

    DDS_DataWriter_return_sample_payload(datawriter,cdr_sample);
    REDA_BufferPool_return_buffer(datawriter->sample_pool,cdr_sample);

    if (datawriter->mask & DDS_DATA_WRITER_SAMPLE_REMOVED_STATUS)
    {
        if ((datawriter->listener.on_sample_removed != NULL ))
        {
            if (cdr_sample->instance_data != NULL)
            {
                struct DDS_Cookie_t loaned_sample_cookie = DDS_COOKIE_DEFAULT;

                /* To avoid dynamic memory allocation when resizing a sequence
                 * we will loan a sequence this buffer. Max size is 8 to accomdate
                 * 8 byte pointers in 64 bit systems
                 */
                DDS_Octet temporary_loaned_buffer[8];

                if (!DDS_OctetSeq_loan_contiguous(
                        &loaned_sample_cookie.value,
                        temporary_loaned_buffer,
                        sizeof(void*),
                        sizeof(void*)))
                {
                    /* Because there is no specific log message for octet sequence
                     * we re-use initialize passing in a specific cookie sequence kind
                     */
                    DDSC_LOG_SEQ_INITIALIZE(
                            OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_COOKIE_PAYLOAD_SEQUENCE);
                }

                *OSAPI_Compiler_reinterpret_cast(void**,
                        DDS_OctetSeq_get_contiguous_buffer(
                                &loaned_sample_cookie.value)) =
                                        (void*)cdr_sample->instance_data;

                datawriter->listener.on_sample_removed(
                        datawriter->listener.as_listener.listener_data,
                        datawriter,
                        &loaned_sample_cookie);
            }
        }
    }

    OSAPI_Trace_write("returned sample and cdr buffer for %S on topic %s",
                      sn,
                      DDS_TopicDescription_get_name(
                              DDS_Topic_as_topicdescription(datawriter->topic)),
                              NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

    brc = OSAPI_Mutex_give(datawriter->write_lock); 
#if OSAPI_ENABLE_LOG
    if (!brc)
    {
        OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,-1)
    }
#else   
    IGNORE_RETVAL(brc);
#endif
}

#ifndef RTI_CERT
/*ci
 * \brief Handle key removed events from the writer history
 *
 * \details
 * The DDS writer history interface includes a notification for when
 * a key is removed from the history cache. It is up to the
 * user of this history cache callback to appropriately release any resources
 * used by the history key.
 *
 * \param[in] config    Configuration data passed to the history cache
 * \param[in] key       The instance that was removed
 * \param[in] kind      The reason for the key being removed
 */
void
DDS_DataWriterEvent_on_key_removed(void *config,
                                   DDS_InstanceHandle_t *key,
                                   DDSHST_WriterKeyRemovedKind_T kind)
{
    UNUSED_ARG(key);
    UNUSED_ARG(config);
    UNUSED_ARG(kind);
}
#endif


/*ci
 * \brief Handle deadline missed events from the writer history
 *
 * \details
 * The DDS writer history interface includes a notification for when
 * a key misses its deadline in the history cache.
 *
 * \param[in] listener_data  Configuration data passed to the history cache
 * \param[in] key            The instance the deadline was missed on
 */
void
DDS_DataWriterEvent_on_deadline_missed(void *listener_data,
                                        DDS_InstanceHandle_t *key)
{
    struct DDS_DataWriterImpl *datawriter =
                                    (struct DDS_DataWriterImpl *)listener_data;

    DDS_DataWriterEvent_deadline_missed(datawriter,key);
}

/*ci
 * \brief Check for instances which has not been updated
 *        in the last deadline period
 *
 * \details
 * The datawriter uses one timer to check for deadlines missed. However,
 * the datawriter does not maintain instance state, this is left to the
 * history cache implementation. The datawriter only posts an event to the
 * history cache to check the state. The history cache notifies the datawriter
 * of which instance has lost liveliness through the
 * \ref DDS_DataWriterEvent_on_deadline_missed event.
 *
 * \param[in] storage  Configuration data passed to the timeout event
 *
 * \return Always OSAPI_TIMEOUT_OP_AUTOMATIC, restarting the timer
 */
OSAPI_TimeoutOp_t
DDS_DataWriterEvent_on_deadline_expired(struct OSAPI_TimeoutUserData *storage)
{
    struct DDS_DataWriterImpl *datawriter =
                                (struct DDS_DataWriterImpl *)storage->field[0];
    struct DDSHST_WriterEvent event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    event.kind = DDSHST_WRITEREVENT_KIND_DEADLINE_EXPIRED;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    DDSHST_Writer_post_event(datawriter->wh, &event, &now);

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}

/*ci
 * \brief Notify a datawriter listener of a change in the state of a matched
 *        reliable reader
 *
 * \param[in] self The datawriter the event occurred on
 */
void
DDS_DataWriterEvent_on_reliable_reader_activity_changed(
                                            struct DDS_DataWriterImpl *self)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    struct DDS_ReliableReaderActivityChangedStatus status;

    /* status is set by DDS_DataWriterInterface_post_event */

    status = self->reliable_reader_activity_changed_status;

     if (self->mask & DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS)
     {
         if (self->listener.on_reliable_reader_activity_changed != NULL)
         {
             DDS_EntityImpl_disable_status(&self->as_entity,
                DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS);
             DDS_ReliableReaderActivityChangedStatus_reset(
                     &self->reliable_reader_activity_changed_status);
             self->listener.on_reliable_reader_activity_changed(
                self->listener.as_listener.listener_data, self,&status);
             event_consumed = DDS_BOOLEAN_TRUE;
         }
     }
     else
     {
         event_consumed =
            DDS_PublisherEvent_on_reliable_reader_activity_changed(
                self->publisher, self, &status);
     }

     DDS_EntityImpl_enable_status(&self->as_entity,
                DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS, event_consumed);
}

void
DDS_OfferedDeadlineMissedStatus_reset(struct DDS_OfferedDeadlineMissedStatus *s)
{
    s->total_count_change = 0;
}

void
DDS_LivelinessLostStatus_reset(struct DDS_LivelinessLostStatus *s)
{
    s->total_count_change = 0;
}

void
DDS_OfferedIncompatibleQosStatus_reset(
                             struct DDS_OfferedIncompatibleQosStatus *s)
{
    s->total_count_change = 0;
}

void
DDS_ReliableReaderActivityChangedStatus_reset(
                             struct DDS_ReliableReaderActivityChangedStatus *s)
{
    s->active_count_change = 0;
    s->inactive_count_change = 0;
}

/*ci @} */

