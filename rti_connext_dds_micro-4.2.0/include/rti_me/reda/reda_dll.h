/*
 * FILE: reda_dll.h - REDA external linkage
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
#ifndef reda_dll_h
#define reda_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_reda_DLL_EXPORT
#define RTI_reda_DLL_EXPORT
#endif
#ifndef RTI_reda_DLL_VARIABLE
#define RTI_reda_DLL_VARIABLE
#endif
#endif

#if defined(RTI_reda_DLL_EXPORT)
#define REDADllExport __declspec( dllexport )
#else
#define REDADllExport
#endif /* RTI_reda_DLL_EXPORT */

#if defined(RTI_reda_DLL_VARIABLE)
#if defined(RTI_reda_DLL_EXPORT)
#define REDADllVariable __declspec( dllexport )
#else
#define REDADllVariable __declspec( dllimport )
#endif /* RTI_reda_DLL_EXPORT */
#else
#define REDADllVariable
#endif /* RTI_reda_DLL_VARIABLE */
#else
#define REDADllExport
#define REDADllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* reda_dll_h */
