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

IF (RTIME_EXCLUDE_DPDE AND NOT RTIME_SHIP_SOURCE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPDE=0)
    return()
ENDIF()

IF (NOT RTIME_SHIP_SOURCE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPDE=1)
ENDIF()

SET(RTI_MODULE_NAME disc_dpde)
SET(RTI_SUBMODULES dpde_plugin)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (RTIME_PIL)
        SET(RTI_MODULE_IMPORTS wh_sm rti_me_psl::netiopsl dds_c)
    ELSE()
        SET(RTI_MODULE_IMPORTS wh_sm rh_sm dds_c)
    ENDIF()
ELSE()
    SET(RTI_MODULE_DEPENDS wh_sm rh_sm rtps netio dds_c rt db cdr reda osapi)
    LIST(APPEND RTI_MODULE_DEPENDS rti_me_psl::ospsl)
ENDIF()

SET(RTI_MODULE_TEST_IMPORT rh_sm wh_sm netio_tintf)
SET(RTI_MODULE_EXPORT discdpde)
SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
