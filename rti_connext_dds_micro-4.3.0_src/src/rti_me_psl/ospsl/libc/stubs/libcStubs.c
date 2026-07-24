/*
 * FILE: libcStubs.c - Stubbed String functions
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
#include "rti_me_psl.h"

/*** SOURCE_BEGIN ***/

void
OSAPI_Memory_copy(void *dest,const void *src, RTI_SIZE_T size)
{
    UNUSED_ARG(dest);
    UNUSED_ARG(src);
    UNUSED_ARG(size);
}

void
OSAPI_Memory_zero(void *mem, RTI_SIZE_T size)
{
    UNUSED_ARG(mem);
    UNUSED_ARG(size);
}

RTI_INT32
OSAPI_Memory_compare(const void *left,const void *right, RTI_SIZE_T size)
{
    UNUSED_ARG(left);
    UNUSED_ARG(right);
    UNUSED_ARG(size);

    return -1;
}

void
OSAPI_Memory_move(void *dest,const void *src, RTI_SIZE_T size)
{
    UNUSED_ARG(dest);
    UNUSED_ARG(src);
    UNUSED_ARG(size);
}

void*
OSAPI_Memory_fndchr(const void *s, RTI_INT32 c, RTI_SIZE_T n)
{
    UNUSED_ARG(s);
    UNUSED_ARG(c);
    UNUSED_ARG(n);

    return NULL;
}

RTI_SIZE_T
OSAPI_String_length(const char *s)
{
    UNUSED_ARG(s);

    return 0;
}

RTI_INT32
OSAPI_String_cmp(const char *left,const char *right)
{
    UNUSED_ARG(left);
    UNUSED_ARG(right);

    return -1;
}

RTI_INT32
OSAPI_String_ncmp(const char *left,const char *right, RTI_SIZE_T num)
{
    UNUSED_ARG(left);
    UNUSED_ARG(right);
    UNUSED_ARG(num);

    return -1;
}

RTI_BOOL
OSAPI_String_parse_unsigned_long_long(const char *str, unsigned long long int *result)
{
    UNUSED_ARG(str);
    UNUSED_ARG(result);

    return RTI_FALSE;
}

RTI_SIZE_T
OSAPI_String_length_w_max(const char *s,RTI_SIZE_T max)
{
    UNUSED_ARG(s);
    UNUSED_ARG(max);

    return 0;
}

int
OSAPI_String_char_to_lowercase(const int c)
{
    UNUSED_ARG(c);

    return 0;
}

char*
OSAPI_String_strchr(const char *s, int c)
{
    UNUSED_ARG(s);
    UNUSED_ARG(c);

    return NULL;
}

#if DDS_FILTERING_ENABLED
RTI_BOOL
OSAPI_String_parse_float(const char *str, float *result)
{
    UNUSED_ARG(str);
    UNUSED_ARG(result);

    return 0;
}

RTI_BOOL
OSAPI_String_parse_double(const char *str, double *result)
{
    UNUSED_ARG(str);
    UNUSED_ARG(result);

    return 0;
}
#endif /* DDS_FILTERING_ENABLED */
