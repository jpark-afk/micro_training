###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

#

SET(RTI_SUBMODULE_SRC_CC
        Entity.cxx
        Conditions.cxx
        Waitset.cxx
        StringSeq.cxx
        SampleInfoSeq.cxx
        QosPolicy.cxx
        Property.cxx
        BuiltinData.cxx
        Duration.cxx
        Time.cxx
        SequenceNumber.cxx
        Locator.cxx
        DDSCPPVersion.cxx
        EncapsulationSeq.cxx
        Cookie.cxx
        PropertyQosPolicy.cxx
        InstanceHandleSeq.cxx
)

SET(RTI_SUBMODULE_HDR
        Entity.hxx
        Conditions.hxx
        SupportMethodsGen.hxx
)

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
