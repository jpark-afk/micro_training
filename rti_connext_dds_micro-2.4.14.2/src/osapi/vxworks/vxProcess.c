/*
 * FILE: vxProcess.c - VxWorks process functionality
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
 * \brief VxWorks implementation of OSAPI process routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_VXWORKS

#include <vxWorks.h>
#if defined(RTI_RTP)
#include <rtpLib.h>
#include <unistd.h>
#else
#include <taskLib.h>
#endif

#include "osapi/osapi_process.h"
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS
/* ----------------------------------------------------------------- */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
#if defined(RTI_RTP)
    return getpid();
#else
    return taskIdSelf();
#endif
}
#endif
