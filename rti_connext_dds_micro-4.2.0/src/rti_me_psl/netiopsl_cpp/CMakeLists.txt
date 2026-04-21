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
SET(RTIME_SOURCE_STRIP_PATH "${RTI_MODULE_SRCROOT}/srcCxx/netiopsl_cpp")

IF (NOT DEFINED RTIME_PSL_NETIO)
    RETURN()
ENDIF()

LIST(APPEND RTIME_NETIOPSL_SUBMODULES
    "shmem/common"
)

if (RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    LIST(APPEND RTIME_NETIOPSL_SUBMODULES 
        "udp"
        "stubs"
        "shmem/posix"
        "shmem/win"
        "shmem/stubs"
        "shmem/zcopy/posix"
        "shmem/zcopy/stubs"
    )
ENDIF()


rtime_add_psl_library(LIBRAY_NAME 
                        netiopsl_cpp
                      SRCROOT 
                        "netiopsl_cpp"
                      SUBMODULES
                          ${RTIME_NETIOPSL_SUBMODULES}
                      OUTPUT_SRC
                          ${RTI_SUBMODULE_NAME}_SRC_C
                      OUTPUT_HDR
                          ${RTI_SUBMODULE_NAME}_SRC_H
                      OUTPUT_CMAKELIST  
                          RTI_SUBMODULE_CMAKELIST
                      LANG 
                        Cxx
)

SET(${RTI_MODULE_NAME}_${RTI_SUBMODULE_NAME}_SOURCE 
    ${${RTI_SUBMODULE_NAME}_SRC_C} PARENT_SCOPE
)

SET(${RTI_MODULE_NAME}_${RTI_SUBMODULE_NAME}_HEADER 
    ${${RTI_SUBMODULE_NAME}_SRC_H} PARENT_SCOPE
)
