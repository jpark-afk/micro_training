
###############################################################################
# (c) Copyright, Real-Time Innovations 2024-2025
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

INCLUDE(CMakeForceCompiler)
SET(_RTIME_OSAPI_PLATFORM freertos)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

SET(RTIME_NO_SHARED_LIB TRUE)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
SET(RTI_BUILD_UNITTESTS true)
SET(RTI_BUILD_UNITTESTS_AS_LIBS true)
SET(RTIME_EXCLUDE_DT true)
SET(RTIME_EXCLUDE_QTS true)
SET(RTIME_EXCLUDE_CPP true)
SET(RTIME_EXCLUDE_SHMEM true)