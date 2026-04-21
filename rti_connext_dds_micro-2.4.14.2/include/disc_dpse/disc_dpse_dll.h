/*
 * FILE: disc_dpse_dll.h - DPSE DLL external linkage
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
 * \brief DPSE DLL external linkage
 */
#ifndef disc_dpse_dll_h
#define disc_dpse_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_disc_dpse_DLL_EXPORT
#define RTI_disc_dpse_DLL_EXPORT
#endif
#ifndef RTI_disc_dpse_DLL_VARIABLE
#define RTI_disc_dpse_DLL_VARIABLE
#endif
#endif

#if defined(RTI_disc_dpse_DLL_EXPORT)
#define DISC_DPSEDllExport __declspec( dllexport )
#else
#define DISC_DPSEDllExport
#endif /* RTI_disc_dpse_DLL_EXPORT */

#if defined(RTI_disc_dpse_DLL_VARIABLE)
#if defined(RTI_disc_dpse_DLL_EXPORT)
#define DISC_DPSEDllVariable __declspec( dllexport )
#else
#define DISC_DPSEDllVariable __declspec( dllimport )
#endif /* RTI_disc_dpse_DLL_EXPORT */
#else
#define DISC_DPSEDllVariable
#endif /* RTI_disc_dpse_DLL_VARIABLE */
#else
#define DISC_DPSEDllExport
#define DISC_DPSEDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */




#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* disc_dpse_dll_h */
