/*
 * FILE: NotificationInterface.c - Notification Interface implementation
 *
 * Copyright (c) 2023-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \file
 * \brief Notification Interface implementation
 *
 * \addtogroup ZCOPY_NotifInterfaceClass
 * @{
 */
#include "NotificationInterface.h"
#include "Notifiee.h"
#include "netio_zcopy/netio_zcopy_log.h"
#include "netio/netio_sample_access.h"
#include "osapi/osapi_string.h"
#include "netio_zcopy/netio_zcopy.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "reda/reda_indexer.h"


RTI_PRIVATE struct NETIO_InterfaceI ZCOPY_NotifInterface_fv_Intf;

const char* const NETIO_DEFAULT_NOTIF_NAME = "notif";

/*ci
 * \brief Initialize a NETIO_Address from a ZCOPY_Guid
 *
 * \param[out] addr_ The NETIO_Address
 * \param[in]  guid_ The ZCOPY_Guid
 */
#define NETIO_Address_set_from_notif_guid(addr_, guid_)                             \
    OSAPI_Memory_copy(&(addr_)->value, &(guid_)->value, sizeof(struct ZCOPY_Guid)); \
    (addr_)->kind = NETIO_ADDRESS_KIND_NOTIF;                                       \
    (addr_)->port = 0

/*ci
 * \brief Initialize a ZCOPY_Guid from a NETIO_Address containing an RTPS_Guid
 *
 * \param[out] guid_ The ZCOPY_Guid
 * \param[in]  addr_ The NETIO_Address
 */
#define ZCOPY_Guid_set_from_netio_addr(guid_, addr_)                         \
    OSAPI_Memory_copy(&(guid_)->value, &(addr_)->value, sizeof(struct ZCOPY_Guid))


/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compares two notification interface routes. Implements DB_IndexCompare_T.
 *
 * \param[in] flags Unused.
 * \param[in] op1 The first route to compare.
 * \param[in] op2 The second route to compare.
 *
 * \return A value less than zero if op1 is less than op2, zero if op1 is equal to op2,
 *         or a value greater than zero if op1 is greater than op2.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
ZCOPY_NotifInterface_compare_route(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    RTI_INT32 diff;
    const struct NETIORouteEntryKey *lkey = (const struct NETIORouteEntryKey *)op1;
    const struct NETIORouteEntryKey *rkey = (const struct NETIORouteEntryKey *)op2;

    UNUSED_ARG(flags);

    diff = NETIO_Address_compare(&lkey->intf_address, &rkey->intf_address);
    if (diff != 0)
    {
        return diff;
    }

    if (lkey->intf > rkey->intf)
    {
        return 1;
    }

    if (lkey->intf < rkey->intf)
    {
        return -1;
    }

    return NETIO_Address_compare(&lkey->destination, &rkey->destination);
}

/*ci
 * \brief Compare two notification interface ports. Implements DB_IndexCompare_T.
 *
 * \param[in] flags Unused.
 * \param[in] op1 The first port to compare.
 * \param[in] op2 The second port to compare.
 *
 * \return A value less than zero if op1 is less than op2, zero if op1 is equal to op2,
 *         or a value greater than zero if op1 is greater than op2.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
ZCOPY_NotifInterface_compare_port(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    const struct ZCOPY_NotifInterfacePortEntry *lkey =
            (const struct ZCOPY_NotifInterfacePortEntry *)op1;
    const struct ZCOPY_NotifInterfacePortEntry *rkey =
            (const struct ZCOPY_NotifInterfacePortEntry *)op2;

    UNUSED_ARG(flags);

    return NETIO_Address_compare(&lkey->address, &rkey->address);
}

/*ci
 * \brief Iterator callback function for visiting unseen samples in shared queue.
 *
 * \param[in] sample_id The ID of the visited sample.
 * \param[in] sample_info The sample info associated with the visited sample.
 * \param[in] visit_data The visit data to track the visited sample.
 *
 * \return RTI_TRUE if iteration should continue, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_on_sample_visit(
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr sample_seq_nr,
        const struct SQ_SampleInfo *sample_info,
        void *visit_data)
{
    RTI_BOOL result = RTI_FALSE;
    struct ZCOPY_NotifInterfaceSampleVisit *sample_visit_data =
            (struct ZCOPY_NotifInterfaceSampleVisit *)visit_data;
    struct NETIO_PacketInfo *pinfo = sample_visit_data->info;

    if (sample_visit_data->is_dirty)
    {
        /* Only want to see a single sample so break off loop */
        sample_visit_data->are_more_samples = RTI_TRUE;
        goto done;
    }

    sample_visit_data->is_dirty = RTI_TRUE;

    /* Fill with the desired info */

    /* Instance handle associated with the sample */
    OSAPI_Memory_copy(&pinfo->instance, sample_info->key.value, 16);
    pinfo->valid_key = 1;

    /* Source timestamp */
    pinfo->timestamp = sample_info->timestamp;

    /* RTPS flag for the kind of update. */
    if (SQ_SAMPLE_KIND_WRITE == sample_info->kind)
    {
        pinfo->valid_data = 1;
        pinfo->rtps_flags = NETIO_RTPS_FLAGS_DATA;
    }

    if (sample_info->kind & SQ_SAMPLE_KIND_DISPOSE)
    {
        pinfo->valid_data = 0;
        pinfo->rtps_flags |= NETIO_RTPS_FLAGS_DISPOSE;
    }
    if (sample_info->kind & SQ_SAMPLE_KIND_UNREGISTER)
    {
        pinfo->valid_data = 0;
        pinfo->rtps_flags |= NETIO_RTPS_FLAGS_UNREGISTER;

    }
    if (sample_info->kind & SQ_SAMPLE_KIND_PULSE)
    {
        pinfo->valid_key = 0;
        pinfo->valid_data = 0;
        pinfo->rtps_flags = NETIO_RTPS_FLAGS_LIVELINESS;
    }

    /* Sequence number */
    pinfo->sn.high = sample_info->seq_nr.high;
    pinfo->sn.low = sample_info->seq_nr.low;
    /* The protocol dictates that committable is everything up to last SN + 1 */
    pinfo->committable_sn = pinfo->sn;
    REDA_SequenceNumber_plusplus(&pinfo->committable_sn);

    /* Virtual number is only used for batching, which is not supported by ZC */
    REDA_SequenceNumber_set_zero(&pinfo->virtual_sn);

    /* Notif-specific information: sample id */
    pinfo->protocol_data.notif_info.opaque_data.opaque_id_32 = sample_id;
    pinfo->protocol_data.notif_info.opaque_data.opaque_id_64 = sample_seq_nr;

    pinfo->encapsulation = DDS_ENCAPSULATION_ID_SHMEM_V2;
    result = RTI_TRUE;

done:
    return result;
}

/*ci
 * \brief Loans the data from the shared queue for a specified sample.
 *
 * \param[in] history_user_data The sample info associated with the sample.
 * \param[out] retrieved_data A pointer to the retrieved data.
 *
 * \return RTI_TRUE if the loaned data was successfully retrieved, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_retrieve_data_func(
        const void *const history_user_data,
        void **retrieved_data)
{
    const struct NETIO_OpaqueInfo *const notif =
            (const struct NETIO_OpaqueInfo *const)history_user_data;
    struct ZCOPY_NotifInterfaceSqRecordEntry *reader =
            (struct ZCOPY_NotifInterfaceSqRecordEntry *)(notif->reader);
    SQ_Length user_data_size;

    if (!NETIO_ZCOPY_SharedQReader_get_loan(
                ((struct ZCOPY_NotifInterfaceSqRecordEntry *)reader)->sq_reader,
                notif->opaque_id_32,
                notif->opaque_id_64,
                &user_data_size,
                (SQ_MemPtr *)retrieved_data))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Returns the loaned data to the shared queue.
 *
 * \param[in] user_data The sample info for the sample to return.
 * \param[in] retrieved_data Not used
 *
 * \return RTI_TRUE if the loaned data was successfully returned, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_return_data_func(
        const void *const user_data,
        const void *retrieved_data)
{
    const struct NETIO_OpaqueInfo *const notif =
            (const struct NETIO_OpaqueInfo *const)user_data;
    struct ZCOPY_NotifInterfaceSqRecordEntry *reader =
            (struct ZCOPY_NotifInterfaceSqRecordEntry *)(notif->reader);
    UNUSED_ARG(retrieved_data);

    if (!NETIO_ZCOPY_SharedQReader_return_loan(
                ((struct ZCOPY_NotifInterfaceSqRecordEntry *)reader)->sq_reader,
                notif->opaque_id_32))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

/*ci
 * \brief Increases the sq readers sample ref count
 *
 * \param[in] user_data  The sq reader entry
 *
 * \return RTI_TRUE on success
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_increment_ref_count_func(
        const void *sq_reader_entry)
{
    struct ZCOPY_NotifInterfaceSqRecordEntry *reader =
            (struct ZCOPY_NotifInterfaceSqRecordEntry *)(sq_reader_entry);

    OSAPI_PRECONDITION((reader->ref_count == LLONG_MAX),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("reader", reader, RTI_TRUE);)

        reader->ref_count++;

    return RTI_TRUE;
}

/*ci
 * \brief Decreases the sq readers sample ref count
 *
 * \param[in] user_data  The sq reader entry
 *
 * \return RTI_TRUE on success
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_decrement_ref_count_func(
        const void *sq_reader_entry)
{
    struct ZCOPY_NotifInterfaceSqRecordEntry *reader =
        (struct ZCOPY_NotifInterfaceSqRecordEntry *)(((struct NETIO_OpaqueInfo *)sq_reader_entry)->reader);

    OSAPI_PRECONDITION((reader->ref_count == (-LLONG_MAX-1LL)),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("reader", reader, RTI_TRUE);)

    /* Decrement can be called before increment resulting in negitive values.
     * ref_count is signed in order to account for that situation.
     */
    reader->ref_count--;

    return RTI_TRUE;
}

/*ci
 * \brief Initialization of the sample accessor interface.
 */
RTI_PRIVATE NETIO_SampleI ZCOPY_NotifInterface_fv_sample_accesor =
{
    ZCOPY_NotifInterface_retrieve_data_func,
    ZCOPY_NotifInterface_return_data_func,
    ZCOPY_NotifInterface_decrement_ref_count_func,
    ZCOPY_NotifInterface_increment_ref_count_func
};

/*ci
 * \brief Peeks at the latest unseen sample in the shared queue and retrieves its packet
 * info.
 *
 * \param[in] sq_reader The shared memory queue reader to peek at.
 * \param[out] info_out The packet info for the latest unseen sample.
 * \param[out] has_more_data A flag indicating whether there are more unseen samples.
 *
 * \return RTI_TRUE if the packet info was successfully retrieved, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_peek_info(
        NETIO_ZCOPY_SharedQReader *sq_reader,
        struct NETIO_PacketInfo *info_out,
        RTI_BOOL *has_more_data)
{
    RTI_BOOL result = RTI_FALSE;
    struct ZCOPY_NotifInterfaceSampleVisit sample_visit_data;
    sample_visit_data.are_more_samples = RTI_FALSE;
    sample_visit_data.is_dirty = RTI_FALSE;
    sample_visit_data.info = info_out;

    /* Get the info for the latest unseen sample */
    if (!NETIO_ZCOPY_SharedQReader_peek_unseen(
                sq_reader,
                ZCOPY_NotifInterface_on_sample_visit,
                &sample_visit_data))
    {
        goto done;
    }
    *has_more_data = sample_visit_data.are_more_samples;

    if (!sample_visit_data.is_dirty)
    {
        /* Not a bug, just no unseen samples available */
        goto done;
    }
    result = RTI_TRUE;

done:
    return result;
}

/*ci
 * \brief Create a port entry for the notification interface to receive data on
 *
 * \details
 * This function is expected to fail if another interface on the same machine
 * has already bound to the same port.
 *
 * \param[in] self  The notification interface creating the port entry
 * \param[in] port  The port to create the entry for
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_create_port_entry(
        struct ZCOPY_NotifInterface *self,
        struct NETIO_Address *port)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifInterfacePortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct ZCOPY_Guid notif_locator;
    RTI_BOOL user_reserved = RTI_FALSE;
    RTI_BOOL result;

    dbrc = DB_Table_select_match(
            self->port_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&port_entry,
            (DB_Key_T)port);
    if (dbrc == DB_RETCODE_OK)
    {
        ok = RTI_TRUE;
        goto done;
    }
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    port_entry = NULL;
    dbrc = DB_Table_create_record(self->port_table, (DB_Record_T *)&port_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_CREATE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    port_entry->address = *port;
    port_entry->_ref_count = 0;
    port_entry->_notif_intf = self;
    port_entry->_notifiee = NULL;
     /* Initialize as not scheduled so it can be scheduled */
    port_entry->_timeout_scheduled = RTI_FALSE;
    port_entry->_writer_match_timeout = (OSAPI_TimeoutHandle_T)OSAPI_TimeoutHandle_INITIALIZER;

    ZCOPY_Guid_set_from_netio_addr(&notif_locator, port);
    port_entry->_notifiee = ZCOPY_Notifiee_create(
            &notif_locator,
            port->port,
            ((RTI_UINT32)self->property.num_remote_writers));
    if (port_entry->_notifiee == NULL)
    {
        /* This is an expected error if this port has already been bound to */
        goto done;
    }

    if (!ZCOPY_NotifUserInterface_reserve_address(self, port, &port_entry->_user_entry))
    {
        /* Unexpected error. Should have succeeded because the Notifiee was successfully
         * created */
        ZCOPY_LOG_NOTIF_MECHANISM_RESERVE_ADDR(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    user_reserved = RTI_TRUE;

    dbrc = DB_Table_insert_record(self->port_table, (DB_Record_T)port_entry);
    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_EXISTS))
    {
        ZCOPY_LOG_NOTIF_DB_INSERT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
    if ((ok == RTI_FALSE) && (port_entry != NULL))
    {
        if (user_reserved)
        {
            result = ZCOPY_NotifUserInterface_release_address(self, port, port_entry->_user_entry);
#if OSAPI_ENABLE_LOG
            if (!result)
            {
                ZCOPY_LOG_NOTIF_MECHANISM_RELEASE_ADDR(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(result);
#endif
        }
        if (port_entry->_notifiee != NULL)
        {
            result = ZCOPY_Notifiee_destroy(port_entry->_notifiee);
#if OSAPI_ENABLE_LOG
            if (!result)
            {
                ZCOPY_LOG_NOTIF_NOTIFIEE_DESTROY(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(result);
#endif
        }
        dbrc = DB_Table_delete_record(self->port_table, (DB_Record_T)port_entry);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }
    return ok;
}

#ifndef RTI_CERT
/*ci
 * \brief Close all the unbound sq readers from the indexer
 *
 * \details Called during the finalization of the interface
 *           Iterates over the unbound_index and
 *           closes them regardless of the ref count.
 */
RTI_PRIVATE void
ZCOPY_NotifInterface_close_unbound_sq_reader(REDA_BufferPool_T pool,
                                    struct REDA_Indexer *indexer)
{
    REDA_IndexIterator_T *it = NULL;
    struct ZCOPY_NotifInterfaceSqRecordEntry *record = NULL;
    void* obj_val = NULL;
    RTI_BOOL retval = RTI_FALSE;

    it = REDA_Indexer_iterator_begin(indexer);
    if (it == NULL)
    {
        return;
    }

    record = (struct ZCOPY_NotifInterfaceSqRecordEntry *)REDA_Indexer_iterator_next(it);
    while (record != NULL)
    {
        obj_val = REDA_Indexer_remove_entry(indexer,record->sq_reader);
        IGNORE_RETVAL(obj_val);
        retval = NETIO_ZCOPY_SharedQReader_close(record->sq_reader);
#if OSAPI_ENABLE_LOG
        if (retval == RTI_FALSE)
        {
            ZCOPY_LOG_NOTIF_SQ_CLOSE(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        if (record->sq_reader != NULL)
        {
            REDA_BufferPool_return_buffer(pool, record);
        }
        record = (struct ZCOPY_NotifInterfaceSqRecordEntry *)REDA_Indexer_iterator_next(it);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize a shared queue reader
 *
 * \param[in] initialize_param Unused
 * \param[in] buffer A pointer to the memory to be initialized
 *
 * \return RTI_TRUE if the shared queue reader was successfully initialized,
 *   RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_initialize_sq_reader_records(void *initialize_param, void *buffer)
{
    struct ZCOPY_NotifInterfaceSqRecordEntry *entry =
                            (struct ZCOPY_NotifInterfaceSqRecordEntry *)buffer;

    UNUSED_ARG(initialize_param);

    OSAPI_Heap_allocate_buffer((char**)&entry->sq_reader,
                               NETIO_ZCOPY_SharedQReader_get_size(),
                               OSAPI_ALIGNMENT_DEFAULT);
    if (entry->sq_reader == NULL)
    {
        return RTI_FALSE;
    }

    if (!NETIO_ZCOPY_SharedQReader_initialize(entry->sq_reader,
                                    OSAPI_SharedMemory_is_robust_mutex_supported() ? SQ_CSTY_MODE_ROBUST : SQ_CSTY_MODE_STRONG))
    {
        return RTI_FALSE;
    }

    entry->ref_count = 0;

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a shared queue reader
 *
 * \param[in] finalize_param Unused
 * \param[in] buffer A pointer to the memory containing the shared queue reader
 *
 * \return RTI_TRUE if the shared queue reader was successfully finalized, RTI_FALSE
 * otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_finalize_sq_reader_records(void *finalize_param, void *buffer)
{
    struct ZCOPY_NotifInterfaceSqRecordEntry *reader =
                            (struct ZCOPY_NotifInterfaceSqRecordEntry *)buffer;

    UNUSED_ARG(finalize_param);

    if (!NETIO_ZCOPY_SharedQReader_finalize(reader->sq_reader))
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free_buffer(reader->sq_reader);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize a notifier
 *
 * \param[in] initialize_param Unused
 * \param[in] buffer A pointer to the memory to be initialized
 *
 * \return RTI_TRUE if the notifier was successfully initialized, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_initialize_notifier(void *initialize_param, void *buffer)
{
    struct ZCOPY_Notifier *notifier = (struct ZCOPY_Notifier *)buffer;

    UNUSED_ARG(initialize_param);

    if (!ZCOPY_Notifier_initialize(notifier))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a notifier
 *
 * \param[in] finalize_param Unused
 * \param[in] buffer A pointer to the memory containing the notifier
 *
 * \return RTI_TRUE if the notifier was successfully finalized, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_finalize_notifier(void *finalize_param, void *buffer)
{
    struct ZCOPY_Notifier *notifier = (struct ZCOPY_Notifier *)buffer;

    UNUSED_ARG(finalize_param);

    if (!ZCOPY_Notifier_finalize(notifier))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Compares two bind entries
 *
 * \param[in] flags Unused
 * \param[in] op1 The first bind entry
 * \param[in] op2 The second bind entry
 *
 * \return A negative value if op1 < op2, a positive value if op1 > op2, 0 if op1 == op2.
 */
RTI_PRIVATE RTI_INT32
ZCOPY_NotifInterface_bind_compare(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    RTI_INT32 diff;
    const struct ZCOPY_NotifInterfaceBindEntryKey *lkey =
            (const struct ZCOPY_NotifInterfaceBindEntryKey *)op1;
    const struct ZCOPY_NotifInterfaceBindEntryKey *rkey =
            (const struct ZCOPY_NotifInterfaceBindEntryKey *)op2;
    const NETIO_Interface_T *left_intf = lkey->interface;
    const NETIO_Interface_T *right_intf = rkey->interface;

    UNUSED_ARG(flags);

    diff = NETIO_Address_compare(&lkey->source, &rkey->source);
    if (diff != 0)
    {
        return diff;
    }

    if (right_intf > left_intf)
    {
        return -1;
    }

    if (left_intf > right_intf)
    {
        return 1;
    }

    return 0;
}

/*ci
 * \brief Compare two SharedQ Readers
 *
 * @param[in] record    The record to compare against
 * @param[in] is_record Not used
 * @param[in] key       The key/record to compare with
 *
 * \return return 0 if the record and key is equal
 *         return positive value if the record is larger then the key
 *         return negative value if the record is smaller then the key
 */
RTI_PRIVATE RTI_INT32
ZCOPY_NotifInterface_record_compare(const void *const record,
                                  RTI_BOOL key_is_record,
                                  const void *const key)
{
    const struct ZCOPY_NotifInterfaceSqRecordEntry *lval =
            (const struct ZCOPY_NotifInterfaceSqRecordEntry *)record;
    const NETIO_ZCOPY_SharedQReader *rval;

    if (key_is_record)
    {
        rval = ((const struct ZCOPY_NotifInterfaceSqRecordEntry *)key)->sq_reader;
    }
    else
    {
        rval = (const NETIO_ZCOPY_SharedQReader *)key;
    }

    if (lval->sq_reader > rval)
    {
        return 1;
    }
    else if (lval->sq_reader < rval)
    {
        return -1;
    }

    return 0;
}

/*ci
 * \brief Timeout handler to detect a remote writer match.
 *
 * \details When a Remote writer is matched we notify the receive port where
 *          this writer has attached. This is a preemptive notification to the
 *          reader to allow it to catch up to the DataWriter.
 *
 * \param[in] storage The User data provided to the create_timeout function.
 *
 * \return Always returns OSAPI_TIMEOUT_OP_MANUAL indicating that the timer
 *         is not to be rescheduled.
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
ZCOPY_NotifInterface_writer_match_event(struct OSAPI_TimeoutUserData *storage)
{
    RTI_BOOL retcode = RTI_FALSE;
    struct ZCOPY_NotifInterfacePortEntry *port_entry =
            (struct ZCOPY_NotifInterfacePortEntry *)storage->field[0];
    struct ZCOPY_NotifInterface *notif_intf = port_entry->_notif_intf;
    port_entry->_timeout_scheduled = RTI_FALSE;
    retcode = ZCOPY_NotifUserInterface_notify_receive_port(notif_intf, port_entry->_user_entry);

#if OSAPI_ENABLE_LOG
    if (retcode == RTI_FALSE)
    {
        ZCOPY_LOG_NOTIF_RECEIVE_ON_EVENT(OSAPI_LOGKIND_WARNING)
    }
#else
    IGNORE_RETVAL(retcode);
#endif
    return OSAPI_TIMEOUT_OP_MANUAL;
}

/*ci
 * \brief Helper function to create a timeout
 *
 * \details When a Remote writer is matched we create a timeout to to preemptively
 *          notify the reader of past samples. We only create a timeout if a
 *          previous timeout is not already scheduled. We have limited number of
 *          timeouts, so an expired timeout is always deleted before recreating it.
 *
 * \param[in] timer Timer to create timeout on
 * \param[in] port_entry The port entry for which timeout is created
 *
 * \return return RTI_TRUE for success and RTI_FALSE for failure.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_create_timeout(OSAPI_Timer_T timer,
                                    struct ZCOPY_NotifInterfacePortEntry* port_entry)
{
    RTI_BOOL retval = RTI_FALSE;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    if (!port_entry->_timeout_scheduled)
    {
        port_entry->_timeout_scheduled = RTI_TRUE;
        storage.field[0] = port_entry; /* port entry pointer from the buffer pool */

        /* We call timeout handle_is_valid to ensure that we not calling delete (below)
         * on an invalid timeout which will happen the first time the timeout is created
         */
        if (OSAPI_Timer_handle_is_valid(&port_entry->_writer_match_timeout))
        {
            /* We have a limited amount of timeouts (equal to the number of port entries)
             * When a timeout event is called the timeout is not put back into the free list
             * Calling delete below adds the timeout back to the free list so it can be used again
             */
            if (!OSAPI_Timer_delete_timeout(timer,
                        &port_entry->_writer_match_timeout))
            {
                port_entry->_timeout_scheduled = RTI_FALSE;
                ZCOPY_LOG_NOTIF_TIMEOUT_DELETE(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }

        /* The timeout is scheduled for the shortest possible time (1ns)
         * so that it will occur during the next timer tick.
         */
        if (!OSAPI_Timer_create_timeout(timer,
                    &port_entry->_writer_match_timeout,
                    0,
                    1,
                    OSAPI_TIMER_ONE_SHOT,
                    ZCOPY_NotifInterface_writer_match_event,
                    &storage))
        {
            port_entry->_timeout_scheduled = RTI_FALSE;
            ZCOPY_LOG_NOTIF_TIMEOUT_CREATE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }
    retval = RTI_TRUE;
done:
    return retval;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a notification interface instance with or without a lock
 *
 * \param[in] self      The notification interface instance to finalize
 * \param[in] have_lock Whether the caller has the database lock
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_finalize(struct ZCOPY_NotifInterface *self, RTI_BOOL have_lock)
{
    DB_ReturnCode_T dbrc;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (self->user_netio != NULL)
    {
        ZCOPY_NotifUserInterface_delete_instance(self, self->user_netio);
        self->user_netio = NULL;
    }

    if (self->port_table != NULL)
    {
        dbrc = DB_Database_delete_table(self->property._parent._parent.db, self->port_table);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_TABLE_DELETE(OSAPI_LOGKIND_ERROR, dbrc)
            goto done;
        }
        self->port_table = NULL;
    }

    if (self->property._parent._parent.timer != NULL)
    {
        if (have_lock)
        {
            /* Release the lock associated with the timer
             * so that the timer can be deleted
             */
            dbrc = DB_Database_unlock(self->property._parent._parent.db);
            if (dbrc != DB_RETCODE_OK)
            {
                ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
                goto done;
            }
        }

        OSAPI_Timer_delete(self->property._parent._parent.timer);

        if (have_lock)
        {
            /* Reacquire the lock */
            dbrc = DB_Database_lock(self->property._parent._parent.db);
            if (dbrc != DB_RETCODE_OK)
            {
                ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
                goto done;
            }
        }
    }

    if (self->notifier_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->notifier_pool))
        {
            goto done;
        }
        self->notifier_pool = NULL;
    }

    if (self->_parent._rtable != NULL)
    {
        dbrc = DB_Database_delete_table(
                self->property._parent._parent.db,
                self->_parent._rtable);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_TABLE_DELETE(OSAPI_LOGKIND_ERROR, dbrc)
            goto done;
        }
        self->_parent._rtable = NULL;
    }


    if (self->sq_reader_pool != NULL)
    {
        if (self->unbound_index != NULL)
        {
            ZCOPY_NotifInterface_close_unbound_sq_reader(self->sq_reader_pool,
                                                       self->unbound_index);
            if (!REDA_Indexer_delete(self->unbound_index))
            {
                goto done;
            }
        }

        if (!REDA_BufferPool_delete(self->sq_reader_pool))
        {
            goto done;
        }
        self->sq_reader_pool = NULL;
    }

    if (self->_parent._btable != NULL)
    {
        dbrc = DB_Database_delete_table(
                self->property._parent._parent.db,
                self->_parent._btable);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_TABLE_DELETE(OSAPI_LOGKIND_ERROR, dbrc)
            goto done;
        }
        self->_parent._btable = NULL;
    }

    if (self->reliable_table != NULL)
    {
        dbrc = DB_Database_delete_table(
                self->property._parent._parent.db,
                self->reliable_table);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_TABLE_DELETE(OSAPI_LOGKIND_ERROR, dbrc)
            goto done;
        }
        self->reliable_table = NULL;
    }

    if (self->user_netio != NULL)
    {
        if (!NETIO_Interface_finalize(self->user_netio))
        {
            goto done;
        }
    }

    if (!NETIO_Interface_finalize(&self->_parent))
    {
        goto done;
    }

    self->factory = NULL;

    ok = RTI_TRUE;

done:
    return ok;
}
#endif /* !RTI_CERT */

RTI_PRIVATE RTI_INT32
ZCOPY_NotifInterface_route_key_compare(RTI_INT32 flags,
                                        const DB_Record_T op1,
                                        void *op2)
{
    const struct ZCOPY_NotifInterfaceRouteKeyEntry *lval =
            (const struct ZCOPY_NotifInterfaceRouteKeyEntry *)op1;
    const struct ZCOPY_NotifInterfaceRouteKeyEntry *rval =
            (const struct ZCOPY_NotifInterfaceRouteKeyEntry *)op2;

    UNUSED_ARG(flags);

    /* Compare the route_ptr fields first */
    if (lval->route_ptr > rval->route_ptr)
    {
        return 1;
    }
    else if (lval->route_ptr < rval->route_ptr)
    {
        return -1;
    }

    /* The key fields */
    return OSAPI_Memory_compare(&lval->key, &rval->key, sizeof(struct NETIO_Guid));
}

/*ci
 * \brief Initialize a new Notification interface instance
 *
 * \param[in] self      The new Notification interface instance to initialize
 * \param[in] factory   Factory initialing the new Notification interface
 * \param[in] property  The property to use to initialize
 * \param[in] listener  The interface listener
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_initialize(
        struct ZCOPY_NotifInterface *self,
        struct ZCOPY_NotifInterfaceFactory *factory,
        const struct ZCOPY_NotifInterfaceProperty *const property,
        const struct NETIO_InterfaceListener *const listener)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;
    struct REDA_BufferPoolProperty pool_property = REDA_BufferPoolProperty_INITIALIZER;
    RTI_BOOL ok = RTI_FALSE;
    struct OSAPI_TimerProperty timer_property = OSAPI_TimerProperty_INITIALIZER;
    struct REDA_IndexerProperty ip = REDA_IndexerProperty_INITIALIZER;

    OSAPI_PRECONDITION((self == NULL) || (factory == NULL) || (property == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    /* Validate property */
    if (property->num_remote_writers <= 0)
    {
        ZCOPY_LOG_NOTIF_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!NETIO_Interface_initialize(
                &self->_parent,
                &ZCOPY_NotifInterface_fv_Intf,
                &property->_parent,
                listener))
    {
        goto done;
    }

    self->property = *property;
    self->factory = factory;

    self->user_netio = ZCOPY_NotifUserInterface_create_instance(self);
    if (self->user_netio == NULL)
    {
        ZCOPY_LOG_NOTIF_MECHANISM_CREATE_INST(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    self->_user_netio_intf.resolve_address = factory->property.user_intf->resolve_address;
    self->_user_netio_intf.get_route_table = factory->property.user_intf->get_route_table;
    if (!NETIO_Interface_initialize(
                self->user_netio,
                &self->_user_netio_intf,
                NULL,
                NULL))
    {
        goto done;
    }

    /* Initialize the timer and timer handler */
    timer_property.max_entries = (RTI_INT32)property->max_receive_ports; /*safe to cast max ports micro assigns is 4 */
    timer_property.max_slots = timer_property.max_entries;
    /* The lock used here is the db_lock provided by the participant
     * This allows the bind/unbind calls which create the timeout to be synchronized
     * with the timeout event as they are served under the same lock.
     */
    self->property._parent._parent.timer = OSAPI_Timer_new(
                    &timer_property,self->property.lock);
    if (self->property._parent._parent.timer == NULL)
    {
        ZCOPY_LOG_NOTIF_TIMER_CREATE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* Create bind table */
    tbl_property.max_records = property->_parent.max_binds;
    tbl_property.max_indices = 1;
    tbl_property.max_cursors = 1;

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name, &id, 'b',  (RTI_INT32)factory->instance_counter);

    dbrc = DB_Database_create_table(
            &self->_parent._btable,
            self->property._parent._parent.db,
            &tbl_name[0],
            sizeof(struct ZCOPY_NotifInterfaceBindEntry),
            ZCOPY_NotifInterface_bind_compare,
            &tbl_property);

    if (DB_RETCODE_OK != dbrc)
    {
        ZCOPY_LOG_NOTIF_DB_TABLE_CREATE(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* It's unlikely that writer/reader allocation is greater than INT_MAX */
    if (property->_parent.max_binds > INT_MAX)
    {
        goto done;
    }
    ip.max_entries = (RTI_INT32)property->_parent.max_binds;
    self->unbound_index = REDA_Indexer_new(ZCOPY_NotifInterface_record_compare,
                                            &ip);
    if (self->unbound_index == NULL)
    {
        goto done;
    }

    pool_property.buffer_size = sizeof(struct ZCOPY_NotifInterfaceSqRecordEntry);
    pool_property.max_buffers = property->_parent.max_binds;
    pool_property.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
    self->sq_reader_pool = REDA_BufferPool_new(
            "sq_readers",
            &pool_property,
            ZCOPY_NotifInterface_initialize_sq_reader_records,
            NULL,
#ifndef RTI_CERT
            ZCOPY_NotifInterface_finalize_sq_reader_records,
#else
            NULL,
#endif
            NULL);
    if (self->sq_reader_pool == NULL)
    {
        goto done;
    }

    /* Create route table */
    tbl_property.max_records = property->_parent.max_routes;
    tbl_property.max_indices = 1;
    tbl_property.max_cursors = 1;

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name, &id, 'r',  (RTI_INT32)factory->instance_counter);

    dbrc = DB_Database_create_table(
            &self->_parent._rtable,
            self->property._parent._parent.db,
            &tbl_name[0],
            sizeof(struct ZCOPY_NotifInterfaceRouteEntry),
            ZCOPY_NotifInterface_compare_route,
            &tbl_property);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_TABLE_CREATE(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    pool_property.buffer_size = ZCOPY_Notifier_get_size();
    pool_property.max_buffers = property->_parent.max_routes;
    pool_property.flags = 0;
    self->notifier_pool = REDA_BufferPool_new(
            "notifiers",
            &pool_property,
            ZCOPY_NotifInterface_initialize_notifier,
            NULL,
#ifndef RTI_CERT
            ZCOPY_NotifInterface_finalize_notifier,
#else
            NULL,
#endif
            NULL);
    if (self->notifier_pool == NULL)
    {
        goto done;
    }

    /* Create port table */
    tbl_property.max_records = property->max_receive_ports;
    tbl_property.max_indices = 1;
    tbl_property.max_cursors = 1;

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name, &id, 'p',  (RTI_INT32)factory->instance_counter);

    self->port_table = NULL;
    dbrc = DB_Database_create_table(
            &self->port_table,
            self->property._parent._parent.db,
            &tbl_name[0],
            sizeof(struct ZCOPY_NotifInterfacePortEntry),
            ZCOPY_NotifInterface_compare_port,
            &tbl_property);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_TABLE_CREATE(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* Create table of reliable readers. This table won't have best effort
     * readers.
     */
    tbl_property.max_records = property->_parent.max_routes;
    tbl_property.max_indices = 1;
    tbl_property.max_cursors = 1;

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name, &id, 't',
                                       (RTI_INT32)factory->instance_counter);

    self->reliable_table = NULL;
    dbrc = DB_Database_create_table(
            &self->reliable_table,
            self->property._parent._parent.db,
            &tbl_name[0],
            sizeof(struct ZCOPY_NotifInterfaceRouteKeyEntry),
            ZCOPY_NotifInterface_route_key_compare,
            &tbl_property);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_TABLE_CREATE(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
#ifndef RTI_CERT
    if (!ok)
    {
        /* initialize() is only called during domain participant
         * creation, where we do not hold the database lock.
         */
        if (!ZCOPY_NotifInterface_finalize(self, RTI_FALSE))
        {
            return RTI_FALSE;
        }
    }
#endif
    return ok;
}

/*ci
 * \brief Cancel transmission of a packet on the Notification interface
 *
 * \details
 * Implementation of the NETIO_Interface_xmit_remove function.
 * Although the Notification interface cannot cancel transmission of a packet,
 * an upstream interface does not necessarily keep track of the capabilities
 * of the downstream interface and will call the xmite_remove function on the
 * downstream interface.
 *
 * \param[in] intf        NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_xmit_remove(
        NETIO_Interface_T *intf,
        struct NETIO_Address *destination,
        NETIO_PacketId_T *packet_id)
{
    /* xmit_remove is NOOP for Notification intf */
    UNUSED_ARG(intf);
    UNUSED_ARG(destination);
    UNUSED_ARG(packet_id);

    return RTI_TRUE;
}

/*ci
 * \brief Send a packet over one or all known routes for this interface
 *
 * \details
 * Implementation of the NETIO_Interface_send function.
 *
 * \param[in] netio_intf NETIO interface to send from
 * \param[in] source     The source of the packet
 * \param[in] address    The destination address.
 * \param[in] packet     The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_send(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Interface *source,
        struct NETIO_Address *address,
        NETIO_Packet_T *packet)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL result;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    struct NETIORouteEntryKey key_low, key_high;
    struct ZCOPY_NotifInterfaceRouteEntry *route_entry;
    struct NETIO_Guid guid_max = {
            {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}},
            {{0xff, 0xff, 0xff, 0xff}}};
    DB_Cursor_T cursor;
    DB_ReturnCode_T dbrc;
    RTI_UINT32 total_ref_count = 0;
    RTI_UINT32 i;

    UNUSED_ARG(source);
    UNUSED_ARG(address);

    OSAPI_PRECONDITION((netio_intf == NULL) || (packet == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("packet", packet, RTI_TRUE);)

    key_low.intf_address = packet->source;
    key_low.intf = netio_intf;
    NETIO_Address_init(&key_low.destination, NETIO_ADDRESS_KIND_NOTIF);

    key_high.intf_address = packet->source;
    key_high.intf = netio_intf;
    NETIO_Address_set_guid(&key_high.destination, 0, &guid_max);
    key_high.destination.port = UINT_MAX;
    key_high.destination.kind = NETIO_ADDRESS_KIND_NOTIF;

    /* The packet->dests always contains an unknown packet destination because
     * the DWI would normally pass this sequence to RTPS, and RTPS would
     * determine the appropriate packet destinations. Therefore, we only look
     * at the packet source and then determine which address to send to for
     * ourselves.
     */
    dbrc = DB_Table_select_range(
            self->_parent._rtable,
            DB_TABLE_DEFAULT_INDEX,
            &cursor,
            (DB_Key_T)&key_low,
            (DB_Key_T)&key_high);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&route_entry);
        if (dbrc == DB_RETCODE_OK)
        {
            result = ZCOPY_Notifier_notify(route_entry->_notifier);
#if OSAPI_ENABLE_LOG
            if (!result)
            {
                ZCOPY_LOG_NOTIF_NOTIFIER_NOTIFY(OSAPI_LOGKIND_WARNING)
            }
#else
            IGNORE_RETVAL(result);
#endif
            /* It doesn't matter if the send is successful or not. The sample
             * is already in the shared queue. The only thing that happens here
             * is signalling that there is a message.
             */
            result = ZCOPY_NotifUserInterface_send(self, &packet->source, route_entry);

            /* Keep track of the reliable readers. */
            total_ref_count += route_entry->_reliable_count;
#if OSAPI_ENABLE_LOG
            if (!result)
            {
                ZCOPY_LOG_NOTIF_MECHANISM_SEND(OSAPI_LOGKIND_WARNING)
            }
#else
            IGNORE_RETVAL(result);
#endif
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(self->_parent._rtable, cursor);

    /* Ack each reliable reader. ZCV2 does not have acknack feedback. This
     * means that when we talk with reliable readers we don't get acks from
     * that reader. However, we still need to adhere the acknack expections
     * in order for the WHSM to know when it can remove samples and free keys.
     * This operation still needs to occur once even if total_ref_count is 0
     * in order for best effort readers to be able to remove samples from the
     * WH.
     */
    for (i = 0; i < (total_ref_count > 0 ? total_ref_count : 1); ++i)
    {
        if (!NETIO_Interface_acknack(source, NULL, &packet->info.sn, RTI_FALSE))
        {
            ZCOPY_LOG_NOTIF_MECHANISM_SEND(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }


    if (dbrc != DB_RETCODE_NO_DATA)
    {
        ZCOPY_LOG_NOTIF_DB_CURSOR_NEXT(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_BOOL
ZCOPY_NotifInterface_receive(
        NETIO_Interface_T *netio_intf,
        const struct NETIO_Address *port,
        RTI_BOOL *has_more_data_out)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    struct NETIO_Packet packet = NETIO_Packet_INITIALIZER;
    struct ZCOPY_Guid source;
    DB_Cursor_T cursor = NULL;
    struct ZCOPY_NotifInterfacePortEntry *port_entry = NULL;
    struct ZCOPY_NotifInterfaceBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_UINT32 i;
    RTI_BOOL local_has_more_data = RTI_FALSE;
    RTI_BOOL notified;
    RTI_BOOL rtn = RTI_FALSE;
    struct ZCOPY_NotifInterfaceBindEntryKey key_low = {
            NETIO_Address_INITIALIZER,
            (NETIO_Interface_T *)0x00};
    /* Casting -1 to a pointer gives the maximum possible pointer value */
    struct ZCOPY_NotifInterfaceBindEntryKey key_high = {
            NETIO_Address_INITIALIZER,
            (NETIO_Interface_T *)(-1)
    };

    OSAPI_PRECONDITION_ALWAYS(
            (netio_intf == NULL) || (port == NULL) || (has_more_data_out == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("port", port, RTI_FALSE);
            OSAPI_Log_entry_add_pointer(
                    "has_more_data_out",
                    has_more_data_out,
                    RTI_TRUE));

    *has_more_data_out = RTI_FALSE;

    if (DB_Database_lock(self->property._parent._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    packet.info.protocol_id = NETIO_PROTOCOL_NOTIF;
    packet.info.protocol_data.notif_info.sample_accesor =
                                        &ZCOPY_NotifInterface_fv_sample_accesor;

    /* Find the port entry corresponding to dst_addr */
    dbrc = DB_Table_select_match(
            self->port_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&port_entry,
            (DB_Key_T)port);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* Process notifications from the list of pending notifications */
    do
    {
        if (!ZCOPY_Notifiee_next(port_entry->_notifiee, &source, &notified))
        {
            ZCOPY_LOG_NOTIF_NOTIFIEE_NEXT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        if (notified)
        {
            NETIO_Address_set_from_notif_guid(&key_low.source, &source);
            NETIO_Address_set_from_notif_guid(&key_high.source, &source);

            dbrc = DB_Table_select_range(
                    self->_parent._btable,
                    DB_TABLE_DEFAULT_INDEX,
                    &cursor,
                    &key_low,
                    &key_high);

            if (dbrc != DB_RETCODE_OK)
            {
                ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
                goto done;
            }

            dbrc = DB_Cursor_get_next(cursor, (DB_Record_T)&bind_entry);
            while (dbrc == DB_RETCODE_OK)
            {
                NETIO_Address_set_guid(
                        &packet.source,
                        0,
                        &bind_entry->key.source.value.guid);

                /* Upstream DataReader saves entries as intra */
                packet.source.kind = NETIO_ADDRESS_KIND_INTRA;
                packet.info.protocol_data.notif_info.opaque_data.reader =
                                                bind_entry->_sq_reader_entry;

                for (i = 0; i < self->factory->property.max_samples_per_notif; ++i)
                {
                    if (!ZCOPY_NotifInterface_peek_info(
                                bind_entry->_sq_reader_entry->sq_reader,
                                &packet.info,
                                &local_has_more_data))
                    {
                        /* No samples available on sq_reader */
                        break;
                    }

                    if (!NETIO_Interface_receive(
                                bind_entry->key.interface,
                                &port_entry->address,
                                NULL,
                                &packet))
                    {
                        ZCOPY_LOG_NOTIF_INTF_RECEIVE(OSAPI_LOGKIND_ERROR)
                    }

                    rtn = NETIO_SampleI_on_add(
                            packet.info.protocol_data.notif_info.sample_accesor,
                            bind_entry->_sq_reader_entry);
                    UNUSED_ARG(rtn);
                }

                if (local_has_more_data)
                {
                    /* Potentially more samples to be processed */
                    if (!ZCOPY_Notifiee_raise_flag(port_entry->_notifiee, &source))
                    {
                        ZCOPY_LOG_NOTIF_NOTIFIEE_RAISE_FLAG(OSAPI_LOGKIND_ERROR)
                        DB_Cursor_finish(self->_parent._btable, cursor);
                        goto done;
                    }
                    *has_more_data_out = RTI_TRUE;
                }

                dbrc = DB_Cursor_get_next(cursor, (DB_Record_T)&bind_entry);
            }
            DB_Cursor_finish(self->_parent._btable, cursor);

            if (dbrc != DB_RETCODE_NO_DATA)
            {
                ZCOPY_LOG_NOTIF_DB_CURSOR_NEXT(OSAPI_LOGKIND_ERROR, dbrc)
                goto done;
            }
        }
    } while (notified);

    ok = RTI_TRUE;
done:
    if (DB_Database_unlock(self->property._parent._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return ok;
}

/*ci
 * \brief Add this reliable reader to our list of reliable readers
 *
 * \details
 *    ZCV2 does not have acknack feedback for reliable reader. This
 *    means that when we talk with reliable readers we
 *    don't get acks from that reader. However, we still need to adhere the
 *    acknack expections in order for the WHSM to know when it can remove
 *    samples and free keys. The keys are handled by
 *    WHSM_History_analyze_sample_state which requires the ack count to be 0
 *    before keys and samples can be removed.
 *
 * \param[in] self        The notif interface
 * \param[in] via_intf    The interface used to hold the GUID of the reader
 * \param[in] route_entry The port entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_add_reliable_route(struct ZCOPY_NotifInterface *self,
                            NETIO_Interface_T *via_intf,
                            struct ZCOPY_NotifInterfaceRouteEntry *route_entry)
{
    struct ZCOPY_NotifInterfaceRouteKeyEntry *key_entry = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_create_record(self->reliable_table,(DB_Record_T *)&key_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_CREATE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    key_entry->key = via_intf->local_address.value.guid;
    key_entry->route_ptr = route_entry;

    dbrc = DB_Table_insert_record(self->reliable_table,(DB_Record_T)key_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(self->reliable_table,
                                     (DB_Record_T)key_entry);
        IGNORE_RETVAL(dbrc);

        ZCOPY_LOG_NOTIF_DB_INSERT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    route_entry->_reliable_count++;

    return RTI_TRUE;
}

/*ci
 * \brief Delete a reader from our list of reliable readers
 *
 * \details delete an entry from the reliable table. It's possible that the
 *           reader is not in our list. That's not an error.
 *
 * \param[in] self        The notif interface
 * \param[in] via_intf    The interface used to hold the GUID of the reader
 * \param[in] route_entry The port entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_delete_reliable_route(struct ZCOPY_NotifInterface *self,
                            NETIO_Interface_T *via_intf,
                            struct ZCOPY_NotifInterfaceRouteEntry *route_entry)
{
    struct ZCOPY_NotifInterfaceRouteKeyEntry *key_entry = NULL;
    struct ZCOPY_NotifInterfaceRouteKeyEntry key =
                            ZCOPY_NOTIFINTERFACE_ROUTE_KEY_ENTRY_INITIALIZER;
    DB_ReturnCode_T dbrc;

    key.key = via_intf->local_address.value.guid;
    key.route_ptr = route_entry;
    dbrc = DB_Table_select_match(
            self->reliable_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&key_entry,
            (DB_Key_T)&key);

    /* We don't have to find an entry. If we do find one, delete it. */
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(self->reliable_table,
                                      (DB_Record_T)key_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
            return RTI_FALSE;
        }

        if (route_entry->_reliable_count > 0)
        {
            route_entry->_reliable_count--;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Add a route to the Notification interface
 *
 * \details
 * Implementation of the NETIO add_route function.
 *
 * \param[in] netio_intf NETIO interface to add the route too
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_add_route(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *dst_addr, /* GUID of local DW */
        NETIO_Interface_T *via_intf,
        struct NETIO_Address *via_addr, /* Locator of remote DR */
        struct NETIORouteProperty *property,
        RTI_BOOL *existed)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    struct ZCOPY_Notifier *notifier = NULL;
    struct NETIORouteEntryKey route_key;
    struct ZCOPY_NotifInterfaceRouteEntry *route_entry = NULL;
    struct ZCOPY_Guid notifier_guid = ZCOPY_Guid_INITIALIZER;
    struct ZCOPY_Guid notifiee_guid = ZCOPY_Guid_INITIALIZER;
    DB_ReturnCode_T dbrc;
    RTI_BOOL result;
    RTI_BOOL connected = RTI_FALSE;
    RTI_BOOL user_added = RTI_FALSE;

    OSAPI_PRECONDITION((netio_intf == NULL) || (via_addr == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_addr", dst_addr, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("via_addr", via_addr, RTI_TRUE);)

    route_key.destination = *via_addr;
    route_key.intf_address = *dst_addr;
    route_key.intf = netio_intf;

    /* Check if the route already exists. A route goes from a local
     * DataWriter to a remote participant's notification interface. If there are
     * multiple DataReaders on the remote participant which match the same local
     * DataWriter, they will share the same route.
     */
    dbrc = DB_Table_select_match(
            self->_parent._rtable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&route_entry,
            (DB_Key_T)&route_key);

    /* Need to update existed out-param before returning */
    if (existed != NULL)
    {
        *existed = (dbrc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if (dbrc == DB_RETCODE_OK)
    {
        /* It existed so we can reuse the route. However, we have to increase
         * the refcount so we also know when (not) to destroy the route
         * when it is removed.
         */
        ++route_entry->_ref_count;
        if (property->is_reliable)
        {
            if (!ZCOPY_NotifInterface_add_reliable_route(self,via_intf,
                                                         route_entry))
            {
                ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
        ok = RTI_TRUE;
        goto done;
    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* Since it did not exist, create a new route */
    dbrc = DB_Table_create_record(self->_parent._rtable, (DB_Record_T *)&route_entry);
    if (DB_RETCODE_OK != dbrc)
    {
        ZCOPY_LOG_NOTIF_DB_CREATE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* Initialize route entry to be inserted */
    notifier = (struct ZCOPY_Notifier *)REDA_BufferPool_get_buffer(self->notifier_pool);
    if (notifier == NULL)
    {
        ZCOPY_LOG_NOTIF_NO_MORE_NOTIFIERS(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    ZCOPY_Guid_set_from_netio_addr(&notifier_guid, dst_addr);
    ZCOPY_Guid_set_from_netio_addr(&notifiee_guid, via_addr);
    if (!ZCOPY_Notifier_open(
                notifier,
                &notifier_guid,
                &notifiee_guid,
                via_addr->port))
    {
        ZCOPY_LOG_NOTIF_NOTIFIER_CONNECT(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    connected = RTI_TRUE;


    route_entry->_parent.intf_address = *dst_addr;
    route_entry->_parent.destination = *via_addr;
    route_entry->_parent.intf = netio_intf;
    route_entry->_ref_count = 1;
    route_entry->_reliable_count = 0;
    route_entry->_notifier = notifier;

    if (property->is_reliable)
    {
        if (!ZCOPY_NotifInterface_add_reliable_route(self, via_intf,
                                                     route_entry))
        {
            ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (!ZCOPY_NotifUserInterface_add_route(self, route_entry))
    {
        result = ZCOPY_NotifInterface_delete_reliable_route(self, via_intf,
                route_entry);
        UNUSED_ARG(result);

        ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    user_added = RTI_TRUE;

    dbrc = DB_Table_insert_record(self->_parent._rtable, (DB_Record_T)route_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_INSERT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
    if (!ok)
    {
        /* Roll back */
        if (user_added)
        {
            result = ZCOPY_NotifUserInterface_delete_route(self, route_entry);
#if OSAPI_ENABLE_LOG
            if (!result) {
                ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(result);
#endif
        }
        if (connected)
        {
            result = ZCOPY_Notifier_close(notifier);
#if OSAPI_ENABLE_LOG
            if (!result)
            {
                ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(result);
#endif
        }
        if (notifier != NULL)
        {
            REDA_BufferPool_return_buffer(self->notifier_pool, notifier);
        }
        if (route_entry != NULL)
        {
            dbrc = DB_Table_delete_record(
                    self->_parent._rtable,
                    (DB_Record_T)route_entry);
            IGNORE_RETVAL(dbrc);
        }
    }

    return ok;
}

/*ci
 * \brief Delete a route from the Notification interface
 *
 * \details
 * Implementation of the NETIO delete_route function.
 *
 * \param[in]  netio_intf The notification interface
 * \param[in]  dst_addr   The destination address for the route
 * \param[in]  via_intf   Not used
 * \param[in]  via_addr   The address to pass to the downstream interface
 * \param[out] existed    Whether the route existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_delete_route(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *dst_addr,
        NETIO_Interface_T *via_intf,
        struct NETIO_Address *via_addr,
        RTI_BOOL *existed)
{
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    struct NETIORouteEntryKey route_key;
    struct ZCOPY_NotifInterfaceRouteEntry *route_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((netio_intf == NULL) || (dst_addr == NULL) || (via_addr == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_addr", dst_addr, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("via_addr", via_addr, RTI_TRUE);)

    route_key.destination = *via_addr;
    route_key.intf_address = *dst_addr;
    route_key.intf = netio_intf;

    /* See if this route exists */
    dbrc = DB_Table_select_match(
            self->_parent._rtable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&route_entry,
            (DB_Key_T)&route_key);

    if (existed != NULL)
    {
        *existed = (dbrc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /* Route does not exist */
        ok = RTI_TRUE;
        goto done;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    if (!ZCOPY_NotifInterface_delete_reliable_route(self,via_intf,route_entry))
    {
        ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    --route_entry->_ref_count;
    if (route_entry->_ref_count == 0)
    {
        if (!ZCOPY_NotifUserInterface_delete_route(self, route_entry))
        {
            ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if (!ZCOPY_Notifier_close(route_entry->_notifier))
        {
            ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        REDA_BufferPool_return_buffer(self->notifier_pool, route_entry->_notifier);
        route_entry->_notifier = NULL;

        dbrc = DB_Table_delete_record(self->_parent._rtable, (DB_Record_T *)route_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
            goto done;
        }
    }

    ok = RTI_TRUE;

done:
    return ok;
}

/*ci
 * \brief Add a bind to the Notification interface
 *
 * \details
 * Implementation of the NETIO bind function.
 *
 * Notification does not maintain any state information about its peers, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf Not used
 * \param[in]  src_addr   Not used
 * \param[in]  property   Not used
 * \param[out] existed    If not NULL, the value is always set to RTI_TRUE
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_bind(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        struct NETIOBindProperty *property,
        RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(property);

    if (existed != NULL)
    {
        *existed = RTI_TRUE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Remove a peer listener on a notification interface
 *
 * \details
 * Implementation of the NETIO unbind function.
 *
 * Notification does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf Not used
 * \param[in]  src_addr   Not used
 * \param[in]  dst_intf   Not used
 * \param[out] existed    If not NULL, the value is always set to RTI_TRUE
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref ZCOPY_NotifInterface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_unbind(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_intf);

    if (existed != NULL)
    {
        *existed = RTI_TRUE;
    }

    return RTI_TRUE;
}



/*ci
 * \brief gets a sq_reader buffer.
 *
 * \details
 * If a sq_reader still has referenced samples when used in unbind_external
 * then the entry is not closed and the entry is added to the indexer. When we
 * fill the sq_reader pool, we check if there are records that aren't
 * referencing samples any longer and we return those buffers.
 *
 *
 * \param[in]  pool     The buffer pool
 * \param[in]  indexer  The indexer
 *
 * \return This function returns a buffer on success or NULL on failure.
 */
RTI_PRIVATE struct ZCOPY_NotifInterfaceSqRecordEntry *
ZCOPY_NotifInterface_get_sq_buffer(REDA_BufferPool_T pool,
                                    struct REDA_Indexer *indexer)
{
    struct ZCOPY_NotifInterfaceSqRecordEntry *rtn = NULL;
    struct ZCOPY_NotifInterfaceSqRecordEntry *record = NULL;
    REDA_IndexIterator_T *it = NULL;

    rtn = (struct ZCOPY_NotifInterfaceSqRecordEntry *)REDA_BufferPool_get_buffer(pool);
    if (rtn != NULL)
    {
        goto done;
    }

    /* All of the buffers have been consumed because there were still referenced
     * samples for that SQ reader when unbind was called. Now that all of the
     * pools have been consumed, loop through all unbound entries and free up
     * the resources that no longer have referenced samples.
     */
    it = REDA_Indexer_iterator_begin(indexer);
    record = (struct ZCOPY_NotifInterfaceSqRecordEntry *)REDA_Indexer_iterator_next(it);
    while (record != NULL)
    {
        if (record->ref_count == 0)
        {
            if (!NETIO_ZCOPY_SharedQReader_close(record->sq_reader))
            {
                ZCOPY_LOG_NOTIF_SQ_CLOSE(OSAPI_LOGKIND_ERROR)
                return NULL;
            }

            if (REDA_Indexer_remove_entry(indexer,record->sq_reader) == NULL)
            {
                ZCOPY_LOG_REMOVE_ENTRY(OSAPI_LOGKIND_ERROR)
                return NULL;
            }

            if (record->sq_reader != NULL)
            {
                REDA_BufferPool_return_buffer(pool, record);
            }
        }
        record = (struct ZCOPY_NotifInterfaceSqRecordEntry *)REDA_Indexer_iterator_next(it);
    }

    rtn = REDA_BufferPool_get_buffer(pool);
done:
    if (rtn != NULL)
    {
        NETIO_ZCOPY_SharedQReader_reset(rtn->sq_reader);
    }

    return rtn;
}

/*ci
 * \brief Bind the Notification interface to a upstream interface
 *
 * \details
 * Implementation of the NETIO bind_external function.
 *
 * \param[in]  src_intf  The notification interface
 * \param[in]  src_addr  Notification address of local DataReader
 * \param[in]  dst_intf  The upstream interface
 * \param[in]  dst_addr  GUID of remote DataWriter
 * \param[in]  property  Not used
 * \param[out] existed   Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref ZCOPY_NotifInterface_unbind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_bind_external(
        NETIO_Interface_T *src_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        struct NETIO_Address *dst_addr,
        struct NETIOBindProperty *property,
        RTI_BOOL *existed)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL rc = RTI_FALSE;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)src_intf;
    struct ZCOPY_NotifInterfacePortEntry *port_entry = NULL;
    struct ZCOPY_NotifInterfaceBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct ZCOPY_NotifInterfaceBindEntryKey bind_key;
    struct ZCOPY_Guid notif_guid;
    struct ZCOPY_Guid owner_key;
    RTI_BOOL opened = RTI_FALSE;
    RTI_BOOL allowed = RTI_FALSE;
    RTI_BOOL bound = RTI_FALSE;

    UNUSED_ARG(property);

    OSAPI_PRECONDITION((src_intf == NULL) || (src_addr == NULL) || (dst_intf == NULL) ||
                               (dst_addr == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("src_intf", src_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("src_addr", src_addr, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_intf", dst_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_addr", dst_addr, RTI_TRUE);)

    OSAPI_TRACE_NET("bind external:", RTI_FALSE)
    OSAPI_TRACE_GUID("source", &src_addr->value.as_int32.value, RTI_FALSE)
    OSAPI_TRACE_GUID("intf", &dst_addr->value.as_int32.value, RTI_TRUE)

    /* Find the receive entry */
    dbrc = DB_Table_select_match(
            self->port_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&port_entry,
            (DB_Key_T)src_addr);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("address not found:", RTI_FALSE)
        OSAPI_TRACE_INT32("port", src_addr->port, RTI_FALSE)
        OSAPI_TRACE_GUID("address", &src_addr->value.rtps_guid, RTI_TRUE)
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    bind_key.source = *dst_addr;
    bind_key.interface = dst_intf;

    /* Check to see if this bind already exists */
    dbrc = DB_Table_select_match(
            self->_parent._btable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&bind_entry,
            (DB_Key_T)&bind_key);
    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_NO_DATA))
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed != NULL)
        {
            *existed = RTI_TRUE;
        }
        ok = RTI_TRUE;
        goto done;
    }

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    /* Create new bind entry */
    dbrc = DB_Table_create_record(self->_parent._btable, (DB_Record_T *)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_CREATE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    bind_entry->key = bind_key;
    bind_entry->_sq_reader_entry = ZCOPY_NotifInterface_get_sq_buffer(
                                    self->sq_reader_pool, self->unbound_index);
    if ((bind_entry->_sq_reader_entry == NULL))
    {
        ZCOPY_LOG_NOTIF_NO_MORE_NOTIFIERS(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    OSAPI_Memory_copy(owner_key.value, &bind_entry->key.source.value.guid, 16);
    if (!NETIO_ZCOPY_SharedQReader_open(
                bind_entry->_sq_reader_entry->sq_reader,
                &owner_key,
                (RTI_UINT32)self->property.domain_id))
    {
        ZCOPY_LOG_NOTIF_SQ_OPEN(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    opened = RTI_TRUE;

    ZCOPY_Guid_set_from_netio_addr(&notif_guid, &bind_entry->key.source);
    if (!ZCOPY_Notifiee_allow_notifier(port_entry->_notifiee, &notif_guid))
    {
        ZCOPY_LOG_NOTIF_NOTIFIER_ALLOW(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    allowed = RTI_TRUE;

    if (!ZCOPY_NotifUserInterface_bind(
                self,
                src_addr,
                dst_addr,
                port_entry->_user_entry,
                &bind_entry->_user_entry))
    {
        ZCOPY_LOG_NOTIF_MECHANISM_BIND(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    bound = RTI_TRUE;

    /* raise flag preemptively for late joining readers*/
    if (!ZCOPY_Notifiee_raise_flag(port_entry->_notifiee, &notif_guid))
    {
        goto done;
    }

    /* create timeout*/
    if (!ZCOPY_NotifInterface_create_timeout(self->property._parent._parent.timer, port_entry))
    {
        goto done;
    }

    dbrc = DB_Table_insert_record(self->_parent._btable, (DB_Record_T)bind_entry);
    if (DB_RETCODE_OK != dbrc)
    {
        ZCOPY_LOG_NOTIF_DB_INSERT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ++port_entry->_ref_count;

    ok = RTI_TRUE;

done:
    if ((ok == RTI_FALSE) && (bind_entry != NULL))
    {
        if (bound)
        {
            /* The timeout does not need to cancelled nor the flag lowered
             * The timeout can run as scheduled as the flag will be lowered in
             * the remove notifier call below and no notifications will be processed.
             */
            rc = ZCOPY_NotifUserInterface_unbind(
                    self,
                    src_addr,
                    dst_addr,
                    port_entry->_user_entry,
                    bind_entry->_user_entry);
            IGNORE_RETVAL(rc);
        }

        /* Return values are ignored in the if blocks below because the operation
         * has failed at this point and we are attempting to clean up
         */
        if (allowed)
        {
            rc = ZCOPY_Notifiee_remove_notifier(port_entry->_notifiee, &notif_guid);
            IGNORE_RETVAL(rc);
        }

        if (opened)
        {
            rc = NETIO_ZCOPY_SharedQReader_close(bind_entry->_sq_reader_entry->sq_reader);
            IGNORE_RETVAL(rc);
        }

        if ((bind_entry != NULL) && (bind_entry->_sq_reader_entry != NULL))
        {
            REDA_BufferPool_return_buffer(self->sq_reader_pool, bind_entry->_sq_reader_entry);
        }

        dbrc = DB_Table_delete_record(self->_parent._btable, (DB_Record_T)bind_entry);
        IGNORE_RETVAL(dbrc);
    }

    return ok;
}

/*ci
 * \brief  Remove an upstream interface as a listener to the Notification interface.
 *
 * \details
 * Implementation of the NETIO unbind_external function.
 *
 * \param[in]  src_intf  The notification interface
 * \param[in]  src_addr  Notification address of local DataReader
 * \param[in]  dst_intf  The upstream interface
 * \param[in]  dst_addr  GUID of remote DataWriter
 * \param[out] existed   Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref ZCOPY_NotifInterface_bind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_unbind_external(
        NETIO_Interface_T *src_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        struct NETIO_Address *dst_addr,
        RTI_BOOL *existed)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)src_intf;
    struct ZCOPY_NotifInterfaceBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct ZCOPY_NotifInterfaceBindEntryKey bind_key;
    struct ZCOPY_NotifInterfacePortEntry *port_entry = NULL;
    struct ZCOPY_Guid notif_guid;

    OSAPI_PRECONDITION((src_intf == NULL) || (src_addr == NULL) || (dst_intf == NULL) ||
                               (dst_addr == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("src_intf", src_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("src_addr", src_addr, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_intf", dst_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dst_addr", dst_addr, RTI_TRUE);)

    OSAPI_TRACE_NET("unbind external:", RTI_FALSE)
    OSAPI_TRACE_GUID("source", &src_addr->value.as_int32.value, RTI_FALSE)
    OSAPI_TRACE_GUID("intf", &dst_addr->value.as_int32.value, RTI_TRUE)

    bind_key.source = *dst_addr;
    bind_key.interface = dst_intf;

    /* Check to see if this bind already exists */
    dbrc = DB_Table_select_match(
            self->_parent._btable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&bind_entry,
            (DB_Key_T)&bind_key);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_NET("bind does not exist", RTI_FALSE)
        OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(src_addr), RTI_FALSE)
        OSAPI_TRACE_INT32("port", src_addr->port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", src_addr->value.ipv4.address, RTI_TRUE)

        if (existed != NULL)
        {
            *existed = RTI_FALSE;
        }
        ok = RTI_TRUE;
        goto done;
    }
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    if (existed != NULL)
    {
        *existed = RTI_TRUE;
    }

    bind_entry = NULL;
    dbrc = DB_Table_remove_record(
            self->_parent._btable,
            (DB_Record_T *)&bind_entry,
            &bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_REMOVE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    /* Find the port entry */
    dbrc = DB_Table_select_match(
            self->port_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&port_entry,
            (DB_Key_T)src_addr);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }
    --port_entry->_ref_count;

    ZCOPY_Guid_set_from_netio_addr(&notif_guid, &bind_entry->key.source);
    if (!ZCOPY_Notifiee_remove_notifier(port_entry->_notifiee, &notif_guid))
    {
        ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* Close sq readers after there are no referenced samples */
    if (bind_entry->_sq_reader_entry->ref_count == 0)
    {
        if (!NETIO_ZCOPY_SharedQReader_close(bind_entry->_sq_reader_entry->sq_reader))
        {
            ZCOPY_LOG_NOTIF_SQ_CLOSE(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        REDA_BufferPool_return_buffer(self->sq_reader_pool, bind_entry->_sq_reader_entry);
    }
    else
    {
        if (!REDA_Indexer_add_entry(self->unbound_index,bind_entry->_sq_reader_entry))
        {
            ZCOPY_LOG_ADD_ENTRY(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }
    bind_entry->_sq_reader_entry = NULL;

    if (!ZCOPY_NotifUserInterface_unbind(
                self,
                src_addr,
                dst_addr,
                port_entry->_user_entry,
                bind_entry->_user_entry))
    {
        ZCOPY_LOG_NOTIF_MECHANISM_UNBIND(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dbrc = DB_Table_delete_record(self->_parent._btable, (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
    return ok;
}

/*ci
 * \brief Set the state of the Notification interface
 *
 * \details
 * Implementation of the NETIO set_state function. The Notification interface is
 * always enabled. This function is only set to comply with the NETIO interface.
 *
 * \param[in] src_intf NETIO interface to set state on
 * \param[in] state    New state
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_set_state(NETIO_Interface_T *src_intf, NETIO_InterfaceState_T state)
{
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)src_intf;

    OSAPI_PRECONDITION((src_intf == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("src_intf", src_intf, RTI_TRUE);)

    self->_parent.state = state;

    return RTI_TRUE;
}

/*ci
 * \brief Resolve a Notification address
 *
 * \details
 * Implementation of the NETIO resolve_address function.
 *
 * The Notification interface defers the implementation of this function
 * to the user notification mechanism.
 *
 * \param[in]  netio_intf     The notification interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return This function return RTI_TRUE if the address string is empty,
 *         RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_resolve_address(
        NETIO_Interface_T *netio_intf,
        const char *address_string,
        struct NETIO_Address *address_value,
        RTI_BOOL *is_invalid)
{
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION(
            (netio_intf == NULL) || (address_string == NULL) || (address_value == NULL) ||
                    (is_invalid == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_string", address_string, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_value", address_value, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("is_invalid", is_invalid, RTI_TRUE);)

    /* Assume that the address is always valid to allow other transports
     * to resolve this address if we do not understand it
     */
    *is_invalid = RTI_FALSE;

    if (!NETIO_Interface_resolve_address(
                self->user_netio,
                address_string,
                address_value,
                is_invalid))
    {
        goto done;
    }

    if (NETIO_Address_get_kind(address_value) != NETIO_ADDRESS_KIND_NOTIF)
    {
        goto done;
    }

    ok = RTI_TRUE;

done:
    return ok;
}

/*ci
 * \brief Release a previously reserved address on a notification interface
 *
 * \details
 * Implementation of the NETIO release_address function.
 *
 * \param[in] netio_intf The notification interface
 * \param[in] address    Address to release
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_release_address(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *address)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    DB_ReturnCode_T dbrc;
    struct ZCOPY_NotifInterfacePortEntry *port_entry = NULL;
    struct ZCOPY_NotifInterfacePortEntry *port_entry1 = NULL;

    OSAPI_PRECONDITION(((netio_intf == NULL) || (address == NULL)),
                       goto done,
                       OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("resvd_addr", address, RTI_TRUE);)

    OSAPI_TRACE_NET("release address:", RTI_FALSE)
    OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(address), RTI_FALSE)
    OSAPI_TRACE_INT32("port", address->port, RTI_FALSE)
    OSAPI_TRACE_INT32("address", address->value.ipv4.address, RTI_TRUE)

    dbrc = DB_Database_lock(self->property._parent._parent.db);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(
            self->port_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T *)&port_entry,
            (DB_Key_T)address);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    if (port_entry->_ref_count != 0)
    {
        ZCOPY_LOG_NOTIF_PORT_IN_USE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dbrc = DB_Table_remove_record(
            self->port_table,
            (DB_Record_T *)&port_entry1,
            (DB_Key_T)address);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_REMOVE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    if (OSAPI_Timer_handle_is_valid(&port_entry->_writer_match_timeout))
    {
        if (!OSAPI_Timer_delete_timeout(self->property._parent._parent.timer,
                                &port_entry->_writer_match_timeout))
        {
            ZCOPY_LOG_NOTIF_TIMEOUT_DELETE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    /* Database must be unlocked before destroying a receive thread
     * because the receive thread may take the lock on the database.
     */
    dbrc = DB_Database_unlock(self->property._parent._parent.db);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    if (!ZCOPY_NotifUserInterface_release_address(self, address, port_entry->_user_entry))
    {
        ZCOPY_LOG_NOTIF_MECHANISM_RELEASE_ADDR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
    port_entry->_user_entry = NULL;

    if (DB_Database_lock(self->property._parent._parent.db) != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    if (!ZCOPY_Notifiee_destroy(port_entry->_notifiee))
    {
        ZCOPY_LOG_NOTIF_NOTIFIEE_DESTROY(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    port_entry->_notifiee = NULL;

    dbrc = DB_Table_delete_record(self->port_table, (DB_Record_T)port_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        goto done;
    }

    ok = RTI_TRUE;

done:
    dbrc = DB_Database_unlock(self->property._parent._parent.db);
    if (dbrc != DB_RETCODE_OK)
    {
        ZCOPY_LOG_NOTIF_DB_LOCK(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }
    return ok;
}

/*ci
 * \brief Reserve addresses on a notification interface
 *
 * \details
 * Implementation of the NETIO reserve_address function.
 *
 * \param[in]    netio_intf The notification interface
 * \param[in]    req_addr   List of requested addresses
 * \param[inout] resvd_addr List of reserved address
 * \param[in]    property   Not used
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise
 *
 * \sa \ref ZCOPY_NotifInterface_release_address
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_reserve_address(
        NETIO_Interface_T *netio_intf,
        struct NETIO_AddressSeq *req_addr,
        struct NETIO_AddressSeq *resvd_addr,
        struct NETIOBindProperty *property)
{
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    RTI_INT32 start_addr_len;
    RTI_INT32 cur_addr;
    RTI_INT32 max_size;
    RTI_INT32 i, j;
    struct NETIO_Address *an_address = NULL;
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL result;

    UNUSED_ARG(property);

    OSAPI_PRECONDITION(
            ((netio_intf == NULL) || (req_addr == NULL) || (resvd_addr == NULL)),
            goto done,
            OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("req_addr", req_addr, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("resvd_addr", resvd_addr, RTI_TRUE);)

    /* Check available space in reserved address sequence */
    start_addr_len = NETIO_AddressSeq_get_length(resvd_addr);
    cur_addr = start_addr_len;
    max_size = NETIO_AddressSeq_get_maximum(resvd_addr);

    for (i = 0; ((i < NETIO_AddressSeq_get_length(req_addr)) && (cur_addr < max_size));
         ++i)
    {
        /* Since i < get_length(), it is assumed that get_reference succeeds */
        an_address = NETIO_AddressSeq_get_reference(req_addr, i);

        if (NETIO_Address_get_kind(an_address) != NETIO_ADDRESS_KIND_NOTIF)
        {
            OSAPI_TRACE_NET("ignoring address, unsupported kind:", RTI_FALSE)
            OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(an_address), RTI_FALSE)
            OSAPI_TRACE_INT32("port", an_address->port, RTI_FALSE)
            OSAPI_TRACE_GUID("address", an_address->value.as_int32.value, RTI_TRUE)
            continue;
        }

        /* Check if this address has already been reserved */
        for (j = 0; j < NETIO_AddressSeq_get_length(resvd_addr); ++j)
        {
            if (NETIO_Address_compare(
                        (NETIO_AddressSeq_get_reference(resvd_addr, j)),
                        an_address) == 0)
            {
                break;
            }
        }
        if (j < NETIO_AddressSeq_get_length(resvd_addr))
        {
            OSAPI_TRACE_NET("ignoring duplicate address:", RTI_FALSE)
            OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(an_address), RTI_FALSE)
            OSAPI_TRACE_INT32("port", an_address->port, RTI_FALSE)
            OSAPI_TRACE_GUID("address", an_address->value.as_int32.value, RTI_TRUE)
            continue;
        }

        /* Create port entry for address */
        if (!ZCOPY_NotifInterface_create_port_entry(self, an_address))
        {
            /* Expected error if the address is already reserved */
            goto done;
        }

        /* Successfully reserved an address */
        OSAPI_TRACE_NET("reserve address:", RTI_FALSE)
        OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(an_address), RTI_FALSE)
        OSAPI_TRACE_INT32("port", an_address->port, RTI_FALSE)
        OSAPI_TRACE_GUID("address", an_address->value.as_int32.value, RTI_TRUE)

        /* Since cur_addr < max_size, it is assumed that set_length succeeds */
        result = NETIO_AddressSeq_set_length(resvd_addr, cur_addr + 1);
        IGNORE_RETVAL(result);

        /* Since cur_addr < max_size, it is assumed that get_references succeeds */
        /* coverity[dereference] */
        /* coverity[cert_exp34_c_violation] */
        *NETIO_AddressSeq_get_reference(resvd_addr, cur_addr) = *an_address;
        ++cur_addr;
    }

    ok = RTI_TRUE;

done:
    if (!ok)
    {
        for (i = start_addr_len; i < cur_addr; ++i)
        {
            an_address = NETIO_AddressSeq_get_reference(resvd_addr, i);
            result = ZCOPY_NotifInterface_release_address(netio_intf, an_address);
            IGNORE_RETVAL(result);
        }
        result = NETIO_AddressSeq_set_length(resvd_addr, start_addr_len);
        IGNORE_RETVAL(result);
    }
    return ok;
}

/*ci
 * \brief Get the route table for a notification interface
 *
 * \details
 * Implementation of the NETIO get_route_table function.
 *
 * The notification interface defers the implementation of this function to
 * the user notification mechanism.
 *
 * \param[in]    netio_intf The notification interface
 * \param[inout] address    Sequence of NETIO addresses this interface understands
 * \param[inout] netmask    Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_get_route_table(
        NETIO_Interface_T *netio_intf,
        struct NETIO_AddressSeq *address,
        struct NETIO_NetmaskSeq *netmask)
{
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;

    OSAPI_PRECONDITION(((netio_intf == NULL) || (address == NULL) || (netmask == NULL)),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("address", address, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("netmask", netmask, RTI_TRUE);)

    return NETIO_Interface_get_route_table(self->user_netio, address, netmask);
}

/*ci
 * \brief Check if an address is reachable by a given notification interface
 *
 * \details
 * Implementation of the NETIO is_address_reachable function.
 *
 * \param[in] netio_intf    The notification interface
 * \param[in] addr          The address
 * \param[out] is_reachable RTI_TRUE if the address is reachable, RTI_FALSE otherwise
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifInterface_is_address_reachable(
        struct NETIO_Interface *netio_intf,
        const struct NETIO_AddressEx *const addr,
        RTI_BOOL *is_reachable)
{
    struct ZCOPY_Guid notif_guid;
    struct ZCOPY_NotifInterface *self = (struct ZCOPY_NotifInterface *)netio_intf;
    struct ZCOPY_Guid owner_key;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    RTI_UINT8 zero_key[16] = {0};
    DDS_BuiltinTopicKey_t key = DDS_BuiltinTopicKey_t_INITIALIZER;

    OSAPI_PRECONDITION(
            (addr == NULL) || (is_reachable == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("addr", addr, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("is_reachable", is_reachable, RTI_TRUE);)

    UNUSED_ARG(netio_intf);

    ZCOPY_Guid_set_from_netio_addr(&notif_guid, addr);

    *is_reachable = ZCOPY_Notifiee_exists(&notif_guid, addr->port);

    if (OSAPI_Memory_compare(addr->data,zero_key,NETIO_ADDRESS_MAX_8BIT) != 0)
    {
        /* If data field is filled we also have the key to the writer
         * Check if we can reach the shared queue
         */
        OSAPI_Memory_copy(&key,addr->data,NETIO_ADDRESS_MAX_8BIT);
        /* set guid from key below takes care of the endianness */
        NETIO_Address_set_guid_from_key(&src_writer, 0, (struct NETIO_AddressInt32*)&key);
        NETIO_Address_set_kind(&src_writer, NETIO_ADDRESS_KIND_NOTIF, 0);
        OSAPI_Memory_copy(owner_key.value, &src_writer.value.guid, NETIO_ADDRESS_MAX_8BIT);
        *is_reachable = NETIO_ZCOPY_SharedQ_can_attach(
                            &owner_key, (RTI_UINT32)self->property.domain_id);
    }

    return RTI_TRUE;
}

RTI_PRIVATE void
Notification_Interface_get_transport_properties(struct NETIO_Interface *netio_intf,
                                       struct NETIO_TransportProperty *properties)
{
    UNUSED_ARG(netio_intf);
    /* The assumption these cannot be negative */
    properties->recv_size_max = (RTI_UINT32)65535;
    properties->send_size_max = (RTI_UINT32)65535;
}

/******************************************************************************
 *
 * Notification Component Interface
 */
/*ci
 * \brief The NETIO Notification interface implementation
 *
 * \details
 *
 * The Notification interface is always at the bottom of the stack, thus receive
 * is not implemented. It also does not receive any events.
 */
RTI_PRIVATE struct NETIO_InterfaceI ZCOPY_NotifInterface_fv_Intf =
{
    ._parent = RT_COMPONENTI_BASE,
    .send = ZCOPY_NotifInterface_send,
    .xmit_remove = ZCOPY_NotifInterface_xmit_remove,
    .add_route = ZCOPY_NotifInterface_add_route,
    .delete_route = ZCOPY_NotifInterface_delete_route,
    .reserve_address = ZCOPY_NotifInterface_reserve_address,
    .bind = ZCOPY_NotifInterface_bind,
    .unbind = ZCOPY_NotifInterface_unbind,
    .bind_external = ZCOPY_NotifInterface_bind_external,
    .unbind_external = ZCOPY_NotifInterface_unbind_external,
    .set_state = ZCOPY_NotifInterface_set_state,
    .release_address = ZCOPY_NotifInterface_release_address,
    .resolve_address = ZCOPY_NotifInterface_resolve_address,
    .get_route_table = ZCOPY_NotifInterface_get_route_table,
    .is_address_reachable = ZCOPY_NotifInterface_is_address_reachable,
    .get_properties = Notification_Interface_get_transport_properties,

    /* Notification interface does not support the following operations */
    .acknack = NULL,
    .request = NULL,
    .return_loan = NULL,
    .receive = NULL,
    .get_external_interface = NULL,
    .post_event = NULL,
    .lookup_route = NULL,
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

RTI_PRIVATE struct RT_ComponentFactoryI ZCOPY_NotifInterfaceFactory_fv_Intf;

/*ci
 * \brief Create an instance of the Notification interface.
 *
 * \details
 * Implementation of the RT ComponentFactory create component method. This
 * method creates a new instance of the Notification interface. It is never
 * called directly, only via the component factory interface.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new instance of the Notification interface on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T *
ZCOPY_NotifInterfaceFactory_create_component(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentProperty *property,
        struct RT_ComponentListener *listener)
{
    struct ZCOPY_NotifInterfaceFactory *self =
            (struct ZCOPY_NotifInterfaceFactory *)factory;
    struct ZCOPY_NotifInterface *notif_intf = NULL;
    struct ZCOPY_NotifInterfaceProperty *notif_prop =
            (struct ZCOPY_NotifInterfaceProperty *)property;
    struct NETIO_InterfaceListener *notif_listener =
            (struct NETIO_InterfaceListener *)listener;
    RT_Component_T *result = NULL;

    OSAPI_PRECONDITION_ALWAYS((factory == NULL) || (property == NULL),
                       return NULL,
                       OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&notif_intf, struct ZCOPY_NotifInterface);
    if (notif_intf == NULL)
    {
        ZCOPY_LOG_NOTIF_ALLOC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!ZCOPY_NotifInterface_initialize(notif_intf, self, notif_prop, notif_listener))
    {
        ZCOPY_LOG_NOTIF_INTF_INIT(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    ++self->instance_counter;
    result = &notif_intf->_parent._parent;

done:
#ifndef RTI_CERT
    if ((result == NULL) && (notif_intf != NULL))
    {
        OSAPI_Heap_free_struct(notif_intf);
    }
#endif /* !RTI_CERT */
    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the Notification interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This
 * method deletes an instance of the Notification interface. It is never
 * called directly, only via the component factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref ZCOPY_NotifInterfaceFactory_create_component
 */
RTI_PRIVATE void
ZCOPY_NotifInterfaceFactory_delete_component(
        struct RT_ComponentFactory *factory,
        RT_Component_T *component)
{
    struct ZCOPY_NotifInterfaceFactory *self =
            (struct ZCOPY_NotifInterfaceFactory *)factory;
    struct ZCOPY_NotifInterface *notif_intf = (struct ZCOPY_NotifInterface *)component;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS(
            (factory == NULL) || (component == NULL), return;
            , OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("component", component, RTI_TRUE);)

    /* delete_component() is only called during DDS_DomainParticipant_finalize,
     * where we hold the database lock.
     */
    ok = ZCOPY_NotifInterface_finalize(notif_intf, RTI_TRUE);
    if (!ok)
    {
        ZCOPY_LOG_NOTIF_INTF_FINALIZE(OSAPI_LOGKIND_ERROR)
        return;
    }

    --self->instance_counter;
    OSAPI_Heap_free_struct(notif_intf);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the Notification interface factory
 *
 * \details
 * Notification specific implementation of the RT ComponentFactory initialize
 * method. This method is never called directly. It is called by the
 * RT when the Notification factory is registered.
 *
 * \param[in] property The property the factory was registered with
 * \param[in] listener The listener the factory was registered with
 *
 * \return A fully initialized factory on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory *
ZCOPY_NotifInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    struct RT_ComponentFactory *result = NULL;
    struct ZCOPY_NotifInterfaceFactory *factory = NULL;
    struct ZCOPY_NotifInterfaceFactoryProperty *f_property =
            (struct ZCOPY_NotifInterfaceFactoryProperty *)property;

    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS(
            (property == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)


    OSAPI_Heap_allocate_struct(&factory, struct ZCOPY_NotifInterfaceFactory);
    if (factory == NULL)
    {
        ZCOPY_LOG_NOTIF_ALLOC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    factory->_parent._factory = &factory->_parent;
    factory->_parent.intf = &ZCOPY_NotifInterfaceFactory_fv_Intf;

    factory->property = *f_property;
    factory->instance_counter = 0;

    result = (struct RT_ComponentFactory *)factory;
done:
    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the Notification interface factory
 *
 * Notification specific implementation of the RT ComponentFactory finalize
 * method. This method is never called directly. It is called by the
 * RT when the Notification factory is unregistered.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref ZCOPY_NotifInterfaceFactory_initialize
 */
RTI_PRIVATE void
ZCOPY_NotifInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    struct ZCOPY_NotifInterfaceFactory *self =
            (struct ZCOPY_NotifInterfaceFactory *)factory;

    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS(
            (factory == NULL),
            return,
            OSAPI_Log_entry_add_pointer("factory", factory, RTI_TRUE);)

    /* Do some consistency checking */
    if (self->instance_counter != 0)
    {
        ZCOPY_LOG_NOTIF_FACTORY_IN_USE(OSAPI_LOGKIND_ERROR)
    }
    else
    {
        OSAPI_Heap_free_struct(self);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
RTI_PRIVATE struct RT_ComponentFactoryI ZCOPY_NotifInterfaceFactory_fv_Intf = {
        .id = NOTIF_INTERFACE_INTERFACE_ID,
        .initialize = ZCOPY_NotifInterfaceFactory_initialize,
        .create_component = ZCOPY_NotifInterfaceFactory_create_component,
#ifndef RTI_CERT
        .finalize = ZCOPY_NotifInterfaceFactory_finalize,
        .delete_component = ZCOPY_NotifInterfaceFactory_delete_component,
#endif

        /* The Notification interface factory does not support the following operations */
#ifdef RTI_CERT
        .finalize = NULL,
        .delete_component = NULL,
#endif
        .get_if = NULL,
        .get_property = NULL,
};

MUST_CHECK_RETURN RTI_BOOL
ZCOPY_NotifInterfaceFactory_register(
        RT_Registry_T *registry,
        const char *name,
        struct ZCOPY_NotifInterfaceFactoryProperty *property)
{
    struct ZCOPY_NotifUserInterfaceI *user_intf = NULL;

    OSAPI_PRECONDITION_ALWAYS(
            (registry == NULL) || (name == NULL) || (property == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("registry", registry, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name", name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    user_intf = property->user_intf;

    if ((user_intf == NULL) || (user_intf->create_instance == NULL) ||
#ifndef RTI_CERT
        (user_intf->delete_instance == NULL) ||
#endif
        (user_intf->resolve_address == NULL) || (user_intf->get_route_table == NULL) ||
        (user_intf->reserve_address == NULL) || (user_intf->release_address == NULL) ||
        (user_intf->add_route == NULL) || (user_intf->delete_route == NULL) ||
        (user_intf->bind == NULL) || (user_intf->unbind == NULL) ||
        (user_intf->send == NULL) || (user_intf->notify_recv_port == NULL))
    {
        return RTI_FALSE;
    }

    return RT_Registry_register(
            registry,
            name,
            &ZCOPY_NotifInterfaceFactory_fv_Intf,
            (struct RT_ComponentFactoryProperty *)property,
            NULL);
}

MUST_CHECK_RETURN RTI_BOOL
ZCOPY_NotifInterfaceFactory_unregister(RT_Registry_T *registry, const char *name)
{
    RT_ComponentFactory_T *c_factory = NULL;

    OSAPI_PRECONDITION_ALWAYS(
            (registry == NULL) || (name == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("registry", registry, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name", name, RTI_TRUE);)

    c_factory = RT_Registry_lookup(registry, name);
    if (c_factory == NULL)
    {
        return RTI_FALSE;
    }

    if (c_factory->intf != &ZCOPY_NotifInterfaceFactory_fv_Intf)
    {
        return RTI_FALSE;
    }

    return RT_Registry_unregister(registry, name, NULL, NULL);
}

/*e
 * \dref_ZCOPY_NotifInterfaceFactory_get_interface
 */
MUST_CHECK_RETURN NETIO_ZCOPYDllExport struct RT_ComponentFactoryI*
ZCOPY_NotifInterfaceFactory_get_interface(void)
{
    return &ZCOPY_NotifInterfaceFactory_fv_Intf;
}



/*ci @} */
