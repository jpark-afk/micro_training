/*
 * FILE: ReaderHistory.h - Exported ReaderHistory functions
 *
 * Copyright (c) 2008-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 */
/*ci \addtogroup RHSMModule
 * @{
 */
#ifndef ReaderHistory_h
#define ReaderHistory_h

#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif

/*ci
 * \def MAX_OWNER_BITMAP_LENGTH
 * \brief Length in elements of an array to keep track of owners
 */
#define MAX_OWNER_BITMAP_LENGTH    (1U)

/*ci
 * \def MAX_OWNER_BITMAP_ELEMENT_SIZE
 * \brief Size of one bitmap element
 */
#define MAX_OWNER_BITMAP_ELEMENT_SIZE (sizeof(RTI_UINT32))

/*ci
 * \def RHSM_RW_PER_ELEMENT
 * \brief The number of remote writer that 1 element can keep track of
 */
#define RHSM_RW_PER_ELEMENT        (32U)

/*ci
 * \def RHSM_HISTORY_FLAG_SAMPLE_TAKEN
 * \brief Flag to indicate if a queue entry is taken or read from the cache
 */
#define RHSM_HISTORY_FLAG_SAMPLE_TAKEN 0x01U

/*ci
 * \def RHSM_HISTORY_FLAG_HISTORICAL
 * \brief Flag to indicate if a queue entry is considered a historical sample
 *        or only a cache entry.
 */
#define RHSM_HISTORY_FLAG_HISTORICAL   0x02U

/*ci
 * \def RHSM_HISTORY_FLAG_META
 * \brief Flag to indicate if an entry is a meta sample or not
 */
#define RHSM_HISTORY_FLAG_META         0x04U

/*ci
 * \def RHSM_HISTORY_FLAG_META_INUSE
 * \brief Flag to indicate if the meta sample is in use or not
 */
#define RHSM_HISTORY_FLAG_META_INUSE   0x08U

/*ci
 * \def RHSM_HISTORY_FLAG_RECOMMITTED
 * \brief Flag to indicate if the meta sample needs to be recommitted
 */
#define RHSM_HISTORY_FLAG_RECOMMITTED  0x10U

/*ci
 * \def RHSM_REMOVE_LEVEL_EQ
 * \brief Logical equal level used to sort samples for forced removal if needed
 */
#define RHSM_REMOVE_LEVEL_EQ            0

/*ci
 * \def RHSM_REMOVE_LEVEL_LT
 * \brief Logical less-than used to sort samples for forced removal if needed
 */
#define RHSM_REMOVE_LEVEL_LT            1

/*ci
 * \def RHSM_REMOVE_LEVEL_COUNT
 * \brief The number of levels for sorting samples for removal
 */
#define RHSM_REMOVE_LEVEL_COUNT         2

struct RHSM_HistoryKeyEntry;
struct RHSM_HistoryRWEntry;

/*ci
 * \def RHSM_HistorySample_from_node
 * \brief Macro to convert convert a mixin list node to a RHSM_HistorySamples
 *
 * \param[in] n_    The list node
 * \param[in] name_ The name of the list node
 *
 * \return Pointer to RHSM_HistorySamples
 */
#define RHSM_HistorySample_from_node(n_,name_) \
(struct RHSM_HistorySample*)((char*)(n_) - ((char*)(&((struct RHSM_HistorySample*)0)->name_)))

/*ci
 * \brief Implementation of a historical sample in the reader cache
 */
struct RHSM_HistorySample
{
    /*ci
     * \brief Used to link sample for the same instance _or_ writer
     */
    struct REDA_CircularListNode _node;

    /*ci
     * \brief The committal entry, links samples together in order based on Qos
     */
    struct RHSM_HistorySample *_ordered_prev;

    /*ci
     * \brief The committal entry, links samples together in order based on Qos
     */
    struct RHSM_HistorySample *_ordered_next;

    /*ci
     * \brief Used to link together samples which can be forcefully removed
     *        at a specific level
     */
    struct REDA_CircularListNode prune_node;

    /*ci
     * \brief Pointer to actual user-data
     */
    DDSHST_ReaderSample_T *_sample;

    /*ci
     * \brief Pointer to the key instance this sample belong to
     */
    struct RHSM_HistoryKeyEntry  *_key_entry;

    /*ci
     * \brief The number of references to this sample, cannot be removed
     *        if != 0
     */
    RTI_UINT16 _loan_count;

    /*ci
     * \brief Various status flags on the sample
     */
    RTI_UINT8 _flags;

    /*ci
     * \brief The current sample_state for this sample, read or not_read
     */
    RTI_UINT8 sample_state;

    /*ci
     * \brief The DDS sample info associated with this sample
     */
    struct DDS_SampleInfo _info;
};

/*ci
 * \brief The implementation of a remote writer as seen from the reader cache
 */
struct RHSM_HistoryRWEntry
{
    /*ci
     * \brief The remote writer's key
     */
    DDS_InstanceHandle_t _key;

    /*ci
     * \brief List of non-committed samples
     */
    REDA_CircularList_T _cached;

    /*ci
     * \brief The number of non-committed samples for this writer
     */
    RTI_INT32 _size;

    /*ci
     * \brief The current strength of this writer
     *
     */
    DDS_Long  _strength;

    /*ci
     * \brief The bitnum used by this writer for ownership selection
     */
    RTI_UINT32 bitnum;

    /*ci
     * \brief The array index where the writer bitnum is
     */
    RTI_UINT32 index;

    /*ci
     * \brief The highest uncommittable sequence number. This
     *        is used when samples are reocmmitted based on
     *        the of the cache when the sample was recommited
     *        or the writer sends a new commit.
     */
    struct REDA_SequenceNumber highest_noncommmitable_sn;

    /*ci
     * \brief The highest virtual sequence number
     */
    struct REDA_SequenceNumber last_virtual_sn;

    /*ci
     * \brief Flag for whether listener should be called upon committing samples
     */
    DDS_Boolean _notify_on_commit;
};

/*ci
 * \def RHSM_HistoryKeyEntry_from_node
 * \brief Macro to convert convert a mixin list node to a RHSM_HistoryKeyEntry
 *
 * \param[in] n_    The list node
 * \param[in] name_ The name of the list node
 *
 * \return Pointer to RHSM_HistoryKeyEntry
 */
#define RHSM_HistoryKeyEntry_from_node(n_,name_) \
(struct RHSM_HistoryKeyEntry*)((char*)(n_) - \
 ((char*)(&((struct RHSM_HistoryKeyEntry*)0)->name_)))


/*ci
 * \brief Implementation of a key entry
 */
struct RHSM_HistoryKeyEntry
{
    /*ci
     * \brief Keys are linked together for different reasons
     */
    struct REDA_CircularListNode _node;

    /*ci
     * \brief The key value
     */
    DDS_InstanceHandle_t _key;

    /*ci
     * \brief List of samples for this key
     */
    REDA_CircularList_T _samples;

    /*ci
     * \brief Pointer to the first sample in the current history queue. If
     *        max_samples_per_instance > depth, first_history_sample may not
     *        be the first sample in the _samples list.
     */
    struct RHSM_HistorySample *first_history_sample;

    /*ci
     * \brief Pointer to the last sample in the history queue.
     *        If max_samples_per_instance > depth, last_history_sample is
     *        always the last sample in the _samples list.
     */
    struct RHSM_HistorySample *last_history_sample;

    /*ci
     * \brief The number of samples in the cache
     */
    RTI_INT32 _sample_count;

    /*ci
     * \brief The number of samples that are currently not read in the cache
     */
    RTI_INT32 _not_seen_count;

    /*ci
     * \brief The current number of historical samples
     */
    RTI_INT32 _history_depth;

    /*ci
     * \brief The current instance state for the instance
     */
    DDS_Octet _instance_state;

    /*ci
     * \brief The current view state for the instance
     */
    DDS_Octet _view_state;

    /*ci
     * \brief The current number of writers updating this instance
     */
    RTI_INT32 _writer_count;

    /*ci
     * \brief The current owner, if any, for this instance
     */
    struct RHSM_HistoryRWEntry *_current_owner;

    /*ci
     * \brief Entry in the deadline queue when a deadline is finite
     */
    struct REDA_CircularListNode _deadline_entry;

    /*ci
     * \brief Entry in the timeline queue to determine which is the oldest
     *        instance
     */
    struct REDA_CircularListNode _timeline_entry;

    /*ci
     * \brief Whether the instance has been disposed or not
     */
    RTI_BOOL is_disposed;

    /*ci
     * \brief If the maximum number of owners are <=32 a statically allocated
     *        array is used, otherwise an array is dynamically allocated
     */
    RTI_UINT32 owner_bitmap[MAX_OWNER_BITMAP_LENGTH];

    /*ci
     * \brief If the maximum number of owners > 32 an array is allocated to keep
     *        track of remote writers
     */
    struct RHSM_HistoryRWEntry **owner_array;

    /*ci
     * \brief An instance can only have one outstanding meta sample so it is
     *        statically allocated and it is not counted against the
     *        resource-limits.
     */
    struct RHSM_HistorySample meta_sample;

    /*ci
     * \brief Each instance has a list of samples that can be forcefully removed
     *        from it
     */
    REDA_CircularList_T _prune;

    /*ci
     * \brief The last time this instance was updated in ticktime.
     *        This is used to determine if the deadline period has been
     *        exceeded.
     */
    struct DDS_Duration_t last_update_time;

    /*ci
     * \brief The last time this instance was checked periodically
     */
    struct DDS_Duration_t last_period_time;

    /*ci
     * \brief TRUE if this instance is owned
     */
    RTI_BOOL has_owner;
};

static const RTI_UINT32 RHSM_QUEUE_OPERATION_NONE            = 0;

/*ci
 * \brief A writer was allocated as part of a queue operation, can be cancelled
 */
static const RTI_UINT32 RHSM_QUEUE_OPERATION_ALLOCATE_WRITER = 0x1;

/*ci
 * \brief A number of queue operations can be applied to the queue as part
 *        of reserving an entry.
 */
typedef RTI_UINT32 RHSM_QueueOperation_T;

/*ci
 * \brief Structure used to make reservations in the reader-cache, a
 *        reservation must be committed before ending up in the cache
 */
struct RHSM_HistoryReservedEntry
{
    /*ci
     * \brief A history sample to use
     */
    struct RHSM_HistorySample *sample;

    /*ci
     * \brief The writer the entry was allocated for
     */
    struct RHSM_HistoryRWEntry *rw_entry;

    /*ci
     * \brief The queue operations that where applied to reserve this entry
     */
    RHSM_QueueOperation_T q_op;
};

#if OSAPI_THREAD_SEMAPHORE_ENABLED
/*ci
 * \brief Structure to save state for pending threads
 */
struct RHSM_ThreadSuspendState
{
    /*ci \brief Part of a list
     */
    struct REDA_CircularListNode _node;

    /*ci \brief Semaphore to pend on
     */
    struct OSAPI_Semaphore *wakeup_sem;

    /*ci \brief When the thread was suspended
     */
    struct OSAPI_SystemTime suspend_start;

    /*ci \brief State to restore in case the thread is suspended
     */
    struct RHSM_HistoryReservedEntry reserved_entry;

    /*ci \brief the strength of the writer being suspended
     */
    DDS_Long rw_strength;
};
#endif

/*ci
 * \brief Implementation structure for the RHSM queue
 */
struct RHSM_History
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct DDSHST_Reader _parent;

    /*ci
     * \brief A database to create tables in if needed
     */
    DB_Database_T db;

    /*ci
     * \brief A timer to create timeouts from if needed
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief Configuration of reader-history
     */
    RTI_UINT32 config;

    /*ci
     * \brief The deadline per instance
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*ci
     * \brief The maximum blocking time for a RELIABLE and KEEP_ALL history
     */
    struct DDS_Duration_t max_blocking_time;

    /*ci
     * \brief The maximum number of remote writers
     */
    RTI_INT32 max_remote_writers_per_instance;

    /*ci
     * \brief The cache listener the queue was created with
     */
    struct DDSHST_ReaderListener _listener;

    /*ci
     * \brief List of ordered samples for this queue
     */
    struct RHSM_HistorySample _s_ordered;

    /*ci
     * \brief Index to look up key entries
     */
    REDA_Indexer_T *_key_index;

    /*ci
     * \brief Index to look up remote writer entries
     */
    REDA_Indexer_T *_rw_index;

    /*ci
     * \brief Pool to allocate keys from
     */
    REDA_BufferPool_T _key_pool;

    /*ci
     * \brief Pool to allocate remote writers from
     */
    REDA_BufferPool_T _rw_pool;

    /*ci
     * \brief Pool of history entries
     */
    REDA_BufferPool_T _sample_pool;

    /*ci
     * \brief Pool of sample infos
     */
    REDA_BufferPool_T _sample_info_pool;

    /*ci
     * \brief Pool of arrays to loan samples
     */
    REDA_BufferPool_T _sample_ptr_pool;

    /*ci
     * \brief List of samples ordered by epoch to detect deadline loss
     */
    REDA_CircularList_T _deadline_timer;

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    /*ci
     * \brief List of blocked thread waiting for resources in this
     *        queue.
     */
    REDA_CircularList_T suspended_threads;
#endif

    /*ci
     * \brief List of samples ordered by time to remove the oldest instance
     *        if needed
     */
    REDA_CircularList_T _instance_timeline;

    /*ci
     * \brief Only one outstanding reservation is possible at a time
     */
    struct RHSM_HistoryReservedEntry reserved_entry;

    /*ci
     * \brief The length of the owner array if the number of remote writer > 32
     */
    RTI_INT32 owner_length;

    /*ci
     * \brief The current list to forcefully remove sample from (if any)
     */
    REDA_CircularList_T *remove_level;

    /*ci
     * \brief Internal value for max_samples_per_instance, set to max_samples
     *        if UNLIMITED.
     */
    DDS_Long max_samples_per_instance;

    /*ci
     * \brief Internal value for depth. Set to max_samples_per_instance in the
     *        case of KEEP_ALL.
     */
    DDS_Long history_depth;
};

/* These constants defined the mask and value for the history configuration
 * MASK - Relevant bits for configuration
 * VALUE - A specific value within the mask
 */

 /*ci \brief MASK for bits relevant to DURABILITY kind. The bits are mutually
  *          exclusive
  */
#define RHSM_HISTORY_DURABILITY_KIND_MASK            0x00000007u

/*ci \brief VOLATILE durability, this is the default value (0)
 */
#define RHSM_HISTORY_DURABILITY_KIND_VOLATILE               0x0u

/*ci \brief TRANSIENT_LOCAL durability
 */
#define RHSM_HISTORY_DURABILITY_KIND_TRANSIENT_LOCAL        0x1u

/*ci \brief TRANSIENT durability, reserved but not supported
 */
#define RHSM_HISTORY_DURABILITY_KIND_TRANSIENT              0x2u

/*ci \brief PERSISTENT durability, reserved but not supported
 */
#define RHSM_HISTORY_DURABILITY_KIND_PERSISTENT             0x4u

/*ci \brief Set durability to VOLATILE (no-op)
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_volatile_durability(struct RHSM_History *rh)
{
     rh->config = (rh->config & ~RHSM_HISTORY_DURABILITY_KIND_MASK) |
                    RHSM_HISTORY_DURABILITY_KIND_VOLATILE;
}

/*ci \brief Set durability to TRANSIENT_LOCAL
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_transient_local_durability(struct RHSM_History *rh)
{
    rh->config = (rh->config & ~RHSM_HISTORY_DURABILITY_KIND_MASK) |
                    RHSM_HISTORY_DURABILITY_KIND_TRANSIENT_LOCAL;
}

/*ci \brief Return true if the history is VOLATILE (0). Note that the default
 *   value 0 which is VOLATILE.
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if VOLATILE, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_volatile_durability(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_DURABILITY_KIND_MASK) == 0;
}

/*ci brief Return true if the history is TRANSIENT_LOCAL
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if TRANSIENT_LOCAL, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_transient_local_durability(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_DURABILITY_KIND_TRANSIENT_LOCAL) != 0;
}

/*ci \brief MASK for bits relevant to HISTORY kind.
 */
#define RHSM_HISTORY_HISTORY_KIND_MASK               0x00000008u

/*ci \brief KEEP_ALL HISTORY kind
 */
#define RHSM_HISTORY_HISTORY_KIND_KEEP_ALL           0x00000008u

/*ci \brief Set KEEP_ALL history
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_keep_all(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_HISTORY_KIND_KEEP_ALL;
}

/*ci \brief Return TRUE if history is KEEP_ALL
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if KEEP_ALL, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_keep_all(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_HISTORY_KIND_KEEP_ALL) != 0;
}

/*ci \brief Return TRUE if history is KEEP_LAST (a value of 0)
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if KEEP_LAST, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_keep_last(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_HISTORY_KIND_MASK) == 0;
}

/*ci \brief MASK for bits relevant to RELIABLE kind.
 */
#define RHSM_HISTORY_RELIABLE_KIND_MASK             0x00000010u

/*ci \brief RELIABLE history
 */
#define RHSM_HISTORY_RELIABLE_KIND_RELIABLE         0x00000010u

/*ci \brief Set RELIABLE history
 *
 * \param[in] rh ReaderHistory instance to set
*/
RTI_PRIVATE inline void
RHSM_History_set_reliable(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_RELIABLE_KIND_RELIABLE;
}

/*ci \brief Return TRUE if history is reliable
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if RELIABLE, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_reliable(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_RELIABLE_KIND_RELIABLE) != 0;
}

/*ci \brief Return TRUE if history is best_effort. Note that the default value
 *   0 is BEST_EFFORT.
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if RELIABLE, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_best_effort(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_RELIABLE_KIND_MASK) == 0;
}

/*ci \brief MASK for bits relevant to INSTANCE_REPLACEMENT
 */
#define RHSM_HISTORY_INSTANCE_REPLACEMENT_KIND_MASK     0x00000020u

/*ci \brief Replace oldest instance
 */
#define RHSM_HISTORY_INSTANCE_REPLACEMENT_KIND_OLDEST   0x00000020u

/*ci \brief Set REPLACE_OLDEST instance history
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_replace_oldest_instance(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_INSTANCE_REPLACEMENT_KIND_OLDEST;
}

/*ci \brief Return TRUE if instances are not replaced
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if NO_REPLACEMENT, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_replace_no_instance(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_INSTANCE_REPLACEMENT_KIND_MASK) == 0;
}

/*ci \brief Return TRUE if oldest instance is replaced
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if OLDEST_REPLACEMENT, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_replace_oldest_instance(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_INSTANCE_REPLACEMENT_KIND_OLDEST) != 0;
}

/*ci \brief MASK for bits relevant to OWNERSHIP
 */
#define RHSM_HISTORY_OWNERSHIP_KIND_MASK             0x00000040u

/*ci \brief EXCLUSIVE OWNERSHIP
 */
#define RHSM_HISTORY_OWNERSHIP_KIND_EXCLUSIVE        0x00000040u

/*ci \brief Set EXCLUSIVE_OWNERSHIP history
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_ownership_exclusive(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_OWNERSHIP_KIND_EXCLUSIVE;
}

/*ci \brief Return TRUE if ownership is exclusive
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if EXCLUSIVE ownership, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_ownership_exclusive(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_OWNERSHIP_KIND_EXCLUSIVE) != 0;
}

/*ci \brief Return TRUE if ownership is shared
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if SHARED ownership, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_ownership_shared(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_OWNERSHIP_KIND_MASK) == 0;
}

/*ci \brief MASK for bits relevant to RECOMMIT_SAMPLES
 */
#define RHSM_HISTORY_RECOMMIT_SAMPLES_MASK           0x00000080u

/*ci \brief Indicate samples are recommitted
 */
#define RHSM_HISTORY_RECOMMIT_SAMPLES                0x00000080u

/*ci \brief Set recommit samples to TRUE
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_recommit_samples(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_RECOMMIT_SAMPLES;
}

/*ci \brief Return TRUE if samples are to be recommitted
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if RECOMMIT samples, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_recommit_samples(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_RECOMMIT_SAMPLES) != 0;
}

/*ci \brief Clear recommit samples
 *
 * \param[in] rh ReaderHistory instance to clear
 */
RTI_PRIVATE inline void
RHSM_History_clear_recommit_samples(struct RHSM_History *rh)
{
    rh->config &= ~RHSM_HISTORY_RECOMMIT_SAMPLES;
}

/*ci \brief MASK for bits relevant to enabling deadline
 */
#define RHSM_HISTORY_DEADLINE_ENABLED_MASK           0x00000100u

/*ci \brief Bit to enable deadline
 */
#define RHSM_HISTORY_DEADLINE_ENABLED                0x00000100u

/*ci \brief Enable deadline on history
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_deadline_enabled(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_DEADLINE_ENABLED;
}

/*ci \brief Return TRUE if deadline is enabled
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if deadline is enabled, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_deadline_enabled(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_DEADLINE_ENABLED) != 0;
}

/*ci \brief MASK for bits relevant to indicate that samples are returned
 */
#define RHSM_HISTORY_SAMPLES_RETURNED_MASK           0x00000200u

/*ci \brief Bit to indicate that samples have been returned
 */
#define RHSM_HISTORY_SAMPLES_RETURNED                0x00000200u

/*ci \brief Set samples returned to TRUE
 *
 * \param[in] rh ReaderHistory instance to set
 */
RTI_PRIVATE inline void
RHSM_History_set_samples_returned(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_SAMPLES_RETURNED;
}

/*ci \brief Set samples returned to FALSE
 *
 * \param[in] rh ReaderHistory instance to clear
 */
RTI_PRIVATE inline void
RHSM_History_clear_samples_returned(struct RHSM_History *rh)
{
    rh->config &= ~RHSM_HISTORY_SAMPLES_RETURNED;
}

/*ci \brief Return TRUE if samples have been returned
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if samples have been returned, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_samples_returned(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_SAMPLES_RETURNED) != 0;
}

/*ci \brief MASK for relevant bits to indicate that the history is in a listener
 */
#define RHSM_HISTORY_IN_LISTENER_MASK                0x00000400u

/*ci \brief Bit to indicate that the history is in a listener
 */
#define RHSM_HISTORY_IN_LISTENER                     0x00000400u

/*ci \brief Set that the history is in a listener
 *
 * \param[in] rh ReaderHistory instance to test
 */
RTI_PRIVATE inline void
RHSM_History_set_in_listener(struct RHSM_History *rh)
{
    rh->config |= RHSM_HISTORY_IN_LISTENER;
}

/*ci brief Set that the history is no longer in a listener
 *
 * \param[in] rh ReaderHistory instance to clear
 */
RTI_PRIVATE inline void
RHSM_History_clear_in_listener(struct RHSM_History *rh)
{
    rh->config &= ~RHSM_HISTORY_IN_LISTENER;
}

/*ci \brief Return TRUE if the history is in a listener
 *
 * \param[in] rh ReaderHistory instance to test
 *
 * \return TRUE if history is in listener, FALSE if not
 */
RTI_PRIVATE inline RTI_BOOL
RHSM_History_is_in_listener(const struct RHSM_History *rh)
{
    return (rh->config & RHSM_HISTORY_IN_LISTENER) != 0;
}

#endif

/*ci @} */
