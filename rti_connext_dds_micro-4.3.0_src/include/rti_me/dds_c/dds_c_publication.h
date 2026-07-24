/*
 * FILE: dds_c_publication.h - DDS publication module
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
 * 07apr2016,tk MICRO-1541 Fixed assignment operator issues for C++
 * 04nov2015,tk MICRO-1505 - Reduce memory footprint
 * 29jun2015,tk MICRO-1351/PR#15141 Removed prototypes for non-Cert function
 * 20sep2014,as Explicitly define public support functions for Status types
 * 07may2014,as MICRO-784 Expose get_X_status API in C++
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 04mar2014,tk MICRO-84: Added C listener for writer-side notification of
 *                        unacknowledged samples
 * 19jul2013,as Added support for C++
 * 06feb2013,eh MICRO-262: add writer_resource_limits
 * 30apr2012,tk Written
 */
/*ce
 * \file
 * \brief DDS publication module
 */
/*e
  @addtogroup DDSPublicationModule

  @brief Defines the \dds publication package
*/
#ifndef dds_c_publication_h
#define dds_c_publication_h

#include "dds_c_config.h"
#include "dds_c_sequence.h"

#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef dds_c_topic_h
#include "dds_c/dds_c_topic.h"
#endif
#ifndef dds_c_trust_h
#include "dds_c/dds_c_trust.h"
#endif

#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

#ifndef dds_c_partition_qos_h
#include "dds_c/dds_c_partition_qos.h"
#endif

#ifndef dds_c_user_data_qos_h
#include "dds_c/dds_c_user_data_qos.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Default name for the automatically registered DataWriter NETIO
 *        interface.
 */
NETIODllVariable extern
const char* const DDS_DEFAULT_DATAWRITER_NETIO_NAME;

#if 0
/* ================================================================= */
/*                         Typedef for DataWriter                    */
/* ================================================================= */

/*e \dref_DataWriter
 */
typedef struct DDS_DataWriterImpl DDS_DataWriter;
#endif

/* ================================================================= */
/*                         Typedef for Publisher                     */
/* ================================================================= */

/*e \dref_Publisher
 */
typedef struct DDS_PublisherImpl DDS_Publisher;

/* ================================================================= */
/*                             Status                                */
/* ================================================================= */

/*e \dref_OfferedDeadlineMissedStatus
 */
struct DDSCPPDllExport DDS_OfferedDeadlineMissedStatus
{
    /*e \dref_OfferedDeadlineMissedStatus_total_count
     *
     * \brief total count
     */
    DDS_Long total_count;

    /*e \dref_OfferedDeadlineMissedStatus_total_count_change
     */
    DDS_Long total_count_change;

    /*e \dref_OfferedDeadlineMissedStatus_last_instance_handle
     */
    DDS_InstanceHandle_t last_instance_handle;

    DDSC_CPP_STATUS_METHODS(DDS_OfferedDeadlineMissedStatus)
};

/*ce \dref_OfferedDeadlineMissedStatus_INITIALIZER
 */
#define DDS_OfferedDeadlineMissedStatus_INITIALIZER \
                { 0L, 0L, DDS_HANDLE_NIL_NATIVE }

/*ci
 * \brief Reset the changed counters in DDS_OfferedDeadlineMissedStatus
 *
 * \param[in] s Structure to clear
 */
DDSCDllExport void
DDS_OfferedDeadlineMissedStatus_reset(struct DDS_OfferedDeadlineMissedStatus *s);

/* ----------------------------------------------------------------- */

/*e \dref_LivelinessLostStatus
 */
struct DDSCPPDllExport DDS_LivelinessLostStatus
{
    /*e \dref_LivelinessLostStatus_total_count
     */
    DDS_Long total_count;

    /*e \dref_LivelinessLostStatus_total_count_change
     */
    DDS_Long total_count_change;

    DDSC_CPP_STATUS_METHODS(DDS_LivelinessLostStatus)
};

/*ce \dref_LivelinessLostStatus_INITIALIZER
 */
#define DDS_LivelinessLostStatus_INITIALIZER { 0L, 0L }

/*ci
 * \brief Reset the changed counters in DDS_LivelinessLostStatus
 *
 * \param[in] s Structure to clear
 */
DDSCDllExport void
DDS_LivelinessLostStatus_reset(struct DDS_LivelinessLostStatus *s);

/* ----------------------------------------------------------------- */

/*e \dref_OfferedIncompatibleQosStatus
 */
struct DDSCPPDllExport DDS_OfferedIncompatibleQosStatus
{
    /*e \dref_OfferedIncompatibleQosStatus_total_count
     */
    DDS_Long total_count;

    /*e \dref_OfferedIncompatibleQosStatus_total_count_change
     */
    DDS_Long total_count_change;

    /*e \dref_OfferedIncompatibleQosStatus_last_policy_id
     */
    DDS_QosPolicyId_t last_policy_id;

    /*e \dref_OfferedIncompatibleQosStatus_policies
     */
    struct DDS_QosPolicyCountSeq policies;

    DDSC_CPP_STATUS_METHODS(DDS_OfferedIncompatibleQosStatus)
};

/*ce \dref_OfferedIncompatibleQosStatus_INITIALIZER
 */
#define DDS_OfferedIncompatibleQosStatus_INITIALIZER \
{ \
    0L, 0L,DDS_INVALID_QOS_POLICY_ID, \
    DDS_SEQUENCE_INITIALIZER \
}

/*ci
 * \brief Reset the changed counters in DDS_OfferedIncompatibleQosStatus
 *
 * \param[in] s Structure to clear
 */
DDSCDllExport void
DDS_OfferedIncompatibleQosStatus_reset(struct DDS_OfferedIncompatibleQosStatus *s);

/* ----------------------------------------------------------------- */

/*e \dref_PublicationMatchedStatus
 */
struct DDSCPPDllExport DDS_PublicationMatchedStatus
{
    /*e \dref_PublicationMatchedStatus_total_count
     */
    DDS_Long total_count;

    /*e \dref_PublicationMatchedStatus_total_count_change
     */
    DDS_Long total_count_change;

    /*e \dref_PublicationMatchedStatus_current_count
     */
    DDS_Long current_count;

    /*e \dref_PublicationMatchedStatus_current_count_change
     */
    DDS_Long current_count_change;

    /*e \dref_PublicationMatchedStatus_last_subscription_handle
     */
    DDS_InstanceHandle_t last_subscription_handle;

    DDSC_CPP_STATUS_METHODS(DDS_PublicationMatchedStatus)
};

/*ce \dref_PublicationMatchedStatus_INITIALIZER
 */
#define DDS_PublicationMatchedStatus_INITIALIZER \
        { 0L, 0L, 0L, 0L, DDS_HANDLE_NIL_NATIVE }

/*ci
 * \brief Reset the changed counters in DDS_PublicationMatchedStatus
 *
 * \param[in] s Structure to reset
 */
DDSCDllExport void
DDS_PublicationMatchedStatus_reset(struct DDS_PublicationMatchedStatus *s);

/* ------------------------------------------------------------------------ */

/*e \dref_ReliableReaderActivityChangedStatus
 */
struct DDSCPPDllExport DDS_ReliableReaderActivityChangedStatus
{
    /*e \dref_ReliableReaderActivityChangedStatus_active_count
     */
    DDS_Long active_count;

    /*e \dref_ReliableReaderActivityChangedStatus_inactive_count
     */
    DDS_Long inactive_count;

    /*e \dref_ReliableReaderActivityChangedStatus_active_count_change
     */
    DDS_Long active_count_change;

    /*e \dref_ReliableReaderActivityChangedStatus_inactive_count_change
     */
    DDS_Long inactive_count_change;

    /*e \dref_ReliableReaderActivityChangedStatus_last_instance_handle
     */
    DDS_InstanceHandle_t last_instance_handle;

    DDSC_CPP_STATUS_METHODS(DDS_ReliableReaderActivityChangedStatus)
};

/*e \dref_ReliableReaderActivityChangedStatus_INITIALIZER
 */
#define DDS_ReliableReaderActivityChangedStatus_INITIALIZER \
        {0, 0, 0, 0,DDS_HANDLE_NIL_NATIVE}

/*ci
 * \brief Reset the changed counters in DDS_ReliableReaderActivityChangedStatus
 *
 * \param[in] s Structure to clear
 */
DDSCDllExport void
DDS_ReliableReaderActivityChangedStatus_reset(struct DDS_ReliableReaderActivityChangedStatus *s);

/* ------------------------------------------------------------------------ */

/*e \dref_ReliableSampleUnacknowledgedStatus
 */
struct DDSCPPDllExport DDS_ReliableSampleUnacknowledgedStatus
{
    /*e \dref_ReliableSampleUnacknowledgedStatus_sequence_number
     */
    struct DDS_SequenceNumber_t sequence_number;

    /*e \dref_ReliableSampleUnacknowledgedStatus_unacknowledged_count
     */
    DDS_Long unacknowledged_count;

    /*e \dref_ReliableSampleUnacknowledgedStatus_instance_handle
     */
    DDS_InstanceHandle_t instance_handle;
};

/*i \dref_ReliableSampleUnacknowledgedStatus_INITIALIZER
 */
#define DDS_ReliableSampleUnacknowledgedStatus_INITIALIZER \
{\
    {0,0},\
    0,\
    DDS_HANDLE_NIL_NATIVE\
}

/*ci
 * \brief Initialize a DDS_OfferedDeadlineMissedStatus structure
 *
 * \param[in] self DDS_OfferedDeadlineMissedStatus to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_OfferedDeadlineMissedStatus_initialize(
            struct DDS_OfferedDeadlineMissedStatus *self);

/*ci
 * \brief Initialize a DDS_OfferedIncompatibleQosStatus structure
 *
 * \param[in] self DDS_OfferedIncompatibleQosStatus to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_OfferedIncompatibleQosStatus_initialize(
            struct DDS_OfferedIncompatibleQosStatus *self);

/*ci
 * \brief Initialize a DDS_PublicationMatchedStatus structure
 *
 * \param[in] self DDS_PublicationMatchedStatus to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PublicationMatchedStatus_initialize(
            struct DDS_PublicationMatchedStatus *self);

/*ci
 * \brief Initialize a DDS_LivelinessLostStatus structure
 *
 * \param[in] self DDS_LivelinessLostStatus to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_LivelinessLostStatus_initialize(
            struct DDS_LivelinessLostStatus *self);

/*ci
 * \brief Initialize a DDS_ReliableReaderActivityChangedStatus structure
 *
 * \param[in] self DDS_ReliableReaderActivityChangedStatus to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_ReliableReaderActivityChangedStatus_initialize(
            struct DDS_ReliableReaderActivityChangedStatus *self);

/* ----------------------------------------------------------------- */

#ifdef __cplusplus
}                               /* extern "C" */
#endif

/* ================================================================= */
/*                          QoS                                      */
/* ================================================================= */

/*i \dref_DataWriterData
 */
struct DDS_DataWriterData
{
    /*ci
     * \brief Pointer to the datawriter specific unicast locators, if any
     */
    struct DDS_LocatorSeq *unicast_locator;

    /*ci
     * \brief Pointer to the datawriter unicast locators inherited from the participant
     * \detail Only valid if the length of the unicast_locator above is zero.
     */
    struct DDS_LocatorSeq *resolved_participant_locators;
};

/*e \dref_DataWriterQos
 */
struct DDSCPPDllExport DDS_DataWriterQos
{
    /*e \dref_DataWriterQos_deadline
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*e \dref_DataWriterQos_liveliness
     */
    struct DDS_LivelinessQosPolicy liveliness;

    /*e \dref_DataWriterQos_history
     */
    struct DDS_HistoryQosPolicy history;

    /*e \dref_DataWriterQos_resource_limits
     */
    struct DDS_ResourceLimitsQosPolicy resource_limits;

    /*e \dref_DataWriterQos_ownership
     */
    struct DDS_OwnershipQosPolicy ownership;

    /*e \dref_DataWriterQos_ownership_strength
     */
    struct DDS_OwnershipStrengthQosPolicy ownership_strength;

    /*e \dref_DataWriterQos_latency_budget
     */
    struct DDS_LatencyBudgetQosPolicy latency_budget;

    /*e \dref_DataWriterQos_reliability
     */
    struct DDS_ReliabilityQosPolicy reliability;

    /*e \dref_DataWriterQos_durability
     */
    struct DDS_DurabilityQosPolicy durability;

    /*e \dref_DataWriterQos_destination_order
     */
    struct DDS_DestinationOrderQosPolicy destination_order;

    /*i \dref_DataWriterQos_encapsulation
     */
    struct DDS_TransportEncapsulationQosPolicy encapsulation;

    /*e \dref_DataWriterQos_representation
     */
    struct DDS_DataRepresentationQosPolicy representation;

    /* --- Extensions: ---------------------------------------------------- */

    /*e \dref_DataWriterQos_protocol
     */
    struct DDS_DataWriterProtocolQosPolicy protocol;

    /*i \dref_DataWriterQos_type_support
     */
    struct DDS_TypeSupportQosPolicy type_support;

    /*e \dref_DataWriterQos_transport
     */
    struct DDS_TransportQosPolicy transport;

    /*i \dref_DataWriterQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*e \dref_DataWriterQos_writer_resource_limits
     */
    struct DDS_DataWriterResourceLimitsQosPolicy writer_resource_limits;

    /*e \dref_DataWriterQos_publish_mode
     */
    struct DDS_PublishModeQosPolicy publish_mode;

    /*e \dref_DataWriterQos_user_data
     */
    struct DDS_UserDataQosPolicy user_data;

    /*e \dref_DataWriterQos_property
     */
    struct DDS_PropertyQosPolicy property;

#if DDS_ENABLE_APPGEN
    /*e \dref_DataWriterQos_publication_name
     */
    struct DDS_EntityNameQosPolicy publication_name;
#endif /* DDS_ENABLE_APPGEN */

    /*e \dref_DataWriterTransferModeQosPolicy
     */
    struct DDS_DataWriterTransferModeQosPolicy transfer_mode;

    /*e \dref_DataWriterQos_transport_priority
     */
    struct DDS_TransportPriorityQosPolicy transport_priority;

    DDSC_CPP_QOS_METHODS(DDS_DataWriterQos)
};

#ifdef __cplusplus
extern "C" {
#endif

/*ce \dref_DataWriterQos_initialize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriterQos_initialize(struct DDS_DataWriterQos *self);

/*ce \dref_DataWriterQos_copy
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriterQos_copy(struct DDS_DataWriterQos *self,
                       const struct DDS_DataWriterQos *source);

/*ci
 * \brief Compare two DDS_DataWriterQos policies for equality
 *
 * \param[in] left  The left side of the comparison
 * \param[in] right The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if the structures are equal,
 *         DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriterQos_is_equal(const struct DDS_DataWriterQos *left,
                               const struct DDS_DataWriterQos *right);

#ifndef RTI_CERT
/*ce \dref_DataWriterQos_finalize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriterQos_finalize(struct DDS_DataWriterQos *self);
#endif /* !RTI_CERT */

#if DDS_ENABLE_APPGEN
#define DDS_DATAWRITERQOS_APPGEN_INITIALIZER \
    DDS_ENTITY_NAME_QOS_POLICY_DEFAULT,
#else
#define DDS_DATAWRITERQOS_APPGEN_INITIALIZER
#endif

/*ce \dref_DataWriterQos_INITIALIZER
 */
#define DDS_DataWriterQos_INITIALIZER   {                 \
    DDS_DEADLINE_QOS_POLICY_DEFAULT,                      \
    DDS_LIVELINESS_QOS_POLICY_DEFAULT,                    \
    DDS_HISTORY_QOS_POLICY_DEFAULT,                       \
    DDS_RESOURCE_LIMITS_QOS_POLICY_DEFAULT,               \
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT,                     \
    DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT,            \
    DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT,                \
    DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT,        \
    DDS_DURABILITY_QOS_POLICY_DEFAULT,                    \
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT,             \
    DDS_TRANSPORT_ENCAPSULATION_QOS_POLICY_DEFAULT,       \
    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT,           \
    DDS_DATA_WRITER_PROTOCOL_QOS_POLICY_DEFAULT,          \
    DDS_TYPESUPPORT_QOS_POLICY_DEFAULT,                   \
    DDS_TRANSPORT_QOS_POLICY_DEFAULT,                     \
    RTI_MANAGEMENT_QOS_POLICY_DEFAULT,                    \
    DDS_DATAWRITERRESOURCE_LIMITS_QOS_POLICY_DEFAULT,     \
    DDS_PUBLISH_MODE_QOS_POLICY_DEFAULT,                  \
    DDS_USER_DATA_QOS_POLICY_DEFAULT,                     \
    DDS_PROPERTY_QOS_POLICY_DEFAULT,                      \
    DDS_DATAWRITERQOS_APPGEN_INITIALIZER                  \
    DDS_DataWriterTransferModeQosPolicy_INITIALIZER,      \
    DDS_TRANSPORT_PRIORITY_QOS_POLICY_DEFAULT             \
}

/*e \dref_XTYPES_COMPLIANCE_MASK_PROPERTY
 */
extern DDSCDllVariable const char*
DDS_XTYPES_COMPLIANCE_MASK_PROPERTY;

/*e \dref_DATAWRITER_PROTOCOL_ZCV2_VERSION_PROPERTY
 */
extern DDSCDllVariable const char*
DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_PROPERTY;

extern DDSCDllVariable const DDS_UnsignedShort
DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_DEFAULT;

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ----------------------------------------------------------------- */

/*e \dref_PublisherQos
 */
struct DDSCPPDllExport DDS_PublisherQos
{
    /*e \dref_PublisherQos_entity_factory
     */
    struct DDS_EntityFactoryQosPolicy entity_factory;

    /*e \dref_PublisherQos_partition
     */
    struct DDS_PartitionQosPolicy partition;

    /*i \dref_PublisherQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*e \dref_PublisherQos_group_data
     */
    struct DDS_GroupDataQosPolicy group_data;

#if DDS_ENABLE_APPGEN
    /*e \dref_PublisherQos_publisher_name
     */
    struct DDS_EntityNameQosPolicy publisher_name;
#endif /* DDS_ENABLE_APPGEN */

    DDSC_CPP_QOS_METHODS(DDS_PublisherQos)
};

#ifdef __cplusplus
extern "C" {
#endif

/*ce \dref_PublisherQos_initialize
 */
/* #if INCLUDE_API_QOS */
DDSCDllExport DDS_ReturnCode_t
DDS_PublisherQos_initialize(struct DDS_PublisherQos *self);

/*ce \dref_PublisherQos_copy
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PublisherQos_copy(struct DDS_PublisherQos *self,
                      const struct DDS_PublisherQos *source);

/*ci
 * \brief Compare two DDS_PublisherQos policies for equality
 *
 * \param[in] left  The left side of the comparison
 * \param[in] right The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if the structures are equal,
 *         DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_PublisherQos_is_equal(const struct DDS_PublisherQos *left,
                          const struct DDS_PublisherQos *right);

#ifndef RTI_CERT
/*ce \dref_PublisherQos_finalize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PublisherQos_finalize(struct DDS_PublisherQos *self);
#endif

#if DDS_ENABLE_APPGEN
#define DDS_PUBLISHERQOS_APPGEN_INITIALIZER \
    , DDS_ENTITY_NAME_QOS_POLICY_DEFAULT
#else
#define DDS_PUBLISHERQOS_APPGEN_INITIALIZER
#endif /* DDS_ENABLE_APPGEN */

/*ce \dref_PublisherQos_INITIALIZER
 */
#define DDS_PublisherQos_INITIALIZER   {    \
    DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT,  \
    DDS_PARTITION_QOS_POLICY_DEFAULT,       \
    RTI_MANAGEMENT_QOS_POLICY_DEFAULT,      \
    DDS_GROUP_DATA_QOS_POLICY_DEFAULT       \
    DDS_PUBLISHERQOS_APPGEN_INITIALIZER     \
}

/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

/*ce \dref_DataWriterListener_OfferedDeadlineMissedCallback
 */
typedef void
(*DDS_DataWriterListener_OfferedDeadlineMissedCallback)(
        void *listener_data,
        DDS_DataWriter* writer,
        const struct DDS_OfferedDeadlineMissedStatus* status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriterListener_LivelinessLostCallback
 */
typedef void
(*DDS_DataWriterListener_LivelinessLostCallback)(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_LivelinessLostStatus *status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriterListener_OfferedIncompatibleQosCallback
 */
typedef void
(*DDS_DataWriterListener_OfferedIncompatibleQosCallback)(
        void *listener_data,
        DDS_DataWriter* writer,
        const struct DDS_OfferedIncompatibleQosStatus *status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriterListener_PublicationMatchedCallback
 */
typedef void (*DDS_DataWriterListener_PublicationMatchedCallback)(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_PublicationMatchedStatus *status);

/* ------------------------------------------------------------------------ */
/*e \dref_DataWriterListener_ReliableReaderActivityChangedCallback
 */
typedef void
(*DDS_DataWriterListener_ReliableReaderActivityChangedCallback)(
        void *listener_data,
        DDS_DataWriter * writer,
         const struct DDS_ReliableReaderActivityChangedStatus * status);

/* ------------------------------------------------------------------------ */
/*e \dref_DataWriterListener_ReliableSampleUnacknowledgedCallback
 */
typedef void
(*DDS_DataWriterListener_ReliableSampleUnacknowledgedCallback)(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_ReliableSampleUnacknowledgedStatus *status);

/* ------------------------------------------------------------------------ */
/*e \dref_DataWriterListener_SampleRemovedCallback
 */
typedef void
(*DDS_DataWriterListener_SampleRemovedCallback)(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_Cookie_t* cookie);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriterListener
 */
struct DDS_DataWriterListener
{
    /*ce \dref_DataWriterListener_as_listener
     */
    struct DDS_Listener as_listener;

    /*e \dref_DataWriterListener_on_offered_deadline_missed
     */
    DDS_DataWriterListener_OfferedDeadlineMissedCallback
    on_offered_deadline_missed;

    /*e \dref_DataWriterListener_on_offered_incompatible_qos
     */
    DDS_DataWriterListener_OfferedIncompatibleQosCallback
    on_offered_incompatible_qos;

    /*e \dref_DataWriterListener_on_liveliness_lost
     */
    DDS_DataWriterListener_LivelinessLostCallback on_liveliness_lost;

    /*e \dref_DataWriterListener_on_publication_matched
     */
    DDS_DataWriterListener_PublicationMatchedCallback
    on_publication_matched;

    /*e \dref_DataWriterListener_on_reliable_reader_activity_changed
     */
    DDS_DataWriterListener_ReliableReaderActivityChangedCallback
    on_reliable_reader_activity_changed;

    /*e \dref_DataWriterListener_on_reliable_sample_unacknowledged
     */
    DDS_DataWriterListener_ReliableSampleUnacknowledgedCallback
    on_reliable_sample_unacknowledged;

    /*e \dref_DataWriterListener_on_sample_removed
     */
    DDS_DataWriterListener_SampleRemovedCallback on_sample_removed;
};

/*ce \dref_DataWriterListener_INITIALIZER
 */
#define DDS_DataWriterListener_INITIALIZER  { \
  DDS_Listener_INITIALIZER, \
  (DDS_DataWriterListener_OfferedDeadlineMissedCallback)NULL, \
  (DDS_DataWriterListener_OfferedIncompatibleQosCallback)NULL, \
  (DDS_DataWriterListener_LivelinessLostCallback)NULL, \
  (DDS_DataWriterListener_PublicationMatchedCallback)NULL,\
  NULL,\
  NULL,\
  (DDS_DataWriterListener_SampleRemovedCallback)NULL\
}

/* ----------------------------------------------------------------- */
/*ce \dref_PublisherListener
 */
struct DDS_PublisherListener
{
    /*e \dref_PublisherListener_as_datawriterlistener
     */
    struct DDS_DataWriterListener as_datawriterlistener;
};

/*ce \dref_PublisherListener_INITIALIZER
 */
#define DDS_PublisherListener_INITIALIZER   { \
    DDS_DataWriterListener_INITIALIZER }

/* ================================================================= */
/*                          Publisher                                */
/* ================================================================= */

/* ----------------------------------------------------------------- */

/*e \dref_DATAWRITER_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_DataWriterQos
        DDS_DATAWRITER_QOS_DEFAULT;

/* ----------------------------------------------------------------- */
#define DDS_Publisher_as_entity(publisherPtr) \
            ((DDS_Entity*) publisherPtr)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_Publisher_as_entity
 */
DDS_Entity *DDS_Publisher_as_entity(DDS_Publisher * publisher);
#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */

/* ----------------------------------------------------------------- */
#if INCLUDE_API_QOS
/*ce \dref_Publisher_get_default_datawriter_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_get_default_datawriter_qos(
        DDS_Publisher *self,
        struct DDS_DataWriterQos *qos);

/* ----------------------------------------------------------------- */
/* #if INCLUDE_API_QOS */
/*ce \dref_Publisher_set_default_datawriter_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos(
        DDS_Publisher *self,
        const struct DDS_DataWriterQos *qos);
#endif
/* ----------------------------------------------------------------- */
/*ce \dref_Publisher_create_datawriter
 */
DDSCDllExport DDS_DataWriter*
DDS_Publisher_create_datawriter(
        DDS_Publisher *self,
        DDS_Topic *topic,
        const struct DDS_DataWriterQos *qos,
        const struct DDS_DataWriterListener *listener,
        DDS_StatusMask mask);

/* ----------------------------------------------------------------- */
/*ci \dref_Publisher_enable
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_enable(DDS_Entity *self);

/* ----------------------------------------------------------------- */
#ifndef RTI_CERT
/*ce \dref_Publisher_delete_datawriter
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_delete_datawriter(
        DDS_Publisher *self,
        DDS_DataWriter *a_datawriter);
#endif

/* ----------------------------------------------------------------- */
/*ce \dref_Publisher_lookup_datawriter
 */
/* #if INCLUDE_API_LOOKUP || RTI_CERT */
DDSCDllExport DDS_DataWriter*
DDS_Publisher_lookup_datawriter(
         DDS_Publisher *self,
         const char *topic_name);

/* ----------------------------------------------------------------- */
#if DDS_ENABLE_APPGEN
/*ce \dref_Publisher_lookup_datawriter_by_name
 */
DDSCDllExport DDS_DataWriter*
DDS_Publisher_lookup_datawriter_by_name(
         DDS_Publisher *self,
         const char *writer_name);
#endif /* DDS_ENABLE_APPGEN */

/* ----------------------------------------------------------------- */
/*ce \dref_Publisher_get_participant
 */
DDSCDllExport DDS_DomainParticipant*
DDS_Publisher_get_participant(DDS_Publisher * self);

/* ----------------------------------------------------------------- */

#ifndef RTI_CERT
/*ce \dref_Publisher_delete_contained_entities
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities(DDS_Publisher * self);
#endif

/* ----------------------------------------------------------------- */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_copy_partition(DDS_Publisher *self,
                            struct DDS_PartitionQosPolicy *partition);

DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_copy_group_data(DDS_Publisher *self,
                            struct DDS_GroupDataQosPolicy *group_data);

DDSCDllExport DDS_Boolean
DDS_Publisher_serialize(const DDS_Publisher *self,struct CDR_Stream_t *stream);

DDSCDllExport struct DDS_PartitionQosPolicy*
DDS_Publisher_get_partition_ref(DDS_Publisher *self);

/* ----------------------------------------------------------------- */
#if INCLUDE_API_QOS
/*ce \dref_Publisher_set_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_qos(
        DDS_Publisher *self,
        const struct DDS_PublisherQos *qos);

/* ----------------------------------------------------------------- */
/*ce \dref_Publisher_get_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_get_qos(
        DDS_Publisher *self,
        struct DDS_PublisherQos *qos);
#endif

/* ----------------------------------------------------------------- */
#ifndef RTI_CERT
/*ce \dref_Publisher_set_listener
 */
/* called internally by Pub_new() */
DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_listener(
        DDS_Publisher * self,
        const struct DDS_PublisherListener *listener,
        DDS_StatusMask mask);
#endif

#ifndef RTI_CERT
/*ce \dref_Publisher_get_listener
 */
DDSCDllExport struct DDS_PublisherListener
DDS_Publisher_get_listener(DDS_Publisher * self);
#endif

/* ================================================================= */
/*                      Data Writer                                  */
/* ================================================================= */

/* ----------------------------------------------------------------- */
#define DDS_DataWriter_as_entity(dataWriterPtr) \
            ((DDS_Entity*) dataWriterPtr)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_DataWriter_as_entity
 */
DDS_Entity *DDS_DataWriter_as_entity(DDS_DataWriter * dataWriter);
#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */

/* ----------------------------------------------------------------- */
/*ci \dref_DataWriter_enable
*/
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_enable(DDS_Entity *self);

/*ce \dref_DataWriter_assert_liveliness
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_assert_liveliness(DDS_DataWriter * self);

/* ----------------------------------------------------------------- */
#if INCLUDE_API_LOOKUP
/*ce \dref_DataWriter_get_matched_subscriptions
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscriptions(
        DDS_DataWriter *self,
        struct DDS_InstanceHandleSeq *subscription_handles);
#endif /* INCLUDE_API_LOOKUP */

struct DDS_SubscriptionBuiltinTopicData;

/* ----------------------------------------------------------------- */
#if INCLUDE_API_LOOKUP
/*ce \dref_DataWriter_get_matched_subscription_data
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscription_data(
        DDS_DataWriter *self,
        struct DDS_SubscriptionBuiltinTopicData *subscription_data,
        const DDS_InstanceHandle_t *subscription_handle);
#endif /* INCLUDE_API_LOOKUP */

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_topic
 */
DDSCDllExport DDS_Topic*
DDS_DataWriter_get_topic(DDS_DataWriter *self);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_publisher
 */
DDSCDllExport DDS_Publisher*
DDS_DataWriter_get_publisher(DDS_DataWriter * self);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_liveliness_lost_status
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_liveliness_lost_status(
        DDS_DataWriter *self,
        struct DDS_LivelinessLostStatus *status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_offered_deadline_missed_status
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_offered_deadline_missed_status(
        DDS_DataWriter *self,
        struct DDS_OfferedDeadlineMissedStatus *status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_offered_incompatible_qos_status
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_offered_incompatible_qos_status(
        DDS_DataWriter *self,
        struct DDS_OfferedIncompatibleQosStatus *status);

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_publication_matched_status
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_publication_matched_status(
        DDS_DataWriter *self,
        struct DDS_PublicationMatchedStatus *status);

/* ------------------------------------------------------------------------ */
/*e \dref_DataWriter_get_reliable_reader_activity_changed_status
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_reliable_reader_activity_changed_status(
        DDS_DataWriter *self,
        struct DDS_ReliableReaderActivityChangedStatus *status);

#if INCLUDE_API_QOS
/*ce \dref_DataWriter_set_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_set_qos(
        DDS_DataWriter *self,
        const struct DDS_DataWriterQos *qos);


/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_get_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_qos(DDS_DataWriter *self,
        struct DDS_DataWriterQos *qos);
#endif

/* ----------------------------------------------------------------- */
/*ce \dref_DataWriter_set_listener
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_set_listener(
        DDS_DataWriter * self,
        const struct DDS_DataWriterListener *listener,
        DDS_StatusMask mask);

#ifndef RTI_CERT
/*ce \dref_DataWriter_get_listener
 */
DDSCDllExport struct DDS_DataWriterListener
DDS_DataWriter_get_listener(DDS_DataWriter * self);
#endif

/********************* Untyped Writer API ****************************/
DDSCDllExport DDS_InstanceHandle_t
DDS_DataWriter_register_instance(
        DDS_DataWriter * self,
        const void *instance_data);

DDSCDllExport DDS_InstanceHandle_t
DDS_DataWriter_register_instance_w_timestamp(
        DDS_DataWriter * self,
        const void *instance_data,
        const struct DDS_Time_t *source_timestamp);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_unregister_instance(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_unregister_instance_w_timestamp(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_dispose(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_dispose_w_timestamp(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_write(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t * handle);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_write_w_timestamp(
        DDS_DataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_write_w_params(
        DDS_DataWriter *self,
        const void *instance_data,
        struct DDS_WriteParams_t *params);

/*i \dref_DataWriteR_get_liveliness_count
 */
DDSCDllExport DDS_Long
DDS_DataWriter_get_liveliness_count(DDS_DataWriter * self);

/*i \dref_DataWriter_assert_key
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_assert_instance(DDS_DataWriter *datawriter,
                          DDS_InstanceHandle_t *handle);

/*i \dref_DataWriter_delete_key
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_remove_instance(DDS_DataWriter *datawriter,
                          DDS_InstanceHandle_t *handle);

/*i \dref_DataWriter_get_loan
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_get_loan(DDS_DataWriter *self, void** data);

/*i \dref_DataWriter_discard_loan
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_discard_loan(DDS_DataWriter *self, void *data);

/****************** Additional Internal APIs **************************/

/*ci
 * \brief Increment the sequence number the datawriter is using for samples
 *
 * \details
 * An anonymous datawriter does not increment the sequence number on each
 * send. Instead the application using this datawriter must increment the
 * sequence number. When a datareader receives a sequence number it has
 * already seen it drops the samples. Thus, this function enables an
 * application to determine when the content of a sample has changed and
 * should be processed by a datareader by incrementing the sequence number.
 * This is typically used as part of participant discovery to prevent known
 * remote participants from processing known information, while sending the
 * same sample to newly discovered remote participants.
 *
 * \param[in] self Datawriter to increment the sequence number on
 *
 * \return DDS_RETCODE_OK on success, one the the standard return codes on
 *        failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_advance_sn(DDS_DataWriter *self);

/*ci
 * \brief Add a new peer to a DDS participant
 *
 * \details
 * The peers for a participant is usually specified in the initial peer
 * list in the participant Qos policy. This function lets an application
 * add additional peers.
 *
 * \param[in] datawriter Datawriter to add the peer to
 * \param[in] dst_reader The peer datareader to send data to
 * \param[in] address    The address of the peer datareader in NETIO address
 *                       string format
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_add_anonymous_peer(DDS_DataWriter *datawriter,
                                  struct NETIO_Address *dst_reader,
                                  const char *address);

#ifndef RTI_CERT
/*ci
 * \brief Remove a peer from a DDS participant
 *
 * \details
 * The peers for a participant is usually specified in the initial peer
 * list in the participant Qos policy. This function lets an application
 * remove a peer.
 *
 * \param[in] datawriter Datawriter to remote the peer from
 * \param[in] dst_reader The peer datareader to remove
 * \param[in] address    The address of the peer datareader in NETIO address
 *                       string format
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_delete_anonymous_peer(DDS_DataWriter *datawriter,
                                      struct NETIO_Address *dst_reader,
                                      const char *address);
#endif /* !RTI_CERT */
/*
 * \brief Add an anonymous route to a DDS DataReader
 *
 * \details
 * This function adds an anonymous route to a DataWriter. An anonymous
 * route is a route with no state information and is typically used
 * by a DDS participant to send to discovery traffic to other
 * participants in a domain.
 *
 * \param[in] self         The datawriter to add the route to
 * \param[in] dst_reader   The peer reader to listen to
 * \param[in] via_address  The address to send data on
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_add_anonymous_route(DDS_DataWriter *datawriter,
                                   struct NETIO_Address *dst_reader,
                                   struct NETIO_Address *via_address);

/*ci
 * \brief Remove an anonymous route from a DDS DataWriter
 *
 * \details
 * This function removes an anonymous route from a DataWriter. An anonymous
 * route is a route with no state information and is typically used
 * by a DDS participant to send discovery traffic to other
 * participants in a domain. This function stops sending traffic
 * to a particular address.
 *
 * \param[in] self         The datareader to remove the route from
 * \param[in] src_writer   The peer writer to stop listening to
 * \param[in] from_address The address to stop listening on
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_delete_anonymous_route(DDS_DataWriter *datawriter,
                                      struct NETIO_Address *dst_reader,
                                      struct NETIO_Address *via_address);

/*ci
 * \brief Remove an anonymous route from a DDS DataWriter using a sequence
 *
 * \details
 *
 * \param[in] datawriter   The datawriter to remove the route from
 * \param[in] dst_reader   The peer reader to stop sending to
 * \param[in] locator_seq The addresses to stop sending to
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_delete_anonymous_route_from_seq(DDS_DataWriter *datawriter,
                                               struct NETIO_Address *dst_reader,
                                               const struct DDS_LocatorSeq *locator_seq);

/*ci
 * \brief Check if a route exists from a datawriter to a datareader
 *
 * \details
 * Check if a route exists from a datawriter to a datareader using any of
 * the locators passed in. Note that there is an implicit assumption that
 * the protocol stack is DDS<->RTPS<->"transport" where transport can be
 * any NETIO_Interface compliant layer.
 *
 * \param[in] src_writer Source data-writer
 * \param[in] dst_reader Destination data-reader
 * \param[in] loc_seq    Sequence of addresses the data-reader is located at
 *
 * \return DDS_BOOLEAN_TRUE if a path exists, DDS_BOOLEAN is no
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_lookup_route(DDS_DataWriter *src_writer,
                            struct NETIO_Address *dst_reader,
                            const struct DDS_LocatorSeq *loc_seq);

struct DDS_DataReaderQos;

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_add_route(DDS_DataWriter *datawriter,
                         const DDS_BuiltinTopicKey_t *key,
                         const struct DDS_DataReaderQos *const qos,
                         const struct DDS_LocatorSeq *uc_locator,
                         const struct DDS_LocatorSeq *mc_locator,
                         DDS_EncapsulationId_t encapsulation,
                         DDS_DataRepresentationId_t representation,
                         RTI_BOOL *route_existed);

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DataWriter_delete_route(DDS_DataWriter *datawriter,
                            const DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator,
                            RTI_BOOL *route_existed);

/*ci
 * Returns the pointer to the DataWriter's single instance of the
 * DDS_TypePlugin
 */
MUST_CHECK_RETURN DDSCDllExport DDS_TypePlugin*
DDS_DataWriter_get_type_plugin(DDS_DataWriter *datawriter);

#if DDS_XTYPES_IS_ENABLED
MUST_CHECK_RETURN DDSCDllExport DDS_TypeCode*
DDS_DataWriter_get_typecode(DDS_DataWriter *datawriter);
#endif

DDSCDllExport DDS_Boolean
DDS_DataWriter_is_announced(const DDS_DataWriter *self);

DDSCDllExport DDS_Boolean
DDS_DataWriter_is_anonymous(const DDS_DataWriter *self);

DDSCDllExport const struct DDS_DataRepresentationQosPolicy*
DDS_DataWriter_get_data_representation_ref(const DDS_DataWriter *self);

DDSCDllExport const struct DDS_TransportEncapsulationQosPolicy*
DDS_DataWriter_get_encapsulation_ref(const DDS_DataWriter *self);

DDSCDllExport const struct DDS_LocatorSeq*
DDS_DataWriter_get_unicast_locator_ref(const DDS_DataWriter *self);

DDSCDllExport const struct DDS_LatencyBudgetQosPolicy*
DDS_DataWriter_get_latency_budget_ref(const DDS_DataWriter *self);

DDSCDllExport const struct DDS_LocatorSeq*
DDS_DataWriter_get_resolved_locator_ref(const DDS_DataWriter *self);

DDSCDllExport DDS_Boolean
DDS_DataWriter_serialize(const DDS_DataWriter *self,struct CDR_Stream_t *stream);

DDSCDllExport const struct DDS_DataWriterResourceLimitsQosPolicy*
DDS_DataWriter_get_writer_resource_limits_ref(const DDS_DataWriter *self);

DDSCDllExport DDS_Long
DDS_DataWriter_get_writer_loaned_sample_allocation(const DDS_DataWriter *self);

DDSCDllExport DDS_Boolean
DDS_DataWriter_get_initialize_writer_loaned_sample(const DDS_DataWriter *self);

DDSCDllExport DDS_Long
DDS_DataWriter_get_max_samples(const DDS_DataWriter *self);

DDSCDllExport DDS_Long
DDS_DataWriter_get_max_remote_readers(const DDS_DataWriter *self);

DDSCDllExport DDS_UnsignedShort
DDS_DataWriter_get_zcv2_protocol_version(const DDS_DataWriter *self);

DDSCDllExport void
DDS_DataWriter_get_request_offered_qos(const DDS_DataWriter *self,
                                       struct DDS_DataWriterQos *qos);

/* ----------------------------------------------------------------- */

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#ifndef dds_c_w_history_plugin_h
#include "dds_c/dds_c_wh_plugin.h"
#endif


#endif /* dds_c_publication_h */
