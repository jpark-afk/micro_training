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
 
SET(RTI_MODULE_PUBLIC_HDR 
    dds_cpp_dll.hxx
    dds_cpp_infrastructure.hxx
    dds_cpp_data.hxx
    dds_cpp_domain.hxx
    dds_cpp_topic.hxx
    dds_cpp_publication.hxx
    dds_cpp_subscription.hxx
    dds_cpp_tdatareader_gen.hxx
    dds_cpp_tdatawriter_gen.hxx
    dds_cpp_ttypesupport_gen.hxx
    dds_cpp_sequence_defn.hxx
    dds_cpp_namespace.hxx
    dds_cpp_osapi.hxx
    dds_cpp_rt.hxx
    dds_cpp_rh_sm.hxx
    dds_cpp_wh_sm.hxx
    rti_me_cpp.hxx
)

IF (NOT RTIME_EXCLUDE_DPDE)
    SET(RTI_MODULE_PUBLIC_HDR  dds_cpp_dpde.hxx ${RTI_MODULE_PUBLIC_HDR}) 
ENDIF()

IF (NOT RTIME_EXCLUDE_DPSE)
    SET(RTI_MODULE_PUBLIC_HDR  dds_cpp_dpse.hxx ${RTI_MODULE_PUBLIC_HDR}) 
ENDIF()

IF (NOT RTIME_UDP_EXCLUDE_BUILTIN)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR dds_cpp_netio.hxx)
ENDIF()
