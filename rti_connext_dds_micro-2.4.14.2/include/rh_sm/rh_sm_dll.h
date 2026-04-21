/*
 * FILE: rh_sm.dll - ReaderHistory DLL external linkage
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * \brief ReaderHistory DLL external linkage
 */
#ifndef rh_sm_dll_h
#define rh_sm_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_rh_sm_DLL_EXPORT
#define RTI_rh_sm_DLL_EXPORT
#endif
#ifndef RTI_rh_sm_DLL_VARIABLE
#define RTI_rh_sm_DLL_VARIABLE
#endif
#endif

#if defined(RTI_rh_sm_DLL_EXPORT)
#define RHSMDllExport __declspec( dllexport )
#else
#define RHSMDllExport
#endif /* RTI_rh_sm_DLL_EXPORT */

#if defined(RTI_rh_sm_DLL_VARIABLE)
#if defined(RTI_rh_sm_DLL_EXPORT)
#define RHSMDllVariable __declspec( dllexport )
#else
#define RHSMDllVariable __declspec( dllimport )
#endif /* RTI_rh_sm_DLL_EXPORT */
#else
#define RHSMDllVariable
#endif /* RTI_rh_sm_DLL_EXPORT */
#else
#define RHSMDllExport
#define RHSMDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */




#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* rh_sm_dll_h */
