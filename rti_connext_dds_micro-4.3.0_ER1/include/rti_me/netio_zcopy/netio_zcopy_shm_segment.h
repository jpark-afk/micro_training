/*
 * FILE: netio_zcopy_shm_segment.h
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
#ifndef netio_zcopy_shm_segment_h
#define netio_zcopy_shm_segment_h

#include "osapi/osapi_config.h"
#include "netio_zcopy/netio_zcopy_dll.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OSAPI_SHMEM_MAX_NAME_LENGTH 31

#define OSAPI_SHARED_MEMORY_ALIGNMENT (RTI_SIZE_T)16
#define OSAPI_SHARED_MEMORY_ALIGN(_size, _align) \
    (((_size) + ((_align)-1)) & ~((_align)-1))

/*e
 * \defgroup OSAPI_SharedMemorySegmentClass OSAPI_SharedMemorySegment
 * \ingroup OSAPIModule
 * \brief Allow access to a shared memory segment (memory block) so that
 *        multiple processes can read-and write a common memory area.
 */

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * Mode used to create or attach to a shared memory segment. Each
 * mode defines specific capabilities of the segment.
 */
typedef enum
{
    /*e Invalid mode */
    OSAPI_SHMEM_MODE_UNKNOWN,

    /*e  Read permission */
    OSAPI_SHMEM_MODE_READ,

    /*e Write permission, implies \ref OSAPI_SHMEM_MODE_READ */
    OSAPI_SHMEM_MODE_WRITE,

    /*e Locking capability, implies \ref OSAPI_SHMEM_MODE_WRITE */
    OSAPI_SHMEM_MODE_LOCKABLE,

    /*e Robust locking capability, implies \ref OSAPI_SHMEM_MODE_LOCKABLE */
    OSAPI_SHMEM_MODE_ROBUST,
} OSAPI_SHMEM_MODE;

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * Status used to convey the state of a shared memory object.
 */
typedef enum
{
    /*e Success */
    OSAPI_SHMEM_STATUS_OK,

    /*e Previous owner of lock died while holding it */
    OSAPI_SHMEM_STATUS_OWNER_DEAD,
} OSAPI_SHMEM_STATUS;

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * Status used to convey the state of a shared memory object's attachment
 * to a shared memory segment.
 */
typedef enum
{
    /*e Success */
    OSAPI_SHMEM_ATTACH_STATUS_OK,

    /*e Segment was not found when trying to attach */
    OSAPI_SHMEM_ATTACH_STATUS_SEGMENT_NOT_FOUND,
} OSAPI_SHMEM_ATTACH_STATUS;

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * Opaque header at beginning of shared memory segment to share
 * information about the segment between processes.
 */
struct OSAPI_SharedMemorySegmentHeader
{
    /*e Size requested by the caller of create */
    RTI_UINT64 size;

    /*e Mode requested by the caller of create */
    OSAPI_SHMEM_MODE mode;
};

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * Opaque handle initialized by the create and attach methods
 * and used to interact with a shared memory segment.
 */
struct OSAPI_SharedMemorySegmentHandle
{
    /*e Pointer to segment header in shared memory */
    struct OSAPI_SharedMemorySegmentHeader *ptr_header;

    /*e Pointer to where user data can be stored in shared memory */
    void *ptr_user_data;

    /*e Mode requested by the caller of create or attach */
    OSAPI_SHMEM_MODE mode;
};

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Creates a handle to be used to create or attach to a shared memory
 *        segment.
 *
 * \param[in] mode Mode of the created handle.
 *
 * \return Pointer to shared memory segment handle if successful. NULL on failure.
 */
struct OSAPI_SharedMemorySegmentHandle *
OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE mode);

#ifndef RTI_CERT
/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Delete a handle to a shared memory segment. First detach from the
 *        segment if it is still attached.
 *
 * \param[in] handle Handle to be deleted.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
RTI_BOOL
OSAPI_SharedMemorySegmentHandle_delete(struct OSAPI_SharedMemorySegmentHandle *handle);
#endif /* !RTI_CERT */

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Check if a shared memory segment identified by a given name already exists
 *
 * \param[in] id Guid used to identify the notifiee
 *
 * \return RTI_TRUE if the notifiee exists. Otherwise, RTI_FALSE.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_exists(const char *name);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Creates a new shared memory segment with a given name and attach
 *        a handle to it.
 *
 * \param[in,out] handle Handle to be attached to the created segment.
 *
 * \param[in] name Name to identify the shared memory segment
 *                   across processes. Other processes should call
 *                   \ref OSAPI_SharedMemorySegment_attach with the same
 *                   name to attach to this segment.
 *
 * \param[in] size Size of memory to be allocated.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The creation mode of the segment is determined by the mode of the handle. To
 * create a segment, the mode must be >= OSAPI_SHMEM_MODE_WRITE.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_create(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        RTI_UINT64 size);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Delete a shared memory segment previously created
 *        with \ref OSAPI_SharedMemorySegment_create.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                      segment to delete.
 *
 * \return RTI_TRUE on success. RTI_FALSE if either the the shared memory
 *         segment could not be deleted.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this shared memory segment.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_delete(struct OSAPI_SharedMemorySegmentHandle *handle);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Try to attach to the shared memory previously created with
 *        with \ref OSAPI_SharedMemorySegment_create.
 *
 * \param[in,out] handle Handle to be attached to the segment.
 *
 * \param[in] name Name to identify the shared memory segment across
 *                 processes. Must match that used by other process calling
 *                 \ref OSAPI_SharedMemorySegment_create().
 *
 * \param[out] status_out If successful, the attachment status of the shared
 *                        memory segment.
 *
 * \return RTI_TRUE on success. Note that the attachment status may still be
 *         different from OK, so checking the value of status_out is mandatory.
 *         RTI_FALSE if an error occurred.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_attach(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        OSAPI_SHMEM_ATTACH_STATUS *status_out);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief detach the shared memory segment previously attached to
 *        with \ref OSAPI_SharedMemorySegment_attach.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to detach.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this shared memory segment handle.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_detach(struct OSAPI_SharedMemorySegmentHandle *handle);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Lock a shared memory segment.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to lock.
 *
 * \param[out] status_out If successful, the status of the shared memory segment.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * If this function returns status_out=OSAPI_SHMEM_STATUS_OWNER_DEAD, then the
 * contents of the shared memory segment may be in an inconsistent state. This
 * condition can only be detected if the segment was attached with OSAPI_SHMEM_MODE_ROBUST.
 * If the shared memory segment can be restored to a consistent state, then it
 * can be marked as consistent with \ref OSAPI_SharedMemorySegment_mark_consistent.
 * Otherwise, any subsequent attempts to lock the segment will fail.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_lock(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        OSAPI_SHMEM_STATUS *status_out);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Unlock a shared memory segment previously locked with
 *        \ref OSAPI_SharedMemorySegment_lock.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to unlock.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The behavior of this method is undefined if the calling process has not
 * previously acquired the lock with \ref OSAPI_SharedMemorySegment_lock.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_unlock(struct OSAPI_SharedMemorySegmentHandle *handle);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Mark a shared memory segment as consistent. This function can only
 *        be called if a previous call to \ref OSAPI_SharedMemorySegment_lock
 *        returned OSAPI_SHMEM_FAIL_REASON_OWNER_DEAD.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to mark consistent.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_mark_consistent(struct OSAPI_SharedMemorySegmentHandle *handle);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Get address of segment mapped to process memory space for user data.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment.
 *
 * \return void* Pointer to start of user data in segment.
 *
 * The behavior or this function is undefined if the segment is not currently
 * attached.
 */
void *
OSAPI_SharedMemorySegment_get_address(struct OSAPI_SharedMemorySegmentHandle *handle);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Get total amount of space for user data in segment.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The behavior or this function is undefined if the segment is not currently
 * attached.
 */
RTI_UINT64
OSAPI_SharedMemorySegment_get_size(struct OSAPI_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Get maximum size of a shared memory segment on the current platform.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
RTI_UINT64
OSAPI_SharedMemorySegment_get_max_size(void);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Check if the process which created the shared memory segment is still
 *        alive.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment.
 * \param[out] alive If the process is alive.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The behavior or this function is undefined if the segment is not currently
 * attached.
 */
RTI_BOOL
OSAPI_SharedMemorySegment_is_owner_alive(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        RTI_BOOL *alive);

/*i
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Determine if a handle is currently attached.
 *
 * \param[in] handle The handle.
 *
 * \return RTI_TRUE if the handle is currently attached. RTI_FALSE otherwise.
 */
#define OSAPI_SharedMemorySegmentHandle_is_attached(handle_) \
    ((handle_)->ptr_header != NULL)

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Get the amount of memory required to store a
 *        \ref OSAPI_SharedMemorySegmentHandle on the current platform for a
 *        given mode.
 *
 * \param[in] mode The creation or attachment mode of the handle.
 *
 * \return Number of bytes required to store a handle.
 */
NETIOPSLDllExport RTI_SIZE_T
OSAPI_SharedMemorySegmentHandle_get_size(OSAPI_SHMEM_MODE mode);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Get the amount of memory, aligned to the maximum alignment, required
 *        to store a \ref OSAPI_SharedMemorySegmentHeader on the current
 *        platform for a given mode.
 *
 * \param[in] mode The creation mode of the segment containing the header.
 *
 * \return Number of bytes required to store a header.
 *
 * The number of bytes returned by this function shall be aligned to the
 * maximum alignment of the platform.
 */
NETIOPSLDllExport RTI_SIZE_T
OSAPI_SharedMemorySegmentHeader_get_size(OSAPI_SHMEM_MODE mode);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Creates a new shared memory segment with the provided key
 *         and initialize a handle to it.
 *
 * \param[in,out] handle Handle to be initialized.
 *
 * \param[in] name Name to identify the shared memory segment.
 *
 * \param[in] size Size of memory to be allocated.
 *
 * \param[in] mode Mode used to create the shared memory segment. The mode
 *                  determines the capabilities of the segment. The mode
 *                  must be >= OSAPI_SHMEM_MODE_WRITE.
 *
 * \param[out] exists If the segment already exists
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The new shared memory segment shall be initialized with null bytes ('\0').
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_create_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        RTI_UINT64 size,
        OSAPI_SHMEM_MODE mode,
        RTI_BOOL *exists);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Delete a shared memory segment.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                      segment to delete.
 *
 * \return RTI_TRUE on success. RTI_FALSE if either the precondition was
 *         not met or the shared memory segment could not be deleted.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this shared memory segment.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_delete_impl(struct OSAPI_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Attach to the shared memory and initialize a handle to it.
 *
 * \param[in,out] handle Handle to be initialized.
 *
 * \param[in] name Name to identify the shared memory segment.
 *
 * \param[in] mode Mode used to attach the shared memory segment.
 *
 * \param[out] status_out If successful, the attachment status of the shared
 *                        memory segment.
 *
 * \return RTI_TRUE on success. Note that the attachment status may still be
 *         different from OK, so checking the value of status_out is mandatory.
 *         RTI_FALSE if an error occurred.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_attach_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        OSAPI_SHMEM_MODE mode,
        OSAPI_SHMEM_ATTACH_STATUS *status_out);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief detach the shared memory segment.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to detach.
 *
 * \return RTI_TRUE on success. RTI_FALSE if either the precondition was
 *         not met or the shared memory segment could not be detached.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this shared memory segment handle.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_detach_impl(struct OSAPI_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Lock a shared memory segment.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to lock.
 *
 * \param[out] status_out Success or failure reason of acquiring the lock.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_lock_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        OSAPI_SHMEM_STATUS *status_out);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Unlock a shared memory segment previously locked with
 *        \ref OSAPI_SharedMemorySegment_lock_impl.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to unlock.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The behavior of this method is undefined if the calling process has not
 * previously acquired the lock with \ref OSAPI_SharedMemorySegment_lock_impl.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_unlock_impl(struct OSAPI_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Mark a shared memory segment as consistent. This function can only
 *        be called if a previous call to \ref OSAPI_SharedMemorySegment_lock_impl
 *        returned OSAPI_SHMEM_FAIL_REASON_OWNER_DEAD.
 *
 * \param[in] handle The shared memory segment handle identifying the
 *                   segment to mark consistent.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemorySegment_mark_consistent_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle);


/*e
 * \ingroup OSAPI_SharedMemorySegmentClass
 *
 * \brief Checks wether robust mutex is supported or not .
 * \return RTI_TRUE is supported. RTI_FALSE if not supported.
 */
NETIOPSLDllExport RTI_BOOL
OSAPI_SharedMemory_is_robust_mutex_supported(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_shm_segment_h */