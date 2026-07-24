/* Locator.cxx

 Copyright (c) 2013-2026 Real-Time Innovations,Inc. All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
 ---------------------
 30dec2015,as  Created
 ===================================================================== */

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef dds_cpp_infrastructure_h
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
/*** SOURCE_BEGIN ***/

#define T struct RTPS_Locator
#define TSeq DDS_LocatorSeq
#define TSeq_is_equal
#define REDA_SEQUENCE_USER_API
#include "dds_cpp/dds_cpp_sequence_defn.hxx"

#define T struct DDS_LocatorEx
#define TSeq DDS_LocatorExSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#define TSeq_loan_contiguous
#define TSeq_unloan
#include "dds_cpp/dds_cpp_sequence_defn.hxx"
