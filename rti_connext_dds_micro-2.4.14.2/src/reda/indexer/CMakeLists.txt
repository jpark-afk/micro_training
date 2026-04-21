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
 
SET(RTI_SUBMODULE_SRC REDAIndexer.c )
SET(RTI_SUBMODULE_HDR REDAIndexer.h )
SET(DLL_WIN32APPLICATIONINCLUDES -DRTI_osapi_DLL_EXPORT -DRTIME_DLL_EXPORT)
INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
