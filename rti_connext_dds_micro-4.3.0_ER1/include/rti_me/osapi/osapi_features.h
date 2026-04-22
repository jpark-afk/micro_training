/*
 * FILE: osapi_features.h - OS Configuration
 *
 * Copyright (c) 2024-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_features_h
#define osapi_features_h

/*ci \brief Macro to stringify a macro using the C preprocessor
 */
#define OSAPI_CC_STRINGIFY_DEFINE(v_) OSAPI_CC_STRINGIFY_VALUE(v_)

/*ci \brief Macro to stringify a value using the C preprocessor
 */
#define OSAPI_CC_STRINGIFY_VALUE(v_) #v_

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif

/* The RTI_MICRO macro is used by code from RTI Connext DDS which is shared
 * with RTI Connext DDS Micro. */
#ifndef RTI_MICRO
#define RTI_MICRO
#endif

#ifndef DDS_FILTERING_ENABLED
#define DDS_FILTERING_ENABLED 1
#endif

/*
 * ENABLE_FACE_COMPLIANCE:
 * 0 - No compliance with FACE is required
 * 1 - Comply with the SECURITY profile (not supported)
 * 2 - Comply with the SAFTEY_BASE profile (supported)
 * 3 - Comply with the SAFETY_EXTENDED profile (supported)
 * 4 - Comply with the GENERAL profile (supported)
 */
#define FACE_COMPLIANCE_LEVEL_NONE             0
#define FACE_COMPLIANCE_LEVEL_SECURITY         1
#define FACE_COMPLIANCE_LEVEL_SAFETY_BASE      2
#define FACE_COMPLIANCE_LEVEL_SAFETY_EXTENDED  3
#define FACE_COMPLIANCE_LEVEL_GENERAL          4

#ifndef ENABLE_FACE_COMPLIANCE
#define ENABLE_FACE_COMPLIANCE                 FACE_COMPLIANCE_LEVEL_NONE
#elif (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) && \
      (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFETY_EXTENDED)
/* Safety Base and stricter profiles require RTI Cert flag */
#ifndef RTI_CERT
#define RTI_CERT
#endif /* RTI_CERT */
#endif /* ENABLE_FACE_COMPLIANCE */

#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) && \
    (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFTEY_EXTENDED)
#error "RTI Connext Micro only support FACE profile Safety Extended and higher"
#endif

/* FACE non-General profile compliance */
#if ((ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) && \
     (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_GENERAL))
#define ENABLED_FACE_COMPLIANCE_NON_GENERAL 1
#else
#define ENABLED_FACE_COMPLIANCE_NON_GENERAL 0
#endif

/* Enable/Disable default logging unless already defined */
#ifndef OSAPI_ENABLE_LOG
#if NDEBUG && defined(RTI_CERT)
#define OSAPI_ENABLE_LOG 0
#else
#define OSAPI_ENABLE_LOG 1
#endif
#endif

#ifndef OSAPI_ENABLE_SHMEM
#define OSAPI_ENABLE_SHMEM 1
#endif

/* Enable/Disable default tracing unless already defined */
#ifndef OSAPI_ENABLE_TRACE
#if NDEBUG || defined(RTI_CERT)
#define OSAPI_ENABLE_TRACE 0
#else
#define OSAPI_ENABLE_TRACE 1
#endif
#endif

#if OSAPI_ENABLE_TRACE && !OSAPI_ENABLE_LOG
#warning "OSAPI_ENABLE_TRACE=1 requires OSAPI_ENABLE_LOG=1, but OSAPI_ENABLE_LOG=0. Forcing OSAPI_ENABLE_LOG=1"
#ifdef OSAPI_ENABLE_LOG
#undef OSAPI_ENABLE_LOG
#endif
#define OSAPI_ENABLE_LOG 1
#endif

/* Enable/Disable default precondition unless already defined */
#ifndef OSAPI_ENABLE_PRECONDITION
#if NDEBUG || defined(RTI_CERT)
#define OSAPI_ENABLE_PRECONDITION 0
#else
#define OSAPI_ENABLE_PRECONDITION 1
#endif
#endif

/* Enable native long double (128 bit float). This is not supported on
 * all platforms. The default is to implement it as a 16 byte octet array
 * since Micro does not actually use the value.
 */
#ifndef OSAPI_ENABLE_LONG_DOUBLE
#define OSAPI_ENABLE_LONG_DOUBLE 0
#endif

#ifdef RTI_CERT
#ifndef OSAPI_ENABLE_THREAD_SEMAPHORE
#define OSAPI_ENABLE_THREAD_SEMAPHORE (0)
#endif
#else
#ifndef OSAPI_ENABLE_THREAD_SEMAPHORE
#define OSAPI_ENABLE_THREAD_SEMAPHORE (1)
#endif
#endif

#if OSAPI_ENABLE_THREAD_SEMAPHORE
#define OSAPI_THREAD_SEMAPHORE_ENABLED (1)
#else
#define OSAPI_THREAD_SEMAPHORE_ENABLED (0)
#endif

#endif /* osapi_features_h */
