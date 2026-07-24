/*
 QosPolicy.cxx

 Copyright (c) 2014-2025 Real-Time Innovations, Inc. All rights reserved.

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

#define T struct CDR_Property
#define TSeq CDR_PropertySeq
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T CDR_Property
#define T_RetVal                bool
#define T_Retval_is_bool        1
#define T_RetVal_UNSUPPORTED    (false)
#define T_generate_extended
#define T_generate_assignment
#include "SupportMethodsGen.hxx"

bool
CDR_PropertySeq::assert_property(const char * const name,
                                 const char * const value,
                                 bool propagate)
{
    return (CDR_PropertySeq_assert_property(this,
            name, value,
            propagate? CDR_BOOLEAN_TRUE: CDR_BOOLEAN_FALSE) == RTI_TRUE);
}

CDR_Property*
CDR_PropertySeq::lookup_property(const char * const name)
{
    return CDR_PropertySeq_lookup_property(this, name);
}
