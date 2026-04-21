/*
 * FILE: dds_c_discovery.h - DDS discovery data definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2025.
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
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 15may2015,tk MICRO-1221/PR#14767 Removed send_queue_size
 * 26jan2015,tk MICRO-1028/PR#13473 Added macros to test for builtin topics
 * 23jan2012,tk Written
 */
/*ce
 * \file
 * \brief DDS discovery data definitions
 */
/*e
  @ingroup DDSDiscoveryModule
*/
#ifndef dds_c_discovery_h
#define dds_c_discovery_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
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

#if DDS_FILTERING_ENABLED
#ifndef dds_c_content_filter_h
#include "dds_c/dds_c_content_filter.h"
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER    0x00000001 << 0
#define DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR     0x00000001 << 1
#define DDS_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER    0x00000001 << 2
#define DDS_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR     0x00000001 << 3
#define DDS_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER   0x00000001 << 4
#define DDS_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR    0x00000001 << 5

#if DDS_LIVELINESS_CHANNEL_ENABLED

#define DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_ANNOUNCER   0x00000001 << 10
#define DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_DETECTOR    0x00000001 << 11

/*i \ingroup BuiltIpcBuiltinEndpointModule
 * @brief Builtin Endpoint Qos for a Participant. Defines the QoS used for the
 * buitin endpoints which QoS is not limited to just one value.
 */
typedef CDR_UnsignedLong DDS_BuiltinEndpointQos_t;

#define DDS_BUILTIN_ENDPOINT_QOS_NONE  (0)

/*i \ingroup BuiltIpcBuiltinEndpointModule
 * @brief Builtin Endpoint Qos Bit values.
 */
typedef DDS_UnsignedLong DDS_BuiltinEndpointQosBits;

/*i \dref_BuiltinEndpointQosBits_BEST_EFFORT_PARTICIPANT_MESSAGE_DATA_READER
 */
#define DDS_BUILTIN_ENDPOINT_QOS_BIT_BEST_EFFORT_PARTICIPANT_MESSAGE_DATA_READER (1 << 0)

/*i \dref_BuiltinEndpointQosBits_IS_VALID
 * @brief RTI extension (only used internally, never sent to the wire).
 */
#define DDS_BUILTIN_ENDPOINT_QOS_BIT_IS_VALID (1U << 31)

#define DDS_BUILTIN_ENDPOINT_QOS_IS_VALID(qos) \
            (((qos) & DDS_BUILTIN_ENDPOINT_QOS_BIT_IS_VALID) != 0)

#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */


















/* The default maximum number of route locators for a builtin endpoint.
 * Only used by the builtin IPC channels
 */
#define DDS_DEFAULT_MAX_LOCATORS_PER_DISCOVERED_PARTICIPANT (6)

#define DDSC_PARTICIPANT_ADDRESS_COUNT_MAX RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX

#define DDSC_PARTICIPANT_MAX_PROPERTIES         (4 + 2)
#define DDSC_PARTICIPANT_MAX_BINARY_PROPERTIES  (4 + 2)

#define DDSC_DATAWRITER_MAX_PROPERTIES         (4 + 2)
#define DDSC_DATAWRITER_MAX_BINARY_PROPERTIES  (4 + 2)

#define DDSC_DATAREADER_MAX_PROPERTIES         (4 + 2)
#define DDSC_DATAREADER_MAX_BINARY_PROPERTIES  (4 + 2)

/*ci
 * \brief Array index for the object_id in a BuiltinTopicKey_t
 */
#define DDS_BUILTIN_TOPIC_KEY_OBJECT_ID 3

/*ci
 * \brief Determine if an object id is a built-in topic or
 *        a user-defined topic
 *
 * \param[in] oid Object id to test
 *
 * \return DDS_BOOLEAN_FALSE if the test is false, DDS_BOOLEAN_TRUE otherwise
 */
DDSCDllExport DDS_Boolean
DDS_ObjectId_is_builtin(DDS_UnsignedLong oid);

/*ci
 * \brief Determine if a DDS_BuiltinTopicKey_t is a built-in topic or
 *        a user-defined topic
 *
 * \param[in] key DDS_BuiltinTopicKey_t to test
 *
 * \return DDS_BOOLEAN_FALSE if the test is false, DDS_BOOLEAN_TRUE otherwise
 */
DDSCDllExport DDS_Boolean
DDS_BuiltinTopicKey_is_builtin(const struct DDS_BuiltinTopicKey_t *const key);

DDSCDllExport DDS_Boolean
DDS_ObjectId_is_compatible(DDS_UnsignedLong dr_id,DDS_UnsignedLong dw_id);

DDSCDllExport DDS_Boolean
DDS_ObjectId_is_writer(DDS_UnsignedLong object_id);

/*e \dref_ParticipantBuiltInTopicGroupDocs
 */

/*e \dref_ParticipantBuiltinTopicData
 */
struct DDSCPPDllExport DDS_ParticipantBuiltinTopicData
{
    /*e \dref_ParticipantBuiltinTopicData_key
     */
    struct DDS_BuiltinTopicKey_t key;

    /*e \dref_ParticipantBuiltinTopicData_participant_name
     */
    struct DDS_EntityNameQosPolicy participant_name;

    /*e \dref_ParticipantBuiltinTopicData_dds_builtin_endpoints
     */
    DDS_UnsignedLong dds_builtin_endpoints;

    /*e \dref_ParticipantBuiltinTopicData_rtps_protocol_version
     */
    DDS_ProtocolVersion_t rtps_protocol_version;

    /*e \dref_ParticipantBuiltinTopicData_rtps_vendor_id
     */
    struct DDS_VendorId_t rtps_vendor_id;

    /*e \dref_ParticipantBuiltinTopicData_default_unicast_locators
     */
    struct DDS_LocatorSeq default_unicast_locators;

    /*e \dref_ParticipantBuiltinTopicData_default_multicast_locators
     */
    struct DDS_LocatorSeq default_multicast_locators;

    /*e \dref_ParticipantBuiltinTopicData_metatraffic_unicast_locators
     */
    struct DDS_LocatorSeq metatraffic_unicast_locators;

    /*e \dref_ParticipantBuiltinTopicData_metatraffic_multicast_locators
     */
    struct DDS_LocatorSeq metatraffic_multicast_locators;

    /*e \dref_ParticipantBuiltinTopicData_liveliness_lease_duration
     */
    struct DDS_Duration_t liveliness_lease_duration;

    /*e \dref_ParticipantBuiltinTopicData_product_version
     */
    struct DDS_ProductVersion_t product_version;

    /*e \dref_ParticipantBuiltinTopicData_checksum
     */
    struct DDS_ChecksumProperty checksum;

    /*e \dref_ParticipantBuiltinTopicData_property
     */
    struct DDS_PropertyQosPolicy property;

    /*e \dref_ParticipantBuiltinTopicData_user_data
     */
    struct DDS_UserDataQosPolicy user_data;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /*i \dref_ParticipantBuiltinTopicData_participant_message_reader_reliability_kind
     */
    DDS_ReliabilityQosPolicyKind participant_message_reader_reliability_kind;

    /*i \dref_ParticipantBuiltinTopicData_builtin_endpoint_qos_mask
     */
    DDS_BuiltinEndpointQos_t builtin_endpoint_qos_mask;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    DDSC_CPP_BUILTINTOPICDATA_METHODS(DDS_ParticipantBuiltinTopicData)

#ifdef RTI_CPP
    public:
        DDS_ParticipantBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);
#endif /* RTI_CPP */
};

#ifdef RTI_CPP
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_ParticipantBuiltinTopicDataCpp
 *
 * \brief A Constructor that uses the domain participants resource limits to
 *         initialize the object memory.
 *
 * \param[in] dp_qos The resource limits to initialize the object.
 */
DDS_ParticipantBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);

/*e \dref_ParticipantBuiltinTopicDataCpp
 *
 * \brief Default constructor.
 *
 */
DDS_ParticipantBuiltinTopicData();
#endif /*DOXYGEN_DOCUMENTATION_ONLY */
#endif /*RTI_CPP */

DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize(struct DDS_ParticipantBuiltinTopicData *self);

DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize_shallow(
                                struct DDS_ParticipantBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos);

/*ce \dref_ParticipantBuiltinTopicData_initialize_from_qos
 */
DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize_from_qos(
        struct DDS_ParticipantBuiltinTopicData *self,
        const struct DDS_DomainParticipantQos *dp_qos);

#ifndef RTI_CERT
/*e \dref_ParticipantBuiltinTopicData_finalize
 */
DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize(struct DDS_ParticipantBuiltinTopicData *self);
#endif

/*ce \dref_ParticipantBuiltinTopicData_copy
 */
DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_copy(
        struct DDS_ParticipantBuiltinTopicData *self,
        const struct DDS_ParticipantBuiltinTopicData *source);

/*e \dref_ParticipantBuiltinTopicData_is_equal
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_is_equal(
        const struct DDS_ParticipantBuiltinTopicData *left,
        const struct DDS_ParticipantBuiltinTopicData *right);

DDSCDllExport DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(
                                struct DDS_ParticipantBuiltinTopicData *self,
                                DDS_DomainParticipant *participant);


#if DDS_LIVELINESS_CHANNEL_ENABLED
#define DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER \
           ,DDS_BEST_EFFORT_RELIABILITY_QOS

#define DDS_BUILTIN_ENDPOINT_QOS_MASK \
           ,DDS_BUILTIN_ENDPOINT_QOS_NONE
#else
#define DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER
#define DDS_BUILTIN_ENDPOINT_QOS_MASK
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

/*i \dref_DDS_ParticipantBuiltinTopicData_INITIALIZER
 */
#define DDS_ParticipantBuiltinTopicData_INITIALIZER { \
    DDS_BuiltinTopicKey_t_INITIALIZER, \
    DDS_ENTITY_NAME_QOS_POLICY_DEFAULT, \
    0, /* builtin endpoints */ \
    DDS_PROTOCOL_VERSION_DEFAULT, \
    DDS_VENDOR_ID_DEFAULT, \
    DDS_SEQUENCE_INITIALIZER, \
    DDS_SEQUENCE_INITIALIZER, \
    DDS_SEQUENCE_INITIALIZER, \
    DDS_SEQUENCE_INITIALIZER, \
    {100L,0L}, /* participant_liveliness_lease_duration */\
    DDS_PRODUCTVERSION_UNKNOWN, \
    DDS_ChecksumPropertyWireProtocol_INITIALIZER, \
    DDS_PROPERTY_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT \
    DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER \
    DDS_BUILTIN_ENDPOINT_QOS_MASK \
}

#define T struct DDS_ParticipantBuiltinTopicData
#define TSeq DDS_ParticipantBuiltinTopicDataSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define DDS_ParticipantBuiltinTopicDataSeq_INITIALIZER \
DDS_SEQUENCE_INITIALIZER

/*i
 * \brief Create a new DDS_ParticipantBuiltinTopicData sample.
 *
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_create_sample(
        struct NDDS_Type_Plugin* plugin,
        void **sample);

/*i
 * \brief Delete a DDS_ParticipantBuiltinTopicData sample.
 *
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_delete_sample(
        struct NDDS_Type_Plugin *plugin,
        void *sample);

/*i
 * \brief Retrieve a reference to the type plugin for
 * DDS_ParticipantBuiltinTopicData.
 *
 */
MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePluginI*
DDS_ParticipantBuiltinTopicDataTypePlugin_get(void);

MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant);

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePlugin*
DDS_ParticipantBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_size(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            RTI_UINT32 current_alignment);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_ParticipantBuiltinTopicDataTypePlugin_serialize(struct DDS_TypePlugin *plugin,
                                                     struct CDR_Stream_t *stream,
                                                     const void *data,
                                                     DDS_InstanceHandle_t *destination);

/*e \dref_PublicationBuiltInTopicGroupDocs
 */

/*e \dref_PublicationBuiltinTopicData
 */
struct DDSCPPDllExport DDS_PublicationBuiltinTopicData
{
    /*e \dref_PublicationBuiltinTopicData_key
     */
    struct DDS_BuiltinTopicKey_t key;

    /*e \dref_PublicationBuiltinTopicData_participant_key
     */
    struct DDS_BuiltinTopicKey_t participant_key;

    /*e \dref_PublicationBuiltinTopicData_topic_name
     */
    char *topic_name;

    /*e \dref_PublicationBuiltinTopicData_type_name
     */
    char *type_name;

    /*e \dref_PublicationBuiltinTopicData_deadline
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*e \dref_PublicationBuiltinTopicData_ownership
     */
    struct DDS_OwnershipQosPolicy ownership;

    /*e \dref_PublicationBuiltinTopicData_ownership_strength
     */
    struct DDS_OwnershipStrengthQosPolicy ownership_strength;

    /*e \dref_DataReaderQos_latency_budget
     */
    struct DDS_LatencyBudgetQosPolicy latency_budget;

    /*e \dref_PublicationBuiltinTopicData_reliability
     */
    struct DDS_ReliabilityQosPolicy reliability;

    /*e \dref_PublicationBuiltinTopicData_liveliness
     */
    struct DDS_LivelinessQosPolicy liveliness;

    /*e \dref_PublicationBuiltinTopicData_durability
     */
    struct DDS_DurabilityQosPolicy durability;

    /*e \dref_PublicationBuiltinTopicData_destination_order
     */
    struct DDS_DestinationOrderQosPolicy destination_order;

    /*e \dref_PublicationBuiltinTopicData_unicast_locator
     */
    struct DDS_LocatorSeq unicast_locator;

    /*e \dref_PublicationBuiltinTopicData_representation
     */
    struct DDS_DataRepresentationQosPolicy representation;

    /*e \dref_PublicationBuiltinTopicData_partition
     */
    struct DDS_PartitionQosPolicy partition;

    /*e \dref_PublicationBuiltinTopicData_user_data
     */
    struct DDS_UserDataQosPolicy user_data;

    /*e \dref_PublicationBuiltinTopicData_group_data
     */
    struct DDS_GroupDataQosPolicy group_data;

    /*e \dref_PublicationBuiltinTopicData_topic_data
     */
    struct DDS_TopicDataQosPolicy topic_data;

























    DDSC_CPP_BUILTINTOPICDATA_METHODS(DDS_PublicationBuiltinTopicData)

#ifdef RTI_CPP
    public:
        DDS_PublicationBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);
#endif /* RTI_CPP */
};

#ifdef RTI_CPP
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_PublicationBuiltinTopicDataCpp
 *
 * \brief A Constructor that uses the domain participants resource limits to
 *         initialize the object memory.
 *
 * \param[in] dp_qos The resource limits to initialize the object.
 */
DDS_PublicationBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);

/*e \dref_PublicationBuiltinTopicDataCpp
 *
 * \brief Default constructor.
 *
 */
DDS_PublicationBuiltinTopicData();
#endif /*DOXYGEN_DOCUMENTATION_ONLY */
#endif /*RTI_CPP */

DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize(struct DDS_PublicationBuiltinTopicData *self);

DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize_shallow(
                                struct DDS_PublicationBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos);

/*ce \dref_PublicationBuiltinTopicData_initialize_from_qos
 */
DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize_from_qos(
        struct DDS_PublicationBuiltinTopicData *self,
        const struct DDS_DomainParticipantQos *dp_qos);

#ifndef RTI_CERT
/*e \dref_PublicationBuiltinTopicData_finalize
 */
DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize(struct DDS_PublicationBuiltinTopicData *self);
#endif

/*ce \dref_PublicationBuiltinTopicData_copy
 */
DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_copy(
        struct DDS_PublicationBuiltinTopicData *self,
        const struct DDS_PublicationBuiltinTopicData *source);

/*e \dref_PublicationBuiltinTopicData_is_equal
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_is_equal(
        const struct DDS_PublicationBuiltinTopicData *left,
        const struct DDS_PublicationBuiltinTopicData *right);

DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize_no_dealloc(
                            struct DDS_PublicationBuiltinTopicData *self,
                            DDS_DomainParticipant *participant);

/*ci
 * \brief Construct a DDS_PublicationBuiltinTopicData from a DDS_DataWriter
 *
 * \param[out] data_out   The data to be copied into
 * \param[in]  datawriter The datawriter to copy from
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE otherwise
 */
DDSCDllExport DDS_Boolean
DDS_PublicationBuiltinTopicData_copy_from_datawriter(
        struct DDS_PublicationBuiltinTopicData* data_out,
        DDS_DataWriter *datawriter);











#define DDS_TRUST_PUBLICATION_DATA_INITIALIZER


/*i \dref_PublicationBuiltinTopicData_INITIALIZER
 */
#define DDS_PublicationBuiltinTopicData_INITIALIZER { \
    DDS_BuiltinTopicKey_t_INITIALIZER, \
    DDS_BuiltinTopicKey_t_INITIALIZER, \
    NULL,\
    NULL,\
    DDS_DEADLINE_QOS_POLICY_DEFAULT, \
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT,\
    DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT, \
    DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT,\
    DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT,\
    DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
    DDS_DURABILITY_QOS_POLICY_DEFAULT, \
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT,\
    DDS_SEQUENCE_INITIALIZER, \
    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
    DDS_PARTITION_QOS_POLICY_DEFAULT,\
    DDS_USER_DATA_QOS_POLICY_DEFAULT, \
    DDS_GROUP_DATA_QOS_POLICY_DEFAULT, \
    DDS_TOPIC_DATA_QOS_POLICY_DEFAULT \
    DDS_TRUST_PUBLICATION_DATA_INITIALIZER\
}

#define T struct DDS_PublicationBuiltinTopicData
#define TSeq DDS_PublicationBuiltinTopicDataSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define DDS_PublicationBuiltinTopicDataSeq_INITIALIZER \
DDS_SEQUENCE_INITIALIZER

MUST_CHECK_RETURN extern RTI_BOOL
DDS_PublicationBuiltinTopicDataTypePlugin_create_sample(
        struct NDDS_Type_Plugin *plugin,
        void **sample,
        void *param);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_PublicationBuiltinTopicDataTypePlugin_delete_sample(
        struct NDDS_Type_Plugin *plugin,
        void *sample,
        void *param);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_PublicationBuiltinTopicDataTypePlugin_copy_sample(
        struct NDDS_Type_Plugin *type,
        void *dst,
        const void *src,
        void *param);

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePluginI*
DDS_PublicationBuiltinTopicDataTypePlugin_get(void);

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePlugin*
DDS_PublicationBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant);

/*e \dref_SubscriptionBuiltInTopicGroupDocs
 */

/*e \dref_SubscriptionBuiltinTopicData
 */
struct DDSCPPDllExport DDS_SubscriptionBuiltinTopicData
{
    /*e \dref_SubscriptionBuiltinTopicData_key
     * Key GUID must be first
     */
    struct DDS_BuiltinTopicKey_t key;

    /*e \dref_SubscriptionBuiltinTopicData_participant_key
     */
    struct DDS_BuiltinTopicKey_t participant_key;

    /*e \dref_SubscriptionBuiltinTopicData_topic_name
     */
    char *topic_name;

    /*e \dref_SubscriptionBuiltinTopicData_type_name
     */
    char *type_name;

    /*e \dref_SubscriptionBuiltinTopicData_deadline
     */
    struct DDS_DeadlineQosPolicy deadline;

    /*e \dref_SubscriptionBuiltinTopicData_ownership
     */
    struct DDS_OwnershipQosPolicy ownership;

    /*e \dref_DataReaderQos_latency_budget
     */
    struct DDS_LatencyBudgetQosPolicy latency_budget;

    /*e \dref_SubscriptionBuiltinTopicData_reliability
     */
    struct DDS_ReliabilityQosPolicy reliability;

    /*e \dref_SubscriptionBuiltinTopicData_liveliness
     */
    struct DDS_LivelinessQosPolicy liveliness;

    /*e \dref_SubscriptionBuiltinTopicData_durability
     */
    struct DDS_DurabilityQosPolicy durability;

    /*e \dref_SubscriptionBuiltinTopicData_destination_order
     */
    struct DDS_DestinationOrderQosPolicy destination_order;

    /*e \dref_SubscriptionBuiltinTopicData_unicast_locator
     */
    struct DDS_LocatorSeq unicast_locator;

    /*e \dref_SubscriptionBuiltinTopicData_multicast_locator
     */
    struct DDS_LocatorSeq multicast_locator;

    /*e \dref_SubscriptionBuiltinTopicData_presentation
     */
    struct DDS_PresentationQosPolicy presentation;

    /*e \dref_SubscriptionBuiltinTopicData_representation
     */
    struct DDS_DataRepresentationQosPolicy representation;

    /*e \dref_SubscriptionBuiltinTopicData_partition
     */
    struct DDS_PartitionQosPolicy partition;

    /*e \dref_SubscriptionBuiltinTopicData_user_data
     */
    struct DDS_UserDataQosPolicy user_data;

    /*e \dref_SubscriptionBuiltinTopicData_group_data
     */
    struct DDS_GroupDataQosPolicy group_data;

    /*e \dref_SubscriptionBuiltinTopicData_topic_data
     */
    struct DDS_TopicDataQosPolicy topic_data;

#if DDS_FILTERING_ENABLED
    /*e \dref_SubscriptionBuiltinTopicData_content_filter
     */
    struct DDS_ContentFilterProperty content_filter;
#endif





























    DDSC_CPP_BUILTINTOPICDATA_METHODS(DDS_SubscriptionBuiltinTopicData)

#ifdef RTI_CPP
    public:
        DDS_SubscriptionBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);
#endif /* RTI_CPP */
};

#ifdef RTI_CPP
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_SubscriptionBuiltinTopicDataCpp
 *
 * \brief A Constructor that uses the domain participants resource limits to
 *         initialize the object memory.
 *
 * \param[in] dp_qos The resource limits to initialize the object.
 */
DDS_SubscriptionBuiltinTopicData(const DDS_DomainParticipantQos &dp_qos);

/*e \dref_SubscriptionBuiltinTopicDataCpp
 *
 * \brief Default constructor.
 *
 */
DDS_SubscriptionBuiltinTopicData();
#endif /*DOXYGEN_DOCUMENTATION_ONLY */
#endif /*RTI_CPP */

DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize(struct DDS_SubscriptionBuiltinTopicData *self);

DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize_shallow(
                                struct DDS_SubscriptionBuiltinTopicData *self,
                                DDS_DomainParticipant *participant);

/*ce \dref_SubscriptionBuiltinTopicData_initialize_from_qos
 */
DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize_from_qos(
        struct DDS_SubscriptionBuiltinTopicData *self,
        const struct DDS_DomainParticipantQos *dp_qos);

#ifndef RTI_CERT
/*e \dref_SubscriptionBuiltinTopicData_finalize
 */
DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize(struct DDS_SubscriptionBuiltinTopicData *self);
#endif

/*ce \dref_SubscriptionBuiltinTopicData_copy
 */
DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_copy(
        struct DDS_SubscriptionBuiltinTopicData *self,
        const struct DDS_SubscriptionBuiltinTopicData *source);

/*e \dref_SubscriptionBuiltinTopicData_is_equal
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_is_equal(
        const struct DDS_SubscriptionBuiltinTopicData *left,
        const struct DDS_SubscriptionBuiltinTopicData *right);

/*ci
 * \brief Check if the immutable fields of two DDS_SubscriptionBuiltinTopicData are equal
 *
 * \param[in] left  The first DDS_SubscriptionBuiltinTopicData
 * \param[in] right The second DDS_SubscriptionBuiltinTopicData
 *
 * \return DDS_BOOLEAN_TRUE if the immutable fields are equal, DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_immutable_is_equal(
        const struct DDS_SubscriptionBuiltinTopicData *left,
        const struct DDS_SubscriptionBuiltinTopicData *right);

DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(
                            struct DDS_SubscriptionBuiltinTopicData *self,
                            DDS_DomainParticipant *participant);

/*ci
 * \brief Construct a DDS_SubscriptionBuiltinTopicData from a DDS_DataReader
 *
 * \param[out] data_out   The data to be copied into
 * \param[in]  datareader The datareader to copy from
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE otherwise
 */
DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_copy_from_datareader(
        struct DDS_SubscriptionBuiltinTopicData* data_out,
        DDS_DataReader *datareader);

#if DDS_FILTERING_ENABLED
#define DDS_CONTENT_FILTER_SUBSCRIPTION_DATA_INITIALIZER \
        ,DDS_CONTENT_FILTER_PROPERTY_DEFAULT
#else
#define DDS_CONTENT_FILTER_SUBSCRIPTION_DATA_INITIALIZER
#endif










#define DDS_TRUST_SUBSCRIPTION_DATA_INITIALIZER


/*i \dref_DDS_SubscriptionBuiltinTopicData_INITIALIZER
 */
#define DDS_SubscriptionBuiltinTopicData_INITIALIZER { \
    DDS_BuiltinTopicKey_t_INITIALIZER, \
    DDS_BuiltinTopicKey_t_INITIALIZER, \
    NULL,\
    NULL,\
    DDS_DEADLINE_QOS_POLICY_DEFAULT, \
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT ,\
    DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT,\
    DDS_DATAREADER_RELIABILITY_QOS_POLICY_DEFAULT,\
    DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
    DDS_DURABILITY_QOS_POLICY_DEFAULT, \
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT,\
    DDS_SEQUENCE_INITIALIZER,\
    DDS_SEQUENCE_INITIALIZER,\
    DDS_PRESENTATION_QOS_POLICY_DEFAULT, \
    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
    DDS_PARTITION_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT, \
    DDS_GROUP_DATA_QOS_POLICY_DEFAULT, \
    DDS_TOPIC_DATA_QOS_POLICY_DEFAULT \
    DDS_CONTENT_FILTER_SUBSCRIPTION_DATA_INITIALIZER \
    DDS_TRUST_SUBSCRIPTION_DATA_INITIALIZER \
}

#define T struct DDS_SubscriptionBuiltinTopicData
#define TSeq DDS_SubscriptionBuiltinTopicDataSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define DDS_SubscriptionBuiltinTopicDataSeq_INITIALIZER \
DDS_SEQUENCE_INITIALIZER

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_SubscriptionBuiltinTopicDataTypePlugin_create_sample(
        struct NDDS_Type_Plugin *plugin,
        void **sample,
        void *param);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_SubscriptionBuiltinTopicDataTypePlugin_delete_sample(
        struct NDDS_Type_Plugin *plugin,
        void *sample,
        void *param);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_SubscriptionBuiltinTopicDataTypePlugin_copy_sample(
        struct NDDS_Type_Plugin *type,
        void *dst,
        const void *src,
        void *param);

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePluginI*
DDS_SubscriptionBuiltinTopicDataTypePlugin_get(void);

MUST_CHECK_RETURN DDSCDllExport struct DDS_TypePlugin*
DDS_SubscriptionBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant);

struct NDDS_RemoteEntityImpl;
typedef struct NDDS_RemoteEntityImpl NDDS_RemoteEntity;
struct DDS_RemotePublicationImpl;
typedef struct DDS_RemotePublicationImpl DDS_RemotePublication;
typedef struct DDS_RemoteSubscriptionImpl DDS_RemoteSubscription;
typedef struct DDS_RemoteParticipantImpl DDS_RemoteParticipant;

typedef DDS_UnsignedLong DDS_RemoteParticipantStatusMask;

#define DDS_REMOTE_PARTICIPANT_STATUS_DEFAULT \
            ((DDS_RemoteParticipantStatusMask)0)

/*ci
 * \brief Flag to indicate that the remote participant has
 * been just asserted for the first time.
 */
#define DDS_REMOTE_PARTICIPANT_STATUS_NEW                           (0x01U << 0)

/*ci
 * \brief Flag to indicate that the remote participant has
 * been enabled.
 */
#define DDS_REMOTE_PARTICIPANT_STATUS_ENABLED                       (0x01U << 1)

/*ci
 * \brief Flag to indicate that the remote participant has
 * been asserted statically.
 */
#define DDS_REMOTE_PARTICIPANT_STATUS_STATIC                        (0x01U << 2)












































































































typedef DDS_UnsignedLong DDS_ParticipantDiscoveryStatus_T;
























/*ci
 * \brief Assert a remote participant into a participant
 *
 * \details
 * Assert a remote participant into the participant. This is typically
 * done as part of the discovery process where a discovery plugin has
 * discovered a remote participant via some means. It is legal to assert a
 * remote participant multiple times.
 *
 * If the remote participant does not yet exist in the participant, flag
 * DDS_REMOTE_PARTICIPANT_STATUS_NEW will be set in status, or unset otherwise.
 *
 *
 * Note that asserting a remote participant does not automatically enable it.
 * Either \ref NDDS_DomainParticipant_enable_remote_participant_name or
 * \ref NDDS_DomainParticipant_enable_remote_participant_guid must be called
 * for that.
 *
 * Matching of a remote participant's endpoints with local endpoints
 * does not occur until the remote participant is enabled.
 *
 * \param[in]    participant The participant to assert the remote participant in
 * \param[in]    data        Discovery data for the remote participant
 * \param[inout] status      A status mask containg information about the
 *                           asserted participant.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_participant(
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const data,
        DDS_RemoteParticipantStatusMask *status);

MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_create_remote_participant(
        DDS_DomainParticipant *const self,
        struct DDS_ParticipantBuiltinTopicData *const data,
        RTI_BOOL assign_guid,
        RTI_UINT32 assigned_oid,
        struct DDS_RemoteParticipantImpl **record_out);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
NDDS_DomainParticipant_delete_remote_participant_routes(
        struct DDS_DomainParticipantImpl *participant,
        struct DDS_RemoteParticipantImpl *record);

/*ci
 * \brief Refresh the liveliness for a remote participant
 *
 * \details
 * This function informs the participant that the remote participant with
 * a key is still alive. How this is determined is outside the scope of this
 * function. If a remote participant fails to refresh its liveliness it will
 * be reset/removed from the participant.
 *
 * \param[in] participant The participant to refresh the liveliness in
 * \param[in] key         The key of the remote participant that has refreshed
 *                        its liveliness.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_refresh_remote_participant_liveliness(
        DDS_DomainParticipant * const participant,
        const struct DDS_BuiltinTopicKey_t *const key);

/*ci
 * \brief Reset a remote participant from a participant
 *
 * \details
 * This function resets a remote participant and all its endpoints in the
 * participant. When the remote endpoints are reset they are also unmatched
 * from local endpoints. However, the remote participants and remote endpoints
 * are not removed from the participant. This feature is typically used in
 * static discovery where remote participants and endponts are statically
 * asserted. After they have been reset they can be re-enabled with one the
 * remote participant enable functions
 * \ref NDDS_DomainParticipant_enable_remote_participant_guid or
 * \ref NDDS_DomainParticipant_enable_remote_participant_name
 *
 * \param[in] participant The participant to remove the remote participant from
 * \param[in] key         The key of the remote participant to remove
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_reset_remote_participant(
        DDS_DomainParticipant *const participant,
        const struct DDS_BuiltinTopicKey_t *const key);

#ifndef RTI_CERT
/*ci
 * \brief Remove a remote participant from a participant
 *
 * \details
 * This function removes a remote participant and all its endpoints from the
 * participant. When the remote endpoints are removed they are also unmatched
 * from local endpoints.
 *
 * \param[in] participant The participant to remove the remote participant from
 * \param[in] key         The key of the remote participant to remove
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
SHOULD_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_remove_remote_participant(
        DDS_DomainParticipant *const participant,
        const DDS_BuiltinTopicKey_t *const key);
#endif /* !RTI_CERT */

/*ci
 * \brief Enable a remote participant by name
 *
 * \details
 * When a remote participant is asserted it is initially in a disabled state.
 * Any endpoint asserted as part of the remote participant is also disabled
 * and no matching occur. This function enables a remote participant which
 * causes matching with local entities. In addition the liveliness timer
 * is started and the remote participant must maintain its liveliness to
 * avoid being reset/removed. This function is called when a participant has
 * been asserted statically and is only known by name. On successful
 * return the remote participant has been updated with a GUID (taken from the
 * data input argument) and all its remote endpoints have been matched with
 * local endpoints.
 *
 * \param[in] participant The participant to enable the remote participant in
 * \param[in] data        The discovery data to enable the remote participant
 *                        with
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_enable_remote_participant_name(
        DDS_DomainParticipant * const participant,
        const struct DDS_ParticipantBuiltinTopicData *const data);

/*ci
 * \brief Enable a remote participant by GUID
 *
 * \details
 * When a remote participant is asserted it is initially in a disabled state.
 * Any endpoint asserted as part of the remote participant is also disabled
 * and no matching occur. This function enables a remote participant which
 * causes matching with local entities. In addition the liveliness timer
 * is started and the remote participant must maintain its liveliness to
 * avoid being reset/removed. This function is called when participant has
 * been detected based on discovery data with a valid GUID. On successful
 * return the remote participant is enabled and all its remote endpoints have
 * been matched with local endpoints.
 *
 * \param[in] participant The participant to enable the remote participant in
 * \param[in] data        The discovery data to enable the remote participant
 *                        with
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_enable_remote_participant_guid(
         DDS_DomainParticipant * const participant,
         const struct DDS_ParticipantBuiltinTopicData *const data);

/*ci
 * \brief Assert a remote publication into a participant
 *
 * \details
 * This function adds a remote publication as an endpoint in a remote
 * participant. Either the name or the GUID of the participant must be
 * specified, but not both. The name is used for static discovery
 * when the GUID of the participant is not known, while the participant
 * key in the data input is used for dynamic discovery.
 *
 * If the remote participant is already enabled the remote publication is
 * also enabled automatically. Otherwise the remote publication is created
 * in a disabled state and enabled when the remote participant is enabled.
 *
 * \param[in] participant The participant to assert the remote publication in
 * \param[in] participant_name The name of the remote publication's parent
 * \param[in] data        Discovery data for the remote publication
 * \param[in] key_kind    The type of key for the topic published by the remote
 *                        publication
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_publication(
        DDS_DomainParticipant *const participant,
        const char *const participant_name,
        const struct DDS_PublicationBuiltinTopicData *const data,
       NDDS_TypePluginKeyKind key_kind);

/*ci
 * \brief Assert a remote subscription into a participant
 *
 * \details
 * This function adds a remote subscription as an endpoint in a remote
 * participant. Either the name or the GUID of the participant must be
 * specified, but not both. The name is typically used for static discovery
 * when the GUID of the participant is not known, while the participant
 * key in the data input is used for dynamic discovery.
 *
 * If the remote participant is already enabled the remote subscription is
 * also enabled automatically. Otherwise the remote subscription is created
 * in a disabled state and enabled when the remote participant is enabled.
 *
 * \param[in] participant The participant to assert the remote publication in
 * \param[in] participant_name The name of the remote subscription's parent
 * \param[in] data        Discovery data for the remote subscription
 * \param[in] key_kind    The type of key for the topic subscribed to by the
 *                        remote subscription
 * \param[out] is_queued  If DDS_RETCODE_OK is returned and is_queued is
 *                        DDS_BOOLEAN_TRUE, the message was queued.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_assert_or_queue_remote_publication(
                    DDS_DomainParticipant *const participant,
                    const char *const participant_name,
                    struct DDS_PublicationBuiltinTopicData *data,
                    struct DDS_SampleInfo *info,
                    NDDS_TypePluginKeyKind key_kind,
                    DDS_Boolean *is_queued);

#ifndef RTI_CERT
/*ci
 * \brief Remove a remote publication from a participant
 *
 * \details
 * This function removes a remote publication as an endpoint from a remote
 * participant. It is unmatched from local endpoints before removal.
 *
 * \param[in] participant The participant to remove the remote publication from
 * \param[in] key         The key for the remote publication that is removed
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_remove_remote_publication(
        DDS_DomainParticipant *const participant,
        const DDS_BuiltinTopicKey_t *const key);
#endif /* !RTI_CERT */

/*ci
 * \brief Assert a remote subscription into a participant
 *
 * \details
 * This function adds a remote subscription as an endpoint in a remote
 * participant. Either the name or the GUID of the participant must be
 * specified, but not both. The name is typically used for static discovery
 * when the GUID of the participant is not known, while the participant
 * key in the data input is used for dynamic discovery.
 *
 * If the remote participant is already enabled the remote subscription is
 * also enabled automatically. Otherwise the remote subscription is created
 * in a disabled state and enabled when the remote participant is enabled.
 *
 * \param[in] participant The participant to assert the remote publication in
 * \param[in] participant_name The name of the remote subscription's parent
 * \param[in] data        Discovery data for the remote subscription
 * \param[in] key_kind    The type of key for the topic subscribed to by the
 *                        remote subscription
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_subscription(
        DDS_DomainParticipant* const participant,
        const char *const participant_name,
        const struct DDS_SubscriptionBuiltinTopicData *const data,
        NDDS_TypePluginKeyKind key_kind);

/*ci
 * \brief Assert a remote subscription into a participant
 *
 * \details
 * This function adds a remote subscription as an endpoint in a remote
 * participant. Either the name or the GUID of the participant must be
 * specified, but not both. The name is typically used for static discovery
 * when the GUID of the participant is not known, while the participant
 * key in the data input is used for dynamic discovery.
 *
 * If the remote participant is already enabled the remote subscription is
 * also enabled automatically. Otherwise the remote subscription is created
 * in a disabled state and enabled when the remote participant is enabled.
 *
 * \param[in] participant The participant to assert the remote publication in
 * \param[in] participant_name The name of the remote subscription's parent
 * \param[in] data        Discovery data for the remote subscription
 * \param[in] key_kind    The type of key for the topic subscribed to by the
 *                        remote subscription
 * \param[out] is_queued  If DDS_RETCODE_OK is returned and is_queued is
 *                        DDS_BOOLEAN_TRUE, the message was queued.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_assert_or_queue_remote_subscription(
                    DDS_DomainParticipant *const participant,
                    const char *const participant_name,
                    struct DDS_SubscriptionBuiltinTopicData *data,
                    struct DDS_SampleInfo *info,
                    NDDS_TypePluginKeyKind key_kind,
                    DDS_Boolean *is_queued);

#ifndef RTI_CERT
/*ci
 * \brief Remove a remote subscription from a participant
 *
 * \details
 * This function removes a remote subscription as an endpoint from a remote
 * participant. It is unmatched from local endpoints before removal.
 *
 * \param[in] participant The participant to remove the remote subscription from
 * \param[in] key         The key for the remote subscription that is removed
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
NDDS_DomainParticipant_remove_remote_subscription(
        DDS_DomainParticipant* const participant,
        const DDS_BuiltinTopicKey_t* const key);
#endif /* !RTI_CERT */


/*ci
 * \brief Remove a remote DDS DataWriter as a peer for a local DDS datareader
 *
 * \param[in] datareader The local datareader
 * \param[in] dp_key     The peer's participant key
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataReader_remove_peer(DDS_DataReader *const datareader,
                           const DDS_BuiltinTopicKey_t *const dp_key,
                           DDS_UnsignedLong entity_id);

/*ci
 * \brief Remove remote routes to the specified participant
 *
 * \param[in] datawriter The participant announcement writer
 * \param[in] dp_key     The remote participant key
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_remove_participant_routes(DDS_DataWriter *const datawriter,
                                 const DDS_BuiltinTopicKey_t *const dp_key);

/*ci
 * \brief Remove a remote DDS DataReader as a peer for a local DDS datawriter
 *
 * \param[in] datawriter The local datawriter
 * \param[in] dp_key     The peer's participant key
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_remove_peer(DDS_DataWriter *const datawriter,
                           const DDS_BuiltinTopicKey_t *const dp_key,
                           DDS_UnsignedLong entity_id);

/*ci
 * \brief Remove all remote DDS DataWriters with the entity_id as a peer
 *        from all discovered participants
 *
 * \param[in] datareader The local datareader
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataReader_remove_all_peers(DDS_DataReader *const datareader,
                                DDS_UnsignedLong entity_id);

/*ci
 * \brief Remove all remote DDS DataReaders with the entity_id as a peer
 *        from all discovered participants
 *
 * \param[in] datawriter The local datawriter
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_remove_all_peers(DDS_DataWriter *const datawriter,
                                DDS_UnsignedLong entity_id);

/*ci
 * \brief Add a remote DDS DataReader with the entity_id as a peer to a local
 *        DDS DataWriter
 *
 * \param[in] datawriter The local datawriter
 * \param[in] dp_key     The peer's participant key
 * \param[in] dr_qos     The peer's Qos policy
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_add_peer(DDS_DataWriter *const datawriter,
                        const DDS_BuiltinTopicKey_t *const dp_key,
                        const struct DDS_DataReaderQos *const dr_qos,
                        DDS_UnsignedLong entity_id);

/*ci
 * \brief Add a remote DDS DataWriter with the entity_id as a peer to a local
 *        DDS Datareader
 *
 * \param[in] datareader The local datareader
 * \param[in] dp_key     The peer's participant key
 * \param[in] dw_qos     The peer's Qos policy
 * \param[in] entity_id  The peer's entity id (The prefix is the same as dp_key)
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DataReader_add_peer(DDS_DataReader *const datareader,
                        const DDS_BuiltinTopicKey_t *const dp_key,
                        const struct DDS_DataWriterQos *const dw_qos,
                        DDS_UnsignedLong entity_id);
#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_discovery_h */
