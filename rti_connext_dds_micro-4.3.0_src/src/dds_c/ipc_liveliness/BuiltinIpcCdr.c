/*
 * FILE: BuiltinIpcCdr.c - CDR helpers for Builtin Inter-Participant Channels
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

#ifndef dds_log_h
#include "dds_c/dds_c_log.h"
#endif

#include "dds_c/dds_c_infrastructure.h"
#include "dds_c/dds_c_discovery.h"
#include "cdr/cdr_stream.h"
#include "cdr/cdr_serialize.h"
#include "BuiltinIpcCdr.h"
#include "BuiltinCdr.h"

/*** SOURCE_BEGIN ***/

/* ------------------------------------------------------------------------- *
 *        DDS_GUID_t CDR Helpers
 * ------------------------------------------------------------------------- */
RTI_BOOL
DDS_GUID_serialize(
        struct CDR_Stream_t *stream,
        const DDS_GUID_t *const sample)
{
    DDS_UnsignedLong *guid_field = NULL;
    RTI_UINT32 i = 0;
    RTI_UINT32 field_num = DDS_GUID_LENGTH /
                            (sizeof(DDS_UnsignedLong)/sizeof(DDS_Octet));

    for (i = 0; i < field_num; ++i)
    {
         guid_field = OSAPI_Compiler_reinterpret_cast(
                                     DDS_UnsignedLong*,sample->value) + i;

        if (!CDR_Stream_serialize_unsigned_long_to_big_endian(stream,guid_field))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_GUID_deserialize(struct CDR_Stream_t *stream,
                     DDS_GUID_t *sample)
{
    DDS_UnsignedLong *guid_field = NULL;
    RTI_UINT32 i = 0;
    RTI_UINT32 field_num = DDS_GUID_LENGTH /
                           (sizeof(DDS_UnsignedLong)/sizeof(DDS_Octet));

    for (i = 0; i < field_num; ++i)
    {
        guid_field = OSAPI_Compiler_reinterpret_cast(
                                    DDS_UnsignedLong*,sample->value) + i;

        if (!CDR_Stream_deserialize_unsigned_long_from_big_endian(
                stream,guid_field))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_GUID_get_max_size_serialized(RTI_UINT32 current_alignment)
{
    RTI_UINT32 initial_alignment = current_alignment;

    current_alignment += CDR_get_max_size_serialized_primitive_array(
                                        current_alignment,
                                        DDS_GUID_LENGTH,
                                        CDR_OCTET_TYPE);

    return  current_alignment - initial_alignment;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_builtin_endpoint_qos(
    struct CDR_Stream_t *stream,
    const DDS_BuiltinEndpointQos_t *builtin_endpoint_qos,
    void *param)
{
    DDS_BuiltinEndpointQos_t local_builtin_endpoint_qos = 
                                                         *builtin_endpoint_qos;
    UNUSED_ARG(param);

    /* ensure that the internal valid bit is not serialized */
    local_builtin_endpoint_qos &= (~DDS_BUILTIN_ENDPOINT_QOS_BIT_IS_VALID);

    return DDS_CdrStream_serialize_4_byte_parameter(stream,
                                   &local_builtin_endpoint_qos,
                                   RTPS_PID_BUILTIN_ENDPOINT_QOS);
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_builtin_endpoint_qos(
    struct CDR_Stream_t *stream,
    DDS_BuiltinEndpointQos_t *builtin_endpoint_qos,
    void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = ok
            && CDR_Stream_deserialize_unsigned_long(stream,
                    (RTI_UINT32 *)builtin_endpoint_qos);

    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_builtin_endpoint_qos(
    RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}
