/*
 * FILE: cdr_cdr_type.h - CDR types
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 *
 * Modification History
 * --------------------
 * 10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 15sep2014,eh  Updated documentation
 * 14aug2012,kaj Written
 */

/*ci
 * \file
 * \defgroup CDRTypeClass CDR Types
 * \ingroup CDRModule
 * \brief CDR types
 *
 * \details
 * Defines supported primitive CDR types and sequences.
 *
 * Note, a sequence of a type B will be defined as another type A when
 * sizes of type A and B are equal.  For example, CDR_BooleanSeq is defined
 * as CDR_OctetSeq.  This is done to minimize the number of lines of code.
 */

/*ci \addtogroup CDRTypeClass
 *   @{
 */
#ifndef cdr_cdr_type_h
#define cdr_cdr_type_h

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef cdr_dll_h
#include "cdr/cdr_dll.h"
#endif

#ifdef RTI_CERT
#define CDR_CPP_CERT_PRIVATE_OPERATORS(T) \
    private:\
        T(const T& from);\
        T& operator=(const T& from);\
        bool operator==(const T& other);\
        bool operator!=(const T& other);

#define CDR_CPP_QOS_OPERATORS(T) \
        CDR_CPP_CERT_PRIVATE_OPERATORS(T)

#define CDR_CPP_QOS_POLICY_OPERATORS(T) \
        CDR_CPP_CERT_PRIVATE_OPERATORS(T)

#define CDR_CPP_STATUS_OPERATORS(T) \
        CDR_CPP_CERT_PRIVATE_OPERATORS(T)
#else
#define CDR_CPP_QOS_OPERATORS(T) \
    public:\
        bool operator==(const T& other);\
        bool operator!=(const T& other);\
        T& operator=(const T& other);

#define CDR_CPP_QOS_POLICY_OPERATORS(T)

#define CDR_CPP_STATUS_OPERATORS(T)
#endif

#ifdef RTI_CPP
#define CDR_CPP_QOS_METHODS(T) \
    public:\
        T(); \
        T(const T& from);\
        ~T(); \
        bool copy(const T& from); \
    CDR_CPP_QOS_OPERATORS(T)

#define CDR_CPP_QOS_POLICY_METHODS(T) \

#define CDR_CPP_STATUS_METHODS(T)

#else
#define CDR_CPP_QOS_METHODS(T)
#define CDR_CPP_QOS_POLICY_METHODS(T)
#define CDR_CPP_STATUS_METHODS(T)
#endif

#define CDR_QOS_POLICY_METHODS_DECL(T)

#define CDR_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_FULL(T) \
 struct T; \
 CDRDllExport RTI_BOOL T ## _initialize(struct T* self); \
 CDRDllExport RTI_BOOL T ## _finalize(struct T* self); \
 CDRDllExport RTI_BOOL T ## _copy(struct T* self, const struct T* from);\
 MUST_CHECK_RETURN CDRDllExport RTI_BOOL T ## _is_equal(const struct T* self, const struct T* from)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ================================================================= */
/*                       CDR Primitive Types                         */
/* ================================================================= */

/*ci \brief Char */
typedef char CDR_Char;

/*ci \brief Octet */
typedef RTI_UINT8 CDR_Octet;

/*ci \brief Boolean */
typedef RTI_UINT8 CDR_Boolean;

/*ci \brief Short */
typedef RTI_INT16 CDR_Short;

/*ci \brief Unsigned Short */
typedef RTI_UINT16 CDR_UnsignedShort;

/*ci \brief Long */
typedef RTI_INT32 CDR_Long;

/*ci \brief Unsigned Long */
typedef RTI_UINT32 CDR_UnsignedLong;

/*ci \brief Enum */
typedef RTI_UINT32 CDR_Enum;

/*ci \brief WChar */
typedef RTI_UINT32 CDR_Wchar;

/*ci \brief Long Long */
typedef RTI_INT64 CDR_LongLong;

/*ci \brief Unsigned Long Long */
typedef RTI_UINT64 CDR_UnsignedLongLong;

/*ci \brief Float */
typedef RTI_FLOAT32 CDR_Float;

/*ci \brief Double */
typedef RTI_DOUBLE64 CDR_Double;

/*ci \brief Long Double */
typedef RTI_DOUBLE128 CDR_LongDouble;

/*ci \brief String */
typedef char *CDR_String;

/*ci \brief WString */
typedef CDR_Wchar *CDR_Wstring;

#if CDR_ENABLE_WCHAR32
#define CDR_WCHAR_MEM_TYPE RTI_UINT32
#else
#define CDR_WCHAR_MEM_TYPE RTI_UINT16
#endif

/* ================================================================= */
/*                 CDR Primitive Type Sequences                      */
/* ================================================================= */

/*ci \brief Octet Sequence
 *
 * \details
 * Other CDR types of same length as Octet (i.e. Char, Boolean) have their
 * sequences aliased to this sequence.
 */
#define T CDR_Octet
#define TSeq CDR_OctetSeq
#define TSeq_is_equal
#define TSeq_get_contiguous_buffer
#define TSeq_loan_contiguous
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T CDR_Char
#define TSeq CDR_CharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Boolean
#define TSeq CDR_BooleanSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Short
#define TSeq CDR_ShortSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Long
#define TSeq CDR_LongSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Enum
#define TSeq CDR_EnumSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Wchar
#define TSeq CDR_WcharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_LongLong
#define TSeq CDR_LongLongSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>


#define T    CDR_Float
#define TSeq CDR_FloatSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    CDR_Double
#define TSeq CDR_DoubleSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

/*ci \brief Unsigned Short Sequence
 *
 * \details
 * Other CDR types of same length as Unsigned Short (i.e. Short) have their
 * sequences aliased to this sequence.
 */
#define T CDR_UnsignedShort
#define TSeq CDR_UnsignedShortSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

/*ci \brief Unsigned Long Sequence
 *
 * \details
 * Other CDR types of same length as Unsigned Long (i.e. Long, Enum, Wchar,
 * Float) have their sequences aliased to this sequence.
 */
#define T CDR_UnsignedLong
#define TSeq CDR_UnsignedLongSeq
#if DDS_FILTERING_ENABLED
#define TSeq_loan_contiguous
#endif
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

/*ci \brief Unsigned Long Long Sequence
 *
 * \details
 * Other CDR types of same length as Unsigned Long Long (i.e. Long Long,
 * Double) have their sequences aliased to this sequence.
 */
#define T CDR_UnsignedLongLong
#define TSeq CDR_UnsignedLongLongSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

/*ci \brief Unsigned Long Double Sequence */
#define T CDR_LongDouble
#define TSeq CDR_LongDoubleSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

/*ci \brief String Sequence is an alias for REDA_StringSeq */

/*ci \brief WString Sequence */
#define T CDR_Wstring
#define TSeq CDR_WstringSeq
#define TSeq_isCDRStringType
#define TSeq_isCDRStringType_no_max
#define REDA_SEQUENCE_USER_API
#ifndef RTI_CERT
#define TSeq_is_equal
#endif
#include <reda/reda_sequence_decl.h>

#ifdef SEQUENCE_ALIAS
/*ci \brief Assign CDR_BooleanSeq as alias of CDR_OctetSeq */
#define CDR_BooleanSeq                 CDR_OctetSeq

/*ci \brief Assign CDR_CharSeq as alias of CDR_OctetSeq */
#define CDR_CharSeq                    CDR_OctetSeq

/*ci \brief Assign CDR_ShortSeq as alias of CDR_UnsignedShortSeq */
#define CDR_ShortSeq                   CDR_UnsignedShortSeq

/*ci \brief Assign CDR_LongSeq as alias of CDR_UnsignedLongSeq */
#define CDR_LongSeq                    CDR_UnsignedLongSeq

/*ci \brief Assign CDR_EnumSeq as alias of CDR_UnsignedLongSeq */
#define CDR_EnumSeq                    CDR_UnsignedLongSeq

/*ci \brief Assign CDR_WcharSeq as alias of CDR_UnsignedLongSeq */
#define CDR_WcharSeq                   CDR_UnsignedLongSeq

/*ci \brief Assign CDR_FloatSeq as alias of CDR_UnsignedLongSeq */
#define CDR_FloatSeq                   CDR_UnsignedLongSeq

/*ci \brief Assign CDR_LongLongSeq as alias of CDR_UnsignedLongLongSeq */
#define CDR_LongLongSeq                CDR_UnsignedLongLongSeq

/*ci \brief Assign CDR_DoubleSeq as alias of CDR_UnsignedLongLongSeq */
#define CDR_DoubleSeq                  CDR_UnsignedLongLongSeq
#endif

/*ci \brief Assign CDR_StringSeq as alias of REDA_StringSeq */
#define CDR_StringSeq                 REDA_StringSeq

/*ci \brief Assign CDR_StringSeq_initialize as alias of REDA_StringSeq_initialize */
#define CDR_StringSeq_initialize      REDA_StringSeq_initialize

/*ci \brief Assign CDR_StringSeq_get_maximum as alias of REDA_StringSeq_get_maximum */
#define CDR_StringSeq_get_maximum     REDA_StringSeq_get_maximum

/*ci \brief Assign CDR_StringSeq_set_maximum as alias of REDA_StringSeq_set_maximum */
#define CDR_StringSeq_set_maximum     REDA_StringSeq_set_maximum_w_max

/*ci \brief Assign CDR_BooleanSeq_get_length as alias of REDA_StringSeq_get_length */
#define CDR_StringSeq_get_length      REDA_StringSeq_get_length

/*ci \brief Assign CDR_StringSeq_set_length as alias of REDA_StringSeq_set_length */
#define CDR_StringSeq_set_length      REDA_StringSeq_set_length

/*ci \brief Assign CDR_StringSeq_get_reference as alias of REDA_StringSeq_get_reference */
#define CDR_StringSeq_get_reference   REDA_StringSeq_get_reference

/*ci \brief Assign CDR_StringSeq_copy as alias of REDA_StringSeq_copy_w_max */
#define CDR_StringSeq_copy             REDA_StringSeq_copy_w_max

#ifndef RTI_CERT
/*ci \brief Assign CDR_StringSeq_finalize as alias of REDA_StringSeq_finalize */
#define CDR_StringSeq_finalize         REDA_StringSeq_finalize
#endif

/* Define the following aliases if the DEFAULT API is > BASIC */
#if REDA_SEQUENCE_API_USER_DEFAULT > REDA_SEQUENCE_API_BASIC

/*ci \brief Assign CDR_StringSeq_is_equal as alias of REDA_StringSeq_is_equal */
#define CDR_StringSeq_is_equal                 REDA_StringSeq_is_equal

/*ci \brief Assign CDR_String_loan_contiguous as alias of REDA_StringSeq_loan_contiguous */
#define CDR_StringSeq_loan_contiguous          REDA_StringSeq_loan_contiguous

/*ci \brief Assign CDR_StringSeq_loan_discontiguous as alias of REDA_StringSeq_loan_discontiguous */
#define CDR_StringSeq_loan_discontiguous       REDA_StringSeq_loan_discontiguous

/*ci \brief Assign CDR_StringSeq_unloan as alias of REDA_StringSeq_unloan */
#define CDR_StringSeq_unloan                   REDA_StringSeq_unloan

/*ci \brief Assign CDR_StringSeq_has_ownership as alias of REDA_StringSeq_has_ownership */
#define CDR_StringSeq_has_ownership            REDA_StringSeq_has_ownership

/*ci \brief Assign CDR_StringSeq_get_buffer as alias of REDA_StringSeq_get_buffer */
#define CDR_StringSeq_get_contiguous_buffer    REDA_StringSeq_get_contiguous_buffer

/*ci \brief Assign CDR_StringSeq_set_buffer as alias of REDA_StringSeq_set_buffer */
#define CDR_StringSeq_set_contiguous_buffer    REDA_StringSeq_set_contiguous_buffer

/*ci \brief Assign CDR_StringSeq_has_discontiguous_buffer as alias of REDA_StringSeq_has_discontiguous_buffer */
#define CDR_StringSeq_has_discontiguous_buffer REDA_StringSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_StringSeq_ensure_length as alias of REDA_StringSeq_ensure_length_w_max */
#define CDR_StringSeq_ensure_length             REDA_StringSeq_ensure_length_w_max

/*ci \brief Assign CDR_StringSeq_to_array as alias of REDA_StringSeq_to_array_w_max */
#define CDR_StringSeq_to_array                 REDA_StringSeq_to_array_w_max

/*ci \brief Assign CDR_StringSeq_from_array as alias of REDA_StringSeq_from_array_w_max */
#define CDR_StringSeq_from_array               REDA_StringSeq_from_array_w_max

#endif

#if SEQUENCE_ALIAS
/* CDR BooleanSeq operations */

/*ci \brief Assign CDR_BooleanSeq_initialize as alias of CDR_OctetSeq_initialize */
#define CDR_BooleanSeq_initialize      CDR_OctetSeq_initialize

/*ci \brief Assign CDR_BooleanSeq_get_maximum as alias of CDR_OctetSeq_get_maximum */
#define CDR_BooleanSeq_get_maximum     CDR_OctetSeq_get_maximum

/*ci \brief Assign CDR_BooleanSeq_set_maximum as alias of CDR_OctetSeq_set_maximum */
#define CDR_BooleanSeq_set_maximum     CDR_OctetSeq_set_maximum

/*ci \brief Assign CDR_BooleanSeq_get_length as alias of CDR_OctetSeq_get_length */
#define CDR_BooleanSeq_get_length      CDR_OctetSeq_get_length

/*ci \brief Assign CDR_BooleanSeq_set_length as alias of CDR_OctetSeq_set_length */
#define CDR_BooleanSeq_set_length      CDR_OctetSeq_set_length

/*ci \brief Assign CDR_BooleanSeq_get_reference as alias of CDR_OctetSeq_get_reference */
#define CDR_BooleanSeq_get_reference   (CDR_Boolean*)CDR_OctetSeq_get_reference

/* CDR_CharSeq operations */

/*ci \brief Assign CDR_CharSeq_initialize as alias of CDR_OctetSeq_initialize */
#define CDR_CharSeq_initialize         CDR_OctetSeq_initialize

/*ci \brief Assign CDR_CharSeq_get_maximum as alias of CDR_OctetSeq_get_maximum */
#define CDR_CharSeq_get_maximum        CDR_OctetSeq_get_maximum

/*ci \brief Assign CDR_CharSeq_set_maximum as alias of CDR_OctetSeq_set_maximum */
#define CDR_CharSeq_set_maximum        CDR_OctetSeq_set_maximum

/*ci \brief Assign CDR_CharSeq_get_length as alias of CDR_OctetSeq_get_length */
#define CDR_CharSeq_get_length         CDR_OctetSeq_get_length

/*ci \brief Assign CDR_CharSeq_set_length as alias of CDR_OctetSeq_set_length */
#define CDR_CharSeq_set_length         CDR_OctetSeq_set_length

/*ci \brief Assign CDR_CharSeq_get_reference as alias of CDR_OctetSeq_get_reference */
#define CDR_CharSeq_get_reference      (CDR_Char*)CDR_OctetSeq_get_reference

/* CDR_ShortSeq operations */

/*ci \brief Assign CDR_ShortSeq_initialize as alias of CDR_UnsignedShortSeq_initialize */
#define CDR_ShortSeq_initialize        CDR_UnsignedShortSeq_initialize

/*ci \brief Assign CDR_ShortSeq_get_maximum as alias of CDR_UnsignedShortSeq_get_maximum */
#define CDR_ShortSeq_get_maximum       CDR_UnsignedShortSeq_get_maximum

/*ci \brief Assign CDR_ShortSeq_set_maximum as alias of CDR_UnsignedShortSeq_set_maximum */
#define CDR_ShortSeq_set_maximum       CDR_UnsignedShortSeq_set_maximum

/*ci \brief Assign CDR_ShortSeq_get_length as alias of CDR_UnsignedShortSeq_get_length */
#define CDR_ShortSeq_get_length        CDR_UnsignedShortSeq_get_length

/*ci \brief Assign CDR_ShortSeq_set_length as alias of CDR_UnsignedShortSeq_set_length */
#define CDR_ShortSeq_set_length        CDR_UnsignedShortSeq_set_length

/*ci \brief Assign CDR_ShortSeq_get_reference as alias of CDR_UnsignedShortSeq_get_reference */
#define CDR_ShortSeq_get_reference     (CDR_Short*)CDR_UnsignedShortSeq_get_reference


/* CDR_LongSeq operations */

/*ci \brief Assign CDR_LongSeq_initialize as alias of CDR_UnsignedLongSeq_initialize */
#define CDR_LongSeq_initialize         CDR_UnsignedLongSeq_initialize

/*ci \brief Assign CDR_LongSeq_get_maximum as alias of CDR_UnsignedLongSeq_get_maximum */
#define CDR_LongSeq_get_maximum        CDR_UnsignedLongSeq_get_maximum

/*ci \brief Assign CDR_LongSeq_set_maximum as alias of CDR_UnsignedLongSeq_set_maximum */
#define CDR_LongSeq_set_maximum        CDR_UnsignedLongSeq_set_maximum

/*ci \brief Assign CDR_LongSeq_get_length as alias of CDR_UnsignedLongSeq_get_length */
#define CDR_LongSeq_get_length         CDR_UnsignedLongSeq_get_length

/*ci \brief Assign CDR_LongSeq_set_length as alias of CDR_UnsignedLongSeq_set_length */
#define CDR_LongSeq_set_length         CDR_UnsignedLongSeq_set_length

/*ci \brief Assign CDR_LongSeq_get_reference as alias of CDR_UnsignedLongSeq_get_reference */
#define CDR_LongSeq_get_reference      (CDR_Long*)CDR_UnsignedLongSeq_get_reference


/* CDR_EnumSeq operations */

/*ci \brief Assign CDR_EnumSeq_initialize as alias of CDR_UnsignedLongSeq_initialize */
#define CDR_EnumSeq_initialize         CDR_UnsignedLongSeq_initialize

/*ci \brief Assign CDR_EnumSeq_get_maximum as alias of CDR_UnsignedLongSeq_get_maximum */
#define CDR_EnumSeq_get_maximum        CDR_UnsignedLongSeq_get_maximum

/*ci \brief Assign CDR_EnumSeq_set_maximum as alias of CDR_UnsignedLongSeq_set_maximum */
#define CDR_EnumSeq_set_maximum        CDR_UnsignedLongSeq_set_maximum

/*ci \brief Assign CDR_EnumSeq_get_length as alias of CDR_UnsignedLongSeq_get_length */
#define CDR_EnumSeq_get_length         CDR_UnsignedLongSeq_get_length

/*ci \brief Assign CDR_EnumSeq_set_length as alias of CDR_UnsignedLongSeq_set_length */
#define CDR_EnumSeq_set_length         CDR_UnsignedLongSeq_set_length

/*ci \brief Assign CDR_EnumSeq_get_reference as alias of CDR_UnsignedLongSeq_get_reference */
#define CDR_EnumSeq_get_reference      (CDR_Enum*)CDR_UnsignedLongSeq_get_reference


/* CDR_WcharSeq operations */

/*ci \brief Assign CDR_WcharSeq_initialize as alias of CDR_UnsignedLongSeq_initialize */
#define CDR_WcharSeq_initialize        CDR_UnsignedLongSeq_initialize

/*ci \brief Assign CDR_WcharSeq_get_maximum as alias of CDR_UnsignedLongSeq_get_maximum */
#define CDR_WcharSeq_get_maximum       CDR_UnsignedLongSeq_get_maximum

/*ci \brief Assign CDR_WcharSeq_set_maximum as alias of CDR_UnsignedLongSeq_set_maximum */
#define CDR_WcharSeq_set_maximum       CDR_UnsignedLongSeq_set_maximum

/*ci \brief Assign CDR_WcharSeq_get_length as alias of CDR_UnsignedLongSeq_get_length */
#define CDR_WcharSeq_get_length        CDR_UnsignedLongSeq_get_length

/*ci \brief Assign CDR_WcharSeq_set_length as alias of CDR_UnsignedLongSeq_set_length */
#define CDR_WcharSeq_set_length        CDR_UnsignedLongSeq_set_length

/*ci \brief Assign CDR_WcharSeq_get_reference as alias of CDR_UnsignedLongSeq_get_reference */
#define CDR_WcharSeq_get_reference     (CDR_Wchar*)CDR_UnsignedLongSeq_get_reference


/* CDR_FloatSeq operations */

/*ci \brief Assign CDR_FloatSeq_initialize as alias of CDR_UnsignedLongSeq_initialize */
#define CDR_FloatSeq_initialize        CDR_UnsignedLongSeq_initialize

/*ci \brief Assign CDR_FloatSeq_get_maximum as alias of CDR_UnsignedLongSeq_get_maximum */
#define CDR_FloatSeq_get_maximum       CDR_UnsignedLongSeq_get_maximum

/*ci \brief Assign CDR_FloatSeq_set_maximum as alias of CDR_UnsignedLongSeq_set_maximum */
#define CDR_FloatSeq_set_maximum       CDR_UnsignedLongSeq_set_maximum

/*ci \brief Assign CDR_FloatSeq_get_length as alias of CDR_UnsignedLongSeq_get_length */
#define CDR_FloatSeq_get_length        CDR_UnsignedLongSeq_get_length

/*ci \brief Assign CDR_FloatSeq_set_length as alias of CDR_UnsignedLongSeq_set_length */
#define CDR_FloatSeq_set_length        CDR_UnsignedLongSeq_set_length

/*ci \brief Assign CDR_FloatSeq_get_reference as alias of CDR_UnsignedLongSeq_get_reference */
#define CDR_FloatSeq_get_reference     (CDR_Float*)CDR_UnsignedLongSeq_get_reference


/* CDR_LongLongSeq operations */

/*ci \brief Assign CDR_LongLongSeq_initialize as alias of CDR_UnsignedLongLongSeq_initialize */
#define CDR_LongLongSeq_initialize     CDR_UnsignedLongLongSeq_initialize

/*ci \brief Assign CDR_LongLongSeq_get_maximum as alias of CDR_UnsignedLongLongSeq_get_maximum */
#define CDR_LongLongSeq_get_maximum    CDR_UnsignedLongLongSeq_get_maximum

/*ci \brief Assign CDR_LongLongSeq_set_maximum as alias of CDR_UnsignedLongLongSeq_set_maximum */
#define CDR_LongLongSeq_set_maximum    CDR_UnsignedLongLongSeq_set_maximum

/*ci \brief Assign CDR_LongLongSeq_get_length as alias of CDR_UnsignedLongLongSeq_get_length */
#define CDR_LongLongSeq_get_length     CDR_UnsignedLongLongSeq_get_length

/*ci \brief Assign CDR_LongLongSeq_set_length as alias of CDR_UnsignedLongLongSeq_set_length */
#define CDR_LongLongSeq_set_length     CDR_UnsignedLongLongSeq_set_length

/*ci \brief Assign CDR_LongLongSeq_get_reference as alias of CDR_UnsignedLongLongSeq_get_reference */
#define CDR_LongLongSeq_get_reference  (CDR_LongLong*)CDR_UnsignedLongLongSeq_get_reference


/* CDR_DoubleSeq operations */

/*ci \brief Assign CDR_DoubleSeq_initialize as alias of CDR_UnsignedLongLongSeq_initialize */
#define CDR_DoubleSeq_initialize       CDR_UnsignedLongLongSeq_initialize

/*ci \brief Assign CDR_DoubleSeq_get_maximum as alias of CDR_UnsignedLongLongSeq_get_maximum */
#define CDR_DoubleSeq_get_maximum      CDR_UnsignedLongLongSeq_get_maximum

/*ci \brief Assign CDR_DoubleSeq_set_maximum as alias of CDR_UnsignedLongLongSeq_set_maximum */
#define CDR_DoubleSeq_set_maximum      CDR_UnsignedLongLongSeq_set_maximum

/*ci \brief Assign CDR_DoubleSeq_get_length as alias of CDR_UnsignedLongLongSeq_get_length */
#define CDR_DoubleSeq_get_length       CDR_UnsignedLongLongSeq_get_length

/*ci \brief Assign CDR_DoubleSeq_set_length as alias of CDR_UnsignedLongLongSeq_set_length */
#define CDR_DoubleSeq_set_length       CDR_UnsignedLongLongSeq_set_length

/*ci \brief Assign CDR_DoubleSeq_get_reference as alias of CDR_UnsignedLongLongSeq_get_reference */
#define CDR_DoubleSeq_get_reference    (CDR_Double*)CDR_UnsignedLongLongSeq_get_reference

#ifndef RTI_CERT

/*ci \brief Assign CDR_BooleanSeq_finalize as alias of CDR_OctetSeq_finalize */
#define CDR_BooleanSeq_finalize        CDR_OctetSeq_finalize

/*ci \brief Assign CDR_CharSeq_finalize as alias of CDR_OctetSeq_finalize */
#define CDR_CharSeq_finalize           CDR_OctetSeq_finalize

/*ci \brief Assign CDR_ShortSeq_finalize as alias of CDR_UnsignedShortSeq_finalize */
#define CDR_ShortSeq_finalize          CDR_UnsignedShortSeq_finalize

/*ci \brief Assign CDR_LongSeq_finalize as alias of CDR_UnsignedLongSeq_finalize */
#define CDR_LongSeq_finalize           CDR_UnsignedLongSeq_finalize

/*ci \brief Assign CDR_EnumSeq_finalize as alias of CDR_UnsignedLongSeq_finalize */
#define CDR_EnumSeq_finalize           CDR_UnsignedLongSeq_finalize

/*ci \brief Assign CDR_WcharSeq_finalize as alias of CDR_UnsignedLongSeq_finalize */
#define CDR_WcharSeq_finalize          CDR_UnsignedLongSeq_finalize

/*ci \brief Assign CDR_FloatSeq_finalize as alias of CDR_UnsignedLongSeq_finalize */
#define CDR_FloatSeq_finalize          CDR_UnsignedLongSeq_finalize

/*ci \brief Assign CDR_LongLongSeq_finalize as alias of CDR_UnsignedLongLongSeq_finalize */
#define CDR_LongLongSeq_finalize       CDR_UnsignedLongLongSeq_finalize

/*ci \brief Assign CDR_DoubleSeq_finalize as alias of CDR_UnsignedLongLongSeq_finalize */
#define CDR_DoubleSeq_finalize         CDR_UnsignedLongLongSeq_finalize
#endif /* !RTI_CERT */

/*ci \brief Assign CDR_BooleanSeq_copy as alias of CDR_OctetSeq_copy */
#define CDR_BooleanSeq_copy            CDR_OctetSeq_copy

/*ci \brief Assign CDR_CharSeq_copy as alias of CDR_OctetSeq_copy */
#define CDR_CharSeq_copy               CDR_OctetSeq_copy

/*ci \brief Assign CDR_ShortSeq_copy as alias of CDR_UnsignedShortSeq_copy */
#define CDR_ShortSeq_copy              CDR_UnsignedShortSeq_copy

/*ci \brief Assign CDR_LongSeq_copy as alias of CDR_UnsignedLongSeq_copy */
#define CDR_LongSeq_copy               CDR_UnsignedLongSeq_copy

/*ci \brief Assign CDR_EnumSeq_copy as alias of CDR_UnsignedLongSeq_copy */
#define CDR_EnumSeq_copy               CDR_UnsignedLongSeq_copy

/*ci \brief Assign CDR_WcharSeq_copy as alias of CDR_UnsignedLongSeq_copy */
#define CDR_WcharSeq_copy              CDR_UnsignedLongSeq_copy

/*ci \brief Assign CDR_FloatSeq_copy as alias of CDR_UnsignedLongSeq_copy */
#define CDR_FloatSeq_copy              CDR_UnsignedLongSeq_copy

/*ci \brief Assign CDR_LongLongSeq_copy as alias of CDR_UnsignedLongLongSeq_copy */
#define CDR_LongLongSeq_copy           CDR_UnsignedLongLongSeq_copy

/*ci \brief Assign CDR_DoubleSeq_copy as alias of CDR_UnsignedLongLongSeq_copy */
#define CDR_DoubleSeq_copy             CDR_UnsignedLongLongSeq_copy


/* Define the following aliases if the DEFAULT API is > BASIC */
#if REDA_SEQUENCE_API_USER_DEFAULT > REDA_SEQUENCE_API_BASIC


/*ci \brief Assign CDR_BooleanSeq_is_equal as alias of CDR_OctetSeq_is_equal */
#define CDR_BooleanSeq_is_equal                 CDR_OctetSeq_is_equal

/*ci \brief Assign CDR_BooleanSeq_loan_contiguous as alias of CDR_OctetSeq_loan_contiguous */
#define CDR_BooleanSeq_loan_contiguous          CDR_OctetSeq_loan_contiguous

/*ci \brief Assign CDR_BooleanSeq_loan_discontiguous as alias of CDR_OctetSeq_loan_discontiguous */
#define CDR_BooleanSeq_loan_discontiguous       CDR_OctetSeq_loan_discontiguous

/*ci \brief Assign CDR_BooleanSeq_unloan as alias of CDR_OctetSeq_unloan */
#define CDR_BooleanSeq_unloan                   CDR_OctetSeq_unloan

/*ci \brief Assign CDR_BooleanSeq_has_ownership as alias of CDR_OctetSeq_has_ownership */
#define CDR_BooleanSeq_has_ownership            CDR_OctetSeq_has_ownership

/*ci \brief Assign CDR_BooleanSeq_get_contiguous_buffer as alias of CDR_OctetSeq_get_contiguous_buffer */
#define CDR_BooleanSeq_get_contiguous_buffer    (CDR_Boolean*)CDR_OctetSeq_get_contiguous_buffer

/*ci \brief Assign CDR_BooleanSeq_set_contiguous_buffer as alias of CDR_OctetSeq_set_contiguous_buffer */
#define CDR_BooleanSeq_set_contiguous_buffer(self_,buff_)  CDR_OctetSeq_set_contiguous_buffer(self_,(CDR_Octet*)(buff_))

/*ci \brief Assign CDR_BooleanSeq_has_discontiguous_buffer as alias of CDR_OctetSeq_has_discontiguous_buffer */
#define CDR_BooleanSeq_has_discontiguous_buffer CDR_OctetSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_BooleanSeq_ensure_length as alias of CDR_OctetSeq_ensure_length */
#define CDR_BooleanSeq_ensure_length            CDR_OctetSeq_ensure_length

/*ci \brief Assign CDR_BooleanSeq_to_array as alias of CDR_OctetSeq_to_array */
#define CDR_BooleanSeq_to_array(seq_,elms_seq_,length_) CDR_OctetSeq_to_array(seq_,(CDR_Octet*)elms_seq_,length_)

/*ci \brief Assign CDR_BooleanSeq_from_array as alias of CDR_OctetSeq_from_array */
#define CDR_BooleanSeq_from_array(seq_,elms_seq_,length_)  CDR_OctetSeq_from_array(seq_,(const CDR_Octet*)elms_seq_,length_)

/* CDR_CharSeq operations */

/*ci \brief Assign CDR_CharSeq_is_equal as alias of CDR_OctetSeq_is_equal */
#define CDR_CharSeq_is_equal                    CDR_OctetSeq_is_equal

/*ci \brief Assign CDR_CharSeq_loan_contiguous as alias of CDR_OctetSeq_loan_contiguous */
#define CDR_CharSeq_loan_contiguous             CDR_OctetSeq_loan_contiguous

/*ci \brief Assign CDR_CharSeq_loan_discontiguous as alias of CDR_OctetSeq_loan_discontiguous */
#define CDR_CharSeq_loan_discontiguous          CDR_OctetSeq_loan_discontiguous

/*ci \brief Assign CDR_CharSeq_unloan as alias of CDR_OctetSeq_unloan */
#define CDR_CharSeq_unloan                      CDR_OctetSeq_unloan

/*ci \brief Assign CDR_CharSeq_has_ownership as alias of CDR_OctetSeq_has_ownership */
#define CDR_CharSeq_has_ownership               CDR_OctetSeq_has_ownership

/*ci \brief Assign CDR_CharSeq_get_contiguous_buffer as alias of CDR_OctetSeq_get_contiguous_buffer */
#define CDR_CharSeq_get_contiguous_buffer      (CDR_Char*)CDR_OctetSeq_get_contiguous_buffer

/*ci \brief Assign CDR_CharSeq_set_buffer as alias of CDR_OctetSeq_set_buffer */
#define CDR_CharSeq_set_contiguous_buffer(self_,buff_) CDR_OctetSeq_set_contiguous_buffer(self_,(CDR_Octet*)(buff_))

/*ci \brief Assign CDR_CharSeq_has_discontiguous_buffer as alias of CDR_OctetSeq_has_discontiguous_buffer */
#define CDR_CharSeq_has_discontiguous_buffer    CDR_OctetSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_CharSeq_ensure_length as alias of CDR_OctetSeq_ensure_length */
#define CDR_CharSeq_ensure_length               CDR_OctetSeq_ensure_length

/*ci \brief Assign CDR_CharSeq_to_array as alias of CDR_OctetSeq_to_array */
#define CDR_CharSeq_to_array(seq_,elms_seq_,length_)  CDR_OctetSeq_to_array(seq_,(CDR_Octet*)elms_seq_,length_)

/*ci \brief Assign CDR_CharSeq_from_array as alias of CDR_OctetSeq_from_array */
#define CDR_CharSeq_from_array(seq_,elms_seq_,length_)  CDR_OctetSeq_from_array(seq_,(const CDR_Octet*)elms_seq_,length_)

/* CDR_ShortSeq operations */

/*ci \brief Assign CDR_ShortSeq_is_equal as alias of CDR_UnsignedShortSeq_is_equal */
#define CDR_ShortSeq_is_equal                   CDR_UnsignedShortSeq_is_equal

/*ci \brief Assign CDR_ShortSeq_loan_contiguous as alias of CDR_UnsignedShortSeq_loan_contiguous */
#define CDR_ShortSeq_loan_contiguous            CDR_UnsignedShortSeq_loan_contiguous

/*ci \brief Assign CDR_ShortSeq_loan_discontiguous as alias of CDR_UnsignedShortSeq_loan_discontiguous */
#define CDR_ShortSeq_loan_discontiguous         CDR_UnsignedShortSeq_loan_discontiguous

/*ci \brief Assign CDR_ShortSeq_unloan as alias of CDR_UnsignedShortSeq_unloan */
#define CDR_ShortSeq_unloan                     CDR_UnsignedShortSeq_unloan

/*ci \brief Assign CDR_ShortSeq_has_ownership as alias of CDR_UnsignedShortSeq_has_ownership */
#define CDR_ShortSeq_has_ownership              CDR_UnsignedShortSeq_has_ownership

/*ci \brief Assign CDR_ShortSeq_get_contiguous_buffer as alias of CDR_UnsignedShortSeq_get_contiguous_buffer */
#define CDR_ShortSeq_get_contiguous_buffer      (CDR_Short*)CDR_UnsignedShortSeq_get_contiguous_buffer

/*ci \brief Assign CDR_ShortSeq_set_contiguous_buffer as alias of CDR_UnsignedShortSeq_set_contiguous_buffer */
#define CDR_ShortSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedShortSeq_set_contiguous_buffer(self_,(CDR_UnsignedShort*)(buff_))

/*ci \brief Assign CDR_ShortSeq_has_discontiguous_buffer as alias of CDR_UnsignedShortSeq_has_discontiguous_buffer */
#define CDR_ShortSeq_has_discontiguous_buffer   CDR_UnsignedShortSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_ShortSeq_ensure_length as alias of CDR_UnsignedShortSeq_ensure_length */
#define CDR_ShortSeq_ensure_length              CDR_UnsignedShortSeq_ensure_length

/*ci \brief Assign CDR_ShortSeq_to_array as alias of CDR_UnsignedShortSeq_to_array */
#define CDR_ShortSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedShortSeq_to_array(seq_,(CDR_UnsignedShort*)elms_seq_,length_)

/*ci \brief Assign CDR_ShortSeq_from_array as alias of CDR_UnsignedShortSeq_from_array */
#define CDR_ShortSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedShortSeq_from_array(seq_,(const CDR_UnsignedShort*)elms_seq_,length_)

/* CDR_LongSeq operations */

/*ci \brief Assign CDR_LongSeq_is_equal as alias of CDR_UnsignedLongSeq_is_equal */
#define CDR_LongSeq_is_equal                    CDR_UnsignedLongSeq_is_equal

/*ci \brief Assign CDR_LongSeq_loan_contiguous as alias of CDR_UnsignedLongSeq_loan_contiguous */
#define CDR_LongSeq_loan_contiguous             CDR_UnsignedLongSeq_loan_contiguous

/*ci \brief Assign CDR_LongSeq_loan_discontiguous as alias of CDR_UnsignedLongSeq_loan_discontiguous */
#define CDR_LongSeq_loan_discontiguous          CDR_UnsignedLongSeq_loan_discontiguous

/*ci \brief Assign CDR_LongSeq_unloan as alias of CDR_UnsignedLongSeq_unloan */
#define CDR_LongSeq_unloan                      CDR_UnsignedLongSeq_unloan

/*ci \brief Assign CDR_LongSeq_has_ownership as alias of CDR_UnsignedLongSeq_has_ownership */
#define CDR_LongSeq_has_ownership               CDR_UnsignedLongSeq_has_ownership

/*ci \brief Assign CDR_LongSeq_get_contiguous_buffer as alias of CDR_UnsignedLongSeq_get_contiguous_buffer */
#define CDR_LongSeq_get_contiguous_buffer       (CDR_Long *)CDR_UnsignedLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_LongSeq_set_contiguous_buffer as alias of CDR_UnsignedLongSeq_set_contiguous_buffer */
#define CDR_LongSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLong*)(buff_))

/*ci \brief Assign CDR_LongSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongSeq_has_discontiguous_buffer */
#define CDR_LongSeq_has_discontiguous_buffer    CDR_UnsignedLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_LongSeq_ensure_length as alias of CDR_UnsignedLongSeq_ensure_length */
#define CDR_LongSeq_ensure_length               CDR_UnsignedLongSeq_ensure_length

/*ci \brief Assign CDR_LongSeq_to_array as alias of CDR_UnsignedLongSeq_to_array */
#define CDR_LongSeq_to_array(seq_,elms_seq_,length_)   CDR_UnsignedLongSeq_to_array(seq_,(CDR_UnsignedLong*)elms_seq_,length_)

/*ci \brief Assign CDR_LongSeq_from_array as alias of CDR_UnsignedLongSeq_from_array */
#define CDR_LongSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_from_array(seq_,(const CDR_UnsignedLong*)elms_seq_,length_)

/* CDR_EnumSeq operations */

/*ci \brief Assign CDR_EnumSeq_is_equal as alias of CDR_UnsignedLongSeq_is_equal */
#define CDR_EnumSeq_is_equal                    CDR_UnsignedLongSeq_is_equal

/*ci \brief Assign CDR_EnumSeq_loan_contiguous as alias of CDR_UnsignedLongSeq_loan_contiguous */
#define CDR_EnumSeq_loan_contiguous             CDR_UnsignedLongSeq_loan_contiguous

/*ci \brief Assign CDR_EnumSeq_loan_discontiguous as alias of CDR_UnsignedLongSeq_loan_discontiguous */
#define CDR_EnumSeq_loan_discontiguous          CDR_UnsignedLongSeq_loan_discontiguous

/*ci \brief Assign CDR_EnumSeq_unloan as alias of CDR_UnsignedLongSeq_unloan */
#define CDR_EnumSeq_unloan                      CDR_UnsignedLongSeq_unloan

/*ci \brief Assign CDR_EnumSeq_has_ownership as alias of CDR_UnsignedLongSeq_has_ownership */
#define CDR_EnumSeq_has_ownership               CDR_UnsignedLongSeq_has_ownership

/*ci \brief Assign CDR_EnumSeq_get_contiguous_buffer as alias of CDR_UnsignedLongSeq_get_contiguous_buffer */
#define CDR_EnumSeq_get_contiguous_buffer       (CDR_Enum*)CDR_UnsignedLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_EnumSeq_set_contiguous_buffer as alias of CDR_UnsignedLongSeq_set_contiguous_buffer */
#define CDR_EnumSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLong*)(buff_))

/*ci \brief Assign CDR_EnumSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongSeq_has_discontiguous_buffer */
#define CDR_EnumSeq_has_discontiguous_buffer    CDR_UnsignedLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_EnumSeq_ensure_length as alias of CDR_UnsignedLongSeq_ensure_length */
#define CDR_EnumSeq_ensure_length               CDR_UnsignedLongSeq_ensure_length

/*ci \brief Assign CDR_EnumSeq_to_array as alias of CDR_UnsignedLongSeq_to_array */
#define CDR_EnumSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_to_array(seq_,(CDR_UnsignedLong*)elms_seq_,length_)

/*ci \brief Assign CDR_EnumSeq_from_array as alias of CDR_UnsignedLongSeq_from_array */
#define CDR_EnumSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_from_array(seq_,(const CDR_UnsignedLong*)elms_seq_,length_)

/* CDR_WcharSeq operations */

/*ci \brief Assign CDR_WcharSeq_is_equal as alias of CDR_UnsignedLongSeq_is_equal */
#define CDR_WcharSeq_is_equal                   CDR_UnsignedLongSeq_is_equal

/*ci \brief Assign CDR_WcharSeq_loan_contiguous as alias of CDR_UnsignedLongSeq_loan_contiguous */
#define CDR_WcharSeq_loan_contiguous            CDR_UnsignedLongSeq_loan_contiguous

/*ci \brief Assign CDR_WcharSeq_loan_discontiguous as alias of CDR_UnsignedLongSeq_loan_discontiguous */
#define CDR_WcharSeq_loan_discontiguous         CDR_UnsignedLongSeq_loan_discontiguous

/*ci \brief Assign CDR_WcharSeq_unloan as alias of CDR_UnsignedLongSeq_unloan */
#define CDR_WcharSeq_unloan                     CDR_UnsignedLongSeq_unloan

/*ci \brief Assign CDR_WcharSeq_has_ownership as alias of CDR_UnsignedLongSeq_has_ownership */
#define CDR_WcharSeq_has_ownership              CDR_UnsignedLongSeq_has_ownership

/*ci \brief Assign CDR_WcharSeq_get_contiguous_buffer as alias of CDR_UnsignedLongSeq_get_contiguous_buffer */
#define CDR_WcharSeq_get_contiguous_buffer      (CDR_Wchar *)CDR_UnsignedLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_WcharSeq_set_contiguous_buffer as alias of CDR_UnsignedLongSeq_set_contiguous_buffer */
#define CDR_WcharSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLong*)(buff_))

/*ci \brief Assign CDR_WcharSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongSeq_has_discontiguous_buffer */
#define CDR_WcharSeq_has_discontiguous_buffer   CDR_UnsignedLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_WcharSeq_ensure_length as alias of CDR_UnsignedLongSeq_ensure_length */
#define CDR_WcharSeq_ensure_length               CDR_UnsignedLongSeq_ensure_length

/*ci \brief Assign CDR_WcharSeq_to_array as alias of CDR_UnsignedLongSeq_to_array */
#define CDR_WcharSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_to_array(seq_,(CDR_UnsignedLong*)elms_seq_,length_)

/*ci \brief Assign CDR_WcharSeq_from_array as alias of CDR_UnsignedLongSeq_from_array */
#define CDR_WcharSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_from_array(seq_,(const CDR_UnsignedLong*)elms_seq_,length_)

/* CDR_FloatSeq operations */

/*ci \brief Assign CDR_FloatSeq_is_equal as alias of CDR_UnsignedLongSeq_is_equal */
#define CDR_FloatSeq_is_equal                   CDR_UnsignedLongSeq_is_equal

/*ci \brief Assign CDR_FloatSeq_loan_contiguous as alias of CDR_UnsignedLongSeq_loan_contiguous */
#define CDR_FloatSeq_loan_contiguous            CDR_UnsignedLongSeq_loan_contiguous

/*ci \brief Assign CDR_FloatSeq_loan_discontiguous as alias of CDR_UnsignedLongSeq_loan_discontiguous */
#define CDR_FloatSeq_loan_discontiguous         CDR_UnsignedLongSeq_loan_discontiguous

/*ci \brief Assign CDR_FloatSeq_unloan as alias of CDR_UnsignedLongSeq_unloan */
#define CDR_FloatSeq_unloan                     CDR_UnsignedLongSeq_unloan

/*ci \brief Assign CDR_FloatSeq_has_ownership as alias of CDR_UnsignedLongSeq_has_ownership */
#define CDR_FloatSeq_has_ownership              CDR_UnsignedLongSeq_has_ownership

/*ci \brief Assign CDR_FloatSeq_get_contiguous_buffer as alias of CDR_UnsignedLongSeq_get_contiguous_buffer */
#define CDR_FloatSeq_get_contiguous_buffer      (CDR_Float*)CDR_UnsignedLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_FloatSeq_set_contiguous_buffer as alias of CDR_UnsignedLongSeq_set_contiguous_buffer */
#define CDR_FloatSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLong*)(buff_))

/*ci \brief Assign CDR_FloatSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongSeq_has_discontiguous_buffer */
#define CDR_FloatSeq_has_discontiguous_buffer   CDR_UnsignedLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_FloatSeq_ensure_length as alias of CDR_UnsignedLongSeq_ensure_length */
#define CDR_FloatSeq_ensure_length               CDR_UnsignedLongSeq_ensure_length

/*ci \brief Assign CDR_FloatSeq_to_array as alias of CDR_UnsignedLongSeq_to_array */
#define CDR_FloatSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_to_array(seq_,(CDR_UnsignedLong*)elms_seq_,length_)

/*ci \brief Assign CDR_FloatSeq_from_array as alias of CDR_UnsignedLongSeq_from_array */
#define CDR_FloatSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongSeq_from_array(seq_,(const CDR_UnsignedLong*)elms_seq_,length_)

/* CDR_LongLongSeq operations */

/*ci \brief Assign CDR_LongLongSeq_is_equal as alias of CDR_UnsignedLongLongSeq_is_equal */
#define CDR_LongLongSeq_is_equal                 CDR_UnsignedLongLongSeq_is_equal

/*ci \brief Assign CDR_LongLongSeq_loan_contiguous as alias of CDR_UnsignedLongLongSeq_loan_contiguous */
#define CDR_LongLongSeq_loan_contiguous          CDR_UnsignedLongLongSeq_loan_contiguous

/*ci \brief Assign CDR_LongLongSeq_loan_discontiguous as alias of CDR_UnsignedLongLongSeq_loan_discontiguous */
#define CDR_LongLongSeq_loan_discontiguous       CDR_UnsignedLongLongSeq_loan_discontiguous

/*ci \brief Assign CDR_LongLongSeq_unloan as alias of CDR_UnsignedLongLongSeq_unloan */
#define CDR_LongLongSeq_unloan                   CDR_UnsignedLongLongSeq_unloan

/*ci \brief Assign CDR_LongLongSeq_has_ownership as alias of CDR_UnsignedLongLongSeq_has_ownership */
#define CDR_LongLongSeq_has_ownership            CDR_UnsignedLongLongSeq_has_ownership

/*ci \brief Assign CDR_LongLongSeq_get_contiguous_buffer as alias of CDR_UnsignedLongLongSeq_get_contiguous_buffer */
#define CDR_LongLongSeq_get_contiguous_buffer    (CDR_LongLong*)CDR_UnsignedLongLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_LongLongSeq_set_contiguous_buffer as alias of CDR_UnsignedLongLongSeq_set_contiguous_buffer */
#define CDR_LongLongSeq_set_contiguous_buffer(self_,buff_) CDR_UnsignedLongLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLongLong*)(buff_))

/*ci \brief Assign CDR_LongLongSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongLongSeq_has_discontiguous_buffer */
#define CDR_LongLongSeq_has_discontiguous_buffer CDR_UnsignedLongLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_LongLongSeq_ensure_length as alias of CDR_UnsignedLongLongSeq_ensure_length */
#define CDR_LongLongSeq_ensure_length            CDR_UnsignedLongLongSeq_ensure_length

/*ci \brief Assign CDR_LongLongSeq_to_array as alias of CDR_UnsignedLongLongSeq_to_array */
#define CDR_LongLongSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedLongLongSeq_to_array(seq_,(CDR_UnsignedLongLong*)elms_seq_,length_)

/*ci \brief Assign CDR_LongLongSeq_from_array as alias of CDR_UnsignedLongLongSeq_from_array */
#define CDR_LongLongSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongLongSeq_from_array(seq_,(const CDR_UnsignedLongLong*)elms_seq_,length_)

/* CDR_DoubleSeq operations */

/*ci \brief Assign CDR_DoubleSeq_is_equal as alias of CDR_UnsignedLongLongSeq_is_equal */
#define CDR_DoubleSeq_is_equal                   CDR_UnsignedLongLongSeq_is_equal

/*ci \brief Assign CDR_DoubleSeq_loan_contiguous as alias of CDR_UnsignedLongLongSeq_loan_contiguous */
#define CDR_DoubleSeq_loan_contiguous            CDR_UnsignedLongLongSeq_loan_contiguous

/*ci \brief Assign CDR_DoubleSeq_loan_discontiguous as alias of CDR_UnsignedLongLongSeq_loan_discontiguous */
#define CDR_DoubleSeq_loan_discontiguous         CDR_UnsignedLongLongSeq_loan_discontiguous

/*ci \brief Assign CDR_DoubleSeq_unloan as alias of CDR_UnsignedLongLongSeq_unloan */
#define CDR_DoubleSeq_unloan                     CDR_UnsignedLongLongSeq_unloan

/*ci \brief Assign CDR_DoubleSeq_has_ownership as alias of CDR_UnsignedLongLongSeq_has_ownership */
#define CDR_DoubleSeq_has_ownership              CDR_UnsignedLongLongSeq_has_ownership

/*ci \brief Assign CDR_DoubleSeq_get_contiguous_buffer as alias of CDR_UnsignedLongLongSeq_get_contiguous_buffer */
#define CDR_DoubleSeq_get_contiguous_buffer      (CDR_Double *)CDR_UnsignedLongLongSeq_get_contiguous_buffer

/*ci \brief Assign CDR_DoubleSeq_set_contiguous_buffer as alias of CDR_UnsignedLongLongSeq_set_contiguous_buffer */
#define CDR_DoubleSeq_set_contiguous_buffer(self_,buff_)    CDR_UnsignedLongLongSeq_set_contiguous_buffer(self_,(CDR_UnsignedLongLong*)(buff_))

/*ci \brief Assign CDR_DoubleSeq_has_discontiguous_buffer as alias of CDR_UnsignedLongLongSeq_has_discontiguous_buffer */
#define CDR_DoubleSeq_has_discontiguous_buffer   CDR_UnsignedLongLongSeq_has_discontiguous_buffer

/*ci \brief Assign CDR_DoubleSeq_ensure_length as alias of CDR_UnsignedLongLongSeq_ensure_length */
#define CDR_DoubleSeq_ensure_length              CDR_UnsignedLongLongSeq_ensure_length

/*ci \brief Assign CDR_DoubleSeq_to_array as alias of CDR_UnsignedLongLongSeq_to_array */
#define CDR_DoubleSeq_to_array(seq_,elms_seq_,length_) CDR_UnsignedLongLongSeq_to_array(seq_,(CDR_UnsignedLongLong*)elms_seq_,length_)

/*ci \brief Assign CDR_DoubleSeq_from_array as alias of CDR_UnsignedLongLongSeq_from_array */
#define CDR_DoubleSeq_from_array(seq_,elms_seq_,length_) CDR_UnsignedLongLongSeq_from_array(seq_,(const CDR_UnsignedLongLong*)elms_seq_,length_)

#endif

#endif /* REDA_SEQUENCE_API_USER_DEFAULT > REDA_SEQUENCE_API_BASIC */

#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL
#error "REDA_SEQUENCE_API_UNTYPED <= REDA_SEQUENCE_API <= REDA_SEQUENCE_API_FULL"
#endif

/* ================================================================= */
/*          CDR Primitive Type Enumeration and Attributes            */
/* ================================================================= */
/*ci \brief Primitive type enum */
typedef enum
{
    CDR_CHAR_TYPE,
    CDR_WCHAR_TYPE,
    CDR_OCTET_TYPE,
    CDR_SHORT_TYPE,
    CDR_UNSIGNED_SHORT_TYPE,
    CDR_LONG_TYPE,
    CDR_UNSIGNED_LONG_TYPE,
    CDR_LONG_LONG_TYPE,
    CDR_UNSIGNED_LONG_LONG_TYPE,
    CDR_FLOAT_TYPE,
    CDR_DOUBLE_TYPE,
    CDR_LONG_DOUBLE_TYPE,
    CDR_BOOLEAN_TYPE,
    CDR_ENUM_TYPE
} CdrPrimitiveType;

/*ci \brief Boolean type size */
#define CDR_BOOLEAN_SIZE      1U

/*ci \brief Octet type size */
#define CDR_OCTET_SIZE        1U

/*ci \brief Char type size */
#define CDR_CHAR_SIZE         1U

/*ci \brief Short type size */
#define CDR_SHORT_SIZE        2U

/*ci \brief Unsigned Short type size */
#define CDR_UNSIGNED_SHORT_SIZE 2U

/*ci \brief Long type size */
#define CDR_LONG_SIZE         4U

/*ci \brief Unsigned Long type size */
#define CDR_UNSIGNED_LONG_SIZE  4U

/*ci \brief Float type size */
#define CDR_FLOAT_SIZE        4U

/*ci \brief WChar type size */
#define CDR_WCHAR_SIZE        4U

/*ci \brief Enum type size */
#define CDR_ENUM_SIZE         4U

/*ci \brief Long Long type size */
#define CDR_LONG_LONG_SIZE    8U

/*ci \brief Unsigned Long Long type size */
#define CDR_UNSIGNED_LONG_LONG_SIZE 8U

/*ci \brief Double type size */
#define CDR_DOUBLE_SIZE       8U

/*ci \brief Long Double type size */
#define CDR_LONG_DOUBLE_SIZE  16U

/******************************************************************************/
/*ci \brief Octet type alignment */
#define CDR_OCTET_ALIGN       1U

/*ci \brief Short type alignment */
#define CDR_SHORT_ALIGN       2U

/*ci \brief Long type alignment */
#define CDR_LONG_ALIGN        4U

/*ci \brief Float type alignment */
#define CDR_FLOAT_ALIGN       4U

/*ci \brief Long Long type alignment */
#define CDR_LONG_LONG_ALIGN   8U

/*ci \brief Double type alignment */
#define CDR_DOUBLE_ALIGN      8U

/*ci \brief Long Double type alignment */
#define CDR_LONG_DOUBLE_ALIGN 8U

/*ci \brief Get alignment of a given type
 *
 * \param[in] type Type to get alignment
 *
 * \return Alignment in bytes of input type
 */
RTI_UINT8
CDR_Primitive_get_alignment(CdrPrimitiveType type);


/*ci \brief Get size of a given type
 *
 * \param[in] type Type to get size
 *
 * \return Size in bytes of input type
 */
RTI_UINT32
CDR_Primitive_get_size(CdrPrimitiveType type);


/* ================================================================= */
/*               Type Specific Initialization & Copy                 */
/* ================================================================= */
/* not all types are atomic so we define macros to emulate as needed */

/*ci \brief Initialize Char
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_char(value)              *(value) = 0

/*ci \brief Initialize Octet
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_octet(value)             *(value) = 0

/*ci \brief Initialize Boolean
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_boolean(value)           *(value) = 0

/*ci \brief Initialize Short
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_short(value)             *(value) = 0

/*ci \brief Initialize Unsigned Short
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_unsigned_short(value)     *(value) = 0

/*ci \brief Initialize Long
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_long(value)              *(value) = 0

/*ci \brief Initialize Unsigned Long
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_unsigned_long(value)      *(value) = 0

/*ci \brief Initialize WChar
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_wchar(value)             *(value) = 0

/*ci \brief Initialize Enum
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_enum(value)              *(value) = 0

/*ci \brief Initialize Float
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_float(value)             *(value) = 0

/*ci \brief Initialize Long Long
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_long_long(value)          *(value) = 0

/*ci \brief Initialize Unsigned Long Long
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_unsigned_long_long(value)  *(value) = 0

/*ci \brief Initialize Double
 * \param[inout] value Pointer to value to initialize
 */
#define CDR_Primitive_init_double(value)            *(value) = 0

/*ci \brief Initialize Long Double
 * \param[inout] value Pointer to value to initialize
 */
#ifdef RTI_HAVE_LONG_DOUBLE
#define CDR_Primitive_init_long_double(value)        *(value) = 0
#else
#define CDR_Primitive_init_long_double(value) \
    OSAPI_Memory_zero(value,sizeof(RTI_DOUBLE128))
#endif

/*ci \brief Initialize String
 * \param[inout] value Pointer to String to initialize
 * \param[in] maxsize Maximum string size
 */
#define CDR_Primitive_init_string(value,maxsize) \
    OSAPI_Memory_zero(value,maxsize)

/*ci \brief Initialize WString
 * \param[inout] value Pointer to WString to initialize
 * \param[in] maxsize Maximum string size
 */
#define CDR_Primitive_init_wstring(value,maxsize) \
    OSAPI_Memory_zero(value,((maxsize)*sizeof(CDR_Wchar)))

/*ci \brief Initialize Array
 * \param[inout] value Pointer to array to initialize
 * \param[in] maxsize Maximum Array length
 */
#define CDR_Primitive_init_array(value,maxsize) \
    OSAPI_Memory_zero(value,maxsize)

/******************************************************************************/
/*ci \brief Copy Char
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_char(dst,src)            *(dst) = *(src)

/*ci \brief Copy Octet
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_octet(dst,src)           *(dst) = *(src)

/*ci \brief Copy Boolean
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_boolean(dst,src)         *(dst) = *(src)

/*ci \brief Copy Short
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_short(dst,src)           *(dst) = *(src)

/*ci \brief Copy Unsigned Short
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_unsigned_short(dst,src)   *(dst) = *(src)

/*ci \brief Copy Long
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_long(dst,src)            *(dst) = *(src)

/*ci \brief Copy Unsigned Long
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_unsigned_long(dst,src)    *(dst) = *(src)

/*ci \brief Copy WChar
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_wchar(dst,src)           *(dst) = *(src)

/*ci \brief Copy Enum
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_enum(dst,src)            *(dst) = *(src)

/*ci \brief Copy Float
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_float(dst,src)           *(dst) = *(src)

/*ci \brief Copy Long Long
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_long_long(dst,src)        *(dst) = *(src)

/*ci \brief Copy Unsigned Long Long
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_unsigned_long_long(dst,src) *(dst) = *(src)

/*ci \brief Copy Double
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_double(dst,src)          *(dst) = *(src)

/*ci \brief Copy Long Double
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#ifdef RTI_HAVE_LONG_DOUBLE
#define CDR_Primitive_copy_long_double(dst,src)      *(dst) = *(src)
#else
#define CDR_Primitive_copy_long_double(dst,src) \
    OSAPI_Memory_copy((void*)dst,(void*)src,sizeof(RTI_DOUBLE128))
#endif

/*ci \brief Copy String
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_string(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,maxsize)

/*ci \brief Copy WString
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_wstring(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,((maxsize)*CDR_WCHAR_SIZE))

/*ci \brief Copy Array
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define CDR_Primitive_copy_array(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,maxsize)

/*==============================================================================
 * Type Helper Functions and
 * String Sequence support wrapper functions to convert from String* to Char*
 * ===========================================================================*/

/*ci
 * \brief
 * Initialize memory for string
 *
 * \param[inout] string String to initialize
 * \param[in] max_str_len Maximum string length
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_initialize(CDR_String *, RTI_UINT32 max_str_len);

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize string
 *
 * \details
 * Note, CDR_String_finalize is required to return an RTI_BOOL to comply with
 * the expected interface for finalize functions needed by sequence methods
 *
 * \param[inout] string String to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_finalize(CDR_String * string);
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Copy from source to destination string
 *
 * \param[inout] dst Destination string
 * \param[in] src Source string
 * \param[in] max_str_len Maximum length of source string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_copy(CDR_String * dst, const CDR_String * src, RTI_SIZE_T max_str_len);

/*ci
 * \brief
 * Compare two strings
 *
 * \param[in] left Left string
 * \param[in] right Right string
 *
 * \return 1, 0, or -1 if the left string is greater than, equal to, or less
 * than the right string, respectively.
 */
MUST_CHECK_RETURN CDRDllExport RTI_INT32
CDR_String_compare(const CDR_String * left, const CDR_String * right);

/*ci
 * \brief
 * Allocates and initializes memory for a wide char string
 *
 * \param[inout] Wstring to initialize
 * \param[in] max_str_len
 *
 * \return RTI_BOOL on success with wstring initialized, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Wstring_initialize(CDR_Wstring *, RTI_UINT32 max_str_len);

MUST_CHECK_RETURN CDRDllExport CDR_Wstring
CDR_Wstring_initialize_ex(RTI_UINT32 max_str_len);

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize a wide char string
 *
 * \details
 * Note, CDR_Wstring_finalize is required to return an RTI_BOOL to comply with
 * the expected interface for finalize functions needed by sequence methods
 *
 * \param[in] wstring Wstring to finalize
 *
 * \return RTI_TRUE
 */
SHOULD_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Wstring_finalize(CDR_Wstring * wstring);
#endif /* !RTI_CERT */

CDRDllExport void
CDR_Wstring_finalize_ex(CDR_Wstring wstring);

/*ci
 * \brief
 * Returns length in bytes of wide char string
 *
 * \param[in] wstring
 *
 * \return Length of wstring in bytes
 */
CDRDllExport RTI_SIZE_T
CDR_Wstring_length(const CDR_Wchar * wstring);

CDRDllExport RTI_SIZE_T
CDR_Wstring16_length(const RTI_UINT16 *wstring);

/*ci
 * \brief
 * Copy from source to destination wide char string
 *
 * \param[inout] dst Destination string
 * \param[in] src Source string
 * \param[in] max_str_len Maximum length of source string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Wstring_copy(CDR_Wstring * dst, const CDR_Wstring * src,
                 RTI_UINT32 max_str_len);

/*ci
 * \brief
 * Compare two wide char strings
 *
 * \param[in] left Left string
 * \param[in] right Right string
 *
 * \return 1, 0, or -1 if the left string is greater than, equal to, or less
 * than the right string, respectively.
 */
MUST_CHECK_RETURN CDRDllExport RTI_INT32
CDR_Wstring_compare(const CDR_Wstring * left, const CDR_Wstring * right);

/*ci
 * \brief
 * Initialize an array of char or wide char strings
 *
 * \param[inout] value Preallocated buffer for array
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of each string
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success with each string of array initialized,
 * RTI_FALSE on failure
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_StringArray_initialize(void *value, RTI_UINT32 length,
                           RTI_UINT32 max_str_len,
                           CdrPrimitiveType type);

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize an array of char or wide char strings
 *
 * \param[inout] value Array pointer
 * \param[in] length Length of array
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success with each string of array finalized,
 * RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_StringArray_finalize(void *value, RTI_UINT32 length,
                         CdrPrimitiveType type);
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Copy an array of char or wide char strings to another array
 *
 * \param[out] out Destination array
 * \param[in] in Source array
 * \param[in] max_str_len Maximum string length of source
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_StringArray_copy(void *out, const void* in,
                     RTI_UINT32 length,
                     RTI_UINT32 max_str_len,
                     CdrPrimitiveType type);

/*==============================================================================
 * CDR Boolean True/False
 * ===========================================================================*/

#define CDR_BOOLEAN_TRUE    (1)

#define CDR_BOOLEAN_FALSE   (0)

/*==============================================================================
 * CDR Property and CDR Binary Property Types
 * ===========================================================================*/

#define CDR_PROPERTY_NAME_MAX_LEN       255
#define CDR_PROPERTY_VALUE_MAX_LEN      255

/*ci \brief Property */
struct DDSCPPDllExport CDR_Property
{
    /*ci \brief Name */
    CDR_String name;

    /*ci \brief Value */
    CDR_String value;

    /*ci \brief Propagate flag */
    CDR_Boolean propagate;

    CDR_CPP_QOS_METHODS(CDR_Property)
};

#define CDR_Property_INITIALIZER \
{ \
    NULL,               /* name */\
    NULL,               /* value */\
    CDR_BOOLEAN_TRUE    /* propagate */\
}

/* ci
 *
 * \brief
 * Initialize a CDR_Property instance.
 *
 * \details
 * Attributes of the CDR_Property will
 * be initialized to their default values specified by
 * CDR_Property_INITIALIZER
 *
 * \param[out] self the CDR_Property instance to be initialized
 *
 * \result RTI_FALSE if CDR_Property instance passed is correctly
 *         initialized RTI_TRUE otherwise
 */
CDRDllExport RTI_BOOL
CDR_Property_initialize(struct CDR_Property* self);

/* ci
 *
 * \brief
 * Finalize the CDR_Property instance by freeing
 * up memory allocated to it
 *
 * \details
 * The memory allocated to the CDR_Property
 * instance will be freed.
 *
 * \param[out] self - CDR_Property instance to be finalized
 *
 * \result RTI_TRUE if the CDR_Property instance is successfully
 *         finalized RTI_FALSE otherwise.
 */
CDRDllExport RTI_BOOL
CDR_Property_finalize(struct CDR_Property* self);

/* ci
 *
 * \brief
 * Copy attributes of a CDR_Property instance from source to
 * destination.
 *
 * \details
 * Copy the attributes of source CDR_Property instance to destination
 * CDR_Property instance. This function assumes that memory for the
 * source and destination CDR_Property instances are pre-allocated.
 *
 * \param[out]  dst - destination CDR_Property instance
 * \param[in]  src - source CDR_Property instance
 *
 * \result RTI_TRUE if copied successfully, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Property_copy(struct CDR_Property* dst,
                  const struct CDR_Property* src);

/* ci
 *
 * \brief
 * Checks CDR_Property instance for consistency
 *
 * \details
 * The current version checks that the attributes of the CDR_Property
 * instance passed are not NULL
 *
 * \param[in] self - CDR_Property instance that needs to be checked for
 *                   consistency
 *
 * \result RTI_TRUE if the CDR_Property instance is consistent, RTI_FALSE
 *         otherwise.
 *
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Property_is_consistent(const struct CDR_Property *self);

MUST_CHECK_RETURN CDRDllExport RTI_INT32
CDR_Property_compare(const struct CDR_Property *prop1,
                     const struct CDR_Property *prop2);

MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Property_is_equal(const struct CDR_Property *prop1,
                     const struct CDR_Property *prop2);

/*ci \brief Property Sequence */

#define CDR_PROPERTY_SEQ_CPP_EXTRA_METHODS(ptype_,vtype_) \
    public: \
        bool\
        assert_property(const char * const name,\
                        vtype_ value,\
                        bool propagate);\
        ptype_*\
        lookup_property(const char * const name);

#define T struct CDR_Property
#define TSeq CDR_PropertySeq
#define TSeq_is_equal
#define REDA_SEQUENCE_USER_API
#define TSeq_cpp_extra \
        CDR_PROPERTY_SEQ_CPP_EXTRA_METHODS(T,const char * const)
#include <reda/reda_sequence_decl.h>

#define CDR_PropertySeq_INITIALIZER \
            REDA_DEFINE_SEQUENCE_INITIALIZER(struct CDR_Property)

#define CDR_PROPERTY_SEQ_MAX_LEN       10


MUST_CHECK_RETURN CDRDllExport struct CDR_Property*
CDR_PropertySeq_lookup_property_w_prefix(
        const struct CDR_PropertySeq *self,
        const char *const user_prefix,
        const char *const name);

/* ci
 *
 * \brief
 * Look up specific property in CDR_PropertySeq instance
 *
 * \details
 * This function searches a CDR_PropertySequence instance for a
 * CDR_Property with the specified name.
 *
 * \param[in]  self CDR_PropertySequence within which the property
 *                    value pairs are stored.
 * \param[in]  name  the property whose corresponding value is required
 *
 * \return CDR_Property instance with the specified name, if found. NULL
 *         otherwise.
 */
MUST_CHECK_RETURN CDRDllExport struct CDR_Property*
CDR_PropertySeq_lookup_property(
        const struct CDR_PropertySeq *self, const char * const name);

/*ce
 * \brief Checks if the name-value pair of the property is present in the
 * property sequence.  If a value for the specified name is not found, then add the
 * (name, value) pair, with the appropriate propagate flag.
 *
 * If a property of the same name already exist, the value will be over-written with
 * the new value passed to the function call.
 *
 * \param[in] self      pointer to the Property Sequence
 * \param[in] name      name of the property being searched
 * \param[in] value     value to be filled corresponding to the property
 * \param[in] propagate whether this property needs to be propagated over the wire
 *
 * \return Returns RTI_TRUE if the property is successfully asserted, RTI_FALSE
 * otherwise.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_PropertySeq_assert_property(
        struct CDR_PropertySeq *self,
        const char * const name,
        const char * const value,
        CDR_Boolean propagate);

/* ci
 *
 * \brief
 * Check if the CDR_PropertySeq instance has been populated consistently.
 *
 * \details
 *  This function checks that a CDR_PropertySeq has been consistently
 *  populated by a user. A CDR_PropertySeq is considered consistent
 *  if both its name and value are set to non-NULL values.
 *
 * \param[in]  self   CDR_PropertySeq that has to be checked for
 *                    consistency
 *
 * \return RTI_TRUE if the property sequence is consistent,
 * RTI_FALSE otherwise
 */

MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_PropertySeq_is_consistent(const struct CDR_PropertySeq *self);

/* -------- String parsing utilities -------- */

MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_parse_int32(CDR_String *str, CDR_Long *result);

MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_parse_uint32(CDR_String *str, CDR_UnsignedLong *result);

MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_String_parse_bool(CDR_String *str, CDR_Boolean *result);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* cdr_cdr_type_h */

/*ci @} */
