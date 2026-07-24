/*
 * FILE: DataReaderEvent.c - DataReader event implementations
 *
 * Copyright (c) 2008-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 14jul2015,eh MICRO-1429 Independent on_data_available and on_data_on_readers
 *              status updates in on_data_available()
 * 11may2015,eh MICRO-1195/PR#14747 Update comments for on_remote_writer_deleted
 * 20feb2015,eh MICRO-813/PR#9172   Fix Lint warnings
 * 05dec2014,as MICRO-969           Additional fixes for
 * 16sep2014,as MICRO-903/PR#11236  Incorrect handling of status events and
 *                                  listeners
 * 11nov2013,as MICRO-718           Add get_X_status() API to DataReader and
 *                                  DataWriter
 * 08nov2013,as MICRO-681           Complete implementation of WaitSets
 *                                  and support for StatusConditions
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DataReader event implementation
 *
 * \details
 * This file implements functionality to manage various events related to a
 * DDS datareader, such as liveliness, deadlines, return samples to pool etc.
 */
/*ci
 * \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
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
#include "QosPolicy.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "Conditions.h"
#include "DataReaderQos.h"
#include "DataReaderInterface.h"
#include "DataReaderImpl.h"
#include "DataReaderEvent.h"
#include "SubscriberEvent.h"
#include "SubscriberImpl.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Notify a datareader listener of an incompatible qos event
 *
 * \param[in] self The datareader the event occurred on
 * \param[in] key  The key of the incompatible datawriter
 */
void
DDS_DataReaderEvent_on_incompatible_qos(struct DDS_DataReaderImpl *self,
                                        const DDS_BuiltinTopicKey_t *key)
{
    struct DDS_RequestedIncompatibleQosStatus status;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    UNUSED_ARG(key);

    ++self->req_incompatible_qos_status.total_count;
    ++self->req_incompatible_qos_status.total_count_change;
    self->req_incompatible_qos_status.last_policy_id = self->last_policy_id;

    status = self->req_incompatible_qos_status;

    if (self->mask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)
    {
        /* Check that installed listener is not NULL */
        if (self->listener.on_requested_incompatible_qos != NULL)
        {
            DDS_RequestedIncompatibleQosStatus_reset(
                    &self->req_incompatible_qos_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
            self->listener.on_requested_incompatible_qos(
                    self->listener.as_listener.listener_data,self, &status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_requested_incompatible_qos(
                                        self->subscriber,self,&status);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
               DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS, event_consumed);
}

/*ci
 * \brief Notify a datareader listener of a subscription match/unmatch event
 *
 * \details
 *
 * When a datareader matches a datawriter (compatible topic, type, and QoS)
 * the application is notified with a callback. In addition, when the datareader
 * and datawriter no longer matches, for example on datawriter deletion or
 * an incompatible qos due to a change after a match, the application is
 * also notified.
 *
 * \param[in] self          The datareader the event occurred on
 * \param[in] key           The key of the datawriter the event occurred on
 * \param[in] route_existed Whether this match already existed or not
 * \param[in] matched       Whether this was a match or unmatch event
 */
void
DDS_DataReaderEvent_on_subscription_matched(struct DDS_DataReaderImpl *self,
                                            const DDS_BuiltinTopicKey_t *key,
                                            DDS_Boolean route_existed,
                                            DDS_Boolean matched)
{
    struct DDS_SubscriptionMatchedStatus pms;
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

    DDS_InstanceHandle_from_rtps(&pms.last_publication_handle,
                                 (struct RTPS_Guid*)key);

    if (!route_existed && matched)
    {
        ++self->subscription_matched_status.current_count;
        ++self->subscription_matched_status.current_count_change;
        ++self->subscription_matched_status.total_count;
        ++self->subscription_matched_status.total_count_change;
    }
    else if (route_existed && !matched)
    {
        --self->subscription_matched_status.current_count;
        --self->subscription_matched_status.current_count_change;
    }

    DDS_InstanceHandle_from_rtps(
            &self->subscription_matched_status.last_publication_handle,
            (struct RTPS_Guid*)key);

    pms = self->subscription_matched_status;

    if (self->mask & DDS_SUBSCRIPTION_MATCHED_STATUS)
    {
        /* Check that installed listener is not NULL */
        if (self->listener.on_subscription_matched != NULL)
        {
            DDS_SubscriptionMatchedStatus_reset(
                    &self->subscription_matched_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_SUBSCRIPTION_MATCHED_STATUS);
            self->listener.on_subscription_matched(
                    self->listener.as_listener.listener_data,self,&pms);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_subscription_matched(
                                             self->subscriber,self,&pms);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_SUBSCRIPTION_MATCHED_STATUS, event_consumed);
}

/*ci
 * \brief Notify a datareader listener of an data available event
 *
 * \details
 * This function is called when new data has been committed to the datareader
 * sample cache. Note that this function is a listener installed on the
 * cache and do not use some of the arguments. This is because these
 * arguments are not supported by DDS. DDS only notifies of new data and
 * other APIs must be used to retrieve the data.
 *
 * \param[in] rh            The datareader cache with new data
 * \param[in] listener_data Opaque listener data passed from the source
 * \param[in] key           The instance with new data
 * \param[in] sample        The new sample
 */
void
DDS_DataReaderEvent_on_data_available(struct DDSHST_Reader *rh,
                                      void *listener_data,
                                      DDS_InstanceHandle_t *key,
                                      DDSHST_ReaderSample_T *sample)
{
    struct DDS_DataReaderImpl *self = (struct DDS_DataReaderImpl *)listener_data;
    DDS_Boolean event_consumed_dor = DDS_BOOLEAN_FALSE;
    DDS_Boolean event_consumed_dav = DDS_BOOLEAN_FALSE;
    UNUSED_ARG(rh);
    UNUSED_ARG(key);
    UNUSED_ARG(sample);

    DDS_EntityImpl_enable_status_only(&self->as_entity,
                                      DDS_DATA_AVAILABLE_STATUS);

    /* Forward event to Subscriber first, because on_data_on_readers
     * has precedence over the DataReader's on_data_available
     */
    event_consumed_dor = DDS_SubscriberEvent_on_data_on_readers(self->subscriber);

    /* If the event was not consumed by the subscriber and the DataReader
     * has a listener attached to it with the DDS_DATA_AVAILABLE_STATUS
     * enabled, call the listener's on_data_available
     */
    if (!event_consumed_dor)
    {
        if (self->mask & DDS_DATA_AVAILABLE_STATUS)
        {
            if (self->listener.on_data_available != NULL)
            {
                DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_DATA_AVAILABLE_STATUS);
                /* DATA_ON_READERS needs to be reset before calling listener */
                DDS_EntityImpl_disable_status(&self->subscriber->as_entity,
                    DDS_DATA_ON_READERS_STATUS);
                self->listener.on_data_available(
                    self->listener.as_listener.listener_data, self);
                /* Mark event as consumed */
                event_consumed_dav = DDS_BOOLEAN_TRUE;
            }
        }
        else
        {
            /* If the event still hasn't been consumed yet, we propagate the
             * event to the Subscriber as on_data_available. Its listener or
             * the DomainParticipant's listener will be called if enabled and
             * consume the event.
             */
            event_consumed_dav =
                DDS_SubscriberEvent_on_data_available(self->subscriber, self);
        }
    }

    DDS_EntityImpl_enable_status(&self->subscriber->as_entity,
            DDS_DATA_ON_READERS_STATUS, event_consumed_dor);

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_DATA_AVAILABLE_STATUS, event_consumed_dav);
}

/*ci
 * \brief Called by the datareader history cache when a sample is removed
 *       from the cache and is no longer needed
 *
 * \details
 * The datareader creates a pool of samples to receive data in. When a sample
 * is no longer needed, for example removed from the cache, it is returned
 * to the pool. The cache calls this listener when it no longer needs the
 * sample and it can safely be returned to the pool.
 *
 * \param[in] rh            The datareader cache the sample is removed from
 * \param[in] listener_data Opaque listener data passed from the source
 * \param[in] key           The instance that is removed
 * \param[in] sample        The sample that is removed
 */
void
DDS_DataReaderEvent_on_sample_removed(struct DDSHST_Reader *rh,
                                        void *listener_data,
                                        DDS_InstanceHandle_t *key,
                                        DDSHST_ReaderSample_T *sample)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)listener_data;
    struct RTI_DataReaderSample* reader_sample = (struct RTI_DataReaderSample*)sample;

    UNUSED_ARG(rh);
    UNUSED_ARG(key);

    if (sample->holder != NULL)
    {
        if (reader_sample->hst_sample.sample_access_intf != NULL)
        {
            if (!NETIO_SampleI_on_remove(
                                reader_sample->hst_sample.sample_access_intf,
                                sample->holder->sample))
            {
                DDSC_LOG_DR_ON_SAMPLE_REMOVED(OSAPI_LOGKIND_ERROR)
                return;
            }
        }

        DDS_TypePlugin_return_sample(dr->type_plugin,sample->holder);
        sample->holder = NULL;
    }
    REDA_BufferPool_return_buffer(dr->cdr_samples,reader_sample);
}

/*ci
 * \brief Called by the datareader history cache when a sample is rejected
 *        by the cache (not stored)
 *
 * \param[in] rh            The datareader cache the sample was rejected from
 * \param[in] listener_data Opaque listener data passed from the source
 * \param[in] key           The instance that was rejected
 * \param[in] reason        The reason the sample was rejected
 */
void
DDS_DataReaderEvent_on_hst_sample_rejected(struct DDSHST_Reader *rh,
                                           void *listener_data,
                                           DDS_InstanceHandle_t *key,
                                           DDS_SampleRejectedStatusKind reason)
{
    struct DDS_DataReaderImpl *self = (struct DDS_DataReaderImpl *)listener_data;
    struct DDS_SampleRejectedStatus status;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    UNUSED_ARG(rh);

    ++self->sample_rejected_status.total_count;
    ++self->sample_rejected_status.total_count_change;
    self->sample_rejected_status.last_reason = reason;
    self->sample_rejected_status.last_instance_handle = *key;

    status = self->sample_rejected_status;

    if (self->mask & DDS_SAMPLE_REJECTED_STATUS)
    {
        if (self->listener.on_sample_rejected != NULL)
        {
            DDS_SampleRejectedStatus_reset(&self->sample_rejected_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_SAMPLE_REJECTED_STATUS);
            self->listener.on_sample_rejected(
                            self->listener.as_listener.listener_data,
                            self,
                            &status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_sample_rejected(
                                        self->subscriber, self,&status);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_SAMPLE_REJECTED_STATUS, event_consumed);
}

/*ci
 * \brief Handle as sample lost event
 *
 * \details
 *
 * This function may be called, directly or indirectly, by the datareader
 * interface or the downstream interface to indicate that the reader has
 * lost 1 or more samples. The reason for the lost sample is protocol dependent
 * and typical reasons includes removal from the reader/writer cache due to
 * history depth. The reason for why the sample was lost is propagated to the
 * user (an extension to the standard DDS API).
 *
 * \param[in] self The datareader which has lost 1 or more samples
 */
RTI_PRIVATE void
DDS_DataReaderEvent_sample_lost(struct DDS_DataReaderImpl *self)
{
    struct DDS_SampleLostStatus sample_lost_info;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    sample_lost_info = self->sample_lost_status;

    if (self->mask & DDS_SAMPLE_LOST_STATUS)
    {
        if (self->listener.on_sample_lost != NULL)
        {
            DDS_SampleLostStatus_reset(&self->sample_lost_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_SAMPLE_LOST_STATUS);
            self->listener.on_sample_lost(
                  self->listener.as_listener.listener_data,self,&sample_lost_info);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_sample_lost(
                                self->subscriber,self,&sample_lost_info);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_SAMPLE_LOST_STATUS, event_consumed);
}

/*ci
 * \brief Handle as sample lost event from the datareader history cache
 *
 * \details
 * A sample lost event means the application did not process a sample
 * for some reason, for example it was never received from a datawriter
 * or the sample was removed from the cache due to KEEP_LAST.
 *
 * \param[in] rh            The datareader cache the loss occurred on
 * \param[in] listener_data Opaque data pass from the source
 * \param[in] sample_info   A sample_info structure with information about
 *                          the instance being lost. Note that no sample
 *                          data is available.
 * \param[in] reason        The reason the sample was lost
 */
void
DDS_DataReaderEvent_on_hst_sample_lost(struct DDSHST_Reader *rh,
                                       void *listener_data,
                                       struct DDS_SampleInfo *sample_info,
                                       DDS_SampleLostStatusKind reason)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)listener_data;
    UNUSED_ARG(rh);

    ++dr->sample_lost_status.total_count;
    ++dr->sample_lost_status.total_count_change;
    dr->sample_lost_status.reason = reason;

    if (sample_info != NULL)
    {
        dr->sample_lost_status.sample_info = *sample_info;
    }

    DDS_DataReaderEvent_sample_lost(dr);
}

/*ci
 * \brief Handle a sample lost event from the datareader interface
 *
 * \param[in] dr       The datareader the loss occurred on
 * \param[in] source   The datawriter address the loss occurred from
 * \param[in] first_sn The first SN in the loss detection. If count > 1, the
 *                     only guarantee is that count number of samples starting
 *                     on first_sn is lost. That is, it is not guaranteed that
 *                     the last SN lost is (first_sn + (count - 1).
 * \param[in] count    The number of samples lost
 */
void
DDS_DataReaderEvent_on_sample_lost(struct DDS_DataReaderImpl *dr,
                                   struct NETIO_Address *source,
                                   struct REDA_SequenceNumber *first_sn,
                                   RTI_INT32 count)
{
    UNUSED_ARG(first_sn);

    if ((count == 0) ||
        (source->value.rtps_guid.object_id ==
                RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT))
    {
        return;
    }

    dr->sample_lost_status.total_count += count;
    dr->sample_lost_status.total_count_change += count;

    dr->sample_lost_status.reason = DDS_SAMPLE_LOST_BY_DATAWRITER;

    /* Sample info populated with only datawriter handle */
    DDS_InstanceHandle_from_netio_address(
            &dr->sample_lost_status.sample_info.publication_handle, source);

    DDS_DataReaderEvent_sample_lost(dr);
}

/*ci
 * \brief Notify a datareader listener that a liveliness change for an instance
 *        has been detected.
 *
 * \param[in] self       The datareader with a change in instance liveliness
 * \param[in] instance   The instance with the liveliness change
 */
void
DDS_DataReaderEvent_on_liveliness_changed(struct DDS_DataReaderImpl *self,
                                          DDS_InstanceHandle_t *instance)
{
    struct DDS_LivelinessChangedStatus status;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    self->liveliness_changed_status.last_publication_handle = *instance;

    status = self->liveliness_changed_status;

    if (self->mask & DDS_LIVELINESS_CHANGED_STATUS)
    {
        if (self->listener.on_liveliness_changed != NULL)
        {
            DDS_LivelinessChangedStatus_reset(
                &self->liveliness_changed_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                        DDS_LIVELINESS_CHANGED_STATUS);
            self->listener.on_liveliness_changed(
                    self->listener.as_listener.listener_data,
                    self,&status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_liveliness_changed(
                                self->subscriber,self,&status);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_LIVELINESS_CHANGED_STATUS, event_consumed);
}

/*ci
 * \brief Handle liveliness detection from a peer NETIO interface
 *
 * \param[in] dr     The datareader that detected the change in liveliness
 * \param[in] peer   The GUID of the peer that changed to alive
 */
void
DDS_DataReaderEvent_on_liveliness_detected(struct DDS_DataReaderImpl *dr,
                                           struct NETIO_Guid  *peer)
{
    DDS_InstanceHandle_t publisher_handle = DDS_HANDLE_NIL;
    struct DDSHST_ReaderEvent event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return;
    }

    OSAPI_Memory_copy(&publisher_handle.octet,peer,16);
    publisher_handle.is_valid = DDS_BOOLEAN_TRUE;

    if (dr->liveliness_changed_status.not_alive_count > 0)
    {
        --dr->liveliness_changed_status.not_alive_count_change;
        --dr->liveliness_changed_status.not_alive_count;
    }
    ++dr->liveliness_changed_status.alive_count_change;
    ++dr->liveliness_changed_status.alive_count;

    event.kind = DDSHST_READEREVENT_KIND_LIVELINESS_DETECTED;
    event.data.liveliness.rw_guid = publisher_handle;
    event.data.liveliness.instance.is_valid = DDS_BOOLEAN_FALSE;

    DDSHST_Reader_post_event(dr->_rh, &event, &now);

    DDS_DataReaderEvent_on_liveliness_changed(dr,&publisher_handle);
}

/*ci
 * \brief Handle changes in a peer NETIO interface and notify the reader
 *        cache.
 *
 * \param[in] datareader    The datareader with a change in instance liveliness
 * \param[in] source        The peer NETIO interface the event was detected on
 * \param[in] kind          The event kind
 * \param[in] is_inactive   The remote writer has become inactive, signal change
 */
RTI_PRIVATE void
DDS_DataReaderEvent_on_remote_writer_event(struct DDS_DataReaderImpl *datareader,
                                           struct NETIO_Guid *peer,
                                           DDSHST_ReaderEventKind_T kind,
                                           RTI_BOOL is_inactive)
{
    DDS_InstanceHandle_t publisher_handle = DDS_HANDLE_NIL;
    struct DDSHST_ReaderEvent event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return;
    }

    OSAPI_Memory_copy(&publisher_handle.octet,peer,16);
    publisher_handle.is_valid = DDS_BOOLEAN_TRUE;

    event.kind = kind;
    event.data.liveliness.rw_guid = publisher_handle;
    event.data.liveliness.instance.is_valid = DDS_BOOLEAN_FALSE;

    DDSHST_Reader_post_event(datareader->_rh, &event, &now);

    if (is_inactive)
    {
        --datareader->liveliness_changed_status.alive_count;
        --datareader->liveliness_changed_status.alive_count_change;
        ++datareader->liveliness_changed_status.not_alive_count_change;
        ++datareader->liveliness_changed_status.not_alive_count;

        DDS_DataReaderEvent_on_liveliness_changed(datareader,&publisher_handle);
    }
}

/*ci
 * \brief Signal liveliness lost on peer NETIO interface to the datareader
 *        cache
 *
 * \param[in] datareader The datareader with a change in peer liveliness
 * \param[in] peer       The peer GUID the loss was detected on
 */
void
DDS_DataReaderEvent_on_liveliness_lost(struct DDS_DataReaderImpl *datareader,
                                       struct NETIO_Guid *peer)
{
    DDS_DataReaderEvent_on_remote_writer_event(
            datareader,peer,DDSHST_READEREVENT_KIND_LIVELINESS_LOST,RTI_TRUE);
}

/*ci
 * \brief Signal peer NETIO interface deleted to the datareader cache
 *
 * \param[in] datareader  The datareader with a change in instance liveliness
 * \param[in] peer        The peer GUID that was deleted
 * \param[in] is_active   RTI_TRUE when the datawriter is active at the time of
 *                        being deleted.  Otherwise, RTI_FALSE when the
 *                        datawriter is already inactive.
 */
void
DDS_DataReaderEvent_on_remote_writer_deleted(struct DDS_DataReaderImpl *datareader,
                                             struct NETIO_Guid *peer,
                                             RTI_BOOL is_active)
{
    DDS_DataReaderEvent_on_remote_writer_event(
      datareader,peer,DDSHST_READEREVENT_KIND_REMOTE_WRITER_DELETED,is_active);
}

/*ci
 * \brief A key is removed from the reader cache, update local data structures
 *
 * \param[in] rh            The reader cache
 * \param[in] listener_data ReaderHistory listener data
 * \param[in] key           The instance that was removed
 */
void
DDS_DataReaderEvent_on_key_removed(struct DDSHST_Reader *rh,
                                   void *listener_data,
                                   DDS_InstanceHandle_t *key)
{
    struct DDS_DataReaderImpl *self = (struct DDS_DataReaderImpl *)listener_data;
    UNUSED_ARG(rh);

    DDS_DataReaderImpl_delete_local_keyhash(self,key);
}

/*ci
 * \brief Notify a datareader listener that an instance has been replaced in
 *        the reader cache.
 *
 * \details
 *
 * The datareader cache may be configured to replace instances in the cache
 * if the instance resources are exhausted. This may be useful in cases where
 * the instance updates are lost due to KEEP_LAST semantics.
 *
 * \param[in] rh                  The datareader cache the replacement occurred
 *                                in
 * \param[in] listener_data       Opaque listener data passed from the source
 * \param[in] replaced_key        The key that was replaced
 * \param[in] replaced_by_key     The new key added
 * \param[in] publisher           The publisher that replaced the key
 * \param[in] min_removed_samples The minimum number of samples not seen for
 *                                the replaced key due to being replaced.
 */
void
DDS_DataReaderEvent_on_instance_replaced(struct DDSHST_Reader *rh,
                                         void *listener_data,
                                         DDS_InstanceHandle_t *replaced_key,
                                         DDS_InstanceHandle_t *replaced_by_key,
                                         DDS_InstanceHandle_t *publisher,
                                         DDS_Long min_removed_samples)
{
    struct DDS_DataReaderImpl *self = (struct DDS_DataReaderImpl *)listener_data;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    struct DDS_DataReaderInstanceReplacedStatus status;
    UNUSED_ARG(rh);

    ++self->instance_replaced_status.total_count;
    ++self->instance_replaced_status.total_count_change;
    self->instance_replaced_status.last_instance_handle = *replaced_key;
    self->instance_replaced_status.last_replacement_instance = *replaced_by_key;
    self->instance_replaced_status.publication_handle = *publisher;
    self->instance_replaced_status.lost_samples = min_removed_samples;

    status = self->instance_replaced_status;

    if (self->mask & DDS_INSTANCE_REPLACED_STATUS)
    {
        if (self->listener.on_instance_replaced != NULL)
        {
            DDS_DataReaderInstanceReplacedStatus_reset(
                &self->instance_replaced_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_INSTANCE_REPLACED_STATUS);
            self->listener.on_instance_replaced(
                            self->listener.as_listener.listener_data,
                            self,&status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_instance_replaced(
                                self->subscriber,self,
                                &status);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_INSTANCE_REPLACED_STATUS, event_consumed);
}

/*ci
 * \brief Notify a datareader listener that a datawriter as committed 1
 *        or more samples to the reader cache.
 *
 * \param[in] rh                  The datareader cache
 * \param[in] listener_data       Opaque listener data passed from the source
 * \param[in] publisher           The publisher that committed samples
 * \param[in] committed_samples   The number of committed samples
 */
void
DDS_DataReaderEvent_on_sample_committed(struct DDSHST_Reader *rh,
                                        void *listener_data,
                                        DDS_InstanceHandle_t *publisher,
                                        DDS_Long committed_samples)
{
    struct DDS_DataReaderImpl *self = (struct DDS_DataReaderImpl *)listener_data;
    struct NETIO_Event evnt;
    UNUSED_ARG(rh);

    evnt.kind = NETIO_EVENTKIND_RESOURCES_FREED;
    evnt.value.resources_freed.count = committed_samples;

    OSAPI_Memory_copy(&evnt.value.resources_freed.entity,
                      &publisher->octet[0],
                      sizeof(struct NETIO_Guid));

    if (!NETIO_Interface_post_event(self->rtps_intf,NULL,&evnt))
    {
        /* Nothing to do, rely on RTPS signaling the error */
    }
}

#ifdef ENABLE_QOS_DEADLINE
/*ci
 * \brief Notify a datareader listener of a deadline missed event
 *
 * \param[in] self The datareader the event occurred on
 * \param[in] ih   The instance the deadline was missed on
 */
RTI_PRIVATE void
DDS_DataReaderEvent_deadline_missed(struct DDS_DataReaderImpl *self,
                                    DDS_InstanceHandle_t *ih)
{
    struct DDS_RequestedDeadlineMissedStatus dms;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    OSAPI_TRACE_DDS("Deadline missed for reader:",RTI_FALSE)
    OSAPI_TRACE_INT32("ID",self->as_entity.entity_id,RTI_TRUE)

    ++self->req_deadline_missed_status.total_count;
    ++self->req_deadline_missed_status.total_count_change;
    self->req_deadline_missed_status.last_instance_handle = *ih;

    dms = self->req_deadline_missed_status;

    if (self->mask & DDS_REQUESTED_DEADLINE_MISSED_STATUS)
    {
        if (self->listener.on_requested_deadline_missed != NULL)
        {
            DDS_RequestedDeadlineMissedStatus_reset(
                &self->req_deadline_missed_status);
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_REQUESTED_DEADLINE_MISSED_STATUS);
            self->listener.on_requested_deadline_missed(
                self->listener.as_listener.listener_data,
                self,
                &dms);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        event_consumed = DDS_SubscriberEvent_on_requested_deadline_missed(
                            self->subscriber,
                            self,
                            &dms);
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
               DDS_REQUESTED_DEADLINE_MISSED_STATUS, event_consumed);
}

/*ci
 * \brief Handle deadline missed events from the reader history cache
 *
 * \details
 *
 * The DDS reader history interface includes a notification for when
 * a key misses its deadline in the history cache.
 *
 * \param[in] rh            The datareader cache the deadline missed occurred in
 * \param[in] listener_data Opaque pointer passed to the history cache
 * \param[in] key           The instance that missed its deadline
 */
void
DDS_DataReaderEvent_on_deadline_missed(struct DDSHST_Reader *rh,
                                       void *listener_data,
                                       DDS_InstanceHandle_t *key)
{
    struct DDS_DataReaderImpl *datareader =
            (struct DDS_DataReaderImpl *)listener_data;
    UNUSED_ARG(rh);

    DDS_DataReaderEvent_deadline_missed(datareader,key);
}

/*ci
 * \brief Function used to notify instances which has not been updated
 *        in the last deadline period
 *
 * \details
 * The datareader uses one timer to check for deadlines missed on instances.
 * However, the datareader does not maintain instance state, this is left to the
 * history cache implementation, and the datareader only posts an event to the
 * history cache to check the state. The history cache notifies the datareader
 * of which instance has lost liveliness through the
 * \ref DDS_DataReaderEvent_on_deadline_missed event
 *
 * \param[in] storage  Configuration data passed to the timeout event
 *
 * \return Always OSAPI_TIMEOUT_OP_AUTOMATIC restarting the timer
 */
OSAPI_TimeoutOp_t
DDS_DataReaderEvent_on_deadline_timeout(struct OSAPI_TimeoutUserData *storage)
{
    struct DDS_DataReaderImpl *datareader =
                                (struct DDS_DataReaderImpl *)storage->field[0];
    struct DDSHST_ReaderEvent event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    DB_ReturnCode_T dbrc;

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    event.kind = DDSHST_READEREVENT_KIND_DEADLINE_EXPIRED;

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    DDSHST_Reader_post_event(datareader->_rh, &event, &now);

    /* This function is called from a timer event and there is nothing
     * to do if the unlock call fails, ignore the result.
     */
    dbrc = DB_Database_unlock(datareader->config->db);
    IGNORE_RETVAL(dbrc);

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}
#endif

/* see dds_c_subscription.h for documentation */
void
DDS_RequestedDeadlineMissedStatus_reset(struct DDS_RequestedDeadlineMissedStatus *s)
{
    s->total_count_change = 0;
}

/* see dds_c_subscription.h for documentation */
void
DDS_LivelinessChangedStatus_reset(struct DDS_LivelinessChangedStatus *s)
{
    s->alive_count_change = 0;
    s->not_alive_count_change = 0;
}

/* see dds_c_subscription.h for documentation */
void
DDS_RequestedIncompatibleQosStatus_reset(struct DDS_RequestedIncompatibleQosStatus *s)
{
    s->total_count_change = 0;
}

/* see dds_c_subscription.h for documentation */
void
DDS_SampleRejectedStatus_reset(struct DDS_SampleRejectedStatus *s)
{
    s->total_count_change = 0;
}

/* see dds_c_subscription.h for documentation */
void
DDS_DataReaderInstanceReplacedStatus_reset(struct DDS_DataReaderInstanceReplacedStatus *s)
{
    s->total_count_change = 0;
}

/*ci @} */





