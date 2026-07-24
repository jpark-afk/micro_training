/*
 * FILE: String.c - Implementation of String API
 *
 * Copyright 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "osapi/osapi_config.h"
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#include "osapi/osapi_string.h"

MUST_CHECK_RETURN RTI_BOOL
OSAPI_String_parse_unsigned_long(const char *str, RTI_UINT32 *result)
{
    unsigned long long int temp_result;

    if (!OSAPI_String_parse_unsigned_long_long(str, &temp_result))
    {
        return RTI_FALSE;
    }

    /* Ensure this value fits in an unsigned long */
    if (temp_result > UINT_MAX)
    {
        return RTI_FALSE;
    }

    *result = (unsigned int)temp_result;

    return RTI_TRUE;
}
