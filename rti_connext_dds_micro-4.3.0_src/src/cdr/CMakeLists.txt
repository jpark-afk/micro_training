#
# (c) Copyright, Real-Time Innovations, 2016-2016. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description
# -----------
#
# Modification history
# --------------------
# 04may2016 tk Written
##########################################################################

SET(RTI_MODULE_NAME cdr)

SET(RTI_SUBMODULES stream property)

SET(RTI_MODULE_IMPORTS reda osapi)
SET(RTI_OBJECT_MODULE TRUE) 

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
