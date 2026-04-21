/*
 * FILE: netio_zcopy_shm_monitor.h - Shared memory monitor
 *
 * Copyright 2022-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_zcopy_shm_monitor_h
#define netio_zcopy_shm_monitor_h

#include "netio_zcopy_shm_segment.h"
#include "osapi/osapi_config.h"
#include "netio_zcopy/netio_zcopy_dll.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*e
 * \defgroup OSAPI_SharedMemoryMonitorClass OSAPI_SharedMemoryMonitor
 * \ingroup OSAPIModule
 * \brief A shared memory monitor to provide mutual exclusion between
 *        processes and inter-process signaling.
 */

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * Opaque implementation of a shared memory monitor.
 */
struct OSAPI_SharedMemoryMonitor;

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Get the amount of memory required to store
 * OSAPI_SharedMemoryMonitor on the current platform.
 *
 * \return The number of bytes required to store a shared memory monitor.
 */
NETIOPSLDllExport RTI_SIZE_T
OSAPI_SharedMemoryMonitor_get_size(void);

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Initialize a shared memory monitor in a provided memory segment.
 *
 * \param[in] mem_len Size of provided memory.
 *
 * \param[in] mem Memory segment.
 *
 * \param[out] self_out Pointer to initialized monitor.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_initialize(
        RTI_SIZE_T mem_len,
        void *mem,
        struct OSAPI_SharedMemoryMonitor **self_out);

#ifndef RTI_CERT
/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Finalize a shared memory monitor.
 *
 * \param[in] self Monitor to finalize.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this monitor.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_finalize(struct OSAPI_SharedMemoryMonitor *self);
#endif /* !RTI_CERT */

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Acquire the lock protecting the monitor.
 * 
 * \details  
 * If this function returns RTI_TRUE, the calling thread is guaranteed
 * mutual exclusion by the monitor. No other thread can acquire the lock
 * until the lock is returned with \ref OSAPI_SharedMemoryMonitor_release.
 * This function shall not be called by a thread which currently holds the lock.
 *
 * If this function returns status_out=OSAPI_SHMEM_STATUS_OWNER_DEAD, then any
 * shared data protected by the monitor may be in an inconsistent state. If the
 * shared data can be restored to a consistent state, then it can be marked as
 * consistent with \ref OSAPI_SharedMemoryMonitor_mark_consistent. Otherwise,
 * any subsequent attempts to acquire or wait on the monitor will fail.
 *
 * \param[in] self Monitor to acquire.
 *
 * \param[out] status_out If successful, the status of the protected state.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_acquire(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out);

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Release the lock protecting the monitor.
 *
 * \param[in] self Monitor to release.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * This function can only be called on a monitor which is currently locked by
 * the calling thread.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_release(struct OSAPI_SharedMemoryMonitor *self);

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Wait for the monitor to be signaled.
 * 
 * \details 
 * Only one thread can wait on a monitor at any one time. This function can only
 * be called on a monitor which is currently locked by the calling thread. The
 * function will return successfully only when another thread has called
 * \ref OSAPI_SharedMemoryMonitor_signal and when the calling thread has
 * acquired the lock.
 *
 * If this function returns status_out=OSAPI_SHMEM_STATUS_OWNER_DEAD, then any
 * shared data protected by the monitor may be in an inconsistent state. If the
 * shared data can be restored to a consistent state, then it can be marked as
 * consistent with \ref OSAPI_SharedMemoryMonitor_mark_consistent. Otherwise,
 * any subsequent attempts to acquire or wait on the monitor will fail.
 *
 * \param[in] self Monitor to wait on.
 *
 * \param[out] status_out If successful, the status of the protected state.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * 
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_wait(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out);

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Signal a thread waiting on a monitor to wake up.
 *
 * \param[in] self Monitor to signal.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * This function can only be called on a monitor which is currently locked by
 * the calling thread.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_signal(struct OSAPI_SharedMemoryMonitor *self);

/*e
 * \ingroup OSAPI_SharedMemoryMonitorClass
 *
 * \brief Mark the shared state protected by the monitor as consistent after
 *        a process died while holding the lock on the monitor.
 * 
 * \details 
 * This function can only be called on a monitor which is currently locked by
 * the calling thread and currently inconsistent. The function shall return the
 * monitor to a consistent state and allow other functions to be called on it.
 * This function may not be supported on all platforms.
 *
 * \param[in] self Monitor to mark consistent.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemoryMonitor_mark_consistent(struct OSAPI_SharedMemoryMonitor *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_shm_monitor_h */
