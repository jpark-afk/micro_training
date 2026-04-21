/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/

#include <math.h>

/*** INTERNAL API: OSAPI, CDR, OSAPI Headers ***/
/*#include "osapi/osapi_system.h"*/

#include "LatencyExample.h"

const double PERCENTILE_POINTS[PERCENTILE_COUNT] =
    { 0.5f, 0.90f, 0.99f, 0.9999f };

double
LatencyDataProcessor_clock_overhead(LatencyDataProcessor * self)
{
    return self->_clock_overhead;
}

void
LatencyDataProcessor_start_one_round(LatencyDataProcessor * self,
                                     int message_size)
{
    LatencyDataProcessor_reset(self);
    self->_message_size = message_size;
    self->_sequence_number = 0;
}

DDS_Boolean
LatencyDataProcessor_is_finished(LatencyDataProcessor * self)
{
    return self->_got_valid_echo;
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_delete(LatencyDataProcessor * self)
{

    if (self->_roundtrip_time != NULL)
    {
        free(self->_roundtrip_time);
    }

    if (self->_roundtrip_time_timestamp != NULL)
    {
        free(self->_roundtrip_time_timestamp);
    }

    if (self->_roundtrip_time_sorted != NULL)
    {
        free(self->_roundtrip_time_sorted);
    }
}

/* ---------------------------------------------------------------------*/
int
LatencyDataProcessor_find_hist_slot(LatencyDataProcessor * self,
                                    double latency, int low_slot, int high_slot,
                                    int slot_guess)
{
    if (self->_latency_slot[slot_guess] <= latency)
    {
        if (self->_latency_slot[slot_guess + 1] > latency)
        {
            return slot_guess;
        }
        else
        {
            return LatencyDataProcessor_find_hist_slot(self, latency,
                                                       slot_guess + 1,
                                                       high_slot,
                                                       (slot_guess +
                                                        high_slot) / 2);
        }
    }
    else
    {
        if (self->_latency_slot[slot_guess - 1] <= latency)
        {
            return (slot_guess - 1);
        }
        else
        {
            return LatencyDataProcessor_find_hist_slot(self, latency,
                                                       low_slot,
                                                       slot_guess,
                                                       (low_slot +
                                                        slot_guess) / 2);
        }
    }
    AppLog_exception("*** Error in find_hist_slot\n");
    return -1;
}

#if USE_QSORT
static int
compare_two_doubles(const void *double1, const void *double2)
{
    return ((*(double *)double1) >= (*(double *)double2));
}
#endif

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_compute_latency_percentiles(LatencyDataProcessor * self)
{
int slot;
int cumulative_count = 0;
int percentile_index;

    /* Define to use quicksort for sorting statistics.
     ** Note that not using quicksort is likely to be more stable.
     */
#if USE_QSORT
    AppLog_warn("starting qsort()\n");
    qsort(self->_roundtrip_time_sorted, self->_roundtrip_time_len,
          sizeof(*self->_roundtrip_time_sorted), compare_two_doubles);
    AppLog_warn("ending qsort()\n");

    percentile_index = (int)(double)(0.5 * self->_roundtrip_time_len);
    self->_latency_percentiles[PERCENTILE_INDEX_50] =
        self->_roundtrip_time_sorted[percentile_index];
    percentile_index = (int)(double)(0.9 * self->_roundtrip_time_len);
    self->_latency_percentiles[PERCENTILE_INDEX_90] =
        self->_roundtrip_time_sorted[percentile_index];
    percentile_index = (int)(double)(0.99 * self->_roundtrip_time_len);
    self->_latency_percentiles[PERCENTILE_INDEX_99] =
        self->_roundtrip_time_sorted[percentile_index];
    percentile_index = (int)(double)(0.9999 * self->_roundtrip_time_len);
    self->_latency_percentiles[PERCENTILE_INDEX_99_99] =
        self->_roundtrip_time_sorted[percentile_index];
#else

    for (slot = 0, percentile_index = 0; slot < self->_latency_slot_count;
         ++slot)
    {
        cumulative_count += self->_histogram[slot];
        while ((percentile_index < PERCENTILE_COUNT) &&
               (cumulative_count >=
                self->_message_count * PERCENTILE_POINTS[percentile_index]))
        {
            self->_latency_percentiles[percentile_index] =
                self->_latency_slot[slot];
            ++percentile_index;
        }
    }
#endif
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyDataProcessor_calculate_clock_overhead(LatencyDataProcessor * self)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    int i = 0;

    OSAPI_NtpTime begin_time = DDS_TIME_ZERO, 
        clock_roundtrip_time = DDS_TIME_ZERO;

    if (!OSAPI_System_get_time(&begin_time))
    {
        return DDS_BOOLEAN_FALSE;
    }


    for (i = 0; i < NUM_OF_LOOPS_CLOCK; ++i)
    {
        if (!OSAPI_System_get_time(&clock_roundtrip_time))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    OSAPI_NtpTime_decrement(&clock_roundtrip_time, &begin_time);
    self->_clock_overhead = 1E6 * OSAPI_NtpTime_toDouble(&clock_roundtrip_time) /
        (double)NUM_OF_LOOPS_CLOCK;

    return DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyDataProcessor_initialize(LatencyDataProcessor * self,
                                struct OSAPI_System * clock,
                                int roundtrip_array_len)
{
    int latency, count;

    if (!OSAPI_System_initialize())
    {
        return DDS_BOOLEAN_FALSE;
    }

    self->_array_index = 0;

    count = 0;
    for (latency = 0; latency < 200; ++latency)
    {
        self->_latency_slot[count++] = (double)latency;
    }
    for (latency = 200; latency < 1000; latency += (latency / 100))
    {
        latency -= (latency % 2);
        self->_latency_slot[count++] = (double)((int)latency);
    }
    for (latency = 1000; latency < 10000; latency += (latency / 100))
    {
        latency -= (latency % 10);
        self->_latency_slot[count++] = (double)((int)latency);
    }
    for (latency = 10000; latency < 100000; latency += (latency / 100))
    {
        latency -= (latency % 100);
        self->_latency_slot[count++] = (double)((int)latency);
    }
    for (latency = 100000; latency < 1000000; latency += (latency / 100))
    {
        latency -= (latency % 1000);
        self->_latency_slot[count++] = (double)((int)latency);
    }
    self->_latency_slot[count] = 1e9;

    if (count >= MAX_LATENCY_SLOTS)
    {
        AppLog_exception("configuration error. "
                         "Please increase MAX_LATENCY_SLOTS to at least %d\n",
                         count + 1);
        return DDS_BOOLEAN_FALSE;
    }

    self->_latency_slot_count = count;

    self->_clock = clock;
    LatencyDataProcessor_reset(self);
    self->_roundtrip_time_len = roundtrip_array_len;
    self->_roundtrip_time = (double *)calloc(roundtrip_array_len,
                                             sizeof(double));
    self->_roundtrip_time_timestamp = (double *)calloc(roundtrip_array_len,
                                                       sizeof(double));
    self->_roundtrip_time_sorted = (double *)calloc(roundtrip_array_len,
                                                    sizeof(double));
    return DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyDataProcessor_echo_received(LatencyDataProcessor * self)
{                               /* stop timer */
    if (!OSAPI_System_get_time(&self->_finish_time))
    {
        return DDS_BOOLEAN_FALSE;
    }
    return DDS_BOOLEAN_TRUE;
}


/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyDataProcessor_start_one_issue(LatencyDataProcessor * self)
{                               /* start timer */
    self->_got_valid_echo = DDS_BOOLEAN_FALSE;

   if (!OSAPI_System_get_time(&self->_start_time))
   {
       return DDS_BOOLEAN_FALSE;
   }
   return DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_reset(LatencyDataProcessor * self)
{                               /* start one new round (messageSize) */
int slot;

    /*** INTERNAL API: RTINtpTime_XXX ***/
    OSAPI_NtpTime_setZero(&self->_start_time);
    OSAPI_NtpTime_setZero(&self->_finish_time);
    OSAPI_NtpTime_setZero(&self->_recv_signaled_time);
    self->_got_valid_echo = DDS_BOOLEAN_FALSE;
    self->_message_count = -1;
    self->_message_size = 4;
    self->_sigma_roundtrip_time = 0.0;
    self->_sigma_roundtrip_time_squared = 0.0;
    self->_min_roundtrip_time = 1e6;
    self->_max_roundtrip_time = 0;
    for (slot = 0; slot < self->_latency_slot_count; ++slot)
    {
        self->_histogram[slot] = 0;
    }
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_finish_one_issue_recv_thread(LatencyDataProcessor * self)
{
    /*** INTERNAL API: RTINtpTime ***/
    OSAPI_NtpTime roundtrip = { 0, 0 };
    double roundtrip_in_double = 0.0;

    ++self->_sequence_number;   /* always increase the next sequence number */

    /* drop the first result since we do lazy alloc */
    if (self->_message_count >= 0)
    {

        /* The total roundtrip time */
        OSAPI_NtpTime_subtract(&roundtrip, &self->_finish_time,
                              &self->_start_time);
        roundtrip_in_double =
            1E6 * OSAPI_NtpTime_toDouble(&roundtrip) - self->_clock_overhead;
        if (roundtrip_in_double <= 0)
        {
            AppLog_exception("roundtrip time <= 0\n");
        }

        if (roundtrip_in_double < self->_min_roundtrip_time)
        {
            self->_min_roundtrip_time = roundtrip_in_double;
        }
        if (roundtrip_in_double > self->_max_roundtrip_time)
        {
            self->_max_roundtrip_time = roundtrip_in_double;
        }

        self->_roundtrip_time[self->_message_count] = roundtrip_in_double;
        self->_roundtrip_time_sorted[self->_message_count] =
            roundtrip_in_double;
        self->_roundtrip_time_timestamp[self->_message_count] =
            OSAPI_NtpTime_toDouble(&self->_start_time);
        self->_sigma_roundtrip_time += roundtrip_in_double;
        self->_sigma_roundtrip_time_squared +=
            (roundtrip_in_double * roundtrip_in_double);
    }

    ++self->_message_count;
    self->_got_valid_echo = DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_compute_statistics(LatencyDataProcessor * self)
{
int i, slot;

double time_ave = self->_sigma_roundtrip_time / (double)self->_message_count;

    self->_roundtrip_time_mean = time_ave;
    self->_roundtrip_time_std = sqrt((self->_sigma_roundtrip_time_squared /
                                      (double)self->_message_count) -
                                     (time_ave * time_ave));

    /* Compute latency_50_percentile, latency_99_percentile and 
     * latency_99.99_percentile.
     * In general of a value P of the percentile latency_P_percentile is the
     * value of the latency such that P% of the samples verify:
     *    latency <= latency_P_percentile.
     * For example latency_50_percentile is the median i.e. 50% of the 
     * samples have latency <= latency_50_percentile 
     */
    /* There are two ways to compute the percentiles. The most direct one
     * would be to sort all the samples, but this is an O(Nlog(N)) operation
     * A more efficient approach is to use the histogram bins which
     * we need for the output and is an O(N) operation and although not exact
     * gives the resolution of one bin.
     */
    for (i = 0; i < self->_message_count; ++i)
    {
        if (self->_roundtrip_time[i] < self->_latency_slot[0])
        {
            slot = 0;
        }
        else if (self->_roundtrip_time[i] >
                 self->_latency_slot[self->_latency_slot_count - 1])
        {
            slot = self->_latency_slot_count - 1;
        }
        else
        {
            slot = LatencyDataProcessor_find_hist_slot(self,
                                                       self->_roundtrip_time[i],
                                                       0,
                                                       self->
                                                       _latency_slot_count,
                                                       self->
                                                       _latency_slot_count / 2);
        }
        self->_histogram[slot]++;
    }

    LatencyDataProcessor_compute_latency_percentiles(self);
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_finish_one_round(LatencyDataProcessor * self)
{
    /* Must compute before printing/writing any results */
    LatencyDataProcessor_compute_statistics(self);

    /* print out result */
    AppLog_report("%6d,%7.1f,%7.1f,%7.1f,%7.1f,%7.1f,%7.1f,%7.1f,%7.1f\n",
                  self->_message_size,
                  self->_roundtrip_time_std, self->_roundtrip_time_mean,
                  self->_min_roundtrip_time,
                  self->_latency_percentiles[PERCENTILE_INDEX_50],
                  self->_latency_percentiles[PERCENTILE_INDEX_90],
                  self->_latency_percentiles[PERCENTILE_INDEX_99],
                  self->_latency_percentiles[PERCENTILE_INDEX_99_99],
                  self->_max_roundtrip_time);
    AppLog_flush;
}

/* ---------------------------------------------------------------------*/
void
LatencyDataProcessor_finish_one_round_average_only(LatencyDataProcessor * self)
{
    /* Must compute before printing/writing any results */
    OSAPI_NtpTime total_time;
    double time_avg;

    OSAPI_NtpTime_subtract(&total_time, &self->_finish_time, &self->_start_time);

    time_avg =
        (1E6 * OSAPI_NtpTime_toDouble(&total_time) -
         self->_clock_overhead) / (double)self->_message_count;

    self->_roundtrip_time_mean = time_avg;

    /* print out result */
    AppLog_report("%6d,%7.1f\n", self->_message_size,
                  self->_roundtrip_time_mean);
    AppLog_flush;
}
