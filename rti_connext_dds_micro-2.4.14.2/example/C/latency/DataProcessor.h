/*
 (c) Copyright, Real-Time Innovations, 2009-2015
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/
#ifndef DataProcessor_h
#define DataProcessor_h

/* MAX_LATENCY_SLOTS
** Number of histogram bins to store count vs latency range. 
** Used to size LatencyDataProcessor::_latencySlot
** and LatencyDataProcessor::_histogram.
*/
#define MAX_LATENCY_SLOTS (20000)

/* PERCENTILE_COUNT
** Defines the values of the percentiles for which we want to compute 
** the latency. 
** The algorithm used requires that values in the PERCENTILE_POINTS
** are sorted in ascending order.
*/
#define PERCENTILE_COUNT   (4)

#define PERCENTILE_INDEX_50 (0)
#define PERCENTILE_INDEX_90 (1)
#define PERCENTILE_INDEX_99 (2)
#define PERCENTILE_INDEX_99_99 (3)

typedef struct LatencyDataProcessor {
  
    DDS_Long _sequence_number;
    DDS_Boolean _got_valid_echo;
    int _message_count;
    int _message_size;
    int _array_index;

    struct OSAPI_System *_clock;
    OSAPI_NtpTime _start_time, _finish_time, _recv_signaled_time;
    double _clock_overhead;

    double _min_roundtrip_time, _max_roundtrip_time;
    double _sigma_roundtrip_time, _sigma_roundtrip_time_squared;
    double _roundtrip_time_mean, _roundtrip_time_std;
    
    double _sigma_roundtrip_time_array[LATENCY_ROUND_MAX];
    double _sigma_roundtrip_time_squared_array[LATENCY_ROUND_MAX];
    int _message_size_array[LATENCY_ROUND_MAX];
    int _count_array[LATENCY_ROUND_MAX];
    int _roundtrip_time_len;

    /* array with _roundtripTimeLen elements */
    double *_roundtrip_time; 

    /* Array that keps sort order of _roundtripTime */
    double *_roundtrip_time_sorted;    

    /* array with _roundtripTimeLen elements */
    double *_roundtrip_time_timestamp; 

    int _latency_slot_count;
    double _latency_slot[MAX_LATENCY_SLOTS];
    int _histogram[MAX_LATENCY_SLOTS];
    double _latency_percentiles[PERCENTILE_COUNT];
} LatencyDataProcessor;

void LatencyDataProcessor_delete(LatencyDataProcessor *self);
void LatencyDataProcessor_compute_statistics(LatencyDataProcessor *self);
DDS_Boolean LatencyDataProcessor_calculate_clock_overhead(LatencyDataProcessor *self);
double LatencyDataProcessor_clock_overhead(LatencyDataProcessor *self);
DDS_Boolean LatencyDataProcessor_initialize(LatencyDataProcessor *self, struct OSAPI_System *clock, int roundtrip_array_len);
void LatencyDataProcessor_reset(LatencyDataProcessor *self);
DDS_Boolean LatencyDataProcessor_start_one_issue(LatencyDataProcessor *self);
void LatencyDataProcessor_finish_one_issue_recv_thread(LatencyDataProcessor *self);
void LatencyDataProcessor_start_one_round(LatencyDataProcessor *self, int message_size);
void LatencyDataProcessor_finish_one_round(LatencyDataProcessor *self);
void LatencyDataProcessor_finish_one_round_average_only(LatencyDataProcessor *self);
int LatencyDataProcessor_find_hist_slot(LatencyDataProcessor *self, double latency, int low_slot, int high_slot, int slot_guess);
void LatencyDataProcessor_compute_latency_percentiles(LatencyDataProcessor *self);
DDS_Boolean LatencyDataProcessor_echo_received(LatencyDataProcessor *self);
DDS_Boolean LatencyDataProcessor_is_finished(LatencyDataProcessor *self);

#define OSAPI_NtpTime_toDouble(t) \
   (((double)((t)->frac)/4294967296.0f) + (double)(t)->sec)

#define OSAPI_NtpTime_setZero(time) \
{ \
    (time)->sec = 0; \
    (time)->frac = 0; \
}

#endif/*DataProcessor_hxx*/
