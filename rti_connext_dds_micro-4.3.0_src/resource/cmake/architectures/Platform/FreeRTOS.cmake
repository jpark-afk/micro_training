###############################################################################
# (c) Copyright, Real-Time Innovations 2019-2026
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

SET(RTIME_OSAPI_PLATFORM freertos)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

ADD_DEFINITIONS(-D__freertos__)

SET(RTIME_NO_SHARED_LIB TRUE)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#Folder with your FreeRTOSConfig.h and lwipopts.h
IF (DEFINED ENV{CONFIG_PATH})
    STRING(REPLACE "\\" "/" CONFIG_PATH $ENV{CONFIG_PATH})
ELSE()
    # Library-only build: fallback to osconfig stubs
    GET_FILENAME_COMPONENT(CONFIG_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../osconfig/freertos/${RTIME_TARGET_NAME}" ABSOLUTE)
    IF (NOT EXISTS "${CONFIG_PATH}/FreeRTOSConfig.h")
        MESSAGE("CONFIG_PATH is not set and no osconfig stubs found for ${RTIME_TARGET_NAME}.")
        MESSAGE("Please set the CONFIG_PATH environment variable with the path where ")
        MESSAGE("your FreeRTOSConfig.h and lwipopts.h files are located.")
        MESSAGE(FATAL_ERROR "")
    ENDIF()
    MESSAGE(STATUS "CONFIG_PATH not set, using osconfig fallback: ${CONFIG_PATH}")
ENDIF()

INCLUDE_DIRECTORIES(${CONFIG_PATH})
SET(RTI_BUILD_UNITTESTS_AS_LIBS true)
SET(RTIME_EXCLUDE_DT true)
SET(RTIME_EXCLUDE_QTS true)
SET(RTIME_EXCLUDE_CPP true)
SET(RTIME_EXCLUDE_SHMEM true)


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


IF (${RTIME_TARGET_NAME} MATCHES "10.3.1")
    IF (DEFINED ENV{RTD_PATH})
        ADD_DEFINITIONS(-DUSING_RTD=1)
        STRING(REPLACE "\\" "/" RTD_PATH $ENV{RTD_PATH})
        INCLUDE_DIRECTORIES(${RTD_PATH}/Base_TS_T40D11M40I0R0/header)
        INCLUDE_DIRECTORIES(${RTD_PATH}/Base_TS_T40D11M40I0R0/include)
        INCLUDE_DIRECTORIES(${RTD_PATH}/Platform_TS_T40D11M40I0R0/startup/include)
    ELSE()
        MESSAGE(STATUS "RTD_PATH not set, building without RTD support")
    ENDIF()

    IF (DEFINED ENV{LWIP_PORTS_PATH})
        STRING(REPLACE "\\" "/" LWIP_PORTS_PATH $ENV{LWIP_PORTS_PATH})
        INCLUDE_DIRECTORIES(${LWIP_PORTS_PATH})
    ELSE()
        # osconfig stubs contain arch/cc.h
        INCLUDE_DIRECTORIES(${CONFIG_PATH})
    ENDIF()

ENDIF()

#FreeRTOS port include folder
IF (${RTIME_TARGET_NAME} MATCHES "cortexm7")
    INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/portable/GCC/ARM_CM7/r0p1)
ELSEIF (${RTIME_TARGET_NAME} MATCHES "armv7em")
    INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/portable/GCC/ARM_CM7/r0p1)
ELSEIF (${RTIME_TARGET_NAME} MATCHES "cortexm4f")
    INCLUDE_DIRECTORIES(${FREERTOS_PATH}/Source/portable/GCC/ARM_CM4F)
ENDIF()

SET(RTIME_PLATFORM_PSL
    netiopsl::udp
    ospsl::freertos
    ospsl::libc/string
)
