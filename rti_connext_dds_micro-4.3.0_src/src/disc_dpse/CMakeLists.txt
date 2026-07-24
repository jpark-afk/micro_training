###############################################################################
#
# Copyright (c) 2018-2024 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME disc_dpse)
SET(RTI_SUBMODULES dpse_plugin)

IF (RTIME_EXCLUDE_DPSE AND NOT RTIME_SHIP_SOURCE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPSE=0)
    return()
ENDIF()

IF (NOT RTIME_SHIP_SOURCE)
    ADD_DEFINITIONS(-DRTIME_HAVE_DPSE=1)
ENDIF()

SET(LAST_MODULE disc_dpse)

IF (RTIME_DEVTREE)

    # Only needed for testing purposes
    SET(RTI_BUILD_INCLUDE
            dds_c.2.0/srcC/infrastructure
            dds_c.2.0/srcC/topic
            dds_c.2.0/srcC/type
            dds_c.2.0/srcC/publication
            dds_c.2.0/srcC/subscription
            dds_c.2.0/srcC/domain
            dds_c.2.0/srcC/ipc_liveliness
    )

    SET(RTI_SOURCE_INCLUDE
            dds_c.2.0/srcC/infrastructure
            dds_c.2.0/srcC/topic
            dds_c.2.0/srcC/type
            dds_c.2.0/srcC/publication
            dds_c.2.0/srcC/subscription
            dds_c.2.0/srcC/domain
            dds_c.2.0/srcC/discovery
            dds_c.2.0/srcC/ipc_liveliness
    )
ENDIF()

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_MODULE_IMPORTS dds_c)
    SET(RTI_MODULE_TEST_IMPORT rh_sm wh_sm netio_tintf)
    SET(RTI_MODULE_EXPORT discdpse)
ELSE()
    SET(RTI_MODULE_TEST_IMPORT netio_tintf)
    # CPP is in its own library, stop at DPDE. RTIME_EXCLUDE_CPP AND
    SET(RTI_OBJECT_DEPENDS rh_sm wh_sm)
    IF (NOT RTIME_PIL)
        LIST(APPEND RTI_OBJECT_DEPENDS
            rti_me_psl::netiopsl
        )
    ENDIF()
    LIST(APPEND RTI_OBJECT_DEPENDS dds_c)
    IF (NOT RTIME_DDS_EXCLUDE_XTYPES)
        LIST(APPEND RTI_OBJECT_DEPENDS
             xcdr
        )
    ENDIF()
    LIST(APPEND RTI_OBJECT_DEPENDS
         rtps netio_dgram netio rt db cdr reda osapi
    )
    IF (NOT RTIME_PIL)
        LIST(APPEND RTI_OBJECT_DEPENDS
            rti_me_psl::ospsl
        )
    ELSE()
        SET(RTI_MODULE_IMPORTS rti_me_psl::ospsl)
    ENDIF()
    SET(RTI_MODULE_EXPORT librti_me)
    SET(dds_c_EXPORT_NAME librti_me)
    SET(dds_c_EXPORT_NAME librti_me PARENT_SCOPE)

    SET(RTI_SOURCE_INCLUDE dds_c.2.0/srcC/infrastructure
                       dds_c.2.0/srcC/domain
                       dds_c.2.0/srcC/publication
                       dds_c.2.0/srcC/subscription
                       dds_c.2.0/srcC/topic
                       dds_c.2.0/srcC/builtin_ipc
                       dds_c.2.0/srcC/discovery
                       dds_c.2.0/srcC/type)
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
