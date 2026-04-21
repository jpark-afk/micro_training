/*
 * FILE: ReaderHistory.c - Reader History implementation
 *
 * Copyright 2011-2021 Real-Time Innovations, Inc.
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
 * 28oct2021,tk MICRO-3321 / PR.29843
 * - Conditionally include the suspended_threads variable.
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include code for suspended threads when DDS_BLOCKING_READER_ENABLED
 *   is TRUE.
 * 14sep2021,tk MICRO-3148/PR.29492
 * - Properly determine remaining blocking time in case the semaphore is
 *   unblocked but fails to reserve a resource and blocks again in
 *   reserve_entry.
 * - Support a maximum blocking time of 1 year.
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 14apr2021,tk MICRO-2910/PR.28834
 * - Return DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT instead of
 *   DDS_REJECTED_BY_SAMPLES_LIMIT when pruning and the max_samples_per_instance
 *   limit has been reached.
 * - Changed prune_sample() to return DDS_SampleRejectedStatusKind instead of
 *   RTI_BOOL.
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in RHSM_History_return_sample()
 *  - Removed empty blocks in RHSM_History_reserve_entry()
 *  - Removed empty blocks in RHSM_History_on_recommit_event()
 *  - Only updated committed state on success in RHSM_History_on_recommit_event()
 * 04apr2021,tk MICRO-2909/PR.28810
 *  - Set timestamp to zero if it cannot be retrieved in RHSM_History_commit_rw()
 * 21feb2021,tk MICRO-2871/PR#28748
 *   - Return failure if OSAPI_System_get_time() failed in
 *     RHSM_History_reserve_entry().
 *   - Added comment for why it is not safe to return if
 *     a Semaphore_take() call failed in RHSM_History_reserve_entry().
 *   - Block forever on the thread semaphore in RHSM_History_reserve_entry()
 *     if the database cannot be locked.
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 08may2017,tk MICRO-1580/PR#19774 Fixed reclaiming of instances while in a
 *                                  listener.
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 11sep2015,tk MICRO-1500/PR#16545 Correctly increment rw_entry->_size, removed test
 *                                  since it can never be true when using RTPS and it
 *                                  does not impact the INTRA transport since those
 *                                  samples are always committable
 * 29jul2015,tk MICRO-1463/PR#15595 Do not remove a sample with a loan
 *                                  in has_space()
 * 27jul2015,tk MICRO-1426/PR#15358 Check the elapsed time in the periodic
 *                                  timeout without an epoch
 * 27jul2015,tk MICRO-1464/PR#15597 Corrected instance state implementation
 *                                  to be consistent with the specification
 * 20jul2015,tk MICRO-1457/PR#15574 Fixed issues introduced with deadline
 *                                  changes. Reset owner after deadline missed
 *                                  not on periodic deadline check which was
 *                                  correct before.
 * 20jul2015,tk MICRO-1460/PR#15578 Return OUT_OF_RESOURCES if no loan is
 *                                  possible
 * 20jul2015,tk MICRO-1426/PR#15358 Check the elapsed time in the periodic
 *                                  timeout
 * 15jul2015,tk MICRO-1440/PR#15456 Initialize owner_bitmap in initialize_key()
 * 15jul2015,tk MICRO-1434/PR#15398 Remove a disposed instance from the deadline
 *                                  queue even if it has not been read/taken
 * 15jul2015,tk MICRO-1390/PR#15198 Revert change for ownership check, the
 *                                  strongest writer always owns an instance.
 *                                  Fixed bugs related to keeping track of
 *                                  remote writers.
 * 15jul2015,tk MICRO-1426/PR#15358 Check deadline period on each instance
 *                                  update in addition to the deadline timeout
 * 10jul2015,tk MICRO-1398/PR#15292 Fixed updating last history sample
 * 09jul2015,tk MICRO-1398/PR#15264 Fixed error with multiple returns of key
 *                                  buffer when a sequence of samples are
 *                                  returned for a disposed key
 * 29jun2015,tk MICRO-1335/PR#15096 Fixed comment errors and removed redundant
 *                                  code
 * 29jun2015,tk MICRO-1225/PR#14526 Removed redundant code
 * 24jun2015,eh MICRO-1169/PR#14650 Further remove property check from create
 * 15mar2015,tk MICRO-1214/PR#14756 Reiterate over removed index when pruning rw
 * 15mar2015,tk MICRO-1226/PR#14528 Removed redundant KEEP_LAST test in create()
 * 15mar2015,tk MICRO-1225/PR#14526 Consistent check on property in create()
 * 05may2015,tk MICRO-1137/PR#14518 return TRUE in RHSM_History_writer_limit_reached
 *                                  independent of whether listener is installed
 *                                  or not.
 * 12mar2015,tk MICRO-1089 Fixed resource exhaustion with read call
 * 20feb2014,eh MICRO-813/PR#9172 Fix Lint warnings
 * 26jan2015,tk MICRO-1028/PR#13473 Removed magic number 0xc0
 * 08oct2014,tk MICRO-919 Added SampleLostReason to SampleLost status
 * 21sep2014,tk MICRO-879 Reset view_state to NEW when transition from any
 *                        NOT_ALIVE state to ALIVE
 * 31jul2014,tk MICRO-172/PR#1064 - Removed superfluous REDA_Indexer fields
 * 29may2014,tk - ownership code improvement
 *              - resource policy improvements
 *              - cancellation of a queue reservation does not impact resources
 *              - MICRO-790: KEEP_LAST semantics are more precise
 * 10apr2014,tk MICRO-752: Restart deadline timer when instance changes state from
 *                         not_alive to alive.
 * 11mar2014,eh MICRO-731: NULLify ptrs in unlink_ordered_sample(), and get
 *                         next_entry before unlink
 * 05jan2014,tk MICRO-722: Support for instance replacement policy
 * 01aug2013,tk MICRO-676: Stop deadline_missed callback when instance changes
 *                         state to not_alive
 * 14jun2013,tk MICRO-239: Fixed dispose handling
 * 13jun2013,eh MICRO-657: Set reception_timestamp upon commit_sample
 * 12sep2012,tk Major re-write, support for ownership.
 * 25aug2011,yy Fixed instance_state setting
 * 12jun2011,tk Written.
 */
/*ce
 * \file
 * \brief Reader History implementation
 *
 * \details
 * This file implements the DDS data-reader history cache based on the
 * resource-limits and Qos policies. It implements the DDSHST_ReaderI and
 * the methods in ths file are not called directly. Instead they are called
 * with the DDSHST_ReaderI interface.
 */
/*ci \addtogroup RHSMModule
 * @{
 */
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_semaphore
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef dds_c_common_impl
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef rh_sm_log_h
#include "rh_sm/rh_sm_log.h"
#endif

#include "rh_sm/rh_sm_history.h"

#include "ReaderHistory.h"

/*ci
 * \brief Default DDSHST_ReaderListener
 */
RTI_PRIVATE const struct DDSHST_ReaderListener RTI_DEFAULT_READERHISTORY_LISTENER =
                                            DDSHST_ReaderListener_INITIALIZE;

/*ci
 * \brief The RHSM_History interface
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct DDSHST_ReaderI RHSM_HistoryI_fv_Intf;

/*ci
 * \brief Structure used to initialize entries in the pool of remote writers
 */
struct RHSM_HistoryRWEntryInit
{
    /*ci
     * \brief The current index in the pool, incremented for each pool entry
     */
    RTI_UINT32 current_index;

    /*ci
     * \brief The maximum number of entries in the buffer pool
     */
    RTI_INT32 max_rw;
};

SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_History_delete_key(struct RHSM_History *self,
                        struct RHSM_HistoryKeyEntry *key);

/*ci
 * \brief The index for the object id of an entity in an octet array
 */
#define RHSM_HISTORY_OBJECTID_INDEX (15)

/*ci
 * \brief The maximum blocking time in a call to Semaphore_take in
 *        reserve_entry.
 *
 * \details
 * If DataReader's maximum_blocking_time.sec exceeds 2000000L, multiple
 * calls to Semaphore_take is used to avoid overflow in conversion to
 * signed 32-bit millisecond value.
 */
#define RHSM_MAX_BLOCKING_TIME_SEC (2000000L)

/*** SOURCE_BEGIN ***/

/*******************************************************************************
 *                                RTI_PRIVATE API
 ******************************************************************************/
/*ci
 * \brief REDA_Indexer_T compare function for keys
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just the key
 * \param[in] key           An existing indexed record or a key
 *
 * \return 0 if record = key,
 *         positive integer if record > key,
 *         negative integer if record < key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RHSM_History_compare_key_record(const void *const record,
                                RTI_BOOL key_is_record,
                                const void *const key)
{
    struct RHSM_HistoryKeyEntry *record_left =
                                        (struct RHSM_HistoryKeyEntry*)record;
    const DDS_InstanceHandle_t *key_right;

    if (key_is_record)
    {
        key_right = &((struct RHSM_HistoryKeyEntry*)key)->_key;
    }
    else
    {
        key_right = (const DDS_InstanceHandle_t*)key;
    }

    return DDS_InstanceHandle_compare(&record_left->_key,key_right);
}

/*ci
 * \brief REDA_Indexer_T compare function for remote writers
 *
 * \param[in] record        An existing indexed record
 * \param[in] key_is_record Whether the key is a full record or just the key
 * \param[in] key           An existing indexed record or a key
 *
 * \return 0 if record = key,
 *         positive integer if record > key,
 *         negative integer if record < key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RHSM_History_compare_rw_record(const void *const record,
                               RTI_BOOL key_is_record,
                               const void *const key)
{
    struct RHSM_HistoryRWEntry *record_left =
                                        (struct RHSM_HistoryRWEntry*)record;
    const DDS_InstanceHandle_t *key_right;

    if (key_is_record)
    {
        key_right = &((struct RHSM_HistoryRWEntry*)key)->_key;
    }
    else
    {
        key_right = (const DDS_InstanceHandle_t*)key;
    }

    return DDS_InstanceHandle_compare(&record_left->_key,key_right);
}

/*ci
 * \brief Initialize a remote writer structure
 *
 * \param[inout] rw_entry The entry to initialize
 * \param[in]    key      The entry's key
 */
RTI_PRIVATE void
RHSM_HistoryRWEntry_initialize(struct RHSM_HistoryRWEntry *rw_entry,
                               DDS_InstanceHandle_t *key)
{
    rw_entry->_key = *key;
    REDA_CircularList_init(&rw_entry->_cached);
    rw_entry->_size = 0;
    rw_entry->_strength = 0;
    REDA_SequenceNumber_set_zero(&rw_entry->highest_noncommmitable_sn);
    REDA_SequenceNumber_set_zero(&rw_entry->last_virtual_sn);
    rw_entry->_notify_on_commit = DDS_BOOLEAN_TRUE;

    /* NOTE:
     * bitnum and index is initialized _once_ when the buffer is created and
     * must never change for the duration of the buffer-pool
     */
}

/*ci
 * \brief Initialize the ordered list elements in a sample
 *
 * \param[in] sample The sample to initialize
 */
RTI_PRIVATE void
RHSM_History_init_ordered_list(struct RHSM_HistorySample *sample)
{
    sample->_ordered_next = sample;
    sample->_ordered_prev = sample;
}

/*ci
 * \brief Check if the ordered sample list is empty
 *
 * \param[in] sample The sample to check
 *
 * \return DDS_BOOLEAN_TRUE if empty, DDS_BOOLEAN_FALSE if not
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
RHSM_History_ordered_list_is_empty(struct RHSM_HistorySample *sample)
{
    return (sample->_ordered_next == sample ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Initialize a sample entry in the history cache
 *
 * \param[in] sample The sample entry to initialize
 */
RTI_PRIVATE void
RHSM_History_initialize_sample(struct RHSM_HistorySample *sample)
{
    REDA_CircularListNode_init(&sample->_node);
    REDA_CircularListNode_init(&sample->prune_node);
    sample->_key_entry = NULL;
    sample->_loan_count = 0;
    sample->_flags = 0;
    sample->_ordered_next = NULL;
    sample->_ordered_prev = NULL;
    sample->_sample = NULL;
}

/*ci
 * \brief Initialize a key entry in the history cache
 *
 * \param[in] self The history cache
 * \param[in] key  The key entry to initialize
 */
RTI_PRIVATE void
RHSM_History_initialize_key(struct RHSM_History *self,
                            struct RHSM_HistoryKeyEntry *key)
{
    key->_sample_count = 0;
    key->_not_seen_count = 0;
    key->_history_depth = 0;
    REDA_CircularList_init(&key->_samples);
    REDA_CircularList_init(&key->_prune);
    key->_key = DDS_HANDLE_NIL;
    key->_current_owner = NULL;
    key->has_owner = RTI_FALSE;
    key->_writer_count = 0;
    key->is_disposed = RTI_FALSE;
    key->first_history_sample = (struct RHSM_HistorySample*)&key->_samples;
    key->last_history_sample = (struct RHSM_HistorySample*)&key->_samples;

    DDS_Duration_set(&key->last_update_time,0,DDS_DURATION_INFINITE_NSEC);
    DDS_Duration_set(&key->last_period_time,0,DDS_DURATION_INFINITE_NSEC);

    if (self->owner_length > 0)
    {
        OSAPI_Memory_zero(key->owner_array,
                  (RTI_SIZE_T)self->owner_length *
                  (RTI_SIZE_T)sizeof(struct RHSM_HistoryRWEntry*));
    }
    else
    {
        OSAPI_Memory_zero(key->owner_bitmap,
                       MAX_OWNER_BITMAP_LENGTH * MAX_OWNER_BITMAP_ELEMENT_SIZE);
    }

    REDA_CircularListNode_init(&key->_node);
    REDA_CircularListNode_init(&key->_deadline_entry);
    REDA_CircularListNode_init(&key->_timeline_entry);
    RHSM_History_initialize_sample(&key->meta_sample);

    key->meta_sample._key_entry = key;
}

/*ci
 * \brief Append a sample to the ordered list
 *
 * \param[in] after  An existing sample to append the sample to
 * \param[in] sample The sample to append
 */
RTI_PRIVATE void
RHSM_History_insert_ordered_sample_after(struct RHSM_HistorySample *after,
                                         struct RHSM_HistorySample *sample)
{
    sample->_ordered_next = after->_ordered_next;
    sample->_ordered_next->_ordered_prev = sample;
    sample->_ordered_prev = after;
    after->_ordered_next = sample;
}

/*ci
 * \brief Unlink a sample from the ordered list
 *
 * \param[in] sample The sample to unlink
 */
RTI_PRIVATE void
RHSM_History_unlink_ordered_sample(struct RHSM_HistorySample *sample)
{
    sample->_ordered_prev->_ordered_next = sample->_ordered_next;
    sample->_ordered_next->_ordered_prev = sample->_ordered_prev;
    sample->_ordered_prev = NULL;
    sample->_ordered_next = NULL;
}

/*ci
 * \brief Return a sample to the pool of free sample entries
 *
 * \details
 *
 * A sample is no longer available to the user and should be returned to the
 * sample pool. If the sample is loaned it is appended after the current
 * historical samples (based on keep_last and depth). When the loan is returned
 * it is returned to the pool. RHSM_History_return_sample() may be called
 * multiple time on the same sample. This happens if the sample is loaned
 * and then later returned.
 *
 * \param[in] self        The history cache
 * \param[in] sample      The sample to be returned to the history cache
 * \param[in] lost_reason The reason for the sample being returned to the sample
 *                        pool. Note that the lost_reason
 *                        DDS_SAMPLE_LOST_NOT_LOST indicates a normal return
 */
SHOULD_CHECK_RETURN RTI_PRIVATE void
RHSM_History_return_sample(struct RHSM_History *self,
                           struct RHSM_HistorySample *sample,
                           DDS_SampleLostStatusKind lost_reason)
{
    /* A returned sample is always made unavailable to the user
     * If the sample is unlinked multiple times then nothing happens here after
     * the 1st unlink.
     */
    if ((sample->_ordered_next != NULL) && (sample->_ordered_prev != NULL))
    {
        RHSM_History_unlink_ordered_sample(sample);
    }

    if (REDA_CircularListNode_is_linked(&sample->_node))
    {
        /* Update the first and last sample if this sample is either or both */
        if (sample->_key_entry != NULL)
        {
            if (sample == sample->_key_entry->first_history_sample)
            {
                sample->_key_entry->first_history_sample =
                    (struct RHSM_HistorySample*)REDA_CircularListNode_get_next(
                            &sample->_key_entry->first_history_sample->_node);
            }

            /* Unlink the removed here, otherwise last_history_sample will point
             * to the removed node.
             */
            REDA_CircularList_unlink_node(&sample->_node);

            if (sample == sample->_key_entry->last_history_sample)
            {
                sample->_key_entry->last_history_sample =
                        (struct RHSM_HistorySample*)REDA_CircularList_get_last(
                                                &sample->_key_entry->_samples);
            }
        }
        else
        {
            REDA_CircularList_unlink_node(&sample->_node);
        }
    }

    REDA_CircularList_unlink_node(&sample->prune_node);

    if (sample->_flags & RHSM_HISTORY_FLAG_META)
    {
        if ((sample->sample_state == DDS_NOT_READ_SAMPLE_STATE) &&
            (self->_listener.on_sample_lost != NULL) &&
            (lost_reason != DDS_SAMPLE_LOST_NOT_LOST))
        {
            /* A sample with META information must belong to a
             * key.
             */
            /* coverity[var_deref_op] */
            sample->_info.view_state = (DDS_ViewStateKind)sample->_key_entry->_view_state;
            sample->_info.instance_state = (DDS_InstanceStateKind)sample->_key_entry->_instance_state;
            self->_listener.on_sample_lost((struct DDSHST_Reader*)self,
                                            self->_listener.listener_data,
                                            &sample->_info,lost_reason);
        }

        /* If the meta sample is returned and it was in use,
         * decrement the _not_seen_count, it will be updated
         * when a new meta sample is added.
         */
        if ((sample->_loan_count == 0) &&
            (sample->_flags & RHSM_HISTORY_FLAG_META_INUSE))
        {
            sample->_flags &= (RTI_UINT8)~RHSM_HISTORY_FLAG_META_INUSE;

            /* sample->_key_entry cannot be INUSE and
             * sample->_key_entry == NULL. Mark the Coverity [var_deref_op]
             * event as 'Intentional'.
             */
            /* coverity[var_deref_op] */
            --sample->_key_entry->_not_seen_count;
        }

        return;
    }

    if (sample->_flags & RHSM_HISTORY_FLAG_HISTORICAL)
    {
        sample->_flags &= (RTI_UINT8)~RHSM_HISTORY_FLAG_HISTORICAL;
        /* A historical sample must belong to a
         * key.
         */
        /* coverity[var_deref_op] */
        --sample->_key_entry->_history_depth;
    }

    if (sample->_loan_count != 0)
    {
        /* If the sample is loaned it cannot be returned to the pool yet */
        return;
    }

    /* The sample can be safely returned to the pool */
    if (sample->_sample && self->_listener.on_sample_removed)
    {
        self->_listener.on_sample_removed((struct DDSHST_Reader *)self,
                                          self->_listener.listener_data,
                                          &sample->_key_entry->_key,
                                          sample->_sample);
    }

    if ((sample->sample_state == DDS_NOT_READ_SAMPLE_STATE) &&
        (self->_listener.on_sample_lost != NULL) &&
        (sample->_key_entry != NULL) &&
        (lost_reason != DDS_SAMPLE_LOST_NOT_LOST))
    {
        sample->_info.view_state = (DDS_ViewStateKind)sample->_key_entry->_view_state;
        sample->_info.instance_state = (DDS_InstanceStateKind)sample->_key_entry->_instance_state;
        self->_listener.on_sample_lost((struct DDSHST_Reader*)self,
                                        self->_listener.listener_data,
                                        &sample->_info,lost_reason);
    }

    sample->_sample = NULL;

    if (sample->_key_entry != NULL)
    {
        --sample->_key_entry->_sample_count;
        --sample->_key_entry->_not_seen_count;
    }

    REDA_BufferPool_return_buffer(self->_sample_pool,sample);

#if DDS_BLOCKING_READER_ENABLED
    {
        RTI_BOOL brc;

        /* Only unblock one blocked thread since only one sample has been returned.
         * The blocking list is FIFO and new threads are appended. Note that
         * it is not guaranteed that the blocked thread will get a resource and
         * may end up in the list again.
         */
        if (!REDA_CircularList_is_empty(&self->suspended_threads))
        {
            struct RHSM_ThreadSuspendState *pobj =
                    (struct RHSM_ThreadSuspendState*)
                    REDA_CircularList_get_first(&self->suspended_threads);
            REDA_CircularList_unlink_node(&pobj->_node);
            brc = OSAPI_Semaphore_give(pobj->wakeup_sem);
            IGNORE_RETVAL(brc);
        }
    }
#endif
}

/*ci
 * \brief Return all samples in a sample list to the pool of free samples
 *
 * \param[in] self    The history cache
 * \param[in] samples A list of sample to return
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_History_delete_sample_list(struct RHSM_History *self,
                                REDA_CircularList_T *samples)
{
    struct RHSM_HistorySample *sample = NULL;
    struct RHSM_HistorySample *sample2 = NULL;

    sample = (struct RHSM_HistorySample*)REDA_CircularList_get_first(samples);
    while (!REDA_CircularList_node_at_head(samples,&sample->_node))
    {
        sample2 = (struct RHSM_HistorySample*)
                                REDA_CircularListNode_get_next(&sample->_node);
        RHSM_History_return_sample(self,sample,
                                   DDS_SAMPLE_LOST_BY_NOT_READ_ON_CACHE_DELETION);
        sample = sample2;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Return all samples for a key to the sample pool and return the
 *        key to the pool of available keys
 *
 * \param[in] self The history cache
 * \param[in] key  The key to delete
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_History_delete_key(struct RHSM_History *self,
                        struct RHSM_HistoryKeyEntry *key)
{
    if (!RHSM_History_delete_sample_list(self,&key->_samples))
    {
        return RTI_FALSE;
    }

    if (self->_listener.on_key_removed)
    {
        self->_listener.on_key_removed((struct DDSHST_Reader *)self,
                                       self->_listener.listener_data,
                                       &key->_key);
    }

    if (self->deadline_enabled)
    {
        REDA_CircularList_unlink_node(&key->_deadline_entry);
    }

    if (self->_qos->reader_resource_limits.instance_replacement ==
                                  DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS)
    {
        REDA_CircularList_unlink_node(&key->_timeline_entry);
    }

    REDA_CircularList_unlink_node(&key->_node);

    REDA_Indexer_remove_entry(self->_key_index,&key->_key);

    REDA_BufferPool_return_buffer(self->_key_pool,key);

    return RTI_TRUE;
}

/*ci
 * \brief Free all samples currently owned by a remote writer entry and return
 *        the remote writer entry to the pool of free remote writers
 *
 * \param[in] self The history cache
 * \param[in] rw   The remote writer to delete
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_History_delete_rw(struct RHSM_History *self,
                       struct RHSM_HistoryRWEntry *rw)
{
    if (!RHSM_History_delete_sample_list(self,&rw->_cached))
    {
        return RTI_FALSE;
    }

    REDA_Indexer_remove_entry(self->_rw_index,&rw->_key);

    REDA_BufferPool_return_buffer(self->_rw_pool,rw);

    return RTI_TRUE;
}

/*ci
 * \brief After new samples have been committed update the committed state
 *
 * \details
 * This function is called after new samples have been committed. The only
 * action required with DDS is to notify the datareader (the history owner)
 * that data is available. The DDS APIs makes the data available to the
 * application.
 *
 * \param[in] self This history cache
 */
RTI_PRIVATE void
RHSM_History_update_committed(struct RHSM_History *self)
{
    self->in_listener = RTI_TRUE;

    if (self->_listener.on_data_available)
    {
        self->_listener.on_data_available((struct DDSHST_Reader *)self,
                                          self->_listener.listener_data,
                                          NULL, NULL);
    }

    self->in_listener = RTI_FALSE;
}

/*ci
 * \brief Update the history cache with a new sample
 *
 * \details
 * A sample must go through a committing process to determine if the sample
 * can be added to the cache or rejected for some reason. If the sample
 * cannot be added it is returned to a pool of free sample entries. If the
 * sample can be added the state of the cache may be updated, such as which
 * samples are historical.
 *
 * \param[in]    self        The history cache
 * \param[in]    sample      The sample to add to the cache
 * \param[inout] new_samples TRUE is a new sample was added to the queue
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_History_commit_sample(struct RHSM_History *self,
                           struct RHSM_HistorySample *sample,
                           RTI_BOOL *new_samples)
{
    struct RHSM_HistoryKeyEntry *key;
    RTI_INT32 level;
    RTI_BOOL sample_rejected = RTI_FALSE;

    key = sample->_key_entry;

    /* _sample_count and sample->_key_entry->_not_seen_count is updated first
     * in case the sample is rejected and returned with return_sample().
     * Meta samples do not count against the resources
     */
    if (!(sample->_flags & RHSM_HISTORY_FLAG_META))
    {
        ++sample->_key_entry->_sample_count;
    }

    ++sample->_key_entry->_not_seen_count;

    if (sample->_info.valid_data)
    {
        /* _sample_count was incremented optimistically, thus check against
         * self->_qos->resource_limits.max_samples_per_instance + 1. This
         * does _not_ mean there is an additional sample.
         */
        if (key->_sample_count ==
            (self->_qos->resource_limits.max_samples_per_instance + 1))
        {
            if (REDA_CircularList_is_empty(&key->_samples))
            {
                /* This can happen if all the samples in the current history
                 * has been "taken()".
                 */
                sample_rejected = RTI_TRUE;
            }
            else
            {
                /* This can happen if all the samples in the current history
                 * are "read()" from the cache and still on loan.
                 */
                if (key->first_history_sample->_loan_count > 0)
                {
                    /* The sample will be removed from the history, but cannot
                     * be reused.
                     */
                    sample_rejected = RTI_TRUE;
                }

                /* Remove the first sample even if it cannot be reused.
                 */
                if (!(sample->_flags & RHSM_HISTORY_FLAG_RECOMMITTED))
                {
                    RHSM_History_return_sample(self,key->first_history_sample,
                                           DDS_SAMPLE_LOST_BY_HISTORY_DEPTH_LIMIT);
                }
            }

            if (sample_rejected)
            {
                /* _sample_count and _not_seen_count is decremented again here.
                 * Note that the sample is not returned because it it kept in
                 * the writer cache.
                 */
                --sample->_key_entry->_sample_count;
                --sample->_key_entry->_not_seen_count;

                return RTI_FALSE;
            }
        }

        if (key->_history_depth == self->_qos->history.depth)
        {
            /* max_samples_per_instance has not been reached or it has
             * been reached but the first historical sample can be
             * reclaimed or depth < max_samples_per_instance.
             *
             * In all these cases remove the oldest sample.
             */
            RHSM_History_return_sample(self,key->first_history_sample,
                                   DDS_SAMPLE_LOST_BY_HISTORY_DEPTH_LIMIT);
        }
    }

    /* At this point there is guaranteed to be some activity on the
     * instance, move it to the end of the timeline
     */
    if (self->_qos->reader_resource_limits.instance_replacement ==
                            DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS)
     {
        REDA_CircularList_unlink_node(&key->_timeline_entry);
        REDA_CircularList_append(&self->_instance_timeline,
                                 &key->_timeline_entry);
     }

    if ((!sample->_info.valid_data) && (sample != &key->meta_sample))
    {
        /* Replace previously allocated sample with the internal one
         * NOTE: sample_count is decremented and so is _not_seen_count
         */
        key->meta_sample._info = sample->_info;
        RHSM_History_return_sample(self,sample,DDS_SAMPLE_LOST_NOT_LOST);

        /* The meta sample could already be used, set it as meta in
         * case it is not used
         */
        sample = &key->meta_sample;
        sample->_flags |= RHSM_HISTORY_FLAG_META;
        RHSM_History_return_sample(self,sample,
                                   DDS_SAMPLE_LOST_BY_META_SAMPLE_LIMIT);
        RHSM_History_initialize_sample(&key->meta_sample);
        key->meta_sample._key_entry = key;
        sample->_flags = RHSM_HISTORY_FLAG_META |
                         RHSM_HISTORY_FLAG_META_INUSE |
                         RHSM_HISTORY_FLAG_HISTORICAL;

        /* The _not_seen_count counter was decremented when the real
         * sample was returned, increment again here. Do not increment
         * sample_count because the meta sample does not count against
         * it
         */
        ++sample->_key_entry->_not_seen_count;
    }
    else if (sample->_info.valid_data)
    {
        sample->_flags |= RHSM_HISTORY_FLAG_HISTORICAL;
        ++key->_history_depth;
        REDA_CircularList_append(&key->_prune,&sample->prune_node);
    }

    sample->sample_state = DDS_NOT_READ_SAMPLE_STATE;

    if (sample->_info.valid_data)
    {
        REDA_CircularList_unlink_node(&key->_node);

        if (key->_history_depth >= self->_qos->history.depth)
        {
            level = RHSM_REMOVE_LEVEL_EQ;
        }
        else
        {
            level = RHSM_REMOVE_LEVEL_LT;
        }

        REDA_CircularList_append(&self->remove_level[level],&key->_node);
    }

    /* Always append to the end of the history
     */
    REDA_CircularList_link_node_after(&key->last_history_sample->_node,
                                      &sample->_node);
    key->last_history_sample = sample;

    /* There are two cases for updating the first_history_sample:
     * - This is the first sample added to the history. In this case update
     *   update first_history_sample.
     *
     * - There are existing sample. In this case nothing needs to be done.
     *
     * Any removal of the oldest sample due to the history depth being reached
     * has already happened.
     */
    if (REDA_CircularList_node_at_head(&key->_samples,key->first_history_sample))
    {
        key->first_history_sample = key->last_history_sample;
    }

    RHSM_History_insert_ordered_sample_after(self->_s_ordered._ordered_prev,sample);

    (*new_samples) = RTI_TRUE;

    return RTI_TRUE;
}

/*ci
 * \brief Add meta-sample to reader queue to signal change in instance state
 *
 * \details
 * Add a meta-sample to the reader queue to signal the state of an instance.
 * This function is only called to add not_alive_no_writers or not_alive_diposed
 * instance state. The alive instance state is only set when accompanied
 * by a valid sample.
 *
 * \param self The history cache
 * \param key  The key the meta sample applies to
 * \param publisher_handle The publisher that caused the meta sample to be
 *                         added
 * \param instance_state   The new instance state for the key
 * \param now              The current time-stamp
 */
RTI_PRIVATE void
RHSM_History_add_meta_sample(struct RHSM_History *self,
                             struct RHSM_HistoryKeyEntry *key,
                             const DDS_InstanceHandle_t *publisher_handle,
                             DDS_InstanceStateKind instance_state,
                             struct OSAPI_NtpTime *now)
{
    struct DDS_SampleInfo *sample_info;
    struct RHSM_HistorySample *new_entry = NULL;
    RTI_BOOL new_samples = 0;

    /* Because the instance state is either not_alive or disposed remove it
     * from the deadline queue to prevent any more call backs for deadline
     * missed.
     */
    if (self->deadline_enabled)
    {
        REDA_CircularList_unlink_node(&key->_deadline_entry);
    }

    new_entry = &key->meta_sample;
    new_entry->_flags |= RHSM_HISTORY_FLAG_META;

    /* The meta-sample can be reused even if it is loaned since there is no
     * data associated with it. Setting the loan_count to 0 ensure that it
     * is actually released.
     */
    new_entry->_loan_count = 0;

    RHSM_History_return_sample(self,new_entry,
                               DDS_SAMPLE_LOST_BY_META_SAMPLE_LIMIT);

    RHSM_History_initialize_sample(new_entry);
    new_entry->_flags |= RHSM_HISTORY_FLAG_META | RHSM_HISTORY_FLAG_META_INUSE;
    new_entry->_key_entry = key;
    sample_info = &new_entry->_info;
    sample_info->sample_state = DDS_NOT_READ_SAMPLE_STATE;
    sample_info->view_state = (DDS_ViewStateKind)key->_view_state;
    key->_instance_state = (DDS_Octet)instance_state;
    sample_info->instance_state = (DDS_InstanceStateKind)key->_instance_state;
    new_entry->sample_state = (RTI_UINT8)sample_info->sample_state;

    OSAPI_NtpTime_to_nanosec(&sample_info->source_timestamp.sec,
                             &sample_info->source_timestamp.nanosec,now);

    sample_info->instance_handle = key->_key;
    sample_info->instance_handle.is_valid = DDS_BOOLEAN_TRUE;
    sample_info->publication_handle = *publisher_handle;
    sample_info->publication_handle.is_valid = DDS_BOOLEAN_TRUE;

    /* None of the counters are supported */
#if DDS_INCLUDE_SAMPLE_INFO_RANKS
    sample_info->disposed_generation_count = 0;
    sample_info->no_writers_generation_count = 0;
    sample_info->sample_rank = 0;
    sample_info->generation_rank = 0;
    sample_info->absolute_generation_rank = 0;
#endif

    /* This is a meta-sample */
    sample_info->valid_data = DDS_BOOLEAN_FALSE;

    /* The meta sample does not count against the resource limits and hence
     * cannot block.
     */
    if (!RHSM_History_commit_sample(self,new_entry,&new_samples))
    {
        RHSM_LOG_HISTORY_UPDATE(OSAPI_LOGKIND_ERROR)
        return;
    }

    RHSM_History_update_committed(self);
}

/*ci
 * \brief Find a remote writer entry by its key
 *
 * \param[in] self The history cache
 * \param[in] rw   The remote writer key
 *
 * \return Pointer to remote writer entry if it exists, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RHSM_HistoryRWEntry*
RHSM_History_find_rw(struct RHSM_History *self,
                     const DDS_InstanceHandle_t *const rw)
{
    return (struct RHSM_HistoryRWEntry*)REDA_Indexer_find_entry(
                                                        self->_rw_index,rw);
}

/*ci
 * \brief Remove a remote writer as an updater of the specified key
 *
 * \param[in] self            The history cache
 * \param[in] key_entry       The key to remove from the writer
 * \param[in] rw_entry        The remote writer
 * \param[in] check_key_alive Check if the key is still alive and add
 *                            meta sample if not
 * \param[in] now             The current time-stamp
 */
RTI_PRIVATE void
RHSM_History_remove_remote_writer(struct RHSM_History *self,
                                  struct RHSM_HistoryKeyEntry *key_entry,
                                  struct RHSM_HistoryRWEntry *rw_entry,
                                  RTI_BOOL check_key_alive,
                                  struct OSAPI_NtpTime *now)
{
    RTI_INT32 i,j;

    if (self->owner_length == 0)
    {
        /* Not a writer for this instance, or already removed */
        if (!(key_entry->owner_bitmap[rw_entry->index] & rw_entry->bitnum))
        {
            return;
        }
        key_entry->owner_bitmap[rw_entry->index] &= ~rw_entry->bitnum;
    }
    else
    {
        j = 0;
        for (i = 0; (i < self->owner_length) && (j < key_entry->_writer_count); ++i)
        {
            if (key_entry->owner_array[i] == rw_entry)
            {
                key_entry->owner_array[i] = NULL;
                break;
            }
            else if (key_entry->owner_array[i] != NULL)
            {
                ++j;
            }
        }

        if (j == key_entry->_writer_count)
        {
            return;
        }
    }

    --key_entry->_writer_count;

    if ((key_entry->_current_owner == rw_entry) ||
        (key_entry->_writer_count == 0))
    {
        key_entry->_current_owner = NULL;
        key_entry->has_owner = RTI_FALSE;
    }

    /* NOTE: It does not matter if the instance was disposed or not.
     * When the last known writer times out, the instance can either
     * be already disposed or not. If it is disposed it will be reclaimed
     * when the dispose is returned and if it was not disposed then it will
     * not be reclaimed. However, even if the sample was disposed the
     * instance state will still be NOT_ALIVE, because that is the true
     * instance state.
     */
    if (check_key_alive && (key_entry->_writer_count == 0) &&
        !DDS_ObjectId_is_builtin(rw_entry->_key.octet[RHSM_HISTORY_OBJECTID_INDEX]) &&
        !key_entry->is_disposed)
    {
        RHSM_History_add_meta_sample(self,
                key_entry,&key_entry->_key,
                DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE,now);
    }
}

/*ci
 * \brief Check if the limit for the number of remote writers have been reached
 *        for a key
 *
 * \param[in] self      The history cache
 * \param[in] key_entry The key to check the limit on
 *
 * \return RTI_TRUE if the maximum number of writers for a key has been reached,
 *         RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
RHSM_History_writer_limit_reached(struct RHSM_History *self,
                                  struct RHSM_HistoryKeyEntry *key_entry)
{
    return ((key_entry->_writer_count ==
            self->_qos->reader_resource_limits.max_remote_writers_per_instance)
            ? RTI_TRUE : RTI_FALSE);
}

/*ci
 * \brief Add a remote writer as an updater for the specified key
 *
 * \details
 *
 * For each key the cache keeps track of up to max_remote_writers_per_instance
 * remote writers updating the key. This is used to signal
 * NOT_ALIVE_NO writers in case liveliness expires on all the known writers
 * updating a key. It is important to note that the cache does not keep track
 * of the max_remote_writers_per_instance strongest writers. This is not
 * required. There may be more than max_remote_writers_per_instance
 * updating an instance, but only the state of the current view of the first
 * max_remote_writers_per_instance alive writers are maintained. It is not
 * common that an instance is updated by more than a few writers
 * (for redundancy), thus this function uses a simple linear search.
 *
 * NOTE: Ownership is updated _after_ this function returns.
 *
 * \param[in] self      The history cache
 * \param[in] key_entry The key the remote writer is updating
 * \param[in] rw_entry  The remote writer updating the key
 * \param[in] is_owner  TRUE if the writer is the new owner of the instance
 *
 * \return RTI_TRUE if the remote writer was successfully added,
 *         RTI_FALSE if the writer could not be added for the instance
 */
RTI_PRIVATE RTI_BOOL
RHSM_History_add_remote_writer(struct RHSM_History *self,
                               struct RHSM_HistoryKeyEntry *key_entry,
                               struct RHSM_HistoryRWEntry *rw_entry,
                               RTI_BOOL is_owner)
{
    RTI_INT32 i,j,k;
    RTI_INT32 owner_index = -1;

    if (self->owner_length == 0)
    {
        /* Already a writer for this instance, or already added */
        if (!(key_entry->owner_bitmap[rw_entry->index] & rw_entry->bitnum))
        {
            if (RHSM_History_writer_limit_reached(self,key_entry))
            {
                if (!is_owner)
                {
                    return RTI_FALSE;
                }
                /* Replace the owner, There may must always be an owner if the
                 * resource-limit is reached otherwise the resource-limits could
                 * not be reached.
                 */
                key_entry->owner_bitmap[rw_entry->index] &= ~key_entry->_current_owner->bitnum;
                key_entry->owner_bitmap[rw_entry->index] |= rw_entry->bitnum;
            }
            else
            {
                key_entry->owner_bitmap[rw_entry->index] |= rw_entry->bitnum;
                ++key_entry->_writer_count;
            }
        }

        return RTI_TRUE;
    }

    /* Find space if the writer does not already exist. If no space exists,
     * return RTI_FALSE unless the writer is the new owner in which the current
     * owner is replaced.
     */
    j = 0;
    k = -1;
    for (i = 0; (i < self->owner_length) &&
                ((j < key_entry->_writer_count) ||
                (k == -1)); ++i)
    {
        if (key_entry->owner_array[i] == rw_entry)
        {
            return RTI_TRUE;
        }

        /* If the resource limit has been exceeded and the writer is the
         * owner, the current owner will be replaced, save the index for
         * the current owner.
         */
        if (key_entry->owner_array[i] == key_entry->_current_owner)
        {
            owner_index = i;
        }

        if (key_entry->owner_array[i] != NULL)
        {
            ++j;
        }
        else if (k == -1)
        {
            /* Take the first available spot */
            k = i;
        }
    }

    /* The writer was not known, add it if possible */
    if (!RHSM_History_writer_limit_reached(self,key_entry))
    {
        /* If the resource-limit has not been reached then k must != -1 */
        key_entry->owner_array[k] = rw_entry;
        ++key_entry->_writer_count;
        return RTI_TRUE;
    }
    else if (is_owner)
    {
        /* Resource-limit has been reached, replace the current owner since the
         * new writer is the new owner.
         */
        key_entry->owner_array[owner_index] = rw_entry;

        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci
 * \brief Remove a writer as an updater to the instance
 *
 * \param[in] self        The history cache
 * \param[in] writer      The writer removed from the instance
 * \param[in] now         The time of the event
 * \param[in] delete_key  Whether to remove the writer key itself
 */
RTI_PRIVATE void
RHSM_History_remove_writer_from_instance(struct RHSM_History *self,
                                         struct RHSM_HistoryRWEntry *writer,
                                         struct OSAPI_NtpTime *now,
                                         RTI_BOOL delete_key)
{
    struct RHSM_HistoryKeyEntry *key_entry;
    REDA_IndexIterator_T *iterator;

    iterator = REDA_Indexer_iterator_begin(self->_key_index);

    key_entry = REDA_Indexer_iterator_next(iterator);

    while (key_entry != NULL)
    {
        RHSM_History_remove_remote_writer(self,key_entry,writer,RTI_TRUE,now);

        /* If a built-in writer is removed then also delete the writer key
         * itself.
         */
        if (delete_key && DDS_ObjectId_is_builtin(
                writer->_key.octet[RHSM_HISTORY_OBJECTID_INDEX]))
        {
            RHSM_History_delete_key(self, key_entry);
        }

        key_entry = REDA_Indexer_iterator_next(iterator);
    }
}

/*ci
 * \brief Handle liveliness detected on a datawriter
 *
 * \details
 * Handle liveliness detected on a datawriter. Liveliness detected on a
 * writer will cause instances written by that writer be to alive.
 *
 * \param[in] self  The history cache
 * \param[in] event The liveliness event
 * \param[in] now   The time of the event
 */
RTI_PRIVATE void
RHSM_History_update_liveliness(struct RHSM_History *self,
                               const struct DDSHST_ReaderEvent *const event,
                               struct OSAPI_NtpTime *now)
{
    struct RHSM_HistoryRWEntry *rw_entry;

    /* It is not possible to update any instance state based on a
     * datawriter coming coming back alive. Instance state can only
     * switch back to alive if samples are received.
     */
    if (event->kind != DDSHST_READEREVENT_KIND_LIVELINESS_LOST)
    {
        return;
    }

    rw_entry = (struct RHSM_HistoryRWEntry *)REDA_Indexer_find_entry(
                             self->_rw_index,&event->data.liveliness.rw_guid);
    if (rw_entry == NULL)
    {
        /* This is not an error */
        return;
    }

    RHSM_History_remove_writer_from_instance(self,rw_entry,now,RTI_FALSE);
}

/*ci
 * \brief Remove a remote writer
 *
 * \param[in] rh       The reader history cache
 * \param[in] rw_entry The remote writer to remove
 */
RTI_PRIVATE void
RHSM_History_prune_rw(struct DDSHST_Reader *rh,
                      struct RHSM_HistoryRWEntry *rw_entry)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct OSAPI_NtpTime now;

    if (!OSAPI_System_get_time(&now))
    {
        /* If we fail to get the time, set time to maximum in the future.
         * It does not have any impact on functionality.
         */
        now.sec = OSAPI_NTP_TIME_SEC_MAX;
        now.frac = OSAPI_NTP_TIME_FRAC_MAX;
        RHSM_LOG_GET_TIME(OSAPI_LOGKIND_WARNING)
    }

    RHSM_History_remove_writer_from_instance(self,rw_entry,&now,RTI_TRUE);

    RHSM_History_delete_rw(self,rw_entry);
}

/*ci
 * \brief Check if deadline has expired on any instance in the history cache
 *
 * \details
 * NOTE: Timeouts are async and to other code paths and must thus protect
 * any data-structure access.
 * \p
 * This function is called periodically based on events posted to the
 * cache.
 *
 * \param[in] self The history cache
 */
#ifdef ENABLE_QOS_DEADLINE
RTI_PRIVATE void
RHSM_History_on_deadline_expired(struct RHSM_History *self)
{
    struct RHSM_HistoryKeyEntry *key_entry;
    struct REDA_CircularListNode *this_node;
    struct DDS_Duration_t tick_time;

    this_node = REDA_CircularList_get_first(&self->_deadline_timer);
    if (!OSAPI_System_get_ticktime(&tick_time.sec,&tick_time.nanosec))
    {
        return;
    }

    while (!REDA_CircularList_node_at_head(&self->_deadline_timer,this_node))
    {
        key_entry = RHSM_HistoryKeyEntry_from_node(this_node,_deadline_entry);

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

            /* Since the deadline was missed, reset sample update timeout
             * so that the deadline period starts again from the next
             * sample received.
             */
            key_entry->last_update_time.nanosec = DDS_DURATION_INFINITE_NSEC;

            if (self->_listener.on_deadline_missed)
            {
                self->_listener.on_deadline_missed((struct DDSHST_Reader*)self,
                           self->_listener.listener_data,&key_entry->_key);
            }
            key_entry->has_owner = RTI_FALSE;

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
RTI_PRIVATE void
RHSM_History_delete(struct RHSM_History *self)
{
    struct RHSM_HistoryKeyEntry *key_entry;
    struct RHSM_HistoryRWEntry *rw_entry;
    RTI_INT32 count,i;

    if (self->_key_index != NULL)
    {
        count = REDA_Indexer_get_count(self->_key_index);

        for (i = 0; i < count;)
        {
            key_entry = (struct RHSM_HistoryKeyEntry*)
                            REDA_Indexer_get_entry(self->_key_index,i);

            if (!RHSM_History_delete_key(self, key_entry))
            {
                RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_KEY_OBJECT)
                return;
            }

            count = REDA_Indexer_get_count(self->_key_index);
        }

        if (!REDA_Indexer_delete(self->_key_index))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_KEYINDEXPOOL_OBJECT)
            return;
        }
    }

    if (self->_key_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->_key_pool))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_KEYPOOL_OBJECT)
            return;
        }
    }

    if (self->_rw_index != NULL)
    {
        count = REDA_Indexer_get_count(self->_rw_index);

        for (i = 0; i < count;)
        {
            rw_entry = (struct RHSM_HistoryRWEntry*)
                                    REDA_Indexer_get_entry(self->_rw_index,i);

            if (!RHSM_History_delete_rw(self, rw_entry))
            {
                RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_RW_OBJECT)
                return;
            }

            count = REDA_Indexer_get_count(self->_rw_index);
        }

        if (!REDA_Indexer_delete(self->_rw_index))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_RWINDEX_OBJECT)
            return;
        }
    }

    if (self->_rw_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->_rw_pool))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_RWPOOL_OBJECT)
            return;
        }
    }

    if (self->_sample_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->_sample_pool))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_SAMPLEPOOL_OBJECT)
            return;
        }
    }

    if (self->_sample_info_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->_sample_info_pool))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_SAMPLEINFO_OBJECT)
            return;
        }
    }

    if (self->_sample_ptr_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->_sample_ptr_pool))
        {
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,RHSM_LOG_SAMPLEPTPOOL_OBJECT)
            return;
        }
    }

    if (self->remove_level != NULL)
    {
        OSAPI_Heap_free_array(self->remove_level);
    }

    OSAPI_Heap_free_struct(self);
}
#endif

/*ci
 * \brief Initialize an entry in the key buffer-pool
 *
 * \details
 * Function passed as argument to REDA_BufferPool_new to initialize
 * an entry in the key buffer-pool. Note that this function is called only once.
 *
 * \param[in] initialize_param Opaque parameter passed from the pool
 * \param[in] buffer           The buffer to initialize
 *
 * \return RTI_TRUE if the initialization was success, RTI_FALSE is not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_HistoryKeyEntry_initialize(void *initialize_param, void *buffer)
{
    struct RHSM_History *history = (struct RHSM_History*)initialize_param;
    struct RHSM_HistoryKeyEntry *key_entry = (struct RHSM_HistoryKeyEntry*)buffer;

    if (history->owner_length > 0)
    {
        OSAPI_Heap_allocate_array(&key_entry->owner_array,
                                  (RTI_SIZE_T)history->owner_length,
                                  struct RHSM_HistoryRWEntry*);

        if (key_entry->owner_array == NULL)
        {
            RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_OWNER_OBJECT)
            return RTI_FALSE;
        }
    }
    else
    {
        key_entry->owner_array = NULL;
    }

    RHSM_History_initialize_key(history, key_entry);

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize an entry in the key buffer-pool
 *
 * \details
 * Function passed as argument to REDA_BufferPool_new to finalize
 * an entry in the key buffer-pool. Note that this function is called only
 * once.
 *
 * \param[in] finalize_param   Opaque parameter passed from the pool
 * \param[in] buffer           The buffer to initialize
 *
 * \return RTI_TRUE
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_HistoryKeyEntry_finalize(void *finalize_param, void *buffer)
{
    struct RHSM_HistoryKeyEntry *key_entry = (struct RHSM_HistoryKeyEntry*)buffer;
    UNUSED_ARG(finalize_param);
    UNUSED_ARG(buffer);

    if (key_entry->owner_array != NULL)
    {
        OSAPI_Heap_free_array(key_entry->owner_array);
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize an entry in the remote writer buffer-pool
 *
 * \details
 * Function passed as argument to REDA_BufferPool_new to initialize
 * an entry in the remote writer buffer-pool. Note that this function is
 * called only once.
 *
 * \param[in] param  Opaque parameter passed from the pool
 * \param[in] buffer The buffer to initialize
 *
 * \return RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RHSM_HistoryRWEntry_buffer_init(void *param, void *buffer)
{
    struct RHSM_HistoryRWEntry *rw_entry = (struct RHSM_HistoryRWEntry*)buffer;
    struct RHSM_HistoryRWEntryInit *init = (struct RHSM_HistoryRWEntryInit*)param;

    rw_entry->index = init->current_index / RHSM_RW_PER_ELEMENT;

    rw_entry->bitnum = 1U << (init->current_index % RHSM_RW_PER_ELEMENT);

    ++init->current_index;

    return RTI_TRUE;
}

/*ci
 * \brief Create a new instance of the RHSM history cache
 *
 * \param[in] property The RHSM properties
 * \param[in] listener The reader history listener
 *
 * \return Pointer to new instance of the history cache on success,
 *         NULL on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RHSM_History*
RHSM_History_create(const struct RHSM_HistoryProperty *const property,
                    const struct DDSHST_ReaderListener *const listener)
{
    struct RHSM_History *history = NULL;
    struct RHSM_History *retval = NULL;
    const struct DDS_DataReaderQos *qos;
    DDS_Long read_seq_max;
    struct REDA_BufferPoolProperty pool_property =
                                        REDA_BufferPoolProperty_INITIALIZER;
    struct REDA_IndexerProperty index_prop = REDA_IndexerProperty_INITIALIZER;
    struct RHSM_HistoryRWEntryInit rw_init;

    if (property == NULL)
    {
        RHSM_LOG_NO_PROPERTY(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    qos = property->_parent.qos;

    if (qos == NULL)
    {
        RHSM_LOG_NO_PROPERTY_QOS(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (qos->history.kind != DDS_KEEP_LAST_HISTORY_QOS)
    {
        RHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (qos->destination_order.kind != DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        RHSM_LOG_DESTINATION_BY_SOURCE_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if ((qos->resource_limits.max_instances == DDS_LENGTH_UNLIMITED) ||
        (qos->resource_limits.max_samples == DDS_LENGTH_UNLIMITED) ||
        (qos->resource_limits.max_samples_per_instance == DDS_LENGTH_UNLIMITED))
    {
        RHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }


#ifdef RTI_CERT
    if (qos->resource_limits.max_samples <
                            (qos->resource_limits.max_instances
                             * qos->resource_limits.max_samples_per_instance))
    {
        RHSM_LOG_MAX_SAMPLE_TOO_SMALL(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif

    if ((qos->reader_resource_limits.instance_replacement !=
         DDS_NO_INSTANCE_REPLACEMENT_QOS) &&
        (qos->reader_resource_limits.instance_replacement !=
         DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS))
    {
        RHSM_LOG_INVALID_INSTANCE_REPLACEMENT(OSAPI_LOGKIND_ERROR,
                qos->reader_resource_limits.instance_replacement)
        goto done;
    }

    OSAPI_Heap_allocate_struct(&history, struct RHSM_History);

    if (history == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_HISTORY_OBJECT)
        goto done;
    }

    RT_Component_initialize(&history->_parent._parent,
                           &RHSM_HistoryI_fv_Intf._parent,
                           0,
                           &property->_parent._parent,
                           (listener ? &listener->_parent : NULL));

    history->_qos = qos;
    history->_property = *(struct RHSM_HistoryProperty*)property;

    if (listener)
    {
        history->_listener = *listener;
    }
    else
    {
        history->_listener = RTI_DEFAULT_READERHISTORY_LISTENER;
    }

    RHSM_History_init_ordered_list(&history->_s_ordered);

    /* If there are more remote writers than can be stored in a bitmap
     * allocate an array for the maximum number of remote writers to keep
     * track of (up to  max_remote_writers_per_instance).
     */
    if (history->_qos->reader_resource_limits.max_remote_writers >
                               (RTI_INT32)(RHSM_RW_PER_ELEMENT * MAX_OWNER_BITMAP_LENGTH))
    {
        history->owner_length = history->_qos->reader_resource_limits.max_remote_writers_per_instance;
    }
    else
    {
        history->owner_length = 0;
    }

    OSAPI_Heap_allocate_array(&history->remove_level,
                              RHSM_REMOVE_LEVEL_COUNT,REDA_CircularList_T);

    if (history->remove_level == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_REMOVELIST_OBJECT)
        goto done;
    }

    REDA_CircularList_init(&history->remove_level[RHSM_REMOVE_LEVEL_EQ]);
    REDA_CircularList_init(&history->remove_level[RHSM_REMOVE_LEVEL_LT]);
#if DDS_BLOCKING_READER_ENABLED
    REDA_CircularList_init(&history->suspended_threads);
#endif

    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct RHSM_HistoryRWEntry);
    pool_property.max_buffers =
                (RTI_SIZE_T)history->_qos->reader_resource_limits.max_remote_writers;

    rw_init.current_index = 0;
    rw_init.max_rw = history->_qos->reader_resource_limits.max_remote_writers;

    history->_rw_pool = REDA_BufferPool_new("rw_pool",&pool_property,
                            RHSM_HistoryRWEntry_buffer_init,(void*)&rw_init,
                            NULL,NULL);

    if (history->_rw_pool == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_RWPOOL_OBJECT)
        goto done;
    }

    index_prop.max_entries =
                history->_qos->reader_resource_limits.max_remote_writers;
    history->_rw_index = REDA_Indexer_new(RHSM_History_compare_rw_record,
                                          &index_prop);
    if (history->_rw_index == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_RWINDEX_OBJECT)
        goto done;
    }

    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct RHSM_HistoryKeyEntry);
    pool_property.max_buffers = (RTI_SIZE_T)history->_qos->resource_limits.max_instances;
#ifndef RTI_CERT
    history->_key_pool = REDA_BufferPool_new("key_pool",&pool_property,
                                RHSM_HistoryKeyEntry_initialize,(void*)history,
                                RHSM_HistoryKeyEntry_finalize,(void*)history);
#else
    history->_key_pool = REDA_BufferPool_new("key_pool",&pool_property,
                                RHSM_HistoryKeyEntry_initialize,(void*)history,
                                NULL, NULL);
#endif
    if (history->_key_pool == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_KEYPOOL_OBJECT)
        goto done;
    }

    index_prop.max_entries = history->_qos->resource_limits.max_instances;
    history->_key_index = REDA_Indexer_new(RHSM_History_compare_key_record,
                                           &index_prop);
    if (history->_key_index == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_KEYINDEXPOOL_OBJECT)
        goto done;
    }

    /* Allocate max_samples + 1. With:
     *
     * max_samples = max_instances * max_samples_per_instance
     * it is guaranteed that the sample pool is never exhausted unless all
     * samples are loaned to an application and never returned.
     */
    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct RHSM_HistorySample);
    pool_property.max_buffers = (RTI_SIZE_T)history->_qos->resource_limits.max_samples;
    history->_sample_pool = REDA_BufferPool_new("sample_pool",&pool_property,NULL,NULL,NULL,NULL);
    if (history->_sample_pool == NULL)
    {
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,RHSM_LOG_SAMPLEPOOL_OBJECT)
        goto done;
    }

    REDA_CircularList_init(&history->_deadline_timer);
    REDA_CircularList_init(&history->_instance_timeline);

    /* pool of SampleInfo's, for read/take with loans. Allow
     * 1 extra for meta samples. Only KEEP_LAST is supported and a precondition
     * is that max_instances is finite.
     */
    read_seq_max = (qos->resource_limits.max_instances * (qos->history.depth + 1));

    /* allocate internal sample and sample_info sequences */
    pool_property.max_buffers = (RTI_SIZE_T)qos->reader_resource_limits.max_outstanding_reads;

    /* buffers of loaned sample infos */
    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_SampleInfo*) * (RTI_SIZE_T)read_seq_max;
    history->_sample_info_pool = REDA_BufferPool_new("sample_info_pool",
                                                     &pool_property,
                                                     NULL, NULL, NULL, NULL);
    if (history->_sample_info_pool == NULL)
    {
        RHSM_LOG_SAMPLE_INFO_POOL(OSAPI_LOGKIND_ERROR,
                                  pool_property.buffer_size,
                                  pool_property.max_buffers)
        goto done;
    }

    /* buffers of pointers to loaned sample */
    pool_property.buffer_size = ((RTI_SIZE_T)sizeof(void *) * (RTI_SIZE_T)read_seq_max);
    history->_sample_ptr_pool = REDA_BufferPool_new("sample_ptr_pool",
                                                    &pool_property,
                                                    NULL, NULL, NULL, NULL);
    if (history->_sample_ptr_pool == NULL)
    {
        RHSM_LOG_SAMPLE_POOL(OSAPI_LOGKIND_ERROR,
                             pool_property.buffer_size,
                             pool_property.max_buffers)
        goto done;
    }

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

    history->in_listener = RTI_FALSE;

    retval = history;

done:
#ifndef RTI_CERT
    if ((retval == NULL) && (history != NULL))
    {
        RHSM_History_delete(history);
    }
#endif
    return retval;
}

/*ci
 * \brief Find and return a pointer to a key entry based on the instance handle
 *
 * \details
 * This function is the implementation specific version of
 * \ref RHSM_History_lookup_key.
 *
 * \param[in] self The history cache
 * \param[in] key  The key entry to search for
 *
 * \return A key entry on success, NULL if the entry was not found
 *
 * \sa \ref RHSM_History_lookup_key
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RHSM_HistoryKeyEntry*
RHSM_History_find_key(struct RHSM_History *self,
                      const DDS_InstanceHandle_t *const key)
{
    return (struct RHSM_HistoryKeyEntry*)REDA_Indexer_find_entry(
                                                        self->_key_index,key);
}

/*ci
 * \brief Find and return a pointer to a key entry based on the instance handle
 *
 * \param[in] rh   The history cache
 * \param[in] key  The key entry to search for
 *
 * \return A key entry on success, NULL if the entry was not found
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReaderKeyEntryRef_T
RHSM_History_lookup_key(struct DDSHST_Reader *rh,
                        const DDS_InstanceHandle_t *const key)
{
    struct RHSM_History *self = (struct RHSM_History*)rh;
    return (DDSHST_ReaderKeyEntryRef_T)RHSM_History_find_key(self, key);
}

/*ci
 * \brief Remove a sample from the history cache
 *
 * \details
 *
 * Try to remove a sample from the committed queue. The removal logic is
 * a compromise between performance and memory. A reasonable effort is made
 * to maintain at least depth samples for an instance, but if that is not
 * possible, a sample will be removed from any available instance.
 *
 * \param[in] self The history cache to remove the sample from
 *
 * \return DDS_NOT_REJECTED if a new sample will not be rejected.
 *         DDS_REJECTED_BY_SAMPLES_LIMIT if a new sample will be rejected
 *         due to reaching the max_samples limit.
 *         DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT if a new sample will
 *         be rejected due to reaching the max_samples_per_instance.
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_SampleRejectedStatusKind
RHSM_History_prune_sample(struct RHSM_History *self,DDS_InstanceHandle_t *key)
{
    struct RHSM_HistorySample *rm_sample = NULL;
    struct RHSM_HistoryKeyEntry *key_entry = NULL;
    RTI_INT32 level;
    DDS_SampleRejectedStatusKind retval = DDS_REJECTED_BY_SAMPLES_LIMIT;

    key_entry = RHSM_History_find_key(self,key);

    if ((key_entry != NULL) && !REDA_CircularList_is_empty(&key_entry->_prune))
    {
        rm_sample = RHSM_HistorySample_from_node(
                                REDA_CircularList_get_first(&key_entry->_prune),
                                prune_node);

        /* In the case of read the sample is not removed from the
         * prune-list as this makes it difficult to return it to the
         * cache in the same place without traversing the cache.
         */
        if (rm_sample->_loan_count == 0)
        {
            /* It is possible to remove a sample, a new sample will not be
             * rejected.
             */
            retval = DDS_NOT_REJECTED;
        }
        else
        {
            /* There are no samples to remove for the key, must have
             * reached the max_samples_per_instance limit instead
             * of max_samples limit.
             */
            retval = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
        }
        /* The reason passed to RHSM_History_return_sample is the reason
         * a sample that was already committed is lost.
         *
         * The retval is the reason a sample is rejected if it
         * cannot be commit in the first place. This value is only used
         * by reserve_entry if there is a failure to reserve an entry.
         */
        RHSM_History_return_sample(self,rm_sample,
                                   DDS_SAMPLE_LOST_BY_MAX_SAMPLES_LIMIT);
    }
    else if ((key_entry != NULL) &&
             (key_entry->_sample_count ==
              self->_qos->resource_limits.max_samples_per_instance))
    {
        /* all the samples may be taken and the max_samples_per_instance per
         * instance limit is reached.
         */
        retval = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    }
    else
    {
        /* max_samples_per_instance has not been exceeded, try to take from other
         * instances. This can only happen if
         * max_samples < max_instances * max_samples_per_instance
         */
        if (!REDA_CircularList_is_empty(&self->remove_level[RHSM_REMOVE_LEVEL_EQ]))
        {
            level = RHSM_REMOVE_LEVEL_EQ;
        }
        else if (!REDA_CircularList_is_empty(&self->remove_level[RHSM_REMOVE_LEVEL_LT]))
        {
            level = RHSM_REMOVE_LEVEL_LT;
        }
        else
        {
            return DDS_REJECTED_BY_SAMPLES_LIMIT;
        }

        key_entry = (struct RHSM_HistoryKeyEntry*)
                        REDA_CircularList_get_first(&self->remove_level[level]);

        while (!REDA_CircularList_node_at_head(&self->remove_level[level],key_entry))
        {
            if (!REDA_CircularList_is_empty(&key_entry->_prune))
            {
                rm_sample = RHSM_HistorySample_from_node(
                        REDA_CircularList_get_first(&key_entry->_prune),prune_node);

                /* In the case of read the sample is not removed from the
                 * prune-list as this makes it difficult to return it to the
                 * cache in the same place without traversing the cache.
                 */
                if (rm_sample->_loan_count == 0)
                {
                    retval = DDS_NOT_REJECTED;
                }
                RHSM_History_return_sample(self,rm_sample,
                                           DDS_SAMPLE_LOST_BY_MAX_SAMPLES_LIMIT);
                break;
            }
            key_entry = (struct RHSM_HistoryKeyEntry*)
                                 REDA_CircularListNode_get_next(&key_entry->_node);
        }
    }

    return retval;
}

/*ci
 * \brief Implementation of the DDSHST_Reader_reserve_entry function
 *
 * \details
 * This function tries to reserve an entry in the cache based on the current
 * resource limits. A reserved entry can either be committed to the queue
 * or returned unused (cancelled). If an entry is returned (caller decided
 * to not use it), all operations that took place in the reservation are
 * reversed. That is, any resource allocated are freed.
 *
 * \param[in]  rh                The history cache
 * \param[in]  rw                The remote writer the reservation is for
 * \param[in]  key               The key the reservation is for
 * \param[out] sample_info       A pointer to a sample info structure for this
 *                               sample, memory owned by the queue
 * \param[in]  strength          The current strength for the remote writer
 *                               the reservation for
 * \param[in]  sample_sn         The SN for this entry
 * \param[in]  sample_virtual_sn The virtual SN for this entry
 * \param[out] reject_reason     If the reservation failed, this is the reason
 *
 * \return A reference to a reservation, NULL otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReaderSampleEntryRef_T
RHSM_History_reserve_entry(struct DDSHST_Reader *rh,
                           DDS_InstanceHandle_t *rw,
                           DDS_InstanceHandle_t *key,
                           struct DDS_SampleInfo **sample_info,
                           DDS_Long strength,
                           struct REDA_SequenceNumber *sample_sn,
                           struct REDA_SequenceNumber *sample_virtual_sn,
                           DDS_SampleRejectedStatusKind *reject_reason)
{
    struct RHSM_History *self = (struct RHSM_History*)rh;
    DDS_SampleRejectedStatusKind last_reason = DDS_REJECTED_BY_SAMPLES_LIMIT;
    struct RHSM_HistoryRWEntry *rw_entry = NULL;
#if DDS_BLOCKING_READER_ENABLED
    struct OSAPI_NtpTime suspend_end;
    RTI_BOOL is_suspended = RTI_FALSE;
    struct RHSM_ThreadSuspendState suspend_state;
    RTI_INT32 sec;
    struct OSAPI_NtpTime diff;
    RTI_INT32 sem_reason;
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL brc;
    struct OSAPI_NtpTime max_blocking_time;
    struct OSAPI_NtpTime rem_blocking_time;
    RTI_UINT32 block_time_ms;
#endif

    last_reason = DDS_NOT_REJECTED;

    if (reject_reason)
    {
        *reject_reason = last_reason;
    }

    self->reserved_entry.sample = NULL;
    self->reserved_entry.rw_entry = NULL;
    self->reserved_entry.q_op = RHSM_QUEUE_OPERATION_NONE;

    rw_entry = RHSM_History_find_rw(self, rw);

    if (rw_entry == NULL)
    {
        rw_entry = REDA_BufferPool_get_buffer(self->_rw_pool);
        if (rw_entry == NULL)
        {
            RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_WARNING,RHSM_LOG_RW_OBJECT)
            last_reason = DDS_REJECTED_BY_REMOTE_WRITERS_LIMIT;
            goto failure;
        }
        RHSM_HistoryRWEntry_initialize(rw_entry,rw);
        rw_entry->last_virtual_sn = *sample_virtual_sn;
        self->reserved_entry.q_op |= RHSM_QUEUE_OPERATION_ALLOCATE_WRITER;
    }

    rw_entry->_strength = strength;
    self->reserved_entry.rw_entry = rw_entry;

#if DDS_BLOCKING_READER_ENABLED
    while (!done)
#endif
    {
        self->reserved_entry.sample = REDA_BufferPool_get_buffer(self->_sample_pool);

        if (self->reserved_entry.sample == NULL)
        {
            last_reason = RHSM_History_prune_sample(self,key);
            if (last_reason == DDS_NOT_REJECTED)
            {
                self->reserved_entry.sample = REDA_BufferPool_get_buffer(self->_sample_pool);
            }
        }

#if DDS_BLOCKING_READER_ENABLED
        if (self->reserved_entry.sample != NULL)
        {
            break;
        }

        if ((self->_qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS) ||
            (self->_qos->history.kind == DDS_KEEP_ALL_HISTORY_QOS) ||
            (DDS_Duration_compare(&self->_qos->reliability.max_blocking_time,
                                  &DDS_DURATION_ZERO) == 0))
        {
            /* In the reliable case let the allocation fail and drop
             * the sample. This will cause a retransmission by the
             * sender. The advantage with dropping it is that new
             * samples can be received on the same thread.
             */
            RHSM_LOG_ENTRY_RESERVATION_FAILED(OSAPI_LOGKIND_ERROR,last_reason)
            goto failure;
        }

        /* In the case of best-effort reliability dropping the
         * sample will cause it to be missed. This can
         * happen if all the samples are on loan which can easily
         * happen with depth=max_samples_per_instance = 1.
         * Since the sample is already received suspend the
         * calling thread and try again. The disadvantage with
         * suspending the thread is that _no_ new samples may be
         * received for _any_ topic on the same thread. However, the
         * suspension allows user thread thread to process already
         * received samples, for example threads blocked on a
         * waitset or a polling loop.
         */
        if (!is_suspended)
        {
            REDA_CircularListNode_init(&suspend_state._node);
            suspend_state.wakeup_sem = OSAPI_System_get_thread_semaphore();

            if (suspend_state.wakeup_sem == NULL)
            {
                goto failure;
            }

            if (!OSAPI_System_get_time(&suspend_state.suspend_start))
            {
                goto failure;
            }

            is_suspended = RTI_TRUE;

            OSAPI_NtpTime_from_nanosec(&max_blocking_time,
                           self->_qos->reliability.max_blocking_time.sec,
                           self->_qos->reliability.max_blocking_time.nanosec);

            rem_blocking_time = max_blocking_time;
        }
        else
        {
            if (!OSAPI_System_get_time(&suspend_end))
            {
                OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
                /* The other option would be to set suspend_end such that the
                 * max_blocking time would be exceeded below. However
                 * returning here is simpler.
                 */
                goto failure;
            }

            /* It could happen that the thread is unblocked many times,
             * but never gets a sample. Check if the total accumulated
             * time blocked exceeds max_blocking_time.
             */
            OSAPI_NtpTime_subtract(&diff,
                                   &suspend_end,&suspend_state.suspend_start);

            if (OSAPI_NtpTime_compare(&diff,&max_blocking_time) > 0)
            {
                /* May have exceeded DataReaderQos.resource_limtis.max_samples */
                RHSM_LOG_ENTRY_RESERVATION_FAILED(OSAPI_LOGKIND_ERROR,last_reason)
                goto failure;
            }

            /* Calculate remaining max blocking time, diff is the total elapsed
             * blocking time
             */
            OSAPI_NtpTime_subtract(&rem_blocking_time,
                                   &max_blocking_time,
                                   &diff);
        }

        REDA_CircularList_append(&self->suspended_threads,&suspend_state._node);

        /* Save the current reservation state in case another thread will
         * reserve an entry for the same reader.
         */
        suspend_state.reserved_entry = self->reserved_entry;
        suspend_state.rw_strength = rw_entry->_strength;

        /* Upon entering this code the lock is taken exactly once (from the
         * downstream transport. Thus, a single unlock is sufficient.
         */
        if (DB_Database_unlock(self->_property._parent._parent.db) != DB_RETCODE_OK)
        {
            OSAPI_System_return_thread_semaphore(suspend_state.wakeup_sem);
            goto failure;
        }

        /* We could have a max_blocking_time on the receive end
         * in case we are blocked. That will be symmetric to the
         * sending side.
         *
         * It is not ok to return failure if the take() call
         * fails because the database must be locked to restore the state
         * for the suspended thread before returning.
         *
         * If OSAPI_SEMAPHORE_RESULT_TIMEOUT is returned, then the take() call
         * succeeded and the wait is over, otherwise try again until the
         * max_blocking_time is exceeded. Thus, a failed take() call is benign.
         *
         * The maximum blocking time for a DataReader is 1 year = 31536000L,
         * which will exceed the maximum value for a signed 32-bit integer
         * when converted to ms. Thus, multiple calls to OSAPI_Semaphore_take
         * is be necessary.
         */
        if (rem_blocking_time.sec > RHSM_MAX_BLOCKING_TIME_SEC)
        {
            block_time_ms = RHSM_MAX_BLOCKING_TIME_SEC * 1000U;
        }
        else
        {
            /* This cannot overflow a signed 32-bit integer
             * since rem_blocking_time.frac is < 1s.
             */
            OSAPI_NtpTime_to_millisec(&sec,&block_time_ms,&rem_blocking_time);
            block_time_ms = block_time_ms + ((RTI_UINT32)sec * 1000U);
        }

        brc = OSAPI_Semaphore_take(suspend_state.wakeup_sem,
                                   (RTI_INT32)block_time_ms,
                                   &sem_reason);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(brc);
#endif

        /* If rem_blocking_time.sec was > RHSM_MAX_BLOCKING_TIME_SEC,
         * the semaphore did not really time out and another call to
         * OSAPI_Semaphore_take is necessary.
         */
        if ((sem_reason == OSAPI_SEMAPHORE_RESULT_TIMEOUT) &&
            (rem_blocking_time.sec <= RHSM_MAX_BLOCKING_TIME_SEC))
        {
            OSAPI_System_return_thread_semaphore(suspend_state.wakeup_sem);
            REDA_CircularList_unlink_node(&suspend_state._node);
            done = RTI_TRUE;
        }

        if (DB_Database_lock(self->_property._parent._parent.db) != DB_RETCODE_OK)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)

            /* If the database lock call failed the system is assumed to be
             * in a non-deterministic state as this is unexpected. It is not
             * safe to return because the state has not been restored
             * for the calling thread and thus protection of critical sections
             * have not been restored. Instead of returning, block on the
             * thread semaphore forever. This will cause reception of data to
             * stop and eventually a system failure error should be detected.
             *
             * A do-while loop is used to handle the case where the
             * take() call fails.
             */
            do
            {
                RTI_BOOL semrc;

                semrc = OSAPI_Semaphore_take(suspend_state.wakeup_sem,
                                             OSAPI_SEMAPHORE_TIMEOUT_INFINITE,
                                             NULL);
#if OSAPI_ENABLE_LOG
                OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR);
#endif
                IGNORE_RETVAL(semrc);
            } while (1);
        }

        /* Restore the reservation state */
        /* Save the current reservation state in case another thread will
         * reserve an entry for the same reader.
         */
        self->reserved_entry = suspend_state.reserved_entry;
        rw_entry->_strength = suspend_state.rw_strength;
#endif
    } /* end while */

    /* This handles the case were the Semaphore_take() operation times out */
    if (self->reserved_entry.sample == NULL)
    {
        RHSM_LOG_ENTRY_RESERVATION_FAILED(OSAPI_LOGKIND_ERROR,last_reason)
        goto failure;
    }

    RHSM_History_initialize_sample(self->reserved_entry.sample);

    self->reserved_entry.sample->_info.instance_handle = *key;
    self->reserved_entry.sample->_info.publication_handle = *rw;
    self->reserved_entry.sample->_info.publication_sequence_number =
                                    *(struct DDS_SequenceNumber_t*)sample_sn;
    self->reserved_entry.sample->_info.publication_virtual_sequence_number =
                              *(struct DDS_SequenceNumber_t*)sample_virtual_sn;

    *sample_info = &self->reserved_entry.sample->_info;

    return (DDSHST_ReaderSampleEntryRef_T)&self->reserved_entry;

failure:

    if (reject_reason)
    {
        *reject_reason = last_reason;
    }

    if (self->_listener.on_sample_rejected)
    {
        self->_listener.on_sample_rejected(
                        rh,self->_listener.listener_data,key,last_reason);
    }

    if (self->reserved_entry.q_op & RHSM_QUEUE_OPERATION_ALLOCATE_WRITER)
    {
        REDA_BufferPool_return_buffer(self->_rw_pool,rw_entry);
    }

    return NULL;
}

/*ci
 * \brief Return/Cancel a queue reservation
 *
 * \details
 *
 * Return a previous reservation. Returning a reservation has no effect
 * on the queue, and any resources allocated are returned.
 *
 * \param[in] rh     self
 * \param[in] entry  reservation to cancel
 *
 * \sa \ref RHSM_History_reserve_entry
 */
RTI_PRIVATE void
RHSM_History_return_entry(struct DDSHST_Reader *rh,
                                 DDSHST_ReaderSampleEntryRef_T entry)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistoryReservedEntry *resvd = (struct RHSM_HistoryReservedEntry *)entry;

    if (resvd->q_op & RHSM_QUEUE_OPERATION_ALLOCATE_WRITER)
    {
        REDA_BufferPool_return_buffer(self->_rw_pool,resvd->rw_entry);
    }

    REDA_BufferPool_return_buffer(self->_sample_pool,resvd->sample);
}

/*ci
 * \brief Check if a remote writer should be the owner of an instance
 *
 * \param[in] self      The history cache
 * \param[in] key_entry The key to check ownership on
 * \param[in] rw_entry  The remote writer to check ownership for
 *
 * \return DDS_BOOLEAN_TRUE if the remote writer should be the owner,
 *         DDS_BOOLEAN_FALSE if not
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
RHSM_History_writer_is_owner(struct RHSM_History *self,
                             struct RHSM_HistoryKeyEntry *key_entry,
                             struct RHSM_HistoryRWEntry *rw_entry)
{
    if (self->_property._parent.qos->ownership.kind == DDS_SHARED_OWNERSHIP_QOS)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (key_entry->_current_owner == rw_entry)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (!key_entry->has_owner ||
        (key_entry->_current_owner->_strength < rw_entry->_strength))
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (key_entry->_current_owner->_strength > rw_entry->_strength)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Two writers cannot have the same key, thus this test must either
     * be less than or greater than
     */
    if (DDS_InstanceHandle_compare(&rw_entry->_key,
                                   &key_entry->_current_owner->_key) < 0)
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Unregister a key from the cache
 *
 * \details
 * Unregistering a key means a remote writer is no longer updating the
 * key, but it is not deleted. When a remote writer unregisters itself
 * the ownership of the instance may change.
 *
 * \param[in] rh        The history cache
 * \param[in] key_entry The key to check ownership on
 * \param[in] rw_entry  The remote writer to check ownership for
 */
RTI_PRIVATE void
RHSM_History_unregister_key(struct DDSHST_Reader *rh,
                            struct RHSM_HistoryRWEntry *rw_entry,
                            struct RHSM_HistoryKeyEntry *key_entry)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;

    RHSM_History_remove_remote_writer(self,key_entry,rw_entry,RTI_FALSE,NULL);
}

/*ci
 * \brief Assert a key in the history cache
 *
 * \details
 * Find or add key to the history cache if possible. If the key already
 * exists a pointer to the existing entry is returned. If the key does
 * not exist the cache is checked for space, and if there is space
 * a new entry is added and a pointer returned. If the entry does not exist
 * and cannot be added NULL is returned and the reason for the failure
 * is set in the reason out parameter.
 *
 * \param[in]  self     The history cache
 * \param[in]  rw_entry The remote writer updating the key
 * \param[in]  key      The instance/key handle to assert
 * \param[out] reason   The reason for failing to assert the key to the cache
 *
 * \return Pointer to key entry on success, NULL on failure
 */
RTI_PRIVATE struct RHSM_HistoryKeyEntry*
RHSM_History_assert_key(struct RHSM_History *self,
                        struct RHSM_HistoryRWEntry *rw_entry,
                        DDS_InstanceHandle_t *key,
                        DDS_SampleRejectedStatusKind *reason)
{
    struct RHSM_HistoryKeyEntry *key_entry = NULL;
    struct REDA_CircularListNode *node;
    struct RHSM_HistoryKeyEntry *r_key_entry = NULL;

    *reason = DDS_NOT_REJECTED;

    key_entry = RHSM_History_find_key(self,key);

    if (key_entry != NULL)
    {
        return key_entry;
    }

    key_entry = (struct RHSM_HistoryKeyEntry*)
                            REDA_BufferPool_get_buffer(self->_key_pool);

    if ((key_entry == NULL) &&
        (self->_qos->reader_resource_limits.instance_replacement ==
                                    DDS_NO_INSTANCE_REPLACEMENT_QOS))
    {
        *reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
        RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_WARNING,RHSM_LOG_KEY_OBJECT)
        goto done;
    }

    if (key_entry == NULL)
    {
        node = REDA_CircularList_get_first(&self->_instance_timeline);

        /* The node _cannot_ be the head, that would mean the
         * instance resource-limit was 0.
         */
        r_key_entry = RHSM_HistoryKeyEntry_from_node(node,_timeline_entry);

        if (self->_listener.on_instance_replaced)
        {
            self->_listener.on_instance_replaced(
                    &self->_parent,self->_listener.listener_data,
                    &r_key_entry->_key,key,&rw_entry->_key,
                    r_key_entry->_sample_count);
        }

        if (!RHSM_History_delete_key(self,r_key_entry))
        {
            *reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
            RHSM_LOG_OBJECT_DELETE(OSAPI_LOGKIND_WARNING,RHSM_LOG_KEY_OBJECT)
            goto done;
        }

        key_entry = (struct RHSM_HistoryKeyEntry*)
                                REDA_BufferPool_get_buffer(self->_key_pool);

        if (key_entry == NULL)
        {
            RHSM_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_WARNING,RHSM_LOG_KEY_OBJECT)
            *reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
            goto done;
        }
    }

    RHSM_History_initialize_key(self, key_entry);

    key_entry->_key = *key;
    key_entry->_view_state = DDS_NEW_VIEW_STATE;
    key_entry->is_disposed = RTI_FALSE;

    if (!REDA_Indexer_add_entry(self->_key_index,key_entry))
    {
        RHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_WARNING,RHSM_LOG_KEY_OBJECT)
        *reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
        REDA_BufferPool_return_buffer(self->_key_pool,key_entry);
        key_entry = NULL;
    }

done:

    if (key_entry == NULL)
    {
        if (self->_listener.on_sample_rejected)
        {
            self->_listener.on_sample_rejected((struct DDSHST_Reader*)self,
                                    self->_listener.listener_data,key,*reason);
        }
    }

    return key_entry;
}

/*ci
 * \brief Commit all cached samples up to a specific SN for a remote writer
 *
 * \details
 * The history cache stores samples received out of order to improve
 * performance for reliable communication. If samples are received out of
 * order there may be a backlog of samples until a sample is received that
 * completes a sequence of samples. This function adds all these samples
 * to the ordered list of samples.
 *
 * \param[in] rh               The history cache
 * \param[in] rw_entry         The remote writer to commit samples for
 * \param[in] sn               The first non commitable sequence number
 * \param[in] strength         The strength of the writer
 * \param[out] last_virtual_sn The virtual SN of the last committed sample
 * \param[out] missing_vsn     The number of missing vsn in a range
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *       \ref DDSHST_ReturnCode_T on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
RHSM_History_commit_rw(struct DDSHST_Reader *rh,
                       struct RHSM_HistoryRWEntry *rw_entry,
                       struct REDA_SequenceNumber *sn,
                       DDS_Long strength,
                       struct REDA_SequenceNumber *last_virtual_sn,
                       DDS_Long *missing_vsn)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistorySample *s1_entry, *s2_entry;
    struct REDA_CircularListNode *relink_entry;
    RTI_BOOL new_samples = RTI_FALSE;
    DDS_Boolean is_owner;
    DDS_Boolean got_timestamp = DDS_BOOLEAN_FALSE;
    struct OSAPI_NtpTime rcv_timestamp;
    struct RHSM_HistoryKeyEntry *key_entry = NULL;
    DDS_SampleRejectedStatusKind reason;
    DDS_Boolean valid_data = DDS_BOOLEAN_FALSE;
    struct DDS_Duration_t tick_time;
    struct REDA_SequenceNumber delta_sn = REDA_SEQUENCE_NUMBER_ZERO;
    DDS_Long committed_samples = 0;
    DDSHST_ReturnCode_T retcode = DDSHST_RETCODE_ERROR;
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL commit_by_vsn = RTI_FALSE;

    /* If the input SN is <> 0 then it is the last committable SN on input.
     * It is always the last committed vsn on output.
     */
    rw_entry->_strength = strength;
    *missing_vsn = 0;

    /* Save the SN for use in case samples have to be recommitted */
    if (REDA_SequenceNumber_compare(sn,&rw_entry->highest_noncommmitable_sn) > 0)
    {
        rw_entry->highest_noncommmitable_sn = *sn;
    }

    if (!REDA_SequenceNumber_is_zero(last_virtual_sn))
    {
        if (REDA_CircularList_is_empty(&rw_entry->_cached))
        {
            if (REDA_SequenceNumber_compare(last_virtual_sn,
                                            &rw_entry->last_virtual_sn) > 0)
            {
                rw_entry->last_virtual_sn = *last_virtual_sn;
            }
            return DDSHST_RETCODE_SUCCESS;
        }

        commit_by_vsn = RTI_TRUE;
    }

    while (!done)
    {
        s1_entry = (struct RHSM_HistorySample*)
                            REDA_CircularList_get_first(&rw_entry->_cached);

        while (!REDA_CircularList_node_at_head(&rw_entry->_cached,&s1_entry->_node))
        {
            s2_entry = (struct RHSM_HistorySample*)
                              REDA_CircularListNode_get_next(&s1_entry->_node);

            if (commit_by_vsn &&
                REDA_SequenceNumber_compare(
                        (struct REDA_SequenceNumber *)
                        &s1_entry->_info.publication_virtual_sequence_number,
                        last_virtual_sn) >= 0)
            {
                /* Committed all VSNs */
                break;
            }
            else if (REDA_SequenceNumber_compare(
                        (struct REDA_SequenceNumber *)
                        &s1_entry->_info.publication_sequence_number,
                        sn) >= 0)
            {
                /* Committed all SNs */
                break;
            }

            /* If there are any holes a gap larger than one for vsn. The
             * number of "holes" in the sequence number is reported back
             * down-stream since these samples cannot be accounted for.
             */
            REDA_SequenceNumber_subtract(&delta_sn,
                     (struct REDA_SequenceNumber*)
                         &s1_entry->_info.publication_virtual_sequence_number,
                         &rw_entry->last_virtual_sn);

            if (delta_sn.low > 1U)
            {
                *missing_vsn += (RTI_INT32)(delta_sn.low - 1U);
            }

            OSAPI_Memory_copy(&rw_entry->last_virtual_sn,
                              &s1_entry->_info.publication_virtual_sequence_number,
                              sizeof(struct REDA_SequenceNumber));

            /* The entry is not committed to an instance yet */
            s1_entry->_key_entry = NULL;
            valid_data = s1_entry->_sample->_info->valid_data;

            key_entry = RHSM_History_assert_key(self,rw_entry,
                                                &s1_entry->_info.instance_handle,
                                                &reason);

            if ((key_entry == NULL) && (reason != DDS_REJECTED_BY_INSTANCE_LIMIT))
            {
                ++committed_samples;
                goto done;
            }
            else if (key_entry == NULL)
            {
                /* Rejected due to instance limit */
                RHSM_History_return_sample(self,s1_entry,DDS_SAMPLE_LOST_NOT_LOST);
                --rw_entry->_size;
                s1_entry = s2_entry;
                ++committed_samples;
                continue;
            }

            is_owner = RHSM_History_writer_is_owner(self,key_entry,rw_entry);

            /* Any writer can unregister itself from an instance */
            if (s1_entry->_sample->status_info & RTPS_UNREGISTER_STATUS_INFO)
            {
                RHSM_History_unregister_key(rh,rw_entry,key_entry);
            }
            else
            {
                if (!RHSM_History_add_remote_writer(self,key_entry,rw_entry,is_owner))
                {
                    /* The writer could not be added as an updater of the instance.
                     * This implies the reason is max_remote_writers_per_instance
                     * has been reached.
                     */
                    is_owner = RTI_FALSE;
                }

                if (is_owner)
                {
                    key_entry->_current_owner = rw_entry;
                    key_entry->has_owner = RTI_TRUE;
                }
            }

            if (!is_owner)
            {
                /* if not the owner, nothing else to do and return the sample */
                RHSM_History_return_sample(self,s1_entry,DDS_SAMPLE_LOST_NOT_LOST);
                --rw_entry->_size;
                s1_entry = s2_entry;
                ++committed_samples;
                continue;
            }

            /* If UNREGISTER or DISPOSED is received:
             * - If the key is disposed then do return the sample and reclaim
             *   the key if possible
             * - If the key is not disposed, but has no writers, reclaim the
             *   key if possible
             */
            if (!valid_data && (key_entry->is_disposed ||
                    ((s1_entry->_sample->status_info == RTPS_UNREGISTER_STATUS_INFO) &&
                            ((key_entry->_writer_count > 0) ||
                                    (key_entry->_instance_state == DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)))))
            {
                /* The instance state is already NOT_ALIVE, so ignore the sample
                 */
                RHSM_History_return_sample(self,s1_entry,DDS_SAMPLE_LOST_NOT_LOST);

                /* We reclaim resources only if there are no live writers and
                 * there are no samples in the key. This is very strict. The
                 * instance replacement policy can be used to prevent running out
                 * of resources.
                 */
                if ((key_entry->_not_seen_count == 0) &&
                        (key_entry->_writer_count == 0))
                {
                    RHSM_History_delete_key(self,key_entry);
                }
                --rw_entry->_size;
                s1_entry = s2_entry;
                ++committed_samples;
                continue;
            }

            s1_entry->_key_entry = key_entry;
            s1_entry->sample_state = DDS_NOT_READ_SAMPLE_STATE;
#if DDS_INCLUDE_SAMPLE_INFO_RANKS
            s1_entry->_sample->_info->disposed_generation_count = 0;
            s1_entry->_sample->_info->no_writers_generation_count = 0;
            s1_entry->_sample->_info->sample_rank = 0;
            s1_entry->_sample->_info->generation_rank = 0;
            s1_entry->_sample->_info->absolute_generation_rank = 0;
#endif

            s1_entry->_sample->_ref = (DDSHST_ReaderSampleEntryRef_T)s1_entry;


            /* If the instance was both unregistered _and_ disposed, set the
             * state here. The cache is updated after this test, so although
             * logically a dispose should come before an unregister this is still
             * ok
             */
            if (s1_entry->_sample->status_info & RTPS_DISPOSE_STATUS_INFO)
            {
                s1_entry->_key_entry->_instance_state = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
                s1_entry->_key_entry->is_disposed = RTI_TRUE;
                REDA_CircularList_unlink_node(&key_entry->_deadline_entry);

                /* Reset the last update time so the first new sample does not
                 * cause a deadline missed if a new sample for the disposed key
                 * is received before the key is deleted.
                 */
                key_entry->last_update_time.nanosec = DDS_DURATION_INFINITE_NSEC;
            }

            if ((s1_entry->_key_entry->_writer_count == 0) &&
                    !s1_entry->_key_entry->is_disposed)
            {
                /* This test is not reached unless the writer that performed the
                 * unregister or dispose and unregister was the owner. If the
                 * key has been disposed of then keep the disposed state, otherwise
                 * go to NOT_ALIVE_NO_WRITERS
                 */
                s1_entry->_key_entry->_instance_state = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
            }
            else if (valid_data)
            {
                if (s1_entry->_key_entry->_instance_state != DDS_ALIVE_INSTANCE_STATE)
                {
                    s1_entry->_key_entry->_view_state = DDS_NEW_VIEW_STATE;

                    /* If the instance transitions back to alive _and_ the
                     * meta-sample still exists in the queue (it has not
                     * been read/taken), remove it. Instance state transitions
                     * are not maintained, it can only be in one absolute state.
                     */
                    if (REDA_CircularListNode_is_linked(&s1_entry->_key_entry->meta_sample._node))
                    {
                        RHSM_History_return_sample(self,
                                           &s1_entry->_key_entry->meta_sample,
                                           DDS_SAMPLE_LOST_NOT_LOST);
                    }
                }

                s1_entry->_key_entry->_instance_state = DDS_ALIVE_INSTANCE_STATE;
            }

            /* Reception timestamp must be updated no earlier than when the
             * sample is committed
             */
            if (!got_timestamp)
            {
                if (!OSAPI_System_get_time(&rcv_timestamp))
                {
                    /* Maximum time */
                    rcv_timestamp.sec = 0;
                    rcv_timestamp.frac = 0;
                    RHSM_LOG_GET_RECEPTION_TIMESTAMP(OSAPI_LOGKIND_ERROR)
                }
                got_timestamp = DDS_BOOLEAN_TRUE;
            }

            /* Save the position in the list in case the sample is rejected */
            relink_entry = REDA_CircularListNode_get_prev(&s1_entry->_node);

            REDA_CircularList_unlink_node(&s1_entry->_node);
            --rw_entry->_size;

            OSAPI_NtpTime_to_nanosec(&s1_entry->_info.reception_timestamp.sec,
                                     &s1_entry->_info.reception_timestamp.nanosec,
                                     &rcv_timestamp);

            if (!RHSM_History_commit_sample(self,s1_entry,&new_samples))
            {
                /* If the sample cannot be committed, it means the queue if full.
                 * Since the sample has been received already, leave it on the
                 * writer queue and try again later. In the case of reliable
                 * communication the end result may be that RTPS starts rejecting
                 * samples and they will be resent. In the case of best-effort it
                 * means that  the sample is held on-to as long as possible.
                 * However, at some point the sample is discarded.
                 *
                 * With KEEP_LAST is used the primary reason for this to happen
                 * is that an application is loaning samples for longer than
                 * there is resources to receive new data.
                 *
                 * With KEEP_LAST it means that samples are either not "taken()"
                 * out of the cache fast enough.
                 */

                /* Remember that it is necessary there are samples in the cache
                 * that needs to be recommitted.
                 */
                if (!(s1_entry->_flags & RHSM_HISTORY_FLAG_RECOMMITTED))
                {
                    ++committed_samples;
                }
                self->recommit_samples = RTI_TRUE;
                s1_entry->_flags |= RHSM_HISTORY_FLAG_RECOMMITTED;

                /* Add s1_entry back to the linked list, in the same position. The
                 * samples are kept in increasing order of the sequence number.
                 * Note that samples may be committed out of order when the
                 * instance ordering is used. For topic order it is necessary
                 * to back out as soon as a sample cannot be added.
                 */
                REDA_CircularList_link_node_after(relink_entry,&s1_entry->_node);
                ++rw_entry->_size;
#if 0
                /* With instance_ordering it is possible that there
                 * are other instances which can be committed. However,
                 * although Micro has never officially supported topic_order
                 * it has always been topic_order on the reader. In order to
                 * not change behavior, back out as soon as a sample cannot
                 * be committed.
                 */
                s1_entry = s2_entry;
                continue;
#endif
                /* Stop trying to commit more samples, exit the while loop
                 * !REDA_CircularList_node_at_head()
                 */
                break;
            }

            if (!(s1_entry->_flags & RHSM_HISTORY_FLAG_RECOMMITTED))
            {
                ++committed_samples;
            }

            if (valid_data)
            {
                s1_entry->_key_entry->is_disposed = RTI_FALSE;

                if (self->deadline_enabled)
                {
                    /* Check if the deadline was missed for this sample based on the
                     * last time the sample was updated.
                     */
                    if (OSAPI_System_get_ticktime(&tick_time.sec,&tick_time.nanosec))
                    {
                        if (key_entry->last_update_time.nanosec != DDS_DURATION_INFINITE_NSEC)
                        {
                            if (DDS_Duration_delta_gt(&self->_qos->deadline.period,
                                                      &tick_time,&key_entry->last_update_time))
                            {
                                if (self->_listener.on_deadline_missed)
                                {
                                    self->_listener.on_deadline_missed(
                                            (struct DDSHST_Reader*)self,
                                            self->_listener.listener_data,&key_entry->_key);
                                }
                                key_entry->has_owner = RTI_FALSE;
                            }
                        }

                        key_entry->last_update_time = tick_time;
                        key_entry->last_period_time = tick_time;

                        REDA_CircularList_unlink_node(&key_entry->_deadline_entry);
                        REDA_CircularList_append(&self->_deadline_timer,
                                                 &key_entry->_deadline_entry);

                    }
                }
            }

            s1_entry = s2_entry;
        } /* !REDA_CircularList_node_at_head() */

        self->samples_returned = RTI_FALSE;

        if (new_samples)
        {
            /* Call the upstream listener that data is available. If a listener
             * reads data from the cache it makes it available by returning loans
             * then try again and re-commit any outstanding samples. If that is
             * not possible then the function is done.
             */
            RHSM_History_update_committed(self);
        }

        /* If some samples couldn't be committed due to resource-limits
         * and some samples were returned in the listener (called above)
         * then try again.
         */
        if (!(self->recommit_samples && self->samples_returned))
        {
            done = RTI_TRUE;
        }
    }

    retcode = DDSHST_RETCODE_SUCCESS;

done:

    /* When samples are committed to history the are also reclaimable
     * (KEEP_LAST). In addition when samples are rejected they are also
     * reclaimable.
     *
     */
    if ((committed_samples > 0) &&
        (self->_listener.on_sample_committed) &&
        (rw_entry->_notify_on_commit))
    {
        self->_listener.on_sample_committed(rh,self->_listener.listener_data,
                                            &rw_entry->_key,committed_samples);
    }

    *last_virtual_sn = rw_entry->last_virtual_sn;

    return retcode;
}

/*ci
 * \brief Commit all cached samples up to a specific SN for a remote writer
 *
 * \details
 * The history cache stores samples received out of order to improve
 * performance for reliable communication. If samples are received out of
 * order there may be a backlog of samples until a sample is received that
 * completes a sequence of samples. This function adds all these samples
 * to the ordered list of samples.
 *
 * \param[in] rh               The history cache
 * \param[in] rw               The remote writer to commit samples for
 * \param[in] sn               The first non commitable sequence number
 * \param[in] strength         The strength of the writer
 * \param[in] notify_on_commit Whether to callback upon committing samples
 * \param[out] last_virtual_sn The virtual SN of the last committed sample
 * \param[out] missing_vsn_    The number of missing vsn in a range
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref RHSM_History_commit_rw
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
RHSM_History_commit(struct DDSHST_Reader *rh,
                    DDS_InstanceHandle_t *rw,
                    struct REDA_SequenceNumber *sn,
                    DDS_Long strength,
                    DDS_Boolean notify_on_commit,
                    struct REDA_SequenceNumber *last_virtual_sn,
                    DDS_Long *missing_vsn)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistoryRWEntry *rw_entry = NULL;

    /* Commit all samples [oldest,sn> for the specific writer
     * Committing samples means moving samples into the sample committed list,
     * and ordered according to the presentation and destination order qos
     */
    rw_entry = RHSM_History_find_rw(self, rw);

    if (rw_entry == NULL)
    {
        return DDSHST_RETCODE_SUCCESS;
    }

    rw_entry->_notify_on_commit = notify_on_commit;

    return RHSM_History_commit_rw(rh,rw_entry,sn,strength,
                                  last_virtual_sn,missing_vsn);
}

/*ci
 * \brief Add a reserved sample to the queue and commit one or more
 *
 * \details
 * When a reservation is made the entry does not exist in the cache, only
 * a sample to fill in exists. If the holder of the reservation decides to
 * keep the sample it must be added. When a new sample is added it may also
 * result in 1 or more samples being committed (for example if the newly
 * added sample complete a sequence of samples).
 *
 * \param[in] rh               The history cache
 * \param[in] entry            The reserved entry to commit
 * \param[in] sample           The sample associated with the reservation
 * \param[in] sn               The first non-commitable sequence number
 * \param[in] notify_on_commit Whether to callback upon committing samples
 * \param[out] last_virtual_sn Last committed sample virtual SN
 * \param[out] missing_vsn_    The number of missing vsn in a range
 *
 * \sa \ref RHSM_History_reserve_entry
 */
MUST_CHECK_RETURN RTI_PRIVATE DDSHST_ReturnCode_T
RHSM_History_add_and_commit(struct DDSHST_Reader *rh,
                            DDSHST_ReaderSampleEntryRef_T entry,
                            DDSHST_ReaderSample_T *sample,
                            struct REDA_SequenceNumber *sn,
                            DDS_Boolean notify_on_commit,
                            struct REDA_SequenceNumber *last_virtual_sn,
                            DDS_Long *missing_vsn)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistoryReservedEntry *resvd =
                                    (struct RHSM_HistoryReservedEntry *)entry;
    struct RHSM_HistorySample *s1_entry;
    struct RHSM_HistorySample *new_entry = NULL;
    struct RHSM_HistoryRWEntry *rw_entry = NULL;

    rw_entry = resvd->rw_entry;
    if (resvd->q_op & RHSM_QUEUE_OPERATION_ALLOCATE_WRITER)
    {
        if (!REDA_Indexer_add_entry(self->_rw_index,resvd->rw_entry))
        {
            /* May have exceeded
             * DataReaderQos.resource_limits.max_remote_writers
             */
            RHSM_LOG_OBJECT_INDEX(OSAPI_LOGKIND_ERROR,RHSM_LOG_RW_OBJECT)
            return DDSHST_RETCODE_ERROR;
        }
    }

    ++rw_entry->_size;

    rw_entry->_notify_on_commit = notify_on_commit;

    new_entry = resvd->sample;

    new_entry->_sample = sample;

    if (REDA_CircularList_is_empty(&rw_entry->_cached))
    {
        REDA_CircularList_append(&rw_entry->_cached,&new_entry->_node);
    }
    else
    {
        s1_entry = (struct RHSM_HistorySample*)
                               REDA_CircularList_get_last(&rw_entry->_cached);
        if (DDS_SequenceNumber_compare(
                        &s1_entry->_info.publication_sequence_number,
                        &new_entry->_info.publication_sequence_number) > 0)
        {
            s1_entry = (struct RHSM_HistorySample*)
                               REDA_CircularList_get_first(&rw_entry->_cached);
            while (!REDA_CircularList_node_at_head(&rw_entry->_cached,
                                                   &s1_entry->_node))
            {
                if (DDS_SequenceNumber_compare(
                        &s1_entry->_info.publication_sequence_number,
                        &new_entry->_info.publication_sequence_number) > 0)
                {
                    break;
                }

                s1_entry = (struct RHSM_HistorySample*)
                              REDA_CircularListNode_get_next(&s1_entry->_node);
            }
            REDA_CircularList_append(&s1_entry->_node,&new_entry->_node);
        }
        else
        {
            REDA_CircularList_link_node_after(REDA_CircularList_get_last(
                                       &rw_entry->_cached),&new_entry->_node);
        }
    }

    return RHSM_History_commit_rw(rh,rw_entry,sn,
                                  rw_entry->_strength,
                                  last_virtual_sn,missing_vsn);
}

/*ci
 * \brief Read or take 1 or more samples matching a particular state from the
 *        history cache
 *
 * \details
 * Return 0 or more samples and sample info. The sample_ptr_array and
 * info_array must be pre-allocated to hold at least max_samples
 * pointers. Note that the history _always_ operates on pointer to samples,
 * it has no knowledge about types. Thus, this function _loans_ samples
 * to the caller and the caller _must_ return the loan by calling
 * \ref RHSM_History_finish_read_or_take with the exact same
 *     sequences return by this function.
 *
 * \param[in]    rh               The history cache to read from
 * \param[inout] sample_ptr_array A sequence of pointers to samples
 * \param[inout] info_array       A sequence of pointers to sample info
 * \param[in]    sample_count     The actual number of samples returned
 * \param[in]    handle           The instance to return (NIL is any instance)
 * \param[in]    max_samples      The max number of samples to return (including
 *                                the meta sample)
 * \param[in]    sample_states    The requested sample state
 * \param[in]    view_states      The requested view state
 * \param[in]    instance_states  The requested instance state
 * \param[in]    take             Whether this removes (RTI_TRUE) or leaves
 *                                samples in the cache (RTI_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref RHSM_History_finish_read_or_take
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
RHSM_History_read_or_take(struct DDSHST_Reader *rh,
                          void ***sample_ptr_array,
                          struct DDS_SampleInfo ***info_array,
                          DDS_Long *sample_count,
                          const DDS_InstanceHandle_t *handle,
                          DDS_Long max_samples,
                          DDS_SampleStateMask sample_states,
                          DDS_ViewStateMask view_states,
                          DDS_InstanceStateMask instance_states,
                          DDS_Boolean take)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistorySample *entry = NULL;
    struct RHSM_HistorySample *last_entry = NULL;
    struct RHSM_HistoryKeyEntry *instance_entry = NULL;
    RTI_INT32 i;
    struct RHSM_HistorySample *next_entry = NULL;

    if ((rh == NULL) || (sample_ptr_array == NULL) || (info_array == NULL) || 
        (sample_count == NULL) || ((handle != NULL) &&
        (handle != &DDS_HANDLE_NIL) && !handle->is_valid))
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    /* get/check instance */
    if (handle != NULL)
    {
        instance_entry = RHSM_History_find_key(self, handle);
        if (instance_entry == NULL)
        {
            RHSM_LOG_OBJECT_INVALID(OSAPI_LOGKIND_ERROR,
                                    RHSM_LOG_INSTANCEHANDLE_OBJECT)
            return DDS_RETCODE_BAD_PARAMETER;
        }
    }

    *sample_count = 0;

    if (instance_entry != NULL)
    {
        /* read/take for particular instance, from instance's sample list */
        if (REDA_CircularList_is_empty(&instance_entry->_samples))
        {
            return DDS_RETCODE_NO_DATA;
        }

        entry = (struct RHSM_HistorySample*)instance_entry->first_history_sample;
        last_entry = (struct RHSM_HistorySample*)&instance_entry->_samples;
    }
    else
    {
        /* not particular instance, so read/take from general sample list */
        if (RHSM_History_ordered_list_is_empty(&self->_s_ordered))
        {
            return DDS_RETCODE_NO_DATA;
        }

        entry = (struct RHSM_HistorySample*)self->_s_ordered._ordered_next;
        last_entry = (struct RHSM_HistorySample*)&self->_s_ordered;
    }

    *sample_ptr_array = (void **)
                            REDA_BufferPool_get_buffer(self->_sample_ptr_pool);
    if (*sample_ptr_array == NULL)
    {

        RHSM_LOG_SAMPLE_PTR_ARRAY(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    *info_array = (struct DDS_SampleInfo **)
                           REDA_BufferPool_get_buffer(self->_sample_info_pool);
    if (*info_array == NULL)
    {
        REDA_BufferPool_return_buffer(self->_sample_ptr_pool, *sample_ptr_array);
        RHSM_LOG_INFO_ARRAY(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    /* return sequence of samples, up to maximum. The meta-sample does not
     * count against max_samples, hence +1.
     */
    while ((entry != last_entry) && (*sample_count < (max_samples)))
    {
        if (instance_entry != NULL)
        {
            next_entry = (struct RHSM_HistorySample*)
                REDA_CircularListNode_get_next(&entry->_node);
        }
        else
        {
            next_entry = entry->_ordered_next;
        }

        /* for untaken samples, check sample/instance/view states */
        if (!(entry->_flags & RHSM_HISTORY_FLAG_SAMPLE_TAKEN) &&
             (entry->sample_state & sample_states) &&
             (entry->_key_entry->_instance_state & instance_states) &&
             (entry->_key_entry->_view_state & view_states))
        {
            if (entry->_sample != NULL)
            {   
                (*sample_ptr_array)[*sample_count] = entry->_sample->_user_data;
            }
            else 
            {
                (*sample_ptr_array)[*sample_count] = NULL;
            }

            /* update entry as in-use, possibly loaned and/or taken */
            if (take)
            {
                entry->_flags = entry->_flags | RHSM_HISTORY_FLAG_SAMPLE_TAKEN;
            }

            ++entry->_loan_count;
            /* update entry's sample info, then set sample info ref */
            entry->_info.instance_state = (DDS_InstanceStateKind)entry->_key_entry->_instance_state;
            entry->_info.view_state = (DDS_ViewStateKind)entry->_key_entry->_view_state;
            entry->_info.sample_state = (DDS_SampleStateKind)entry->sample_state;
            entry->_info.reserved_data = (void *)entry;

            /* copy entry's sampleInfo, as view/instance states will be updated */
            (*info_array)[*sample_count] = &entry->_info;
            ++(*sample_count);

            /* update sample state upon read/take */
            entry->sample_state = DDS_READ_SAMPLE_STATE;

            if (take)
            {
                RHSM_History_return_sample(self,entry,DDS_SAMPLE_LOST_NOT_LOST);
            }
        }

        entry = next_entry;
    }

    /* update view state, only after samples read/taken */
    for (i = 0; i < *sample_count; ++i)
    {
        entry = (struct RHSM_HistorySample *)((*info_array)[i]->reserved_data);
        entry->_key_entry->_view_state = DDS_NOT_NEW_VIEW_STATE;
    }

    if (*sample_count == 0)
    {
        /* no data --> return internal buffers */
        REDA_BufferPool_return_buffer(self->_sample_ptr_pool, *sample_ptr_array);
        REDA_BufferPool_return_buffer(self->_sample_info_pool, *info_array);
        *sample_ptr_array = NULL;
        *info_array = NULL;
    }

    return ((*sample_count == 0) ? DDS_RETCODE_NO_DATA : DDS_RETCODE_OK);
}

/*i
 * \brief This function is called when resources are freed up to try and commit
 *        more samples.
 * 
 * \param[in] rh Reader history cache
 */
RTI_PRIVATE void
RHSM_History_on_recommit_event(struct DDSHST_Reader *rh)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    REDA_IndexIterator_T *iterator = NULL;
    struct RHSM_HistoryRWEntry *rw_entry = NULL;
    RTI_INT32 rw_size = 0;
    RTI_INT32 committed = 0;
    DDS_Long mvsn = 0;
    struct REDA_SequenceNumber vsn = REDA_SEQUENCE_NUMBER_ZERO;
    DDSHST_ReturnCode_T hstrc;

    iterator = REDA_Indexer_iterator_begin(self->_rw_index);

    rw_entry = (struct RHSM_HistoryRWEntry*)REDA_Indexer_iterator_next(iterator);

    /* Reset the re-commit flag. If any of the datawriters have uncommitted
     * samples it will be set again.
     */
    self->recommit_samples = RTI_FALSE;
    while (rw_entry != NULL)
    {
        rw_size = rw_entry->_size;

        hstrc = RHSM_History_commit_rw(rh,rw_entry,
                                       &rw_entry->highest_noncommmitable_sn,
                                       rw_entry->_strength,
                                       &vsn,&mvsn);

        if (hstrc == DDSHST_RETCODE_SUCCESS)
        {
            committed = rw_size - rw_entry->_size;

            if (committed > 0)
            {
                if ((self->_listener.on_sample_committed) &&
                        (rw_entry->_notify_on_commit))
                {
                    self->_listener.on_sample_committed(rh,
                                self->_listener.listener_data,
                                &rw_entry->_key,
                                committed);
                }
            }
        }
#if OSAPI_ENABLE_LOG
        else
        {
            RHSM_LOG_RECOMMIT_FAILURE(OSAPI_LOGKIND_ERROR)
        }
#endif
        rw_entry = (struct RHSM_HistoryRWEntry*)REDA_Indexer_iterator_next(iterator);
    }
}

/*ci
 * \brief Return a loan after calling \ref RHSM_History_read_or_take
 *
 * \param[in] rh               The history cache to return the sequences to
 * \param[in] sample_ptr_array A sequence of pointers to samples to return
 * \param[in] info_array       A sequence of pointers to sample info to return
 * \param[in] sample_count     The number of samples to return in each sequence
 * \param[in] taken            Whether the samples where taken or not
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref RHSM_History_read_or_take
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
RHSM_History_finish_read_or_take(struct DDSHST_Reader *rh,
                                 void ***sample_ptr_array,
                                 struct DDS_SampleInfo ***info_array,
                                 DDS_Long sample_count,
                                 DDS_Boolean taken)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    DDS_Long i;
    struct RHSM_HistorySample *entry = NULL;
    RTI_INT32 old_sample_count = 0;

    UNUSED_ARG(taken);

    for (i = 0; i < sample_count; ++i)
    {
        entry = (struct RHSM_HistorySample*)((*info_array)[i]->reserved_data);
        if (entry == NULL)
        {
            continue;
        }

        --entry->_loan_count;

        /* When a sample is taken the entry is marked as taken for
         * removal when returned. In addition, samples that are returned
         * but has been removed from the history to make room is also
         * returned.
         */
        if ((entry->_flags & RHSM_HISTORY_FLAG_SAMPLE_TAKEN) ||
            !(entry->_flags & RHSM_HISTORY_FLAG_HISTORICAL))
        {
            old_sample_count = entry->_key_entry->_sample_count;
            RHSM_History_return_sample(self,entry,DDS_SAMPLE_LOST_NOT_LOST);
            if (entry->_key_entry->_sample_count < old_sample_count)
            {
                self->samples_returned = RTI_TRUE;
            }
        }

        /* An instance can only be removed under in the following case:
         *
         * The instance is disposed and the instance state is
         * NOT_ALIVE_NO_WRITERS. This means the
         * owner has both disposed and unregistered the instance and there
         * are no active writers updating the instance
         */
        if ((entry->_key_entry->_instance_state != DDS_ALIVE_INSTANCE_STATE) &&
            (entry->_key_entry->_not_seen_count == 0) &&
            (entry->_key_entry->_writer_count == 0))
        {
            RHSM_History_delete_key(self,entry->_key_entry);
        }
    }

    /* return sample_ptr and info arrays to pools */
    if (*sample_ptr_array != NULL)
    {
        REDA_BufferPool_return_buffer(self->_sample_ptr_pool, *sample_ptr_array);
        *sample_ptr_array = NULL;
    }
    if (*info_array != NULL)
    {
        REDA_BufferPool_return_buffer(self->_sample_info_pool, *info_array);
        *info_array = NULL;
    }

    if (self->recommit_samples && !self->in_listener)
    {
        RHSM_History_on_recommit_event(rh);
    }

    return DDS_RETCODE_OK;
}


/*ci
 * \brief Post an external event to the history cache
 *
 * \details
 * This function is the entry point to signal the queue for various
 * types of external events.
 *
 * \param[in] rh    The history cache
 * \param[in] event The event that occurred
 * \param[in] now   The time the event occurred
 */
RTI_PRIVATE void
RHSM_History_post_event(struct DDSHST_Reader *rh,
                               struct DDSHST_ReaderEvent *event,
                               struct OSAPI_NtpTime *now)
{
    struct RHSM_History *self = (struct RHSM_History *)rh;
    struct RHSM_HistoryRWEntry *rw_entry = NULL;

    switch (event->kind)
    {
        case DDSHST_READEREVENT_KIND_LIVELINESS_LOST:
        case DDSHST_READEREVENT_KIND_LIVELINESS_DETECTED:
            RHSM_History_update_liveliness(
                                        (struct RHSM_History*)self,event,now);
            break;
        case DDSHST_READEREVENT_KIND_REMOTE_WRITER_DELETED:
            rw_entry = RHSM_History_find_rw(self,&event->data.rw_deleted.rw_guid);
            if (rw_entry != NULL)
            {
                RHSM_History_prune_rw(rh,rw_entry);
            }
            break;
        case DDSHST_READEREVENT_KIND_DEADLINE_EXPIRED:
            RHSM_History_on_deadline_expired(self);
            break;
        default:
            break;
    }
}

/*******************************************************************************
 *                                Plugin API
 ******************************************************************************/

/*ci
 * \brief Implementation of the DDSHST_ReaderI interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct DDSHST_ReaderI RHSM_HistoryI_fv_Intf =
{
    RT_COMPONENTI_BASE,
    RHSM_History_return_entry,
    RHSM_History_read_or_take,
    RHSM_History_finish_read_or_take,
    RHSM_History_post_event,
    RHSM_History_lookup_key,
    RHSM_History_reserve_entry,
    RHSM_History_add_and_commit,
    RHSM_History_commit
};

/*ci
 * \brief Create a new instance of the reader history cache
 *
 * \details
 * Implementation of the RT ComponentFactory create component method. This
 * method is not called directly, only via the RT factory interface.
 *
 * \param[in] factory  The factory creating the component
 * \param[in] property The component property
 * \param[in] listener The component listener
 *
 * \return A new component on success, NULL on failure
 *
 * \sa \ref RHSM_HistoryFactory_delete_component
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
RHSM_HistoryFactory_create_component(struct RT_ComponentFactory *factory,
                                     struct RT_ComponentProperty *property,
                                     struct RT_ComponentListener *listener)
{
    struct RHSM_History *retval = NULL;
    UNUSED_ARG(factory);

    retval = RHSM_History_create(
                        (const struct RHSM_HistoryProperty *)property,
                        (const struct DDSHST_ReaderListener *)listener);

    if (retval == NULL)
    {
        return NULL;
    }

    return &retval->_parent._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the reader history instance
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This
 * method is not called directly, only via the RT factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref RHSM_HistoryFactory_create_component
 */
RTI_PRIVATE void
RHSM_HistoryFactory_delete_component(struct RT_ComponentFactory *factory,
                                     RT_Component_T *component)
{
    UNUSED_ARG(factory);
    RHSM_History_delete((struct RHSM_History *)component);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN static struct RT_ComponentFactory *
RHSM_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
RHSM_HistoryFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI RHSM_HistoryFactory_fv_Intf =
{
    RHSM_HISTORY_INTERFACE_ID,
    RHSM_HistoryFactory_initialize,
    RHSM_HistoryFactory_finalize,
    RHSM_HistoryFactory_create_component,
    RHSM_HistoryFactory_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI RHSM_HistoryFactory_fv_Intf =
{
    RHSM_HISTORY_INTERFACE_ID,
    RHSM_HistoryFactory_initialize,
    NULL, /* RHSM_HistoryFactory_finalize, */
    RHSM_HistoryFactory_create_component,
    NULL, /* RHSM_HistoryFactory_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief Singleton for the RHSM_HistoryFactory
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactory RHSM_HistoryFactory_fv_Factory =
{
    &RHSM_HistoryFactory_fv_Intf,
    NULL,
    {{{0,0}}}
};

/*ci
 * \brief Initialize the reader history cache factory
 *
 * \details
 * RHSM specific implementation of the RT ComponentFactory initialize method.
 * This method is not called directly, only when the factory is registered
 * with the RT.
 *
 * \param[in] property The properties registered with the history interface
 * \param[in] listener The listener registered with the history interface
 *
 * \return A fully initialized factory
 *
 * \sa \ref RHSM_HistoryFactory_finalize
 */
MUST_CHECK_RETURN struct RT_ComponentFactory*
RHSM_HistoryFactory_initialize(struct RT_ComponentFactoryProperty *property,
                               struct RT_ComponentFactoryListener *listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    RHSM_HistoryFactory_fv_Factory._factory = &RHSM_HistoryFactory_fv_Factory;

    return &RHSM_HistoryFactory_fv_Factory;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the reader history cache factory
 *
 * \details
 *
 * Implementation of the RT ComponentFactory finalize method. This method is
 * not called directly, only when the factory is unregistered from the RT.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref RHSM_HistoryFactory_initialize
 */
void
RHSM_HistoryFactory_finalize(struct RT_ComponentFactory *factory,
                             struct RT_ComponentFactoryProperty **property,
                             struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
RHSM_HistoryFactory_get_interface(void)
{
    return &RHSM_HistoryFactory_fv_Intf;
}

/*ci @} */
