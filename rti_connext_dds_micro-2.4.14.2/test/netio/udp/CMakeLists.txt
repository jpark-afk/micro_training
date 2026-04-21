###############################################################################
# (c) Copyright, Real-Time Innovations 2020-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

IF ((RTIME_SHIP_SOURCE) OR ((DEFINED RTIME_DEVTREE) AND (NOT ${RTIME_DEVTREE})))
    SET(RTI_SUBMODULE_TEST_SRC UDPInterfaceTester.c UDPTransformTester.c)
    SET(RTI_SUBMODULE_TEST_HDR UDPInterfaceTester.h UDPTransformTester.h)
ELSE()
    SET(RTI_SUBMODULE_TEST_SRC UDPInterfaceTester.c UDPInterfaceTesterInternal.c UDPTransformTester.c)
    SET(RTI_SUBMODULE_TEST_HDR UDPInterfaceTester.h UDPInterfaceTesterInternal.h UDPTransformTester.h)
ENDIF()

SET(RTI_SHIP_TEST true)

