/*
 * FILE: posixProcess.c - POSIX process functionality
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI process routines
 */
#include "rti_me_psl.h"

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

/*** SOURCE_BEGIN ***/

OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    OSAPI_ProcessId pid;
#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
    (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)
    pid = (RTI_UINT64)getpid();
    return pid;
#else
    pid = 0;
    return pid;
#endif
}

OSAPIDllExport RTI_BOOL
OSAPI_Process_is_alive(OSAPI_ProcessId pid)
{
    if (pid == 0)
    {
        return RTI_FALSE;
    }

    return (kill((pid_t)pid, 0) == 0) || (errno != ESRCH);
}
