/* 
 (c) Copyright, Real-Time Innovations, 2011-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
------------ -------
1.0a,2011-05-18,eh  Created, from micro 1.0
=========================================================================*/

/*e \file
 * \brief RTPS Configuration
 *  
 * \details 
 * Configurable build flags for RTPS functionality 
 */

#ifndef rtps_dll_h
#define rtps_dll_h

#include "osapi/osapi_config.h"

#ifndef rtps_config_h
#include "rtps/rtps_config.h"
#endif


#if defined(RTI_WIN32) || defined(RTI_WINCE30)

#if defined(RTIME_DLL_EXPORT)
#ifndef RTI_rtps_DLL_EXPORT
#define RTI_rtps_DLL_EXPORT
#endif
#ifndef RTI_rtps_DLL_VARIABLE
#define RTI_rtps_DLL_VARIABLE
#endif
#endif

#if defined(RTI_rtps_DLL_EXPORT)
#define RTPSDllExport __declspec( dllexport )
#else
#define RTPSDllExport
#endif /* RTI_RTPS_DLL_EXPORT */

#if defined(RTI_rtps_DLL_VARIABLE)
#if defined(RTI_rtps_DLL_EXPORT)
#define RTPSDllVariable __declspec( dllexport )
#else
#define RTPSDllVariable __declspec( dllimport )
#endif /* RTI_RTPS_DLL_EXPORT */
#else
#define RTPSDllVariable
#endif /* RTI_RTPS_DLL_VARIABLE */
#else
#define RTPSDllExport
#define RTPSDllVariable
#endif /* RTI_WIN32 || RTI_WINCE30 */

#if defined(__cplusplus) && defined(RTI_USE_CPP_API)
#define RTI_CPP
#endif

#endif /* rtps_dll_h */
