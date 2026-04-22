/*
 * FILE: rt_dll.h - DB external linkage
 *
 * Copyright 2012-2015 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04jan2012,tk Written
 */
/*ce
 * \file
 */
#ifndef rt_dll_h
#define rt_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)

#ifndef RTI_rt_DLL_EXPORT
#define RTI_rt_DLL_EXPORT
#endif
#ifndef RTI_rt_DLL_VARIABLE
#define RTI_rt_DLL_VARIABLE
#endif

#endif

#if defined(RTI_rt_DLL_EXPORT)
#define RTDllExport __declspec( dllexport )
#else
#define RTDllExport
#endif /* RTI_rt_DLL_EXPORT */

#if defined(RTI_rt_DLL_VARIABLE)
#if defined(RTI_rt_DLL_EXPORT)
#define RTDllVariable __declspec( dllexport )
#else
#define RTDllVariable __declspec( dllimport )
#endif /* RTI_rt_DLL_EXPORT */
#else
#define RTDllVariable
#endif /* RTI_rt_DLL_VARIABLE */
#else
#define RTDllExport
#define RTDllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#else
#define DDSCPPDllExport
#endif

#endif /* rt_dll_h */
