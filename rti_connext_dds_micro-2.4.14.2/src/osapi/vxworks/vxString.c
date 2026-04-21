/*
 * FILE: vxString.c - VxWorks string functions
 *
 * Copyright 2013-2021 Real-Time Innovations, Inc.
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
 * \brief VxWorks implementation of OSAPI string routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_VXWORKS
#include "osapi/osapi_types.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS

void
OSAPI_Memory_copy(void *dest,const void *src, RTI_SIZE_T size)
{
    (void)bcopy((void*)src,dest,size);
}

void
OSAPI_Memory_zero(void *mem, RTI_SIZE_T size)
{
    (void)bfill((char *)mem, size, 0);
}

RTI_INT32
OSAPI_Memory_compare(const void *left,const void *right, RTI_SIZE_T size)
{
    return bcmp((char *)left, (char *)right, size);
}

void
OSAPI_Memory_move(void *dest,const void *src, RTI_SIZE_T size)
{
    bcopy((void*)src,dest,size);
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

#endif
