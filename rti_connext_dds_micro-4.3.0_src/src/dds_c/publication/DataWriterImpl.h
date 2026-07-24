/*
 * FILE: DataWriterImpl.h - DataWriter implementation
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 06jun2012,tk Written
 */
/*ce
 * \file
 * \brief DataWriter implementation
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef DataWriterImpl_h
#define DataWriterImpl_h

#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif

#include "Entity.h"

#if DDS_FILTERING_ENABLED
#include "dds_c/dds_c_content_filter.h"
#endif

/*ci
 * The maximum number of interfaces a datawriter can create. Only
 * RTPS and DataWriterInterface is supported
 */
#define DDS_MAX_INTERFACES_PER_DATAWRITER         (2)

/*ci
 * \def DDS_DATAWRITER_MAX_DESTINATION_PER_READER
 * \brief The maximum number of destination a datawriter will keep per
 *        matched datareader
 */
#define DDS_DATAWRITER_MAX_DESTINATION_PER_READER (1)

/*ci
 * \def DDS_INFINITE_MAX_SAMPLES
 * \brief "Infinite" max_samples when calculating heartbeats per max_samples
 *  and datawriter->qos.resource_limits.max_samples is DDS_UNLIMITED_LENGTH.
 */
#define DDS_INFINITE_MAX_SAMPLES (100000000)

/* The qos parameter has been deprecated and is no longer used. The API
 * has not been changed.
 */
typedef RTI_BOOL
(*DataWriterLifeCycleListener_on_after_enabled)(
                        DDS_DataWriter *const writer,
                        const struct DDS_DataWriterQos *const qos);

typedef void
(*DataWriterLifeCycleListener_on_before_deleted)(DDS_DataWriter *const dw);

typedef void
(*DataWriterLifeCycleListener_on_after_deleted)(DDS_DataWriter *const dw);

typedef DDS_ReturnCode_t
(*DataWriterLifeCycleListener_on_before_datawriter_created)(
                                DDS_Publisher *const publisher,
                                struct DDS_BuiltinTopicKey_t *const dw_key,
                                const struct DDS_DataWriterQos *const dw_qos,
                                DDS_Boolean reserved);

/*ci
 * \brief Configuration data for a datawriter
 */
struct NDDS_DataWriterConfig
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
     * \brief A bind-resolver to find NETIO interfaces to listen to data on
     */
    NETIO_BindResolver_T *bind_resolver;

    /*ci
     * \brief Callback to call after the datawriter is enabled
     */
    DataWriterLifeCycleListener_on_after_enabled on_after_enabled;

    /*ci
     *\brief Function to retrieved the parent instance handle
     */
    RTIDDS_EntityGetInstanceHandleFunction get_parent_handle;

    /*ci
     * \brief The default unicast locators this datawriter should listen on
     *        if none are specified for the datawriter
     */
    struct DDS_LocatorSeq *default_unicast;

    /*ci
     * \brief The default multicast locators this datawriter should listen on
     *        if none are specified for the datawriter
     */
    struct DDS_LocatorSeq *default_multicast;

    /*ci
     * \brief The default discovery unicast locators this datawriter
     *        should listen on if none are specified for the datawriter
     */
    struct DDS_LocatorSeq *default_meta_unicast;

    /*ci
     * \brief The default discovery multicast locators this datawriter
     *        should listen on if none are specified for the datawriter
     */
    struct DDS_LocatorSeq *default_meta_multicast;

    /*ci
     * \brief The transports that are enabled for this datawriter
     */
    struct REDA_StringSeq *enabled_transports;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_RouteResolver_T *route_resolver;

    NETIO_AddressResolver_T *addr_resolver;

    /*ci
     * \brief Pointer to a participant_id in case none has yet been assigned
     *        by the participant due to no unicast addresses specified
     */
    DDS_Long *participant_id;

    /*ci
     * \brief Pointer to DDS domain ID of owning DomainParticipant
     */
    DDS_DomainId_t *domain_id;

    /*ci \brief The parent participant's DDS_PropertyQosPolicy
     *
     * This property is propagated because some properties may have
     * a value on the participant that can be overridden at a lower
     * level.
     */
    struct DDS_PropertyQosPolicy *participant_property_qos_policy;

    /*ci \brief The external interface created by the parent Participant
     */
    NETIO_Interface_T *ext_rtps_intf;

    /*ci \brief Shared packet pool
     */
    REDA_BufferPool_T packet_pool;

    /*ci \brief The resolved unicast locators for this datawriter. This is used to
     *        keep track of the resolved unicast locators for this datawriter so
     *        they can be used for discovery and to add routes to matched
     *        datareaders.
     */
    struct DDS_LocatorSeq *resolved_unicast_seq;

    /*ci \brief The resolved multicast locators for this datawriter. This is used to
     *        keep track of the resolved multicast locators for this datawriter so
     *        they can be used for discovery and to add routes to matched
     *        datareaders.
     */
    struct DDS_LocatorSeq *resolved_multicast_seq;

#if DDS_FLOW_CONTROLLER_ENABLED
    /*ci
     * \brief Callback to acquire a flow controller for this datawriter from the participant
     */
    DDS_DomainParticipant_acquire_flowcontroller_T acquire_flowcontroller;

    /*ci
     * \brief Callback to release the flow controller from being in use by this datawriter
     */
    DDS_DomainParticipant_release_flowcontroller_T release_flowcontroller;
#endif
#if DDS_FILTERING_ENABLED
    struct DDS_FilterPlugin *filter_plugin;
#endif
};

#if DDS_FLOW_CONTROLLER_ENABLED
#define DDS_DataWriterConfig_FlowController_INITIALIZER \
    ,\
    NULL, \
    NULL
#else
#define DDS_DataWriterConfig_FlowController_INITIALIZER
#endif

#if DDS_FILTERING_ENABLED
#define NDDS_DataWriterConfig_Filter_INITIALIZER \
    ,\
    NULL
#else
#define NDDS_DataWriterConfig_Filter_INITIALIZER
#endif

/*ci
 * \def NDDS_DataWriterConfig_INITIALIZER
 * \brief Constant to initialize \ref NDDS_DataWriterConfig
 */
#define NDDS_DataWriterConfig_INITIALIZER \
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
     NULL, \
     NULL, \
     NULL, \
     NULL, \
     NULL, \
     NULL\
     DDS_DataWriterConfig_FlowController_INITIALIZER\
     NDDS_DataWriterConfig_Filter_INITIALIZER \
}

/*ci
 * \brief The datawriter local state
 */
typedef enum
{
    /*ci
     * \brief The datawriter is not alive, liveliness lost
     */
    WRITERSTATE_NOT_ALIVE,

    /*ci
     * \brief The datawriter interface is alive, samples are sent or liveliness
     *        is asserted timely
     */
    WRITERSTATE_ALIVE
} WriterState_t;

/*ci
 * \brief Implementation of the DDS DataWriter entity
 */
struct DDS_DataWriterImpl
{
    /*ci
     * \brief Inherit from DDS_Entity
     */
    struct DDS_EntityImpl as_entity;

    /*----- Support for DDS API -----*/
    /*ci \dref_DataWriterQos_deadline
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*ci \dref_DataWriterQos_liveliness
     */
    struct DDS_LivelinessQosPolicy liveliness;

    /*ci \dref_DataWriterQos_history
     */
    struct DDS_HistoryQosPolicy history;

    /*ci \dref_DataWriterQos_resource_limits
     */
    struct DDS_ResourceLimitsQosPolicy resource_limits;

    /*ci \dref_DataWriterQos_ownership
     */
    struct DDS_OwnershipQosPolicy ownership;

    /*ci \dref_DataWriterQos_ownership_strength
     */
    struct DDS_OwnershipStrengthQosPolicy ownership_strength;

    /*ci \dref_DataWriterQos_reliability
     */
    struct DDS_ReliabilityQosPolicy reliability;

    /*ci \dref_DataWriterQos_durability
     */
    struct DDS_DurabilityQosPolicy durability;

    /*ci \dref_DataWriterQos_destination_order
     */
    struct DDS_DestinationOrderQosPolicy destination_order;

    /* --- Extensions: ---------------------------------------------------- */

    /*ci \dref_DataWriterQos_protocol
     */
    struct DDS_DataWriterProtocolQosPolicy protocol;

    /*i \dref_DataWriterQos_type_support
     */
    struct DDS_TypeSupportQosPolicy type_support;

    /*ci \dref_DataWriterQos_transport
     */
    struct DDS_TransportQosPolicy transport;

    /*ci \dref_DataWriterQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*ci \dref_DataWriterQos_writer_resource_limits
     */
    struct DDS_DataWriterResourceLimitsQosPolicy writer_resource_limits;

    /*ci \dref_DataWriterQos_property
     */
    struct DDS_PropertyQosPolicy property;

    /*-----------------------------  4.0  ------------------------------------*/

    /*ci \dref_DataWriterQos_user_data
     */
    struct DDS_UserDataQosPolicy *user_data;

    /*ci \dref_DataWriterQos_encapsulation
     */
    struct DDS_TransportEncapsulationQosPolicy *encapsulation;

    /*ci \dref_DataWriterQos_representation
     */
    struct DDS_DataRepresentationQosPolicy *representation;

    /*ci \dref_DataWriterQos_publish_mode
     */
    struct DDS_PublishModeQosPolicy *publish_mode;

    /*ci \dref_DataWriterQos_latency_budget
     */
    struct DDS_LatencyBudgetQosPolicy latency_budget;

    /*ci \dref_DataWriterQos_publication_name
     */
    char *publication_name;

    /*ci \dref_DataWriterTransferModeQosPolicy
     */
    struct DDS_DataWriterTransferModeQosPolicy transfer_mode;

    /* end Qos policies */

    /*-----------------------------  4.0  ------------------------------------*/

    /*ci
     * \brief The current DataWriter listener, as specified by the user
     */
    struct DDS_DataWriterListener listener;

    /*ci
     * \brief The currently status' the DataWriter is interested in
     *        as specified by the user
     */
    DDS_StatusMask mask;

    /*----- Internal state management -----*/

    /*ci
     * \brief The deadline event handle from timer
     */
    OSAPI_TimeoutHandle_T deadline_event;

    /*ci
     * \brief The state of the datawriter
     */
    WriterState_t writer_state;

    /*ci
     * \brief The liveliness event handle from timer
     */
    OSAPI_TimeoutHandle_T liveliness_event;

    /*ci
     * \brief Deadline status, updated on deadline events
     */
    struct DDS_OfferedDeadlineMissedStatus off_deadline_missed_status;

    /*ci
     * \brief Incompatible qos status, updated on matching events
     */
    struct DDS_OfferedIncompatibleQosStatus off_incompatible_qos_status;

    /*ci
     * \brief Liveliness lost status, updated on loss of liveliness
     */
    struct DDS_LivelinessLostStatus liveliness_lost_status;

    /*ci
     * \brief Publication matched status, updated on match/unmatch event
     */
    struct DDS_PublicationMatchedStatus publication_matched_status;

    /*ci
     * \brief Reliable reader status, updated on change in per activity detection
     */
    struct DDS_ReliableReaderActivityChangedStatus
                                      reliable_reader_activity_changed_status;

    /*ci
     * \brief Pointer to topic structure published by the datawriter
     */
    DDS_Topic *topic;

    /*ci
     * \brief Pointer to publisher that created this datawriter
     */
    DDS_Publisher *publisher;

    /* --- DDS History Cache --- */
    /*ci
     * \brief Pointer to a run-time component implementing the DDS Writer
     *        history cache API
     */
    struct DDSHST_Writer *wh;

    /* --- Data-path interfaces --- */
    /*ci
     * \brief Pointer to a run-time component implementing the NETIO
     *        interface API to send DDS samples
     */
    NETIO_Interface_T *dw_intf;

    /*ci
     * \brief Pointer to a run-time component implementing the NETIO
     *        interface API supporting the RTPS protocol.
     */
    NETIO_Interface_T *rtps_intf;

    /* --- Type-Plugin --- */
    /*ci
     * \brief Pointer to type-plugin managing the Topic type
     */
    struct NDDS_Type_Plugin *type_plugin;

    /*ci
     * \brief The last SN written by the datawriter
     */
    struct REDA_SequenceNumber last_sn;

    /*ci
     * \brief Stream to generate keyhashes with
     */
    struct CDR_Stream_t *md5_stream;

    /*ci
     * \brief Pool of CDR samples
     */
    REDA_BufferPool_T sample_pool;

    /* --- Properties structure --- */
    /*ci
     * \brief Copy of the configuration data received during initialization
     */
    struct NDDS_DataWriterConfig *config;

    /*ci
     * \brief Information about the writers locators, typically passed together
     *        with the datawriter qos policy to discovery plugins
     */
    struct DDS_DataWriterData writer_data;

    /*ci
     * \brief The unicast locators used by the datawriter
     */
    struct DDS_LocatorSeq uc_locator_seq;

    /*ci
     * \brief The multicast locators used by the datawriter, derived from the
     *        participant and datawriter qos policies
     */
    struct DDS_LocatorSeq mc_locator_seq;

    /*ci
     * \brief Packet to send downstream. The datawriter only supports one
     *        packet a time.
     */
    //NETIO_Packet_T *packet;

    /*ci
     * \brief Current encapsulations
     */
    DDS_EncapsulationId_t active_encapsulation;

    /*ci
     * \brief Zero Copy v2 protocol version to use
     */
    DDS_UnsignedShort zcv2_protocol_version;

    /*ci
     * \brief Data representation QoS policy
     */
    struct DDS_DataRepresentationQosPolicy representations;

    /*ci
     * \brief Transport encapsulation QoS policy
     */
    struct DDS_TransportEncapsulationQosPolicy encapsulations;

    /* The effective  cdr_id for this writer */
    DDS_EncapsulationId_t cdr_id;

    /*ci \brief If not NULL, tHe current packet being sent
     */
    struct NETIO_Packet *packet;

    OSAPI_Mutex_T *write_lock;

#if DDS_XTYPES_IS_ENABLED
    /*ci \brief Internal mask for x-types compliance set based on the
     * PropertyQos.
     */
    NDDS_Config_XTypesComplianceMask xtypes_compliance_mask;
#endif

    /*ci \brief Set to TRUE if padding should added.
     */
    RTI_BOOL set_cdr_options_padding;

#if DDS_FILTERING_ENABLED
    /*ci
     * \brief Handle for the writer to perform writer-side filtering
     */
    struct RTPS_FilterPluginWriterFilter *writer_filter;
#endif
    RTI_BOOL send_key_hash;

#if DDS_FLOW_CONTROLLER_ENABLED
    /*ci
     * \brief The flow controller currently in use by this datawriter, if any
     */
    struct DDS_FlowController *flow_controller;
#endif
};

/*ci
 * \brief Meta-data about a sample being written
 */
struct NDDS_DataWriterSampleInfo
{
    /*ci
     * \brief The current timestamp
     */
    struct OSAPI_SystemTime timestamp;

    /*ci
     * \brief What kind of sample it is
     */
    RTPS_StatusInfo status_info;
};

/*ci
 * \brief Header for the CDR samples sent by the datawriter
 */
struct DDS_DataWriterSample
{
    /*ci
     * \brief Inherited from base-class
     */
    struct DDSHST_WriterSample _sample;

    /*ci
     * \brief Pointer to opaque payload
     */
    struct DDS_TypePluginBuffer *payload;

    /*ci
     * \brief The key_hash for the sample
     */
    // DDS_KeyHash_t key_hash;

    /*ci
     * \brief Meta data about the sample type, needed by RTPS to set flags
     */
    struct NDDS_DataWriterSampleInfo sample_info;

    /*ci
     * \brief The RTPS flags when restoring a packet for this payload
     */
    RTI_UINT32 rtps_flags;

    /*ci
     * \brief A pointer to the original instance for this payload.
     *        NOTE: currently this must be valid for the life-cycle of the writer
     */
    const void *instance_data;

    /*ci
     * \brief Each sample has state, the state is shared for all payloads for
     * the same sample.
     */
    struct NETIO_PacketBuffer inline_pbuf;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterListener_is_consistent(const struct DDS_DataWriterListener *l,
                                     DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_DataWriterImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

#if DDS_LIVELINESS_CHANNEL_ENABLED

MUST_CHECK_RETURN extern RTI_INT32
DDS_DataWriterImpl_compare_lease_duration(RTI_INT32 flags,
                                          const DB_Record_T op1,
                                          void *op2);

MUST_CHECK_RETURN extern RTI_INT32
DDS_DataWriterImpl_compare_automatic_liveliness_kind(RTI_INT32 flags,
                                                     const DB_Record_T op1,
                                                     void *op2);
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */


MUST_CHECK_RETURN extern DDS_UnsignedLong
DDS_DataWriter_get_objectid(DDS_DataWriter * self);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterImpl_finalize(DDS_DataWriter *self);
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterImpl_initialize(
        struct DDS_DataWriterImpl *datawriter,
        DDS_Publisher *publisher,
        DDS_Topic *topic,
        const struct DDS_DataWriterQos *qos,
        const struct DDS_DataWriterListener *listener,
        DDS_StatusMask mask,
        DDS_UnsignedLong object_id,
        struct NDDS_DataWriterConfig *config);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_DataWriter_write_untyped(DDS_DataWriter *self,
                             const void *instance_data,
                             const DDS_InstanceHandle_t *handle,
                             struct NDDS_DataWriterSampleInfo *sample_info);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_DataWriter_register_key(struct DDS_DataWriterImpl *datawriter, /* self */
                            DDS_InstanceHandle_t *handle, /* inout */
                            const void *instance_data, /* in */
                            const struct OSAPI_SystemTime *timestamp /* in */);

extern RTI_BOOL
DDS_DataWriter_update_liveliness(DDS_DataWriter *self);

extern void
DDS_DataWriter_update_historical_ackcount(DDS_DataWriter *self);

extern DDS_ReturnCode_t
DDS_DataWriter_prepare_sample(struct DDS_DataWriterImpl *self,
                                struct DDS_DataWriterSample *sample,
                                NETIO_Packet_T *packet,
                                struct REDA_SequenceNumber *sn);

extern void
DDS_DataWriter_return_sample_payload(DDS_DataWriter *self,
                                     struct DDS_DataWriterSample *sample);

extern DDS_ReturnCode_t
DDS_DataWriter_finalize_managed(
    DDS_DataWriter *self,
    DDS_DomainParticipant *participant);

    DDS_Boolean
DDS_DataWriter_is_immutable_qos_equal(const DDS_DataWriter *left,
                                      const struct DDS_DataWriterQos *right);

extern DDS_ReturnCode_t
DDS_DataWriter_get_qos_from(
        DDS_DataWriter *self,
        struct DDS_DataWriterQos *out);

extern DDS_ReturnCode_t
DDS_DataWriter_set_qos_from(
        DDS_DataWriter *out,
        const struct DDS_DataWriterQos *in,
        DDS_DomainParticipant *participant);

#endif

/*ci @} */
