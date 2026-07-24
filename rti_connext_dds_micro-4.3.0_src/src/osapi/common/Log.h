/*
 * FILE: Log.h - Log functionality
 *
 * Copyright 2008-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 02feb2015,tk MICRO-1043/PR#13558 Removed OSAPI_Log_msg_pN_X2_i prototype
 * 22sep2011,tk Updated
 * 23sep2008,yy Created
 *
 */
#ifndef Log_h
#define Log_h

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif

#if OSAPI_ENABLE_LOG
extern RTI_SIZE_T
OSAPI_Log_ptr_diff(const char *inptr,RTI_SIZE_T offset,RTI_UINT32 alignment);
#endif

#ifndef RTI_CERT
extern void*
OSAPI_Log_get_log_buffer(void);

extern void
OSAPI_Log_set_log_buffer(void *buffer);
#endif

extern void
OSAPI_Log_write_stringz(char *buffer);

#endif /* Log_h */
