/*
 * FILE: stubProcess.c - Stub process functionality
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI process routines
 */
#include "rti_me_psl.h"

#include "osapi/osapi_process.h"

/*** SOURCE_BEGIN ***/

/* ----------------------------------------------------------------- */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    return 0;
}

RTI_BOOL
OSAPI_Process_is_alive(OSAPI_ProcessId pid)
{
    UNUSED_ARG(pid);

    return RTI_FALSE;
}
