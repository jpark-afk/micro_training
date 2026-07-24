/*
 * FILE: PartitionQosPolicy.c - PartitionPolicy Helper Functions
 *
 * (c) Copyright, Real-Time Innovations, 2024
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
 * 01Dec2023,ad Created.
 */

/*ci
 * \brief PartitionQosPolicy.c
 */

#ifndef PartitionQosPolicy_h
#define PartitionQosPolicy_h

#include "dds_c/dds_c_partition_qos.h"
#include "dds_c/dds_c_discovery.h"
#include "dds_c/dds_c_string_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif
DDS_ReturnCode_t
DDS_PartitionQosPolicy_initialize(struct DDS_PartitionQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_is_consistent_w_limits(
        const struct DDS_PartitionQosPolicy *self,
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *dp_qos);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_copy(struct DDS_PartitionQosPolicy *left,
                               const struct DDS_PartitionQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_set_from(struct DDS_PartitionQosPolicy *left,
                                const struct DDS_PartitionQosPolicy *right,
                                DDS_StringManager_T *string_manager);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_is_equal(const struct DDS_PartitionQosPolicy *left,
                               const struct DDS_PartitionQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_is_compatible(
        const struct DDS_StringSeq *request,
        const struct DDS_StringSeq *offered);

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_PartitionQosPolicy_finalize(struct DDS_PartitionQosPolicy *self);
#endif /* !RTI_CERT */

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_PartitionQosPolicy_finalize_no_dealloc(struct DDS_PartitionQosPolicy *self,
                                    DDS_StringManager_T *string_manager);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_PartitionQosPolicy_clear_strings(DDS_StringManager_T *string_manager,
                                    struct DDS_PartitionQosPolicy *partition);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PartitionQosPolicy_set_maximum_w_max(struct DDS_PartitionQosPolicy *partition,
                                const struct DDS_DomainParticipantQos *dp_qos);

#ifdef __cplusplus
}
#endif

#endif /* PartitionPolicy_h*/

/*ci @} */
