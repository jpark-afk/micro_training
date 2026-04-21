/*
 * FILE: osapi_shm_semaphore.h
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_shmem_semaphore
#define netio_shmem_semaphore

#include "netio_shmem/netio_shmem_dll.h"
#include "osapi/osapi_types.h"

struct NETIO_SharedMemorySignalingSemaphoreHandle
{
    RTI_UINT64 reserved[4];
};

/*e
 * \defgroup NETIO_SharedMemorySignalingSemaphoreClass \
 * Shared Memory Signaling Semaphore
 * \ingroup NETIO_SharedMemoryClass
 */

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 *
 * \brief Opaque handle initialized by the '_create' or '_attach' function
 * that must be used to interact with a shared memory binary semaphore.
 *
 * Allow access to a shared memory binary semaphore so that
 * multiple processes can signal and synchronize each other.
 *
 * A shared binary semaphore is a kernel object that allows multiple threads in
 * different processes to synchronize with each other.
 *
 * The lifecycle of a semaphore reflect the same lifecycle of the other
 * shared memory objects (segment, mutex, semaphore).
 *
 * Signaling Semaphores are initialized with a count of zero. That is, it
 * initially not-signaled. If a thread tries to take the semaphore
 * before it is given it will block.
 *
 * Signaling semaphores differs from Semaphores that the upper value
 * is 1. That is, you can call several times the _signal call, but the
 * next call to the _wait function, will bring the semaphore value
 * to 0 (and the next subsequent call to _wait will block the caller).
 */
struct NETIO_SharedMemorySignalingSemaphoreHandle;

/*e \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 *  \brief Initializes a shared memory binary semaphore with the provided
 *  key and sets up the handle to access it.
 *
 *  @pre Handle in the DETACHED state.  Signaling semaphore in the
 *  INITIAL, that is, Nobody has called create() before with
 *  that same key
 *
 *  @post handle in the ATTACHED state,  Semaphore exists and ready to
 *  be used.
 *
 *  @param[in,out] handle The shared memory semaphore handle to
 *  initialize.
 *
 *  @param[out] status_out Optional: if not NULL is set to the reason
 *  why the call has succeeded or failed. In particular:
 *  <ul>
 *    <li><b>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</b> =
 *        precondition check failed.
 *    <li><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 *        an error occurred in the os-specific shared memory implementation
 *        (see error log for details).
 *    <li><B>OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED</B> =
 *        semaphore already exist (no checks is performed whether the creator
 *        is still alive or not).
 *    <li><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 *        the semaphore has been successfully created.
 *  </ul>
 *
 *  @param[in] key Key to identify the shared memory semaphore
 *  across processes. Other processes should call
 *  \ref NETIO_SharedMemorySignalingSemaphore_attach with the same
 *  key to attach to this semaphore.
 *
 *  @return RTI_TRUE on success. RTI_FALSE on error.
 *
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_create(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 * \brief Attach to an existing shared memory binary semaphore with the provided
 * key and sets up the handle to access it.
 *
 * @pre Handle in the DETACHED state. Signaling semaphore in the
 * CREATED state. Some other process has called
 * \ref NETIO_SharedMemorySignalingSemaphore_create using the same
 * key in the shared memory semaphore
 *
 * @post handle in the ATTACHED state, Semaphore exists and is ready to
 * be used.
 *
 * @param[out] handle The shared memory semaphore handle to
 * initialize.
 *
 * @param[out] status_out Optional. If not NULL, will be set to:
 * <UL>
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</B> =
 * precondition check failed.
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B> =
 * no such a semaphore.
 * <LI><B>OSAPI_SHARED_MEMORY_ATTACHED</B> =
 * semaphore attached successfully.
 * </UL>
 *
 * @param[in] key Key to identify the shared memory semaphore
 * across processes.
 *
 * @return RTI_TRUE on success. RTI_FALSE on error.
 *
 * @note On some platforms this method makes sure that the given key represent
 * a shared memory semaphore object (to avoid situations like the
 * same key is used to create a mutex and then used to attach it as a
 * semaphore). If such a situation is detected, the function will
 * fail and set status_out to:
 * OSAPI_SHARED_MEMORYFAIL_REASON_NO_ENTRY.
 * Not all the platforms support this feature.
 *
 * @see NETIO_SharedMemorySignalingSemaphore_detach
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_attach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 * \brief Attempts to create a new binary semaphore with the provided
 * key and if the semaphore already exist, attach to it.
 *
 * @pre Handle in the DETACHED state.  Signaling semaphore in the
 * INITIAL, that is, Nobody has called create() before with
 * that same key.
 *
 * @post handle in the ATTACHED state,  Semaphore is in the CREATED state
 * and ready to be used.
 *
 * @param[in,out] handle The shared memory semaphore handle to
 * initialize.
 *
 * @param[out] status_out Optional: if not NULL is set to the reason
 * why the call has succeeded or failed. In particular:
 * <ul>
 * <li><b>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</b> =
 * precondition check failed.
 * <li><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B> =
 * this error should never happen and is caused by a failed create()
 * followed by a failed attach().
 * <li><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 * the semaphore has been successfully created.
 * </ul>
 *
 * @param[in] key Key to identify the shared memory binary semaphore
 * across processes.
 *
 * @return RTI_TRUE on success. RTI_FALSE on error.
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_create_or_attach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 * \brief Give the semaphore
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post Semaphore count is set to 1 and any thread blocked on this
 * semaphore is awaken.
 *
 * @param[in] handle Handle to the semaphore. Must match the one
 * returned by \ref NETIO_SharedMemorySignalingSemaphore_create
 *
 * @param[out] status_out Optional: if not NULL, will be set to:
 * <UL>
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B> =
 * the creator of the mutex has shutdown and removed this mutex.
 * This is expected to happen routinely.
 * <LI><B>OSAPI_SHARED_MEMORY_SUCCESS</B> =
 * Generic no error condition.
 * </UL>
 *
 * @return RTI_TRUE if the semaphore is given. RTI_FALSE if an error
 * occurs. If an error occurs the handle should not be used anymore. In
 * particular detach() should not be called either. An error may occur
 * in certain OSs if the handle that created the semaphore calls
 * \ref NETIO_SharedMemorySignalingSemaphore_detach.
 *
 * @see NETIO_SharedMemorySignalingSemaphore_wait
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_signal(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out);

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 * \brief Take the semaphore
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post Semaphore count is decremented.
 *
 * @param[in] handle Handle to the semaphore. Must match the one
 * returned by \ref NETIO_SharedMemorySignalingSemaphore_create
 *
 * @param[out] status_out Optional: if not NULL, will be set to:
 * <UL>
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B> =
 * the creator of the mutex has shutdown and removed this mutex.
 * This is expected to happen routinely.
 * <LI><B>OSAPI_SHARED_MEMORY_SUCCESS</B> =
 * Generic no error condition.
 * </UL>
 *
 * @return RTI_TRUE if the semaphore is taken. RTI_FALSE if an error
 * occurs. If an error occurs the handle should not be used anymore. In
 * particular \ref NETIO_SharedMemorySignalingSemaphore_detach should not
 * be called either.
 *
 * @see NETIO_SharedMemorySignalingSemaphore_signal
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_wait(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out);

/*e \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 *  \brief Detach from the shared memory binary semaphore.
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post handle in the DETACHED state.
 *
 * @param[in] handle Handle to the shared memory semaphore
 * obtained from \ref NETIO_SharedMemorySignalingSemaphore_attach
 *
 * @see NETIO_SharedMemorySignalingSemaphore_attach
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_detach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle);

/*e
 * \ingroup NETIO_SharedMemorySignalingSemaphoreClass
 * \brief Detach from the shared memory semaphore and deletes it.
 *
 * The exact behavior of this may be OS dependent. In some OSs calling
 * this will immediately cause the semaphore to be removed, and other
 * threads waiting on it to be woken up. This is the typical behavior
 * on Unix. In other OSs (windows) this call just requests the
 * semaphore to be removed but delay the removal until all handles are
 * detached.
 *
 * The only way to ensure the same behavior across multiple OSs is to
 * only call this method on the handle that created the semaphore after all
 * the other handles have been closed. In this scenario no handles will
 * get an error. Alternatively the code could be written to always
 * expect the \ref NETIO_SharedMemorySignalingSemaphore_wait or
 * \ref NETIO_SharedMemorySignalingSemaphore_signal call to fail and when that
 * occurs close the handle.
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post handle in the DETACHED state.  The numHandles to the semaphore
 * is decremented. When the last handle is detached, the semaphore is
 * removed by the OS.
 *
 * @param[in] handle Handle to the shared memory semaphore
 * obtained from \ref NETIO_SharedMemorySignalingSemaphore_create
 *
 * @see NETIO_SharedMemorySignalingSemaphore_create
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySignalingSemaphore_delete(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle);

#endif /* netio_shmem_semaphore */
