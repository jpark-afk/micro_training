###############################################################################
# (c) Copyright, Real-Time Innovations 2022-2022
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
SET(RTI_MODULE_NAME netio_zcopy)
SET(RTI_SUBMODULES netio_zcopy notif notif_interface sharedq whsq)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (RTIME_PIL)
        if (RTIME_INCLUDE_API_ONLY)
            SET(RTI_MODULE_IMPORTS rti_me_psl::netiopsl dds_c)
        else()
            SET(RTI_MODULE_IMPORTS wh_sm rti_me_psl::netiopsl dds_c)
        ENDIF()
    ELSE()
        IF (RTIME_INCLUDE_API_ONLY)
            SET(RTI_MODULE_IMPORTS dds_c)
        ELSE()
            SET(RTI_MODULE_IMPORTS wh_sm rh_sm dds_c)
        ENDIF()
    ENDIF()
ELSE()
    SET(RTI_MODULE_DEPENDS dds_c wh_sm rh_sm rtps netio dds_c rt db cdr reda osapi)
    LIST(APPEND RTI_MODULE_DEPENDS rti_me_psl::ospsl)
ENDIF()

SET(RTI_MODULE_EXPORT netiozcopy)
SET(RTI_SHIP true)

IF (RTIME_INCLUDE_ZCOPY_V2 OR RTIME_SHIP_SOURCE OR RTI_CMAKE_SOURCE OR RTI_MAKEDOC)
    INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
ENDIF()



