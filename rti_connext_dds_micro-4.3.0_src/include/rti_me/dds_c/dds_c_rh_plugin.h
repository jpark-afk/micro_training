/*
 * FILE: dds_c_rh_plugin.h - DDS reader history interface
 *
 * Copyright (c) 2011-2026., Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 08oct2014,tk  MICRO-919 Added SampleLostReason to SampleLost status
 * 16aug2011,tk  Written
 */
/*ce
 * \file
 * \brief DDS reader history interface
 */
/*ci \defgroup DDSHistoryModule DDS History module
 *   \ingroup DDSCModule
 */
/*ci \defgroup DDSReaderHistoryModule DDS Reader History module
 *   \ingroup DDSHistoryModule
 */
/*ci \addtogroup DDSReaderHistoryModule
 * @{
 */
#ifndef dds_c_rh_plugin_h
#define dds_c_rh_plugin_h


#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif
#ifndef netio_sample_access_h
#include "netio/netio_sample_access.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ce
 * \brief Default reader history plug-in name
 */
DDSCDllVariable extern const char* const DDSHST_READER_DEFAULT_HISTORY_NAME;

/*ci
 * \brief Enumeration of supported reader history events
 */
typedef enum
{
    /*ci
     * \brief Liveliness was lost on a remote writer
     */
    DDSHST_READEREVENT_KIND_LIVELINESS_LOST = 1,

    /*ci
     * \brief Liveliness was detected on a remote writer
     */
    DDSHST_READEREVENT_KIND_LIVELINESS_DETECTED,

    /*ci
     * \brief A remote writer was deleted
     */
    DDSHST_READEREVENT_KIND_REMOTE_WRITER_DELETED,

    /*ci
     * \brief The deadline timer expired, check deadlines
     */
    DDSHST_READEREVENT_KIND_DEADLINE_EXPIRED
} DDSHST_ReaderEventKind_T;

/*ci
 * \brief A reader history event that can be posted to the cache
 */
struct DDSHST_ReaderEvent
{
    /*ci
     * \brief The event type
     */
    DDSHST_ReaderEventKind_T kind;

    /*ci
     * \brief event data based on the kind
     */
    union
    {
        /*ci
         * \brief Event data associated with liveliness loss/detection
         */
        struct
        {
            /*ci
             * \brief Remote writer key liveliness changed on
             */
            DDS_InstanceHandle_t rw_guid;

            /*ci
             * \brief The key for the instance that changed liveliness
             */
            DDS_InstanceHandle_t instance;
        } liveliness;

        /*ci
         * \brief Event data associated with remote writer deletion
         */
        struct
        {
            /*ci
             * \brief The GUID of the deleteted remote writer
             */
            DDS_InstanceHandle_t rw_guid;
        } rw_deleted;
    } data;
};

/*ci
 * \brief Base-class for reader history cache
 */
struct DDSHST_Reader
{
    /*ci
     * \brief Inherit from base-class
     */
    struct RT_Component _parent;
};

typedef struct DDSHST_Reader DDSHST_Reader_T;

struct DDSHST_ReaderSampleEntry;

/*ci
 * \brief Implementation independent reference to a sample in the
 *        reader history cache
 */
typedef struct DDSHST_ReaderSampleEntry* DDSHST_ReaderSampleEntryRef_T;

struct DDSHST_ReaderKeyEntry;

/*ci
 * \brief Implementation independent reference to a key in the
 *        reader history cache
 */
typedef struct DDSHST_ReaderKeyEntry* DDSHST_ReaderKeyEntryRef_T;

/*ci
 * \brief Implementation independent reader cache sample
 */
typedef struct DDSHST_ReaderSample
{

    struct DDS_TypePluginSampleHolder *holder;


    /*ci
     * \brief Pointer meta information about the sample, memory owner by
     *        history implementation
     */
    struct DDS_SampleInfo *_info;

    /*ci
     * \brief Status information related to the sample, such as disposed
     */
    DDS_UnsignedLong status_info;

    /*ci
     * \brief Reference to the corresponding entry in the implementation specific
     *        cache
     */
    DDSHST_ReaderSampleEntryRef_T _ref;

    /*ci
     * \brief Interface to allow access of sample managed outside the history
     */
    NETIO_SampleI *sample_access_intf;
} DDSHST_ReaderSample_T;

#define T DDSHST_ReaderSample_T
#define TSeq DDSHST_ReaderSample_TSeq
#include <reda/reda_sequence_decl.h>

/*ci
 * \brief Base-class properties for the DDS reader history cache
 */
struct DDSHST_ReaderProperty
{
    /*ci
     * \brief Inherit from a component
     */
    struct RT_ComponentProperty _parent;

    DDS_DataReader *datareader;

    struct DDS_DeadlineQosPolicy deadline;

    struct DDS_LivelinessQosPolicy liveliness;

    struct DDS_DestinationOrderQosPolicy destination_order;

    struct DDS_HistoryQosPolicy history;

    struct DDS_DurabilityQosPolicy durability;

    struct DDS_ReliabilityQosPolicy reliability;

    struct DDS_ResourceLimitsQosPolicy resource_limits;

    struct DDS_OwnershipQosPolicy ownership;

    struct DDS_DataReaderResourceLimitsQosPolicy reader_resource_limits;
};

/*ci
 * \def DDSHST_ReaderProperty_INITIALIZER
 * \brief Constant to initialize \ref DDSHST_ReaderProperty
 */
#define DDSHST_ReaderProperty_INITIALIZER \
{\
    RT_ComponentProperty_INITIALIZER,\
    NULL,\
    DDS_DEADLINE_QOS_POLICY_DEFAULT,\
    DDS_LIVELINESS_QOS_POLICY_DEFAULT,\
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT,\
    DDS_HISTORY_QOS_POLICY_DEFAULT,\
    DDS_DURABILITY_QOS_POLICY_DEFAULT,\
    DDS_RELIABILITY_QOS_POLICY_DEFAULT,\
    DDS_RESOURCE_LIMITS_QOS_POLICY_DEFAULT,\
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT,\
    DDS_DATAREADERRESOURCE_LIMITS_QOS_POLICY_DEFAULT\
}

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_sample_removed callback
 *
 * \details
 * When the history cache removes a sample it calls this listener if it is
 * installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key the sample belonged too
 * \param[in] sample        The sample being removed
 */
typedef void
(*DDSHST_ReaderListener_OnSampleRemovedFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *key,
        DDSHST_ReaderSample_T *sample);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_key_removed callback
 *
 * \details
 * When the history cache removes a key it calls this listener if it is
 * installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key being removed
 */
typedef void
(*DDSHST_ReaderListener_OnKeyRemovedFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *key);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_deadline_missed callback
 *
 * \details
 * When an instance is not being updated according to the deadline the cache
 * calls this listener if it is installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key that missed its deadline
 */
typedef void
(*DDSHST_ReaderListener_OnDeadlineMissedFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *key);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_data_available callback
 *
 * \details
 * When new data is available in the history cache this listener is called
 * if it is installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key data is available in
 * \param[in] sample        The new data
 */
typedef void
(*DDSHST_ReaderListener_OnDataAvailableFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *key,
        DDSHST_ReaderSample_T *sample);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_sample_rejected callback
 *
 * \details
 * If a sample is rejected from the history cache this listener is called
 * if it is installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] key           The key that was rejected
 * \param[in] reason        The reason the sample was rejected
 */
typedef void
(*DDSHST_ReaderListener_OnSampleRejectedFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *key,
        DDS_SampleRejectedStatusKind reason);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_sample_lost callback
 *
 * \details
 * When the history cache determines a sample is permanently lost
 * the history cache calls this listener if it is installed.
 *
 * \param[in] rh            The history cache
 * \param[in] listener_data Opaque pointer to listener data
 * \param[in] sample_info   The sample info structure with additional information
 *                          about the last sample
 *                          (instance_handle, publication_handle)
 * \param[in] reason        The reason the sample was lost
 */
typedef void
(*DDSHST_ReaderListener_OnSampleLostFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        struct DDS_SampleInfo *sample_info,
        DDS_SampleLostStatusKind reason);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_instance_replaced callback
 *
 * \details
 * When the history cache replaces an instance in the history cache it calls
 * this listener if it is installed.
 *
 * \param[in] rh                  The history cache
 * \param[in] listener_data       Opaque pointer to listener data
 * \param[in] replaced_key        The key that was replaced
 * \param[in] replaced_by_key     The new key
 * \param[in] publisher           The publisher publishing the replacement key
 * \param[in] min_removed_samples Minimum number of samples removed due to a
 *                                key being replaced.
 */
typedef void
(*DDSHST_ReaderListener_OnInstanceReplacedFunc)(
        struct DDSHST_Reader *rh,
        void *listener_data,
        DDS_InstanceHandle_t *replaced_key,
        DDS_InstanceHandle_t *replaced_by_key,
        DDS_InstanceHandle_t *publisher,
        DDS_Long min_removed_samples);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderListener on_sample_commited callback
 *
 * \details
 * Normally when add_and_commit and commit is called the samples are added to
 * the commit queue and are readily available for reuse by being reclaimed
 * (in the case of KEEP_LAST).
 *
 * However, in cases where resource limits are exceeded, such as
 * max_samples_per_instance, samples may be committed at a later time
 * when resources are freed up. This is done asynchronously to the receive thread
 * This listener notifies that samples have been committed and are available to be
 * reclaimed. It is up to the listener to pass the information downstream to
 * RTPS (or other layer).
 *
 *
 * \param[in] rh                  The history cache
 * \param[in] listener_data       Opaque pointer to listener data
 */
typedef void
(*DDSHST_ReaderListener_OnSampleCommittedFunc)(struct DDSHST_Reader *rh,
                                               void *listener_data,
                                               DDS_InstanceHandle_t *publisher,
                                               DDS_Long committed_samples);

/*ci
 * \brief Base-class for  DDS Reader history listener
 */
struct DDSHST_ReaderListener
{
    /*ci
     * \brief Inherit from parent class
     */
    struct RT_ComponentListener _parent;

    /*ci
     * \brief Called when the cache removed a sample
     */
    DDSHST_ReaderListener_OnSampleRemovedFunc on_sample_removed;

    /*ci
     * \brief Called when the cache removed a key
     */
    DDSHST_ReaderListener_OnKeyRemovedFunc on_key_removed;

    /*ci
     * \brief Called when the cache when new data is available in the cache
     */
    DDSHST_ReaderListener_OnDataAvailableFunc on_data_available;

    /*ci
     * \brief Called when the cache detected deadline missed on an instance
     */
    DDSHST_ReaderListener_OnDeadlineMissedFunc on_deadline_missed;

    /*ci
     * \brief Called when the cache rejected a sample
     */
    DDSHST_ReaderListener_OnSampleRejectedFunc on_sample_rejected;

    /*ci
     * \brief Called when the cache detected loss of samples
     */
    DDSHST_ReaderListener_OnSampleLostFunc on_sample_lost;

    /*ci
     * \brief Called when the cache forcefully replaced an instance
     */
    DDSHST_ReaderListener_OnInstanceReplacedFunc on_instance_replaced;

    /*ci
     * \brief Called when new samples are added to the committed cache.
     */
    DDSHST_ReaderListener_OnSampleCommittedFunc on_sample_committed;

    /*ci
     * \brief Opaque listener data passed back to listener functions
     */
    void *listener_data;
};

/*ci
 * \brief Constant to initialize \ref DDSHST_ReaderListener
 */
#define DDSHST_ReaderListener_INITIALIZE {\
        RT_ComponentListener_INITIALIZER,\
        NULL,\
        NULL,\
        NULL,\
        NULL,\
        NULL,\
        NULL,\
        NULL,\
        NULL,\
        NULL\
}

/* ------------------------------ Life-cycle -------------------------------- */
/*ci
 * \brief Definition of \ref RT_ComponentFactory delete_component method
 *
 * \param[in] self  The reader history instance to delete
 *
 * \sa \ref RT_ComponentFactoryI
 */
typedef void
(*DDSHST_Reader_deleteFunc)(struct DDSHST_Reader *self);

/*ci
 * \brief Definition of \ref RT_ComponentFactory create_component method
 *
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A reference to a new reader cache on success, NULL otherwise
 *
 * \sa \ref RT_ComponentFactoryI
 */
FUNCTION_MUST_TYPEDEF(
struct DDSHST_Reader*
(*DDSHST_Reader_createFunc)(const struct DDSHST_ReaderProperty *const property,
                            const struct DDSHST_ReaderListener *const listener)
)

/* ------------------------------ Methods ----------------------------------- */
/*ci
 * \brief Definition of the \ref DDSHST_ReaderI reserve_entry method
 *
 * \param[in]  self              The history cache
 * \param[in]  rw                The remote writer the reservation is for
 * \param[in]  key               The key the reservation is for
 * \param[out] sample_info       A pointer to a sample info structure for this
 *                               sample, memory owned by the queue
 * \param[in]  strength          The current strength for the remote writer
 *                               the reservation for
 * \param[in]  sample_sn         The SN for this entry
 * \param[in]  sample_virtual_sn The SN for this entry
 * \param[out] reject_reason     If the reservation failed, this is the reason
 *
 * \return A reference to a reservation, NULL otherwise
 *
 * \sa \ref DDSHST_Reader_reserve_entry, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReaderSampleEntryRef_T
(*DDSHST_Reader_reserve_entryFunc)(
        struct DDSHST_Reader *self,
        DDS_InstanceHandle_t *rw,
        DDS_InstanceHandle_t *key,
        struct DDS_SampleInfo **sample_info,
        DDS_Long strength,
        struct REDA_SequenceNumber *sample_sn,
        struct REDA_SequenceNumber *sample_virtual_sn,
        DDS_SampleRejectedStatusKind *reject_reason)
)

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI return_entry method
 *
 * \param[in] self   self
 * \param[in] entry  reservation to cancel
 *
 * \sa \ref DDSHST_Reader_return_entry, \ref DDSHST_ReaderI
 */
typedef void
(*DDSHST_Reader_return_entryFunc)(struct DDSHST_Reader *self,
                                  DDSHST_ReaderSampleEntryRef_T entry);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI commit method
 *
 * \param[in] self             The history cache
 * \param[in] rw               The remote writer to commit samples for
 * \param[in] sn               The first non committable sequence number
 * \param[in] strength         The strength of the writer
 * \param[in] notify_on_commit  Whether to post event upon sample commit
 * \param[out] last_virtual_sn The virtual SN of the last committed sample
 * \param[out] missing_vsn_    The number of missing vsn in a range
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *       \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref DDSHST_Reader_commit, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Reader_commitFunc)(struct DDSHST_Reader *self,
                            DDS_InstanceHandle_t *rw,
                            struct REDA_SequenceNumber *sn,
                            DDS_Long strength,
                            DDS_Boolean notify_on_commit,
                            struct REDA_SequenceNumber *last_virtual_sn,
                            DDS_Long *missing_vsn)
)

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI add_and_commit method
 *
 * \param[in] rh               The history cache
 * \param[in] entry            The reserved entry to commit
 * \param[in] sample           The sample associated with the reservation
 * \param[in] commit_sn        The first non-committable sequence number
 * \param[in] notify_on_commit  Whether to post event upon sample commit
 * \param[out] last_virtual_sn The virtual SN of the last committed sample
 * \param[out] missing_vsn_    The number of missing vsn in a range
 *
 * \sa \ref DDSHST_Reader_add_and_commit, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReturnCode_T
(*DDSHST_Reader_add_and_commitFunc)(struct DDSHST_Reader *rh,
                                    DDSHST_ReaderSampleEntryRef_T entry,
                                    DDSHST_ReaderSample_T *sample,
                                    struct REDA_SequenceNumber *commit_sn,
                                    DDS_Boolean notify_on_commit,
                                    struct REDA_SequenceNumber *last_virtual_sn,
                                    DDS_Long *missing_vsn)
)

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI post_event method
 *
 * \param[in] self  The history cache
 * \param[in] event The event that occurred
 * \param[in] now   The time the event occurred
 *
 * \sa \ref DDSHST_Reader_post_event, \ref DDSHST_ReaderI
 */
typedef void
(*DDSHST_Reader_post_eventFunc)(struct DDSHST_Reader *self,
                                struct DDSHST_ReaderEvent *event,
                                struct OSAPI_SystemTime *now);

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI read_or_take method
 *
 * \param[in]    self             The history cache to read from
 * \param[inout] sample_ptr_array A sequence of pointers to samples
 * \param[inout] info_array       A sequence of pointers to sample info
 * \param[in]    sample_count     The actual number of samples returned
 * \param[in]    a_handle         The instance to return (NIL is any instance)
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
 * \sa \ref DDSHST_Reader_read_or_take, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*DDSHST_Reader_read_or_takeFunc)(struct DDSHST_Reader *self,
                                  void ***sample_ptr_array,
                                  struct DDS_SampleInfo ***info_array,
                                  DDS_Long *sample_count,
                                  const DDS_InstanceHandle_t *a_handle,
                                  DDS_Long max_samples,
                                  DDS_SampleStateMask sample_states,
                                  DDS_ViewStateMask view_states,
                                  DDS_InstanceStateMask instance_states,
                                  DDS_Boolean take)
)

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI finish_read_or_take method
 *
 * \param[in] self             The history cache to return the sequences to
 * \param[in] sample_ptr_array A sequence of pointers to samples to return
 * \param[in] info_array       A sequence of pointers to sample info to return
 * \param[in] sample_count     The number of samples to return in each sequence
 * \param[in] taken            Whether the samples where taken or not
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref DDSHST_Reader_finish_read_or_take, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*DDSHST_Reader_finish_read_or_takeFunc)(struct DDSHST_Reader *self,
                                         void ***sample_ptr_array,
                                         struct DDS_SampleInfo ***info_array,
                                         DDS_Long sample_count,
                                         DDS_Boolean taken)
)

/*ci
 * \brief Definition of the \ref DDSHST_ReaderI lookup_key method
 *
 * \param[in] self The history cache
 * \param[in] key  The key entry to search for
 *
 * \return A key entry on success, NULL if the entry was not found
 *
 * \sa \ref DDSHST_Reader_lookup_key, \ref DDSHST_ReaderI
 */
FUNCTION_MUST_TYPEDEF(
DDSHST_ReaderKeyEntryRef_T
(*DDSHST_Reader_lookup_keyFunc)(struct DDSHST_Reader* self,
                                const DDS_InstanceHandle_t *const key)
)

/*ci
 * \brief Definition of the DDSHST_Reader history interface
 */
struct DDSHST_ReaderI
{
    /*ci
     * \brief The base-class listener
     */
    struct RT_ComponentI _parent;

    /*ci
     * \brief Method to return an entry to the cache
     */
    DDSHST_Reader_return_entryFunc return_entry;

    /*ci
     * \brief Method to read or take an entry from the cache
     */
    DDSHST_Reader_read_or_takeFunc read_or_take;

    /*ci
     * \brief Method to return a loan to the cache
     */
    DDSHST_Reader_finish_read_or_takeFunc finish_read_or_take;

    /*ci
     * \brief Method to post an event to the cache
     */
    DDSHST_Reader_post_eventFunc post_event;

    /*ci
     * \brief Method to lookup the key from the cache
     */
    DDSHST_Reader_lookup_keyFunc lookup_key;

    /*ci
     * \brief Method to reserve a spot in the cache
     */
    DDSHST_Reader_reserve_entryFunc reserve_entry;

    /*ci
     * \brief Method to add a reservation to the cache and commit the
     *        entries
     */
    DDSHST_Reader_add_and_commitFunc add_and_commit;

    /*ci
     * \brief Method to commit existing entries in the cac (if possible)
     */
    DDSHST_Reader_commitFunc commit;
};

/*ci
 * \brief Create a new instance of the reader history cache
 */
#define DDSHST_ReaderFactory_create_component(f_,p_,l_) \
(struct DDSHST_Reader*)((f_)->intf)->create_component(f_,p_,l_)

/*ci
 * \brief Delete an instance of the reader history cache
 */
#define DDSHST_ReaderFactory_delete_component(f_,c_) \
        ((f_)->intf)->delete_component(f_,(RT_Component_T*)(c_))

/*ci
 * \brief Implementation of the DDSHST_Reader_reserve_entry function
 *
 * \details
 * This function tries to reserve an entry in the cache based on the current
 * resource limits. A reserved entry can either be committed to the queue
 * or returned unused (cancelled). If an entry is returned (not called decided
 * to not use it), all operations that took place in the reservation are
 * reversed. That is, any resource allocated are freed.
 *
 * \param[in]  self_              The history cache
 * \param[in]  rw_                The remote writer the reservation is for
 * \param[in]  key_               The key the reservation is for
 * \param[out] si_                A pointer to a sample info structure for this
 *                                sample, memory owned by the queue
 * \param[in]  strength_          The current strength for the remote writer
 *                                the reservation for
 * \param[in]  sample_sn_         The SN for this entry
 * \param[in]  sample_virtual_sn_ The virtual SN for this entry
 * \param[out] reject_reason_     If the reservation failed, this is the reason
 *
 * \return A reference to a reservation, NULL otherwise
 */
#define DDSHST_Reader_reserve_entry(self_,rw_,key_,si_,strength_,\
                                    sample_sn_,sample_virtual_sn_,\
                                    reject_reason_) \
        ((struct DDSHST_ReaderI*)((self_)->_parent._intf))->reserve_entry(\
                                  self_,rw_,key_,si_,strength_,\
                                  sample_sn_,sample_virtual_sn_,reject_reason_)
/*ci
 * \brief Return/Cancel a queue reservation
 *
 * \details
 *
 * Return a previous reservation. Returning a reservation has no effect
 * on the queue, and any resources allocated are returned.
 *
 * \param[in] self_  self
 * \param[in] entry_ reservation to cancel
 *
 * \sa \ref RHSM_History_reserve_entry
 */
#define DDSHST_Reader_return_entry(self_,entry_) \
        ((struct DDSHST_ReaderI*)((self_)->_parent._intf))->return_entry(\
                self_,entry_)

/*ci
 * \brief Add a reserved sample to the queue and commit one or more
 *
 * \details
 * When a reservation is made the entry does not exist in the cache, only
 * a sample to fill in exists. If the holder of the reservation decides to
 * keep the sample it must be added. When a new sample is added it may also
 * result in 1 or more samples being committed (for example if the newly
 * added sample completes a sequence of samples).
 *
 * \param[in] self_             The history cache
 * \param[in] entry_            The reserved entry to commit
 * \param[in] sample_           The sample associated with the reservation
 * \param[in] sn_               The first non-committable sequence number
 * \param[in] noc_              Whether to post event for committed samples
 * \param[out] last_virtual_sn_ The virtual SN of the last committed sample
 * \param[out] missing_vsn_     The number of missing vsn in a range
 *
 * \sa \ref RHSM_History_reserve_entry
 */
#define DDSHST_Reader_add_and_commit(self_,entry_,sample_,sn_,noc_,\
                                     last_virtual_sn_,missing_vsn_) \
        ((struct DDSHST_ReaderI*)((self_)->_parent._intf))->add_and_commit(\
                self_,entry_,sample_,sn_,noc_,last_virtual_sn_,missing_vsn_)

/*ci
 * \brief Commit all cached samples up to a specific SN for a remote writer
 *
 * \details
 * The history cache may store samples received out of order to improve
 * performance for reliable communication. If samples are received out of
 * order there may be a backlog of samples until a sample is received that
 * completes a sequence of samples. This function adds all these samples.
 *
 * \param[in] self_             The history cache
 * \param[in] rw_               The remote writer to commit samples for
 * \param[in] sn_               The first non commitable sequence number
 * \param[in] s_                The strength of the writer
 * \param[in] noc_              Whether to post event for committed samples
 * \param[out] last_virtual_sn_ The virtual SN of the last committed sample
 * \param[out] missing_vsn_     The number of missing vsn in a range
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *       \ref DDSHST_ReturnCode_T on failure
 */
#define DDSHST_Reader_commit(self_,rw_,sn_,s_,noc_,last_virtual_sn_,missing_vsn_) \
        ((struct DDSHST_ReaderI*)((self_)->_parent._intf))->commit(\
                self_,rw_,sn_,s_,noc_,last_virtual_sn_,missing_vsn_)

/*
 * \brief Read or take 1 or more samples matching a particular state from the
 *        history cache
 *
 * \details
 * Return 0 or more samples and sample info. The sample_ptr_array and
 * info_array must be pre-allocated to hold at least max_samples
 * pointers. Note that the history _always_ operates on pointer to samples,
 * it has no knowledge about types.
 *
 * \param[in]    self_            The history cache to read from
 * \param[inout] sample_arr_      A sequence of pointers to samples
 * \param[inout] info_arr_        A sequence of pointers to sample info
 * \param[in]    count_           The actual number of samples returned
 * \param[in]    handle_          The instance to return (NIL is any instance)
 * \param[in]    max_samples_     The max number of samples to return (including
 *                                the meta sample)
 * \param[in]    sample_states_   The requested sample state
 * \param[in]    view_states_     The requested view state
 * \param[in]    instance_states_ The requested instance state
 * \param[in]    take_             Whether this removes (RTI_TRUE) or leaves
 *                                 samples in the cache (RTI_FALSE)
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 */
#define DDSHST_Reader_read_or_take(self_,\
sample_arr_,info_arr_,count_,handle_,max_samples_,sample_states_,view_states_,\
        instance_states_,take_) \
    ((struct DDSHST_ReaderI*)(\
            (self_)->_parent._intf))->read_or_take(self_,\
            sample_arr_,info_arr_,count_,handle_,max_samples_,\
            sample_states_,view_states_,instance_states_,take_)

/*ci
 * \brief Return a loan after calling \ref RHSM_History_read_or_take
 *
 * \param[in] self_       The history cache to return the sequences to
 * \param[in] sample_seq_ A sequence of pointers to samples to return
 * \param[in] info_seq_   A sequence of pointers to sample info to return
 * \param[in] count_      The number of samples to return (same count for each
 *                        sequence)
 * \param[in] take_       Whether the samples where taken or not
 *
 * \return DDSHST_RETCODE_SUCCESS on success, one of the standard
 *         \ref DDSHST_ReturnCode_T on failure
 *
 * \sa \ref RHSM_History_read_or_take
 */
#define DDSHST_Reader_finish_read_or_take(self_,sample_seq_,info_seq_,count_,take_) \
        ((struct DDSHST_ReaderI*)(\
                (self_)->_parent._intf))->finish_read_or_take(self_,\
                        sample_seq_,info_seq_,count_,take_)

/*ci
 * \brief Post an external event to the history cache
 *
 * \details
 * This function is the entry point to signal the queue for various
 * types of external events.
 *
 * \param[in] self_  The history cache
 * \param[in] event_ The event that occurred
 * \param[in] time_  The time the event occurred
 */
#define DDSHST_Reader_post_event(self_,event_,time_) \
        ((struct DDSHST_ReaderI*)(\
                (self_)->_parent._intf))->post_event(self_,event_,time_)

/*ci
 * \brief Find and return a pointer to a key entry based on the instance handle
 *
 * \param[in] self_ The history cache
 * \param[in] key_  The key entry to search for
 *
 * \return A key entry on success, NULL if the entry was not found
 */
#define DDSHST_Reader_lookup_key(self_,key_) \
        ((struct DDSHST_ReaderI*)(\
                (self_)->_parent._intf))->lookup_key(self_,key_)

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_rh_plugin_h */

/*ci @} */
