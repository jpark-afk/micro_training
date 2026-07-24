/*
 * FILE: BuiltinIpcCdr.h - CDR helpers for Builtin Inter-Participant Channels
 *
 * Copyright (c) 2017-2025 Real-Time Innovations,Inc. All rights reserved.
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 06Nov2017,as  Created
 */

#ifndef BuiltinIpcCdr_h
#define BuiltinIpcCdr_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif

#include "dds_c/dds_c_infrastructure.h"
#include "dds_c/dds_c_discovery.h"

/* ------------------------------------------------------------------------- *
 *        DDS_GUID_t CDR Helpers
 * ------------------------------------------------------------------------- */
RTI_BOOL
DDS_GUID_serialize(struct CDR_Stream_t *stream,
                   const DDS_GUID_t *const sample);

RTI_BOOL
DDS_GUID_deserialize(struct CDR_Stream_t *stream,
                     DDS_GUID_t *sample);

RTI_UINT32
DDS_GUID_get_max_size_serialized(RTI_UINT32 current_alignment);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_serialize_builtin_endpoint_qos(
    struct CDR_Stream_t *stream,
    const DDS_BuiltinEndpointQos_t *builtin_endpoint_qos,
    void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DDS_CdrQosPolicy_deserialize_builtin_endpoint_qos(
    struct CDR_Stream_t *stream,
    DDS_BuiltinEndpointQos_t *builtin_endpoint_qos,
    void *param);

MUST_CHECK_RETURN extern RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_builtin_endpoint_qos(
    RTI_UINT32 size);


#endif /* BuiltinIpcCdr_h */

