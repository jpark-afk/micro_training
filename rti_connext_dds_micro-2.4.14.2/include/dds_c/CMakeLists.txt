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
    dds_c_dll.h
    dds_c_config.h
    dds_c_log.h
    dds_c_string.h
    dds_c_common.h
    dds_c_sequence.h
    dds_c_infrastructure.h
    dds_c_topic.h
    dds_c_type.h
    dds_c_discovery.h
    dds_c_discovery_plugin.h
    dds_c_publication.h
    dds_c_subscription.h
    dds_c_wh_plugin.h
    dds_c_rh_plugin.h
    dds_c_domain.h
    dds_c_data.h
    dds_c_tpolicy_gen.h
    dds_c_tdatareader_gen.h
    dds_c_tdatawriter_gen.h
    dds_c_tstatus_gen.h
    dds_c_common_impl.h 
    rti_me_c.h
)
    
IF (NOT RTIME_CERT)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR dds_c_string_manager.h)
ENDIF()
