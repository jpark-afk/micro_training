/*
 BuiltinData.cxx

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

/* SubscriptionBuiltinTopicData */
#define T DDS_SubscriptionBuiltinTopicData
#define T_constructor T_constructor

T&
T::operator=(const T& from)
{
    DDS_SubscriptionBuiltinTopicData_copy((T*)this, &from);
    return *this;
}

T::T(const DDS_DomainParticipantQos &dp_qos)
{
    IGNORE_RETVAL(DDS_SubscriptionBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

/* Initializes max size for each structure member */
T::T()
{
    DDS_DomainParticipantQos dp_qos;
    /* Partition string_size is set to DDS_LENGTH_UNLIMITED by default.
     * DDS_LENGTH_UNLIMITED means that the partition size is set to the max.
     * User data does not have an absolute maximum. The constructor configures
     * an empty user data sequence.*/

#if DDS_FILTERING_ENABLED
    /* Clear the filter plugin name so that memory is not preallocated for the
     * content filter info because we do not know the actual resource limits at
     * this point.
     */
    RT_ComponentFactoryId_clear(&dp_qos.filter.name);
#endif

    IGNORE_RETVAL(DDS_SubscriptionBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

#include "SupportMethodsGen.hxx"


/* PublicationBuiltinTopicData */
#define T DDS_PublicationBuiltinTopicData
#define T_constructor T_constructor

T&
T::operator=(const T& from)
{
    DDS_PublicationBuiltinTopicData_copy((T*)this, &from);
    return *this;
}

T::T(const DDS_DomainParticipantQos &dp_qos)
{
    IGNORE_RETVAL(DDS_PublicationBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

/* Initializes max size for each structure member */
T::T()
{
    DDS_DomainParticipantQos dp_qos;
    /* Partition string_size is set to DDS_LENGTH_UNLIMITED by default.
     * DDS_LENGTH_UNLIMITED means that the partition size is set to the max.
     * User data does not have an absolute maximum. The constructor configures
     * an empty user data sequence.*/

    IGNORE_RETVAL(DDS_PublicationBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

#include "SupportMethodsGen.hxx"


/* ParticipantBuiltinTopicData */
#define T DDS_ParticipantBuiltinTopicData
#define T_constructor T_constructor

T&
T::operator=(const T& from)
{
    DDS_ParticipantBuiltinTopicData_copy((T*)this, &from);
    return *this;
}

T::T(const DDS_DomainParticipantQos &dp_qos)
{
    IGNORE_RETVAL(DDS_ParticipantBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

/* Initializes max size for each structure member */
T::T()
{
    DDS_DomainParticipantQos dp_qos;
    IGNORE_RETVAL(DDS_ParticipantBuiltinTopicData_initialize_from_qos(
                                                            (T*)this,&dp_qos));
}

#include "SupportMethodsGen.hxx"

