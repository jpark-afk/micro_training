###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2019
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_NAME dds_c)

SET(RTI_SUBMODULES 
        infrastructure 
        type
        topic 
        publication 
        subscription 
        domain 
)

SET(RTI_LIBRARY_DEPENDS)

IF (NOT RTIME_BUILD_SINGLE_LIB)
    SET(RTI_OBJECT_DEPENDS rtps netio rt db cdr reda osapi)
    if (RTIME_INCLUDE_ARINC)
        #LIST(APPEND RTI_OBJECT_DEPENDS netio_arinc)
    ENDIF()

    SET(RTI_MODULE_EXPORT librti_me)
ELSE()
    SET(RTI_MODULE_IMPORTS rtps netio rt db cdr reda osapi)
    if (RTIME_INCLUDE_ARINC)
        #LIST(APPEND RTI_MODULE_IMPORTS netio_arinc)
    ENDIF()

    SET(RTI_OBJECT_MODULE TRUE)
ENDIF()

SET(RTI_BUILDID_MASTER true)
SET(RTI_MODULE_TEST_IMPORT)
SET(RTI_SHIP true)

INCLUDE (${RESOURCEHOME}/cmake/cmake_module.txt)
