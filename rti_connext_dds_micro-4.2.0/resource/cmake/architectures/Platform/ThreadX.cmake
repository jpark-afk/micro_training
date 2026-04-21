###############################################################################
# (c) Copyright, Real-Time Innovations 2019-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# Top level CMakeLists.txt for building Micro
#
# Modification History
# --------------------
# 03jan2019 fmt Written
################################################################################
INCLUDE(CMakeForceCompiler)
SET(_RTIME_OSAPI_PLATFORM threadx)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

#Folder needed by RTI DDS Micro
IF (IS_DIRECTORY ${RTIME_SOURCE_ROOT}/osapi.2.0)
    INCLUDE_DIRECTORIES(${RTIME_SOURCE_ROOT}/osapi.2.0/srcC/threadx)
ELSE()
    INCLUDE_DIRECTORIES(${RTIME_SOURCE_ROOT}/src/osapi/threadx)
ENDIF()

SET(RTIME_NO_SHARED_LIB TRUE)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#Folders with ThreadX/NetX header files
IF (DEFINED ENV{SYNERGY_PATH})
    STRING(REPLACE "\\" "/" SYNERGY_PATH $ENV{SYNERGY_PATH})
ELSE()
    MESSAGE("Please set the SYNERGY_PATH environment variable. Example:\n")
    MESSAGE("set SYNERGY_PATH=/home/user/e2_studio/workspace/dds_c/synergy")
    MESSAGE(FATAL_ERROR "")
ENDIF()

INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/framework/el)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/framework/el/nx)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}_cfg/ssp_cfg/framework/el)

#Folders with bsp header files
INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/bsp)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}_cfg/ssp_cfg/bsp)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/bsp/cmsis/Include)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/driver/api)
INCLUDE_DIRECTORIES(${SYNERGY_PATH}/ssp/inc/driver/instances)

IF (${TARGET_HELP})
    MESSAGE("PLATFORM NOTES\n")
    MESSAGE(" The following libraries are included for ThreadX:")
    MESSAGE("\n")
ENDIF()

#Needed if port types.h is not included
ADD_DEFINITIONS(-DSYNERGY_S7G2)

#Renesas synergy S7G2 has only 640Kbytes of RAM
ADD_DEFINITIONS(-DOSAPI_PLATFORM_THREADX_HEAP_SIZE=400*1024)

