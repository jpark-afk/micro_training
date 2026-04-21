/*
 * FILE: Time.c - Implementation of Time API
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_NtpTime_from_microsec
 *   OSAPI_NtpTime_to_microsec
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 29may2015,tk MICRO-1329/PR#15063 Moved implementation comments from
 *                                  osapi_time.h here
 * 25feb2015,tk MICRO-1084/PR#14056 Improved conversion between NTP and nanosec
 *                                  The implementation is taken from the Core
 *                                  libraries.
 * 24feb2015,tk Refactored from osapi_time.h
 */
/*ce
 * \file
 * \brief Platform independent time functionality
 * \ingroup OSAPIModule
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_time.h"

/*** SOURCE_BEGIN ***/

/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from seconds and milliseconds to struct OSAPI_NtpTime format.
 *
 * @param time (struct OSAPI_NtpTime) contains the answer.
 * @param s  (integer) Seconds to convert.
 * @param msec (millisecond) Fraction portion (less than 1000).
 *
 * \verbatim
 *  struct OSAPI_NtpTime time;
 *  long sec, msec;
 *  sec = 10;
 *  msec = 577;
 *
 *  OSAPI_NtpTime_from_millisec(time, sec, msec);
 * \endverbatim
 *
 * After the above call is made, the variable time will contain the equivalent
 * timestamp in struct OSAPI_NtpTime representation.
 *
 * This macro assumes that msec<1000. It is the caller's responsibility
 * to ensure this. This is done for performance reasons (the extra check slows
 * the execution by a factor of 5). If msec may be greater than 1000, you can invoke
 * the macro as follows:
 * \verbatim
 *  OSAPI_NtpTime_from_millisec(time, sec + msec/1000, msec%1000);
 * \endverbatim
 *
 * This macro only evaluates its arguments once, so it is safe to invoke it
 * with an argument that has side effects; that is, if you write:
 * \verbatim
 *  RTI_UINT32 count = 10;
 *  OSAPI_NtpTime_from_millisec(time, sec, count++)
 * \endverbatim
 * After the macro count is guaranteed to be 11.
 *
 * The accuracy of this conversion over the whole range of millisecond values
 * is 0.013 milliseconds.
 *
 * This is a fairly efficient macro. On a 400MHz Pentium-II Linux box
 * the execution time is about 0.06 usec.
 *
 * \verbatim
 *    2^22*time.frac = ms + ms/2^6 + ms/2^7 + ms/2^11 + ms/2^14 + 1.3e-5
 *                  = ms + ms*393/2^14;
 *        time.frac = (ms<<22) + (ms<<16) + (ms<<15) + (ms<<11) + (ms<<8)
 *                  = (ms<<22) + ((ms*393)<<8)
 * \endverbatim
 *
 * @see struct OSAPI_NtpTime OSAPI_NtpTime_to_millisec
 */
void
OSAPI_NtpTime_from_millisec(struct OSAPI_NtpTime *const time,
                            RTI_INT32 s, RTI_UINT32 msec)
{
    time->sec  = s;
    time->frac = (msec << 22) + ((msec*393) << 8);
}

/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from struct OSAPI_NtpTime to seconds and milliseconds.
 *
 * @param s (integer) Holds the seconds answer.
 * @param msec (integer) Holds the millisecond answer.
 * @param time (struct OSAPI_NtpTime) The time to convert to seconds and milliseconds.
 *
 * This macro does not check for overflow, so for a near-infinite time value,
 * the conversion result may end up being negative. It is the responsibility
 * of the user to avoid passing these large time values.
 *
 * Use as indicated by the following code:
 * \verbatim}
 *   struct OSAPI_NtpTime time;
 *  long sec, msec;
 *
 *  time.sec  = 10;
 *  time.frac = 577;
 *
 *  OSAPI_NtpTime_to_millisec(sec, msec, time);
 * \endverbatim
 *
 * After the above call is made, the variables "sec" and "msec" will
 * contain the equivalent timestamp.
 *
 * The accuracy of this conversion over the whole range of struct OSAPI_NtpTime
 * values is 0.5 milliseconds, except for ntp values above
 * {0x7fffffff, 0x7fffffff}, accuracy is up to nearly 1 millisecond.
 *
 * This is a fairly efficient macro. On a 400MHz Pentium-II Linux box
 * the execution time is about 0.08 usec.
 *
 * \verbatim
 *    ms/2^22 = frac*1000/1024 = frac*(1 - 24/1024) = frac*(1 - 3/2^7) =
 *              frac*(1 - 1/2^6 - 1/2^7)
 * \endverbatim
 *
 * @see struct OSAPI_NtpTime OSAPI_NtpTime_from_millisec
 */
void
OSAPI_NtpTime_to_millisec(RTI_INT32 *const s, RTI_UINT32 *const msec,
                          const struct OSAPI_NtpTime *const time)
{
    *s = time->sec;
    *msec = (time->frac - (time->frac >> 6) -
             (time->frac >> 7) + (1 << 21)) >> 22;

    if ((*msec >= 1000) && (*s != 0x7FFFFFFF))
    {
        *msec -= 1000;
        (*s)++;
    }
}

#ifndef RTI_CERT
/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from seconds and microseconds to struct OSAPI_NtpTime format.
 *
 * @param time (struct OSAPI_NtpTime) to contain the answer.
 * @param s (integer) seconds of the time to covert.
 * @param usec (integer) microseconds fraction to convert from.
 *
 * This macro does not check for overflow, so for a near-infinite time value,
 * the conversion result may end up being negative. It is the caller's
 * responsibility to avoid passing these large time values.
 *
 * Use as indicated by the following code:
 * \verbatim
 *  struct OSAPI_NtpTime ntp;
 *  long sec, usec;
 *  sec  = 10;
 *  usec = 577;
 *
 *  OSAPI_NtpTime_from_microsec(ntp, sec, usec);
 * \endverbatim
 *
 * After the above call is made, the variable ntp will contain the equivalent
 * time stamp in struct OSAPI_NtpTime representation.
 *
 * This macro assumes that msec<1000000. It is the caller's responsibility
 * to ensure this. This is done for performance reasons, the extra check
 * slows the execution a factor of 5! If msec may be greater than 1000000,
 * you can invoke the macro as follows:
 * \verbatim
 *  OSAPI_NtpTime_from_microsec(ntp, sec + usec/1000000, usec%1000000);
 * \endverbatim
 *
 * This macro only evaluates its arguments once, so it is safe to invoke it
 * with an argument that has side effects; that is, if you write:
 * \verbatim
 *  RTI_UINT32 count = 10;
 *  OSAPI_NtpTime_from_microsec(ntp, sec, count++)
 * \endverbatim
 *
 * After the macro, count is guaranteed to be 11.
 *
 * The accuracy of this conversion over the whole range of microsecond values
 * is 0.0028 microseconds.
 *
 * This is a fairly efficient macro. On a 400MHz Pentium-II Linux box
 * the execution time is about 0.08 usec.
 *
 * \verbatim
 * 2^12 * ntp.frac = us + us/2^5  + us/2^6  + us/2^10 + us/2^11
 *                      + us/2^13 + us/2^14 + us/2^15 + us/2^16
 *                      + us/2^18 + us/2^19 + us/2^20 + us/2^21
 *                      + us/2^23 + 3e-9
 *                 = us + us*99/2^11 + us*15/2^16 + us*61/2^23
 *         ntp.frac = (us<<12) + (us<<7) + (us<<6) + (us<<2) + (us<<1)
 *                             + (us>>1) + (us>>2) + (us>>3) + (us>>4)
 *                             + (us>>6) + (us>>7) + (us>>8) + (us>>9) + (us>>11)
 *                  = (us<<12) + ((us*99)<<1)+ ((us*15)>>4)+ ((us*61)>>11)
 * \endverbatim
*/
void
OSAPI_NtpTime_from_microsec(struct OSAPI_NtpTime *const time,
                            RTI_INT32 s, RTI_UINT32 usec)
{
    time->sec  = s;
    time->frac = (usec << 12) + ((usec * 99) << 1) +
                 ((usec * 15 + ((usec * 61) >> 7)) >> 4);
}

/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from struct OSAPI_NtpTime to seconds and microseconds.
 *
 * @param s (integer) Holds the second portion.
 * @param usec (integer) holds the microsecond fraction.
 * @param time (struct OSAPI_NtpTime) to be converted.
 *
 * This macro does not check for overflow, so for a near-infinite time value,
 * the conversion result may end up being negative. It is the caller's
 * responsibility to avoid passing these large time values.
 *
 * Use as indicated by the following code:
 * \verbatim
 *  struct OSAPI_NtpTime ntp;
 *  long sec, usec;
 *
 *  ntp.sec  = 10;
 *  ntp.frac = 577;
 *
 *  OSAPI_NtpTime_to_microsec(sec, usec, ntp);
 * \endverbatim
 *
 * After the above call is made, the variables "sec" and "usec" will
 * contain the equivalent timestamp.
 *
 * The accuracy of this conversion over the whole range of struct OSAPI_NtpTime values
 * is 0.5 microseconds, except for ntp value above {0x7fffffff, 0x7fffffff},
 * accuracy is up to nearly 1 millisecond.
 *
 * This is a fairly efficient macro. On a 400MHz Pentium-II Linux box
 * the execution time is about 0.12 usec.
 *
 * \verbatim
 *   us = frac*1000000/2^20 = frac*( 1 - 48576/2^20 )
 *      = frac*(1 - 47/2^10 - 7/2^14)
 *      = frac - ((47*frac)>>10) - ((7*frac)>>14)
 *      = frac - (frac>>5)-(frac>>7)-(frac>>8)-(frac>>9)-(frac>>10)
 *           - (frac>>12)-(frac>>13)-(frac>>14)
 * \endverbatim
 */
void
OSAPI_NtpTime_to_microsec(RTI_INT32 *const s, RTI_UINT32 *const usec,
                          const struct OSAPI_NtpTime *const time)
{
    RTI_UINT32 temp = time->frac;

    *s = time->sec;
    *usec = (time->frac - (temp >> 5) - (temp >> 7) - (temp >> 8)-
            (temp >> 9) - (temp >> 10) - (temp >> 12) -
            (temp >> 13) - (temp >> 14) + (1 << 11)) >> 12;

    if ((*usec >= 1000000) && (*s != 0x7FFFFFFF))
    {
        *usec -= 1000000;
        (*s)++;
    }
}
#endif

/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from seconds and nanoseconds to struct OSAPI_NtpTime format.
 *
 * @param time (struct OSAPI_NtpTime) Holds the answer.
 * @param s (integer) Seconds to convert from.
 * @param nsec (integer) Nanosecond fraction to convert from.
 *
 * Use as indicated by the following code:
 * \verbatim
 *  struct OSAPI_NtpTime time;
 *  long sec, nsec;
 *  sec = 10;
 *  nsec = 577;
 *
 *  OSAPI_NtpTime_from_nanosec(time, sec, msec);
 * \endverbatim
 *
 * After the above call is made, the variable time will contain the equivalent
 * timestamp in struct OSAPI_NtpTime representation.
 *
 * This macro assumes that nsec<1000000000. It is the caller's responsibility
 * to ensure this. This is done for performance reasons (the extra check
 * slows the execution a factor of 5). If msec may be greater than 1000000, you
 * can invoke the macro as follows:
 * \verbatim}
 *   OSAPI_NtpTime_from_nanosec(time, sec + nsec/1000000000, nsec%1000000000);
 * \endverbatim
 *
 * This macro only evaluates its arguments once, so it is safe to invoke it
 * with an argument that has side effects; that is, if you write:
 * \verbatim
 *   RTI_UINT32 count = 10;
 * OSAPI_NtpTime_from_nanosec(time, sec, count++)
 * \endverbatim
 * After the macro, count is guaranteed to be 11.
 *
 * The accuracy of this conversion over the whole range of nanosecond values
 * is at least 1 nanosecond.
 */
void
OSAPI_NtpTime_from_nanosec(struct OSAPI_NtpTime *const time,
                           RTI_INT32 s, RTI_UINT32 nsec)
{
    time->sec  = s;
    time->frac = (RTI_UINT32)(((RTI_UINT64)(nsec) * 2305843009U) >> 29);
}

/* \ingroup OSAPITimeClass
 *
 * @brief Macro to convert from struct OSAPI_NtpTime to seconds and nanoseconds.
 *
 * @param s (integer) Second of the time to covert.
 * @param nsec (integer) Nanosecond fraction to convert.
 * @param time (struct OSAPI_NtpTime) to convert from.
 *
 * Use as indicated by the following code:
 * \verbatim
 *   struct OSAPI_NtpTime time;
 *  long sec, nsec;
 *  time.sec  = 10;
 *  time.frac = 577;
 *
 *  OSAPI_NtpTime_to_nanosec(sec, nsec, time);
 * \endverbatim
 * After the above call is made, the variables "sec" and "nsec" will
 * contain the equivalent timestamp.
 *
 * The accuracy of this conversion over the whole range of
 * struct OSAPI_NtpTime values is at least 1 nanosecond.
 */
void
OSAPI_NtpTime_to_nanosec(RTI_INT32 *const s, RTI_UINT32 *const nsec,
                         const struct OSAPI_NtpTime *const time)
{
    *s = time->sec;

    *nsec = (RTI_UINT32)((((RTI_UINT64)time->frac << 29)/2305843009U) + 1);

    if ((time->frac == 0) || (time->frac == 2305843009U))
    {
        (*nsec)--;
    }

     if ((*nsec >= 1000000000) && (*s != 0x7FFFFFFF))
     {
        (*nsec) -= 1000000000;
        (*s)++;
     }
}

/*  \ingroup OSAPITimeClass
 *
 * A NULL struct OSAPI_NtpTime pointer is considered infinity. This is consistent
 * with the concept of infinite time on UNIX systems.
 *
 * In addition, if the seconds field equals OSAPI_NTP_TIME_SEC_MAX, the
 * time value is also considered infinite.
 *
 * @param time Pointer to RTITime.
 *
 * @return RTI_TRUE if time is infinite, RTI_FALSE otherwise.
*/
RTI_BOOL
OSAPI_NtpTime_is_infinite(const struct OSAPI_NtpTime *const time)
{
    return ((time->sec == OSAPI_NTP_TIME_SEC_MAX) ? RTI_TRUE : RTI_FALSE);
}

/* documented in osapi_time.h */
void
OSAPI_NtpTime_subtract(struct OSAPI_NtpTime *const answer,
                       const struct OSAPI_NtpTime *const t1,
                       const struct OSAPI_NtpTime *const t2)
{
    if (OSAPI_NtpTime_is_infinite(t1))
    {
        *answer = *t1;
    }
    else if (OSAPI_NtpTime_is_infinite(t2))
    {
        answer->sec = 0;
        answer->frac = 0;
    }
    else
    {
        answer->sec  = t1->sec - t2->sec;
        answer->frac = t1->frac - t2->frac;

        if (answer->frac > t1->frac)
        {
            answer->sec--;
        }
    }
}

/* documented in osapi_time.h */
RTI_INT32
OSAPI_NtpTime_compare(const struct OSAPI_NtpTime *time1,
                       const struct OSAPI_NtpTime *time2)
{
    return ((((time1)->sec) > ((time2)->sec)) ? 1 :
     ((((time1)->sec) < ((time2)->sec)) ? -1 :
      ((((time1)->frac) > ((time2)->frac)) ? 1 :
       ((((time1)->frac) < ((time2)->frac)) ? -1 : 0))));
}
