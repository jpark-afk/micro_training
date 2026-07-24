###############################################################################

# Copyright (c) 2023-2024 Real-Time Innovations, Inc.
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

 
SET(RTI_SUBMODULE_SRC 
    NETIOZCOPYConsistentSet.c
    NETIOZCOPYSharedMemPool.c
    NETIOZCOPYSharedQ.c
    NETIOZCOPYSharedQAdmin.c
)

SET(RTI_SUBMODULE_HDR 
    NETIOZCOPYConsistentSet.h
    NETIOZCOPYSharedMemPool.h
    NETIOZCOPYSharedQAdmin.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
