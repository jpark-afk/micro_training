/*
 * FILE: posixShmMonitor.h - POSIX shared monitor definitions
 *
 * Copyright 2022-2022 Real-Time Innovations, Inc.
 * All rights reserved.
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef posixShmMonitor_h
#define posixShmMonitor_h

#include "rti_me_psl.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "netio_zcopy/netio_zcopy_shm_monitor.h"

#include <pthread.h>

struct OSAPI_SharedMemoryMonitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* posixShmMonitor_h */
