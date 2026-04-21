###############################################################################
#
# Copyright (c) 2018-2025, Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_SHARED_HDR_DIR hpp)
SET(RTI_SHARED_MODULE_EXPORT_DIR "")

SET(RTI_MODULE_SHARED_HDR
    Interpreter.hpp
)

IF (UPDATE_PATCHED)
    LIST(APPEND RTI_MODULE_SHARED_HDR
         Stream.hpp
    )
    SET(RTI_MODULE_SHARED_HDR_PATCHED)
ELSE()
    SET(RTI_MODULE_SHARED_HDR_PATCHED
        Stream.hpp
    )
ENDIF()
