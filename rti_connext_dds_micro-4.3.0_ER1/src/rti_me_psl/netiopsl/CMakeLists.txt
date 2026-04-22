################################################################################
# Copyright (c) 2024-2025 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
################################################################################

SET(RTIME_SOURCE_STRIP_PATH "${RTI_MODULE_SRCROOT}/srcC/netiopsl")

IF (NOT DEFINED RTIME_PSL_NETIO)
    RETURN()
ENDIF()

LIST(APPEND RTIME_NETIOPSL_SUBMODULES
    "shmem/common"
)
    
IF (RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    LIST(APPEND RTIME_NETIOPSL_SUBMODULES 
        "udp"
        "stubs"
        "shmem/posix"
        "shmem/win"
        "shmem/stubs"
        "shmem/zcopy/stubs"
        "shmem/zcopy/posix"
    )
ENDIF()

rtime_add_psl_library(LIBRARY_NAME 
                        netiopsl
                      SRCROOT 
                        "netiopsl"
                      SUBMODULES
                          ${RTIME_NETIOPSL_SUBMODULES}
                      OUTPUT_SRC
                          ${RTI_SUBMODULE_NAME}_SRC_C
                      OUTPUT_HDR
                          ${RTI_SUBMODULE_NAME}_SRC_H
                      OUTPUT_CMAKELIST  
                          RTI_SUBMODULE_CMAKELIST
                      LANG 
                        C
)

SET(${RTI_MODULE_NAME}_${RTI_SUBMODULE_NAME}_SOURCE 
    ${${RTI_SUBMODULE_NAME}_SRC_C} PARENT_SCOPE
)

SET(${RTI_MODULE_NAME}_${RTI_SUBMODULE_NAME}_HEADER 
    ${${RTI_SUBMODULE_NAME}_SRC_H} PARENT_SCOPE
)

IF (RTI_OBJECT_MODULE)
    SET(RTI_netiopsl_EXPORTS netio_shmem PARENT_SCOPE)
    IF (RTIME_INCLUDE_ZCOPY_V2)
        LIST(APPEND RTI_netiopsl_EXPORTS netio_zcopy)
    ENDIF()
ENDIF()
