###############################################################################
# 
# Copyright (c) 2018-2024 Real-Time Innovations, Inc.
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_MODULE_NAME netio)
SET(RTI_OBJECT_MODULE TRUE) 
SET(RTI_SUBMODULES 
    common  
    loopback 
)
    
SET(RTI_MODULE_IMPORTS  rt db cdr reda osapi)

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
