/*
 * FILE: Duration.c - Duration implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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


/*ci
 * \brief Number of nanoseconds in a second
 */
#define NANOSEC_IN_SEC   (1000000000)

/*** SOURCE_BEGIN ***/

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
int
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


/*i @ingroup DDSBasicTypesModule
 *  \brief Convert from Duration format to ntp time format
 *
 *  @pre Arguments must be non-NULL.
 *
 *  \param[in]  self     The duration to convert
 *  \param[out] sec_out  The ntp seconds part of the time
 *  \param[out] frac_out The ntp frac part of the time
 *
 */
void
DDS_Duration_to_ntp_format(const struct DDS_Duration_t *self,
    RTI_INT32 *sec_out, RTI_UINT32 *frac_out)
{
    DDS_Long origSec = 0;
    DDS_Long sec = 0;
    RTI_UINT32 unsigned_sec = 0;
    OSAPI_SystemTime time;

    if (DDS_Duration_is_infinite(self))
    {
        *sec_out = RTPS_NTP_TIME_SEC_MAX;
        *frac_out = RTPS_NTP_TIME_FRAC_MAX;
    }
    else
    {
        /* OSAPI_SystemTime_to_ntp assumes that nanoseconds is less
         * than one billion. We need to make sure of that here.
         */
        origSec = self->sec;
        sec = self->sec + (DDS_Long) (self->nanosec / OSAPI_TIME_NSEC_PER_SEC);
        if (sec < origSec)
        {
            /*overflow! */
            sec = RTPS_NTP_TIME_SEC_MAX;
        }
        time.nanosec = (self->nanosec % OSAPI_TIME_NSEC_PER_SEC);
        time.sec = (RTI_INT64)sec;
        OSAPI_SystemTime_to_ntp(&unsigned_sec, frac_out, &time);

        /* duration has a max size of INT_MAX. This cast is safe. */
        *sec_out = (RTI_INT32)unsigned_sec;
    }
}

void
DDS_Duration_div(struct DDS_Duration_t *duration,
                 DDS_UnsignedLong div)
{
    if (DDS_Duration_is_infinite(duration) ||
        (div == 0))
    {
        duration->sec = DDS_DURATION_INFINITE_SEC;
        duration->nanosec = DDS_DURATION_INFINITE_NSEC;
    }
    else
    {
        /* div is only used for positive numbers what fits in DDS_Long */
        DDS_Long sdiv = (DDS_Long)div;

        duration->nanosec /= div;
        if (duration->sec % sdiv)
        {
            duration->sec /= sdiv;
            /* note that this cannot be greater than 1 sec as we
             * previously divided duration->nanosec
             */
            duration->nanosec += (NANOSEC_IN_SEC / div);
        }
        else
        {
            duration->sec /= sdiv;
        }
    }
}

DDS_Boolean
DDS_Duration_is_infinite(const struct DDS_Duration_t *duration)
{
    return (duration->sec == DDS_DURATION_INFINITE_SEC) &&
           ((duration->nanosec == DDS_DURATION_INFINITE_NSEC) ||
            (duration->nanosec == DDS_DURATION_INFINITE_NSEC_INITIALIZER_OLD));
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

RTI_BOOL
DDS_Duration_is_normalized(const struct DDS_Duration_t *const self)
{

    if (!DDS_Duration_is_infinite(self) &&
        ((self->nanosec >= NANOSEC_IN_SEC) || (self->sec < 0)))
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
        diff.nanosec += NANOSEC_IN_SEC;
    }

    diff.nanosec -= begin->nanosec;

    if (DDS_Duration_compare(&diff,delta) > 0)
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_BOOLEAN_FALSE;
}


RTI_BOOL
DDS_Duration_serialize(struct CDR_Stream_t *stream,
                                    const struct  DDS_Duration_t *duration_in,
                                    void *param)
{
    const struct DDS_Duration_t *duration =
                                    (const struct DDS_Duration_t *)duration_in;
    RTI_INT32 sec;
	RTI_UINT32 frac;

    UNUSED_ARG(param);

    DDS_Duration_to_ntp_format(duration, &sec, &frac);

    if (!CDR_Stream_serialize_long(stream, &sec))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_long(stream, &frac))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_Duration_deserialize(struct CDR_Stream_t *stream,
                                      struct DDS_Duration_t *duration,
                                      void *param)
{
    RTI_INT32 ntp_sec;
    RTI_UINT32 ntp_frac;
    RTI_UINT32 out_sec;
    RTI_UINT32 out_nanosec;
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_long(stream, &ntp_sec))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_unsigned_long(stream, &ntp_frac))
    {
        return RTI_FALSE;
    }

    if (RTPS_Duration_is_infinite(ntp_sec))
    {
        duration->sec = DDS_DURATION_INFINITE_SEC;
        duration->nanosec = DDS_DURATION_INFINITE_NSEC;
    }
    else
    {
        /* Negative time is not supported. */
        OSAPI_Time_from_ntp(&out_sec, &out_nanosec,(RTI_UINT32)ntp_sec,ntp_frac);
        duration->sec = (DDS_Long)out_sec;
        duration->nanosec = out_nanosec;
    }

    return RTI_TRUE;
}


/*ci @} */

