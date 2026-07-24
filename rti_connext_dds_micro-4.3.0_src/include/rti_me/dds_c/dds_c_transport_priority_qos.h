/*
 * FILE: dds_c_transport_priority_qos.h - Transport Priority QoS Types
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Transport Priority QoS Types
 * \details This file is always included via dds_c_infrastructure.h and not directly.
 */

#ifndef dds_c_transport_priority_qos_h
#define dds_c_transport_priority_qos_h
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_TransportPriorityQosGroupDocs
 */

/*e \dref_TransportPriorityQosPolicy
 */
struct DDSCPPDllExport DDS_TransportPriorityQosPolicy
{
    /*e \dref_TransportPriorityQosPolicy_value
     */
    DDS_Long value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TransportPriorityQosPolicy)
};

/*i \dref_TransportPriorityQosPolicy_DEFAULT
 * Default is empty sequence
 */
#define DDS_TRANSPORT_PRIORITY_QOS_POLICY_DEFAULT       \
{                                                       \
   0                                                    \
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_c_transport_priority_qos_h */
