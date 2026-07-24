###############################################################################
# (c) Copyright, Real-Time Innovations 2020 - 2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC 
    RTPSChecksum.c
    RTPSCrc32.c
    RTPSCrc32Pro.c
    RTPSCrc64.c
    RTPSChecksumMd5.c
)

SET(RTI_SUBMODULE_HDR 
    RTPSChecksum.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
