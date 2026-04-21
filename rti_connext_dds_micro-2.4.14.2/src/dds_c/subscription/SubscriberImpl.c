/*
 * FILE: SubscriberImpl.c - DDS Subscriber implementation
 *
 * (c) Copyright 2008-2021 Real-Time Innovations,
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Fixed parameter names in comment for DDS_SubscriberListener_is_consistent
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 16sep2014,as MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 04jun2012,tk Major update
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS Subscriber implementation
 *
 * \details
 * The functions in this file is related to the management of a DDS subscriber.
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
#include "InstanceHandle.h"
#include "Conditions.h"
#include "DataReaderImpl.h"
#include "SubscriberEvent.h"
#include "SubscriberQos.h"
#include "SubscriberImpl.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of subscriber entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_SubscriberImpl already in the database
 * \param[in] op2   Either a DDS_SubscriberImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_SubscriberImpl_compare(RTI_INT32 flags,
                           const DB_Record_T op1, void *op2)
{
    struct DDS_SubscriberImpl *record_left = (struct DDS_SubscriberImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_SubscriberImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

/*ci
 * \brief Check if a subscriber is hidden.
 *
 * \details
 *
 * A subscriber that is hidden is not included in functions that operate on
 * an entire factory, such as delete_containted_entities. Entities that
 * are hidden must be explicitly deleted.
 *
 * \param[in] self Subscriber to test
 *
 * \return DDS_BOOLEAN_TRUE if hidden, DDS_BOOLEAN_FALSE if not hidden
 */
DDS_Boolean
DDS_SubscriberImpl_is_hidden(struct DDS_SubscriberImpl *self)
{
    return self->qos.management.is_hidden;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a subscriber
 *
 * \details
 * Free up all resources used by a subscriber. Note that this function does
 * not free the memory used to hold the subscriber itself, this memory is
 * freed by the participant because the participant is the factory.
 *
 * \param[in] self subscriber to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriberImpl_finalize(struct DDS_SubscriberImpl *self)
{

    if (self->dr_count != 0)
    {
        DDSC_LOG_ENTITY_NOT_EMPTY(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_SUBSCRIBER_ENTITY,self->dr_count)
        return DDS_BOOLEAN_FALSE;
    }

#if INCLUDE_API_QOS
    if (DDS_DataReaderQos_finalize(&self->default_qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        return DDS_BOOLEAN_FALSE;
    }
#endif

    if (DDS_SubscriberQos_finalize(&self->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
#endif

/*ci
 * \brief Return the instance handle of a subscriber
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function.
 *
 * \param[in] entity The base-class for the subscriber
 *
 * \return The instance handle
 */
DDS_InstanceHandle_t
DDS_SubscriberImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_SubscriberImpl *sub = (struct DDS_SubscriberImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = sub->config->get_parent_handle((DDS_Entity*)sub->participant);

    DDS_InstanceHandle_set_suffix(&retval,sub->as_entity.entity_id);

    return retval;
}

/*ci
 * \brief Initialize a subscriber
 *
 * \details
 * The participant allocates memory to store the subscriber data and passes
 * it to the subscriber for initialization. The subscriber allocates all its
 * internal resources. The subscriber is passed shared resources in the
 * config structure, such as database, timers resolvers etc. The resources
 * are typically managed by the domain participant.
 *
 * \param[in] subscriber  A subscriber structure to initialize
 * \param[in] participant The participant creating the subscriber
 * \param[in] qos         The subscriber qos policy
 * \param[in] listener    The subscriber listener
 * \param[in] mask        Mask with enabled statuses on the subscriber
 * \param[in] object_id   The subscriber object id generated by the factory
 * \param[in] config      General subscriber configuration

 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriberImpl_initialize(struct DDS_SubscriberImpl *subscriber,
                              DDS_DomainParticipant *participant,
                              const struct DDS_SubscriberQos *qos,
                              const struct DDS_SubscriberListener *listener,
                              DDS_StatusMask mask,
                              DDS_UnsignedLong object_id,
                              struct NDDS_SubscriberConfig *config)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
#endif
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    struct DDS_SubscriberListener nil_listener =
        DDS_SubscriberListener_INITIALIZER;

#if !defined(INCLUDE_API_QOS)
    struct DDS_SubscriberQos DEFAULT_SUB_QOS = DDS_SubscriberQos_INITIALIZER;
#endif

    if ((qos != &DDS_SUBSCRIBER_QOS_DEFAULT) &&
        !DDS_SubscriberQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

    if ((listener != NULL) &&
            !DDS_SubscriberListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_LISTENER,mask)
        return DDS_BOOLEAN_FALSE;
    }

#if defined(INCLUDE_API_QOS)
    retcode = DDS_DataReaderQos_initialize(&subscriber->default_qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS,retcode)
        return DDS_BOOLEAN_FALSE;
    }
#endif

    /* Initialize the Subscriber */
    if (qos == &DDS_SUBSCRIBER_QOS_DEFAULT)
    {
#if defined(INCLUDE_API_QOS)
        if (DDS_DomainParticipant_get_default_subscriber_qos(participant,
                &subscriber->qos) != DDS_RETCODE_OK)
        {
            goto done;
        }
#else
        subscriber->qos = DEFAULT_SUB_QOS;
#endif
    }
    else
    {
        subscriber->qos = *qos;
    }

    if (!DDS_EntityImpl_initialize(&subscriber->as_entity,
                          DDS_SUBSCRIBER_ENTITY_KIND,
                          object_id,
                          DDS_Subscriber_enable,
                          DDS_SubscriberImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_ENTITY)
        goto done;
    }

    subscriber->config = config;
    subscriber->dr_count = 0;
    subscriber->participant = participant;

    if (listener == NULL)
    {
        subscriber->listener = nil_listener;
    }
    else
    {
        subscriber->listener = *listener;
    }

    subscriber->mask = mask;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        DDS_SubscriberImpl_finalize(subscriber);
    }
#endif

    return retval;
}

/*ci
 * \brief Check if a DDS_SubscriberListener is consistent
 *
 * \param[in] l DDS_SubscriberListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
DDS_Boolean
DDS_SubscriberListener_is_consistent(const struct DDS_SubscriberListener *l,
                                     DDS_StatusMask m)
{
    return (DDS_DataReaderListener_is_consistent(
            &(l->as_datareaderlistener),m) &&
            ((!((m) & DDS_DATA_ON_READERS_STATUS)) ||
                    (l->on_data_on_readers != NULL))) ? 
             DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */
