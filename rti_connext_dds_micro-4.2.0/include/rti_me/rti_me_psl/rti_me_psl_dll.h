/*
 * FILE: rti_me_psl_dll.h - PSL external linkage
 *
 * Copyright (c) 2024-2025 Real-Time Innovations, Inc.
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
 */
#ifndef rti_me_psl_dll_h
#define rti_me_psl_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_rti_me_psl_DLL_EXPORT
#define RTI_rti_me_psl_DLL_EXPORT
#endif
#ifndef RTI_netiopsl_DLL_EXPORT
#define RTI_netiopsl_DLL_EXPORT
#endif
#ifndef RTI_netiopsl_cpp_DLL_EXPORT
#define RTI_netiopsl_cpp_DLL_EXPORT
#endif
#ifndef RTI_pskpsl_DLL_EXPORT
#define RTI_pskpsl_DLL_EXPORT
#endif
#ifndef RTI_ospsl_DLL_EXPORT
#define RTI_ospsl_DLL_EXPORT
#endif
#ifndef RTI_rti_me_psl_DLL_VARIABLE
#define RTI_rti_me_psl_DLL_VARIABLE
#endif
#ifndef RTI_netiopsl_DLL_VARIABLE
#define RTI_netiopsl_DLL_VARIABLE
#endif
#ifndef RTI_netiopsl_cpp_DLL_VARIABLE
#define RTI_netiopsl_cpp_DLL_VARIABLE
#endif
#ifndef RTI_pskpsl_DLL_VARIABLE
#define RTI_pskpsl_DLL_VARIABLE
#endif
#ifndef RTI_ospsl_DLL_VARIABLE
#define RTI_ospsl_DLL_VARIABLE
#endif
#endif /* RTIME_DLL_EXPORT */

#ifndef RTIME_PSLDllExport
#if defined(RTI_rti_me_psl_DLL_EXPORT)
#define RTIME_PSLDllExport __declspec( dllexport )
#else
#define RTIME_PSLDllExport
#endif /* RTI_reda_DLL_EXPORT */
#endif

#ifndef NETIOPSLDllExport
#if defined(RTI_netiopsl_DLL_EXPORT) || \
    (!defined(RTI_PSL) && defined(RTI_netio_DLL_EXPORT))
#define NETIOPSLDllExport __declspec( dllexport )
#else
#define NETIOPSLDllExport
#endif /* RTI_netiopsl_DLL_EXPORT */
#endif

#ifndef NETIOPSL_CPPDllExport
#if defined(RTI_netiopsl_cpp_DLL_EXPORT) || \
    (!defined(RTI_PSL) && (defined(RTI_netio_DLL_EXPORT) || defined(RTI_dds_cpp_DLL_EXPORT)))
#define NETIOPSL_CPPDllExport __declspec( dllexport )
#else
#define NETIOPSL_CPPDllExport
#endif /* RTI_netiopsl_cpp_DLL_EXPORT */
#endif

#ifndef PSKPSLDllExport
#if defined(RTI_pskpsl_DLL_EXPORT) || \
    (!defined(RTI_PSL) && defined(RTI_transform_DLL_EXPORT))
#define PSKPSLDllExport __declspec( dllexport )
#else
#define PSKPSLDllExport
#endif /* RTI_pskpsl_DLL_EXPORT */
#endif

#ifndef OSPSLDllExport
#if defined(RTI_ospsl_DLL_EXPORT) || \
    (!defined(RTI_PSL) && defined(RTI_osapi_DLL_EXPORT))
#define OSPSLDllExport __declspec( dllexport )
#else
#define OSPSLDllExport
#endif /* RTI_ospsl_DLL_EXPORT */
#endif

#ifndef RTIME_PSLDllVariable
#if defined(RTI_rti_me_psl_DLL_VARIABLE)
#if defined(RTI_rti_me_psl_DLL_EXPORT)
#define RTIME_PSLDllVariable __declspec( dllexport )
#else
#define RTIME_PSLDllVariable __declspec( dllimport )
#endif /* RTI_rti_me_psl_DLL_EXPORT */
#else
#define RTIME_PSLDllVariable
#endif /* RTI_rti_me_psl_DLL_VARIABLE */
#endif

#ifndef NETIOPSLDllVariable
#if defined(RTI_netiopsl_DLL_VARIABLE) || RTI_rti_me_DLL_VARIABLE || \
    (!defined(RTI_PSL) && defined(RTI_netio_DLL_VARIABLE))
#if defined(RTI_netiopsl_DLL_EXPORT) || defined(RTI_netio_DLL_EXPORT)
#define NETIOPSLDllVariable __declspec( dllexport )
#else
#define NETIOPSLDllVariable __declspec( dllimport )
#endif /* RTI_rti_me_psl_DLL_EXPORT */
#else
#define NETIOPSLDllVariable
#endif /* RTI_rti_me_psl_DLL_VARIABLE */
#endif

#ifndef NETIOPSL_CPPDllVariable
#if defined(RTI_netiopsl_cpp_DLL_VARIABLE) || RTI_rti_me_DLL_VARIABLE || \
    (!defined(RTI_PSL) && defined(RTI_netio_DLL_VARIABLE))
#if defined(RTI_netiopsl_cpp_DLL_EXPORT) || defined(RTI_netio_DLL_EXPORT) || defined(RTI_dds_cpp_DLL_EXPORT)
#define NETIOPSL_CPPDllVariable __declspec( dllexport )
#else
#define NETIOPSL_CPPDllVariable __declspec( dllimport )
#endif /* RTI_rti_me_psl_DLL_EXPORT */
#else
#define NETIOPSL_CPPDllVariable
#endif /* RTI_rti_me_psl_DLL_VARIABLE */
#endif

#ifndef PSKPSLDllVariable
#if defined(RTI_pskpsl_DLL_VARIABLE) || RTI_rti_me_DLL_VARIABLE || \
    (!defined(RTI_PSL) && defined(RTI_transform_DLL_VARIABLE))
#if defined(RTI_pskpsl_DLL_EXPORT) || RTI_transform_DLL_EXPORT
#define PSKPSLDllVariable __declspec( dllexport )
#else
#define PSKPSLDllVariable __declspec( dllimport )
#endif /* RTI_pskpsl_DLL_EXPORT */
#else
#define PSKPSLDllVariable
#endif /* RTI_pskpsl_DLL_VARIABLE */
#endif

#ifndef OSPSLDllVariable
#if defined(RTI_ospsl_DLL_VARIABLE) || RTI_rti_me_DLL_VARIABLE || \
    (!defined(RTI_PSL) && defined(RTI_osapi_DLL_VARIABLE))
#if defined(RTI_ospsl_DLL_EXPORT) || RTI_osapi_DLL_EXPORT
#define OSPSLDllVariable __declspec( dllexport )
#else
#define OSPSLDllVariable __declspec( dllimport )
#endif /* RTI_ospsl_DLL_EXPORT */
#else
#define OSPSLDllVariable
#endif /* RTI_ospsl_DLL_VARIABLE */
#endif

#else

#ifndef RTIME_PSLDllExport
#define RTIME_PSLDllExport
#endif

#ifndef RTIME_PSLDllVariable
#define RTIME_PSLDllVariable
#endif

#ifndef NETIOPSL_CPPDllExport
#define NETIOPSL_CPPDllExport
#endif

#ifndef NETIOPSL_CPPDllVariable
#define NETIOPSL_CPPDllVariable
#endif

#ifndef NETIOPSLDllExport
#define NETIOPSLDllExport
#endif

#ifndef NETIOPSLDllVariable
#define NETIOPSLDllVariable
#endif

#ifndef PSKPSLDllExport
#define PSKPSLDllExport
#endif

#ifndef PSKPSLDllVariable
#define PSKPSLDllVariable
#endif

#ifndef OSPSLDllExport
#define OSPSLDllExport
#endif

#ifndef OSPSLDllVariable
#define OSPSLDllVariable
#endif

#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* rti_me_psl_dll_h */
