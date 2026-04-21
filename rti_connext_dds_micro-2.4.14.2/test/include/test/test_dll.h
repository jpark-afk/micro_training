/*
 * FILE: test_dll.h - Unit-test dll definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2020.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 14Oct2020,fmt Copyright added.
 */
/*ce
 * \file
 * \brief Unit-test DLL external linkage
 */
#ifndef test_dll_h
#define test_dll_h

#if defined(RTI_WIN32) || defined(RTI_WINCE)
#if defined(RTI_test_DLL_EXPORT)
#define RTITestDllExport __declspec( dllexport )
#else
#define RTITestDllExport
#endif /* RTI_test_DLL_EXPORT */

#if defined(RTI_test_DLL_VARIABLE)
#if defined(RTI_test_DLL_EXPORT)
#define RTITestDllVariable __declspec( dllexport )
#else
#define RTITestDllVariable __declspec( dllimport )
#endif /* RTI_test_DLL_EXPORT */
#else
#define RTITestDllVariable
#endif /* RTI_test_DLL_VARIABLE */
#else
#define RTITestDllExport
#define RTITestDllVariable
#endif /* RTI_WIN32 || RTI_WINCE */

#endif /* test_dll_h */
