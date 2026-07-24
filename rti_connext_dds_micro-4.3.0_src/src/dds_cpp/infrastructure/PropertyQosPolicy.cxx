/*
 * FILE: PropertyQosPolicy.cxx - PropertyQosPolicy Helper Functions
 *
 * (c) Copyright, Real-Time Innovations, 2024-2024
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

#define UNUSED_ARG(x) (void)(x)

DDS_PropertyQosPolicy::DDS_PropertyQosPolicy()
{
    DDS_PropertyQosPolicy_initialize(this);
}

DDS_PropertyQosPolicy::~DDS_PropertyQosPolicy()
{
    DDS_ReturnCode_t rtn;
    rtn = DDS_PropertyQosPolicy_finalize(this);
    UNUSED_ARG(rtn);
    
}

DDS_ReturnCode_t
DDSPropertyQosPolicyHelper::assert_property(
    DDS_PropertyQosPolicy& policy,
    const char *name,const char *value,
    DDS_Boolean propagate)
{
    return DDS_PropertyQosPolicyHelper_assert_property(&policy,name,value,propagate);
}

/*i
 */
DDS_ReturnCode_t
DDSPropertyQosPolicyHelper::add_property(
    DDS_PropertyQosPolicy& policy,
    const char *name,const char *value,
    DDS_Boolean propagate)
{
    return DDS_PropertyQosPolicyHelper_add_property(&policy,name,value,propagate);
}

struct DDS_Property_t*
DDSPropertyQosPolicyHelper::lookup_property(
    DDS_PropertyQosPolicy& policy,
    const char *name)
{
    return DDS_PropertyQosPolicyHelper_lookup_property(&policy,name);
}

/*i
 */
DDS_ReturnCode_t
DDSPropertyQosPolicyHelper::remove_property(
    DDS_PropertyQosPolicy& policy,
    const char *name)
{
    return DDS_PropertyQosPolicyHelper_remove_property(&policy,name);
}
