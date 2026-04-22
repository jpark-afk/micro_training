/*
 * FILE: netio_shmem_segment.h
 *
 * Copyright 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_shmem_segment_h
#define netio_shmem_segment_h

#include "netio_shmem/netio_shmem_dll.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_process.h"

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * Opaque handle initialized by the attach methods and
 * used to interact with a shared memory segment.
 * This is a Forward Declaration.
 */
struct NETIO_SharedMemorySegmentHeader;

/*e
 * \defgroup NETIO_SharedMemorySegmentClass Shared Memory Segment API
 * \ingroup NETIO_SharedMemoryClass
 * \brief Allow access to a shared memory segment (memory block) so that
 *        multiple processes can read-and write a common memory area.
 */

struct NETIO_SharedMemorySegmentHandle
{
    union
    {
        RTI_INT32 reserved_int[8];
        void *reserver_ptr;
    } reserved;
};

/*i
 * \ingroup NETIO_SharedMemoryClass
 *
 * This are the guts of the shared memory handle. It is kept as a
 * private structure so it can be modified without breaking backwards
 * compatibility.
 *
 * The structure itself can be modified, provided that we don't exceed
 * the size of the padding.
 */
struct NETIO_SharedMemorySegmentHandleImpl
{
    /*i
     * handle to the shared memory.
     * Used as a reference when delete the shared memory.
     */
    NETIO_SharedMemoryNativeHandleSegment native_handle;

    /*i
     * pointer to the first byte of the shared segment. The segment header
     * can be found on top.
     * NULL = unattached
     */
    struct NETIO_SharedMemorySegmentHeader *ptr_header;

    /*i
     * Pointer to the user's data inside the segment.
     * This is equivalent of ptr_header + 
     * sizeof(NETIO_SharedMemorySegmentHeader)
     */
    void *ptr_user_data;
};

#define NETIO_SharedMemorySegment_MACRO_GETSIZE(userDataSize) \
    (userDataSize +                                           \
    (RTI_INT32)sizeof(struct NETIO_SharedMemorySegmentHeader))

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * \brief Attempts to create a new shared memory segment. If the segment
 *        already exists and the creator process is dead, attach and recycle it.
 *
 * @param[in,out] handle The shared memory segment handle to initialize.
 *
 * @param[out] status_out Optional: if not NULL is set to the reason why
 *                        the call has succeeded or failed. In particular,
 * <UL>
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</B> =
 *       precondition check failed.
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 *       an error occurred in the os-specific shared memory implementation
 *       (see error log for details).
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED</B> =
 *       someone is already sitting on this address.
 *   <LI><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 *       the segment has been created new.
 *   <LI><B>OSAPI_SHARED_MEMORY_ATTACHED</B> =
 *       the segment was reused from an existing one.
 *
 * @param[in] key  Key to identify the shared memory segment
 *                   Across processes. Other processes should call
 *                   \ref NETIO_SharedMemorySegment_attach with the same
 *                   key to attach to this segment.
 *
 * @param[in] size Size of memory to be initialized.
 *
 * @param[in] pid_in PID of the process that calls this method.
 *
 * @return RTI_TRUE if success, or RTI_FALSE if something failed.
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySegment_create_or_attach(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_UINT32 size,
        OSAPI_ProcessId pid_in);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 * \brief Creates a new shared memory segment with the provided key
 *         and returns the handle to access it.
 *
 * @param[in,out] handle The shared memory segment handle to initialize.
 * @param[out] status_out Optional: if not NULL is set to the reason why
 * the call has succeeded or failed. In particular,
 * <UL>
 *  <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</B> =
 *      precondition check failed.
 *  <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 *      an error occurred in the os-specific shared memory implementation
 *      (see error log for details).
 *  <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED</B> =
 *      segment already exist (no checks is performed whether the creator
 *      is still alive or not).
 *  <LI><B>OSAPI_SHARED_MEMORY_CREATED</B> =
 *      the segment has been successfully created.
 *
 * @param[in] key \b Key to identify the shared memory segment
 *                   across processes. Other processes should call
 *                   \ref NETIO_SharedMemorySegment_attach with the same
 *                   key to attach to this segment.
 *
 * @param[in] size \b Size of memory to be initialized.
 *
 * @param[in] pid_in \b PID of the process that calls this method.
 *
 * @return RTI_TRUE if success, or RTI_FALSE if something failed.
 *
 * @note Depending on the underlying OS, the key may need to be
 * transformed to the actual representation needed by the operating
 * system
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySegment_create(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_UINT32 size,
        OSAPI_ProcessId pid_in);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * \brief deletes the shared memory segment previously create
 *        with \ref NETIO_SharedMemorySegment_create. If the segment is still
 *        attached, it is detached first.
 *
 * @param[in] handle The shared memory segment handle identifying the
 *                      segment to detach
 *
 * @return RTI_TRUE on success. RTI_FALSE if either the precondition was
 *         not met or the shared memory segment couldn't be removed (i.e.
 *         the user running this app is not the owner or creator).
 *
 * @note This function does not perform any control on the ownership of the
 *       segment. If you attach to a segment, then when you call this function
 *       the segment will be destroyed (if you have enough permission to
 *       complete the operation)
 *
 * @note When you create a segment with \ref NETIO_SharedMemorySegment_create
 *       or with \ref NETIO_SharedMemorySegment_create_or_attach, you can
 *       delete it by calling directly NETIO_SharedMemorySegment_delete
 *       without calling \ref NETIO_SharedMemorySegment_detach.
 *
 * Once called this function (even if it returns RTI_FALSE) you cannot use
 * anymore this shared memory segment.
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySegment_delete(
        struct NETIO_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * \brief Attach to the shared memory previously initialized
 *        with \ref NETIO_SharedMemorySegment_create_or_attach and returns
 *        a handle to it.
 *
 * @param[in,out] handle The shared memory segment handle to initialize.
 *
 * @param[out] status_out Optional. If not NULL, will be set to:
 * <UL>
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION</B> =
 *       precondition check failed.
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN</B> =
 *       an error occurred in the os-specific shared memory implementation
 *       (see error log for details).
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY</B> =
 *       no such a segment.
 *   <LI><B>OSAPI_SHARED_MEMORY_FAIL_REASON_OUT_OF_MEMORY</B> =
 *       out of memory allocating the handle structure.
 *   <LI><B>OSAPI_SHARED_MEMORY_ATTACHED</B> =
 *       segment attached successfully.
 * </UL>
 *
 * @param[in] key Key to identify the shared memory segment across
 *                   processes. Must match that used by other processes calling
 *                   \ref NETIO_SharedMemorySegment_create_or_attach or
 *                   \ref NETIO_SharedMemorySegment_attach().
 *
 * @return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySegment_attach(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * \brief Detach from the shared memory previously created
 *        with \ref NETIO_SharedMemorySegment_attach.
 *
 * @param[in] handle The shared memory segment handle identifying the
 *                      segment to detach
 *
 * @return RTI_TRUE on success. RTI_FALSE if the precondition was not met.
 *
 * The shared memory segment is always detached. Once the precondition is
 * met, no errors will be generated.
 *
 * Once called this function (even if it returns RTI_FALSE) you cannot use
 * anymore this shared memory segment, except for destroying it.
 *
 */
NETIO_SHMEMDllExport RTIBool
NETIO_SharedMemorySegment_detach(
        struct NETIO_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 * \brief Get the address of the shared memory attached by the handle
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post none
 *
 * @param[in] handle Handle to the shared memory segment
 *                   obtained from an
 *                   \ref NETIO_SharedMemorySegment_attach or a
 *                   \ref NETIO_SharedMemorySegment_create_or_attach
 *
 * @return the address that can be used to read and write the memory.
 *         For performance this function never checks pre-conditions. It is the
 *         caller's responsibility to ensure the handle is NON null and
 *         ATTACHED. A NULL handle will cause a seg-fault. An un-attached
 *         handle may give invalid values for the
 *         \ref NETIO_SharedMemorySegment_get_size
 */
NETIO_SHMEMDllExport char*
NETIO_SharedMemorySegment_get_address(
        struct NETIO_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 * \brief Get the size (available for user data) of the shared memory segment.
 *
 * This size will match the one passed to the
 * \ref NETIO_SharedMemorySegment_create_or_attach call
 *
 * @pre Handle in the ATTACHED state.
 *
 * @post none
 *
 * @param[in] handle   Handle to the shared memory segment
 *                     obtained from an \ref NETIO_SharedMemorySegment_attach
 *                     or a \ref NETIO_SharedMemorySegment_create_or_attach
 *                     call.
 *
 * @return the number of bytes that can be read/written stating at the
 *         address provided by \ref NETIO_SharedMemorySegment_get_address.  For
 *         performance this function never checks pre-conditions. It is the
 *         caller's responsibility to ensure the handle is NON null and
 *         ATTACHED. A NULL handle will cause a seg-fault. An un-attached
 *         handle may give invalid values for the size.
 */
NETIOPSLDllExport RTI_INT32
NETIO_SharedMemorySegment_get_size(
        struct NETIO_SharedMemorySegmentHandle *handle);

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 * \brief Maximum allowable size of a shared memory segment
 */
NETIOPSLDllExport RTI_UINT32
NETIO_SharedMemorySegment_get_max_size(void);

NETIOPSLDllExport RTIBool
NETIO_SharedMemorySegment_create_or_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        RTI_INT32 *status_out,
        RTI_INT32 segment_key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in);

NETIOPSLDllExport RTIBool
NETIO_SharedMemorySegment_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key);

NETIOPSLDllExport RTIBool
NETIO_SharedMemorySegment_create_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in);

NETIOPSLDllExport RTIBool
NETIO_SharedMemorySegment_detach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTIBool clear_pid);

NETIOPSLDllExport RTIBool
NETIO_SharedMemorySegment_delete_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl);

#endif /* netio_shmem_segment_h */
