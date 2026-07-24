###############################################################################
# Copyright (c) 2018-2024 Real-Time Innovations , Inc.
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
SET(RTI_MODULE_NAME app_gen)

SET(RTI_SUBMODULES app_gen_plugin)

if (NOT RTIME_CERT)
    set(import_modules disc_dpde)
endif()

list(APPEND import_modules disc_dpse rh_sm wh_sm)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    IF (RTIME_PIL)
        SET(RTI_MODULE_IMPORTS ${import_modules} rti_me_psl::netiopsl dds_c)
    ELSE()
        SET(RTI_MODULE_IMPORTS ${import_modules} dds_c)
    ENDIF()
ELSE()
    SET(RTI_MODULE_DEPENDS
            disc_dpse
            wh_sm
            rh_sm
            rtps
            netio
            dds_c
            rt
            db
            cdr
            reda
            osapi
            rti_me_psl::ospsl
    )
ENDIF()

SET(RTI_MODULE_EXPORT appgen)

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
