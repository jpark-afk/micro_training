/*
 * FILE: winProcess.c - Win process functionality
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
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI process routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_WINDOWS

#include <Windows.h>
#include "osapi/osapi_process.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_WINDOWS

/* ----------------------------------------------------------------- */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    return GetCurrentProcessId();
}

#endif
