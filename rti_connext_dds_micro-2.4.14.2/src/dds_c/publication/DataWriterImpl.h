/*
 * FILE: DataWriterImpl.h - DataWriter implementation
 *
 * (c) Copyright 2012-2015 Real-Time Innovations
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_DataWriter_get_objectid
 * 20feb2021,tk  MICRO-2840/PR#28681
 *   - Removed writer from the comment for on_before_datawriter_created
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
 * \def DDS_INLINE_QOS_SIZE
 * \brief The size of the inline QOS parameters sent with a sample.
 *
 * \details
 * The inline QoS is used to send keys and status information. The maximum
 * size needed for this is 32 bytes: 20 for key-hash, 8 for status info and 4
 * bytes for the sentinel.
 */
#define DDS_INLINE_QOS_SIZE                       (32)

/*ci
 * \def DDS_INFINITE_MAX_SAMPLES
 * \brief "Infinite" max_samples when calculating heartbeats per max_samples
 *  and datawriter->qos.resource_limits.max_samples is DDS_UNLIMITED_LENGTH.
 */
#define DDS_INFINITE_MAX_SAMPLES (100000000)

/*ci \brief Typedefinition for function called when a datawriter is enabled
 * 
 * \param[in] writer The writer being enabled
 * \param[in] qos The qos for the writer being enabled
 * 
 * \return TRUE if the call succeeded, FALSE otherwise.
 */
typedef RTI_BOOL
(*DataWriterLifeCycleListener_on_after_enabled)(
                        DDS_DataWriter *const writer,
                        const struct DDS_DataWriterQos *const qos);

/*ci \brief Typedefinition for function called before a writer is deleted
 * 
 * \param[in] writer The writer being deleted
 */
typedef void
(*DataWriterLifeCycleListener_on_before_deleted)(DDS_DataWriter *const dw);

/*ci \brief Typedefinition for function called before a writer is created
 * 
 * \param[in] publisher The factory of the writer
 * \param[in] dw_key The key of the writer being deleted
 * \param[in] reserved Whether to reserve space for the writer or not
 * 
 * \return RETCODE_OK on success, one of the standard return codes on error
 */
typedef DDS_ReturnCode_t
(*DataWriterLifeCycleListener_on_before_datawriter_created)(
                                DDS_Publisher *const publisher,
                                struct DDS_BuiltinTopicKey_t *const dw_key,
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

    /*ci
     * \brief Pointer to a participant_id in case none has yet been assigned
     *        by the participant due to no unicast addresses specified
     */
    DDS_Long *participant_id;

    /*ci
     * \brief Pointer to DDS domain ID of owning DomainParticipant
     */
    DDS_DomainId_t *domain_id;
};


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
     NULL\
}

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
    /*ci
     * \brief The current DataWriter Qos, as specified by the user
     */
    struct DDS_DataWriterQos qos;

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
     * \brief Stream to serialize samples with
     */
    struct CDR_Stream_t stream;

    /*ci
     * \brief Stream to generate keyhashes with
     */
    struct CDR_Stream_t *md5_stream;

    /*ci
     * \brief Pool of CDR samples
     */
    REDA_BufferPool_T cdr_samples;

    /*ci
     * \brief Pool of CDR samples
     */
    REDA_BufferPool_T cdr_payloads;

    /*ci
     * \brief The maximum serialized length of a sample, calculated
     *        based on the type
     */
    RTI_UINT32 max_cdr_serialized_length;

    /*ci
     * \brief The maximum length of the NETIO_Packet needed to send 1 sample
     */
    RTI_UINT32 max_packet_length;


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
    NETIO_Packet_T packet;
};

/*ci
 * \brief Meta-data about a sample being written
 */
struct NDDS_DataWriterSampleInfo
{
    /*ci
     * \brief The current timestamp
     */
    struct OSAPI_NtpTime timestamp;

    /*ci
     * \brief What kind of sample it is
     */
    RTPS_StatusInfo status_info;

    /*ci
     * \brief Whether to send the keyhash or not
     */
    DDS_Boolean send_key_hash;
};

/*ci
 * \brief Header for the CDR samples sent by the datawriter
 */
struct RTI_TransformCDR_Sample
{
    /*ci
     * \brief Inherited from base-class
     */
    struct DDSHST_WriterSample _sample;

    /*ci
     * \brief Pointer to opaque payload
     */
    void *payload;

    /*ci
     * \brief The key_hash for the sample
     */
    DDS_KeyHash_t key_hash;

    struct NDDS_DataWriterSampleInfo sample_info;

    /*ci
     * \brief The current head when restoring a packet for this payload
     */
    RTI_UINT32 head;

    /*ci
     * \brief The current tail when restoring a packet for this payload
     */
    RTI_UINT32 tail;

    /*ci
     * \brief The RTPS flags when restoring a packet for this payload
     */
    RTI_UINT32 rtps_flags;

    /*ci
     * \brief A pointer to the original instance for this payload.
     *        NOTE: currently this must be valid for the life-cycle of the writer
     */
    const void *instance_data;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterListener_is_consistent(const struct DDS_DataWriterListener *l,
                                     DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_DataWriterImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_UnsignedLong
DDS_DataWriter_get_objectid(DDS_DataWriter * self);
#endif

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterImpl_finalize(DDS_DataWriter *self);
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterImpl_initialize(struct DDS_DataWriterImpl *datawriter,
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
                            const struct OSAPI_NtpTime *timestamp /* in */);

extern RTI_BOOL
DDS_DataWriter_update_liveliness(DDS_DataWriter *self);

extern void
DDS_DataWriter_update_historical_ackcount(DDS_DataWriter *self);

extern DDS_ReturnCode_t
DDS_DataWriter_serialize_sample(struct DDS_DataWriterImpl *self,
                                struct RTI_TransformCDR_Sample *sample,
                                NETIO_Packet_T *packet,
                                struct REDA_SequenceNumber *sn);
#endif

/*ci @} */
