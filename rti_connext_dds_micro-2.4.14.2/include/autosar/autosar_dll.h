/*
 * FILE: autosar_dll.h - AutoSAR external linkage
 *
 * (c) Copyright, Real-Time Innovations, 2020 - 2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief External linkage definitions for AutoSAR API
 */
#ifndef autosar_dll_h
#define autosar_dll_h

#if defined(RTI_WIN32) || defined(RTI_WINCE)
#if defined(RTI_autosar_DLL_EXPORT)
#define AutoSARDllExport __declspec( dllexport )
#else
#define AutoSARDllExport
#endif /* RTI_autosar_DLL_EXPORT */

#if defined(RTI_autosar_DLL_VARIABLE)
#if defined(RTI_autosar_DLL_EXPORT)
#define AutoSARDllVariable __declspec( dllexport )
#else
#define AutoSARDllVariable __declspec( dllimport )
#endif /* RTI_autosar_DLL_EXPORT */
#else
#define AutoSARDllVariable
#endif /* RTI_autosar_DLL_VARIABLE */
#else
#define AutoSARDllExport
#define AutoSARDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* autosar_dll_h */
