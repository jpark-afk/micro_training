###############################################################################
#
# Copyright (c) 2018-2025, Real-Time Innovations, Inc. All rights reserved.
#
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
        dds_c_profile.h
        dds_c_profile_plugin.h
        dds_c_common_impl.h
        dds_c_flowcontroller.h
        dds_c_buffer.h
        dds_c_string_manager.h
        dds_c_user_data_manager.h
        dds_c_user_data_qos.h
        dds_c_partition_qos.h
        dds_c_property_qos.h
        rti_me_c.h
        dds_c_trust.h
        dds_c_domain_trust.h
        dds_c_transport_priority_qos.h
)

IF (NOT RTIME_DDS_EXCLUDE_XTYPES OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         dds_c_typecode.h
    )
ENDIF()

IF (NOT RTIME_DDS_EXCLUDE_FILTERING OR RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
        dds_c_content_filter.h
        dds_c_filter_plugin.h
    )
ENDIF()
