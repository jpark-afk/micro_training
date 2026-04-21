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
SET(_RTIME_OSAPI_PLATFORM freertos)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

SET(RTIME_NO_SHARED_LIB TRUE)
SET(RTI_BUILD_UNITTESTS_AS_LIBS TRUE)
SET(RTIME_EXCLUDE_DT TRUE)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#Folder with your FreeRTOSConfig.h and lwipopts.h
IF (DEFINED ENV{CONFIG_PATH})
    STRING(REPLACE "\\" "/" CONFIG_PATH $ENV{CONFIG_PATH})
ELSE()
    MESSAGE("Please set the CONFIG_PATH environment variable with the path were ")
    MESSAGE("your FreeRTOSConfig.h and lwipopts.h files are located.")
    MESSAGE(FATAL_ERROR "")
ENDIF()

INCLUDE_DIRECTORIES(${CONFIG_PATH})

#FreeRTOS path
IF (DEFINED ENV{FREERTOS_PATH})
    STRING(REPLACE "\\" "/" FREERTOS_PATH $ENV{FREERTOS_PATH})
ELSE()
    MESSAGE("Please set the FREERTOS_PATH environment variable. Example:\n")
    MESSAGE("set FREERTOS_PATH=/home/FreeRTOSv10.2.0/FreeRTOS")
    MESSAGE(FATAL_ERROR "")
ENDIF()

#FreeRTOS include folder
INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/include)

#lwIP include folders
IF (NOT RTIME_UDP_EXCLUDE_BUILTIN)
    IF (DEFINED ENV{LWIP_PATH})
        STRING(REPLACE "\\" "/" LWIP_PATH $ENV{LWIP_PATH})
    ELSE()
        MESSAGE("Please set the LWIP_PATH environment variable. Example:\n")
        MESSAGE("set LWIP_PATH=/home/LwIP")
        MESSAGE(FATAL_ERROR "")
    ENDIF()

    INCLUDE_DIRECTORIES(${LWIP_PATH}/src/include)
    INCLUDE_DIRECTORIES(${LWIP_PATH}/system)

    #Some systems define its own 'struct timeval' which might collide with the
    #one provide by lwIP. This should prevent system 'struct timeval' definition.
    ADD_DEFINITIONS(-D_TIMEVAL_DEFINED=0)
ENDIF()

#FreeRTOS port include folder
IF (${RTIME_TARGET_NAME} MATCHES "cortexm7")
    INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/portable/GCC/ARM_CM7/r0p1)
ELSEIF (${RTIME_TARGET_NAME} MATCHES "cortexm4f")
    INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/portable/GCC/ARM_CM4F)
ENDIF()
