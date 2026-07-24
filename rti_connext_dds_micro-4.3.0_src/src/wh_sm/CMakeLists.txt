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

SET(RTI_MODULE_NAME wh_sm)
SET(RTI_SUBMODULES history)

IF (NOT RTIME_BUILD_SINGLE_LIB)
SET(RTI_MODULE_IMPORTS dds_c)
SET(RTI_MODULE_EXPORT whsm)
SET(LAST_MODULE wh_sm)
ELSE()
    IF (RTIME_EXCLUDE_DPSE AND RTIME_EXCLUDE_DPDE)
        SET(RTI_OBJECT_DEPENDS dds_c rtps netio rt db cdr reda osapi dds_c rh_sm)
        SET(RTI_MODULE_EXPORT librti_me)
        SET(RTI_SOURCE_INCLUDE dds_c.2.0/srcC/infrastructure
                           dds_c.2.0/srcC/domain
                           dds_c.2.0/srcC/publication
                           dds_c.2.0/srcC/subscription
                           dds_c.2.0/srcC/topic
                           dds_c.2.0/srcC/type)
    ELSE()
        SET(RTI_OBJECT_MODULE TRUE)
        SET(RTI_MODULE_IMPORTS dds_c rtps netio rt db cdr reda osapi)
    ENDIF()
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)