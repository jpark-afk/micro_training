/*
 * FILE: cdr_dll.h - External linkage definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 22sep2008,kaj Created
 */
#ifndef cdr_dll_h
#define cdr_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)
 
#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_cdr_DLL_EXPORT
#define RTI_cdr_DLL_EXPORT
#endif
#ifndef RTI_cdr_DLL_VARIABLE
#define RTI_cdr_DLL_VARIABLE
#endif
#endif

#if defined(RTI_cdr_DLL_EXPORT)
#define CDRDllExport __declspec( dllexport )
#else
#define CDRDllExport
#endif /* RTI_cdr_DLL_EXPORT */

#if defined(RTI_cdr_DLL_VARIABLE)
#if defined(RTI_cdr_DLL_EXPORT)
#define CDRDllVariable __declspec( dllexport )
#else
#define CDRDllVariable __declspec( dllimport )
#endif /* RTI_cdr_DLL_EXPORT */
#else
#define CDRDllVariable
#endif /* RTI_cdr_DLL_VARIABLE */
#else
#define CDRDllExport
#define CDRDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */


#endif /* cdr_dll_h */
