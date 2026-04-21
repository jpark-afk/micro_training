/*
 * FILE: REDASequence.c - Sequence implementation
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   REDA_Sequence_set_buffer
 * 23feb2021,tk MICRO-2917/PR#28872
 *   - Removed precondition set_maximum() since this is not a public API.
 * 23feb2021,tk MICRO-2861/PR#28592
 *   - Removed precondition checks from copy() since this is not a public API.
 * 20feb2021,tk MICRO-2862/PR#28594
 *   - Simplified REDA_Sequence_copy() by removing the use of el_size
 *     and assigning self->_element_size if self is resized.
 * 17feb2021,tk MICRO-2869/PR#28745
 *   - Only enable robustness check on left->_element_size in
 *     REDA_Sequence_is_equal in Debug builds when preconditions are
 *     enabled to avoid dead code with correct use.
 * 04jan2021,tk MICRO-2767/PR#28488
 *   - Removed checking element size in REDA_Sequence_is_equal() for RTI_CERT
 *     as the function already handles it and allows uninitialized sequences
 *     of length 0 to be compared. This is the same as for non RTI_CERT.
 *   - Move the test on element size _after_ both sequences must be of equal
 *     length with length > 0.
 * 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq 
 * 12mar2015,tk MICRO-1094/PR#14142 Removed redundant code _copy()
 * 02dec2014,tk MICRO-965/PR#12386 Removed redundant code from set_maximum()
 * 14oct2014,tk MICRO-945/PR#11899 Enabled precondition checks for public
 *                                 REDA APIs for Cert.
 * 16sep2014,tk MICRO-905/PR#11239 Always clear discontinuous loan flag when
 *                                  doing a contiguous loan
 * 08may2014,eh MICRO-169/Verocel PR #1047: updated copy()
 * 18apr2014,tk MICRO-168/Verocal PR #1046: Simplified set_maximum for Cert
 * 07oct2013,tk MICRO-702: Removed test/debug statements
 * 09aug2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Implementation of the generic REDA sequence API
 */
#ifndef util_makeheader_h
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif

#include "REDASequence.h"

/*** SOURCE_BEGIN ***/

/* ------------------------------------------------------------------
 * Public Methods
 * ------------------------------------------------------------------*/
RTI_BOOL
REDA_Sequence_initialize(struct REDA_Sequence *self, RTI_INT32 element_size)
{
#ifdef RTI_CERT
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (element_size <= 0),
                  return RTI_FALSE,
                  OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                  OSAPI_Log_entry_add_int("element_size",element_size,RTI_TRUE);)
#else
    OSAPI_PRECONDITION((self == NULL) || (element_size <= 0),
               return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_int("element_size",element_size,RTI_TRUE);)
#endif

    self->_contiguous_buffer = NULL;
    self->_maximum = 0;
    self->_length = 0;
    self->_element_size = element_size;
    self->_flags = 0;
    self->_token1 = NULL;
    self->_token2 = NULL;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
REDA_Sequence_finalize(struct REDA_Sequence *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) ||
                      !REDA_Sequence_has_ownership(self),
                      return RTI_FALSE,
                      OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                      OSAPI_Log_entry_add_int("ownership",
                self != NULL ? REDA_Sequence_has_ownership(self) : 0,RTI_TRUE);)

    if (self->_contiguous_buffer != NULL)
    {
        OSAPI_Heap_free_buffer((char *)self->_contiguous_buffer);
        self->_contiguous_buffer = NULL;
    }

    self->_element_size = 0;
    self->_length = 0;
    self->_maximum = 0;
    self->_flags = 0;
    self->_token1 = NULL;
    self->_token2 = NULL;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_INT32
REDA_Sequence_get_maximum(const struct REDA_Sequence * self)
{
#ifdef RTI_CERT
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
            return -1,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
#else
    OSAPI_PRECONDITION((self == NULL),
            return -1,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
#endif

    return self->_maximum;
}

#ifdef RTI_CERT
RTI_BOOL
REDA_Sequence_set_maximum(struct REDA_Sequence *self, RTI_INT32 new_max,
                          RTI_BOOL copy_content)
{
    void *new_buffer = NULL;
    UNUSED_ARG(copy_content);

    /* VEROCEL #1046:
     * Calling set_maximum with the current size is ok. No new allocation is
     * needed (new_max > 0)
     */
    if (new_max == self->_maximum)
    {
        return RTI_TRUE;
    }

    /* VEROCEL #1046:
     * Do not allow resizing of a sequence. If the sequence has already been
     * allocated (self->_maximum > 0) return error.
     */
    if (self->_maximum > 0)
    {
        REDA_LOG_SEQUENCE_REALLOCATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* VEROCEL #1046:
     * At this point:
     * - Sequence is initialized, but has not yet been allocated
     * - new_max > 0
     */
    OSAPI_Heap_allocate_buffer((char **)&new_buffer,
                               (RTI_SIZE_T)new_max * (RTI_SIZE_T)self->_element_size,
                               OSAPI_ALIGNMENT_DEFAULT);
    if (new_buffer == NULL)
    {
        REDA_LOG_SEQUENCE_ALLOC_FAILED(
                       OSAPI_LOGKIND_ERROR,new_max * self->_element_size,
                       OSAPI_ALIGNMENT_DEFAULT)
        return RTI_FALSE;
    }

    /* VEROCEL #1046:
     * Since it is impossible to resize an existing sequence in Cert there
     * cannot be any existing content to copy.
     */

    /* --- Swap buffers --- */
    self->_contiguous_buffer = new_buffer;
    self->_maximum = new_max;
    self->_length = 0;

    return RTI_TRUE;
}
#else
RTI_BOOL
REDA_Sequence_set_maximum(struct REDA_Sequence *self, RTI_INT32 new_max,
                          RTI_BOOL copy_content)
{
    void *new_buffer = NULL;
    void *old_buffer = NULL;
    RTI_INT32 new_length = 0;

    if (new_max == self->_maximum)
    {
        return RTI_TRUE;
    }

    if (new_max > 0)
    {
        /* -- Allocate new contiguous buffer --- */
        OSAPI_Heap_allocate_buffer((char **)&new_buffer,
                                   (RTI_UINT32)(new_max * self->_element_size),
                                   OSAPI_ALIGNMENT_DEFAULT);
        if (new_buffer == NULL)
        {
            REDA_LOG_SEQUENCE_ALLOC_FAILED(
                    OSAPI_LOGKIND_ERROR,new_max * self->_element_size,
                    OSAPI_ALIGNMENT_DEFAULT)
            return RTI_FALSE;
        }
    }

    new_length = (self->_length < (RTI_INT32) new_max)
                                        ? (RTI_INT32) self->_length : new_max;

    /* If the sequence element is contiguos in memory, copy_content
     * is true and the entire sequence can be copied in a single copy
     * Otherwise a deep copy of each element must be performed using the
     * supplied T_copy method.
     */
    if (copy_content && (new_length > 0))
    {
        OSAPI_Memory_copy(new_buffer,
                           self->_contiguous_buffer,
                           (RTI_UINT32)(new_length * self->_element_size));
    }

    /* --- Swap buffers --- */
    old_buffer = self->_contiguous_buffer;
    self->_contiguous_buffer = new_buffer;
    self->_maximum = new_max;
    self->_length = new_length;
    if (new_buffer == NULL)
    {
        self->_flags = 0;
    }

    /* --- Free original buffer --- */
    /* If copy_content is false, it is up to the called to free the old
     * buffer
     */
    if ((old_buffer != NULL) && copy_content)
    {
        OSAPI_Heap_free_buffer(old_buffer);
    }

    return RTI_TRUE;
}
#endif /* RTI_CERT */

RTI_INT32
REDA_Sequence_get_length(const struct REDA_Sequence * self)
{
#ifdef RTI_CERT
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
            return -1,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
#endif

    return self->_length;
}

RTI_BOOL
REDA_Sequence_set_length(struct REDA_Sequence * self, RTI_INT32 new_length)
{
#ifdef RTI_CERT
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
#endif

    if ((new_length < 0) || (new_length > self->_maximum))
    {
        REDA_LOG_SEQUENCE_INVALID_LENGTH(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    self->_length = new_length;

    return RTI_TRUE;
}

void*
REDA_Sequence_get_reference(const struct REDA_Sequence *self, RTI_INT32 i)
{
#ifdef RTI_CERT
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
#else
    OSAPI_PRECONDITION((self == NULL)||(self->_contiguous_buffer == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("self->_contiguous",
                       (self == NULL)?NULL:self->_contiguous_buffer,RTI_FALSE);
            OSAPI_Log_entry_add_int("self->_element_size",
                       (self == NULL)?0:self->_element_size,RTI_TRUE););
#endif

    if ((i < 0) || ((RTI_INT32) i >= self->_length))
    {
        REDA_LOG_SEQUENCE_INDEX_OUT_OF_BOUNDS(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (self->_flags & REDA_SEQUENCE_FLAG_DISCONTIGUOUS)
    {
        /* return the i-th pointer in array of pointers */
        return (void *)( ((void **)self->_contiguous_buffer)[i] );
    }

    /* contiguous */
    return (void *)((char*)self->_contiguous_buffer + (i * self->_element_size));

}

struct REDA_Sequence*
REDA_Sequence_copy(struct REDA_Sequence *self, const struct REDA_Sequence *src,
                   RTI_BOOL copy_content)
{
    RTI_INT32 self_max = 0;
    RTI_INT32 src_max = 0;

#ifndef RTI_CERT
#if STRICTER_PRECOND
    OSAPI_PRECONDITION(((self == NULL) || (src == NULL) ||
            ((self->_element_size > 0) && (src->_element_size > 0) &&
            (self->_element_size != src->_element_size)) ||
            src->_element_size == 0),
            return NULL,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("src",src,RTI_FALSE);
            OSAPI_Log_entry_add_int("self->_element_size",self != NULL ? self->_element_size : 0,RTI_FALSE);
            OSAPI_Log_entry_add_int("src->_element_size",src != NULL ? src->_element_size : 0,RTI_TRUE);)
#else
    OSAPI_PRECONDITION(((self == NULL) || (src == NULL) ||
            ((self->_element_size > 0) && (src->_element_size > 0) &&
            (self->_element_size != src->_element_size))),
            return NULL,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("src",src,RTI_FALSE);
            OSAPI_Log_entry_add_int("self->_element_size",self != NULL ? self->_element_size : 0,RTI_FALSE);
            OSAPI_Log_entry_add_int("src->_element_size",src != NULL ? src->_element_size : 0,RTI_TRUE);)
#endif
#endif

    /*if copy content is true, we cannot have discontiguous buffers here*/
    if ((copy_content) && (REDA_Sequence_has_discontiguous_buffer(self) ||
                REDA_Sequence_has_discontiguous_buffer(src)))
    {
        return NULL;
    }
    
    if (self == src)
    {
        return self;
    }
        
    self_max = REDA_Sequence_get_maximum(self);
    if (self_max < REDA_Sequence_get_length(src))
    {
        /* In this condition the src must have at least one
         * element, which means that src->_element_size _must_
         * be set. Thus, if self->_element_size is not set (0) it
         * means self_max also must be 0, so assign self->_element_size
         * here. This is done regardless of whether copy_content is
         * TRUE/FALSE.
         */
        if (self->_element_size == 0)
        {
            self->_element_size = src->_element_size;
        }

        /* For Cert, reallocation is not allowed by REDA_Sequence_get_maximum(),
         * so the below set_maximum() will fail when self_max is > 0. 
         *  
         * Also, copy_content must be a parameter, as sequences of elements 
         * requiring deep copies will later call type-specific copy functions. 
         */
        src_max = REDA_Sequence_get_maximum(src);
        if (!REDA_Sequence_set_maximum(self, src_max,copy_content))
        {
            REDA_LOG_SEQUENCE_SET_MAX_FAILED(OSAPI_LOGKIND_ERROR,self,src_max)
            return NULL;
        }
    }

    self->_length = src->_length;
    self->_token1 = src->_token1;
    self->_token2 = src->_token2;

    if (src->_length == 0)
    {
        return self;
    }

    if (copy_content)
    {
        if (self->_contiguous_buffer != src->_contiguous_buffer)
        {
            OSAPI_Memory_copy(self->_contiguous_buffer,
                        src->_contiguous_buffer,
                        (RTI_UINT32)(REDA_Sequence_get_length(src) * self->_element_size));
        }
    }

    return self;
}

RTI_BOOL
REDA_Sequence_is_equal(const struct REDA_Sequence *left,
                       const struct REDA_Sequence *right,
                       RTI_BOOL compare_content)
{
    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (left->_length != right->_length)
    {
       return RTI_FALSE;
    }

    if ((left->_length == 0) || !compare_content)
    {
        return RTI_TRUE;
    }

    if (left->_element_size != right->_element_size)
    {
        REDA_LOG_SEQUENCE_INVALID_OPERATION(OSAPI_LOGKIND_ERROR,
                left->_length,right->_length,
                left->_element_size,right->_element_size)
        return RTI_FALSE;
    }

#if OSAPI_ENABLE_PRECONDITION
    if (left->_element_size == 0)
    {
        REDA_LOG_SEQUENCE_INVALID_OPERATION(OSAPI_LOGKIND_ERROR,
                left->_length,right->_length,
                left->_element_size,right->_element_size)
        return RTI_FALSE;
    }
#endif

    return !OSAPI_Memory_compare(left->_contiguous_buffer,
                                 right->_contiguous_buffer,
                                 (RTI_UINT32)(left->_length * left->_element_size));
}

RTI_BOOL
REDA_Sequence_loan_contiguous(struct REDA_Sequence *self,
                              void *buffer,
                              RTI_INT32 new_length,
                              RTI_INT32 new_max)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL || buffer == NULL ||
            !REDA_Sequence_has_ownership(self),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_FALSE);
            OSAPI_Log_entry_add_int("own",
            REDA_Sequence_has_ownership(self),RTI_TRUE);)

    self->_contiguous_buffer = buffer;
    self->_length = new_length;
    self->_maximum = new_max;
        
    /* mark seq as using loaned buffer */
    self->_flags |= REDA_SEQUENCE_FLAG_LOAN;
    self->_flags &= (RTI_UINT8)~REDA_SEQUENCE_FLAG_DISCONTIGUOUS;

    return RTI_TRUE;
}

RTI_BOOL
REDA_Sequence_loan_discontiguous(struct REDA_Sequence *self,
                              void *buffer,
                              RTI_INT32 new_length,
                              RTI_INT32 new_max)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL || buffer == NULL ||
            !REDA_Sequence_has_ownership(self),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_FALSE);
            OSAPI_Log_entry_add_int("own",
                (self != NULL)?REDA_Sequence_has_ownership(self):0,RTI_TRUE);)

    self->_contiguous_buffer = buffer;
    self->_length = new_length;
    self->_maximum = new_max;
        
    /* flagged as both loaned and discontinuous */
    self->_flags |= (REDA_SEQUENCE_FLAG_LOAN|REDA_SEQUENCE_FLAG_DISCONTIGUOUS);

    return RTI_TRUE;
}

RTI_BOOL
REDA_Sequence_has_ownership(const struct REDA_Sequence *self)
{
    return ((self == NULL) || (self->_flags & REDA_SEQUENCE_FLAG_LOAN)) ?
        RTI_FALSE : RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
REDA_Sequence_set_buffer(struct REDA_Sequence *self,
                         void *buffer)
{
    self->_contiguous_buffer = buffer;

    return RTI_TRUE;
}
#endif

void*
REDA_Sequence_get_buffer(const struct REDA_Sequence *self)
{
    return (self == NULL ? NULL : self->_contiguous_buffer);
}

RTI_BOOL
REDA_Sequence_unloan(struct REDA_Sequence *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) ||
            REDA_Sequence_has_ownership(self),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_int("ownership",
                (self != NULL)?REDA_Sequence_has_ownership(self):0,RTI_TRUE);)

    self->_contiguous_buffer = NULL;
    self->_length = 0;
    self->_maximum = 0;

    /* clear flags, as seq is unloaned, and has no buffer */
    self->_flags = 0;

    return RTI_TRUE;
}

RTI_BOOL
REDA_Sequence_has_discontiguous_buffer(const struct REDA_Sequence *self)
{

    return ((self == NULL) || 
            !(self->_flags & REDA_SEQUENCE_FLAG_DISCONTIGUOUS)) ?
            RTI_FALSE : RTI_TRUE;
}

void
REDA_Sequence_set_token(struct REDA_Sequence *self,void *token1,void *token2)
{
    OSAPI_PRECONDITION(self == NULL,
                       return,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    self->_token1 = token1;
    self->_token2 = token2;
}

void
REDA_Sequence_get_token(const struct REDA_Sequence *self,void **token1,void **token2)
{
    OSAPI_PRECONDITION((self == NULL) || (token1 == NULL) || (token2 == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("token1",token1,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("token2",token2,RTI_TRUE);)

    *token1 = self->_token1;
    *token2 = self->_token2;
}

