/*
 * FILE: xcdr_dll_psm.h
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef xcdr_dll_psm_h
#define xcdr_dll_psm_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)
  #if defined(RTI_xcdr_DLL_EXPORT)
    #define RTIXCdrDllExport __declspec( dllexport )
  #else
    #define RTIXCdrDllExport
  #endif /* RTI_xcdr_DLL_EXPORT */

  #if defined(RTI_xcdr_DLL_VARIABLE)
    #if defined(RTI_xcdr_DLL_EXPORT)
      #define RTIXCdrDllVariable __declspec( dllexport )
    #else
      #define RTIXCdrDllVariable __declspec( dllimport )
    #endif /* RTI_xcdr_DLL_EXPORT */
  #else 
    #define RTIXCdrDllVariable
  #endif /* RTI_xcdr_DLL_VARIABLE */
#else
  #define RTIXCdrDllExport
  #define RTIXCdrDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */


#endif /* xcdr_dll_psm_h */
