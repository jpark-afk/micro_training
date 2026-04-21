/*
 * FILE: dds_filter_dll.h - DDS_FILTER external linkage definitions
 *
 * (c) Copyright, Real-Time Innovations, 2025-2025.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief DDS_FILTER external linkage definitions
 */
#ifndef dds_filter_dll_h
#define dds_filter_dll_h

#include "osapi/osapi_config.h"

#if defined(RTI_WIN32) || defined(RTI_WINCE)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_dds_filter_DLL_EXPORT
#define RTI_dds_filter_DLL_EXPORT
#endif
#ifndef RTI_dds_filter_DLL_VARIABLE
#define RTI_dds_filter_DLL_VARIABLE
#endif
#endif

#if defined(RTI_dds_filter_DLL_EXPORT)
#define DDS_FILTER_DllExport __declspec( dllexport )
#else
#define DDS_FILTER_DllExport
#endif /* RTI_dds_filter_DLL_EXPORT */

#if defined(RTI_dds_filter_DLL_VARIABLE)
#if defined(RTI_dds_filter_DLL_EXPORT)
#define DDS_FILTER_DllVariable __declspec( dllexport )
#else
#define DDS_FILTER_DllVariable __declspec( dllimport )
#endif /* RTI_dds_filter_DLL_EXPORT */
#else
#define DDS_FILTER_DllVariable
#endif /* RTI_dds_filter_DLL_VARIABLE */
#else
#define DDS_FILTER_DllExport
#define DDS_FILTER_DllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

#endif /* dds_filter_dll_h */
