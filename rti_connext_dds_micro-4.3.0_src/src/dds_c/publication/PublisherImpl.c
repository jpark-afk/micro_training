/*
 * FILE: PublisherImpl.c - Publisher implementation
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
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
#include "Topic.h"
#include "Type.h"
#include "PublisherImpl.h"
#include "UserDataQosPolicy.h"
#include "QosPolicy.h"
#include "BuiltinCdr.h"
#include "PartitionQosPolicy.h"

#if INCLUDE_API_QOS
#include "DomainParticipant.h"
#endif

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
    return self->management.is_hidden;
}

#ifndef RTI_CERT
RTI_PRIVATE DDS_ReturnCode_t
DDS_PublisherImpl_finalize_managed(
        DDS_Publisher *self,
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
                DDS_USER_DATA_PUBLISHER_TYPE) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }

        OSAPI_Heap_free_struct(self->group_data);
        /* the cast is ok, we never write to this variable unless allocated */
        self->group_data = (struct DDS_GroupDataQosPolicy*)&DDS_GROUP_DATA_QOS_DEFAULT;
    }

    if (self->publisher_name != DDS_ENTITY_NAME_DEFAULT)
    {
        OSAPI_Heap_free_string(self->publisher_name);
        /* the cast is ok, we never write to this variable unless allocated */
        self->publisher_name = (char *)DDS_ENTITY_NAME_DEFAULT;
    }

    return DDS_RETCODE_OK;
}
#endif

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
    if (self->default_qos != &DDS_DATAWRITER_QOS_DEFAULT)
    {
        if (DDS_DataWriterQos_finalize_managed(self->default_qos, self->participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
            return DDS_BOOLEAN_FALSE;
        }
        OSAPI_Heap_free_struct(self->default_qos);
    }
#endif

    if (DDS_PublisherImpl_finalize_managed(self, self->participant) != DDS_RETCODE_OK)
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
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DDS_PublisherListener nil_listener =
                                        DDS_PublisherListener_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos = NULL;
    struct RTI_ManagementQosPolicy m = RTI_MANAGEMENT_QOS_POLICY_DEFAULT;


    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);
    if (!DDS_PartitionQosPolicy_is_consistent_w_limits(&qos->partition,
                                            &dp_qos->resource_limits))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_GroupDataQosPolicy_is_consistent(&qos->group_data,
            dp_qos->resource_limits.publisher_group_data_max_length))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

    if ((listener != NULL) &&
         !DDS_PublisherListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_LISTENER,mask)
        return DDS_BOOLEAN_FALSE;
    }

    publisher->config = config;
    publisher->dw_count = 0;
    publisher->participant = participant;
    /* the cast is ok, we never write to these variables unless allocated */
    publisher->partition = (struct DDS_PartitionQosPolicy*)&DDS_PARTITION_QOS_DEFAULT;
    publisher->group_data = (struct DDS_GroupDataQosPolicy*)&DDS_GROUP_DATA_QOS_DEFAULT;
    publisher->publisher_name = (char*)DDS_ENTITY_NAME_DEFAULT;

    if (listener == NULL)
    {
        publisher->listener = nil_listener;
    }
    else
    {
        publisher->listener = *listener;
    }

    publisher->mask = mask;
    publisher->management = m;
#if INCLUDE_API_QOS
    /* The const is casted away on purpose. We never write to this variable
     * unless it has been allocated from the heap.
     */
    publisher->default_qos = (struct DDS_DataWriterQos *)&DDS_DATAWRITER_QOS_DEFAULT;
#endif

    /* Initialize the Publisher */
#if INCLUDE_API_QOS
    if (qos == &DDS_PUBLISHER_QOS_DEFAULT)
    {
        if (DDS_Publisher_set_qos_from(publisher,
                           participant->_default_publisher_qos,
                           participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (DDS_Publisher_set_qos_from(publisher, qos,
                                       participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
            goto done;
        }
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

DDS_Boolean
DDS_Publisher_serialize(const DDS_Publisher *self,struct CDR_Stream_t *stream)
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
DDS_Publisher_copy_partition(DDS_Publisher *self,
                            struct DDS_PartitionQosPolicy *partition)
{
    return DDS_PartitionQosPolicy_copy(partition,self->partition);
}

struct DDS_PartitionQosPolicy*
DDS_Publisher_get_partition_ref(DDS_Publisher *self)
{
    return self->partition;
}

DDS_ReturnCode_t
DDS_Publisher_copy_group_data(DDS_Publisher *self,
                              struct DDS_GroupDataQosPolicy *group_data)
{
    return DDS_GroupDataQosPolicy_copy(group_data,self->group_data);
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Publisher_delete_datawriter_no_lock(DDS_Publisher *self,
                                        DDS_DataWriter *datawriter)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;
    void *wrapper = NULL;
    DDS_Boolean dw_ok = DDS_BOOLEAN_TRUE;
    DDS_Topic *topic = NULL;
    DDS_DataWriter *dw_record = NULL;

    if (DDS_DataWriter_get_publisher(datawriter) != self)
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_PUBLISHER_ENTITY,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (publisher->config->on_before_datawriter_deleted)
    {
        publisher->config->on_before_datawriter_deleted(datawriter);
    }

    /* Unwrap C datawriter if type plugin contains unwrapping function */
    if ((datawriter->type_plugin != NULL) &&
        DDS_TypePlugin_has_delete_typed_datawriter(datawriter->type_plugin))
    {
        wrapper = DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(datawriter));
        if (wrapper != NULL)
        {
            DDS_TypePlugin_delete_typed_datawriter(datawriter->type_plugin,
                                                   wrapper);
            DDS_Entity_set_wrapper(DDS_DataWriter_as_entity(datawriter), NULL);
        }
    }

    /* Topic cannot be NULL if a writer was successfully created */
    topic = DDS_DataWriter_get_topic(datawriter);

    dbrc = DB_Table_remove_record(publisher->config->local_writer_table,
                                  (DB_Record_T*)&dw_record,
                                  (DB_Key_T)&datawriter->as_entity.entity_id);

    if ((dbrc != DB_RETCODE_OK) || (dw_record != datawriter))
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_RECORD,dbrc)
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

    dw_ok = DDS_DataWriterImpl_finalize(datawriter);

    dbrc = DB_Table_delete_record(publisher->config->local_writer_table,
                                  (DB_Record_T*)datawriter);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_RECORD,dbrc)
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

    --self->dw_count;
    DDS_TopicImpl_detach(topic);

    if (publisher->config->on_after_datawriter_deleted)
    {
        publisher->config->on_after_datawriter_deleted(datawriter);
    }

    if (!dw_ok)
    {
        retval = DDS_RETCODE_ERROR;
    }

done:

    return retval;
}

DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities_no_lock(DDS_Publisher *self)
{
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterImpl *datawriter = NULL;
    DB_Cursor_T handle = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    if (self->management.is_hidden)
    {
        return DDS_RETCODE_OK;
    }

    dbrc = DB_Table_select_all_default(publisher->config->local_writer_table,
                                      &handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              publisher->config->local_writer_table,dbrc)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datawriter);
        if ((dbrc == DB_RETCODE_OK) && (datawriter->publisher == self))
        {
            retcode = DDS_Publisher_delete_datawriter_no_lock(publisher,datawriter);
            if (retcode != DDS_RETCODE_OK)
            {
                DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_DATAWRITER_RECORD,dbrc)
                break;
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(publisher->config->local_writer_table,handle);

    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        retcode = DDS_RETCODE_ERROR;
    }

done:

    return retcode;
}
#endif

DDS_ReturnCode_t
DDS_Publisher_get_qos_from(const DDS_Publisher *self,
                               struct DDS_PublisherQos *out)
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
    if (!REDA_String_copy(out->publisher_name.name,
                          DDS_ENTITYNAME_QOS_NAME_MAX,
                          self->publisher_name))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_Publisher_set_qos_from(
        DDS_Publisher *self,
        const struct DDS_PublisherQos *in,
        DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (in == NULL) || (participant == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

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
                DDS_USER_DATA_PUBLISHER_TYPE,
                &in->group_data.value,
                &self->group_data->value))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (REDA_String_compare(self->publisher_name,in->publisher_name.name))
    {
#ifndef RTI_CERT
        if (self->publisher_name != DDS_ENTITY_NAME_DEFAULT)
        {
            REDA_String_free(self->publisher_name);
        }
#endif
        self->publisher_name = REDA_String_dup(in->publisher_name.name);
    }

    return DDS_RETCODE_OK;
}

/*ci @} */
