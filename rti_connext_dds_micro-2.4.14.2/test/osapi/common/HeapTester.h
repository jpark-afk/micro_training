/*
 * FILE: HeapTester.h - Heap unit-test declarations
 *
 * (c) Copyright, Real-Time Innovations, 2016-2021.
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
 * 15sep2016,eh  Written
 */
/*ce
 * \file
 * \brief HeapTester
 */
#ifndef HeapTester_h
#define HeapTester_h

/*ci
 * \brief Runs OSAPI Heap unit-tests
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if all tests passed correctly. RTI_FALSE if any
 *         of the tests failed.
 */
extern unsigned char
OSAPI_HeapTester_run(struct UTEST_Context *setting);

#endif
