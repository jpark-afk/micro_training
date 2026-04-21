/*
 * FILE: test_ios.c - Unit-test support for iOS
 *
 * (c) Copyright, Real-Time Innovations, 2004-2020.
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
 * 30apr2014,tk  iOS specific test settings.
 *
 */
/*ce
 * \file
 */
#ifndef test_ios_h
#define test_ios_h

#if defined(RTI_IOS)

#include "test/test_setting.h"

#if TARGET_IPHONE_SIMULATOR

#define UTEST_ARG_STRING(argv0_) \
        "-property netio.udp.allow_interface_multicast=1 " \
        "-property netio.udp.allow_interface=en0 " \
        "-property netio.udp.allow_interface_address=0xc0a80a84 " \
        "-property netio.udp.allow_interface_netmask=0xff000000 " \
        "-property netio.udp.multicast_if=en0 " \
        "-property osapi.system.my_hostname=iOSs-MacBook-Pro.local " \
        "-id 120"

#elif TARGET_OS_IPHONE

/* "-property netio.udp.allow_interface_address=0x0a1e018c " */

#define UTEST_ARG_STRING(argv0_) \
        " -property netio.udp.allow_interface_multicast=1 " \
        "-property netio.udp.allow_interface=en0 " \
        "-property netio.udp.allow_interface_address=0x0a1e01ca " \
        "-property netio.udp.allow_interface_netmask=0xff000000 " \
        "-property netio.udp.multicast_if=en0 " \
        "-property osapi.system.my_hostname=iPadMicro " \
        "-id 120"

#else
#error "Unknown iOS target"
#endif

#endif
#endif

