###############################################################################
# Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC 
    XcdrTypeInterface.c
    XTypesV2TypePlugin.c 
    XTypesHeapMgr.c
    Interpreter.c 
    InterpreterSupport.c
)

SET(RTI_SUBMODULE_HDR 
    Interpreter.h
)
                                 
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
