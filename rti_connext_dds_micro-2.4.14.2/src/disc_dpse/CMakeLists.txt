###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2018
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME disc_dpse)
SET(RTI_SUBMODULES dpse_plugin)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_MODULE_IMPORTS dds_c)
    SET(RTI_MODULE_TEST_IMPORT rh_sm wh_sm netio_tintf)
    SET(RTI_MODULE_EXPORT discdpse)
ELSE()
    SET(RTI_MODULE_TEST_IMPORT netio_tintf)
    # CPP is in its own libray, stop at DPDE. RTIME_EXCLUDE_CPP AND
    IF (RTIME_EXCLUDE_DPDE)
        SET(RTI_OBJECT_DEPENDS dds_c rtps netio rt db cdr reda osapi dds_c
                               rh_sm wh_sm)
        SET(RTI_MODULE_EXPORT librti_me)
        SET(RTI_SOURCE_INCLUDE dds_c.2.0/srcC/infrastructure
                           dds_c.2.0/srcC/domain
                           dds_c.2.0/srcC/publication
                           dds_c.2.0/srcC/subscription
                           dds_c.2.0/srcC/topic
                           dds_c.2.0/srcC/type)
    ELSE()
        SET(RTI_MODULE_IMPORTS dds_c rtps netio rt db cdr reda osapi)
        SET(RTI_OBJECT_MODULE TRUE)
    ENDIF()
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
