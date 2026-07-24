/*
 * FILE: autosarProcess.c - AutoSAR process functionality
 *
 * Copyright 2019-2026 Real-Time Innovations, Inc.
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
 * 09mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of OSAPI process routines
 */

 #include "rti_me_psl.h"
// #include "osapi/osapi_process.h"
#include "Compiler_Cfg.h"

/*** SOURCE_BEGIN ***/

/* ----------------------------------------------------------------- */
FUNC(OSAPI_ProcessId, SOAD_CODE)
OSAPI_Process_getpid(void)
{
    OSAPI_ThreadId pid;

    pid = OSAPI_Thread_self();

    return (OSAPI_ProcessId)pid.handle.data;
}
