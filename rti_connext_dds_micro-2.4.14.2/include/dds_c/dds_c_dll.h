/*
 * FILE: dds_c_dll.h - External linkage definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS_C external linkage definitions
 */
#ifndef dds_c_dll_h
#define dds_c_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_dds_c_DLL_EXPORT
#define RTI_dds_c_DLL_EXPORT
#endif
#ifndef RTI_dds_c_DLL_VARIABLE
#define RTI_dds_c_DLL_VARIABLE
#endif
#endif

#if defined(RTI_dds_c_DLL_EXPORT)
#define DDSCDllExport __declspec( dllexport )
#else
#define DDSCDllExport
#endif /* RTI_dds_c_DLL_EXPORT */

#if defined(RTI_dds_c_DLL_VARIABLE)
#if defined(RTI_dds_c_DLL_EXPORT)
#define DDSCDllVariable __declspec( dllexport )
#else
#define DDSCDllVariable __declspec( dllimport )
#endif /* RTI_dds_c_DLL_EXPORT */
#else
#define DDSCDllVariable
#endif /* RTI_dds_c_DLL_VARIABLE */
#else
#define DDSCDllExport
#define DDSCDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#else
#define DDSCPPDllExport
#endif

#endif /* dds_c_dll_h */
