/*
 * FILE: threadXProcess.c - ThreadX process functionality
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
 * 13dec2016,francisco  File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI process routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

#include "osapi/osapi_process.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

/* ----------------------------------------------------------------- */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    return (OSAPI_ProcessId)tx_thread_identify();
}

#endif
