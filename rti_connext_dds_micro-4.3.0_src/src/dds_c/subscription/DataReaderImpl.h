/*
 * FILE: DataReaderImpl.h - DDS DataReader implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 07feb2013,eh  MICRO-238: removed debug msg causing segfault
 * 06feb2013,eh  MICRO-262: assign RTPS resource limits
 * 08jun2012,tk  Refactored from DataReader.c
 */
/*ce
 * \file
 * \brief DDS DataReader implementation
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef DataReaderImpl_h
#define DataReaderImpl_h


#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif

#include "Entity.h"

#if DDS_FILTERING_ENABLED
#include "dds_c/dds_c_filter_plugin.h"
#endif

/*ci
 * The maximum number of interfaces a datareader can create. Only RTPS and
 * DataReaderInterface is supported.
 */
#define DDS_MAX_INTERFACES_PER_DATAREADER          (2)

/*ci
 * \def DDS_DATAREADER_MAX_DESTINATIONS_PER_WRITER
 * \brief The maximum number of destination a datareader will keep per
 *        matched datawriter
 */
#define DDS_DATAREADER_MAX_DESTINATIONS_PER_WRITER (1)

typedef RTI_BOOL
(*DataReaderLifeCycleListener_on_after_enabled)(
                        DDS_DataReader *const writer,
                        const struct DDS_DataReaderQos *const qos,
                        DDS_Boolean already_enabled);

typedef void
(*DataReaderLifeCycleListener_on_before_deleted)(DDS_DataReader *const dw);

typedef DDS_ReturnCode_t
(*DataReaderLifeCycleListener_on_before_datareader_created)(
                                DDS_Subscriber *const subscriber,
                                struct DDS_BuiltinTopicKey_t *const dr_key,
                                const struct DDS_DataReaderQos *const dr_qos,
                                DDS_Boolean reserved);
/*ci
 * \brief Configuration data for a datareader
 */
struct NDDS_DataReaderConfig
{
    /*ci
     * \brief The database to create tables in
     */
    DB_Database_T db;

    /*ci
     * \brief The timer to create timeouts from
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief run-time registry to look for components in
     */
    RT_Registry_T *registry;

    /*ci
     * \brief Callback to call after the datareader is enabled
     */
    DataReaderLifeCycleListener_on_after_enabled on_after_enabled;

    /*ci
     *\brief Function to retrieved the parent instance handle
     */
    RTIDDS_EntityGetInstanceHandleFunction get_parent_handle;

    /*ci
     * \brief The default unicast locators this datareader should listen on
     *        if none are specified for the datareader
     */
    struct DDS_LocatorSeq *default_unicast;

    /*ci
     * \brief The default multicast locators this datareader should listen on
     *        if none are specified for the datareader
     */
    struct DDS_LocatorSeq *default_multicast;

    /*ci
     * \brief The default discovery unicast locators this datareader
     *        should listen on if none are specified for the datareader
     */
    struct DDS_LocatorSeq *default_meta_unicast;

    /*ci
     * \brief The default discovery multicast locators this datareader
     *        should listen on if none are specified for the datareader
     */
    struct DDS_LocatorSeq *default_meta_multicast;

    /*ci
     * \brief A bind-resolver to find NETIO interface to listen for data on
     */
    NETIO_BindResolver_T *bind_resolver;

    /*ci
     * \brief The transports that are enabled for this datareader
     */
    struct REDA_StringSeq *enabled_transports;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_RouteResolver_T *route_resolver;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_AddressResolver_T *address_resolver;

    /*ci
     * \brief Pointer to a participant_id in case none has yet been assigned
     *        by the participant due to no unicast addresses specified
     */
    DDS_Long *participant_id;

    /*ci
     * \brief Pointer to DDS domain ID of the owning DomainParticipant
     */
    DDS_DomainId_t *domain_id;

    /*ci \brief The external interface created by the parent Participant
     */
    NETIO_Interface_T *ext_rtps_intf;

    /*ci \brief Shared packet pool
     */
    REDA_BufferPool_T packet_pool;

    struct DDS_LocatorSeq *resolved_unicast_seq;

    struct DDS_LocatorSeq *resolved_multicast_seq;

#if DDS_FILTERING_ENABLED
    struct DDS_FilterPlugin *filter_plugin;
#endif
};

#if DDS_FILTERING_ENABLED
#define NDDS_DataReaderConfig_Filter_INITIALIZER \
    ,\
    NULL
#else
#define NDDS_DataReaderConfig_Filter_INITIALIZER
#endif

/*ci
 * \def NDDS_DataReaderConfig_INITIALIZER
 * \brief Constant to initialize \ref NDDS_DataReaderConfig
 */
#define NDDS_DataReaderConfig_INITIALIZER \
{\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL,\
     NULL \
     NDDS_DataReaderConfig_Filter_INITIALIZER \
}

/*ci
 * \brief Implementation of a sample
 */
struct RTI_DataReaderSample
{
    /*ci
     * \brief Inherited from base-class
     */
    DDSHST_ReaderSample_T hst_sample;
};

/*ci
 * \brief Entry in keyhash table to support v1 & v2 keyhashes
 */
struct DDS_DataReaderKeyHashEntry
{
    /*ci
     * \brief The received keyhash from a writer
     */
    DDS_InstanceHandle_t remote_hash;

    /*ci
     * \brief The equivalent local keyhash
     */
    DDS_InstanceHandle_t local_hash;
};

/*ci
 * \brief Implementation of the DDS DataReader entity
 */
struct DDS_DataReaderImpl
{
    /*ci
     * \brief Inherit from DDS_Entity
     */
    struct DDS_EntityImpl as_entity;

    /*----- From create call -----*/

    /*e \dref_DataReaderQos_deadline
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*e \dref_DataReaderQos_liveliness
     */
    struct DDS_LivelinessQosPolicy liveliness;

    /*e \dref_DataReaderQos_history
     */
    struct DDS_HistoryQosPolicy history;

    /*e \dref_DataReaderQos_resource_limits
     */
    struct DDS_ResourceLimitsQosPolicy resource_limits;

    /*e \dref_DataReaderQos_ownership
     */
    struct DDS_OwnershipQosPolicy ownership;

    /*e \dref_DataReaderQos_reliability
     */
    struct DDS_ReliabilityQosPolicy reliability;

    /*e \dref_DataReaderQos_durability
     */
    struct DDS_DurabilityQosPolicy durability;

    /*e \dref_DataReaderQos_destination_order
     */
    struct DDS_DestinationOrderQosPolicy destination_order;

    /* --- Extensions: ---------------------------------------------------- */

    /*i \dref_DataWriterQos_type_support
     */
    struct DDS_TypeSupportQosPolicy type_support;

    /*e \dref_DataReaderQos_protocol
     */
    struct DDS_DataReaderProtocolQosPolicy protocol;

    /*e \dref_DataReaderQos_transport
     */
    struct DDS_TransportQosPolicy transport;

    /*e \dref_DataReaderQos_reader_resource_limits
     */
    struct DDS_DataReaderResourceLimitsQosPolicy reader_resource_limits;

    /*i \dref_DataReaderQos_management
     */
    struct RTI_ManagementQosPolicy management;


    /*e \dref_DataReaderQos_property
     */
    struct DDS_PropertyQosPolicy property;


    /*-----------------------------  4.0  ------------------------------------*/

    /*e \dref_DataReaderQos_user_data
     */
    struct DDS_UserDataQosPolicy *user_data;

    /*i \dref_DataReaderQos_encapsulation
     */
    struct DDS_TransportEncapsulationQosPolicy *encapsulation;

    /*e \dref_DataReaderQos_representation
     */
    struct DDS_DataRepresentationQosPolicy *representation;

    /*i \dref_DataReaderQos_latency_budget
     */
    struct DDS_LatencyBudgetQosPolicy latency_budget;


    char *subscription_name;

    /*-----------------------------  4.0  ------------------------------------*/

    /**************************************************/

    /*ci
     * \brief The current DataReader listener, as specified by the user
     */
    struct DDS_DataReaderListener listener;

    /*ci
     * \brief The currently status' the DataReader is interested in
     *        as specified by the user
     */
    DDS_StatusMask mask;

    /*----- Internal management -----*/
    /*ci
     * \brief Pointer to topic structure subscribed to by the datareader
     */
    DDS_Topic *topic;

    /*ci
     * \brief Pointer to subscriber that created this datareader
     */
    DDS_Subscriber *subscriber;

    /*ci
     * \brief Pointer to type-plugin mananging the Topic type
     */
    struct NDDS_Type_Plugin *type_plugin;

    /*ci
     * \brief The key-type
     */
    NDDS_TypePluginKeyKind key_kind;

    /*---- DDS ReaderState to manage DDS semantics ----*/

    /*ci
     * \brief The deadline event handle from timer
     */
    OSAPI_TimeoutHandle_T deadline_event;

    /*ci
     * \brief The liveliness event handle from timer
     */
    OSAPI_TimeoutHandle_T liveliness_event;

    /*ci
     * \brief Deadline status, updated on deadline events
     */
    struct DDS_RequestedDeadlineMissedStatus req_deadline_missed_status;

    /*ci
     * \brief Incompatible qos status, updated on matching events
     */
    struct DDS_RequestedIncompatibleQosStatus req_incompatible_qos_status;

    /*ci
     * \brief Liveliness changed status, updated on change in liveliness
     */
    struct DDS_LivelinessChangedStatus liveliness_changed_status;

    /*ci
       * \brief Subscription matched status, updated on match/unmatch event
       */
    struct DDS_SubscriptionMatchedStatus subscription_matched_status;

    /*ci
       * \brief Sample lost status, updated when a sample is removed from local
       *        or remote cache without being seen by the user
       */
    struct DDS_SampleLostStatus sample_lost_status;

    /*ci
       * \brief Sample rejected status, updated when a sample is rejected by the
       *        reader history cache due to out of resources
       */
    struct DDS_SampleRejectedStatus sample_rejected_status;

    /*ci
       * \brief Instance replacementstatus, updated when a sample an instance
       *        is replaced if the replacement policy allows it and the reader
       *        cache is able remove one.
       */
    struct DDS_DataReaderInstanceReplacedStatus instance_replaced_status;

    /*ci
     * \brief The last incompatible Qos policy
     */
    DDS_QosPolicyId_t   last_policy_id;

    /*ci
     * \brief Stream to generate keyhashes with
     */
    struct CDR_Stream_t *md5_stream;

    /*ci
     * \brief Stream to deserialize samples with
     */
    struct CDR_Stream_t stream;

    /*ci
     * \brief Pointer to a run-time component implementing in the DDS reader
     *        history cache API
     */
    struct DDSHST_Reader *_rh;

    /*ci
     * \brief Pointer to a run-time component implementing in the NETIO
     *        interface API to receive DDS samples
     */
    NETIO_Interface_T *dr_intf;

    /*ci
     * \brief Pointer to a run-time component implementing in the NETIO
     *        interface API support the RTPS protocol.
     */
    NETIO_Interface_T *rtps_intf;

    /*ci
     * \brief Pool of CDR samples, size by Qos policies, to receive data in
     */
    REDA_BufferPool_T cdr_samples;

    /*ci
     * \brief The maximum size of a sequence when reading samples calculated
     *        from resource limits. Note that UNLIMITED is still limited.
     */
    DDS_Long read_seq_max;

    /*ci
     * \brief Configuration data received during initialization
     */
    struct NDDS_DataReaderConfig *config;

    /*ci
     * \brief Information about the readers locators, typically passed together
     *        with the datareader qos policy to discovery plugins
     */
    struct DDS_DataReaderData reader_data;

    /*ci
     * \brief The unicast locators used by the datareader, derived from the
     *        participant and datareader qos policies
     */
    struct DDS_LocatorSeq *uc_locator_seq;

    /*ci
     * \brief The multicast locators used by the datareader, derived from the
     *        participant and datareader qos policies
     */
    struct DDS_LocatorSeq *mc_locator_seq;

    /*ci
     * \brief The current encapsulation id.
     *
     * \details For batched samples this field is only set on the first sample.
     */
    DDS_EncapsulationId_t current_eid;

    /*ci
     * \brief
     *
     * The effective CDR used by this reader. The effective CDR encapsulation
     * is with all public APIs, for example lookup_instance.
     */
    DDS_EncapsulationId_t cdr_id;

    /*ci
     * \brief Keyhash cache to map between keys for different encapsulation
     *        if needed.
     */
    struct REDA_Indexer *keyhash_local_index;

    /*ci
     * \brief Keyhash cache to map between keys for different encapsulations
     *        if needed.
     */
    struct REDA_Indexer *keyhash_remote_index;

    /*ci
     * \brief Keyhash pool to allocate keyhashes from
     */
    struct REDA_BufferPool *keyhash_pool;

    /*ci
     * \brief Pool of opaque samples, size by Qos policies, to receive data in
     */
    REDA_BufferPool_T opaque_samples;

#if DDS_FILTERING_ENABLED
    /*ci
     * \dref_DataReaderQos_content_filter
     */
    struct DDS_ContentFilterQosPolicy *content_filter_qos;

    /*ci
     * \brief Opaque pointer to the reader's compiled content filter if it has one configured
     */
    struct DDS_ContentFilterCompiledFilter *compiled_filter;
#endif
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderListener_is_consistent(const struct DDS_DataReaderListener *l,
                                     DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_DataReaderImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

extern DDS_UnsignedLong
DDS_DataReader_get_objectid(DDS_DataReader * self);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderImpl_finalize(DDS_DataReader *self);
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderImpl_initialize(struct DDS_DataReaderImpl *datareader,
                       DDS_Subscriber * subscriber,
                       DDS_TopicDescription * topic_description,
                       const struct DDS_DataReaderQos *qos,
                       const struct DDS_DataReaderListener *listener,
                       DDS_StatusMask mask,
                       DDS_UnsignedLong object_id,
                       struct NDDS_DataReaderConfig *config);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReader_is_enabled(DDS_DataReader *self);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_DataReader_get_locator_interface(NETIO_AddressResolver_T *ar,
                                     NETIO_RouteResolver_T *rr,
                                     const struct DDS_Locator *locator,
                                     RT_ComponentFactoryId_T *name);

extern void
DDS_DataReaderImpl_delete_local_keyhash(struct DDS_DataReaderImpl *datareader,
                                        const DDS_InstanceHandle_t *local_key);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_DataReaderImpl_add_remote_keyhash(struct DDS_DataReaderImpl *datareader,
                                      const DDS_InstanceHandle_t *remote_key,
                                      const DDS_InstanceHandle_t *local_key);

extern const struct DDS_DataReaderKeyHashEntry*
DDS_DataReaderImpl_lookup_local_keyhash(struct DDS_DataReaderImpl *datareader,
                                        const DDS_InstanceHandle_t *remote_key);

extern  DDS_Boolean
DDS_DataReader_immutable_is_equal(const DDS_DataReader *left,
                                  const struct DDS_DataReaderQos *right);

extern DDS_ReturnCode_t
DDS_DataReader_get_qos_from(DDS_DataReader *self,
                            struct DDS_DataReaderQos *out);

extern DDS_ReturnCode_t
DDS_DataReader_set_qos_from(
        DDS_DataReader *out,
        const struct DDS_DataReaderQos *in,
        DDS_DomainParticipant *participant);

extern DDS_ReturnCode_t
DDS_DataReader_finalize_managed(
    DDS_DataReader *self,
    DDS_DomainParticipant *participant);

#endif

/*ci @} */
