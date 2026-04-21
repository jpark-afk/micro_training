/*
 * FILE: test_android.h - Unit-test support for Android
 *
 * (c) Copyright, Real-Time Innovations, 2011-2020.
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
 * 30apr2014,tk  Android specific test settings.
 */
/*ce
 * \file
 * \brief Unit-test support for Android
 */
#ifndef test_android_h
#define test_android_h


#define UTEST_ARG_STRING(argv0_) \
            "-property netio.udp.allow_interface_multicast=1 " \
            "-property netio.udp.allow_interface=wlan0 " \
            "-property netio.udp.allow_interface_address=0x0a1e0171 " \
            "-property netio.udp.allow_interface_netmask=0xff000000 " \
            "-property netio.udp.multicast_if=lo " \
            "-property osapi.system.my_hostname=localhost " \
            "-id 80"

#endif
