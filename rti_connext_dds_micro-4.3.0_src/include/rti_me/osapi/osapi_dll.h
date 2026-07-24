/*
 * FILE: osapi_dll.h - External linkage definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2024
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
 * 22sep2008,tk Created
 */
#ifndef osapi_dll_h
#define osapi_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_PSL) || defined(RTI_PIL)
#define OSAPI_INTEGRATED_OSPSL 0
#else
#define OSAPI_INTEGRATED_OSPSL 1
#endif

#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_osapi_DLL_EXPORT
#define RTI_osapi_DLL_EXPORT
#endif
#ifndef RTI_osapi_DLL_VARIABLE
#define RTI_osapi_DLL_VARIABLE
#endif
#endif

#ifndef OSPSLDllExport
#if defined(RTI_ospsl_DLL_EXPORT) ||  \
    (OSAPI_INTEGRATED_OSPSL &&  defined(RTI_osapi_DLL_EXPORT))
#define OSPSLDllExport __declspec( dllexport )
#else
#define OSPSLDllExport
#endif /* RTI_ospsl_DLL_EXPORT */
#endif /* OSPSLDllExport */

#ifndef OSPSLDllVariable
#if defined(RTI_ospsl_DLL_VARIABLE) || RTI_rti_me_DLL_VARIABLE || \
(OSAPI_INTEGRATED_OSPSL && defined(RTI_osapi_DLL_VARIABLE))
#if defined(RTI_ospsl_DLL_EXPORT) || \
    (OSAPI_INTEGRATED_OSPSL &&  defined(RTI_osapi_DLL_EXPORT))
#define OSPSLDllVariable  __declspec( dllexport )
#else
#define OSPSLDllVariable  __declspec( dllimport )
#endif /* RTI_ospsl_DLL_EXPORT */
#else
#define OSPSLDllVariable
#endif /* RTI_ospsl_DLL_VARIABLE */
#endif /* OSPSLDllVariable */

#if defined(RTI_osapi_DLL_EXPORT)
#define OSAPIDllExport __declspec( dllexport)
#else
#define OSAPIDllExport
#endif /* RTI_osapi_DLL_EXPORT */

#if defined(RTI_osapi_DLL_VARIABLE)
#if defined(RTI_osapi_DLL_EXPORT)
#define OSAPIDllVariable __declspec( dllexport )
#else
#define OSAPIDllVariable  __declspec( dllimport )
#endif /* RTI_osapi_DLL_EXPORT */
#else
#define OSAPIDllVariable
#endif /* RTI_osapi_DLL_VARIABLE */
#else
#define OSAPIDllExport
#define OSAPIDllVariable
#ifndef OSPSLDllExport
#define OSPSLDllExport
#endif
#ifndef OSPSLDllVariable
#define OSPSLDllVariable
#endif
#endif /* RTI_WIN32 || RTI_WINCE30 */

#endif /* osapi_dll_h */
