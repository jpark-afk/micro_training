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
 * 06sep2022,tk MICRO-4151/PR.30884
 * - OSAPI_Semaphore_new
 *   - Corrected description to state that a binary semaphore
 *     is created and that the initial value is 0.
 * - OSAPI_Semaphore_take
 *   - Renamed parameter from timeout to timeout_ms
 *   - Corrected description of fail_reason parameter.
 * 08sep2020,tk MICRO-2506/PR#28039 Corrected internal/external doxygen tags
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

/*e
 * Infinite timeout
 */
#define OSAPI_SEMAPHORE_TIMEOUT_INFINITE -1

/*e
 * Semaphore ok
 */
#define OSAPI_SEMAPHORE_RESULT_OK          0

/*e
 * Semaphore timed out
 */
#define OSAPI_SEMAPHORE_RESULT_TIMEOUT     1

/*e
 * Semaphore error
 */
#define OSAPI_SEMAPHORE_RESULT_ERROR       2

struct OSAPI_Semaphore;

/*e \ingroup OSAPI_SemaphoreClass
 *
 * \brief Abstract semaphore object.
 */
typedef struct OSAPI_Semaphore OSAPI_Semaphore_T;

/*e \ingroup OSAPI_SemaphoreClass
 *
 * \brief Create a semaphore.
 *
 * \details
 * The created semaphore is a binary semaphore with an initial semaphore value
 * set to 0 and a maximum value of 1. Thus, a call to OSAPI_Semaphore_take will
 * block until the semaphore is signaled with a call to OSAPI_Semaphore_give.
 *
 * \return Pointer to a semaphore with a value of 0 on success, NULL on failure.
 *
 * \ifnot C_LANGUAGE_CERT
 * \sa \ref OSAPI_Semaphore_delete
 * \endif
 *
 */
MUST_CHECK_RETURN OSAPIDllExport OSAPI_Semaphore_T*
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
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *self);
#endif /* !RTI_CERT */

/*e \ingroup OSAPI_SemaphoreClass
 *
 *  \brief Take a semaphore.
 *
 *  \param [in] self Semaphore previously created with \ref OSAPI_Semaphore_new.
 *
 *  \param [in] timeout_ms Timeout in ms. The `take` call will wait at most
 *                         `timeout` ms before returning. If \ref
 *                         OSAPI_SEMAPHORE_TIMEOUT_INFINITE is specified, the
 *                         call will not return until the semaphore has a value
 *                         of 1 or higher.
 *
 *  \param [out] fail_reason Additional information when the call returns.
 *                           If RTI_TRUE is returned, but timed out, fail_reason
 *                           is set to OSAPI_SEMAPHORE_RESULT_TIMEOUT, otherwise
 *                           fail_reason is set to OSAPI_SEMAPHORE_RESULT_OK.
 *                           If RTI_FALSE is returned, fail_reason is set to
 *                           OSAPI_SEMAPHORE_RESULT_ERROR.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Semaphore_give
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Semaphore_take(OSAPI_Semaphore_T *self,RTI_INT32 timeout_ms,
                     RTI_INT32 *fail_reason);

/*e \ingroup OSAPI_SemaphoreClass
 *
 *  \brief Give a semaphore.
 *
 *  \param [in] self Semaphore previously created with \ref OSAPI_Semaphore_new.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Semaphore_take
 *
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *self);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_semaphore_h */
