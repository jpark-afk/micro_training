/*
 * FILE: netio_tintf_dll.h - NETIO external linkage
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
 * 04jan2012,tk Written
 */
/*ce
 * \file
 */
#ifndef netio_tintf_dll_h
#define netio_tintf_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_netio_tintf_DLL_EXPORT
#define RTI_netio_tintf_DLL_EXPORT
#endif
#ifndef RTI_netio_tintf_DLL_VARIABLE
#define RTI_netio_tintf_DLL_VARIABLE
#endif
#endif

#if defined(RTI_netio_tintf_DLL_EXPORT)
#define NETIO_TINTFDllExport __declspec( dllexport )
#else
#define NETIO_TINTFDllExport
#endif /* RTI_netio_tintf_DLL_EXPORT */

#if defined(RTI_netio_tintf_DLL_VARIABLE)
#if defined(RTI_netio_tintf_DLL_EXPORT)
#define NETIO_TINTFDllVariable __declspec( dllexport )
#else
#define NETIO_TINTFDllVariable __declspec( dllimport )
#endif /* RTI_netio_tintf_DLL_EXPORT */
#else
#define NETIO_TINTFDllVariable
#endif /* RTI_netio_tintf_DLL_VARIABLE */
#else
#define NETIO_TINTFDllExport
#define NETIO_TINTFDllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* netio_tintf_dll_h */
