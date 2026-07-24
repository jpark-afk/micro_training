/*
 * FILE: NETIOZCOPYSharedMemPool.c - Relocatable memory pool -- implementation
 *
 * (c) Copyright 2022-2023 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \addtogroup NETIOZCOPYSharedMemPoolClass
 * @{
 */

/* Interface */
#include "NETIOZCOPYSharedMemPool.h"

/* Implementation */
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "netio_zcopy/netio_zcopy_log.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"

/*i
 * \note In the case where all compilers in question support C11, the _Alignof
 *       operator could be used to determine RTI_ALIGN_MAX, as shown below:
 *
 *       typedef long double max_align_t;
 *       RTI_PRIVATE const RTI_SIZE_T RTI_ALIGN_MAX = _Alignof(max_align_t);
 *
 * \note Until C11 support can be guaranteed, we will use a hard-coded value of 16.
 */
RTI_PRIVATE const RTI_SIZE_T RTI_ALIGN_MAX = 16;

/* Functions and types to be implemented by OSAPI */

/*i
 * \brief A constant representing a process ID (PID).
 */
#define SQ_PID_LENGTH (16)

/*i
 * \brief A struct representing a process ID (PID).
 */
struct SQ_PID
{
    /*i
     * \brief The value of the process ID (PID).
     */
    unsigned char value[SQ_PID_LENGTH];
};

/*i
 * \brief An invalid/unused PID indicator
 */
static const struct SQ_PID SQ_PID_NONE = {0};


/*** SOURCE_BEGIN ***/

/*i
 * \brief Checks if a given process ID (PID) is invalid/unused.
 *
 * \param[in] pid A pointer to the process ID (PID) to check.
 *
 * \return RTI_TRUE if the given PID is invalid/unused, RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
pid_is_none(const struct SQ_PID *pid)
{
    return (OSAPI_Memory_compare(pid->value, &SQ_PID_NONE, sizeof(pid->value)) == 0);
}

/*i
 * \brief Gets the process ID (PID).
 *
 * \param[inout] pid_inout A pointer to the process ID (PID) to get.
 *
 * \return RTI_TRUE if the PID was successfully retrieved, RTI_FALSE otherwise.
 *
 * \note Dummy function for now, needs to be implemented in OSAPI
 */
RTI_PRIVATE RTI_BOOL
pid_get(struct SQ_PID *pid_inout)
{
    RTI_BOOL result = RTI_FALSE;
    OSAPI_ProcessId pid_os;
    struct SQ_PID pid_sq = SQ_PID_NONE;
    RTI_SIZE_T pid_size =
        (sizeof(pid_os) < sizeof(pid_sq.value) ? sizeof(pid_os) : sizeof(pid_sq.value));

    if (pid_inout == NULL)
    {
        goto done;
    }

    pid_os = OSAPI_Process_getpid();
    OSAPI_Memory_copy(pid_sq.value, &pid_os, pid_size);
    *pid_inout = pid_sq;

    result = RTI_TRUE;

done:
    return result;
}

/* Relocatable Memory Pool */

/*i
 * \brief A default properties of a shared memory pool.
 * NOTE: This should not be used directly by the user. Instead they should call
 *       NETIO_ZCOPY_SharedMemPool_get_default_properties. 
 */
const struct NETIO_ZCOPY_SharedMemPoolProperties
        NETIO_ZCOPY_SharedMemPoolProperties_DEFAULT = {
                .max_segment_size = NETIO_ZCOPY_SharedMemPool_max_segment_size_auto,
                .max_segment_count = 20,
                .max_consistent_obsrv_count = 32,
                .robust_locking = RTI_FALSE
};

void 
NETIO_ZCOPY_SharedMemPool_get_default_properties(struct NETIO_ZCOPY_SharedMemPoolProperties *properties_out)
{
    *properties_out = NETIO_ZCOPY_SharedMemPoolProperties_DEFAULT;
    properties_out->robust_locking = OSAPI_SharedMemory_is_robust_mutex_supported();
}



/* If the type name starts with with PI (for Position Independent),
 * it can (and will) be stored in shared memory. */

#define SHAREDMEM_ALIGNSIZE RTI_ALIGN_MAX

/*i
 * \brief Aligns a size up to the next multiple of a given alignment.
 *
 * \param[in] size The size to align.
 * \param[in] alignment The alignment to use.
 *
 * \return The aligned size.
 */
RTI_PRIVATE SQ_Length
pi_align_size_up(SQ_Length size, SQ_Length alignment)
{
    return ((size + (alignment - 1U)) & ~(alignment - 1U));
}

/*i
 * \struct SQ_PISharedMemPoolReference
 * \brief A struct representing a reference to an segment in a shared memory
 *        pool.
 */
typedef struct SQ_PISharedMemPoolReference
{
    SQ_Index segment_index;
    SQ_Length offset;
} SQ_PISharedMemPoolReference;

/*i
 * \struct SQ_PISharedMemPoolAdminImpl
 * \brief Relocatable version of the pool admin
 */
typedef struct SQ_PISharedMemPoolAdminImpl
{
    /*i
     * \brief The version info of the pool admin.
     */
    struct NETIO_ZCOPY_PIVersionInfo version_info;
    /*i
     * \brief The size of the user admin in bytes.
     */
    SQ_Length user_admin_size;
    /*i
     * \brief The offset of the user admin.
     */
    SQ_Length user_admin_offset;
    /*i
     * \brief The number of segments in the pool.
     */
    SQ_Index elmt_seg_count;
    /*i
     * \brief The size of each element in bytes.
     */
    SQ_Length elmt_size;
    /*i
     * \brief The count of elements in the pool.
     */
    SQ_Index elmt_count;
    /*i
     * \brief The offset of the element references.
     */
    SQ_Length elmt_refs_offset;
    /*i
     * \brief Whether the pool uses robust locking.
     */
    RTI_BOOL robust_locking;
    /*i
     * \brief The count of consistent observers.
     *
     * \note Contains count if robust_locking is RTI_TRUE, SQ_INDEX_NONE
     *       otherwise.
     */
    SQ_Index obsrv_count;
    /*i
     * \brief The array of consistent observers.
     */
    struct SQ_PID observers[/*robust_slots_count*/];
} SQ_PISharedMemPoolAdmin;

/*i
 * \brief Gets a pointer to the element reference at a given index.
 *
 * \param[in] admin A pointer to the pool admin.
 * \param[in] index The index of the element reference to get.
 *
 * \return A pointer to the element reference at the given index.
 */
RTI_PRIVATE SQ_PISharedMemPoolReference *
NETIO_ZCOPY_SharedMemPool_get_element_reference(SQ_PISharedMemPoolAdmin *admin, SQ_Index index)
{
    SQ_Length offset = admin->elmt_refs_offset +
                       index * sizeof(SQ_PISharedMemPoolReference);
    /* Casting to a wider type is safe here because we own all memory and
     * calculations
     */
    return (SQ_PISharedMemPoolReference *)&((unsigned char *)admin)[offset];
}

/*ci
 * \struct NETIO_ZCOPY_SharedMemPoolImpl
 * \brief A struct representing a shared memory pool.
 */
struct NETIO_ZCOPY_SharedMemPoolImpl
{
    /*i
     * \brief The handle to the shared memory segment containing the pool admin.
     */
    struct OSAPI_SharedMemorySegmentHandle *admin_seg_handle;
    /*i
     * \brief The index of this observer in the observers array.
     *
     * \note Relevant for consistent observers only
     */
    SQ_Index my_obsrv_index;
    /*i
     * \brief The count of element segments actually in use.
     */
    SQ_Index elmt_seg_handles_count;
    /*i
     * \brief The maximum allowed count of element segments.
     */
    SQ_Index max_elmt_seg_handles_count;
    /*i
     * \brief The array of size max_elmt_seg_handles_count, holding the
     *        handles to the element segments.
     */
    struct OSAPI_SharedMemorySegmentHandle **elmt_seg_handles;
    /*i
     * \brief The global lock associates with this shared memory pool,
     *        determining whether this memory pool has been claimed by an admin.
     * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
     * \sa \ref NETIO_ZCOPY_SharedMemPool_release
     */
    RTI_BOOL have_admin_claim;
    /*i
     * \brief Whether this memory pool is the owner of the shared memory.
     */
    RTI_BOOL am_owner;
};

#define SQ_SUFFIX_LEN  (3)
#define SQ_SUFFIX_MAX_VALUE  (0xFFFFFF)
/*i
 * \brief Creates a suffixed name from a given name and suffix value.
 *
 * \param[in] name The name to create suffix from.
 * \param[in] suffix_value The suffix value to use.
 * \param[in] suffixed_name_size The size of the suffixed name buffer.
 * \param[inout] suffixed_name_inout The suffixed name buffer.
 *
 * \return RTI_TRUE if the suffixed name was successfully created, RTI_FALSE
 *         otherwise.
 */
RTI_PRIVATE RTI_BOOL
make_suffixed_name(
        const char *name,
        SQ_Index suffix_value,
        RTI_UINT8 suffixed_name_size,
        char *suffixed_name_inout)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_SIZE_T name_len;
    unsigned char suffix[SQ_SUFFIX_LEN];
    RTI_UINT32 i;

    name_len = OSAPI_String_length_w_max(name, suffixed_name_size);
    if ((name_len == 0) || (name_len == suffixed_name_size) ||
        (suffix_value > SQ_SUFFIX_MAX_VALUE))
    {
        goto done;
    }

    /* Convert suffix_value to bytes */
    for (i = 0; i < SQ_SUFFIX_LEN; ++i)
    {
        suffix[SQ_SUFFIX_LEN - 1 - i] = (unsigned char)(suffix_value & 0xFF);
        suffix_value >>= 8;
    }

    /* Copy base name into output */
    OSAPI_Memory_zero(suffixed_name_inout, suffixed_name_size);
    OSAPI_Memory_copy(suffixed_name_inout, name, name_len);

    /* Append suffix */
    if (!ZCOPY_Guid_bytes_to_string(
                suffix,
                SQ_SUFFIX_LEN,
                suffixed_name_inout + name_len,
                suffixed_name_size - name_len))
    {
        goto done;
    }
    result = RTI_TRUE;

done:
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedMemPool_create(
        const char *name,
        const struct NETIO_ZCOPY_PIVersionInfo *version_info,
        SQ_Length admin_size,
        SQ_Index elmt_count,
        SQ_Length elmt_size,
        const struct NETIO_ZCOPY_SharedMemPoolProperties *properties,
        NETIO_ZCOPY_SharedMemPool **mem_pool_out)
{
    RTI_BOOL result = RTI_FALSE;
    NETIO_ZCOPY_SharedMemPool *mem_pool = NULL;
    const struct NETIO_ZCOPY_SharedMemPoolProperties *props;
    SQ_PISharedMemPoolAdmin *pi_admin_ptr;
    RTI_BOOL with_consistent_observers;
    SQ_Length max_admin_seg_size;
    SQ_Length max_elmt_seg_size;
    SQ_Length elmt_size_aligned;
    SQ_Length max_elmts_per_seg;
    SQ_Length admin_size_aligned;
    SQ_Length total_admin_size;
    SQ_Length elmt_refs_offset;
    SQ_Length admin_offset;
    SQ_Index elmts_left = SQ_INDEX_NONE;
    SQ_Index elmt_seg_handles_count = SQ_INDEX_NONE;
    struct OSAPI_SharedMemorySegmentHandle *admin_seg_handle = NULL;
    struct OSAPI_SharedMemorySegmentHandle **elmt_seg_handles = NULL;
    struct NETIO_ZCOPY_SharedMemPoolProperties default_props;
    RTI_BOOL admin_seg_locked = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            name == NULL,
            goto done,
            OSAPI_Log_entry_add_string("name", name, RTI_TRUE);)
    OSAPI_PRECONDITION(
            (properties == NULL) || (mem_pool_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("properties", properties, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("mem_pool_out", mem_pool_out, RTI_TRUE);)

    /* Check validity of count and size */
    if ((elmt_count == 0) || (elmt_size == 0))
    {
        goto done;
    }

    /* Normalize input */
    if (properties == NULL)
    {
        NETIO_ZCOPY_SharedMemPool_get_default_properties(&default_props);
        props = &default_props;
    }
    else
    {
        props = properties;
    }

    admin_size_aligned = pi_align_size_up(admin_size, SHAREDMEM_ALIGNSIZE);
    elmt_size_aligned = pi_align_size_up(elmt_size, SHAREDMEM_ALIGNSIZE);
    with_consistent_observers = (props->max_consistent_obsrv_count != 0) &&
                                (props->max_consistent_obsrv_count != SQ_INDEX_NONE);

    /* Calculate max sizes allowed in shared mem */
    if (NETIO_ZCOPY_SharedMemPool_max_segment_size_auto != props->max_segment_size)
    {
        max_admin_seg_size = props->max_segment_size;
        max_elmt_seg_size = props->max_segment_size;
    }
    else
    {
        max_admin_seg_size = OSAPI_SharedMemorySegment_get_max_size();
        max_elmt_seg_size = OSAPI_SharedMemorySegment_get_max_size();
    }

    /* Calculate the offset where the array of references starts */
    elmt_refs_offset = 0;
    if (with_consistent_observers)
    {
        elmt_refs_offset += props->max_consistent_obsrv_count * sizeof(struct SQ_PID);
    }
    elmt_refs_offset = pi_align_size_up(
            elmt_refs_offset + sizeof(SQ_PISharedMemPoolAdmin),
            SHAREDMEM_ALIGNSIZE);

    /* After the references is where the admin resides */
    admin_offset = pi_align_size_up(
            elmt_refs_offset + elmt_count * sizeof(SQ_PISharedMemPoolReference),
            SHAREDMEM_ALIGNSIZE);

    /* Check to see how much space is left and whether we exceed any of the
     * limits
     */
    if ((max_admin_seg_size < admin_offset) ||
        (max_admin_seg_size - admin_offset < admin_size_aligned) ||
        (max_elmt_seg_size < elmt_size_aligned))
    {
        goto done;
    }

    /* Now we know it will all fit so we can add without the risk of overflow */
    total_admin_size = admin_offset + admin_size_aligned;

    /* Calculate how many elements fit in a segment and the number of
     * segments needed
     */
    /* elmt_size_aligned is set with pi_align_size_up which can never return
     * 0 in this function.
     */
    /* coverity[divide_by_zero : FALSE] */
    max_elmts_per_seg = max_elmt_seg_size / elmt_size_aligned;
    elmt_seg_handles_count = (SQ_Index)(1 + (elmt_count - 1) / max_elmts_per_seg);
    if (elmt_seg_handles_count > props->max_segment_count)
    {
        goto done;
    }

    /* Allocate array to hold handles for all segments */
    OSAPI_Heap_allocate_array(
            &elmt_seg_handles,
            elmt_seg_handles_count,
            struct OSAPI_SharedMemorySegmentHandle *);
    if (elmt_seg_handles == NULL)
    {
        goto done;
    }

    admin_seg_handle = OSAPI_SharedMemorySegmentHandle_new(
            props->robust_locking ? OSAPI_SHMEM_MODE_ROBUST : OSAPI_SHMEM_MODE_LOCKABLE);
    if (admin_seg_handle == NULL)
    {
        goto done;
    }

    /* Create shared memory segment for pool admin */
    if (!OSAPI_SharedMemorySegment_create(admin_seg_handle, name, total_admin_size))
    {
        goto done;
    }
    admin_seg_locked = RTI_TRUE;

    pi_admin_ptr = OSAPI_SharedMemorySegment_get_address(admin_seg_handle);
    if (pi_admin_ptr == NULL)
    {
        goto done;
    }

    /* Initialize base admin */
    *pi_admin_ptr = (SQ_PISharedMemPoolAdmin)
    {
            .version_info = *version_info,
            .user_admin_size = admin_size,
            .user_admin_offset = admin_offset,
            .elmt_seg_count = elmt_seg_handles_count,
            .elmt_size = elmt_size,
            .elmt_count = elmt_count,
            .elmt_refs_offset = elmt_refs_offset,
            .robust_locking = props->robust_locking,
            .obsrv_count = with_consistent_observers ? props->max_consistent_obsrv_count : 0,
    };

    /* Initialize observer slots */
    for (SQ_Index i = 0; i < pi_admin_ptr->obsrv_count; i++)
    {
        pi_admin_ptr->observers[i] = SQ_PID_NONE;
    }

    /* Create all segments as needed */
    elmts_left = elmt_count;
    for (SQ_Index i = 0; i < elmt_seg_handles_count; i++)
    {
        char elmt_seg_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
        SQ_Length elmt_offset;
        SQ_Length elmts_in_seg_count;

        /* Create name for this segment */
        if (!make_suffixed_name(name, i, sizeof(elmt_seg_name), &elmt_seg_name[0]))
        {
            goto done;
        }

        /* Create segment of the right size */
        elmts_in_seg_count =
                (elmts_left < max_elmts_per_seg ? elmts_left : max_elmts_per_seg);
        elmt_seg_handles[i] = OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE_WRITE);
        if (elmt_seg_handles[i] == NULL)
        {
            goto done;
        }
        if (!OSAPI_SharedMemorySegment_create(
                    elmt_seg_handles[i],
                    &elmt_seg_name[0],
                    elmts_in_seg_count * elmt_size_aligned))
        {
            goto done;
        }

        /* Maintain references to elements in this segment */
        elmt_offset = 0;
        for (SQ_Index j = 0; j < elmts_in_seg_count; j++)
        {
            *NETIO_ZCOPY_SharedMemPool_get_element_reference(pi_admin_ptr, elmt_count - elmts_left) =
                    (SQ_PISharedMemPoolReference) {
                            .offset = elmt_offset,
                            .segment_index = i,
                    };
            elmt_offset += elmt_size_aligned;
            elmts_left--;
        }
    }

    /* Create pool admin object on the heap */
    OSAPI_Heap_allocate_struct(&mem_pool, struct NETIO_ZCOPY_SharedMemPoolImpl);
    if (mem_pool == NULL)
    {
        goto done;
    }

    *mem_pool = (NETIO_ZCOPY_SharedMemPool)
    {
            .admin_seg_handle = admin_seg_handle,
            .my_obsrv_index = SQ_INDEX_NONE,
            .max_elmt_seg_handles_count = elmt_seg_handles_count,
            .elmt_seg_handles_count = elmt_seg_handles_count,
            .elmt_seg_handles = elmt_seg_handles,
            .have_admin_claim = RTI_FALSE,
            .am_owner = RTI_TRUE,
    };

    *mem_pool_out = mem_pool;
    result = RTI_TRUE;

done:
    if (admin_seg_locked)
    {
        RTI_BOOL retval = OSAPI_SharedMemorySegment_unlock(admin_seg_handle);
        IGNORE_RETVAL(retval);
    }

#ifndef RTI_CERT
    if (!result)
    {
        if (admin_seg_handle != NULL)
        {
            OSAPI_SharedMemorySegmentHandle_delete(admin_seg_handle);
        }
        if (elmt_seg_handles != NULL)
        {
            for (SQ_Index i = 0; i < elmt_seg_handles_count; i++)
            {
                if (elmt_seg_handles[i] != NULL)
                {
                    OSAPI_SharedMemorySegmentHandle_delete(elmt_seg_handles[i]);
                }
            }
            OSAPI_Heap_free_array(elmt_seg_handles);
        }
        if (mem_pool != NULL)
        {
            OSAPI_Heap_free_struct(mem_pool);
        }
    }
#endif
    /* The memory allocated in this function is leaked only when compiling
     * with RTI_CERT because memory is intentionally never freed for cert.
     */
    /* coverity[leaked_storage] */
    return result;
}

void
NETIO_ZCOPY_SharedMemPool_reset(NETIO_ZCOPY_SharedMemPool *self)
{
    struct NETIO_ZCOPY_SharedMemPoolProperties observer_props;
    NETIO_ZCOPY_SharedMemPool_get_default_properties(&observer_props);
    
    if (self != NULL)
    {
        self->my_obsrv_index = SQ_INDEX_NONE;
        self->max_elmt_seg_handles_count = observer_props.max_segment_count;
        self->elmt_seg_handles_count = SQ_INDEX_NONE;
        self->have_admin_claim = RTI_FALSE;
        self->am_owner = RTI_FALSE;
        self->admin_seg_handle->ptr_header = NULL;
        self->admin_seg_handle->ptr_user_data = NULL;
        for (SQ_Index i = 0; i < observer_props.max_segment_count; i++)
        {
            self->elmt_seg_handles[i]->ptr_header = NULL;
            self->elmt_seg_handles[i]->ptr_user_data = NULL;
        }
    }
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_destroy(NETIO_ZCOPY_SharedMemPool *self)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL had_error = RTI_FALSE;

    if (self == NULL)
    {
        result = RTI_TRUE;
        goto done;
    }

    if  (self->have_admin_claim)
    {
        goto done;
    }

    for (SQ_Index i = 0; i < self->max_elmt_seg_handles_count; i++)
    {
        /* If we own the memory we need to go ahead and unlink the segment */
        if (self->am_owner)
        {
            had_error |= !OSAPI_SharedMemorySegment_delete(self->elmt_seg_handles[i]);
        }

#ifndef RTI_CERT
        had_error |= !OSAPI_SharedMemorySegmentHandle_delete(self->elmt_seg_handles[i]);
#endif /* !RTI_CERT */
    }

    if (self->am_owner)
    {
        had_error |= !OSAPI_SharedMemorySegment_delete(self->admin_seg_handle);
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_array(self->elmt_seg_handles);
    had_error |= !OSAPI_SharedMemorySegmentHandle_delete(self->admin_seg_handle);
    OSAPI_Heap_free_struct(self);
#endif /* !RTI_CERT */

    result = !had_error;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_initialize(
        SQ_ConsistencyMode mode,
        NETIO_ZCOPY_SharedMemPool **mem_pool_out)
{
    RTI_BOOL result = RTI_FALSE;
    NETIO_ZCOPY_SharedMemPool *mem_pool = NULL;
    struct NETIO_ZCOPY_SharedMemPoolProperties observer_props;
    struct OSAPI_SharedMemorySegmentHandle *admin_seg_handle = NULL;
    struct OSAPI_SharedMemorySegmentHandle **elmt_seg_handles = NULL;

    OSAPI_PRECONDITION(
            (mem_pool_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("mem_pool_out", mem_pool_out, RTI_TRUE);)

    if (mode == SQ_CSTY_MODE_NONE)
    {
        goto done;
    }

    NETIO_ZCOPY_SharedMemPool_get_default_properties(&observer_props);

    admin_seg_handle = OSAPI_SharedMemorySegmentHandle_new(
            (mode == SQ_CSTY_MODE_ROBUST) ? OSAPI_SHMEM_MODE_ROBUST
                                          : OSAPI_SHMEM_MODE_LOCKABLE);
    if (admin_seg_handle == NULL)
    {
        goto done;
    }

    /* The elmt_seg_handles array will hold pointers to valid handles for all
     * segments created by the owner of the SharedMemPool, but the exact number
     * of handles can only be known *after* attaching to an existing
     * SharedMemPool. Because we need to allocate memory now (at initialization
     * time) and the segment count isn't know until after DDS discovery
     * completes (at run time), we will use the default maximum number of
     * handles any writer can support.
     */
    OSAPI_Heap_allocate_array(
            &elmt_seg_handles,
            observer_props.max_segment_count,
            struct OSAPI_SharedMemorySegmentHandle *);
    if (elmt_seg_handles == NULL)
    {
        goto done;
    }
    for (SQ_Index i = 0; i < observer_props.max_segment_count; i++)
    {
        elmt_seg_handles[i] =
                OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE_READ);
        if (elmt_seg_handles[i] == NULL)
        {
            goto done;
        }
    }

    /* Create pool admin object on the heap */
    OSAPI_Heap_allocate_struct(&mem_pool, NETIO_ZCOPY_SharedMemPool);
    if (mem_pool == NULL)
    {
        goto done;
    }

    *mem_pool = (NETIO_ZCOPY_SharedMemPool)
    {
        .admin_seg_handle = admin_seg_handle,
        .my_obsrv_index = SQ_INDEX_NONE,
        .max_elmt_seg_handles_count = observer_props.max_segment_count,
        .elmt_seg_handles_count = SQ_INDEX_NONE,
        .elmt_seg_handles = elmt_seg_handles,
        .have_admin_claim = RTI_FALSE,
        .am_owner = RTI_FALSE,
    };

    *mem_pool_out = mem_pool;
    result = RTI_TRUE;

done:
    /* MICRO-5236 */
    /* coverity[leaked_storage] */
    return result;
}

RTI_PRIVATE RTI_BOOL
NETIO_ZCOPY_SharedMemPool_claim_observer_index(
    NETIO_ZCOPY_SharedMemPool *self,
    SQ_Index *claimed_index_out)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL slot_found = RTI_FALSE;
    OSAPI_SHMEM_STATUS shmem_status;
    SQ_Index claimed_index = SQ_INDEX_NONE;
    RTI_BOOL admin_seg_locked = RTI_FALSE;

    /* If this observer needs to be strongly consistent, find a slot for its PID */
    if ((self->admin_seg_handle->mode == OSAPI_SHMEM_MODE_LOCKABLE) ||
            (self->admin_seg_handle->mode == OSAPI_SHMEM_MODE_ROBUST))
    {
        SQ_PISharedMemPoolAdmin *pi_admin_ptr;

        if (!OSAPI_SharedMemorySegment_lock(self->admin_seg_handle, &shmem_status) ||
            (shmem_status == OSAPI_SHMEM_STATUS_OWNER_DEAD))
        {
            /* Currently there is no way to recover from a process dying while
            *   holding a lock. In stead of trying something, just return
            *   with failure and let others get stuck on the lock...
            */
           goto done;
        }
        admin_seg_locked = RTI_TRUE;

        pi_admin_ptr = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
        if (pi_admin_ptr == NULL)
        {
            goto done;
        }

        for (SQ_Index i = 0; (i < pi_admin_ptr->obsrv_count) && !slot_found; i++)
        {
            if (pid_is_none(&pi_admin_ptr->observers[i]))
            {
                /* Set the pid in the found slot to ours*/
                if (!pid_get(&pi_admin_ptr->observers[i]))
                {
                    break;
                }
                claimed_index = i;
                slot_found = RTI_TRUE;
            }
        }

        if (!slot_found)
        {
            goto done;
        }
    }

    *claimed_index_out = claimed_index;
    result = RTI_TRUE;
done:
    if (admin_seg_locked)
    {
        if (!OSAPI_SharedMemorySegment_unlock(self->admin_seg_handle))
        {
            result = RTI_FALSE;
        }
    }
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_open(
        NETIO_ZCOPY_SharedMemPool *self,
        const char *name,
        RTI_BOOL *is_connected_out,
        SQ_Index *elmt_count_out,
        SQ_Length *elmt_size_out,
        SQ_Index *obsrv_count_out)
{
    RTI_BOOL result = RTI_FALSE;
    OSAPI_SHMEM_ATTACH_STATUS shmem_status;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (name == NULL) || (name[0] == '\0'),
            goto done,
            OSAPI_Log_entry_add_string("name", name, RTI_TRUE);)
    OSAPI_PRECONDITION(
            (is_connected_out == NULL) ||
                (elmt_count_out == NULL) || (elmt_size_out == NULL) ||
                (obsrv_count_out == NULL) || (self == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("is_connected_out", is_connected_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("elmt_count_out", elmt_count_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("elmt_size_out", elmt_size_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("obsrv_count_out", obsrv_count_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* Attach to shared memory segment for pool admin */
    if (!OSAPI_SharedMemorySegment_attach(self->admin_seg_handle, name, &shmem_status))
    {
        goto done;
    }

    /* In this case, the absence of the shared memory segment is acceptable
     *   but will skip the rest of the actions */
    if (shmem_status == OSAPI_SHMEM_ATTACH_STATUS_OK)
    {
        SQ_Index my_obsrv_index;
        SQ_PISharedMemPoolAdmin *pi_admin_ptr;
        SQ_Index elmts_found;
        SQ_Index elmt_segs_opened;
        SQ_Index elmt_seg_handles_count;

        if (!NETIO_ZCOPY_SharedMemPool_claim_observer_index(self, &my_obsrv_index))
        {
            goto done;
        }

        /* Get the memory location of the PI admin and check consistency */
        pi_admin_ptr = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
        if (pi_admin_ptr == NULL)
        {
            goto done;
        }

        /* Create handles to element segments */
        elmt_seg_handles_count = pi_admin_ptr->elmt_seg_count;

        /* Open all segments as needed */
        elmts_found = 0;
        elmt_segs_opened = 0;

        /* Sanitizing pi_admin_ptr->elmt_count not required in this case.
         * Layout and values in shared memory, including the value of pi_admin_ptr->elmt_count ,
         * have been manually determined by the SharedMemPool code (i.e. no automated assignments
         * nor placement by the linker.)
         */

        /* coverity[tainted_scalar] */
        while ((elmts_found < pi_admin_ptr->elmt_count) &&
            (elmt_segs_opened < pi_admin_ptr->elmt_seg_count))
        {
            RTI_BOOL is_new_seg = RTI_FALSE;

            if (elmts_found > 0)
            {
                /* coverity[tainted_scalar] */
                SQ_PISharedMemPoolReference *ref_cur =
                    NETIO_ZCOPY_SharedMemPool_get_element_reference(pi_admin_ptr, elmts_found);
                /* coverity[tainted_scalar] */
                SQ_PISharedMemPoolReference *ref_prev =
                    NETIO_ZCOPY_SharedMemPool_get_element_reference(pi_admin_ptr, elmts_found - 1);
                if (ref_cur->segment_index != ref_prev->segment_index)
                {
                    /* This element resides in a segment with a different index
                     *   than the previous one, so it is a new segment */
                    is_new_seg = RTI_TRUE;
                }
            }
            else
            {
                /* This is the first element, so by definition a new segment */
                is_new_seg = RTI_TRUE;
            }

            if (is_new_seg)
            {
                /* Create name for this segment */
                char elmt_seg_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
                if (!make_suffixed_name(
                            name,
                            elmt_segs_opened,
                            sizeof(elmt_seg_name),
                            &elmt_seg_name[0]))
                {
                    goto done;
                }

                /* Open associated shmem */
                if (!OSAPI_SharedMemorySegment_attach(
                        self->elmt_seg_handles[elmt_segs_opened],
                        &elmt_seg_name[0], &shmem_status) ||
                    (shmem_status != OSAPI_SHMEM_ATTACH_STATUS_OK))
                {
                    /* In this case, non-existence of the shared memory segment
                     *   is fatal, because success to open the main admin segment
                     *   should also imply existence of the data segments*/
                    goto done;
                }

                /* Keep track of the number of segments that was opened */
                elmt_segs_opened++;
            }

            /* Keep track of the number of elements that we have looped over */
            elmts_found++;
        }

        self->my_obsrv_index = my_obsrv_index;
        self->elmt_seg_handles_count = elmt_seg_handles_count;

        *is_connected_out = RTI_TRUE;
        *elmt_count_out = pi_admin_ptr->elmt_count;
        *elmt_size_out = pi_admin_ptr->elmt_size;
        *obsrv_count_out = pi_admin_ptr->obsrv_count;
    }
    else if (shmem_status == OSAPI_SHMEM_ATTACH_STATUS_SEGMENT_NOT_FOUND)
    {
        *is_connected_out = RTI_FALSE;
        *elmt_count_out = 0;
        *elmt_size_out = 0;
        *obsrv_count_out = 0;
    }
    else
    {
        /* Unknown value, defensive programming */
        goto done;
    }

    result = RTI_TRUE;
done:
    /* MICRO-5236 */
    /* coverity[leaked_storage] */
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_close(NETIO_ZCOPY_SharedMemPool *self)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL had_error = RTI_FALSE;

    if (self == NULL)
    {
        result = RTI_TRUE;
        goto done;
    }

    /* This is always called by the reader and am_owner will always be false.
     * This is defensive programming.
     */
    if (self->am_owner || self->have_admin_claim)
    {
        goto done;
    }

    if (self->my_obsrv_index != SQ_INDEX_NONE)
    {
        SQ_PISharedMemPoolAdmin *pi_admin_ptr;

        /* Get the memory location of the PI admin and check consistency */
        pi_admin_ptr = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
        if (pi_admin_ptr == NULL)
        {
            had_error = RTI_TRUE;
        }
        else
        {
            pi_admin_ptr->observers[self->my_obsrv_index] = SQ_PID_NONE;
        }
    }

    /* Only detach from shared memory segments if a previous attach occurred. */
    if (self->elmt_seg_handles_count != SQ_INDEX_NONE)
    {
        for (SQ_Index i = 0; i < self->elmt_seg_handles_count; i++)
        {
            had_error |= !OSAPI_SharedMemorySegment_detach(self->elmt_seg_handles[i]);
        }
        had_error |= !OSAPI_SharedMemorySegment_detach(self->admin_seg_handle);
    }

    result = !had_error;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_claim(NETIO_ZCOPY_SharedMemPool *self)
{
    RTI_BOOL result = RTI_FALSE;
    unsigned char *base_mem = NULL;
    OSAPI_SHMEM_STATUS shm_status;

    /* Preconditions */
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (self->have_admin_claim)
    {
        goto done;
    }

    if (!OSAPI_SharedMemorySegment_lock(self->admin_seg_handle, &shm_status))
    {
        goto done;
    }

    if (shm_status == OSAPI_SHMEM_STATUS_OWNER_DEAD)
    {
        /* We have successfully acquired the lock, but the previous owner died
         * before releasing it. The previous owner may have left the pool in
         * an inconsistent state, so we should not claim it.
         */
        ZCOPY_LOG_SHARED_MEM_POOL_OWNER_DEAD(OSAPI_LOGKIND_ERROR)

        /* Releasing the lock without calling mark_consistent() will cause all
         * subsequent attempts to take the lock to fail so that no one else
         * can observe an inconsistent state.
         */
        OSAPI_SharedMemorySegment_unlock(self->admin_seg_handle);
        goto done;
    }

    base_mem = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
    if (base_mem == NULL)
    {
        goto done;
    }

    self->have_admin_claim = RTI_TRUE;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_release(NETIO_ZCOPY_SharedMemPool *self)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!self->have_admin_claim)
    {
        goto done;
    }

    if (!OSAPI_SharedMemorySegment_unlock(self->admin_seg_handle))
    {
        goto done;
    }
    self->have_admin_claim = RTI_FALSE;
    result = RTI_TRUE;

done:
    return result;
}

/* Getters */

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_admin_info(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Length *user_admin_size_out,
        unsigned char **user_admin_out)
{
    RTI_BOOL result = RTI_FALSE;
    unsigned char *base_mem = NULL;
    SQ_PISharedMemPoolAdmin *pi_pool_admin_ptr = NULL;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (user_admin_size_out == NULL) || (user_admin_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_admin_size_out", user_admin_size_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_admin_out", user_admin_out, RTI_TRUE);)

    base_mem = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
    if (base_mem == NULL)
    {
        goto done;
    }

    /* From admin, get pointer to user admin */
    pi_pool_admin_ptr = (SQ_PISharedMemPoolAdmin *)base_mem;
    *user_admin_size_out = pi_pool_admin_ptr->user_admin_size;

    /* MICRO-5244: The user_admin_offset is considered tainted here because it
     * comes from a shared memory segment. This value is trusted because it was
     * set by this class.
     */
    /* coverity[tainted_data] */
    *user_admin_out = &base_mem[pi_pool_admin_ptr->user_admin_offset];

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_version_info(
        NETIO_ZCOPY_SharedMemPool *self,
        struct NETIO_ZCOPY_PIVersionInfo *version_info_out)
{
    RTI_BOOL result = RTI_FALSE;
    SQ_PISharedMemPoolAdmin *pi_pool_admin_ptr = NULL;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (version_info_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("version_info_out", version_info_out, RTI_TRUE);)

    /* Get pointer to the pool admin. No need to lock, this information is
     * immutable
     */
    pi_pool_admin_ptr = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
    if (pi_pool_admin_ptr == NULL)
    {
        goto done;
    }

    *version_info_out = pi_pool_admin_ptr->version_info;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_element(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Index elmt_index,
        SQ_Length *elmt_size_out,
        unsigned char **elmt_out)
{
    RTI_BOOL result = RTI_FALSE;
    SQ_PISharedMemPoolAdmin *pi_pool_admin_ptr = NULL;
    SQ_PISharedMemPoolReference *elmt_ref;
    SQ_Index seg_index;
    unsigned char *seg_ptr = NULL;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (elmt_size_out == NULL) || (elmt_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("elmt_size_out", elmt_size_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("elmt_out", elmt_out, RTI_TRUE);)

    /* Get pointer to the pool admin. No need to lock, this information is
     * immutable
     */
    pi_pool_admin_ptr = OSAPI_SharedMemorySegment_get_address(self->admin_seg_handle);
    if (pi_pool_admin_ptr == NULL)
    {
        goto done;
    }

    /* From admin, get pointer to the element (if it is within range) */
    if (elmt_index >= pi_pool_admin_ptr->elmt_count)
    {
        goto done;
    }

    /* MICRO-5240: The elmt_refs_offset is considered tainted here because it
     * comes from a shared memory segment. This value is trusted because it was
     * set by this class.
     */
    /* coverity[tainted_data] */
    /* coverity[tainted_data_transitive] */
    elmt_ref = NETIO_ZCOPY_SharedMemPool_get_element_reference(pi_pool_admin_ptr, elmt_index);
    seg_index = elmt_ref->segment_index;
    if (seg_index >= self->elmt_seg_handles_count)
    {
        /* This would be a bug */
        goto done;
    }

    seg_ptr = OSAPI_SharedMemorySegment_get_address(self->elmt_seg_handles[seg_index]);
    if (seg_ptr == NULL)
    {
        goto done;
    }

    *elmt_size_out = pi_pool_admin_ptr->elmt_size;
    *elmt_out = &seg_ptr[elmt_ref->offset];
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_observer_index(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Index *obsrv_index_out)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (obsrv_index_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("obsrv_index_out", obsrv_index_out, RTI_TRUE);)

    if (self->my_obsrv_index == SQ_INDEX_NONE)
    {
        goto done;
    }

    *obsrv_index_out = self->my_obsrv_index;
    result = RTI_TRUE;

done:
    return result;
}

/*i @} */
