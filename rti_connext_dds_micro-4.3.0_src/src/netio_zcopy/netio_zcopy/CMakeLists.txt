###############################################################################
# (c) Copyright, Real-Time Innovations 2022-2022
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
 
SET(RTI_SUBMODULE_SRC 
    NETIO_ZeroCopy.c
    NETIO_ZeroCopyPlugin.c
    NETIO_ZeroCopyGuid.c
    NETIO_ZeroCopyLoader.c
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
