/*
 * FILE: libcStdio.c - Standard In/Out functions
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"
#include <stdlib.h>

void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length)
{
#ifdef RTI_WIN32
    int n;
    n = _write(1,buffer,(size_t)length);
#else
    ssize_t n;
    n = write(1,buffer,(size_t)length);
#endif
    IGNORE_RETVAL(n);
}
