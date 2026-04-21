/*
 * FILE: db_dll.h - DB external linkage
 *
 * (c) Copyright, Real-Time Innovations, 2012-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 28may2012,tk Written
 */
/*ce
 * \file
 * \brief External linkage definitions for DB API
 */
#ifndef db_dll_h
#define db_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)
#if defined(RTI_db_DLL_EXPORT)
#define DBDllExport __declspec( dllexport )
#else
#define DBDllExport
#endif /* RTI_db_DLL_EXPORT */

#if defined(RTI_db_DLL_VARIABLE)
#if defined(RTI_db_DLL_EXPORT)
#define DBDllVariable __declspec( dllexport )
#else
#define DBDllVariable __declspec( dllimport )
#endif /* RTI_db_DLL_EXPORT */
#else
#define DBDllVariable
#endif /* RTI_db_DLL_VARIABLE */
#else
#define DBDllExport
#define DBDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* db_dll_h */
