/*
 * FILE: netio_zcopy_dll.h - ZeroCopy DLL external linkage
 *
 * (c) Copyright 2024-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief ZeroCopy DLL external linkage
 */
#ifndef netio_zcopy_dll_h
#define netio_zcopy_dll_h

#include "osapi/osapi_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_netio_zcopy_DLL_EXPORT
#define RTI_netio_zcopy_DLL_EXPORT
#endif
#ifndef RTI_netio_zcopy_DLL_VARIABLE
#define RTI_netio_zcopy_DLL_VARIABLE
#endif
#endif

#if defined(RTI_PSL) || defined(RTI_PIL)netio_zcopy_defaul
#define NETIO_SHMEMV2_INTEGRATED_NETIOPSL 0
#else
#define NETIO_SHMEMV2_INTEGRATED_NETIOPSL 1
#endif

#ifndef NETIOPSLDllExport
#if defined(RTI_netiopsl_DLL_EXPORT) || \
         (NETIO_SHMEM_LINKED_NETIOPSL && defined(RTI_netio_zcopy_DLL_EXPORT))
#define NETIOPSLDllExport __declspec( dllexport )
#else
#define NETIOPSLDllExport
#endif /* RTI_netiopsl_DLL_EXPORT */
#endif /*NETIOPSLDllExport*/

#ifndef NETIOPSLDllVariable
#if defined(RTI_netiopsl_DLL_VARIABLE) || \
            (NETIO_SHMEMV2_INTEGRATED_NETIOPSL && defined(RTI_netio_zcopy_DLL_VARIABLE))
#if defined(RTI_netiopsl_DLL_EXPORT) || \
            (NETIO_SHMEMV2_INTEGRATED_NETIOPSL && defined(RTI_netio_zcopy_DLL_EXPORT))
#define NETIOPSLDllVariable __declspec( dllexport )
#else
#define NETIOPSLDllVariable __declspec( dllimport )
#endif /* RTI_netiopsl_DLL_EXPORT */
#else
#define NETIOPSLDllVariable
#endif /* RTI_netiopsl_DLL_EXPORT */
#endif /*NETIOPSLDllVariable*/

#if defined(RTI_netio_zcopy_DLL_EXPORT)
#define NETIO_ZCOPYDllExport __declspec( dllexport )
#else
#define NETIO_ZCOPYDllExport
#endif /* RTI_netio_zcopy_DLL_EXPORT */

#if defined(RTI_netio_zcopy_DLL_VARIABLE)
#if defined(RTI_netio_zcopy_DLL_EXPORT)
#define NETIO_ZCOPYDllVariable __declspec( dllexport )
#else
#define NETIO_ZCOPYDllVariable __declspec( dllimport )
#endif /* RTI_netio_zcopy_DLL_EXPORT */
#else
#define NETIO_ZCOPYDllVariable
#endif /* RTI_netio_zcopy_DLL_EXPORT */
#else
#define NETIO_ZCOPYDllExport
#define NETIO_ZCOPYDllVariable
#ifndef NETIOPSLDllExport
#define NETIOPSLDllExport
#endif
#ifndef NETIOPSLDllVariable
#define NETIOPSLDllVariable
#endif


#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_dll_h */
