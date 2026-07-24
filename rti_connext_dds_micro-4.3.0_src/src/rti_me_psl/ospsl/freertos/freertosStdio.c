/*
 * FILE: freertosStdio.c - Stdio functionality
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
 * \brief FreeRTOS implementation of Stdio process routines
 */

#include "rti_me_psl.h"

/*ci
 * \brief Write a log message
 *
 * \details
 * This function writes a log message to the appropriate output.
 * This is dependant on the OS but on the target platform.
 * 
 * \param[in] buffer Pointer to the buffer containing the log message.
 * \param[in] length Length of the log message.
 */
void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length)
{
    IGNORE_RETVAL(buffer);
    IGNORE_RETVAL(length);
}