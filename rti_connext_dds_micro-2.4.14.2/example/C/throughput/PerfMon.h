/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/09 14:17:37 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef PerfMon_h
#define PerfMon_h
#ifdef RTI_WIN32
  #include <sys/timeb.h>
#endif
#include "osapi/osapi_types.h"
#ifdef RTI_LINUX
#include <sys/time.h>
#endif

#ifdef RTI_LINUX
typedef struct Mstat_process {
    unsigned long long utime, stime; /* [jiffies] */
    unsigned long vsize; /* [bytes] */
} Mstat_process;

typedef struct NetDevStat {
    unsigned long byte_recv, packet_recv, err_recv, drop_recv, mcast_recv,
        byte_sent, packet_sent, err_sent, drop_sent, colls_sent;
} NetDevStat;
#endif

typedef struct PerfMon {
    RTI_UINT32 _pid;
#ifdef RTI_LINUX
    int Cpu_tot;
    unsigned long long Hertz, prevtic;
    struct timeval oldtimev;
    Mstat_process mStat_process;
    NetDevStat mStatNetDev_current, mStatNetDev_initial;
#endif
#if defined(RTI_WIN32)
    struct _timeb mTimebLast;
    __int64 mIKernelLast, mIUserLast;
#endif
    unsigned long memBase;
} PerfMon;

void PerfMon_delete(PerfMon *self);
void PerfMon_create(PerfMon *self);

int PerfMon_isValid(PerfMon *self);
float PerfMon_getCpu(PerfMon *self);
unsigned long PerfMon_getMemory(PerfMon *self);
#ifdef RTI_LINUX
RTI_BOOL PerfMon_readProcStat(PerfMon *self);
#endif

#endif/*PerfMon_h*/
