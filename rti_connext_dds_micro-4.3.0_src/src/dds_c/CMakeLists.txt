###############################################################################
# Copyright (c) 2018-2025 Real-Time Innovations, Inc. 
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME dds_c)

SET(RTI_SUBMODULES
        infrastructure
        type
        topic
        publication
        subscription
        domain
        discovery
        profile
)

IF (NOT RTIME_DDS_DISABLE_LIVELINESS_CHANNEL)
    LIST(APPEND RTI_SUBMODULES
         ipc_liveliness
    )
ENDIF()

SET(RTI_LIBRARY_DEPENDS)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_OBJECT_DEPENDS 
        rtps 
    )
    IF (NOT RTIME_PIL)
        LIST(APPEND RTI_OBJECT_DEPENDS
            rti_me_psl::netiopsl
        )
    ENDIF()
    LIST(APPEND RTI_OBJECT_DEPENDS
        netio_dgram 
        netio 
        rt 
        db 
        cdr 
        reda 
        osapi
    )
    IF (NOT RTIME_PIL)
        LIST(APPEND RTI_OBJECT_DEPENDS
            rti_me_psl::ospsl
        )
    ENDIF()
    IF (RTIME_PIL)
        SET(RTI_MODULE_IMPORTS 
            rti_me_psl::ospsl
        ) 
    ENDIF()

    SET(RTI_MODULE_EXPORT librti_me)
ELSE()
    SET(RTI_MODULE_IMPORTS 
        rtps 
    )
    IF (NOT RTIME_PIL)
        LIST(APPEND RTI_MODULE_IMPORTS
            rti_me_psl::netiopsl
        )
    ENDIF()
    LIST(APPEND RTI_MODULE_IMPORTS
        netio_dgram 
        netio 
        rt 
        db 
        cdr 
        reda 
        osapi
    )
    IF (NOT RTIME_PIL)
        SET(RTI_MODULE_IMPORTS rti_me_psl::ospsl) 
    ENDIF()
    SET(RTI_OBJECT_MODULE TRUE)
    SET(RTI_ADD_SUBMOUDLE_INCLUDE TRUE)
ENDIF()

SET(RTI_BUILDID_MASTER true)
SET(RTI_MODULE_TEST_IMPORT netio_tintf ddsxtypes)
SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
