/*
 * FILE: REDAString.c - String API implementation
 *
 * Copyright 2012-2026 Real-Time Innovations, Inc.
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
 * 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 30jun2015,tk MICRO-1377/PR#15199 Corrected/Updated comments
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 20feb2015,eh MICRO-908/PR#11248 Remove unused function from Cert
 * 29jul2014,tk MICRO-838 Unconditional compile of free() in REDA_String_replace
 * 21may2014,as MICRO-801 REDA_String operations should always
 *              check for NULL arguments
 * 15may2014,as MICRO-317 (Verocel PR#1443) Added REDA_String_ncompare
 * 20jul2012,tk Written
 */
/*ce
 * \file
 */
/*ci
 * \addtogroup REDAStringClass
 * @{
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

/*** SOURCE_BEGIN ***/

/* Define support functions for REDA_String (initialize, finalize, copy and compare),
 * before defining REDA_StringSeq via template. The template checks if the corresponding
 * T_<operation> functions have been defined and if this is the case those are called
 * as needed.
 */

/*ci
 *
 * \brief Initialize a string sequence element to NULL
 *
 * \details
 *
 * A string sequence is implemented as an array of pointers to ASCIIZ strings.
 * Each element is initialized to NULL when a sequence is initialized. This
 * function is called by the REDA_StringSeq functions, never directly by
 * users.
 *
 * \param[in] str_ptr     String pointer to initialize
 * \param[in] max_str_len If max_str_len == 0, memory for the string is not
 *                        allocated, otherwise the string is allocated to
 *                        max_str_len + 1
 *
 * \return This function returns RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_String_initialize_ptr(char **str_ptr, RTI_UINT32 max_str_len)
{
    if (max_str_len == REDA_SEQUENCE_ELEMENT_REPLACE)
    {
        return RTI_TRUE;
    }

    if (max_str_len == REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        *str_ptr = NULL;
        return RTI_TRUE;
    }

    if (*str_ptr != NULL)
    {
        return RTI_FALSE;
    }

    *str_ptr = REDA_String_alloc(max_str_len);
    if (*str_ptr == NULL)
    {
        return RTI_FALSE;
    }

    **str_ptr = '\0';

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Finalize a string sequence element
 *
 * \details
 *
 * A string sequence is implemented as an array of pointers to ASCIIZ strings.
 * Each element is initialized to NULL when a sequence is initialized.
 * A string element in use is assumed to be pointing to a valid string
 * and when a sequence is finalized the element is finalized as well.
 * Note that the signature for a sequence initialization function is
 * the same for all types, thus in this case it can only return RTI_TRUE.
 *
 * \param[in] e String pointer to finalize
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_String_finalize_ptr(char **str_ptr)
{
    if (*str_ptr != NULL)
    {
        REDA_String_free(*str_ptr);
    }

    *str_ptr = NULL;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 *
 * \brief Copy a string sequence element
 *
 * \details
 *
 * \param[in] dst         String to copy to
 * \param[in] src         String to copy from
 * \param[in] max_str_len The maximum space in the destination buffer. If
 *                        max_str_len == 0 the memory already pointed to
 *                        by the *dst (if != NULL) is freed and sufficient
 *                        memory needed to hold the src string is allocated.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_String_copy_ptr(char **dst, const char **src, RTI_UINT32 max_str_len)
{
    char *lst_chr;
    RTI_UINT32 len = 0;

    OSAPI_PRECONDITION((*src == NULL) ||
            ((max_str_len != REDA_SEQUENCE_ELEMENT_ALLOC) &&
             (*dst == NULL)),
            return RTI_FALSE,
             OSAPI_Log_entry_add_pointer("*dst",*dst,RTI_FALSE);
             OSAPI_Log_entry_add_pointer("*src",*src,RTI_TRUE);)

    if (max_str_len == REDA_SEQUENCE_ELEMENT_ALLOC)
    {
#ifndef RTI_CERT
        if (*dst != NULL)
        {
            REDA_String_free(*dst);
        }
#endif
        len = REDA_String_length(*src);
        *dst = REDA_String_alloc(len > 0 ? len : len + 1);
        if (*dst == NULL)
        {
            return RTI_FALSE;
        }
    }
    else if (max_str_len < REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        /* calculate string length, or max_str_len+1 if unterminated */
        lst_chr = OSAPI_Memory_fndchr(*src, '\0', max_str_len+1);
        len = (lst_chr ? (RTI_UINT32)(lst_chr - *src) : max_str_len + 1U);

        /* max_str_len is IDL length, which does not include terminating NUL
           all strings in CDR samples have been allocated at mxStrLen + 1 */
        if (len >  max_str_len)
        {
            return RTI_FALSE;
        }
    }
    else if (max_str_len == REDA_SEQUENCE_ELEMENT_REPLACE)
    {
        len = REDA_String_length(*src);
    }
    else
    {
        return RTI_FALSE;
    }

    if (len == UINT_MAX)
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_copy((void*)*dst,(void*)*src,len + 1);

    return RTI_TRUE;
}

/*ci
 *
 * \brief Compare two string sequence elements
 *
 * \details
 *
 * Compare two string sequence elements
 *
 * \param[in] left  left side in comparison
 * \param[in] right right side in comparison
 *
 * \return positive integer if left is greater than right,
 *         negative integer if left is less than right,
 *         zero if left is equal to right
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_String_compare_ptr(const char **left, const char **right)
{
    if ((left == NULL) || (right == NULL))
    {
        return 0;
    }

    if ((*left == NULL) && (*right == NULL))
    {
        return 0;
    }

    return REDA_String_compare(*left,*right);
}

/* The REDA_StringSeq type is defined at the REDA_SEQUENCE_API_CORE level
 * but has also TSeq_is_equal defined (form the FULL level).
 *
 */
#define T char*
#define TSeq REDA_StringSeq
#define REDA_SEQUENCE_USER_API
#define T_initialize REDA_String_initialize_ptr
#define T_finalize   REDA_String_finalize_ptr
#define T_copy       REDA_String_copy_ptr
#define T_compare    REDA_String_compare_ptr
#define TSeq_isCDRStringType
#define TSeq_is_equal
#include "reda/reda_sequence_defn.h"

char*
REDA_String_alloc(RTI_SIZE_T length)
{
    char *string = NULL;

    OSAPI_PRECONDITION_ALWAYS((length == RTI_SIZE_INVALID),
                           return NULL,
                           OSAPI_Log_entry_add_uint("length",length,RTI_TRUE);)

    OSAPI_Heap_allocate_string(&string, length);

    return string;
}

#ifndef RTI_CERT
void
REDA_String_free(char *string)
{
    if (string == NULL)
    {
        return;
    }

    OSAPI_PRECONDITION_ALWAYS(string == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("string",string,RTI_TRUE);)

    OSAPI_Heap_free_string(string);
}
#endif /* !RTI_CERT */

char*
REDA_String_dup(const char *string)
{
    char *clone = NULL;
    RTI_SIZE_T len;

    OSAPI_PRECONDITION_ALWAYS(string == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("string",string,RTI_TRUE);)

    len = OSAPI_String_length(string);

    clone = REDA_String_alloc(len);
    if (clone == NULL)
    {
        REDA_LOG_STRING_ALLOC_FAILED(OSAPI_LOGKIND_ERROR,len+1)
        goto done;
    }

    OSAPI_Memory_copy(clone,string,len+1);

done:
    return clone;
}

char*
REDA_String_replace(char **string_ptr, const char *new_value)
{
    char *result = NULL;
    RTI_SIZE_T len;

    OSAPI_PRECONDITION_ALWAYS(string_ptr == NULL,
                return NULL,
                OSAPI_Log_entry_add_pointer("string_ptr",string_ptr,RTI_TRUE);)

    if (new_value == NULL)
    {
#ifndef RTI_CERT
        REDA_String_free(*string_ptr);
#endif
        *string_ptr = result = NULL;
    }
    else
    {
        len = OSAPI_String_length(new_value)+1;

    /* Assume Cert does not support realloc */
#if (defined(RTI_CERT) || OSAPI_DONT_HAVE_REALLOC)
#if !defined(RTI_CERT)
        REDA_String_free(*string_ptr);
#endif
        *string_ptr = result = (char *)OSAPI_Heap_allocate(1, len);
        if (result)
        {
            OSAPI_Memory_copy(*string_ptr,new_value,len);
        }
#else
        /* realloc can return NULL but in that case the input parameter buffer
         * is not released, so we need to store the result in a different 
         * variable
         */
        result = (char *)OSAPI_Heap_realloc(*string_ptr,len);

        if (result)
        {
            *string_ptr = result;
            OSAPI_Memory_copy(*string_ptr,new_value,len);
        }
        else
        {
            REDA_String_free(*string_ptr);
            *string_ptr = NULL;
        }
#endif
    }

    return result;
}

RTI_SIZE_T
REDA_String_length(const char *string)
{
    OSAPI_PRECONDITION_ALWAYS(string == NULL,
                   return RTI_SIZE_INVALID,
                   OSAPI_Log_entry_add_pointer("string",string,RTI_TRUE);)

    return OSAPI_String_length(string);
}

RTI_INT32
REDA_String_compare(const char *left, const char *right)
{

    if ((left == NULL) && (right == NULL))
    {
        return 0;
    }

    if (left == NULL)
    {
        return -1;
    }

    if (right == NULL)
    {
        return 1;
    }

    return OSAPI_String_cmp(left, right);
}

RTI_INT32
REDA_String_ncompare(const char *left, const char *right, RTI_SIZE_T num)
{

    if ((left == NULL) && (right == NULL))
    {
        return 0;
    }

    if (left == NULL)
    {
        return -1;
    }

    if (right == NULL)
    {
        return 1;
    }

    return OSAPI_String_ncmp(left, right, num);
}

RTI_BOOL
REDA_String_copy(char *dst,RTI_SIZE_T max_length,const char *src)
{
    RTI_SIZE_T len;

    OSAPI_PRECONDITION((dst == NULL) || (src == NULL) || (max_length == 0),
               return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("dst",dst,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("src",src,RTI_FALSE);
               OSAPI_Log_entry_add_uint("max_length",max_length,RTI_TRUE);)

    len = REDA_String_length(src);
    if (len > max_length)
    {
        return RTI_FALSE;
    }

    if (len == RTI_SIZE_MAX)
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(dst,src,len+1);

    return RTI_TRUE;
}

RTI_BOOL
REDA_String_copy_w_max(char **dst, const char *src, RTI_SIZE_T max_length)
{
    OSAPI_PRECONDITION((dst == NULL) || (max_length == 0),
               return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("dst",*dst,RTI_TRUE);
               OSAPI_Log_entry_add_pointer("src",src,RTI_TRUE);
               OSAPI_Log_entry_add_uint("max_length",max_length,RTI_TRUE);)

    if (src != NULL)
    {
        if (*dst == NULL)
        {
            *dst = REDA_String_alloc(max_length);
            if (*dst == NULL)
            {
                return RTI_FALSE;
            }
        }
        if (!REDA_String_copy(*dst, max_length, src))
        {
            return RTI_FALSE;
        }
    }
    else if (*dst != NULL)
    {
        /* Copy requires that src and dst are equal after the call. The only
         * correct way to handle src == NULL && dst != NULL is to free the
         * dst string and set it to NULL. Users of this API should ensure
         * that src is not NULL if they want to not deallocate the dst string.
         */
#ifndef RTI_CERT
        REDA_String_free(*dst);
#endif
        *dst = NULL;
    }

    return RTI_TRUE;
}

/*ci @} */
