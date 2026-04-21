###############################################################################
# (c) Copyright, Real-Time Innovations 2022-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
INCLUDE(CMakeForceCompiler)

SET(TARGET_SUPPORTS_SHARED_LIBS TRUE)
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
SET(CMAKE_SHARED_LIBRARY_SUFFIX_C ".dylib")
SET(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".dylib")
SET(CMAKE_C_OUTPUT_EXTENSION ".c.o")
SET(CMAKE_CXX_OUTPUT_EXTENSION ".c.o")
SET(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
SET(CMAKE_INSTALL_RPATH "")
SET(RTIME_UDP_EXCLUDE_BUILTIN TRUE)
SET(_RTIME_OSAPI_PLATFORM generic)

SET(CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -C -E <SOURCE> > <PREPROCESSED_SOURCE>")
SET(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -C -E <SOURCE> > <PREPROCESSED_SOURCE>")
SET(CMAKE_MAKE_PROGRAM make)
