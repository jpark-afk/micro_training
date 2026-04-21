/*
 * FILE: REDABuffer.c - Buffer implementation
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
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 25feb2015,tk Written, Refactored from reda_buffer.h
 */
/*ce
 * \file
 * \brief REDA Buffer implementation
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_buffer.h"
#endif

/*** SOURCE_BEGIN ***/

void
REDA_Buffer_set(struct REDA_Buffer *buffer,char *pointer,RTI_UINT32 length)
{
    buffer->length = length;
    buffer->pointer = pointer;
}
