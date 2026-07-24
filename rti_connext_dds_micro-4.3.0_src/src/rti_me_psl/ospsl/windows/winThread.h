/*
 * FILE: winThread.h - Windows Thread API
 *
 * Copyright (c) 2024-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */

#ifndef winThread_h
#define winThread_h

#include "rti_me_psl.h"

struct OSAPI_WinNativeThread;

extern void
OSAPI_Thread_start_native(void *thread);

extern RTI_BOOL
OSAPI_Thread_destroy_win_native(struct OSAPI_WinNativeThread *thread);

extern struct OSAPI_WinNativeThread*
OSAPI_Thread_create_win_native(const char *name,
                                 const struct OSAPI_ThreadProperty *property,
                                 OSAPI_ThreadRoutine thread_entry,
                                 void *thread_data,
                                 OSAPI_ThreadRoutine thread_wakeup);

#endif
