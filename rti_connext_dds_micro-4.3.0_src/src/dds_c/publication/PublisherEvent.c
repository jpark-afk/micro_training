/*
 * FILE: PublisherEvent.c - PublisherEvent implementation
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
 * 16sep2014,as MICRO-903/PR#11236 Incorrect handling of status events and listeners
 * 08nov2013,as MICRO-681          Complete implementation of WaitSets
 *                                 and support for StatusConditions
 * 23mar2013,tk Updated            logging
 * 05jul2013,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief PublisherEvent implementation
 *
 * \details
 * This file implements functionality to manage various events related to a
 * DDS publisher, such as handling propagated listener events from contained
 * datawriters and handle discovery events related to creation and deletion of
 * local datawriters.
 *
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
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
#include "PublisherEvent.h"
#include "PublisherImpl.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Listener called by a datawriter for the OFFERED_INCOMPATIBLE_QOS_STATUS
 *        change if not handled by the datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter
 * it is propagated to the publisher which may or may not consume the event.
 * This function checks if the event is consumed by the publisher. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] self   The publisher the event is forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_PublisherEvent_on_offered_incompatible_qos(DDS_Publisher *self,
                    DDS_DataWriter *writer,
                    const struct DDS_OfferedIncompatibleQosStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)
    {
        if (self->listener.as_datawriterlistener.
                on_offered_incompatible_qos != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
            DDS_OfferedIncompatibleQosStatus_reset(
                        &writer->off_incompatible_qos_status);
            self->listener.as_datawriterlistener.
                on_offered_incompatible_qos(self->listener.
                                        as_datawriterlistener.as_listener.
                                        listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (self->config->on_offered_incompatible_qos != NULL)
    {
        event_consumed = self->config->on_offered_incompatible_qos(
                                      self->participant, writer, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datawriter for the PUBLICATION_MATCHED_STATUS
 *        change if not handled by the datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter
 * it is propagated to the publisher which may or may not consume the event.
 * This function checks if the event is consumed by the publisher. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] self   The publisher the event is forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_PublisherEvent_on_publication_matched(DDS_Publisher *self,
                     DDS_DataWriter *writer,
                     const struct DDS_PublicationMatchedStatus *status)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (publisher->mask & DDS_PUBLICATION_MATCHED_STATUS)
    {
        if (publisher->listener.as_datawriterlistener.
                on_publication_matched != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_PUBLICATION_MATCHED_STATUS);
            DDS_PublicationMatchedStatus_reset(
                    &writer->publication_matched_status);
            publisher->listener.as_datawriterlistener.
                on_publication_matched(publisher->listener.as_datawriterlistener.
                                   as_listener.listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (publisher->config->on_publication_matched != NULL)
    {
        event_consumed = publisher->config->on_publication_matched(
                                      publisher->participant, writer, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datawriter for the LIVELINESS_LOST_STATUS
 *        change if not handled by the datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter
 * it is propagated to the publisher which may or may not consume the event.
 * This function checks if the event is consumed by the publisher. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] self   The publisher the event is forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_PublisherEvent_on_liveliness_lost(DDS_Publisher *self,
                                  DDS_DataWriter *writer,
                                  const struct DDS_LivelinessLostStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_LIVELINESS_LOST_STATUS)
    {
        if (self->listener.as_datawriterlistener.
                on_liveliness_lost != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_LIVELINESS_LOST_STATUS);
            DDS_LivelinessLostStatus_reset(&writer->liveliness_lost_status);
            self->listener.as_datawriterlistener.on_liveliness_lost
                (self->listener.as_datawriterlistener.as_listener.
                        listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (self->config->on_liveliness_lost != NULL)
    {
        event_consumed = self->config->on_liveliness_lost(
                                self->participant,writer, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datawriter for the OFFERED_DEADLINE_MISSED_STATUS
 *        change if not handled by the datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter
 * it is propagated to the publisher which may or may not consume the event.
 * This function checks if the event is consumed by the publisher. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] self   The publisher the event is forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_PublisherEvent_on_offered_deadline_missed(DDS_Publisher *self,
                    DDS_DataWriter *writer,
                    const struct DDS_OfferedDeadlineMissedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_OFFERED_DEADLINE_MISSED_STATUS)
    {
        if (self->listener.as_datawriterlistener.
                on_offered_deadline_missed != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_OFFERED_DEADLINE_MISSED_STATUS);
            DDS_OfferedDeadlineMissedStatus_reset(
                    &writer->off_deadline_missed_status);
            self->listener.as_datawriterlistener.
                on_offered_deadline_missed(self->listener.
                                       as_datawriterlistener.as_listener.
                                       listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (self->config->on_offered_deadline_missed != NULL)
    {
        event_consumed = self->config->on_offered_deadline_missed(
                                self->participant, writer, status);
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a datawriter for the
 *        RELIABLE_READER_ACTIVITY_CHANGED_STATUS
 *        change if not handled by the datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter
 * it is propagated to the publisher which may or may not consume the event.
 * This function checks if the event is consumed by the publisher. If the
 * event is not consumed it is propagated to the parent entity.
 *
 * \param[in] self   The publisher the event is forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
DDS_PublisherEvent_on_reliable_reader_activity_changed(DDS_Publisher *self,
                  DDS_DataWriter *writer,
                  const struct DDS_ReliableReaderActivityChangedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS)
    {
        if (self->listener.as_datawriterlistener.
                on_reliable_reader_activity_changed != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS);
            DDS_ReliableReaderActivityChangedStatus_reset(
                     &writer->reliable_reader_activity_changed_status);
            self->listener.as_datawriterlistener.
                on_reliable_reader_activity_changed(self->listener.
                                       as_datawriterlistener.as_listener.
                                       listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else if (self->config->on_reliable_reader_activity_changed != NULL)
    {
        event_consumed = self->config->on_reliable_reader_activity_changed(
                                self->participant, writer, status);
    }

    return event_consumed;
}
/*#endif*/

void
DDS_PublicationMatchedStatus_reset(struct DDS_PublicationMatchedStatus *s)
{
    s->total_count_change = 0;
    s->current_count_change = 0;
}



/*ci @} */
