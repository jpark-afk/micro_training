/*
 * FILE: dds_c_user_data_qos.h - User Data QoS Types
 *
 * (c) Copyright, Real-Time Innovations, 2023-2025.
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
 * \brief User Data QoS Types
 */

#ifndef dds_c_user_data_qos_h
#define dds_c_user_data_qos_h

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

/*e \dref_UserDataQosGroupDocs
 */

/*e \dref_UserDataQosPolicy
 */
struct DDSCPPDllExport DDS_UserDataQosPolicy
{
    /*e \dref_UserDataQosPolicy_value
     */
    struct DDS_OctetSeq value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_UserDataQosPolicy)
};

/*i \dref_UserDataQosPolicy_DEFAULT
 * Default is empty sequence
 */
#define DDS_USER_DATA_QOS_POLICY_DEFAULT        \
{                                               \
    REDA_DEFINE_SEQUENCE_INITIALIZER(DDS_Octet) \
}

/*e \dref_GroupDataQosGroupDocs
 */

/*e \dref_GroupDataQosPolicy
 */
struct DDSCPPDllExport DDS_GroupDataQosPolicy
{
    /*e \dref_GroupDataQosPolicy_value
     */
    struct DDS_OctetSeq value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_GroupDataQosPolicy)
};

/*i \dref_GroupDataQosPolicy_DEFAULT
 * Default is empty sequence
 */
#define DDS_GROUP_DATA_QOS_POLICY_DEFAULT       \
{                                               \
    REDA_DEFINE_SEQUENCE_INITIALIZER(DDS_Octet) \
}


/*e \dref_TopicDataQosGroupDocs
 */

/*e \dref_TopicDataQosPolicy
 */
struct DDSCPPDllExport DDS_TopicDataQosPolicy
{
    /*e \dref_TopicDataQosPolicy_value
     */
    struct DDS_OctetSeq value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TopicDataQosPolicy)
};

/*i \dref_TopicDataQosPolicy_DEFAULT
 * Default is empty sequence
 */
#define DDS_TOPIC_DATA_QOS_POLICY_DEFAULT       \
{                                               \
    REDA_DEFINE_SEQUENCE_INITIALIZER(DDS_Octet) \
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_c_user_data_qos_h */
