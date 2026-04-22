/*
  (c) Copyright, Real-Time Innovations, 2013-2024.

  All rights reserved.
  No duplications, whole or partial, manual or electronic, may be made
  without express written permission.  Any such copies, or
  revisions thereof, must display this notice unaltered.
  This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
 ------------ -------
 12feb2016,tk MICRO-1529 Added primitive sequences
 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 10feb2015,tk MICRO-1527 Made [] const
 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 17apr2015,eh MICRO-1143 Define destructor for Cert
 16may2014,as MICRO-794  Remove C++ TODO and commented out code
 19jul2013,as Created
==============================================================================*/

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif

#define concatenate(A, B)   A ## B

#ifndef T
#define T RTI_UINT32
#endif

#ifdef REDA_SEQUENCE_USER_API
#ifndef REDA_SEQUENCE_API
#define REDA_SEQUENCE_API    REDA_SEQUENCE_API_USER_DEFAULT
#else
#if REDA_SEQUENCE_API < REDA_SEQUENCE_API_BASIC
#error "REDA_SEQUENCE_USER_API defined along with API level < REDA_SEQUENCE_API_BASIC"
#endif
#endif
#endif /* REDA_SEQUENCE_USER_API */

#ifndef REDA_SEQUENCE_API
#define REDA_SEQUENCE_API REDA_SEQUENCE_API_DEFAULT
#endif /* REDA_SEQUENCE_API */

#if REDA_SEQUENCE_API < REDA_SEQUENCE_API_UNTYPED
#error "REDA_SEQUENCE_API_UNTYPED <= REDA_SEQUENCE_API <= REDA_SEQUENCE_API_FULL"
#endif

/* This corresponds to the BASIC API, the lowest level */
#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_UNTYPED
#ifndef TSeq_initialize
#define TSeq_initialize
#endif
#ifndef TSeq_get_maximum
#define TSeq_get_maximum
#endif
#ifndef TSeq_set_maximum
#define TSeq_set_maximum
#endif
#ifndef TSeq_get_length
#define TSeq_get_length
#endif
#ifndef TSeq_set_length
#define TSeq_set_length
#endif
#ifndef TSeq_get_reference
#define TSeq_get_reference
#endif
#ifndef RTI_CERT
#ifndef TSeq_finalize
#define TSeq_finalize
#endif
#endif /* RTI_CERT */
#ifdef REDA_SEQUENCE_USER_API
#ifndef TSeq_copy
#define TSeq_copy
#endif
#endif /* REDA_SEQUENCE_USER_API */
#endif /* REDA_SEQUENCE_API > REDA_SEQUENCE_API_UNTYPED */

/* The FULL API adds the following methods */
#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_BASIC
#ifndef TSeq_copy
#define TSeq_copy
#endif
#ifndef TSeq_is_equal
#define TSeq_is_equal
#endif
#ifndef TSeq_loan_contiguous
#define TSeq_loan_contiguous
#endif
#ifndef TSeq_loan_discontiguous
#define TSeq_loan_discontiguous
#endif
#ifndef TSeq_has_ownership
#define TSeq_has_ownership
#endif
#ifndef TSeq_has_discontiguous_buffer
#define TSeq_has_discontiguous_buffer
#endif
#ifndef TSeq_get_contiguous_buffer
#define TSeq_get_contiguous_buffer
#endif
#ifndef TSeq_get_discontiguous_buffer
#define TSeq_get_discontiguous_buffer
#endif
#ifndef TSeq_set_contiguous_buffer
#define TSeq_set_contiguous_buffer
#endif
#ifndef TSeq_unloan
#define TSeq_unloan
#endif
#ifndef TSeq_set_token
#define TSeq_set_token
#endif
#ifndef TSeq_get_token
#define TSeq_get_token
#endif
#ifndef TSeq_ensure_length
#define TSeq_ensure_length
#endif
#ifndef TSeq_from_array
#define TSeq_from_array
#endif
#ifndef TSeq_to_array
#define TSeq_to_array
#endif
#endif /* REDA_SEQUENCE_API > REDA_SEQUENCE_API_BASIC */

#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL
#error "REDA_SEQUENCE_API_UNTYPED <= REDA_SEQUENCE_API <= REDA_SEQUENCE_API_FULL"
#endif

TSeq::~TSeq()
{
#ifdef TSeq_finalize
#undef TSeq_finalize
#define _finalize(TSeq) concatenate(TSeq, _finalize)
#define TSeq_finalize _finalize(TSeq)
    /* coverity[check_return : FALSE] */
    TSeq_finalize(this);
#undef _finalize
#undef TSeq_finalize
#endif /* TSeq_finalize */
}


#ifdef TSeq_get_maximum
#undef TSeq_get_maximum
#define _get_maximum(TSeq) concatenate(TSeq, _get_maximum)
#define TSeq_get_maximum _get_maximum(TSeq)
/* Define method (get) maximum() of C++ TSeq class */
RTI_INT32
TSeq::maximum() const
{
    return TSeq_get_maximum(this);
}
#undef _get_maximum
#undef TSeq_get_maximum
#endif /* TSeq_get_maximum */

#ifdef TSeq_set_maximum
#undef TSeq_set_maximum
#define _set_maximum(TSeq) concatenate(TSeq, _set_maximum)
#define TSeq_set_maximum _set_maximum(TSeq)
/* Define method (set) maximum() of C++ TSeq class */
#ifndef TSeq_isCDRStringType
bool
TSeq::maximum(RTI_INT32 new_max)
{
    return (TSeq_set_maximum(this, new_max) == RTI_TRUE);
}
#else
#ifdef TSeq_isCDRStringType_no_max
bool
TSeq::maximum(RTI_INT32 new_max,RTI_UINT32 max_str_len)
{
    return (TSeq_set_maximum(this, new_max,max_str_len) == RTI_TRUE);
}
#else
#undef TSeq_set_maximum_w_max
#define _set_maximum_w_max(TSeq) concatenate(TSeq, _set_maximum_w_max)
#define TSeq_set_maximum_w_max _set_maximum_w_max(TSeq)
bool
TSeq::maximum(RTI_INT32 new_max,RTI_UINT32 max_str_len)
{
    return (TSeq_set_maximum_w_max(this, new_max,max_str_len) == RTI_TRUE);
}
bool

TSeq::maximum(RTI_INT32 new_max)
{
    return (TSeq_set_maximum(this, new_max) == RTI_TRUE);
}
#endif
#endif
#undef _set_maximum
#undef TSeq_set_maximum
#undef _set_maximum_w_max
#undef TSeq_set_maximum_w_max
#endif /* TSeq_set_maximum */

#ifdef TSeq_get_length
#undef TSeq_get_length
#define _get_length(TSeq) concatenate(TSeq, _get_length)
#define TSeq_get_length _get_length(TSeq)
/* Define method (get) length() of C++ TSeq class */
RTI_INT32
TSeq::length() const
{
    return TSeq_get_length(this);
}
#undef _get_length
#undef TSeq_get_length
#endif /* TSeq_get_length */

#ifdef TSeq_set_length
#undef TSeq_set_length
#define _set_length(TSeq) concatenate(TSeq, _set_length)
#define TSeq_set_length _set_length(TSeq)
/* Define method (set) length() of C++ TSeq class */
bool
TSeq::length(RTI_INT32 new_length)
{
    return (TSeq_set_length(this, new_length) == RTI_TRUE);
}
#undef _set_length
#undef TSeq_set_length
#endif /* TSeq_set_length */

#ifdef TSeq_get_reference
#undef TSeq_get_reference
#define _get_reference(TSeq) concatenate(TSeq, _get_reference)
#define TSeq_get_reference _get_reference(TSeq)
T*
TSeq::get_reference(RTI_INT32 i)
{
    return TSeq_get_reference(this,i);
}

#ifndef RTI_CERT
T&
TSeq::operator[] (RTI_INT32 index)
{
    return *TSeq_get_reference(this,index);
}

const T&
TSeq::operator[] (RTI_INT32 index) const
{
    return *(const T*)TSeq_get_reference(this,index);
}

#endif

#undef _get_reference
#undef TSeq_get_reference
#endif /* TSeq_get_reference */

#ifdef TSeq_copy
#undef TSeq_copy
#define _copy(TSeq) concatenate(TSeq, _copy)
#define TSeq_copy _copy(TSeq)
#ifndef TSeq_isCDRStringType
/* Define method copy() of C++ TSeq class */
bool
TSeq::copy(const TSeq& src_seq)
{
    TSeq *result = TSeq_copy(this,&src_seq);
    return (result != NULL);
}
#else
#ifdef TSeq_isCDRStringType_no_max
bool
TSeq::copy(const TSeq& src_seq,RTI_UINT32 max_str_len)
{
    TSeq *result = TSeq_copy(this,&src_seq,max_str_len);
    return (result != NULL);
}

bool
TSeq::copy(const TSeq& src_seq)
{
    if (this->_flags & REDA_SEQUENCE_FLAG_PTR_ALLOCATION)
    {
        return TSeq_copy(this,&src_seq,REDA_SEQUENCE_ELEMENT_REPLACE) != NULL;
    }
    else
    {
        return TSeq_copy(this,&src_seq,REDA_SEQUENCE_ELEMENT_ALLOC) != NULL;
    }
}
#else
#undef TSeq_copy_w_max
#define _copy_w_max(TSeq) concatenate(TSeq, _copy_w_max)
#define TSeq_copy_w_max _copy_w_max(TSeq)
bool
TSeq::copy(const TSeq& src_seq,RTI_UINT32 max_str_len)
{
    TSeq *result = TSeq_copy_w_max(this,&src_seq,max_str_len);
    return (result != NULL);
}

bool
TSeq::copy(const TSeq& src_seq)
{
    TSeq *result = TSeq_copy(this,&src_seq);
    return (result != NULL);
}
#endif
#endif

/* Define operator = of C++ TSeq class */
TSeq&
TSeq::operator=(const TSeq& src_seq)
{
#ifdef RTI_CERT
    UNUSED_ARG(src_seq);
#else
    this->copy(src_seq);
#endif
    return *this;
}
#endif /* TSeq_copy */

#ifdef TSeq_initialize
#undef TSeq_initialize
#define _initialize(TSeq) concatenate(TSeq, _initialize)
#define TSeq_initialize _initialize(TSeq)

/* Define default constructor of C++ TSeq class */
TSeq::TSeq()
{
    TSeq_initialize(this);
}

/* Define copy constructor of C++ TSeq class */
#ifndef RTI_CERT
TSeq::TSeq(const TSeq& seq)
{
    TSeq_initialize(this);
    this->copy(seq);
}
#endif
#undef _initialize
#undef TSeq_initialize
#endif

#undef _copy
#undef TSeq_copy

#ifdef TSeq_is_equal
#undef TSeq_is_equal
#define _is_equal(TSeq) concatenate(TSeq, _is_equal)
#define TSeq_is_equal _is_equal(TSeq)
/* Define method is_equal() of C++ TSeq class */
bool
TSeq::is_equal(const TSeq& other) const
{
    return (TSeq_is_equal(this,&other) == RTI_TRUE);
}

bool
TSeq::operator==(const TSeq& other) const
{
#ifndef RTI_CERT
    return this->is_equal(other);
#else /* RTI_CERT */
    UNUSED_ARG(other);
    return false;
#endif /* RTI_CERT */
}

bool
TSeq::operator!=(const TSeq& other) const
{
#ifndef RTI_CERT
    return !(this->is_equal(other));
#else /* RTI_CERT */
    UNUSED_ARG(other);
    return false;
#endif /* RTI_CERT */
}

#undef _is_equal
#undef TSeq_is_equal
#endif /* TSeq_is_equal */

#ifdef TSeq_loan_contiguous
#undef TSeq_loan_contiguous
#define _loan_contiguous(TSeq) concatenate(TSeq, _loan_contiguous)
#define TSeq_loan_contiguous _loan_contiguous(TSeq)
/* Define method loan_contiguous() of C++ TSeq class */
bool
TSeq::loan_contiguous(void* buffer, RTI_INT32 new_length, RTI_INT32 new_max)
{
    return (TSeq_loan_contiguous(this,buffer,new_length,new_max) == RTI_TRUE);
}
#undef _loan_contiguous
#undef TSeq_loan_contiguous
#endif /* TSeq_loan_contiguous */

#ifdef TSeq_loan_discontiguous
#undef TSeq_loan_discontiguous
#define _loan_discontiguous(TSeq) concatenate(TSeq, _loan_discontiguous)
#define TSeq_loan_discontiguous _loan_discontiguous(TSeq)
/* Define method loan_contiguous() of C++ TSeq class */
bool
TSeq::loan_discontiguous(void* buffer, RTI_INT32 new_length, RTI_INT32 new_max)
{
    return (TSeq_loan_discontiguous(this,buffer,new_length,new_max) == RTI_TRUE);
}
#undef _loan_discontiguous
#undef TSeq_loan_discontiguous
#endif /* TSeq_loan_discontiguous */


#ifdef TSeq_has_ownership
#undef TSeq_has_ownership
#define _has_ownership(TSeq) concatenate(TSeq, _has_ownership)
#define TSeq_has_ownership _has_ownership(TSeq)
/* Define method has_ownership() of C++ TSeq class */
bool
TSeq::has_ownership()
{
    return (TSeq_has_ownership(this) == RTI_TRUE);
}
#undef _has_ownership
#undef TSeq_has_ownership
#endif /* TSeq_has_ownership */


#ifdef TSeq_has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#define _has_discontiguous_buffer(TSeq) concatenate(TSeq, _has_discontiguous_buffer)
#define TSeq_has_discontiguous_buffer _has_discontiguous_buffer(TSeq)
/* Define method has_discontiguous_buffer() of C++ TSeq class */
bool
TSeq::has_discontiguous_buffer()
{
    return (TSeq_has_discontiguous_buffer(this) == RTI_TRUE);
}
#undef _has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#endif /* TSeq_has_discontiguous_buffer */


#ifdef TSeq_get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#define _get_contiguous_buffer(TSeq) concatenate(TSeq, _get_contiguous_buffer)
#define TSeq_get_contiguous_buffer _get_contiguous_buffer(TSeq)
/* Define method get_contiguous_buffer() of C++ TSeq class */
T*
TSeq::get_contiguous_buffer() const
{
    return TSeq_get_contiguous_buffer(this);
}
#undef _get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#endif /* TSeq_get_buffer */


#ifdef TSeq_set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#define _set_contiguous_buffer(TSeq) concatenate(TSeq, _set_contiguous_buffer)
#define TSeq_set_contiguous_buffer _set_contiguous_buffer(TSeq)
/* Define method set_contiguous_buffer() of C++ TSeq class */
bool
TSeq::set_contiguous_buffer(T *buffer)
{
    return (TSeq_set_contiguous_buffer(this,buffer) == RTI_TRUE);
}
#undef _set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#endif /* TSeq_set_contiguous_buffer */


#ifdef TSeq_unloan
#undef TSeq_unloan
#define _unloan(TSeq) concatenate(TSeq, _unloan)
#define TSeq_unloan _unloan(TSeq)
/* Define method get_buffer() of C++ TSeq class */
bool
TSeq::unloan()
{
    return (TSeq_unloan(this) == RTI_TRUE);
}
#undef _unloan
#undef TSeq_unloan
#endif /* TSeq_unloan */


#ifdef TSeq_set_token
#undef TSeq_set_token
#define _set_token(TSeq) concatenate(TSeq, _set_token)
#define TSeq_set_token _set_token(TSeq)
void
TSeq::set_token(void *token1,void *token2)
{
    TSeq_set_token(this,token1,token2);
}
#undef _set_token
#undef TSeq_set_token
#endif /* TSeq_set_token */

#ifdef TSeq_get_token
#undef TSeq_get_token
#define _get_token(TSeq) concatenate(TSeq, _get_token)
#define TSeq_get_token _get_token(TSeq)
void
TSeq::get_token(void **token1,void **token2)
{
    TSeq_get_token(this,token1,token2);
}
#undef _get_token
#undef TSeq_get_token
#endif /* TSeq_get_token */

#ifdef TSeq_ensure_length
#undef TSeq_ensure_length
#define _ensure_length(TSeq) concatenate(TSeq, _ensure_length)
#define TSeq_ensure_length _ensure_length(TSeq)
#ifndef TSeq_isCDRStringType
bool
TSeq::ensure_length(RTI_INT32 length, RTI_INT32 max)
{
    return (TSeq_ensure_length(this, length,max) == RTI_TRUE);
}
#else
#ifndef TSeq_isCDRStringType_no_max
#undef TSeq_ensure_length_w_max
#define _ensure_length_w_max(TSeq) concatenate(TSeq, _ensure_length_w_max)
#define TSeq_ensure_length_w_max _ensure_length_w_max(TSeq)
#endif
bool
TSeq::ensure_length(RTI_INT32 length, RTI_INT32 max,RTI_UINT32 max_str_len)
{
#ifdef TSeq_isCDRStringType_no_max
    return (TSeq_ensure_length(this, length,max,max_str_len) == RTI_TRUE);
#else
    return (TSeq_ensure_length_w_max(this, length,max,max_str_len) == RTI_TRUE);
#endif
}

#ifndef TSeq_isCDRStringType_no_max
bool
TSeq::ensure_length(RTI_INT32 length, RTI_INT32 max)
{
    return (TSeq_ensure_length(this, length,max) == RTI_TRUE);
}
#endif

#endif
#undef _ensure_length
#undef TSeq_ensure_length
#undef _ensure_length_w_max
#undef TSeq_ensure_length_w_max

#endif /* TSeq_ensure_length */

#ifdef TSeq_from_array
#undef TSeq_from_array
#define _from_array(TSeq) concatenate(TSeq, _from_array)
#define TSeq_from_array _from_array(TSeq)
#ifndef TSeq_isCDRStringType
bool
TSeq::from_array(const T elv[], RTI_INT32 length)
{
    return (TSeq_from_array(this,elv,length) == RTI_TRUE);
}
#else
#ifndef TSeq_isCDRStringType_no_max
#undef TSeq_from_array_w_max
#define _from_array_w_max(TSeq) concatenate(TSeq, _from_array_w_max)
#define TSeq_from_array_w_max _from_array_w_max(TSeq)
#endif
bool
TSeq::from_array(const T elv[], RTI_INT32 length, RTI_UINT32 max_str_len)
{
#ifdef TSeq_isCDRStringType_no_max
    return (TSeq_from_array(this,elv,length,max_str_len) == RTI_TRUE);
#else
    return (TSeq_from_array_w_max(this,elv,length,max_str_len) == RTI_TRUE);
#endif
}

#ifndef TSeq_isCDRStringType_no_max
bool
TSeq::from_array(const T elv[], RTI_INT32 length)
{
    return (TSeq_from_array(this,elv,length) == RTI_TRUE);
}
#endif

#endif
#undef _from_array
#undef TSeq_from_array
#undef _from_array_w_max
#undef TSeq_from_array_w_max

#endif /* TSeq_from_array */

#ifdef TSeq_to_array
#undef TSeq_to_array
#define _to_array(TSeq) concatenate(TSeq, _to_array)
#define TSeq_to_array _to_array(TSeq)
#ifndef TSeq_isCDRStringType
bool
TSeq::to_array(T elv[], RTI_INT32 length)
{
    return (TSeq_to_array(this,elv,length) == RTI_TRUE);
}
#else
#ifndef TSeq_isCDRStringType_no_max
#undef TSeq_to_array_w_max
#define _to_array_w_max(TSeq) concatenate(TSeq, _to_array_w_max)
#define TSeq_to_array_w_max _to_array_w_max(TSeq)
#endif
bool
TSeq::to_array(T elv[], RTI_INT32 length,RTI_UINT32 max_str_len)
{
#ifdef TSeq_isCDRStringType_no_max
    return (TSeq_to_array(this,elv,length,max_str_len) == RTI_TRUE);
#else
    return (TSeq_to_array_w_max(this,elv,length,max_str_len) == RTI_TRUE);
#endif
}

#ifndef TSeq_isCDRStringType_no_max
bool
TSeq::to_array(T elv[], RTI_INT32 length)
{
    return (TSeq_to_array(this,elv,length) == RTI_TRUE);
}
#endif
#endif
#undef _to_array
#undef TSeq_to_array
#undef _to_array_w_max
#undef TSeq_to_array_w_max
#endif /* TSeq_to_array */

#ifdef TSeq_finalize
#undef TSeq_finalize
#endif

#undef concatenate
#undef REDA_SEQUENCE_API
#undef T
#undef TSeq
#undef TSeq_isStringType
#undef TSeq_isCDRStringType
#undef TSeq_isCDRStringType_no_max
#undef TSeq_isCDRCharStringType
#undef REDA_SEQUENCE_USER_API

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*e \dref_FooSeq
 */
struct FooSeq {
    /*e \dref_FooSeq_copy
     */
    FooSeq& operator=(const struct FooSeq& src_seq);

    /*e \dref_FooSeq_copy_no_alloc
     */
    bool copy_no_alloc(const struct FooSeq& src_seq);

    /*e \dref_FooSeq_from_array
     */
    bool from_array(const Foo array[], DDS_Long length);

    /*e \dref_FooSeq_to_array
     */
    bool to_array(Foo array[], DDS_Long length);

    /*e \dref_FooSeq_operatorBracket
     */
    Foo& operator[] (DDS_Long i);

    /*e \dref_FooSeq_operatorBracketConst
     */
    const Foo& operator[] (DDS_Long i) const;

    /*e \dref_FooSeq_get_length
     */
    DDS_Long length() const;

    /*e \dref_FooSeq_set_length
     */
    bool length(DDS_Long new_length);

    /*e \dref_FooSeq_ensure_length
    */
    bool ensure_length(DDS_Long length, DDS_Long max);

    /*e \dref_FooSeq_get_maximum
     */
    DDS_Long maximum() const;

    /*e \dref_FooSeq_set_maximum
     */
    bool maximum(DDS_Long new_max);

    /*e \dref_FooSeq_loan_contiguous
     */
    bool loan_contiguous(Foo* buffer, DDS_Long new_length, DDS_Long new_max);

    /*e \dref_FooSeq_unloan
     */
    bool unloan();

    /*e \dref_FooSeq_get_contiguous_buffer
     */
    Foo* get_contiguous_buffer() const;


    /*e \dref_FooSeq_has_ownership
     */
    bool has_ownership();

    /*e \dref_FooSeq_delete
     */
    ~FooSeq();

    /*e \dref_FooSeq_new
     */
    FooSeq(DDS_Long new_max = 0);

    /*e \dref_FooSeq_new_from_sequence
     */
    FooSeq(const struct FooSeq& foo_seq);
};


#endif /* DOXYGEN_DOCUMENTATION_ONLY */
