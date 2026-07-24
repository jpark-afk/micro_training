/*
 * FILE: Interpreter.h - Interpreter header files
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef Interpreter_h
#define Interpreter_h

#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"

RTI_BOOL
XCDR_Interpreter_serialize_key(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        RTI_BOOL serializeEncapsulation,
        RTI_BOOL serializeKey,
        DDS_EncapsulationId_t encapsulationId);

RTI_BOOL
XCDR_Interpreter_deserialize_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeKey,
        DDS_EncapsulationId_t encapsulationId);

RTI_UINT32
XCDR_Interpreter_get_serialized_key_size(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL includeEncapsulation,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment);

RTI_BOOL
XCDR_Interpreter_serialize_key_for_keyhash(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        DDS_EncapsulationId_t encapsulationId);

RTI_UINT32
XCDR_Interpreter_get_serialized_key_size_for_keyhash(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment);

#endif /* Interpreter_h */
