/*
 * FILE: osapi_timer.h - Definition of Timer interface
 *
 * Copyright 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_TimeoutHandle_get_user_data
 * 25feb2021,tk MICRO-2914/PR#28853
 *    - Use OSAPI_THREAD_PRIORITY_DEFAULT, not OSAPI_THREAD_PRIORITY_NORMAL
 * 07jan2021,tk
 *   - MICRO-2794/PR#28559
 *     - Added OSAPI_TIMER_MIN_SLOTS
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 08sep2020,tk  MICRO-2506/PR#28039 Corrected internal/external doxygen tags
 * 22mar2012,tk Written
 */
/*ci
 * \file
 * \brief Timer interface definition
 *
 * \details
 * The Timer API provides a platform independent set of function to start
 * timers and be notified when the timer expires. The concrete implementation
 * of the platform dependent timer API are located in the platform specific
 * files, such as vxTimer.c
 */
/*i
 * \defgroup OSAPI_TimerClass OSAPI Timer
 * \ingroup OSAPIModule
 *
 *  \details
 */
#ifndef osapi_timer_h
#define osapi_timer_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPITimerGroupDocs
 */

/*i \defgroup OSAPI_TimerClass OSAPI Timer
 *  \ingroup OSAPIModule
*/

/*i \file
  \brief Timer API
*/

/*i \ingroup OSAPI_TimerClass
 *  \brief This is a new timer entry
 */
#define OSAPI_TIMER_NEW_ENTRY (0x2)

/*i \ingroup OSAPI_TimerClass
 *  \brief Create a periodic timer
 */
#define OSAPI_TIMER_PERIODIC  (0x1)

/*i \ingroup OSAPI_TimerClass
 *  \brief Create a one-shot timer
 */
#define OSAPI_TIMER_ONE_SHOT  (0x0)

/*i \ingroup OSAPI_TimerClass
 */
#define OSAPI_TIMER_MIN_SLOTS  (32L)

/*i \ingroup OSAPI_TimerClass
 */
#define OSAPI_TIMER_MAX_SLOTS  (256L)

/*i \ingroup OSAPI_TimerClass
 *  \brief Action taken by timer module when a timer callback returns.
 */
typedef enum
{
    /*ci  \brief Automatically reschedule the timer
     */
    OSAPI_TIMEOUT_OP_AUTOMATIC = 1,

    /*ci \brief The timer is manually rescheduled by the application
     */
    OSAPI_TIMEOUT_OP_MANUAL
} OSAPI_TimeoutOp_t;

/*i \ingroup OSAPI_TimerClass
 *  \brief User-data passed to timer-handler.
 */
struct OSAPI_TimeoutUserData
{
    /*ci \brief Two user defined counters
     */
    RTI_UINT32 count[2];

    /*ci \brief Two user defined pointers
     */
    void *field[2];
};

#define OSAPI_TimeoutUserData_INITIALIZER \
{ \
    {0,0},\
    {NULL,NULL}\
}

struct OSAPI_TimerEntry;

/*ci \brief a Handle to a timer object
 */
struct OSAPI_TimeoutHandle
{
    /* In case an entry is re-used */
    struct OSAPI_TimerEntry *_entry;

    /* In case an entry is re-used */
    RTI_INT32 epoch;
};

typedef struct OSAPI_TimeoutHandle OSAPI_TimeoutHandle_T;

#define OSAPI_TimeoutHandle_INITIALIZER \
{\
    NULL,\
    -1\
}

/*e \ingroup OSAPI_TimerClass
 *  \brief Timer properies
 */
struct OSAPI_TimerProperty
{
    /*ci
     * \brief The maximum number of timeouts
     */
    RTI_INT32 max_entries;

    /*ci
     * \brief The maximum number slots in one round
     */
    RTI_INT32 max_slots;

    /*e
     * \brief The thread settings for the timer thread, if a thread is created
     */
    struct OSAPI_ThreadProperty thread;
};

/*ci
 * \brief Initializer for the \ref OSAPI_TimerProperty
 */
#define OSAPI_TimerProperty_INITIALIZER \
{\
    128,\
    32,\
    { \
        OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE, \
        OSAPI_THREAD_PRIORITY_DEFAULT, \
        OSAPI_THREAD_DEFAULT_OPTIONS \
    } \
}

struct OSAPI_Timer;
typedef struct OSAPI_Timer *OSAPI_Timer_T;

FUNCTION_MUST_TYPEDEF(
OSAPI_TimeoutOp_t
(*OSAPI_TimeoutFunction_T)(struct OSAPI_TimeoutUserData *user_data)
)

typedef void
(*OSAPI_TimerTickHandlerFunction)(OSAPI_Timer_T timer);

/*i \ingroup OSAPI_TimerClass
 *  \brief Create a Timer.
 *
 *  \details Create a new Timer. A Timer can manage multiple timeouts.
 *
 *  Example:
 *  \code
 *  OSAPI_Timer_t my_timer;
 *  struct OSAPI_TimerProperty timer_property = OSAPI_TimerProperty_INITIALIZER;
 *  timer = OSAPI_Timer_new(&timer_property);
 *  if (timer == NULL) {
 *      return error;
 *  }
 *  \endcode
 *
 *  The created Timer should be deleted with \ref OSAPI_Timer_delete.
 *
 *  @param[in] property Timer property.
 *  @param[in] mutex Shared mutex.
 *
 *  @return New timer on success, NULL on failure
 */
MUST_CHECK_RETURN OSAPIDllExport OSAPI_Timer_T
OSAPI_Timer_new(struct OSAPI_TimerProperty *property,struct OSAPI_Mutex *mutex);

#ifndef RTI_CERT
/*i \ingroup OSAPI_TimerClass
 *  \brief Delete a Timer.
 *
 *  \details Delete a previously created Timer. All timeouts are cancelled.
 *
 *  Example:
 *  \code
 *  OSAPI_Timer_t my_timer;
 *
 *  ......
 *
 *  OSAPI_Timer_delete(my_timer);
 *  if (timer == NULL) {
 *      return error;
 *  }
 *  \endcode
 *
 *  The created Timer should be deleted with \ref OSAPI_Timer_delete.
 *
 *  @param [in] timer Timer.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 *
 *  @mtsafety SAFE
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Timer_delete(OSAPI_Timer_T timer);
#endif /* !RTI_CERT */

/*i \ingroup OSAPI_TimerClass
 *   \brief Schedule a timeout.
 *
 *  \details This function schedules a timeout with the specified period. The
 *   timeout can either be rescheduled automatically or a new timeout must
 *  be created.
 *
 *  Example:
 *  \code
 *  OSAPI_Timer_t my_timer;
 *  OSAPITimeoutHandle_t my_handle = OSAPITimeoutHandle_t_INITIALIZER;
 *  RTI_BOOL result;
 *  struct OSAPI_TimerEntryUserData user_data;
 *
 *  result = OSAPI_Timer_create_timeout(my_timer,
 *                                  &my_handle,
 *                                  1000,0,
 *                                  OSAPI_TIMER_PERIODIC,
 *                                  &user_data);
 *
 *  if (!result) {
 *      report error;
 *  }
 *
 *  ......
 *
 *  \endcode
 *
 *  A timeout can cancelled  with \ref OSAPI_Timer_delete_timeout or the timeout
 *  can be modified with \ref OSAPI_Timer_update_timeout.
 *
 *  @param[in]  timer Timer object. Cannot be NULL.
 *  @param[in]  handle Reference to the timeout. Cannot be NULL.
 *  @param[in]  timeout_sec  The number of seconds before timeout
 *  @param[in]  timeout_nsec Additional number of nanoseconds before timeout
 *  @param[in]  flags Flags OSAPI_TIMER_PERIODIC or OSAPI_TIMER_ONE_SHOT
 *  @param[in]  timeout_handler Function to call at timeout. Cannot be NULL.
 *  @param[in]  user_data User data associated with the timeout. Can be NULL.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 *
 *  @mtsafety SAFE
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Timer_create_timeout(OSAPI_Timer_T timer,
                          OSAPI_TimeoutHandle_T *handle,
                          RTI_INT32 timeout_sec,
                          RTI_INT32 timeout_nsec,
                          RTI_INT32 flags,
                          OSAPI_TimeoutFunction_T timeout_handler,
                          struct OSAPI_TimeoutUserData *user_data);

/*i \ingroup OSAPI_TimerClass
 *  \brief Reschedule a timeout.
 *
 *  \details This function reschedules a previously scheduled timeout with
 *  the specified period.
 *
 *  Example:
 *  \code
 *  OSAPI_Timer_t my_timer;
 *  OSAPITimeoutHandle_t my_handle = OSAPITimeoutHandle_t_INITIALIZER;
 *  RTI_BOOL result;
 *  struct OSAPI_TimerEntryUserData user_data;
 *
 *  result = OSAPI_Timer_update_timeout(my_timer,
 *                                      &my_handle,
 *                                      2000,0);
 *
 *  if (!result) {
 *      report error;
 *  }
 *
 *  ......
 *
 *  \endcode
 *
 *  @param [in]  timer Timer object. Cannot be NULL.
 *  @param [out] out_handle Reference to the timeout. Cannot be NULL.
 *  @param[in]   timeout_sec  The number of seconds before timeout
 *  @param[in]   timeout_nsec Additional number of nanoseconds before timeout
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Timer_update_timeout(OSAPI_Timer_T timer,
                           OSAPI_TimeoutHandle_T *out_handle,
                           RTI_INT32 timeout_sec,
                           RTI_INT32 timeout_nsec);

/*i \ingroup OSAPI_TimerClass
 *  \brief Stop a previously scheduled timeout
 *
 *  \details This function stops a previously scheduled timeout.
 *
 *  Example:
 *  \code
 *  OSAPI_Timer_t my_timer;
 *  OSAPITimeoutHandle_t my_handle = OSAPITimeoutHandle_t_INITIALIZER;
 *  RTI_BOOL result;
 *
 *  ....
 *
 *  result = OSAPI_Timer_stop_timer(my_timer,&my_handle);
 *
 *  if (!result) {
 *      report error;
 *  }
 *
 *  ......
 *
 *  \endcode
 *
 *  @param [in] timer Timer object. Cannot be NULL.
 *  @param [in] handle Reference to the timeout. Cannot be NULL.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 *
 *  @mtsafety SAFE
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Timer_delete_timeout(OSAPI_Timer_T timer, OSAPI_TimeoutHandle_T *handle);

/*i \ingroup OSAPI_TimerClass
 * \brief Check if a handle references a valid timer entry.
 *
 * This function can be helpful for code which must ensure a timer has
 * been finalized.
 *
 * Since OSAPI_Timer_delete_timeout() will fail if the specified handle does
 * not match an existing timeout entry, this function can be used to detect
 * a handle is invalid (and thus if has been likely already finalized).
 *
 * This function does not perform an exhaustive search of existing timeouts
 * to find an exact match. It only evaluates whether the handle is explicitly
 * marked as invalid or not (which occurs during finalization).
 *
 * There is no guarantee that a timeout handle actually exists for this
 * specific handle.
 *
 * \return RTI_TRUE if the specified is not explicitly marked as invalid,
 *  RTI_FALSE otherwise.
 *
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Timer_handle_is_valid(OSAPI_TimeoutHandle_T *h);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_timer_h */
