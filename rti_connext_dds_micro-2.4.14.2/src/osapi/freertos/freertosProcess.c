/*
 * FILE: freertosProcess.c - Process functionality
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
 * 21oct2014,eh MICRO-953: update FreeRTOS port
 * 06mar2012,tk Written
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI process routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "osapi/osapi_process.h"
#endif

/*** SOURCE_BEGIN ***/
#if OSAPI_INCLUDE_FREERTOS
RTI_UINT32
OSAPI_Process_getpid(void)
{
    return (RTI_UINT32) xTaskGetCurrentTaskHandle();
}

#endif
