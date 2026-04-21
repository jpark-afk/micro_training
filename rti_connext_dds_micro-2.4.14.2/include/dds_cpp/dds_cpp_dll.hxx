/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

Description
-----------
Strictly for Windows DLL linkage resolution

modification history
------------ -------
2.0,2013jan11,eh  Created. 
===================================================================== */

#ifndef dds_cpp_dll_hxx
#define dds_cpp_dll_hxx

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)
  #if defined(RTI_dds_cpp_DLL_EXPORT)
    #define DDSCPPDllExport __declspec( dllexport )
  #else
    #define DDSCPPDllExport
  #endif /* RTI_dds_cpp_DLL_EXPORT */

  #if defined(RTI_dds_cpp_DLL_VARIABLE) 
    #if defined(RTI_dds_cpp_DLL_EXPORT)
      #define DDSCPPDllVariable __declspec( dllexport )
    #else
      #define DDSCPPDllVariable __declspec( dllimport )
    #endif /* RTI_dds_cpp_DLL_EXPORT */
  #else 
    #define DDSCPPDllVariable
  #endif /* RTI_dds_cpp_DLL_VARIABLE */
#else
  #define DDSCPPDllExport
  #define DDSCPPDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

/* Use the C++ constructs that are included in the DDS/C interfaces. */
#ifndef RTI_USE_CPP_API
#define RTI_USE_CPP_API
#endif

#endif /* dds_cpp_dll_hxx */


