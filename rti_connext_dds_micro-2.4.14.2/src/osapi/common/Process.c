/*
 * FILE: Process.c - Platform independent Process functionality
 *
 * (c) Copyright 2016 Real-Time Innovations
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
 * 15sep2016,eh  Written. MICRO-1559: 64-bit compatibility
 */
/*ce
 * \file
 * \brief Implementation of Process API.
 *
 * \details
 * This file implements the platform independent process APIs. Platform
 * dependent process functionality is find in the platform specific code.
 */
#include "osapi/osapi_process.h"
#include "osapi/osapi_types.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Convert a process ID to a string in decimal format
 * *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    pid_in      Process ID to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
RTI_SIZE_T
OSAPI_Process_pid_as_string(char *buffer,
                            RTI_SIZE_T max_length, OSAPI_ProcessId pid)
{
    const char digit[] = "0123456789abcdef";
    RTI_SIZE_T rlen = 0;
    RTI_INT32 shift_num;
    RTI_UINT64 pid_calc = (RTI_UINT64)pid; 

    if (max_length > 0)
    {
        shift_num = (sizeof(OSAPI_ProcessId) * 8) - 4;
        buffer[max_length - 1] = 0;

        while (shift_num >= 0)
        {
            if (rlen >= max_length - 1)
            {
                return max_length;
            }

            buffer[rlen] = digit[(pid_calc >> shift_num) & 0xf];
            shift_num -= 4;

            /* No leading zeros.  Allow pid == 0 */
            if ((buffer[rlen] == '0') && (rlen == 0) && (shift_num >= 0))
            {
                continue;
            }
            ++rlen;
        }
        buffer[rlen] = 0;
    }
    return rlen;
}

