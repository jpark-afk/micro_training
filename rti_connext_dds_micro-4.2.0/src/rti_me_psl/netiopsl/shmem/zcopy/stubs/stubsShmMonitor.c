/*
 * FILE: stubsShmMonitor.c -  shared memory monitor stubbed implementation
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rti_me_psl.h"
#include "netio_zcopy/netio_zcopy_shm_monitor.h"


/*** SOURCE_BEGIN ***/

RTI_SIZE_T
OSAPI_SharedMemoryMonitor_get_size(void)
{
    return 0;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_initialize(
        RTI_SIZE_T mem_len,
        void *mem,
        struct OSAPI_SharedMemoryMonitor **self_out)
{
    UNUSED_ARG(mem_len);
    UNUSED_ARG(mem);
    UNUSED_ARG(self_out);
    return RTI_FALSE;
   
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_SharedMemoryMonitor_finalize(struct OSAPI_SharedMemoryMonitor *self)
{
    UNUSED_ARG(self);
    return RTI_FALSE;
}
#endif /* !RTI_CERT */

RTI_BOOL
OSAPI_SharedMemoryMonitor_acquire(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out)
{
    UNUSED_ARG(self);
    UNUSED_ARG(status_out);
    return RTI_FALSE;
    
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_release(struct OSAPI_SharedMemoryMonitor *self)
{
    UNUSED_ARG(self);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_wait(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out)
{
    UNUSED_ARG(self);
    UNUSED_ARG(status_out);
    return RTI_FALSE;
    
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_signal(struct OSAPI_SharedMemoryMonitor *self)
{
    UNUSED_ARG(self); 
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_mark_consistent(struct OSAPI_SharedMemoryMonitor *self)
{
    UNUSED_ARG(self);
    return RTI_FALSE;
}

