/*
 * FILE: ospsl_os_posix.h - OS configuration file for POSIX/POSIX like OSs
 *
 * Copyright (c) 2023-2026 Real-Time Innovations, Inc.
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
 * 13apr2023,tk Copied and renamed from osapi_os_posix.h.
 * - Note that the implementation is identical and nothing has been renamed
 */
/*ce \file
 *   \brief OS PSL support for POSIX
 */
#ifndef ospsl_os_posix_h
#define ospsl_os_posix_h

#include "osapi/osapi_features.h"

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
#else
#error "Unable to detect POSIX variant. Please define RTIPSL_OS_DEF_H."
#endif

/* Claim compliance with IEEE Std 1003.1, 2004 Edition */
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200112L

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 600

#ifndef RTI_UNIX
#define RTI_UNIX
#endif

/* This is needed to get non-POSIX APIs for Linux/Darwin */
#if !OSAPI_ENABLE_STRICT_POSIX
#ifdef RTI_DARWIN
#define _DARWIN_C_SOURCE
#endif
#ifdef RTI_LINUX
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#endif /* OSAPI_STRICT_POSIX */

/* Standard POSIX headers */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>

#define HAVE_SSIZE_T 1

/* Assume all supported UNIX variations support the BSD socket API */
#define HAVE_SOCKET_API        1

/* Use free() */
#define OSAPI_ENABLE_STDC_FREE 1
#define OSAPI_ENABLE_STDC_REALLOC 1


/******************************************************************************
 * QNX Specific definitions
 ******************************************************************************/
#if defined(RTI_QNX6)
#if OSAPI_ENABLE_SHMEM
#define OSAPI_SHMEM_IS_ENABLED 1
#else
#define OSAPI_SHMEM_IS_ENABLED 0
#endif
#define OSAPI_HAVE_NANOSLEEP (1)

/*
 * MICRO-6013: Automatically enable using the robust pthread mutex for QNX
 *             instead of the default SemMutex unless explicitly disabled.
 */
#ifndef RTI_DISABLE_ROBUST_PTHREAD_MUTEX
#define OSAPI_ROBUST_SHMEM_MUTEX 1
#endif
#endif

/*******************************************************************************
 * Darwin (Mac OS X, macOS) specific definitions
 ******************************************************************************/
#ifdef RTI_DARWIN

#if !defined(RTI_DARWIN7)
#include <uuid/uuid.h>
#endif

#if OSAPI_ENABLE_SHMEM
#define OSAPI_SHMEM_IS_ENABLED 1
#else
#define OSAPI_SHMEM_IS_ENABLED 0
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

#define OSAPI_HAVE_NANOSLEEP (1)

#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif

/*******************************************************************************
 * Linux specific definitions
 ******************************************************************************/
#elif defined(RTI_LINUX)

#if OSAPI_ENABLE_SHMEM
#define OSAPI_SHMEM_IS_ENABLED 1
#else
#define OSAPI_SHMEM_IS_ENABLED 0
#endif

#if defined(__ANDROID__)
#include <endian.h>
#else
#include <features.h>
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
extern int clock_nanosleep(clockid_t clock_id, int flags,
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
#ifdef USE_TIMER_SIGNAL
#include <signal.h>
#endif
#define USE_TIMER_THREAD_RT
#else /* ENABLE_FACE_COMPLIANCE */
#define USE_TIMER_THREAD_SLEEP
#endif /* !ENABLE_FACE_COMPLIANCE */
#endif

#define OSAPI_HAVE_NANOSLEEP (1)

#else /* __linux__ */
/*******************************************************************************
 * Generic POSIX definitions
 ******************************************************************************/
#define USE_TIMER_THREAD_SLEEP
#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif
#endif

/* Disable printf for FACE compliance
 */
#if  ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE
#undef OSAPI_LOG_WRITE_BUFFER
#endif

#endif /* osapi_os_posix_h */
