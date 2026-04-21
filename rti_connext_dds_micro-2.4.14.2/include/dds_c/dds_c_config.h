/*
 * FILE: dds_c_config.h - DDS C configuration file
 *
 * (c) Copyright, Real-Time Innovations, 2012-2016.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Added configuration definition to enable support for blocked
 *   DataReaders.
 * 13sep2021,tk MICRO-3198/PR.29540
 * - Added comment with the supported ranges for the 4 version numbers.
 * 23feb2012,tk  Written
 */
/*e \file
  \brief DDS C configuration file
*/
#ifndef dds_c_config_h
#define dds_c_config_h

/* Product version. The supported ranges are:
 * RTIME_DDS_VERSION_MAJOR - must be [0,9]
 * RTIME_DDS_VERSION_MINOR - must be [0,9]
 * RTIME_DDS_VERSION_REVISION - must [0,99]
 * RTIME_DDS_VERSION_RELEASE - must [0,99]
 *
 *  If any of the ranges are exceeded, DDS_DomainParticipant_initialize must
 *  be updated to support the increased range.
 */
#define RTIME_DDS_VERSION_MAJOR    2
#define RTIME_DDS_VERSION_MINOR    4
#define RTIME_DDS_VERSION_REVISION 14
#define RTIME_DDS_VERSION_RELEASE  2

/* The default is engineering build */
#ifndef RTIME_DDS_MATURITY
#define RTIME_DDS_MATURITY         "RTI_GAR"
#endif

/*ci The product name */
#define RTI_CONNEXT_MICRO_NAME "RTI Connext Micro"

/*ci utility macros for concatenating strings for the build-id
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/*ci FORMAT:  <version>_<date>T<hhmmss>Z_RTI[ENG|GAR|EAR|RC] */
#define RTIME_BUILD_STRING_BUILDER(prefix_) prefix_ "_BUILD_"  \
                                STR(RTIME_DDS_VERSION_MAJOR) "." \
                                STR(RTIME_DDS_VERSION_MINOR) "."  \
                                STR(RTIME_DDS_VERSION_REVISION) "." \
                                STR(RTIME_DDS_VERSION_RELEASE) "_" \
                                RTIME_BUILD_ID "_" RTIME_DDS_MATURITY

/*i \brief The OMG Vendor id used by Micro
 */
#define RTI_CONNEXT_MICRO_VENDOR_ID_MAJOR  (0x01)
#define RTI_CONNEXT_MICRO_VENDOR_ID_MINOR  (0x0a)

/*ci The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif

/* RTPS_RELIABILITY
 *
 * Set RTPS_RELIABILITY to enable reliable RTPS communication.
 *
 */
#ifndef RTPS_RELIABILITY
#define RTPS_RELIABILITY  1
#endif

#ifdef RTI_CERT
#undef INCLUDE_API_LOOKUP 
#else
#ifndef INCLUDE_API_LOOKUP
#define INCLUDE_API_LOOKUP 1
#endif
#endif

#ifdef RTI_CERT
#undef INCLUDE_API_QOS 
#else
#ifndef INCLUDE_API_QOS
#define INCLUDE_API_QOS 1
#endif
#endif

#ifndef ENABLE_STATUS_LISTENER
#define ENABLE_STATUS_LISTENER 1
#endif

#ifndef ENABLE_QOS_DEADLINE
#define ENABLE_QOS_DEADLINE 1
#endif

#ifdef RTI_CERT
#undef INCLUDE_API_LISTENER
#else
#ifndef INCLUDE_API_LISTENER
#define INCLUDE_API_LISTENER 1
#endif
#endif

#ifndef INCLUDE_API_INSTANCE
#define INCLUDE_API_INSTANCE 1
#endif

#ifndef INCLUDE_API_DELETE
#define INCLUDE_API_DELETE 1
#endif

#ifdef RTI_CERT
#undef DDS_ENABLE_BLOCKING_READER
#else
#ifndef DDS_ENABLE_BLOCKING_READER
#define DDS_ENABLE_BLOCKING_READER (0)
#endif
#endif

#if DDS_ENABLE_BLOCKING_READER
#define DDS_BLOCKING_READER_ENABLED (1)
#else
#define DDS_BLOCKING_READER_ENABLED (0)
#endif

#endif /* dds_c_config_h */
