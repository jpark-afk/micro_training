/*
 * FILE: osapi_os_windows.h - OS configuration file for MS Windows
 *
 * (c) Copyright, Real-Time Innovations, 2012-2016
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
#ifndef osapi_os_windows_h
#define osapi_os_windows_h

#define OSAPI_INCLUDE_WINDOWS 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <windows.h>
#include <process.h>
#include <io.h>

#define _WINSOCKAPI_
#define HAVE_SOCKET_API
#define OSAPI_ThreadHandle HANDLE
#define OSAPI_ThreadId DWORD
#define OSAPI_ProcessId DWORD
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (_write(1,buf_,len_)){}

#ifndef RTI_WIN32
#define RTI_WIN32 (1)
#endif

#endif /* osapi_os_windows_h */
