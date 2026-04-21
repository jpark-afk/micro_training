/*
 * FILE: autosarSocketTester.h - AUTOSAR socket unit tests
 *
 * (c) Copyright, Real-Time Innovations, 2020-2021
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
 * 17mar2020,fmt Written
 */
/*ce
 * \file
 * \brief NETIO Autosar socket unit-tests
 */
#ifndef autosarSocketTester_h
#define autosarSocketTester_h

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

/*ci
 * \brief Runs NETIO Autosar socket unit-tests
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if all tests passed correctly. RTI_FALSE if any
 *         of the tests failed.
 */
extern unsigned char
autosarSocketTester_run(struct UTEST_Context *setting);

#endif /* autosarSocketTester_h */
