/*
 * FILE: Publisher.c - Publisher implementation
 *
 * (c) Copyright 2008-2016 Real-Time Innovations,
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
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in DDS_Publisher_create_datawriter()
 * 27jun2016,tk  MICRO-1545 Properly keep track of when a topic is in use
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 28jul2015,tk  MICRO-1470/PR#15623 Do not enable contained entities unless
 *                                   auto_enable is true (in enable())
 * 09feb2015,tk  MICRO-1003/PR#13282 Do not free resource on failure
 * 02feb2015,tk  MICRO-1037/PR#13485 Assign retcode in case enable fails
 * 21jan2015,tk  MICRO-1001/PR#13243 Return error if DB cannot be unlocked
 *               MICRO-1003/PR#13282 Increment counter after creation
 *               MICRO-1004/PR#13284 Removed comment
 *               MICRO-1005/PR#13285 Log pointer, not value
 *               MICRO-1006/PR#13286 Fixed loop condition
 * 10dec2014,tk  MICRO-978/PR#12914 Consistently return records on failure
 * 16sep2014,tk  MICRO-874 Check specified topic and publisher was created by participant
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 05may2014,as  MICRO-270 Always enable precondition
 *               checks for public API operations
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 19jul2013,as  Added support for C++
 * 23mar2013,tk  Updated logging
 * 04jun2012,tk  Major update
 * 19aug2011,yy  Fixed get_default_datawriter_qos
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief Publisher implementation
 *
 * \details
 * This file implements the public DDS publisher API. Support functions are
 * found in the PublisherNNN.c files.
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

#include "Entity.h"
#include "Topic.h"
#include "DataWriterImpl.h"
#include "DataWriterQos.h"
#include "PublisherImpl.h"
#include "PublisherQos.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_Publisher_enable(DDS_Entity *self)
{
    DB_ReturnCode_T dbrc;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*) self;
    struct DDS_DataWriterImpl *writer;
    DB_Cursor_T handle = NULL;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;

    OSAPI_PRECONDITION(publisher == NULL,
                   return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_TRUE);)

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (publisher->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        goto done;
    }

    if (!DDS_Entity_is_enabled(
              DDS_DomainParticipant_as_entity(publisher->participant)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        ddsrc = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    publisher->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

    if (!publisher->qos.entity_factory.autoenable_created_entities)
    {
        goto done;
    }

    dbrc = DB_Table_select_all_default(
                                publisher->config->local_writer_table,&handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              publisher->config->local_writer_table,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&writer);
    while (dbrc == DB_RETCODE_OK)
    {
        if (writer->publisher == publisher)
        {
            ddsrc = DDS_DataWriter_enable(&writer->as_entity);
            if (ddsrc != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
                DB_Cursor_finish(publisher->config->local_writer_table,handle);
                goto done;
            }
        }
        dbrc = DB_Cursor_get_next(handle,(DB_Record_T*)&writer);
    }
    DB_Cursor_finish(publisher->config->local_writer_table,handle);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        ddsrc = DDS_RETCODE_ERROR;
    }

done:

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}

DDS_DataWriter*
DDS_Publisher_create_datawriter(DDS_Publisher *self,
                                DDS_Topic *topic,
                                const struct DDS_DataWriterQos *qos,
                                const struct DDS_DataWriterListener *listener,
                                DDS_StatusMask mask)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    struct DDS_DataWriterImpl *datawriter = NULL;
    DDS_DataWriter *retval = NULL;
    DB_ReturnCode_T dbrc;
    DDS_UnsignedLong dw_oid;
    void *wrapper = NULL;
    DDS_ReturnCode_t ddsrc;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (topic == NULL) || (qos == NULL),
                       return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("topic",topic,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DDS_TopicDescription_get_participant(
                                      DDS_Topic_as_topicdescription(topic)) !=
        DDS_Publisher_get_participant(publisher))
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_TOPIC_ENTITY,DDSC_LOG_PUBLISHER_ENTITY)
        return NULL;
    }

    ih = DDS_Entity_get_instance_handle(DDS_Publisher_as_entity(publisher));
    DDS_InstanceHandle_to_rtps((struct RTPS_Guid *)&dw_key,&ih);

    publisher->config->dw_config->db = publisher->config->db;
    publisher->config->dw_config->registry = publisher->config->registry;
    publisher->config->dw_config->timer = publisher->config->timer;
    publisher->config->dw_config->bind_resolver = publisher->config->bind_resolver;
    publisher->config->dw_config->on_after_enabled = publisher->config->on_after_datawriter_enabled;
    publisher->config->dw_config->get_parent_handle = DDS_PublisherImpl_get_instance_handle;
    publisher->config->dw_config->default_multicast = publisher->config->default_multicast;
    publisher->config->dw_config->default_unicast = publisher->config->default_unicast;
    publisher->config->dw_config->default_meta_multicast = publisher->config->default_meta_multicast;
    publisher->config->dw_config->default_meta_unicast = publisher->config->default_meta_unicast;
    publisher->config->dw_config->enabled_transports = publisher->config->enabled_transports;
    publisher->config->dw_config->route_resolver = publisher->config->route_resolver;
    publisher->config->dw_config->participant_id = publisher->config->participant_id;
    publisher->config->dw_config->domain_id = publisher->config->domain_id;

    datawriter = NULL;

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }
    
    if (qos->management.is_hidden)
    {
        dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = qos->protocol.rtps_object_id;
    }
    else
    {
        if (qos->protocol.rtps_object_id == DDS_RTPS_AUTO_ID)
        {
            dw_oid = publisher->config->object_id_generator(
                            DDS_Publisher_get_participant(publisher));
        }
        else
        {
            dw_oid = qos->protocol.rtps_object_id;
        }

        dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                     DDS_Topic_get_object_suffix(topic,
                                                qos->type_support.plugin_data,
                                                dw_oid,RTI_TRUE);
    }

    if (dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] == 0)
    {
        dbrc = DB_Database_unlock(publisher->config->db);
        /* ignore this error as we're returning an error anyway */
        IGNORE_RETVAL(dbrc);
        return NULL;
    }

    /* First check if it is ok to create a datawriter upstream. For example,
     * a discovery plugin may be out of resources or a security plugin
     * may deny the operation.
     */
    ddsrc = publisher->config->on_before_datawriter_created(
                                            publisher,&dw_key,DDS_BOOLEAN_TRUE);
    if (ddsrc != DDS_RETCODE_OK)
    {
        dbrc = DB_Database_unlock(publisher->config->db);
        /* ignore this error as we're returning an error anyway */
        IGNORE_RETVAL(dbrc);
        return NULL;
    }

    dbrc = DB_Table_create_record(publisher->config->local_writer_table,
                                  (DB_Record_T*)&datawriter);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_RECORD,dbrc)
        goto done;
    }

    if (!DDS_DataWriterImpl_initialize(datawriter,publisher,topic,qos,
                   listener,mask,dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID],
                   publisher->config->dw_config))
    {
#ifndef RTI_CERT
        (void)DB_Table_delete_record(publisher->config->local_writer_table,
                                     (DB_Record_T)datawriter);
        datawriter = NULL;
#endif
        goto done;
    }

    /* Wrap C datawriter if type plugin contains wrapping function */
    if (datawriter->type_plugin->create_typed_datawriter != NULL)
    {
        wrapper = datawriter->type_plugin->create_typed_datawriter(datawriter);
        if (wrapper != NULL)
        {
            DDS_Entity_set_wrapper(DDS_DataWriter_as_entity(datawriter),
                                   wrapper);
        }
        else
        {
#ifndef RTI_CERT
            (void)DDS_DataWriterImpl_finalize(datawriter);
            (void)DB_Table_delete_record(publisher->config->local_writer_table,
                                         (DB_Record_T)datawriter);
            datawriter = NULL;
#endif
            DDSC_LOG_DW_CREATE_TYPED_WRITER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    dbrc = DB_Table_insert_record(publisher->config->local_writer_table,
                                  (DB_Record_T)datawriter);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_RECORD,dbrc)
        goto done;
    }

    ++publisher->dw_count;
    DDS_TopicImpl_attach(topic);

    if ((self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) &&
         self->qos.entity_factory.autoenable_created_entities)
    {
        if (DDS_DataWriter_enable(
                DDS_DataWriter_as_entity(datawriter)) != DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
            goto done;
        }
    }

    retval = (DDS_DataWriter*)datawriter;

done:

#ifndef RTI_CERT
    if ((retval == NULL) && (datawriter != NULL))
    {
        /* This means that the datawriter could not be enabled, in all
         * other cases datawriter is NULL.
         */
        if (datawriter->type_plugin != NULL &&
            datawriter->type_plugin->create_typed_datawriter != NULL)
        {
            wrapper = DDS_Entity_get_wrapper(
                                         DDS_DataWriter_as_entity(datawriter));
            if (wrapper != NULL &&
                (datawriter->type_plugin->delete_typed_datawriter != NULL))
            {
                datawriter->type_plugin->delete_typed_datawriter(wrapper);
                /* the function has a lock on the database so no need to
                 * set user data to NULL because the datawriter will be
                 * deleted before the lock is released.
                 */
            }
        }

        (void)DDS_Publisher_delete_datawriter(publisher,datawriter);
        datawriter = NULL;
    }
    else if (datawriter == NULL)
    {
        ddsrc = publisher->config->on_before_datawriter_created(
                                        publisher,&dw_key,DDS_BOOLEAN_FALSE);
        IGNORE_RETVAL(ddsrc);
    }
#endif

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
#ifndef RTI_CERT
        if (datawriter != NULL)
        {
            (void)DDS_Publisher_delete_datawriter(publisher,datawriter);
        }
#endif
        return NULL;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Publisher_delete_datawriter(DDS_Publisher *self,
                                DDS_DataWriter *datawriter)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;
    void *wrapper = NULL;
    DDS_Boolean dw_ok = DDS_BOOLEAN_TRUE;
    DDS_Topic *topic = NULL;

    OSAPI_PRECONDITION((self == NULL) || (datawriter == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_TRUE);)

    if (DDS_DataWriter_get_publisher(datawriter) != self)
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_PUBLISHER_ENTITY,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (publisher->config->on_before_datawriter_deleted)
    {
        publisher->config->on_before_datawriter_deleted(datawriter);
    }

    /* Unwrap C datawriter if type plugin contains unwrapping function */
    if ((datawriter->type_plugin != NULL)
        && datawriter->type_plugin->delete_typed_datawriter != NULL)
    {
        wrapper = DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(datawriter));
        if (wrapper != NULL)
        {
            datawriter->type_plugin->delete_typed_datawriter(wrapper);
            DDS_Entity_set_wrapper(
                    DDS_DataWriter_as_entity(datawriter), NULL);
        }
    }

    /* Topic cannot be NULL if a writer was successfully created */
    topic = DDS_DataWriter_get_topic(datawriter);

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

    if (!dw_ok)
    {
        retval = DDS_RETCODE_ERROR;
    }

done:

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }


    return retval;
}
#endif

DDS_DomainParticipant*
DDS_Publisher_get_participant(DDS_Publisher *self)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(publisher == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return publisher->participant;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities(DDS_Publisher *self)
{
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterImpl *datawriter = NULL;
    DB_Cursor_T handle = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    OSAPI_PRECONDITION(publisher == NULL,
                return DDS_RETCODE_BAD_PARAMETER,
                OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_TRUE);)

    if (self->qos.management.is_hidden)
    {
        return DDS_RETCODE_OK;
    }

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
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
            retcode = DDS_Publisher_delete_datawriter(publisher,datawriter);
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

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Publisher_set_listener(DDS_Publisher * self,
                           const struct DDS_PublisherListener *l,
                           DDS_StatusMask mask)
{
    struct DDS_PublisherListener nil_listener =
                    DDS_PublisherListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((l != NULL) && !DDS_PublisherListener_is_consistent(l,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_LISTENER,mask)
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
#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Publisher_get_default_datawriter_qos(DDS_Publisher *self,
                                         struct DDS_DataWriterQos * qos)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION((publisher == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataWriterQos_copy(qos, &self->default_qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAWRITER_QOS)
        retcode = DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos(DDS_Publisher * self,
                                         const struct DDS_DataWriterQos * qos)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION((publisher == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_DataWriterQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAWRITER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataWriterQos_copy(&self->default_qos, qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAWRITER_QOS)
        retcode = DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Publisher_get_qos(DDS_Publisher *self, struct DDS_PublisherQos *qos)
{
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    OSAPI_PRECONDITION((publisher == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PublisherQos_copy(qos, &self->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
        return DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Publisher_set_qos(DDS_Publisher *self, const struct DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*) self;

    OSAPI_PRECONDITION((publisher == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_PublisherQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_QOS_SET_ON_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    retcode = DDS_PublisherQos_copy(&self->qos, qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS)
    }
#endif

done:

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_OK;
    }

    return retcode;
}
#endif /* INCLUDE_API_QOS */

DDS_DataWriter*
DDS_Publisher_lookup_datawriter(DDS_Publisher *self, const char *topic_name)
{
    DB_ReturnCode_T dbrc;
    DB_Cursor_T handle = NULL;
    struct DDS_DataWriterImpl *datawriter = NULL;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    OSAPI_PRECONDITION_ALWAYS((publisher == NULL) || (topic_name == NULL),
               return NULL,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("topic_name",topic_name,RTI_TRUE);)

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_all_default(publisher->config->local_writer_table,
                                       &handle);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              publisher->config->local_writer_table,dbrc)
        goto done;
    }

    dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datawriter);
    while (dbrc == DB_RETCODE_OK)
    {
        if (!DDS_String_ncmp(DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(datawriter->topic)), topic_name,
                RTPS_PATHNAME_LEN_MAX) && (datawriter->publisher == publisher))
        {
            break;
        }
        datawriter = NULL;
        dbrc = DB_Cursor_get_next(handle, (DB_Record_T*)&datawriter);
    }
    DB_Cursor_finish(publisher->config->local_writer_table,handle);

#if OSAPI_ENABLE_LOG
    if (dbrc == DB_RETCODE_INVALIDATED_CURSOR)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif

done:

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return datawriter;
}

#ifndef RTI_CERT
struct DDS_PublisherListener
DDS_Publisher_get_listener(DDS_Publisher * self)
{
    struct DDS_PublisherListener retval = DDS_PublisherListener_INITIALIZER;
    struct DDS_PublisherListener nil_retval = DDS_PublisherListener_INITIALIZER;
    struct DDS_PublisherImpl *publisher = (struct DDS_PublisherImpl*)self;

    OSAPI_PRECONDITION(publisher == NULL,
                   return retval,
                   OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_TRUE);)

    if (DB_Database_lock(publisher->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    retval = publisher->listener;

    if (DB_Database_unlock(publisher->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    return retval;
}
#endif

/*ci @} */
