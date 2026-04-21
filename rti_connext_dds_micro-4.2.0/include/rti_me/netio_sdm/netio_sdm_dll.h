/*
 * FILE: netio_sdm_dll.h
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_sdm_dll_h
#define netio_sdm_dll_h

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_netio_sdm_DLL_EXPORT
#define RTI_netio_sdm_DLL_EXPORT
#endif
#ifndef RTI_netio_sdm_DLL_VARIABLE
#define RTI_netio_sdm_DLL_VARIABLE
#endif
#endif

#if defined(RTI_netio_sdm_DLL_EXPORT)
#define NETIO_SDMDllExport __declspec( dllexport )
#else
#define NETIO_SDMDllExport
#endif /* RTI_NETIO_NAME_DLL_EXPORT */

#if defined(RTI_netio_sdm_DLL_VARIABLE)
#if defined(RTI_netio_sdm_DLL_EXPORT)
#define NETIO_SDMDllVariable __declspec( dllexport )
#else
#define NETIO_SDMDllVariable __declspec( dllimport )
#endif /* RTI_NETIO_NAME_DLL_EXPORT */
#else
#define NETIO_SDMDllVariable
#endif /* RTI_NETIO_NAME_DLL_VARIABLE */
#else
#define NETIO_SDMDllExport
#define NETIO_SDMDllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* netio_sdm_dll_h */
