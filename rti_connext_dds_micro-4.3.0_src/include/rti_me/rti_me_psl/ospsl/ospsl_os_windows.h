/*
 * FILE: ospsl_os_windows.h - OS configuration file for MS Windows
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 28jul2016,tk  Refactored from osapi_config.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef ospsl_os_windows_h
#define ospsl_os_windows_h

#include "osapi/osapi_features.h"

#define OSAPI_INCLUDE_WINDOWS 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>

#define _WINSOCKAPI_
#define HAVE_SOCKET_API

#ifndef RTI_WIN32
#define RTI_WIN32 (1)
#endif

#if OSAPI_ENABLE_SHMEM
  #define OSAPI_SHMEM_IS_ENABLED 1
#else
  #define OSAPI_SHMEM_IS_ENABLED 0
#endif

/*ci \brief Enable the timer thread instead of the default queue timer.
 *
 * \details
 * This option is for internal use and kept as an option incase the timer
 * callback cannot be used in specialized windows environments.
 */
#ifndef OSAPI_ENABLE_TIMER_THREAD
#define OSAPI_ENABLE_TIMER_THREAD (0)
#endif

/*ci \brief Enable the high precision timer callback
 */
#ifndef OSAPI_ENABLE_HIGH_PRECISION_TIMER
#define OSAPI_ENABLE_HIGH_PRECISION_TIMER (1)
#endif

#if OSAPI_ENABLE_TIMER_THREAD
#define OSAPI_TIMER_THREAD_ENABLED (1)
#else
#define OSAPI_TIMER_THREAD_ENABLED (0)
#endif

#if OSAPI_ENABLE_HIGH_PRECISION_TIMER
#define OSAPI_HIGH_PRECISION_TIMER_ENABLED (1)
/* A precision timer tick is 100 nanoseconds in duration */
#define NS_PER_OS_TICK 100
#else
#define OSAPI_HIGH_PRECISION_TIMER_ENABLED (0)
/* A non-precision timer tick is 1000000 nanoseconds */
#define NS_PER_OS_TICK 1000000
#endif

#endif /* ospsl_os_windows_h */
