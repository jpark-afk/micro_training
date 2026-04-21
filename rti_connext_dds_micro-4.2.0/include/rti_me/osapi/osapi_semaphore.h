/*
 * FILE: osapi_semaphore.h - Definition of semaphore interface
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc.
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
 * 29jun2015,tk MICRO-1333/PR#15091 Corrected comment for _delete()
 * 12mar2012,tk Written
 */
/*e \file
 *  \brief Semaphore interface definition
 */
#ifndef osapi_semaphore_h
#define osapi_semaphore_h

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

/*e \defgroup OSAPI_SemaphoreClass OSAPI Semaphore
    \ingroup OSAPIModule

    \brief Abstract Semaphore API.
 */

/*e \dref_OSAPI_SEMAPHORE_TIMEOUT_INFINITE
 */
#define OSAPI_SEMAPHORE_TIMEOUT_INFINITE -1

/*e \dref_OSAPI_SEMAPHORE_RESULT_OK
 */
#define OSAPI_SEMAPHORE_RESULT_OK          0

/*e \dref_OSAPI_SEMAPHORE_RESULT_TIMEOUT
 */
#define OSAPI_SEMAPHORE_RESULT_TIMEOUT     1

/*e \dref_OSAPI_SEMAPHORE_RESULT_ERROR
 */
#define OSAPI_SEMAPHORE_RESULT_ERROR       2

/*i \brief Infinite wait time seconds
 */
#define OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC -1

/*i \brief Infinite wait time nano seconds
 */
#define OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC 0xffffffffL

/*i \brief Maximum number of seconds in an INT_MAX as milliseconds
 */
#define OSAPI_SEMAPHORE_MAX_SEC_AS_MS (INT_MAX / OSAPI_TIME_MSEC_PER_SEC)

struct OSAPI_Semaphore;

/*e \dref_OSAPI_Semaphore_T
 */
typedef struct OSAPI_Semaphore OSAPI_Semaphore_T;

/*e \dref_OSAPI_Semaphore_new
 */
MUST_CHECK_RETURN OSPSLDllExport OSAPI_Semaphore_T*
OSAPI_Semaphore_new(void);

#ifndef RTI_CERT
/*e \ingroup OSAPI_SemaphoreClass
 *
 *  \brief Delete a semaphore.
 *
 *  \param [in] self Semaphore created with \ref OSAPI_Semaphore_new.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Semaphore_new
 */
SHOULD_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *self);
#endif /* !RTI_CERT */

/*e \dref_OSAPI_Semaphore_take
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Semaphore_take(OSAPI_Semaphore_T *self,RTI_INT32 timeout_ms,
                    RTI_INT32 *fail_reason);

/*e \dref_OSAPI_Semaphore_give
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *self);

/*ci \brief Extended semaphore wait function
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Semaphore_take_sec_nanosec(OSAPI_Semaphore_T *self,
                                 RTI_INT32 sec,
                                 RTI_UINT32 nanosec,
                                 RTI_INT32 *reason);
#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* osapi_semaphore_h */
