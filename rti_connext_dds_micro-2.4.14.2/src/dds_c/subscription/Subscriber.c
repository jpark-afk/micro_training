/*
 * FILE: Subscriber.c - Subscriber implementation
 *
 * (c) Copyright 2008-2020 Real-Time Innovations,
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
 * 20oct2021,tk MICRO-3311/PR.29816
 * - Removed race-conditions in _enable()
 * 04apr2021,tk MICRO-2845/PR.28682
 *   - Removed empty blocks in DDS_Subscriber_create_datareader()
 * 27jun2016,tk MICRO-1545 Properly keep track of when a topic is in use
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 28jul2015,tk MICRO-1470/PR#15623 Do not enable contained entities unless
 *                                  auto_enable is true (in enable())
 * 09feb2015,tk MICRO-1003/PR#13282 Do not free resource on failure
 * 02feb2015,tk MICRO-1038/PR#13520 Assign retcode in case enable fails
 * 21jan2015,tk MICRO-1003/PR#13282 increment counter after creation
 *              MICRO-1004/PR#13284 Removed comment
 *              MICRO-1005/PR#13285 Log pointer, not value
 *              MICRO-1006/PR#13286 Fixed loop condition
 *              MICRO-1008/PR#13302 Fixed log message
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 16sep2014,as MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 05may2014,as MICRO-270 Always enable precondition
 *              checks for public API operations
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 19jul2013,as Added support for C++
 * 04jun2012,tk Major update
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber implementation
 *
 * \details
 * This file implements the public DDS subscriber API. Support functions are
 * found in the SubscriberNNN.c files.
 *
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
#include "Topic.h"
#include "Conditions.h"
#include "SubscriberEvent.h"
#include "SubscriberQos.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"
#include "DataReaderImpl.h"
#include "SubscriberImpl.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_Subscriber_enable(DDS_Entity *self)
{
    DB_ReturnCode_T dbrc;
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DB_Cursor_T handle;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;
    struct DDS_DataReaderImpl *reader;

    OSAPI_PRECONDITION(subscriber == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (subscriber->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        goto done;
    }

    if (!DDS_Entity_is_enabled
        (DDS_DomainParticipant_as_entity(subscriber->participant)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        ddsrc = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    self->state = RTIDDS_ENTITY_STATE_ENABLED;

    if (!subscriber->qos.entity_factory.autoenable_created_entities)
    {
        goto done;
    }

    dbrc = DB_Table_select_all_default(subscriber->config->local_reader_table,
                                       &handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              subscriber->config->local_reader_table,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(handle, (DB_Record_T*) &reader);
        if ((dbrc == DB_RETCODE_OK) && (reader->subscriber == subscriber))
        {
            ddsrc = DDS_DataReader_enable(&reader->as_entity);
            if (ddsrc != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_DATAREADER_ENTITY)
                DB_Cursor_finish(subscriber->config->local_reader_table,handle);
                goto done;
            }
        }
    }  while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(subscriber->config->local_reader_table,handle);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        ddsrc = DDS_RETCODE_ERROR;
    }

 done:

     if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
     {
         return DDS_RETCODE_ERROR;
     }

     return ddsrc;
}

DDS_DataReader*
DDS_Subscriber_create_datareader(DDS_Subscriber *self,
                                 DDS_TopicDescription *topic_desc,
                                 const struct DDS_DataReaderQos *qos,
                                 const struct DDS_DataReaderListener *listener,
                                 DDS_StatusMask mask)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    struct DDS_DataReaderImpl *datareader = NULL;
    DDS_DataReader *retval = NULL;
    DB_ReturnCode_T dbrc;
    DDS_UnsignedLong dr_oid;
    void* wrapper = NULL;
    DDS_ReturnCode_t ddsrc;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dr_key;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (topic_desc == NULL) || (qos == NULL),
                           return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("topic",topic_desc,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DDS_TopicDescription_get_participant(topic_desc) !=
        DDS_Subscriber_get_participant(subscriber))
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_TOPIC_ENTITY,DDSC_LOG_SUBSCRIBER_ENTITY)
        return NULL;
    }

    ih = DDS_Entity_get_instance_handle(DDS_Subscriber_as_entity(subscriber));
    DDS_InstanceHandle_to_rtps((struct RTPS_Guid *)&dr_key,&ih);

    subscriber->config->dr_config->db = subscriber->config->db;
    subscriber->config->dr_config->registry = subscriber->config->registry;
    subscriber->config->dr_config->timer = subscriber->config->timer;
    subscriber->config->dr_config->on_after_enabled = subscriber->config->on_after_datareader_enabled;
    subscriber->config->dr_config->get_parent_handle = DDS_SubscriberImpl_get_instance_handle;
    subscriber->config->dr_config->default_multicast = subscriber->config->default_multicast;
    subscriber->config->dr_config->default_unicast = subscriber->config->default_unicast;
    subscriber->config->dr_config->default_meta_multicast = subscriber->config->default_meta_multicast;
    subscriber->config->dr_config->default_meta_unicast = subscriber->config->default_meta_unicast;
    subscriber->config->dr_config->enabled_transports = subscriber->config->enabled_transports;
    subscriber->config->dr_config->route_resolver = subscriber->config->route_resolver;
    subscriber->config->dr_config->bind_resolver = subscriber->config->bind_resolver;
    subscriber->config->dr_config->participant_id = subscriber->config->participant_id;
    subscriber->config->dr_config->domain_id = subscriber->config->domain_id;

    datareader = NULL;

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }
    
    if (qos->management.is_hidden)
    {
        dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = qos->protocol.rtps_object_id;
    }
    else
    {
        if (qos->protocol.rtps_object_id == DDS_RTPS_AUTO_ID)
        {
            dr_oid = subscriber->config->object_id_generator(
                            DDS_Subscriber_get_participant(subscriber));
        }
        else
        {
            dr_oid = qos->protocol.rtps_object_id;
        }

        dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
             DDS_Topic_get_object_suffix(DDS_Topic_narrow(topic_desc),
                                         qos->type_support.plugin_data,
                                         dr_oid,RTI_FALSE);
    }

    if (dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] == 0)
    {
        dbrc = DB_Database_unlock(subscriber->config->db);
        /* ignore this error as we're returning an error anyway */
        IGNORE_RETVAL(dbrc);
        return NULL;
    }

    /* First check if it is ok to create a datawriter upstream. For example,
     * a discovery plugin may be out of resources.
     */
    ddsrc = subscriber->config->on_before_datareader_created(
                                        subscriber,&dr_key,DDS_BOOLEAN_TRUE);
    if (ddsrc != DDS_RETCODE_OK)
    {
        dbrc = DB_Database_unlock(subscriber->config->db);
        /* ignore this error as we're returning an error anyway */
        IGNORE_RETVAL(dbrc);
        return NULL;
    }

    dbrc = DB_Table_create_record(subscriber->config->local_reader_table,
                                 (DB_Record_T*)&datareader);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_RECORD,dbrc)
        goto done;
    }

    if (!DDS_DataReaderImpl_initialize(datareader,subscriber,topic_desc,qos,
                           listener,mask,
                           dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID],
                           subscriber->config->dr_config))
    {
#ifndef RTI_CERT
        (void)DB_Table_delete_record(subscriber->config->local_reader_table,
                                     (DB_Record_T)datareader);
        datareader = NULL;
#endif
        goto done;
    }

    /* Wrap C datareader if type plugin contains wrapping function */
    if (datareader->type_plugin->create_typed_datareader != NULL)
    {
        wrapper = datareader->type_plugin->create_typed_datareader(datareader);
        if (wrapper != NULL)
        {
            DDS_Entity_set_wrapper(DDS_DataWriter_as_entity(datareader),
                                   wrapper);
        }
        else
        {
#ifndef RTI_CERT
            (void)DDS_DataReaderImpl_finalize(datareader);
            (void)DB_Table_delete_record(subscriber->config->local_reader_table,
                                         (DB_Record_T)datareader);
            datareader = NULL;
#endif
            DDSC_LOG_DR_CREATE_TYPED_READER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    dbrc = DB_Table_insert_record(subscriber->config->local_reader_table,
                                  (DB_Record_T)datareader);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_RECORD,dbrc)
        goto done;
    }

    DDS_TopicImpl_attach(DDS_Topic_narrow(topic_desc));
    ++subscriber->dr_count;

    if ((self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) &&
         self->qos.entity_factory.autoenable_created_entities)
    {
        if (DDS_DataReader_enable(DDS_DataReader_as_entity(datareader))
                != DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
            goto done;
        }
    }

    retval = (DDS_DataReader*)datareader;

done:

#ifndef RTI_CERT
    if ((retval == NULL) && (datareader != NULL))
    {
        if (datareader->type_plugin != NULL
                      && datareader->type_plugin->create_typed_datareader != NULL)
        {
            wrapper = DDS_Entity_get_wrapper(
                           DDS_DataWriter_as_entity(datareader));
            if (wrapper != NULL
                           && datareader->type_plugin->delete_typed_datareader != NULL)
            {
                datareader->type_plugin->delete_typed_datareader(wrapper);
                /* the function has a lock on the database so no need to
                 * set user data to NULL because the datareader will be
                 * deleted before the lock is released.
                 */
            }
        }
        (void)DDS_Subscriber_delete_datareader(subscriber,datareader);
        datareader = NULL;
    }
    else if (datareader == NULL)
    {
        ddsrc = subscriber->config->on_before_datareader_created(
                                    subscriber,&dr_key,DDS_BOOLEAN_FALSE);
        IGNORE_RETVAL(ddsrc);
    }

#endif

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
#ifndef RTI_CERT
        if (datareader != NULL)
        {
            (void)DDS_Subscriber_delete_datareader(subscriber,datareader);
        }
#endif
        return NULL;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Subscriber_delete_datareader(DDS_Subscriber *self,
                                 DDS_DataReader *datareader)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;
    void *wrapper = NULL;
    DDS_Boolean dr_ok = DDS_BOOLEAN_TRUE;
    DDS_TopicDescription *topic_desc;

    OSAPI_PRECONDITION((self == NULL) || (datareader == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("datareader",datareader,RTI_TRUE);)

    if (DDS_DataReader_get_subscriber(datareader) != self)
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_SUBSCRIBER_ENTITY,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (subscriber->config->on_before_datareader_deleted)
    {
        subscriber->config->on_before_datareader_deleted(datareader);
    }

    if (datareader->type_plugin != NULL
            && datareader->type_plugin->delete_typed_datareader != NULL)
    {
           wrapper = DDS_Entity_get_wrapper(
                    DDS_DataWriter_as_entity(datareader));
           if (wrapper != NULL)
           {
               datareader->type_plugin->delete_typed_datareader(wrapper);
               DDS_Entity_set_wrapper(
                                   DDS_DataWriter_as_entity(datareader), NULL);
           }
       }

    topic_desc = DDS_DataReader_get_topicdescription(datareader);

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

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retval;

}
#endif

DDS_DomainParticipant*
DDS_Subscriber_get_participant(DDS_Subscriber *self)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(subscriber == NULL,
               return NULL,
               OSAPI_Log_entry_add_pointer("subscriber",subscriber,RTI_TRUE);)

    return subscriber->participant;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Subscriber_delete_contained_entities(DDS_Subscriber * self)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl*)self;
    struct DDS_DataReaderImpl *datareader = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T handle = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION(subscriber == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->qos.management.is_hidden)
    {
        return DDS_RETCODE_OK;
    }

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
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
            retcode = DDS_Subscriber_delete_datareader(subscriber,datareader);
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

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Subscriber_set_listener(DDS_Subscriber * self,
                            const struct DDS_SubscriberListener * l,
                            DDS_StatusMask mask)
{
    struct DDS_SubscriberListener nil_listener =
                    DDS_SubscriberListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((l != NULL) && !DDS_SubscriberListener_is_consistent(l,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_LISTENER,mask)
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (l == NULL)
    {
        self->listener = nil_listener;
    }
    else
    {
        self->listener = *l;
    }

    self->mask = mask;

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

/*******************************************************************************
 *                             OPTIONAL APIs
 ******************************************************************************/
#ifndef RTI_CERT
struct DDS_SubscriberListener
DDS_Subscriber_get_listener(DDS_Subscriber *self)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl*)self;
    struct DDS_SubscriberListener retval = DDS_SubscriberListener_INITIALIZER;
    struct DDS_SubscriberListener nil_retval = DDS_SubscriberListener_INITIALIZER;

    OSAPI_PRECONDITION(subscriber == NULL,
                            return retval,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)


    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    retval = subscriber->listener;

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    return retval;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Subscriber_get_default_datareader_qos(DDS_Subscriber * self,
                                          struct DDS_DataReaderQos * qos)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION((subscriber == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)


    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataReaderQos_copy(qos, &self->default_qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAREADER_QOS)
        retcode = DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
/* Not in CERT */
DDS_ReturnCode_t
DDS_Subscriber_set_default_datareader_qos(DDS_Subscriber *self,
                                          const struct DDS_DataReaderQos * qos)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;

    OSAPI_PRECONDITION((subscriber == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_DataReaderQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAREADER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataReaderQos_copy(&self->default_qos, qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAREADER_QOS)
        retval = DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        retval = DDS_RETCODE_ERROR;
    }

    return retval;
}
#endif

#if INCLUDE_API_QOS
/* Not in CERT */
DDS_ReturnCode_t
DDS_Subscriber_set_qos(DDS_Subscriber *self,
                       const struct DDS_SubscriberQos *qos)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl*)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((subscriber == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_SubscriberQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_QOS_SET_ON_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    retcode = DDS_SubscriberQos_copy(&self->qos, qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
    }
#endif

done:

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
/* Not in CERT */
DDS_ReturnCode_t
DDS_Subscriber_get_qos(DDS_Subscriber *self, struct DDS_SubscriberQos *qos)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl*)self;

    OSAPI_PRECONDITION((subscriber == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_SubscriberQos_copy(qos, &self->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        return DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

DDS_DataReader*
DDS_Subscriber_lookup_datareader(DDS_Subscriber *self, const char *topic_name)
{
    struct DDS_SubscriberImpl *subscriber = (struct DDS_SubscriberImpl *)self;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T handle;
    struct DDS_DataReaderImpl *datareader = NULL;

    OSAPI_PRECONDITION_ALWAYS((subscriber == NULL) || (topic_name == NULL),
                           return NULL,
               OSAPI_Log_entry_add_pointer("subscriber",subscriber,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("topic_name",topic_name,RTI_TRUE);)

    if (DB_Database_lock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_all_default(subscriber->config->local_reader_table,
                                           &handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              subscriber->config->local_reader_table,dbrc)
        goto done;
    }

    dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datareader);
    while (dbrc == DB_RETCODE_OK)
    {
        if (!DDS_String_ncmp(DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(datareader->topic)), topic_name,
                RTPS_PATHNAME_LEN_MAX) && (datareader->subscriber == subscriber))
        {
            break;
        }
        datareader = NULL;
        dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datareader);
    }
    DB_Cursor_finish(subscriber->config->local_reader_table,handle);

#if OSAPI_ENABLE_LOG
    if (dbrc == DB_RETCODE_INVALIDATED_CURSOR)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif

done:

    if (DB_Database_unlock(subscriber->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return datareader;
}

/*ci @} */
