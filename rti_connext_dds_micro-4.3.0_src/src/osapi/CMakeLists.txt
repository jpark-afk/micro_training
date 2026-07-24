###############################################################################
# Copyright (c) 2018-2026 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME osapi)

SET(RTI_SUBMODULES common generic)

SET(RTI_OBJECT_MODULE TRUE)

IF (NOT RTIME_PIL)
    SET(RTI_OBJECT_DEPENDS rti_me_psl::ospsl)
ELSE()
    SET(RTI_MODULE_IMPORTS rti_me_psl::ospsl)
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)


