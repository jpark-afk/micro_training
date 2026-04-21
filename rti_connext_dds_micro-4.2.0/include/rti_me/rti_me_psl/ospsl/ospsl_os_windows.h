/*
 * FILE: osapi_os_windows.h - OS configuration file for MS Windows
 *
 * (c) Copyright, Real-Time Innovations, 2012-2024
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

#endif /* ospsl_os_windows_h */
