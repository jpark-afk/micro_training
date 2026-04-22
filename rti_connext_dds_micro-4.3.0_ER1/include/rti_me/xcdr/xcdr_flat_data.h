/*
(c) Copyright, Real-Time Innovations, 2018-2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_flat_data_h
#define xcdr_flat_data_h


#include "xcdr/xcdr_dll.h"
#include "xcdr/xcdr_stream.h"
#include "xcdr/xcdr_interpreter.h"

#ifdef __cplusplus
    extern "C" {
#endif


#define RTI_XCDR_ENCAPSULATION_SIZE (4)


extern RTIXCdrDllExport
unsigned char * RTIXCdrFlatData_createSampleFinal(
        RTIXCdrUnsignedLongLong serialized_size,
        struct RTIXCdrInterpreterPrograms *programs);

extern RTIXCdrDllExport
unsigned char * RTIXCdrFlatData_createSampleMutable(
        struct RTIXCdrInterpreterPrograms *programs);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrFlatData_initializeMutableSample(unsigned char *sample);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrFlatSample_initializeEncapsulation(
        char *buffer,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrFlatSample_initializeEncapsulationAndStream(
        char *buffer,
        struct RTIXCdrStream *stream,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrUnsignedLong bufferSize);

extern RTIXCdrDllExport
RTIXCdrEncapsulationId RTIXCdrFlatSample_getEncapsulation(
        const char *buffer);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrFlatData_initializeSample(
        char *buffer,
        RTIXCdrUnsignedLongLong serialized_size,
        struct RTIXCdrInterpreterPrograms *programs);

extern RTIXCdrDllExport
unsigned char * RTIXCdrFlatData_cloneSample(
        const unsigned char *sample,
        RTIXCdrUnsignedLongLong serialized_size);

extern RTIXCdrDllExport
void RTIXCdrFlatData_deleteSample(void *sample);

extern RTIXCdrDllExport
void RTIXCdrFlatData_initializeStream(
        struct RTIXCdrStream *stream,
        unsigned char *flat_sample,
        RTIXCdrUnsignedLongLong offset,
        RTIXCdrUnsignedLongLong serialized_size);

extern RTIXCdrDllExport
RTIXCdrUnsignedLongLong RTIFlatSample_findMember(
        const unsigned char *sample,
        RTIXCdrUnsignedLongLong offset,
        RTIXCdrUnsignedLong searchMemberId,
        RTIXCdrUnsignedLong *sizeOut);

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrFlatSample_getMutableSampleSize(
        const unsigned char *flat_sample,
        RTIXCdrUnsignedLongLong offset);

/*
 * Logging functions
 *
 * These functions are only used when exceptions are disabled (Traditional and
 * Micro C++ APIs) but they have to be compiled into the xcdr library regardless
 * since the selection of an API happens when the user compiles its application
 * with the xcdr hpp headers.
 */
extern RTIXCdrDllExport
void RTIXCdrFlatData_logPreconditionError(
        const char *fileName,
        RTIXCdrUnsignedLong line,
        const char *errorMessage);

extern RTIXCdrDllExport
void RTIXCdrFlatData_logCreationError(
        const char *fileName,
        RTIXCdrUnsignedLong line,
        const char *entityName);

extern RTIXCdrDllExport
void RTIXCdrFlatData_logBuilderOutOfResources(
        const char *fileName,
        RTIXCdrUnsignedLong line);


#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_flat_data_h */
