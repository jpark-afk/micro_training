/*
 * FILE: Interpreter.c - wrapper around the XCDR Interpreter functions
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "cdr/cdr_md5.h"

#include "Interpreter.h"
#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"
#include "dds_c/dds_c_typecode.h"
#include "xcdr/xcdr_dds_interpreter.h"

struct PRESTypePluginStreamContext 
{
    char * buffer;
    RTIXCdrUnsignedLong bufferLength;
};

#define PRESTypePluginStreamContext_INITIALIZER \
{ \
    NULL, \
    0 \
}

#define PRESTypePlugin_saveStreamContext(streamContext, stream) \
    (streamContext)->bufferLength = (stream)->_bufferLength; \
    (streamContext)->buffer = (stream)->_buffer

#define PRESTypePlugin_restoreStreamContext(stream, streamContext) \
    if ((streamContext)->buffer != NULL) { \
        (stream)->_bufferLength = (streamContext)->bufferLength; \
        (stream)->_buffer = (streamContext)->buffer; \
    }
/*
 * For C and C++ the deserialization operation does not need to
 * initialize the target sample because all fields will be populated if the
 * extensibility is FINAL
 */
#define PRESTypePlugin_isInitializationNeeded(program) \
    ((program)->extKind != RTI_XCDR_FINAL_EXTENSIBILITY)

void
XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(
        RTIXCdrStream *pro_xcdr_stream,
        struct CDR_Stream_t *micro_cdr_stream)
{
    /* This sets _nativeEndian member */
    RTIXCdrStream_init(pro_xcdr_stream);

    /* Pro's _buffer maps to Micro's real_buffer. Both point to the absolute start
     * of the buffer used for serialization
     */
    pro_xcdr_stream->_buffer = micro_cdr_stream->buffer;
    pro_xcdr_stream->_relativeBuffer = micro_cdr_stream->buff_ptr;
    pro_xcdr_stream->_currentPosition = micro_cdr_stream->buff_ptr;
    pro_xcdr_stream->_tmpRelativeBuffer = micro_cdr_stream->buff_ptr;
    pro_xcdr_stream->_bufferLength = micro_cdr_stream->length;
    pro_xcdr_stream->_encapsulationKind = RTI_XCDR_ENCAPSULATION_ID_CDR_NATIVE_ENDIAN;
    pro_xcdr_stream->_encapsulationOptions = RTI_XCDR_ENCAPSULATION_OPTIONS_NONE;
    pro_xcdr_stream->_endian = micro_cdr_stream->endian;
    pro_xcdr_stream->_needByteSwap = micro_cdr_stream->need_byte_swap;
    pro_xcdr_stream->_zeroOnAlign = RTI_FALSE;
}

RTIXCdrBoolean
XCDR_Interpreter_isEncapsulationCdrV2(DDS_EncapsulationId_t id)
{
    if (((id >= RTI_XCDR_ENCAPSULATION_ID_CDR2_BE)
          && (id <= RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE)) ||
        (id == DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_NATIVE))
    {
        return RTI_XCDR_TRUE;
    }

    return RTI_XCDR_FALSE;
}

MUST_CHECK_RETURN RTI_BOOL
XCDR_Interpreter_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        RTI_BOOL serializeEncapsulation,
        RTI_BOOL serializeSample,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    char *position = NULL;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;
    struct PRESTypePluginStreamContext streamContext = PRESTypePluginStreamContext_INITIALIZER;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

            
    if (serializeEncapsulation || context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2;
        RTIXCdrBoolean isLittleEndian;

        PRESTypePlugin_saveStreamContext(&streamContext, xcdrStream);
        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(encapsulationId);
        isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId);
        context->onlyKey = RTI_XCDR_FALSE;
        context->program = RTIXCdrInterpreterPrograms_getSerProgram(
                plugin->property.programs,
                isLittleEndian ? RTI_TRUE : RTI_FALSE,
                isCdrV2 ? RTI_TRUE : RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->inBaseClass = RTI_XCDR_FALSE;
        nullifyProgram = RTI_XCDR_TRUE;
    }

    if (serializeEncapsulation)
    {
        result = RTIXCdrStream_serializeAndSetCdrEncapsulation(
                xcdrStream,
                context->program->encapsulationId);
        if (!result)
        {
            goto done;
        }

        position = RTIXCdrStream_resetAlignment(xcdrStream);
    }
    else
    {
        RTIXCdrStream_setEncapsulationId(xcdrStream, context->program->encapsulationId);
    }

    if (serializeSample)
    {
        result = (RTIBool) RTIXCdrInterpreter_serializeSample(
                xcdrStream,
                (void *) data,
                context->typeCode,
                context->program,
                context);
        if (!result)
        {
            goto done;
        }
    }

    if (serializeEncapsulation)
    {
        RTIXCdrStream_restoreAlignment(xcdrStream, position);
    }

done:

    if (!result)
    {
        PRESTypePlugin_restoreStreamContext(xcdrStream, &streamContext);
    }

    if (nullifyProgram)
    {
        context->program = NULL;
    }

    stream->buff_ptr = xcdrStream->_currentPosition;

    return result;
}

MUST_CHECK_RETURN RTI_BOOL
XCDR_Interpreter_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeSample,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    char *position = NULL;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

    if (deserializeEncapsulation)
    {
        result = RTIXCdrStream_deserializeAndSetCdrEncapsulation(xcdrStream);

        if (!result)
        {
            goto done;
        }

        position = RTIXCdrStream_resetAlignment(xcdrStream);
    }

    if (deserializeEncapsulation || context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2;
        RTIXCdrBoolean isLittleEndian;

        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(encapsulationId);
        isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId);
        context->onlyKey = RTI_XCDR_FALSE;
        context->program = RTIXCdrInterpreterPrograms_getDeserProgram(
                plugin->property.programs,
                isLittleEndian ? RTI_TRUE : RTI_FALSE,
                isCdrV2 ? RTI_TRUE : RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->inBaseClass = RTI_XCDR_FALSE;
        nullifyProgram = RTI_XCDR_TRUE;

        if (deserializeSample)
        {
            if (context->typeCode->_typePlugin != NULL)
            {
                if (PRESTypePlugin_isInitializationNeeded(context->program))
                {
                    if (context->typeCode->_typePlugin->initializeSampleFnc != NULL)
                    {
                        if (!context->typeCode->_typePlugin->initializeSampleFnc(sample, 0, 0))
                        {
                            goto done;
                        }
                    }
                    else if (context->typeCode->_typePlugin->initializeSampleWParamsFnc != NULL)
                    {
                        if (!context->typeCode->_typePlugin->initializeSampleWParamsFnc(
                                sample,
                                context->typeCode,
                                NULL,
                                context->programData,
                                context->typeCode->_typePlugin->typePluginParam))
                        {
                            goto done;
                        }
                    }
                }
            }
        }

    }

    if (deserializeSample)
    {

        /* In Pro, this property is received from PRES */
        struct RTIXCdrSampleAssignabilityProperty ap =
        RTIXCdrSampleAssignabilityProperty_INITIALIZER;
        result = (RTIBool) RTIXCdrInterpreter_deserializeSample(
                (void *) sample,
                xcdrStream,
                context->typeCode,
                context->program,
                &ap,
                context);

        if (!result)
        {
            goto done;
        }
    }

    if (deserializeEncapsulation)
    {
        RTIXCdrStream_restoreAlignment(xcdrStream, position);
    }

    done:

    if (nullifyProgram)
    {
        context->program = NULL;
    }

    stream->buff_ptr = xcdrStream->_currentPosition;

    return result;
}

RTI_BOOL
XCDR_Interpreter_serialize_key(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        RTI_BOOL serializeEncapsulation,
        RTI_BOOL serializeKey,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    char *position = NULL;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;
    struct PRESTypePluginStreamContext streamContext = PRESTypePluginStreamContext_INITIALIZER;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

    if (serializeEncapsulation || context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2 = RTI_TRUE;
        RTIXCdrBoolean isLittleEndian;

        PRESTypePlugin_saveStreamContext(&streamContext, xcdrStream);
        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(encapsulationId);
        isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId);
        context->onlyKey = RTI_XCDR_TRUE;
        context->program = RTIXCdrInterpreterPrograms_getSerProgram(
                plugin->property.programs,
                isLittleEndian?RTI_TRUE:RTI_FALSE,
                isCdrV2?RTI_TRUE:RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->inBaseClass = RTI_XCDR_FALSE;
        nullifyProgram = RTI_XCDR_TRUE;

/*
        if (!isCdrV2) {
            if (PRESTypePluginDefaultEndpointData_getMaxSizeSerializedSample(epd) >
                    RTI_UINT16_MAX) {
                context->useXcdr1ExtendedId = RTI_XCDR_TRUE;
            } else {
                context->useXcdr1ExtendedId = RTI_XCDR_FALSE;
            }
        }
*/

    }

    if (serializeEncapsulation)
    {
        result = RTIXCdrStream_serializeAndSetCdrEncapsulation(
                xcdrStream,
                context->program->encapsulationId);
        if (!result)
        {
            goto done;
        }

        position = RTIXCdrStream_resetAlignment(xcdrStream);
    }
    else
    {
        RTIXCdrStream_setEncapsulationId(xcdrStream, context->program->encapsulationId);
    }

    if (serializeKey)
    {
        result = (RTIBool) RTIXCdrInterpreter_serializeSample(xcdrStream,
                                                              (void *) sample,
                                                              context->typeCode,
                                                              context->program,
                                                              context);
        if (!result)
        {
            goto done;
        }
    }

    if (serializeEncapsulation)
    {
        RTIXCdrStream_restoreAlignment(xcdrStream, position);
    }

done:

    if (!result)
    {
        PRESTypePlugin_restoreStreamContext(xcdrStream, &streamContext);
    }

    if (nullifyProgram)
    {
        context->program = NULL;
    }

    stream->buff_ptr = xcdrStream->_currentPosition;

    return result;
}

RTI_BOOL
XCDR_Interpreter_deserialize_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeKey,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    char *position = NULL;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

    if (deserializeEncapsulation)
    {
        result = RTIXCdrStream_deserializeAndSetCdrEncapsulation(xcdrStream);

        if (!result)
        {
            goto done;
        }

        position = RTIXCdrStream_resetAlignment(xcdrStream);
    }

    if (deserializeEncapsulation || context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2;
        RTIXCdrBoolean isLittleEndian;

        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(encapsulationId);
        encapsulationId = RTIXCdrStream_getEncapsulationId(xcdrStream);
        isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId);
        context->onlyKey = RTI_XCDR_TRUE;
        context->program = RTIXCdrInterpreterPrograms_getDeserProgram(
                plugin->property.programs,
                isLittleEndian?RTI_TRUE:RTI_FALSE,
                isCdrV2?RTI_TRUE:RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->inBaseClass = RTI_XCDR_FALSE;
        nullifyProgram = RTI_XCDR_TRUE;
    }

    if (deserializeKey)
    {
        struct RTIXCdrSampleAssignabilityProperty ap = RTIXCdrSampleAssignabilityProperty_INITIALIZER;
        result = (RTIBool) RTIXCdrInterpreter_deserializeSample(
                (void *) sample,
                xcdrStream,
                context->typeCode,
                context->program,
                &ap,
                context);

        if (!result)
        {
            goto done;
        }
    }

    if (deserializeEncapsulation)
    {
        RTIXCdrStream_restoreAlignment(xcdrStream, position);
    }

done: if (nullifyProgram)
    {
        context->program = NULL;
    }

    stream->buff_ptr = xcdrStream->_currentPosition;

    return result;
}

RTI_UINT32
XCDR_Interpreter_get_serialized_key_size(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL includeEncapsulation,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment)
{
    RTIBool result = RTI_FALSE;
    RTIXCdrUnsignedLong size = 0;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    UNUSED_ARG(current_alignment);

    OSAPI_PRECONDITION_ALWAYS(context == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("context",context,RTI_TRUE);)
                       
    if (includeEncapsulation || context->program == NULL)
    {
        context->onlyKey = RTI_XCDR_TRUE;
        context->program = RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(
                plugin->property.programs,
                isCdrV2?RTI_TRUE:RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->overflow = RTI_XCDR_FALSE;
        context->xcdrStream = NULL;
        context->inBaseClass = RTI_XCDR_FALSE;

        /*
        if (overflow != NULL)
        {
            *overflow = context->overflow;
        }
        */
        nullifyProgram = RTI_XCDR_TRUE;
    }

    result = (RTIBool) RTIXCdrInterpreter_getSerSampleMaxSize(
            &size,
            context->typeCode,
            context->program,
            context);
    if (!result)
    {
        goto done;
    }

    if (includeEncapsulation)
    {
        size += RTI_XCDR_ENCAPSULATION_HEADER_SIZE;
    }

    /*
    if (overflow != NULL)
    {
        *overflow = context->overflow;
    }
    */

    result = RTI_XCDR_TRUE;
done:

    if (nullifyProgram)
    {
        context->program = NULL;
    }
    if (!result)
    {
        return 0;
    }

    return size;
}

RTI_UINT32
XCDR_Interpreter_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL includeEncapsulation,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment)
{
    RTIBool result = RTI_FALSE;
    RTIXCdrUnsignedLong size = 0;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    UNUSED_ARG(current_alignment);

    /* Defensive check. This should not happen. */
    OSAPI_PRECONDITION_ALWAYS(context == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("context",context,RTI_TRUE);)
    
    if (includeEncapsulation || context->program == NULL)
    {
        context->onlyKey = RTI_XCDR_FALSE;
        context->program = RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(
                plugin->property.programs,
                isCdrV2?RTI_TRUE:RTI_FALSE,
                context->onlyKey);
        context->typeCode = context->program->typeCode;
        context->overflow = RTI_XCDR_FALSE;
        context->xcdrStream = NULL;
        context->inBaseClass = RTI_XCDR_FALSE;

        /*
        if (overflow != NULL)
        {
            *overflow = context->overflow;
        }
        */
        nullifyProgram = RTI_XCDR_TRUE;
    }

    result = (RTIBool) RTIXCdrInterpreter_getSerSampleMaxSize(
            &size,
            context->typeCode,
            context->program,
            context);
    if (!result)
    {
        goto done;
    }

    if (includeEncapsulation)
    {
        size += RTI_XCDR_ENCAPSULATION_HEADER_SIZE;
    }

    /*
    if (overflow != NULL)
    {
        *overflow = context->overflow;
    }
    */

    result = RTI_XCDR_TRUE;

done: if (nullifyProgram)
    {
        context->program = NULL;
    }

    if (!result)
    {
        return 0;
    }

    return size;
}

/******************************************************************************
 *                               Keyhash functions
 ******************************************************************************/

RTI_BOOL
XCDR_Interpreter_serialize_key_for_keyhash(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;
    struct PRESTypePluginStreamContext streamContext = PRESTypePluginStreamContext_INITIALIZER;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

    if (context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2;

        PRESTypePlugin_saveStreamContext(&streamContext, xcdrStream);
        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(encapsulationId);
        context->onlyKey = RTI_XCDR_TRUE;
        context->program = RTIXCdrInterpreterPrograms_getSerProgramForKeyhash(
                plugin->property.programs,
                isCdrV2);
        context->typeCode = context->program->typeCode;
        context->inBaseClass = RTI_XCDR_FALSE;
        nullifyProgram = RTI_XCDR_TRUE;

//        if (!isCdrV2)
//        {
//            if (PRESTypePluginDefaultEndpointData_getMaxSizeSerializedSample(epd)
//                    > RTI_UINT16_MAX)
//            {
//                context->useXcdr1ExtendedId = RTI_XCDR_TRUE;
//            }
//            else
//            {
//                context->useXcdr1ExtendedId = RTI_XCDR_FALSE;
//            }
//        }

    }
    RTIXCdrStream_setEncapsulationId(xcdrStream, context->program->encapsulationId);
    result = (RTIBool) RTIXCdrInterpreter_serializeSample(xcdrStream,
                                                          (void *) sample,
                                                          context->typeCode,
                                                          context->program,
                                                          context);
    if (!result)
    {
        goto done;
    }

done:

    if (!result)
    {
        PRESTypePlugin_restoreStreamContext(xcdrStream, &streamContext);
    }

    if (nullifyProgram)
    {
        context->program = NULL;
    }

    stream->buff_ptr = xcdrStream->_currentPosition;

    return result;
}

RTI_UINT32
XCDR_Interpreter_get_serialized_key_size_for_keyhash(
        struct DDS_TypePlugin *plugin,
        RTI_BOOL isCdrV2,
        RTI_UINT32 current_alignment)
{
    RTIBool result = RTI_FALSE;
    RTIXCdrUnsignedLong size = 0;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    UNUSED_ARG(current_alignment);

    OSAPI_PRECONDITION_ALWAYS(context == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("context",context,RTI_TRUE);)
                       
    if (context->program == NULL)
    {
        context->onlyKey = RTI_XCDR_TRUE;
        context->program = RTIXCdrInterpreterPrograms_getMaxSerSizeProgramForKeyhash(
                plugin->property.programs,
                isCdrV2);
        context->typeCode = context->program->typeCode;
        context->overflow = RTI_XCDR_FALSE;
        context->xcdrStream = NULL;
        context->inBaseClass = RTI_XCDR_FALSE;
        /*
        if (overflow != NULL)
        {
            *overflow = context->overflow;
        }
        */
        nullifyProgram = RTI_XCDR_TRUE;
    }

    result = (RTIBool) RTIXCdrInterpreter_getSerSampleMaxSize(
            &size,
            context->typeCode,
            context->program,
            context);
    if (!result)
    {
        goto done;
    }

    /*
    if (overflow != NULL)
    {
        *overflow = context->overflow;
    }
    */

    result = RTI_XCDR_TRUE;
done:

    if (nullifyProgram)
    {
        context->program = NULL;
    }
    if (!result)
    {
        return 0;
    }

    return size;
}

RTI_BOOL
XCDR_Interpreter_serialized_sample_to_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeKey,
        DDS_EncapsulationId_t encapsulationId)
{
    RTIXCdrStream temp;
    RTIXCdrStream *xcdrStream = &temp;
    char *position = NULL;
    RTIBool result = RTI_FALSE;
    struct RTIXCdrTypePluginProgramContext *context = plugin->property.program_context;
    RTIXCdrBoolean nullifyProgram = RTI_XCDR_FALSE;

    XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(xcdrStream, stream);
    xcdrStream->_encapsulationKind = encapsulationId;

    if (deserializeEncapsulation)
    {
        result = RTIXCdrStream_deserializeAndSetCdrEncapsulation(xcdrStream);

        if (!result)
        {
            goto done;
        }

        position = RTIXCdrStream_resetAlignment(xcdrStream);
    }

    if (deserializeEncapsulation || context->program == NULL)
    {
        RTIXCdrBoolean isCdrV2;
        RTIXCdrBoolean isLittleEndian;
        RTIXCdrEncapsulationId stream_eid;

        stream_eid = RTIXCdrStream_getEncapsulationId(xcdrStream);
        isCdrV2 = XCDR_Interpreter_isEncapsulationCdrV2(stream_eid);
        isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(stream_eid);
        context->program = RTIXCdrInterpreterPrograms_getSerToKeyProgram(
                                    plugin->property.programs,
                                    isLittleEndian ? RTI_TRUE : RTI_FALSE,
                                    isCdrV2 ? RTI_TRUE : RTI_FALSE);
        context->typeCode = context->program->typeCode;
        nullifyProgram = RTI_XCDR_TRUE;
    }

    if (deserializeKey)
    {
        struct RTIXCdrSampleAssignabilityProperty ap = RTIXCdrSampleAssignabilityProperty_INITIALIZER;
        result = (RTIBool) RTIXCdrInterpreter_serializedSampleToKey(
                (void *)sample,
                xcdrStream,
                context->typeCode,
                context->program,
                &ap,
                context);

        if (!result) {
            goto done;
        }
    }

    if (deserializeEncapsulation) {
        RTIXCdrStream_restoreAlignment(xcdrStream, position);
    }

  done:
    if (nullifyProgram) {
        context->program = NULL;
    }
    return result;
}

DDS_ReturnCode_t
XCDR_Interpreter_serialized_sample_to_buffer(
                                 struct DDS_TypePlugin *tp,
                                 char *buffer,
                                 unsigned int *length,
                                 const void *a_data,
                                 DDS_DataRepresentationId_t representation,
                                 DDS_EncapsulationId_t encapsulation)
{
    RTI_UINT32 ser_length;
    struct CDR_Stream_t stream;
    struct DDS_TypePluginBuffer tbuf = DDS_TypePluginBuffer_INITIALIZER;

    /* NOTE: Returns max size and not actual size which is fast because the
    * max size is cached as programs are generated
    */
    ser_length = XCDR_Interpreter_get_serialized_sample_size(tp,RTI_TRUE,
                         (representation == DDS_XCDR2_DATA_REPRESENTATION),0);

    if (buffer == NULL)
    {
        *length = ser_length;
        return DDS_RETCODE_OK;
    }
    else if (*length < ser_length)
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    CDR_Stream_initialize(&stream);
    DDS_TypePluginBuffer_initialize_static(&tbuf,buffer,*length);

    if (!DDS_TypePlugin_set_stream(tp,&stream,&tbuf))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!XCDR_Interpreter_serialize(tp,&stream,a_data,
                                    RTI_TRUE,RTI_TRUE,encapsulation))
    {
        return DDS_RETCODE_ERROR;
    }

    *length = CDR_Stream_get_current_position_offset(&stream);

    return DDS_RETCODE_OK;
}
