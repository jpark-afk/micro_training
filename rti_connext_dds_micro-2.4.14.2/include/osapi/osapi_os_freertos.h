/*
 * FILE: osapi_os_freertos.h - OS configuration file for FreeRTOS OSs
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021
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
 * 21jul2021,tk MICRO-3045 Fixed filenames and dates in file header comments
 * 15nov2016,francisco  Refactored from osapi_config.h
 *
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_os_freertos_h
#define osapi_os_freertos_h

#ifndef RTI_FREERTOS
#define RTI_FREERTOS
#endif

/* This is needed to compile the FREERTOS source files */
#define OSAPI_INCLUDE_FREERTOS 1

#define OSAPI_DONT_HAVE_REALLOC 1

/* Standard FREERTORS headers */
#include "FreeRTOS.h"
#include <string.h>
#include <errno.h>
#include <task.h>
#include <unistd.h>

/* Needed definitions */
#define OSAPI_PLATFORM_FREERTOS_HOSTNAME "FreeRTOS-host"

/* The number of words (not bytes!) to allocate for use as the task's stack. */
#ifndef OSAPI_PLATFORM_FREERTOS_STACK_SIZE_DEFAULT
#define OSAPI_PLATFORM_FREERTOS_STACK_SIZE_DEFAULT   2048
#endif

/* Define the thread handle */
#define OSAPI_ThreadHandle TaskHandle_t
#define OSAPI_ThreadId     TaskHandle_t

#define OSAPI_ProcessId    RTI_UINT32

/* Assume LWIP is used */
#define HAVE_SOCKET_API
#define LWIP_SYS

#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif

#endif /* osapi_os_freertos_h */
