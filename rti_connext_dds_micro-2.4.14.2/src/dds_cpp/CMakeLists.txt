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

SET(RTI_MODULE_NAME dds_cpp)

SET(RTI_SUBMODULES infrastructure topic subscription publication
                   domain namespace osapi rt wh_sm rh_sm)

IF (NOT RTIME_UDP_EXCLUDE_BUILTIN)
    LIST(APPEND RTI_SUBMODULES netio)
ENDIF()

IF (NOT RTIME_EXCLUDE_DPDE)
    SET(RTI_SUBMODULES  disc_dpde ${RTI_SUBMODULES})
ENDIF()

IF (NOT RTIME_EXCLUDE_DPSE)
    SET(RTI_SUBMODULES  disc_dpse ${RTI_SUBMODULES})
ENDIF()

SET(RTI_SOURCE_INCLUDE dds_c.2.0/srcC/infrastructure
                       dds_c.2.0/srcC/domain
                       dds_c.2.0/srcC/publication
                       dds_c.2.0/srcC/subscription
                       dds_c.2.0/srcC/topic
                       dds_c.2.0/srcC/type)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_MODULE_IMPORTS rh_sm wh_sm dds_c)
    IF (NOT RTIME_EXCLUDE_DPSE)
        SET(RTI_MODULE_IMPORTS disc_dpse ${RTI_MODULE_IMPORTS})
    ENDIF()
    IF (NOT RTIME_EXCLUDE_DPDE)
        SET(RTI_MODULE_IMPORTS disc_dpde ${RTI_MODULE_IMPORTS})
    ENDIF()

    SET(RTI_MODULE_EXPORT cpp)
ELSEIF(RTIME_BUILD_SINGLE_LIB AND NOT RTIME_EXCLUDE_CPP)

# The commented lines includs CPP in the single library
#    SET(RTI_OBJECT_DEPENDS rtps netio rt db cdr reda osapi dds_c
#                           rh_sm wh_sm)
#    IF (NOT RTIME_EXCLUDE_DPDE)
#        SET(RTI_OBJECT_DEPENDS ${RTI_OBJECT_DEPENDS} disc_dpde)
#    ENDIF()
#    IF (NOT RTIME_EXCLUDE_DPSE)
#        SET(RTI_OBJECT_DEPENDS ${RTI_OBJECT_DEPENDS} disc_dpse)
#    ENDIF()
#    SET(RTI_MODULE_EXPORT librti_me)

    SET(RTI_MODULE_IMPORTS  ${LAST_MODULE})
    SET(RTI_MODULE_EXPORT cpp)
ELSE()
    RTI_MESSAGE_FATAL("dds_cpp included, but not last module")
ENDIF()

SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
