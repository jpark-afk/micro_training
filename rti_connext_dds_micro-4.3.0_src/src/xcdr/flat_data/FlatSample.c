/*
(c) Copyright, Real-Time Innovations, 2018-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_flat_data.h"
#include "xcdr/xcdr_stream.h"
#include "../infrastructure/Infrastructure.h"
#include "../stream/Stream.h"


/*
 * Infrastructure for FlatData
 */
RTIXCdrBoolean RTIXCdrFlatSample_initializeEncapsulationAndStream(
        char *buffer,
        struct RTIXCdrStream *stream,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrUnsignedLong bufferSize)
{

    RTIXCdrLog_testPrecondition(buffer == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);

    /* serialize encapsulation into the buffer */
    RTIXCdrStream_initWithBuffer(stream, buffer, bufferSize);
    return RTIXCdrStream_serializeAndSetCdrEncapsulation(
            stream,
            encapsulationId);
}

RTIXCdrBoolean RTIXCdrFlatSample_initializeEncapsulation(
        char *buffer,
        RTIXCdrEncapsulationId encapsulationId)
{
    struct RTIXCdrStream tmp_stream;

    return RTIXCdrFlatSample_initializeEncapsulationAndStream(
            buffer,
            &tmp_stream,
            encapsulationId,
            RTI_XCDR_ENCAPSULATION_SIZE);
}

RTIXCdrEncapsulationId RTIXCdrFlatSample_getEncapsulation(
        const char *buffer)
{
    RTIXCdrEncapsulationId encapsulationId;

    RTIXCdrStream_getEncapsulationIdFromBuffer(&encapsulationId, buffer);
    return encapsulationId;
}

RTIXCdrUnsignedLong RTIXCdrFlatSample_getMutableSampleSize(
        const unsigned char *sample,
        RTIXCdrUnsignedLongLong offset)
{
    struct RTIXCdrStream stream;
    RTIXCdrUnsignedLong size;

    RTIXCdrLog_testPrecondition(sample == NULL, return 0);

    RTIXCdrFlatData_initializeStream(
            &stream, 
            (unsigned char *) sample, 
            offset, 
            RTI_XCDR_DHEADER_SIZE);

    if (!RTIXCdrStream_deserializeDHeader(
            &stream,
            NULL, /* invalidDHeader */
            &size,
            NULL, /* dheaderPosition */
            NULL /* state */)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
                "cannot deserialize DHeader");
        return 0;
    }

    return size + RTI_XCDR_DHEADER_SIZE;
}

RTIXCdrBoolean RTIXCdrFlatData_initializeSample(
        char *buffer,
        RTIXCdrUnsignedLongLong serialized_size,
        struct RTIXCdrInterpreterPrograms *programs)
{
    struct RTIXCdrProgram *program = NULL;
    struct RTIXCdrSampleProgramContext context =
            RTIXCdrSampleProgramContext_INITIALIZER;
    RTIXCdrInitializeSampleProperty property =
            RTIXCdrInitializeSampleProperty_INITIALIZER;

    RTIXCdrLog_testPrecondition(buffer == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);

    context.languageBinding = RTI_XCDR_TYPE_BINDING_FLAT_DATA_MASK;
    program = RTIXCdrInterpreterPrograms_getInitializeSampleProgram(programs);
    if (program == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
                "get initialize program");
        return RTI_XCDR_FALSE;
    }

    RTIXCdrLog_testPrecondition(program->typeCode == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            serialized_size > RTI_XCDR_MAX_SERIALIZED_SIZE,
            return RTI_XCDR_FALSE);

    RTIXCdrMemory_zero(buffer, (RTIXCdrUnsignedLong) serialized_size);

    property.initializeToZero = RTI_XCDR_FALSE;
    if (!RTIXCdrSampleInterpreter_initializeSample(
            buffer,
            program->typeCode,
            program,
            &property,
            &context)) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss,
                "FlatData initialization",
                program->typeCode->_name == NULL ?
                        "anonymous" : program->typeCode->_name);
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

/* Allocate 4-byte-aligned buffer that can contain the sample and end at
 * MIG_RTPS_SUBMESSAGE_ALIGNMENT boundary.
 * After initializing a sample, we align the stream to
 * MIG_RTPS_SUBMESSAGE_ALIGNMENT. Hence we allocate extra memory, so that
 * the allocated buffer ends at MIG_RTPS_SUBMESSAGE_ALIGNMENT boundary.
 * */
RTI_PRIVATE
char * RTIXCdrFlatData_allocateSample(
        RTIXCdrUnsignedLongLong serialized_size)
{
    char *buffer = NULL;
    RTIXCdrUnsignedLong buffer_size;

    if (serialized_size > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
                "serialized size maximum allowed size");
        return NULL;
    }

    buffer_size = (RTIXCdrUnsignedLong)(RTI_XCDR_ENCAPSULATION_SIZE
            + RTIXCdrAlignment_alignSizeUp(
                    serialized_size,
                    RTI_XCDR_RTPS_SUBMESSAGE_ALIGNMENT));

    if (buffer_size > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
                "serialized size maximum allowed size");
        return NULL;
    }

    /* Allocate 4-byte-aligned buffer that can contain the sample */
    buffer = RTIXCdrHeap_allocate(buffer_size);
    if (buffer == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_BUFFER_FAILURE_MSG_ID_d,
                (RTIXCdrLong)buffer_size);
        return NULL;
    }

    return buffer;
}

/*  
 * Creates a Final FlatData sample.
 *
 * A FlatData sample is a buffer that can hold the serialized type plus
 * the CDR encapsulation. 
 * 
 * This function returns that buffer where only the encapsulation (initial 4
 * bytes) has been initialized.
 */
unsigned char * RTIXCdrFlatData_createSampleFinal(
        RTIXCdrUnsignedLongLong serialized_size,
        struct RTIXCdrInterpreterPrograms *programs)
{
    char *buffer = NULL;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(programs == NULL, return NULL);
    
    buffer = RTIXCdrFlatData_allocateSample(serialized_size);
    if (buffer == NULL) {
        return NULL;
    }

    /* initialize encapsulation */
    if (!RTIXCdrFlatSample_initializeEncapsulation(
            buffer,
            RTIXCdrEncapsulationId_getNativePlainCdr2())) {
        RTIXCdrLog_logStr(
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
            "FlatData sample encapsulation header initialization");
        goto done;
    }

    /* initialize the sample */
    if (!RTIXCdrFlatData_initializeSample(
            buffer + RTI_XCDR_ENCAPSULATION_SIZE,
            serialized_size,
            programs)) {
        RTIXCdrLog_logStr(
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
            "FlatData sample initialization");
        goto done;
    }

    ok = RTI_XCDR_TRUE;
done:
    if (!ok) {
        RTIXCdrHeap_free(buffer);
        buffer = NULL;
    }
    return (unsigned char *) buffer;   
}

RTIXCdrBoolean RTIXCdrFlatData_initializeMutableSample(
        unsigned char *sample)
{
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);

    /* initialize encapsulation */
    if (!RTIXCdrFlatSample_initializeEncapsulation(
            (char *) sample,
            RTIXCdrEncapsulationId_getNativePlCdr2())) {
        RTIXCdrLog_logStr(
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
            "FlatData sample encapsulation header initialization");
        return RTI_XCDR_FALSE;
    }

    /* set D Header to 0 */
    RTIXCdrMemory_zero(
            sample + RTI_XCDR_ENCAPSULATION_SIZE,
            RTI_XCDR_DHEADER_SIZE);

    return RTI_XCDR_TRUE;
}

/*
 * Creates a Mutable FlatData sample with max serialized size
 *
 * A FlatData sample is a buffer that can hold the serialized type plus
 * the CDR encapsulation.
 *
 * This function returns that buffer where only the encapsulation (initial 4
 * bytes) has been initialized.
 */
unsigned char * RTIXCdrFlatData_createSampleMutable(
        struct RTIXCdrInterpreterPrograms *programs)
{
    char *buffer = NULL;
    struct RTIXCdrProgram *program = NULL;
    struct RTIXCdrTypePluginProgramContext defaultProgramContext =
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    RTIXCdrUnsignedLong serialized_size = 0;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(programs == NULL, return NULL);

    program = RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(
            programs,
            RTI_XCDR_TRUE,
            RTI_XCDR_FALSE);

    RTIXCdrLog_testPrecondition(program == NULL, return NULL);

    RTIXCdrLog_testPrecondition(
            program->typeCode == NULL,
            return NULL);

    if (!RTIXCdrInterpreter_getSerSampleMaxSize(
            &serialized_size,
            program->typeCode,
            program,
            &defaultProgramContext)) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GET_FAILURE_ID_ss,
                "get max serialized size",
                program->typeCode->_name == NULL ?
                        "anonymous" : program->typeCode->_name);
    }

    if (serialized_size >= RTIXCdrLong_MAX) {
        return NULL;
    }
    buffer = RTIXCdrFlatData_allocateSample(serialized_size);
    if (buffer == NULL) {
        return NULL;
    }

    if (!RTIXCdrFlatData_initializeMutableSample((unsigned char *) buffer)) {
        goto done;
    }

    ok = RTI_XCDR_TRUE;
done:
    if (!ok) {
        RTIXCdrHeap_free(buffer);
        buffer = NULL;
    }
    return (unsigned char *) buffer;
}

unsigned char * RTIXCdrFlatData_cloneSample(
        const unsigned char *sample,
        RTIXCdrUnsignedLongLong serialized_size)
{
    char *buffer = NULL;

    RTIXCdrLog_testPrecondition(sample == NULL, return NULL);

    if (serialized_size
        > (RTI_XCDR_MAX_SERIALIZED_SIZE - RTI_XCDR_ENCAPSULATION_SIZE)) {
        return NULL;
    }

    buffer = RTIXCdrFlatData_allocateSample(serialized_size);
    if (buffer == NULL) {
        return NULL;
    }

    RTIXCdrMemory_copy(
            buffer,
            sample,
            (RTIXCdrUnsignedLong) (serialized_size
                                   + RTI_XCDR_ENCAPSULATION_SIZE));

    return (unsigned char *) buffer;   
}

void RTIXCdrFlatData_deleteSample(void *sample)
{
    RTIXCdrHeap_free(sample);
}

/*
 * Initializes a stream that shares the buffer flat_sample, beginning at an
 * offset from the beginning of the buffer, and with a smaller size than
 * the overall buffer.
 */
void RTIXCdrFlatData_initializeStream(
        struct RTIXCdrStream *stream,
        unsigned char *flat_sample,
        RTIXCdrUnsignedLongLong offset,
        RTIXCdrUnsignedLongLong serialized_size)
{
    /* Initialize stream and set the encapsulation (already in the flat_sample
    buffer */
    RTIXCdrStream_initWithBuffer(
            stream, 
            (char *) flat_sample, 
            RTI_XCDR_ENCAPSULATION_SIZE);
    RTIXCdrStream_deserializeAndSetCdrEncapsulation(stream);

    /* Position the buffer at the member with the specified offset and size */
    RTIXCdrStream_set(
            stream, 
            ((char *)flat_sample) + offset,
            (RTIXCdrUnsignedLong) serialized_size);
}
