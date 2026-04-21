/*
 * FILE: rtps_config.h - Rtps configuration
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021.
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
 * 1.0a,02-23-2012 tk  Written
 */
/*e \file
 * \brief RTPS Configuration
 *  
 * \details 
 * Configurable build flags for RTPS functionality 
 */

#ifndef rtps_config_h
#define rtps_config_h

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif
  
/*ci
 * \def RTPS_RELIABILITY
 * \brief Set RTPS_RELIABILITY to enable reliable RTPS communication.
 */
#ifndef RTPS_RELIABILITY
#define RTPS_RELIABILITY  1
#endif

/*ci
 * \def RTPS_ENABLE_DATA_BATCH
 * \brief Set RTPS_ENABLE_DATA_BATCH to enable batching reception RTPS communication.
 */
#ifdef RTI_CERT
#undef RTPS_ENABLE_DATA_BATCH
#else
#ifndef RTPS_ENABLE_DATA_BATCH
#define RTPS_ENABLE_DATA_BATCH  (1)
#endif
#endif /* RTI_CERT */

#if RTPS_ENABLE_DATA_BATCH
#define RTPS_DATA_BATCH_ENABLED (1)
#else
#define RTPS_DATA_BATCH_ENABLED (0)
#endif

#ifndef RTPS_ENABLE_CHECKSUM
#define RTPS_ENABLE_CHECKSUM  (1)
#endif

#ifndef RTPS_ENABLE_CHECKSUM_BUILTIN
#define RTPS_ENABLE_CHECKSUM_BUILTIN (1)
#endif

#if RTPS_ENABLE_CHECKSUM
#define RTPS_CHECKSUM_ENABLED 1
#endif

#if RTPS_ENABLE_CHECKSUM_BUILTIN
#define RTPS_CHECKSUM_PLUGIN_ENABLED 1
#endif

#endif /* rtps_config_h */
