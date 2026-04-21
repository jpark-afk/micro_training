/*
 * FILE: freertosProcess.c - Process functionality
 *
 * (c) Copyright, Real-Time Innovations 2024-2024
 *  
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI process routines
 */

#include "FreeRTOS.h"
#include "task.h"

#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Get the current process ID
 *
 * \param[in] None.
 *
 * \return The process ID of the current task.
 */
OSAPI_ProcessId
OSAPI_Process_getpid(void)
{
    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();
    return (OSAPI_ProcessId)(uintptr_t)taskHandle;
}