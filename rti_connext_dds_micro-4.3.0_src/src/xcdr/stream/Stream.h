/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_stream.h"
#include "xcdr/xcdr_typeCode.h"

#define RTIXCdrStreamState_INITIALIZER {\
    NULL, /* buffer */ \
    NULL, /* relativeBuffer */ \
    0 /* bufferLength */ \
}

/* This structure is used to store a stream aligned position
 *
 * An aligned position consists of two pieces of information:
 * 1) The stream position itself
 * 2) The stream buffer position that is used as the logical 0 for
 * alignment purposes during serialization
 *
 * currentPosition = stream->_currentPosition
 * alignmentPosition = stream->_relativeBuffer
 */
struct RTIXCdrStreamAlignedPosition {
    char *currentPosition;
    char *alignmentPosition;
};

#define RTIXCdrStreamAlignedPosition_INITIALIZER { \
    NULL, \
    NULL \
}

#define RTI_XCDR_PARAMETER_HEADER_ALIGNMENT 4
#define RTI_XCDR_V1_PARAMETER_SHORT_HEADER_SIZE 4u
#define RTI_XCDR_V1_PARAMETER_LONG_HEADER_SIZE 12u
#define RTI_XCDR_V1_PID_EXTENDED 0x3F01
#define RTI_XCDR_V1_PARAMETER_FLAG_MUST_UNDERSTAND 0x4000
#define RTI_XCDR_V1_LONG_PARAMETER_FLAG_MUST_UNDERSTAND 0x40000000

#define RTI_XCDR_V1_PID_LIST_END 0x3F02

#define RTI_XCDR_RTPS_SUBMESSAGE_ALIGNMENT (4)

extern void RTIXCdrStream_initialize(RTIXCdrStream *me);

/* Align, check size and set padding to zero */
extern RTIXCdrBoolean RTIXCdrStream_align_ex(
        RTIXCdrStream *me,
        RTIXCdrAlignment alignment,
        RTIXCdrBoolean zeroOnAlign);

extern void RTIXCdrStream_pushState(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState * state,
        RTIXCdrBoolean setNewLength,
        RTIXCdrUnsignedLong newLength);

extern void RTIXCdrStream_popStateNoAlign(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState * state);

extern void RTIXCdrStream_getAlignedPosition(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamAlignedPosition *position);

extern void RTIXCdrStream_restoreAlignedPosition(
        struct RTIXCdrStream *me,
        const struct RTIXCdrStreamAlignedPosition *position);

extern RTIXCdrBoolean RTIXCdrStream_skipV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrBoolean extended);

extern RTIXCdrBoolean RTIXCdrStream_skipV2ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong LC);

extern RTIXCdrBoolean RTIXCdrStream_deserializeAndSkipV1ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean *isSentinel);

extern void RTIXCdrStream_moveToNextParameterHeader(
        struct RTIXCdrStream *me,
        const struct RTIXCdrStreamState * state,
        RTIXCdrUnsignedLong length);

extern RTIXCdrBoolean RTIXCdrStream_deserializeV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrUnsignedLong *parameterId,
        RTIXCdrUnsignedLong *length,
        RTIXCdrBoolean *extended,
        RTIXCdrBoolean *mustUnderstand);

RTIXCdrBoolean RTIXCdrStream_deserializeV2ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrUnsignedLong *parameterId,
        RTIXCdrUnsignedLong *length,
        RTIXCdrBoolean *mustUnderstand);

extern char * RTIXCdrStream_serializeV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrBoolean extended,
        RTIXCdrUnsignedLong parameterId,
        RTIXCdrBoolean mustUnderstand);

extern RTIXCdrBoolean RTIXCdrStream_finishV1ParameterHeader(
        struct RTIXCdrStream *me,
        const struct RTIXCdrStreamState *state,
        RTIXCdrBoolean extended,
        RTIXCdrBoolean addPaddingToParameterLenght,
        char *lengthPosition);

RTIXCdrEncapsulationId RTIXCdrEncapsulation_getEncapsulationId(
        RTIXCdrBoolean littleEndian,
        RTIXCdrBoolean v2,
        RTIXCdrExtensibilityKind extKind);

RTIXCdrAlignment RTIXCdrStream_getCurrentAlignment(
        RTIXCdrStream *me,
        RTIXCdrAlignment maxAlignment);

/* \brief An array size iterator will be used by the getMaxSizeSer and
 * getMinSizeSer to calculate the maximum size of arrays and sequences.
 *
 * Why do we need the iterator? When the size of arrays or the maximum
 * size of sequences is big, getting the max size or min size using brute force
 * will take a lot of time: O(n) execution time.
 *
 * With the iterator we will go to a O(1) execution time
 * in which we will only need to do a few iterations.
 *
 * The way the iterator work is by finding an alignment loop. When the loop
 * is found, calculating the size becomes a matter of multiplying
 * the size of the alignment group by the number of groups.This multiplication
 * is done inside the iterator.
 */
struct RTIXCdrArraySizeIterator {
    RTIXCdrLong align[8];
    RTIXCdrUnsignedLong sizeAlign[8];
    RTIXCdrBoolean leftover;
    RTIXCdrBoolean isStarted;
    RTIXCdrUnsignedLong currentElIndex;
};

#define RTIXCdrArraySizeIterator_INITIALIZER { \
    {-1,-1,-1,-1,-1,-1,-1,-1}, \
    {0,0,0,0,0,0,0,0}, \
    RTI_XCDR_FALSE, \
    RTI_XCDR_FALSE, \
    0 \
}

RTIXCdrBoolean RTIXCdrArraySizeIterator_next(
        struct RTIXCdrArraySizeIterator *me,
        RTIXCdrStream *stream,
        RTIXCdrBoolean *endIt,
        RTIXCdrUnsignedLong maxElCount);

RTIXCdrBoolean RTIXCdrArraySizeIterator_first(
        struct RTIXCdrArraySizeIterator *me,
        RTIXCdrStream *stream,
        RTIXCdrBoolean *endIt,
        RTIXCdrUnsignedLong maxElCount);

/* ------------------------------------------------------------------------- */
/* ---- Implementation ----------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrStream_initialize(me) \
    (me)->_buffer = NULL; \
    (me)->_bufferLength = RTIXCdrUnsignedLong_MAX; \
    (me)->_relativeBuffer = NULL; \
    (me)->_tmpRelativeBuffer = NULL; \
    (me)->_currentPosition = NULL; \
    (me)->_needByteSwap = RTI_XCDR_FALSE; \
    (me)->_endian = RTI_XCDR_ENDIAN_LITTLE; \
    (me)->_nativeEndian = RTI_XCDR_ENDIAN_LITTLE; \
    (me)->_encapsulationKind = RTI_XCDR_ENCAPSULATION_ID_CDR_LE; \
    (me)->_encapsulationOptions = RTI_XCDR_ENCAPSULATION_OPTIONS_NONE; \
    (me)->_zeroOnAlign = RTI_XCDR_FALSE

#define RTIXCdrStream_alignMacro(me, align, returnIfError) \
{ \
    RTIXCdrUnsignedLong __dataLength; \
    RTIXCdrUnsignedLong __alignedDataLength; \
 \
    __dataLength = RTIXCdrStream_getRelativeCurrentPositionOffset((me)); \
    __alignedDataLength = RTIXCdrAlignment_alignSizeUp(__dataLength, (align)); \
 \
    if (__alignedDataLength > __dataLength) { \
        if (!RTIXCdrStream_checkSize((me), __alignedDataLength - __dataLength)) { \
            returnIfError; \
        } \
 \
        if ((me)->_zeroOnAlign) { \
            RTIXCdrMemory_zero( \
                (me)->_currentPosition, \
                (RTIXCdrUnsignedLong) (((me)->_relativeBuffer + __alignedDataLength) - (me)->_currentPosition)); \
        } \
    } \
 \
    (me)->_currentPosition = (me)->_relativeBuffer + __alignedDataLength; \
}

#define RTIXCdrStream_deserializeNByteFast(me, val, n) \
    (void)(RTIXCdrMemory_copy((val), (me)->_currentPosition, n), \
           (me)->_currentPosition += (n))

#define RTIXCdrStream_deserializeNByte(me, val, alignment, n) \
  ((RTIXCdrStream_align((me), (alignment)) && \
    RTIXCdrStream_checkSize((me), (n))) ? \
   (RTIXCdrStream_deserializeNByteFast((me), (RTIXCdr1Byte *)(val), (n)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

#define RTIXCdrStream_getAlignedPosition(me, position) \
    (position)->currentPosition = (me)->_currentPosition; \
    (position)->alignmentPosition = (me)->_relativeBuffer

#define RTIXCdrStream_restoreAlignedPosition(me, position) \
    (me)->_currentPosition = (position)->currentPosition; \
    (me)->_relativeBuffer = (position)->alignmentPosition

#define RTIXCdrStream_pushState(me, state, setNewLength, newLength) \
    (state)->buffer = (me)->_buffer; \
    (state)->relativeBuffer = (me)->_relativeBuffer; \
    (state)->bufferLength = (me)->_bufferLength; \
    if (!setNewLength) { \
        (me)->_bufferLength -= (RTIXCdrUnsignedLong) \
              ((me)->_currentPosition - (me)->_buffer); \
    } else { \
        (me)->_bufferLength = (newLength); \
    } \
    (me)->_buffer = (me)->_currentPosition; \
    (me)->_relativeBuffer = (me)->_currentPosition

#define RTIXCdrStream_popStateNoAlign(me, state) \
    (me)->_bufferLength = (state)->bufferLength; \
    (me)->_buffer = (state)->buffer

#define RTIXCdrStream_moveToNextParameterHeader(me, state, length) \
    RTIXCdrStream_setCurrentPosition((me), (me)->_buffer + (length)); \
    RTIXCdrStream_popStateNoAlign((me), (state))

#define RTIXCdrArraySizeIterator_first RTIXCdrArraySizeIterator_next

/*
 * GREEN-3300: Casting pointers to size_t to guarantee that the subtraction 
 * returns and unsigned integer. The subtraction was returning a negative number 
 * when the difference was greater then the maximum signed int.
 * 
 * This was mainly a problem in 32-bit platforms where the size of a
 * signed int is 32 bits.
 */
#define RTIXCdrStream_getCurrentAlignment(me__, maxAlignment__) \
    ((RTIXCdrAlignment) (((RTIXCdrUnsignedLongLong) (me__)->_currentPosition \
                          - (RTIXCdrUnsignedLongLong) (me__)->_relativeBuffer) \
                         % (RTIXCdrUnsignedLongLong) (maxAlignment__)))
