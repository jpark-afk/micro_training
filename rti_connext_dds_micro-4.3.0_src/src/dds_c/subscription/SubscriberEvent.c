/*
 * FILE: SubscriberEvent.c - Subscriber event implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 05dec2014,as Additional fixes for MICRO-969
 * 16sep2014,as MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 05jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber event implementation
 *
 * \details
 * This file implements functionality to manage various events related to a
 * DDS subscriber, such as handling propagated listener events from contained
 * datareaders and handle discovery events related to creation and deletion of
 * local datareaders.
 */
/*ci \addtogroup DDSSubscriptionModule
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
#include "Conditions.h"
#include "SubscriberQos.h"
#include "SubscriberImpl.h"
#include "SubscriberEvent.h"
#include "DataReaderQos.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Listener called by a datareader for the OFFERED_INCOMPATIBLE_QOS_STATUS
 *        change if not handled by the datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber the event is forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_requested_incompatible_qos(
        DDS_Subscriber *subscriber,
        DDS_DataReader *reader,
        const struct DDS_RequestedIncompatibleQosStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.
                on_requested_incompatible_qos != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
            DDS_RequestedIncompatibleQosStatus_reset(
                    &reader->req_incompatible_qos_status);
            subscriber->listener.as_datareaderlistener.
                on_requested_incompatible_qos(
                    subscriber->listener.as_datareaderlistener.as_listener.
                    listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_requested_incompatible_qos != NULL)
    {
        event_consumed = subscriber->config->on_requested_incompatible_qos(
                                subscriber->participant,reader, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the SUBSCRIPTION_MATCHED_STATUS
 *        change if not handled by the datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber the event is forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_subscription_matched(
                            DDS_Subscriber *subscriber,
                            DDS_DataReader *reader,
                            const struct DDS_SubscriptionMatchedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_SUBSCRIPTION_MATCHED_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.
                 on_subscription_matched != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SUBSCRIPTION_MATCHED_STATUS);
            DDS_SubscriptionMatchedStatus_reset(
                     &reader->subscription_matched_status);
            subscriber->listener.as_datareaderlistener.on_subscription_matched(
                subscriber->listener.as_datareaderlistener.
                as_listener.listener_data,
                reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_subscription_matched != NULL)
    {
        event_consumed = subscriber->config->on_subscription_matched(
                                subscriber->participant,reader, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the ON_DATA_ON_READERS
 *        before handling ON_DATA_AVAILABLE for datareaders
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. This listener is unique in that it is called _before_ notifying
 * a datareader of data available. If this event is consumed, individual
 * datareaders are not notified. This function checks if the event is consumed
 * by the subscriber. If the event is not consumed it is propagated to the
 * parent entity.
 *
 * \param[in] subscriber  The subscriber the event occurred on
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_data_on_readers(DDS_Subscriber *subscriber)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_DATA_ON_READERS_STATUS)
    {
        if (subscriber->listener.on_data_on_readers != NULL)
        {
            DDS_EntityImpl_disable_status(&subscriber->as_entity,
                    DDS_DATA_ON_READERS_STATUS);
            subscriber->listener.on_data_on_readers(subscriber->listener.
                        as_datareaderlistener.as_listener.listener_data,
                        subscriber);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_data_on_readers != NULL)
    {
        event_consumed =
                subscriber->config->on_data_on_readers(
                        subscriber->participant, subscriber);
    }

    /* Macro DDS_EntityImpl_enable_status is not used in this case
     * because the Subscriber's StatusCondition must not be triggered
     * until listeners for DATA_AVAILABLE are also checked.
     * The Subscriber's enabled status/status-condition will be triggered
     * by the DataReader's event-handler (which called this handler)
     */

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the ON_DATA_AVAILABLE
 *        change if not handled by the datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. This listener is unique in that it is called only if the
 * ON_DATA_ON_READERS event is not consumed.  This function checks if the event
 * is consumed by the subscriber. If the event is not consumed it is propagated
 * to the parent entity.
 *
 * \param[in] subscriber  The subscriber the event occurred in
 * \param[in] reader      The reader the event occurred in
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_data_available(DDS_Subscriber *subscriber,
                                      DDS_DataReader *reader)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_DATA_AVAILABLE_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.on_data_available != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_DATA_AVAILABLE_STATUS);
            DDS_EntityImpl_disable_status(&subscriber->as_entity,
                    DDS_DATA_ON_READERS_STATUS);
            subscriber->listener.as_datareaderlistener.on_data_available(
                        subscriber->listener.as_datareaderlistener.
                        as_listener.listener_data,
                        reader);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_data_available != NULL)
    {
        event_consumed = subscriber->config->on_data_available(
                subscriber->participant, subscriber, reader);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the SAMPLE_REJECTED_STATUS
 *        change if not handled by a datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_sample_rejected(
                                DDS_Subscriber * subscriber,
                                DDS_DataReader *reader,
                                const struct DDS_SampleRejectedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_SAMPLE_REJECTED_STATUS)
    {
        if (subscriber->listener
                .as_datareaderlistener.on_sample_rejected != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SAMPLE_REJECTED_STATUS);
            DDS_SampleRejectedStatus_reset(&reader->sample_rejected_status);
            subscriber->listener.as_datareaderlistener.on_sample_rejected(
                        subscriber->listener.as_datareaderlistener.as_listener.
                        listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_sample_rejected != NULL)
    {
        event_consumed = subscriber->config->on_sample_rejected(
                                subscriber->participant,reader,status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the
 *        REQUESTED_DEADLINE_MISSED_STATUS change if not handled by a datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_requested_deadline_missed(
        DDS_Subscriber *subscriber,
        DDS_DataReader *reader,
        const struct DDS_RequestedDeadlineMissedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_REQUESTED_DEADLINE_MISSED_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.
                on_requested_deadline_missed != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_REQUESTED_DEADLINE_MISSED_STATUS);
            DDS_RequestedDeadlineMissedStatus_reset(
                &reader->req_deadline_missed_status);
            subscriber->listener.as_datareaderlistener.on_requested_deadline_missed(
                subscriber->listener.as_datareaderlistener.
                as_listener.listener_data,
                reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_requested_deadline_missed != NULL)
    {
        event_consumed = subscriber->config->on_requested_deadline_missed(
                                subscriber->participant,reader,status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the LIVELINESS_CHANGED_STATUS
 *        change if not handled by a datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_liveliness_changed(DDS_Subscriber *subscriber,
                           DDS_DataReader *reader,
                           const struct DDS_LivelinessChangedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_LIVELINESS_CHANGED_STATUS)
    {
        if (subscriber->listener
                .as_datareaderlistener.on_liveliness_changed != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_LIVELINESS_CHANGED_STATUS);
            DDS_LivelinessChangedStatus_reset(
                &reader->liveliness_changed_status);
            subscriber->listener.as_datareaderlistener.on_liveliness_changed(
                        subscriber->listener.as_datareaderlistener.
                        as_listener.listener_data,
                        reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_liveliness_changed != NULL)
    {
        event_consumed = subscriber->config->on_liveliness_changed(
                                    subscriber->participant,reader,status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the SAMPLE_LOST_STATUS
 *        change if not handled by a datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_sample_lost(DDS_Subscriber *subscriber,
                               DDS_DataReader *reader,
                               const struct DDS_SampleLostStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_SAMPLE_LOST_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.on_sample_lost != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SAMPLE_LOST_STATUS);
            DDS_SampleLostStatus_reset(&reader->sample_lost_status);
            subscriber->listener.as_datareaderlistener.on_sample_lost(
                    subscriber->listener.as_datareaderlistener.as_listener.
                    listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_sample_lost != NULL)
    {
        event_consumed =
                subscriber->config->on_sample_lost(
                            subscriber->participant,reader,status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datareader for the INSTANCE_REPLACED_STATUS
 *        change if not handled by a datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader
 * it is propagated to the subscriber which may or may not consume the event.
 * This function checks if the event is consumed by the subscriber. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] subscriber The subscriber forwarded to
 * \param[in] reader     The DDS datareader the notification originated in
 * \param[in] status     The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_SubscriberEvent_on_instance_replaced(
        DDS_Subscriber *subscriber,
        DDS_DataReader *reader,
        const struct DDS_DataReaderInstanceReplacedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (subscriber->mask & DDS_INSTANCE_REPLACED_STATUS)
    {
        if (subscriber->listener.as_datareaderlistener.
                on_instance_replaced != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_INSTANCE_REPLACED_STATUS);
            DDS_DataReaderInstanceReplacedStatus_reset(
                &reader->instance_replaced_status);
            subscriber->listener.as_datareaderlistener.on_instance_replaced(
                subscriber->listener.as_datareaderlistener.
                as_listener.listener_data,
                reader,status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (subscriber->config->on_instance_replaced != NULL)
    {
        event_consumed = subscriber->config->on_instance_replaced(
                                subscriber->participant,reader,status);
    }

    return event_consumed;
}

void
DDS_SubscriptionMatchedStatus_reset(struct DDS_SubscriptionMatchedStatus *s)
{
    s->total_count_change = 0;
    s->current_count_change = 0;
}

void
DDS_SampleLostStatus_reset(struct DDS_SampleLostStatus *s)
{
    s->total_count_change = 0;
}
/*ci @} */
