/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_stream.h"
#include "../infrastructure/Infrastructure.h"
#include "../typeCode/TypeCode.h"
#include "Stream.h"


/* **************************** Globals *********************************** */
const RTIXCdrUnsignedLong
        RTIXCdr_TCKind_g_primitiveSizes[RTI_XCDR_TK_NUM_PRIMITIVE_SIZES] = {
            0,                               /* TK_NULL */
            sizeof(RTIXCdrShort),            /* TK_SHORT */
            sizeof(RTIXCdrLong),             /* TK_LONG */
            sizeof(RTIXCdrUnsignedShort),    /* TK_USHORT */
            sizeof(RTIXCdrUnsignedLong),     /* TK_ULONG */
            sizeof(RTIXCdrFloat),            /* TK_FLOAT */
            sizeof(RTIXCdrDouble),           /* TK_DOUBLE */
            sizeof(RTIXCdrBoolean),          /* TK_BOOLEAN */
            sizeof(RTIXCdrChar),             /* TK_CHAR */
            sizeof(RTIXCdrOctet),            /* TK_OCTET */
            0,                               /* TK_STRUCT */
            0,                               /* TK_UNION */
            sizeof(RTIXCdrLong),             /* TK_ENUM */
            sizeof(RTIXCdrChar *),           /* TK_STRING */
            0,                               /* TK_SEQUENCE */
            0,                               /* TK_ARRAY */
            0,                               /* TK_ALIAS */
            sizeof(RTIXCdrLongLong),         /* TK_LONGLONG */
            sizeof(RTIXCdrUnsignedLongLong), /* TK_ULONGLONG */
            sizeof(RTIXCdrLongDouble),       /* TK_LONGDOUBLE */
            sizeof(RTIXCdrWchar),            /* TK_WCHAR */
            sizeof(RTIXCdrWchar *),          /* TK_WSTRING */
            0,                               /* TK_VALUE */
            sizeof(RTIXCdrInt8),             /* TK_INT8 */
            sizeof(RTIXCdrUInt8)             /* TK_UINT8 */
        };

const RTIXCdrUnsignedLong RTIXCdr_TCKind_g_primitiveCdrSizes
        [RTI_XCDR_TK_NUM_CDR_TYPES][RTI_XCDR_TK_NUM_PRIMITIVE_SIZES] = {
            { 0,                          /* TK_NULL */
              RTI_XCDR_TWO_BYTE_SIZE,     /* TK_SHORT */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_LONG */
              RTI_XCDR_TWO_BYTE_SIZE,     /* TK_USHORT */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_ULONG */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_FLOAT */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_DOUBLE */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_BOOLEAN */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_CHAR */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_OCTET */
              0,                          /* TK_STRUCT */
              0,                          /* TK_UNION */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_ENUM */
              0,                          /* TK_STRING */
              0,                          /* TK_SEQUENCE */
              0,                          /* TK_ARRAY */
              0,                          /* TK_ALIAS */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_LONGLONG */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_ULONGLONG */
              RTI_XCDR_SIXTEEN_BYTE_SIZE, /* TK_LONGDOUBLE */
              RTI_XCDR_LEGACY_WCHAR_SIZE, /* TK_WCHAR */
              0,                          /* TK_WSTRING */
              0,                          /* TK_VALUE */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_INT8 */
              RTI_XCDR_ONE_BYTE_SIZE },   /* TK_UINT8 */
            { 0,                          /* TK_NULL */
              RTI_XCDR_TWO_BYTE_SIZE,     /* TK_SHORT */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_LONG */
              RTI_XCDR_TWO_BYTE_SIZE,     /* TK_USHORT */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_ULONG */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_FLOAT */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_DOUBLE */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_BOOLEAN */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_CHAR */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_OCTET */
              0,                          /* TK_STRUCT */
              0,                          /* TK_UNION */
              RTI_XCDR_FOUR_BYTE_SIZE,    /* TK_ENUM */
              0,                          /* TK_STRING */
              0,                          /* TK_SEQUENCE */
              0,                          /* TK_ARRAY */
              0,                          /* TK_ALIAS */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_LONGLONG */
              RTI_XCDR_EIGHT_BYTE_SIZE,   /* TK_ULONGLONG */
              RTI_XCDR_SIXTEEN_BYTE_SIZE, /* TK_LONGDOUBLE */
              RTI_XCDR_WCHAR_SIZE,        /* TK_WCHAR */
              0,                          /* TK_WSTRING */
              0,                          /* TK_VALUE */
              RTI_XCDR_ONE_BYTE_SIZE,     /* TK_INT8 */
              RTI_XCDR_ONE_BYTE_SIZE }    /* TK_UINT8 */
        };

const RTIXCdrAlignment RTIXCdr_TCKind_g_primitiveCdrAlignments
        [RTI_XCDR_TK_NUM_CDR_TYPES][RTI_XCDR_TK_NUM_PRIMITIVE_SIZES] = {
            { 0,                                  /* TK_NULL */
              RTI_XCDR_TWO_BYTE_ALIGNMENT,        /* TK_SHORT */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_LONG */
              RTI_XCDR_TWO_BYTE_ALIGNMENT,        /* TK_USHORT */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_ULONG */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_FLOAT */
              RTI_XCDR_V1_EIGHT_BYTE_ALIGNMENT,   /* TK_DOUBLE */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_BOOLEAN */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_CHAR */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_OCTET */
              0,                                  /* TK_STRUCT */
              0,                                  /* TK_UNION */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_ENUM */
              0,                                  /* TK_STRING */
              0,                                  /* TK_SEQUENCE */
              0,                                  /* TK_ARRAY */
              0,                                  /* TK_ALIAS */
              RTI_XCDR_V1_EIGHT_BYTE_ALIGNMENT,   /* TK_LONGLONG */
              RTI_XCDR_V1_EIGHT_BYTE_ALIGNMENT,   /* TK_ULONGLONG */
              RTI_XCDR_V1_SIXTEEN_BYTE_ALIGNMENT, /* TK_LONGDOUBLE */
              RTI_XCDR_LEGACY_WCHAR_ALIGNMENT,    /* TK_WCHAR */
              0,                                  /* TK_WSTRING */
              0,                                  /* TK_VALUE */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_INT8 */
              RTI_XCDR_ONE_BYTE_ALIGNMENT },      /* TK_UINT8 */
            { 0,                                  /* TK_NULL */
              RTI_XCDR_TWO_BYTE_ALIGNMENT,        /* TK_SHORT */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_LONG */
              RTI_XCDR_TWO_BYTE_ALIGNMENT,        /* TK_USHORT */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_ULONG */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_FLOAT */
              RTI_XCDR_V2_EIGHT_BYTE_ALIGNMENT,   /* TK_DOUBLE */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_BOOLEAN */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_CHAR */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_OCTET */
              0,                                  /* TK_STRUCT */
              0,                                  /* TK_UNION */
              RTI_XCDR_FOUR_BYTE_ALIGNMENT,       /* TK_ENUM */
              0,                                  /* TK_STRING */
              0,                                  /* TK_SEQUENCE */
              0,                                  /* TK_ARRAY */
              0,                                  /* TK_ALIAS */
              RTI_XCDR_V2_EIGHT_BYTE_ALIGNMENT,   /* TK_LONGLONG */
              RTI_XCDR_V2_EIGHT_BYTE_ALIGNMENT,   /* TK_ULONGLONG */
              RTI_XCDR_V2_SIXTEEN_BYTE_ALIGNMENT, /* TK_LONGDOUBLE */
              RTI_XCDR_WCHAR_ALIGNMENT,           /* TK_WCHAR */
              0,                                  /* TK_WSTRING */
              0,                                  /* TK_VALUE */
              RTI_XCDR_ONE_BYTE_ALIGNMENT,        /* TK_INT8 */
              RTI_XCDR_ONE_BYTE_ALIGNMENT }       /* TK_UINT8 */
        };
/* ************************************************************************* */


RTIXCdrEncapsulationId RTIXCdrEncapsulationId_getNativePlainCdr2(void)
{
    return RTI_XCDR_ENCAPSULATION_ID_CDR2_NATIVE_ENDIAN;
}

RTIXCdrEncapsulationId RTIXCdrEncapsulationId_getNativePlCdr2(void)
{
    return RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_NATIVE_ENDIAN;
}

void RTIXCdrStream_init(struct RTIXCdrStream *me)
{
    RTIXCdrLog_testPrecondition(me == NULL, return);
#ifdef RTI_ENDIAN_LITTLE
    me->_nativeEndian = RTI_XCDR_ENDIAN_LITTLE;
#else
    me->_nativeEndian = RTI_XCDR_ENDIAN_BIG;
#endif

    me->_zeroOnAlign = RTI_FALSE;
}

void RTIXCdrStream_initWithBuffer(
        struct RTIXCdrStream *me,
        char *buffer,
        RTIXCdrUnsignedLong bufferLength)
{
    RTIXCdrLog_testPrecondition(me == NULL, return);
    RTIXCdrLog_testPrecondition(buffer == NULL, return);

    RTIXCdrStream_init(me);
    RTIXCdrStream_set(me, buffer, bufferLength);
}

RTIXCdrBoolean RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndianness(
        RTIXCdrStream *me,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrBoolean littleEndian)
{

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            !RTIXCdrEncapsulationId_isCdr(encapsulationId),
            return RTI_XCDR_FALSE);
    RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndiannessMacro(
            me,
            encapsulationId,
            littleEndian,
            return RTI_XCDR_FALSE);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_serializeAndSetCdrEncapsulationFromHeader(
        struct RTIXCdrStream *me,
        const char *encapsulationHeader)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            encapsulationHeader == NULL,
            return RTI_XCDR_FALSE);

    RTIXCdrStream_serializeAndSetCdrEncapsulationFromHeaderMacro(
            me,
            encapsulationHeader,
            return RTI_XCDR_FALSE);

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_deserializeAndSetCdrEncapsulation(
        RTIXCdrStream *me)
{

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrStream_deserializeAndSetCdrEncapsulationMacro(
            me,
            return RTI_XCDR_FALSE);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_serializeCdrEncapsulationDefault(
        RTIXCdrStream *me)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_checkSize(me, 4)) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrStream_serializeUnsignedShortToBigEndianFast(
            me, 
            &me->_encapsulationKind);

    RTIXCdrStream_serializeUnsignedShortToBigEndianFast(
            me,
            &me->_encapsulationOptions);

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_align(RTIXCdrStream *me, RTIXCdrAlignment align)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            !RTIXCdrAlignment_isValid(align),
            return RTI_XCDR_FALSE);

    RTIXCdrStream_alignMacro(me, align, return RTI_XCDR_FALSE);

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_align_ex(
        RTIXCdrStream *me, 
        RTIXCdrAlignment align, 
        RTIXCdrBoolean zeroOnAlign)
{
    RTIXCdrLong oldZeroOnAlign;
    RTIXCdrBoolean result;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    oldZeroOnAlign = me->_zeroOnAlign;
    me->_zeroOnAlign = (RTIXCdrLong)zeroOnAlign;
    result = RTIXCdrStream_align(me,align);
    me->_zeroOnAlign = oldZeroOnAlign;

    return result;
}

#define RTI_XCDR_SUBMESSAGE_HEADER_ALIGNMENT \
    RTI_XCDR_FOUR_BYTE_ALIGNMENT

RTIXCdrBoolean RTIXCdrStream_finish(
        struct RTIXCdrStream *me,
        RTIXCdrOctet *paddingBytes,
        RTIXCdrBoolean serializePadding)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrEncapsulationOptions options;
    RTIXCdrUnsignedLong beforePaddingOffset;
    RTIXCdrUnsignedLong afterPaddingOffset;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    beforePaddingOffset = RTIXCdrStream_getCurrentPositionOffset(me);

    if (!RTIXCdrStream_align_ex(
            me,
            RTI_XCDR_SUBMESSAGE_HEADER_ALIGNMENT,
            RTI_TRUE)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_s,
                "aligning to a 4-byte boundary");
        goto done;
    }
    
    afterPaddingOffset = RTIXCdrStream_getCurrentPositionOffset(me);

    *paddingBytes = (RTIXCdrOctet) (afterPaddingOffset - beforePaddingOffset);

    RTIXCdrEncapsulationOptions_pack(
            &options,
            serializePadding ? *paddingBytes : 0,
            RTI_XCDR_COMPRESSION_CLASS_ID_NONE,
            0 /* compressionNumPaddingBytes */);
    RTIXCdrStream_setEncapsulationOptions(me, options);

    RTIXCdrStream_setCurrentPositionOffset(
            me,
            RTI_XCDR_ENCAPSULATION_ID_SIZE);

    RTIXCdrStream_serialize2ByteBigEndianFast(me, &options);

    RTIXCdrStream_setCurrentPositionOffset(
            me,
            afterPaddingOffset);

    result = RTI_XCDR_TRUE;
done:

    return result;
}

#define RTIXCdrParameterId_stripOutV1Flags(pId) \
    (*(pId)) = ((*(pId)) & 0x3FFF)

#define RTIXCdrLongParameterId_stripOutV1Flags(pId) \
    (*(pId)) = ((*(pId)) & 0x0FFFFFFF)

/*
 * Deserializes a XCDR V2 parameter header (EMHEADER)
 * 
 * @param state (optional) where to save this stream's current state.
 * @param parameterId (required) Will contain the member id
 * @param length (required) Will contain the member length (excluding the header)
 * @param mustUnderstand Will contain the must-understand flag
 * 
 * @post me is located after the parameter (which can be 1 or 2 bytes), 
 * at the beginning of the member; if this state is not null, me's length
 * is set to *length.
 */
RTIXCdrBoolean RTIXCdrStream_deserializeV2ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrUnsignedLong *parameterId,
        RTIXCdrUnsignedLong *length,
        RTIXCdrBoolean *mustUnderstand)
{
    RTIXCdrUnsignedLong headerVal;
    RTIXCdrUnsignedLong lcVal;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(parameterId == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(length == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(mustUnderstand == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_deserialize4Byte(me, &headerVal, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    lcVal = (headerVal & 0x70000000) >> 28;
    *mustUnderstand = (RTIXCdrBoolean)((headerVal & 0x80000000)?
            RTI_XCDR_TRUE:
            RTI_XCDR_FALSE);
    *parameterId = headerVal & 0x0FFFFFFF;

    switch (lcVal) {
        case 0:
            /* LC = 0 = 0b000 indicates serialized member length is 1 Byte */
            *length = RTI_XCDR_ONE_BYTE_SIZE;
            break;
        case 1:
            /* LC = 1 = 0b001 indicates serialized member length is 2 Bytes */
            *length = RTI_XCDR_TWO_BYTE_SIZE;
            break;
        case 2:
            /* LC = 2 = 0b010 indicates serialized member length is 4 Bytes */
            *length = RTI_XCDR_FOUR_BYTE_SIZE;
            break;
        case 3:
            /* LC = 3 = 0b011 indicates serialized member length is 8 Bytes */
            *length = RTI_XCDR_EIGHT_BYTE_SIZE;
            break;
        default:
            {
                char *position = RTIXCdrStream_getCurrentPosition(me);
                if (!RTIXCdrStream_deserialize4Byte(me, length, RTI_XCDR_FALSE)) {
                    return RTI_XCDR_FALSE;
                }

                if (lcVal == 5) {
                    /* LC = 5 = 0b101 indicates serialized member length is also NEXTINT */
                    RTIXCdrStream_setCurrentPosition(me, position);
                    *length += RTI_XCDR_FOUR_BYTE_SIZE; /* For the integer encoding string/sequence length */
                } else if (lcVal == 6) {
                    /* LC = 6 = 0b110 indicates serialized member length is 4*NEXTINT */
                    *length <<= 2;
                    *length += RTI_XCDR_FOUR_BYTE_SIZE; /* For the integer encoding string/sequence length */
                    RTIXCdrStream_setCurrentPosition(me, position);
                } else if (lcVal == 7) {
                    /* LC = 7 = 0b111 indicates serialized member length is 8*NEXTINT */
                    *length <<= 3;
                    *length += RTI_XCDR_FOUR_BYTE_SIZE; /* For the integer encoding string/sequence length */
                    RTIXCdrStream_setCurrentPosition(me, position);
                }
            }
            break;
    }

    if (state != NULL) {
        if (!RTIXCdrStream_checkSize(me, *length)) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrStream_pushState(
                me,
                state,
                RTI_XCDR_TRUE,
                *length);
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_deserializeV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrUnsignedLong *parameterId,
        RTIXCdrUnsignedLong *length,
        RTIXCdrBoolean *extended,
        RTIXCdrBoolean *mustUnderstand)
{
    RTIXCdrUnsignedShort sTmp = 0;
    
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(state == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(parameterId == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(length == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(extended == NULL, return RTI_XCDR_FALSE);
    
    if (!RTIXCdrStream_align(me, RTI_XCDR_PARAMETER_HEADER_ALIGNMENT)) {
        return RTI_XCDR_FALSE;
    }
    
    if (!RTIXCdrStream_checkSize(
            me, 
            RTI_XCDR_V1_PARAMETER_SHORT_HEADER_SIZE)) {
        return RTI_XCDR_FALSE;
    }
    
    RTIXCdrStream_deserialize2ByteFast(me, &sTmp);

    if (mustUnderstand != NULL) {
        *mustUnderstand = (sTmp & RTI_XCDR_V1_PARAMETER_FLAG_MUST_UNDERSTAND)?
                RTI_XCDR_TRUE:RTI_XCDR_FALSE;
    }

    RTIXCdrParameterId_stripOutV1Flags(&sTmp);
    *parameterId = sTmp;
    
    RTIXCdrStream_deserialize2ByteFast(me, &sTmp);
    *length = sTmp;
    *extended = RTI_FALSE;

    if (*parameterId == RTI_XCDR_V1_PID_EXTENDED) {
        *extended = RTI_XCDR_TRUE;

        if (!RTIXCdrStream_checkSize(
                me, 
                RTI_XCDR_V1_PARAMETER_LONG_HEADER_SIZE - 
                RTI_XCDR_V1_PARAMETER_SHORT_HEADER_SIZE)) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrStream_deserialize4ByteFast(me, parameterId);
        
        if (mustUnderstand != NULL) {
            *mustUnderstand = ((*parameterId) & RTI_XCDR_V1_LONG_PARAMETER_FLAG_MUST_UNDERSTAND)?
                    RTI_XCDR_TRUE:RTI_XCDR_FALSE;
        }
        
        RTIXCdrLongParameterId_stripOutV1Flags(parameterId);
        
        RTIXCdrStream_deserialize4ByteFast(me, length);
        
        if (sTmp > (2*RTI_XCDR_UNSIGNED_LONG_SIZE)) {
            sTmp = (RTIXCdrUnsignedShort)(sTmp - (2 * RTI_XCDR_UNSIGNED_LONG_SIZE));
            if (!RTIXCdrStream_skipNByte(
                    me, 
                    RTI_XCDR_ONE_BYTE_ALIGNMENT, 
                    sTmp)) {
                return RTI_XCDR_FALSE;
            }
        }
    }

    if (*length > RTIXCdrLong_MAX
            || *length > RTIXCdrStream_getRemainder(me)) {
        /* CORE-7926: The length of the parameter cannot be greater than the
         * bytes left on the CDR stream. Also we are limited to the maximum
         * range of an int (RTIXCdrLong_MAX)
         */
        return RTI_XCDR_FALSE;
    }
    
    RTIXCdrStream_pushState(me, state, RTI_XCDR_TRUE, *length);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_deserializeAndSkipV1ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean *isSentinel)
{
    struct RTIXCdrStreamState memberStreamState;
    RTIXCdrUnsignedLong memberLength;
    RTIXCdrUnsignedLong parameterId;
    RTIXCdrBoolean extended;
    
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    
    if (!RTIXCdrStream_deserializeV1ParameterHeader(
            me,
            &memberStreamState,
            &parameterId,
            &memberLength,
            &extended,
            NULL)) {
        return RTI_XCDR_FALSE;
    }

    if (isSentinel != NULL) {
        if (parameterId == RTI_XCDR_V1_PID_LIST_END) {
            *isSentinel = RTI_XCDR_TRUE;
        } else {
            *isSentinel = RTI_XCDR_FALSE;
        }
    }

    RTIXCdrStream_moveToNextParameterHeader(
            me, 
            &memberStreamState,
            memberLength);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipV2ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong LC)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_skipNByte(
            me,
            RTI_XCDR_PARAMETER_HEADER_ALIGNMENT,
            RTI_XCDR_V2_PARAMETER_HEADER_SIZE)) {
        return RTI_XCDR_FALSE;
    }

    if (LC == 4) {
        /* Length */
        if (!RTIXCdrStream_skipNByte(
                me,
                RTI_XCDR_FOUR_BYTE_SIZE,
                RTI_XCDR_FOUR_BYTE_SIZE)) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrBoolean extended)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_skipNByte(
            me, 
            RTI_XCDR_PARAMETER_HEADER_ALIGNMENT,
            extended?
                    RTI_XCDR_V1_PARAMETER_LONG_HEADER_SIZE:
                    RTI_XCDR_V1_PARAMETER_SHORT_HEADER_SIZE)) {
        return RTI_XCDR_FALSE;
    }
    
    if (state != NULL) {
        RTIXCdrStream_pushState(
                me,
                state,
                RTI_XCDR_FALSE,
                0);
    } else {
        RTIXCdrUtility_unusedReturnValue(
                RTIXCdrStream_resetAlignment(me),
                char *);
    }
    
    return RTI_XCDR_TRUE;
}

char * RTIXCdrStream_serializeV2ParameterHeader(
        struct RTIXCdrStream *me,
        RTIXCdrBoolean *failure,
        RTIXCdrUnsignedLong parameterId,
        RTIXCdrBoolean mustUnderstand,
        RTIXCdrUnsignedLong LC)
{
    RTIXCdrUnsignedLong headerVal = parameterId;
    char *lengthPosition = NULL;
    RTIXCdrLog_testPrecondition(me == NULL, return NULL);
    RTIXCdrLog_testPrecondition(failure == NULL, return NULL);

    *failure = RTI_XCDR_TRUE;

    if (!RTIXCdrStream_align(me, RTI_XCDR_PARAMETER_HEADER_ALIGNMENT)) {
        return NULL;
    }

    if (mustUnderstand) {
        headerVal |= 0x80000000;
    }

    headerVal |= (LC <<28);
    if (!RTIXCdrStream_serialize4Byte(me, &headerVal, RTI_XCDR_FALSE)) {
        return NULL;
    }

    if (LC == 4) {
        RTIXCdrLong length = 0;
        lengthPosition = RTIXCdrStream_getCurrentPosition(me);
        if (!RTIXCdrStream_serialize4Byte(me, &length, RTI_XCDR_FALSE)) {
            return NULL;
        }
    }

    *failure = RTI_XCDR_FALSE;
    return lengthPosition;
}

char * RTIXCdrStream_serializeV1ParameterHeader(
        struct RTIXCdrStream *me,
        struct RTIXCdrStreamState *state,
        RTIXCdrBoolean extended,
        RTIXCdrUnsignedLong parameterId,
        RTIXCdrBoolean mustUnderstand)
{
    RTIXCdrUnsignedLong length = 0;
    RTIXCdrUnsignedShort sParameterId, sLength;   
    char *position = NULL;

    RTIXCdrLog_testPrecondition(me == NULL, return NULL);

    if (!RTIXCdrStream_align(me, RTI_XCDR_PARAMETER_HEADER_ALIGNMENT)) {
        return NULL;
    }
    
    if (extended) {
        if (!RTIXCdrStream_checkSize(
                me, 
                RTI_XCDR_V1_PARAMETER_LONG_HEADER_SIZE)) {
            return NULL;
        }

        sParameterId = (RTI_XCDR_V1_PID_EXTENDED | 
                RTI_XCDR_V1_PARAMETER_FLAG_MUST_UNDERSTAND);
        sLength = 8;

        if (mustUnderstand) {
            parameterId |= RTI_XCDR_V1_LONG_PARAMETER_FLAG_MUST_UNDERSTAND;   
        }
    } else {
        if (!RTIXCdrStream_checkSize(
                me, 
                RTI_XCDR_V1_PARAMETER_SHORT_HEADER_SIZE)) {
            return NULL;
        }

        sParameterId = (RTIXCdrUnsignedShort) parameterId;
        sLength = 0;

        if (mustUnderstand) {
            sParameterId |= RTI_XCDR_V1_PARAMETER_FLAG_MUST_UNDERSTAND;
        }
    }
    
    RTIXCdrStream_serialize2ByteFast(me, &sParameterId);

    if (!extended) {
        position = RTIXCdrStream_getCurrentPosition(me);
    }

    RTIXCdrStream_serialize2ByteFast(me, &sLength);
        
    if (extended) {
        RTIXCdrStream_serialize4ByteFast(me, &parameterId);
        position = RTIXCdrStream_getCurrentPosition(me); 
        RTIXCdrStream_serialize4ByteFast(me, &length);
    }
    
    if (state != NULL) {
        RTIXCdrStream_pushState(
                me,
                state,
                RTI_XCDR_FALSE,
                0);
    } else {
        RTIXCdrUtility_unusedReturnValue(
                RTIXCdrStream_resetAlignment(me),
                char *);
    }
    return position;
}

RTIXCdrBoolean RTIXCdrStream_finishV1ParameterHeader(
    struct RTIXCdrStream *me,
    const struct RTIXCdrStreamState *state,
    RTIXCdrBoolean extended,
    RTIXCdrBoolean addPaddingToParameterLenght,
    char *lengthPosition)
{
    char *position;
    RTIXCdrUnsignedLong memberLength;
    RTIXCdrUnsignedShort sMemberLength;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(state == NULL, return RTI_XCDR_FALSE);

    if (addPaddingToParameterLenght) {
        if (!RTIXCdrStream_align_ex(
                me,
                RTI_XCDR_PARAMETER_HEADER_ALIGNMENT,
                lengthPosition?RTI_XCDR_TRUE:RTI_XCDR_FALSE)) {
            return RTI_XCDR_FALSE;
        }
    }

    if (lengthPosition != NULL) {
        position = RTIXCdrStream_getCurrentPosition(me);

        if (extended) {
            memberLength = (RTIXCdrUnsignedLong)(position -
                    (lengthPosition +  RTI_XCDR_UNSIGNED_LONG_SIZE));
            RTIXCdrStream_setCurrentPosition(me, lengthPosition);
            RTIXCdrStream_serialize4ByteFast( 
                    me, 
                    &memberLength);
        } else {
            sMemberLength = (RTIXCdrUnsignedShort)(position - 
                    (lengthPosition +  RTI_XCDR_UNSIGNED_SHORT_SIZE));
            RTIXCdrStream_setCurrentPosition(me, lengthPosition);
            RTIXCdrStream_serialize2ByteFast( 
                     me, 
                     &sMemberLength);
        }
        
        RTIXCdrStream_setCurrentPosition(me, position);
    }
    
    /* We have to preserve the current alignment */
    RTIXCdrStream_popStateNoAlign(me, state);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_serializeDHeaderLength(
        RTIXCdrStream *stream,
        char *dHeaderPosition)
{
    RTIXCdrUnsignedLongLong ullSize;
    RTIXCdrUnsignedLong lSize;
    char *currentPosition;

    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(dHeaderPosition == NULL, return RTI_XCDR_FALSE);

    currentPosition = RTIXCdrStream_getCurrentPosition(stream);
    ullSize = (RTIXCdrUnsignedLongLong)(currentPosition - dHeaderPosition) -
            RTI_XCDR_DHEADER_SIZE;

    if (ullSize > 0xFFFFFFFF) {
        return RTI_XCDR_FALSE;
    }

    lSize = (RTIXCdrUnsignedLong)ullSize;
    RTIXCdrStream_setCurrentPosition(stream, dHeaderPosition);
    RTIXCdrStream_serialize4ByteFast(stream, &lSize);
    RTIXCdrStream_setCurrentPosition(stream, currentPosition);
    return RTI_XCDR_TRUE;
}

char * RTIXCdrStream_serializeDHeader(RTIXCdrStream *stream) {
    RTIXCdrUnsignedLong size = 0;
    char * dHeaderPosition;

    RTIXCdrLog_testPrecondition(stream == NULL, return NULL);

    /* We do not use RTIXCdrStream_serialize4Byte because we want to return
     * a dHeaderPosition that is align so that the storage of the length
     * at the end of the type serialization is faster
     */
    if (!RTIXCdrStream_align(stream, RTI_XCDR_DHEADER_ALIGNMENT)) {
        return NULL;
    }

    if (!RTIXCdrStream_checkSize(stream, RTI_XCDR_DHEADER_SIZE)) {
        return NULL;
    }

    dHeaderPosition = RTIXCdrStream_getCurrentPosition(stream);
    RTIXCdrStream_serialize4ByteFast(stream, &size);
    return dHeaderPosition;
}

RTIXCdrBoolean RTIXCdrStream_deserializeDHeader(
        RTIXCdrStream *stream,
        RTIXCdrBoolean *corruptedHeader,
        RTIXCdrUnsignedLong *size,
        char **dheaderPosition,
        struct RTIXCdrStreamState *state)
{
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(size == NULL, return RTI_XCDR_FALSE);

    if (corruptedHeader != NULL) {
        *corruptedHeader = RTI_XCDR_FALSE;
    }

    if (!RTIXCdrStream_deserialize4Byte(stream, size, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    if (dheaderPosition != NULL) {
        *dheaderPosition = RTIXCdrStream_getCurrentPosition(stream);
    }

    if (state != NULL) {
        /* 
         * The size the DHeader indicates cannot be larger than the remaining
         * stream size 
         */
        if (!RTIXCdrStream_checkSize(stream, *size)) {
            if (corruptedHeader != NULL) {
                *corruptedHeader = RTI_XCDR_TRUE;
            }
            if (dheaderPosition != NULL) {
                *dheaderPosition = NULL;
            }
            return RTI_XCDR_FALSE;
        }

        RTIXCdrStream_pushState(
                stream,
                state,
                RTI_XCDR_TRUE,
                *size);
    }
    
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipDHeader(
        struct RTIXCdrStream *stream,
        RTIXCdrBoolean *corruptedHeader)
{
    RTIXCdrUnsignedLong dheaderSize;

    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);

    if (corruptedHeader != NULL) {
        *corruptedHeader = RTI_XCDR_FALSE;
    }

    if (!RTIXCdrStream_deserializeDHeader(
            stream,
            corruptedHeader,
            &dheaderSize,
            NULL,
            NULL)) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrStream_increaseCurrentPosition(stream, dheaderSize);
    return RTI_XCDR_TRUE;
}

RTIXCdrEncapsulationId RTIXCdrEncapsulation_getEncapsulationId(
        RTIXCdrBoolean littleEndian,
        RTIXCdrBoolean v2,
        RTIXCdrExtensibilityKind extKind)
{
    RTIXCdrEncapsulationId encapsulationId;
    static RTIXCdrEncapsulationId mutableEncapsulationId[2][2] =
    {
            {
                RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE,
                RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE
            },
            {
                RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE,
                RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE
            }
    };
    static RTIXCdrEncapsulationId appendEncapsulationId[2][2] =
    {
            {
                RTI_XCDR_ENCAPSULATION_ID_CDR_BE,
                RTI_XCDR_ENCAPSULATION_ID_CDR_LE
            },
            {
                RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE,
                RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE
            }
    };
    static RTIXCdrEncapsulationId finalEncapsulationId[2][2] =
    {
            {
                RTI_XCDR_ENCAPSULATION_ID_CDR_BE,
                RTI_XCDR_ENCAPSULATION_ID_CDR_LE
            },
            {
                RTI_XCDR_ENCAPSULATION_ID_CDR2_BE,
                RTI_XCDR_ENCAPSULATION_ID_CDR2_LE
            }
    };


    if (extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
        encapsulationId = mutableEncapsulationId[v2][littleEndian];
    } else if (extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY) {
        encapsulationId = appendEncapsulationId[v2][littleEndian];
    } else {
        encapsulationId = finalEncapsulationId[v2][littleEndian];
    }

    return encapsulationId;
}

RTIXCdrBoolean RTIXCdrArraySizeIterator_next(
        struct RTIXCdrArraySizeIterator *me,
        RTIXCdrStream *stream,
        RTIXCdrBoolean *endIt,
        RTIXCdrUnsignedLong maxElCount)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(endIt == NULL, return RTI_XCDR_FALSE);

    *endIt = RTI_XCDR_FALSE;

    if (!me->leftover) {
        RTIXCdrAlignment maxAlignment;
        RTIXCdrAlignment alignment;

        maxAlignment =
                RTIXCdrEncapsulationId_isCdrV2(
                        stream->_encapsulationKind)?4:8;
        alignment = RTIXCdrStream_getCurrentAlignment(
                stream,
                maxAlignment);

        if (me->isStarted) {
            me->currentElIndex++;
        }

        if (me->align[alignment] < 0) {
            me->align[alignment] = (RTIXCdrLong)me->currentElIndex;
            me->sizeAlign[alignment] =
                    RTIXCdrStream_getCurrentPositionOffset(stream);
        } else {
            RTIXCdrUnsignedLong samplesInLoop =
                    me->currentElIndex - (RTIXCdrUnsignedLong)me->align[alignment];
            RTIXCdrUnsignedLong loopSize =
                    RTIXCdrStream_getCurrentPositionOffset(stream) -
                    me->sizeAlign[alignment];
            RTIXCdrUnsignedLong loopCount =
                    (maxElCount - me->currentElIndex)/samplesInLoop;
            RTIXCdrUnsignedLongLong bytesToSkip =
                    (RTIXCdrUnsignedLongLong) loopSize
                    * (RTIXCdrUnsignedLongLong) loopCount;

            if (bytesToSkip > RTIXCdrUnsignedLong_MAX) {
                return RTI_XCDR_FALSE;
            }
            if (!RTIXCdrStream_skipNByte(
                        stream,
                        RTI_XCDR_ONE_BYTE_ALIGNMENT,
                        (RTIXCdrUnsignedLong) bytesToSkip)) {
                return RTI_XCDR_FALSE;
            }

            me->currentElIndex += (samplesInLoop*loopCount);
            me->leftover = RTI_XCDR_TRUE;
        }
    } else {
        me->currentElIndex++;
    }

    me->isStarted = RTI_XCDR_TRUE;

    if (me->currentElIndex >= maxElCount) {
        *endIt = RTI_XCDR_TRUE;
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipWString(
        struct RTIXCdrStream *me)
{
    RTIXCdrUnsignedLong serializedLength;
    RTIXCdrUnsignedLong numBytes;

    RTIXCdrLog_testPrecondition(
            me == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            me->_currentPosition == NULL,
            return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_deserialize4Byte(me, &serializedLength, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    if (serializedLength == 0) {
        return RTI_XCDR_TRUE;
    }

    if (!RTIXCdrEncapsulationId_isCdrV2(me->_encapsulationKind)) {
        numBytes = RTI_XCDR_LEGACY_WCHAR_SIZE* serializedLength;
    } else {
        /* In v2 the serializedLength is in bytes*/
        numBytes = serializedLength;
    }

    if (!RTIXCdrStream_checkSize(me, numBytes)) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrStream_skipNByteFast(me, numBytes);

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipWStringArray(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong length)
{
    RTIXCdrUnsignedLong i;

    RTIXCdrLog_testPrecondition(
            me == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            me->_currentPosition == NULL,
            return RTI_XCDR_FALSE);

    for (i = 0; i < length; i++) {
        if (!RTIXCdrStream_skipWString(me)) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_skipWStringSequence(
        struct RTIXCdrStream *me,
        RTIXCdrUnsignedLong *numberOfElements)
{
    RTIXCdrUnsignedLong serializedLength;

    RTIXCdrLog_testPrecondition(
            me == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            me->_currentPosition == NULL,
            return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_deserialize4Byte(me, &serializedLength, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    /* numberOfElements can be null */
    if (numberOfElements != NULL) {
      *numberOfElements = serializedLength;
    }

    if (serializedLength == 0) {
        return RTI_XCDR_TRUE;
    }

    return RTIXCdrStream_skipWStringArray(me, serializedLength);
}

/*
 * Finds a member moving the stream to the position where it begins
 * 
 * @param stream The stream, located exactly at the DHeader of a mutable sample
 * @param searchMemberId The member id to find
 * @param sizeOut The size of the member (zero if not found)
 * 
 * @return True if found, false if the member id wasn't found after skipping
 * every member within the bounds that the DHeader indicates
 */
RTIXCdrBoolean RTIXCdrStream_findV2MutableSampleMember(
        struct RTIXCdrStream *stream,
        RTIXCdrUnsignedLong searchMemberId,
        RTIXCdrUnsignedLong *sizeOut)
{
    RTIXCdrUnsignedLong memberId, memberSize, typeSize;
    struct RTIXCdrStreamState state;
    RTIXCdrBoolean mustUnderstand;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(sizeOut == NULL, return RTI_XCDR_FALSE);
    *sizeOut = 0;
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);

    /* This also sets the stream length to this type's length */
    if (!RTIXCdrStream_deserializeDHeader(
            stream,
            NULL, /* invalidDHeader */
            &typeSize,
            NULL,
            &state)) {
        return RTI_XCDR_FALSE;
    }

    if (typeSize == 0) {
        goto done;
    }

    if (!RTIXCdrStream_deserializeV2ParameterHeader(
            stream, 
            NULL, 
            &memberId, 
            &memberSize, 
            &mustUnderstand)) {
        goto done;
    }
    while (memberId != searchMemberId) {
        if (!RTIXCdrStream_checkSize(stream, memberSize)) {
            goto done;
        }
        
        RTIXCdrStream_skipNByteFast(stream, memberSize); /* No need to align */

        if (!RTIXCdrStream_deserializeV2ParameterHeader(
                stream, 
                NULL, 
                &memberId, 
                &memberSize, 
                &mustUnderstand)) {
            goto done;
        }        
    }

    if (!RTIXCdrStream_checkSize(stream, memberSize)) {
        goto done;
    }

    *sizeOut = memberSize;

    ok = RTI_XCDR_TRUE;
done:
    RTIXCdrStream_popState(stream, &state);
    return ok;

}

#define RTIXCdrStream_serializePrimitiveArrayMacro(\
        me,\
        in, \
        align,\
        alignment,\
        elementSize,\
        length,\
        actionIfError) \
{ \
    RTIXCdrUnsignedLong i; \
    RTIXCdr2Byte *in2 = NULL; \
    RTIXCdr4Byte *in4 = NULL; \
    RTIXCdr8Byte *in8 = NULL; \
    RTIXCdr16Byte *in16 = NULL; \
  \
    if (align) { \
        if (!RTIXCdrStream_align(me, alignment)) { \
            actionIfError; \
        } \
    } \
    if (!RTIXCdrStream_checkSize(me, \
            (elementSize * length))) { \
        actionIfError; \
    } \
\
    if (me->_needByteSwap && elementSize != RTI_XCDR_ONE_BYTE_SIZE) { \
        switch (elementSize) { \
          case RTI_XCDR_TWO_BYTE_SIZE: \
            in2 = (RTIXCdr2Byte *)in; \
            for (i = 0; i < length; ++i) { \
                RTIXCdrStream_serialize2ByteFast(me, in2); \
                ++in2; \
            } \
            break; \
          case RTI_XCDR_FOUR_BYTE_SIZE:\
            in4 = (RTIXCdr4Byte *)in; \
            for (i = 0; i < length; ++i) {\
                RTIXCdrStream_serialize4ByteFast(me, in4); \
                ++in4; \
            }\
            break; \
          case RTI_XCDR_EIGHT_BYTE_SIZE:\
            in8 = (RTIXCdr8Byte *)in; \
            for (i = 0; i < length; ++i) {\
                RTIXCdrStream_serialize8ByteFast(me, in8); \
                ++in8; \
            }\
            break; \
          case RTI_XCDR_SIXTEEN_BYTE_SIZE:\
            in16 = (RTIXCdr16Byte *)in; \
            for (i = 0; i < length; ++i) {\
                RTIXCdrStream_serialize16ByteFast(me, in16); \
                ++in16; \
            }\
            break; \
            default:\
            actionIfError; /* otherwise, ERROR */ \
        }       \
    } else {\
        RTIXCdrMemory_copy(me->_currentPosition, in, elementSize * length); \
        me->_currentPosition += elementSize * length; \
    } \
}


RTIXCdrBoolean RTIXCdrStream_serializePrimitiveArray(
        struct RTIXCdrStream *me,
        const void *in,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong elementSize,
        RTIXCdrUnsignedLong length){
    RTIXCdrLog_testPrecondition(
            (me == NULL) || (me->_currentPosition == NULL) || (in == NULL),
            return RTI_FALSE);

    RTIXCdrStream_serializePrimitiveArrayMacro(
            me,
            in,
            align,
            alignment,
            elementSize,
            length,
            return RTI_XCDR_FALSE);
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_serializePrimitiveSequence(
    struct RTIXCdrStream *me,
    const void *in,
    RTIXCdrUnsignedLong length,
    RTIXCdrUnsignedLong maximumLength,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize)
{

    RTIXCdrLog_testPrecondition(
        (me == NULL) || (me->_currentPosition == NULL) ||
        ((in == NULL) && (length > 0)),
        return RTI_FALSE);

    if (length > maximumLength) {
        RTIXCdrLog_logTwoLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_dd,
                (RTIXCdrLong) length,
                (RTIXCdrLong) maximumLength);
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrStream_serialize4Byte(me, &length, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    if (length == 0) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrStream_serializePrimitiveArrayMacro(
             me,
             in,
             (alignment > 4) ? RTI_XCDR_TRUE:RTI_XCDR_FALSE,
             alignment,
             elementSize,
             length,
             return RTI_XCDR_FALSE);
     return RTI_XCDR_TRUE;

}


/*
*/
#define RTIXCdrStream_deserializePrimitiveArrayMacro( \
        me,\
        in, \
        align,\
        alignment,\
        elementSize,\
        length,\
        actionIfError) \
                { \
    RTIXCdrUnsignedLong i; \
    RTIXCdr2Byte *out2 = NULL; \
    RTIXCdr4Byte *out4 = NULL; \
    RTIXCdr8Byte *out8 = NULL; \
    RTIXCdr16Byte *out16 = NULL; \
    \
        if (align) { \
            if (!RTIXCdrStream_align(me, alignment)) { \
                actionIfError; \
            } \
        } \
        if (!RTIXCdrStream_checkSize(me, \
                ((RTIXCdrUnsignedLong)(elementSize * length)))) { \
            actionIfError; \
        } \
        \
        if (me->_needByteSwap && elementSize != RTI_XCDR_ONE_BYTE_SIZE) { \
            switch (elementSize) { \
            case RTI_XCDR_TWO_BYTE_SIZE: \
            out2 = (RTIXCdr2Byte *)out; \
            for (i = 0; i < (RTIXCdrUnsignedLong)length; ++i) { \
                RTIXCdrStream_deserialize2ByteFast(me, out2); \
                ++out2; \
            } \
            break; \
            case RTI_XCDR_FOUR_BYTE_SIZE: \
            out4 = (RTIXCdr4Byte *)out; \
            for (i = 0; i < (RTIXCdrUnsignedLong)length; ++i) { \
                RTIXCdrStream_deserialize4ByteFast(me, out4); \
                ++out4; \
            } \
            break; \
            case RTI_XCDR_EIGHT_BYTE_SIZE: \
            out8 = (RTIXCdr8Byte *)out; \
            for (i = 0; i < (RTIXCdrUnsignedLong)length; ++i) { \
                RTIXCdrStream_deserialize8ByteFast(me, out8); \
                ++out8; \
            } \
            break; \
            case RTI_XCDR_SIXTEEN_BYTE_SIZE: \
            out16 = (RTIXCdr16Byte *)out; \
            for (i = 0; i < (RTIXCdrUnsignedLong)length; ++i) { \
                RTIXCdrStream_deserialize16ByteFast(me, out16); \
                ++out16; \
            } \
            break; \
            \
            default: \
            actionIfError; /* otherwise, ERROR */ \
        } \
        } else { \
            RTIXCdrMemory_copy(out, me->_currentPosition, elementSize*length); \
            me->_currentPosition += elementSize * length; \
        } \
        \
}

/*
 */
RTIXCdrBoolean RTIXCdrStream_deserializePrimitiveArray(
        struct RTIXCdrStream *me,
        void *out,
        RTIXCdrBoolean align,
        RTIXCdrAlignment alignment,
        RTIXCdrUnsignedLong elementSize,
        RTIXCdrUnsignedLong length)
{
    RTIXCdrLog_testPrecondition(
        (me == NULL) || (me->_currentPosition == NULL) || (out == NULL),
        return RTI_FALSE);

    RTIXCdrStream_deserializePrimitiveArrayMacro(
            me,
            out,
            align,
            alignment,
            elementSize,
            length,
            return RTI_XCDR_FALSE);

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrStream_deserializePrimitiveSequence(
    struct RTIXCdrStream *me,
    void *out,
    RTIXCdrUnsignedLong *length,
    RTIXCdrUnsignedLong maximumLength,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize)
{
    RTIXCdrBoolean retVal = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong serializedLength;

    RTIXCdrLog_testPrecondition(me == NULL, goto done);
    RTIXCdrLog_testPrecondition(me->_currentPosition == NULL, goto done);

    if (!RTIXCdrStream_deserialize4Byte(me, &serializedLength, RTI_XCDR_TRUE)) {
        goto done;
    }

    if (serializedLength > maximumLength) {
        goto done;
    }

    if (serializedLength == 0) {
        retVal = RTI_XCDR_TRUE;
        goto done;
    }

    RTIXCdrLog_testPrecondition((out == NULL), goto done);

    RTIXCdrStream_deserializePrimitiveArrayMacro(
               me,
               out,
               (alignment > 4) ? RTI_XCDR_TRUE:RTI_XCDR_FALSE,
               alignment,
               elementSize,
               serializedLength,
               goto done);
    retVal = RTI_XCDR_TRUE;
done:

    if (!retVal) {
        /* Something went wrong: set length to zero. */
        serializedLength = 0;
    }

    if (length != NULL) {
        *length = serializedLength;
    }

    return retVal;
}


/*
*/
#define RTIXCdrStream_skipPrimitiveArrayMacro(\
     me,\
     align,\
     alignment,\
     elementSize,\
     length,\
     actionIfError)\
{\
    if (align) { \
        if (!RTIXCdrStream_align(me, alignment)) { \
            actionIfError; \
        } \
    } \
    if (!RTIXCdrStream_checkSize(me, \
            (RTIXCdrUnsignedLong)(elementSize * length))) { \
        actionIfError; \
    } \
    \
    RTIXCdrStream_skipNByteFast(me, length*elementSize); \
}

/*
*/
RTIXCdrBoolean RTIXCdrStream_skipPrimitiveArray(
    struct RTIXCdrStream *me,
    RTIXCdrBoolean align,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize,
    RTIXCdrUnsignedLong length)
{
    RTIXCdrLog_testPrecondition(
        (me == NULL) || (me->_currentPosition == NULL),
        return RTI_FALSE);

    RTIXCdrStream_skipPrimitiveArrayMacro(
            me,
            align,
            alignment,
            elementSize,
            length,
             return RTI_XCDR_FALSE);
    return RTI_XCDR_TRUE;
}


/*
*/
RTIXCdrBoolean RTIXCdrStream_skipPrimitiveSequence (
    struct RTIXCdrStream *me,
    RTIXCdrUnsignedLong *length,
    RTIXCdrAlignment alignment,
    RTIXCdrUnsignedLong elementSize)
{
    RTIXCdrUnsignedLong seqLength = 0;

    RTIXCdrLog_testPrecondition(
        (me == NULL) || (me->_currentPosition == NULL),
        return RTI_XCDR_FALSE);

    if (!RTIXCdrStream_deserialize4Byte((RTIXCdrStream*) me, &seqLength, RTI_XCDR_TRUE)) {
        return RTI_XCDR_FALSE;
    }

    if (length != NULL) {
        *length = seqLength;
    }

    if (seqLength == 0) {
        return RTI_XCDR_TRUE;
    }

    RTIXCdrStream_skipPrimitiveArrayMacro(
               me,
               (alignment>4) ? RTI_XCDR_TRUE:RTI_XCDR_FALSE,
               alignment,
               elementSize,
               seqLength,
               return RTI_XCDR_FALSE);

    return RTI_XCDR_TRUE;
}
