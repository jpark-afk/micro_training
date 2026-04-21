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
 
SET(RTI_MODULE_NAME rtps)
SET(RTI_SUBMODULES rtps checksum interface )
SET(RTI_MODULE_IMPORTS  netio rt db cdr reda osapi)
SET(RTI_OBJECT_MODULE TRUE) 

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
