################################################################################
#
# Copyright (c) 2024-2025 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
################################################################################
SET(RTIME_SOURCE_STRIP_PATH "${RTI_MODULE_SRCROOT}/srcC/ospsl")

if (RTI_CMAKE_SOURCE OR TEST_BUNDLE)
    set(RTIME_OSPSL_SUBMODULES 
        "libc/malloc"
        "libc/string" 
        "libc/stubs" 
        "libc/stdio"
        freertos
        posix
        windows
        stubs
        common
        autosar
    )
ENDIF()

rtime_add_psl_library(LIBRARY_NAME 
                        ospsl
                      SRCROOT 
                        "ospsl"
                      SUBMODULES
                          ${RTIME_OSPSL_SUBMODULES}
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
    SET(RTI_ospsl_EXPORTS osapi PARENT_SCOPE)
ENDIF()
