/*
 * FILE: posixProcess.c - POSIX process functionality
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
 * \brief POSIX implementation of OSAPI process routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_POSIX

#include <sys/types.h>
#include <unistd.h>

#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_POSIX
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
    (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)
    return getpid();
#else
    return 0;
#endif
}

#endif
