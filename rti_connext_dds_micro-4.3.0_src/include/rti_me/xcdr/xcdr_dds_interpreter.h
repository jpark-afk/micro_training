/*
 * FILE: xcdr_dds_interpreter.h - helper functions needed by interpreter
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef xcdr_dds_interpreter_h
#define xcdr_dds_interpreter_h

#include "xcdr/xcdr_dll.h"
#ifndef xcdr_typeCode_h
#include "xcdr/xcdr_typeCode.h"
#endif
#include "dds_c/dds_c_typecode.h"
#include "dds_c/dds_c_type.h"

#ifdef __cplusplus
    extern "C" {
#endif

extern RTIXCdrDllVariable RTIXCdrSampleAccessInfo DDS_g_sai_seq;
extern RTIXCdrDllVariable const struct DDS_TypeAllocationParams_t DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;
extern RTIXCdrDllVariable const struct DDS_TypeDeallocationParams_t DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;

RTIXCdrDllExport RTIXCdrMemberValue 
DDS_Sequence_get_member_value_pointer(
        void *sample,
        RTIXCdrUnsignedLong *elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        RTIXCdrUnsignedLong elementIndex,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        void *programData);

RTIXCdrDllExport RTIXCdrMemberValue 
DDS_Sequence_set_member_element_count(
        RTIXCdrBoolean *failure,
        void *sample,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        RTIXCdrBoolean trimToSize,
        RTIXCdrBoolean initializeElement,
        void *programData);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
XCDR_Interpreter_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        RTI_BOOL serializeEncapsulation,
        RTI_BOOL serializeSample,
        DDS_EncapsulationId_t encapsulationId);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
XCDR_Interpreter_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeSample,
        DDS_EncapsulationId_t encapsulationId);

RTIXCdrDllExport RTI_UINT32
XCDR_Interpreter_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL includeEncapsulation,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment);

RTIXCdrDllExport DDS_ReturnCode_t
XCDR_Interpreter_serialized_sample_to_buffer(
                                 struct DDS_TypePlugin *tp,
                                 char *buffer,
                                 unsigned int *length,
                                 const void *a_data,
                                 DDS_DataRepresentationId_t representation,
                                 DDS_EncapsulationId_t encapsulation);
#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_dds_interpreter_h */
