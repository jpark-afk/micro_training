/*
 * FILE: History.c - Writer History implementation
 *
 * Copyright 2011-2023 Real-Time Innovations, Inc.
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
 * 22may2023,tk MICRO-4930/PR.31592
 * - Handle BEST_EFFORT and TRANSIENT_LOCAL as VOLATILE in
 *   WHSM_History_commit_entry to avoid exhausting samples
 *   resources.
 * 14sep2021,tk MICRO-3229/PR.29586
 * - Changed expression in WHSM_History_analyze_sample_state to explicitly test
 *   that (sample == key_entry->last_history_sample) also for
 *   (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL) instead
 *   of implicitly assuming so even if it is always TRUE.
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 05may2017,tk MICRO-1590/PR#20076 Do not free historical unregister samples
 * 22feb2017,tk MICRO-1583/PR#19776 Do not delete a key if the unregister was
 *                                  pruned since that means the instance is
 *                                  still in use.
 * 21sep2016,tk MICRO-1546/PR#18501 Remove a key if an empty instance is
 *                                  unregistered and transient_local
 * 27jun2016,tk MICRO-1546 Remove a key if an empty instance is unregistered
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 27jul2015,tk MICRO-1464/PR#15597 Only remove an uregistered instance from 
 *                                  the cache
 * 27jul2015,tk MICRO-1426/PR#15358 Check the elapsed time in the periodic
 *                                  timeout without epoch
 * 25jul2015,eh MICRO-1461/PR#15579 Fix update of state's low and high sn
 * 20jul2015,tk MICRO-1426/PR#15358 Check the elapsed time in the periodic
 *                                  timeout
 * 15jul2015,tk MICRO-1426/PR#15358 Properly initialize a key's last update
 *                                  period
 * 15jul2015,tk MICRO-1426/PR#15358 Check deadline period on each instance
 *                                  update in addition to the deadline timeout
 * 24jun2015,eh MICRO-1169/PR#14650 Further remove property check from create
 * 05may2015,tk MICRO-1167/PR#14642 Simplified code in on_deadline_expired()
 * 14may2015,eh MICRO-1169/PR#14650 Remove redundant property check from create
 * 20feb2014,eh MICRO-813/PR#9172   Fix Lint warnings
 * 31jul2014,tk MICRO-172/PR#1064   Removed superfluous REDA_Indexer fields
 * 03mar2014,eh MICRO-714: Support  ack/nack
 * 03feb2014,eh MICRO-714: Fix ack  and nack behavior
 * 14sep2011,tk Bug fixes and performance improvements
 * 12jun2011,tk Written
 */
/*ce
 * \file
 * \brief Writer History implementation
 *
 * \details
 * This file implements the DDS DataWriter history cache and access to it is
 * via the DDSHST_WriterI interface. It manages samples according to Qos
 * policies such as resource-limits and deadlines.
 */
/*ci \addtogroup WHSMModule
 * @{
 */
#include "History.h"

/*ci
 * \brief WriterHistory listener initializer
 */
RTI_PRIVATE const struct DDSHST_WriterListener 
RTI_DEFAULT_SMWRITERHISTORY_LISTENER = DDSHST_WriterListener_INITIALIZE;

/*ci
 * \brief WriterHistory interface
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct DDSHST_WriterI WHSM_HistoryI_fv_Intf;

SHOULD_CHECK_RETURN RTI_PRIVATE DDS_Boolean
WHSM_History_delete_key(struct WHSM_History *self,
                        struct WHSM_HistoryKeyEntry *key,
                        DDSHST_WriterKeyRemovedKind_T kind);

/*** SOURCE_BEGIN ***/

/*******************************************************************************
 *                                RTI_PRIVATE API
 ******************************************************************************/
/*ci
 * \brief REDA_Indexer_T compare function for sample sequence numbers
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just the key
 * \param[in] key           An existing indexed record or a key
 *
 * \return positive integer if record is greater than key,
 *         negative integer if record is less than key,
 *         zero if record is equal to key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
WHSM_History_indexer_sample_compare(const void *const record,
                                    RTI_BOOL key_is_record,
                                    const void *const key)
{
    struct WHSM_HistorySampleEntry *key_entry = (struct WHSM_HistorySampleEntry*)record;
    struct REDA_SequenceNumber *r_key;

    if (key_is_record)
    {
        r_key = &((struct WHSM_HistorySampleEntry*)key)->sn;
    }
    else
    {
        r_key = (struct REDA_SequenceNumber *)key;
    }

    return REDA_SequenceNumber_compare(&key_entry->sn,r_key);
}

/*ci
 * \brief REDA_Indexer_T compare function for keys
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just the key
 * \param[in] key           An existing indexed record or a key
 *
 * \return positive integer if record is greater than key,
 *         negative integer if record is less than key,
 *         zero if record is equal to key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
WHSM_History_indexer_key_compare(const void *const record,
                                 RTI_BOOL key_is_record,
                                 const void *const key)
{
    struct WHSM_HistoryKeyEntry *key_entry = (struct WHSM_HistoryKeyEntry*)record;
    DDS_InstanceHandle_t *r_key;

    if (key_is_record)
    {
        r_key = &((struct WHSM_HistoryKeyEntry*)key)->key;
    }
    else
    {
        r_key = (DDS_InstanceHandle_t *)key;
    }

    return DDS_InstanceHandle_compare(&key_entry->key,r_key);
}

/*ci
 * \brief Initialize a key entry in the history cache
 *
 * \param[in] self      The history cache
 * \param[in] key       The key entry to initialize
 * \param[in] key_value The key value to initialize the entry with
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
WHSM_History_initialize_key(struct WHSM_History *self,
                            struct WHSM_HistoryKeyEntry *key,
                            const DDS_InstanceHandle_t *const key_value)
{
    key->sample_count = 0;
    REDA_CircularList_init(&key->samples);
    key->key = *key_value;
    key->last_history_sample = NULL;
    key->state = WHSM_HISTORY_KEY_STATE_ALIVE;

    DDS_Duration_set(&key->last_update_time,0,DDS_DURATION_INFINITE_NSEC);
    DDS_Duration_set(&key->last_period_time,0,DDS_DURATION_INFINITE_NSEC);

    if (!REDA_Indexer_add_entry(self->key_index,key))
    {
        WHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEY_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    REDA_CircularListNode_init(&key->_deadline_entry);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Remove a sample from the history cache
 *
 * \param[in] self           The history cache
 * \param[in] sample         The sample to remove
 * \param[in] sample_rm_kind The reason for the sample removal
 */
SHOULD_CHECK_RETURN RTI_PRIVATE void
WHSM_History_remove_sample(struct WHSM_History *self,
        struct WHSM_HistorySampleEntry *sample,
        DDSHST_WriterSampleRemovedKind_T sample_rm_kind)
{
    REDA_CircularList_unlink_node(&sample->_node);

    /* We need to indicate if this sample was forcefully removed
     * This will result in a discard downstream so that downstream interfaces
     * will stop trying to send the sample and free any resources
     * used for it. Check if the the entry is associated with a sample in
     * case an empty queue entry is returned.
     */
    if (!(sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_RETURNED))
    {
        if (self->_listener.on_sample_removed)
        {
            self->_listener.on_sample_removed(self->_listener.listener_data,
                    &sample->key_entry->key, sample->_sample,
                    &sample->sn,sample_rm_kind,sample->ack_count);
        }

        if (REDA_Indexer_remove_entry(self->sample_index,&sample->sn) != sample)
        {
            WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR)
            return;
        }

        /* A sample will never have the HISTORICAL state unless durability kind
         * > VOLATILE
         */
        if (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL)
        {
            if (REDA_Indexer_remove_entry(self->historical_index,&sample->sn) != sample)
            {
                WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR)
                return;
            }
        }
    }

    --sample->key_entry->sample_count;
}

/*ci
 * \brief Analyze the state of a sample and update the cache as required
 *
 * \details
 * This function is called whenever the state changes on a sample. Based on the
 * state of the sample combined with QoS settings, the sample may be returned
 * to the buffer pool, kept in the history due to non-volatile history,
 * cause a key to be reclaimed or other action may be taken. The function
 * only analyzes one sample.
 *
 * \param[in] self   The history cache
 * \param[in] sample The sample to analyze
 */
SHOULD_CHECK_RETURN RTI_PRIVATE void
WHSM_History_analyze_sample_state(struct WHSM_History *self,
                                  struct WHSM_HistorySampleEntry *sample)
{
    DDSHST_WriterSampleRemovedKind_T sample_rm_kind =
                                        DDSHST_WRITER_SAMPLE_REMOVED_NORMAL;
    RTI_BOOL free_sample = RTI_FALSE;
    RTI_BOOL free_key = RTI_FALSE;
    RTI_BOOL unregister_sample = RTI_FALSE;
    RTI_BOOL sample_acknacked = RTI_FALSE;
    struct WHSM_HistoryKeyEntry *key_entry = sample->key_entry;
    struct WHSM_HistorySampleEntry *h_entry = NULL;

    if (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED)
    {
        sample_rm_kind = DDSHST_WRITER_SAMPLE_REMOVED_UNACKED;
    }
    else if (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED)
    {
        sample_rm_kind = DDSHST_WRITER_SAMPLE_REMOVED_MAX_SAMPLES_PER_INSTANCE;
    }

    /* RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL is a transition
     * state when the current last history sample is pushed out. The
     * 2nd case is when a sample is pruned from the queue.
     */
    if ((sample == key_entry->last_history_sample) &&
         ((sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL) ||
          (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED)))
    {
        /* key_entry->last_history_sample _must_ be != NULL because
         * sample is always != NULL or if the sample state is
         * RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL it means it
         * was previously historical and was pushed off the cache.
         */
        key_entry->last_history_sample->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL;

        if (REDA_Indexer_remove_entry(self->historical_index,
                              &key_entry->last_history_sample->sn) != sample)
        {
            WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR)
            return;
        }

        h_entry = (struct WHSM_HistorySampleEntry*)
                               REDA_CircularListNode_get_prev(
                                       &key_entry->last_history_sample->_node);

        if (REDA_CircularList_node_at_head(&key_entry->samples,h_entry))
        {
            key_entry->last_history_sample = NULL;
        }
        else
        {
            key_entry->last_history_sample = h_entry;
        }
    }

    sample_acknacked = ((sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED) ||
                        (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED) ?
                            RTI_TRUE : RTI_FALSE);

    /* This test handles the normal case where a sample is fully acked and
     * not historical or is forcefully removed due to keep_last
     */
    if ((sample_acknacked  &&
         !(sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL)) ||
         (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED))
    {
        free_sample = RTI_TRUE;
    }

    unregister_sample = ((sample->kind == DDSHST_WRITER_ENTRY_UNREGISTER) ||
                         (sample->kind == DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE) ?
                            RTI_TRUE : RTI_FALSE);

    /* This handles the case where the current state of the instance is not
     * alive, and the sample being analyzed is the normal unregister
     * sample, i.e has not been pruned. This overrides the case where a
     * historical sample cannot be deleted above.
     */
    if (sample->key_entry->state == WHSM_HISTORY_KEY_STATE_NOT_ALIVE)
    {
        if (!(sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED) &&
            !(sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL) &&
            unregister_sample && sample_acknacked)
        {
            free_key = RTI_TRUE;
            free_sample = RTI_TRUE;
            sample_rm_kind = DDSHST_WRITER_SAMPLE_REMOVED_NORMAL;
        }
    }

    /* Make sure this sample cannot change the last history sample again */
    sample->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL;

    if (!free_sample)
    {
        return;
    }

    WHSM_History_remove_sample(self,sample,sample_rm_kind);

    if (free_key)
    {
        /* It is not considered an error if the key cannot be deleted at this
         * point, may still be other samples left
         */
        WHSM_History_delete_key(self,sample->key_entry,
                                DDSHST_WRITER_KEY_REMOVED_UNREGISTERED);
    }

    REDA_BufferPool_return_buffer(self->sample_pool,sample);
}

/*ci
 * \brief Delete a key from the cache
 *
 * \details
 * This function deletes a key from the history cache and notifies the listener
 * if present. Note that it is illegal to delete a key with outstanding loans
 * as it may cause memory access violations.
 *
 * \param[in] self The history cache
 * \param[in] key  The key to remove from cache
 * \param[in] kind The reason the key was removed
 *
 * \return DDS_BOOLEAN_TRUE if the sample was successfully removed,
 *         DDS_BOOLEAN_FALSE if the sample could not be removed
 */
SHOULD_CHECK_RETURN RTI_PRIVATE DDS_Boolean
WHSM_History_delete_key(struct WHSM_History *self,
                        struct WHSM_HistoryKeyEntry *key,
                        DDSHST_WriterKeyRemovedKind_T kind)
{
    struct WHSM_HistorySampleEntry *sample = NULL;
    struct WHSM_HistorySampleEntry *sample2 = NULL;

    sample = (struct WHSM_HistorySampleEntry*)
                                  REDA_CircularList_get_first(&key->samples);

    while (!REDA_CircularList_node_at_head(&key->samples,&sample->_node))
    {
        sample2 = (struct WHSM_HistorySampleEntry*)
                                REDA_CircularListNode_get_next(&sample->_node);
        WHSM_History_remove_sample(self,sample,DDSHST_WRITER_SAMPLE_REMOVED_NORMAL);
        REDA_BufferPool_return_buffer(self->sample_pool,sample);
        sample = sample2;
    }

    /* Do not allow deletion of a key if there are outstanding samples */
    if (key->sample_count > 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->_listener.on_key_removed)
    {
        self->_listener.on_key_removed(self->_listener.listener_data,&key->key,
                                       kind);
    }

    REDA_CircularList_unlink_node(&key->_deadline_entry);

    REDA_Indexer_remove_entry(self->key_index,&key->key);

    REDA_BufferPool_return_buffer(self->key_pool,key);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Remove a sample from the cache if possible
 *
 * \details
 * When the cache is out of resources it may try to remove a sample from the
 * cache to receive new data. This function tries to remove a sample.
 *
 * \param[in] self      The history cache
 * \param[in] key_entry The key entry to prune a sample from
 *
 * \return DDS_BOOLEAN_TRUE if a sample was successfully removed,
 *         DDS_BOOLEAN_FALSE if a sample could not be removed
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
WHSM_History_prune_samples(struct WHSM_History *self,
                           struct WHSM_HistoryKeyEntry *const key_entry)
{
    struct WHSM_HistorySampleEntry *sample;

    if (REDA_CircularList_is_empty(&key_entry->samples))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* When pruning a sample from the queue, always chop of at the end so
     * that samples drop of at the end of the queue.
     */
    sample = (struct WHSM_HistorySampleEntry *)
                               REDA_CircularList_get_last(&key_entry->samples);

    /* A pruned sample cannot delete a key if the pruned samples is of the
     * unregister kind. Set the state here since the samples is analyzed
     * multiple places.
     */
    sample->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED;

    WHSM_History_analyze_sample_state(self,sample);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Find a sample based on the sequence number
 *
 * \param[in] self The history cache
 * \param[in] sn   The sequence number to search for
 *
 * \return The sample entry if it was found, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE struct WHSM_HistorySampleEntry*
WHSM_History_find_entry(struct WHSM_History *self,
                        const struct REDA_SequenceNumber *const sn)
{
    return (struct WHSM_HistorySampleEntry*)
                               REDA_Indexer_find_entry(self->sample_index,sn);
}

/*ci
 * \brief Find a historical sample based on the sequence number
 *
 * \details
 * This function does not perform an exact match. If the requested sequence
 * number is not found it returns the next historical sequence number if any.
 * It is up to the caller to determine if the returned sample is useful or not.
 *
 * \param[in] self The history cache
 * \param[in] sn   The sequence number to search for among the historical samples
 *
 * \return A sample entry if it was found, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE struct WHSM_HistorySampleEntry*
WHSM_History_find_history_entry(struct WHSM_History *self,
                                const struct REDA_SequenceNumber *const sn)
{
    return (struct WHSM_HistorySampleEntry *)
                   REDA_Indexer_find_entry_eq_or_gt(self->historical_index,sn);
}

/*ci
 * \brief Find a key entry in the cache
 *
 * \param[in] self The history cache
 * \param[in] key  The key to search for
 *
 * \return Pointer to the key entry if found, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE struct WHSM_HistoryKeyEntry*
WHSM_History_find_key(struct WHSM_History *self,
                      const DDS_InstanceHandle_t *const key)
{
    return (struct WHSM_HistoryKeyEntry *)
                            REDA_Indexer_find_entry(self->key_index,key);
}

#ifdef ENABLE_QOS_DEADLINE
/*ci
 * \brief Check if deadline has expired on any instance in the history cache
 *
 * \details
 * This function is called periodically based on events posted to the
 * cache. When an instance is updated its deadline period is increased and
 * the instance is moved to the end of a deadline list. This function starts
 * at the beginning of the list and checks if there are any instances in the
 * current period, if so it means the deadline was missed. NOTE: The instances
 * are updated in-place and not moved to the end of the list (MICRO-1167). This
 * simplifies the code.
 *
 * \param[in] self The history cache
 * \param[in] now  The time the event was posted
 */
RTI_PRIVATE void
WHSM_History_on_deadline_expired(struct WHSM_History *self,
                                 struct OSAPI_NtpTime *now)
{
    struct WHSM_HistoryKeyEntry *key_entry;
    struct REDA_CircularListNode *this_node;
    struct DDS_Duration_t tick_time;

    UNUSED_ARG(now);

    /* Check if the time elapsed since last update exceeded the deadline
     * period. If we cannot get the tick time, try again on the next period
     */
    if (!OSAPI_System_get_ticktime(&tick_time.sec,&tick_time.nanosec))
    {
        return;
    }
    this_node = REDA_CircularList_get_first(&self->_deadline_timer);

    while (!REDA_CircularList_node_at_head(&self->_deadline_timer,this_node))
    {
        key_entry = WHSM_HistoryKeyEntry_from_node(this_node,_deadline_entry);

        /* Get the next node in the list since the current one may be
         * removed by add_meta_sample due to not being alive
         */
        this_node = REDA_CircularListNode_get_next(this_node);

        if (DDS_Duration_delta_gt(&self->_qos->deadline.period,
                &tick_time,&key_entry->last_period_time))
        {
            /* Since the deadline was missed, reset periodic update timeout
             * so that the deadline period starts from this time. It is
             * set to the sample update time when a new sample is received.
             */
            key_entry->last_period_time = tick_time;

            /* Since the deadline was missed, reset the timeout so that the
             * deadline period starts again from the next sample received.
             */
            key_entry->last_update_time.nanosec = DDS_DURATION_INFINITE_NSEC;

            if (self->_listener.on_deadline_missed)
            {
                self->_listener.on_deadline_missed(
                        self->_listener.listener_data,&key_entry->key);
            }

            REDA_CircularList_unlink_node(&key_entry->_deadline_entry);
            REDA_CircularList_append(&self->_deadline_timer,
                    &key_entry->_deadline_entry);

        }
        else
        {
            break;
        }
    }
}
#endif

/*******************************************************************************
 *                                Public API
 ******************************************************************************/
#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the history cache
 *
 * \param[in] self The history cache to delete
 */
SHOULD_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_delete(struct WHSM_History *self)
{
    struct WHSM_HistoryKeyEntry *key;
    DDS_Long i,count;

    count = REDA_Indexer_get_count(self->key_index);

    for (i = 0; i < count; ++i)
    {
        key = REDA_Indexer_get_entry(self->key_index,0);
        if (!WHSM_History_delete_key(self,key,DDSHST_WRITER_KEY_REMOVED_NORMAL))
        {
            WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEY_OBJECT)
            return DDSHST_RETCODE_ERROR;
        }
    }

    if (!REDA_Indexer_delete(self->sample_index))
    {
        WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEINDEX_OBJECT)
        return DDSHST_RETCODE_ERROR;
    }

    if (!REDA_Indexer_delete(self->key_index))
    {
        WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEYINDEX_OBJECT)
        return DDSHST_RETCODE_ERROR;
    }


    if ((self->historical_index != NULL) &&
         !REDA_Indexer_delete(self->historical_index))
    {
        WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_HISTORYINDEX_OBJECT)
        return DDSHST_RETCODE_ERROR;
    }

    if (!REDA_BufferPool_delete(self->key_pool))
    {
        WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEYPOOL_OBJECT)
        return DDSHST_RETCODE_ERROR;
    }

    if (!REDA_BufferPool_delete(self->sample_pool))
    {
        WHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEPOOL_OBJECT)
        return DDSHST_RETCODE_ERROR;
    }

    OSAPI_Heap_free(self);

    return DDSHST_RETCODE_SUCCESS;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Create a new instance of the WHSM history cache
 *
 * \details
 * This function creates a new instance of the history cache. It should be
 * noted that it is possible that the qos policy contains legal and consistent
 * DDS values, but may be unsupported by the cache. In this case the cache
 * will return NULL and log an error (if logging is enabled).
 *
 * \param[in] property The WHSM properties, cannot be NULL
 * \param[in] listener The writer history listener
 *
 * \return Pointer to new instance of the history cache on success,
 *         NULL on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE struct WHSM_History*
WHSM_History_create(const struct WHSM_HistoryProperty *const property,
                    const struct DDSHST_WriterListener *const listener)
{
    struct WHSM_History *history = NULL;
    struct WHSM_History *retval = NULL;
    const struct DDS_DataWriterQos *qos;
    struct REDA_BufferPoolProperty bp_property = REDA_BufferPoolProperty_INITIALIZER;
    struct REDA_IndexerProperty ix_property = REDA_IndexerProperty_INITIALIZER;

    if (property == NULL)
    {
        WHSM_LOG_NO_PROPERTY(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    qos = property->_parent.qos;

    if (qos->history.kind != DDS_KEEP_LAST_HISTORY_QOS)
    {
        WHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if ((qos->resource_limits.max_instances == DDS_LENGTH_UNLIMITED) ||
        (qos->resource_limits.max_samples == DDS_LENGTH_UNLIMITED) ||
        (qos->resource_limits.max_samples_per_instance == DDS_LENGTH_UNLIMITED))
    {
        WHSM_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

#ifdef RTI_CERT
    if ((qos->resource_limits.max_samples <
        (qos->resource_limits.max_instances *
         qos->resource_limits.max_samples_per_instance)) ||
         (qos->resource_limits.max_samples <
          (qos->history.depth * qos->resource_limits.max_instances)))
    {
        WHSM_LOG_MAX_SAMPLES_TOO_SMALL(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif

    OSAPI_Heap_allocate_struct(&history, struct WHSM_History);
    if (history == NULL)
    {
        goto done;
    }

    RT_Component_initialize(&history->_parent._parent,
                           &WHSM_HistoryI_fv_Intf._parent,
                           0,
                           &property->_parent._parent,
                           (listener ? &listener->_parent : NULL));

    history->_qos = qos;
    history->_property = *property;

    if (listener)
    {
        history->_listener = *listener;
    }
    else
    {
        history->_listener = RTI_DEFAULT_SMWRITERHISTORY_LISTENER;
    }

    bp_property.buffer_size =  sizeof(struct WHSM_HistoryKeyEntry);
    bp_property.max_buffers = (RTI_SIZE_T)history->_qos->resource_limits.max_instances;

    history->key_pool = REDA_BufferPool_new("key_pool",&bp_property,
                                            NULL,NULL,NULL,NULL);
    if (history->key_pool == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEYPOOL_OBJECT)
        goto done;
    }

    bp_property.buffer_size =  sizeof(struct WHSM_HistorySampleEntry);
    bp_property.max_buffers = (RTI_SIZE_T)history->_qos->resource_limits.max_samples;

    history->sample_pool = REDA_BufferPool_new("sample_pool",&bp_property,
                                               NULL,NULL,NULL,NULL);
    if (history->sample_pool == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEPOOL_OBJECT)
        goto done;
    }

    ix_property.max_entries = history->_qos->resource_limits.max_samples;
    history->sample_index = REDA_Indexer_new(WHSM_History_indexer_sample_compare,
                                             &ix_property);

    if (history->sample_index == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEINDEX_OBJECT)
        goto done;
    }

    if (history->_qos->durability.kind > DDS_VOLATILE_DURABILITY_QOS)
    {
        ix_property.max_entries = history->_qos->resource_limits.max_instances *
                history->_qos->history.depth;
        history->historical_index = REDA_Indexer_new(WHSM_History_indexer_sample_compare,
                                                     &ix_property);

        if (history->historical_index == NULL)
        {
            WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_HISTORYINDEX_OBJECT)
            goto done;
        }
    }
    else
    {
        history->historical_index = NULL;
    }

    ix_property.max_entries = history->_qos->resource_limits.max_instances;
    history->key_index = REDA_Indexer_new(WHSM_History_indexer_key_compare,
                                          &ix_property);

    if (history->key_index == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEYINDEX_OBJECT)
        goto done;
    }

    REDA_SequenceNumber_set_zero(&history->_state.high_sn);
    REDA_SequenceNumber_set_zero(&history->_state.low_sn);
    REDA_SequenceNumber_set_zero(&history->_state.history_high_sn);
    REDA_SequenceNumber_set_zero(&history->_state.history_low_sn);

    if (!DDS_Duration_is_infinite(&history->_qos->destination_order.source_timestamp_tolerance))
    {
        OSAPI_NtpTime_from_nanosec(&history->source_timestamp_tolerance,
           history->_qos->destination_order.source_timestamp_tolerance.sec,
           history->_qos->destination_order.source_timestamp_tolerance.nanosec);
    }

    REDA_CircularList_init(&history->_deadline_timer);

#ifdef ENABLE_QOS_DEADLINE
    if (!DDS_Duration_is_infinite(&history->_qos->deadline.period))
    {
        history->deadline_enabled = RTI_TRUE;
    }
    else
    {
        history->deadline_enabled = RTI_FALSE;
    }
#else
    history->deadline_enabled = RTI_FALSE;
#endif

    retval = history;

done:
#ifndef RTI_CERT
    if ((retval == NULL) && (history != NULL))
    {
        WHSM_History_delete(history);
    }
#endif
    return retval;
}

/*ci
 * \brief Reserve an entry in the history cache
 *
 * \details
 * Implementation of the DDSHST_Writer_get_entry function.
 * Get an entry in the history cache if there is space.
 *
 * \param[in] wh         The writer cache to get an entry from
 * \param[in] key        The key to get an entry for
 * \param[in] kind       The kind of entry (dispose, unregister, normal etc.)
 * \param[in] assert_key If TRUE, assert the key if it does not already exist.
 *                       Otherwise return NULL if the key does not exist.
 * \param[in] source_ts  The timestamp the entry was reserved, used for source-ordering
 *                       time-stamping.
 *
 * \return A reference to new entry if there is space, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_WriterSampleEntryRef_T
WHSM_History_get_entry(struct DDSHST_Writer *wh,
                       const DDS_InstanceHandle_t *const key,
                       DDSHST_WriterEntryKind_T kind,
                       RTI_BOOL assert_key,
                       struct OSAPI_NtpTime *source_ts)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *new_entry = NULL;
    struct WHSM_HistoryKeyEntry *key_entry = NULL;
    struct OSAPI_NtpTime diff;

    key_entry = WHSM_History_find_key(self, key);

    if (key_entry == NULL)
    {
        /* Do not allocate the key unless it is allowed by the caller. This is
         * to support the case where unregister cannot auto-register a key
         * but dispose can.
         */
        if (!assert_key)
        {
            return NULL;
        }

        key_entry = REDA_BufferPool_get_buffer(self->key_pool);
        if (key_entry == NULL)
        {
            /* This is not an error */
            /* Exceeded max_instance resource limit */
            return NULL;
        }

        if (!WHSM_History_initialize_key(self, key_entry,key))
        {
            return NULL;
        }

        key_entry->source_timestamp = *source_ts;

        REDA_CircularList_append(&self->_deadline_timer,
                                 &key_entry->_deadline_entry);
    }

    /* For SOURCE_TIMESTAMP_DESTINATIONORDER, check if the difference between
     * the last time the instance was written and the supplied source timestamp
     * is larger than allowed. If so reject the entry request.
     */
    if ((self->_qos->destination_order.kind == DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS) &&
            !DDS_Duration_is_infinite(&self->_qos->destination_order.source_timestamp_tolerance))
    {
        if (OSAPI_NtpTime_compare(source_ts,&key_entry->source_timestamp) < 0)
        {
            /* source_ts is less then the previous sample's source time stamp
             * (per instance). Check if the difference exceeds the tolerance
             * by calculating by checking if last - new > tolerance.
             * if last - new > tolerance return NULL, otherwise the the
             * output source_ts to the last source_ts for the instance.
             */
            OSAPI_NtpTime_subtract(&diff,&key_entry->source_timestamp,source_ts);
            if (OSAPI_NtpTime_compare(&diff,&self->source_timestamp_tolerance) > 0)
            {
                return NULL;
            }
            *source_ts = key_entry->source_timestamp;
        }
        else
        {
            /* The new source_ts was >= to the last source_ts, update it */
            key_entry->source_timestamp = *source_ts;
        }
    }
    else
    {
        /* The tolerance is infinite or by_reception_timestamp_order */
        key_entry->source_timestamp = *source_ts;
    }

    /* NOTE; Samples are only pruned if the resource-limit is met. That means
     * it is only allowed to have at most one get_entry called outstanding,
     * that is if get_entry is called twice without returning or committing
     * the previous sample, then the 2nd call will fail.
     */
    if (key_entry->sample_count == self->_qos->resource_limits.max_samples_per_instance)
    {
        /* Not allowed to exceed resource-limit */
        if (!WHSM_History_prune_samples(self,key_entry))
        {
            return NULL;
        }
    }

    /* We are allowed to allocate sample */
    new_entry = (struct WHSM_HistorySampleEntry*)
                                 REDA_BufferPool_get_buffer(self->sample_pool);

    if (new_entry == NULL)
    {
        /* This is not an error */
        return NULL;
    }

    ++key_entry->sample_count;
    new_entry->key_entry = key_entry;
    new_entry->kind = kind;
    new_entry->_sample = NULL;
    REDA_SequenceNumber_set_zero(&new_entry->sn);
    new_entry->_state = RTI_SMWRITERHISTORY_SAMPLE_STATE_FREE;

    if ((kind == DDSHST_WRITER_ENTRY_UNREGISTER) ||
        (kind == DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE))
    {
        key_entry->state = WHSM_HISTORY_KEY_STATE_NOT_ALIVE;
    }
    else
    {
        key_entry->state = WHSM_HISTORY_KEY_STATE_ALIVE;
    }

    REDA_CircularListNode_init(&new_entry->_node);

    return (DDSHST_WriterSampleEntryRef_T)new_entry;
}

/*ci
 * \brief Return an unused queue entry to the queue
 *
 * \details
 * Implementation of the DDSHST_Writer_return_entry function.
 * Return an entry to cache, it is unused.
 *
 * \param[in] wh    The history cache
 * \param[in] entry A reference to the entry to return
 */
RTI_PRIVATE void
WHSM_History_return_entry(struct DDSHST_Writer *wh,
                                 DDSHST_WriterSampleEntryRef_T const entry)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *s_entry =
                                       (struct WHSM_HistorySampleEntry *)entry;

    /* When an entry is returned it has not been added to the queue. Thus,
     * set in in normal state.
     */
    s_entry->_state = RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED |
                      RTI_SMWRITERHISTORY_SAMPLE_STATE_RETURNED;

    WHSM_History_analyze_sample_state(self,s_entry);
}

/*ci
 * \brief Commit a previously reserved entry to the history cache
 *
 * \details
 * Implementation of the DDSHST_Writer_commit_entry function.
 * Commit an entry to the writer cache, updating the state of the cache.
 * The ack_count is the expected number of acknowledgments before the sample
 * SN can be marked as ACK'ed
 *
 * \param[in] wh        The writer cache
 * \param[in] entry     A reference to a previously allocated entry
 * \param[in] sample    The entries's payload (opaque to the cache)
 * \param[in] sn        The sequence number of the entry
 * \param[in] ack_count The expected number of acknowledgment for this sample
 *                      based on the number of reliable peers
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the other \ref
 *         DDSHST_ReturnCode_T on failure
 *
 * \sa \ref WHSM_History_get_entry
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_commit_entry(struct DDSHST_Writer *wh,
                                 DDSHST_WriterSampleEntryRef_T const entry,
                                 DDSHST_WriterSample_T *const sample,
                                 const struct REDA_SequenceNumber *const sn,
                                 DDS_Long ack_count)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *new_entry =
                                       (struct WHSM_HistorySampleEntry *)entry;
    struct WHSM_HistorySampleEntry *s1_entry;
    struct WHSM_HistoryKeyEntry *w_entry = NULL;
    struct DDS_Duration_t tick_time;

    new_entry->sn = *sn;
    new_entry->_sample = sample;
    w_entry = new_entry->key_entry;

    /* Always advancing to the next written SN */
    self->_state.high_sn = *sn;

    /* The highest SN is stored first in the list */
    s1_entry = (struct WHSM_HistorySampleEntry*)
                                REDA_CircularList_get_first(&w_entry->samples);

    if ((s1_entry == (struct WHSM_HistorySampleEntry *)&w_entry->samples)
         || (REDA_SequenceNumber_compare(&s1_entry->sn, sn) < 0))
    {
        REDA_CircularList_prepend(&w_entry->samples,&new_entry->_node);

        if (self->deadline_enabled)
        {
            /* Check if the time elapsed since last update exceeded the deadline
             * period.
             */
            if (OSAPI_System_get_ticktime(&tick_time.sec,&tick_time.nanosec))
            {
                if (w_entry->last_update_time.nanosec != DDS_DURATION_INFINITE_NSEC)
                {
                    if (DDS_Duration_delta_gt(&self->_qos->deadline.period,
                            &tick_time,&w_entry->last_update_time))
                    {
                        if (self->_listener.on_deadline_missed)
                        {
                            self->_listener.on_deadline_missed(
                                    self->_listener.listener_data,&w_entry->key);
                        }
                    }
                }

                w_entry->last_update_time = tick_time;
                w_entry->last_period_time = tick_time;

                REDA_CircularList_unlink_node(&w_entry->_deadline_entry);
                REDA_CircularList_append(&self->_deadline_timer,
                        &w_entry->_deadline_entry);
            }
        }

        if (!REDA_Indexer_add_entry(self->sample_index,new_entry))
        {
            WHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLE_OBJECT)
            return DDSHST_RETCODE_ERROR;
        }
    }
    else
    {
        /* This means the sample has a lower SN than the currently highest SN,
         * is not possible since samples are always committed in increasing
         * order
         */
        return DDSHST_RETCODE_ERROR;
    }

    if ((self->_qos->durability.kind > DDS_VOLATILE_DURABILITY_QOS) &&
        (self->_qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS))
    {
        /* Update the historical view
         * - The committed samples is always the most recent sample across
         *   all samples, thus add to the end of the list
         * - The committed sample may push out a sample from the historical view
         *   If this is the case, remove it from the history index.
         */
        if (w_entry->last_history_sample == NULL)
        {
            /* First sample, depth must be >= 1 */
            w_entry->last_history_sample = new_entry;
        }
        else if (w_entry->sample_count > self->_qos->history.depth)
        {
            /* NON_HISTORICAL indicates a _transition_ from historical to
             * a non historical sample, meaning it only happens once and
             * it means that w_entry->last_history_sample is no longer
             * the last history.
             */
            w_entry->last_history_sample->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL;
            WHSM_History_analyze_sample_state(self,w_entry->last_history_sample);
        }

        new_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL;
        if (!REDA_Indexer_add_entry(self->historical_index,new_entry))
        {
            WHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLE_OBJECT)
            return DDSHST_RETCODE_ERROR;
        }
    }

    if (ack_count == 0)
    {
        new_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED;
    }

    new_entry->ack_count = ack_count;

    WHSM_History_analyze_sample_state(self,new_entry);

    return DDSHST_RETCODE_SUCCESS;
}

/*ci
 * \brief Request a sample from the queue
 *
 * \details
 * Implementation of the DDSHST_Writer_request_sample function
 * Return a reference to a history sample based on the requested SN.
 * A sample which is requested is automatically considered
 * unacked and will not be removed from the queue unless forced by an acknack.
 * Thus, a sample which was ACK'ed may go back to UNACKed state. A request
 * does not change the history.
 * \p
 * The cache can search in all samples or only the historical samples
 * based on the historical_only flag. This function always returns the next
 * available SN if the requested cannot be found. If the requested SN is found
 * the first available is set to the requested SN.
 *
 * \param[in]  wh              The history cache
 * \param[out] sample          The requested sample if found, NULL otherwise
 * \param[in]  sn              The requested SN
 * \param[out] sn_ge           The first available SN
 * \param[in]  historical_only Whether only historical samples are relevant
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref DDSHST_RETCODE_SUCCESS
 *         on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_request_sample(struct DDSHST_Writer *wh,
                            struct DDSHST_WriterSample **sample,
                            const struct REDA_SequenceNumber *const sn,
                            struct REDA_SequenceNumber *const sn_ge,
                            DDS_Boolean historical_only)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *s1_entry = NULL;
    DDSHST_ReturnCode_T retval = DDSHST_RETCODE_NOT_EXISTS;

    /* If the requested SN is */
    if (historical_only)
    {
        s1_entry = WHSM_History_find_history_entry(self, sn);
    }
    else
    {
        s1_entry = (struct WHSM_HistorySampleEntry*)
                    REDA_Indexer_find_entry_eq_or_gt(self->sample_index, sn);
    }

    if (s1_entry != NULL)
    {
        *sn_ge = s1_entry->sn;
        *sample = s1_entry->_sample;
        (*sample)->_ref = (DDSHST_WriterSampleEntryRef_T) s1_entry;
        s1_entry->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED;
        retval = DDSHST_RETCODE_SUCCESS;
    }

    return retval;
}

/*ci
 * \brief Ack or Nack a sample in the history cache.
 *
 * \details
 * Implementation of the DDSHST_Writer_acknack_sample function.
 * When a SN is acked it tells the queue that as far as the external
 * view is concerned that sample is no longer needed. However, the queue
 * can be configured to keep samples for delivery to late joiners.
 *
 * There are 3 types of sample that can be ack'ed:
 * - normal sample: This is regular user-data and does not change the key state.
 *                  Normal data samples that are acked are usually returned to
 *                  the buffer-pool or kept around for history
 * - unregister sample: An ACK'ed unregister sample means that everybody has
 *                      acknowledged the un-registration from this queue. If
 *                      there are no more samples in the queue then the
 *                      key will be reclaimed. If the key is not empty then
 *                      the key resources are not reclaimed.
 * - dispose sample: An ACK'ed dispose sample means that everybody has
 *                   acknowledged the disposal from this queue. However,
 *                   disposed keys cannot be reclaimed.
 *
 * If nack is DDS_BOOLEAN true the sample was not delivered to all, but
 * it can be removed as no further attempts to deliver it will be made.
 *
 * \param[in] wh   The writer cache
 * \param[in] sn   The SN to acknack
 * \param[in] nack Whether this is a NACK (DDS_BOOLEAN_TRUE) or ACK
 *                 (DDS_BOOLEAN_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, of of the \ref
 *         DDSHST_ReturnCode_T on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_acknack_sample(struct DDSHST_Writer *wh,
                            const struct REDA_SequenceNumber *const sn,
                            DDS_Boolean nack)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *s1_entry;

    s1_entry = WHSM_History_find_entry(self, sn);

    if (s1_entry == NULL)
    {
        /* not necessarily an error */
        return DDSHST_RETCODE_NOT_EXISTS;
    }

    if (s1_entry->ack_count > 0)
    {
        --s1_entry->ack_count;
    }

    if (s1_entry->ack_count <= 0)
    {
        if (nack)
        {
            s1_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED;
        }
        else
        {
            s1_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED;
        }
    }

    WHSM_History_analyze_sample_state(self,s1_entry);

    return DDSHST_RETCODE_SUCCESS;
}

/*ci
 * \brief Register a key in the cache.
 *
 * \details
 * Implementation of the DDSHST_Writer_register_key function.
 * Add a key to the cache if there is space.
 *
 * \param[in] wh  The history cache
 * \param[in] key The key to add to the cache
 * \param[in] timestamp The time the entry was registered, used with source order
 *                       timestamps.
 *
 * \return DDSHST_RETCODE_SUCCESS on success, of of the \ref
 *         DDSHST_ReturnCode_T on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_register_key(struct DDSHST_Writer *wh,
                          const DDS_InstanceHandle_t *const key,
                          const struct OSAPI_NtpTime *timestamp)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistoryKeyEntry *key_entry;

    key_entry = WHSM_History_find_key(self, key);

    if (key_entry != NULL)
    {
        return DDSHST_RETCODE_SUCCESS;
    }

    key_entry = REDA_BufferPool_get_buffer(self->key_pool);
    if (key_entry == NULL)
    {
        /* This is not necessarliy an error */
        /* Exceeded DataWriterQos.resource_limits.max_instances */
        return DDSHST_RETCODE_NOSPACE;
    }

    if (!WHSM_History_initialize_key(self,key_entry,key))
    {
        return DDSHST_RETCODE_ERROR;
    }

    if (timestamp == NULL)
    {
        if (!OSAPI_System_get_time(&key_entry->source_timestamp))
        {
            return DDSHST_RETCODE_ERROR;
        }
    }
    else
    {
        key_entry->source_timestamp = *timestamp;
    }

    return DDSHST_RETCODE_SUCCESS;
}

/*ci
 * \brief Get the state of the history cache
 *
 * \details
 * Implementation of the DDSHST_Writer_get_state function. Return the current
 * state of the writer cache. This information is used to determine which
 * sequence numbers are relevant for a datareader when durability
 * is > VOLATILE.
 *
 * \param[in] wh The history cache
 *
 * \return Pointer to the structure containing the state
 */
MUST_CHECK_RETURN RTI_PRIVATE struct DDSHST_WriterState*
WHSM_History_get_state(struct DDSHST_Writer *wh)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *entry;

    entry = REDA_Indexer_get_first_entry(self->sample_index);

    if (entry != NULL)
    {
        self->_state.low_sn = entry->sn;
        entry = REDA_Indexer_get_last_entry(self->sample_index);
        self->_state.high_sn = entry->sn;
    }
    else
    {
        /* Default to the last written SN if the cache is empty */
        self->_state.low_sn = self->_state.high_sn;
    }

    if (self->historical_index)
    {
        entry = REDA_Indexer_get_first_entry(self->historical_index);
        if (entry != NULL)
        {
            self->_state.history_low_sn = entry->sn;
            entry = REDA_Indexer_get_last_entry(self->historical_index);
            self->_state.history_high_sn = entry->sn;
        }
    }

    return &self->_state;
}

/*ci
 * \brief Update the number of expected acknacks for a sample
 *
 * \details
 * For durable data, when a late reliable joiner is added as peer the
 * expected number of acknacks are increased.
 *
 * \param[in] wh The history cache
 */
RTI_PRIVATE void
WHSM_History_update_historical_ackcount(struct DDSHST_Writer *wh)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    RTI_INT32 i,j;
    struct WHSM_HistorySampleEntry *sample;

    j = REDA_Indexer_get_count(self->historical_index);

    for (i = 0; i < j; ++i)
    {
        sample = (struct WHSM_HistorySampleEntry*)
                              REDA_Indexer_get_entry(self->historical_index,i);
        ++sample->ack_count;
    }
}

/*ci
 * \brief Post an event to the writer cache
 *
 * \details
 * Implementation of the DDSHST_Writer_post_event function.
 * Handle possible events posted to the cache by the user. The events
 * handled are deadline tests (the deadline timer is not owned by the cache)
 * and update expected acknacks when a reliable late joiner as added. Note
 * that the cache does not keep track of peers, only how many there are.
 *
 * \param[in] wh    The history cache
 * \param[in] event The event
 * \param[in] now   The timestamp for the event
 */
RTI_PRIVATE void
WHSM_History_post_event(struct DDSHST_Writer *wh,
                        struct DDSHST_WriterEvent *event,
                        struct OSAPI_NtpTime *now)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;

    switch (event->kind)
    {
        case DDSHST_WRITEREVENT_KIND_DEADLINE_EXPIRED:
            WHSM_History_on_deadline_expired(self,now);
            break;
        case DDSHST_WRITEREVENT_KIND_HISTORICAL_DATA_REQUESTED:
            WHSM_History_update_historical_ackcount(wh);
            break;
        default:
            break;
    }
}

/*ci
 * \brief Get the current state of an instance in the cache.
 *
 * \param[in] self  The history cache
 * \param[in] key   The key to retrieve state for
 * \param[in] state The current state of an instance, if found
 *
 * \return DDSHST_RETCODE_NOT_EXISTS if the instance does not exist,
 *         DDSHST_RETCODE_OK if the instance exists
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_get_instance_state(struct DDSHST_Writer *self,
                                const DDS_InstanceHandle_t *const key,
                                struct DDSHST_InstanceState *state)
{
    struct WHSM_History *wh = (struct WHSM_History *)self;
    struct WHSM_HistoryKeyEntry *key_entry;

    key_entry = WHSM_History_find_key(wh, key);

    if (key_entry == NULL)
    {
        return DDSHST_RETCODE_NOT_EXISTS;
    }

    state->sample_count = key_entry->sample_count;

    if (key_entry->sample_count == 0)
    {
        state->last_sample_kind = DDSHST_WRITER_ENTRY_NONE;
    }
    else
    {
        state->last_sample_kind = ((struct WHSM_HistorySampleEntry*)key_entry->samples._prev)->kind;
    }

    return DDSHST_RETCODE_SUCCESS;
}

/*ci
 * \brief Unregister a key from the history cache.
 *
 * \details
 *
 * Unregister a key from the history. Note that this is not the same as a DDS
 * unregister_instance. Unregistering a key simple deletes it from the
 * writer's cache without any instance updates.
 *
 * \param[in]  self  The history cache
 * \param[in]  key   The key to unregister
 *
 * \return DDSHST_RETCODE_OK on success, DDSHST_RETCODE_NOT_EXISTS if the
 *         instance does not exist.
 */
RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_unregister_key(struct DDSHST_Writer *self,
                            const DDS_InstanceHandle_t *const key)
{
    struct WHSM_History *wh = (struct WHSM_History *)self;

    struct WHSM_HistoryKeyEntry *key_entry;

    key_entry = WHSM_History_find_key(wh, key);

    if (key_entry == NULL)
    {
        return DDSHST_RETCODE_NOT_EXISTS;
    }

    if (!WHSM_History_delete_key(wh,key_entry,DDSHST_WRITER_KEY_REMOVED_DELETED))
    {
        return DDSHST_RETCODE_ERROR;
    }

    return DDSHST_RETCODE_SUCCESS;
}

/*********************** MicroDDS COMPONENT Interface *************************/

/*ci
 * \brief Implementation of the DDSHST_WriterI interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct DDSHST_WriterI WHSM_HistoryI_fv_Intf =
{
    RT_COMPONENTI_BASE,
    WHSM_History_get_entry,
    WHSM_History_return_entry,
    WHSM_History_commit_entry,
    WHSM_History_request_sample,
    WHSM_History_acknack_sample,
    WHSM_History_register_key,
    WHSM_History_get_state,
    WHSM_History_post_event,
    WHSM_History_get_instance_state,
    WHSM_History_unregister_key
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
 * \sa \ref WHSM_HistoryFactory_delete_component
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
WHSM_HistoryFactory_create_component(struct RT_ComponentFactory *factory,
                                     struct RT_ComponentProperty *property,
                                     struct RT_ComponentListener *listener)
{
    struct WHSM_History *retval = NULL;
    UNUSED_ARG(factory);

    retval = WHSM_History_create(
                        (const struct WHSM_HistoryProperty *)property,
                        (const struct DDSHST_WriterListener *)listener);

    if (retval == NULL)
    {
        return NULL;
    }

    return &retval->_parent._parent;
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
 * \sa \ref WHSM_HistoryFactory_create_component
 */
RTI_PRIVATE void
WHSM_HistoryFactory_delete_component(struct RT_ComponentFactory *factory,
                                     RT_Component_T *component)
{
    struct WHSM_History *self = (struct WHSM_History *)component;
    UNUSED_ARG(factory);

    WHSM_History_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
WHSM_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
WHSM_HistoryFactory_finalize(struct RT_ComponentFactory *factory,
                             struct RT_ComponentFactoryProperty **property,
                             struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI WHSM_HistoryFactory_fv_Intf =
{
    WHSM_HISTORY_INTERFACE_ID,
    WHSM_HistoryFactory_initialize,
#ifndef RTI_CERT
    WHSM_HistoryFactory_finalize,
#else
    NULL,
#endif
    WHSM_HistoryFactory_create_component,
#ifndef RTI_CERT
    WHSM_HistoryFactory_delete_component,
#else
    NULL,
#endif
    NULL,
    NULL
};

/*ci
 * \brief Singleton for the WHSM_HistoryFactory
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactory WHSM_HistoryFactory_fv_Factory =
{
    &WHSM_HistoryFactory_fv_Intf,
    NULL,
    {{{0,0}}}
};

/*ci
 * \brief Method to initialize the writer history factory
 *
 * \details
 * RHSM specific implementation of the RT ComponentFactory initialize method.
 * This method is called when the writer history factory is registered with the
 * RT.
 *
 * \param[in] property The properties registered with the history interface
 * \param[in] listener The listener registered with the history interface
 *
 * \return A fully initialized factory
 *
 * \sa \ref WHSM_HistoryFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
WHSM_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
    WHSM_HistoryFactory_fv_Factory._factory = &WHSM_HistoryFactory_fv_Factory;

    return &WHSM_HistoryFactory_fv_Factory;
}

#ifndef RTI_CERT
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
 * \sa \ref WHSM_HistoryFactory_initialize
 */
RTI_PRIVATE void
WHSM_HistoryFactory_finalize(struct RT_ComponentFactory *factory,
                    struct RT_ComponentFactoryProperty **property,
                    struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
WHSM_HistoryFactory_get_interface(void)
{
    return &WHSM_HistoryFactory_fv_Intf;
}

/*ci @} */

