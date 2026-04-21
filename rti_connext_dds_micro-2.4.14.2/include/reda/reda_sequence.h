/*
 * FILE: reda_sequence.h - Sequence API
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   REDA_Sequence_set_buffer
 * 17feb2021,tk  MICRO-2863/PR#28608
 *   - Added additional comments for REDA_SEQUENCE_FLAG_PTR_ALLOCATION.
 * 10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq 
 * 17may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 19jul2013,as  Added support for C++
 * 03jun2013,kaj MICRO-502 support for string sequences req. max string length
 * 20oct2008,tk Written
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
#ifndef reda_sequence_h
#define reda_sequence_h

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

#ifdef __cplusplus
extern "C"
{
#endif

/*\ci
 * \brief Do not define a typed API
 */
#define REDA_SEQUENCE_API_UNTYPED    0

/*\ci
 * \brief Define minimal interface available to every sequence type
 *
 * \details
 * The following typed sequence functions are defined for every sequence type:
 * TSeq_initialize
 * TSeq_get_maximum
 * TSeq_set_maximum
 * TSeq_get_length
 * TSeq_set_length
 * TSeq_get_reference
 * TSeq_finalize
 */
#define REDA_SEQUENCE_API_BASIC       1

/*\ci
 * \brief Define the full typed sequence API.
 *
 * \details
 * The BASIC API plus:
 * TSeq_copy
 * TSeq_is_equal
 * TSeq_loan_contiguous
 * TSeq_loan_discontiguous
 * TSeq_has_ownership
 * TSeq_has_discontiguous_buffer
 * TSeq_get_buffer
 * TSeq_set_buffer
 * TSeq_unloan
 * TSeq_set_token
 * TSeq_get_token
 */
#define REDA_SEQUENCE_API_FULL       2

/*ci The default sequence API for core types unless a different one is specified
 * when a sequence is declared/defined
 */
#ifndef REDA_SEQUENCE_API_DEFAULT
#ifndef RTI_CERT
#define REDA_SEQUENCE_API_DEFAULT      REDA_SEQUENCE_API_FULL
#else
#define REDA_SEQUENCE_API_DEFAULT      REDA_SEQUENCE_API_BASIC
#endif
#else
#if REDA_SEQUENCE_API_DEFAULT < REDA_SEQUENCE_API_BASIC
#error "REDA_SEQUENCE_API_DEFAULT must be at least REDA_SEQUENCE_API_BASIC"
#endif
#endif

/*ci The default sequence API for user types unless a one is specified
 * when a sequence is declared/defined
 */
#ifndef REDA_SEQUENCE_API_USER_DEFAULT
#ifndef RTI_CERT
#define REDA_SEQUENCE_API_USER_DEFAULT  REDA_SEQUENCE_API_FULL
#else
#define REDA_SEQUENCE_API_USER_DEFAULT  REDA_SEQUENCE_API_BASIC
#endif
#else
#if REDA_SEQUENCE_API_USER_DEFAULT < REDA_SEQUENCE_API_BASIC
#error "REDA_SEQUENCE_API_USER_DEFAULT must be at least REDA_SEQUENCE_API_BASIC"
#endif
#endif

/* Initialize a REDA sequence structure for a type */
#define REDA_DEFINE_SEQUENCE_INITIALIZER(t_) \
{ NULL,0,0,sizeof(t_),NULL,NULL,0 }

/* Initialize a REDA sequence structure for no type (0 size) */
#define REDA_DEFINE_EMPTY_SEQUENCE_INITIALIZER \
{ NULL,0,0,0,NULL,NULL,0 }

/* forward declaration of the REDA_Sequence structure for the API below.
 * The structure is defined in REDA_Sequence.c
 */
struct REDA_Sequence;

/*ci
 *\brief Internal flag set when sequence is using a loaned buffer.
 */
#define REDA_SEQUENCE_FLAG_LOAN  (RTI_UINT8)0x01

/*ci
 *\brief Internal flag set when the sequencebuffer contains pointers to elements
 */
#define REDA_SEQUENCE_FLAG_DISCONTIGUOUS    (RTI_UINT8)0x02

/*ci
 *\brief Internal flag used to indicate that the sequence is a sequence of
 *       pointers where each pointer is pointing to a buffer allocated to a
 *       maximum size.
 *
 * \details
 *
 * The code-generator generates code for strings from IDL which always have a
 * maximum size. Thus, it is necessary to preallocate memory for each string
 * element in a sequence of strings to avoid dynamic memory allocation while
 * samples are received. Sequence methods with the suffix w_max takes a
 * maximum string length and when it is less than REDA_SEQUENCE_ALLOC the
 * method will allocate a buffer of maximum length for each pointer element.
 *
 * It is important that a sequence created with a maximum size is not resized
 * with a different maximum size or the specical value REDA_SEQUENCE_ELEMENT_ALLOC.
 * There is no protection against this. The APIs ending in _w_max() are not
 * directly accessible as a public API and should not be used outside of
 * code-generation.
 */
#define REDA_SEQUENCE_FLAG_PTR_ALLOCATION   (RTI_UINT8)0x04

/*ci
 * \brief Constant passed as max_str_length to indicate that that memory
 *        for the pointer element should be allocated as needed
 *
 */
#define REDA_SEQUENCE_ELEMENT_ALLOC    (0xfffffffeU)

/*ci
 * \brief Constant passed as max_str_length to indicate that that memory
 *        for the pointer element should is already sized and should be
 *        assumed to be sufficiently large as a destination
 */
#define REDA_SEQUENCE_ELEMENT_REPLACE  (0xffffffffU)

/*ci
 * \brief Initialize a sequence
 *
 * \details
 *
 * Initializes a sequence structure based on the element size.
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \param[in] self         Sequence structure to initialize
 *
 * \param[in] element_size Size in bytes of the sequence structure
 *
 * \sa REDA_Sequence_finalize
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_initialize(struct REDA_Sequence *self, RTI_INT32 element_size);

#ifndef RTI_CERT
/*ci
 * \brief Finalize a sequence
 *
 * \details
 *
 * Finalize a sequence structure based
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \param[in] self         Sequence structure to finalize
 *
 * \sa REDA_Sequence_initialize
 */
SHOULD_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_finalize(struct REDA_Sequence *self);
#endif /* !RTI_CERT */

/*ci
 * \brief Get the current maximum of elements the sequence can store
 *
 * \details
 *
 * Return the current maximum number of elements a sequence can hold. The
 * caller must ensure that self is not NULL.
 *
 * \param[in] self  Sequence to return the maximum number of elements for
 *
 * \return maximum number of elements the sequence can hold.
 *
 * \sa REDA_Sequence_set_maximum
 */
REDADllExport RTI_INT32
REDA_Sequence_get_maximum(const struct REDA_Sequence *self);

/*ci
 * \brief Set the current maximum of elements the sequence can store
 *
 * \details
 *
 * Set maximum number of elements a sequence can hold. The caller must
 * ensure that self is not NULL. If new_max is larger than the current
 * max additional memory is allocated if possible. If new_max is less
 * than the current maximum memory may be freed.
 *
 * \param[in] self         Sequence to change
 * \param[in] new_max      The new maximum number of elements the sequence can
 *                         hold.
 * \param[in] copy_content Whether the content of the sequence element should
 *                         be copied or not. If copy_content is RTI_FALSE only
 *                         the element array is resized, but the content of
 *                         each element is not touched. If copy_content is
 *                         RTI_TRUE then the content of each element is
 *                         copied. This parameter is used by the typed wrapper
 *                         functions to manage elements that manage their
 *                         own memory, such as string sequences.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa REDA_Sequence_set_maximum
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_set_maximum(struct REDA_Sequence *self, RTI_INT32 new_max,
                          RTI_BOOL copy_content);

/*ci
 * \brief Get the current number of elements in the sequence
 *
 * \details
 *
 * Return the current number of elements in the sequence.
 *
 * \param[in] self Sequence to return the current number of elements for
 *
 * \return Number of elements in the sequence.
 *
 * \sa REDA_Sequence_set_length
 */
REDADllExport RTI_INT32
REDA_Sequence_get_length(const struct REDA_Sequence *self);

/*ci
 * \brief Set the current number of elements in the sequence
 *
 * \details
 *
 * Set the current number of elements in the sequence. Note that this function
 * does not expand the sequence, rather it changes how many of the currently
 * maximum number of elements are valid. Thus it is illegal to specify
 * a new_length > get_maximum(self)
 *
 * \param[in] self       Sequence to set the current number of elements for
 * \param[in] new_length The number of elements in the sequence
 *
 * \return RTI_TRUE if the length was successfully changed, RTI_FALSE otherwise.
 *
 * \sa REDA_Sequence_get_length
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_set_length(struct REDA_Sequence *self, RTI_INT32 new_length);

/*ci
 * \brief Return the underlying buffer holding sequence elements
 *
 * \details
 *
 * This function returns the underlying buffer holding up to max elements.
 * It is guaranteed that the buffer is a contiguous memory area of at least
 * element_size * maximum bytes.
 *
 * \param[in] self Sequence to get the current buffer for
 *
 * \return Current buffer or NULL if the sequence is not initialized.
 *
 * \sa REDA_Sequence_set_buffer
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Sequence_get_buffer(const struct REDA_Sequence *self);

#ifndef RTI_CERT
/*ci
 * \brief Set the underlying buffer holding sequence elements
 *
 * \details
 *
 * This function sets the underlying buffer holding up to max elements.
 * It is guaranteed that the buffer is a contiguous memory area of at least
 * element_size * maximum bytes.
 *
 * \param[in] self Sequence to set the current buffer for
 *
 * \param[in] buffer Buffer with sequence elements
 *
 * \return Current buffer or NULL if the sequence is not initialized.
 *
 * \sa REDA_Sequence_set_buffer
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_set_buffer(struct REDA_Sequence *self, void *buffer);
#endif

/*ci
 * \brief Return a pointer to an element in the vector
 *
 * \details
 *
 * This function returns a pointer (reference) to an element in the vector
 * given an index. The indexing is 0-based. If an index higher than the current
 * length is specified NULL is returned.
 *
 * \param[in] self  Sequence to return the element from
 * \param[in] index Which element to return
 *
 * \return A reference to element index is returned, or NULL if the index
 * is not within the current length.
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Sequence_get_reference(const struct REDA_Sequence *self,RTI_INT32 index);

/*ci
 * \brief Copy the content of one sequence to another
 *
 * \details
 *
 * This function copies the content of the source sequence to the destination
 * sequence, expanding the destination sequence if needed. The length of the
 * destination will be equal to the source after the successful copying.
 * it is not guaranteed that the maximum length of the two sequences are equal,
 * only that the destination sequence is at least as large as the source
 * sequence.
 *
 * \param[in] self         Sequence to copy to
 * \param[in] src          Sequence to copy from
 * \param[in] copy_content Whether the content of the sequence element should
 *                         be copied or not. If copy_content is RTI_FALSE only
 *                         the element array is resized, but the content of
 *                         each element is not touched. If copy_content is
 *                         RTI_TRUE then the content of each element is
 *                         copied. This parameter is used by the typed wrapper
 *                         functions to manage elements that manage their
 *                         own memory, such as string sequences.
 *
 * \return A reference to destination sequence is returned on success, otherwise
 *         NULL is returned.
 */
MUST_CHECK_RETURN REDADllExport struct REDA_Sequence*
REDA_Sequence_copy(struct REDA_Sequence *self,
                   const struct REDA_Sequence *src,
                   RTI_BOOL copy_content);

/*ci
 * \brief Compare two sequences
 *
 * \details
 *
 * Compare two function for equality. Two sequences are considered to be equal
 * if the sequences are of the same length and the content of each element
 * in the same position is identical.
 *
 * \param[in] left            Left side of the comparison
 * \param[in] right           Right side of the comparison
 * \param[in] compare_content Whether the content of the sequence element should
 *                            be compared or not. If compare_content is
 *                            RTI_FALSE only the element array sizes are
 *                            compared, but the content of
 *                            each element is not compared. If compare_content
 *                            is RTI_TRUE then the content of each element is
 *                            compared. This parameter is used by the typed
 *                            wrapper functions to manage elements that manage
 *                            their own memory, such as string sequences.
 *
 * \return RTI_TRUE if the sequences are equal, RTI_FALSE otherwise.
 */
REDADllExport RTI_BOOL
REDA_Sequence_is_equal(const struct REDA_Sequence *left,
                       const struct REDA_Sequence *right,
                       RTI_BOOL compare_content);

/*ci
 * \brief Loan an external, contiguous buffer to a sequence
 *
 * \details
 *
 * Typically a sequence manages its own memory. However, it is possible to
 * loan the sequence a buffer of elements. When a sequence is loaning a
 * buffer it is not allowed to resize it. The caller must first unloan the
 * sequence and then loan the sequence a different buffer. In a contiguous
 * buffer it is assumed that each element is contained in the buffer. It
 * is not legal to loan a buffer to a sequence currently managing its own
 * element buffer. In this case the sequence must first be finalized.
 *
 * \param[in] self       Sequence to loan buffer to
 * \param[in] buffer     Buffer to loan
 * \param[in] new_length The number of valid elements in the buffer
 * \param[in] new_max    The maximum number of elements the buffer can hold
 *
 * \return RTI_TRUE is returned if the loan is successful, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_loan_contiguous(struct REDA_Sequence *self, void *buffer,
                              RTI_INT32 new_length, RTI_INT32 new_max);

/*ci
 * \brief Loan an external, discontiguous buffer to a sequence
 *
 * \details
 *
 * Typically a sequence manages its own memory. However, it is possible to
 * loan the sequence a buffer of elements. When a sequence is loaning a
 * buffer it is not allowed to resize it. The caller must first unloan the
 * sequence and then loan the sequence a different buffer. In a discontiguous
 * buffer it is assumed that each element is a reference to the actual element
 * contained in the buffer. It is not legal to loan a buffer to a sequence
 * currently managing its own element buffer. In this case the sequence must
 * first be finalized.
 *
 * \param[in] self       Sequence to loan buffer to
 * \param[in] buffer     Buffer to loan
 * \param[in] new_length The number of valid elements in the buffer
 * \param[in] new_max    The maximum number of elements the buffer can hold
 *
 * \return RTI_TRUE is returned if the loan is successful, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_loan_discontiguous(struct REDA_Sequence *self, void *buffer,
                                 RTI_INT32 new_length, RTI_INT32 new_max);
/*ci
 * \brief Remove a loan of an external buffer to a sequence
 *
 * \details
 *
 * Remove a previously loaned buffer from a sequence.
 *
 * \param[in] self  Sequence to unloan the buffer from
 *
 * \return RTI_TRUE is returned if the unloan is successful, RTI_FALSE otherwise
 *
 * \sa \ref REDA_Sequence_loan_discontiguous, \ref REDA_Sequence_loan_contiguous
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Sequence_unloan(struct REDA_Sequence *self);

/*ci
 * \brief Check if a sequence owns the current element buffer or not
 *
 * \details
 *
 * Check if a sequence owns the current element buffer or not
 *
 * \param[in] self  Sequence to check ownership for
 *
 * \return RTI_TRUE if the sequence owns the buffer, RTI_FALSE if not
 *
 * \sa \ref REDA_Sequence_loan_discontiguous, \ref REDA_Sequence_loan_contiguous
 */
REDADllExport RTI_BOOL
REDA_Sequence_has_ownership(const struct REDA_Sequence *self);

/*ci
 * \brief Check if a sequence uses a contiguous or discontinuous buffer
 *
 * \details
 *
 * Check if a sequence uses a contiguous or discontinuous buffer
 *
 * \param[in] self  Sequence to check ownership for
 *
 * \return RTI_TRUE if the sequence is contiguous, RTI_FALSE if not
 *
 * \sa \ref REDA_Sequence_loan_discontiguous
 */
REDADllExport RTI_BOOL
REDA_Sequence_has_discontiguous_buffer(const struct REDA_Sequence *self);

/*ci
 * \brief Set a token on a sequence
 *
 * \details
 * A token is an opaque pointer stored in the sequence. It has no meaning
 * to the sequence and is not considered part of the content.
 *
 * \param[in] self   Sequence to get the current buffer for
 * \param[in] token1 The first token to associate with the sequence
 * \param[in] token2 The second token to associate with the sequence
 *
 * \sa REDA_Sequence_get_token
 */
REDADllExport void
REDA_Sequence_set_token(struct REDA_Sequence *self,void *token1,void *token2);

/*ci
 * \brief Get the token from a sequence
 *
 * \details
 * A token is an opaque pointer stored in the sequence. It has no meaning
 * to the sequence and is not considered part of the content.
 *
 * \param[in]  self   Sequence to get the current buffer for
 * \param[out] token1 The first token associated with the sequence
 * \param[out] token2 The second token associated with the sequence
 *
 * \sa REDA_Sequence_set_token
 */
REDADllExport void
REDA_Sequence_get_token(const struct REDA_Sequence *self,void **token1,void **token2);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_sequence_h */

/*ci @} */
