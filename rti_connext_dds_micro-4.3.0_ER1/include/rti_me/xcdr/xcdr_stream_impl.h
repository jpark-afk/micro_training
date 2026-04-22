/*
(c) Copyright, Real-Time Innovations, 2018-2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_stream_impl_h
#define xcdr_stream_impl_h


#ifdef __cplusplus
    extern "C" {
#endif


#define RTIXCdrStream_DEFINITION(BufferLengthType) \
    /* Memory address for the serialization/deserialization buffer */ \
    char *_buffer; \
    /* Memory address from which we will do the alignment */ \
    char *_relativeBuffer; \
    char *_tmpRelativeBuffer; \
    BufferLengthType _bufferLength; \
    char *_currentPosition; \
    /* If 1, then deserialize methods will flip contents */ \
    RTIXCdrLong _needByteSwap; \
    /* Endianess of the stream */ \
    RTIXCdrEndian _endian; \
    /* Native endianess of the system */ \
    RTIXCdrEndian _nativeEndian; \
    RTIXCdrEncapsulationId _encapsulationKind; \
    RTIXCdrEncapsulationOptions _encapsulationOptions; \
    /* Clear memory on alignment */ \
    RTIXCdrLong _zeroOnAlign

struct RTIXCdrStreamState {
    char * buffer;
    char * relativeBuffer;
    RTIXCdrUnsignedLong bufferLength;
};

#define RTIXCdrStreamState_INITIALIZER { \
    NULL, /* buffer */ \
    NULL, /* relativeBuffer */ \
    0 /* bufferLength */ \
}

/* This structure has to be the same than RTICdrStream up to _zeroOnAlign */
typedef struct RTIXCdrStream {
    RTIXCdrStream_DEFINITION(RTIXCdrUnsignedLong);
} RTIXCdrStream;



#define RTI_XCDR_DHEADER_SIZE 4
#define RTI_XCDR_DHEADER_ALIGNMENT 4
#define RTI_XCDR_SEQ_LENGTH_ALIGNMENT 4

#define RTIXCdrStream_set(me, buffer, length) \
    (me)->_buffer = (buffer); \
    (me)->_relativeBuffer = (me)->_buffer; \
    (me)->_bufferLength = (length); \
    (me)->_currentPosition = (me)->_buffer;


#define RTIXCdrStream_setEncapsulationId(me, encapsulationId) \
    if ((me)->_encapsulationKind != (encapsulationId)) { \
        if (RTIXCdrEncapsulationId_isLittleEndianCdr((encapsulationId))) { \
            (me)->_endian = RTI_XCDR_ENDIAN_LITTLE; \
            (me)->_encapsulationKind = (encapsulationId); \
            (me)->_needByteSwap = \
            ((me)->_nativeEndian == RTI_XCDR_ENDIAN_BIG)? \
                    RTI_XCDR_TRUE: \
                    RTI_XCDR_FALSE; \
        } else { \
            (me)->_endian = RTI_XCDR_ENDIAN_BIG; \
            (me)->_encapsulationKind = (encapsulationId); \
            (me)->_needByteSwap = \
            ((me)->_nativeEndian == RTI_XCDR_ENDIAN_LITTLE)? \
                    RTI_XCDR_TRUE: \
                    RTI_XCDR_FALSE; \
        } \
    }

#define RTIXCdrStream_setEncapsulationOptions(me, encapsulationOpts) \
        (me)->_encapsulationOptions = (encapsulationOpts);

/* For performance reasons, this macro does not really check if the
 * encapsulation is CDR. It is assumed that if you call it you are calling it
 * in an encapsulation that is CDR
 */
#define RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId) \
    ((encapsulationId) & 0x0001)

#define RTIXCdrEncapsulationId_isBigEndianCdr(encapsulationId) \
    (!RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId))

#define RTIXCdrEncapsulationId_isCdr(encapsulationId) \
    (((encapsulationId) <= RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE) && ((encapsulationId) != 4))

#define RTIXCdrEncapsulationId_isCdrV2(encapsulationId) \
        (((encapsulationId) >= RTI_XCDR_ENCAPSULATION_ID_CDR2_BE) \
                && ((encapsulationId) <= RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE))

#define RTIXCdrStream_getEncapsulationId(me) \
    (me)->_encapsulationKind

#define RTIXCdrStream_getEncapsulationOptions(me) \
    (me)->_encapsulationOptions

#define RTIXCdrStream_resetAlignment(me) \
    (((me)->_tmpRelativeBuffer = (me)->_relativeBuffer), \
     ((me)->_relativeBuffer = (me)->_currentPosition), \
     ((me)->_tmpRelativeBuffer))

#define RTIXCdrStream_restoreAlignment(me, position) \
    (me)->_relativeBuffer = (position)

#define RTIXCdrStream_getCurrentPosition(me) \
    (me)->_currentPosition

#define RTIXCdrStream_setCurrentPosition(me, position) \
    (me)->_currentPosition = (position);

#define RTIXCdrStream_increaseCurrentPosition(me, count) \
    ((me)->_currentPosition += (count))

#define RTIXCdrStream_getBuffer(me) \
    (me)->_buffer

#define RTIXCdrStream_getRemainder(me) \
    ((me)->_bufferLength - RTIXCdrStream_getCurrentPositionOffset(me))

#define RTIXCdrStream_initFromStream(me, srcStream) \
    (me)->_needByteSwap = (srcStream)->_needByteSwap; \
    (me)->_endian = (srcStream)->_endian; \
    (me)->_nativeEndian = (srcStream)->_nativeEndian; \
    (me)->_encapsulationKind = (srcStream)->_encapsulationKind; \
    (me)->_encapsulationOptions = (srcStream)->_encapsulationOptions; \
    (me)->_zeroOnAlign = (srcStream)->_zeroOnAlign;

/* 
 * Primitive-type serialization 
 */

#define RTIXCdrStream_serializeWcharFast(me, val, v1) \
  if ((v1)) { \
    RTIXCdrLong __val4 = (RTIXCdrLong)(*(val));\
    RTIXCdrStream_serialize4ByteFast((me), &__val4);\
  } else {\
    RTIXCdrStream_serialize2ByteFast((me), (val));\
  }
  
#define RTIXCdrStream_deserializeWcharFast(me, val, v1) \
   if (v1) {\
      RTIXCdrLong __val4;\
      RTIXCdrStream_deserialize4ByteFast((me), &__val4); \
      *(val)= (RTIXCdrWchar) __val4;\
    } else {\
      RTIXCdrStream_deserialize2ByteFast((me), (val));\
    }
    
#define RTIXCdrStream_serialize1ByteFast(me, val) \
    *((me)->_currentPosition++) = *((char *)(val))

#define RTIXCdrStream_deserialize1ByteFast(me, val) \
    *((char *)(val)) = *((me)->_currentPosition++)

#define RTIXCdrStream_serialize2ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((me)->_currentPosition++) = *((char *)(val) + 1), \
            *((me)->_currentPosition++) = *((char *)(val)    ) \
        ) : \
        (void)( \
            *(RTIXCdr2Byte *)((void *)((me)->_currentPosition)) = \
                (*(RTIXCdr2Byte *)(val)), \
            (me)->_currentPosition += RTI_XCDR_TWO_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserialize2ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((char *)(val) + 1) = *((me)->_currentPosition++), \
            *((char *)(val)    ) = *((me)->_currentPosition++) \
        ) : \
        (void)( \
            (*(RTIXCdr2Byte *)(val)) = *(RTIXCdr2Byte *)((void *)((me)->_currentPosition)), \
            (me)->_currentPosition += RTI_XCDR_TWO_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_serialize4ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((me)->_currentPosition++) = *((char *)(val) + 3), \
            *((me)->_currentPosition++) = *((char *)(val) + 2), \
            *((me)->_currentPosition++) = *((char *)(val) + 1), \
            *((me)->_currentPosition++) = *((char *)(val)    ) \
        ) : \
        (void)( \
            *(RTIXCdr4Byte *)((void *)((me)->_currentPosition)) = \
                (*(RTIXCdr4Byte *)(val)), \
            (me)->_currentPosition += RTI_XCDR_FOUR_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserialize4ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((char *)(val) + 3) = *((me)->_currentPosition++), \
            *((char *)(val) + 2) = *((me)->_currentPosition++), \
            *((char *)(val) + 1) = *((me)->_currentPosition++), \
            *((char *)(val))     = *((me)->_currentPosition++) \
        ) : \
        (void)( \
            (*(RTIXCdr4Byte *)(val)) = *(RTIXCdr4Byte *)((void *)((me)->_currentPosition)), \
            (me)->_currentPosition += RTI_XCDR_FOUR_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_serialize8ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((me)->_currentPosition++) = *((char *)(val) + 7), \
            *((me)->_currentPosition++) = *((char *)(val) + 6), \
            *((me)->_currentPosition++) = *((char *)(val) + 5), \
            *((me)->_currentPosition++) = *((char *)(val) + 4), \
            *((me)->_currentPosition++) = *((char *)(val) + 3), \
            *((me)->_currentPosition++) = *((char *)(val) + 2), \
            *((me)->_currentPosition++) = *((char *)(val) + 1), \
            *((me)->_currentPosition++) = *((char *)(val)    )  \
        ) : \
        (void)( \
            /* Important to use copy function for 8-byte or 16-byte types because \
             * otherwise the pointer in the stream is not guaranteed to be aligned \
             * to 8 or 16 and the assignment could lead to bus errors in \
             * architectures such as Solaris. \
             */ \
            RTIXCdrMemory_copy((me)->_currentPosition, \
                    (val), \
                    RTI_XCDR_EIGHT_BYTE_SIZE), \
            (me)->_currentPosition += RTI_XCDR_EIGHT_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserialize8ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((char *)(val) + 7) = *((me)->_currentPosition++), \
            *((char *)(val) + 6) = *((me)->_currentPosition++), \
            *((char *)(val) + 5) = *((me)->_currentPosition++), \
            *((char *)(val) + 4) = *((me)->_currentPosition++), \
            *((char *)(val) + 3) = *((me)->_currentPosition++), \
            *((char *)(val) + 2) = *((me)->_currentPosition++), \
            *((char *)(val) + 1) = *((me)->_currentPosition++), \
            *((char *)(val))     = *((me)->_currentPosition++)  \
        ) : \
        (void)( \
            RTIXCdrMemory_copy((val), \
                    (me)->_currentPosition, \
                    RTI_XCDR_EIGHT_BYTE_SIZE), \
            (me)->_currentPosition += RTI_XCDR_EIGHT_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserializeNByteFast(me, val, n) \
    (void)(RTIXCdrMemory_copy((val), (me)->_currentPosition, n), \
           (me)->_currentPosition += (n))

#define RTIXCdrStream_deserializeNByte(me, val, alignment, n) \
  ((RTIXCdrStream_align((me), (alignment)) && \
    RTIXCdrStream_checkSize((me), (n))) ? \
   (RTIXCdrStream_deserializeNByteFast((me), (RTIXCdr1Byte *)(val), (n)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

#define RTIXCdrStream_popState(me, state) \
    (me)->_bufferLength = (state)->bufferLength; \
    (me)->_buffer = (state)->buffer; \
    (me)->_relativeBuffer = (state)->relativeBuffer

#define RTIXCdrEncapsulation_getAppendableCdrEncapsulationId(_id) \
( \
    (_id) == RTI_CDR_ENCAPSULATION_ID_CDR2_BE \
            ? RTI_CDR_ENCAPSULATION_ID_D_CDR2_BE : ( \
    (_id) == RTI_CDR_ENCAPSULATION_ID_CDR2_LE \
            ? RTI_CDR_ENCAPSULATION_ID_D_CDR2_LE : \
    (_id)) \
)

/* N-Byte */
#define RTIXCdrStream_serializeNByteFast(me, val, n) \
    (void)(RTIXCdrMemory_copy((me)->_currentPosition, (val), n), \
           (me)->_currentPosition += (n))

#define RTIXCdrStream_serializeNByte(me, val, alignment, n) \
  ((RTIXCdrStream_align((me), (alignment)) && \
    RTIXCdrStream_checkSize((me), (n))) ? \
   (RTIXCdrStream_serializeNByteFast((me), (const RTIXCdr1Byte *)(val), (n)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)  
   
#define RTIXCdrStream_serializeWchar(me, result, val, v1) \
  if (v1) { \
    RTIXCdrLong __val4 = (RTIXCdrLong)(*(val));\
    *(result) = RTIXCdrStream_serialize4Byte((me), &(__val4), RTI_XCDR_TRUE);\
  } else {\
    *(result) = RTIXCdrStream_serialize2Byte((me), (val), RTI_XCDR_TRUE);\
  }
  
#define RTIXCdrStream_deserializeWchar(me, result, val, v1) \
    if (v1) {\
        RTIXCdrLong __val4;\
        *(result) = RTIXCdrStream_deserialize4Byte((me), &__val4, RTI_XCDR_TRUE);\
        *(val)= (RTIXCdrWchar) __val4;\
    } else {\
        *(result) = RTIXCdrStream_deserialize2Byte((me), (val), RTI_XCDR_TRUE);\
    }
  
#define RTIXCdrStream_serialize1Byte(me, val) \
  ((RTIXCdrStream_align((me), 1) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_ONE_BYTE_SIZE)) ? \
   (RTIXCdrStream_serialize1ByteFast((me), (const RTIXCdr1Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
     
   
#define RTIXCdrStream_serialize2Byte(me, val, align) \
  (((!(align) || RTIXCdrStream_align((me), 2)) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_TWO_BYTE_SIZE)) ? \
   (RTIXCdrStream_serialize2ByteFast((me), (const RTIXCdr2Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
   
#define RTIXCdrStream_serialize4Byte(me, val, align) \
   (((!(align) || RTIXCdrStream_align((me), 4)) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_FOUR_BYTE_SIZE)) ? \
   (RTIXCdrStream_serialize4ByteFast((me), (const RTIXCdr4Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
   
  
 #define RTIXCdrStream_serialize8Byte(me, val, align, alignment) \
   (((!(align) || RTIXCdrStream_align((me), (alignment))) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_EIGHT_BYTE_SIZE)) ? \
   (RTIXCdrStream_serialize8ByteFast((me), (const RTIXCdr8Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
   
 #define RTIXCdrStream_serialize16Byte(me, val, align, alignment) \
   (((!(align) || RTIXCdrStream_align((me), (alignment))) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_SIXTEEN_BYTE_SIZE)) ? \
   (RTIXCdrStream_serialize16ByteFast((me), (const RTIXCdr16Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
 

#define RTIXCdrStream_serialize16ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((me)->_currentPosition++) = *((char *)(val) + 15), \
            *((me)->_currentPosition++) = *((char *)(val) + 14), \
            *((me)->_currentPosition++) = *((char *)(val) + 13), \
            *((me)->_currentPosition++) = *((char *)(val) + 12), \
            *((me)->_currentPosition++) = *((char *)(val) + 11), \
            *((me)->_currentPosition++) = *((char *)(val) + 10), \
            *((me)->_currentPosition++) = *((char *)(val) + 9), \
            *((me)->_currentPosition++) = *((char *)(val) + 8), \
            *((me)->_currentPosition++) = *((char *)(val) + 7), \
            *((me)->_currentPosition++) = *((char *)(val) + 6), \
            *((me)->_currentPosition++) = *((char *)(val) + 5), \
            *((me)->_currentPosition++) = *((char *)(val) + 4), \
            *((me)->_currentPosition++) = *((char *)(val) + 3), \
            *((me)->_currentPosition++) = *((char *)(val) + 2), \
            *((me)->_currentPosition++) = *((char *)(val) + 1), \
            *((me)->_currentPosition++) = *((char *)(val)    )  \
        ) : \
        (void)( \
            RTIXCdrMemory_copy((me)->_currentPosition, \
                    (val), \
                    RTI_XCDR_SIXTEEN_BYTE_SIZE), \
            (me)->_currentPosition += RTI_XCDR_SIXTEEN_BYTE_SIZE \
        ) \
    ) 
 

#define RTIXCdrStream_deserialize1Byte(me, val) \
  ((RTIXCdrStream_align((me), 1) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_ONE_BYTE_SIZE)) ? \
   (RTIXCdrStream_deserialize1ByteFast((me), (RTIXCdr1Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

#define RTIXCdrStream_deserialize2Byte(me, val, align) \
  (((!(align) || RTIXCdrStream_align((me), 2)) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_TWO_BYTE_SIZE)) ? \
   (RTIXCdrStream_deserialize2ByteFast((me), (RTIXCdr2Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)
   

#define RTIXCdrStream_deserialize4Byte(me, val, align) \
  (((!(align) || RTIXCdrStream_align((me), 4)) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_FOUR_BYTE_SIZE)) ? \
   (RTIXCdrStream_deserialize4ByteFast((me), (RTIXCdr4Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

/* 8-Byte */

#define RTIXCdrStream_deserialize8Byte(me, val, align, alignment) \
   (((!(align) || RTIXCdrStream_align((me), (alignment))) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_EIGHT_BYTE_SIZE)) ? \
   (RTIXCdrStream_deserialize8ByteFast((me), (RTIXCdr8Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

/* 16-Byte */

#define RTIXCdrStream_deserialize16ByteFast(me, val) \
    (((me)->_needByteSwap) ? \
        (void)( \
            *((char *)(val) + 15) = *((me)->_currentPosition++), \
            *((char *)(val) + 14) = *((me)->_currentPosition++), \
            *((char *)(val) + 13) = *((me)->_currentPosition++), \
            *((char *)(val) + 12) = *((me)->_currentPosition++), \
            *((char *)(val) + 11) = *((me)->_currentPosition++), \
            *((char *)(val) + 10) = *((me)->_currentPosition++), \
            *((char *)(val) + 9)  = *((me)->_currentPosition++), \
            *((char *)(val) + 8)  = *((me)->_currentPosition++), \
            *((char *)(val) + 7)  = *((me)->_currentPosition++), \
            *((char *)(val) + 6)  = *((me)->_currentPosition++), \
            *((char *)(val) + 5)  = *((me)->_currentPosition++), \
            *((char *)(val) + 4)  = *((me)->_currentPosition++), \
            *((char *)(val) + 3)  = *((me)->_currentPosition++), \
            *((char *)(val) + 2)  = *((me)->_currentPosition++), \
            *((char *)(val) + 1)  = *((me)->_currentPosition++), \
            *((char *)(val))      = *((me)->_currentPosition++)  \
        ) : \
        (void)( \
            RTIXCdrMemory_copy((val), \
                    (me)->_currentPosition, \
                    RTI_XCDR_SIXTEEN_BYTE_SIZE), \
            (me)->_currentPosition += RTI_XCDR_SIXTEEN_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserialize16Byte(me, val, align, alignment) \
   (((!(align) || RTIXCdrStream_align((me), (alignment))) && \
    RTIXCdrStream_checkSize((me), RTI_XCDR_SIXTEEN_BYTE_SIZE)) ? \
   (RTIXCdrStream_deserialize16ByteFast((me), (RTIXCdr16Byte *)(val)), \
   RTI_XCDR_TRUE) : RTI_XCDR_FALSE)

#define RTIXCdrStream_deserializeAndSetCdrEncapsulationMacro( \
        me, \
        failAction) \
{ \
    if (!RTIXCdrStream_checkSize((me), 4)) { \
        failAction; \
    } \
    RTIXCdrStream_deserializeUnsignedShortFromBigEndianFast( \
            (me), \
            &(me)->_encapsulationKind); \
    RTIXCdrStream_deserializeUnsignedShortFromBigEndianFast( \
            (me), \
            &(me)->_encapsulationOptions); \
    if (RTIXCdrEncapsulationId_isLittleEndianCdr((me)->_encapsulationKind)) { \
        (me)->_endian = RTI_XCDR_ENDIAN_LITTLE; \
        (me)->_needByteSwap = \
                ((me)->_nativeEndian == RTI_XCDR_ENDIAN_BIG)? \
                        RTI_XCDR_TRUE: \
                        RTI_XCDR_FALSE; \
    } else { \
        (me)->_endian = RTI_XCDR_ENDIAN_BIG; \
        (me)->_needByteSwap = \
                ((me)->_nativeEndian == RTI_XCDR_ENDIAN_LITTLE)? \
                        RTI_XCDR_TRUE: \
                        RTI_XCDR_FALSE; \
    } \
}

#define RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndiannessMacro( \
        me, \
        encapsulationId, \
        littleEndian, \
        failAction) \
{ \
    RTIXCdrUnsignedShort __options = 0; \
    if ((littleEndian)) { \
        (me)->_endian = RTI_XCDR_ENDIAN_LITTLE; \
        (me)->_encapsulationKind = (encapsulationId); \
        (me)->_needByteSwap = \
        ((me)->_nativeEndian == RTI_XCDR_ENDIAN_BIG)? \
                RTI_XCDR_TRUE: \
                RTI_XCDR_FALSE; \
    } else { \
        (me)->_endian = RTI_XCDR_ENDIAN_BIG; \
        (me)->_encapsulationKind = (encapsulationId); \
        (me)->_needByteSwap = \
        ((me)->_nativeEndian == RTI_XCDR_ENDIAN_LITTLE)? \
                RTI_XCDR_TRUE: \
                RTI_XCDR_FALSE; \
    } \
    if (!RTIXCdrStream_checkSize((me), 4)) { \
        failAction; \
    } \
    RTIXCdrStream_serializeUnsignedShortToBigEndianFast((me), &(encapsulationId)); \
    RTIXCdrStream_serializeUnsignedShortToBigEndianFast((me), &__options); \
}

#define RTIXCdrStream_serializeAndSetCdrEncapsulationFromHeaderMacro( \
        me__, \
        encapsulationHeader__, \
        failAction__) \
{ \
    RTIXCdrEncapsulationId encapsulationId__ = RTIXCdrUtility_ntohs( \
            * (RTIXCdrEncapsulationId *) (void *) (encapsulationHeader__)); \
    RTIXCdrLog_testPrecondition( \
            !RTIXCdrEncapsulationId_isCdr(encapsulationId__), \
            failAction__); \
 \
    RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndiannessMacro( \
            (me__), \
            (encapsulationId__), \
            RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId__), \
            failAction__); \
}

#define RTIXCdrStream_skipNByteFast(me, n) \
    ((me)->_currentPosition+=(n))

#define RTIXCdrStream_skipNByte(me, alignment, n) \
  ((RTIXCdrStream_align((me), (alignment)) \
           && RTIXCdrStream_checkSize((me), (n))) \
                  ? (RTIXCdrStream_skipNByteFast((me), (n)), RTI_XCDR_TRUE) \
                  : RTI_XCDR_FALSE)

#define RTIXCdrStream_alignFast(me, align) \
  me->_currentPosition = \
          me->_relativeBuffer \
          + RTIXCdrAlignment_alignSizeUp( \
                  RTIXCdrStream_getRelativeCurrentPositionOffset(me), \
                  align)

#define RTIXCdrStream_skipNByteNoCheckSize(me, alignment, n) \
  RTIXCdrStream_alignFast((me), (alignment)); \
  RTIXCdrStream_skipNByteFast((me), (n))

#define RTIXCdrStream_getRelativeCurrentPositionOffset(me) \
  ((RTIXCdrUnsignedLong) ((me)->_currentPosition - (me)->_relativeBuffer))

#define RTIXCdrStream_deserialize2ByteBigEndianFast(me, val) \
    (((me)->_nativeEndian == RTI_XCDR_ENDIAN_LITTLE) ? \
        (void)( \
            *((char *)(val) + 1) = *((me)->_currentPosition++), \
            *((char *)(val)) = *((me)->_currentPosition++) \
        ) : \
        (void)( \
            (*(val))  = *(RTIXCdr2Byte *)((void *)((me)->_currentPosition)), \
            (me)->_currentPosition += RTI_XCDR_TWO_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_serialize2ByteBigEndianFast(me, val) \
    (((me)->_nativeEndian == RTI_XCDR_ENDIAN_LITTLE) ? \
        (void)( \
            *((me)->_currentPosition++) = *((char *)(val) + 1), \
            *((me)->_currentPosition++) = *((char *)(val)    ) \
        ) : \
        (void)( \
            *(RTIXCdr2Byte *)((void *)((me)->_currentPosition)) = \
                (*(RTIXCdr2Byte *)(val)), \
            (me)->_currentPosition += RTI_XCDR_TWO_BYTE_SIZE \
        ) \
    )

#define RTIXCdrStream_deserializeUnsignedShortFromBigEndianFast(me, val) \
    RTIXCdrStream_deserialize2ByteBigEndianFast((me), (val))

#define RTIXCdrStream_serializeUnsignedShortToBigEndianFast(me, val) \
    RTIXCdrStream_serialize2ByteBigEndianFast((me), (val))

#define RTIXCdrStream_checkSize(me, size) \
  ((((RTIXCdrUnsignedLong) (size) > (me)->_bufferLength) \
           || (RTIXCdrStream_getCurrentPositionOffset((me)) \
                   > ((me)->_bufferLength - (RTIXCdrUnsignedLong) (size)))) \
                  ? RTI_XCDR_FALSE \
                  : RTI_XCDR_TRUE)

#define RTIXCdrStream_serializeAndSetCdrEncapsulation(me, encapsulationId) \
        RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndianness( \
            (me), \
            (encapsulationId), \
            RTIXCdrEncapsulationId_isLittleEndianCdr((encapsulationId)))

#define RTIXCdrStream_getCurrentPositionOffset(me) \
    ((RTIXCdrUnsignedLong)((me)->_currentPosition - (me)->_buffer))

#define RTIXCdrStream_setCurrentPositionOffset(me__, offset__) \
    (me__)->_currentPosition = (me__)->_buffer + (offset__)

#define RTIXCdrStream_popState(me, state) \
    (me)->_bufferLength = (state)->bufferLength; \
    (me)->_buffer = (state)->buffer; \
    (me)->_relativeBuffer = (state)->relativeBuffer



#define RTIXCdrStream_finishV2ParameterHeader(me, lengthPosition) \
if ((lengthPosition) != NULL) { \
    char *__position; \
    RTIXCdrUnsignedLong __memberLength; \
 \
    __position = RTIXCdrStream_getCurrentPosition((me)); \
    RTIXCdrStream_setCurrentPosition((me), (lengthPosition)); \
    __memberLength = (RTIXCdrUnsignedLong)(__position - \
            ((lengthPosition) +  RTI_XCDR_UNSIGNED_LONG_SIZE)); \
    RTIXCdrStream_serialize4ByteFast( \
            (me), \
            &__memberLength); \
    RTIXCdrStream_setCurrentPosition((me), __position); \
}

#ifdef RTI_ENDIAN_LITTLE
#define RTIXCdrStream_getEncapsulationIdFromBuffer(outId, buffer) \
        *((char *) (outId) + 1) = *((const char *) (buffer)); \
        *((char *) (outId)) = *((const char *) (buffer) + 1)
#else
#define RTIXCdrStream_getEncapsulationIdFromBuffer(outId, buffer) \
        *((RTIXCdr2Byte *) (outId)) = *((const RTIXCdr2Byte *) (buffer))
#endif

#ifdef RTI_ENDIAN_LITTLE
#define RTIXCdrStream_getEncapsulationOptionsFromBuffer(outOptions__, buffer__) \
        *((char *) (outOptions__) + 1) = *((const char *) (buffer__) + 2); \
        *((char *) (outOptions__)) = *((const char *) (buffer__) + 3)
#else
#define RTIXCdrStream_getEncapsulationOptionsFromBuffer(outOptions__, buffer__) \
        *((RTIXCdr2Byte *) (outOptions__)) = *((const RTIXCdr2Byte *) ((buffer__) + 2))
#endif


#define RTIXCdrStream_setEncapsulationOptionsCompressionId( \
        me__, \
        compressionId__) \
    ((me__)->_encapsulationOptions = \
            (RTIXCdrEncapsulationOptions) \
                    ((((me__)->_encapsulationOptions & ~(7 << 2)) \
                            | (((compressionId__) & 7) << 2)))) \

#define RTIXCdrStream_getEncapsulationOptionsCompressionId(me__) \
    (((me__)->_encapsulationOptions >> 2) & 7)

#define RTIXCdrEncapsulationOptions_pack( \
        options__, \
        serializedDataNumPaddingBytes__, \
        compressionId__, \
        compressionNumPaddingBytes__) \
    (*(options__) = \
            ((compressionId__) != RTI_XCDR_COMPRESSION_CLASS_ID_NONE) \
            ? (RTIXCdrEncapsulationOptions)( \
                    (((serializedDataNumPaddingBytes__) & 3) << 5) \
                    | (((compressionId__) & 7) << 2) \
                    | ((compressionNumPaddingBytes__) & 3)) \
            : (RTIXCdrEncapsulationOptions)( \
                    ((serializedDataNumPaddingBytes__) & 3)) \
    )

#define RTIXCdrEncapsulationOptions_unpack( \
        options__, \
        serializedDataNumPaddingBytes__, \
        compressionId__, \
        compressionNumPaddingBytes__) \
    { \
        *(compressionId__) = \
                (RTIXCdrCompressionId)(((options__) >> 2) & 7); \
        if (*(compressionId__) == RTI_XCDR_COMPRESSION_CLASS_ID_NONE) { \
            *(compressionNumPaddingBytes__) = 0; \
            *(serializedDataNumPaddingBytes__) = \
                    (RTIXCdrOctet)((options__) & 3); \
        } else { \
            *(compressionNumPaddingBytes__) = \
                    (RTIXCdrOctet)((options__) & 3); \
            *(serializedDataNumPaddingBytes__) = \
                    (RTIXCdrOctet)(((options__) >> 5) & 3); \
        } \
    }

#define RTIXCdrStream_packEncapsulationOptions( \
        me__, \
        serializedDataNumPaddingBytes__, \
        compressionId__, \
        compressionNumPaddingBytes__) \
    RTIXCdrEncapsulationOptions_pack( \
            &(me__)->_encapsulationOptions, \
            (serializedDataNumPaddingBytes__), \
            (compressionId__), \
            (compressionNumPaddingBytes__))

#define RTIXCdrStream_unpackEncapsulationOptions( \
        me__, \
        serializedDataNumPaddingBytes__, \
        compressionId__, \
        compressionNumPaddingBytes__) \
    RTIXCdrEncapsulationOptions_unpack( \
            (me__)->_encapsulationOptions, \
            (serializedDataNumPaddingBytes__), \
            (compressionId__), \
            (compressionNumPaddingBytes__))

#define RTIXCdrEncapsulationId_toStr(encapsulationId__) \
( \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_CDR_BE) ? RTI_XCDR_ENCAPSULATION_ID_CDR_BE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_CDR_LE) ? RTI_XCDR_ENCAPSULATION_ID_CDR_LE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE) ? RTI_XCDR_ENCAPSULATION_ID_PL_CDR_BE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE) ? RTI_XCDR_ENCAPSULATION_ID_PL_CDR_LE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_CDR2_BE) ? RTI_XCDR_ENCAPSULATION_ID_CDR2_BE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_CDR2_LE) ? RTI_XCDR_ENCAPSULATION_ID_CDR2_LE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE) ? RTI_XCDR_ENCAPSULATION_ID_D_CDR2_BE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE) ? RTI_XCDR_ENCAPSULATION_ID_D_CDR2_LE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE) ? RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_BE_STR : \
    ((encapsulationId__) == RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE) ? RTI_XCDR_ENCAPSULATION_ID_PL_CDR2_LE_STR : \
    "Unknown Encapsulation ID" \
) \

#define RTIXCdrStream_isAligned(me__, alignment__) \
    (((RTIXCdrStream_getRelativeCurrentPositionOffset((me__))) % (RTI_UINT32) (alignment__)) == 0)

#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_stream_impl_h */
