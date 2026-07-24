/*
 * FILE: reda_sequence_decl.h - Typed Sequence API
 *
 * Copyright (c) 2008-2016 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 18feb2015,tk Factored out sequence declarations from reda_sequence.h
 */
/*ci
 * \file
 *
 * \brief The REDA Sequence API provides a strongly typed vector API.
 *
 * \defgroup REDASequenceClass REDA Sequence
 * \ingroup REDAModule
 *
 * \details
 *
 * The REDA sequence API provides a strongly typed API to manage variable
 * size vectors/arrays of the same type. The typing is achieved by encapsulating
 * generic function in a typed wrapper using macro's. The documentation
 * is for the generic functions.
 */
/*ci
 * \addtogroup REDASequenceClass
 * @{
 */
#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#include "reda/reda_sequence.h"

#ifdef REDADllExport
#undef REDADllExport
#endif

#if defined(RTI_WIN32) || defined(RTI_WINCE30)
#define REDADllExport __declspec( dllexport )
#else
#define REDADllExport
#endif

#define concatenate(A, B)   A ## B

#ifndef T
#define T RTI_INT32
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
#endif /* REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL */

/*ci
 * \addtogroup REDASequenceClass
 * @{
 */
#ifndef REDA_SEQUENCE_EXCLUDE_STRUCT

struct REDADllExport TSeq
{
    T* _contiguous_buffer;
    RTI_INT32 _maximum;
    RTI_INT32 _length;
    RTI_INT32 _element_size;
    void *_token1;
    void *_token2;
    RTI_UINT8 _flags;
#ifdef RTI_CPP
public:
#ifdef TSeq_get_maximum
    RTI_INT32 maximum() const;
#endif
#ifdef TSeq_set_maximum
#ifndef TSeq_isCDRStringType
    bool maximum(RTI_INT32 new_max);
#else
#ifdef TSeq_isCDRStringType_no_max
    bool maximum(RTI_INT32 new_max,RTI_UINT32 max_str_len);
#else
    bool maximum(RTI_INT32 new_max,RTI_UINT32 max_str_len);
    bool maximum(RTI_INT32 new_max);
#endif
#endif
#endif
#ifdef TSeq_get_length
    RTI_INT32 length() const;
#endif
#ifdef TSeq_set_length
    bool length(RTI_INT32 new_length);
#endif
#ifdef TSeq_get_reference
    T* get_reference(RTI_INT32 i);
#endif

#ifdef TSeq_copy
#ifndef TSeq_isCDRStringType
    bool copy(const TSeq& src_seq);
#else
    bool copy(const TSeq& src_seq,RTI_UINT32 max_str_len);
    bool copy(const TSeq& src_seq);
#endif
#endif

#ifdef TSeq_is_equal
    bool is_equal(const TSeq& other) const;
#endif
#ifdef TSeq_loan_contiguous
    bool loan_contiguous(void* buffer, RTI_INT32 new_length, RTI_INT32 new_max);
#endif
#ifdef TSeq_loan_discontiguous
    bool loan_discontiguous(void* buffer, RTI_INT32 new_length, RTI_INT32 new_max);
#endif
#ifdef TSeq_unloan
    bool unloan();
#endif
#ifdef TSeq_has_ownership
    bool has_ownership();
#endif
#ifdef TSeq_get_contiguous_buffer
    T* get_contiguous_buffer() const;
#endif
#ifdef TSeq_set_contiguous_buffer
    bool set_contiguous_buffer(T* buffer);
#endif
#ifdef TSeq_has_discontiguous_buffer
    bool has_discontiguous_buffer();
#endif
#ifdef TSeq_set_token
    void set_token(void *token1,void *token2);
#endif
#ifdef TSeq_get_token
    void get_token(void **token1,void **token2);
#endif
#ifdef TSeq_ensure_length
#ifndef TSeq_isCDRStringType
    bool ensure_length(RTI_INT32 length, RTI_INT32 max);
#else
#ifdef TSeq_isCDRStringType_no_max
    bool ensure_length(RTI_INT32 length, RTI_INT32 max,RTI_UINT32 max_str_len);
#else
    bool ensure_length(RTI_INT32 length, RTI_INT32 max,RTI_UINT32 max_str_len);
    bool ensure_length(RTI_INT32 length, RTI_INT32 max);
#endif
#endif
#endif

#ifdef TSeq_from_array
#ifndef TSeq_isCDRStringType
    bool from_array(const T elv[], RTI_INT32 length);
#else
#ifdef TSeq_isCDRStringType_no_max
    bool from_array(const T elv[], RTI_INT32 length,RTI_UINT32 max_str_len);
#else
    bool from_array(const T elv[], RTI_INT32 length,RTI_UINT32 max_str_len);
    bool from_array(const T elv[], RTI_INT32 length);
#endif
#endif
#endif

#ifdef TSeq_to_array
#ifndef TSeq_isCDRStringType
    bool to_array(T elv[], RTI_INT32 length);
#else
#ifdef TSeq_isCDRStringType_no_max
    bool to_array(T elv[], RTI_INT32 length,RTI_UINT32 max_str_len);
#else
    bool to_array(T elv[], RTI_INT32 length,RTI_UINT32 max_str_len);
    bool to_array(T elv[], RTI_INT32 length);
#endif
#endif
#endif
    TSeq();
    ~TSeq();

#ifdef RTI_CERT
private:
#else
public:
#endif /* RTI_CERT */
    TSeq(const TSeq& seq);
#ifdef TSeq_copy
    TSeq& operator=(const TSeq& src_seq);
#endif
#ifdef TSeq_get_reference
    T& operator[] (RTI_INT32 index);
    const T& operator[] (RTI_INT32 index) const;
#endif
#ifdef TSeq_is_equal
    bool operator==(const TSeq& other) const;
    bool operator!=(const TSeq& other) const;
#endif
#ifdef TSeq_cpp_extra
    TSeq_cpp_extra
#undef TSeq_cpp_extra
#endif

#endif /* RTI_CPP */
};

/* REDA_DEFINE_SEQUENCE_OVERLOADED_METHODS(TSeq,T) */

#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL
#error "REDA_SEQUENCE_API_UNTYPED <= REDA_SEQUENCE_API <= REDA_SEQUENCE_API_FULL"
#endif
#endif /* REDA_SEQUENCE_EXCLUDE_STRUCT */

#ifndef REDA_SEQUENCE_EXCLUDE_C_METHODS

#if defined(__cplusplus) && !defined(REDA_SEQUENCE_USER_CPP)
extern "C"
{
#endif

#ifdef TSeq_initialize
#undef TSeq_initialize
#define _initialize(TSeq) concatenate(TSeq, _initialize)
#define TSeq_initialize _initialize(TSeq)
/*ci
 * \brief Initialize a sequence of type T
 *
 * \details
 *
 * Initializes a sequence structure TSeq based on the element size of
 * sequence type T. The initialization may call a type specific T_initialize
 * for each element type T if it is defined.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \param[in] self Sequence structure to initialize
 *
 * \sa TSeq_finalize
 */
REDADllExport RTI_BOOL
TSeq_initialize(struct TSeq* self);
#undef _initialize
#undef TSeq_initialize
#endif

#ifdef TSeq_finalize
#undef TSeq_finalize
#define _finalize(TSeq) concatenate(TSeq, _finalize)
#define TSeq_finalize _finalize(TSeq)
/*ci
 * \brief Finalize a sequence of type T
 *
 * \details
 *
 * Finalizes a sequence TSeq based and releases all memory allocated to it.
 * Each sequence element may be finalized individually if T_finalize is defined
 * for the sequence type.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \param[in] self Sequence structure to finalize
 *
 * \sa TSeq_finalize
 */
REDADllExport RTI_BOOL
TSeq_finalize(struct TSeq *self);
#undef _finalize
#undef TSeq_finalize
#endif

#ifdef TSeq_get_maximum
#undef TSeq_get_maximum
#define _get_maximum(TSeq) concatenate(TSeq, _get_maximum)
#define TSeq_get_maximum _get_maximum(TSeq)
REDADllExport RTI_INT32
TSeq_get_maximum(const struct TSeq *self);
#undef _get_maximum
#undef TSeq_get_maximum
#endif

#ifdef TSeq_set_maximum
#undef TSeq_set_maximum
#define _set_maximum(TSeq) concatenate(TSeq, _set_maximum)
#define TSeq_set_maximum _set_maximum(TSeq)
#ifndef TSeq_isCDRStringType
REDADllExport RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max);
#else
#ifdef TSeq_isCDRStringType_no_max
REDADllExport RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max, RTI_UINT32 max_str_len);
#else
#undef TSeq_set_maximum_w_max
#define _set_maximum_w_max(TSeq) concatenate(TSeq,_set_maximum_w_max)
#define TSeq_set_maximum_w_max _set_maximum_w_max(TSeq)
REDADllExport RTI_BOOL
TSeq_set_maximum_w_max(struct TSeq *self, RTI_INT32 new_max, RTI_UINT32 max_str_len);

REDADllExport RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max);
#endif
#endif
#undef _set_maximum
#undef TSeq_set_maximum
#undef _set_maximum_w_max
#undef TSeq_set_maximum_w_max
#endif

#ifdef TSeq_get_length
#undef TSeq_get_length
#define _get_length(TSeq) concatenate(TSeq, _get_length)
#define TSeq_get_length _get_length(TSeq)
REDADllExport RTI_INT32
TSeq_get_length(const struct TSeq *self);
#undef _get_length
#undef TSeq_get_length
#endif

#ifdef TSeq_set_length
#undef TSeq_set_length
#define _set_length(TSeq) concatenate(TSeq, _set_length)
#define TSeq_set_length _set_length(TSeq)
REDADllExport RTI_BOOL
TSeq_set_length(struct TSeq *self, RTI_INT32 new_length);
#undef _set_length
#undef TSeq_set_length
#endif

#ifdef TSeq_get_reference
#undef TSeq_get_reference
#define _get_reference(TSeq) concatenate(TSeq, _get_reference)
#define TSeq_get_reference _get_reference(TSeq)
REDADllExport T*
TSeq_get_reference(const struct TSeq *self, RTI_INT32 i);
#undef _get_reference
#undef TSeq_get_reference
#endif

#ifdef TSeq_copy
#undef TSeq_copy
#define _copy(TSeq) concatenate(TSeq, _copy)
#define TSeq_copy _copy(TSeq)
#ifndef TSeq_isCDRStringType
REDADllExport struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src);
#else
#ifdef TSeq_isCDRStringType_no_max
REDADllExport struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src, RTI_UINT32 max_str_len);
#else
#undef TSeq_copy_w_max
#define _copy_w_max(TSeq) concatenate(TSeq, _copy_w_max)
#define TSeq_copy_w_max _copy_w_max(TSeq)
REDADllExport struct TSeq*
TSeq_copy_w_max(struct TSeq *self, const struct TSeq *src, RTI_UINT32 max_str_len);

REDADllExport struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src);
#endif
#endif
#undef _copy
#undef TSeq_copy
#undef _copy_w_max
#undef TSeq_copy_w_max
#endif

#ifdef TSeq_is_equal
#undef TSeq_is_equal
#define _is_equal(TSeq) concatenate(TSeq, _is_equal)
#define TSeq_is_equal _is_equal(TSeq)
REDADllExport RTI_BOOL
TSeq_is_equal(const struct TSeq *left, const struct TSeq *right);
#undef _is_equal
#undef TSeq_is_equal
#endif

#ifdef TSeq_loan_contiguous
#undef TSeq_loan_contiguous
#define _loan_contiguous(TSeq) concatenate(TSeq, _loan_contiguous)
#define TSeq_loan_contiguous _loan_contiguous(TSeq)
REDADllExport RTI_BOOL
TSeq_loan_contiguous(struct TSeq *self, void *buffer,
                     RTI_INT32 new_length, RTI_INT32 new_max);
#undef _loan_contiguous
#undef TSeq_loan_contiguous
#endif

#ifdef TSeq_loan_discontiguous
#undef TSeq_loan_discontiguous
#define _loan_discontiguous(TSeq) concatenate(TSeq, _loan_discontiguous)
#define TSeq_loan_discontiguous _loan_discontiguous(TSeq)
REDADllExport RTI_BOOL
TSeq_loan_discontiguous(struct TSeq *self, void *buffer,
                        RTI_INT32 new_length, RTI_INT32 new_max);
#undef _loan_discontiguous
#undef TSeq_loan_discontiguous
#endif /* TSeq_loan_discontiguous */

#ifdef TSeq_unloan
#undef TSeq_unloan
#define _unloan(TSeq) concatenate(TSeq, _unloan)
#define TSeq_unloan _unloan(TSeq)
REDADllExport RTI_BOOL
TSeq_unloan(struct TSeq *self);
#undef _unloan
#undef TSeq_unloan
#endif /* TSeq_unloan */

#ifdef TSeq_has_ownership
#undef TSeq_has_ownership
#define _has_ownership(TSeq) concatenate(TSeq, _has_ownership)
#define TSeq_has_ownership _has_ownership(TSeq)
REDADllExport RTI_BOOL
TSeq_has_ownership(const struct TSeq *self);
#undef _has_ownership
#undef TSeq_has_ownership
#endif /* TSeq_has_ownership */

#ifdef TSeq_get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#define _get_contiguous_buffer(TSeq) concatenate(TSeq, _get_contiguous_buffer)
#define TSeq_get_contiguous_buffer _get_contiguous_buffer(TSeq)
REDADllExport T*
TSeq_get_contiguous_buffer(const struct TSeq *self);
#undef _get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#endif /* TSeq_get_buffer */

#ifdef TSeq_get_discontiguous_buffer
#undef TSeq_get_discontiguous_buffer
#define _get_discontiguous_buffer(TSeq) concatenate(TSeq, TSeq_get_discontiguous_buffer)
#define TSeq_get_discontiguous_buffer _get_discontiguous_buffer(TSeq)
REDADllExport T*
TSeq_get_discontiguous_buffer(const struct TSeq *self);
#undef _get_discontiguous_buffer
#undef TSeq_get_discontiguous_buffer
#endif /* TSeq_get_discontiguous_buffer */

#ifdef TSeq_set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#define _set_contiguous_buffer(TSeq) concatenate(TSeq, _set_contiguous_buffer)
#define TSeq_set_contiguous_buffer _set_contiguous_buffer(TSeq)
REDADllExport RTI_BOOL
TSeq_set_contiguous_buffer(struct TSeq *self, T *buffer);
#undef _set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#endif /* TSeq_set_buffer */

#ifdef TSeq_has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#define _has_discontiguous_buffer(TSeq) concatenate(TSeq, _has_discontiguous_buffer)
#define TSeq_has_discontiguous_buffer _has_discontiguous_buffer(TSeq)
REDADllExport RTI_BOOL
TSeq_has_discontiguous_buffer(const struct TSeq *self);
#undef _has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#endif /* TSeq_has_discontiguous_buffer */

#ifdef TSeq_set_token
#undef TSeq_set_token
#define _set_token(TSeq) concatenate(TSeq, _set_token)
#define TSeq_set_token _set_token(TSeq)
REDADllExport void
TSeq_set_token(struct TSeq *self,void *token1,void *token2);
#undef _set_token
#undef TSeq_set_token
#endif /* TSeq_set_token */

#ifdef TSeq_get_token
#undef TSeq_get_token
#define _get_token(TSeq) concatenate(TSeq, _get_token)
#define TSeq_get_token _get_token(TSeq)
REDADllExport void
TSeq_get_token(struct TSeq *self,void **token1,void **token2);
#undef _get_token
#undef TSeq_get_token
#endif /* TSeq_get_token */

#ifdef TSeq_ensure_length
#undef TSeq_ensure_length
#define _ensure_length(TSeq) concatenate(TSeq, _ensure_length)
#define TSeq_ensure_length _ensure_length(TSeq)
#ifndef TSeq_isCDRStringType
REDADllExport RTI_BOOL
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max);
#else
#ifdef TSeq_isCDRStringType_no_max
REDADllExport RTI_BOOL
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max,
                           RTI_UINT32 max_str_len);
#else
#undef TSeq_ensure_length_w_max
#define _ensure_length_w_max(TSeq) concatenate(TSeq, _ensure_length_w_max)
#define TSeq_ensure_length_w_max _ensure_length_w_max(TSeq)
REDADllExport RTI_BOOL
TSeq_ensure_length_w_max(struct TSeq *self,RTI_INT32 length, RTI_INT32 max,
                         RTI_UINT32 max_str_len);
REDADllExport RTI_BOOL
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max);
#endif
#endif
#undef _ensure_length
#undef TSeq_ensure_length
#undef _ensure_length_w_max
#undef TSeq_ensure_length_max
#endif /* TSeq_ensure_length */

#ifdef TSeq_from_array
#undef TSeq_from_array
#define _from_array(TSeq) concatenate(TSeq, _from_array)
#define TSeq_from_array _from_array(TSeq)
#ifndef TSeq_isCDRStringType
REDADllExport RTI_BOOL
TSeq_from_array(struct TSeq *self, const T elms_seq[], RTI_INT32 length);
#else
#ifdef TSeq_isCDRStringType_no_max
REDADllExport RTI_BOOL
TSeq_from_array(struct TSeq *self, const T elms_seq[], RTI_INT32 length,RTI_UINT32 max_str_len);
#else
#undef TSeq_from_array_w_max
#define _from_array_w_max(TSeq) concatenate(TSeq, _from_array_w_max)
#define TSeq_from_array_w_max _from_array_w_max(TSeq)
REDADllExport RTI_BOOL
TSeq_from_array_w_max(struct TSeq *self, const T elms_seq[], RTI_INT32 length,RTI_UINT32 max_str_len);

REDADllExport RTI_BOOL
TSeq_from_array(struct TSeq *self, const T elms_seq[], RTI_INT32 length);
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
REDADllExport RTI_BOOL
TSeq_to_array(struct TSeq *self,T elms_seq[], RTI_INT32 length);
#else
#ifdef TSeq_isCDRStringType_no_max
REDADllExport RTI_BOOL
TSeq_to_array(struct TSeq *self,T elms_seq[], RTI_INT32 length,RTI_UINT32 max_str_len);
#else
#undef TSeq_to_array_w_max
#define _to_array_w_max(TSeq) concatenate(TSeq, _to_array_w_max)
#define TSeq_to_array_w_max _to_array_w_max(TSeq)
REDADllExport RTI_BOOL
TSeq_to_array_w_max(struct TSeq *self,T elms_seq[], RTI_INT32 length,RTI_UINT32 max_str_len);

REDADllExport RTI_BOOL
TSeq_to_array(struct TSeq *self,T elms_seq[], RTI_INT32 length);
#endif
#endif
#undef _to_array
#undef TSeq_to_array
#undef _to_array_w_max
#undef TSeq_to_array_max
#endif /* TSeq_to_array */

#if defined(__cplusplus) && !defined(REDA_SEQUENCE_USER_CPP)
}                               /* extern "C" */
#endif

#endif /* REDA_SEQUENCE_EXCLUDE_C_METHODS */

#undef TSeq
#undef T
#undef REDA_SEQUENCE_API
#undef concatenate
#undef TSeq_isCDRStringType
#undef TSeq_isCDRStringType_no_max
#undef TSeq_isCDRCharStringType
#undef REDA_SEQUENCE_EXCLUDE_STRUCT
#undef REDA_SEQUENCE_EXCLUDE_C_METHODS
#undef REDA_SEQUENCE_USER_API
#undef REDA_SEQUENCE_USER_CPP
#undef TSeq_cpp_extra

/*ci @} */
