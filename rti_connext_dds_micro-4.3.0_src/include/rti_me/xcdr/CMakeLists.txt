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

SET(RTI_MODULE_SHARED_HDR
    xcdr_typeCode_impl.h
    xcdr_flat_data.h
    xcdr_stream.h
    xcdr_stream_impl.h
    xcdr_interpreter.h
    xcdr_dll.h
    xcdr_rtiddsgen_infrastructure.h
    xcdr_infrastructure.h
    xcdr_infrastructure_impl.h
    xcdr_interpreter_impl.h
    xcdr_typeCode.h
)

IF (UPDATE_PATCHED)
    LIST(APPEND RTI_MODULE_SHARED_HDR
    )
    SET(RTI_MODULE_SHARED_HDR_PATCHED)
ELSE()
    SET(RTI_MODULE_SHARED_HDR_PATCHED
    )
ENDIF()

SET(RTI_MODULE_PUBLIC_HDR
    xcdr_infrastructure_psm.h
    xcdr_infrastructure_psm_impl.h
    xcdr_dll_psm.h
    xcdr_log.h
    xcdr_dds_interpreter.h
    xcdr_dds_xcdr_type_plugin.h
    xcdr_heapmgr.h
)
