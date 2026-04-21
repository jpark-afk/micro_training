###############################################################################
# (c) Copyright, Real-Time Innovations 2021-2021
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
    STRING(REPLACE "\\" "/" TRESOS_BASE $ENV{OSEK_PATH})
ELSE()
    MESSAGE("Please set the OSEK_PATH environment variable pointing to your installation directory. Example:")
    MESSAGE("set OSEK_PATH=C:\\EB\\tresos")
    MESSAGE(FATAL_ERROR "")
ENDIF()

# Set the specific compiler for wincore
#  CAUTION! Remember to add the Make and GCC path to your PATH envvar
# set(CMAKE_MAKE_PROGRAM "${TRESOS_BASE}/plugins/Make_TS_TxDxM4I0R0/tools/GNU_Make/make.exe")
# set(CMAKE_C_COMPILER   "${TRESOS_BASE}/plugins/Platforms_TS_T19D1M3I0R0/tools/mingw-gcc-6.2.0/bin/gcc.exe" CACHE STRING "C" FORCE)
# set(CMAKE_CXX_COMPILER "${TRESOS_BASE}/plugins/Platforms_TS_T19D1M3I0R0/tools/mingw-gcc-6.2.0/bin/gcc.exe" CACHE STRING "C++" FORCE)
# set(CMAKE_AR           "${TRESOS_BASE}/plugins/Platforms_TS_T19D1M3I0R0/tools/mingw-gcc-6.2.0/bin/ar.exe"  CACHE STRING "" FORCE) 
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Add system includes
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Eth_TS_T19D1M0I0R0/include")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Os_TS_T19D1M6I0R0/include")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Compiler_TS_TxDxM1I0R0/include")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Compiler_TS_TxDxM1I0R0/include/WINDOWS")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/TcpIp_TS_TxDxM3I5R0/include")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Base_TS_TxDxM5I0R0/include")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Platforms_TS_T19D1M3I0R0/include/WINDOWS")
INCLUDE_DIRECTORIES("${TRESOS_BASE}/plugins/Atomics_TS_TxDxM1I0R0/include")

ADD_DEFINITIONS(-DOS_RELEASE_SUFFIX=OS_AS403)
ADD_DEFINITIONS(-DOS_SCHEDULING_ALGORITHM=CLZ_QUEUE)
ADD_DEFINITIONS(-DOS_KERNEL_TYPE=OS_SYSTEM_CALL)
ADD_DEFINITIONS(-DATOMICS_USE_GENERIC_IMPL=1)

IF (RTIME_TARGET_NAME MATCHES "^i86")
    ADD_DEFINITIONS(-DOS_TOOL=OS_gnu)
    ADD_DEFINITIONS(-DOS_ARCH=OS_WINDOWS)
    ADD_DEFINITIONS(-DOS_RELEASE_SUFFIX=OS_AS403)
    ADD_DEFINITIONS(-DOS_CPU=OS_WIN32X86)
    ADD_DEFINITIONS(-DTS_ARCH_FAMILY=TS_WINDOWS)
    ADD_DEFINITIONS(-DTS_ARCH_DERIVATE=TS_WIN32X86)
    SET(RTI_ENDIAN_LITTLE 1)
    ADD_DEFINITIONS(-DRTIME_AUTOSAR_ENABLE_SPINLOCK=0)    # Disable Spinlocks as OS_DSYNC is not defined in x86
ELSE()
    MESSAGE(FATAL_ERROR "No architecture recognized, please select one from: i86.")
ENDIF()

IF (${RTI_BUILD_UNITTESTS})
    # As of 2.4.14.2, the UTP is not prepared for multicast 
    #  (lack of license to modify the project)
    SET(RTIME_UDP_ENABLE_MULTICAST FALSE)
ENDIF()

ADD_DEFINITIONS(-DRTIME_AUTOSAR_WINCORE)