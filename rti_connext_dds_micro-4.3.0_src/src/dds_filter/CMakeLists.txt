################################################################################
#
# Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
IF (RTIME_DDS_EXCLUDE_FILTERING AND NOT RTIME_SHIP_SOURCE)
    RETURN()
ENDIF()

SET(RTI_MODULE_NAME dds_filter)

IF (RTIME_DDS_EXCLUDE_SQL_FILTER AND NOT RTIME_SHIP_SOURCE)
    SET(RTI_SUBMODULES filter plugin)
ELSE()
    SET(RTI_SUBMODULES filter plugin sql)
ENDIF()

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (RTIME_DDS_EXCLUDE_SQL_FILTER AND NOT RTIME_SHIP_SOURCE)
        IF (RTIME_PIL)
            SET(RTI_MODULE_IMPORTS
                rti_me_psl::netiopsl
                dds_c
            )
        ELSE()
            SET(RTI_MODULE_IMPORTS dds_c)
        ENDIF()
    ELSE()
        IF (RTIME_PIL)
            SET(RTI_MODULE_IMPORTS
                rti_me_psl::netiopsl
                xcdr
                dds_c
            )
        ELSE()
            SET(RTI_MODULE_IMPORTS xcdr dds_c)
        ENDIF()
    ENDIF()
ELSE()
    SET(RTI_MODULE_DEPENDS
        dds_c rtps netio rt db cdr reda osapi)
    LIST(APPEND RTI_MODULE_DEPENDS
        rti_me_psl::ospsl)
    IF (NOT RTIME_DDS_EXCLUDE_SQL_FILTER
            OR RTIME_SHIP_SOURCE)
        LIST(APPEND RTI_MODULE_DEPENDS xcdr)
    ENDIF()
ENDIF()

SET(RTI_MODULE_EXPORT ddsfilter)
SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
