################################################################################
# Copyright (c) 2018-2024 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
IF (RTIME_EXCLUDE_CPP)
    return()
ENDIF()

SET(RTI_MODULE_NAME dds_cpp)

SET(RTI_SUBMODULES 
    infrastructure 
    topic 
    subscription 
    publication
    domain 
    namespace 
    osapi 
    rt 
    netio_shmem 
)

LIST(APPEND RTI_SUBMODULES
    disc_dpse 
    disc_dpde
    wh_sm 
    rh_sm
)

IF (NOT RTIME_PIL AND NOT RTIME_PSL)
    SET(RTI_OBJECT_DEPENDS 
        rti_me_psl::netiopsl_cpp
    )
ELSE()
    SET(RTI_MODULE_IMPORTS 
        rti_me_psl::netiopsl_cpp
    )
ENDIF()

SET(RTI_MODULE_IMPORTS)

IF (RTIME_INCLUDE_ZCOPY_V2)
    LIST(APPEND RTI_MODULE_IMPORTS netio_zcopy)
ENDIF()

IF (NOT RTIME_DDS_EXCLUDE_XTYPES)
    LIST(APPEND RTI_MODULE_IMPORTS
        netio_sdm
    )
ENDIF()

LIST(APPEND RTI_MODULE_IMPORTS
    netio_shmem 
    disc_dpse 
    disc_dpde 
    rh_sm 
    wh_sm
)

IF (NOT RTIME_DDS_EXCLUDE_XTYPES)
    LIST(APPEND RTI_MODULE_IMPORTS xcdr)
ENDIF()

LIST(APPEND RTI_MODULE_IMPORTS dds_c)

SET(RTI_SOURCE_INCLUDE dds_c.2.0/srcC/infrastructure)

SET(RTI_MODULE_EXPORT cpp)

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
