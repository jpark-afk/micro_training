###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME rh_sm)
SET(RTI_SUBMODULES history)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_MODULE_IMPORTS dds_c)
    SET(RTI_MODULE_EXPORT rhsm)
ELSE()
    SET(RTI_OBJECT_MODULE TRUE)
    SET(RTI_MODULE_IMPORTS dds_c rtps netio rt db cdr reda osapi)
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
