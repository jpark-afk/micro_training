/*
 * FILE: osapi_time.h - Definition of Time API
 *
 * Copyright 2005-2025 Real-Time Innovations, Inc.
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
 * 29may2015,tk MICRO-1329/PR#15063 Moved implementation comments from
 *                                  to Time.c. API documentation is kept here.
 * 16mar2015,tk  MICRO-1084 Changed comment for to/from_ntp_time to state
 *                          resolution is at least 1ns.
 * 20mar2013,tk  Updated
 * 06dec2005,rh  Created
 */
/*e \file
 * \brief Time API definition
 *
 * \details
 * This file implements functions to convert between time formats.
 */
#ifndef osapi_time_h
#define osapi_time_h

#include "osapi/osapi_dll.h"
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \defgroup OSAPI_TimeClass OSAPI Time
 *  \ingroup OSAPIModule
 */

/*i \ingroup OSAPITimeClass
 *
 *   \brief Time API.
 */
/* ================================================================= */
/*                           Definition: Time                        */
/* ----------------------------------------------------------------- */

/*e \dref_OSAPI_SystemTime
 */
typedef struct OSAPI_SystemTime
{
    /*e \brief Seconds.
     */
    RTI_INT64 sec;
    
    /*e \brief Nanoseconds.
     */
    RTI_UINT32 nanosec;
} OSAPI_SystemTime;

/*i \ingroup OSAPITimeClass
 * The maximum number of system time seconds.
 */
#define OSAPI_SYSTEM_TIME_SEC_MAX      0x7FFFFFFFFFFFFFFF

/*i \ingroup OSAPITimeClass
 * The maximum number of system time nanoseconds.
 */
#define OSAPI_SYSTEM_TIME_NANO_MAX     0xFFFFFFFF

/*i \ingroup OSAPITimeClass
 * System time invalid seconds.
 */
#define OSAPI_SYSTEM_TIME_SEC_INVALID  -1

/*i \ingroup OSAPITimeClass
 * System time invalid nanoseconds.
 */
#define OSAPI_SYSTEM_TIME_NANO_INVALID  0xFFFFFFFF

/*i \ingroup OSAPITimeClass
 *
 * @brief answer = t1 - t2.
 *
 * @param answer The results
 * @param t1     Time minuend
 * @param t2     Time subtrahend
 */
OSAPIDllExport void
OSAPI_SystemTime_subtract(struct OSAPI_SystemTime *const answer,
                        const struct OSAPI_SystemTime *const t1,
                        const struct OSAPI_SystemTime *const t2);

/*i \ingroup OSAPITimeClass
 *
 * @brief Compare two times
 *
 * @param time1  Time to compare
 * @param time2  Time to compare
 *
 * @return 0 if t1 = t2
 *         a positive integer if t1 > t2
 *         a negative integer if t1 < t2
 */
OSAPIDllExport RTI_INT32
OSAPI_SystemTime_compare(const struct OSAPI_SystemTime *time1,
                         const struct OSAPI_SystemTime *time2);


/* \i
 *
 * @brief Convert from ntp Time to seconds and nanoseconds.
 *
 * @param s         Second of the time to covert.
 * @param nsec      Nanosecond fraction to convert.
 * @param from_sec  Seconds to convert from.
 * @param from_frac ntp nanoseconds to convert from.
 *
 *
 * The accuracy of this conversion is at least 1 nanosecond.
 */
OSAPIDllExport void
OSAPI_Time_from_ntp(RTI_UINT32 *const to_sec,
    RTI_UINT32 *const to_nsec,
    RTI_UINT32 from_sec,
    RTI_UINT32 from_frac);

/* \i
 *
 * @brief Convert from system time to ntp format;
 *
 * @param out_sec    Converted seconds.
 * @param out_frac   Converted nanoseconds.
 * @param time       Time to convert from.
 *
 * The accuracy of this conversion over the whole range of nanosecond values
 * is at least 1 nanosecond.
 */
OSAPIDllExport void
OSAPI_SystemTime_to_ntp(RTI_UINT32 *const out_sec,
                        RTI_UINT32 *const out_frac,
                        const OSAPI_SystemTime *const time);

/*i \ingroup OSAPITimeClass
 * @return RTI_TRUE if time is infinite, RTI_FALSE otherwise.
*/
OSAPIDllExport RTI_BOOL
OSAPI_SystemTime_is_infinite(const OSAPI_SystemTime *time);

/*i \ingroup OSAPITimeClass
 * @return RTI_TRUE if time is invalid, RTI_FALSE otherwise.
*/
OSAPIDllExport RTI_BOOL
OSAPI_SystemTime_is_invalid(const OSAPI_SystemTime *time);

/*i \ingroup OSAPITimeClass
 *
 * @brief Zero time.
 *
 * This global variable is for convenience. It allows you to see if a
 * RTITime variable is negative or positive by comparing against this.
 *
 * @see RtiTimeCompare
 */
#define OSAPI_TIME_ZERO {0,0}

/*i \ingroup OSAPITimeClass
 *
 * Represents the maximum time value that can be represented for system
 * time. For all practical purposes, it can be considered equivalent to
 * infinity.
 */
#define OSAPI_TIME_MAX {OSAPI_SYSTEM_TIME_SEC_MAX,OSAPI_SYSTEM_TIME_NANO_MAX}

/*i \ingroup OSAPITimeClass
 *  The number of nanoseconds per second. 1e9.
 *  @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_NSEC_PER_SEC  (1000000000)

/*i \ingroup OSAPITimeClass
 *  The number of microseconds per second. 1e6.
 * @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_USEC_PER_SEC  (1000000)

/*i \ingroup OSAPITimeClass
 *  The number of milliseconds per second. 1e3.
 *  @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_MSEC_PER_SEC  (1000)

/*i \ingroup OSAPITimeClass
 * The number of nanoseconds per microseconds. 1e3.
 * @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_NSEC_PER_USEC (1000)

/*i \ingroup OSAPITimeClass
 * The number of microseconds per milliseconds. 1e3.
 * @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_USEC_PER_MSEC (1000)

/*i \ingroup OSAPITimeClass
 * The number of seconds per second. 1.
 * @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_SEC_PER_SEC   (1)

/*i \ingroup OSAPITimeClass
 * The number of nano seconds per milli second. 1e6.
 * @see RtiTimePack RtiTimeUnpack
 */
#define OSAPI_TIME_NSEC_PER_MSEC   (1000000)

#ifdef __cplusplus
}                  /* extern "C" */
#endif

#endif /* osapi_time_h */
