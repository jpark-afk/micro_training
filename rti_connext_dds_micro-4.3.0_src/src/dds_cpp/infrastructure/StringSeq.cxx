/* StringSeq.cxx

 (c) Copyright, Real-Time Innovations, 2013-2016.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
12feb2016,tk  MICRO-1529 Added primitive sequences
10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq 
19may2015,as  MICRO-1193 Refactoring of Sequence API levels
19jul2013,as  Created
===================================================================== */
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
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

#define T char*
#define TSeq REDA_StringSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_isCDRStringType
#define TSeq_isCDRCharStringType
#define TSeq_is_equal
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Octet
#define TSeq DDS_OctetSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_UnsignedShort
#define TSeq DDS_UnsignedShortSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_UnsignedLong
#define TSeq DDS_UnsignedLongSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_UnsignedLongLong
#define TSeq DDS_UnsignedLongLongSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_LongDouble
#define TSeq DDS_LongDoubleSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Wstring
#define TSeq DDS_WstringSeq
#define TSeq_isCDRStringType
#define TSeq_isCDRStringType_no_max
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Char
#define TSeq DDS_CharSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Boolean
#define TSeq DDS_BooleanSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Short
#define TSeq DDS_ShortSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Long
#define TSeq DDS_LongSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Enum
#define TSeq DDS_EnumSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Wchar
#define TSeq DDS_WcharSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_LongLong
#define TSeq DDS_LongLongSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"


#define T    DDS_Float
#define TSeq DDS_FloatSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T    DDS_Double
#define TSeq DDS_DoubleSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#if 0
#define T    DDS_String
#define TSeq DDS_StringSeq
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"
#endif

