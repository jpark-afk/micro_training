###############################################################################
# Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
IF (RTIME_DDS_EXCLUDE_XTYPES AND NOT RTIME_SHIP_SOURCE)
    RETURN()
ENDIF()

SET(RTI_MODULE_NAME xcdr)

# This is the shared module with Connext Pro
# RTI_SHARED_HOME must point to the root of the DDS repository
SET(RTI_SHARED_MODULE xcdr.1.0)
SET(RTI_MODULE_SHARED_EXTRA_INCLUDES "rti/flat" "rti/xcdr")


IF (NOT RTIME_SHIP_SOURCE)
    ADD_DEFINITIONS(
        -DRTI_XCDR_DISABLE_SEQUENCE=1
        -DRTI_XCDR_DISABLE_XTYPES_COMPLIANCE_MASK_FROM_ENV_VAR=1
    )
ENDIF()

SET(RTI_SUBMODULES
# This module is copied from Pro , but it not used by Micro.
#    typeObjectV2
    typeCode
    infrastructure
    interpreter
    stream
    flat_data
    heapmgr
    type_plugin
)

IF (RTIME_PIL)
    SET(RTI_MODULE_IMPORTS
        rti_me_psl::netiopsl
        dds_c
    )
ELSE()
    SET(RTI_MODULE_IMPORTS dds_c)
ENDIF()

SET(RTI_MODULE_EXPORT ddsxtypes)
SET(RTI_SHIP true)

IF(RTI_RUN_COV)
    SET(RTI_COMPILE_DEFINITIONS RTI_PRECONDITION_TEST=1)
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
