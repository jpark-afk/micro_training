/*
 * FILE: BuiltinCdr.h - Exported Builtin CDR functions
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
 */
#ifndef BuiltinCdr_pkg_h
#define BuiltinCdr_pkg_h

#include "dds_c/dds_c_common.h"
#include "dds_c/dds_c_string_manager.h"
#include "dds_c/dds_c_user_data_manager.h"

/*e \ingroup DISCRtpsPidModule
  Used for field Participant::productVersion
 */
#define DISC_RTPS_PID_PRODUCT_VERSION                            (0x8000)

#define DISC_RTPS_PID_ENTITY_NAME   RTPS_PID_ENTITY_NAME


MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrStream_serialize_4_byte_parameter(struct CDR_Stream_t *stream,
                                   const void *in,
                                   DDS_UnsignedShort parameterId);

extern DDS_UnsignedLong
DDS_Cdr_get_header_max_size_serialized(DDS_UnsignedLong size);

typedef RTI_BOOL(*DDS_Cdr_DeserializeParameterValueFunction)
    (RTI_BOOL * ok, void *parameters, struct CDR_Stream_t * stream,
     unsigned short parameterId, unsigned short parameterLength,void *param);

typedef void (*DDS_Cdr_SetDefaultParameterValuesFunction)
    (void *parameter);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrStream_deserialize_parameter_sequence(void *parameter,
                                     struct CDR_Stream_t *stream,
                                     DDS_Cdr_SetDefaultParameterValuesFunction
                                     setDefaultParameterValuesFnc,
                                     DDS_Cdr_DeserializeParameterValueFunction
                                     deserializeParameterValueFnc,void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_entity_name(
    struct CDR_Stream_t *stream,
    const struct DDS_EntityNameQosPolicy *entityName,
    void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_entity_name(
    struct CDR_Stream_t *stream,
    struct DDS_EntityNameQosPolicy *entityName,
    void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_entity_name(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_topic_name(struct CDR_Stream_t *stream,
                                    const char *topic_name,
                                    void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_topic_name(struct CDR_Stream_t *stream,
                                        char **topic_name,
                                        DDS_StringManager_T *string_manager);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_topic_name(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_type_name(struct CDR_Stream_t *stream,
                                   const char *type_name,
                                   void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_type_name(struct CDR_Stream_t *stream,
                                        char **type_name,
                                        DDS_StringManager_T *string_manager);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_type_name(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_partition_string_seq(struct CDR_Stream_t *stream,
                                                const void *data,
                                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_partition(struct CDR_Stream_t *stream,
                                const struct DDS_PartitionQosPolicy *partition,
                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_partition(struct CDR_Stream_t *stream,
                                struct DDS_PartitionQosPolicy *partition,
                                DDS_StringManager_T *string_manager,
                                RTI_INT32 max_cumulative_chars,
                                RTI_INT32 max_string_size);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_partition(RTI_UINT32 size,
                                                    RTI_UINT32 max_length);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_deadline(struct CDR_Stream_t *stream,
                                    const struct DDS_DeadlineQosPolicy *deadline,
                                    void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_deadline(struct CDR_Stream_t *stream,
                                      struct DDS_DeadlineQosPolicy *deadline,
                                      void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_deadline(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_ownership(struct CDR_Stream_t *stream,
                                    const struct DDS_OwnershipQosPolicy
                                    *ownership,
                                    void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_ownership(struct CDR_Stream_t *stream,
                                      struct DDS_OwnershipQosPolicy *ownership,
                                      void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ownership(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_ownership_strength(struct CDR_Stream_t *stream,
                                            const struct
                                            DDS_OwnershipStrengthQosPolicy
                                            *ownership_strength,
                                            void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_ownership_strength(struct CDR_Stream_t *stream,
                                              struct
                                              DDS_OwnershipStrengthQosPolicy
                                              *ownership_strength,
                                              void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ownership_strength(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_reliability(struct CDR_Stream_t *stream,
                                      const struct DDS_ReliabilityQosPolicy
                                      *reliability,
                                      void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_reliability(struct CDR_Stream_t *stream,
                                        struct DDS_ReliabilityQosPolicy
                                        *reliability,
                                        void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_reliability(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_liveliness(struct CDR_Stream_t *stream,
                            const struct DDS_LivelinessQosPolicy *liveliness,
                            void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_liveliness(struct CDR_Stream_t *stream,
                                       struct DDS_LivelinessQosPolicy
                                       *liveliness,
                                       void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_liveliness(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_key(struct CDR_Stream_t *stream,
                              const struct DDS_BuiltinTopicKey_t *key,
                              DDS_UnsignedShort id);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_key(struct CDR_Stream_t *stream,
                                struct DDS_BuiltinTopicKey_t *key,
                                void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_key(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_locator_sequence(struct CDR_Stream_t *stream,
                                  const struct DDS_LocatorSeq *locator,
                                  RTI_UINT16 locator_id,
                                  RTI_UINT16 locator_id_ex,
                                  struct DDS_TransportEncapsulationQosPolicy *policy,
                                  NETIO_BindResolver_T* bresolver);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_locator_sequence(struct CDR_Stream_t *stream,
                                    struct DDS_LocatorSeq *locator,
                                    DDS_UnsignedShort kind,
                                    void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_locator_ex_sequence(struct CDR_Stream_t *stream,
                                    struct DDS_LocatorExSeq *locator,
                                    DDS_UnsignedShort kind,
                                    void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_locator(struct DDS_LocatorSeq *locator,
                                             RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ex_locator(struct DDS_LocatorExSeq *locator,
                                                    RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_protocol_version(struct CDR_Stream_t *stream,
                                          const struct DDS_ProtocolVersion
                                          *protocol,
                                          void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_protocol_version(struct CDR_Stream_t *stream,
                                            struct DDS_ProtocolVersion
                                            *protocol,
                                            void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_protocol_version(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_vendor_id(struct CDR_Stream_t *stream,
                                   const struct DDS_VendorId *vendor,
                                   void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_vendor_id(struct CDR_Stream_t *stream,
                                     struct DDS_VendorId *vendor,
                                     void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_vendor_id(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_durability(struct CDR_Stream_t *stream,
                           const struct DDS_DurabilityQosPolicy *durability,
                           void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_durability(struct CDR_Stream_t *stream,
                                struct DDS_DurabilityQosPolicy *durability,
                                void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_durability(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_destination_order(struct CDR_Stream_t *stream,
                           const struct DDS_DestinationOrderQosPolicy *durability,
                           void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_destination_order(struct CDR_Stream_t *stream,
                                struct DDS_DestinationOrderQosPolicy *durability,
                                void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_destination_order(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_product_version(struct CDR_Stream_t *stream,
                                           const struct DDS_ProductVersion *product,
                                           void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_product_version(struct CDR_Stream_t *stream,
                                          struct DDS_ProductVersion *product,
                                          void *param);
MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_product_version(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_presentation(struct CDR_Stream_t *stream,
                        const struct DDS_PresentationQosPolicy *presentation,
                        void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_presentation(struct CDR_Stream_t *stream,
                        struct DDS_PresentationQosPolicy *presentation,
                        void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_serialized_size_presentation(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_lease_duration(struct CDR_Stream_t *stream,
                                const struct DDS_Duration_t *lease_duration,
                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_lease_duration(struct CDR_Stream_t *stream,
                                          struct DDS_Duration_t *lease_duration,
                                          void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_lease_duration(RTI_UINT32 size);


MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_send_queue_size(struct CDR_Stream_t *stream,
                                        DDS_Long send_queue_size,
                                        void * param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserializeChecksumProperty(struct CDR_Stream_t *stream,
                                struct DDS_ChecksumProperty_t *checksum_property,
                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serializeChecksumProperty(struct CDR_Stream_t *stream,
                              const struct DDS_ChecksumProperty_t *checksum_property,
                              void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_getChecksumPropertyMaxSerializedSize(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_property(struct CDR_Stream_t *stream,
                                    const struct DDS_PropertyQosPolicy *properties);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_property(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_serialized_size_data_representation(
                                struct DDS_DataRepresentationQosPolicy *policy,
                                RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_data_representation(
                                struct CDR_Stream_t *stream,
                                struct DDS_DataRepresentationQosPolicy *policy,
                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_data_representation(
                                struct CDR_Stream_t *stream,
                                const struct DDS_DataRepresentationQosPolicy *policy);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_user_data(struct CDR_Stream_t *stream,
                                     const struct DDS_UserDataQosPolicy *user_data);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_user_data(struct CDR_Stream_t *stream,
                                       struct DDS_UserDataQosPolicy *user_data,
                                       DDS_UserDataManager_T *user_data_manager,
                                       DDS_UserDataType user_data_type);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_user_data(RTI_UINT32 size,
                                                   RTI_UINT32 max_length);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_group_data(struct CDR_Stream_t *stream,
                                      const struct DDS_GroupDataQosPolicy *group_data);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_group_data(struct CDR_Stream_t *stream,
                                       struct DDS_GroupDataQosPolicy *group_data,
                                       DDS_UserDataManager_T *user_data_manager,
                                       DDS_UserDataType group_data_type);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_group_data(RTI_UINT32 size,
                                                    RTI_UINT32 max_length);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_topic_data(struct CDR_Stream_t *stream,
                                      const struct DDS_TopicDataQosPolicy *topic_data);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_topic_data(struct CDR_Stream_t *stream,
                                        struct DDS_TopicDataQosPolicy *topic_data,
                                        DDS_UserDataManager_T *user_data_manager,
                                        DDS_UserDataType topic_data_type);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_topic_data(RTI_UINT32 size,
                                                    RTI_UINT32 max_length);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_latency_budget(struct CDR_Stream_t *stream,
                  const struct DDS_LatencyBudgetQosPolicy *latency_budget,
                  void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_latency_budget(struct CDR_Stream_t *stream,
                      struct DDS_LatencyBudgetQosPolicy *latency_budget,
                      void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_latency_budget(RTI_UINT32 size);


MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_presentation(RTI_UINT32 size);

#endif /* BuiltinCdr_pkg_h */
