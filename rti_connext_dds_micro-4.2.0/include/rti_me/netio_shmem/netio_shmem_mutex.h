/*
 * FILE: osapi_shm_mutex.h
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_shmem_mutex
#define netio_shmem_mutex

#include "netio_shmem/netio_shmem_dll.h"
#include "osapi/osapi_types.h"

#define SEMMUTEX_TYPE_SEMAPHORE        0
#define SEMMUTEX_TYPE_BINARYSEM        1
#define SEMMUTEX_TYPE_MUTEX            2
#define SEMMUTEX_TYPE_ROBUST_MUTEX     3

/*e
 * \defgroup NETIO_SharedMemoryMutexClass Shared Memory Mutex API
 * \ingroup NETIO_SharedMemoryClass
 */

struct NETIO_SharedMemoryMutexHandle
{
    RTI_UINT64 reserved[4];
};

/*i
 * \ingroup NETIO_SharedMemoryMutexClass
 *
 * This is the guts of the shared memory handle. It is kept as a
 * private structure so it can be modified without breaking backwards
 * compatibility.
 *
 * The structure itself can be modified, provided that we don't exceed
 * the size of the padding.
 */
struct NETIO_SharedMemorySemMutexHandleImpl
{
    /*i
     * handle to the shared memory.
     * Used as a reference when delete the shared memory
     */
    NETIO_SharedMemoryNativeHandleSemMutex _native_hndl;

    /*i
     * Copy of the key used to retrieve this mutex.
     * Some architectures requires it when the resource is deleted.
     */
    RTI_INT32 _key;

    /*i
     * The type of semaphore: SEMMUTEX_TYPE_SEMAPHORE...
     * This value is used to check the precondition of each SemMutex
     * function, to make sure we don't mix and match create/attach of
     * semaphore of one type and perform operation with another type.
     * This value is used ONLY when RTI_PRECONDITION_TEST is defined.
     */
    RTI_INT32 _sem_type;

    /*i
     * This counter is used by the MUTEX type only to keep track of how
     * many locks() has been performed.
     * Probably this field should stay only in the MUTEX definition...
     */
    RTI_INT32 _lock_count;

    /*i
     * This value is used by the MUTEX type only to store the PID of the
     * thread/process that performed the MUTEX Lock. This is to ensure
     * that a thread sharing the same mutex handle doesn't call the _unlock
     * if is not the owner of the mutex.
     * Probably this field should stay only in the MUTEX definition...
     */
    OSAPI_ThreadId _lock_pid;
};

/*i
 *\ingroup NETIO_SharedMemoryMutexClass
 *
 * This are the guts of the shared memory handle. It is kept as a
 * private structure so it can be modified without breaking backwards
 * compatibility. As long as this structure is not modified and
 * sizeof(struct NETIO_SharedMemoryHandleImpl) is smaller than the
 * pad compatibility is preserved
 */
struct NETIO_SharedMemorySemMutexHandle
{
    union
    {
        struct NETIO_SharedMemorySemMutexHandleImpl handle;
        RTI_UINT64 pad[4];
    } impl;
};

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
 * \brief Create a shared memory mutex.
 * 
 * @param handle out Pointer to handle on successful return
 * @param status_out \b out. Optional: if not NULL is set to the reason
 * why the call has succeeded or failed. In particular:
 * <ul>
 * <li><b>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</b> =
 * precondition check failed.
 * <li><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B>=
 * this error should never happen and is caused by a failed create()
 * followed by a failed attach().
 * <li><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 * the semaphore has been successfully created.
 * </ul>
 * @param key \b in. Key to identify the shared memory semaphore
 * across processes.
 * 
 * @return RTI_TRUE on success. RTI_FALSE on error.
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_create(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
 * \brief Attach to an existing shared memory binary semaphore with the provided
 * key and sets up the handle to access it.
 *
 * @pre Handle in the DETACHED state. Signaling semaphore in the
 * CREATED state. Some other process has called
 * \ref NETIO_SharedMemoryMutex_create using the same
 * key in the shared memory semaphore
 *
 * @post handle in the ATTACHED state, Semaphore exists and is ready to
 * be used.
 *
 * @param handle \b out. The shared memory semaphore handle to
 * initialize.
 *
 * @param status_out \b out.  Optional. If not NULL, will be set to:
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
 * @param key \b in. Key to identify the shared memory semaphore
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
 * @see NETIO_SharedMemoryMutex_detach
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_attach(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
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
 * @param handle \b inOut. The shared memory semaphore handle to
 * initialize.
 *
 * @param status_out \b out. Optional: if not NULL is set to the reason
 * why the call has succeeded or failed. In particular:
 * <ul>
 * <li><b>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</b> =
 * precondition check failed.
 * <li><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 * an error occurred in the os-specific shared memory implementation
 * (see error log for details).
 * <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B>=
 * this error should never happen and is caused by a failed create()
 * followed by a failed attach().
 * <li><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 * the semaphore has been successfully created.
 * </ul>
 *
 * @param key \b in. Key to identify the shared memory binary semaphore
 * across processes.
 *
 * @return RTI_TRUE on success. RTI_FALSE on error.
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_create_or_attach(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
 * \brief Give the semaphore
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post Semaphore count is set to 1 and any thread blocked on this
 * semaphore is awaken.
 *
 * @param handle \b in. Handle to the semaphore. Must match the one
 * returned by \ref NETIO_SharedMemoryMutex_create
 *
 * @param status_out \b out. Optional: if not NULL, will be set to:
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
 * \ref NETIO_SharedMemoryMutex_detach.
 *
 * @see NETIO_SharedMemorySignalingSemaphore_wait
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_unlock(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
 * \brief Take the semaphore
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post Semaphore count is decremented.
 *
 * @param handle \b in. Handle to the semaphore. Must match the one
 * returned by \ref NETIO_SharedMemoryMutex_create
 *
 * @param status_out \b out. Optional: if not NULL, will be set to:
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
 * particular \ref NETIO_SharedMemoryMutex_detach should not
 * be called either.
 *
 * @see NETIO_SharedMemorySignalingSemaphore_signal
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_lock(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
 * \brief Detach from the shared memory binary semaphore.
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post handle in the DETACHED state.
 *
 * @param handle \b in handle. Handle to the shared memory semaphore
 * obtained from \ref NETIO_SharedMemoryMutex_attach
 *
 * @see NETIO_SharedMemoryMutex_attach
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_detach(struct NETIO_SharedMemoryMutexHandle *handle);

/*e
 * \ingroup NETIO_SharedMemoryMutexClass
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
 * @param handle \b in handle. Handle to the shared memory semaphore
 * obtained from \ref NETIO_SharedMemoryMutex_create
 *
 * @see NETIO_SharedMemoryMutex_create
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemoryMutex_delete(struct NETIO_SharedMemoryMutexHandle *handle);

/*i
 * \ingroup NETIO_SharedMemoryMutexClass
 *
 * \brief Check if the platform is using a robust mutex implementation
 *        for shared memory mutexes.
 *
 * \details It is necessary for the PIL to know if the platform is using
 *          a robust mutex implementation for shared memory mutexes because
 *          it is incompatible with the SemMutex implementation.
 *
 * \return RTI_TRUE if the platform is using a robust mutex implementation
 *         for shared memory mutexes, RTI_FALSE otherwise.
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemoryMutex_is_robust(void);

/* implementation APIs */

struct NETIO_SharedMemoryMutexHandleImpl;
struct NETIO_SharedMemorySemMutexHandleImpl;

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_create function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type);

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_attach function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type);

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_give function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type);

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_take function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type);

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_detach function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type);

/*i
 * Definition of the platform specific NETIO_SharedMemorySemMutex_delete function
 */
NETIOPSLDllExport RTIBool
NETIO_SharedMemorySemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type);

#endif /* netio_shmem_mutex */
