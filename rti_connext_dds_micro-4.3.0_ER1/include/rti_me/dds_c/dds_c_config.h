/*
 * FILE: dds_c_config.h - DDS C configuration file
 *
 * (c) Copyright, Real-Time Innovations, 2012-2024.
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
 * 23feb2012,tk  Written
 */
/*e \file
  \brief DDS C configuration file
*/
#ifndef dds_c_config_h
#define dds_c_config_h

#ifndef netio_config_h
#include "netio/netio_config.h"
#endif

/* Product version:
 */
#define RTIME_DDS_VERSION_MAJOR    4
#define RTIME_DDS_VERSION_MINOR    3
#define RTIME_DDS_VERSION_RELEASE  0
#define RTIME_DDS_VERSION_REVISION 0

/* The default is engineering build */
#ifndef RTIME_DDS_MATURITY
#define RTIME_DDS_MATURITY         "ER1_RTI_ENG"
#endif

#define RTI_CONNEXT_MICRO_NAME "RTI Connext DDS Micro"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* FORMAT:  <version>_<date>T<hhmmss>Z_RTI[ENG|GAR|EAR|RC] */
#define RTIME_BUILD_STRING_BUILDER(prefix_) prefix_ "_BUILD_"  \
                                STR(RTIME_DDS_VERSION_MAJOR) "." \
                                STR(RTIME_DDS_VERSION_MINOR) "."  \
                                STR(RTIME_DDS_VERSION_RELEASE) "_" \
                                STR(RTIME_DDS_VERSION_REVISION) "." \
RTIME_BUILD_ID "_" RTIME_DDS_MATURITY

/*i \brief The OMG Vendor id used by Micro
 */
#define RTI_CONNEXT_MICRO_VENDOR_ID_MAJOR  (0x01)
#define RTI_CONNEXT_MICRO_VENDOR_ID_MINOR  (0x0a)

/*i \brief The OMG Vendor id used by Pro
 */
#define RTI_CONNEXT_PRO_VENDOR_ID_MAJOR  (0x01)
#define RTI_CONNEXT_PRO_VENDOR_ID_MINOR  (0x01)

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif
#include "osapi/osapi_config.h"

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

#ifndef ENABLE_BUILTIN_IPC
#define ENABLE_BUILTIN_IPC 1
#endif

#ifndef DDS_ENABLE_QOS_PROFILE
#define DDS_ENABLE_QOS_PROFILE        (1)
#endif

#define DDS_QOS_PROFILE_IS_ENABLED  DDS_ENABLE_QOS_PROFILE

#ifndef DDS_ENABLE_XTYPES
#ifdef RTI_CERT
#define DDS_ENABLE_XTYPES 0
#else
#define DDS_ENABLE_XTYPES 1
#endif
#endif

#if DDS_ENABLE_XTYPES
#define DDS_XTYPES_IS_ENABLED 1
#else
#define DDS_XTYPES_IS_ENABLED 0
#endif

#ifndef DDS_ENABLE_FLOW_CONTORL
#define DDS_ENABLE_FLOW_CONTORL (1)
#endif

#ifndef DDS_ENABLE_FRAGMENTATION
#define DDS_ENABLE_FRAGMENTATION (1)
#endif

#if DDS_ENABLE_FRAGMENTATION
#define DDS_FRAGMENTATION_ENABLED (1)
#else
#define DDS_FRAGMENTATION_ENABLED (0)
#endif

#if DDS_ENABLE_FLOW_CONTROL
#define DDS_FLOW_CONTROLLER_ENABLED (1)
#else
#define DDS_FLOW_CONTROLLER_ENABLED (0)
#endif

/*ci
 * \brief Enable Applicaton Generation
 *
 * \details
 * Setting this option to 0 disables the feature application generation
 */
#ifndef DDS_ENABLE_APPGEN
#define DDS_ENABLE_APPGEN  1
#endif

#endif /* dds_c_config_h */
