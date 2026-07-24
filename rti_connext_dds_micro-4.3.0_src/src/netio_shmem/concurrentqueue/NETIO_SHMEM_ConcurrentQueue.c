/*
 * FILE: NETIO_SHMEM_ConcurrentQueue.c
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_heap.h"
#include "netio_shmem/netio_shmem_log.h"
#include "NETIO_SHMEM_ConcurrentQueue.h"

/*i
 * \file
 * \brief Implementation of the NETIO_SHMEM_ConcurrentQueue
 */

/* This is used to sign the first four bytes of the memory such that
 * we can identify it as a queue and we also know the version
 */
/*i CONCURRENT_QUEUE_FIRST_BYTE should never be changed */
#define CONCURRENT_QUEUE_FIRST_BYTE  'C'

/*i CONCURRENT_QUEUE_SECOND_BYTE should never be changed */
#define CONCURRENT_QUEUE_SECOND_BYTE 'Q'

/*i DELETED_CONCURRENT_QUEUE_FIRST_BYTE should never be changed */
#define DELETED_CONCURRENT_QUEUE_FIRST_BYTE  'D'

/*i DELETED_CONCURRENT_QUEUE_SECOND_BYTE  should never be changed */
#define DELETED_CONCURRENT_QUEUE_SECOND_BYTE 'Q'

/*i
 * \ingroup NETIO_SHMEM_ConcurrentQueue
 *
 * An internal structure to determine the alignment requirements
 * of an architecture.
 *
 * The basic idea is that the sizeof() operation needs to return a size
 * large enough that successive structures of this type can be placed
 * sizeof() apart from each other and will preserve the alignment
 * requirements of all members. This is so because pointer arithmetic
 * must work on arrays of such structures.
 * By including in the union the largest primitive data types that we
 * find inside the NETIO_SHMEM_ConcurrentQueue we exercise the alignment limit and we
 * can therefore determine the alignment requirement.
 *
 * Since we do no insert double values or long long values we do not
 * need to take those into consideration. If we did then we would have
 * to include those here.
 *
 * Important:
 * We are taking out the pointers because
 * we were getting a precondition error when running 32-bit and
 * 64-bit applications on the same host.
 *
 * For some reason, when Windows and Linux (at least) provide you the address of
 * a shared memory segment they do it with the original alignment that the segment
 * got when it was created.
 *
 * The problem is that if the shared memory segment was created by the 32-bit
 * application the 64-bit application will get an address aligned to 4 but
 * not necessarily to 8. This was triggering a precondition problem
 * since the alignment of NETIO_SHMEM_ConcurrentQueueStructToDetermineAlignment is
 * 8 for 64-bit archs and the input address was only aligned to 4.
 *
 * Taking out the pointers from NETIO_SHMEM_ConcurrentQueueStructToDetermineAlignment
 * should be safe as the only pointer inside NETIO_SHMEM_ConcurrentQueueHeader
 * is not used
 */
struct NETIO_SHMEM_ConcurrentQueueStructToDetermineAlignment
{
    union
    {
        char c;
        int i;
    } u;
};

#define NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES \
    (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueStructToDetermineAlignment)

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * copies the state variables modified during the operations that execute
 * inside the WRITE_EA
 *
 * The only operation that modifies state in the WRITE_EA is startWriteEA
 * startWriteEA modifies the following states:
 * msgEmptyIndex, bufferEmptyIndex,
 * bytesWrittenCounter,
 * msg_infos[msgEmptyIndex]
 *
 * We do not need to save/restore the value of
 * msg_infos[msgEmptyIndex]
 * because if the state is rolled back the contents of this msg_info slot
 * have no meaning.
 */
#define NETIO_SHMEM_ConcurrentQueue_copyWriteStateWriteEA(out, in)      \
    out->_bytes_written_counter    = in->_bytes_written_counter; \
    out->_buffer_empty_index       = in->_buffer_empty_index;    \
    out->_msg_empty_index          = in->_msg_empty_index;

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * copies the state variables modified during the operations that execute
 * inside the READ_EA
 *
 * The operations that modify the state in the READ_EA are: readEA
 * and finishReadEA
 *
 * readEA modifies:
 * msgReadIndex, bufferReadIndex
 * msg_infos[msgReadIndex]._cookie
 *
 * finishReadEA modifies:
 * bufferInUseIndex, msgInUseIndex,
 * bytesFullyReadCounter,
 * msg_infos[finishedHandle]._msgSize
 */
#define NETIO_SHMEM_ConcurrentQueue_copyReadStateReadEA(out, in)            \
    out->_buffer_read_index         = in->_buffer_read_index;        \
    out->_msg_read_index            = in->_msg_read_index;           \
    out->_msg_read_index_cookie     = in->_msg_read_index_cookie;    \
    out->_buffer_in_use_index       = in->_buffer_in_use_index;      \
    out->_msg_in_use_index          = in->_msg_in_use_index;         \
    out->_bytes_fully_read_counter  = in->_bytes_fully_read_counter; \
    out->_read_finished_hndl        = in->_read_finished_hndl;       \
    out->_read_finished_msg_size    = in->_read_finished_msg_size;   \
    out->_read_finished_cookie      = in->_read_finished_cookie;

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Checks if the state is consistent and if not it restores it from the
 * backup.
 *
 * NOTE: It is important that the implementation compares
 * q->_state->_modifyingWriteState with 0 and not with 1 or some other value
 * this is so that we are robust to the endianess
 */
#define NETIO_SHMEM_ConcurrentQueue_ensureConsistentWriteStateWriteEA(q)          \
    if ( q->_state->_modifying_write_state != 0 ) {                        \
        NETIO_SHMEM_ConcurrentQueue_copyWriteStateWriteEA(q->_state, q->_backup); \
    }

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Checks if the state is consistent and if not it restores it from the
 * backup.
 *
 * NOTE: It is important that the implementation compares
 * q->_state->_modifyingWriteState with 0 and not with 1 or some othe value
 * this is so that we are robust to the endianess
 */
#define NETIO_SHMEM_ConcurrentQueue_ensureConsistentReadStateReadEA(q)          \
    if ( q->_state->_modifying_read_state != 0 ) {                       \
        NETIO_SHMEM_ConcurrentQueue_copyReadStateReadEA(q->_state, q->_backup); \
    }

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Does the arithmentic equivalent of:
 * \code
 * return (index+1) % (maxIndex+1);
 * \endcode
 */
#define NETIO_SHMEM_ConcurrentQueue_getNextWrappedIndex(index, max_index) \
    ((index == (max_index)) ? 0 : (index + 1))

#define NETIO_SHMEM_CONCURRENT_QUEUE_SIZE_INVALID (-1)

/*** SOURCE_BEGIN ***/

/*
 * The baseline version of the concurrent queue which is normally used.
 */
const struct NETIO_SHMEM_ConcurrentQueueVersion
NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_BUG_14240_FIX =
{
    2, /* major */
    0 /* minor: not relevant*/
};

/*
 * The _msg_infos member inside a NETIO_SHMEM_ConcurrentQueueHandle structure had a type
 * of NETIO_SHMEM_ConcurrentQueueMsgInfoV3 before this version.
 * After that, the writer_cookie was added (in CORE-10280).
 *
 * This change would break compatibility, but to prevent this we have:
 *  - Maintained the default major and minor for the concurrent queue and the
 *    shared memory transport.
 *  - Added a property (minimum_compatibility_version). Depending on this
 *    property, we will create a queue with the default version or with
 *    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT.
 *
 *    By default NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower returns true
 *    because the default version is less than than
 *    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT. But, if the
 *    property sets the compatibility to 6.1.0 (no compatibility with 6.0.1
 *    needed), then the version of the concurrent queue matches
 *    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT and compatibility
 *    is broken. The writer cookie can only be used in this scenario. This means
 *    that when NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower is false, the source
 *    PID for inbound traffic will be always set to zero.
 */
const struct NETIO_SHMEM_ConcurrentQueueVersion
NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT =
{
    4, /* major: Do not use number 3 but 4 because 3 was being used before */
    0  /* minor: not relevant */
};

/*
 * Default version for the concurrent queue.
 *
 * Because the concurrent queue is a data structure, this default must always be
 * the latest. It is the responsibility of the user to set the target version
 * (this is, the user's default could be different from this default).
 *
 * Major: It should only be modified if we make an incompatible change to the
 * NETIO_SHMEM_ConcurrentQueue. If the Major version differs the
 * NETIO_SHMEM_ConcurrentQueue_attach operation will fail.
 *
 * Minor: It should change each time we release a compatible change to the
 * NETIO_SHMEM_ConcurrentQueue. It can be used by the implementation with the higher
 * minor version to perform the logic necessary to interoperate with earlier
 * versions. Differences in Minor version will not cause
 * NETIO_SHMEM_ConcurrentQueue_attach operation to fail.
 */
const struct NETIO_SHMEM_ConcurrentQueueVersion NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT =
{
    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MAJOR,
    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MINOR
};

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueueClass
 *
 * Checks the signature stored in the memory region and checks for
 * major version compatibility.
 *
 * @return RTI_TRUE if the signature is correct and the major version
 * match. Otherwise it logs an error message and returns RTI_FALSE
 */
RTI_PRIVATE RTIBool
NETIO_SHMEM_ConcurrentQueue_check_signature_and_version(
        struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr)
{
    /* Check the headers, signature and version compatibility */
    if ((q_hdr->_signature.first_byte
            == DELETED_CONCURRENT_QUEUE_FIRST_BYTE) &&
            (q_hdr->_signature.second_byte
            == DELETED_CONCURRENT_QUEUE_SECOND_BYTE))
    {
        NETIO_SHMEM_LOG_CQ_UNLINKED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }
    if ((q_hdr->_signature.first_byte != CONCURRENT_QUEUE_FIRST_BYTE) ||
            (q_hdr->_signature.second_byte
            != CONCURRENT_QUEUE_SECOND_BYTE))
    {
        NETIO_SHMEM_LOG_CQ_INVALID_SIGNATURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }
    if (q_hdr->_signature.version.major > NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT.major)
    {
        NETIO_SHMEM_LOG_CQ_INCOMPATIBLE_VERSION(
                OSAPI_LOGKIND_ERROR,
                NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT.major,
                q_hdr->_signature.version.major);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


RTI_PRIVATE RTI_INT32
NETIO_SHMEM_ConcurrentQueueState_get_alignment(void)
{
    struct S_ { char c; struct NETIO_SHMEM_ConcurrentQueueState member; };     
    return (RTI_INT32)((char*)&(((struct S_*)OSAPI_CC_NullPtr)->member) - (char*)OSAPI_CC_NullPtr);
}

RTI_PRIVATE RTI_INT32
NETIO_SHMEM_ConcurrentQueueMsgInfo_get_alignment(void)
{
    struct S_ { char c; struct NETIO_SHMEM_ConcurrentQueueMsgInfo member; };
    return (RTI_INT32)((char*)&(((struct S_*)OSAPI_CC_NullPtr)->member) - (char*)OSAPI_CC_NullPtr);
}

RTI_INT32
NETIO_SHMEM_ConcurrentQueue_get_overhead(
        RTI_INT32 msg_size_max,
        RTI_INT32 msg_count_max,
        const struct NETIO_SHMEM_ConcurrentQueueVersion *queue_version)
{
    RTI_INT32 size = 0;

    size = (RTI_SIZE_T)sizeof(struct NETIO_SHMEM_ConcurrentQueueHeader);

    /* First thing after the Header is _sharedState */
    size = NETIO_SHMEM_ConcurrentQueue_alignSize(
            size,
            NETIO_SHMEM_ConcurrentQueueState_get_alignment());
    size += (RTI_INT32)sizeof(struct NETIO_SHMEM_ConcurrentQueueState);

    /* Next thing is NETIO_SHMEM_ConcurrentQueueState _sharedStateBackup */
    size = NETIO_SHMEM_ConcurrentQueue_alignSize(
            size,
            NETIO_SHMEM_ConcurrentQueueState_get_alignment());
    size += (RTI_INT32)sizeof(struct NETIO_SHMEM_ConcurrentQueueState);

    /* Next thing is the _msg_infos[messageCountMax+1].
     *
     * We need one extra because we treat them as a circular
     * queue and we need to indicate the case where the queue is empty.
     *
     * Note: The alignment should be the same for the different versions of the
     * concurrent queue. We use the size of NETIO_SHMEM_ConcurrentQueueMsgInfo.
     */
    size = NETIO_SHMEM_ConcurrentQueue_alignSize(
            size,
            NETIO_SHMEM_ConcurrentQueueMsgInfo_get_alignment());
    size += NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(*queue_version)
            ? (msg_count_max + 1) * (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3)
            : (msg_count_max + 1) * (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueMsgInfo);

    /* Space for aligning messageCount+1 messages. Note that even though we
     * can only have messageCountMessages we need to reserve space for one more
     * which corresponds to the one we may place to avoid wrapping a message */
    size += (msg_count_max + 1) * (NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES - 1);

    /* Space for the extra message that we place to avoid wrapping */
    size += msg_size_max;

    /* Align to NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES. To avoid returning
     * an odd number */
    size = NETIO_SHMEM_ConcurrentQueue_alignSize(
            size,
            NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES);

    return size;
}

int
NETIO_SHMEM_ConcurrentQueue_get_size_required(
        int msg_size_max,
        int msg_count_max,
        int max_bytes_buffered,
        const struct NETIO_SHMEM_ConcurrentQueueVersion *queue_version)
{
    int size;

    /* Align to NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES. To avoid returning
     * an odd number */
    size = (max_bytes_buffered
            + NETIO_SHMEM_ConcurrentQueue_get_overhead(msg_size_max,msg_count_max,queue_version));

    return size;
}

RTIBool
NETIO_SHMEM_ConcurrentQueue_attach(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        char *mem_addr)
{
    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr;
    int shared_size_of_int;
    int message_size_max, message_count_max, max_data_bytes;
    int offset_to_state;
    int offset_to_state_backup;
    int offset_to_mgs_infos;
    int offset_to_buffer;
    unsigned int bufferSize;

    OSAPI_PRECONDITION(
            q == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("q",q,RTI_TRUE); )

    OSAPI_PRECONDITION(
            !OSAPI_Heap_is_address_aligned(
            mem_addr,
            NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES),
            return RTI_FALSE,
            );

    q->_corrupted_queue = RTI_FALSE;

    /* We assume that memAddress is aligned to
     * NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES i.e. the maximum alignment
     * required by any element in NETIO_SHMEM_ConcurrentQueueHeader
     *
     * We could try to add padding here but it is hard to do that in way
     * that would be robust to architectures where
     *        sizeof(char *) >sizeof(int)
     *
     * The simplest, more robust thing is to require that
     * the caller does that as the know how they got their memory
     */
    q->_desc._mem_address = mem_addr;
    q_hdr = OSAPI_Compiler_reinterpret_cast(struct NETIO_SHMEM_ConcurrentQueueHeader*,
                                            mem_addr);
    if (!NETIO_SHMEM_ConcurrentQueue_check_signature_and_version(q_hdr))
    {
        return RTI_FALSE;
    }

    /* ----------------------------- For Endianness ----------------------- */
    /* Determine whether we need to swap the endianess when we write to
     * the queue */

    /* The _sharedIsQueueHeaderBigEndian should only be compared against 0
     * 0 ==> little endian. !=0 ==> big-endian */
#ifdef RTI_ENDIAN_BIG
    if ( q_hdr->_shared_is_queue_hdr_big_endian == 0 )
    {
        /* ConcurrentQueue representation is little-endian, I'm big-endian */
        q->_adjust_for_endianness = 1;
    }
    else
    {
        /* ConcurrentQueue representation is big-endian, I'm big-endian */
        q->_adjust_for_endianness = 0;
    }
#else /* RTI_ENDIAN_LITTLE */
    if (q_hdr->_shared_is_queue_hdr_big_endian == 0)
    {
        /* ConcurrentQueue uses little-endian, I'm little-endian */
        q->_adjust_for_endianness = 0;
    }
    else
    {
        /* ConcurrentQueue uses big-endian, I'm little-endian */
        q->_adjust_for_endianness = 1;
    }
#endif

    /* Check for sizeof(int) compatibility. Requires that we have set
     * the q->_adjustForEndianness first
     */
    shared_size_of_int = q_hdr->_shared_size_of_int;
    if (q->_adjust_for_endianness == 1)
    {
        shared_size_of_int = (int)swap4Bytes(shared_size_of_int);
    }
    if (q_hdr->_shared_size_of_int != shared_size_of_int)
    {
        NETIO_SHMEM_LOG_CQ_INT_REPRESENTATION(
                OSAPI_LOGKIND_ERROR,
                q_hdr->_shared_size_of_int,
                shared_size_of_int);
        return RTI_FALSE;
    }

    bufferSize = (unsigned int)q_hdr->_shared_buffer_size;
    if (q->_adjust_for_endianness == 1)
    {
        bufferSize = swap4Bytes(bufferSize);
    }
    q->_buffer_size = bufferSize;

    /* Save a copy of the description of the queue, adjusted for
     * endianess
     */
    message_size_max = q_hdr->_shared_queue_description._message_size_max;
    message_count_max = q_hdr->_shared_queue_description._message_count_max;
    max_data_bytes = q_hdr->_shared_queue_description._max_data_bytes;
    if (q->_adjust_for_endianness == 1)
    {
        message_size_max = (int)swap4Bytes(message_size_max);
        message_count_max = (int)swap4Bytes(message_count_max);
        max_data_bytes = (int)swap4Bytes(max_data_bytes);
    }

    q->_desc._message_size_max = message_size_max;
    q->_desc._message_count_max = message_count_max;
    q->_desc._max_data_bytes = max_data_bytes;

    /* Set the pointers to the shared state */
    offset_to_state = q_hdr->_shared_offset_to_state;
    offset_to_state_backup = q_hdr->_shared_offset_to_state_backup;
    offset_to_mgs_infos = q_hdr->_shared_offset_to_msg_infos;
    offset_to_buffer = q_hdr->_shared_offset_to_buffer;
    if (q->_adjust_for_endianness == 1)
    {
        offset_to_state = (int)swap4Bytes(offset_to_state);
        offset_to_state_backup = (int)swap4Bytes(offset_to_state_backup);
        offset_to_mgs_infos = (int)swap4Bytes(offset_to_mgs_infos);
        offset_to_buffer = (int)swap4Bytes(offset_to_buffer);
    }

    q->_state = (void*)(char*)(mem_addr + offset_to_state);
    q->_backup = (void*)(char*)(mem_addr + offset_to_state_backup);
    q->_msg_infos = (void*)(char*)(mem_addr + offset_to_mgs_infos);
    q->_buffer = (void*)(char*)(mem_addr + offset_to_buffer);

    return RTI_TRUE;
}

RTIBool
NETIO_SHMEM_ConcurrentQueue_create(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int msg_count_max,
        int msg_size_max,
        char *mem_addr,
        int mem_addr_num_bytes,
        const struct NETIO_SHMEM_ConcurrentQueueProperty *property)
{
    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr;
    int offset;
    int cq_overhead;

    cq_overhead = NETIO_SHMEM_ConcurrentQueue_get_overhead(
            msg_size_max,
            msg_count_max,
            &property->version);

    OSAPI_PRECONDITION_ALWAYS(
            (q == NULL) || (mem_addr == NULL) || (msg_size_max <= 0) ||
            (msg_count_max <= 0),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("q",q,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("memAddress",mem_addr,RTI_FALSE);
            OSAPI_Log_entry_add_int("msg_size_max", msg_size_max, RTI_FALSE);
            OSAPI_Log_entry_add_int("msg_count_max",msg_count_max,RTI_TRUE); )

    /* Queue must fit at least 1 byte of user-data */
    OSAPI_PRECONDITION_ALWAYS(
            (mem_addr_num_bytes <= cq_overhead),
            return RTI_FALSE,
            OSAPI_Log_entry_add_int("overhead", cq_overhead, RTI_FALSE);
            OSAPI_Log_entry_add_int("bytes",mem_addr_num_bytes,RTI_TRUE); )

    OSAPI_PRECONDITION_ALWAYS(
            !OSAPI_Heap_is_address_aligned(
            mem_addr,
            NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES),
            return RTI_FALSE,
            OSAPI_Log_entry_add_int("overhead", cq_overhead, RTI_FALSE);
            OSAPI_Log_entry_add_int("bytes", mem_addr_num_bytes, RTI_TRUE); )

    /* We assume that memAddress is aligned to NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES
     * i.e. the maximum alignment required by any element
     * in NETIO_SHMEM_ConcurrentQueueHeader
     *
     * We could try to add padding here but it is hard to do that in way that
     * would be robust to architectures where sizeof(char *) > sizeof(int)
     * The simplest, more robust thing is to require that the caller does that
     * as the know how they got their memory
     */
     q_hdr = OSAPI_Compiler_reinterpret_cast(struct NETIO_SHMEM_ConcurrentQueueHeader*,
                                             mem_addr);

    /* Zero the header. This initializes the shared state to the proper
     * values (all zeros) for an empty queue. It also sets the flags that
     * track when the state is modifying to false
     */
    OSAPI_Memory_zero(q_hdr, sizeof(struct NETIO_SHMEM_ConcurrentQueueHeader));

    q_hdr->_signature.first_byte = CONCURRENT_QUEUE_FIRST_BYTE;
    q_hdr->_signature.second_byte = CONCURRENT_QUEUE_SECOND_BYTE;

    q_hdr->_signature.version = property->version;

#ifdef RTI_ENDIAN_BIG
    q_hdr->_shared_is_queue_hdr_big_endian = 1;
#else /* RTI_ENDIAN_LITTLE */
    q_hdr->_shared_is_queue_hdr_big_endian = 0;
#endif
    /* For the logic to be correct everybody attached to the queue must
     * use  the same size to represent integers. The logic could be
     * adjusted to not require that but there is no need to do the work until
     * we encounter the use-case. In the mean-time we check and give
     * an error in attach if the size is different
     */
    q_hdr->_shared_size_of_int = sizeof(int);

    /* Store the description of the queue */
    q_hdr->_shared_queue_description._max_data_bytes = mem_addr_num_bytes
            - cq_overhead;
    q_hdr->_shared_queue_description._message_size_max = msg_size_max;
    q_hdr->_shared_queue_description._message_count_max = msg_count_max;

    /* --------------------------------------------------------------------- */
    /* Set the offsets
     * PROPOSITION: The alignemt requirement met by memAddress which is
     * NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES is greater than that for 'int',
     * struct NETIO_SHMEM_ConcurrentQueueMsgInfo, and any other alignment requirements
     * so we can we perform the align
     * logic 'as if' q_hdr==0; i.e. we can align the offset starting at
     * q_hdr.
     */
    offset = (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueHeader);

    /* First thing after the Header is _sharedState */
    offset = NETIO_SHMEM_ConcurrentQueue_alignSize(
            offset,
            NETIO_SHMEM_ConcurrentQueueState_get_alignment());
    OSAPI_Memory_zero(
            ((char*)mem_addr + offset),
            sizeof(struct NETIO_SHMEM_ConcurrentQueueState));
    q_hdr->_shared_offset_to_state = offset;
    offset += (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueState);

    /* Next thing is NETIO_SHMEM_ConcurrentQueueState _sharedStateBackup */
    offset = NETIO_SHMEM_ConcurrentQueue_alignSize(
            offset,
            NETIO_SHMEM_ConcurrentQueueState_get_alignment());
    OSAPI_Memory_zero(
            ((char*)mem_addr + offset),
            sizeof(struct NETIO_SHMEM_ConcurrentQueueState));
    q_hdr->_shared_offset_to_state_backup = offset;
    offset += (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueState);

    /*
     * Next thing is the _msg_infos[messageCountMax+1]
     *
     * Note: The alignment should be the same for the different versions of the
     * concurrent queue. We use the size of NETIO_SHMEM_ConcurrentQueueMsgInfo.
     */
    offset = NETIO_SHMEM_ConcurrentQueue_alignSize(
            offset,
            NETIO_SHMEM_ConcurrentQueueMsgInfo_get_alignment());
    q_hdr->_shared_offset_to_msg_infos = offset;
    offset += NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
                      q_hdr->_signature.version)
            ? ((msg_count_max + 1) * (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3))
            : ((msg_count_max + 1) * (int)sizeof(struct NETIO_SHMEM_ConcurrentQueueMsgInfo));

    /* Next is the memory to hold messages.
     * Must be aligned to NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES
     */
    offset = NETIO_SHMEM_ConcurrentQueue_alignSize(
            offset,
            (int)NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES);
    q_hdr->_shared_offset_to_buffer = offset;
    q_hdr->_shared_buffer_size = mem_addr_num_bytes
            - q_hdr->_shared_offset_to_buffer;

    /* Attach the NETIO_SHMEM_ConcurrentQueue handle to the newly formatted queue */
    return NETIO_SHMEM_ConcurrentQueue_attach(q, mem_addr);
}

RTIBool
NETIO_SHMEM_ConcurrentQueue_start_write_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int *finished_hhdl,
        char **where_to,
        int mgs_size,
        unsigned int writer_cookie)
{
    /* These are things we will read from NETIO_SHMEM_ConcurrentQueue */
    unsigned int r__bytesWrittenCounter = 0, r__bytesFullyReadCounter = 0;
    int r__msgInUseIndex = -1, r__msgEmptyIndex = -1, r__bufferEmptyIndex = -1;

    /* Things we will write to NETIO_SHMEM_ConcurrentQueue */
    unsigned int w__bytesWrittenCounter = 0;
    int w__msgEmptyIndex = -1, w__bufferEmptyIndex = -1;

    /* These are constants we access from NETIO_SHMEM_ConcurrentQueue */
    int const__messageCountMax = -1, const__maxDataBytes = -1;

    /* scratch variables */
    int writeIndex = -1, availableSize = 0;

    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfo *msg_info = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *msg_info_v3 = NULL;
    RTIBool is_v3_or_lower;

    OSAPI_PRECONDITION(
            (q == NULL) || (finished_hhdl == NULL) || (where_to == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("q",q,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("f_handle",finished_hhdl,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("where_to",where_to,RTI_TRUE); )

    OSAPI_PRECONDITION(
            (q->_state == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("q->state",q->_state,RTI_FALSE); )

    if (q->_corrupted_queue)
    {
        /*
         * The queue has been corrupted and possibly tampered, so there's no
         * point in writing anything else.
         */
        return RTI_FALSE;
    }

    if (mgs_size > q->_desc._message_size_max)
    {
        return RTI_FALSE;
    }

    q_hdr = OSAPI_Compiler_reinterpret_cast(
                    struct NETIO_SHMEM_ConcurrentQueueHeader*,q->_desc._mem_address);

    is_v3_or_lower = NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            q_hdr->_signature.version);
    if (is_v3_or_lower)
    {
        msg_info_v3 = (struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *) q->_msg_infos;
    }
    else
    {
        msg_info = (struct NETIO_SHMEM_ConcurrentQueueMsgInfo *) q->_msg_infos;
    }

    /* Check consistency of state and restore if needed */
    NETIO_SHMEM_ConcurrentQueue_ensureConsistentWriteStateWriteEA(q);

    /* Read the needed fields from the NETIO_SHMEM_ConcurrentQueue */
    /* We need to check if byte swap is needed for all fields that
     * we read from the NETIO_SHMEM_ConcurrentQueueState.
     *  If not needed, then the performance hit is an if (...) check.
     */
    r__msgInUseIndex = q->_state->_msg_in_use_index;
    r__bytesWrittenCounter = q->_state->_bytes_written_counter;
    r__bytesFullyReadCounter = q->_state->_bytes_fully_read_counter;
    r__msgEmptyIndex = q->_state->_msg_empty_index;
    r__bufferEmptyIndex = q->_state->_buffer_empty_index;
    if (q->_adjust_for_endianness)
    {
        r__msgInUseIndex = (int)swap4Bytes(r__msgInUseIndex);
        r__bytesWrittenCounter = swap4Bytes(r__bytesWrittenCounter);
        r__bytesFullyReadCounter = swap4Bytes(r__bytesFullyReadCounter);
        r__msgEmptyIndex = (int)swap4Bytes(r__msgEmptyIndex);
        r__bufferEmptyIndex = (int)swap4Bytes(r__bufferEmptyIndex);
    }

    const__messageCountMax = q->_desc._message_count_max;

    if (r__msgEmptyIndex < 0 || r__msgEmptyIndex > const__messageCountMax)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (r__bufferEmptyIndex < 0 || (unsigned int) r__bufferEmptyIndex > q->_buffer_size)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    const__maxDataBytes = NETIO_SHMEM_ConcurrentQueue_getSize(q);

    /* check there is space and reserve the buffer */
    w__msgEmptyIndex = NETIO_SHMEM_ConcurrentQueue_getNextWrappedIndex(
            r__msgEmptyIndex,
            const__messageCountMax);

    /* At first glance this computation may appear suspect because it
     * only looks at the number of bytes written by the user when in reality more
     * bytes are potentially consumed due to the aligment which can cause each
     * message written by the user to be increased by up-to MSG_ALIGMENT-1
     * However these extra bytes (maxMessages)*(MSG_ALIGMNET-1) are already
     * accounted as "overhead" by the function NETIO_SHMEM_ConcurrentQueue_getOverhead()
     * So the determination of available size can be based solely in the
     * bytes read/written by the caller.
     */
    NETIO_SHMEM_ConcurrentQueue_get_available_size(
            &availableSize,
            r__bytesWrittenCounter,
            r__bytesFullyReadCounter,
            const__maxDataBytes);

    if (w__msgEmptyIndex == r__msgInUseIndex || availableSize < mgs_size)
    {
        return RTI_FALSE;
    }

    /* Account for the data as written, but indicate the writing is in
     * progress */
    w__bytesWrittenCounter = r__bytesWrittenCounter + (unsigned int)mgs_size;

    /* Advance the writeIndex. Start at beginning if this advance would
     * cause the message to wrap around. That way we keep it contiguous.
     * We know there is always space to keep it contiguous because the
     * buffer can physically hold  maxDataBytes+messageSizeMax but we never
     * let it hold more that maxDataBytes bytes.
     */
    if (NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(q, r__bufferEmptyIndex, mgs_size))
    {
        writeIndex = 0;
    }
    else
    {
        writeIndex = r__bufferEmptyIndex;
    }
    w__bufferEmptyIndex = NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(
            writeIndex + mgs_size);

    /* Write the needed fields from the NETIO_SHMEM_ConcurrentQueue */
    /* The algorithm to modify the state is always:
     * (1) save a copy of the '_state' into the '_backup'
     * (2) set the '_modifyingXXXState' flag
     * (3) modify the state
     * (4) unset the  '_modifyingXXXState' flag
     *
     * This ensures that if the process crashes while the state is
     * being modified, then whoever detects that can restore the state
     * from the backup
     */

    /* (1) save a copy of the '_state' into the '_backup' */
    NETIO_SHMEM_ConcurrentQueue_copyWriteStateWriteEA(q->_backup, q->_state);

    /* (2) set the '_modifyingXXXState' flag */
    q->_state->_modifying_write_state = 1; /* endianess OK. We check if == 0 */

    /* (3) modify the state. Must use endianess of the queue */
    /* VERY IMPORTANT: the _msgEmptyIndex must be the last state modified
     * because it is read outside the WRITER_EA by the reading functions.
     * Setting this field last and given that an integer write is atomic
     * ensures that if a reader sees the change, it will have a consistent
     * picture for (e.g the _msg_info) expecially the size.
     * q->_msg_infos must also be modified before _msgEmptyIndex.
     */
    if (q->_adjust_for_endianness)
    {
        q->_state->_bytes_written_counter = swap4Bytes(w__bytesWrittenCounter);
        if (is_v3_or_lower)
        {
            msg_info_v3[r__msgEmptyIndex]._size = (int) swap4Bytes(-mgs_size);
            msg_info_v3[r__msgEmptyIndex]._cookie = swap4Bytes(writer_cookie);
        }
        else
        {
            msg_info[r__msgEmptyIndex]._size = (int) swap4Bytes(-mgs_size);
            msg_info[r__msgEmptyIndex]._cookie = swap4Bytes(writer_cookie);
            msg_info[r__msgEmptyIndex]._writer_cookie = (RTI_UINT32)swap4Bytes(writer_cookie);
        }
        q->_state->_buffer_empty_index = (int) swap4Bytes(w__bufferEmptyIndex);
        /* _msgEmptyIndex must be the last state modified */
        q->_state->_msg_empty_index = (int) swap4Bytes(w__msgEmptyIndex);
    }
    else
    {
        q->_state->_bytes_written_counter = w__bytesWrittenCounter;
        if (is_v3_or_lower)
        {
            msg_info_v3[r__msgEmptyIndex]._size = -mgs_size;
            msg_info_v3[r__msgEmptyIndex]._cookie = writer_cookie;
        }
        else
        {
            msg_info[r__msgEmptyIndex]._size = -mgs_size;
            msg_info[r__msgEmptyIndex]._cookie = writer_cookie;
            msg_info[r__msgEmptyIndex]._writer_cookie = (RTI_UINT32)writer_cookie;
        }
        q->_state->_buffer_empty_index = w__bufferEmptyIndex;
        /* _msgEmptyIndex must be the last state modified */
        q->_state->_msg_empty_index = w__msgEmptyIndex;
    }

    /* (4) unset the  '_modifyingXXXState' flag */
    q->_state->_modifying_write_state = 0;

    /* Set the output parameters */
    *where_to = &q->_buffer[writeIndex];
    *finished_hhdl = r__msgEmptyIndex;

    return RTI_TRUE;
}

int
NETIO_SHMEM_ConcurrentQueue_start_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int *finishedHandle,
        char **whereFrom,
        unsigned int readerCookie)
{
    /* These are things we will read from NETIO_SHMEM_ConcurrentQueue */
    int r__msg_empty_index = -1;
    int r__msg_read_index = -1;
    int r__buffer_read_index = -1;

    /* Things we will write to NETIO_SHMEM_ConcurrentQueue */
    int w__msg_read_index = -1;
    int w__buffer_read_index = -1;

    /* These are constants we access from NETIO_SHMEM_ConcurrentQueue */
    int const__msg_count_max = 0;

    /* scratch variables */
    int size = 0;

    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfo *msg_info = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *msg_info_v3 = NULL;
    RTIBool is_v3_or_lower;

    OSAPI_PRECONDITION(
            (q == NULL) || (finishedHandle == NULL) || (whereFrom == NULL),
            return 0,
            OSAPI_Log_entry_add_pointer("q",q,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("f_handle",finishedHandle,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("where_from",whereFrom,RTI_TRUE); )

    OSAPI_PRECONDITION(
            (q->_state == NULL),
            return 0,
            OSAPI_Log_entry_add_pointer("q->state",q->_state,RTI_FALSE); )

    if (q->_corrupted_queue)
    {
        /*
         * The queue has been corrupted and possibly tampered, so there's no
         * point in reading anything else.
         */
        return RTI_FALSE;
    }

    q_hdr = OSAPI_Compiler_reinterpret_cast(
                    struct NETIO_SHMEM_ConcurrentQueueHeader *,q->_desc._mem_address);

    is_v3_or_lower = NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            q_hdr->_signature.version);
    if (is_v3_or_lower)
    {
        msg_info_v3 = (struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *) q->_msg_infos;
    }
    else
    {
        msg_info = (struct NETIO_SHMEM_ConcurrentQueueMsgInfo *) q->_msg_infos;
    }

    /* --------------------------------------------- */
    /* Check consistency of state and restore if needed */
    NETIO_SHMEM_ConcurrentQueue_ensureConsistentReadStateReadEA(q);

    /* --------------------------------------------- */
    /* Read the needed fields from the NETIO_SHMEM_ConcurrentQueue */
    /* _msgEmptyIndex is the last one modified by the writer
     * outside the READ_EA so it is good practice to read it
     * first. But the algorithm does not require it */
    r__msg_empty_index = q->_state->_msg_empty_index;
    r__msg_read_index = q->_state->_msg_read_index;
    r__buffer_read_index = q->_state->_buffer_read_index;
    size = is_v3_or_lower
            ? msg_info_v3[r__msg_read_index]._size
            : msg_info[r__msg_read_index]._size;
    if (q->_adjust_for_endianness)
    {
        r__msg_empty_index = (int)swap4Bytes(r__msg_empty_index);
        r__msg_read_index = (int)swap4Bytes(r__msg_read_index);
        r__buffer_read_index = (int)swap4Bytes(r__buffer_read_index);
        size = (int)swap4Bytes(size);
    }

    const__msg_count_max = q->_desc._message_count_max;

    if (r__msg_read_index < 0 || r__msg_read_index > const__msg_count_max)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return 0;
    }

    if (r__buffer_read_index < 0 || (unsigned int) r__buffer_read_index > q->_buffer_size)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return 0;
    }

    /* --------------------------------------------- */

    /* Check there is data */
    if ((r__msg_read_index == r__msg_empty_index) || (size <= 0))
    {
        return 0;
    }

    /* Check if the message fit as a contiguous chunk after the
     * bufferReadIndex; if not, need to wrap-around the readIndex because
     * the NETIO_SHMEM_ConcurrentQueue_write() would have done exactly that so
     * the current message is at index 0.
     */
    if (NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(q, r__buffer_read_index, size))
    {
        r__buffer_read_index = 0;
    }

    /* indicate msg is in use by advancing the read-index but not the
     * inUse index */
    w__buffer_read_index = r__buffer_read_index
            + NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(size);
    w__msg_read_index = NETIO_SHMEM_ConcurrentQueue_getNextWrappedIndex(
            r__msg_read_index,
            const__msg_count_max);

    /* Write the needed fields from the NETIO_SHMEM_ConcurrentQueue */
    /* (1) save a copy of the '_state' into the '_backup'
     * This MUST be kept consistent with the macro
     * NETIO_SHMEM_ConcurrentQueue_copyReadStateReadEA
     * We do not use the macro because it copies more than we need
     * Note that it is safe to restore the whole set as the
     * backup will already contain correct values for
     * the other read state variables, as they would not have
     * been modified since the last time teh state was saved...
     */
    q->_backup->_buffer_read_index = q->_state->_buffer_read_index;
    q->_backup->_msg_read_index = q->_state->_msg_read_index;
    q->_backup->_msg_read_index_cookie = q->_state->_msg_read_index_cookie;

    /* (2) set the '_modifyingXXXState' flag */
    q->_state->_modifying_read_state = 1; /* endianess OK. We check if == 0 */

    /* (3) modify the state. Must use endianess of the queue */
    if (q->_adjust_for_endianness)
    {
        if (is_v3_or_lower)
        {
            msg_info_v3[r__msg_read_index]._cookie = swap4Bytes(readerCookie);
        }
        else
        {
            msg_info[r__msg_read_index]._cookie = swap4Bytes(readerCookie);
        }
        q->_state->_buffer_read_index = (int) swap4Bytes(w__buffer_read_index);
        q->_state->_msg_read_index = (int) swap4Bytes(w__msg_read_index);
    }
    else
    {
        if (is_v3_or_lower)
        {
            msg_info_v3[r__msg_read_index]._cookie = readerCookie;
        }
        else
        {
            msg_info[r__msg_read_index]._cookie = readerCookie;
        }
        q->_state->_buffer_read_index   = w__buffer_read_index;
        q->_state->_msg_read_index      = w__msg_read_index;
    }

    /* (4) unset the  '_modifyingXXXState' flag */
    q->_state->_modifying_read_state = 0;

    /* Set the output parameters */
    *whereFrom = &q->_buffer[r__buffer_read_index];
    *finishedHandle = r__msg_read_index;

    return size;
}

void
NETIO_SHMEM_ConcurrentQueue_finish_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int finished_hndl)
{
    /* These are things we will read from NETIO_SHMEM_ConcurrentQueue */
    int r__msg_read_index = -1;

    /* These are things we will read and write from NETIO_SHMEM_ConcurrentQueue */
    unsigned int rw__bytes_fully_read_counter = 0;
    int rw__buffer_inuse_index = -1;
    int rw__msg_inuse_index = -1;
    int rw__msg_size;

    /* These are constants we access from NETIO_SHMEM_ConcurrentQueue */
    int const__msg_count_max = 0;

    /* Scratch variables */
    int msg_read_size;

    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfo *msg_info = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *msg_info_v3 = NULL;
    RTIBool is_v3_or_lower;

    OSAPI_PRECONDITION(
            (q == NULL) || (finished_hndl < 0) || (finished_hndl >
            q->_desc._message_count_max),
            return ,
            OSAPI_Log_entry_add_pointer("q",q,RTI_FALSE);
            OSAPI_Log_entry_add_int("f_handle",finished_hndl,RTI_FALSE);
            OSAPI_Log_entry_add_int(
            "msg_count_max",
            q->_desc._message_count_max,
            RTI_TRUE); )

    OSAPI_PRECONDITION(
            (q->_state == NULL),
            return ,
            OSAPI_Log_entry_add_pointer("q->state",q->_state,RTI_FALSE); )

    q_hdr = OSAPI_Compiler_reinterpret_cast(
                    struct NETIO_SHMEM_ConcurrentQueueHeader *,q->_desc._mem_address);

    is_v3_or_lower = NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            q_hdr->_signature.version);
    if (is_v3_or_lower)
    {
        msg_info_v3 = (struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *) q->_msg_infos;
    }
    else
    {
        msg_info = (struct NETIO_SHMEM_ConcurrentQueueMsgInfo *) q->_msg_infos;
    }

    /* Check consistency of state and restore if needed */
    NETIO_SHMEM_ConcurrentQueue_ensureConsistentReadStateReadEA(q);

    /* Read the needed fields from the NETIO_SHMEM_ConcurrentQueue */
    rw__bytes_fully_read_counter = q->_state->_bytes_fully_read_counter;
    rw__buffer_inuse_index = q->_state->_buffer_in_use_index;
    rw__msg_inuse_index = q->_state->_msg_in_use_index;
    r__msg_read_index = q->_state->_msg_read_index;
    rw__msg_size = is_v3_or_lower
            ? msg_info_v3[finished_hndl]._size
            : msg_info[finished_hndl]._size;
    if (q->_adjust_for_endianness)
    {
        rw__bytes_fully_read_counter = swap4Bytes(rw__bytes_fully_read_counter);
        rw__buffer_inuse_index = (int)swap4Bytes(rw__buffer_inuse_index);
        rw__msg_inuse_index = (int)swap4Bytes(rw__msg_inuse_index);
        r__msg_read_index = (int)swap4Bytes(r__msg_read_index);
        rw__msg_size = (int)swap4Bytes(rw__msg_size);
    }

    const__msg_count_max = q->_desc._message_count_max;

    if (rw__msg_inuse_index < 0 || rw__msg_inuse_index > const__msg_count_max)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return;
    }

    if (r__msg_read_index < 0 || r__msg_read_index > const__msg_count_max)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)
        return;
    }

    /* ---------------------------------------------- */
    /* Write the needed fields from the NETIO_SHMEM_ConcurrentQueue */

    /* (1) save a copy of the '_state' into the '_backup'
     * This code MUST be kept consistent with the macro
     * NETIO_SHMEM_ConcurrentQueue_copyReadStateReadEA
     *
     * We only copy the things we need. It is not
     * appropriate to use the macro as it does not have
     * all the information we need.
     *
     * Note that it is safe to restore the whole set as the
     * backup will already contain the other read state variables.
     *
     * The state and the backup must be maintained using the
     * endianess of the queue. So it is easier to copy from
     * the existing state when we can.
     */
    q->_backup->_buffer_in_use_index = q->_state->_buffer_in_use_index;
    q->_backup->_msg_in_use_index = q->_state->_msg_in_use_index;
    q->_backup->_bytes_fully_read_counter =
            q->_state->_bytes_fully_read_counter;
    q->_backup->_read_finished_msg_size = is_v3_or_lower
            ? msg_info_v3[finished_hndl]._size
            : msg_info[finished_hndl]._size;
    q->_backup->_read_finished_cookie = is_v3_or_lower
            ? msg_info_v3[finished_hndl]._cookie
            : msg_info[finished_hndl]._cookie;
    if (q->_adjust_for_endianness)
    {
        q->_backup->_read_finished_hndl = (int)swap4Bytes(finished_hndl);
    }
    else
    {
        q->_backup->_read_finished_hndl = finished_hndl;
    }

    /* (2) set the '_modifyingXXXState' flag */
    q->_state->_modifying_read_state = 1; /* endianess OK. We check if == 0 */

    /* change of sign (to negative) to indicate that read is finished */
    rw__msg_size = -rw__msg_size;
    if (q->_adjust_for_endianness)
    {
        if (is_v3_or_lower)
        {
            msg_info_v3[finished_hndl]._size = (int) swap4Bytes(rw__msg_size);
            /* No need to adjust NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID
            because it is guaranteed to be == 0
            */
            msg_info_v3[finished_hndl]._cookie =
                    NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
        }
        else
        {
            msg_info[finished_hndl]._size = (int) swap4Bytes(rw__msg_size);
            /*
             * No need to adjust NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID
             * because it is guaranteed to be == 0
             */
            msg_info[finished_hndl]._cookie =
                    NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
            msg_info[finished_hndl]._writer_cookie =
                    NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
        }
    }
    else
    {
        if (is_v3_or_lower)
        {
            msg_info_v3[finished_hndl]._size = rw__msg_size;
        }
        else
        {
            msg_info[finished_hndl]._size = rw__msg_size;
            msg_info[finished_hndl]._writer_cookie =
                    NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
        }
    }

    /* Starting with the first message in use, see if you can release
     * some of the messages.
     * This logic accounts for the fact that there may be multiple
     * concurrent readers, so it is not necessarily the case that
     * we can advance the bufferInUseIndex just because we finished
     * reading a buffer.
     * In other words, we will only be able to advance the index if
     * ( q->_msgInUseIndex==finished_hndl )
     * however adding this extra check would not improve performence
     * as the check in the while() loop accomplishes the same.
     */
    while (rw__msg_inuse_index != r__msg_read_index)
    {
        /* Get the size of the next message that perhaps could be read */
        msg_read_size = is_v3_or_lower
                ? msg_info_v3[rw__msg_inuse_index]._size
                : msg_info[rw__msg_inuse_index]._size;
        if (q->_adjust_for_endianness)
        {
            msg_read_size = (int)swap4Bytes(msg_read_size);
        }

        /* Exit if the message is being read (i.e. is not fully read)
         * beingRead <==> _msg_infos[rw__msgInUseIndex]._size >0
         */
        if (msg_read_size > 0)
        {
            break;
        }

        /* Its fully read. So the size has a negative sign, change it */
        msg_read_size = -msg_read_size;
        if (NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(
                q,
                rw__buffer_inuse_index,
                msg_read_size))
        {
            rw__buffer_inuse_index = 0;
        }
        rw__buffer_inuse_index += NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(
                msg_read_size);

        rw__msg_inuse_index = NETIO_SHMEM_ConcurrentQueue_getNextWrappedIndex(
                rw__msg_inuse_index,
                const__msg_count_max);

        /* msg_read_size must be > 0 */
        rw__bytes_fully_read_counter += (unsigned int)msg_read_size;
    }

    /* (3) modify the state. Must use endianess of the queue
     * VERY IMPORTANT: _msgInUseIndex must be the last state modified
     * because it is read by the writer functions outside the READ_EA
     * and it is important that when they see it change, the reader is
     * no longer modifing anything inside (e.g. the message size)
     */
    if (q->_adjust_for_endianness)
    {
        q->_state->_bytes_fully_read_counter = swap4Bytes(
                rw__bytes_fully_read_counter);
        q->_state->_buffer_in_use_index = (int)swap4Bytes(rw__buffer_inuse_index);
        q->_state->_msg_in_use_index = (int)swap4Bytes(rw__msg_inuse_index);
    }
    else
    {
        q->_state->_bytes_fully_read_counter = rw__bytes_fully_read_counter;
        q->_state->_buffer_in_use_index = rw__buffer_inuse_index;
        q->_state->_msg_in_use_index = rw__msg_inuse_index;
    }
    /* (4) unset the  '_modifyingXXXState' flag */
    q->_state->_modifying_read_state = 0;
}

void
NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *info_out)
{
    int inuse_size;
    int to_be_read_size;
    int r__msg_read_index;
    int r__msg_in_use_index;
    int r__msg_empty_index;
    unsigned int r__bytes_fully_read_counter;
    unsigned int r__bytes_written_counter;
    int const__msg_count_max = 0;

    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfo *msg_info = NULL;
    struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *msg_info_v3 = NULL;
    RTIBool is_v3_or_lower;

    /* Check consistency of state and restore if needed */
    NETIO_SHMEM_ConcurrentQueue_ensureConsistentReadStateReadEA(q);

    q_hdr = OSAPI_Compiler_reinterpret_cast(
                    struct NETIO_SHMEM_ConcurrentQueueHeader *,q->_desc._mem_address);

    is_v3_or_lower = NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            q_hdr->_signature.version);
    if (is_v3_or_lower)
    {
        msg_info_v3 = (struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *) q->_msg_infos;
    }
    else
    {
        msg_info = (struct NETIO_SHMEM_ConcurrentQueueMsgInfo *) q->_msg_infos;
    }

    /* Read the needed fields from the NETIO_SHMEM_ConcurrentQueue
     * IMPORTANT: Inside the READ_EA always read the _msgEmptyIndex first!
     */
    r__msg_read_index = q->_state->_msg_read_index;
    r__msg_empty_index = q->_state->_msg_empty_index;
    r__msg_in_use_index = q->_state->_msg_in_use_index;
    to_be_read_size = is_v3_or_lower
            ? msg_info_v3[r__msg_read_index]._size
            : msg_info[r__msg_read_index]._size;
    inuse_size = is_v3_or_lower
            ? msg_info_v3[r__msg_in_use_index]._size
            : msg_info[r__msg_in_use_index]._size;
    r__bytes_fully_read_counter = q->_state->_bytes_fully_read_counter;
    r__bytes_written_counter = q->_state->_bytes_written_counter;
    if (q->_adjust_for_endianness)
    {
        r__msg_empty_index = (int)swap4Bytes(r__msg_empty_index);
        r__msg_read_index = (int)swap4Bytes(r__msg_read_index);
        r__msg_in_use_index = (int)swap4Bytes(r__msg_in_use_index);
        to_be_read_size = (int)swap4Bytes(to_be_read_size);
        inuse_size = (int)swap4Bytes(inuse_size);
        r__bytes_fully_read_counter = swap4Bytes(r__bytes_fully_read_counter);
        r__bytes_written_counter = swap4Bytes(r__bytes_written_counter);
    }

    const__msg_count_max = q->_desc._message_count_max;

    if (r__msg_read_index < 0 || r__msg_read_index > const__msg_count_max)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)

        info_out->_next_to_be_read_msg_is_ready_to_read = RTI_FALSE;
        info_out->_next_to_be_read_msg_is_being_written = RTI_FALSE;
        info_out->_have_msg_being_read = RTI_FALSE;
        return;
    }

    if (r__msg_in_use_index < 0 || r__msg_in_use_index > const__msg_count_max)
    {
        q->_corrupted_queue = RTI_TRUE;
        NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(OSAPI_LOGKIND_ERROR)

        info_out->_next_to_be_read_msg_is_ready_to_read = RTI_FALSE;
        info_out->_next_to_be_read_msg_is_being_written = RTI_FALSE;
        info_out->_have_msg_being_read = RTI_FALSE;
        return;
    }

    /* inUseSize should normally be positive because when it is
     * changed to a negative number by finishReadEA() it also
     * advances the msgInUseIndex. However if we had interrupted
     * the reader (via ^C) in the middle of the finishReadEA operation
     * the sign could be negative and the index have not advanced
     * yet. So force a positive value. */
    if (inuse_size < 0)
    {
        inuse_size = -inuse_size;
    }

    /* Check if there are any messages to be read ready or being written */
    if (r__msg_read_index == r__msg_empty_index)
    {
        info_out->_next_to_be_read_msg_is_being_written = RTI_FALSE;
        info_out->_next_to_be_read_msg_is_ready_to_read = RTI_FALSE;
        info_out->_next_to_read_msg_size = 0;
        info_out->_next_to_be_read_msg_write_finish_hndl = -1;
        info_out->_next_to_be_read_msg_writer_cookie = 0;
    }
    else
    {
        /* Discriminate between ready to be read and being written
         * using the sign of the message-size */
        if (to_be_read_size > 0)
        {
            info_out->_next_to_be_read_msg_is_being_written = RTI_FALSE;
            info_out->_next_to_be_read_msg_is_ready_to_read = RTI_TRUE;
            info_out->_next_to_read_msg_size = to_be_read_size;
        }
        else
        {
            info_out->_next_to_be_read_msg_is_being_written = RTI_TRUE;
            info_out->_next_to_be_read_msg_is_ready_to_read = RTI_FALSE;
            info_out->_next_to_read_msg_size = -to_be_read_size;
        }
        info_out->_next_to_be_read_msg_write_finish_hndl = r__msg_read_index;
        info_out->_next_to_be_read_msg_writer_cookie = is_v3_or_lower
                ? msg_info_v3[r__msg_read_index]._cookie
                : msg_info[r__msg_read_index]._cookie;
    }

    /* Check is there is a message being read */
    if (r__msg_read_index == r__msg_in_use_index)
    {
        /* No messages being read */
        info_out->_have_msg_being_read = RTI_FALSE;
        info_out->_next_being_read_msg_read_finish_hndl = -1;
        info_out->_next_being_read_msg_size = 0;
        info_out->_next_being_read_msg_reader_cookie = 0;
    }
    else
    {
        info_out->_have_msg_being_read = RTI_TRUE;
        info_out->_next_being_read_msg_read_finish_hndl = r__msg_in_use_index;
        info_out->_next_being_read_msg_size = inuse_size;
        info_out->_next_being_read_msg_reader_cookie = is_v3_or_lower
                ? msg_info_v3[r__msg_in_use_index]._cookie
                : msg_info[r__msg_in_use_index]._cookie;
    }

    /* store num bytes read/written */
    info_out->_bytes_fully_read_counter = r__bytes_fully_read_counter;
    info_out->_bytes_written_counter = r__bytes_written_counter;
}

RTIBool
NETIO_SHMEM_ConcurrentQueue_is_writer_potentially_stuck(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *previous_info,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *current_info)
{
    UNUSED_ARG(q);
    return (current_info->_next_to_be_read_msg_is_being_written &&
           (current_info->_bytes_fully_read_counter
           == previous_info->_bytes_fully_read_counter) &&
           (current_info->_next_to_be_read_msg_write_finish_hndl
           == previous_info->_next_to_be_read_msg_write_finish_hndl));
}

void
NETIO_SHMEM_ConcurrentQueue_get_available_size(
        int *available_size_ptr,
        unsigned int bytes_written_counter,
        unsigned int bytes_fully_read_counter,
        int max_data_bytes)
{
    *(available_size_ptr) = (int)(bytes_written_counter - bytes_fully_read_counter);

    if ((unsigned int)(*(available_size_ptr)) > (unsigned int)max_data_bytes)
    {
        *(available_size_ptr) = (-1) - *(available_size_ptr);
    }
    *(available_size_ptr) = max_data_bytes - *(available_size_ptr);
}

void
NETIO_SHMEM_ConcurrentQueue_flush_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        unsigned int read_cookie)
{
    struct NETIO_SHMEM_ConcurrentQueueStateInfo info_out;
    int finished_handle = 0;
    char *where_from;

    OSAPI_PRECONDITION(
            (q == NULL),
            return ,
            OSAPI_Log_entry_add_pointer("q", q,RTI_TRUE); )

    /* Check consistency of state and restore if needed */
    NETIO_SHMEM_ConcurrentQueue_ensureConsistentReadStateReadEA(q);

    while (1)
    {
        NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(q, &info_out);

        if (info_out._have_msg_being_read)
        {
            /* First finish all messages that are being read */
            NETIO_SHMEM_ConcurrentQueue_finish_read_ea(
                    q,
                    info_out._next_being_read_msg_read_finish_hndl);
        }
        else if (info_out._next_to_be_read_msg_is_ready_to_read)
        {
            /* Next read all messages that are ready to be read */
            NETIO_SHMEM_ConcurrentQueue_start_read_ea(
                    q,
                    &finished_handle,
                    &where_from,
                    read_cookie);
            NETIO_SHMEM_ConcurrentQueue_finish_read_ea(q, finished_handle);
        }
        else
        {
            /* Exit when there are no messages being read or in
             * the process of being read */
            break;
        }
    }
}

RTIBool
NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(
        const struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int data_index,
        int data_size)
{
    return (NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(data_index + data_size)
                    > (int)q->_buffer_size);
}

void
NETIO_SHMEM_ConcurrentQueue_finish_write(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int finishedHandle,
        int msgSize)
{
    if (NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            (OSAPI_Compiler_reinterpret_constcast(struct NETIO_SHMEM_ConcurrentQueueHeader *,q->_desc._mem_address))->_signature.version))
    {
        struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *msg_info =
            (struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *) (q)->_msg_infos;

        msg_info[finishedHandle]._size = (q)->_adjust_for_endianness
            ? (int) swap4Bytes(msgSize)
            : (msgSize);
        msg_info[finishedHandle]._cookie = NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
    }
    else
    {
        struct NETIO_SHMEM_ConcurrentQueueMsgInfo *msg_info =
            (struct NETIO_SHMEM_ConcurrentQueueMsgInfo *) (q)->_msg_infos;

        msg_info[finishedHandle]._size = (q)->_adjust_for_endianness
            ? (int) swap4Bytes(msgSize)
            : (msgSize);
        msg_info[finishedHandle]._cookie = NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID;
    }
}

#if 0
void
NETIO_SHMEM_ConcurrentQueue_print(
        const struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        FILE *file)
{
    struct NETIO_SHMEM_ConcurrentQueueHeader *q_hdr =
            (struct NETIO_SHMEM_ConcurrentQueueHeader *) q->_desc._mem_address;
    RTIBool is_v3_or_lower = NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(
            q_hdr->_signature.version);
    int availableSize = 0, numMsgs = 0, i = 0, bufferIndex = -1, absSize = 0;
    char symbol = 0;

    NETIO_SHMEM_ConcurrentQueue_get_available_size(
            &availableSize,
            q->_state->_bytes_written_counter,
            q->_state->_bytes_fully_read_counter,
            NETIO_SHMEM_ConcurrentQueue_getSize(q));

    fprintf(
            file,
            "NETIO_SHMEM_ConcurrentQueue: messageCountMax=%d, messageSizeMax=%d, "
            "maxBytesForData=%d (%d avail) Total written/read=%d/%d\n",
            q->_desc._message_count_max,
            q->_desc._message_size_max,
            q->_desc._max_data_bytes,
            availableSize,
            q->_state->_bytes_written_counter,
            q->_state->_bytes_fully_read_counter);

    numMsgs = q->_state->_msg_empty_index - q->_state->_msg_in_use_index;
    if (numMsgs < 0)
    {
        numMsgs += q->_desc._message_count_max;
    }

    /* Buffer is being read */
    symbol = 'R';
    i = q->_state->_msg_in_use_index;
    bufferIndex = q->_state->_buffer_in_use_index;

    do
    {
        if (i == q->_state->_msg_empty_index)
        { /* Buffer is empty */
            symbol = 'E';
        }
        else if (i == q->_state->_msg_read_index)
        {
            /* full or being written */
            symbol = 'W';
        }

        if (symbol != 'E')
        {
            int origAbsSize = is_v3_or_lower
                    ? ((struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 *)q->_msg_infos + i)->_size
                    : ((struct NETIO_SHMEM_ConcurrentQueueMsgInfo *)q->_msg_infos + i)->_size;
            absSize = origAbsSize;
            if (absSize < 0)
            {
                absSize = -absSize;
            }
            if (NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(q, bufferIndex, absSize))
            {
                bufferIndex = 0;
            }

            fprintf(
                    file,
                    "%c %4d: %8d Bytes@ %8d= %c%c%c%c%c%c...\n",
                    symbol,
                    i,
                    origAbsSize,
                    bufferIndex,
                    q->_buffer[bufferIndex],
                    q->_buffer[bufferIndex + 1],
                    q->_buffer[bufferIndex + 2],
                    q->_buffer[bufferIndex + 3],
                    q->_buffer[bufferIndex + 4],
                    q->_buffer[bufferIndex + 5]);
        }
        else
        {
            fprintf(file, "%c %4d:\n", symbol, i);
        }

        bufferIndex += NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(absSize);
        i = NETIO_SHMEM_ConcurrentQueue_getNextWrappedIndex(
                i,
                q->_desc._message_count_max);
    } while (i != q->_state->_msg_in_use_index);
}
#endif
