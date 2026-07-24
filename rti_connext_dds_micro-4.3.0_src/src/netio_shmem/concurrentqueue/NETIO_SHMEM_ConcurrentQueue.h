/*
 * FILE: NETIO_SHMEM_ConcurrentQueue.h
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef NETIO_SHMEM_ConcurrentQueue_h
#define NETIO_SHMEM_ConcurrentQueue_h

#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES (8)
#define NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID (0)

struct NETIO_SHMEM_ConcurrentQueueStateInfo
{
    /*e
     * Total number of bytes ever written to the queue since it was created
     */
    unsigned int _bytes_written_counter;

    /*e Total number of bytes ever read from the queue since it was created */
    unsigned int _bytes_fully_read_counter;

    /*e
     * Boolean flag indicating whether the next message in the queue
     * is ready to be read. In other words, whether the next call
     * to NETIO_SHMEM_ConcurrentQueue_startReadEA will return a message
     */
    RTIBool _next_to_be_read_msg_is_ready_to_read;

    /*e
     * Boolean flag indicating whether the next message in the queue
     * is still being written. In other words, whether a writer has
     * called NETIO_SHMEM_ConcurrentQueue_startWriteEA but has not yet
     * called NETIO_SHMEM_ConcurrentQueue_finishWriteEA
     */
    RTIBool _next_to_be_read_msg_is_being_written;

    /*e
     * Size of the message that will be returned by the next call to
     * NETIO_SHMEM_ConcurrentQueue_startReadEA, assuming it is known.
     */
    int _next_to_read_msg_size;

    /*e
     * In the event that _nextToBeeadMsgIsBeingWritten==RTI_TRUE, this
     * field contains the value of the finishedHandle that the writer
     * writing that message should be used as a parameter to
     * NETIO_SHMEM_ConcurrentQueue_finishWriteEA
     */
    int _next_to_be_read_msg_write_finish_hndl;

    /*e
     * In the event that _nextToBeeadMsgIsBeingWritten==RTI_TRUE, this
     * field contains the value of the cookie that the writer
     * writing that message used in the call to
     * NETIO_SHMEM_ConcurrentQueue_startWriteEA
     */
    unsigned int _next_to_be_read_msg_writer_cookie;

    /*e
     * Boolean flag indicating whether there is one (or more) messages
     * in the process of being read.
     */
    RTIBool _have_msg_being_read;

    /*e
     * In the event that _haveMsgBeingRead==RTI_TRUE this field contains
     * the size of the 'first' message that is still being read. "First"
     * determined by the order in which NETIO_SHMEM_ConcurrentQueue_startReadEA was
     * called.
     */
    int _next_being_read_msg_size;

    /*e
     * In the event that _haveMsgBeingRead==RTI_TRUE this field contains
     * the finishHandle of the 'first' message that is still being read.
     * That is the finishHandle that should be passed to the
     * NETIO_SHMEM_ConcurrentQueue_finishReadEA to indicate the message has been
     * completely read
     */
    int _next_being_read_msg_read_finish_hndl;

    /*e
     * In the event that _haveMsgBeingRead==RTI_TRUE this field contains
     * the value of the cookie that the reader
     * reading that message used in the call to NETIO_SHMEM_ConcurrentQueue_startReadEA
     */
    unsigned int _next_being_read_msg_reader_cookie;
};

#define NETIO_SHMEM_ConcurrentQueueStateInfo_INITIALIZER \
{ \
    0, /* _bytes_written_counter */ \
    0, /* _bytes_fully_read_counter */ \
    RTI_FALSE, /* _next_to_be_read_msg_is_ready_to_read */ \
    RTI_FALSE, /* _next_to_be_read_msg_is_being_written */ \
    0, /* _next_to_read_msg_size */ \
    0, /* _next_to_be_read_msg_write_finish_hndl */ \
    0, /* _next_to_be_read_msg_writer_cookie */ \
    RTI_FALSE, /* _have_msg_being_read */ \
    0, /* _next_being_read_msg_size */ \
    0, /* _next_being_read_msg_read_finish_hndl */ \
    0  /* _next_being_read_msg_reader_cookie */ \
}


struct NETIO_SHMEM_ConcurrentQueueVersion
{
    char major;
    char minor;
};

extern
const struct NETIO_SHMEM_ConcurrentQueueVersion NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_BUG_14240_FIX;

extern
const struct NETIO_SHMEM_ConcurrentQueueVersion NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT;

extern
const struct NETIO_SHMEM_ConcurrentQueueVersion NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT;

/*
 * Default version values for the concurrent queue. This must always be the
 * latest queue version.
 * See NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT.
 */
#define NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MAJOR (4)
#define NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MINOR (0)

#define NETIO_SHMEM_ConcurrentQueueProperty_INITIALIZER \
{ \
  { \
    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MAJOR, \
    NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_DEFAULT_MINOR \
  } \
}


struct NETIO_SHMEM_ConcurrentQueueProperty
{
    struct NETIO_SHMEM_ConcurrentQueueVersion version;
};

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 *  Data structure containing the state of the buffer defined by the
 *  values of all the indices and counters.
 *
 *  The buffer is segmented by the values of the indices:
 *  bufferInUseIndex, bufferReadIndex,  bufferEmptyIndex
 *
 *  The region bufferReadIndex..bufferEmptyIndex contains messages that
 *  are either written or in the process of being written. The distinction
 *  is kept in the _mesSize[] array for the corresponding messages.
 *
 *  The identity of each message is managed by the contents of the
 *  _msgInfos[]. This array contains information for each message that
 *  is presently in the queue. There are (maxMessageCount+1) slots in
 *  the array to be able to hold information in maxMessageCount
 *  messages (the extra id used by the wrap-around logic, to encode the
 *  fact that there are no messages).
 *
 *  The _msgInfos[] array is  controlled by the indices:
 *  msgReadIndex, msgInUseIndex, msgEmptyIndex
 *
 *  Only slots in the range msgInUseIndex..msgEmptyIndex contain valid
 *  information
 *
 *  This segmentation is depicted in the figure below:
 *
 *  \verbatim
 *           bufferInUseIndex
 *           |               bufferReadIndex
 *           |               |                       bufferEmptyIndex
 *           |               |                       |
 *           |               |                       |
 *   0.......8......16......24......32......40......48......56......64
 *   +-------+-------+-------+-------+-------+-------+-------+-------+
 *     empty | being read    |   being written       |      empty
 *   +---------------------------------------------------------------+
 *                ^  ^           ^   ^
 *                .  .           .   .
 *  +---------+   .  .           .   .
 *  |         |   .  .           .   .
 *  +---------+   .  .           .   .
 *  |  size1  |   msgInUseIndex  .   .
 *  +---------+      .           .   .
 *  |  size2  |      -           .   .
 *  +---------+                  .   .
 *  |  size3  |   msgReadIndex --/   .
 *  +---------+                      .
 *  |  size4  |                ------/
 *  +---------+
 *  |         |    msgEmptyIndex
 *  +---------+
 *  |         |
 *  +---------+
 *
 *  \endverbatim
 *
 *
 *
 * @invariant bufferReadIndex contains the index of the first byte containing
 *  unread data. The data corresponds to the message indicated in msgHeadIndex
 *  and the message is of length msgSizes[msgReadIndex]
 *
 *  @invariant bufferInUseIndex contains the index of the first byte
 *  currently in the process of being read. That is,
 *  NETIO_SHMEM_ConcurrentQueue_startReadEA() has been called for that message, but
 *  NETIO_SHMEM_ConcurrentQueue_finishReadEA() has not yet been called.
 *
 *  @invariant The message indexed bufferInUseIndex corresponds to
 *  msgInUseIndex and has size msgSizes[msgInUseIndex]
 *
 *  @invariant bufferEmptyIndex contains index of the first free byte
 *  (were we can write new data).
 *
 *  @invariant msgEmptyIndex contains the index of the first message
 *  that is empty.
 *
 *  @invariant The following statements hold, provided intervals are
 *  interpreted modulus the wrap-around:
 *  \code
 *  index "k" in [bufferEmptyIndex, bufferInUseIndex] represent space available
 *                                                to write new data
 *
 *  index "k" in [bufferReadIndex,  bufferEmptyIndex] represent data that
 *  is either being written or it has been written(and is available to read)
 *  These two situations are distinguished by the sign of the 'size' value
 *  in the element in the _msgInfos[] array that represents the message
 *  A negative value indicates data being written. A positive value
 *  indicates data fully written and available to be read.
 *  The bufferReadIndex can only advance when data is started to be read and
 *  this can only happen if the next message is fully written. Consequently
 *  there may be messages fully written that are not returned when the
 *  NETIO_SHMEM_ConcurrentQueue_startReadEA operation is called because there may
 *  be prior messages still being written.
 *
 *  index "k" in [bufferInUseIndex, bufferReadIndex] represent data that is
 *  either being read (NETIO_SHMEM_ConcurrentQueue_startReadEA() has been called, but
 *  NETIO_SHMEM_ConcurrentQueue_finishReadEA() has not been called), or has already
 *  been read. These two situations are distinguished by the sign of the 'size'
 *  value in the element in the _msgInfos[] array that represents the message
 *  A negative value indicates fully read. A positive value
 *  indicates data being read.
 *  The bufferInUseIndex can only advance when data completely read
 *  (NETIO_SHMEM_ConcurrentQueue_finishReadEA is called) and there are no
 *  prior messages still being read. Consequently there may be messages
 *  fully read in the interval [bufferInUseIndex, bufferReadIndex] since
 *  the bufferInUseIndex is not able to advance until the message with
 *  index bufferInUseIndex is fully read.
 *  \endcode
 *
 *  @invariant (msgHeadIndex == msgEmptyIndex) <==> No messages
 *
 *  The behavior of the queue is illustrated in this short use-case
 *  for the situation where maxMessageCount = 3, maxDataBytes = 48
 *  maxMessageSize = 16.
 *  To save space, in the depiction it is assumed that
 *  NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES=4
 *
 *  (1) The initial state of the queue is:
 *
 *  \verbatim
 *     bufferInUseIndex
 *     bufferReadIndex
 *     bufferEmptyIndex
 *     |
 *     |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------+-------+-------+-------+-------+-------+-------+-------+
 *
 *     +---------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  N/A  | <- msgInUseIndex  <- msgReadIndex  <- msgEmptyIndex
 *      .   +-------+
 *      1   |  N/A  |
 *      .   +-------+
 *      2   |  N/A  |
 *          +-------+
 *  \endverbatim
 *
 *
 *  (2) After calling _startWrite(size = 8)
 *  \verbatim
 *     bufferInUseIndex
 *     bufferReadIndex
 *     |       bufferEmptyIndex
 *     |       |
 *     |       |
 *     0.......8......16......24......32......40......48......56......64
 *     +---------------------------------------------------------------+
 *     xxxxxxxx
 *     +---------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  -8   | <- msgInUseIndex  <- msgReadIndex  (note negative #)
 *      .   +-------+
 *      1   |  N/A  |                                    <- msgEmptyIndex
 *      .   +-------+
 *      2   |  N/A  |
 *          +-------+
 *                     NOTE NEGATIVE NUMBER
 *  \endverbatim
 *
 *  (3) After calling _startWrite(size = 4)
 *  \verbatim
 *     bufferInUseIndex
 *     bufferReadIndex
 *     |           bufferEmptyIndex
 *     |           |
 *     |           |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     xxxxxxxxxxxx
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  -8   | <- msgInUseIndex  <- msgReadIndex
 *      .   +-------+
 *      1   |  -4   |  (note negative #)
 *      .   +-------+
 *      2   |  N/A  |                                    <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *  (4) After calling _finishWrite(size = 4)
 *  assume contents of the write was 'BBBB'
 *
 *  \verbatim
 *     bufferInUseIndex
 *     bufferReadIndex
 *     |           bufferEmptyIndex
 *     |           |
 *     |           |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     xxxxxxxxBBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  -8   | <- msgInUseIndex  <- msgReadIndex
 *      .   +-------+
 *      1   |  +4   |  (note change from -4 to 4, a positive #)
 *      .   +-------+
 *      2   |  N/A  |                                    <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *
 *  (5) calling _startRead() will return no data as the message pointed by
 *  bufferReadIndex is not fully written as indicated by
 *  _msgSize[ msgReadIndex] < 0
 *
 *  (6) After calling _finishWrite(size = 8)
 *  Assume data written was 'AAAAAAAA'
 *  \verbatim
 *     bufferInUseIndex
 *     bufferReadIndex
 *     |           bufferEmptyIndex
 *     |           |
 *     |           |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     AAAAAAAABBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  +8   | <- msgInUseIndex  <- msgReadIndex
 *      .   +-------+    (note change from -8 to +8, a positive #)
 *      1   |  +4   |
 *      .   +-------+
 *      2   |  N/A  |                                    <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *  (7) After calling _startRead() ==> returns finishHandle=0
 *  \verbatim
 *     bufferInUseIndex
 *             bufferReadIndex
 *     |       |   bufferEmptyIndex
 *     |       |   |
 *     |       |   |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     AAAAAAAABBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  +8   | <- msgInUseIndex
 *      .   +-------+
 *      1   |  +4   |                   <- msgReadIndex
 *      .   +-------+
 *      2   |  N/A  |                                    <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *  (8) After calling _startRead() ==> returns fishishHandle=1
 *  \verbatim
 *     bufferInUseIndex
 *                 bufferReadIndex
 *     |           bufferEmptyIndex
 *     |           |
 *     |           |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     AAAAAAAABBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  +8   | <- msgInUseIndex
 *      .   +-------+
 *      1   |  +4   |
 *      .   +-------+
 *      2   |  N/A  |                   <- msgReadIndex  <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *
 *  (9) After calling _finishRead( finishHandle = 1)
 *  \verbatim
 *     bufferInUseIndex
 *                 bufferReadIndex
 *     |           bufferEmptyIndex
 *     |           |
 *     |           |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     AAAAAAAABBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  +8   | <- msgInUseIndex
 *      .   +-------+
 *      1   |  -4   | (note change from +4 to -4, a negative #)
 *      .   +-------+
 *      2   |  N/A  |                   <- msgReadIndex  <- msgEmptyIndex
 *          +-------+
 *  \endverbatim
 *
 *  (10) After calling _finishRead( finishHandle = 0)
 *  \verbatim
 *                 bufferInUseIndex
 *                 bufferReadIndex
 *                 bufferEmptyIndex
 *                 |
 *                 |
 *     0.......8......16......24......32......40......48......56......64
 *     +-------------------------------------------------------------+
 *     AAAAAAAABBBB
 *     +-------------------------------------------------------------+
 *          msgSizes[]
 *          +-------+
 *      0   |  -8   |
 *      .   +-------+
 *      1   |  -4   |
 *      .   +-------+
 *      2   |  N/A  |  <- msgInUseIndex  <- msgReadIndex  <- msgEmptyIndex
 *          +-------+
 *     _msgSize[                 0 ]  = -8  // NOTE CHANGE TO NEGATIVE
 *     _msgSize[                 1 ]  = -4
 *     _msgSize[ msgInUseIndex = 2 ]  = NA
 *     _msgSize[ msgReadIndex  = 2 ]  = NA
 *     _msgSize[ msgEmptyIndex = 2 ]  = NA
 *  \endverbatim
 */
struct NETIO_SHMEM_ConcurrentQueueState
{
    /* Indicates the WRITE state is being modified */
    int _modifying_write_state;

    /* Indicates the READ state is being modified */
    int _modifying_read_state;

    /* these two counters are used to keep track of the current number
     * of bytes in the buffer in an efficient thread-safe manner
     * without requiring to mutex reads with respect to writes.
     */
    unsigned int _bytes_written_counter;
    unsigned int _bytes_fully_read_counter;

    /* Indicates the first location in use within the buffer */
    int _buffer_in_use_index;

    /* Indicates the next location that contains unread data in the buffer */
    int _buffer_read_index;

    /* Indicates the first free location in buffer */
    int _buffer_empty_index;

    /* Index to the first message in msgData[] that is still in use */
    int _msg_in_use_index;

    /* Index to the first full message in msgData[] */
    int _msg_read_index;

    /* Index to the first empty message in msgData[] */
    int _msg_empty_index;

    /* Extra information modified during readEA */
    int _msg_read_index_cookie;

    /* Extra information modified during finishReadEA */
    int _read_finished_hndl;
    int _read_finished_msg_size;
    unsigned int _read_finished_cookie;
};

struct NETIO_SHMEM_ConcurrentQueueDesc
{
    /*e Maximum number of bytes of all messages in the buffer */
    int _max_data_bytes;

    /*e Maximum number of bytes on a single write/read */
    int _message_size_max;

    /*e Maximum number of individual messages the buffer can hold */
    int _message_count_max;

    /*e The memory address passed to the create or attach call */
    const char *_mem_address;
};

/*i
 * \ingroup NETIO_SHMEM_ConcurrentQueue
 *
 * Signature placed as the first 4 bytes in the REDACOncurrentQueue
 * to identify it as a  REDACOncurrentQueue and store the version number
 * to allow for interoperability with future versions.
 *
 * Inteoperability requirements derive from the potential situation where
 * a shared-memory transport is built on top if the NETIO_SHMEM_ConcurrentQueue
 * and built into a version of a product that then needs to communicate
 * with future versions of teh product where the transport-logic or the
 * NETIO_SHMEM_ConcurrentQueue logic has changed.
 *
 * The firstByte and secondByte should never be changed.
 */
struct NETIO_SHMEM_ConcurrentQueueSignature
{
    char first_byte; /* should be 'C' */
    char second_byte; /* should be 'Q' */
    struct NETIO_SHMEM_ConcurrentQueueVersion version;
};

/*i
 * \ingroup NETIO_SHMEM_ConcurrentQueue
 *
 * Structure to contain the Header that is placed at the beginning of the
 * NETIO_SHMEM_ConcurrentQueue
 *
 * The overall layout of the memory passed to NETIO_SHMEM_ConcurrentQueue_create
 * of NETIO_SHMEM_ConcurrentQueue_attach is as follows:
 *
 * Begin of shared memory passed to _create/attach
 * [ assumed aligned to the maximum of
 * sizeof(int), NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES]
 *
 * SHARED_MEMORY_SEGMENT
 * +--------------------------------------------------------------------+
 * |                                                                    |
 * |  struct NETIO_SHMEM_ConcurrentQueueHeader _header   {                      |
 * |      ...                                                           |
 * |      int                              _sharedBufferSize;    |
 * |      int                              _sharedOffsetToState;        |
 * |      int                              _sharedOffsetToStateBackup;  |
 * |      int                              _sharedOffsetToMsgInfos;     |
 * |      int                              _sharedOffsetToBuffer;       |
 * |      int                              _sharedSizeofInt;            |
 * |      struct NETIO_SHMEM_ConcurrentQueueDesc   _sharedQueueDescription;     |
 * |      ------------------------------------------------------------  |
 * |      <this area can be expanded without breaking interoperability> |
 * |      ------------------------------------------------------------  |
 * |      struct NETIO_SHMEM_ConcurrentQueueState  _sharedState;                |
 * |      ------------------------------------------------------------  |
 * |      <this area can be expanded without breaking interoperability> |
 * |      ------------------------------------------------------------  |
 * |      struct NETIO_SHMEM_ConcurrentQueueState  _sharedStateBackup;          |
 * |  }                                                                 |
 * +--------------------------------------------------------------------+
 * |  EMPTY:                                                            |
 * |     <this area can be expanded without breaking interoperability>  |
 * +--------------------------------------------------------------------+
 * |  padding to align to int                                           |
 * +--------------------------------------------------------------------+
 * |                                                                    |
 * |  struct NETIO_SHMEM_ConcurrentQueueMsgInfo _msgInfos[messageCountMax + 1]  |
 * |                                                                    |
 * +--------------------------------------------------------------------+
 * |  EMPTY:                                                            |
 * |     <this area can be expanded without breaking interoperability>  |
 * +--------------------------------------------------------------------+
 * |  padding to align to                                               |
 * |      NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES                         |
 * +--------------------------------------------------------------------+
 * |                                                                    |
 * |  char _buffer[maxDataBytes+messageSizeMax]                         |
 * |                                                                    |
 * +--------------------------------------------------------------------+
 * |  space for padding (messageCountMax) messages                      |
 * |  such that each is aligned to                                      |
 * |      NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES                         |
 * +-------------------------------------------------------------------+
 *
 * Note: The design was done so that the area before the _sharedQueueDescription
 * could also be expanded. That is the motivation for the  _sharedOffsetToQueueDesc
 * However because of a bug that was present in all versions up and including 1.15
 * it is not possible to expand the space above the _sharedQueueDescription
 * without breaking interoperability with versions 1.15 and prior...
 *
 * Note that we need space for (messageCountMax+1) integers to hold
 * the _msgInfos (which contain the message size and cookie). We
 * need one slot extra beyond the maximum number of messages because
 * the _msgInfos[] is used to implement a circular queue and we need
 * an extra slot to indicate the case where the queue is
 * empty. Consequently all (messageCountMax+1) slots may be used
 * (abeit not all at the same time).
 *
 * The library accesses the shared-memory information via the
 * NETIO_SHMEM_ConcurrentQueueHandle. This during the "attach operation the
 * NETIO_SHMEM_ConcurrentQueueHandle is configured to point to the right places
 * in shared memory by reading the NETIO_SHMEM_ConcurrentQueueHeader and
 * resolving the offsets so that the pointers to shared memory
 * in NETIO_SHMEM_ConcurrentQueueHandle are given the proper address for the
 * process that uses that handle:
 *
 * NETIO_SHMEM_ConcurrentQueueHandle (initialized during attach)
 * +------------------------------------------------+
 * |                                                |
 * + struct NETIO_SHMEM_ConcurrentQueueDesc _desc;          | copied from SHARED_MEMORY_SEGMENT._sharedOffsetToQueueDesc
 * |                                                |
 * +------------------------------------------------+
 * + int _adjustForEndianness;                      | computed from  SHARED_MEMORY_SEGMENT._sharedIsQueueHeaderBigEndian
 * +------------------------------------------------+
 * | struct NETIO_SHMEM_ConcurrentQueueState   *_state;     | --> SHARED_MEMORY_SEGMENT._header._sharedOffsetToState
 * +------------------------------------------------+
 * | struct NETIO_SHMEM_ConcurrentQueueState   *_backup;    | --> SHARED_MEMORY_SEGMENT._header._sharedOffsetToStateBackup
 * +------------------------------------------------+
 * | struct NETIO_SHMEM_ConcurrentQueueMsgInfo *_msgInfos;  | --> SHARED_MEMORY_SEGMENT._header._sharedOffsetToMsgInfos
 * +------------------------------------------------+
 * | char                              *_buffer;    | --> SHARED_MEMORY_SEGMENT._header._sharedOffsetToBuffer
 * +------------------------------------------------+
 *
 *
 * @todo Add a getter operation _getNextMsgWriterId() that allows a
 * user to retrieve the ID. This can be used to store for example
 * the PID or some counter and then check for progress
 */
struct NETIO_SHMEM_ConcurrentQueueHeader
{

    /* Note the layout of the header CANNOT be modified. It will break
     * interoperability */
    /* BEGIN --------- DO NOT MODIFY AREA --------------------- */
    struct NETIO_SHMEM_ConcurrentQueueSignature _signature;

    /* Indicates whether the values in the header are stored in
     * little-endian or big endian notation.
     * The flag _sharedIsQueueHeaderBigEndian should only be compared
     * for (in)equality with zero. That way we don't have to
     * worry about the endianess of the the flag
     */
    RTIBool _shared_is_queue_hdr_big_endian;
    int _shared_buffer_size;
    int _shared_offset_to_state;
    int _shared_offset_to_state_backup;
    int _shared_offset_to_msg_infos;
    int _shared_offset_to_buffer;
    int _shared_size_of_int;
    struct NETIO_SHMEM_ConcurrentQueueDesc _shared_queue_description;
    /* END --------- DO NOT MODIFY AREA --------------------- */

    /* Header can be expanded with additional information below
     * this expansion will not break interoperability
     */
};

struct NETIO_SHMEM_ConcurrentQueueMsgInfo
{
    int _size;
    unsigned int _cookie;
    RTI_UINT32 _writer_cookie;
};


struct NETIO_SHMEM_ConcurrentQueueMsgInfoV3 {
    int _size;
    unsigned int _cookie;
};

struct NETIO_SHMEM_ConcurrentQueueHandle
{
    /* Copy of the Parameters used to configure the queue
     * when it was created. Should be considered constant and read-only
     */
    struct NETIO_SHMEM_ConcurrentQueueDesc _desc;

    /* If a byte swap is required since writing to a different endian
     * representation.
     */
    int _adjust_for_endianness;

    /* Points to the shared state of the queue */
    struct NETIO_SHMEM_ConcurrentQueueState *_state;

    /* Points to a backup of the state. Uset to restore the state if
     * a process crashes while modifying the state
     */
    struct NETIO_SHMEM_ConcurrentQueueState *_backup;

    /*
     * Together with the bufferReadIndex defines the array of messages. Must be
     * able to hold messageCountMax+1 elements.
     *
     * The type of _msg_infos is void so that using it requires a cast.
     * The final type depends on the version of the queue.
     * Possible values: NETIO_SHMEM_ConcurrentQueueMsgInfo and
     * NETIO_SHMEM_ConcurrentQueueMsgInfoV3.
     */
    void *_msg_infos;

    /*
     * The buffer containing the messages. Contains space for bufferSize bytes
     * Note that always: bufferSize > (maxDataBytes + messageSizeMax)
     * in fact currently we set
     *     bufferSize = (maxDataBytes + messageSizeMax)
     *          + (NETIO_SHMEM_CONCURRENT_QUEUE_ALIGN_BYTES -1)*messageSizeMax
     */
    char *_buffer;

    /* The number of bytes available for using starting at buffer */
    unsigned int _buffer_size;

    /*
     * Irreversibly set to true when we detect that the queue has been
     * corrupted. When this happens, no more writes to the queue are allowed.
     */
    RTIBool _corrupted_queue;
};

#define NETIO_SHMEM_ConcurrentQueue_getSize(q) ((q)->_desc._max_data_bytes)

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * \verbatim
 * State Reads:  msgInUseIndex, msgReadIndex, msgEmtyIndex
 * bytesFullyReadCount, bytesWritenCount
 *
 * State Writes: NONE
 *
 * Reserves: Nothing but may access
 * q->_msgInfos[msgInUseIndex], q->_msgInfos[msgReadIndex]
 *
 * \endverbatim
 */
void
NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *info_out);


/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * @implementation the following checks are used to detect that there has
 * been no progress in the 'next writer' i.e.
 * the one writing the message that would be the next one read
 * it necessary and sufficient to check that the following 3 conditions
 * are met:
 * (a) The next message that would be read is being written
 * AND
 * (b) The _bufferReadIndex has not changed (i.e.) we are still
 * at the same location
 * AND
 * (c) The _bytesFullyReadCounter has not changed.
 *
 * (a) Tells us that there is a writer writing the next message that would
 * be read. The combination of (b) and (c) ensure the writer is the same
 * as it was before
 */
RTIBool
NETIO_SHMEM_ConcurrentQueue_is_writer_potentially_stuck(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *previous_info,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *current_info);

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Defines the size required to use a buffer of size maxBytesBuffered to
 * hold messageCountMax number of messages with the max size of a message
 * being messageSizeMax.
 *
 * The size returned also accounts for the overhead corresponding to the
 * space needed to store the NETIO_SHMEM_ConcurrentQueueState, the integer array
 * containing message sizes, plus any wasted space due to alignment.
 *
 */
int
NETIO_SHMEM_ConcurrentQueue_get_size_required(
        int msg_size_max,
        int msg_count_max,
        int max_bytes_buffered,
        const struct NETIO_SHMEM_ConcurrentQueueVersion *queue_version);

#if 0
void
NETIO_SHMEM_ConcurrentQueue_print(
        const struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        FILE *file);
#endif

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * @implementation Uses NETIO_SHMEM_ConcurrentQueue_getQueueStateInfoReadEA,
 * NETIO_SHMEM_ConcurrentQueue_startReadEA and NETIO_SHMEM_ConcurrentQueue_finishReadEA
 * to read all 'being read' or 'ready to read' messages.
 */
void
NETIO_SHMEM_ConcurrentQueue_flush_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        unsigned int readCookie);

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Create a NETIO_SHMEM_ConcurrentQueue using the specified memory and
 * property. Formats it to hold the specified messages and attaches
 * the handle to it.
 */
RTIBool
NETIO_SHMEM_ConcurrentQueue_create(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int msg_count_max,
        int msg_size_max,
        char *mem_addr,
        int mem_addr_num_bytes,
        const struct NETIO_SHMEM_ConcurrentQueueProperty *property);

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * @brief The init call associates a NETIO_SHMEM_ConcurrentQueue structure with
 * a memory location.
 *
 * The NETIO_SHMEM_ConcurrentQueue contains NETIO_SHMEM_ConcurrentQueueDesc and pointers
 * to an integer, the NETIO_SHMEM_ConcurrentQueueState, an integer array
 * containing the sizes of the messages in the queue, and a buffer for
 * the messages.
 *
 * The init call initializes the pointers in the NETIO_SHMEM_ConcurrentQueue as
 * follows. The memory location pointed to by memAddress is divided into
 * an integer, NETIO_SHMEM_ConcurrentQueueState, an integer array for message
 * sizes, and a buffer for the messages. Thus, all the state information
 * for the queue is stored in the memory location, in addition to the
 * message sizes and the messages. Consequently, other
 * NETIO_SHMEM_ConcurrentQueue structures can initialize using the same memory
 * location, memAddressNumBytes, messageCountMax and messageSizeMax and
 * end up using the same concurrent queue.
 *
 * For endianness adjustments, the init call reads the integer field that
 * encodes the endianness of the memory location pointed to by
 * memAddress.  The convention is that the field is always encoded using
 * big-endian representation.
 *
 * The field may contain one of two special integer values,
 * RTI_ENDIAN_BIG_REPRESENTATION or RTI_ENDIAN_LITTLE_REPRESENTATION,
 * that indicate that the memory was initialized using big or little
 * endian representation, respectively. Accordingly, the init call sets
 * the adjustForEndianness flag depending on the local endianness. If the
 * field does not contain the above, then this init call is the first to
 * initialize the memory location and encodes its endianness in the
 * field.
 *
 * @note This code does not take into account alignment issues. We may
 * want to do that to force each message to be aligned to an 8-byte
 * boundary!
 *
 * @param q \b In. The NETIO_SHMEM_ConcurrentQueue data structure to be initialized.
 *
 * @param messageCountMax \b In. The maximum number of messages in the
 * concurrent queue.
 *
 * @param messageSizeMax \b In. The maximum size of a message in the
 * concurrent queue.
 *
 * @param memAddress. \b In. The address of the memory location that will be
 * used for NETIO_SHMEM_ConcurrentQueueState, an integer array containing
 * the sizes of the messages and a buffer for the messages.
 *
 * @param memAddressNumBytes \b In. The size in bytes of the passed
 * memory location.
 *
 * @see NETIO_SHMEM_ConcurrentQueue_reset
 */
RTIBool
NETIO_SHMEM_ConcurrentQueue_attach(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        char *memAddress);

void
NETIO_SHMEM_ConcurrentQueue_finish_write(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int finished_hndl,
        int msg_size);

/*i
 * \ingroup  NETIO_SHMEM_ConcurrentQueue
 * \verbatim
 * State Reads:  msgEmptyIndex, msgReadIndex, bufferReadIndex
 *
 * State Writes: msgReadIndex, bufferReadIndex
 * msgInfos[msgReadIndex]._cookie
 * Reserves:
 * q->_msgInfos[msgReadIndex]
 * q->_buffer[bufferReadIndex]
 *
 * \endverbatim
 *
 * This function reads a message from the buffer as long as
 * there are messages to be read (i.e. the _bufferReadIndex is not
 * equal to _msgEmptyIndex) and q->_buffer[_bufferReadIndex]
 * contains a fully written message. The last condition is denoted
 * by having q->_msgInfos[_msgReadIndex]._size > 0.
 *
 * This function ensures that the new bufferReadIndex is aligned as required
 * by NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES.
 *
 * @note As per implementation, if the message in q->_buffer[_bufferReadIndex]
 * is not valid and there are valid messages ready to be read in
 * any q->_buffer[_bufferReadIndex+1, ..., _bufferEmptyIndex],
 * then no message will be read. This is because _bufferReadIndex will
 * not advance beyond q->_buffer[_bufferReadIndex] until the latter
 * contains a valid message.
 */
int
NETIO_SHMEM_ConcurrentQueue_start_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int *finished_hndl,
        char **whereFrom,
        unsigned int readerCookie);

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * @pre The finishedHandle verifies (modulus wrapping)
 *
 * \code
 * msgInUseIndex <= finishedHandle < msgReadIndex
 * \endcode
 *
 * This function will first mark the message indexed as finishedHandle
 * and being "finished" and then will advance msgInUseIndex and bufferReadIndex
 * from its current value as far as it can go (i.e. advancing over all
 * finished messages). It will stop when it finds an un-finished message
 * or reaches bufferReadIndex.
 *
 * @note It would be possible to make this thread-safe if we moved all the
 * bufferInUse and msgInUseIndex management code elsewhere, protected
 * by a mutex. For instance it could be moved to the startWrite().
 * This would make some sense because its also better not to try to
 * combine the buffers until we need them anyway. This function would
 * be so simple then that it can be done as a macro. However, given
 * that a common use case is to have the Queue memory in shared
 * memory and have it co-located with the reader, it is better to
 * keep as much code as possible n the read side because it will be
 * accessing local memory which is significantly faster if going over
 * a bus or a shated fabric...
 *
 * @note (GPC) From the above reasoning it seems like it would make
 * the most sense to move the *InUse logic to the
 * NETIO_SHMEM_ConcurrentQueue_startReadEA() this is in the reader side, its
 * protected by the same READ_EA, and has the advantage of being the
 * time where we need to see if a new message is available for
 * reading.
 *
 * \verbatim
 * State Reads:  bytesFullyReadCounter, bufferInUseIndex,
 * msgInUseIndex, msgReadIndex
 *
 * State Writes: bytesFullyReadCounter, bufferInUseIndex, msgInUseIndex
 * q->_msgInfos[finishedHandle]._msgSize
 *
 * Releases:
 * q->_msgInfos[msgInUseIndex..msgReadIndex]        (potentially)
 * q->_buffer[bufferInUseIndex .. bufferReadIndex]  (potentially)
 *
 * \endverbatim
 *
 * This function ensures that the new bufferInUseIndex is aligned as required
 * by NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES
 */
void
NETIO_SHMEM_ConcurrentQueue_finish_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int finished_hndl);

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * \verbatim
 * State Reads:  bytesWrittenCounter, bytesFullyReadCounter,
 * msgInUseIndex, msgEmptyIndex, bufferEmptyIndex
 *
 * State Writes: msgEmptyIndex, bufferEmptyIndex,
 * bytesWrittenCounter,
 * msgInfos[r__msgEmptyIndex]
 *
 * Reserves:
 * q->_msgInfos[msgEmptyIndex]
 * q->_buffer[bufferEmptyIndex .. bufferEmptyIndex+msgSize]
 * \endverbatim
 *
 * The way a buffer corresponding to msgEmptyIndex is reserved is that
 * _msgInfos[msgEmptyIndex]._size is set to NETIO_SHMEM_CONCURRENT_QUEUE_SIZE_INVALID.
 *
 * This function takes care of NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES when
 * updating bufferEmptyIndex so this index is always (a multiple of
 * NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES) and thus indicates aligned memory.
 */
RTIBool
NETIO_SHMEM_ConcurrentQueue_start_write_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int *finished_hndl,
        char **where_to,
        int msg_size,
        unsigned int cookie);

#define NETIO_SHMEM_ConcurrentQueue_get_queue_desc(q, descOut) *(descOut) = (q)->_desc

#define swap4ByteUIntArg(x) ( ((x) << 24) | (((x) & 0xff00) << 8) | (((x) & \
    0xff0000) >> 8) | ((x) >> 24) )

#define swap4Bytes(x) swap4ByteUIntArg( ((unsigned int)(x)) )

#define NETIO_SHMEM_ConcurrentQueueVersion_is_v3_or_lower(__version) \
    ((__version).major < NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT.major \
        ? RTI_TRUE \
        : (__version).minor < NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT.minor)


void
NETIO_SHMEM_ConcurrentQueue_finish_write(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int finished_hndl,
        int msg_size);

#define NETIO_SHMEM_ConcurrentQueue_alignBufferIndex(index) \
    NETIO_SHMEM_ConcurrentQueue_alignSize(index, NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES)

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Tests whether data would wrap if it dataSize were to be written at index
 * dataIndex in the data-buffer.
 */
RTIBool
NETIO_SHMEM_ConcurrentQueue_buffer_will_wrap(
        const struct NETIO_SHMEM_ConcurrentQueueHandle *q,
        int data_index,
        int data_size);

/*i \ingroup  NETIO_SHMEM_ConcurrentQueueClass
 *
 * Defines the number of overhead bytes for a given messageSizeMax, and
 * messageCountMax.
 *
 * The space requirements include:
 *
 * The overhead includes:
 * \code
 * sizeof(struct NETIO_SHMEM_ConcurrentQueueHeader)
 * sizeof(int)-1
 * sizeof(int msgSizes[messageCountMax+1])
 * (messageCountMax+1)*(NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES-1)
 * (NETIO_SHMEM_CONCURRENT_QUEUE_MSG_ALIGN_BYTES-1)
 * messageSizeMax
 * \endcode
 *
 * This is conservative but simplifies the implementation.
 *
 * This function must be consistent with the pointer arithmetic performed
 * by NETIO_SHMEM_ConcurrentQueue_attach()
 */
RTI_INT32
NETIO_SHMEM_ConcurrentQueue_get_overhead(
        int msg_size_max,
        int msg_count_max,
        const struct NETIO_SHMEM_ConcurrentQueueVersion *queue_version);

#define NETIO_SHMEM_ConcurrentQueueDesc_SIZEOF_64_BIT_POINTER 8

#define NETIO_SHMEM_ConcurrentQueue_alignSize(rawSize, alignmentBytes) \
    (((rawSize) + (alignmentBytes - 1)) & (~(alignmentBytes - 1)))


struct NETIO_SHMEM_ConcurrentQueueHandle;

/*i \ingroup  NETIO_SHMEM_ConcurrentQueue
 *
 * Returns the count of "empty" bytes (bytes available to write new messages)
 * in the buffer.
 *
 * This function does not consider available bytes from messages that
 * have been read but not finished, nor messages that have been read &
 * finished but have previous messages that have not been finished.
 *
 * This function returns the number of "empty" bytes subject to the
 * limitations on the maxDataBytes deduced from the size of the underlying
 * buffer and the overhead.
 *
 * This function does not take into consideration the alignment that
 * can cause each message written to actually use more space. The reason for
 * this is that the worst-case extra space that can be consumed by this alignment
 * (maxMessages)*(MSG_ALIGMNET-1) is already accounted as overhead by the
 * function NETIO_SHMEM_ConcurrentQueue_getOverhead() so the result of looking just at
 * the bytes written-read is ends up being a conservative estimate of the size
 * available.
 *
 * This function assumes that maxDataBytes < 2^32/2 to ensure
 * race-free operation.
 *
 * This function does not take any locks and therefore returns a
 * "snapshot" of the available bytes. Notice that the implementation
 * is safe because we only read each counter once.
 *
 * The only way availableSize > NETIO_SHMEM_ConcurrentQueue_getSize(q) is if the
 * number is actually negative (which as an unsigned is a huge number)
 *
 * @param availableSizePtr \b out. Pointer to int to hold the answer,
 * which is the number of bytes available in the
 * buffer, subject to the limitation imposed by maxDataBytes
 *
 * @param bytesWrittenCounter \b in. Must be an unsigned int and match
 * NETIO_SHMEM_ConcurrentQueue.bytesWrittenCounter. Represents the number of bytes
 * written so far subject to unsigned int wrap-around.
 *
 * @param bytesReadCounter \b in. Must be an unsigned int and match
 * NETIO_SHMEM_ConcurrentQueue.bytesFullyReadCounter. Represents the number of bytes
 * completely read out of the buffer subject to unsigned int
 * wrap-around.
 *
 * @param maxDataBytes \b in. Must be an int and match
 * NETIO_SHMEM_ConcurrentQueue_getSize(q). Represents the maximum number of data
 * bytes the buffer is allowed to hold.
 */
void
NETIO_SHMEM_ConcurrentQueue_get_available_size(
        int *availableSizePtr,
        unsigned int bytesWrittenCounter,
        unsigned int bytesFullyReadCounter,
        int maxDataBytes);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* reda_concurrentQueue_impl_h */
