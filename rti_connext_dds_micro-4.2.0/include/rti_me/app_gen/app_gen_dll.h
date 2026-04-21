/*
 * FILE: appgen_dll.h - App Generator external linkage
 *
 * (c) Copyright, Real-Time Innovations, 2017-2018.
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
 * \brief App Generator DLL external linkage
 */
#ifndef appgen_dll_h
#define appgen_dll_h

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_app_gen_DLL_EXPORT
#define RTI_app_gen_DLL_EXPORT
#endif
#ifndef RTI_app_gen_DLL_VARIABLE
#define RTI_app_gen_DLL_VARIABLE
#endif
#endif

#if defined(RTI_app_gen_DLL_EXPORT)
#define APPGENDllExport __declspec( dllexport )
#else
#define APPGENDllExport
#endif /* RTI_appgen_DLL_EXPORT */

#if defined(RTI_app_gen_DLL_VARIABLE)
#if defined(RTI_app_gen_DLL_EXPORT)
#define APPGENDllVariable __declspec( dllexport )
#else
#define APPGENDllVariable __declspec( dllimport )
#endif /* RTI_appgen_DLL_EXPORT */
#else
#define APPGENDllVariable
#endif /* RTI_appgen_DLL_VARIABLE */
#else
#define APPGENDllExport
#define APPGENDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */


#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif


#endif /* appgen_dll_h */
