/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_stream_h
#define xcdr_stream_h


#include "xcdr/xcdr_dll.h"
#include "xcdr/xcdr_infrastructure.h"

#ifdef __cplusplus
    extern "C" {
#endif

/* The maximum size of a CDR stream
 *
 * We subtract 1024 because the core does some operations
 * on the returned result that may end up adding some additional bytes
 *
 */
#define RTI_XCDR_MAX_SERIALIZED_SIZE (RTIXCdrLong_MAX-1024)

typedef RTIXCdrUnsignedShort RTIXCdrEncapsulationId;

#define RTI_XCDR_ENCAPSULATION_ID_CDR_BE     0x0000
#define RTI_XCDR_ENCAPSULATION_ID_CDR_LE     0x0001
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE  0x0002
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE  0x0003
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_BE    0x0006
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_LE    0x0007
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE  0x0008
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE  0x0009
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE  0x000a
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE  0x000b

#define RTI_XCDR_ENCAPSULATION_ID_CDR_BE_STR "CDR_BE"
#define RTI_XCDR_ENCAPSULATION_ID_CDR_LE_STR "CDR_LE"
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE_STR "PL_CDR_BE"
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE_STR "PL_CDR_LE"
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_BE_STR "CDR2_BE"
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_LE_STR "CDR2_LE"
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE_STR "D_CDR2_BE"
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE_STR "D_CDR2_LE"
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE_STR "PL_CDR2_BE"
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE_STR "PL_CDR2_LE"

#ifdef RTI_ENDIAN_LITTLE
#define RTI_XCDR_ENCAPSULATION_ID_CDR_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_CDR_LE
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_CDR2_LE
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE
#else
#define RTI_XCDR_ENCAPSULATION_ID_CDR_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_CDR_BE
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE
#define RTI_XCDR_ENCAPSULATION_ID_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_CDR2_BE
#define RTI_XCDR_ENCAPSULATION_ID_D_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE
#define RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_NATIVE_ENDIAN RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE
#endif

/*
 * Represents two bytes of options assigned in RTPS for each representation.
 * We are currently using 3 bits of this 2 bytes for a compression feature to
 * compress the user data and 2 bits are used to indicate how many bytes of
 * padding are at the end of the RTPS submessage.
 */
typedef RTIXCdrUnsignedShort RTIXCdrEncapsulationOptions;

#define RTI_XCDR_ENCAPSULATION_OPTIONS_NONE 0x0000

#define RTI_XCDR_ENCAPSULATION_HEADER_SIZE 4

#define RTI_XCDR_MAX_SERIALIZED_SIZE_NO_ENCAPSULATION \
        (RTI_XCDR_MAX_SERIALIZED_SIZE - RTI_XCDR_ENCAPSULATION_HEADER_SIZE)

#define RTI_XCDR_ENCAPSULATION_ID_SIZE 2
#define RTI_XCDR_ENCAPSULATION_OPTIONS_SIZE 2

/*
 * Size of the DHEADER for XCDR v2 encapsulation. This header is used in mutable
 * and appendable types.
 */
#define RTI_XCDR_V2_PARAMETER_HEADER_SIZE 4

/*
 * Compression header size only applicable for user data compression.
 * This header holds the size of the uncompressed sample providing to the reader
 * the information to allocate enouth memory to hold the uncompressed sample.
 * At this moment this is an extension from RTI.
 */
#define RTI_XCDR_COMPRESSION_HEADER_SIZE  4

#define RTI_XCDR_MAX_ALIGNMENT 8
#define RTI_XCDR_MAX_XCDR2_ALIGNMENT 4

struct RTIXCdrStreamState;

/*
 * The number of entries in the RTIXCdr_TCKind_g_primitiveSizes array so that
 * we can index into the array to get a primitve type's size without calling a
 * function and having to do a switch
 */
#define RTI_XCDR_TK_NUM_PRIMITIVE_SIZES 23

extern RTIXCdrDllVariable const RTIXCdrUnsignedLong
        RTIXCdr_TCKind_g_primitiveSizes[RTI_XCDR_TK_NUM_PRIMITIVE_SIZES];

/*
 * The sizes of primitive kinds in CDR, NOT in a language binding
 * 1st dimension: v1 or v2
 * 2nd dimension: memberKind
 */
extern RTIXCdrDllVariable const RTIXCdrUnsignedLong 
RTIXCdr_TCKind_g_primitiveCdrSizes[2][RTI_XCDR_TK_NUM_PRIMITIVE_SIZES];

/*
 * The alignments of primitive kinds in CDR, NOT in a language binding
 * 1st dimension: v1 or v2
 * 2nd dimension: memberKind
 */
extern RTIXCdrDllVariable const RTIXCdrAlignment
RTIXCdr_TCKind_g_primitiveCdrAlignments[2][RTI_XCDR_TK_NUM_PRIMITIVE_SIZES];

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrEncapsulationId_isCdr(
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrEncapsulationId_isCdrV2(
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrEncapsulationId_isLittleEndianCdr(
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrEncapsulationId_isBigEndianCdr(
        RTIXCdrEncapsulationId encapsulationId);

/*
 * Returns the encapsulation Id that corresponds to the plain (non-PL) CDR
 * version 2 with the native endianness
 */
extern RTIXCdrDllExport
RTIXCdrEncapsulationId RTIXCdrEncapsulationId_getNativePlainCdr2(void);

/*
 * Returns the encapsulation Id that corresponds to the PL CDR
 * version 2 with the native endianness
 */
extern RTIXCdrDllExport
RTIXCdrEncapsulationId RTIXCdrEncapsulationId_getNativePlCdr2(void);

typedef RTIXCdrOctet RTIXCdrEndian;

struct RTIXCdrStream;

extern RTIXCdrDllExport
void RTIXCdrStream_init(struct RTIXCdrStream *me);

extern RTIXCdrDllExport
void RTIXCdrStream_initWithBuffer(
        struct RTIXCdrStream *me,
        char *buffer,
        RTIXCdrUnsignedLong bufferLength);

extern RTIXCdrDllExport
void RTIXCdrStream_set(
        struct RTIXCdrStream *me,
        char *buffer,
        RTIXCdrUnsignedLong bufferLength);

extern RTIXCdrDllExport
RTIXCdrEncapsulationId RTIXCdrStream_getEncapsulationId(
        struct RTIXCdrStream *me);

extern RTIXCdrDllExport
void RTIXCdrStream_setEncapsulationId(
        struct RTIXCdrStream *me,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
void RTIXCdrStream_setEncapsulationOptions(
        struct RTIXCdrStream *me,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeAndSetCdrEncapsulation(
        struct RTIXCdrStream *me,
        RTIXCdrEncapsulationId encapsulationId);

/**
 * @brief Serializes the encapsulation header by using the encapsulation 
 * ID contained in encapsulationHeader.
 *
 * This function serializes the default encapsulation options with value 0.
 *
 * @param me The stream in which to serialize the encapsulation header.
 * @param encapsulationHeader The buffer containing the encapsulation ID in
 * the first 2 bytes in network byte order.
 *
 * @return RTIXCdrBoolean RTI_XCDR_TRUE if success, RTI_XCDR_FALSE otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeAndSetCdrEncapsulationFromHeader(
        struct RTIXCdrStream *me,
        const char *encapsulationHeader);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndianness(
        struct RTIXCdrStream *me,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrBoolean littleEndian);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserializeAndSetCdrEncapsulation(
        struct RTIXCdrStream *me);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_checkSize(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong size);

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrStream_getCurrentPositionOffset(
        struct RTIXCdrStream *me);

/**
 * @brief Sets the current position offset to the stream.
 * 
 * @param me[in,out] The stream in which to set the current position offset.
 * @param offset[in] The offset to set.
 */
extern RTIXCdrDllExport 
void RTIXCdrStream_setCurrentPositionOffset(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong offset);

extern RTIXCdrDllExport
char * RTIXCdrStream_resetAlignment(struct RTIXCdrStream *me);

extern RTIXCdrDllExport
void RTIXCdrStream_restoreAlignment(struct RTIXCdrStream *me, char *position);

extern RTIXCdrDllExport
char * RTIXCdrStream_getCurrentPosition(struct RTIXCdrStream *me);

extern RTIXCdrDllExport
void RTIXCdrStream_setCurrentPosition(
        struct RTIXCdrStream *me, 
        char *position);

extern RTIXCdrDllExport
void RTIXCdrStream_increaseCurrentPosition(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong count);

extern RTIXCdrDllExport
char *RTIXCdrStream_getBuffer(struct RTIXCdrStream *me);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeCdrEncapsulationDefault(
        struct RTIXCdrStream *me);

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrStream_getRemainder(struct RTIXCdrStream *me);

extern RTIXCdrDllExport
void RTIXCdrStream_initFromStream(
        struct RTIXCdrStream *me, 
        struct RTIXCdrStream *srcStream);

/*i
 * @brief Utility functions to set the compression ID to the encapsulation
 * options.
 * @note RTI_EXTENSION
 */
extern RTIXCdrDllExport
void RTIXCdrStream_setEncapsulationOptionsCompressionId(
        struct RTIXCdrStream *me,
        RTIXCdrCompressionId compressionId);

/*i
 * @brief Utility functions to get the compression ID directly from the
 * encapsulation options.
 * @note RTI_EXTENSION
 */
extern RTIXCdrDllExport
RTIXCdrCompressionId RTIXCdrStream_getEncapsulationOptionsCompressionId(
        struct RTIXCdrStream* me);

/**
 * @brief Pack the given parameters into the options (unsigned short type).
 *
 * If compression is not being used (compressionId == 0):
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 |0 0|0 0 0|SP |
 * +---------------+---------------+
 *
 * Otherwise:
 * 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 |SP | ID  |CP |
 * +---------------+---------------+
 * 
 * CP: Compression padding
 * ID: Compression ID
 * SP: Serialized data padding
 * 
 * @param options \n Out. Options.
 * @param compressionId \n In. Compression plugin class ID.
 * @param serializedDataNumPaddingBytes \n In. Number of padding bytes for the
 * serialized data.
 * @param compressionNumPaddingBytes \n In. Number of padding bytes for the
 * compressed data.
 */
extern RTIXCdrDllExport
void RTIXCdrEncapsulationOptions_pack(
        RTIXCdrEncapsulationOptions *options,
        RTIXCdrOctet serializedDataNumPaddingBytes,
        RTIXCdrCompressionId compressionId,
        RTIXCdrOctet compressionNumPaddingBytes);

/**
 * @brief Unpack the given options (unsigned short type) into the given \
 * parameters.
 *
 * @param options \n In. Options.
 * @param serializedDataNumPaddingBytes \n Out. Number of padding bytes for the
 * serialized data.
 * @param compressionId \n Out. Compression plugin class ID.
 * @param compressionNumPaddingBytes \n Out. Number of padding bytes for the
 * compressed data.
 */
extern RTIXCdrDllExport
void RTIXCdrEncapsulationOptions_unpack(
        RTIXCdrEncapsulationOptions options,
        RTIXCdrOctet *serializedDataNumPaddingBytes,
        RTIXCdrCompressionId *compressionId,
        RTIXCdrOctet *compressionNumPaddingBytes);

/**
 * @brief Packs the given parameters into the stream's options and sets 
 * the encapsulation options in the stream.
 *
 * @param me \n Out. The stream in which to set the encapsulation options.
 * @param serializedDataNumPaddingBytes \n In. Number of padding bytes for the
 * serialized data.
 * @param compressionId \n In. Compression plugin class ID.
 * @param compressionNumPaddingBytes \n In. Number of padding bytes for the
 * compressed data.
 */
extern RTIXCdrDllExport
void RTIXCdrStream_packEncapsulationOptions(
        struct RTIXCdrStream *me,
        RTIXCdrOctet serializedDataNumPaddingBytes,
        RTIXCdrCompressionId compressionId,
        RTIXCdrOctet compressionNumPaddingBytes);

/**
 * @brief Unpacks the encapsulation options from the stream into the given 
 * parameters.
 *
 * @param me \n In. The stream from which to get the encapsulation options.
 * @param serializedDataNumPaddingBytes \n Out. Number of padding bytes for the
 * serialized data.
 * @param compressionId \n Out. Compression plugin class ID.
 * @param compressionNumPaddingBytes \n Out. Number of padding bytes for the
 * compressed data.
 */
extern RTIXCdrDllExport
void RTIXCdrStream_unpackEncapsulationOptions(
        const struct RTIXCdrStream *me,
        RTIXCdrOctet *serializedDataNumPaddingBytes,
        RTIXCdrCompressionId *compressionId,
        RTIXCdrOctet *compressionNumPaddingBytes);

/*i
 * @brief Utility function to convert an encapsulationId into a string
 * representation
 */
extern RTIXCdrDllExport
const char * RTIXCdrEncapsulationId_toStr(RTIXCdrEncapsulationId encapsulationId);

/*
 * Primitive-type serialization
 */

extern RTIXCdrDllExport
void RTIXCdrStream_serializeWcharFast(
        struct RTIXCdrStream *me,
        const RTIXCdrWchar *in,
        RTIXCdrBoolean v1);

extern RTIXCdrDllExport
void RTIXCdrStream_deserializeWcharFast(
        struct RTIXCdrStream *me,
        const RTIXCdrWchar *in,
        RTIXCdrBoolean v1);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize1ByteFast(
        struct RTIXCdrStream *me,
        const RTIXCdr1Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserialize1ByteFast(
        struct RTIXCdrStream *me,
        RTIXCdr1Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize2ByteFast(
        struct RTIXCdrStream *me,
        const RTIXCdr2Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserialize2ByteFast(
        struct RTIXCdrStream *me,
        RTIXCdr2Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize4ByteFast(
        struct RTIXCdrStream *me,
        const RTIXCdr4Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserialize4ByteFast(
        struct RTIXCdrStream *me,
        RTIXCdr4Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize8ByteFast(
        struct RTIXCdrStream *me,
        const RTIXCdr8Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserialize8ByteFast(
        struct RTIXCdrStream *me,
        RTIXCdr8Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize16ByteFast(
        struct RTIXCdrStream *me,
        const RTIXCdr8Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserialize16ByteFast(
        struct RTIXCdrStream *me,
        RTIXCdr8Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserializeNByte(
        struct RTIXCdrStream *me,
        RTIXCdr1Byte *in,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong count);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeNByte(
        struct RTIXCdrStream *me,
        const RTIXCdr1Byte *in,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong count);

extern RTIXCdrDllExport   
RTIXCdrBoolean RTIXCdrStream_align(
        struct RTIXCdrStream *me,
        RTIXCdrAlignment alignment);

/**
 * @brief Aligns the stream to the submessage alignment and sets the
 * padding bits in the encapsulation options.
 *
 * @param me[in,out] The stream to align.
 * @param paddingBytes[out] The added padding bytes.
 * @param serializePadding[in] If RTI_XCDR_TRUE, the padding bytes are
 * serialized into the stream as part of the encapsulation options. 
 * If RTI_XCDR_FALSE, the padding bytes are not serialized into the stream.
 *
 * @return RTIXCdrBoolean RTI_XCDR_TRUE if success, RTI_XCDR_FALSE otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_finish(
        struct RTIXCdrStream *me,
        RTIXCdrOctet *paddingBytes,
        RTIXCdrBoolean serializePadding);

/**
 * @brief Helper function to check if the stream is aligned to the alignment
 * we provide.
 *
 * @param me
 * @param alignment
 * @return RTIXCdrBoolean true if it is aligned, false otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_isAligned(
        struct RTIXCdrStream *me,
        RTIXCdrAlignment alignment);

extern RTIXCdrDllExport
void RTIXCdrStream_serializeWchar(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean* result,
        const RTIXCdrWchar *in,
        RTIXCdrBoolean v1);

extern RTIXCdrDllExport
void RTIXCdrStream_deserializeWchar(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean* result,
        const RTIXCdrWchar *in,
        RTIXCdrBoolean v1);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serialize1Byte(
        struct RTIXCdrStream *me,
        const RTIXCdr1Byte *in);

extern RTIXCdrDllExport             
RTIXCdrBoolean RTIXCdrStream_serialize2Byte(
        struct RTIXCdrStream *me,
        const RTIXCdr2Byte *in,
        RTIXCdrBoolean align);

extern RTIXCdrDllExport             
RTIXCdrBoolean RTIXCdrStream_serialize4Byte(
        struct RTIXCdrStream *me,
        const RTIXCdr4Byte *in,
        RTIXCdrBoolean align);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrStream_serialize8Byte(
        struct RTIXCdrStream *me,
        const RTIXCdr8Byte *in,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrStream_serializePrimitiveArray(
        struct RTIXCdrStream *me,
        const void *in,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong elementSize,
        RTIXCdrUnsignedLong length);

extern RTIXCdrDllExport         
RTIXCdrBoolean RTIXCdrStream_serializePrimitiveSequence(
    struct RTIXCdrStream *me,
    const void *in,
    RTIXCdrUnsignedLong length,
    RTIXCdrUnsignedLong maximumLength,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize);
        
extern RTIXCdrDllExport   
RTIXCdrBoolean RTIXCdrStream_deserialize1Byte(
        struct RTIXCdrStream *me,
        RTIXCdr1Byte *in);

extern RTIXCdrDllExport         
RTIXCdrBoolean RTIXCdrStream_deserialize2Byte(
        struct RTIXCdrStream *me,
        RTIXCdr2Byte *in,
        RTIXCdrBoolean align);

extern RTIXCdrDllExport   
RTIXCdrBoolean RTIXCdrStream_deserialize4Byte(
        struct RTIXCdrStream *me,
        RTIXCdr4Byte *in,
        RTIXCdrBoolean align);

extern RTIXCdrDllExport   
RTIXCdrBoolean RTIXCdrStream_deserialize8Byte(
        struct RTIXCdrStream *me,
        RTIXCdr8Byte *in,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrStream_deserialize16Byte(
        struct RTIXCdrStream *me,
        RTIXCdr16Byte *in,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrStream_deserializePrimitiveArray(
        struct RTIXCdrStream *me,
        void *out,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong elementSize,
        RTIXCdrUnsignedLong length);

extern RTIXCdrDllExport        
RTIXCdrBoolean RTIXCdrStream_deserializePrimitiveSequence(
    	struct RTIXCdrStream *me,
    	void *out,
    	RTIXCdrUnsignedLong *length,
    	RTIXCdrUnsignedLong maximumLength,
    	RTIXCdrAlignment alignment,
    	RTIXCdrUnsignedLong elementSize);
   		 
extern RTIXCdrDllExport      		 
RTIXCdrBoolean RTIXCdrStream_skipPrimitiveArray(
    struct RTIXCdrStream *me,
    RTIXCdrBoolean align,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize,
    RTIXCdrUnsignedLong length);   		

extern RTIXCdrDllExport     
RTIXCdrBoolean RTIXCdrStream_skipPrimitiveSequence (
    struct RTIXCdrStream *me,
    RTIXCdrUnsignedLong *length,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize);
        
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_skipNByte(
        struct RTIXCdrStream *me,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong count);

/* Align and does not check size */
extern RTIXCdrDllExport
void RTIXCdrStream_alignFast(
        struct RTIXCdrStream *me,
        RTIXCdrAlignment alignment);

extern RTIXCdrDllExport
void RTIXCdrStream_skipNByteNoCheckSize(
        struct RTIXCdrStream *me,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong count);

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrStream_getRelativeCurrentPositionOffset(
        struct RTIXCdrStream *me);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeUnsignedShortToBigEndianFast(
        struct RTIXCdrStream *me,
        const RTIXCdr2Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserializeUnsignedShortFromBigEndianFast(
        struct RTIXCdrStream *me,
        RTIXCdr2Byte *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_skipWString(
    struct RTIXCdrStream *me);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_skipWStringArray(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong length);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_skipWStringSequence(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong *numberOfElements);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_serializeDHeaderLength(
        struct RTIXCdrStream *stream,
        char *dHeaderPosition);

extern RTIXCdrDllExport
char * RTIXCdrStream_serializeDHeader(
        struct RTIXCdrStream *stream);

/**
 * \brief Deserializes a DHEADER and returns its size.
 *
 * \param corruptedHeader \Out. When there is an error this variable is set
 * to RTI_XCDR_TRUE if the error is caused by an corrupted and invalid length 
 * in the DHEADER. The other reason why this method can fail is because we run 
 * out of space in the stream to deserialize the header. This last error maybe 
 * OK from a deserialization point of view because it may indicate that we just 
 * run out of space while deserializing a base type.
 * 
 * \return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE. If the reason
 * for failure is that the header length is invalid (e.g, it has been tampered)
 * invalidDHeader is set to RTI_XCDR_TRUE.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_deserializeDHeader(
        struct RTIXCdrStream *stream,
        RTIXCdrBoolean *corruptedHeader,
        RTIXCdrUnsignedLong *size,
        char **dheaderPosition,
        struct RTIXCdrStreamState *state);

/**
 * @brief Deserializes a DHEADER and skips the number of bytes stored in it.
 * 
 * \param corruptedHeader \Out. When there is an error this variable is set
 * to RTI_XCDR_TRUE if the error is caused by an corrupted and invalid length 
 * in the DHEADER. The other reason why this method can fail is because we run 
 * out of space in the stream to deserialize the header. This last error maybe 
 * OK from a skip point of view because it may indicate that we just 
 * run out of space while skipping a base type.
 *
 * \return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE. If the reason
 * for failure is that the header length is invalid (e.g, it has been tampered)
 * invalidDHeader is set to RTI_XCDR_TRUE.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_skipDHeader(
        struct RTIXCdrStream *stream,
        RTIXCdrBoolean *corruptedHeader);

/* \brief Serializes a V2 mutable member HEADER
 *
 * The function returns the position that will contain the member length when
 * LC is 4. Otherwise, the function returns NULL.
 *
 * The user has to call RTIXCdrStream_finishV2ParameterHeader only when the
 * returned position is different than NULL.
 */
extern RTIXCdrDllExport
char * RTIXCdrStream_serializeV2ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean *failure,
        RTIXCdrUnsignedLong parameterId,
        RTIXCdrBoolean mustUnderstand,
        RTIXCdrUnsignedLong LC);

extern RTIXCdrDllExport
void RTIXCdrStream_finishV2ParameterHeader(
    struct RTIXCdrStream *me,
    char *lengthPosition);

extern RTIXCdrDllExport
void RTIXCdrStream_popState(
        struct RTIXCdrStream *me,
        const struct RTIXCdrStreamState *state);

extern RTIXCdrDllExport
RTIXCdrEncapsulationId RTIXCdrEncapsulation_getAppendableCdrEncapsulationId(
        RTIXCdrEncapsulationId plainCdrEncapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrStream_findV2MutableSampleMember(
        struct RTIXCdrStream *stream,
        RTIXCdrUnsignedLong searchMemberId,
        RTIXCdrUnsignedLong *sizeOut);

#define RTI_XCDR_ENDIAN_BIG (0)
#define RTI_XCDR_ENDIAN_LITTLE (1)


#ifdef __cplusplus
    }   /* extern "C" */
#endif


#include "xcdr/xcdr_stream_impl.h"

#endif /* xcdr_stream_h */
