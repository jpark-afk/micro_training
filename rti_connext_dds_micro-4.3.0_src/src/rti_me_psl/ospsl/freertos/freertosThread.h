/*
 * FILE: freertosThread.h - FreeRTOS Thread API
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */

#include "rti_me_psl.h"

#ifndef freertosThread_h
#define freertosThread_h

struct OSAPI_FreeRTOSNativeThread;

extern RTI_BOOL
OSAPI_Thread_destroy_freertos_native(struct OSAPI_FreeRTOSNativeThread *thread);

extern struct OSAPI_FreeRTOSNativeThread*
OSAPI_Thread_create_freertos_native(const char *name,
                                    const struct OSAPI_ThreadProperty *property,
                                    OSAPI_ThreadRoutine thread_entry,
                                    void *thread_data,
                                    OSAPI_ThreadRoutine thread_wakeup);

#endif