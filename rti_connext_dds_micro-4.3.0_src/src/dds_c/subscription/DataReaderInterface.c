/*
 * FILE: DataReaderInterface.c - NETIO interface for DDS DataReader
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 17dec2015,eh MICRO-1511: process DATA with serialized key (K-flag)
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 29jun2015,tk MICRO-1322/PR#15018 Removed redundant code
 * 15may2015,tk MICRO-1186/PR#14705 Assign publisher_handle before committing
 *                                  existing samples
 * 15may2015,tk MICRO-1183/PR#14702 Use max_binds instead of max_routes (no
 *                                  functional change)
 * 05may2015,tk MICRO-1184/PR#14704 Removed duplicate instance_counter update
 * 05may2015,tk MICRO-1188/PR#14707 Set interface to ENABLED when enabled
 * 10dec2014,tk MICRO-1134 Redid finding key for anon participant, removing
 *                         dependency on RTPS implementation
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 30sep2013,tk  MICRO-704: Notify reader queue when a remote writer is deleted
 * 07aug2013,tk  MICRO-667: Check return value for instance to keyhash
 * 01aug2013,tk  MICRO-677: on_liveliness_callback on remote deletion
 * 13jun2013,tk  MICRO-365: Do not recreate key for samples received over intra
 * 03jun2013,eh  MICRO-657: update last_sample_recvd_time on receive
 * 06jun2013,kaj MICRO-183: CDR stream alignment reset moved to (de)ser_header
 * 14may2013,eh  MICRO-639: actions from CR-138, for MICRO-394
 * 05mar2013,eh  MICRO-350: unlock DB when rcv liveliness HB
 * 06feb2013,eh  MICRO-216: receive liveliness msg
 * 06feb2013,eh  MICRO-210: fix inlineQoS deserialization's endianness
 * 14may2012,tk  Written
 */
/*ce
 * \file
 * \brief NETIO interface for DDS DataReader
 *
 * \details
 * The datareader sends/receives data by implementing a NETIO interface and
 * implementing the required methods. The datareader interface is the highest
 * layer in the NETIO stack for the DDS datareader and thus does not implement
 * any methods which may be called from an upstream interface. The datareader
 * interface is tightly coupled to the datareader discovery functionality and
 * thus only implements the required methods.
 *
 */
/*ci \addtogroup DDSSubscriptionModule
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

#include "Entity.h"
#include "Conditions.h"
#include "DataReaderQos.h"
#include "DataReaderImpl.h"
#include "DataReaderEvent.h"
#include "DataReaderInterface.h"

#if DDS_FILTERING_ENABLED
#include "DataReaderFilter.h"
#endif /* DDS_FILTERING_ENABLED */

/*ci
 * \brief Datareader NETIO interface factory implementation
 */
struct DDS_DataReaderInterfaceFactory
{
    /*ci
     * \brief base-class
     */
    struct RT_ComponentFactory _parent;
};


/*ci
 * \brief The DataReader NETIO interface factory exists as a singleton
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI DDS_DataReaderInterface_fv_Intf;

/*** SOURCE_BEGIN ***/


/*ci
 * \brief Checks if liveliness is expired for the bind entry
 *
 * \details
 *
 * \param[in] bind_entry Remote datawriter bind entry
 *
 * \return RTI_TRUE is the liveliness is expired. Otherwise RTI_FALSE.
 */
RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_check_liveliness_expired(
                                        struct DataReaderBindEntry *bind_entry)
{
    OSAPI_SystemTime current_time = OSAPI_TIME_ZERO;
    OSAPI_SystemTime diff = OSAPI_TIME_ZERO;

    /* Note that the writer lease duration cannot be infinite.
     * why? a dw with infinite lease duration would only match
     * dr with infinite lease, and in case the dr lease duration
     * is infinite the timeout is not running
     */
    RTI_BOOL get_time_result = OSAPI_System_get_time(&current_time);
    RTI_BOOL liveliness_expired = RTI_TRUE;

    if (!get_time_result)
    {
        /* get_time should never fail, but in case it does there is
         * not much that we can do. to be conservative we will
         * signal liveliness lost.
         */
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
    }
    else
    {
        OSAPI_SystemTime_subtract(&diff, &current_time,
                               &bind_entry->activity_time);


        liveliness_expired = (OSAPI_SystemTime_compare(
                                    &diff,
                                    &bind_entry->lease_duration) > 0);
    }

    /* If the time difference is greater than the writer lease
     * duration signal liveliness lost. Also if we could not get
     * the current time we are conservative and signal liveliness
     * lost as well
     */
    if (liveliness_expired || !get_time_result)
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci
 * \brief Timeout handler for expired liveliness timer
 *
 * \details
 *
 * The datareader interface uses a single timer to monitor activity on
 * each peer interface. On each timeout all interfaces are checked for
 * activity and if a change in activity is detected an internal event
 * is generated to notify the listener (if installed).
 *
 * \param[in] storage Timeout data passed in when the timeout was created
 *
 * \return  Always OSAPI_TIMEOUT_OP_AUTOMATIC
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
DDS_DataReaderInterface_on_liveliness_timeout(
                                        struct OSAPI_TimeoutUserData *storage)
{
    struct DDS_DataReaderInterface *self =
                (struct DDS_DataReaderInterface *)storage->field[0];
    DB_Cursor_T bind_cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct DataReaderBindEntry *bind_entry;

    if (DB_Database_lock(self->db) != DB_RETCODE_OK)
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    dbrc = DB_Table_select_all(
                    self->_parent._btable,DB_TABLE_DEFAULT_INDEX,&bind_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,self->_parent._btable,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(bind_cursor,(DB_Record_T*)&bind_entry);
        if (dbrc == DB_RETCODE_OK)
        {
            /* Check for liveliness lost only if writer is alive. If writer
             * is already not alive we do not notify liveliness lost again
             */
            if (bind_entry->writer_state == REMOTE_WRITERSTATE_ALIVE)
            {
                if (DDS_DataReaderInterface_check_liveliness_expired(
                                        bind_entry))
                {
                    DDS_DataReaderEvent_on_liveliness_lost(
                        self->datareader,&bind_entry->key.peer);
                    bind_entry->writer_state = REMOTE_WRITERSTATE_NOT_ALIVE;
                }
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(self->_parent._btable,bind_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif

done:
    dbrc = DB_Database_unlock(self->db);

    /* The return value is ignored because the timeout routine, not called
     * by a user, must return OSAPI_TIMEOUT_OP_AUTOMATIC regardless of the
     * outcome.
     */
    IGNORE_RETVAL(dbrc);

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}

/*ci
 * \brief Compare entries in the database of DataReaderBindEntry entries. The
 *        function is compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DataReaderBindEntryKey already in the database
 * \param[in] op2   Either a DataReaderBindEntryKey being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_PRIVATE RTI_INT32
DDS_DataReaderInterface_compare_bind(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DataReaderBindEntryKey *lkey = (struct DataReaderBindEntryKey*)op1;
    struct DataReaderBindEntryKey *rkey = (struct DataReaderBindEntryKey*)op2;
    UNUSED_ARG(flags);

    return OSAPI_Memory_compare(&lkey->peer,&rkey->peer,
                                sizeof(struct DataReaderBindEntryKey));
}

/*ci
 * \brief Initialize a Datareader NETIO interface instance
 *
 * \param[in] self     Interface to delete
 * \param[in] factory  Datareader NETIO interface factory that is creating
 *                     the instance
 * \param[in] property The property of the new Datareader NETIO interface.
 *                     Cannot be NULL.
 * \param[in] listener The listener for the new Datareader NETIO interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref DDS_DataReaderInterface_finalize
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_initialize(struct DDS_DataReaderInterface *self,
                struct DDS_DataReaderInterfaceFactory *factory,
                const struct DDS_DataReaderInterfaceProperty *const property,
                const struct NETIO_InterfaceListener *const listener)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;

    if (property == NULL)
    {
        return RTI_FALSE;
    }

    if (!NETIO_Interface_initialize(&self->_parent,
                           &DDS_DataReaderInterface_fv_Intf,
                           &property->_parent,
                           (listener ? listener : NULL)))
    {
        return RTI_FALSE;
    }

    /* Use the entity ID as the unique ID in table names. The entity ID is
     * unique for each DataReader within a DomainParticipant and each
     * DomainParticipant has its own database.
     */
    id._value = factory->_parent._id._value;
    tbl_property.max_records = (RTI_SIZE_T)property->_parent.max_binds;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
        (RTI_INT32)property->intf_address.value.guid_prefix.entity);
    dbrc = DB_Database_create_table(&self->_parent._btable,
                            property->_parent._parent.db,&tbl_name[0],
                            sizeof(struct DataReaderBindEntry),
                            DDS_DataReaderInterface_compare_bind,
                            &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        return RTI_FALSE;
    }

    self->_parent.state = NETIO_INTERFACESTATE_CREATED;
    self->factory = factory;
    self->datareader = property->datareader;
    self->db = property->_parent._parent.db;

    /* Always use the local_address, not the property */
    self->_parent.local_address = property->intf_address;

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Datareader NETIO interface instance
 *
 * \param[in] drintf Interface to finalize
 *
 * \sa \ref DDS_DataReaderInterface_initialize
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_finalize(struct DDS_DataReaderInterface *drintf)
{
    struct DataReaderBindEntry *bind;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;

    NETIO_Interface_finalize(&drintf->_parent);

    if (!DDS_Duration_is_infinite(
            &drintf->datareader->liveliness.lease_duration))
    {
        if (!OSAPI_Timer_delete_timeout(
                            drintf->datareader->config->timer,
                            &drintf->datareader->liveliness_event))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT)
            return RTI_FALSE;
        }
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(drintf->_parent._btable,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,drintf->_parent._btable,dbrc)
        return RTI_FALSE;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind);
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(drintf->_parent._btable,
                                          (DB_Record_T)bind);
            if (dbrc != DB_RETCODE_OK)
            {
                DB_Cursor_finish(drintf->_parent._btable,cursor);
                return RTI_FALSE;
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(drintf->_parent._btable,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    dbrc = DB_Database_delete_table(drintf->db,
                                    drintf->_parent._btable);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,drintf->_parent._btable,dbrc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Delete a Datareader NETIO interface instance
 *
 * \param[in] netio_intf Datareader NETIO Interface to delete
 *
 * \sa \ref DDS_DataReaderInterface_create
 */
RTI_PRIVATE void
DDS_DataReaderInterface_delete(struct DDS_DataReaderInterface *netio_intf)
{
    DDS_DataReaderInterface_finalize(netio_intf);
    OSAPI_Heap_free_struct(netio_intf);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Create a new Datareader NETIO interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new Datareader NETIO interface.
 * \param[in] listener  The listener for the new Datareader NETIO interface
 *
 * \return Pointer to new Datareader NETIO interface instance on success, NULL
 *         on failure
 *
 * \sa \ref DDS_DataReaderInterface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct DDS_DataReaderInterface*
DDS_DataReaderInterface_create(struct DDS_DataReaderInterfaceFactory *factory,
                  const struct DDS_DataReaderInterfaceProperty *const property,
                  const struct NETIO_InterfaceListener *const listener)
{
    struct DDS_DataReaderInterface *dw_intf = NULL;

    OSAPI_Heap_allocate_struct(&dw_intf, struct DDS_DataReaderInterface);
    if (dw_intf == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADERIO_OBJECT)
        return NULL;
    }

    if (!DDS_DataReaderInterface_initialize(dw_intf,factory,property,listener))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(dw_intf);
#endif
        return NULL;
    }

    return dw_intf;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_deserialize_stream_header(
                  struct CDR_Stream_t *stream,
                  struct NETIO_PacketInfo *pkt_info,
                  RTI_UINT16 *out_kind)
{
    /* deserialize header if needed; if already done in a previous
     * sample (can happen if a batch is received) do nothing as the
     * current encapsulation is already set.
     */
    if (!(pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA_BATCH) ||
         (pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA_BATCH_FIRST))
    {
        if (!CDR_Stream_deserialize_header(stream,out_kind))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci \brief Alias for REDA_BufferPool_T
 */
typedef REDA_BufferPool_T RTI_DataReaderSamplePool;

/*ci
 * \brief Helper function to rest the datareader sample.
 *        Should be called  every time a new entry is accessed
 *        via the buffer pool.
 *
 * \param[in] pool Pool to allocate sample from
 *
 * \return NULL on failure, initialized sample on success.
 */
RTI_PRIVATE struct RTI_DataReaderSample*
DDS_DataReaderInterface_get_sample(RTI_DataReaderSamplePool pool)
{
    struct RTI_DataReaderSample *sample = (struct RTI_DataReaderSample*)
                                             REDA_BufferPool_get_buffer(pool);

    if (sample != NULL)
    {
        sample->hst_sample.holder = NULL;
        sample->hst_sample.sample_access_intf = NULL;
        sample->hst_sample.status_info = 0;
        sample->hst_sample._info = NULL;
        sample->hst_sample._ref = NULL;
    }

    return sample;
}

/*ci
 * \brief Implementation of the NETIO_Interface_receive function
 *
 * \details
 *
 *  The Datareader NETIO interface receives data from a downstream interface.
 *  The data can be in either INTRA protocol format (same memory space and
 *  participant) or RTPS format. Depending on the downstream protocol slightly
 *  different actions are taken, but semantically it does not matter where
 *  data is coming from with one exception: The INTRA protocol does _not_
 *  support reliability and thus there is no back-channel for acknack.
 *  <p>
 *  The Datareader NETIO does not store the sample, after a successful reception
 *  it is stored in the datareader history cache. If a sample if filtered
 *  out by the user, no resources are used, including keys. That is, if a
 *  key is not seen _and_ filtered out it is never seen until at least one
 *  sample is received.
 *  <p>
 *  The receive interface is synchronous, _all_ processing takes place in the
 *  context of the downstream receive context, such as a thread.
 *  <p>
 *  The function only returns FALSE if it failed to process the
 *  received data in any way. It is not considered a failure if the data was
 *  processed properly but not made available to the application for valid
 *  reasons, such as resource limits being exhausted.
 *
 * \param[in] netio    The interface receiving data
 * \param[in] src_addr The source address of the data
 * \param[in] dst_addr The destination address of the data
 * \param[in] packet   A NETIO_Packet with the payload
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_receive(NETIO_Interface_T *netio,
                                struct NETIO_Address *src_addr,
                                struct NETIO_Address *dst_addr,
                                NETIO_Packet_T *packet)
{
    struct DDS_DataReaderInterface *drio =
                                      (struct DDS_DataReaderInterface *)netio;
    struct DDS_DataReaderImpl *datareader = drio->datareader;
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_SampleInfo *sample_info = NULL;
    DDSHST_ReturnCode_T hrc;
    NDDS_TypePluginKeyKind key_kind;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    DDS_InstanceHandle_t publisher_handle = DDS_HANDLE_NIL;
    DDS_UnsignedShort pid_id;
    DDS_UnsignedShort pid_length;
    RTI_SIZE_T total_pid_length;
    DDS_UnsignedLong status_info = 0;
    DDS_KeyHash_t key_hash;
    struct RTI_DataReaderSample *sample = NULL;
    DDSHST_ReaderSampleEntryRef_T sample_entry = NULL;
    struct NETIO_PacketInfo *pkt_info = NULL;
    RTI_BOOL has_inline_qos = RTI_FALSE;
    struct DataReaderBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DataReaderBindEntryKey bind_key;
    DDS_Boolean sample_dropped = DDS_BOOLEAN_FALSE;
    RTI_UINT32 cur_stream_pos;
    RTI_BOOL stream_le;
    DDS_SampleRejectedStatusKind reject_reason = DDS_NOT_REJECTED;
    const void *user_data = NULL;
    struct REDA_SequenceNumber epoch = {0,0};
    DDS_Boolean notify_on_committal = DDS_BOOLEAN_TRUE;
    RTI_BOOL has_header = RTI_FALSE;
    RTI_BOOL has_inline_keyhash = RTI_FALSE;
    RTI_BOOL add_new_keyhash = RTI_FALSE;
    RTI_BOOL deser_error = RTI_FALSE;
    DDS_InstanceHandle_t tmp_remote_handle = DDS_HANDLE_NIL;
    DDS_InstanceHandle_t tmp_local_handle = DDS_HANDLE_NIL;

#if DDS_FILTERING_ENABLED
    DDS_Boolean writer_filtered = DDS_BOOLEAN_FALSE;
    DDS_Boolean filter_result = DDS_BOOLEAN_FALSE;
#endif /* DDS_FILTERING_ENABLED */

    UNUSED_ARG(dst_addr);

    /* Make sure we do not read any data before the reader is enabled. This
     * check is added in case we receive data on a port that is shared
     */
    if (!DDS_DataReader_is_enabled(datareader))
    {
        return RTI_TRUE;
    }

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    OSAPI_TRACE_PRINTF1("receive topic: %s",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)))

    bind_key.peer = packet->source.value.guid;

    dbrc = DB_Table_select_match(drio->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&bind_entry,&bind_key);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_NET("dropping topic, no matching writer:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drio->datareader->topic)),RTI_TRUE)
        goto done;
    }

    OSAPI_TRACE_NET("received topic:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
            DDS_Topic_as_topicdescription(
                    drio->datareader->topic)),RTI_TRUE)

    pkt_info = NETIO_Packet_get_info(packet);

    if (pkt_info->protocol_id == NETIO_PROTOCOL_NOTIF)
    {
        OSAPI_Memory_copy(&instance_handle.octet, &pkt_info->instance, 16);
        instance_handle.is_valid = DDS_BOOLEAN_TRUE;
    }

    /* - If the received participant message is AUTO, only AUTO
     * writers are updated; it would be expected that MANUAL_xxxx
     * writers are manually asserted or data is received.
     * - If the participant message is MANUAL_BY_PARTICIPANT both
     * AUTO and MANUAL_BY_PARTICIPANT writers are updated.
     * MANUAL_BY_TOPIC writers are never updated when a participant
     * message is received.
     * - For any other received message liveliness timestamp is always
     * updated.
     */
    if ((pkt_info->rtps_flags & (~(NETIO_RTPS_FLAGS_MANUAL_LIVELINESS |
                                   NETIO_RTPS_FLAGS_AUTO_LIVELINESS)))
#if DDS_LIVELINESS_CHANNEL_ENABLED
        ||
        ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_AUTO_LIVELINESS) &&
         (bind_entry->liveliness_kind == DDS_AUTOMATIC_LIVELINESS_QOS)) ||
        ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_MANUAL_LIVELINESS) &&
         (bind_entry->liveliness_kind != DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS))
#endif
        )
    {
        if (!OSAPI_System_get_time(&bind_entry->activity_time))
        {
            /* get_time should never fail, but in case it does there is
             * not much that we can do so we will continue processing the
             * packet.
             */
            DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        }
    }

    /* process Lost Data first */
    if (pkt_info->rtps_flags & NETIO_RTPS_FLAGS_LOST_DATA)
    {
        OSAPI_TRACE_DDS("lost samples on topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drio->datareader->topic)),RTI_FALSE)
        OSAPI_TRACE_INT32("count",pkt_info->lost_sample_count,RTI_TRUE)

        DDS_DataReaderEvent_on_sample_lost(datareader,
                                           src_addr,&pkt_info->lost_sample_sn,
                                           pkt_info->lost_sample_count);
    }

    if ((pkt_info->rtps_flags & (NETIO_RTPS_FLAGS_LIVELINESS |
                                 NETIO_RTPS_FLAGS_COMMIT_DATA |
                                 NETIO_RTPS_FLAGS_DATA |
                                 NETIO_RTPS_FLAGS_DATA_BATCH))
#if DDS_LIVELINESS_CHANNEL_ENABLED
        ||
        ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_AUTO_LIVELINESS) &&
         (bind_entry->liveliness_kind == DDS_AUTOMATIC_LIVELINESS_QOS)) ||
        ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_MANUAL_LIVELINESS) &&
         (bind_entry->liveliness_kind != DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS))
#endif
        )
    {
        OSAPI_TRACE_NET("liveliness received for topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)

        if ((pkt_info->rtps_flags & (NETIO_RTPS_FLAGS_AUTO_LIVELINESS |
                                     NETIO_RTPS_FLAGS_MANUAL_LIVELINESS)) &&
            (src_addr == NULL))
        {
            src_addr = &packet->source;
        }

        if (bind_entry->writer_state == REMOTE_WRITERSTATE_NOT_ALIVE)
        {
            DDS_DataReaderEvent_on_liveliness_detected(datareader, &src_addr->value.guid);
            bind_entry->writer_state = REMOTE_WRITERSTATE_ALIVE;
        }
    }

    if (pkt_info->rtps_flags & (NETIO_RTPS_FLAGS_LIVELINESS |
                                NETIO_RTPS_FLAGS_AUTO_LIVELINESS |
                                NETIO_RTPS_FLAGS_MANUAL_LIVELINESS))
    {
        goto success;
    }


    OSAPI_Memory_copy(publisher_handle.octet,&packet->source.value.guid,16);
    publisher_handle.is_valid = DDS_BOOLEAN_TRUE;

    /* Only RTPS protocol (not INTRA or NOTIF) requires notification
     * (i.e. NETIO post_event) upon reader history sample committal.
     */
    if (pkt_info->protocol_id == NETIO_PROTOCOL_RTPS)
    {
        notify_on_committal = DDS_BOOLEAN_TRUE;
    }
    else
    {
        notify_on_committal = DDS_BOOLEAN_FALSE;
    }

    /* commit existing samples only */
    if (!pkt_info->valid_data &&
        (pkt_info->rtps_flags & NETIO_RTPS_FLAGS_COMMIT_DATA))
    {
        OSAPI_TRACE_DDS("commiting existing samples for topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)

        sample_dropped = DDS_BOOLEAN_TRUE;

        goto commit_sample;
    }

    if (pkt_info->rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS)
    {
        OSAPI_TRACE_DDS("received inline qos on topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)

        has_inline_qos = RTI_TRUE;
    }

    key_kind = DDS_TypePlugin_get_key_kind(datareader->type_plugin);

    if (pkt_info->protocol_id != NETIO_PROTOCOL_NOTIF)
    {
        if (!has_inline_qos && !pkt_info->valid_data)
        {
            /* no valid data or inline qos to process */
            goto success;
        }

        instance_handle = DDS_HANDLE_NIL;

        if (key_kind == NDDS_TYPEPLUGIN_NO_KEY)
        {
            instance_handle.is_valid = DDS_BOOLEAN_TRUE;
        }

        if (pkt_info->protocol_id == NETIO_PROTOCOL_INTRA)
        {
            has_inline_qos = RTI_FALSE;
            status_info = pkt_info->rtps_flags;
        }

        if (has_inline_qos || (pkt_info->protocol_id == NETIO_PROTOCOL_RTPS))
        {
            if (!CDR_Stream_set_buffer(
                    &datareader->stream,
                    pkt_info->protocol_data.rtps_data.inline_data->pointer,
                    pkt_info->protocol_data.rtps_data.inline_data->length))
            {
                DDSC_LOG_CDR_BUFFER_SET(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAREADER_CDR,&datareader->stream,
                    pkt_info->protocol_data.rtps_data.inline_data->pointer,
                    pkt_info->protocol_data.rtps_data.inline_data->length)

            }
        }

        /* packet's inline_data is buffer containing inline Qos and/or data */
        CDR_Stream_reset(&datareader->stream);

        CDR_Stream_set_vendor(&datareader->stream,
                            pkt_info->vendor_major_id,
                            pkt_info->vendor_minor_id);

        CDR_Stream_set_checksum_info(&datareader->stream,pkt_info->checksum_info);

        /* Set stream to byteswap according to RTPS message */
        stream_le = ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_LITTLE_ENDIAN) ?
                            RTI_TRUE : RTI_FALSE);

        CDR_Stream_byteswap_set(&datareader->stream, stream_le);

        /*
         * Note on deserialization of inline Qos:
         *
         * When a parameter ID is well known, its deserialized length can be
         * verified to be correct.  However, if the ID is unknown, its length
         * cannot be verified.  A bogus length then could cause the parser to skip
         * to a bogus location. The DataReader's inline Qos parser is robust
         * to not skip outside the bounds of the packet buffer, but it cannot
         * prevent the parser from skipping into the payload portion of the buffer
         * and interpreting it as inline Qos.  This can be prevented only if the
         * boundary between inline Qos and payload can be known.
         */
        if (has_inline_qos)
        {
            do
            {
                if (!CDR_Stream_deserialize_unsigned_short(
                                                    &datareader->stream,&pid_id))
                {
                    DDSC_LOG_CDR_DESERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                                                DDSC_LOG_DATAREADER_CDR)
                    deser_error = RTI_TRUE;
                    goto commit_sample;
                }

                if (!CDR_Stream_deserialize_unsigned_short(&datareader->stream,
                        &pid_length))
                {
                    DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                                                        DDSC_LOG_DATAREADER_CDR,
                                                        (RTI_INT32)pid_length)
                    deser_error = RTI_TRUE;
                    goto commit_sample;
                }

                OSAPI_TRACE_DDS("processing inline qos on topic:",RTI_FALSE)
                OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(
                                drio->datareader->topic)),RTI_FALSE)
                OSAPI_TRACE_INT32("id",pid_id,RTI_FALSE)
                OSAPI_TRACE_INT32("length",pid_length,RTI_TRUE)

                switch (pid_id)
                {
                case RTPS_PID_SENTINEL:
                    /* Ignore length for the SENTINEL by setting it to 0 to
                     * prevent the length from being added to the total_pid_length.
                     */
                    pid_length = 0;
                    break;

                case RTPS_PID_KEY_HASH:
                    OSAPI_TRACE_DDS("found inline key hash on topic:",RTI_FALSE)
                    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)

                    if (pid_length != RTPS_KEY_HASH_PARAM_LENGTH)
                    {
                        DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                                                            DDSC_LOG_DATAREADER_CDR,
                                                            pid_length)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    if (!CDR_Stream_deserialize_byte_array(&datareader->stream,
                                                instance_handle.octet, pid_length))
                    {
                        DDSC_LOG_CDR_DESERIALIZE_KEYHASH(OSAPI_LOGKIND_ERROR,
                                                        DDSC_LOG_DATAREADER_CDR,
                                                        pid_length)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    has_inline_keyhash = RTI_TRUE;

                    /* Determine if this keyhash can be used.
                     * if the writer uses a different CDR version than the reader
                     * the keyhash is invalidated. This forces a recalculation
                     * later.
                     */
                    if (datareader->cdr_id != bind_entry->cdr_id)
                    {
                        const struct DDS_DataReaderKeyHashEntry *hash;

                        hash = DDS_DataReaderImpl_lookup_local_keyhash(
                                                    datareader,&instance_handle);

                        if (hash != NULL)
                        {
                            OSAPI_Memory_copy(&instance_handle,&hash->local_hash,
                                            (RTI_SIZE_T)sizeof(DDS_InstanceHandle_t));
                            instance_handle.is_valid = DDS_BOOLEAN_TRUE;
                        }
                        else
                        {
                            instance_handle.is_valid = DDS_BOOLEAN_FALSE;
                        }
                    }
                    else
                    {
                        instance_handle.is_valid = DDS_BOOLEAN_TRUE;
                    }
                    break;

                case RTPS_PID_STATUS_INFO:
                    if (pid_length != RTPS_STATUS_INFO_PARAM_LENGTH)
                    {
                        DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                                                            DDSC_LOG_DATAREADER_CDR,
                                                            pid_length)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }
                    if (!CDR_Stream_deserialize_unsigned_long_from_big_endian(
                                            &datareader->stream, &status_info))
                    {
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    OSAPI_TRACE_DDS("found in-line status info topic:",RTI_FALSE)
                    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_FALSE)
                    OSAPI_TRACE_INT32("status",status_info,RTI_FALSE)
                    OSAPI_TRACE_INT32("unregister",
                            status_info & RTPS_UNREGISTER_STATUS_INFO,RTI_FALSE)
                    OSAPI_TRACE_INT32("dispose",
                                status_info & RTPS_DISPOSE_STATUS_INFO,RTI_TRUE)
                    break;

                case RTPS_PID_SAMPLE_EPOCH:
                    if (!CDR_Stream_is_vendor_rti(&datareader->stream))
                    {
                        /* epoch is RTI vendor-specific ignore the PID*/
                        if (!CDR_Stream_increment_current_position(
                                &datareader->stream, pid_length))
                        {
                            DDSC_LOG_CDR_INCREMENT_POS(OSAPI_LOGKIND_ERROR,
                                                    DDSC_LOG_DATAREADER_CDR,
                                                    pid_length)
                            deser_error = RTI_TRUE;
                            goto commit_sample;
                        }
                        break;
                    }
                    if (!CDR_Stream_deserialize_long(&datareader->stream, &epoch.high))
                    {
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    if (!CDR_Stream_deserialize_long(&datareader->stream, &epoch.low))
                    {
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }
                    break;
#if DDS_FILTERING_ENABLED
                case RTPS_PID_CONTENT_FILTER_INFO:
                    if (!DDS_DataReader_process_filter_info(
                            datareader, packet, &datareader->stream, pid_length,
                            &writer_filtered, &filter_result))
                    {
                        DDSC_LOG_DESERIALIZE_CONTENT_FILTER_INFO(OSAPI_LOGKIND_WARNING)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }
                    break;
#endif /* DDS_FILTERING_ENABLED */
                default:
                    OSAPI_TRACE_DDS("ignoring unknown PID received on topic:",RTI_FALSE)
                    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_FALSE)
                    OSAPI_TRACE_INT32("pid",pid_id,RTI_FALSE)
                    OSAPI_TRACE_INT32("length",pid_length,RTI_TRUE)

                    if (!CDR_Stream_increment_current_position(&datareader->stream,
                                                        pid_length))
                    {
                        DDSC_LOG_CDR_INCREMENT_POS(OSAPI_LOGKIND_ERROR,
                                                DDSC_LOG_DATAREADER_CDR,
                                                pid_length)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }
                    break;
                }

                /* update the pointer to the end of inline_data */
                total_pid_length = pid_length + 2U * (RTI_SIZE_T)sizeof(DDS_UnsignedShort);

                pkt_info->protocol_data.rtps_data.inline_data->pointer += total_pid_length;
                pkt_info->protocol_data.rtps_data.inline_data->length -= total_pid_length;

            }
            while (pid_id != RTPS_PID_SENTINEL);
        }

        /* Reset stream to native endianness before deserializing data. This is
         * needed because the encapsulation header may be in a different endianess
         * than the inline qos and not resetting it will cause the byte swap flag
         * to potentially be incorrect.
         */
#ifdef RTI_ENDIAN_LITTLE
        CDR_Stream_byteswap_set(&datareader->stream, RTI_TRUE);
#else
        CDR_Stream_byteswap_set(&datareader->stream, RTI_FALSE);
#endif

        /* For a DATA_BATCH it is always needed to deserialize header, even if there
         * is no data nor key because the first sample in the batch can be an invalid
         * sample or a dispose or unregister without valid data. But it is always needed
         * to deserialize the header with the first sample as the second and following
         * samples do not have its serialized data next to the header
         */
        if (packet->info.rtps_flags & NETIO_RTPS_FLAGS_DATA_BATCH)
        {
            if (!CDR_Stream_set_buffer(
                    &datareader->stream,
                    pkt_info->protocol_data.rtps_data.serialized_data_batch->pointer,
                    pkt_info->protocol_data.rtps_data.serialized_data_batch->length))
            {
                DDSC_LOG_CDR_BUFFER_SET(
                    OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAREADER_CDR,
                    &datareader->stream,
                    pkt_info->protocol_data.rtps_data.serialized_data_batch->pointer,
                    pkt_info->protocol_data.rtps_data.serialized_data_batch->length)
                goto done;
            }
        }

        /* for a DATA_BATCH without valid data nor valid key and lost data
         * means that the sample is marked as invalid just jump to success
         * (on_sample_lost already called). Note that for unregister or
         * dispose samples it is possible to receive a sample without data nor
         * key, but in that case lost_data flag will not be set
         */
        if ((packet->info.rtps_flags & NETIO_RTPS_FLAGS_DATA_BATCH) &&
            (packet->info.rtps_flags & NETIO_RTPS_FLAGS_LOST_DATA) &&
            (!packet->info.valid_data) && (!packet->info.valid_key))
        {
            /* header has to be deserialized always with the first sample
             * in the batch, even if the sample is invalid, lost, etc.
             */
            if (!DDS_DataReaderInterface_deserialize_stream_header(
                    &datareader->stream,pkt_info,&datareader->current_eid))
            {
                DDSC_LOG_CDR_DESERIALIZE_HEADER(
                    OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
                deser_error = RTI_TRUE;
                goto commit_sample;
            }

            goto success;
        }

        /* If the user has installed this callback, let the user
         * determine whether to keep the sample or not. it is very
         * important to note that only in this case the filtering is done
         * _before_ any resources are allocated.
         */
        if ((datareader->listener.on_before_sample_deserialize) &&
            (pkt_info->protocol_id == NETIO_PROTOCOL_RTPS) &&
            packet->info.valid_data)
        {
            if (!DDS_DataReaderInterface_deserialize_stream_header(
                    &datareader->stream,pkt_info,&datareader->current_eid))
            {
                DDSC_LOG_CDR_DESERIALIZE_HEADER(
                                    OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)

                deser_error = RTI_TRUE;
                goto commit_sample;
            }

            has_header = RTI_TRUE;
            sample_dropped = DDS_BOOLEAN_FALSE;
            cur_stream_pos = CDR_Stream_get_current_position_offset(&datareader->stream);

            if (!DDS_TypePlugin_set_stream_encapsulation(datareader->type_plugin,
                                                        &datareader->stream,
                                                        datareader->current_eid))
            {
                DDSC_LOG_CDR_DESERIALIZE_HEADER(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR);
                sample_dropped = DDS_BOOLEAN_TRUE;
                goto commit_sample;
            }

            if (!datareader->listener.on_before_sample_deserialize(
                                    datareader->listener.as_listener.listener_data,
                                    datareader,datareader->type_plugin,
                                    &datareader->stream,&sample_dropped))
            {
                OSAPI_TRACE_DDS("user filter function failed on topic:",RTI_FALSE)
                OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                                    DDS_Topic_as_topicdescription(
                                        drio->datareader->topic)),RTI_TRUE)
                DDSC_LOG_DR_FILTER_ERROR(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            if (sample_dropped)
            {
                goto commit_sample;
            }

            if (!CDR_Stream_set_current_position_offset(&datareader->stream,
                                                        cur_stream_pos))
            {
                DDSC_LOG_CDR_SET_POS(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR,
                                    cur_stream_pos)
                deser_error = RTI_TRUE;
                goto commit_sample;
            }
        }

#if DDS_FILTERING_ENABLED
        /* Skip deserialization if the writer indicated that the sample should be filtered out */
        if (writer_filtered && !filter_result)
        {
            sample_dropped = DDS_BOOLEAN_TRUE;
            goto commit_sample;
        }
#endif /* DDS_FILTERING_ENABLED */

        sample = DDS_DataReaderInterface_get_sample(datareader->cdr_samples);
        if (sample == NULL)
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES);
            goto done;
        }

        if (!instance_handle.is_valid)
        {
            if (pkt_info->protocol_id == NETIO_PROTOCOL_INTRA)
            {
                OSAPI_Memory_copy(&instance_handle.octet, &pkt_info->instance, 16);
                instance_handle.is_valid = DDS_BOOLEAN_TRUE;
            }
            else if ((pkt_info->protocol_id == NETIO_PROTOCOL_RTPS) &&
                    (packet->info.valid_data || packet->info.valid_key))
            {
                /* Save this handle in case it needs to be added to the keyhash
                * mapping table later.
                */
                tmp_remote_handle = instance_handle;

                /* deserialize header. this needs be done because if a batch is received
                * the serialized data for a sample is only immediately after the
                * encapsulation id for the first sample.
                */
                if (!has_header && !DDS_DataReaderInterface_deserialize_stream_header(
                                            &datareader->stream,pkt_info,
                                            &datareader->current_eid))
                {
                    DDSC_LOG_CDR_DESERIALIZE_HEADER(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
                    deser_error = RTI_TRUE;
                    goto commit_sample;
                }

                has_header = RTI_TRUE;
                if (!DDS_TypePlugin_set_stream_encapsulation(datareader->type_plugin,
                                                            &datareader->stream,
                                                            datareader->current_eid))
                {
                    DDSC_LOG_CDR_DESERIALIZE_HEADER(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR);
                    sample_dropped = DDS_BOOLEAN_TRUE;
                    goto commit_sample;
                }

                if (packet->info.valid_data)
                {
                    sample->hst_sample.holder = DDS_TypePlugin_get_sample(
                                                        datareader->type_plugin,
                                                        &datareader->stream);

                    if ((sample->hst_sample.holder == NULL) ||
                        (sample->hst_sample.holder->sample == NULL))
                    {
                        DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,
                                                DDSC_LOG_SAMPLE_RESOURCES)
                        goto done;
                    }

                    if (DDS_BOOLEAN_TRUE != DDS_TypePlugin_deserialize_sample(
                                                datareader->type_plugin,
                                                sample->hst_sample.holder->sample,
                                                &datareader->stream,&publisher_handle))
                    {
                        DDSC_LOG_CDR_DESERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    OSAPI_TRACE_DDS("key not found for topic :",RTI_FALSE)
                    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                                        DDS_Topic_as_topicdescription(
                                                drio->datareader->topic)),RTI_TRUE)

                    key_hash.length = RTPS_KEY_HASH_MAX_LENGTH;

                    OSAPI_Memory_zero(key_hash.value, key_hash.length);
                    OSAPI_Memory_zero(instance_handle.octet, key_hash.length);
                    CDR_Stream_reset(datareader->md5_stream);

                    if (!DDS_TypePlugin_instance_to_keyhash(datareader->type_plugin,
                                                            datareader->md5_stream,
                                                            &key_hash,
                                                            sample->hst_sample.holder->sample,
                                                            datareader->cdr_id))
                    {
                        DDSC_LOG_DR_INSTANCE_TO_KEYHASH(OSAPI_LOGKIND_ERROR)
                        deser_error = RTI_TRUE;
                        goto commit_sample;
                    }

                    /* If we received an invalid remote inline keyhash, update the
                     * keyhash database
                     */
                    if (has_inline_keyhash &&
                        (datareader->cdr_id != bind_entry->cdr_id))
                    {
                        add_new_keyhash = RTI_TRUE;
                        OSAPI_Memory_copy(&tmp_local_handle.octet,&key_hash.value,
                                        key_hash.length);
                    }

                    OSAPI_Memory_copy(instance_handle.octet, key_hash.value,
                                    key_hash.length);

                    instance_handle.is_valid = DDS_BOOLEAN_TRUE;

                    user_data = sample->hst_sample.holder->sample;
                } /* packet->info.valid_key */
                else if (packet->info.rtps_flags & NETIO_RTPS_FLAGS_HAS_INLINE_KEY)
                {
                    tmp_remote_handle = instance_handle;

                    CDR_Stream_reset(datareader->md5_stream);
                    key_hash.length = RTPS_KEY_HASH_MAX_LENGTH;
                    OSAPI_Memory_zero(key_hash.value, key_hash.length);
                    OSAPI_Memory_zero(instance_handle.octet, key_hash.length);

#if DDS_XTYPES_IS_ENABLED
                    /* This is a special scenario for FLAT_DATA
                     *
                     * If we received a dispose for a FLAT_DATA type without the inline QoS
                     * Then there should be a serialized key.
                     * Call the deserialize_key for FLAT_DATA which is a special typed function
                     * which will calculate the keyhash and store it as the first 16 bytes
                     * of datareader->md5_stream
                     */
                    if ((datareader->type_plugin->property.type_code != NULL)
                        && (datareader->type_plugin->property.type_code->_data._kind
                            & DDS_TK_FLAT_DATA_LANGUAGE_BINDING))
                    {
                        if (!DDS_TypePlugin_deserialize_key(datareader->type_plugin,
                                                            datareader->md5_stream,
                                                            &datareader->stream,
                                                            NULL))
                        {
                            DDSC_LOG_CDR_DESERIALIZE_KEY(OSAPI_LOGKIND_ERROR,
                                                        DDSC_LOG_DATAREADER_CDR)
                            deser_error = RTI_TRUE;
                            goto commit_sample;
                        }

                        OSAPI_Memory_copy(instance_handle.octet,
                                        datareader->md5_stream->real_buff,
                                        key_hash.length);
                    }
                    else
#endif
                    {
                        sample->hst_sample.holder = DDS_TypePlugin_get_sample(
                                                            datareader->type_plugin,
                                                            &datareader->stream);

                        /* We need a sample to deserialize the key into so that we can
                         * calculated the keyhash. However, we only need this buffer
                         * temporarily.
                         */
                        if ((sample->hst_sample.holder == NULL) ||
                            (sample->hst_sample.holder->sample == NULL))
                        {
                            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,
                                                    DDSC_LOG_SAMPLE_RESOURCES)
                            goto done;
                        }

                        /* Regular type. Proceed as usual. Deserialize the key into
                         * the Plain sample and calculate its keyhash
                         */
                        if (!DDS_TypePlugin_deserialize_key(datareader->type_plugin,
                                                            sample->hst_sample.holder->sample,
                                                            &datareader->stream,
                                                            NULL))
                        {
                            DDSC_LOG_CDR_DESERIALIZE_KEY(
                                    OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_DATAREADER_CDR)
                            deser_error = RTI_TRUE;
                            goto commit_sample;
                        }

                        if (!DDS_TypePlugin_instance_to_keyhash(datareader->type_plugin,
                                                            datareader->md5_stream,
                                                            &key_hash,
                                                            sample->hst_sample.holder->sample,
                                                            datareader->cdr_id))
                        {
                            DDSC_LOG_DR_INSTANCE_TO_KEYHASH(OSAPI_LOGKIND_ERROR)
                            deser_error = RTI_TRUE;
                            goto commit_sample;
                        }

                        OSAPI_Memory_copy(instance_handle.octet, key_hash.value,
                                        key_hash.length);

                        /* We calculated the keyhash, return the sample */
                        DDS_TypePlugin_return_sample(datareader->type_plugin,
                                                    sample->hst_sample.holder);
                        sample->hst_sample.holder = NULL;
                    }

                    /* If we received an invalid remote inline keyhash, update the
                     * keyhash database
                     */
                    if (has_inline_keyhash &&
                        (datareader->cdr_id != bind_entry->cdr_id))
                    {
                        add_new_keyhash = RTI_TRUE;

                        OSAPI_Memory_copy(&tmp_local_handle.octet,&key_hash.value,
                                        key_hash.length);
                    }

                    /* If we got to this point, the instance handle should be valid */
                    instance_handle.is_valid = DDS_BOOLEAN_TRUE;
                }
            }
        }
    }
    else if (pkt_info->protocol_id == NETIO_PROTOCOL_NOTIF)
    {
        sample = DDS_DataReaderInterface_get_sample(datareader->cdr_samples);
        if (sample == NULL)
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES);
            goto done;
        }

        if (!DDS_TypePlugin_set_encapsulation(datareader->type_plugin,DDS_ENCAPSULATION_ID_SHMEM_V2))
        {
            /* could not find an encapsulation for Notification */
            deser_error = RTI_TRUE;
            goto commit_sample;

        }
        /* The sample holder is retrieved from the typeplugin */
        sample->hst_sample.holder = DDS_TypePlugin_get_sample(
                                                            datareader->type_plugin,
                                                            &datareader->stream);
        if (sample->hst_sample.holder == NULL)
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES)
            goto done;
        }
        OSAPI_Memory_copy(sample->hst_sample.holder->sample,
                        &(pkt_info->protocol_data.notif_info),
                        sizeof(struct NETIO_OpaqueInfo));
        sample->hst_sample.sample_access_intf = pkt_info->protocol_data.notif_info.sample_accesor;
    }


    if (!instance_handle.is_valid)
    {
        /* If we cannot deduce the key, drop the sample and consider it
         * received. This could happen for example receiving a dispose
         * as the first sample, but the dispose does not contain a key.
         */
        DDSC_LOG_DR_DESERIALIZE_KEYHASH(OSAPI_LOGKIND_ERROR)
        sample_dropped = DDS_BOOLEAN_TRUE;
        goto commit_sample;
    }

    if ((key_kind != NDDS_TYPEPLUGIN_NO_KEY) &&
         !datareader->protocol.propagate_dispose_of_unregistered_instances)
    {
        if (status_info & (RTPS_UNREGISTER_STATUS_INFO | RTPS_DISPOSE_STATUS_INFO))
        {
            DDSHST_ReaderKeyEntryRef_T key_entry = NULL;

            key_entry = DDSHST_Reader_lookup_key(datareader->_rh,&instance_handle);
            if (key_entry == NULL)
            {
                sample_dropped = DDS_BOOLEAN_TRUE;
                goto commit_sample;
            }
        }
    }

    /* Key is valid, check if it exists */
    sample_entry = DDSHST_Reader_reserve_entry(datareader->_rh,
                        &publisher_handle,&instance_handle,
                        &sample->hst_sample._info,bind_entry->strength,
                        &pkt_info->sn,&pkt_info->virtual_sn,&reject_reason);

    if (sample_entry == NULL)
    {
        DDSC_LOG_DR_GET_ENTRY_FAILED(OSAPI_LOGKIND_ERROR,reject_reason)

        OSAPI_TRACE_DDS("no queue entry available on topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_FALSE)
        OSAPI_TRACE_INT32("reason",reject_reason,RTI_TRUE)

        /* These reject reasons are all valid and considered successful
         * from a protocol point view.
         */
        if (reject_reason == DDS_REJECTED_BY_REMOTE_WRITERS_LIMIT)
        {
            sample_dropped = DDS_BOOLEAN_TRUE;
            goto commit_sample;
        }

        goto done;
    }

    /* Have both a sample and a reserved entry */
    sample_info = sample->hst_sample._info;
    sample->hst_sample.status_info = status_info;
    sample_info->encapsulation_id = datareader->current_eid;
    sample_info->related_epoch.high = epoch.high;
    sample_info->related_epoch.low = epoch.low;
    sample_info->transport_type = packet->info.originating_transport;

    sample_info->valid_data = DDS_BOOLEAN_FALSE;
    if ((pkt_info->protocol_id == NETIO_PROTOCOL_INTRA) &&
         (pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA))
    {
        sample_info->encapsulation_id = DDS_ENCAPSULATION_ID_MEMORY;
        sample_info->valid_data = DDS_BOOLEAN_TRUE;

        sample->hst_sample.holder = DDS_TypePlugin_get_sample(
                                            datareader->type_plugin,
                                            &datareader->stream);

        if ((sample->hst_sample.holder == NULL) ||
            (sample->hst_sample.holder->sample == NULL))
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_SAMPLE_RESOURCES)
            goto done;
        }

        if (!DDS_TypePlugin_copy_sample(datareader->type_plugin,
                                sample->hst_sample.holder->sample,
                                pkt_info->protocol_data.intra_info.user_data))
        {
            DDSC_LOG_DR_COPY_DATA_SAMPLE(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        user_data = pkt_info->protocol_data.intra_info.user_data;
    }
    else if ((pkt_info->protocol_id == NETIO_PROTOCOL_RTPS) &&
              packet->info.valid_data && (user_data == NULL))
    {
        if (!has_header && !DDS_DataReaderInterface_deserialize_stream_header(
                  &datareader->stream,pkt_info,
                  &datareader->current_eid))
        {
            DDSC_LOG_CDR_DESERIALIZE_HEADER(
                OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
            deser_error = RTI_TRUE;
            goto commit_sample;
        }

        sample_info->encapsulation_id = datareader->current_eid;

        if (!DDS_TypePlugin_set_stream_encapsulation(datareader->type_plugin,
                                                     &datareader->stream,
                                                     datareader->current_eid))
        {
            OSAPI_TRACE_PRINTF2("failed to set encapsulation to %^d on %G, "
                                "dropping sample",
                                &datareader->current_eid,
                                &drio->_parent.local_address.value.as_uint32.value[0])
            DDSC_LOG_CDR_DESERIALIZE_HEADER(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR);
            sample_dropped = DDS_BOOLEAN_TRUE;
            goto commit_sample;
        }

        sample->hst_sample.holder = DDS_TypePlugin_get_sample(
                                                datareader->type_plugin,
                                                &datareader->stream);

        if ((sample->hst_sample.holder == NULL) ||
            (sample->hst_sample.holder->sample == NULL))
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_SAMPLE_RESOURCES)
            goto done;
        }

        if (DDS_BOOLEAN_TRUE != DDS_TypePlugin_deserialize_sample(
                                            datareader->type_plugin,
                                            sample->hst_sample.holder->sample,
                                            &datareader->stream,&publisher_handle))
        {
            DDSC_LOG_CDR_DESERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
            deser_error = RTI_TRUE;
            goto commit_sample;
        }
        sample_info->valid_data = DDS_BOOLEAN_TRUE;

        user_data = sample->hst_sample.holder->sample;
    }
    else if ((pkt_info->protocol_id == NETIO_PROTOCOL_RTPS) &&
             (user_data != NULL))
    {
        sample_info->valid_data = DDS_BOOLEAN_TRUE;
    }
    else if (pkt_info->protocol_id == NETIO_PROTOCOL_NOTIF)
    {
        sample->hst_sample.status_info = pkt_info->rtps_flags;
        sample_info->valid_data = pkt_info->valid_data;
        sample_info->encapsulation_id = pkt_info->encapsulation;
    }

    sample_info->source_timestamp.sec = pkt_info->timestamp.sec;
    sample_info->source_timestamp.nanosec = pkt_info->timestamp.nanosec;

    /* If the user has installed the on_before_sample_commit listener,
     * let the user determine if the sample should be committed.
     */
    if (datareader->listener.on_before_sample_commit && sample_info->valid_data)
    {
        sample_dropped = DDS_BOOLEAN_FALSE;

        /* Its possible that user data is NULL for ZCV2 */
        /* coverity[var_deref_model : FALSE] */
        /* coverity[cert_exp34_c_violation : FALSE] */
        if (!datareader->listener.on_before_sample_commit(
                            datareader->listener.as_listener.listener_data,
                            datareader,user_data,sample_info,&sample_dropped))
        {
            DDSC_LOG_DR_FILTER_ERROR(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        if (sample_dropped)
        {
            goto commit_sample;
        }
    }

#if DDS_FILTERING_ENABLED
    /* If the writer did not apply already apply our filter, then apply it ourself.
     * If the writer applied the filter, then we know the sample should be allowed
     * because otherwise it would have been filtered out before deserialization.
     */
    if ((datareader->compiled_filter != NULL) && packet->info.valid_data && !writer_filtered)
    {
        if (!DDS_DataReader_evaluate_filter(datareader, user_data, &sample_dropped))
        {
            /* Fail gracefully so that the DataReader can attempt to continue reciving
             * samples, but log an error because this condition is not expected.
             */
            DDSC_LOG_EVALUATE_CONTENT_FILTER(OSAPI_LOGKIND_ERROR)
            sample_dropped = DDS_BOOLEAN_TRUE;
        }
        /* Purposefully fall through to commit_sample which will check if sample_dropped */
    }
#endif /* DDS_FILTERING_ENABLED */

commit_sample:

    if (sample_dropped || deser_error)
    {
        /* We cannot differentiate between a sample which was corrupted in transit
         * or a sample that was serialized incorrectly. The assumption here is that
         * if a sample cannot be deserialized, then it was serialized incorrectly
         * and resending it will not help. The CRC feature can be used to protect
         * against samples being corrupted in transit.
         */
        OSAPI_TRACE_DDS("commit only for topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)


        if (sample != NULL)
        {
            if (sample->hst_sample.holder != NULL)
            {
                DDS_TypePlugin_return_sample(datareader->type_plugin,
                                             sample->hst_sample.holder);
                sample->hst_sample.holder = NULL;
            }
            REDA_BufferPool_return_buffer(datareader->cdr_samples, sample);
        }

        if (sample_entry != NULL)
        {
            DDSHST_Reader_return_entry(datareader->_rh, sample_entry);
        }

        /* MICRO-11917
         * Unless the sample was rejected by the history, we still want to commit
         * dropped samples so that the history is allowed to advance its state
         * and treat them as received (not lost) but unavailable.
         */
        if (reject_reason == DDS_NOT_REJECTED)
        {
            hrc = DDSHST_Reader_commit(datareader->_rh,
                                       &publisher_handle,
                                       &pkt_info->committable_sn,
                                       bind_entry->strength,
                                       notify_on_committal,
                                       &pkt_info->last_committed_virtual_sn,
                                       &pkt_info->lost_sample_count);
        }
        else
        {
            hrc = DDSHST_RETCODE_SUCCESS;
        }

        if (pkt_info->rtps_flags & (NETIO_RTPS_FLAGS_DATA | NETIO_RTPS_FLAGS_DATA_BATCH))
        {
            /* If the sample contained valid data, we need to directly return
             * the sample to the downstream interface because the history will
             * not do this for us for samples which are not added to the history.
             */
            pkt_info->lost_sample_count = 0;
            if (notify_on_committal)
            {
                DDS_DataReaderEvent_on_sample_committed(datareader->_rh,
                                                        datareader,
                                                        &publisher_handle,1);
            }
        }
    }
    else
    {
        OSAPI_TRACE_DDS("adding and committing entry for topic:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(
                                    drio->datareader->topic)),RTI_TRUE)

        hrc = DDSHST_Reader_add_and_commit(
                                  datareader->_rh,
                                  sample_entry,&sample->hst_sample,
                                  &pkt_info->committable_sn,
                                  notify_on_committal,
                                  &pkt_info->last_committed_virtual_sn,
                                  &pkt_info->lost_sample_count);

        if ((hrc == DDSHST_RETCODE_SUCCESS) && add_new_keyhash &&
                !DDS_DataReaderImpl_add_remote_keyhash(datareader,
                                                       &tmp_remote_handle,
                                                       &tmp_local_handle))
        {
            /* Add the keyhash here if a new mapping was detected. It is added
             * after a sample entry has been successfully allocated as it ensures
             * that the resource-limits for the key-hash indices are consistent
             * with the reader cache.
             */
             DDSC_LOG_DR_INSTANCE_MAPPING_EXHAUSTED(OSAPI_LOGKIND_ERROR)
             goto done;
        }
    }

    if (hrc != DDSHST_RETCODE_SUCCESS)
    {
        DDSC_LOG_DR_COMMIT_SAMPLE(OSAPI_LOGKIND_ERROR,
                                  pkt_info->committable_sn.high,
                                  pkt_info->committable_sn.low,hrc)

        /* To late to return sample and entry */
        sample = NULL;
        sample_entry = NULL;
        goto done;
    }

success:

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        if (sample != NULL)
        {
            if (sample->hst_sample.holder != NULL)
            {
                DDS_TypePlugin_return_sample(datareader->type_plugin,sample->hst_sample.holder);
                sample->hst_sample.holder = NULL;
            }
            REDA_BufferPool_return_buffer(datareader->cdr_samples, sample);
        }

        if (sample_entry != NULL)
        {
            DDSHST_Reader_return_entry(datareader->_rh, sample_entry);
        }
    }

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Implementation of the NETIO add_route function
 *
 * \details
 *
 * The datareader does not maintain state on a per route basis, the function
 * is implemented only to simplify a controller, e.g. the controller does
 * not need to know if the interface handles routes or not.
 *
 * \param[in] self       NETIO interface to add the route too
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_add_route(NETIO_Interface_T *self,
                                  struct NETIO_Address *dst_addr,
                                  NETIO_Interface_T *via_intf,
                                  struct NETIO_Address *via_addr,
                                  struct NETIORouteProperty *property,
                                  RTI_BOOL *existed)
{
    UNUSED_ARG(self);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO delete_route function
 *
 * \details
 *
 * The datareader does not maintain state on a per route basis, the function
 * is implemented only to simplify a controller, e.g. the controller does
 * need to know if the interface handles state or not. Thus, this function
 * does nothing.
 *
 * \param[in]  self     NETIO interface to add the route too
 * \param[in]  dst_addr The destination address for the route
 * \param[in]  via_intf The downstream interface
 * \param[in]  via_addr The address to pass to the downstream interface
 * \param[out] existed  Whether the route existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_delete_route(NETIO_Interface_T *self,
                                     struct NETIO_Address *dst_addr,
                                     NETIO_Interface_T *via_intf,
                                     struct NETIO_Address *via_addr,
                                     RTI_BOOL *existed)
{
    UNUSED_ARG(self);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind function
 *
 * \details
 *
 * When a datareader is matched with a datawriter it creates an entry in the
 * bind-table to listen to the datawriter. There is never more than one entry
 * in the bind table per datawriter.
 *
 * \param[in]  netio    NETIO interface to bind
 * \param[in]  src_addr The address to bind to
 * \param[in]  property The property to use for the bind
 * \param[out] existed  Whether a previous bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_bind(NETIO_Interface_T *netio,
                             struct NETIO_Address *src_addr,
                             struct NETIOBindProperty *property,
                             RTI_BOOL *existed)
{
    struct DDS_DataReaderInterface *drintf =
                                    (struct DDS_DataReaderInterface *)netio;
    struct DataReaderBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DataReaderBindEntryKey bind_key;
    DDS_InstanceHandle_t ih_peer;
    const struct OSAPI_SystemTime zero_time = OSAPI_TIME_ZERO;

    bind_key.peer = src_addr->value.guid;

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    OSAPI_TRACE_DDS("datareader binding to datawriter:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drintf->datareader->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&drintf->_parent.local_address.value.guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_TRUE)

    dbrc = DB_Table_select_match(drintf->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&bind_entry,&bind_key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        return RTI_TRUE;
    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,drintf->_parent._btable,dbrc)
        return RTI_FALSE;
    }

    if (DB_Table_create_record(drintf->_parent._btable,
                              (DB_Record_T *)&bind_entry) != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    DDS_InstanceHandle_from_netio_address(&ih_peer,src_addr);

    if (!DDS_TypePlugin_add_peer(drintf->datareader->type_plugin,&ih_peer))
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(bind_entry,sizeof(struct DataReaderBindEntry));
    bind_entry->key.peer = src_addr->value.guid;
    if (!OSAPI_System_get_time(&bind_entry->activity_time))
    {
        /* get_time should never fail, but in case it does there is not
         * much that we can do.
         */
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        bind_entry->activity_time = zero_time;
    }

    bind_entry->writer_state = REMOTE_WRITERSTATE_ALIVE;
    if (property != NULL)
    {
#if DDS_LIVELINESS_CHANNEL_ENABLED
        bind_entry->liveliness_kind = property->liveliness_kind;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
        bind_entry->strength = property->strength;
        bind_entry->lease_duration = property->lease_duration;
        bind_entry->cdr_id = property->data_format;
    }
    else
    {
#if DDS_LIVELINESS_CHANNEL_ENABLED
        bind_entry->liveliness_kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
        bind_entry->strength = 0;
        bind_entry->lease_duration.sec = OSAPI_SYSTEM_TIME_SEC_MAX;
        bind_entry->lease_duration.nanosec = OSAPI_SYSTEM_TIME_NANO_MAX;
        bind_entry->cdr_id = drintf->datareader->cdr_id;
    }

    dbrc = DB_Table_insert_record(drintf->_parent._btable,
                                (DB_Record_T)bind_entry);

    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_EXISTS))
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        (void)DB_Table_delete_record(drintf->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    OSAPI_TRACE_DDS("datareader bound to datawriter:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drintf->datareader->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&drintf->_parent.local_address.value.guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&bind_entry->key.peer,RTI_TRUE)

    DDS_DataReaderEvent_on_liveliness_detected(
            drintf->datareader,&bind_entry->key.peer);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO unbind function
 *
 * \details
 *
 * When a datareader no longer matches with a datawriter the bind entry
 * for the datawriter is removed. No further communication with the datawriter
 * is possible.
 *
 * \param[in]  netio    NETIO interface to bind
 * \param[in]  src_addr The address to bind to
 * \param[in]  dst_intf Interface
 * \param[out] existed  Whether a bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref DDS_DataReaderInterface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_unbind(NETIO_Interface_T *netio,
                        struct NETIO_Address *src_addr,
                        NETIO_Interface_T *dst_intf,
                        RTI_BOOL *existed)
{
    struct DDS_DataReaderInterface *drintf =
                                    (struct DDS_DataReaderInterface *)netio;
    struct DataReaderBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DataReaderBindEntryKey bind_key;
    DDS_InstanceHandle_t ih_peer;

    UNUSED_ARG(dst_intf);

    bind_key.peer = src_addr->value.guid;

    OSAPI_TRACE_DDS("datareader unbinding from datawriter:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drintf->datareader->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&bind_key.peer,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_TRUE)

    dbrc = DB_Table_select_match(drintf->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T *)&bind_entry,
                                &bind_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_DDS("datareader unbinding from datawriter, no entry exsists:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            drintf->datareader->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&bind_key.peer,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_TRUE)

        if (existed)
        {
            *existed = RTI_FALSE;
        }
        return RTI_TRUE;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_DDS("datareader unbinding from datawriter failed:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            drintf->datareader->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&bind_key.peer,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc",dbrc,RTI_TRUE);

        DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)

        return RTI_FALSE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    if (bind_entry->writer_state == REMOTE_WRITERSTATE_ALIVE)
    {
        DDS_DataReaderEvent_on_remote_writer_deleted(
                        drintf->datareader,&bind_key.peer,RTI_TRUE);
    }
    else
    {
        DDS_DataReaderEvent_on_remote_writer_deleted(
                        drintf->datareader,&bind_key.peer,RTI_FALSE);
    }

    DDS_InstanceHandle_from_netio_address(&ih_peer,src_addr);

    if (!DDS_TypePlugin_remove_peer(drintf->datareader->type_plugin,&ih_peer))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(drintf->_parent._btable,
                                 (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_DDS("datareader unbinding from datawriter failed:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            drintf->datareader->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&bind_key.peer,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc",dbrc,RTI_TRUE);

        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                               DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    OSAPI_TRACE_DDS("datareader unbound from datawriter:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        drintf->datareader->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&bind_key.peer,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&src_addr->value.rtps_guid,RTI_TRUE)

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_get_external_interface
 *
 * \details
 *
 * When a datareader is bound to a downstream interface it is requested to
 * provide which interface and address the downstream interface should use
 * when forwarding a NETIO_Packet.
 *
 * \param[in]   netio_intf   NETIO interface to get the external interface for
 * \param[in]   src_addr     The address to send to
 * \param[out]  dst_intf     The interface to use
 * \param[out]  dst_addr     The destination address to use
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_get_external_interface(NETIO_Interface_T *netio_intf,
                                               struct NETIO_Address *src_addr,
                                               NETIO_Interface_T **dst_intf,
                                               struct NETIO_Address *dst_addr)
{
    struct DDS_DataReaderInterface *drio =
                                   (struct DDS_DataReaderInterface *)netio_intf;
    UNUSED_ARG(src_addr);

    *dst_intf = netio_intf;

    *dst_addr = drio->_parent.local_address;

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_set_state function
 *
 * \details
 * The datareader interface does not start to monitor the state of remote
 * peers until it is enabled. Enabling an already enabled interface is a
 * no-op. It is not possible to disable an interface.
 *
 * \param[in] netio NETIO interface to set state on
 * \param[in] state New state
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReaderInterface_set_state(NETIO_Interface_T *netio,
                                  NETIO_InterfaceState_T state)
{

    struct DDS_DataReaderInterface *drio =
                                        (struct DDS_DataReaderInterface*)netio;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;

    if ((drio->_parent.state != NETIO_INTERFACESTATE_CREATED) ||
         state != NETIO_INTERFACESTATE_ENABLED)
    {
        return RTI_TRUE;
    }

    /* start liveliness timer */
    storage.field[0] = (void *)drio;
    if (!DDS_Duration_is_infinite(
                    &drio->datareader->liveliness.lease_duration))
    {
        if (!OSAPI_Timer_create_timeout(drio->datareader->config->timer,
          &drio->datareader->liveliness_event,
          drio->datareader->liveliness.lease_duration.sec,
          (RTI_INT32)drio->datareader->liveliness.lease_duration.nanosec,
          OSAPI_TIMER_PERIODIC,
          DDS_DataReaderInterface_on_liveliness_timeout,
          &storage))
        {
            return RTI_FALSE;
        }
    }

    drio->_parent.state = NETIO_INTERFACESTATE_ENABLED;

    return RTI_TRUE;
}

/*ci
 * \brief The Datareader NETIO interface implementation
 *
 * \details
 *
 * The Datareader NETIO interface communicates either with RTPS or loopback.
 * It is always on top of the stack and does not forward any data.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI DDS_DataReaderInterface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    NULL, /* send, not needed */
    NULL, /* ack */
    NULL, /* request */
    NULL, /* return_loan */
    NULL, /* discard */
    DDS_DataReaderInterface_add_route,
    DDS_DataReaderInterface_delete_route,
    NULL, /* reserve_address */
    DDS_DataReaderInterface_bind,
    DDS_DataReaderInterface_unbind,
    DDS_DataReaderInterface_receive,
    DDS_DataReaderInterface_get_external_interface,
    NULL,
    NULL,
    DDS_DataReaderInterface_set_state,
    NULL, /* release_address */
    NULL, /* resolve_address */
    NULL, /* get_route_table */
    NULL,  /* post_event */
    NULL,
    NULL,
    NULL
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */
/*ci
 * \brief Creates a new datareader interface.
 *
 * \details
 * Implementation of the RT ComponentFactory create_component method. This
 * method is never called directly, only via a factory interface type.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 *
 * \sa \ref DDS_DataReaderInterfaceFactory_delete_component
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
DDS_DataReaderInterfaceFactory_create_component(
                                        struct RT_ComponentFactory *factory,
                                        struct RT_ComponentProperty *property,
                                        struct RT_ComponentListener *listener)
{
    struct DDS_DataReaderInterface *retval = NULL;

    retval = DDS_DataReaderInterface_create(
            (struct DDS_DataReaderInterfaceFactory*)factory,
            (const struct DDS_DataReaderInterfaceProperty*)property,
            (const struct NETIO_InterfaceListener*)listener);

    if (retval != NULL)
    {
        return &retval->_parent._parent;
    }
    else
    {
        return NULL;
    }
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a datareader interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This method deletes
 * a datareader interface. It is never called directly,only via a factory
 * interface type.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref DDS_DataReaderInterfaceFactory_create_component
 */
RTI_PRIVATE void
DDS_DataReaderInterfaceFactory_delete_component(
                                        struct RT_ComponentFactory *factory,
                                        RT_Component_T *component)
{
    struct DDS_DataReaderInterface *self =
                                    (struct DDS_DataReaderInterface*)component;
    UNUSED_ARG(factory);

    DDS_DataReaderInterface_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_DataReaderInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty*property,
        struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
DDS_DataReaderInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI DDS_DataReaderInterfaceFactory_fv_Intf =
{
    DDSRI_INTERFACE_INTERFACE_ID,
    DDS_DataReaderInterfaceFactory_initialize,
    DDS_DataReaderInterfaceFactory_finalize,
    DDS_DataReaderInterfaceFactory_create_component,
    DDS_DataReaderInterfaceFactory_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI DDS_DataReaderInterfaceFactory_fv_Intf =
{
    DDSRI_INTERFACE_INTERFACE_ID,
    DDS_DataReaderInterfaceFactory_initialize,
    NULL, /* DDS_DataReaderInterfaceFactory_finalize, */
    DDS_DataReaderInterfaceFactory_create_component,
    NULL, /* DDS_DataReaderInterfaceFactory_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief Datareader NETIO interface factory
 *
 * \details
 * The Datareader NETIO interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct DDS_DataReaderInterfaceFactory DDS_DataReaderInterfaceFactory_fv_Factory =
{
  {
     &DDS_DataReaderInterfaceFactory_fv_Intf,
     NULL,
     {{{0,0}}}
  }
};

/*ci
 * \brief Initialize the datareader NETIO interface factory
 *
 * \details
 * The Datareader NETIO specific implementation of the RT ComponentFactory
 * initialize method. This method is called by RT when the factory is
 * registered.
 *
 * \param[in] property The properties registered with the Datareader NETIO
 *                     interface factory
 * \param[in] listener The listener registered with the Datareader NETIO
 *                     interface factory
 *
 * \return A fully initialized factory
 *
 * \sa \ref DDS_DataReaderInterfaceFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_DataReaderInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    struct DDS_DataReaderInterfaceFactory *factory =
                                    &DDS_DataReaderInterfaceFactory_fv_Factory;
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    DDS_DataReaderInterfaceFactory_fv_Factory._parent._factory = &factory->_parent;

    return &DDS_DataReaderInterfaceFactory_fv_Factory._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the datareader NETIO interface factory
 *
 * \details
 * The Datareader NETIO specific implementation of the RT ComponentFactory
 * finalize method. This method is called by RT when the factory is
 * unregistered.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref DDS_DataReaderInterfaceFactory_initialize
 */
RTI_PRIVATE void
DDS_DataReaderInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
    UNUSED_ARG(factory);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
DDS_DataReaderInterfaceFactory_get_interface(void)
{
    return &DDS_DataReaderInterfaceFactory_fv_Intf;
}

/*ci @} */
