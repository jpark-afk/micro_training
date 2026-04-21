/*
 * FILE: stubStdio.c - Stubbed Standard In/Out functions
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"

void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length)
{
    IGNORE_RETVAL(buffer);
    IGNORE_RETVAL(length);
}
