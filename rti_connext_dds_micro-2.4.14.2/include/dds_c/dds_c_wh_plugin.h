/*
 * FILE: dds_c_wh_plugin.h - DDS writer history interface
 *
 * (c) Copyright, Real-Time Innovations, 2011-2020.
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
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 24feb2014,tk MICRO-714: Updated for improved writer history API.
 * 03feb2014,eh MICRO-714: acknack_sample, on_sample_removed
 * 16aug2011,tk Written
 */
/*ce
 * \file
 * \brief DDS writer history interface
 */
/*ci \defgroup DDSHistoryModule DDS History module
 *   \ingroup DDSCModule
 */
/*ci \defgroup DDSWriterHistoryModule DDS Writer History module
 *   \ingroup DDSHistoryModule
 */
/*ci \addtogroup DDSWriterHistoryModule
 * @{
 */
#ifndef dds_c_wh_plugin_h
#define dds_c_wh_plugin_h

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef dds_c_topic_h
#include "dds_c/dds_c_topic.h"
#endif
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ce
 * \brief Default writer history plug-in name
 */
DDSCDllVariable extern const char* const DDSHST_WRITER_DEFAULT_HISTORY_NAME;

/*ci
 * \brief Enumeration of supported writer history events
 */
typedef enum
{
    /*ci
     * \brief The deadline timer expired, check deadlines
     */
    DDSHST_WRITEREVENT_KIND_DEADLINE_EXPIRED          = 1,

    /*ci
     * \brief A late joiner is requesting historical data
     */
    DDSHST_WRITEREVENT_KIND_HISTORICAL_DATA_REQUESTED
} DDSHST_WriterEventKind_T;

/*ci
 * \brief A writer history event that can be posted to the cache
 */
struct DDSHST_WriterEvent
{
    /*ci
      * \brief The event type
      */
    DDSHST_WriterEventKind_T kind;
};

/*ci
 * \brief Base-class for writer history cache
 */
struct DDSHST_Writer
{
    struct RT_Component _parent;
};

typedef struct DDSHST_Writer DDSHST_Writer_T;

struct DDSHST_WriterSampleEntry;

/*ci
 * \brief Implementation independent reference to a sample in the
 *        writer history cache
 */
typedef struct DDSHST_WriterSampleEntry* DDSHST_WriterSampleEntryRef_T;

/*ci
 * \brief Implementation independent writer cache sample
 */
typedef struct DDSHST_WriterSample
{
    /*ci
     * \brief Reference to the corresponding entry in the implementation specific
     *        cache
     */
    DDSHST_WriterSampleEntryRef_T _ref;
} DDSHST_WriterSample_T;

/*ci
 * \brief The current state of the writer cache
 */
struct DDSHST_WriterState
{
    /*ci
     * \brief The lowest SN available, across all instances
     */
    struct REDA_SequenceNumber low_sn;

    /*ci
     * \brief The highest SN available, across all instances
     */
    struct REDA_SequenceNumber high_sn;

    /*ci
     * \brief The lowest historical SN available, across all instances
     */
    struct REDA_SequenceNumber history_low_sn;

    /*ci
     * \brief The highest historical SN available, across all instances
     */
    struct REDA_SequenceNumber history_high_sn;
};

/*ci
 * \brief Base-class properties for the DDS writer history cache
 */
struct DDSHST_WriterProperty
{
    /*ci
     * \brief Inherit from a component
     */
    struct RT_ComponentProperty _parent;

    /*ci
     * \brief Pointer to the qos policy for the DDS writer that uses the history
     *        cache
     */
    struct DDS_DataWriterQos *qos;
};

/*ci
 * \brief Constant to initialize \ref DDSHST_WriterProperty
 */
#define DDSHST_WriterProperty_INITIALIZER \
{\
  RT_ComponentProperty_INITIALIZER,\
  NULL\
}

/*ci
 * \brief Enumeration of the different reasons a sample was removed from the
 *        writer cache
 */
typedef enum
{
    /*ci
     * \brief Normal removal, the cache no longer needs this sample
     */
    DDSHST_WRITER_SAMPLE_REMOVED_NORMAL,

    /*ci
     * \brief A sample was forcefully removed to make space, 1 or reliable readers
     *        did not receive it
     */
    DDSHST_WRITER_SAMPLE_REMOVED_UNACKED,

    /*ci
     * \brief The sample was removed because the key was unregistered
     */
    DDSHST_WRITER_SAMPLE_REMOVED_KEY_UNREGISTERED,

    /*ci
     * \brief The sample was removed because the key was disposed
     */
    DDSHST_WRITER_SAMPLE_REMOVED_KEY_DISPOSED,

    /*ci
     * \brief The sample was removed because the resource limit for samples
     *        per instance was reached and KEEP_LAST is in effect
     */
    DDSHST_WRITER_SAMPLE_REMOVED_MAX_SAMPLES_PER_INSTANCE
} DDSHST_WriterSampleRemovedKind_T;

/*ci
 * \brief Enumeration of the different reasons a key was removed from the
 *        writer cache
 */
typedef enum
{
    /*ci
     * \brief Normal removal, the key is no longer needed
     */
    DDSHST_WRITER_KEY_REMOVED_NORMAL,

    /*ci
     * \brief The key was removed due to being unregistered
     */
    DDSHST_WRITER_KEY_REMOVED_UNREGISTERED,

    /*ci
     * \brief The key was removed due to being disposed of
     */
    DDSHST_WRITER_KEY_REMOVED_DISPOSED,

    /*ci
     * \brief The key was removed due to being deleted
     */
    DDSHST_WRITER_KEY_REMOVED_DELETED
} DDSHST_WriterKeyRemovedKind_T;

/*ci
 * \brief Enumeration of the different types of queue entries
 */
typedef enum
{
    /*ci
     * \brief The empty entry, used when an entry is only registered
     */
    DDSHST_WRITER_ENTRY_NONE,

    /*ci
     * \brief Normal user data entry
     */
    DDSHST_WRITER_ENTRY_NORMAL,

    /*ci
     * \brief The sample is the result of an unregister operation
     */
    DDSHST_WRITER_ENTRY_UNREGISTER,

    /*ci
     * \brief The sample is the result of a dispose operation
     */
    DDSHST_WRITER_ENTRY_DISPOSE,

    /*ci
     * \brief The sample is the result of a unregister and dispose operation
     */
    DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE
} DDSHST_WriterEntryKind_T;

/*ci
 * \brief The current state of an instance in the instance cache if it exists
 */
struct DDSHST_InstanceState
{
    /*ci The number of samples owned by the instance
     */
    RTI_INT32 sample_count;

    /*ci The type of the last sample added for the instance
     */
    DDSHST_WriterEntryKind_T last_sample_kind;
};

/*ci
 * \def DDSHST_WRITER_ACKCOUNT_ANY
 * \brief Constant to indicate that the number of ACKNACKs for a sample is
 *        not relevant
 */
#define DDSHST_WRITER_ACKCOUNT_ANY (0)

/*ci
 * \brief Definition of the \ref DDSHST_WriterListener on_sample_removed callback
 *
 * \details
 * When the history cache removes a sample it calls this listener if it is
 * installed.
 *
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key the sample belonged too
 * \param[in] sample        The sample being removed
 * \param[in] sn            The sequence number of the removed sample
 * \param[in] kind          The reason the sample is removed
 * \param[in] ack_count     The current number of acknowledgment on the sample
 */
typedef void
(*DDSHST_WriterListener_OnSampleRemovedFunc)(
                                        void *listener_data,
                                        DDS_InstanceHandle_t *key,
                                        struct DDSHST_WriterSample *sample,
                                        struct REDA_SequenceNumber *sn,
                                        DDSHST_WriterSampleRemovedKind_T kind,
                                        DDS_Long ack_count);
/*ci
 * \brief Definition of the \ref DDSHST_WriterListener on_deadline_missed callback
 *
 * \details
 * When an instance is not being updated according to the deadline the cache
 * calls this listener if it is installed.
 *
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key that missed its deadline
 */
typedef void
(*DDSHST_WriterListener_OnDeadlineMissedFunc)(void *listener_data,
                                              DDS_InstanceHandle_t *key);

/*ci
 * \brief Definition of the \ref DDSHST_WriterListener on_key_removed callback
 *
 * \details
 * When the history cache removes a key it calls this listener if it is
 * installed.
 *
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key being removed
 * \param[in] kind          The reason for removing the key
 */
typedef void
(*DDSHST_WriterListener_OnKeyRemovedFunc)(void *listener_data,
                                          DDS_InstanceHandle_t *key,
                                          DDSHST_WriterKeyRemovedKind_T kind);

/*ci
 * \brief Base-class for  DDS Writer history listener
 */
struct DDSHST_WriterListener
{
    /*ci
     * \brief Inherit from parent class
     */
    struct RT_ComponentListener _parent;

    /*ci
     * \brief Called when the cache removed a sample
     */
    DDSHST_WriterListener_OnSampleRemovedFunc on_sample_removed;

    /*ci
     * \brief Called when the cache removed a key
     */
    DDSHST_WriterListener_OnKeyRemovedFunc on_key_removed;

    /*ci
     * \brief Called when the cache detected deadline missed on an instance
     */
    DDSHST_WriterListener_OnDeadlineMissedFunc on_deadline_missed;

    /*ci
     * \brief Opaque listener data passed back to listener functions
     */
    void *listener_data;
};

/*ci
 * \brief Constant to initialize \ref DDSHST_WriterListener
 */
#define DDSHST_WriterListener_INITIALIZE {\
  RT_ComponentListener_INITIALIZER,\
  NULL,\
  NULL,\
  NULL,\
  NULL\
}

/* ------------------------------ Life-cycle -------------------------------- */
/*ci
 * \brief Definition of \ref RT_ComponentFactory delete_component method
 *
 * \param[in] self The writer history instance to delete
 *
 * \sa \ref RT_ComponentFactoryI
 */
typedef void
(*DDSHST_Writer_deleteFunc)(struct DDSHST_Writer *self);

/*ci
 * \brief Definition of \ref RT_ComponentFactory create_component method
 *
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A reference to a new writer cache on success, NULL otherwise
 *
 * \sa \ref RT_ComponentFactoryI
 */
FUNCTION_MUST_TYPEDEF(
struct DDSHST_Writer*
(*DDSHST_Writer_createFunc) (const struct DDSHST_WriterProperty* const property,
                             const struct DDSHST_WriterListener* const listener)
)

/* ------------------------------- Methods ---------------------------------- */
/*ci
 * \brief Definition of \ref DDSHST_WriterI get_entry method
 *
 * \param[in] self       The writer cache to get an entry from
 * \param[in] key        The key to get an entry for
 * \param[in] kind       The kind of entry (dispose, unregister, normal etc.)
 * \param[in] assert_key If TRUE, assert the key if it does not already exist.
 *                       Otherwise return NULL if the key does not exist.
 * \param[in] source_ts  The source time stamp this instance was registered.
 *
 * \return A reference to new entry if there is space, NULL otherwise
 *
 * \sa \ref DDSHST_Writer_get_entry, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_WriterSampleEntryRef_T
(*DDSHST_Writer_get_entryFunc)(struct DDSHST_Writer *self,
                           const DDS_InstanceHandle_t *const key,
                           DDSHST_WriterEntryKind_T kind, RTI_BOOL assert_key,
                           struct OSAPI_NtpTime *source_ts)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI return_entry method
 *
 * \details
 * Return an entry to the cache
 *
 * \param[in] self  The history cache
 * \param[in] entry A reference to the entry to return
 *
 * \sa \ref DDSHST_Writer_return_entry, \ref DDSHST_WriterI
 */
typedef void
(*DDSHST_Writer_return_entryFunc)(struct DDSHST_Writer *self,
                                  DDSHST_WriterSampleEntryRef_T const entry);

/*ci
 * \brief Definition of \ref DDSHST_WriterI commit_entry method
 *
 * \details
 * Method to commit a a sample to the cache
 *
 * \param[in]  self    The history cache
 * \param[out] sample  The requested sample if found, NULL otherwise
 * \param[in]  sn      The requested SN
 * \param[out] sn_ge   The first available SN
 * \param[in]  h       Whether only historical samples are relevant
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref DDSHST_ReturnCode_T
 *         on failure
 *
 * \sa \ref DDSHST_Writer_commit_entry, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Writer_commit_entryFunc)(struct DDSHST_Writer * self,
                                  DDSHST_WriterSampleEntryRef_T entry,
                                  DDSHST_WriterSample_T *const sample,
                                  const struct REDA_SequenceNumber *const sn,
                                  DDS_Long ack_count)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI request_sample method
 *
 * \details
 * Method to request a sample from the cache
 *
 * \param[in]  self    The history cache
 * \param[out] sample  The requested sample if found, NULL otherwise
 * \param[in]  sn      The requested SN
 * \param[out] sn_ge   The first available SN
 * \param[in]  h       Whether only historical samples are relevant
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref DDSHST_ReturnCode_T
 *         on failure
 *
 * \sa \ref DDSHST_Writer_request_sample, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Writer_request_sampleFunc)(struct DDSHST_Writer *self,
                                struct DDSHST_WriterSample **sample,
                                const struct REDA_SequenceNumber *const req_sn,
                                struct REDA_SequenceNumber *const sn_ge,
                                DDS_Boolean historical_only)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI acknack_sample method
 *
 * \details
 * Method to ACK or NACK a sample in the queue
 *
 * \param[in] self The writer cache
 * \param[in] sn   The SN to ACKNACK
 * \param[in] nack Whether this is a NACK (DDS_BOOLEAN_TRUE) or ACK
 *                 (DDS_BOOLEAN_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref
 *         DDSHST_ReturnCode_T on failure
 *
 * \sa \ref DDSHST_Writer_acknack_sample, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Writer_acknack_sampleFunc)(struct DDSHST_Writer *self,
                                    const struct REDA_SequenceNumber *const sn,
                                    DDS_Boolean nack)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI register_key method
 *
 * \details
 * Method to add a key to the cache if there is space
 *
 * \param[in] self The history cache
 * \param[in] key  The key to add to the cache
 * \param[in] timestamp The time the key was registered
 * 
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref
 *         DDSHST_ReturnCode_T on failure
 *
 * \sa \ref DDSHST_Writer_register_key, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Writer_register_keyFunc)(struct DDSHST_Writer *self,
                                  const DDS_InstanceHandle_t *const key,
                                  const struct OSAPI_NtpTime *timestamp)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI get_state method
 *
 * \details
 * Return the current state of the writer cache.
 *
 * \param[in] self  The history cache
 *
 * \return Pointer to the structure containing the state
 *
 * \sa \ref DDSHST_Writer_get_state, \ref DDSHST_WriterI
 */
FUNCTION_MUST_TYPEDEF(
struct DDSHST_WriterState*
(*DDSHST_Writer_get_stateFunc)(struct DDSHST_Writer *self)
)

/*ci
 * \brief Definition of \ref DDSHST_WriterI post_event method
 *
 * \details
 * Handle possible events posted to the cache by the user.
 *
 * \param[in] self  The history cache
 * \param[in] event The event
 * \param[in] now   The time of the event
 *
 * \sa \ref DDSHST_Writer_post_event, \ref DDSHST_WriterI
 */
typedef void
(*DDSHST_Writer_post_eventFunc)(struct DDSHST_Writer *self,
                                struct DDSHST_WriterEvent *event,
                                struct OSAPI_NtpTime *now);

/*ci
 * \brief Definition of \ref DDSHST_WriterI get_instance_state method
 *
 * \details
 * Return the state of an instance if it exists
 *
 * \param[in]  self  The history cache
 * \param[in]  key   The key to retrieve state for
 * \param[out] state The current state of an instance, if found
 *
 * \return DDSHST_RETCODE_NOT_EXISTS if the instance does not exist,
 *         DDSHST_RETCODE_OK if the instance exists
 */
typedef DDSHST_ReturnCode_T
(*DDSHST_Writer_get_instance_stateFunc)(struct DDSHST_Writer *self,
                                        const DDS_InstanceHandle_t *const key,
                                        struct DDSHST_InstanceState *state);

/*ci
 * \brief Definition of \ref DDSHST_WriterI unregister_key method
 *
 * \details
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
typedef DDSHST_ReturnCode_T
(*DDSHST_Writer_unregister_keyFunc)(struct DDSHST_Writer *self,
                                    const DDS_InstanceHandle_t *const key);


/*ci
 * \brief Definition of the DDS Writer History interface
 */
struct DDSHST_WriterI
{
    /*ci
     * \brief Base-class interface
     */
    struct RT_ComponentI _parent;

    /*ci
     * \brief Method to get an entry from the cache, if available
     */
    DDSHST_Writer_get_entryFunc get_entry;

    /*ci
     * \brief Method to return an entry back to the cache if not needed
     */
    DDSHST_Writer_return_entryFunc return_entry;

    /*ci
     * \brief Method to commit an entry to the cache
     */
    DDSHST_Writer_commit_entryFunc commit_entry;

    /*ci
     * \brief Method to request an entry from the cache
     */
    DDSHST_Writer_request_sampleFunc request_sample;

    /*ci
     * \brief Method to ACK or NACK an entry in the cache
     */
    DDSHST_Writer_acknack_sampleFunc acknack_sample;

    /*ci
     * \brief Method to register a key in the cache, if possible
     */
    DDSHST_Writer_register_keyFunc register_key;

    /*ci
     * \brief Method to get the current state of the writer cache
     */
    DDSHST_Writer_get_stateFunc get_state;

    /*ci
     * \brief Method to post an event to the writer cache
     */
    DDSHST_Writer_post_eventFunc post_event;

    /*ci
     * \brief Method to retrieve the instance state
     */
    DDSHST_Writer_get_instance_stateFunc get_instance_state;

    /*ci
     * \brief Method to unregister a key
     */
    DDSHST_Writer_unregister_keyFunc unregister_key;
};

/*ci
 * \brief Wrapper to call of the DDSHST_WriterFactory->create_component
 *
 * \param[in] f_  The writer history factory
 * \param[in] p_  The property
 * \param[in] l_  The listener
 *
 * \return A reference to new writer cache on success, NULL otherwise
 */
#define DDSHST_WriterFactory_create_component(f_,p_,l_) \
    (struct DDSHST_Writer*)((f_)->intf)->create_component(f_,p_,l_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterFactory->delete_component
 *
 * \param[in] f_  The writer history factory
 * \param[in] c_  The cache to delete
 */
#define DDSHST_WriterFactory_delete_component(f_,c_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)(c_))

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->get_entry
 *
 * \param[in] self_       The writer cache to get an entry from
 * \param[in] key_        The key to get an entry for
 * \param[in] kind_       The kind of entry (dispose, unregister, normal etc.)
 * \param[in] assert_key_ If TRUE, assert the key if it does not already exist.
 *                        Otherwise return NULL if the key does not exist.
 * \param[in] ts_         The source time stamp for the sample
 *
 * \return A reference to new entry if there is space, NULL otherwise
 */
#define DDSHST_Writer_get_entry(self_,key_,kind_,assert_key_,ts_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->get_entry(self_,key_,kind_,assert_key_,ts_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->return_entry function
 *
 * \details
 * Return an entry to cache, it is unused
 *
 * \param[in] self_  The history cache
 * \param[in] entry_ A reference to the entry to return
 */
#define DDSHST_Writer_return_entry(self_,entry_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->return_entry(self_,entry_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->commit_entry function
 *
 * \details
 * Commit an entry to the writer cache, updating the state of the cache.
 * The ack_count_ is the expected number of acknowledgments before the sample
 * SN can be marked as ACK'ed
 *
 * \param[in] self_      The writer cache
 * \param[in] entry_     A reference to a previously allocated entry
 * \param[in] sample_    The entries's payload (opaque to the cache)
 * \param[in] sn_        The sequence number of the entry
 * \param[in] ack_count_ The expected number of acknowledgment for this sample
 *                       based on the number of reliable peers
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref
 *         DDSHST_ReturnCode_T on failure
 */
#define DDSHST_Writer_commit_entry(self_,entry_,sample_,sn_,ack_count_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->commit_entry(self_,entry_,\
            sample_,sn_,ack_count_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->request_sample function
 *
 * \details
 * Return a reference to a history sample based on the requested SN.
 * A sample that is requested is automatically considered
 * unacknowledged and will not be removed from the queue unless forced by an
 * ACKNACK. Thus, a sample which was ACK'ed may go back to UNACK'ed state. A
 * request does not change the history.
 * \p
 * The cache can search in all samples or only the historical samples
 * based on the historical_only flag. This function always returns the next
 * available SN if the requested cannot be found. If the requested SN is found
 * the first available is set to the requested SN.
 *
 * \param[in]  self_    The history cache
 * \param[out] sample_  The requested sample if found, NULL otherwise
 * \param[in]  sn_      The requested SN
 * \param[out] sn_ge_   The first available SN
 * \param[in]  h_       Whether only historical samples are relevant
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref DDSHST_ReturnCode_T
 *         on failure
 */
#define DDSHST_Writer_request_sample(self_,sample_,sn_,sn_ge_,h_) \
    ((struct DDSHST_WriterI*)(\
       (self_)->_parent._intf))->request_sample(self_,sample_,sn_,sn_ge_,h_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->acknack_sample function
 *
 * \details
 * When a SN is ACK'ed it tells the queue that as far as the external
 * view is concerned that sample is no longer needed. However, the queue
 * can be configured to keep samples to delivery to late joiners.
 *
 * There are 3 types of sample that can be ACK'ed:
 * - normal sample: This is regular user-data and does not change the key state.
 *                  Normal data samples that are ACK'ed are usually returned to
 *                  the buffer-pool or kept around for history.
 * - unregister sample: An ACK'ed unregister sample means that everybody has
 *                      acknowledged the unregistration from this queue. If
 *                      there are no more samples in the queue then the
 *                      key will be reclaimed. If the key is not empty then
 *                      the key resources are not reclaimed.
 * - dispose sample: An ACK'ed dispose sample means that everybody has
 *                   acknowledged the disposal from this queue. However,
 *                   disposed keys cannot be reclaimed.
 *
 * If NACK is DDS_BOOLEAN true the sample was not delivered to all, but
 * it can be removed as no further attempts to deliver it will be made.
 *
 * \param[in] self_ The writer cache
 * \param[in] sn_   The SN to ACKNACK
 * \param[in] nack_ Whether this is a NACK (DDS_BOOLEAN_TRUE) or ACK
 *                  (DDS_BOOLEAN_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref
 *         DDSHST_ReturnCode_T on failure
 */
#define DDSHST_Writer_acknack_sample(self_,sn_,nack_) \
((struct DDSHST_WriterI*)(\
        (self_)->_parent._intf))->acknack_sample(self_,sn_,nack_)

/*ci
 * \brief Wrapper to call of the DDSHST_WriterI->register_key function
 *
 * \details
 * Add a key to the cache if there is space.
 *
 * \param[in] self_ The history cache
 * \param[in] key_  The key to add to the cache
 * \paran[in] ts_   The timestamp of the operation
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of \ref
 *         DDSHST_ReturnCode_T on failure
 */
#define DDSHST_Writer_register_key(self_,key_,ts_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->register_key(self_,key_,ts_)

/*ci
 * \brief Wrapper to call the \ref DDSHST_WriterI::get_state function
 *
 * \details
 * Return the current state of the writer cache. This information
 * is used to determine which sequence numbers are relevant when
 * the durability is in effect.
 *
 * \param[in] self_  The history cache
 *
 * \return Pointer to the structure containing the state
 */
#define DDSHST_Writer_get_state(self_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->get_state(self_)

/*ci
 * \brief Wrapper to call the \ref DDSHST_WriterI ->post_event function
 *
 * \details
 * Handle possible events posted to the cache by the user. The events
 * handled are deadline tests (the deadline timer is not owned by the cache)
 * and update expected ACKNACKs when a reliable late joiner as added. Note
 * that the cache does not keep track of peers, only how many there are.
 *
 * \param[in] self_  The history cache
 * \param[in] event_ The event
 * \param[in] time_  The time of the event
 */
#define DDSHST_Writer_post_event(self_,event_,time_) \
        ((struct DDSHST_WriterI*)(\
                (self_)->_parent._intf))->post_event(self_,event_,time_)

/*ci
 * \brief Wrapper to call \ref DDSHST_WriterI  ->get_instance_state method
 *
 * \details
 * Return the state of in instance if it exists
 *
 * \param[in]  self_  The history cache
 * \param[in]  key_   The key to retrieve state for
 * \param[out] state_ The current state of an instance, if found
 *
 * \return DDSHST_RETCODE_NOT_EXISTS if the instance does not exists,
 *         DDSHST_RETCODE_OK if the instance exists
 */
#define DDSHST_Writer_get_instance_state(self_,key_,state_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->get_instance_state(self_,key_,state_)

/*ci
 * \brief Wrapper to call \ref DDSHST_WriterI  ->unregister_key method
 *
 * \details
 * Unregister a key from the history. Note that this is not the same as a DDS
 * unregister_instance. Unregistering a key simple deletes it from the
 * writer's cache without any instance updates.
 *
 * \param[in]  self_  The history cache
 * \param[in]  key_   The key to unregister
 *
 * \return DDSHST_RETCODE_OK on success, DDSHST_RETCODE_NOT_EXISTS if the
 *         instance does not exist.
 */
#define DDSHST_Writer_unregister_key(self_,key_) \
    ((struct DDSHST_WriterI*)(\
            (self_)->_parent._intf))->unregister_key(self_,key_)

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_wh_plugin_h */

/*ci @} */
