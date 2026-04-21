/*
 * FILE: OSAPITester.h
 *
 * (c) Copyright, Real-Time Innovations, 2011-2015.
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
 * \brief OSAPITester
 */
#ifndef OSAPITester_h
#define OSAPITester_h

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char
OSAPITester_run(struct UTEST_Context *setting);

extern unsigned char
OSAPITester_start(int beginSubmoduleTestIndex,
                  int beginUnitTestIndex,
                  int endSubmoduleTestIndex,
                  int endUnitTestIndex, RTI_BOOL ignoreFailure,
                  const char* intf_name);

#ifdef __cplusplus
}    /* extern "C" */
#endif

#endif /* OSAPITester_h */
