/*
 * FILE: History.h - Writer History implementation
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
 * 14sep2011 tk Bug fixes and performance improvements
 * 12jun2011 tk Written
 */
/*ce
 * \file
 * \brief Writer History implementation
 */
/*ci \addtogroup WHSMModule
 * @{
 */
#ifndef History_h
#define History_h

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
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
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef wh_sm_log_h
#include "wh_sm/wh_sm_log.h"
#endif

#include "wh_sm/wh_sm_history.h"
#include "reda/reda_indexer.h"

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_FREE
 * \brief Flag to indicate if a history cache entry is free for use
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_FREE        (0x0)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED
 * \brief Flag to indicate if a history cache entry has been acked by all
 *        recipients
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_ACKED       (0x1)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED
 * \brief Flag to indicate if a history cache entry is nacked by 1 or more
 *        recipient
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_NACKED      (0x2)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL
 * \brief Flag to indicate if a history cache entry is considered a historical
 *        sample or only a cache entry
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_HISTORICAL  (0x4)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED
 * \brief Flag to indicate if a history cache entry has been pruned/removed
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_PRUNED      (0x8)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_RETURNED
 * \brief Flag to indicate if a history cache entry was returned without being
 *        used
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_RETURNED        (0x10)

/*ci
 * \def RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL
 * \brief Flag to indicate that a sample is transitioned from a historical sample to
 *        a non historical sample.
 */
#define RTI_SMWRITERHISTORY_SAMPLE_STATE_NON_HISTORICAL  (0x20)

struct WHSM_HistoryKeyEntry;

/*ci
 * \brief A sample entry in the history cache
 */
struct WHSM_HistorySampleEntry
{
    /*ci
     * \brief  _node is used to link all samples for a given instance together.
     */
    struct REDA_CircularListNode _node;

    /*ci
     * \brief Pointer to the actual user data. The queue doesn't care what it is
     */
    struct DDSHST_WriterSample *_sample;

    /*ci
     * \brief The SN of this entry. The SN is guaranteed to be unique for this
     *        instance
     */
    struct REDA_SequenceNumber sn;

    /*ci
     * \brief The current state of the sample
     */
    RTI_INT32 _state;

    /*ci
     * \brief Which key does it belong to
     */
    struct WHSM_HistoryKeyEntry *key_entry;

    /*ci
     * \brief What kind of entry is this in the cache, user or meta data
     */
    DDSHST_WriterEntryKind_T kind;

    /*ci
     * \brief The expected number of recipient for this entry at the time the
     *        same was written (only used for reliable communication)
     */
    DDS_Long ack_count;
};

/*ci
 * \brief The valid states for a key in the writer cache
 */
typedef enum
{
    /*ci
     * \brief The key is alive, it is being updated
     */
    WHSM_HISTORY_KEY_STATE_ALIVE = 1,

    /*ci
     * \brief The key was unregistered from the writer cache
     */
    WHSM_HISTORY_KEY_STATE_NOT_ALIVE,
} WHSM_WriterKeyState_T;

/*ci
 * \brief Convenience function to convert a node to a WHSM_HistoryKeyEntry based
 *        on the name field
 */
#define WHSM_HistoryKeyEntry_from_node(n_,name_) \
(struct WHSM_HistoryKeyEntry*)((char*)(n_) - ((char*)(&((struct WHSM_HistoryKeyEntry*)0)->name_)))

/*ci
 * \brief An entry in the writer cache
 *
 * \details
 * Store information about each unique key registered or detected
 * Each key has a list of samples for that key.
 */
struct WHSM_HistoryKeyEntry
{
    /*ci
     * \brief The instance key
     */
    DDS_InstanceHandle_t key;

    /*ci \brief List of all outstanding samples for a given instance
     *
     * \details
     * A sample can only by in at most one sample structure at any given time.
     * Sample is ordered so that the highest SN (newest sample) is at the
     * beginning of the list.
     */
    REDA_CircularList_T samples;

    /*ci
     * \brief Pointer to the last historical sample for this instance
     */
    struct WHSM_HistorySampleEntry *last_history_sample;

    /*ci
     * \brief
     * The number of samples currently in use by this instance
     */
    DDS_Long sample_count;

    /*ci
     * \brief Entry in the deadline list for a finite deadline qos policy
     */
    struct REDA_CircularListNode _deadline_entry;

    /*ci
     * \brief Current state of the key entry
     */
    WHSM_WriterKeyState_T state;

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
     * \brief The last time this instance was updated based on the source
     *        time-stamp provided by the caller. This is used to support
     *        BY_SOURCE_TIMESTAMP ordering and the source_timestamp_tolerance.
     */
    struct OSAPI_NtpTime source_timestamp;
};

/*ci
 * \brief Definition of the history cache
 */
struct WHSM_History
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct DDSHST_Writer _parent;

    /*ci
     * \brief The properties the cache was created with
     */
    struct WHSM_HistoryProperty _property;

    /*ci
     * \brief Reference to the datawriter creating this cache Qos
     */
    const struct DDS_DataWriterQos *_qos;

    /*ci
     * \brief The listener the cache was created with
     */
    struct DDSHST_WriterListener _listener;

    /*ci
     * \brief Structure holding state information for the writer
     */
    struct DDSHST_WriterState _state;

    /*ci
     * \brief List of instances when they where last updated for deadline
     */
    REDA_CircularList_T _deadline_timer;

    /*ci
     * \brief Whether code to check deadline should be enabled or not
     */
    RTI_BOOL deadline_enabled;

    /*ci
     * \brief Pool with the maximum number of samples in the cache
     */
    REDA_BufferPool_T sample_pool;

    /*ci
     * \brief Pool with the maximum number of keys in the cache
     */
    REDA_BufferPool_T key_pool;

    /*ci
     * \brief Index for fast lookup of any sequence number
     */
    REDA_Indexer_T *sample_index;

    /*ci
     * \brief Index for fast lookup of a key entry
     */
    REDA_Indexer_T *key_index;

    /*ci
     * \brief Index for fast lookup of historical SN
     */
    REDA_Indexer_T *historical_index;

    /*ci
     * \brief Source time stamp tolerance in Ntp format, converted from the
     *        Qos policy Duration_t format.
     */
    struct OSAPI_NtpTime source_timestamp_tolerance;

};

#endif

/*ci @} */
