/*
 * FILE: disc_dpde_dll.h - DPDE DLL external linkage
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
 * 16aug2011,tk Created
 */
/*ce
 * \file
 * \brief DPDE DLL external linkage
 */
#ifndef disc_dpde_dll_h
#define disc_dpde_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_disc_dpde_DLL_EXPORT
#define RTI_disc_dpde_DLL_EXPORT
#endif
#ifndef RTI_disc_dpde_DLL_VARIABLE
#define RTI_disc_dpde_DLL_VARIABLE
#endif
#endif

#if defined(RTI_disc_dpde_DLL_EXPORT)
#define DISC_DPDEDllExport __declspec( dllexport )
#else
#define DISC_DPDEDllExport
#endif /* RTI_disc_dpde_DLL_EXPORT */

#if defined(RTI_disc_dpde_DLL_VARIABLE)
#if defined(RTI_disc_dpde_DLL_EXPORT)
#define DISC_DPDEDllVariable __declspec( dllexport )
#else
#define DISC_DPDEDllVariable __declspec( dllimport )
#endif /* RTI_disc_dpde_DLL_EXPORT */
#else
#define DISC_DPDEDllVariable
#endif /* RTI_disc_dpde_DLL_VARIABLE */
#else
#define DISC_DPDEDllExport
#define DISC_DPDEDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */




#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* disc_dpde_dll_h */
