/*
 QosPolicy.cxx
 
 (c) Copyright, Real-Time Innovations, Sep 19, 2014-2024.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
07apr2016,tk MICRO-1541 Fixed assignment operator issues in C++
10feb2016,tk MICRO-1525 Added assignment operator to DDS_EntityNameQosPolicy
19may2015,as MICRO-1193 Refactoring of Sequence API levels
20sep2014,as Created
===================================================================== */

#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

/*** SOURCE_BEGIN ***/

#define T struct DDS_QosPolicyCount
#define TSeq DDS_QosPolicyCountSeq
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

DDS_EntityNameQosPolicy::DDS_EntityNameQosPolicy()
{
    this->name[0]=0;
}

DDS_EntityNameQosPolicy::~DDS_EntityNameQosPolicy()
{
    this->name[0]=0;
}

DDS_EntityNameQosPolicy::DDS_EntityNameQosPolicy(const DDS_EntityNameQosPolicy& from)
{
    this->name[0]=0;
    set_name(from.name);
}

bool
DDS_EntityNameQosPolicy::set_name(const char *const pname)
{
    return (DDS_EntityNameQosPolicy_set_name(this,pname) == RTI_TRUE);
}

DDS_EntityNameQosPolicy&
DDS_EntityNameQosPolicy::operator=(const char *const pname)
{
    set_name(pname);
    return *this;
}

DDS_EntityNameQosPolicy&
DDS_EntityNameQosPolicy::operator= (const DDS_EntityNameQosPolicy&  from)
{
    set_name(from.name);
    return *this;
}

bool
DDS_EntityNameQosPolicy::operator== (const DDS_EntityNameQosPolicy& from) const
{
    return DDS_String_ncmp(this->name, from.name,DDS_ENTITYNAME_QOS_NAME_MAX) == 0;
}

bool
DDS_EntityNameQosPolicy::operator== (const char *const pname) const
{
    return DDS_String_ncmp(this->name, pname,DDS_ENTITYNAME_QOS_NAME_MAX) == 0;
}

bool
DDS_EntityNameQosPolicy::operator!= (const DDS_EntityNameQosPolicy& from ) const
{
    return !(*this == from);
}

bool
DDS_EntityNameQosPolicy::operator!= (const char *const pname) const
{
    return !(*this == pname);
}
