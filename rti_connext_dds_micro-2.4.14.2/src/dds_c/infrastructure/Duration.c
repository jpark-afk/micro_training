/*
 * FILE: Duration.c - Duration implementation
 *
 * (c) Copyright 2008-2020 Real-Time Innovations,
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
 *   DDS_Duration_from_ntp_time
 * 06apr2021,tk MICRO-3023/PR.29032
 * - Changed DDS_Duration_compare to return DDS_Long instead of int.
 * 15jul2015,tk MICRO-1426/PR#15358 Added Added DDS_Duration_delta_gt
 * 04mar2014,tk MICRO-260: Removed "always false" test in Duration_to_ntp_time()
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Duration implementation
 *
 * \details
 * This file implemets functions to manipulate the DDS_Duration_t type.
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

/* ------------------------------------------------------------------
 * Public Constants
 * ------------------------------------------------------------------ */

/*ci
 * \brief Infinite duration seconds representation
 */
const DDS_Long DDS_DURATION_INFINITE_SEC = DDS_DURATION_INFINITE_SEC_INITIALIZER;

/*ci
 * \brief Infinite duration nanoseconds representation
 */
const DDS_UnsignedLong DDS_DURATION_INFINITE_NSEC = DDS_DURATION_INFINITE_NSEC_INITIALIZER;

/*ci
 * \brief Infinite duration representation
 */
const struct DDS_Duration_t DDS_DURATION_INFINITE =
{
    DDS_DURATION_INFINITE_SEC_INITIALIZER,
    DDS_DURATION_INFINITE_NSEC_INITIALIZER
};

/* documented in dds_c_infrastructure.h */
const struct DDS_Duration_t DDS_DURATION_YEAR = { 31536000L, 0UL };

/* documented in dds_c_infrastructure.h */
const struct DDS_Duration_t DDS_DURATION_NANOSEC = { 0L, 1UL };

/*ci
 * \brief Zero duration seconds representation
 */
const DDS_Long DDS_DURATION_ZERO_SEC = 0;

/*ci
 * \brief Zero duration nanoseconds representation
 */
const DDS_UnsignedLong DDS_DURATION_ZERO_NSEC = 0;

/*ci
 * \brief Zero duration representation
 */
const struct DDS_Duration_t DDS_DURATION_ZERO = { 0, 0 };

/*** SOURCE_BEGIN ***/

/*i @ingroup DDSBasicTypesModule
 *  \brief Convert from Duration format to NTP time format
 *
 *  @pre Both arguments must be non-NULL.
 *
 *  \param[in]  self The duration to convert
 *  \param[out] dst  The duration in NTP time format
 *
 *  \sa DDS_Duration_from_ntp_time
 */
void
DDS_Duration_to_ntp_time(const struct DDS_Duration_t *self,
                         struct OSAPI_NtpTime *dst)
{
    DDS_Long origSec = 0;
    DDS_Long sec = 0;
    DDS_UnsignedLong nsec = 0;

    if (DDS_Duration_is_infinite(self))
    {
        dst->sec = OSAPI_NTP_TIME_SEC_MAX;
        dst->frac = OSAPI_NTP_TIME_FRAC_MAX;
    }
    else
    {
        /* OSAPI_NtpTime_from_nanosec assumes that a.) # seconds and
         * #nanoseconds are both signed and b.) # nanoseconds is less
         * than one billion. We need to make sure of that here.
         */
        origSec = self->sec;
        sec = self->sec + (DDS_Long) (self->nanosec / 1000000000U);
        if (sec < origSec)
        {
            /*overflow! */
            sec = OSAPI_NTP_TIME_SEC_MAX;
        }
        nsec = (self->nanosec % 1000000000U);
        OSAPI_NtpTime_from_nanosec(dst, sec, nsec);
    }
}

#ifndef RTI_CERT
/*i @ingroup DDSBasicTypesModule
 *  \brief Convert from NTP format to Duration time format
 *
 *  @pre Both arguments must be non-NULL.
 *
 *  \param[in]  self The duration with the result
 *  \param[out] src  The NTP time to conver
 *
 *  \sa DDS_Duration_to_ntp_time
 */
void
DDS_Duration_from_ntp_time(struct DDS_Duration_t *self,
                           const struct OSAPI_NtpTime *src)
{

    if (OSAPI_NtpTime_is_infinite(src))
    {
        self->sec = DDS_DURATION_INFINITE_SEC;
        self->nanosec = DDS_DURATION_INFINITE_NSEC;
    }
    else
    {
        OSAPI_NtpTime_to_nanosec(&self->sec, &self->nanosec, src);
    }
}
#endif


/*i @ingroup DDSBasicTypesModule
 *  \brief Compare two durations
 *
 *  @pre Both arguments must be non-NULL.
 *
 *  \param[in]  left  The left side of the comparison
 *  \param[out] right The right side of the comparison
 *
 *  \return If left > right, then return 1
 *          If left < right, then return -1
 *          If left == right, then return 0
 */
DDS_Long
DDS_Duration_compare(const struct DDS_Duration_t *left,
                     const struct DDS_Duration_t *right)
{
    if (DDS_Duration_is_infinite(left))
    {
        if (DDS_Duration_is_infinite(right))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    if (DDS_Duration_is_infinite(right))
    {
        return -1;
    }

    if (left->sec > right->sec)
    {
        return 1;
    }
    else if (left->sec < right->sec)
    {
        return -1;
    }
    else
    {
        if (left->nanosec > right->nanosec)
        {
            return 1;
        }
        else if (left->nanosec < right->nanosec)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

DDS_Boolean
DDS_Duration_is_infinite(const struct DDS_Duration_t *duration)
{
    return (duration->sec == DDS_DURATION_INFINITE_SEC) &&
           (duration->nanosec == DDS_DURATION_INFINITE_NSEC);
}

DDS_Boolean
DDS_Duration_equal(const struct DDS_Duration_t *self,
                   const struct DDS_Duration_t *other)
{
    return (self->sec == other->sec) &&
           (self->nanosec == other->nanosec);
}

void
DDS_Duration_set(struct DDS_Duration_t *self,
                 DDS_Long sec,DDS_UnsignedLong nanosec)
{
    self->sec = sec;
    self->nanosec = nanosec;
}

DDS_Boolean
DDS_Duration_is_zero(const struct DDS_Duration_t *duration)
{
    return (duration->sec == DDS_DURATION_ZERO_SEC) &&
           (duration->nanosec == DDS_DURATION_ZERO_NSEC);
}

DDS_Long
DDS_Duration_to_ms(const struct DDS_Duration_t *const self)
{
    return (self->sec * 1000) + (DDS_Long)(self->nanosec / 1000000U)
           + (self->nanosec % 1000000U ? 1 : 0);
}

RTI_BOOL
DDS_Duration_is_normalized(const struct DDS_Duration_t *const self)
{

    if (!DDS_Duration_is_infinite(self) &&
        ((self->nanosec >= 1000000000) || (self->sec < 0)))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_Duration_delta_gt(const struct DDS_Duration_t *const delta,
                      const struct DDS_Duration_t *const end,
                      const struct DDS_Duration_t *const begin)
{
    struct DDS_Duration_t diff;

    diff.sec = end->sec - begin->sec;
    diff.nanosec = end->nanosec;

    if (diff.nanosec < begin->nanosec)
    {
        --diff.sec;
        diff.nanosec += 1000000000;
    }

    diff.nanosec -= begin->nanosec;

    if (DDS_Duration_compare(&diff,delta) > 0)
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci @} */

