/*
 * FILE: osapi_mutex.h - Definition of mutex interface
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 12mar2012,tk Written
 */
/*ce
 * \file 
 * \brief Definition of mutex interface
 */
#ifndef osapi_mutex_h
#define osapi_mutex_h

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

/*e \dref_OSAPIMutexGroupDocs
 */
 
/*i \defgroup OSAPI_MutexClass OSAPI Mutex
    \ingroup OSAPIModule

    \brief Mutex API
 */

/*e \ingroup OSAPI_MutexClass
 *
 *  Abstract Mutex object
 */
struct OSAPI_Mutex;

#if OSAPI_ENABLE_MUTEX_TRACE
#define OSAPI_MUTEX_TRACE_ENABLED    (1)
#define OSAPI_MUTEX_MAX_STACK_DEPTH (8U)
#else
#define OSAPI_MUTEX_TRACE_ENABLED    (0)
#endif

/*e \ingroup OSAPI_MutexClass
 *  Abstract Mutex type
 */
typedef struct OSAPI_Mutex OSAPI_Mutex_T;

#ifndef RTI_CERT
/*e \ingroup OSAPI_MutexClass
 *
 * \brief Delete a mutex.
 *
 * \param [in] mutex Delete a mutex created with \ref OSAPI_Mutex_new.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Mutex_delete(OSAPI_Mutex_T *mutex);
#endif /* !RTI_CERT */

/*e \ingroup OSAPI_MutexClass
 *
 * \brief Create a mutex.
 *
 * \return Pointer to a mutex in a not taken condition. NULL on failure.
 *
 * \sa \ref OSAPI_Mutex_delete
 */
OSPSLDllExport OSAPI_Mutex_T*
OSAPI_Mutex_new(void);

/*e \ingroup OSAPI_MutexClass
 *
 * \brief Take a mutex.
 *
 * \details
 *
 * A mutex can only be taken if it is not currently already taken 
 * by another thread; it CAN be taken if is it already taken by 
 * the same thread. In order to release a mutex, it must be
 * given as many times a it has been taken. Note that `take` will 
 * block indefinitely.
 *
 * Note: the mutex is not required to support priority inversion;
 *       there is no protection against deadlocks or starvation.
 *
 * \param [in] self Take a mutex previously created with \ref OSAPI_Mutex_new.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref OSAPI_Mutex_give, \ref OSAPI_Mutex_new
 *
 */
#if OSAPI_MUTEX_TRACE_ENABLED
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Mutex_take_(OSAPI_Mutex_T *self,char *file,int lineno);
#define OSAPI_Mutex_take(s_) OSAPI_Mutex_take_(s_,__FILE__,__LINE__)
#else
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Mutex_take(OSAPI_Mutex_T *self);
#endif

/*e \ingroup OSAPI_MutexClass
 *
 * \brief Give a mutex.
 *
 * \details
 *
 * A mutex can only be given if it is owned by the calling thread. Furthermore,
 * it must be given as many times as it has been taken.
 *
 * \param [in] self Take a mutex previously created with \ref OSAPI_Mutex_new.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref OSAPI_Mutex_take, \ref OSAPI_Mutex_new
 *
 */
#if OSAPI_MUTEX_TRACE_ENABLED
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Mutex_give_(OSAPI_Mutex_T *self,char *file,int lineno);

#define OSAPI_Mutex_give(s_) OSAPI_Mutex_give_(s_,__FILE__,__LINE__)
#else
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Mutex_give(OSAPI_Mutex_T *self);
#endif

MUST_CHECK_RETURN OSPSLDllExport RTI_UINT32
OSAPI_Mutex_get_depth(OSAPI_Mutex_T *self);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_mutex_h */
