/*
 * FILE: netio_proxy_dll.h - netio_proxy external linkage
 *
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_proxy_dll_h
#define netio_proxy_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_netio_proxy_DLL_EXPORT
#define RTI_netio_proxy_DLL_EXPORT
#endif
#ifndef RTI_netio_proxy_DLL_VARIABLE
#define RTI_netio_proxy_DLL_VARIABLE
#endif
#endif

#if defined(RTI_netio_proxy_DLL_EXPORT)
#define NETIO_PROXYDllExport __declspec( dllexport )
#else
#define NETIO_PROXYDllExport
#endif /* RTI_NETIO_NAME_DLL_EXPORT */

#if defined(RTI_netio_proxy_DLL_VARIABLE)
#if defined(RTI_netio_proxy_DLL_EXPORT)
#define NETIO_PROXYDllVariable __declspec( dllexport )
#else
#define NETIO_PROXYDllVariable __declspec( dllimport )
#endif /* RTI_NETIO_NAME_DLL_EXPORT */
#else
#define NETIO_PROXYDllVariable
#endif /* RTI_NETIO_NAME_DLL_VARIABLE */
#else
#define NETIO_PROXYDllExport
#define NETIO_PROXYDllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* netio_proxy_dll_h */
