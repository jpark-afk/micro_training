/*
 * FILE: rti_me_psl.h - Definition of PSL APIs
 *
 * Copyright 2023-2024 Real-Time Innovations, Inc.
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
 * 11apr2023,tk Writen
 */
/*i \file
  * \brief Common PSL APIs
  */
#ifndef rti_me_psl_h
#define rti_me_psl_h

#include "osapi/osapi_features.h"

#include "rti_me_psl/ospsl/ospsl_os.h"

#include "osapi/osapi_cc.h"

#include "rti_me_psl/rti_me_psl_dll.h"

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

 extern RTIME_PSLDllVariable const char* empty;

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* rti_me_psl_h */
