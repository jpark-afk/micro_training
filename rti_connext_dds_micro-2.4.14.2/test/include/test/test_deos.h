/*
 * FILE: test_deos.h - Unit-test support for Deos
 *
 * (c) Copyright, Real-Time Innovations, 2022-2022.
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
 * \brief Unit-test support for Deos 653
 */
#ifndef test_deos_h
#define test_deos_h

/*e \brief
 * Arguments used to run port validation tests.
 */
#define UTEST_ARG_STRING(argv0_) \
            "-property osapi.system.my_hostname=Deos653-host"

#endif /* test_deos */