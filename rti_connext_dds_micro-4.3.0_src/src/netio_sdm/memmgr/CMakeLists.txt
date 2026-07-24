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

SET(RTI_SUBMODULE_SRC 
    NETIO_SDMDataWriterMemMgr.c
    NETIO_SDMDataReaderMemMgr.c
    NETIO_SDMMemPool.c
    NETIO_SDMBitmap.c
    NETIO_SDMShmTypePlugin.c
    NETIO_SDMVersion.c
)

SET(RTI_SUBMODULE_HDR 
    NETIO_SDMDataWriterMemMgr.h 
    NETIO_SDMDataReaderMemMgr.h
    NETIO_SDMBitmap.h
    NETIO_SDMMemPool.h
    )

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
