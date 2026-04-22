/*
 * FILE: netio_tintf_config.h - NETIO_Test Interface Configuration API
 *
 * Copyright (c) 2015-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 22dec2014,tk Written
 */
/*ce
 * \file
 */
/*ci
 * \defgroup NETIOTestIntfConfig NETIO_TestIF Configuration
 * \ingroup NETIOModule
 * \brief NETIO Configuration API
 */
/*ci \addtogroup NETIOTestIntfConfig
 * @{
 */
#ifndef netio_tintf_config_h
#define netio_tintf_config_h

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif


#endif /* netio_tintf_config_h */

/*ci @} */
