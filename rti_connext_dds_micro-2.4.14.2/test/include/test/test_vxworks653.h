/*
 * FILE: test_vxworks653.h - Unit-test support for VxWorks 653
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
 * \brief Unit-test support for VxWorks 653
 */
#ifndef test_vxwork653_h
#define test_vxwork653_h

/*e \brief
 * Arguments used to run port validation tests.
 */
#define UTEST_ARG_STRING(argv0_) \
            "-property netio.udp.allow_interface_multicast=1 " \
            "-property netio.udp.allow_interface=eth0 " \
            "-property netio.udp.allow_interface_address=" UTEST_STRINGIFY_DEFINE(RTI_TEST_IP_HEX) " " \
            "-property netio.udp.allow_interface_netmask=" UTEST_STRINGIFY_DEFINE(RTI_TEST_NETMASK) " " \
            "-property netio.udp.multicast_if=eth0 " \
            "-property osapi.system.my_hostname=VxWorks653-host "

#endif /* test_vxwork653 */