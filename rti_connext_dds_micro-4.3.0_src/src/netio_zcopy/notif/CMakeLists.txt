###############################################################################
# (c) Copyright, Real-Time Innovations 2022-2023
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_SUBMODULE_SRC
    NotificationObject.c Notifiee.c Notifier.c ShmSegment.c
)

SET(RTI_SUBMODULE_HDR
    NotificationObject.h
    Notifiee.h
    Notifier.h
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
