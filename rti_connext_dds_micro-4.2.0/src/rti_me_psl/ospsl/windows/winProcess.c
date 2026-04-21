/*
 * FILE: winProcess.c - Win process functionality
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI process routines
 */
#include "rti_me_psl.h"

#include <Windows.h>
#include "osapi/osapi_process.h"

/*** SOURCE_BEGIN ***/

/* ----------------------------------------------------------------- */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    return (RTI_UINT64)GetCurrentProcessId();
}

RTI_BOOL
OSAPI_Process_is_alive(OSAPI_ProcessId pid)
{
    RTI_UINT32 pidIn = (RTI_UINT32)pid;
    if (pid == 0)
    {
        return RTI_FALSE;
    }

    /* GetProcessVersion might return 0 if version really is 0 */
    return(GetProcessVersion(pidIn)
            || GetLastError() != ERROR_INVALID_PARAMETER);
}
