###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2020
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
# 1oct2020,fmt MICRO-2586/PR.28158 Align CPU_BIT_ORDER with MICRO
################################################################################

INCLUDE(CMakeForceCompiler)
SET(_RTIME_OSAPI_PLATFORM autosar)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

SET(RTIME_NO_SHARED_LIB TRUE)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

IF (DEFINED ENV{OSEK_PATH})
    STRING(REPLACE "\\" "/" EBTRESOS_PATH $ENV{OSEK_PATH})
ELSE()
    MESSAGE("Please set the OSEK_PATH environment variable. Example:")
    MESSAGE("set OSEK_PATH=c:\\EB")
    MESSAGE(FATAL_ERROR "")
ENDIF()

INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Os_TS_T16D23M6I0R0/include")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Base_TS_TxDxM5I0R0/include")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Compiler_TS_TxDxM1I0R0/include/TRICORE")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Compiler_TS_TxDxM1I0R0/include/TRICORE/tasking")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/TcpIp_TS_TxDxM3I5R0/include")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/SoAd_TS_TxDxM1I8R0/include")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Platforms_TS_T16D23M3I0R0/include/TRICORE")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Atomics_TS_TxDxM1I0R0/include")
INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/Det_TS_TxDxM6I4R0/include")
#INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/EcuM_TS_TxDxM5I14R0/include")
#INCLUDE_DIRECTORIES("${EBTRESOS_PATH}/tresos/plugins/BswM_TS_TxDxM1I14R0/include")

ADD_DEFINITIONS(-DOS_RELEASE_SUFFIX=OS_AS403)
ADD_DEFINITIONS(-DOS_SCHEDULING_ALGORITHM=CLZ_QUEUE)
ADD_DEFINITIONS(-DOS_KERNEL_TYPE=OS_SYSTEM_CALL)
ADD_DEFINITIONS(-DATOMICS_USE_GENERIC_IMPL=1)

# Assume this is a single core compilation
ADD_DEFINITIONS(-DATOMICS_USER_MULTICORE_CASE=0)
    
IF (RTIME_TARGET_NAME MATCHES "Tasking")
    ADD_DEFINITIONS(-DOS_TOOL=OS_tasking)
ENDIF()

IF (RTIME_TARGET_NAME MATCHES "^tc29xt")
    ADD_DEFINITIONS(-DOS_ARCH=OS_TRICORE)
    ADD_DEFINITIONS(-DOS_TRICOREARCH=OS_TRICOREARCH_16EP)
    ADD_DEFINITIONS(-DOS_CPU=OS_TC29XT)
    ADD_DEFINITIONS(-DTS_ARCH_FAMILY=TS_TRICORE)
    ADD_DEFINITIONS(-DTS_ARCH_DERIVATE=TS_TC29XT)
    SET(RTI_ENDIAN_LITTLE 1)
ELSE()
    MESSAGE(FATAL_ERROR "No architecture recognized, please select one from: tc29xt.")
ENDIF()
SET(RTIME_INCLUDE_AUTOSAR true)

