/*
 * FILE: TimerTester.h - Timer unit-test
 *
 * (c) Copyright Real-Time Innovations, 2008-2021.
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
 * \brief Timer tester
 */
#ifndef TimerTester_h
#define TimerTester_h

/*ci
 * \brief Runs OSAPI Timer unit-tests
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if all tests passed correctly. RTI_FALSE if any
 *         of the tests failed.
 */
extern unsigned char
OSAPI_TimerTester_run(struct UTEST_Context *setting);

#endif /* TimerTester_h */
