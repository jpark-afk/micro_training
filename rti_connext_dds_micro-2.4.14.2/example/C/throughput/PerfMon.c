/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*
To create and use an effective performance monitoring application:

   1. Decide which counters (i.e. processes, etc.) you wish to monitor.
   2. Decide which instances you wish to apply these counters to, if applicable.
   3. Decide upon application design issues, such as the duration of the
      monitoring and monitoring intervals.
   4. Write the code to do the monitoring, incorporating the following basic
      steps:
          * create a query
          * associate counters with the query
          * collect and process the data
          * close the query. 
   5. Run the application.
   6. Analyze the data. 
*/
#include <stdio.h>
#include "osapi/osapi_process.h"

#ifndef ThroughputArgs_h
#include "ThroughputArgs.h"
#endif
#ifndef PerfMon_h
#include "PerfMon.h"
#endif
#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif

#ifdef RTI_LINUX
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/param.h>          /* <elf.h> */

void
PerfMon_delete(PerfMon * self)
{
}

void
PerfMon_create(PerfMon * self)
{
    self->Hertz = 0;
    self->prevtic = 0;
    self->oldtimev.tv_sec = 0;
    self->oldtimev.tv_usec = 0;
#ifdef DIVIDE_BY_NUMBER_OF_PROCESSORS
long smp_num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    /* SPARC glibc is buggy */
    self->Cpu_tot = OSAPI_Utility_max(smp_num_cpus, 1);
#endif

    self->_pid = OSAPI_Process_getpid();
    self->memBase = PerfMon_getMemory(self);
#ifndef HZ
#error "This code will not work"
#endif
    self->Hertz = HZ;
    if (self->Hertz == 0)
    {
        AppLog_exception("sysconf(_SC_NPROCESSORS_ONLN) failed");
    }
    PerfMon_getCpu(self);       /*the first run; result will be garbage so just ignore */
}

int
PerfMon_isValid(PerfMon * self)
{
    return (self->Hertz);
};

RTI_BOOL
PerfMon_readProcStat(PerfMon * self)
{
    RTI_BOOL ok = 0;
    int file = -1, bytesRead;
    char state;
    unsigned long garbage_ulong;
    int garbage_int;
    long garbage_long;
    unsigned long long cutime, cstime, garbage_longlong;
    int numScanned;
    char *S, *tmp, buf[1024], filename[40];
    const char *FUNC_NAME = "PerfMon_readProcStat";

    sprintf(filename, "/proc/%d/stat", self->_pid);
    file = open(filename, O_RDONLY, 0);
    if (file == -1)
    {
        AppLog_exception("failed to open %s", filename);
        goto fin;
    }

    bytesRead = read(file, buf, sizeof(buf) - 1);
    if (bytesRead <= 0)
    {
        AppLog_exception("%s read failed %d", FUNC_NAME, errno);
        goto fin;
    }

    buf[bytesRead] = 0;         /* terminating NULL */

    /* look for the command name inside parenthesis */
    S = strchr(buf, '(') + 1;
    tmp = strrchr(S, ')');
    if (!S || !tmp)
    {
        AppLog_exception("%s strchr failed %d", FUNC_NAME, errno);
        goto fin;
    }

#ifdef UNNECESSARY
    num = tmp - S;
    if (unlikely(num >= sizeof P->cmd))
    {
        num = sizeof P->cmd - 1;
    }
    memcpy(P->cmd, S, num);
    P->cmd[num] = '\0';
#endif
    S = tmp + 2;                /* skip ") " */

    /* and the rest are just bunch of numbers, so just read them in */
    numScanned = sscanf(S, "%c "        /* 1 */
                        "%d %d %d %d %d "       /* + 5 */
                        "%lu %lu %lu %lu %lu "  /* + 5 */
                        "%Lu %Lu %Lu %Lu "      /* utime stime cutime cstime *//* + 4 */
                        "%ld %ld "      /* + 2 */
                        "%d "   /* + 1 */
                        "%ld "  /* + 1 */
                        "%Lu "  /* start_time *//* + 1 */
                        "%lu "  /* vsize *//* + 1 */
                        ,       /* = 21 */
                        &state, &garbage_int, &garbage_int, &garbage_int, &garbage_int, &garbage_int, &garbage_ulong, &garbage_ulong, &garbage_ulong, &garbage_ulong, &garbage_ulong, &self->mStat_process.utime, &self->mStat_process.stime, &cutime, &cstime, &garbage_long, &garbage_long, &garbage_int, &garbage_long, &garbage_longlong,   /* start_time */
                        &self->mStat_process.vsize);    /* vsize [bytes] */
    if (numScanned != 21)
    {
        AppLog_exception("%s sscanf failed %d", FUNC_NAME, numScanned);
        goto fin;
    }

    ok = RTI_TRUE;
    fin:
    if (file != -1)
    {
        close(file);
    }
    return ok;
}

float
PerfMon_getCpu(PerfMon * self)
{
float cpuUsed = -1.0f, et, Frame_tscale;
struct timeval timev;
unsigned long long tics = self->prevtic;
unsigned long pcpu = 0;
const char *FUNC_NAME = "PerfMon::getCpu";

    if (!PerfMon_readProcStat(self))
    {
        AppLog_exception("failed to read /proc/[pid]/stat");
        goto fin;
    }

    tics = self->mStat_process.utime + self->mStat_process.stime;
    pcpu = tics - self->prevtic;

    /* TODO: use RDTSC to get a finer time resolution */
    self->prevtic = tics;       /* save into history, so we can get the delta next time */

    gettimeofday(&timev, NULL); /* don't care about the timezone */
    et =                        /* means elapsed time, in [sec] */
        (timev.tv_sec - self->oldtimev.tv_sec)
        + (float)(timev.tv_usec - self->oldtimev.tv_usec) / 1000000.0;

    Frame_tscale =
        (et >
         0.0f) ? 100.0f / ((float)self->Hertz * et /* * Cpu_tot */ ) : 1.0f;

    self->oldtimev = timev;

cpuUsed = (float)pcpu *Frame_tscale;
    /*if (cpuUsed > 100.0f) cpuUsed = 100.0f; */
    fin:
    return cpuUsed;
}

unsigned long
PerfMon_getMemory(PerfMon * self)
{
const char *FUNC_NAME = "PerfMon::getMemory";

    if (!PerfMon_readProcStat(self))
    {
        AppLog_exception("failed to read /proc/[pid]/stat");
        return 0;
    }
    return self->mStat_process.vsize;
}

#elif defined(RTI_WIN32)

#include <windows.h>

void
PerfMon_delete(PerfMon * self)
{
}

void
PerfMon_create(PerfMon * self)
{
    self->memBase = PerfMon_getMemory(self);
    PerfMon_getCpu(self);
}

int
PerfMon_isValid(PerfMon * self)
{
    return 1;
}

float
PerfMon_getCpu(PerfMon * self)
{
HANDLE pid = GetCurrentProcess();
struct _timeb timebNow, timebElapsed;

FILETIME createTime, exitTime;  
FILETIME kernelTimeProcess, userTimeProcess;
__int64 iKernelElapsed, iKernel, iUserElapsed, iUser;
float fCpuTimeTaken;
int iMs;
float fTimeElapsed;

    GetProcessTimes(pid, &createTime, &exitTime,
                    &kernelTimeProcess, &userTimeProcess);
    _ftime(&timebNow);

    iKernel = ((__int64) kernelTimeProcess.dwHighDateTime << 32)
        + kernelTimeProcess.dwLowDateTime;
    iUser = ((__int64) userTimeProcess.dwHighDateTime << 32)
        + userTimeProcess.dwLowDateTime;

    iKernelElapsed = iKernel - self->mIKernelLast;
    iUserElapsed = iUser - self->mIUserLast;
    fCpuTimeTaken = (float)(iKernelElapsed + iUserElapsed) / 10000000.0f;       //this tick in 100 nanosecond intervals in 1 second

    timebElapsed.time = timebNow.time - self->mTimebLast.time;
    iMs = (int)timebNow.millitm - (int)self->mTimebLast.millitm;
    if (iMs >= 0)
    {
        timebElapsed.millitm = iMs;
    }
    else
    {
        timebElapsed.time--;
        timebElapsed.millitm = 1000 + iMs;
    }
    fTimeElapsed = timebElapsed.time + (float)timebElapsed.millitm / 1000.0f;

    self->mTimebLast = timebNow;
    self->mIKernelLast = iKernel;
    self->mIUserLast = iUser;

    return 100.0f * fCpuTimeTaken / fTimeElapsed;
}

unsigned long
PerfMon_getMemory(PerfMon * self)
{
    return 0;
}

#elif defined(RTI_VXWORKS)

#include <taskLib.h>
#include <memLib.h>
#include <semLib.h>

typedef struct cpuUsage
{
    SEM_ID startSem;
    int didNotComplete;
    unsigned long ticksNoContention;
    int nBurnNoContention;
    unsigned long ticksNow;
    int nBurnNow;
    double usage;
} cpuUsage;

static cpuUsage *pcpuUsage = 0;

static double
cpuBurn()
{
int i;
double result = 0.0;

    for (i = 0; i < 5; i++)
        result += sqrt((double)i);
    return (result);
}

static void
cpuUsageTask()
{
    while (TRUE)
    {
int i;
unsigned long tickStart, tickEnd;

        semTake(pcpuUsage->startSem, WAIT_FOREVER);
        pcpuUsage->ticksNow = 0;
        pcpuUsage->nBurnNow = 0;
        tickStart = tickGet();
        for (i = 0; i < pcpuUsage->nBurnNoContention; i++)
        {
            cpuBurn();
            pcpuUsage->ticksNow = tickGet() - tickStart;
            ++pcpuUsage->nBurnNow;
        }
        tickEnd = tickGet();
        pcpuUsage->didNotComplete = FALSE;
        pcpuUsage->ticksNow = tickEnd - tickStart;
    }
}

static double
getCpu()
{
    if (pcpuUsage->didNotComplete && pcpuUsage->nBurnNow == 0)
    {
        pcpuUsage->usage = 0.0;
    }
    else
    {
double temp;
double ticksNow, nBurnNow;

        ticksNow = (double)pcpuUsage->ticksNow;
        nBurnNow = (double)pcpuUsage->nBurnNow;
        ticksNow *= (double)pcpuUsage->nBurnNoContention / nBurnNow;
        temp = ticksNow - (double)pcpuUsage->ticksNoContention;
        temp = 100.0 * temp / ticksNow;
        if (temp < 0.0 || temp > 100.0)
            temp = 0.0;         /*handle tick overflow */
        pcpuUsage->usage = temp;
    }
    pcpuUsage->didNotComplete = TRUE;
    semGive(pcpuUsage->startSem);
    return (pcpuUsage->usage);
}

static void
cpuUsageInit(void)
{
    unsigned long tickStart, tickNow;
    int nBurnNoContention = 0;
    int ticksToWait;

#define SECONDS_TO_BURN 5
    ticksToWait = SECONDS_TO_BURN * sysClkRateGet();
    pcpuUsage = calloc(1, sizeof(cpuUsage));
    tickStart = tickGet();
    /*wait for a tick */
    while (tickStart == (tickNow = tickGet()))
    {;
    }
    tickStart = tickNow;
    while (TRUE)
    {
        if ((tickGet() - tickStart) >= ticksToWait)
            break;
        cpuBurn();
        nBurnNoContention++;
    }
    pcpuUsage->nBurnNoContention = nBurnNoContention;
    pcpuUsage->startSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    pcpuUsage->ticksNoContention = ticksToWait;
    pcpuUsage->didNotComplete = TRUE;
    taskSpawn("cpuUsageTask", 255, VX_FP_TASK, 1000, (FUNCPTR) cpuUsageTask,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void
PerfMon_delete(PerfMon * self)
{
}

void
PerfMon_create(PerfMon * self)
{
    self->memBase = PerfMon_getMemory(self);
    cpuUsageInit();
}

int
PerfMon_isValid(PerfMon * self)
{
    return 1;
}

float
PerfMon_getCpu(PerfMon * self)
{
    return (float)getCpu();
}

unsigned long
PerfMon_getMemory(PerfMon * self)
{
MEM_PART_STATS mempart;
#ifdef RTI_RTP
    memInfoGet(&mempart);
#else
    memPartInfoGet(memSysPartId, &mempart);
#endif
    return mempart.numBytesAlloc;
    /*return 0; */
}

#else

void
PerfMon_delete(PerfMon * self)
{
}

void
PerfMon_create(PerfMon * self)
{
}

int
PerfMon_isValid(PerfMon * self)
{
    return 1;
}

float
PerfMon_getCpu(PerfMon * self)
{
    return 0.0f;
}

unsigned long
PerfMon_getMemory(PerfMon * self)
{
    return 0;
}

#endif
