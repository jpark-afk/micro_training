/* SampleInfoSeq.cxx

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
19may2015,as  MICRO-1193 Refactoring of Sequence API levels
19jul2013,as  Created
===================================================================== */

#ifndef RTI_CPP
#define RTI_CPP
#endif

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
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
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif

/*** SOURCE_BEGIN ***/

#define T DDS_SampleInfo
#define TSeq DDS_SampleInfoSeq
#define REDA_SEQUENCE_API   REDA_SEQUENCE_API_FULL
#include "dds_cpp/dds_cpp_sequence_defn.hxx"
