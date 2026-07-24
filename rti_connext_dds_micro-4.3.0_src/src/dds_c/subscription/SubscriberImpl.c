/*
 * FILE: SubscriberImpl.c - DDS Subscriber implementation
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
#include "Topic.h"
#include "Type.h"
#include "DataReaderImpl.h"
#include "SubscriberEvent.h"
#include "SubscriberQos.h"
#include "SubscriberImpl.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"
#include "QosPolicy.h"
#include "BuiltinCdr.h"
#include "UserDataQosPolicy.h"
#include "PartitionQosPolicy.h"

#if INCLUDE_API_QOS
#include "DomainParticipant.h"
#endif

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
    return self->management.is_hidden;
}

#ifndef RTI_CERT
RTI_PRIVATE DDS_ReturnCode_t
DDS_Subscriber_finalize_managed(
        DDS_Subscriber *self,
        DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t rtn;

    if (self->partition != &DDS_PARTITION_QOS_DEFAULT)
    {
        if (DDS_PartitionQosPolicy_finalize_no_dealloc(self->partition,
            DDS_DomainParticipant_get_partition_string_manager(participant))
                != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }

        rtn = DDS_PartitionQosPolicy_finalize(self->partition);
        if (rtn != DDS_RETCODE_OK)
        {
            return rtn;
        }

        OSAPI_Heap_free_struct(self->partition);
        /* the cast is ok, we never write to this variable unless allocated */
        self->partition = (struct DDS_PartitionQosPolicy*)&DDS_PARTITION_QOS_DEFAULT;
    }

    if (self->group_data != &DDS_GROUP_DATA_QOS_DEFAULT)
    {
        if (DDS_GroupDataQosPolicy_finalize_no_dealloc(
                self->group_data,
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_SUBSCRIBER_TYPE) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }

        OSAPI_Heap_free_struct(self->group_data);
        /* the cast is ok, we never write to this variable unless allocated */
        self->group_data = (struct DDS_GroupDataQosPolicy*)&DDS_GROUP_DATA_QOS_DEFAULT;
    }

    if (self->subscriber_name != DDS_ENTITY_NAME_DEFAULT)
    {
        OSAPI_Heap_free_string(self->subscriber_name);
        /* the cast is ok, we never write to this variable unless allocated */
        self->subscriber_name = (char*)DDS_ENTITY_NAME_DEFAULT;
    }

    return DDS_RETCODE_OK;
}
#endif

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
    if (self->default_qos != &DDS_DATAREADER_QOS_DEFAULT)
    {
        if (DDS_DataReaderQos_finalize_managed(self->default_qos, self->participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
            return DDS_BOOLEAN_FALSE;
        }
        OSAPI_Heap_free_struct(self->default_qos);
    }
#endif

    if (DDS_Subscriber_finalize_managed(self, self->participant) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
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
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    struct DDS_SubscriberListener nil_listener =
                                            DDS_SubscriberListener_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos = NULL;
    struct RTI_ManagementQosPolicy m = RTI_MANAGEMENT_QOS_POLICY_DEFAULT;

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);
    if (!DDS_PartitionQosPolicy_is_consistent_w_limits(&qos->partition,
                                            &dp_qos->resource_limits))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_GroupDataQosPolicy_is_consistent(&qos->group_data,
            dp_qos->resource_limits.subscriber_group_data_max_length))
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

    subscriber->config = config;
    subscriber->dr_count = 0;
    subscriber->participant = participant;
    /* the cast is ok, we never write to these variables unless allocated */
    subscriber->partition = (struct DDS_PartitionQosPolicy*)&DDS_PARTITION_QOS_DEFAULT;
    subscriber->group_data = (struct DDS_GroupDataQosPolicy*)&DDS_GROUP_DATA_QOS_DEFAULT;
    subscriber->subscriber_name = (char*)DDS_ENTITY_NAME_DEFAULT;

    if (listener == NULL)
    {
        subscriber->listener = nil_listener;
    }
    else
    {
        subscriber->listener = *listener;
    }

    subscriber->mask = mask;
    subscriber->management = m;
#if INCLUDE_API_QOS
    /* The const is casted away on purpose. We never write to this variable
     * unless it has been allocated from the heap.
     */
    subscriber->default_qos = (struct DDS_DataReaderQos*)&DDS_DATAREADER_QOS_DEFAULT;
#endif

    /* Initialize the Subscriber */
#if defined(INCLUDE_API_QOS)
    if (qos == &DDS_SUBSCRIBER_QOS_DEFAULT)
    {
        if (DDS_Subscriber_set_qos_from(subscriber,
                                        participant->_default_subscriber_qos,
                                        participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (DDS_Subscriber_set_qos_from(subscriber, qos,
                                        participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
            goto done;
        }
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
 * \param[in] self DDS_SubscriberListener to test for consistency
 * \param[in] mask The current status mask to test
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

DDS_Boolean
DDS_Subscriber_serialize(const DDS_Subscriber *self,struct CDR_Stream_t *stream)
{
    if (!DDS_PartitionQosPolicy_is_equal(self->partition,
                                         &DDS_PARTITION_QOS_DEFAULT))
    {
        if (!DDS_CdrQosPolicy_serialize_partition(stream,self->partition,NULL))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (!DDS_GroupDataQosPolicy_is_equal(self->group_data,
                                         &DDS_GROUP_DATA_QOS_DEFAULT))
    {
        if (!DDS_CdrQosPolicy_serialize_group_data(stream,self->group_data))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_ReturnCode_t
DDS_Subscriber_copy_partition(DDS_Subscriber *self,
                             struct DDS_PartitionQosPolicy *partition)
{
    return DDS_PartitionQosPolicy_copy(partition,self->partition);
}

struct DDS_PartitionQosPolicy*
DDS_Subscriber_get_partition_ref(DDS_Subscriber *self)
{
    return self->partition;
}

DDS_ReturnCode_t
DDS_Subscriber_copy_group_data(DDS_Subscriber *self,
                               struct DDS_GroupDataQosPolicy *group_data)
{
    return DDS_GroupDataQosPolicy_copy(group_data,self->group_data);
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Subscriber_delete_datareader_no_lock(DDS_Subscriber *self,
                                         DDS_DataReader *datareader)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;
    void *wrapper = NULL;
    DDS_Boolean dr_ok = DDS_BOOLEAN_TRUE;
    DDS_TopicDescription *topic_desc;
    DDS_DataReader *dr_record = NULL;

    if (DDS_DataReader_get_subscriber(datareader) != self)
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_SUBSCRIBER_ENTITY,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (subscriber->config->on_before_datareader_deleted)
    {
        subscriber->config->on_before_datareader_deleted(datareader);
    }

    if ((datareader->type_plugin != NULL) &&
        DDS_TypePlugin_has_delete_typed_datareader(datareader->type_plugin))
    {
        wrapper = DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(datareader));
        if (wrapper != NULL)
        {
            DDS_TypePlugin_delete_typed_datareader(datareader->type_plugin,
                                                   wrapper);
            DDS_Entity_set_wrapper(DDS_DataWriter_as_entity(datareader), NULL);
        }
    }

    topic_desc = DDS_DataReader_get_topicdescription(datareader);

    dbrc = DB_Table_remove_record(subscriber->config->local_reader_table,
                                  (DB_Record_T*)&dr_record,
                                  (DB_Key_T)&datareader->as_entity.entity_id);

    if ((dbrc != DB_RETCODE_OK) || (dr_record != datareader))
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_RECORD,dbrc)
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

    dr_ok = DDS_DataReaderImpl_finalize(datareader);

    dbrc = DB_Table_delete_record(subscriber->config->local_reader_table,
                                  (DB_Record_T)datareader);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_RECORD,dbrc)
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

    --subscriber->dr_count;
    DDS_TopicImpl_detach(DDS_Topic_narrow(topic_desc));

    if (!dr_ok)
    {
        retval = DDS_RETCODE_ERROR;
    }

done:

    return retval;

}

DDS_ReturnCode_t
DDS_Subscriber_delete_contained_entities_no_lock(DDS_Subscriber *self)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl*)self;
    struct DDS_DataReaderImpl *datareader = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T handle = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    if (self->management.is_hidden)
    {
        return DDS_RETCODE_OK;
    }

    dbrc = DB_Table_select_all_default(subscriber->config->local_reader_table,
                                      &handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              subscriber->config->local_reader_table,dbrc)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datareader);
        if ((dbrc == DB_RETCODE_OK) && (datareader->subscriber == self))
        {
            retcode = DDS_Subscriber_delete_datareader_no_lock(subscriber,datareader);
            if (retcode != DDS_RETCODE_OK)
            {
                DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_RECORD,dbrc)
                break;
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(subscriber->config->local_reader_table,handle);

    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        retcode = DDS_RETCODE_ERROR;
    }

done:

    return retcode;
}
#endif

DDS_ReturnCode_t
DDS_Subscriber_get_qos_from(const DDS_Subscriber *self,
                                struct DDS_SubscriberQos *out)
{
    if (DDS_BOOLEAN_TRUE !=
            DDS_PartitionQosPolicy_copy(&out->partition,self->partition))
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_RETCODE_OK !=
            DDS_GroupDataQosPolicy_copy(&out->group_data,self->group_data))
    {
        return DDS_RETCODE_ERROR;
    }

    out->entity_factory = self->entity_factory;
    out->management = self->management;
    if (!REDA_String_copy(out->subscriber_name.name,
                         DDS_ENTITYNAME_QOS_NAME_MAX,
                         self->subscriber_name))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_Subscriber_set_qos_from(
        DDS_Subscriber *self,
        const struct DDS_SubscriberQos *in,
        DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (in == NULL) || (participant == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)
    /* entity_factory
     * management
     * partition (ptr)
     * group_data (ptr)
     * subscriber_name (ptr)
     */
    self->entity_factory = in->entity_factory;
    self->management = in->management;

    if (!DDS_PartitionQosPolicy_is_equal(self->partition,&in->partition))
    {
        if (self->partition == &DDS_PARTITION_QOS_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->partition,
                                       struct DDS_PartitionQosPolicy);
            if (self->partition == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            DDS_PartitionQosPolicy_initialize(self->partition);
        }

        if (!DDS_StringSeq_set_maximum(&self->partition->name,
             DDS_StringSeq_get_length(&in->partition.name)))
        {
            return DDS_RETCODE_ERROR;
        }

        if (!DDS_PartitionQosPolicy_set_from(self->partition,&in->partition,
              DDS_DomainParticipant_get_partition_string_manager(participant)))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (!DDS_GroupDataQosPolicy_is_equal(self->group_data,&in->group_data))
    {
        if (self->group_data == &DDS_GROUP_DATA_QOS_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->group_data,
                                       struct DDS_GroupDataQosPolicy);
            if (self->group_data == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            DDS_GroupDataQosPolicy_initialize(self->group_data);
        }

        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_SUBSCRIBER_TYPE,
                &in->group_data.value,
                &self->group_data->value))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (REDA_String_compare(self->subscriber_name,in->subscriber_name.name))
    {
#ifndef RTI_CERT
        if (self->subscriber_name != DDS_ENTITY_NAME_DEFAULT)
        {
            REDA_String_free(self->subscriber_name);
        }
#endif
        self->subscriber_name = REDA_String_dup(in->subscriber_name.name);
    }

    return DDS_RETCODE_OK;
}

/*ci @} */
