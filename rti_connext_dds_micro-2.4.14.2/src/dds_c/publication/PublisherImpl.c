/*
 * FILE: PublisherImpl.c - Publisher implementation
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
 * 16sep2014,as MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 23mar2012,tk Logging updates
 * 04jun2012,tk Major update
 * 30apr2008,tk Written
 */
/*ce
 * \file
 * \brief Publisher implementation
 *
 * \details
 * The functions in this file is related to the management of a DDS publisher.
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
#include "InstanceHandle.h"
#include "PublisherQos.h"
#include "DataWriterImpl.h"
#include "DataWriterQos.h"
#include "PublisherImpl.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of publisher entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between the op2 being a key or record
 * \param[in] op1   A DDS_PublisherImpl record already in the database
 * \param[in] op2   Either a DDS_PublisherImpl being added or a DDS_Long key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         0 if op1 = op2
 */
RTI_INT32
DDS_PublisherImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_PublisherImpl *record_left = (struct DDS_PublisherImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_PublisherImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

/*ci
 * \brief Check if a publisher is hidden.
 *
 * \details
 * A publisher that is hidden is not included in functions that operate on
 * an entire factory, such as delete_containted_entities. Entities that
 * are hidden must be explicitly deleted.
 *
 * \param[in] self Publisher to test
 *
 * \return DDS_BOOLEAN_TRUE if hidden, DDS_BOOLEAN_FALSE if not hidden
 */
DDS_Boolean
DDS_PublisherImpl_is_hidden(DDS_Publisher *self)
{
    return self->qos.management.is_hidden;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a publisher
 *
 * \details
 * Free up all resources used by a publisher. Note that this function does
 * not free the memory used to hold the publisher itself, this memory is
 * freed by the participant because the participant is the factory.
 *
 * \param[in] self publisher to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublisherImpl_finalize(struct DDS_PublisherImpl *self)
{
    if (self->dw_count != 0)
    {
        DDSC_LOG_ENTITY_NOT_EMPTY(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_PUBLISHER_ENTITY,self->dw_count)
        return DDS_BOOLEAN_FALSE;
    }

#if INCLUDE_API_QOS
    if (DDS_DataWriterQos_finalize(&self->default_qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        return DDS_BOOLEAN_FALSE;
    }
#endif

    if (DDS_PublisherQos_finalize(&self->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
#endif

/*ci
 * \brief Return the instance handle of a publisher
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function.
 *
 * \param[in] entity The base-class for the publisher
 *
 * \return The instance handle
 */
DDS_InstanceHandle_t
DDS_PublisherImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_PublisherImpl *pub = (struct DDS_PublisherImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = pub->config->get_parent_handle((DDS_Entity*)pub->participant);

    DDS_InstanceHandle_set_suffix(&retval,pub->as_entity.entity_id);

    return retval;
}

/*ci
 * \brief Initialize a publisher
 *
 * \details
 * The participant allocates memory to store the publisher data and passes
 * it to the publisher for initialization. The publisher allocates all its
 * internal resources. The publisher is passed shared resources in the
 * config structure, such as database, timers resolvers etc. The shared
 * resources are typically managed by the domain participant.
 *
 * \param[in] publisher   A publisher structure to initialize
 * \param[in] participant The participant creating the publisher
 * \param[in] qos         The publisher qos policy
 * \param[in] listener    The publisher listener
 * \param[in] mask        Mask with enabled statuses on the publisher
 * \param[in] object_id   The publisher object id generated by the factory
 * \param[in] config      General publisher configuration

 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublisherImpl_initialize(struct DDS_PublisherImpl *publisher,
                             DDS_DomainParticipant *participant,
                             const struct DDS_PublisherQos *qos,
                             const struct DDS_PublisherListener *listener,
                             DDS_StatusMask mask,
                             DDS_UnsignedLong object_id,
                             struct NDDS_PublisherConfig *config)
{
#if INCLUDE_API_QOS
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
#endif
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    struct DDS_PublisherListener nil_listener = 
        DDS_PublisherListener_INITIALIZER;

#if !(INCLUDE_API_QOS)
    struct DDS_PublisherQos DEFAULT_PUB_QOS = DDS_PublisherQos_INITIALIZER;
#endif

    if ((qos != &DDS_PUBLISHER_QOS_DEFAULT) &&
        !DDS_PublisherQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

    if ((listener != NULL) &&
         !DDS_PublisherListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_LISTENER,mask)
        return DDS_BOOLEAN_FALSE;
    }

#if INCLUDE_API_QOS
    retcode = DDS_DataWriterQos_initialize(&publisher->default_qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS,retcode)
        goto done;
    }
#endif

    /* Initialize the Publisher */
    if (qos == &DDS_PUBLISHER_QOS_DEFAULT)
    {
#if INCLUDE_API_QOS
        if (DDS_DomainParticipant_get_default_publisher_qos(participant,
                                 &publisher->qos) != DDS_RETCODE_OK)
        {
            goto done;
        }
#else
        /* must be DDS-default QoS */
        publisher->qos = DEFAULT_PUB_QOS;
#endif
    }
    else
    {
        publisher->qos = *qos;
    }

    if (!DDS_EntityImpl_initialize(&publisher->as_entity,
                          DDS_PUBLISHER_ENTITY_KIND,
                          object_id,
                          DDS_Publisher_enable,
                          DDS_PublisherImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_ENTITY)
        goto done;
    }

    publisher->config = config;
    publisher->dw_count = 0;
    publisher->participant = participant;

    if (listener == NULL)
    {
        publisher->listener = nil_listener;
    }
    else
    {
        publisher->listener = *listener;
    }

    publisher->mask = mask;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        (void)DDS_PublisherImpl_finalize(publisher);
    }
#endif
    return retval;
}
/*ci
 * \brief Check if a DDS_PublisherListener is consistent
 *
 * \param[in] l DDS_PublisherListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
DDS_Boolean
DDS_PublisherListener_is_consistent(const struct DDS_PublisherListener *l,
                                    DDS_StatusMask m)
{
    return DDS_DataWriterListener_is_consistent(&l->as_datawriterlistener,m);
}

/*ci @} */
