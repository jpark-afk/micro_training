/*
 (c) Copyright, Real-Time Innovations, 2012-2015.

 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
 ------------ -------
 1.0a,02-23-2012 tk  Written
==============================================================================*/
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


#ifndef DDS_ENABLE_TRUST
#define DDS_ENABLE_TRUST  0
#endif

#ifndef DDS_TRUSTED_DDS_ENABLED
#define DDS_TRUSTED_DDS_ENABLED     DDS_ENABLE_TRUST
#endif

#endif /* rtps_config_h */
