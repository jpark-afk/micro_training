/*
 * FILE: netio_config.h - NETIO Configuration API
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 22jan2014,eh MICRO-1027: disable multicast for RTI_CERT
 * 23feb2012,tk Written
 */
/*ce
 * \file
 */
/*ci
 * \defgroup NETIOConfig NETIO Configuration
 * \ingroup NETIOModule
 * \brief NETIO Configuration API
 *
 * \details
 *
 * This file is the single point where NETIO behavior can be configured.
 * All optional and configurable features shall be added here.
 */
/*ci \addtogroup NETIOConfig
 * @{
 */
#ifndef netio_config_h
#define netio_config_h

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif

/*ci
 * \brief Enable or disable the liveliness channel
 *
 * \details
 * Liveliness channel is needed to have any DataWriter or DataReader configured
 * with finite liveliness lease duration and liveliness kind
 * DDS_AUTOMATIC_LIVELINESS_QOS or DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS.
 */
#ifndef DDS_ENABLE_LIVELINESS_CHANNEL
#define DDS_LIVELINESS_CHANNEL_ENABLED (1)
#else
#define DDS_LIVELINESS_CHANNEL_ENABLED DDS_ENABLE_LIVELINESS_CHANNEL
#endif

#endif /* netio_config_h */

/*ci @} */
