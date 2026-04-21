/*
 * FILE: wh_sm.dll - WriterHistory DLL external linkage
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
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
 * 30apr2011,tk Created
 */
/*ce
 * \file
 * \brief WriterHistory DLL external linkage
 */
#ifndef wh_sm_dll_h
#define wh_sm_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_wh_sm_DLL_EXPORT
#define RTI_wh_sm_DLL_EXPORT
#endif
#ifndef RTI_wh_sm_DLL_VARIABLE
#define RTI_wh_sm_DLL_VARIABLE
#endif
#endif

#if defined(RTI_wh_sm_DLL_EXPORT)
#define WHSMDllExport __declspec( dllexport )
#else
#define WHSMDllExport
#endif /* RTI_history_DLL_EXPORT */

#if defined(RTI_wh_sm_DLL_VARIABLE)
#if defined(RTI_history_DLL_EXPORT)
#define WHSMDllVariable __declspec( dllexport )
#else
#define WHSMDllVariable __declspec( dllimport )
#endif /* RTI_history_DLL_EXPORT */
#else
#define WHSMDllVariable
#endif /* RTI_history_DLL_VARIABLE */
#else
#define WHSMDllExport
#define WHSMDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */




#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* wh_sm_dll_h */
