/*
 * FILE: History.c - Writer History implementation
 *
 * Copyright (c) 2011-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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
    key->state = WHSM_HISTORY_KEY_STATE_ALLOCATED;

    DDS_Duration_set(&key->last_update_time,0,DDS_DURATION_INFINITE_NSEC);
    DDS_Duration_set(&key->last_period_time,0,DDS_DURATION_INFINITE_NSEC);

    if (!REDA_Indexer_add_entry(self->key_index,key))
    {
        WHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEY_OBJECT);
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
            WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR);
            return;
        }

        /* All samples are marked as HISTORICAL state even if the Durability is volaitile
         * but the historical index is only created if history.kind > VOLATILE
         */

        if ((self->historical_index != NULL) &&
                (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL))
        {
            if (REDA_Indexer_remove_entry(self->historical_index,&sample->sn) != sample)
            {
                WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR);
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

    OSAPI_Trace_write("sample state for SN=%S: %^d ",
                      &sample->sn,&sample->_state,
                      NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    if (sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_IN_PROGRESS)
    {
        OSAPI_Trace_write("sample SN=%S in progress, don't remove",
                          &sample->_state,NULL,
                          NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL);
        return;
    }

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
    if ((sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL) ||
        ((sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED) &&
         (sample == key_entry->last_history_sample)))
    {
        /* key_entry->last_history_sample _must_ be != NULL because
         * sample is always != NULL or if the sample state is
         * RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL it means it
         * was previously historical and was pushed off the cache.
         */
        key_entry->last_history_sample->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL;

        /* historical index is only created if history.kind > VOLATILE */
        if ((self->historical_index) && (REDA_Indexer_remove_entry(self->historical_index,
                              &key_entry->last_history_sample->sn) != sample))
        {
            WHSM_LOG_INVALID_INDEX_OBJECT(OSAPI_LOGKIND_ERROR);
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

    if (key_entry->state == WHSM_HISTORY_KEY_STATE_ALLOCATED)
    {
        /* This is a special case for handling when 1) an entry was allocated
         * and 2) a new key was asserted, but 3) the entry is returned
         * without the key ever having any samples committed. In this case
         * the key can be deleted and returned to the key pool.
         */
        free_key = RTI_TRUE;
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

    /* The deletion of the queue entry is an asynchronous operation and
     * in theory it is possible that a key is written again before it
     * is deleted. In that case the key should not be deleted. Thus,
     * mark the key for deletion here. If the key becomes alive then
     * it will not be deleted later.
     */
#if 0
    if (free_key)
    {
        sample->key_entry->state = WHSM_HISTORY_KEY_STATE_DELETED;
    }

#else

    /* This functionality has now been moved to WHSM_History_return_entry */
    if (free_key)
    {
        /* It is not considered an error if the key cannot be deleted at this
         * point, may still be other samples left
         */
        WHSM_History_delete_key(self,sample->key_entry,
                                DDSHST_WRITER_KEY_REMOVED_UNREGISTERED);
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    /* Check if there are any threads pending on freeing up resources. The wake
     * up is done before the sample is actually returned to the buffer. This is
     * ok as this function cannot be interrupted.
     */
    if (!REDA_CircularList_is_empty(&self->suspended_threads))
    {
        struct WHSM_ThreadSuspendState *pobj =
                    (struct WHSM_ThreadSuspendState*)
                        REDA_CircularList_get_first(&self->suspended_threads);

        while (!REDA_CircularList_node_at_head(&self->suspended_threads,pobj))
        {
            if (pobj->key_entry == sample->key_entry)
            {
                REDA_CircularList_unlink_node(&pobj->_node);
                if (!OSAPI_Semaphore_give(pobj->wakeup_sem))
                {

                }
                break;
            }
            pobj = (struct WHSM_ThreadSuspendState*)
                               REDA_CircularListNode_get_next(&pobj->_node);
        }
    }
#endif

    OSAPI_Trace_write("returning sample with state: %^d for SN=%S",
                      &sample->_state,&sample->sn,
                      NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    REDA_BufferPool_return_buffer(self->sample_pool,sample);
#endif
}

RTI_PRIVATE RTI_BOOL
WHSM_History_set_send_state(struct DDSHST_Writer *wh,
                                const struct REDA_SequenceNumber *const sn,
                                DDSHST_WriterEventKind_T state)
{
    struct WHSM_History *self = (struct WHSM_History*)wh;
    struct WHSM_HistorySampleEntry *sample;

    sample = REDA_Indexer_find_entry(self->sample_index,sn);
    if (sample == NULL)
    {
        OSAPI_Trace_write("Unknown sample %S",
                          sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        return RTI_FALSE;
    }

    if (state == DDSHST_WRITEREVENT_KIND_ENTRY_STATE_QUEUED)
    {
        OSAPI_Trace_write("Q entry %S marked queued",
                          sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        sample->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_QUEUED;
    }
    else if (state == DDSHST_WRITEREVENT_KIND_ENTRY_STATE_IN_PROGRESS)
    {
        OSAPI_Trace_write("Q entry %S marked as in progress",
                          sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
        sample->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_QUEUED;
        sample->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_IN_PROGRESS;
    }
    else if (state == DDSHST_WRITEREVENT_KIND_ENTRY_STATE_SEND_COMPLETED)
    {
        OSAPI_Trace_write("Q entry %S marked as completed",
                          sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);

        sample->_state &= ~(RTI_SMWRITERHISTORY_SAMPLE_STATE_IN_PROGRESS |
                            RTI_SMWRITERHISTORY_SAMPLE_STATE_QUEUED);

        WHSM_History_analyze_sample_state(self,sample);

        OSAPI_Trace_write("Q entry %S analyzed",
                          sn,NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);
    }
    else
    {
        /* Unknown */
    }
    return RTI_TRUE;
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
 * \param[in] self          The history cache
 * \param[in] key_entry     The key entry to prune a sample from
 * \param[in] only_if_acked Only prune sample if it has been ACKed
 *
 * \return DDS_BOOLEAN_TRUE if a sample was successfully removed,
 *         DDS_BOOLEAN_FALSE if a sample could not be removed
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
WHSM_History_prune_samples(struct WHSM_History *self,
                           struct WHSM_HistoryKeyEntry *const key_entry,
                           RTI_BOOL only_if_acked)
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

    if (only_if_acked &&
       !(sample->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED))
    {
        return DDS_BOOLEAN_FALSE;
    }

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
                                 struct OSAPI_SystemTime *now)
{
    struct WHSM_HistoryKeyEntry *key_entry;
    struct REDA_CircularListNode *this_node;
    struct DDS_Duration_t tick_time = DDS_DURATION_ZERO;

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

        if (DDS_Duration_delta_gt(&self->deadline.period,
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

    if ((self->key_pool != NULL) && (!REDA_BufferPool_delete(self->key_pool)))
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
    struct REDA_BufferPoolProperty bp_property = REDA_BufferPoolProperty_INITIALIZER;
    struct REDA_IndexerProperty ix_property = REDA_IndexerProperty_INITIALIZER;

    if (property == NULL)
    {
        WHSM_LOG_NO_PROPERTY(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if ((property->_parent.resource_limits.max_instances == DDS_LENGTH_UNLIMITED) ||
        (property->_parent.resource_limits.max_samples == DDS_LENGTH_UNLIMITED))
    {
        WHSM_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

#ifdef RTI_CERT
    if ((property->_parent.resource_limits.max_samples <
        (property->_parent.resource_limits.max_instances *
         property->_parent.resource_limits.max_samples_per_instance)) ||
         (property->_parent.resource_limits.max_samples <
          (property->_parent.history.depth * property->_parent.resource_limits.max_instances)))
    {
        WHSM_LOG_MAX_SAMPLES_TOO_SMALL(OSAPI_LOGKIND_ERROR)
        goto done;
    }

#endif

#if defined(RTI_CERT) || !OSAPI_THREAD_SEMAPHORE_ENABLED
    if (property->_parent.history.kind == DDS_KEEP_ALL_HISTORY_QOS)
    {
        WHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR);
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

    history->deadline = property->_parent.deadline;
    history->destination_order = property->_parent.destination_order;
    history->history = property->_parent.history;
    history->durability = property->_parent.durability;
    history->reliability = property->_parent.reliability;
    history->db = property->_parent._parent.db;
    history->write_lock = property->_parent.write_lock;

    if (listener)
    {
        history->_listener = *listener;
    }
    else
    {
        history->_listener = RTI_DEFAULT_SMWRITERHISTORY_LISTENER;
    }

    if (property->_parent.resource_limits.max_samples_per_instance == DDS_LENGTH_UNLIMITED)
    {
        history->max_samples_per_instance = property->_parent.resource_limits.max_samples;
    }
    else
    {
        history->max_samples_per_instance = property->_parent.resource_limits.max_samples_per_instance;
    }

    if (history->history.kind == DDS_KEEP_ALL_HISTORY_QOS)
    {
        history->history_depth = history->max_samples_per_instance;
    }
    else
    {
        history->history_depth = history->history.depth;
    }

    bp_property.buffer_size =  (RTI_SIZE_T)sizeof(struct WHSM_HistoryKeyEntry);
    bp_property.max_buffers = (RTI_SIZE_T)property->_parent.resource_limits.max_instances;

    history->key_pool = REDA_BufferPool_new("key_pool",&bp_property,
                                            NULL,NULL,NULL,NULL);
    if (history->key_pool == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_KEYPOOL_OBJECT)
        goto done;
    }

    bp_property.buffer_size =  (RTI_SIZE_T)sizeof(struct WHSM_HistorySampleEntry);
    bp_property.max_buffers = (RTI_SIZE_T)property->_parent.resource_limits.max_samples;

    history->sample_pool = REDA_BufferPool_new("sample_pool",&bp_property,
                                               NULL,NULL,NULL,NULL);
    if (history->sample_pool == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEPOOL_OBJECT)
        goto done;
    }

    ix_property.max_entries = property->_parent.resource_limits.max_samples;
    history->sample_index = REDA_Indexer_new(WHSM_History_indexer_sample_compare,
                                             &ix_property);

    if (history->sample_index == NULL)
    {
        WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLEINDEX_OBJECT)
        goto done;
    }

    if (history->durability.kind > DDS_VOLATILE_DURABILITY_QOS)
    {
        ix_property.max_entries = property->_parent.resource_limits.max_instances * history->history_depth;
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

    ix_property.max_entries = property->_parent.resource_limits.max_instances;
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
#if OSAPI_THREAD_SEMAPHORE_ENABLED
    REDA_CircularList_init(&history->suspended_threads);
#endif

    if (!DDS_Duration_is_infinite(&history->destination_order.source_timestamp_tolerance))
    {
        history->source_timestamp_tolerance.sec =
            history->destination_order.source_timestamp_tolerance.sec;
        history->source_timestamp_tolerance.nanosec =
            history->destination_order.source_timestamp_tolerance.nanosec;
    }

    REDA_CircularList_init(&history->_deadline_timer);

#ifdef ENABLE_QOS_DEADLINE
    if (!DDS_Duration_is_infinite(&history->deadline.period))
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
 *
 * \return A reference to new entry if there is space, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_WriterSampleEntryRef_T
WHSM_History_get_entry(struct DDSHST_Writer *wh,
                       const DDS_InstanceHandle_t *const key,
                       DDSHST_WriterEntryKind_T kind,
                       RTI_BOOL assert_key,
                       struct OSAPI_SystemTime *source_ts,
                       DDSHST_WriterErrorKind_T *ec)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *new_entry = NULL;
    struct WHSM_HistoryKeyEntry *key_entry = NULL;
#if OSAPI_THREAD_SEMAPHORE_ENABLED
    struct WHSM_ThreadSuspendState suspend_state;
    RTI_BOOL is_suspended = RTI_FALSE;
    RTI_INT32 sem_reason = OSAPI_SEMAPHORE_RESULT_OK;
    struct OSAPI_SystemTime suspend_end;
    struct OSAPI_SystemTime max_blocking_time;
    struct OSAPI_SystemTime blocking_time;
#endif
    struct OSAPI_SystemTime elapsed_time;
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL prune_sample = RTI_FALSE;
    RTI_BOOL prune_if_acked = RTI_FALSE;

    *ec = DDSHST_WRITER_ERROR_NONE;

    key_entry = WHSM_History_find_key(self, key);

    if (key_entry == NULL)
    {
        /* Do not allocate the key unless it is allowed by the caller. This is
         * to support the case where unregister cannot auto-register a key
         * but dispose can.
         */
        if (!assert_key)
        {
            /* key does not exist */
            *ec = DDSHST_WRITER_ERROR_UNKNOWN_KEY;
            return NULL;
        }

        key_entry = REDA_BufferPool_get_buffer(self->key_pool);
        if (key_entry == NULL)
        {
            /* This is not an error */
            *ec = DDSHST_WRITER_ERROR_OUT_OF_INSTANCE_RESOURCES;
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
     * the last time the instance was written and the supplied source
     * time-stamp is larger than allowed. If so reject the entry request.
     */
    if ((self->destination_order.kind ==
         DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS) &&
        !DDS_Duration_is_infinite(
                    &self->destination_order.source_timestamp_tolerance))
    {
        if (OSAPI_SystemTime_compare(source_ts,&key_entry->source_timestamp) < 0)
        {
            /* source_ts is less then the previous sample's source time stamp
             * (per instance). Check if the difference exceeds the tolerance
             * by calculating by checking if last - new > tolerance.
             * if last - new > tolerance return NULL, otherwise the the
             * output source_ts to the last source_ts for the instance.
             */
            OSAPI_SystemTime_subtract(&elapsed_time,&key_entry->source_timestamp,source_ts);
            if (OSAPI_SystemTime_compare(&elapsed_time,&self->source_timestamp_tolerance) > 0)
            {
                *ec = DDSHST_WRITER_ERROR_SOURCE_TIMESTAMP_ORDER;
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

    /* KEEP_LAST:
     * It is allowed to forcefully remove an instance even when unacknowledged
     * by a data-writer.
     *
     * KEEP_ALL:
     * In the case of KEEP_ALL no samples can be pruned. In this case, if there
     * are no samples available, it is necessary to block and wait for
     * resources.
     *
     * NOTE; Samples are only pruned if the resource-limit is met. That means
     * it is only allowed to have at most one get_entry called outstanding,
     * that is if get_entry is called twice without returning or committing
     * the previous sample, then the 2nd call will fail.
     */

    OSAPI_Trace_write("key_entry->sample_count = %^d, max_samples_per_instance = %^d",
                        &key_entry->sample_count,&self->max_samples_per_instance,
                        NULL,NULL,NULL,NULL,
                        NULL,NULL,NULL,NULL);

    if (key_entry->sample_count == self->max_samples_per_instance)
    {
        /* The resource-limit can be only be reached in the following cases:
         *
         * - reliable KEEP_LAST and depth = max_samples_per_instance and
         *   there are unacked samples in the cache.
         *
         * - reliable KEEP_LAST and depth = max_samples_per_instances and
         *   durability is > VOLATILE (acked/unacked samples in the cache).
         *
         * - reliable KEEP_ALL and there are unacked samples in the cache.
         *
         * - reliable KEEP_ALL and durability > VOLATILE (ack/unacked samples
         *   in the cache.
         *
         * - best_effort KEEP_LAST - Fragmented samples are flow-controlled,
         *                          may have to block to finish the sample.
         *
         * - best_effort KEEP_ALL - Fragmented samples are flow-controlled,
         *                          may have to block to finish the sample.
         */
        if ((self->history.kind == DDS_KEEP_ALL_HISTORY_QOS) &&
            (self->durability.kind > DDS_VOLATILE_DURABILITY_QOS))
        {
            prune_sample = RTI_TRUE;
            prune_if_acked = RTI_TRUE;
        }
        else if (self->history.kind == DDS_KEEP_ALL_HISTORY_QOS)
        {
            if (self->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS)
            {
                prune_sample = RTI_FALSE;
            }
            else
            {
                prune_sample = RTI_TRUE;
                prune_if_acked = RTI_FALSE;
            }
        }
        else if (self->history.kind == DDS_KEEP_LAST_HISTORY_QOS)
        {
            prune_sample = RTI_TRUE;
            prune_if_acked = RTI_FALSE;
        }

        done = RTI_FALSE;
        new_entry = NULL;

        while (!done && (new_entry == NULL))
        {
            OSAPI_TRACE_PRINTF2("prune sample = %^d, prune_if_acked = %^d\n,",
                                &prune_sample,&prune_if_acked);
            if (prune_sample)
            {
                /* In the case of transient_local it is possible that all
                 * samples are in use, but that all are ACKed. Thus, for
                 * TRANSIENT_LOCAL try to remove the oldest sample if it
                 * has been ACKED.
                 */
                if (WHSM_History_prune_samples(self,key_entry,prune_if_acked))
                {
                    /* Sample removed, try to allocate */
                    new_entry = (struct WHSM_HistorySampleEntry*)
                                 REDA_BufferPool_get_buffer(self->sample_pool);

                    if (new_entry != NULL)
                    {
                        /* Got a sample, nothing more to do */
                        break;
                    }
                    /* Didn't get a sample, continue with blocking */
                }
            }

            if (DDS_Duration_is_zero(&self->reliability.max_blocking_time))
            {
                *ec = DDSHST_WRITER_ERROR_TIMEOUT;
                break;
            }
#if OSAPI_THREAD_SEMAPHORE_ENABLED
            /* There are un-acked samples in the cache, wait for resources
             * to be released.
             */
            if (!is_suspended)
            {
                /* For the first block allocate a thread semaphore */
                REDA_CircularListNode_init(&suspend_state._node);
                suspend_state.wakeup_sem = OSAPI_System_get_user_thread_semaphore();

                if (suspend_state.wakeup_sem == NULL)
                {
                    WHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                             WHSM_LOG_THREAD_SEMAPHORE_OBJECT)
                    *ec = DDSHST_WRITER_ERROR_BLOCKING;
                    break;
                }

                if (!OSAPI_System_get_time(&suspend_state.suspend_start))
                {
                    break;
                }

                /* Record the instance the thread is blocking on so the
                 * thread is only woken up when instance resources are
                 * available.
                 */
                suspend_state.key_entry = key_entry;

                if (!DDS_Duration_is_infinite(&self->reliability.max_blocking_time))
                {
                    max_blocking_time.sec = self->reliability.max_blocking_time.sec;
                    max_blocking_time.nanosec = self->reliability.max_blocking_time.nanosec;
                }
                else
                {
                    max_blocking_time.sec = OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC;
                    max_blocking_time.nanosec = OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC;
                }
                is_suspended = RTI_TRUE;
                blocking_time = max_blocking_time;
            }
            else
            {
                /* Have blocked at least one time */
                if (!OSAPI_System_get_time(&suspend_end))
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                }

                /* Calculate remaining blocking time and subtract total waited
                 * Assumption is that both are INFINITE if max_blocking_time_sec
                 * is OSAPI_SEMAPHORE_TIMEOUT_INFINITE.
                 */
                if (max_blocking_time.sec != OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC)
                {
                    OSAPI_SystemTime_subtract(&elapsed_time,
                                              &suspend_end,
                                              &suspend_state.suspend_start);


                    if (OSAPI_SystemTime_compare(&elapsed_time,&max_blocking_time) >= 0)
                    {
                        *ec = DDSHST_WRITER_ERROR_TIMEOUT;
                        break;
                    }

                    OSAPI_SystemTime_subtract(&blocking_time,
                                              &max_blocking_time,
                                              &elapsed_time);
                }
            }

            REDA_CircularList_append(&self->suspended_threads,&suspend_state._node);

            {
                OSAPI_Mutex_T *write_lock = self->write_lock;
                OSAPI_Mutex_T *m;
                RTI_UINT32 m_depth = 0,l_count = 0;

                /* Release the write lock while suspended */
                if ((write_lock != NULL ) && (!OSAPI_Mutex_give(write_lock)))
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                    break;
                }

                /* This call is already within the db lock. Get the current
                 * depth to release it while suspended so data can be received.
                 */
                m = DB_Database_get_lock(self->db);
                if (m != NULL)
                {
                    m_depth = OSAPI_Mutex_get_depth(m);

                    for (l_count = 0; l_count < m_depth; l_count++)
                    {
                        /* Coverity warns that this is an attempt to release
                         * a non-recursive lock. However, this is a
                         * recursive lock. */

                        /* coverity[unlock : FALSE] */
                        if (!OSAPI_Mutex_give(m))
                        {
                            break;
                        }
                    }

                    if (l_count < m_depth)
                    {
                        break;
                    }
                }

                /* Block on write */
                if (!OSAPI_Semaphore_take_sec_nanosec(suspend_state.wakeup_sem,
                                          (RTI_INT32)blocking_time.sec,
                                          blocking_time.nanosec,
                                          &sem_reason))
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                    break;
                }

                /* Thread unblocked. At this point it is important that the
                 * database lock-level is resumed. Do not return before
                 * DB_Database_lock_resume() has been called.
                 */

                if (sem_reason == OSAPI_SEMAPHORE_RESULT_TIMEOUT)
                {
                    done = RTI_TRUE;
                    *ec = DDSHST_WRITER_ERROR_TIMEOUT;
                }

                if (m != NULL)
                {
                    for (l_count = 0; l_count < m_depth; l_count++)
                    {
                        /* coverity[use : FALSE] */
                        if (!OSAPI_Mutex_take(m))
                        {
                            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                            break;
                        }
                    }
                }

                if ((write_lock != NULL ) && (!OSAPI_Mutex_take(write_lock)))
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                    break;
                }
            }

            /* Woke up and inside critical section, try to allocate a sample
             * (even if the wake up was due to a timeout as a last attempt).
             */
            if (key_entry->sample_count < self->max_samples_per_instance)
            {
                /* We are allowed to allocate sample */
                new_entry = (struct WHSM_HistorySampleEntry*)
                                 REDA_BufferPool_get_buffer(self->sample_pool);
            }
#else
        done = RTI_TRUE;
#endif
        } /* while */
    }
    else
    {
        /* We are allowed to allocate sample */
        new_entry = (struct WHSM_HistorySampleEntry*)
                                  REDA_BufferPool_get_buffer(self->sample_pool);
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    /* Return resources used for a suspended thread */
    if (is_suspended)
    {
        OSAPI_System_return_user_thread_semaphore(suspend_state.wakeup_sem);
        REDA_CircularList_unlink_node(&suspend_state._node);
    }
#endif

    if (new_entry == NULL)
    {
        if (*ec == DDSHST_WRITER_ERROR_NONE)
        {
            *ec = DDSHST_WRITER_ERROR_OUT_OF_SAMPLE_RESOURCES;
        }

        return NULL;
    }

    ++key_entry->sample_count;
    new_entry->key_entry = key_entry;
    new_entry->kind = kind;
    new_entry->_sample = NULL;
    REDA_SequenceNumber_set_zero(&new_entry->sn);
    new_entry->_state = RTI_SMWRITERHISTORY_SAMPLE_STATE_NOT_COMMITTED;

    /* If the sample was already registered then it is ALIVE and the state
     * can be changed. If the key was allocated on an as needed basis, then
     * do not change the state until a sample is committed.
     */
    if (key_entry->state == WHSM_HISTORY_KEY_STATE_ALIVE)
    {
        if ((kind == DDSHST_WRITER_ENTRY_UNREGISTER) ||
            (kind == DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE))
        {
            key_entry->state = WHSM_HISTORY_KEY_STATE_NOT_ALIVE;
        }
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

    /* When an entry and has not been added to the queue _sample = NULL. In this
     * the sample "acked" and causes the queue entry to be removed to the
     * sample pool without any further actions required.
     */
    if (s_entry->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_NOT_COMMITTED)
    {
        /* When an entry is in NOT_COMMITTED state it has not been added
         * to the queue. Set in in normal state that will remove it
         * from the cache.
         */
        s_entry->_state = RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED |
                          RTI_SMWRITERHISTORY_SAMPLE_STATE_RETURNED;
    }

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
    struct DDS_Duration_t tick_time = DDS_DURATION_ZERO;


    new_entry->sn = *sn;
    new_entry->_sample = sample;
    w_entry = new_entry->key_entry;
    sample->_ref = entry;

    /* If the key was allocated in get_entry then change the state of
     * the key to ALIVE or NOT_ALIVE depending on the entry kind to indicate
     * that samples have been committed and that returning an entry should
     * not free the key.
     */
    if (w_entry->state == WHSM_HISTORY_KEY_STATE_ALLOCATED)
    {
        if ((new_entry->kind == DDSHST_WRITER_ENTRY_UNREGISTER) ||
            (new_entry->kind == DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE))
        {
            w_entry->state = WHSM_HISTORY_KEY_STATE_NOT_ALIVE;
        }
        else
        {
            w_entry->state = WHSM_HISTORY_KEY_STATE_ALIVE;
        }
    }

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
                    if (DDS_Duration_delta_gt(&self->deadline.period,
                                              &tick_time,
                                              &w_entry->last_update_time))
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
            REDA_CircularList_unlink_node(&new_entry->_node);
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
    else if (w_entry->sample_count > self->history_depth)
    {
        /* NON_HISTORICAL indicates a _transition_ from historical to
         * a non historical sample, meaning it only happens once and
         * it means that w_entry->last_history_sample is no longer
         * the last history.
         */
        w_entry->last_history_sample->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL;
        WHSM_History_analyze_sample_state(self, w_entry->last_history_sample);
    }

    if ((self->history.kind == DDS_KEEP_LAST_HISTORY_QOS) ||
        (self->durability.kind > DDS_VOLATILE_DURABILITY_QOS))
    {
        new_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL;
    }
    if (self->durability.kind > DDS_VOLATILE_DURABILITY_QOS)
    {
        if (!REDA_Indexer_add_entry(self->historical_index,new_entry))
        {
            REDA_CircularList_unlink_node(&new_entry->_node);
            WHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,WHSM_LOG_SAMPLE_OBJECT)
            return DDSHST_RETCODE_ERROR;
        }
    }

    if (ack_count == 0)
    {
        new_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED;
    }

    new_entry->ack_count = ack_count;
    new_entry->_state &= ~RTI_SMWRITERHISTORY_SAMPLE_STATE_NOT_COMMITTED;

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

#if DDS_FILTERING_ENABLED
/*ci
 * \brief Peek the next sequence number in the history cache
 *
 * \details
 * This function is equivalent to request_sample, but does not return the sample
 * itself or change any state in the history cache. It is used to just peek
 * at the next available sequence number after a given sequence number.
 *
 * \param[in]    wh              The history cache
 * \param[inout] sn_inout        The starting sequence number and is updated
 *                               to the next available sequence number if found
 * \param[in]    historical_only If TRUE, only historical samples are relevant
 */
RTI_PRIVATE void
WHSM_History_peek_next_sn(struct DDSHST_Writer *wh,
                          struct REDA_SequenceNumber *sn_inout,
                          DDS_Boolean historical_only)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistorySampleEntry *s1_entry = NULL;

    if (historical_only)
    {
        s1_entry = WHSM_History_find_history_entry(self, sn_inout);
    }
    else
    {
        s1_entry = (struct WHSM_HistorySampleEntry*)
                    REDA_Indexer_find_entry_eq_or_gt(self->sample_index, sn_inout);
    }

    if (s1_entry != NULL)
    {
        *sn_inout = s1_entry->sn;
    }
}
#endif /* DDS_FILTERING_ENABLED */

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
        OSAPI_Trace_write("received acknack on non-existent SN=%S, nack=%d",
                          sn,OSAPI_TRACE_INT_AS_PTR(nack),
                          NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL);
        return DDSHST_RETCODE_NOT_EXISTS;
    }

    if (s1_entry->ack_count > 0)
    {
        --s1_entry->ack_count;
    }

    OSAPI_Trace_write("received acknack on SN=%S, nack=%d,count=%^d",
                      sn,OSAPI_TRACE_INT_AS_PTR(nack),&s1_entry->ack_count,
                      NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    if (nack)
    {
        s1_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_ATLEAST_ONE_NACK;
    }
    else
    {
        s1_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_ATLEAST_ONE_ACK;
    }

    if (s1_entry->ack_count <= 0)
    {
        if (s1_entry->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_ATLEAST_ONE_NACK)
        {
            s1_entry->_state |= RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED;
        }

        if (s1_entry->_state & RTI_SMWRITERHISTORY_SAMPLE_STATE_ATLEAST_ONE_ACK)
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
 * \param[in] timestamp The timestamp to use. If NULL get the current time.
 *
 * \return DDSHST_RETCODE_SUCCESS on success, of of the \ref
 *         DDSHST_ReturnCode_T on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
WHSM_History_register_key(struct DDSHST_Writer *wh,
                          const DDS_InstanceHandle_t *const key,
                          const struct OSAPI_SystemTime *timestamp)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    struct WHSM_HistoryKeyEntry *key_entry = NULL;
    struct OSAPI_SystemTime current_timestamp = OSAPI_SystemTime_INITIALIZER;
    struct OSAPI_SystemTime elapsed_time = OSAPI_SystemTime_INITIALIZER;

    if (timestamp == NULL)
    {
        if (!OSAPI_System_get_time(&current_timestamp))
        {
            return DDSHST_RETCODE_ERROR;
        }
    }
    else
    {
        current_timestamp = *timestamp;
    }

    key_entry = WHSM_History_find_key(self, key);

    if (key_entry == NULL)
    {
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
    }
    else if ((self->destination_order.kind ==
              DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
             && !DDS_Duration_is_infinite(
                        &self->destination_order.source_timestamp_tolerance)
             && (OSAPI_SystemTime_compare(&current_timestamp,
                                          &key_entry->source_timestamp) < 0))
    {
       /* Check if the last timestamp of the key is later than the current
        * timestamp when ordering by source timestamp. If it is and the tolerace
        * is exceeded, return error.
        */
        OSAPI_SystemTime_subtract(&elapsed_time,
                                  &key_entry->source_timestamp,
                                  &current_timestamp);
        if (OSAPI_SystemTime_compare(&elapsed_time,
                                     &self->source_timestamp_tolerance) > 0)
        {
            return DDSHST_RETCODE_INVALID_ENTRY_REQUEST;
        }
        current_timestamp = key_entry->source_timestamp;
    }

    /* Even if no samples have yet been committed, the key is ALIVE. This
     * state is used to distinguish between a key that was explicitly registered
     * vs a key that was allocated when need. The former should not be freed
     * in case a queue entry is returned and no samples have been committed,
     * but the latter can.
     */
    key_entry->state = WHSM_HISTORY_KEY_STATE_ALIVE;

    /* Update the key's timestamp to the determined timestamp. This means
     * that the key timestamp is updated on every registration using the same
     * rule as for samples. This is consistent with the DDS specification that
     * states (2.2.2.4.2.6): The source_timestamp potentially affects the
     * relative order in which readers observe events from multiple writers.
     * For details see 2.2.3.17 for the QoS policy DESTINATION_ORDER).
     */
    key_entry->source_timestamp = current_timestamp;

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
                        struct OSAPI_SystemTime *now)
{
    struct WHSM_History *self = (struct WHSM_History *)wh;
    event->event_ignored = RTI_FALSE;

    switch (event->kind)
    {
        case DDSHST_WRITEREVENT_KIND_DEADLINE_EXPIRED:
            WHSM_History_on_deadline_expired(self,now);
            break;
        case DDSHST_WRITEREVENT_KIND_HISTORICAL_DATA_REQUESTED:
            WHSM_History_update_historical_ackcount(wh);
            break;
        case DDSHST_WRITEREVENT_KIND_ENTRY_STATE_QUEUED:
            WHSM_History_set_send_state(wh,&event->data.sn,
                        DDSHST_WRITEREVENT_KIND_ENTRY_STATE_QUEUED);
            break;
        case DDSHST_WRITEREVENT_KIND_ENTRY_STATE_IN_PROGRESS:
            if (!WHSM_History_set_send_state(wh,&event->data.sn,
                        DDSHST_WRITEREVENT_KIND_ENTRY_STATE_IN_PROGRESS))
            {
                event->event_ignored = RTI_TRUE;
            }
            break;
        case DDSHST_WRITEREVENT_KIND_ENTRY_STATE_SEND_COMPLETED:
            WHSM_History_set_send_state(wh,&event->data.sn,
                        DDSHST_WRITEREVENT_KIND_ENTRY_STATE_SEND_COMPLETED);
            break;
#if DDS_FILTERING_ENABLED
        case DDSHST_WRITEREVENT_KIND_PEEK_NEXT_SN:
            WHSM_History_peek_next_sn(wh, event->data.peek_next.sn,
                                      event->data.peek_next.historical_only);
            break;
#endif /* DDS_FILTERING_ENABLED */
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
        state->last_sample_kind =
            ((struct WHSM_HistorySampleEntry*)REDA_CircularList_get_last(&key_entry->samples))->kind;
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

