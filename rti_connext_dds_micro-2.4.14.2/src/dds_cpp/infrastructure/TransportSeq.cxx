/* StringSeq.cxx

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

#ifndef dds_cpp_infrastructure_h
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
#if !UDP_EXCLUDE_BUILTIN
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif
#endif
/*** SOURCE_BEGIN ***/

#if !UDP_EXCLUDE_BUILTIN
#ifndef RTI_CERT
#define T struct UDP_NatEntry
#define TSeq UDP_NatEntrySeq
#include "dds_cpp/dds_cpp_sequence_defn.hxx"
#endif /* !RTI_CERT */

#define T struct UDP_InterfaceTableEntry
#define TSeq UDP_InterfaceTableEntrySeq
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#if UDP_TRANSFORMS_ENABLED
#define T struct UDP_TransformRule
#define TSeq UDP_TransformRuleSeq
#include "dds_cpp/dds_cpp_sequence_defn.hxx"
#endif
#endif


