/* 

 (c) Copyright, Real-Time Innovations, 2013-2024.
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

#if defined(RTI_dds_cpp_DLL_EXPORT)
  #define DDSCPPDllExport OSAPI_CC_EXPORT_LIB_FUNCTION
#else
  #define DDSCPPDllExport
#endif /* RTI_dds_cpp_DLL_EXPORT */

#if defined(RTI_dds_cpp_DLL_VARIABLE) 
  #if defined(RTI_dds_cpp_DLL_EXPORT)
    #define DDSCPPDllVariable OSAPI_CC_EXPORT_LIB_VARIABLE
  #else
    #define DDSCPPDllVariable OSAPI_CC_IMPORT_LIB_VARIABLE
  #endif /* RTI_dds_cpp_DLL_EXPORT */
#else 
  #define DDSCPPDllVariable
#endif /* RTI_dds_cpp_DLL_VARIABLE */

/* Use the C++ constructs that are included in the DDS/C interfaces. */
#ifndef RTI_USE_CPP_API
#define RTI_USE_CPP_API
#endif

#endif /* dds_cpp_dll_hxx */


