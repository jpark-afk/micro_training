###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2018
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_SUBMODULE_SRC LOOPInterface.c )
SET(RTI_SUBMODULE_HDR LOOPInterface.h)
SET(DLL_WIN32APPLICATIONINCLUDES -DRTI_cdr_DLL_VARIABLE -DRTI_reda_DLL_VARIABLE -DRTI_osapi_DLL_VARIABLE -DRTI_db_DLL_VARIABLE -DRTI_rt_DLL_VARIABLE -DRTI_DLL_VARIABLE -DRTIME_DLL_EXPORT)
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
