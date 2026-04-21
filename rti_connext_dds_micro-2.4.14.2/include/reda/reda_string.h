/*
 * FILE: reda_string.h - String API
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   REDA_String_replace
 * 10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq 
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 15may2014,as MICRO-317 (Verocel PR#1443) Added REDA_String_ncompare
 * 20jul2012,tk Written
 */
/*ci
 * \file
 *
 * \brief The REDA string API provides commonly used functions to manipulate
 * C style ASCIIZ strings beyond what OSAPI provides.
 *
 * \defgroup REDAStringClass REDA String
 *
 * \ingroup REDAModule
 *
 * \details
 *
 * TThe REDA string API provides frequently used functions to manipulate
 * C ASCIIZ strings beyond what OSAPI provides. In general REDA does access
 * platform dependent functions, but uses OSAPI ass the basis. The API
 * defines the REDA_String_T as a convenience, but it is guaranteed to
 * be compatible with char*
 */
#ifndef reda_string_h
#define reda_string_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#include "reda/reda_sequence.h"


#ifdef __cplusplus
extern "C"
{
#endif
/*ci
 * \addtogroup REDAStringClass
 * @{
 */

/*ci
 * \brief Alias for char*
 */
typedef char* REDA_String_T;

/*
 * define the REDA_StringSeq
 */
#define T char*
#define TSeq REDA_StringSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#define TSeq_isCDRStringType
#define TSeq_isCDRCharStringType
#include <reda/reda_sequence_decl.h>

/*ci
 * \def REDA_StringSeq_INITIALIZER
 * \brief Initializer for REDA_StringSeq variables
 */
#define REDA_StringSeq_INITIALIZER REDA_DEFINE_SEQUENCE_INITIALIZER(char*)

/*ci
 * \brief Allocate a string of length bytes, excluding the NULL character.
 *
 * \details
 *
 * Allocate a buffer to hold a string of length bytes, excluding the
 * NULL termination.
 *
 * \param[in] length Length of string to allocate, does not include the
 *                   NULL termination character.
 *
 * \return Valid buffer on success, NULL on failure
 *
 * \sa \ref REDA_String_free
 */
MUST_CHECK_RETURN REDADllExport char*
REDA_String_alloc(RTI_SIZE_T length);

#ifndef RTI_CERT
/*ci
 * \brief Free a string buffer allocated with \ref REDA_String_alloc
 *
 * \details
 *
 * Free a string buffer allocated with \ref REDA_String_alloc
 *
 * \param[in] string String buffer to free
 *
 * \sa \ref REDA_String_alloc
 */
REDADllExport void
REDA_String_free(char *string);
#endif /* !RTI_CERT */

/*ci
 * \brief Return the length of a string excluding the NULL termination
 *
 * \details
 *
 * Return the length of a string, does not include the NULL termination
 *
 * \param[in] string String buffer to return length of
 *
 * \return Return the length of the string excluding the NULL termination
 */
REDADllExport RTI_SIZE_T
REDA_String_length(const char *string);

/*ci
 * \brief Return a duplicate of a string
 *
 * \details
 *
 * Allocate a new buffer and copy the content of a string.
 *
 * \param[in] string String to duplicate
 *
 * \return Returns a pointer to the duplicate string on success, NULL on failure
 */
MUST_CHECK_RETURN REDADllExport char*
REDA_String_dup(const char *string);

#ifndef RTI_CERT
/*ci
 * \brief Replace the content of a previously allocated string buffer
 *
 * \details
 *
 * This function replaces the content of a previously allocated string buffer.
 * If the replacement string is longer than the previously allocated string
 * a new buffer is allocated and the content copied.
 *
 * \param[in] string_ptr String to replace
 * \param[in] new_value  String replacement
 *
 * \return Returns a pointer to the replaced string on success, NULL on failure
 */
MUST_CHECK_RETURN REDADllExport char*
REDA_String_replace(char **string_ptr, const char *new_value);
#endif

/*ci
 * \brief Compare two strings
 *
 * \details
 *
 * This function compares two strings for lexicographic equivalence.
 *
 * \param[in] left  Left side of comparison
 *
 * \param[in] right Right side of comparison
 *
 * \return Returns 0 if left is == right, < 0 if left < right, > 0 if left > right
 *
 * \sa \ref REDA_String_ncompare
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_String_compare(const char *left, const char *right);

/*ci
 * \brief Compare two strings
 *
 * \details
 *
 * This function compares two strings for lexicographic equivalence, but only
 * up to num bytes.
 *
 * \param[in] left  Left side of comparison
 *
 * \param[in] right Right side of comparison
 *
 * \param[in] num   Maximum number of bytes to compare
 *
 * \return positive integer if left is greater than right,
 *         negative integer if left is less than right,
 *         zero if left is equal to right
 *
 * \sa \ref REDA_String_compare
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_String_ncompare(const char *left, const char *right, RTI_SIZE_T num);

/*ci
 * \brief Copy a string into a buffer of maximum length
 *
 * \details
 * Copy a NUL terminated string into a buffer with a maximum length. If the
 * source string is to long, RTI_FALSE is returned.
 *
 * \param[in] dst        Destination string buffer
 * \param[in] max_length Maximum length (excluding NUL) of destination
 *                       buffer
 * \param[in] src Source string to copy
 *
 * \return Returns RTI_TRUE on success, RTI_FALSE on failure
 */
REDADllExport RTI_BOOL
REDA_String_copy(char *dst,RTI_SIZE_T max_length,const char *src);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_string_h */

/*ci @} */

