/*
 * FILE: autosarProcess.c - AutoSAR process functionality
 *
 * Copyright 2019-2021 Real-Time Innovations, Inc.
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
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_AUTOSAR

#include "osapi/osapi_process.h"
#include "osapi/osapi_thread.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_AUTOSAR

/* ----------------------------------------------------------------- */
FUNC(OSAPI_ProcessId, SOAD_CODE)
OSAPI_Process_getpid(void)
{
    return (OSAPI_ProcessId)OSAPI_Thread_self();
}

#endif
