###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SHARED_SUBMODULE interpreter)

SET(RTI_SHARED_SRC
    ProgramSet.c
    InstructionIndex.c
    ProgramSupport.c
    SampleProgramExecution.c
    SampleProgramGeneration.c
    TypePluginProgramExecution.c
    TypePluginProgramGeneration.c
)

SET(RTI_SHARED_HDR
    InstructionIndex.h
    ProgramSupport.h
)

IF (UPDATE_PATCHED)
    LIST(APPEND RTI_SHARED_SRC
    )
    LIST(APPEND RTI_SHARED_HDR
    )
    SET(RTI_SHARED_SRC_PATCHED)
    SET(RTI_SHARED_HDR_PATCHED)
ELSE()
    SET(RTI_SHARED_SRC_PATCHED
    )

    SET(RTI_SHARED_HDR_PATCHED
    )
ENDIF()
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
