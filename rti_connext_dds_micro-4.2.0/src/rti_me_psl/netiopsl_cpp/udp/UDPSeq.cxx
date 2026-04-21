/*
 * FILE: UDPSeq.cxx - UDP Seq C++ API
 *
 * Copyright (c) 2013-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#ifndef dds_cpp_netio_hxx
#include "rti_me_psl/netio/netio_udp_cpp.hxx"
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
/*** SOURCE_BEGIN ***/

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


