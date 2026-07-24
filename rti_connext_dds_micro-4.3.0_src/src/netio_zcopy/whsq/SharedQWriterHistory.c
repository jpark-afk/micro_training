
/*
 * FILE: SharedQWriterHistory.c - Shared Queue Writer History implementation
 *
 * Copyright 2022-2023 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "SharedQWriterHistory.h"

const char* const  DDSHST_WRITER_WRAPPED_HISTORY_NAME = "dds_wh";

/*ci
 * \brief SQWriterHistory interface
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct DDSHST_WriterI WHSQ_HistoryI_fv_Intf;


/*** SOURCE_BEGIN ***/

/*******************************************************************************
 *                                RTI_PRIVATE API
 ******************************************************************************/
/*ci
 * \brief Helper function to convert the sample kinds understood by the shared queue.
 *
 * \param[in] wh_kind        writer history kind to convert
 *
 * \return The converted sample kind of SQ_SampleKind_t type.
 */
RTI_PRIVATE SQ_SampleKind_t
WHSQ_sq_kind_from_wh_samplekind(
    DDSHST_WriterEntryKind_T wh_kind)
{
    SQ_SampleKind_t result;

    switch (wh_kind)
    {
        case DDSHST_WRITER_ENTRY_NONE:
        {
            result = SQ_SAMPLE_KIND_NONE;
            break;
        }
        case DDSHST_WRITER_ENTRY_NORMAL:
        {
            result = SQ_SAMPLE_KIND_WRITE;
            break;
        }
        case DDSHST_WRITER_ENTRY_UNREGISTER:
        {
            result = SQ_SAMPLE_KIND_UNREGISTER;
            break;
        }
        case DDSHST_WRITER_ENTRY_DISPOSE:
        {
            result = SQ_SAMPLE_KIND_DISPOSE;
            break;
        }
        case DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE:
        {
            result = SQ_SAMPLE_KIND_UNREGISTER | SQ_SAMPLE_KIND_DISPOSE;
            break;
        }
        default:
        {
            result = SQ_SAMPLE_KIND_NONE;
            break;
        }
    }

    return result;
}


/*ci
 * \brief Handle sample removed events from the writer history
 *
 * \details
 * The DDS writer history interface includes a notification for when a sample is removed.
 * Here we intercept the callback and purge sample from the Shared Queue. We forward the
 * callback to the user provided callback if available.
 *
 *
 * \param[in] config    Configuration data passed to the history cache
 * \param[in] key       The instance the history sample belonged to
 * \param[in] sample    The removed sample
 * \param[in] sn        The sequence number for the removed entry
 * \param[in] kind      The reason for the sample being removed
 * \param[in] ack_count The number of outstanding acknowledgments (only
 *                      applicable for reliable communication)
 */
RTI_PRIVATE void
WHSQ_DataWriterEvent_on_sample_removed(
        void *config,
        DDS_InstanceHandle_t *key,
        struct DDSHST_WriterSample *sample,
        struct REDA_SequenceNumber *sn,
        DDSHST_WriterSampleRemovedKind_T kind,
        DDS_Long ack_count)
{
    struct WHSQ_History *history = (struct WHSQ_History *)config;
    RTI_BOOL ret;

    /* Purge this sample from the SharedQ */
    ret = NETIO_ZCOPY_SharedQWriter_purge(history->sq_writer, sample->sample_index);
#if OSAPI_ENABLE_LOG
    if (ret == RTI_FALSE)
    {
        /* should never happen defensive programming*/
        WHSQ_LOG_PURGE_FAILED(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(ret);
#endif
    /* Forward to the original listener */
    if (NULL != history->listener_orig.on_sample_removed)
    {
        history->listener_orig.on_sample_removed(
                history->listener_orig.listener_data, key, sample,
                sn, kind, ack_count);
    }
}

/*ci
 * \brief Handle key removed events from the writer history
 *
 * \details
 * This function forwards the callback to the user provided
 *
 * \param[in] config    Configuration data passed to the history cache
 * \param[in] key       The instance that was removed
 * \param[in] kind      The reason for the key being removed
 */
RTI_PRIVATE void
WHSQ_DataWriterEvent_on_key_removed(
        void *config,
        DDS_InstanceHandle_t *key,
        DDSHST_WriterKeyRemovedKind_T kind)
{
    struct WHSQ_History *history = (struct WHSQ_History *)config;
    history->listener_orig.on_key_removed(
            history->listener_orig.listener_data, key, kind);
}

/*ci
 * \brief Handle deadline missed events from the writer history
 *
 * \details
 * The DDS writer history interface includes a notification for when
 * a key misses its deadline in the history cache.
 *
 * \param[in] config  Configuration data passed to the history cache
 * \param[in] key            The instance the deadline was missed on
 */
RTI_PRIVATE void
WHSQ_DataWriterEvent_on_deadline_missed(
        void *config,
        DDS_InstanceHandle_t *key)
{
    struct WHSQ_History *history = (struct WHSQ_History *)config;
    history->listener_orig.on_deadline_missed(
            history->listener_orig.listener_data, key);
}


/*ci
 * \brief Implementation of the get_entry function.
 *
 * \details
 * Gets an entry from the history cache. The function also fills in the relevant
 * details in the intercepted_sample_info field which is to be used later during
 * committing. Only one outstanding entry to be commited is allowed
 *
 *
 * \param[in] wh            shared queue writer history
 * \param[in] key           The key to get an entry for
 * \param[in] kind          The kind of entry (dispose, unregister, normal etc.)
 * \param[in] assert_key    Boolean to determine if the key is asserted if it
 *                          does not exist.
 * \param[inout] source_ts  The timestamp the entry was reserved
 * \param[out]   ec         Additional error code in case NULL is returned.
 *
 * \return A reference to new entry in the Data Writer History
 */
RTI_PRIVATE DDSHST_WriterSampleEntryRef_T
WHSQ_History_get_entry(
        struct DDSHST_Writer *wh,
        const DDS_InstanceHandle_t *const key,
        DDSHST_WriterEntryKind_T kind,
        RTI_BOOL assert_key,
        struct OSAPI_SystemTime *source_ts,
        DDSHST_WriterErrorKind_T *ec)
{
    DDSHST_WriterSampleEntryRef_T entry_ref = NULL;
    struct WHSQ_History *self = (struct WHSQ_History *)wh;

    /* depending on the value of destination_order.source_timestamp_tolerance,
     * the value of source_ts may be modified by DDSHST_Writer_get_entry()
     */
    entry_ref = DDSHST_Writer_get_entry(
            self->dds_wh, key, kind, assert_key, source_ts, ec);
    if (entry_ref == NULL)
    {
        return NULL;
    }

    /* Save the sample info wh to be used later */
    self->intercepted_sample_info.timestamp.sec = source_ts->sec;
    self->intercepted_sample_info.timestamp.nanosec = source_ts->nanosec;
    OSAPI_Memory_copy(&self->intercepted_sample_info.key.value[0],
            key->octet,ZCOPY_GUID_LENGTH);
    self->intercepted_sample_info.kind = WHSQ_sq_kind_from_wh_samplekind(kind);

    return entry_ref;
}

/*ci
 * \brief Implementation of the return entry function.

 * \details
 * Returns the entry to the history cache.
 *
 * \param[in] wh            shared queue writer history
 * \param[in] entry         The entry being returned
 *
 */
RTI_PRIVATE void
WHSQ_History_return_entry(
        struct DDSHST_Writer *wh,
        DDSHST_WriterSampleEntryRef_T const entry)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    DDSHST_Writer_return_entry(self->dds_wh, entry);
}


/*ci
 * \brief Implementation of the commit_entry function.

 * \details
 * This function commits a previously allocated entry into the history cache. Commits
 * the entry to the Shared Queue and then commits the sample to the history cache.
 *
 * \param[in] wh            shared queue writer history
 * \param[in] entry         The reference to a previously allocated entry in the history cache
 * \param[in] sample        The entries payload
 * \param[in] sn            The sequence number of the entry
 * \param[in] ack_count     The expected number of acknowledgment for this sample based
 *                          on the number of reliable peers
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the other \ref
 *         DDSHST_ReturnCode_T on failure
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_commit_entry(
        struct DDSHST_Writer *wh,
        DDSHST_WriterSampleEntryRef_T const entry,
        DDSHST_WriterSample_T *const sample,
        const struct REDA_SequenceNumber *const sn,
        DDS_Long ack_count)
{
    DDSHST_ReturnCode_T result = DDSHST_RETCODE_ERROR;
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    RTI_BOOL ret = RTI_FALSE;

    self->intercepted_sample_info.seq_nr.high =  sn->high;
    self->intercepted_sample_info.seq_nr.low =  sn->low;

    if (!NETIO_ZCOPY_SharedQWriter_commit(self->sq_writer,
                                            sample->sample_index,
                                            &self->intercepted_sample_info))
    {
        /* This should not happen */
        WHSQ_LOG_COMMIT_FAILED(OSAPI_LOGKIND_ERROR);
        return DDSHST_RETCODE_ERROR;
    }

    result = DDSHST_Writer_commit_entry(self->dds_wh,
            entry, sample, sn, ack_count);
    if (result != DDSHST_RETCODE_SUCCESS)
    {
        ret = NETIO_ZCOPY_SharedQWriter_purge(self->sq_writer, sample->sample_index);
#if OSAPI_ENABLE_LOG
        if (ret == RTI_FALSE)
        {
            /*This should not happen*/
            WHSQ_LOG_PURGE_FAILED(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(ret);
#endif
    }
    return result;
}

/*ci
 * \brief Implementation of the request sample function.
 *        Forwards to the underlying writer history.

 * \param[in]  wh              The history cache
 * \param[out] sample          The requested sample if found, NULL otherwise
 * \param[in]  sn              The requested SN
 * \param[out] sn_ge           The first available SN
 * \param[in]  historical_only Whether only historical samples are relevant
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref DDSHST_RETCODE_SUCCESS
 *         on failure
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_request_sample(
        struct DDSHST_Writer *wh,
        struct DDSHST_WriterSample **sample,
        const struct REDA_SequenceNumber *const sn,
        struct REDA_SequenceNumber *const sn_ge,
        DDS_Boolean historical_only)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_request_sample(self->dds_wh, sample, sn, sn_ge, historical_only);
}

/*ci
 * \brief Implementation of the acknack_sample function.
 *        Forwards the function to the underlying writer history
 *
 * \param[in] wh   The writer cache
 * \param[in] sn   The SN to acknack
 * \param[in] nack Whether this is a NACK (DDS_BOOLEAN_TRUE) or ACK
 *                 (DDS_BOOLEAN_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, of of the \ref
 *         DDSHST_ReturnCode_T on failure
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_acknack_sample(
        struct DDSHST_Writer *wh,
        const struct REDA_SequenceNumber *const sn,
        DDS_Boolean nack)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_acknack_sample(self->dds_wh, sn, nack);
}

/*ci
 * \brief Implementation of the register_key function.
 *         Forwards the function to the underlying writer history   =
 *
 * \param[in] wh  The history cache
 * \param[in] key The key to add to the cache
 * \param[in] timestamp The time the entry was registered, used with source order
 *                       timestamps.
 *
 * \return DDSHST_RETCODE_SUCCESS on success, of of the \ref
 *         DDSHST_ReturnCode_T on failure
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_register_key(
        struct DDSHST_Writer *wh,
        const DDS_InstanceHandle_t *const key,
        const struct OSAPI_SystemTime *timestamp)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_register_key(self->dds_wh, key, timestamp);
}

/*ci
 * \brief Implementation of the get_state function.
 *         Forwards the function to the underlying writer history
 *
 * \param[in] wh The history cache
 *
 * \return Pointer to the structure containing the state
 */
RTI_PRIVATE struct DDSHST_WriterState*
WHSQ_History_get_state(struct DDSHST_Writer *wh)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_get_state(self->dds_wh);
}

/*ci
 * \brief Implementation of the post_Event function.
 *         Forwards the function to the underlying writer history
 *
 * \param[in] wh    The history cache
 * \param[in] event The event
 * \param[in] now   The timestamp for the event
 *
 */
RTI_PRIVATE void
WHSQ_History_post_event(
        struct DDSHST_Writer *wh,
        struct DDSHST_WriterEvent *event,
        struct OSAPI_SystemTime *now)
{
    RTI_BOOL retval;
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    switch (event->kind)
    {
        case DDSHST_WRITEREVENT_KIND_LIVELINESS_ASSERTED:
            /* This return value is ignored because WHSQ_History_post_event()
             * returns void and can't propagate the potential error upstream.
             */
            retval = NETIO_ZCOPY_SharedQWriter_pulse(self->sq_writer,now);
            IGNORE_RETVAL(retval);
        default:
            break;
    }

    DDSHST_Writer_post_event(self->dds_wh, event, now);
}

/*ci
 * \brief Implementation of the get_instance_state function.
 *         Forwards the function to the underlying writer history
 *
 * \param[in] wh  The history cache
 * \param[in] key   The key to retrieve state for
 * \param[in] state The current state of an instance, if found
 *
 * \return DDSHST_RETCODE_NOT_EXISTS if the instance does not exist,
 *         DDSHST_RETCODE_OK if the instance exists
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_get_instance_state(
        struct DDSHST_Writer *wh,
        const DDS_InstanceHandle_t *const key,
        struct DDSHST_InstanceState *state)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_get_instance_state(self->dds_wh, key, state);
}

/*ci
 * \brief Implementation of the unregister_key function.
 *         Forwards the function to the underlying writer history
 *
 *
 * \param[in]  wh  The history cache
 * \param[in]  key   The key to unregister
 *
 * \return DDSHST_RETCODE_OK on success, DDSHST_RETCODE_NOT_EXISTS if the
 *         instance does not exist.
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_unregister_key(
        struct DDSHST_Writer *wh,
        const DDS_InstanceHandle_t *const key)
{
    struct WHSQ_History *self = (struct WHSQ_History *)wh;
    return DDSHST_Writer_unregister_key(self->dds_wh, key);
}

/*ci
 * \brief Delete an instance of the history cache
 *
 * \param[in] self The history cache to delete
 * \param[in] whsq_factory TThe factory which created this cache.
 */
#ifndef RTI_CERT
RTI_PRIVATE DDSHST_ReturnCode_T
WHSQ_History_delete(
        struct DDSHST_Writer *self,
         struct WHSQ_HistoryFactory *whsq_factory)

{
    if (self != NULL)
    {
        /* Only destroy components that I created myself */
        if (RT_ComponentFactory_get_id(&whsq_factory->_parent) ==
                (RTI_INT32)self->_parent.id)
        {
            struct WHSQ_History *whsq = (struct WHSQ_History *)self;

            /* We encapsulated the creation, encapsulate the destruction as well */
            if (whsq->dds_wh != NULL)
            {
                DDSHST_WriterFactory_delete_component(
                        whsq_factory->dds_wh_factory, &whsq->dds_wh->_parent);
            }
            OSAPI_Heap_free(self);

        }
        else
        {
            /* We proxied the creation, proxy the destruction as well */
            DDSHST_WriterFactory_delete_component(
                    whsq_factory->dds_wh_factory, &self->_parent);

        }
        self = NULL;
    }
    return DDSHST_RETCODE_SUCCESS;
}

#endif

/*ci
 * \brief Create a new instance of the WHSQ history cache
 *
 * \details
 * This function creates a new instance of the history cache.The WHSQ
 * can create itself itself depending on the properties but will always
 * try to create the underlying writer history.
 *
 * It should be noted that it is possible that the qos policy contains legal
 * and consistent DDS values, but may be unsupported by the cache. In this
 * case the cache will return NULL and log an error (if logging is enabled).
 *
 * \param[in] property The WHSM properties, cannot be NULL
 * \param[in] listener The writer history listener
 *
 * \return Pointer to new instance of the history cache on success,
 *         NULL on failure.
 */
RTI_PRIVATE struct DDSHST_Writer*
WHSQ_History_create(
    struct WHSQ_HistoryFactory* whsq_factory,
    struct DDSHST_WriterProperty* property,
    struct DDSHST_WriterListener* listener)
{
    struct DDSHST_Writer *retval = NULL;
    struct DDSHST_Writer *dds_history = NULL;
    struct WHSQ_History *whsq = NULL;
    struct DDSHST_WriterListener dds_wh_listener = DDSHST_WriterListener_INITIALIZE;
    RTI_BOOL create_self = RTI_FALSE;

    if (NULL == whsq_factory->dds_wh_factory)
    {
        WHSQ_LOG_NO_DDS_FACTORY(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if ((property->type_plugin != NULL) &&
        (DDS_TypePlugin_get_allocator_plugin_memory_kind(
            property->type_plugin) == RTI_MEMORY_MANAGER_SHMEMV2))
    {
        create_self = RTI_TRUE;
    }


    /* if creating self (notif is being used) fail since KEEP_ALL is not supported */
    if ((create_self == RTI_TRUE) && (property->history.kind != DDS_KEEP_LAST_HISTORY_QOS))
    {
        WHSQ_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (property->resource_limits.max_samples == DDS_LENGTH_UNLIMITED)
    {
        WHSQ_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (create_self == RTI_TRUE)
    {
        OSAPI_Heap_allocate_struct(&whsq, struct WHSQ_History);
        if (whsq == NULL)
        {
            WHSQ_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSQ_LOG_HISTORY_OBJECT)
            goto done;
        }

        RT_Component_initialize(&whsq->_parent._parent,
                            &WHSQ_HistoryI_fv_Intf._parent,
                            (RTI_UINT32)RT_ComponentFactory_get_id(&whsq_factory->_parent),
                            &property->_parent,
                            &listener->_parent);

        whsq->property._parent = *property;
        whsq->listener_orig = *listener;
        whsq->sq_writer = ((struct ZCOPY_ShmV2Plugin*)property->type_plugin->allocator_plugin)->sq_writer;

        dds_wh_listener.on_sample_removed = WHSQ_DataWriterEvent_on_sample_removed;
        dds_wh_listener.on_key_removed =
            listener->on_key_removed ? WHSQ_DataWriterEvent_on_key_removed : NULL;
        dds_wh_listener.on_deadline_missed =
            listener->on_deadline_missed ? WHSQ_DataWriterEvent_on_deadline_missed : NULL;
        dds_wh_listener.listener_data = whsq;
        dds_history = DDSHST_WriterFactory_create_component(whsq_factory->dds_wh_factory,
                &property->_parent, &dds_wh_listener._parent);
        if (NULL == dds_history)
        {
            WHSQ_LOG_WH_CREATE_FAILED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        whsq->dds_wh = dds_history;
        retval = &whsq->_parent;
    }
    else
    {
        /* No need to insert ourselves, just forward the parameters
         * to the other factory */
        dds_history = DDSHST_WriterFactory_create_component(
                whsq_factory->dds_wh_factory, &property->_parent,
                &listener->_parent);
        retval = dds_history;
    }

done:
#ifndef RTI_CERT
    if (retval == NULL)
    {
        if (NULL != dds_history)
        {
            WHSQ_History_delete(dds_history,whsq_factory);
        }
        if (NULL != whsq)
        {
            WHSQ_History_delete(&whsq->_parent,whsq_factory);
        }
    }
#endif
    return retval;
}


/*********************** MicroDDS COMPONENT Interface *************************/

/*ci
 * \brief Implementation of the DDSHST_WriterI interface
 */
RTI_PRIVATE struct DDSHST_WriterI WHSQ_HistoryI_fv_Intf =
{
    ._parent = RT_COMPONENTI_BASE,
    .get_entry = WHSQ_History_get_entry,
    .return_entry = WHSQ_History_return_entry,
    .commit_entry = WHSQ_History_commit_entry,
    .request_sample = WHSQ_History_request_sample,
    .acknack_sample = WHSQ_History_acknack_sample,
    .register_key = WHSQ_History_register_key,
    .get_state = WHSQ_History_get_state,
    .post_event = WHSQ_History_post_event,
    .get_instance_state = WHSQ_History_get_instance_state,
    .unregister_key = WHSQ_History_unregister_key
};


/*ci
 * \brief Create a new instance of the writer history cache
 *
 * \details
 * Implementation of the RT ComponentFactory create component method
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 *
 * \sa \ref WHSQ_HistoryFactory_delete_component
 */
RTI_PRIVATE RT_Component_T*
WHSQ_HistoryFactory_create_component(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentProperty *property,
        struct RT_ComponentListener *listener)
{
    RT_Component_T *result = NULL;
    struct DDSHST_Writer *writer = NULL;
    struct WHSQ_HistoryFactory *self = (struct WHSQ_HistoryFactory *)factory;
    struct DDSHST_WriterProperty *wh_property = (struct DDSHST_WriterProperty *)property;
    struct DDSHST_WriterListener *wh_listener = (struct DDSHST_WriterListener *)listener;

    OSAPI_PRECONDITION_ALWAYS(
                        (factory == NULL || property == NULL || listener == NULL),
                        goto done,
                        OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("listener", listener, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    writer = WHSQ_History_create(self, wh_property, wh_listener);
    if (NULL == writer)
    {
        goto done;
    }

    result = (RT_Component_T*)writer;

done:
    return result;
}

#ifndef RTI_CERT

/*ci
 * \brief Delete an instance of the history cache.
 *
 * \details
 * Implementation of the RT ComponentFactory delete method
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref WHSQ_HistoryFactory_create_component
 */
RTI_PRIVATE void
WHSQ_HistoryFactory_delete_component(
        struct RT_ComponentFactory *factory,
        RT_Component_T *component)
{
    struct WHSQ_HistoryFactory *self =
            (struct WHSQ_HistoryFactory *)factory;
    struct DDSHST_Writer *writer = (struct DDSHST_Writer *)component;

    WHSQ_History_delete(writer, self);
}


/*ci
 * \brief Finalize the writer history factory
 *
 * \details
 * Implementation of the RT ComponentFactory finalize method. This method
 * is called when the writer history factory is unregistered from the RT.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref WHSQ_HistoryFactory_initialize
 */
RTI_PRIVATE void
WHSQ_HistoryFactory_finalize(struct RT_ComponentFactory *factory,
                    struct RT_ComponentFactoryProperty **property,
                    struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}

#endif /*RTI_CERT*/

RTI_PRIVATE struct RT_ComponentFactory*
WHSQ_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener);


/*ci
 * \brief Implementation of the RT_ComponentFactoryI interface
 */
RTI_PRIVATE struct RT_ComponentFactoryI WHSQ_HistoryFactory_fv_Intf =
{
    .id = WHSQ_HISTORY_INTERFACE_ID,
    .initialize = WHSQ_HistoryFactory_initialize,
    .create_component = WHSQ_HistoryFactory_create_component,
#ifndef RTI_CERT
    .finalize = WHSQ_HistoryFactory_finalize,
    .delete_component = WHSQ_HistoryFactory_delete_component,
#else
    /* WHSQ does not support the following operations */
    .finalize = NULL,
    .delete_component = NULL,
#endif /*RTI_CERT*/
    .get_if = NULL,
    .get_property = NULL,
};


RTI_PRIVATE struct WHSQ_HistoryFactory WHSQ_HistoryFactory_fv_Factory =
{
    ._parent = {
        .intf = &WHSQ_HistoryFactory_fv_Intf,
        ._factory = NULL,  /* filled during factory initialization */
        ._id = {{{0,0}}},
    },
    .dds_wh_factory = NULL  /* filled during factory initialization */
};


/*ci
 * \brief Method to initialize the writer history factory
 *
 * \details
 * WHSQ specific implementation of the RT ComponentFactory initialize method.
 * This method is called when the writer history factory is registered with the
 * RT. Looks up the factory for the underlying writer history from the property.
 *
 * \param[in] property The properties registered with the history interface
 * \param[in] listener The listener registered with the history interface
 *
 * \return A fully initialized factory
 *
 * \sa \ref WHSQ_HistoryFactory_finalize
 */
RTI_PRIVATE struct RT_ComponentFactory*
WHSQ_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener)
{
    struct WHSQ_HistoryFactoryProperty *whsq_prop =
        (struct WHSQ_HistoryFactoryProperty *)property;
    struct RT_ComponentFactory *dds_wh_factory = NULL;
    RT_Registry_T *registry = RT_Registry_get_instance();

    UNUSED_ARG(listener);
    if (whsq_prop == NULL)
    {
        return NULL;
    }

    if (registry == NULL)
    {
        return NULL;
    }

    dds_wh_factory = RT_Registry_lookup(registry,
                                        whsq_prop->dds_wh_factory_name);

    if (dds_wh_factory == NULL)
    {
        return NULL;
    }

    WHSQ_HistoryFactory_fv_Factory._parent._factory = &WHSQ_HistoryFactory_fv_Factory._parent;
    WHSQ_HistoryFactory_fv_Factory.dds_wh_factory = dds_wh_factory;

    return &WHSQ_HistoryFactory_fv_Factory._parent;
}


struct RT_ComponentFactoryI*
WHSQ_HistoryFactory_get_interface(void)
{
    return &WHSQ_HistoryFactory_fv_Intf;
}
