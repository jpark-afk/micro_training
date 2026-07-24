/*
 * FILE: dds_c_partition_qos.h - Partition QoS
 *
 * (c) Copyright, Real-Time Innovations, 2024
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Partition QoS
 */

#ifndef dds_c_partition_qos_h
#define dds_c_partition_qos_h

#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif

#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_PartitionQosGroupDocs
 */

/*e \dref_PartitionQosPolicy
 */
struct DDSCPPDllExport DDS_PartitionQosPolicy
{
    /*e \dref_PartitionQosPolicy_name
     */
    struct DDS_StringSeq name;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_PartitionQosPolicy)
};

/*i \dref_PartitionQosPolicy_DEFAULT
 */
#define DDS_PARTITION_QOS_POLICY_DEFAULT \
{ \
        REDA_DEFINE_SEQUENCE_INITIALIZER(DDS_String) \
}

/*i
 * max_partitions
 */
#define DDS_PARTITIONQOSPOLICY_MAX_PARTITIONS 64

/*i
 * max_partition_cumulative_characters
 */
#define DDS_PARTITIONQOSPOLICY_MAX_PARTITION_CHARACTERS 256

/*i
 * max_partition_string_size
 */
#define DDS_PARTITIONQOSPOLICY_MAX_PARTITION_STRING_SIZE \
                                DDS_PARTITIONQOSPOLICY_MAX_PARTITION_CHARACTERS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_c_partition_qos_h */
