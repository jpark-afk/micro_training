/*
 * FILE: DataWriter.c - DDS DataWriter implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 20jul2015,tk  MICRO-1426/PR#15358 Use Deadline_get_sample_freq() for deadline
 *                                   check
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to get_key_kind
 * 08jun2015,tk  MICRO-1283/PR#14924 Release DB lock in case of error in
 *                                   assert_liveliness
 *               MICRO-1284/PR#14925 Release DB lock in case of error in
 *                                   register_instance_w_timestamp
 * 17feb2015,tk  MICRO-1073/PR#13972 Added comment to packet initialization
 *                                   in assert_liveliness
 *               MICRO-1074/PR#13973 Removed redundant code in write_w_params
 *               MICRO-1072/PR#13971 Adding missing precondition checks
 * 26jan2015,tk  MICRO-1028/PR#13473 Removed magic number 0xc0
 * 20sep2014,as  Moved implementation of support functions for Status types to
 *               DataWriterStatus.c
 * 16sep2014,tk  MICRO-875 Added checks for writer enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 07may2014,as  MICRO-784 Expose get_X_status API in C++ (implementation of
 *               support functions of status types)
 * 05may2014,as  MICRO-270 Always enable precondition
 *               checks for public API operations
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 13mar2013,eh  Fix MICRO-358 (assert liveliness)
 * 18may2012,tk  Updated
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief DDS DataWriter implementation
 *
 * \details
 * This file implements the public DDS datawriter API. Functionality to support
 * the public APIs is implemented in the supporting DataWriterNNN files.
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
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "QosPolicy.h"
#include "Entity.h"
#include"TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataWriterQos.h"
#include "DataWriterEvent.h"
#include "DataWriterInterface.h"
#include "DataWriterImpl.h"
#include "DomainParticipant.h"
#include "RemoteSubscription.h"
#include "UserDataQosPolicy.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataWriter_advance_sn(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    REDA_SequenceNumber_plusplus(&(datawriter->last_sn));

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriter_get_loan(DDS_DataWriter *self, void **sample)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl*)self;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS(self == NULL || sample == NULL,
                              return DDS_RETCODE_PRECONDITION_NOT_MET,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("data",sample,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(DDS_DataWriter_as_entity(datawriter)))
    {
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (!DDS_TypePlugin_is_managed_samples(datawriter->type_plugin))
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TypePlugin_has_create_sample(datawriter->type_plugin))
    {
        if (!DDS_TypePlugin_create_sample(datawriter->type_plugin, sample))
        {
#if OSAPI_ENABLE_LOG
            RTI_INT32 last_error_code = OSAPI_Log_get_last_error_code();
            if (last_error_code == DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES_EC)
            {
                rc = DDS_RETCODE_OUT_OF_RESOURCES;
            }
            else
            {
                rc = DDS_RETCODE_ERROR;
            }
#endif /*OSAPI_ENABLE_LOG*/
            goto done;
        }

        /* function will return DDS_RETCODE_OK if it gets here */
    }
    else
    {
        /* create_sample is only called on the DataWriter and only when getting a loan
         * Thus, if the type plugin does not have that installed, the type plugin
         * does not support loaned samples
         */
        rc = DDS_RETCODE_UNSUPPORTED;
        goto done;
    }

    rc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return rc;
}

DDS_ReturnCode_t
DDS_DataWriter_discard_loan(DDS_DataWriter *self, void *data)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl*)self;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS(self == NULL || data == NULL,
                              return DDS_RETCODE_PRECONDITION_NOT_MET,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("data",data,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(DDS_DataWriter_as_entity(datawriter)))
    {
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (!DDS_TypePlugin_is_managed_samples(datawriter->type_plugin))
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TypePlugin_has_delete_sample(datawriter->type_plugin))
    {
        if (!DDS_TypePlugin_delete_sample(datawriter->type_plugin,data))
        {
#if OSAPI_ENABLE_LOG
            RTI_INT32 last_error_code = OSAPI_Log_get_last_error_code();
            if (last_error_code == DDSC_LOG_MEMORY_MANAGER_NOT_OWNER_EC)
            {
                rc = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            else
            {
                rc = DDS_RETCODE_ERROR;
            }
#endif /*OSAPI_ENABLE_LOG*/
            goto done;
        }
    }
    else
    {
        rc = DDS_RETCODE_UNSUPPORTED;
        goto done;
    }

    rc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return rc;
}

DDS_ReturnCode_t
DDS_DataWriter_assert_liveliness(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    struct NETIO_Address destination = NETIO_Address_INITIALIZER;
    NETIO_Packet_T packet;
    struct NETIO_PacketInfo *pkt_info = NULL;
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;
    struct DDSHST_WriterEvent event;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (datawriter->liveliness.kind != DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        return DDS_DomainParticipant_assert_liveliness(
            DDS_Publisher_get_participant(datawriter->publisher));
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    if (!OSAPI_System_get_time(&system_timestamp))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    event.kind = DDSHST_WRITEREVENT_KIND_LIVELINESS_ASSERTED;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_DataWriter_update_liveliness(self))
    {
        goto done;
    }

    /* The history might need to update the cache based on a liveliness assert */
    DDSHST_Writer_post_event(datawriter->wh,&event,&system_timestamp);

    /* A liveliness message does not contain any data, hence the NULL buffer
     * as well as 0 length header and trail space. The downstream interface
     * must use its own buffer to create the liveliness message.
     */
    if (!NETIO_Packet_initialize(&packet,NULL,0,0,NULL))
    {
        goto done;
    }

    pkt_info = NETIO_Packet_get_info(&packet);
    pkt_info->sn = self->last_sn;
    pkt_info->protocol_id = NETIO_PROTOCOL_INTRA;
    pkt_info->rtps_flags = NETIO_RTPS_FLAGS_LIVELINESS;

    /* Destination is unknown address, so liveliness message is broadcast
     * to all matched readers
     */
    if (!NETIO_Interface_send(datawriter->dw_intf,
                              datawriter->dw_intf,&destination,&packet))
    {
        goto done;
    }

    rc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return rc;
}

DDS_ReturnCode_t
DDS_DataWriter_enable(DDS_Entity *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    struct DDS_Duration_t deadline_sample_hz = DDS_DURATION_ZERO;

    OSAPI_PRECONDITION(self == NULL,
                            return DDS_RETCODE_PRECONDITION_NOT_MET,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        goto done;
    }

    if (!DDS_Entity_is_enabled(DDS_Publisher_as_entity(datawriter->publisher)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_ENTITY)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_Entity_is_enabled(DDS_Topic_as_entity(datawriter->topic)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    OSAPI_TRACE_DDS("enable datawriter",RTI_TRUE)

    if (!NETIO_Interface_set_state(datawriter->dw_intf,
                                   NETIO_INTERFACESTATE_ENABLED))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!NETIO_Interface_set_state(datawriter->rtps_intf,
                                   NETIO_INTERFACESTATE_ENABLED))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    storage.field[0] = (void *)datawriter;

#ifdef ENABLE_QOS_DEADLINE
    if (!DDS_Duration_is_infinite(&datawriter->deadline.period))
    {
        DDS_DeadlineQosPolicy_get_sample_freq(&datawriter->deadline,
                                              &deadline_sample_hz);

        if (!OSAPI_Timer_create_timeout(datawriter->config->timer,
                        &datawriter->deadline_event,
                        deadline_sample_hz.sec,
                        (RTI_INT32)deadline_sample_hz.nanosec,
                        OSAPI_TIMER_PERIODIC,
                        DDS_DataWriterEvent_on_deadline_expired,
                        &storage))
        {
            DDSC_LOG_TIMER_CREATE_TIMEOUT(OSAPI_LOGKIND_ERROR)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }
#endif

    /* No need to check liveliness lost if :
     * 1) lease duration is infinite.
     * 2) liveliness kind is automatic. The participant will correctly assert
     *    liveliness when necessary.
     */
    datawriter->writer_state = WRITERSTATE_ALIVE;
    if ((datawriter->liveliness.kind != DDS_AUTOMATIC_LIVELINESS_QOS) &&
        !DDS_Duration_is_infinite(&datawriter->liveliness.lease_duration))
    {
        if (!OSAPI_Timer_create_timeout(datawriter->config->timer,
                &datawriter->liveliness_event,
                datawriter->liveliness.lease_duration.sec,
                (RTI_INT32)datawriter->liveliness.lease_duration.nanosec,
                OSAPI_TIMER_PERIODIC,
                DDS_DataWriterEvent_on_liveliness,
                &storage))
        {
            DDSC_LOG_TIMER_CREATE_TIMEOUT(OSAPI_LOGKIND_ERROR)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }

    datawriter->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

    /* NOTE: If we match built-in entities as normal entities are matched,
     * then if the ignore API is implemented it would work the same for
     * internal and external entities
     */
#if !ENABLE_DISCOVERY_MATCH_BUILTIN
    if (!DDS_ObjectId_is_builtin(datawriter->as_entity.entity_id) &&
          datawriter->config->on_after_enabled)
    {
        if (!datawriter->config->on_after_enabled((DDS_DataWriter*)datawriter,NULL))
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
#else
    if (datawriter->config->on_after_enabled)
    {
        if (!datawriter->config->on_after_enabled((DDS_DataWriter*)datawriter,NULL))
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
#endif

done:

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_Topic*
DDS_DataWriter_get_topic(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return datawriter->topic;
}

DDS_Publisher*
DDS_DataWriter_get_publisher(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return datawriter->publisher;
}

DDS_ReturnCode_t
DDS_DataWriter_set_listener(DDS_DataWriter *self,
                            const struct DDS_DataWriterListener *l,
                            DDS_StatusMask mask)
{
    struct DDS_DataWriterListener nil_listener =
                                        DDS_DataWriterListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((l != NULL) && !DDS_DataWriterListener_is_consistent(l,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_DATAWRITER_LISTENER,mask)
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

#ifndef RTI_CERT
struct DDS_DataWriterListener
DDS_DataWriter_get_listener(DDS_DataWriter *self)
{
    struct DDS_DataWriterListener retval = DDS_DataWriterListener_INITIALIZER;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    struct DDS_DataWriterListener nil_retval = DDS_DataWriterListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                            return retval,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    retval = self->listener;

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    return retval;
}
#endif

DDS_ReturnCode_t
DDS_DataWriter_write(DDS_DataWriter *self,
                     const void *instance_data,
                     const DDS_InstanceHandle_t *handle)
{
    struct DDS_WriteParams_t params = DDS_WRITEPARAMS_DEFAULT;
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;

    OSAPI_PRECONDITION_ALWAYS(
       (self == NULL) || (instance_data == NULL) || (handle == NULL),
       return DDS_RETCODE_BAD_PARAMETER,
                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("instance",instance_data,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("handle",handle,RTI_TRUE);)

    params.handle = *handle;

    if (!OSAPI_System_get_time(&system_timestamp))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    params.source_timestamp.sec = system_timestamp.sec;
    params.source_timestamp.nanosec = system_timestamp.nanosec;

    return DDS_DataWriter_write_w_params(self, instance_data, &params);
}

DDS_ReturnCode_t
DDS_DataWriter_write_w_timestamp(DDS_DataWriter *self,
                                 const void *instance_data,
                                 const DDS_InstanceHandle_t *handle,
                                 const struct DDS_Time_t *source_timestamp)
{
    struct DDS_WriteParams_t params = DDS_WRITEPARAMS_DEFAULT;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (instance_data == NULL) ||
            (handle == NULL) || (source_timestamp == NULL) ||
            (source_timestamp->nanosec >= OSAPI_TIME_NSEC_PER_SEC),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("instance",instance_data,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("source_timestamp",source_timestamp,RTI_TRUE);)

    params.handle = *handle;
    params.source_timestamp = *source_timestamp;

    return DDS_DataWriter_write_w_params(self, instance_data, &params);
}

DDS_ReturnCode_t
DDS_DataWriter_write_w_params(DDS_DataWriter *self,
                              const void *instance_data,
                              struct DDS_WriteParams_t *params)
{
    struct DDS_DataWriterImpl *writer = (struct DDS_DataWriterImpl *)self;
    struct NDDS_DataWriterSampleInfo sample_info;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION(
        (self == NULL) || (instance_data == NULL) || (params == NULL),
        return DDS_RETCODE_BAD_PARAMETER,
        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("instance_data",instance_data,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("params",params,RTI_TRUE);)

    sample_info.timestamp.sec = params->source_timestamp.sec;
    sample_info.timestamp.nanosec = params->source_timestamp.nanosec;

    if ((sample_info.timestamp.sec > UINT_MAX) ||
        (sample_info.timestamp.sec < 0))
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    sample_info.status_info = RTPS_NO_STATUS_INFO;

    if (DB_Database_lock(writer->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DataWriter_write_untyped(writer,
                                instance_data, &params->handle, &sample_info);

    if (DB_Database_unlock(writer->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /* a write operation in a data writer with liveliness manual_by_participant
     * asserts liveliness in all data writers with this same liveliness
     */
    if (writer->liveliness.kind == DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        retcode = DDS_DomainParticipant_assert_liveliness(
                      DDS_Publisher_get_participant(writer->publisher));
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    return retcode;
}

#ifdef INCLUDE_API_INSTANCE
DDS_InstanceHandle_t
DDS_DataWriter_register_instance_w_timestamp(DDS_DataWriter *self,
                                    const void *instance_data,
                                    const struct DDS_Time_t *timestamp)
{
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL_NATIVE;
    DDS_InstanceHandle_t nil_handle = DDS_HANDLE_NIL_NATIVE;
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (instance_data == NULL) ||
                              (timestamp == NULL) ||
                              (timestamp->nanosec >= OSAPI_TIME_NSEC_PER_SEC),
                           return nil_handle,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("instance_data",instance_data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("timestamp",timestamp,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return handle;
    }

    if (DDS_TypePlugin_get_key_kind(datawriter->type_plugin) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        return handle;
    }

    if (DDS_TypePlugin_is_managed_samples(datawriter->type_plugin))
    {
        if (!DDS_TypeMemoryPlugin_is_owner(datawriter->type_plugin->allocator_plugin,
                                           instance_data))
        {
            return nil_handle;
        }
    }

    system_timestamp.sec = timestamp->sec;
    system_timestamp.nanosec = timestamp->nanosec;

    if ((system_timestamp.sec > UINT_MAX) ||
        (system_timestamp.sec < 0))
    {
        return nil_handle;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return nil_handle;
    }

    if (DDS_DataWriter_register_key(datawriter, &handle, instance_data,
                                    &system_timestamp) != DDS_RETCODE_OK)
    {
        handle = nil_handle;
    }

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return nil_handle;
    }

    return handle;
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t
DDS_DataWriter_unregister_instance_w_timestamp(DDS_DataWriter *self,
                                   const void *instance_data,
                                   const DDS_InstanceHandle_t *handle,
                                   const struct DDS_Time_t *source_timestamp)
{
    struct NDDS_DataWriterSampleInfo sample_info;
    DDS_ReturnCode_t retcode;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (handle == NULL) ||
                              (DDS_InstanceHandle_is_nil(handle) &&
                              (instance_data == NULL)) ||
                              (source_timestamp == NULL) ||
                              (source_timestamp->nanosec >= OSAPI_TIME_NSEC_PER_SEC),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("instance_data",instance_data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("source_timestamp",
                                   source_timestamp,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DDS_TypePlugin_get_key_kind(datawriter->type_plugin) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        return DDS_RETCODE_OK;
    }

    sample_info.timestamp.sec = source_timestamp->sec;
    sample_info.timestamp.nanosec = source_timestamp->nanosec;
    if ((sample_info.timestamp.sec > UINT_MAX) ||
        (sample_info.timestamp.sec < 0))
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    sample_info.status_info = RTPS_UNREGISTER_STATUS_INFO | RTPS_DISPOSE_STATUS_INFO;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DataWriter_write_untyped(datawriter,
                                           instance_data,handle,&sample_info);

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t
DDS_DataWriter_dispose(DDS_DataWriter *self,
                       const void *instance_data,
                       const DDS_InstanceHandle_t *handle)
{
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;
    struct DDS_Time_t dds_timestamp;

    if (!OSAPI_System_get_time(&system_timestamp))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    dds_timestamp.sec = system_timestamp.sec;
    dds_timestamp.nanosec = system_timestamp.nanosec;

    return DDS_DataWriter_dispose_w_timestamp(
                                self, instance_data, handle, &dds_timestamp);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t
DDS_DataWriter_dispose_w_timestamp(DDS_DataWriter *self,
                                   const void *instance_data,
                                   const DDS_InstanceHandle_t *handle,
                                   const struct DDS_Time_t *source_timestamp)
{
    struct NDDS_DataWriterSampleInfo sample_info;
    DDS_ReturnCode_t retcode;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (handle == NULL) ||
                              (DDS_InstanceHandle_is_nil(handle) &&
                              (instance_data == NULL)) ||
                              (source_timestamp == NULL) ||
                              (source_timestamp->nanosec >= OSAPI_TIME_NSEC_PER_SEC),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("instance_data",instance_data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("source_timestamp",
                               source_timestamp,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DDS_TypePlugin_get_key_kind(datawriter->type_plugin) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        return DDS_RETCODE_OK;
    }

    sample_info.timestamp.sec = source_timestamp->sec;
    sample_info.timestamp.nanosec = source_timestamp->nanosec;
    if ((sample_info.timestamp.sec > UINT_MAX) ||
        (sample_info.timestamp.sec < 0))
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    sample_info.status_info = RTPS_DISPOSE_STATUS_INFO;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DataWriter_write_untyped(datawriter,
                                           instance_data,
                                           handle,
                                           &sample_info);

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

DDS_ReturnCode_t
DDS_DataWriter_assert_instance(DDS_DataWriter *datawriter,
                               DDS_InstanceHandle_t *handle)
{
    DDSHST_ReturnCode_T whrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    whrc = DDSHST_Writer_register_key(datawriter->wh, handle,NULL);

    if (whrc != DDSHST_RETCODE_SUCCESS)
    {
        ddsrc = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}

DDS_ReturnCode_t
DDS_DataWriter_remove_instance(DDS_DataWriter *datawriter,
                               DDS_InstanceHandle_t *handle)
{
    DDSHST_ReturnCode_T whrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    whrc = DDSHST_Writer_unregister_key(datawriter->wh, handle);

    if (whrc != DDSHST_RETCODE_SUCCESS)
    {
        ddsrc = DDS_RETCODE_BAD_PARAMETER;
    }

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}

/*******************************************************************************
 *                             OPTIONAL APIs
 ******************************************************************************/
#if INCLUDE_API_LOOKUP
/* Not in CERT */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscriptions(DDS_DataWriter *self,
                            struct DDS_InstanceHandleSeq *subscription_handles)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;

    struct DDS_DataWriterInterface *dw_intf = NULL;
    DB_Cursor_T cursor = NULL;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DDS_InstanceHandle_t *handle;
    RTI_INT32 length = 0;

    OSAPI_PRECONDITION_ALWAYS(
            (datawriter == NULL) || (subscription_handles == NULL),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("subscription_handles",subscription_handles,RTI_TRUE);)

    dw_intf = (struct DDS_DataWriterInterface *)self->dw_intf;

    if (!DDS_Entity_is_enabled(&datawriter->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    dbrc = DB_Database_lock(datawriter->config->db);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_InstanceHandleSeq_set_length(subscription_handles, 0))
    {
        goto done;
    }

    dbrc = DB_Table_select_all_default(dw_intf->_parent._btable,
                                      &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR, dw_intf->_parent._btable, dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
        if (dbrc == DB_RETCODE_OK)
        {
            if (!DDS_InstanceHandleSeq_ensure_length(subscription_handles, length + 1,
                    self->writer_resource_limits.max_remote_readers))
            {
                goto done;
            }

            handle = DDS_InstanceHandleSeq_get_reference(subscription_handles, length);
            if (handle == NULL)
            {
                goto done;
            }

            /* Both the instance handle and reader guid are store in network order */
            OSAPI_Memory_copy(handle, &bind_entry->source, RTI_SIZEOF(handle->octet));
            handle->is_valid = DDS_BOOLEAN_TRUE;

            ++length;
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(dw_intf->_parent._btable, cursor);
    cursor = NULL;

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    result = DDS_RETCODE_OK;

done:
    if (cursor != NULL)
    {
        DB_Cursor_finish(dw_intf->_parent._btable, cursor);
    }
    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }
    return result;
}
#endif

#if INCLUDE_API_LOOKUP
/* Not in CERT */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscription_data(DDS_DataWriter * self,
                struct DDS_SubscriptionBuiltinTopicData *subscription_data,
                const DDS_InstanceHandle_t *subscription_handle)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;

    DB_Record_T bind_entry = NULL;
    struct DDS_DataWriterInterface *dw_intf = NULL;

    struct DDS_DomainParticipantImpl *participant = NULL;
    struct DDS_BuiltinTopicKey_t dr_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    struct DDS_RemoteSubscriptionImpl *remote_subscription = NULL;
    DDS_UnsignedLong object_id;
    DDS_DataReader *local_reader = NULL;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (subscription_data == NULL) || (subscription_handle == NULL),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("subscription_data",subscription_data,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("subscription_handle",subscription_handle,RTI_TRUE);)

    dw_intf = (struct DDS_DataWriterInterface *)self->dw_intf;
    if (!DDS_Entity_is_enabled(&datawriter->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    /* Verify that self is matched with subscription_handle */
    dbrc = DB_Table_select_match(dw_intf->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                &bind_entry,
                                (DB_Key_T)&subscription_handle->octet);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /* Did not find match */
        result = DDS_RETCODE_BAD_PARAMETER;
        goto done;
    }
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              dw_intf->_parent._btable,
                              dbrc)
        goto done;
    }

    /* Check if the subscription exists in the remote_subscription_table */
    participant = DDS_Publisher_get_participant(datawriter->publisher);
    DDS_BuiltinTopicKey_from_instance_handle(&dr_key, subscription_handle);
    dbrc = DB_Table_select_match(participant->remote_subscriber_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&remote_subscription,
                                (DB_Key_T)&dr_key);
    if (dbrc == DB_RETCODE_OK)
    {
        /* Copy data from remote_subscription_table */
        if (!DDS_SubscriptionBuiltinTopicData_copy(subscription_data,
                                                  &remote_subscription->data))
        {
            goto done;
        }
        result = DDS_RETCODE_OK;
        goto done;
    }
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->remote_subscriber_table,
                              dbrc)
        goto done;
    }

    /* Find the local reader */
    object_id = dr_key.value[3];
    dbrc = DB_Table_select_match(participant->local_reader_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T)&local_reader,
                                (DB_Key_T)&object_id);
    if (dbrc != DB_RETCODE_OK)
    {
        /* It is unexpected that a matched reader would not be in either
         * the remote_subscription_table or the local_reader_table.
         */
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->local_reader_table,
                              dbrc)
        goto done;
    }

    /* Copy data from local reader */
    if (!DDS_SubscriptionBuiltinTopicData_copy_from_datareader(subscription_data,
                                                               local_reader))
    {
        goto done;
    }

    result = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return result;
}
#endif


#ifdef INCLUDE_API_INSTANCE
DDS_InstanceHandle_t
DDS_DataWriter_register_instance(DDS_DataWriter *self,
                                 const void *instance_data)
{
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;
    struct DDS_Time_t dds_timestamp;

    if (!OSAPI_System_get_time(&system_timestamp))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_HANDLE_NIL;
    }

    dds_timestamp.sec = system_timestamp.sec;
    dds_timestamp.nanosec = system_timestamp.nanosec;

    return DDS_DataWriter_register_instance_w_timestamp(
                                        self, instance_data, &dds_timestamp);
}
#endif


#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t
DDS_DataWriter_unregister_instance(DDS_DataWriter *self,
                                   const void *instance_data,
                                   const DDS_InstanceHandle_t *handle)
{
    struct OSAPI_SystemTime system_timestamp = OSAPI_TIME_ZERO;
    struct DDS_Time_t dds_timestamp;

    if (!OSAPI_System_get_time(&system_timestamp))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    dds_timestamp.sec = system_timestamp.sec;
    dds_timestamp.nanosec = system_timestamp.nanosec;
    return DDS_DataWriter_unregister_instance_w_timestamp(
                                self, instance_data, handle, &dds_timestamp);
}
#endif


#if INCLUDE_API_QOS

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataWriter_set_qos(DDS_DataWriter *self,
                       const struct DDS_DataWriterQos *qos)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_DataWriterQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(datawriter)))
    {
        DDSC_LOG_QOS_SET_ON_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_DataWriter_is_immutable_qos_equal(self,qos))
    {
        DDSC_LOG_QOS_IMMUTABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        retcode = DDS_RETCODE_IMMUTABLE_POLICY;
        goto done;
    }

    retcode = DDS_DataWriter_finalize_managed(self,
                                DDS_Publisher_get_participant(self->publisher));
   if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

    retcode = DDS_DataWriter_set_qos_from(self, qos,
                    DDS_Publisher_get_participant(self->publisher));
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

done:

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */
#endif

#if INCLUDE_API_QOS
#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataWriter_get_qos(DDS_DataWriter *self, struct DDS_DataWriterQos * qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DataWriter_get_qos_from(self, qos);
#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
    }
#endif

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */
#endif


DDS_ReturnCode_t
DDS_DataWriter_get_liveliness_lost_status(
        DDS_DataWriter *self,
        struct DDS_LivelinessLostStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->liveliness_lost_status;

    DDS_LivelinessLostStatus_reset(&self->liveliness_lost_status);
    if (!DDS_EntityImpl_disable_status(
            DDS_DataWriter_as_entity(self), DDS_LIVELINESS_LOST_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;

}

DDS_ReturnCode_t
DDS_DataWriter_get_offered_incompatible_qos_status(
        DDS_DataWriter *self,
        struct DDS_OfferedIncompatibleQosStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->off_incompatible_qos_status;

    DDS_OfferedIncompatibleQosStatus_reset(&self->off_incompatible_qos_status);
    if (!DDS_EntityImpl_disable_status(
            DDS_DataWriter_as_entity(self),
            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataWriter_get_offered_deadline_missed_status(
        DDS_DataWriter *self,
        struct DDS_OfferedDeadlineMissedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->off_deadline_missed_status;

    DDS_OfferedDeadlineMissedStatus_reset(&self->off_deadline_missed_status);
    if (!DDS_EntityImpl_disable_status(
            DDS_DataWriter_as_entity(self),
            DDS_OFFERED_DEADLINE_MISSED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataWriter_get_publication_matched_status(
        DDS_DataWriter *self,
        struct DDS_PublicationMatchedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->publication_matched_status;

    DDS_PublicationMatchedStatus_reset(&self->publication_matched_status);
    if (!DDS_EntityImpl_disable_status(
            DDS_DataWriter_as_entity(self), DDS_PUBLICATION_MATCHED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataWriter_get_reliable_reader_activity_changed_status(
        DDS_DataWriter *self,
        struct DDS_ReliableReaderActivityChangedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->reliable_reader_activity_changed_status;

    DDS_ReliableReaderActivityChangedStatus_reset(
       &self->reliable_reader_activity_changed_status);
    if (!DDS_EntityImpl_disable_status(
            DDS_DataWriter_as_entity(self),
            DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}


#if DDS_XTYPES_IS_ENABLED
DDS_TypeCode*
DDS_DataWriter_get_typecode(DDS_DataWriter *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                             return NULL,
                             OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE); )

    return self->topic->type->typecode;
}
#endif

DDS_TypePlugin*
DDS_DataWriter_get_type_plugin(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                         return NULL,
                         OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE); )

    return datawriter->type_plugin;
}


/*ci @} */
