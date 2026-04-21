/*
 QosPolicy.cxx
 
 (c) Copyright, Real-Time Innovations, Sep 19, 2014-2016.
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

#ifndef dds_cpp_infrastructure_h
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_domain_h
#include "dds_cpp/dds_cpp_domain.hxx"
#endif

#if !UDP_EXCLUDE_BUILTIN
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif
#endif /* !UDP_EXCLUDE_BUILTIN */

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

#define T DDS_SubscriptionBuiltinTopicData
#define T_generate_extended_copy
#include "SupportMethodsGen.hxx"
#undef T_generate_extended
#undef T

#define T DDS_PublicationBuiltinTopicData
#define T_generate_extended_copy
#include "SupportMethodsGen.hxx"
#undef T_generate_extended
#undef T

#define T DDS_ParticipantBuiltinTopicData
#define T_generate_extended_copy
#include "SupportMethodsGen.hxx"
#undef T_generate_extended
#undef T

bool
DDS_EntityNameQosPolicy::set_name(const char *const p_name)
{
    return (DDS_EntityNameQosPolicy_set_name(this,p_name) == RTI_TRUE);
}

DDS_EntityNameQosPolicy&
DDS_EntityNameQosPolicy::operator=(const char *const p_name)
{
    set_name(p_name);
    return *this;
}

DDS_EntityNameQosPolicy&
DDS_EntityNameQosPolicy::operator= (const DDS_EntityNameQosPolicy&  from)
{
    set_name(from.name);
    return *this;
}

const DDS_EntityNameQosPolicy&
DDS_EntityNameQosPolicy::operator= (const DDS_EntityNameQosPolicy&  /* from */) const
{
    return *this;
}

bool
DDS_EntityNameQosPolicy::operator== (const DDS_EntityNameQosPolicy& from) const
{
    return DDS_String_ncmp(this->name, from.name,DDS_ENTITYNAME_QOS_NAME_MAX) == 0;
}

bool
DDS_EntityNameQosPolicy::operator== (const char *const p_name) const
{
    return DDS_String_ncmp(this->name, p_name,DDS_ENTITYNAME_QOS_NAME_MAX) == 0;
}

bool
DDS_EntityNameQosPolicy::operator!= (const DDS_EntityNameQosPolicy& from ) const
{
    return !(*this == from);
}

bool
DDS_EntityNameQosPolicy::operator!= (const char *const p_name) const
{
    return !(*this == p_name);
}

DDS_EntityNameQosPolicy::DDS_EntityNameQosPolicy(const DDS_EntityNameQosPolicy& from)
{
    set_name(from.name);
}

RT_ComponentFactoryId::RT_ComponentFactoryId(RT_ComponentFactoryId& from)
{
#ifndef T_EMPTY_IMPL
    *this = from;
#else
    UNUSED_ARG(from);
#endif
}

#ifndef T_EMPTY_IMPL
RTI_PRIVATE void
DDS_DiscoveryQosPolicy_cpp_initialize(DDS_DiscoveryQosPolicy *self)
{
    RTI_BOOL rc;

    OSAPI_Memory_zero(self,RTI_SIZEOF(DDS_DiscoveryQosPolicy));

    rc = DDS_StringSeq_initialize(&self->initial_peers);
    IGNORE_RETVAL(rc);

    rc = DDS_StringSeq_initialize(&self->enabled_transports);
    IGNORE_RETVAL(rc);

    self->accept_unknown_peers = DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE void
DDS_DiscoveryQosPolicy_cpp_copy(DDS_DiscoveryQosPolicy *self,
                                const DDS_DiscoveryQosPolicy *from)
{
    DDS_DiscoveryQosPolicy_cpp_initialize(self);

    DDS_StringSeq_copy(&self->initial_peers,&from->initial_peers);

    DDS_StringSeq_copy(&self->enabled_transports,&from->enabled_transports);

    self->discovery = from->discovery;

    self->accept_unknown_peers = from->accept_unknown_peers;
}
#endif

DDS_DiscoveryQosPolicy::DDS_DiscoveryQosPolicy()
{
#ifndef T_EMPTY_IMPL
    DDS_DiscoveryQosPolicy_cpp_initialize(this);
#endif
}

#ifndef RTI_CERT
DDS_DiscoveryQosPolicy::~DDS_DiscoveryQosPolicy()
{
    DDS_ReturnCode_t rc;

    rc = DDS_DiscoveryQosPolicy_finalize(this);
    IGNORE_RETVAL(rc);
}
#endif

DDS_DiscoveryQosPolicy::DDS_DiscoveryQosPolicy(const DDS_DiscoveryQosPolicy& from)
{
#ifndef T_EMPTY_IMPL

    DDS_DiscoveryQosPolicy_cpp_initialize(this);

    DDS_DiscoveryQosPolicy_cpp_copy(this,&from);

#else
    UNUSED_ARG(from);
#endif
}

DDS_DiscoveryQosPolicy&
DDS_DiscoveryQosPolicy::operator=(const DDS_DiscoveryQosPolicy& from)
{
    DDS_DiscoveryQosPolicy_cpp_initialize(this);

    DDS_DiscoveryQosPolicy_cpp_copy(this,&from);

    return *this;
}

bool
DDS_DiscoveryQosPolicy::operator==(const DDS_DiscoveryQosPolicy& other)
{
    if (!DDS_DiscoveryQosPolicy_is_equal(this,&other))
    {
        return false;
    }

    return true;
}

bool
DDS_DiscoveryQosPolicy::operator!=(const DDS_DiscoveryQosPolicy& other)
{
    return !(*this == other);
}

DDS_ReturnCode_t
DDS_DiscoveryQosPolicy::copy(const DDS_DiscoveryQosPolicy& from)
{
    OSAPI_Memory_zero(this,RTI_SIZEOF(DDS_DiscoveryQosPolicy));

    return DDS_DiscoveryQosPolicy_copy(this,&from);
}

