###############################################################################
# (c) Copyright, Real-Time Innovations 2023-2023
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

INCLUDE(CMakeForceCompiler)
SET(_RTIME_OSAPI_PLATFORM autosar)

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

IF (DEFINED ENV{OSEK_PATH})
    STRING(REPLACE "\\" "/" USAR_PATH $ENV{OSEK_PATH})
ELSE()
    MESSAGE("Please set the OSEK_PATH environment variable pointing to your SIP. Example:")
    MESSAGE("set OSEK_PATH=C:\\Vector\\CBD1500710_D12")
    MESSAGE(FATAL_ERROR "")
ENDIF()

IF (RTIME_TARGET_NAME MATCHES "i86.*") # Microsar for vVIRTUALtarget
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/VttOs")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/TcpIp")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/IpBase")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/VStdLib")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/SoAd")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/EthIf")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/VttEth")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/Vtt_Common")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/VttCntrl")
    INCLUDE_DIRECTORIES("${USAR_PATH}/BSW/VttEthTrcv_30_Vtt")
    
    SET(RTI_ENDIAN_LITTLE 1)

ELSEIF (RTIME_TARGET_NAME MATCHES "tc29xt.*") # Microsar for TC29x
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/Os/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/_Common/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/TcpIp/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/IpBase/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/EthIf/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/VStdLib/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/Eth_30_Tricore/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/EthTrcv_30_Ethmii/Implementation")

    SET(RTI_ENDIAN_LITTLE 1)

ELSEIF (RTIME_TARGET_NAME MATCHES "tc39xt.*") # Microsar for TC39x
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/_Common/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/Os/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/TcpIp/Implementation")
    INCLUDE_DIRECTORIES("${USAR_PATH}/Components/IpBase/Implementation")

    SET(RTI_ENDIAN_LITTLE 1)

ELSE()
    MESSAGE(FATAL_ERROR "No architecture recognized, please select one from: x86/tc29xt")

ENDIF()

ADD_DEFINITIONS(-DRTIME_AUTOSAR_MICROSAR)