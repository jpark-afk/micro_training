/*
 * FILE: reda_sequence_defn.h - Sequence template
 *
 * Copyright 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 26mar2016,tk  MICRO-1133 Changed how Seq_copy and Seq_set_maximum copied the
 *                          source sequence data locally to avoid C++
 *                          constructor/destructor problems
 * 12mar2015,eh  MICRO-1053/PR#13585 Remove string seq map() from CERT
 * 27jan2015,tk  MICRO-1014/PR#13358 Removed redundant code for CERT in _copy
 *                                  (The preallocated memory version)
 * 01dec2014,tk  MICRO-951/PR#12315  Removed redundant code for CERT in _copy
 * 01dec2014,tk  MICRO-949/PR#11856  Removed redundant code for CERT in set_maximum
 * 04aug2014,tk  MICRO-841 Only copy new_max elements if sequence is reduced
 * 03jun2013,kaj MICRO-502 support for string sequences req. max string length
 * 20apr2008,tk  Written
 */
/*ce
 * \file
 */
#include "reda/reda_sequence.h"
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

#if defined(T_copy) && !defined(T_initialize)
#error "T_copy defined without T_initialize"
#endif

#if defined(T_compare) && !defined(T_initialize)
#error "T_compare defined without T_initialize"
#endif

#if defined(T_is_equal) && !defined(T_initialize)
#error "T_is_equal defined without T_initialize"
#endif

#ifndef RTI_CERT
#if defined(TSeq_isCDRStringType) && (!defined(T_initialize) || !defined(T_finalize) || !defined(T_copy))
#error "TSeq_isCDRStringType defined without T_initialize/T_finalize/T_copy"
#endif
#else /* RTI_CERT */
#if defined(TSeq_isCDRStringType) && (!defined(T_initialize) || !defined(T_copy))
#error "TSeq_isCDRStringType defined without T_initialize/T_copy"
#endif
#endif /* !RTI_CERT */

#if defined(T_initialize) || defined(T_finalize) || defined(T_copy)

#if !(defined(TSeq_isCDRStringType) && defined(RTI_CERT))
#ifndef TSeq_map
#define _map(TSeq) concatenate(TSeq, _map)
#define TSeq_map _map(TSeq)
RTI_PRIVATE RTI_BOOL
TSeq_map(T *buffer,RTI_BOOL(*map)(T*),RTI_INT32 begin, RTI_INT32 end)
{
    RTI_INT32 i;

    for (i = begin; i <= end; i++)
    {
        if (!map(&buffer[i]))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}
#endif /* TSeq_map */
#endif /* !(TSeq_isCDRStringType && defined(RTI_CERT)) */

#ifdef TSeq_isCDRStringType
#ifndef TSeq_map_w_length
#define _map_w_length(TSeq) concatenate(TSeq, _map_w_length)
#define TSeq_map_w_length _map_w_length(TSeq)
RTI_PRIVATE RTI_BOOL
TSeq_map_w_length(T *buffer,RTI_BOOL(*map)(T*, RTI_UINT32),RTI_INT32 begin, RTI_INT32 end, RTI_UINT32 max_str_len)
{
    RTI_INT32 i;

    for (i = begin; i <= end; i++)
    {
        if (!map(&buffer[i], max_str_len))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}
#endif /* TSeq_map_w_length */
#endif /* TSeq_isCDRStringType */

#endif /* TSeq_isComplexType */

#ifdef TSeq_initialize
#undef TSeq_initialize
#define _initialize(TSeq) concatenate(TSeq, _initialize)
#define TSeq_initialize _initialize(TSeq)
RTI_BOOL
TSeq_initialize(struct TSeq *self)
{
    return REDA_Sequence_initialize((struct REDA_Sequence*)self, sizeof(T));
}
#endif /* TSeq_initialize */

#ifdef TSeq_finalize
#undef TSeq_finalize
#define _finalize(TSeq) concatenate(TSeq, _finalize)
#define TSeq_finalize _finalize(TSeq)
RTI_BOOL
TSeq_finalize(struct TSeq *self)
{
#ifdef T_finalize
    RTI_BOOL result;
    result = TSeq_map(self->_contiguous_buffer,T_finalize,0,self->_maximum-1);
    if (!result)
    {
        return result;
    }
    return REDA_Sequence_finalize((struct REDA_Sequence*)self);
#else /* T_finalize */
    return REDA_Sequence_finalize((struct REDA_Sequence*)self);
#endif /* T_finalize */

#undef _finalize
#undef TSeq_finalize
}
#endif /* TSeq_finalize */

#ifdef TSeq_get_maximum
#undef TSeq_get_maximum
#define _get_maximum(TSeq) concatenate(TSeq, _get_maximum)
#define TSeq_get_maximum _get_maximum(TSeq)
RTI_INT32
TSeq_get_maximum(const struct TSeq *self)
{
    return REDA_Sequence_get_maximum((const struct REDA_Sequence *)self);
}
#endif /* TSeq_get_maximum */

#ifdef TSeq_set_maximum
#undef TSeq_set_maximum
#define _set_maximum(TSeq) concatenate(TSeq, _set_maximum)
#define TSeq_set_maximum _set_maximum(TSeq)
#ifndef TSeq_isCDRStringType
RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max)
{
#ifdef T_initialize
#ifndef RTI_CERT
    T* copy_buffer;
    RTI_INT32 copy_length;
    RTI_INT32 copy_max;
#endif
    RTI_BOOL result;
    RTI_INT32 old_max;
#ifndef RTI_CERT
    RTI_INT32 i;
#endif /* !RTI_CERT */

    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    self->_element_size = sizeof(T);
#ifndef RTI_CERT
    copy_buffer = self->_contiguous_buffer;
    copy_length = self->_length;
    copy_max = self->_maximum;
#endif

    old_max = REDA_Sequence_get_maximum((const struct REDA_Sequence *)self);

    /* if not resizing sequence then we are done */
    if (old_max == new_max)
    {
        return RTI_TRUE;
    }

    result = REDA_Sequence_set_maximum((struct REDA_Sequence*)self,
                                        new_max,RTI_FALSE);
    if (!result)
    {
        return result;
    }

    result = TSeq_map(self->_contiguous_buffer,T_initialize,0,self->_maximum-1);
    if (!result)
    {
        return result;
    }

#ifndef RTI_CERT
    if (self->_length > 0)
    {
        /* If the sequence is reduced in size, only copy the first new_max elements
         */
        for (i = 0; i < (new_max < copy_length ? new_max : copy_length); ++i)
        {
            if (!T_copy(&self->_contiguous_buffer[i],(const T*)&copy_buffer[i]))
            {
                return RTI_FALSE;
            }
        }
    }

    /* since the pointers were copied from old seq to new seq,
     * only finalize members between new_max and old_max-1. Note that
     * copy_buffer cannot be the same as _contiguous_buffer if the sequence
     * was resized since a new allocation is made, that is no reallocation.
     * If the sequence size is the same as the existing sequence no reallocation
     * takes place.
     */
    if (copy_buffer != NULL)
    {
        result = TSeq_map(copy_buffer,T_finalize,0,copy_max-1);
        OSAPI_Heap_free_buffer(copy_buffer);
    }
#endif /* !RTI_CERT */
    return result;
#else /* T_initialize */
    self->_element_size = sizeof(T);
    return REDA_Sequence_set_maximum((struct REDA_Sequence*)self, new_max,RTI_TRUE);
#endif /* T_initialize */
}
#else  /* TSeq_isCDRStringType */
#ifdef TSeq_isCDRStringType_no_max
RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max, RTI_UINT32 max_str_len)
#else
#undef TSeq_set_maximum_w_max
#define _set_maximum_w_max(TSeq) concatenate(TSeq, _set_maximum_w_max)
#define TSeq_set_maximum_w_max _set_maximum_w_max(TSeq)
RTI_BOOL
TSeq_set_maximum_w_max(struct TSeq *self, RTI_INT32 new_max, RTI_UINT32 max_str_len)
#endif
{
    RTI_BOOL result = RTI_FALSE;
    RTI_INT32 old_max;
#ifndef RTI_CERT
    T *copy_buffer;
    RTI_INT32 i;
#endif
    struct TSeq original_self = *self;

    OSAPI_PRECONDITION(self == NULL,
               return RTI_FALSE,
               OSAPI_Log_entry_add_int("max_str_len",(RTI_INT32)max_str_len,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    self->_element_size = sizeof(T);
    if (max_str_len < REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        self->_flags |= REDA_SEQUENCE_FLAG_PTR_ALLOCATION;
    }

    old_max = REDA_Sequence_get_maximum((const struct REDA_Sequence *)self);

    /* if not resizing seqeunce then we are done */
    if (old_max == new_max)
    {
        return RTI_TRUE;
    }

#ifndef RTI_CERT
    copy_buffer = self->_contiguous_buffer;
#endif

    /* cannot use copy content because it will free the contiquous buffer
       without finalizing any discarded elements */
    if (!REDA_Sequence_set_maximum((struct REDA_Sequence*)self,
                                        new_max,RTI_FALSE))
    {
        goto done;
    }

#ifndef RTI_CERT
    /* copy elements (string memory) from old buffer to new buffer */
    for (i = 0; i < ((old_max < new_max) ? old_max : new_max); i++)
    {
        self->_contiguous_buffer[i] = copy_buffer[i];
    }
#endif

    /* initialize (allocate) any added elements (from old_max to new_max-1)
     * If old_max > new_max-1 then the map function does not do anything
     */
    if (!TSeq_map_w_length(self->_contiguous_buffer,T_initialize,old_max,new_max-1,max_str_len))
    {
        goto done;
    }

#ifndef RTI_CERT
    /* since the pointers were copied from old seq to new seq,
     * only finalize members between new_max and old_max-1. Note that
     * copy_buffer cannot be the same as _contiguous_buffer if the sequence
     * was resized since a new allocation is made, that is no reallocation.
     * If the sequence size is the same as the existing sequence no reallocation
     * takes place.
     */
    if (copy_buffer != NULL)
    {
        result = TSeq_map(copy_buffer,T_finalize,new_max,old_max-1);
        OSAPI_Heap_free_buffer(copy_buffer);
        if (!result)
        {
            goto done;
        }
    }
#endif /* !RTI_CERT */
    result = RTI_TRUE;

done:
    if (!result)
    {
        /* restore original sequence */
        *self = original_self;
    }
    return result;
}

#ifndef TSeq_isCDRStringType_no_max
RTI_BOOL
TSeq_set_maximum(struct TSeq *self, RTI_INT32 new_max)
{
    return TSeq_set_maximum_w_max(self,new_max,REDA_SEQUENCE_ELEMENT_ALLOC);
}
#endif

#endif /* TSeq_isCDRStringType defined */
#endif /* TSeq_set_maximum */

#ifdef TSeq_get_length
#undef TSeq_get_length
#define _get_length(TSeq) concatenate(TSeq, _get_length)
#define TSeq_get_length _get_length(TSeq)
RTI_INT32
TSeq_get_length(const struct TSeq *self)
{
    return REDA_Sequence_get_length((const struct REDA_Sequence *)self);

#undef _get_length
#undef TSeq_get_length
}
#endif /* TSeq_get_length */

#ifdef TSeq_set_length
#undef TSeq_set_length
#define _set_length(TSeq) concatenate(TSeq, _set_length)
#define TSeq_set_length _set_length(TSeq)
RTI_BOOL
TSeq_set_length(struct TSeq *self, RTI_INT32 new_length)
{
    return REDA_Sequence_set_length((struct REDA_Sequence *)self, new_length);
}
#endif /* TSeq_set_length */

#ifdef TSeq_get_reference
#undef TSeq_get_reference
#define _get_reference(TSeq) concatenate(TSeq, _get_reference)
#define TSeq_get_reference _get_reference(TSeq)
T*
TSeq_get_reference(const struct TSeq *self, RTI_INT32 i)
{
    return (T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)self,i);

#undef _get_reference
#undef TSeq_get_reference
}
#endif /* TSeq_get_reference */

#ifdef TSeq_copy
#undef TSeq_copy
#define _copy(TSeq) concatenate(TSeq, _copy)
#define TSeq_copy _copy(TSeq)
#ifndef TSeq_isCDRStringType
struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src)
{
#ifdef T_copy
    RTI_BOOL result;
    RTI_INT32 i;
    T *copy_buffer;
#ifndef RTI_CERT
    RTI_INT32 copy_max;
#endif

    OSAPI_PRECONDITION(self == NULL,
                       return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    copy_buffer = self->_contiguous_buffer;

#ifndef RTI_CERT
    copy_max = self->_maximum;
#endif

    /* replaces contiguous buffer if self->max < src->size */
    if (REDA_Sequence_copy((struct REDA_Sequence *)self,
                           (const struct REDA_Sequence *)src,
                           RTI_FALSE) == NULL)
    {
        return NULL;
    }

    /* if replaced contiguous buffer then initialize new members */
    if (self->_contiguous_buffer != copy_buffer)
    {
        result = TSeq_map(self->_contiguous_buffer,T_initialize,0,self->_maximum-1);
        if (!result)
        {
            return NULL;
        }
    }

    /* copy content from source elements to destination elements */
    for (i = 0; i < src->_length; i++)
    {

        /*REDA_Sequence_get_reference takes care of a discontiguous vs contiguous seq*/
        if (!T_copy((T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)self,i),
                    (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)src,i)))
        {
            return NULL;
        }
    }

#ifndef RTI_CERT
    /* If replaced contiguous buffer then finalize old new members
     * and free old contiguous buffer
     */
    if ((copy_buffer != NULL) &&
        (copy_buffer != self->_contiguous_buffer))
    {
        result = TSeq_map(copy_buffer,T_finalize,0,copy_max-1);
        if (!result)
        {
            return NULL;
        }
        OSAPI_Heap_free_buffer(copy_buffer);
    }
#endif /* !RTI_CERT */

    return self;
#else /* T_copy */

    OSAPI_PRECONDITION(self == NULL,
                       return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return (struct TSeq*)REDA_Sequence_copy((struct REDA_Sequence *)self,
                                            (const struct REDA_Sequence *)src,
                                            RTI_TRUE);
#endif /* T_copy */

}

#else  /* TSeq_isCDRStringType defined, which requires T_copy defined */
#ifdef TSeq_isCDRStringType_no_max
struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src, RTI_UINT32 max_str_len)
#else
#undef TSeq_copy_w_max
#define _copy_w_max(TSeq) concatenate(TSeq, _copy_w_max)
#define TSeq_copy_w_max _copy_w_max(TSeq)
struct TSeq*
TSeq_copy_w_max(struct TSeq *self, const struct TSeq *src, RTI_UINT32 max_str_len)
#endif
{
    RTI_BOOL result;
    RTI_INT32 i;
    T *copy_buffer;
#ifndef RTI_CERT
    RTI_INT32 copy_max;
#endif

    OSAPI_PRECONDITION(self == NULL,
               return NULL,
               OSAPI_Log_entry_add_int("max_str_len",(RTI_INT32)max_str_len,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (max_str_len < REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        self->_flags |= REDA_SEQUENCE_FLAG_PTR_ALLOCATION;
    }

    copy_buffer = self->_contiguous_buffer;
    if (src->_element_size > 0)
    {
        self->_element_size = sizeof(T);
    }
#ifndef RTI_CERT
    copy_max = self->_maximum;
#endif

    /* replaces contiguous buffer if self->max < src->size */
    if (REDA_Sequence_copy((struct REDA_Sequence *)self,
                            (const struct REDA_Sequence *)src,
                            RTI_FALSE) == NULL)
    {
        return NULL;
    }

    /* if replaced contiguous buffer then copy existing dest string buffers
     * from old contiguous buffer to new dest contiguous buffer,
     * and initialize (allocate string buffers) for new members added
     */
    if (self->_contiguous_buffer != copy_buffer)
    {
#ifndef RTI_CERT
        for (i = 0; i < copy_max; i++)
        {
            self->_contiguous_buffer[i] = copy_buffer[i];
        }
        /* initialize (allocate) any added elements (from old_max to new_max-1)
         * If old_max > new_max-1 then the map function does not do anything
         */
        result = TSeq_map_w_length(self->_contiguous_buffer,T_initialize,copy_max,self->_maximum-1,max_str_len);
#else
        /* For RTI_CERT (self->_contiguous_buffer != copy_buffer) is true only
         * when copy_buffer was NULL, so there is no need for copying elements,
         * but we need to initialize all the elements in the (new) buffer.
         */
        result = TSeq_map_w_length(self->_contiguous_buffer,T_initialize,0,self->_maximum-1,max_str_len);
#endif
        if (!result)
        {
#ifndef RTI_CERT
            /* Free the elements that were allocated in this function call. 
             * Do not free other elements allocated previously so that they are preserved.
             * No risk if a non-allocated element is finalized. Free function is protected 
             */
            TSeq_map(self->_contiguous_buffer,T_finalize,copy_max,self->_maximum-1);
#endif
            return NULL;
        }
    }

    /* copy content from source to destination */
    for (i = 0; i < src->_length; i++)
    {
            if (!T_copy((T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)self,i),
                    (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)src,i),
                    max_str_len))
        {
            return NULL;
        }
    }

#ifndef RTI_CERT
    /* if replaced contiguous buffer then free old contiguous buffer (do not
       finalize string buffers since were transfered to new contiguous buffer */
    if ((copy_buffer != NULL) && (copy_buffer != self->_contiguous_buffer))
    {
        OSAPI_Heap_free_buffer(copy_buffer);
    }
#endif /* !RTI_CERT */

    return self;
}

#ifndef TSeq_isCDRStringType_no_max
struct TSeq*
TSeq_copy(struct TSeq *self, const struct TSeq *src)
{
    if (self->_flags & REDA_SEQUENCE_FLAG_PTR_ALLOCATION)
    {
        return TSeq_copy_w_max(self,src,REDA_SEQUENCE_ELEMENT_REPLACE);
    }
    else
    {
        return TSeq_copy_w_max(self,src,REDA_SEQUENCE_ELEMENT_ALLOC);
    }
}
#endif

#endif /* TSeq_isCDRStringType*/
#endif /* TSeq_copy */

#ifdef TSeq_is_equal
#undef TSeq_is_equal
#define _is_equal(TSeq) concatenate(TSeq, _is_equal)
#define TSeq_is_equal _is_equal(TSeq)
RTI_BOOL
TSeq_is_equal(const struct TSeq *left, const struct TSeq *right)
{
#if defined(T_compare) || defined(T_is_equal)
    RTI_INT32 i;
    RTI_BOOL result;

    result = REDA_Sequence_is_equal((const struct REDA_Sequence*)left,
                                    (const struct REDA_Sequence*)right,RTI_FALSE);
    if (!result)
    {
        return result;
    }

    for (i = 0; i < left->_length; i++)
    {
#ifdef T_compare

        if (T_compare(
                (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)left,i),
                (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)right,i)))

#else

        if (!T_is_equal(
            (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)left,i),
            (const T*)REDA_Sequence_get_reference((const struct REDA_Sequence*)right,i)))
#endif
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
#else /* defined(T_compare) || defined(T_is_equal) */
    return REDA_Sequence_is_equal((const struct REDA_Sequence *)left,
                                  (const struct REDA_Sequence *)right,RTI_TRUE);
#endif /* T_compare */
#undef _is_equal
#undef TSeq_is_equal
}
#endif /* TSeq_is_equal */

#ifdef TSeq_loan_contiguous
#undef TSeq_loan_contiguous
#define _loan_contiguous(TSeq) concatenate(TSeq, _loan_contiguous)
#define TSeq_loan_contiguous _loan_contiguous(TSeq)
RTI_BOOL
TSeq_loan_contiguous(struct TSeq *self, void *buffer,
                     RTI_INT32 new_length, RTI_INT32 new_max)
{

    /* FIXME _element_size is not initialize if TSeq_initialize()
     * is not called, so we assign it here, just to make sure */
    self->_element_size = sizeof(T);

    return REDA_Sequence_loan_contiguous(
       (struct REDA_Sequence *)self, buffer, new_length, new_max);
}
#endif /* TSeq_loan_contiguous */

#ifdef TSeq_loan_discontiguous
#undef TSeq_loan_discontiguous
#define _loan_discontiguous(TSeq) concatenate(TSeq, _loan_discontiguous)
#define TSeq_loan_discontiguous _loan_discontiguous(TSeq)
RTI_BOOL
TSeq_loan_discontiguous(struct TSeq *self, void *buffer,
                        RTI_INT32 new_length, RTI_INT32 new_max)
{
    return REDA_Sequence_loan_discontiguous(
       (struct REDA_Sequence *)self, buffer, new_length, new_max);
}
#endif /* TSeq_loan_discontiguous */


#ifdef TSeq_has_ownership
#undef TSeq_has_ownership
#define _has_ownership(TSeq) concatenate(TSeq, _has_ownership)
#define TSeq_has_ownership _has_ownership(TSeq)
RTI_BOOL
TSeq_has_ownership(const struct TSeq *self)
{
    return REDA_Sequence_has_ownership((const struct REDA_Sequence *)self);
}
#endif /* TSeq_has_ownership */


#ifdef TSeq_has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#define _has_discontiguous_buffer(TSeq) concatenate(TSeq, _has_discontiguous_buffer)
#define TSeq_has_discontiguous_buffer _has_discontiguous_buffer(TSeq)
RTI_BOOL
TSeq_has_discontiguous_buffer(const struct TSeq *self)
{
    return REDA_Sequence_has_discontiguous_buffer(
       (const struct REDA_Sequence *)self);
}

#undef _has_discontiguous_buffer
#undef TSeq_has_discontiguous_buffer
#endif /* TSeq_has_discontiguous_buffer */


#ifdef TSeq_get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#define _get_contiguous_buffer(TSeq) concatenate(TSeq, _get_contiguous_buffer)
#define TSeq_get_contiguous_buffer _get_contiguous_buffer(TSeq)
T*
TSeq_get_contiguous_buffer(const struct TSeq *self)
{
    return (T*)REDA_Sequence_get_buffer((const struct REDA_Sequence *)self);
}
#undef _get_contiguous_buffer
#undef TSeq_get_contiguous_buffer
#endif /* TSeq_get_buffer */

#ifdef TSeq_get_discontiguous_buffer
#undef TSeq_get_discontiguous_buffer
#define _get_discontiguous_buffer(TSeq) concatenate(TSeq, TSeq_get_discontiguous_buffer)
#define TSeq_get_discontiguous_buffer _get_discontiguous_buffer(TSeq)
T*
TSeq_get_discontiguous_buffer(const struct TSeq *self)
{
    return (T*)REDA_Sequence_get_buffer((const struct REDA_Sequence *)self);
}
#undef _get_discontiguous_buffer
#undef TSeq_get_discontiguous_buffer
#endif /* TSeq_get_discontiguous_buffer */

#ifdef TSeq_set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#define _set_contiguous_buffer(TSeq) concatenate(TSeq, _set_contiguous_buffer)
#define TSeq_set_contiguous_buffer _set_contiguous_buffer(TSeq)
RTI_BOOL
TSeq_set_contiguous_buffer(struct TSeq *self, T *buffer)
{
    return REDA_Sequence_set_buffer((struct REDA_Sequence *)self,
                                    (void *)buffer);
}
#undef _set_contiguous_buffer
#undef TSeq_set_contiguous_buffer
#endif /* TSeq_set_buffer */


#ifdef TSeq_unloan
#undef TSeq_unloan
#define _unloan(TSeq) concatenate(TSeq, _unloan)
#define TSeq_unloan _unloan(TSeq)
RTI_BOOL
TSeq_unloan(struct TSeq *self)
{
    return REDA_Sequence_unloan((struct REDA_Sequence *)self);
}
#endif /* TSeq_unloan */

#ifdef TSeq_set_token
#undef TSeq_set_token
#define _set_token(TSeq) concatenate(TSeq, _set_token)
#define TSeq_set_token _set_token(TSeq)
void
TSeq_set_token(struct TSeq *self,void *token1,void *token2)
{
    REDA_Sequence_set_token((struct REDA_Sequence *)self,token1,token2);
}
#undef _set_token
#undef TSeq_set_token
#endif /* TSeq_set_token */

#ifdef TSeq_get_token
#undef TSeq_get_token
#define _get_token(TSeq) concatenate(TSeq, _get_token)
#define TSeq_get_token _get_token(TSeq)
void
TSeq_get_token(struct TSeq *self,void **token1,void **token2)
{
    REDA_Sequence_get_token((struct REDA_Sequence *)self,token1,token2);
}
#undef _get_token
#undef TSeq_get_token
#endif /* TSeq_get_token */

#ifdef TSeq_ensure_length
#undef TSeq_ensure_length
#define _ensure_length(TSeq) concatenate(TSeq, _ensure_length)
#define TSeq_ensure_length _ensure_length(TSeq)
RTI_BOOL
#ifndef TSeq_isCDRStringType
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max)
#else
#ifdef TSeq_isCDRStringType_no_max
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max,
                   RTI_UINT32 max_str_len)
#else
#undef TSeq_ensure_length_w_max
#define _ensure_length_w_max(TSeq) concatenate(TSeq, _ensure_length_w_max)
#define TSeq_ensure_length_w_max _ensure_length_w_max(TSeq)
TSeq_ensure_length_w_max(struct TSeq *self,RTI_INT32 length, RTI_INT32 max,
                         RTI_UINT32 max_str_len)
#endif
#endif
{
    OSAPI_PRECONDITION((self == NULL) || (length > max) ||
                       (length < 0) || (max < 0),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_int("length",length,RTI_FALSE);
                           OSAPI_Log_entry_add_int("max",max,RTI_TRUE);)

    if (!TSeq_has_ownership(self))
    {
        return RTI_FALSE;
    }

    if (length > TSeq_get_maximum(self))
    {
#ifndef TSeq_isCDRStringType
        if (!TSeq_set_maximum(self, max))
#elif defined(TSeq_isCDRStringType_no_max)
        if (!TSeq_set_maximum(self, max,(RTI_UINT32)max_str_len))
#else
        if (!TSeq_set_maximum_w_max(self, max,(RTI_UINT32)max_str_len))
#endif
        {
            return RTI_FALSE;
        }
    }

    return TSeq_set_length(self, length);
}

#if defined(TSeq_isCDRStringType) && !defined(TSeq_isCDRStringType_no_max)
RTI_BOOL
TSeq_ensure_length(struct TSeq *self,RTI_INT32 length, RTI_INT32 max)
{
    return TSeq_ensure_length_w_max(self,length,max,REDA_SEQUENCE_ELEMENT_ALLOC);
}
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
RTI_BOOL
#ifndef TSeq_isCDRStringType
TSeq_from_array(struct TSeq *self, const T elv[], RTI_INT32 length)
#else
#ifdef TSeq_isCDRStringType_no_max
TSeq_from_array(struct TSeq *self, const T elv[], RTI_INT32 length,RTI_UINT32 max_str_length)
#else
#undef TSeq_from_array_w_max
#define _from_array_w_max(TSeq) concatenate(TSeq, _from_array_w_max)
#define TSeq_from_array_w_max _from_array_w_max(TSeq)
TSeq_from_array_w_max(struct TSeq *self, const T elv[], RTI_INT32 length,RTI_UINT32 max_str_length)
#endif
#endif
{
#ifdef RTI_CPP
    struct TSeq elms_seq;
#else
    struct TSeq elms_seq = REDA_DEFINE_EMPTY_SEQUENCE_INITIALIZER;

    if (!TSeq_initialize(&elms_seq))
    {
        return RTI_FALSE;
    }
#endif

    OSAPI_PRECONDITION((self == NULL) || (length < 0),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_int("length",length,RTI_TRUE););

    /* Loan the array to a local sequence on the stack */
    if (!TSeq_loan_contiguous(&elms_seq, (T*)elv, length, length))
    {
        return RTI_FALSE;
    }

    /* Copy into this sequence, resizing if necessary */
#ifndef TSeq_isCDRStringType
    if (TSeq_copy(self, &elms_seq) == NULL)
#elif defined(TSeq_isCDRStringType_no_max)
    if (TSeq_copy(self, &elms_seq,(RTI_UINT32)max_str_length) == NULL)
#else
    if (TSeq_copy_w_max(self, &elms_seq,(RTI_UINT32)max_str_length) == NULL)
#endif
    {
        return RTI_FALSE;
    }

    /* Unloan the array from the local sequence on the stack */
    TSeq_unloan(&elms_seq);

    return RTI_TRUE;
}

#if defined(TSeq_isCDRStringType) && !defined(TSeq_isCDRStringType_no_max)
REDADllExport RTI_BOOL
TSeq_from_array(struct TSeq *self, const T elv[], RTI_INT32 length)
{
    return TSeq_from_array_w_max(self,elv,length,REDA_SEQUENCE_ELEMENT_ALLOC);
}
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
RTI_BOOL
#ifndef TSeq_isCDRStringType
TSeq_to_array(struct TSeq *self,T elv[], RTI_INT32 length)
#else
#ifdef TSeq_isCDRStringType_no_max
TSeq_to_array(struct TSeq *self,T elv[], RTI_INT32 length,RTI_UINT32 max_str_length)
#else
#undef TSeq_to_array_w_max
#define _to_array_w_max(TSeq) concatenate(TSeq, _to_array_w_max)
#define TSeq_to_array_w_max _to_array_w_max(TSeq)
TSeq_to_array_w_max(struct TSeq *self,T elv[], RTI_INT32 length,RTI_UINT32 max_str_length)
#endif
#endif
{
#ifdef RTI_CPP
    struct TSeq elms_seq;
#else
    struct TSeq elms_seq = REDA_DEFINE_EMPTY_SEQUENCE_INITIALIZER;

    if (!TSeq_initialize(&elms_seq))
    {
        return RTI_FALSE;
    }
#endif

    OSAPI_PRECONDITION((self == NULL) || (length < 0),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_int("length",length,RTI_TRUE););

    /* Loan the array to a local sequence on the stack */
    if (!TSeq_loan_contiguous(&elms_seq, (T*)elv, length, length))
    {
        return RTI_FALSE;
    }

    /* Copy into this sequence, resizing if necessary */
#ifndef TSeq_isCDRStringType
    if (TSeq_copy(&elms_seq,self) == NULL)
#elif defined(TSeq_isCDRStringType_no_max)
    if (TSeq_copy(&elms_seq,self,(RTI_UINT32)max_str_length) == NULL)
#else
    if (TSeq_copy_w_max(&elms_seq,self,(RTI_UINT32)max_str_length) == NULL)
#endif
    {
        return RTI_FALSE;
    }

    /* Unloan the array from the local sequence on the stack */
    TSeq_unloan(&elms_seq);

    return RTI_TRUE;
}

#if defined(TSeq_isCDRStringType) && !defined(TSeq_isCDRStringType_no_max)
REDADllExport RTI_BOOL
TSeq_to_array(struct TSeq *self,T elv[], RTI_INT32 length)
{
    return TSeq_to_array_w_max(self,elv,length,REDA_SEQUENCE_ELEMENT_ALLOC);
}
#endif

#undef _to_array
#undef TSeq_to_array
#undef _to_array_w_max
#undef TSeq_to_array_w_max
#endif /* TSeq_from_array */

#undef _map
#undef TSeq_map
#undef _map_w_length
#undef TSeq_map_w_length
#undef concatenate
#undef REDA_SEQUENCE_API
#undef T
#undef TSeq
#undef TSeq_isCDRStringType
#undef TSeq_isCDRStringType_no_max
#undef T_initialize
#undef T_finalize
#undef T_copy
#undef T_compare
#undef T_is_equal
#undef REDA_SEQUENCE_USER_API
#undef _set_length
#undef TSeq_set_length
#undef _set_maximum
#undef _set_maximum_w_max
#undef TSeq_set_maximum
#undef TSeq_set_maximum_w_max
#undef _get_maximum
#undef TSeq_get_maximum
#undef _copy
#undef TSeq_copy
#undef _copy_w_max
#undef TSeq_copy_w_max
#undef _loan_discontiguous
#undef TSeq_loan_discontiguous
#undef _unloan
#undef TSeq_unloan
#undef _loan_contiguous
#undef TSeq_loan_contiguous
#undef _has_ownership
#undef TSeq_has_ownership
#undef _initialize
#undef TSeq_initialize
#undef TSeq_isCDRCharStringType

