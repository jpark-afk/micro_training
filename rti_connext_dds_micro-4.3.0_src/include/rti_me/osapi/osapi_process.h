/*
 * FILE: osapi_process.h - Process interface definition
 *
 * (c) Copyright, Real-Time Innovations, 2012-2024
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
 * 24mar2012,tk Written
 */
/*ce
 * \file 
 * \brief Process interface definition 
 */
#ifndef osapi_process_h
#define osapi_process_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPIProcessGroupDocs
 */
 
/*i \defgroup OSAPI_ProcessClass OSAPI Process
    \ingroup OSAPIModule

    \brief Abstract Process API.
 */

/*ce \dref_OSAPI_ProcessId
 */
typedef RTI_UINT64 OSAPI_ProcessId;

/*e \ingroup OSAPI_ProcessClass
 *  \brief Returns true if a process is alive
 *  \details When passed in PID 0, the function will always return RTI_TRUE
 *  \return RTI_BOOL
*/
OSPSLDllExport RTI_BOOL
OSAPI_Process_is_alive(OSAPI_ProcessId pid);

/*e
 * \file
 * \brief process related utility
 */

/*e \ingroup OSAPI_ProcessClass
 *   \brief Return the process ID, which is unique for the application.
 *
 *  \details
 *   Return the process ID.
 *
 *  \return Process id
*/
OSPSLDllExport OSAPI_ProcessId
OSAPI_Process_getpid(void);

/*ci
 * \brief Convert a process ID to a string in decimal format
 * *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    pid         Process ID to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
OSAPIDllExport RTI_SIZE_T
OSAPI_Process_pid_as_string(char *buffer,RTI_SIZE_T max_length,OSAPI_ProcessId pid);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_process_h */
