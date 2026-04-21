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
 
SET(RTI_SUBMODULE_SRC_CC 
        Entity.cxx 
        Conditions.cxx 
        Waitset.cxx 
        StringSeq.cxx 
        SampleInfoSeq.cxx 
        TransportSeq.cxx 
        QosPolicy.cxx 
        Duration.cxx 
        Time.cxx 
        SequenceNumber.cxx 
        Locator.cxx
)

IF (NOT RTIME_CERT)
    LIST(APPEND RTI_SUBMODULE_SRC_CC DDSCPPVersion.cxx)
ENDIF()
            
SET(RTI_SUBMODULE_HDR 
        Entity.hxx 
        Conditions.hxx 
        SupportMethodsGen.hxx)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)

