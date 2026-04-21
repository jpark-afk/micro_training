/*
 * FILE: osapi_os_posix.h - OS configuration file for POSIX/POSIX like OSs
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
 * 20oct2020,tk  MICRO-2619/PR#28223
 *     - Define a generic RTI_QNX when QNX is detected.
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 9sept2020,fmt MICRO-2519/PR#28069 Remove "TODO" in comments
 * 28jul2016,tk  Refactored from osapi_config.h
 *
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_os_posix_h
#define osapi_os_posix_h


#define OSAPI_INCLUDE_POSIX 1

/* OSAPI_ENABLE_STRICT_POSIX is used to disable using non-POSIX features,
 * even if available. It is set per POSIX architecture
 */
#if defined(__APPLE__) && defined(__MACH__)
#ifndef RTI_DARWIN
#define RTI_DARWIN
#endif
/* OSAPI_ENABLE_STRICT_POSIX is used to disable using non-POSIX features,
 * even if available. It is set per POSIX architecture
 */
#ifndef OSAPI_ENABLE_STRICT_POSIX
#define OSAPI_ENABLE_STRICT_POSIX                     0
#endif
#elif defined(__linux__)
#ifndef RTI_LINUX
#define RTI_LINUX
#endif
#ifndef OSAPI_ENABLE_STRICT_POSIX
#define OSAPI_ENABLE_STRICT_POSIX                     0
#endif
#elif defined(__VOS__)
#ifndef RTI_VOS
#define RTI_VOS
#endif
#ifndef OSAPI_ENABLE_STRICT_POSIX
#define OSAPI_ENABLE_STRICT_POSIX                     1
#endif
#elif defined(__QNXNTO__)
#ifndef RTI_QNX6
#define RTI_QNX6
#endif
#ifndef RTI_QNX
#define RTI_QNX
#endif
#ifndef OSAPI_ENABLE_STRICT_POSIX
#define OSAPI_ENABLE_STRICT_POSIX                     1
#endif
#elif defined(__vxworks)
#ifndef RTI_VXWORKS
#define RTI_VXWORKS
#endif
#elif defined(RTI_DEOS_RTEMS)
#ifndef OSAPI_ENABLE_STRICT_POSIX
#define OSAPI_ENABLE_STRICT_POSIX                     0
#endif
#if defined(RTI_DEOS)
#error "RTI_DEOS must not be defined when RTI_DEOS_RTEMS is defined"
#endif
#if defined(RTI_ARINC653)
#error "RTI_ARINC653 must not be defined when RTI_DEOS_RTEMS is defined"
#endif
#else
#error "Unable to detect POSIX variant. Please define OSAPI_OS_DEF_H."
#endif

#ifndef RTI_VXWORKS
/* Claim compliance with IEEE Std 1003.1, 2004 Edition */
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200112L

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 600
#endif /* !RTI_VXWORKS */

#ifndef RTI_UNIX
#define RTI_UNIX
#endif

/* This is needed to get non-POSIX APIs for Linux/Darwin */
#if !OSAPI_ENABLE_STRICT_POSIX
#ifdef RTI_DARWIN
#define _DARWIN_C_SOURCE
#endif
#if defined(RTI_LINUX) || defined(RTI_DEOS_RTEMS)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#endif /* OSAPI_STRICT_POSIX */

#ifdef RTI_VXWORKS
#define _VX_CPU CPU 
#include <unistd.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <sys/time.h> 
#include <time.h>
#include <string.h> 
#include <sys/types.h> 
#include <pthread.h>
#include <vsbConfig.h> 
#else
/* Standard POSIX headers */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/utsname.h>
#endif /* RTI_VXWORKS */

#define HAVE_SSIZE_T 1

/* Define the thread handle */
#define OSAPI_ThreadHandle pthread_t
#define OSAPI_ThreadId     pthread_t

#define OSAPI_ProcessId    pid_t

/* Assume all supported UNIX variations support the BSD socket API */
#define HAVE_SOCKET_API        1

/* Use free() */
#ifndef RTI_CERT
#define OSAPI_ENABLE_STDC_FREE 1
#define OSAPI_ENABLE_STDC_REALLOC 1
#else
#define OSAPI_ENABLE_STDC_FREE 0
#define OSAPI_ENABLE_STDC_REALLOC 0
#endif

/*******************************************************************************
 * Darwin (Mac OS X, macOS) specific definitions
 ******************************************************************************/
#ifdef RTI_DARWIN

#if !defined(RTI_DARWIN7)
#include <uuid/uuid.h>
#endif

/* Timer selection:
 * For strict POSIX use a sleeping thread, otherwise use a GCD timer
 */
#if !defined(USE_TIMER_THREAD_SEMAPHORE) && \
    !defined(USE_TIMER_GCD) && \
    !defined(USE_TIMER_THREAD_SLEEP)
#if !OSAPI_ENABLE_STRICT_POSIX && (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE)
#include <malloc/malloc.h>
#include <dispatch/dispatch.h>
#define USE_TIMER_THREAD_SEMAPHORE
#define USE_TIMER_GCD
#else
#define USE_TIMER_THREAD_SLEEP
#endif /* !OSAPI_STRICT_POSIX */
#endif

#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif

/*******************************************************************************
 * Linux / RTEMS specific definitions
 ******************************************************************************/
#elif defined(RTI_LINUX) || defined(RTI_DEOS_RTEMS)

#if defined(__ANDROID__)
#include <endian.h>
#else
#if !defined(RTI_DEOS_RTEMS)
#include <features.h>
#endif
#if defined (__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2,9)
#include <endian.h>
#endif
#endif
#endif /* else __ANDROID__ */

#if ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE
#include <malloc.h>
#ifdef __ANDROID__
    /* NOTE: This is supported in R8D, but missing from the header-file
 * https://code.google.com/p/android/issues/detail?id=20140
 */
    extern int
    clock_nanosleep(clockid_t clock_id, int flags,
                    const struct timespec *request,
                    struct timespec *remain);
#if OSAPI_ENABLE_LOG
#include <android/log.h>
#endif
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) \
    if (__android_log_print(ANDROID_LOG_DEBUG,"com.rti.rtime",buffer)){}
#endif
#else  /* __ANDROID__ */
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif
#endif /* __ANDROID__ */

#if !defined(USE_TIMER_SIGNAL) && \
    !defined(USE_TIMER_THREAD_RT) && \
    !defined(USE_TIMER_THREAD_SLEEP)
/*
 * Tick generation on Linux can be done via real-time signals, real-time
 * sleep or sleep. The default is real-time sleep.
 * NOTE: real-time signals on Arm/Linux has odd behavior.
 *
 * To enable real-time signals enable the following macros and comment
 * out USE_TIMER_THREAD_RT
 * #define USE_TIMER_SIGNAL
 * #define USE_TIMER_THREAD_SEMAPHORE
 */
#if OSAPI_ENABLE_POSIX_RTTIMER
#define USE_TIMER_SIGNAL
#define USE_TIMER_THREAD_SEMAPHORE
#else
#if defined(RTI_DEOS_RTEMS)
#define USE_TIMER_THREAD_SLEEP
#else
#define USE_TIMER_THREAD_RT
#endif
#endif /* OSAPI_ENABLE_POSIX_RTTIMER */

#ifdef USE_TIMER_SIGNAL
#include <signal.h>
#endif
#define USE_TIMER_THREAD_RT
#else
#define USE_TIMER_THREAD_SLEEP
#endif
#else 
/* ENABLE_FACE_COMPLIANCE > NONE */
#define USE_TIMER_THREAD_SEMAPHORE
#define USE_TIMER_SIGNAL
#include <signal.h>
#endif

#elif defined(RTI_VXWORKS)
#define USE_TIMER_THREAD_SEMAPHORE
#define USE_TIMER_SIGNAL
#include <signal.h>
#ifndef RTI_CERT
#include <hostLib.h>
#define OSAPI_POSIX_GETHOSTNAME 1
#else
#define OSAPI_POSIX_GETHOSTNAME 0
#endif /* !RTI_CERT */
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_, len_) \
    do { \
        UNUSED_ARG(len_);\
        if (printf("%s", buf_)){}\
    } while (0)
#endif

#elif defined(RTI_QNX) /* __linux__ */
#define USE_TIMER_SIGNAL
#define USE_TIMER_THREAD_SEMAPHORE
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif
#else
/*******************************************************************************
 * Generic POSIX definitions
 ******************************************************************************/
#define USE_TIMER_THREAD_SLEEP
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif
#endif /* OS */

/* Disable printf for FACE compliance
*/
#if  ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE
#undef OSAPI_LOG_WRITE_BUFFER
#endif

#ifndef OSAPI_POSIX_GETHOSTNAME
#define OSAPI_POSIX_GETHOSTNAME 1
#endif 

#endif /* osapi_os_posix_h */
