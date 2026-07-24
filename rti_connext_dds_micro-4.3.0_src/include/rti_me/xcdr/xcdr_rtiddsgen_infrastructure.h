/*
 * (c) Copyright, Real-Time Innovations, 2022.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission. Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef xcdr_rtiddsgen_infrastructure_h
#define xcdr_rtiddsgen_infrastructure_h

#define DDS_SEQUENCE_NO_HEADERS

#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_typeCode.h"

/* Don't redefine it if we are being included from somewhere that already has it */
#ifndef cdr_typeCode_h
#define RTICdrTypeCode RTIXCdrTypeCode
#endif

#define DDS_Char RTIXCdrChar
#define DDS_Wchar RTIXCdrWchar
#define DDS_Int8 RTIXCdrInt8
#define DDS_Octet RTIXCdrOctet
#define DDS_UInt8 RTIXCdrUInt8
#define DDS_Short RTIXCdrShort
#define DDS_UnsignedShort RTIXCdrUnsignedShort
#define DDS_Long RTIXCdrLong
#define DDS_UnsignedLong RTIXCdrUnsignedLong
#define DDS_LongLong RTIXCdrLongLong
#define DDS_UnsignedLongLong RTIXCdrUnsignedLongLong
#define DDS_Float RTIXCdrFloat
#define DDS_Double RTIXCdrDouble
#define DDS_Boolean RTIXCdrBoolean
#define DDS_LongDouble RTIXCdrLongDouble
#define DDS_KeyHash_t struct MIGRtpsKeyHash

#define DDS_TK_STRUCT RTI_XCDR_TK_STRUCT
#define DDS_TK_UNION RTI_XCDR_TK_UNION
#define DDS_TK_STRING RTI_XCDR_TK_STRING
#define DDS_TK_SEQUENCE RTI_XCDR_TK_SEQUENCE
#define DDS_TK_ARRAY RTI_XCDR_TK_ARRAY
#define DDS_TK_ALIAS RTI_XCDR_TK_ALIAS
#define DDS_TK_VALUE RTI_XCDR_TK_VALUE
#define DDS_TK_ENUM RTI_XCDR_TK_ENUM
#define DDS_TK_WSTRING RTI_XCDR_TK_WSTRING
#define DDS_TK_FINAL_EXTENSIBILITY RTI_XCDR_TK_FLAGS_IS_FINAL
#define DDS_TK_MUTABLE_EXTENSIBILITY RTI_XCDR_TK_FLAGS_IS_MUTABLE

#define DDS_TypeCode RTIXCdrTypeCodeWrapper

/*
 * This function allows us to obtain the programs required by the interpreter
 * without needing access to the TypeCodeFactory
 */
extern struct RTIXCdrInterpreterPrograms * RTIXCdrTypeCodeWrapper_createSerializationPrograms(
        const RTIXCdrTypeCodeWrapper *tc,
        const RTIXCdrInterpreterProgramsGenProperty *programProperties);

#define DDS_String_alloc RTIXCdrString_alloc
#define DDS_String_replace RTIXCdrString_replace
#define DDS_String_free RTIXCdrString_free
#define DDS_Wstring_alloc RTIXCdrWstring_alloc
#define DDS_Wstring_replace RTIXCdrWString_replace
#define DDS_Wstring_free RTIXCdrWString_free
#define DDS_TypeCode_Member RTIXCdrTypeCodeMember
#define DDS_LOG_BAD_PARAMETER_s RTI_LOG_BAD_PARAMETER_s
#define DDS_LOG_SET_FAILURE_s RTI_LOG_SET_FAILURE_s
#define DDS_LOG_GET_FAILURE_s RTI_LOG_GET_FAILURE_s
#define DDS_LOG_SEQUENCE_NOT_OWNER RTI_LOG_SEQUENCE_NOT_OWNER


/*
 * Some of these redefinitions are not required if certain cdr header
 * files (specifically cdr_stream.h and cdr_type.h) are included in
 * the generated code. They are both included by pres_typePlugin.h.
 */
#ifndef cdr_stream_h
  #define RTI_CDR_BOOLEAN_SIZE RTI_XCDR_BOOLEAN_SIZE
  #define RTI_CDR_CHAR_SIZE RTI_XCDR_CHAR_SIZE
  #define RTI_CDR_OCTET_SIZE RTI_XCDR_OCTET_SIZE
  #define RTI_CDR_SHORT_SIZE RTI_XCDR_SHORT_SIZE
  #define RTI_CDR_UNSIGNED_SHORT_SIZE RTI_XCDR_UNSIGNED_SHORT_SIZE
  #define RTI_CDR_LONG_SIZE RTI_XCDR_LONG_SIZE
  #define RTI_CDR_UNSIGNED_LONG_SIZE RTI_XCDR_UNSIGNED_LONG_SIZE
  #define RTI_CDR_LONG_LONG_SIZE RTI_XCDR_LONG_LONG_SIZE
  #define RTI_CDR_UNSIGNED_LONG_LONG_SIZE RTI_XCDR_UNSIGNED_LONG_LONG_SIZE
  #define RTI_CDR_FLOAT_SIZE RTI_XCDR_FLOAT_SIZE
  #define RTI_CDR_DOUBLE_SIZE RTI_XCDR_DOUBLE_SIZE
  #define RTI_CDR_LONG_DOUBLE_SIZE RTI_XCDR_LONG_DOUBLE_SIZE
  #define RTI_CDR_MAX_SERIALIZED_SIZE RTI_XCDR_MAX_SERIALIZED_SIZE
#endif
#ifndef cdr_type_h
  #define RTI_CDR_CHAR_TYPE RTI_XCDR_CHAR_TYPE
  #define RTICdrChar RTIXCdrChar
  #define RTICdrWchar RTIXCdrWchar
  #define RTICdrOctet RTIXCdrOctet
  #define RTICdrShort RTIXCdrShort
  #define RTICdrUnsignedShort RTIXCdrUnsignedShort
  #define RTICdrLong RTIXCdrLong
  #define RTICdrUnsignedLong RTIXCdrUnsignedLong
  #define RTICdrLongLong RTIXCdrLongLong
  #define RTICdrUnsignedLongLong RTIXCdrUnsignedLongLong
  #define RTICdrFloat RTIXCdrFloat
  #define RTICdrDouble RTIXCdrDouble
  #define RTICdrBoolean RTIXCdrBoolean
  #define RTICdrEnum RTIXCdrEnum

  #define RTICdrType_copyChar RTIXCdrType_copyChar
  #define RTICdrType_copyEnum RTIXCdrType_copyEnum
  #define RTICdrType_copyWchar RTIXCdrType_copyWchar
  #define RTICdrType_copyOctet RTIXCdrType_copyOctet
  #define RTICdrType_copyInt8 RTIXCdrType_copyInt8
  #define RTICdrType_copyUInt8 RTIXCdrType_copyUInt8
  #define RTICdrType_copyShort RTIXCdrType_copyShort
  #define RTICdrType_copyUnsignedShort RTIXCdrType_copyUnsignedShort
  #define RTICdrType_copyLong RTIXCdrType_copyLong
  #define RTICdrType_copyUnsignedLong RTIXCdrType_copyUnsignedLong
  #define RTICdrType_copyLongLong RTIXCdrType_copyLongLong
  #define RTICdrType_copyUnsignedLongLong RTIXCdrType_copyUnsignedLongLong
  #define RTICdrType_copyFloat RTIXCdrType_copyFloat
  #define RTICdrType_copyDouble RTIXCdrType_copyDouble
  #define RTICdrType_copyLongDouble RTIXCdrType_copyLongDouble
  #define RTICdrType_copyBoolean RTIXCdrType_copyBoolean
  #define RTICdrType_copyArray RTIXCdrType_copyArray
  #define RTICdrType_copyString RTIXCdrType_copyString
  #define RTICdrType_copyStringEx RTIXCdrType_copyStringEx
  #define RTICdrType_copyStringArrayEx RTIXCdrType_copyStringArrayEx
  #define RTICdrType_copyWstring RTIXCdrType_copyWstring
  #define RTICdrType_copyWstringEx RTIXCdrType_copyWstringEx
  #define RTICdrType_initLongDouble RTIXCdrType_init16Byte
  #define RTICdrType_initUnsignedShort RTIXCdrType_initUnsignedShort
  #define RTICdrType_initLong RTIXCdrType_initLong
  #define RTICdrType_initArray RTIXCdrType_initArray
  #define RTICdrType_initArrayUnsafe RTIXCdrType_initArrayUnsafe
  #define RTICdrType_initStringArray RTIXCdrType_initStringArray
  #define RTICdrType_finalizeStringArray RTIXCdrType_finalizeStringArray
#endif /* cdr_type_h */

#define DDS_BOOLEAN_FALSE 0
#define DDS_BOOLEAN_TRUE 1

#define NDDSUSERDllExport
#define DDSCDllExport

#define DDS_VM_NONE (short) 0
#define DDS_PRIVATE_MEMBER 0
#define DDS_PUBLIC_MEMBER (short) 1
#define RTI_CDR_NONKEY_MEMBER 0
#define RTI_CDR_KEY_MEMBER 1
#define RTI_CDR_REQUIRED_MEMBER 2 /* for STRUCT AND VALUE */

#define DDS_TypeAllocationParams_t RTIXCdrTypeAllocationParams
#define DDS_TYPE_ALLOCATION_PARAMS_DEFAULT \
        RTI_XCDR_TYPE_ALLOCATION_PARAMS_DEFAULT

#define DDS_TypeDeallocationParams_t RTIXCdrTypeDeallocationParams
#define DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT \
        RTI_XCDR_TYPE_DEALLOCATION_PARAMS_DEFAULT

#ifndef RTI_XCDR_DISABLE_SEQUENCE
#include "xcdr/xcdr_sequence.h"

DDS_SEQUENCE(RTIXCdrBooleanSeq, RTIXCdrBoolean);
#define DDS_BooleanSeq RTIXCdrBooleanSeq
#define DDS_BooleanSeq_initialize RTIXCdrBooleanSeq_initialize
#define DDS_BooleanSeq_finalize RTIXCdrBooleanSeq_finalize
#define DDS_BooleanSeq_set_maximum RTIXCdrBooleanSeq_set_maximum
#define DDS_BooleanSeq_set_length RTIXCdrBooleanSeq_set_length
#define DDS_BooleanSeq_copy RTIXCdrBooleanSeq_copy
#define DDS_BooleanSeq_set_absolute_maximum \
    RTIXCdrBooleanSeq_set_absolute_maximum

DDS_SEQUENCE(RTIXCdrInt8Seq, RTIXCdrInt8);
#define DDS_Int8Seq RTIXCdrInt8Seq
#define DDS_Int8Seq_initialize RTIXCdrInt8Seq_initialize
#define DDS_Int8Seq_finalize RTIXCdrInt8Seq_finalize
#define DDS_Int8Seq_set_maximum RTIXCdrInt8Seq_set_maximum
#define DDS_Int8Seq_set_length RTIXCdrInt8Seq_set_length
#define DDS_Int8Seq_copy RTIXCdrInt8Seq_copy
#define DDS_Int8Seq_set_absolute_maximum RTIXCdrInt8Seq_set_absolute_maximum

DDS_SEQUENCE(RTIXCdrUInt8Seq, RTIXCdrUInt8);
#define DDS_UInt8Seq RTIXCdrUInt8Seq
#define DDS_UInt8Seq_initialize RTIXCdrUInt8Seq_initialize
#define DDS_UInt8Seq_finalize RTIXCdrUInt8Seq_finalize
#define DDS_UInt8Seq_set_maximum RTIXCdrUInt8Seq_set_maximum
#define DDS_UInt8Seq_set_length RTIXCdrUInt8Seq_set_length
#define DDS_UInt8Seq_copy RTIXCdrUInt8Seq_copy
#define DDS_UInt8Seq_set_absolute_maximum RTIXCdrUInt8Seq_set_absolute_maximum

DDS_SEQUENCE(RTIXCdrShortSeq, RTIXCdrShort);
#define DDS_ShortSeq RTIXCdrShortSeq
#define DDS_ShortSeq_initialize RTIXCdrShortSeq_initialize
#define DDS_ShortSeq_finalize RTIXCdrShortSeq_finalize
#define DDS_ShortSeq_set_length RTIXCdrShortSeq_set_length
#define DDS_ShortSeq_set_maximum RTIXCdrShortSeq_set_maximum
#define DDS_ShortSeq_set_absolute_maximum RTIXCdrShortSeq_set_absolute_maximum
#define DDS_ShortSeq_copy RTIXCdrShortSeq_copy

DDS_SEQUENCE(RTIXCdrLongSeq, RTIXCdrLong);
#define DDS_LongSeq RTIXCdrLongSeq
#define DDS_LongSeq_initialize RTIXCdrLongSeq_initialize
#define DDS_LongSeq_finalize RTIXCdrLongSeq_finalize
#define DDS_LongSeq_set_length RTIXCdrLongSeq_set_length
#define DDS_LongSeq_set_maximum RTIXCdrLongSeq_set_maximum
#define DDS_LongSeq_set_absolute_maximum RTIXCdrLongSeq_set_absolute_maximum
#define DDS_LongSeq_copy RTIXCdrLongSeq_copy

DDS_SEQUENCE(RTIXCdrLongLongSeq, RTIXCdrLongLong);
  #define DDS_LongLongSeq RTIXCdrLongLongSeq
  #define DDS_LongLongSeq_initialize RTIXCdrLongLongSeq_initialize
  #define DDS_LongLongSeq_finalize RTIXCdrLongLongSeq_finalize
  #define DDS_LongLongSeq_set_length RTIXCdrLongLongSeq_set_length
  #define DDS_LongLongSeq_set_maximum RTIXCdrLongLongSeq_set_maximum
  #define DDS_LongLongSeq_set_absolute_maximum \
      RTIXCdrLongLongSeq_set_absolute_maximum
  #define DDS_LongLongSeq_copy RTIXCdrLongLongSeq_copy

DDS_SEQUENCE(RTIXCdrOctetSeq, RTIXCdrOctet);
#define DDS_OctetSeq RTIXCdrOctetSeq
#define DDS_OctetSeq_initialize RTIXCdrOctetSeq_initialize
#define DDS_OctetSeq_finalize RTIXCdrOctetSeq_finalize
#define DDS_OctetSeq_set_length RTIXCdrOctetSeq_set_length
#define DDS_OctetSeq_get_length RTIXCdrOctetSeq_get_length
#define DDS_OctetSeq_ensure_length RTIXCdrOctetSeq_ensure_length
#define DDS_OctetSeq_get_reference RTIXCdrOctetSeq_get_reference
#define DDS_OctetSeq_get_contiguous_buffer RTIXCdrOctetSeq_get_contiguous_buffer
#define DDS_OctetSeq_set_maximum RTIXCdrOctetSeq_set_maximum
#define DDS_OctetSeq_get_maximum RTIXCdrOctetSeq_get_maximum
#define DDS_OctetSeq_set_absolute_maximum \
    RTIXCdrOctetSeq_set_absolute_maximum
#define DDS_OctetSeq_copy RTIXCdrOctetSeq_copy
#define DDS_OctetSeq_get_contiguous_bufferI \
    RTIXCdrOctetSeq_get_contiguous_bufferI
#define DDS_OctetSeq_get_discontiguous_bufferI \
    RTIXCdrOctetSeq_get_discontiguous_bufferI

DDS_SEQUENCE(RTIXCdrUnsignedLongSeq, RTIXCdrUnsignedLong);
#define DDS_UnsignedLongSeq RTIXCdrUnsignedLongSeq
#define DDS_UnsignedLongSeq_initialize RTIXCdrUnsignedLongSeq_initialize
#define DDS_UnsignedLongSeq_finalize RTIXCdrUnsignedLongSeq_finalize
#define DDS_UnsignedLongSeq_set_length RTIXCdrUnsignedLongSeq_set_length
#define DDS_UnsignedLongSeq_get_length RTIXCdrUnsignedLongSeq_get_length
#define DDS_UnsignedLongSeq_ensure_length RTIXCdrUnsignedLongSeq_ensure_length
#define DDS_UnsignedLongSeq_get_reference RTIXCdrUnsignedLongSeq_get_reference
#define DDS_UnsignedLongSeq_get_contiguous_buffer \
    RTIXCdrUnsignedLongSeq_get_contiguous_buffer
#define DDS_UnsignedLongSeq_set_maximum RTIXCdrUnsignedLongSeq_set_maximum
#define DDS_UnsignedLongSeq_get_maximum RTIXCdrUnsignedLongSeq_get_maximum
#define DDS_UnsignedLongSeq_set_absolute_maximum \
    RTIXCdrUnsignedLongSeq_set_absolute_maximum
#define DDS_UnsignedLongSeq_copy RTIXCdrUnsignedLongSeq_copy

typedef char *RTIXCdrString;
DDS_SEQUENCE(RTIXCdrStringSeq, RTIXCdrString);
  #define DDS_StringSeq RTIXCdrStringSeq
  #define DDS_StringSeq_initialize RTIXCdrStringSeq_initialize
  #define DDS_StringSeq_finalize RTIXCdrStringSeq_finalize
  #define DDS_StringSeq_set_length RTIXCdrStringSeq_set_length
  #define DDS_StringSeq_get_length RTIXCdrStringSeq_get_length
  #define DDS_StringSeq_ensure_length RTIXCdrStringSeq_ensure_length
  #define DDS_StringSeq_get_reference RTIXCdrStringSeq_get_reference
  #define DDS_StringSeq_get_contiguous_buffer \
      RTIXCdrStringSeq_get_contiguous_buffer
  #define DDS_StringSeq_set_maximum RTIXCdrStringSeq_set_maximum
  #define DDS_StringSeq_get_maximum RTIXCdrStringSeq_get_maximum
  #define DDS_StringSeq_set_absolute_maximum \
      RTIXCdrStringSeq_set_absolute_maximum
  #define DDS_StringSeq_copy RTIXCdrStringSeq_copy
  #define DDS_StringSeq_get_contiguous_bufferI \
      RTIXCdrStringSeq_get_contiguous_bufferI
  #define DDS_StringSeq_get_discontiguous_bufferI \
      RTIXCdrStringSeq_get_discontiguous_bufferI
#endif

#define RTICdrTypeCodeAnnotations_INITIALIZER \
    RTIXCdrTypeCodeAnnotations_INITIALIZER
#define RTI_CDR_TYPE_CODE_UNION_DEFAULT_LABEL \
    RTI_XCDR_TYPE_CODE_UNION_DEFAULT_LABEL

extern RTIXCdrDllVariable RTIXCdrSampleAccessInfo RTIXCdr_g_sai_seq;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_boolean;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_octet;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_int8;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_uint8;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_short;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ushort;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_long;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ulong;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_longlong;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ulonglong;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_float;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_double;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_longdouble;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_char;
extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_wchar;

#define DDS_g_tc_null RTIXCdr_g_tc_null
#define DDS_g_tc_boolean RTIXCdr_g_tc_boolean
#define DDS_g_tc_octet RTIXCdr_g_tc_octet
#define DDS_g_tc_int8 RTIXCdr_g_tc_int8
#define DDS_g_tc_uint8 RTIXCdr_g_tc_uint8
#define DDS_g_tc_short RTIXCdr_g_tc_short
#define DDS_g_tc_ushort RTIXCdr_g_tc_ushort
#define DDS_g_tc_long RTIXCdr_g_tc_long
#define DDS_g_tc_ulong RTIXCdr_g_tc_ulong
#define DDS_g_tc_longlong RTIXCdr_g_tc_longlong
#define DDS_g_tc_ulonglong RTIXCdr_g_tc_ulonglong
#define DDS_g_tc_float RTIXCdr_g_tc_float
#define DDS_g_tc_double RTIXCdr_g_tc_double
#define DDS_g_tc_longdouble RTIXCdr_g_tc_longdouble
#define DDS_g_tc_char RTIXCdr_g_tc_char
#define DDS_g_tc_wchar RTIXCdr_g_tc_wchar
#define DDS_g_sai_seq RTIXCdr_g_sai_seq

#define DDS_INITIALIZE_STRING_TYPECODE(maximum__)      \
    {                                                  \
        RTI_XCDR_INITIALIZE_STRING_TYPECODE(maximum__) \
    }

#define DDS_INITIALIZE_WSTRING_TYPECODE(maximum__)      \
    {                                                   \
        RTI_XCDR_INITIALIZE_WSTRING_TYPECODE(maximum__) \
    }

#define DDS_INITIALIZE_SEQUENCE_TYPECODE(maximum__, typecode__)      \
    {                                                                \
        RTI_XCDR_INITIALIZE_SEQUENCE_TYPECODE(maximum__, typecode__) \
    }

#define DDS_INITIALIZE_ARRAY_TYPECODE(      \
        dimensionsCount__,                  \
        dimension_1__,                      \
        dimensions__,                       \
        typecode__)                         \
    {                                       \
        RTI_XCDR_INITIALIZE_ARRAY_TYPECODE( \
                dimensionsCount__,          \
                dimension_1__,              \
                dimensions__,               \
                typecode__)                 \
    }

RTIXCdrMemberValue RTIXCdrSequence_getMemberValuePointer(
        void *sample,
        RTIXCdrUnsignedLong *elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        RTIXCdrUnsignedLong elementIndex,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        void *programData);

RTIXCdrMemberValue RTIXCdrSequence_setMemberElementCount(
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

#ifdef RTI_PRECONDITION_TEST
  #define  DDSLog_preconditionOnly( declaration )  declaration
#else /* nothing */
  #define DDSLog_preconditionOnly( declaration )
#endif /* RTI_PRECONDITION_TEST */

#endif /* xcdr_rtiddsgen_infrastructure_h */
