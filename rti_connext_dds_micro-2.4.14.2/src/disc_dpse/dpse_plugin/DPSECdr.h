/*
 * FILE: DPSECdr.c - DPSE CDR functionality
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
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
 * 25jul2011,tk Written
 */
/*ce
 * \file
 * \brief DPSE CDR functionality
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef DPSECdr_h
#define DPSECdr_h

/*ci
 * \brief PID for Participant::product_version
 */
#define DISC_RTPS_PID_PRODUCT_VERSION               (0x8000)

/*ci
 * \brief PID for Participant::entity_name
 */
#define DISC_RTPS_PID_ENTITY_NAME                   RTPS_PID_ENTITY_NAME

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_serialize_product_version(struct CDR_Stream_t *stream,
                         const struct DDS_ProductVersion_t *productVersion,
                         void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_deserialize_product_version(struct CDR_Stream_t *stream,
                                struct DDS_ProductVersion_t *productVersion,
                                void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DPSE_Builtin_get_product_version_max_size_serialized(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_serialize_entity_name_qos_policy(struct CDR_Stream_t *stream,
                            const struct DDS_EntityNameQosPolicy *entityName,
                            void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_deserialize_entity_name_qos_policy(struct CDR_Stream_t *stream,
                            struct DDS_EntityNameQosPolicy *entityName,
                            void * param);

MUST_CHECK_RETURN extern RTI_UINT32
DPSE_Builtin_get_entity_name_qos_policy_max_size_serialized(RTI_UINT32 size);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_serialize_checksum_property(struct CDR_Stream_t *stream,
                     const struct DDS_ChecksumProperty_t *crc_checksum,
                     void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_Builtin_deserialize_checksum_property(struct CDR_Stream_t *stream,
                        struct DDS_ChecksumProperty_t *crc_checksum,
                        void *param);

extern RTI_UINT32
DPSE_Builtin_get_checksum_property_max_size_serialized(RTI_UINT32 size);

#endif

/*ci @} */

