/*
 * FILE: posixString.c - POSIX string functions
 *
 * Copyright (c) 2013-2026 Real-Time Innovations, Inc.
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
 * 23dec2013,tk Written, based on osapi_string_impl.h
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI string routines
 */
#include "rti_me_psl.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
    #define HAVE_STRNLEN 1
#elif defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 10))
    #define HAVE_STRNLEN 1
#else
    #define HAVE_STRNLEN 0
#endif

/*** SOURCE_BEGIN ***/

void
OSAPI_Memory_copy(void *dest,const void *src, RTI_SIZE_T size)
{
    (void)memcpy(dest,(void*)src,size);
}

void
OSAPI_Memory_zero(void *mem, RTI_SIZE_T size)
{
    (void)memset(mem, 0,(size_t)size);
}

RTI_INT32
OSAPI_Memory_compare(const void *left,const void *right, RTI_SIZE_T size)
{
    return memcmp((char *)left, (char *)right, size);
}

void
OSAPI_Memory_move(void *dest,const void *src, RTI_SIZE_T size)
{
    (void)memmove(dest,src,size);
}

void*
OSAPI_Memory_fndchr(const void *s, RTI_INT32 c, RTI_SIZE_T n)
{
    return memchr((const void *)s, c, n);
}

RTI_SIZE_T
OSAPI_String_length(const char *s)
{
    return (RTI_SIZE_T)strlen(s);
}

RTI_INT32
OSAPI_String_cmp(const char *left,const char *right)
{
    return strcmp(left,right);
}

RTI_INT32
OSAPI_String_ncmp(const char *left,const char *right, RTI_SIZE_T num)
{
    return strncmp(left,right,num);
}

RTI_BOOL
OSAPI_String_parse_unsigned_long_long(const char *str, unsigned long long int *result)
{
    char *end_ptr = NULL;

    *result = strtoull(str,&end_ptr,0);

    if (!(*result) && (errno == ERANGE))
    {
        return RTI_FALSE;
    }

    if (end_ptr == str)
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTI_SIZE_T
OSAPI_String_length_w_max(const char *s,RTI_SIZE_T max)
{
#if HAVE_STRNLEN
    return (RTI_SIZE_T)strnlen(s,(size_t)max);
#else
    RTI_SIZE_T length = 0;

    while (length < max && s[length] != '\0')
    {
        length++;
    }

    return length;
#endif
}

int
OSAPI_String_char_to_lowercase(const int c)
{
    /* c shall be representable as an unsigned char or be the value EOF */
    if (c < 0)
    {
        return c;
    }

    return tolower((unsigned char)c);
}

char*
OSAPI_String_strchr(const char *s, int c)
{
    return strchr(s,c);
}

#if DDS_FILTERING_ENABLED
RTI_BOOL
OSAPI_String_parse_float(const char *str, RTI_FLOAT32 *result)
{
    char *end_ptr = NULL;
    float temp_result;

    errno = 0;

    /* Supress a warning that we do not check the value of temp_result. We do
     * not want to treat NaN or Inf as an error if that is explicitly what
     * the user specified. We instead check errno to see if that value was
     * returned because of a range error, but Coverity does not recognized that
     * as a valid error check.
     */
    /* coverity[cert_err33_c_violation] */
    /* coverity[cert_pos54_c_violation] */
    temp_result = strtof(str, &end_ptr);

    /* Check for parsing error */
    if (end_ptr == str)
    {
        return RTI_FALSE;
    }

    /* Check for range error */
    if (errno == ERANGE)
    {
        return RTI_FALSE;
    }

    *result = temp_result;
    return RTI_TRUE;
}

RTI_BOOL
OSAPI_String_parse_double(const char *str, RTI_DOUBLE64 *result)
{
    char *end_ptr = NULL;
    double temp_result;

    errno = 0;

    /* Supress a warning that we do not check the value of temp_result. We do
     * not want to treat NaN or Inf as an error if that is explicitly what
     * the user specified. We instead check errno to see if that value was
     * returned because of a range error, but Coverity does not recognized that
     * as a valid error check.
     */
    /* coverity[cert_err33_c_violation] */
    /* coverity[cert_pos54_c_violation] */
    temp_result = strtod(str, &end_ptr);

    /* Check for parsing error */
    if (end_ptr == str)
    {
        return RTI_FALSE;
    }

    /* Check for range error */
    if (errno == ERANGE)
    {
        return RTI_FALSE;
    }

    *result = temp_result;
    return RTI_TRUE;
}
#endif /* DDS_FILTERING_ENABLED */
