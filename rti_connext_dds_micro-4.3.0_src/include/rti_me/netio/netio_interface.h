/*
 * FILE: netio_interface.h - NETIO API
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 17dec2015,eh MICRO-1511: add NETIO_PacketInfo.valid_key
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 03feb2014,eh MICRO-714  Change ack to acknack
 * 27jan2014,eh MICRO-714  Add post_event. For reliable reader activity change
 * 17may2013,eh MICRO-385  Remove unused fns/fields of packet
 * 25apr2012,tk Written
 */

/*ce
 * \file
 * \brief Generic NETIO_Interface functions
 *
 * \details
 * This file contains generic NETIO interface functions. A NETIO_Interface is
 * a base-class and is typically not instantiated on its own. Derived classes
 * call methods here to initialize/finalize a base-class. This file also
 * defines the NETIO interface structure as well as the NETIO_Packet structure
 * and protocol types.
 */
#ifndef netio_interface_h
#define netio_interface_h

#ifndef netio_config_h
#include "netio/netio_config.h"
#endif

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif

#ifndef db_api_h
#include "db/db_api.h"
#endif

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef netio_address_h
#include "netio/netio_address.h"
#endif

#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif

#ifndef netio_sample_access_h
#include "netio/netio_sample_access.h"
#endif

#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ce
 * \brief Default name for the automatically registered UDP transport
 */
NETIODllVariable extern const char* const NETIO_DEFAULT_UDP_NAME;

/*ce
 * \brief Default name for the automatically registered INTRA transport
 */
NETIODllVariable extern const char* const NETIO_DEFAULT_INTRA_NAME;

/*ce
 * \brief Default name for the automatically registered shared
 *  memory transport
 */
NETIODllVariable extern const char* const NETIO_DEFAULT_SHMEM_NAME;

/*ce
 * \brief Default name for the automatically registered RTPS interface
 */
NETIODllVariable extern const char* const NETIO_DEFAULT_RTPS_NAME;

/*e
 * \defgroup  NETIO_PacketClass NETIO Packet API
 * \ingroup NETIO
 * \brief NETIO_Packet implementation
 */
/*ce
 * \addtogroup NETIO_PacketClass
 * @{
 */
/*
 * The following constants are passed as part of a NETIO_Packet to
 * indicate what protocol it originated from. It may be used the receiver
 */
/*ci
 * \def   NETIO_PROTOCOL_INTRA
 * \brief The packet originates from the INTRA protocol
 */
#define NETIO_PROTOCOL_INTRA               (1)

/*ci
 * \def   NETIO_PROTOCOL_RTPS
 * \brief The packet originates from the RTPS protocol
 */
#define NETIO_PROTOCOL_RTPS                (2)

/*ci
 * \def   NETIO_PROTOCOL_UDP
 * \brief The packet originates from the UDP protocol
 */
#define NETIO_PROTOCOL_UDP                 (3)

/*ci
 * \def   NETIO_PROTOCOL_NOTIF
 * \brief The packet originates from the NOTIF protocol
 */
#define NETIO_PROTOCOL_NOTIF               (4)

/*
 * The following constants are passed as part of a NETIO_Packet to
 * provide meta information about the content (if any).
 */
/*ci
 * \def   NETIO_RTPS_FLAGS_DEFAULT
 * \brief The packet has default flags set
 */
#define NETIO_RTPS_FLAGS_DEFAULT           (0x0U)

/*ci
 * \def   NETIO_RTPS_FLAGS_DISPOSE
 * \brief The packet is a RTPS dispose message, a key is disposed
 */
#define NETIO_RTPS_FLAGS_DISPOSE           (0x1U)

/*ci
 * \def   NETIO_RTPS_FLAGS_UNREGISTER
 * \brief The packet is a RTPS unregister message, a key has been unregistered
 *        by a writer
 */
#define NETIO_RTPS_FLAGS_UNREGISTER        (0x2U)

/*ci
 * \def   NETIO_RTPS_FLAGS_LIVELINESS
 * \brief The packet counts as liveliness, even if it does not contain any data
 */
#define NETIO_RTPS_FLAGS_LIVELINESS        (0x4U)

/*ci
 * \def  NETIO_RTPS_FLAGS_INLINEQOS
 * \brief The packet contains inline Qos as defined by RTPS
 */
#define NETIO_RTPS_FLAGS_INLINEQOS         (0x8U)

/*ci
 * \def  NETIO_RTPS_FLAGS_LOST_DATA
 * \brief The flag indicates that some packets have been permanently lost
 *        and any attempt to wait for them should be aborted
 */
#define NETIO_RTPS_FLAGS_LOST_DATA         (0x10U)

/*ci
 * \def  NETIO_RTPS_FLAGS_COMMIT_DATA
 * \brief The flag indicates that even if the packet does not contain
 *        data, it does contain information about which packets can be considered
 *        to be received in order.
 */
#define NETIO_RTPS_FLAGS_COMMIT_DATA       (0x20U)

/*ci
 * \def  NETIO_RTPS_FLAGS_DATA
 * \brief The flag indicates that the packet contains data
 */
#define NETIO_RTPS_FLAGS_DATA              (0x40U)

/*ci
 * \def  NETIO_RTPS_FLAGS_LAST_WRITE_FOR_SN
 * \brief
 */
#define NETIO_RTPS_FLAGS_LAST_WRITE_FOR_SN (0x80U)

/*ci
 * \def  NETIO_RTPS_FLAGS_LITTLE_ENDIAN
 * \brief The packet payload is in little endian format
 */
#define NETIO_RTPS_FLAGS_LITTLE_ENDIAN     (0x100U)

/*ci
 * \def  NETIO_RTPS_FLAGS_DATA_BATCH
 * \brief The flag indicates that the packet contains batch data
 */
#define NETIO_RTPS_FLAGS_DATA_BATCH        (0x200U)

/*ci \brief In progress status info bit */
#define NETIO_RTPS_FLAGS_IN_PROGRESS       (0x400U)

/*ci \brief removed status info bit */
#define NETIO_RTPS_FLAGS_REMOVED           (0x800U)

/*ci
 * \def  NETIO_RTPS_FLAGS_DATA_BATCH_FIRST
 * \brief The flag indicates that the packet contains the first
 *        sample in a batch
 */
#define NETIO_RTPS_FLAGS_DATA_BATCH_FIRST   (0x1000U)

/*ci
 * \def   NETIO_RTPS_FLAGS_TRUST_LOAN
 * \brief The packet has in operating on a loaned buffer for trust
 */
#define NETIO_RTPS_FLAGS_TRUST_LOAN         (0x2000U)

/*ci
 * \def   NETIO_RTPS_FLAGS_AUTO_LIVELINESS
 * \brief The packet counts as liveliness. This packet does not have
 * any data just a notification that a participant message with
 * AUTOMATIC liveliness was received.
 */
#define NETIO_RTPS_FLAGS_AUTO_LIVELINESS    (0x4000U)

/*ci
 * \def   NETIO_RTPS_FLAGS_MANUAL_LIVELINESS
 * \brief The packet counts as liveliness. This packet does not have
 * any data just a notification that a participant message with
 * MANUAL_BY_PARTICIPANT liveliness was received.
 */
#define NETIO_RTPS_FLAGS_MANUAL_LIVELINESS  (0x8000U)

/*ci
 * \def   NETIO_RTPS_FLAGS_HAS_INLINE_KEY
 * \brief The packet has an inline key
 */
#define NETIO_RTPS_FLAGS_HAS_INLINE_KEY     (0x10000U)

/*ci
 * \def NETIO_RTPS_FLAGS_PACKET_LOANED
 *
 * \brief The packet is loaned
 */
#define NETIO_RTPS_FLAGS_RETURN_PACKET_ONLY (0x20000U)

/*ci
 * \brief RTPS protocol specific data
 */
struct NETIO_RtpsInfo
{
    /*ci
     * \brief Pointer to beginning of data-payload
     * This buffer is modified to point to the end of inline_data; this is
     * needed to read the next sample_info in case a batch was received
     */
    struct REDA_Buffer *inline_data;

    /*ci
     * Pointer to the beginning of serialized sample data when a batch is
     * received. When the first sample in the batch is read this will
     * point to the encapsulation id; for the following samples this will
     * point to the beginning of serialized data
     */
    struct REDA_Buffer *serialized_data_batch;

    /*ci \brief Endianness of stream. Value is only valid if value of field
     * stream_need_byte_swap is not CDR_BYTESWAP_INVALID.
     * This is needed so header in the stream is deserialized only once.
     */
    RTI_UINT16 stream_endian;

    /*ci
     * \brief The GUID prefix of the peer
     */
    struct NETIO_GuidPrefix guid_prefix;
};

/*ci
 * \brief INTRA protocol specific data
 */
struct NETIO_IntraInfo
{
    /*ci
     * \brief opaque pointer to in-memory representation of data
     */
    const void *user_data;

    /*ci
     * \brief Pointer to beginning of intra data-payload. Intra data-payload
     *        only contains inline Qos
     */
    struct REDA_Buffer *inline_data;
};

/*ci
 *\brief NETIO_IntraInfo initializer constant
 */
#define NETIO_IntraInfo_INITIALIZER \
{ \
    NULL, /* user_data */ \
    NULL, /* inline_data */ \
}

/*ci
 * \brief Opaque Info
 */
struct NETIO_OpaqueInfo
{
    /*ci
     * \brief opaque pointer to a protocol specific reader. Only understood by the
     * specific netio interface
     */
    void* reader;

    /*ci
     * \brief An interface specific 32 bit number.
     */

    RTI_UINT32 opaque_id_32;

     /*ci
     * \brief An interface specific 64 bit  number.
     */
    RTI_UINT64 opaque_id_64;
};

/*ci
 *\brief #define NETIO_OpaqueInfo initializer constant
 */
#define NETIO_OpaqueInfo_INITIALIZER \
{ \
    NULL, /* reader */ \
    0, /* opaque_id_32 */ \
    0 /* opaque_id_64 */ \
}


/*ci
 *\brief NOTIF protcol specific data
 */
struct NETIO_NotifInfo
{
    /*ci
     * \brief opaque info for the notification interface
     */
    struct NETIO_OpaqueInfo opaque_data;

    /*ci
     * \brief sample accessor interface to retrieve and return samples.
     */
    NETIO_SampleI *sample_accesor;
};

/*ci
 *\brief #define NETIO_NotifInfo initializer constant
 */
#define NETIO_NotifInfo_INITIALIZER \
{ \
    NETIO_OpaqueInfo_INITIALIZER, /* opaque_data */ \
    NULL /* sample_accesor */ \
}

/*ci
 *\brief Structure containing meta-data about a \ref NETIO_Packet payload
 */
struct NETIO_PacketInfo
{
    /*ci
     * \brief The SN of the packet. The SN and PacketId is synonymous.
     * For a batch data this will be the batch SN (not the sample inside
     * the batch SN).
     */
    struct REDA_SequenceNumber sn;

    /*ci
     * \brief The virtual SN of the sample.
     * If a batch is received 'sn' contains the RTPS packet SN (same for
     * all samples in the batch), while 'virtual_sn' contains the user sample
     * SN. When a sample is received 'sn' == 'virtual_sn'
     */
    struct REDA_SequenceNumber virtual_sn;

    /*ci
     * \brief The reception time-stamp of the packet
     */
    OSAPI_SystemTime timestamp;

    /*ci
     * \brief Whether the serialized-payload is valid data or not
     */
    RTI_UINT8 valid_data;

    /*ci
     * \brief Whether the serialized-payload is a valid key or not
     */
    RTI_UINT8 valid_key;

    /*ci
     * \brief Key for the sample
     */
    struct NETIO_Guid instance;

    /*ci
     * \brief Flags describing the contents of the payload
     */
    RTI_UINT32 rtps_flags;

    /*ci
     * \brief The committable_sn means that all SNs up to, but not including
     *        this SN can be committed (made available to the user).
     */
    struct REDA_SequenceNumber committable_sn;

    /*ci
     * \brief The first_available_sn indicates the first available SN which is
     *        available. It is typically used downstream to boot-strap the
     *        reliability protocol.
     */
    struct REDA_SequenceNumber first_available_sn;

    /*ci
     * \brief If NETIO_RTPS_FLAGS_LOST_DATA is set, this is the highest sequence
     *        number which has been lost.
     */
    struct REDA_SequenceNumber lost_sample_sn;

    /*ci
     * \brief If NETIO_RTPS_FLAGS_LOST_DATA is set, this is the total number
     *        of samples that have been lost up to and including lost_sample_sn
     */
    RTI_INT32 lost_sample_count;

    /*ci
     * \brief Virtual SN of the last committed sample.
     */
    struct REDA_SequenceNumber last_committed_virtual_sn;

    /*ci
     * \brief The protocol which produced this packet, it may change as the
     *        packet traverses a NETIO stack
     */
    RTI_INT32 protocol_id;

    /*ci
     * \brief The vendor id sending this packet
     */
    RTI_UINT8 vendor_major_id;

    /*ci
     * \brief The vendor product sending this packet
     */
    RTI_UINT8 vendor_minor_id;

    /*ci
     * \brief checksum information for this packet
     */
    RTI_UINT32 checksum_info;

    void *queue_entry;

    RTI_UINT16 encapsulation;

    /*ci
     * \brief Protocol specific data based, protocol_id identifies which one
     *        is valid.
     */
    struct
    {
        struct NETIO_IntraInfo intra_info;
        struct NETIO_RtpsInfo rtps_data;
        struct NETIO_NotifInfo notif_info;
    } protocol_data;

    /*ci
     *  \brief Transport priority provided as hint to the transport layer
     */
    RTI_INT32 transport_priority;

    RTI_INT32 originating_transport;
};

/*ci
 * \def NETIO_PacketInfo_INITIALIZER
 * \brief Constant to initialize NETIO_PacketInfo
 */
#define NETIO_PacketInfo_INITIALIZER \
{ \
    REDA_SEQUENCE_NUMBER_ZERO, /* sn */\
    REDA_SEQUENCE_NUMBER_ZERO, /* virtual_sn */\
    OSAPI_TIME_ZERO, /* timestamp */ \
    0, /* valid_data */ \
    0, /* valid_key */ \
    NETIO_ADDRESS_GUID_UNKNOWN, /* instance */ \
    0, /* flags */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* committable_sn */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* first_available_sn */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* lost_sample_sn */ \
    0, /* lost sample count */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* last_committed_virtual_sn */ \
    0, /* protocol_id */\
    0,0, /* vendor ID */\
    0, /* checksum protected */\
    NULL, /* queue_entry */\
    0,\
    {{NULL,NULL},\
     {NULL,NULL,0,\
      {{0,0,0,0,0,0,0,0,0,0,0,0}}},\
      NETIO_NotifInfo_INITIALIZER},\
    0, /* transport_priority */\
    0 /*originating_transport*/ \
}

/*ci
 * \brief The PacketBuffer structure
 *
 * \details
 * The packet-buffer definition is the smallest unit of packet information.
 * A linked list of packet buffers makes up a single logical payload. It is not
 * legal to change the contents of a packet-buffer other than by the owner.
 */
typedef struct NETIO_PacketBuffer
{
    struct NETIO_PacketBuffer *_next;

    /*ci
     * \brief Pointer to the original beginning of the buffer.
     */
    char *buffer;

    /*ci
     * \brief The maximum length of the buffer.
     */
    RTI_SIZE_T max_length;

    /*ci
     * \brief Index to the first octet in the buffer.
     */
    RTI_UINT32 head_pos;

    /*ci
     * \brief Index to the first octet _after_ the last valid octet. The
     * total number of valid octets in buffer is thus tail_pos - head_pos.
     */
    RTI_UINT32 tail_pos;
} NETIO_PacketBuffer_T;

#define NETIO_PacketBuffer_INITIALIZER \
{\
    NULL,\
    NULL,\
    0,\
    0,\
    0\
}

/*ci
 * \brief Adjust the head cursor in the packet payload
 *
 * \details
 *
  * Adjust the head cursor in the packet payload. A positive value moves the
  * cursor forward, a negative value backwards.
  *
  * \param[in] packet Packet to adjust head cursor in
  * \param[in] delta  Adjustment to make
  *
  * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_PacketBuffer_set(struct NETIO_PacketBuffer *pbuf,
                       char *buffer,RTI_SIZE_T max_length,
                       RTI_UINT32 head_pos,RTI_UINT32 tail_pos);

NETIODllExport void
NETIO_PacketBuffer_reset(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport struct NETIO_PacketBuffer*
NETIO_PacketBuffer_get_next(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport struct NETIO_PacketBuffer*
NETIO_PacketBuffer_get_next_non_empty(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport void*
NETIO_PacketBuffer_get_head(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport void*
NETIO_PacketBuffer_get_tail(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport RTI_SIZE_T
NETIO_PacketBuffer_get_length(struct NETIO_PacketBuffer *pbuf);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_PacketBuffer_adjust_head(struct NETIO_PacketBuffer *pbuf,RTI_INT32 delta);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_PacketBuffer_adjust_tail(struct NETIO_PacketBuffer *pbuf,RTI_INT32 delta);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_PacketBuffer_link(struct NETIO_PacketBuffer *pbuf_before,
                        struct NETIO_PacketBuffer *pbuf_after);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_PacketBuffer_unlink(struct NETIO_PacketBuffer *pbuf_before,
                          struct NETIO_PacketBuffer *pbuf_after);

/*e \dref_NETIO_Packet
 * \brief Implementation of the abstract NETIO_Packet type.
 *
 * \details
 * The NETIO layers passed \ref NETIO_Packet structures upstream/downstream.
 * Any meta-data about the packet must be encapsulated either in the packet
 * payload or the packet info field.
 */
struct NETIO_Packet
{
    /*ci
     * \brief The source of the packet, typically the peer address
     */
    struct NETIO_Address source;

    /*ci
     * \brief The local source of the packet, the address it was received on.
     */
    struct NETIO_Address local_source;

    /*ci
     * \brief Buffer holding a NETIO_Packet. The buffer must be allocated
     *        and managed outside of this structure. That, is NETIO_Packet
     *        only manages payloads by reference.
     *
     *  \details
     *  This buffer is mainly for data reception which is always a contiguous
     *  chunk of data.
     */
    char *buffer;

    /*ci
     * \brief The maximum number of bytes buffer can hold
     */
    RTI_SIZE_T max_length;

    /*ci
     * \brief The current position of head cursor in buffer
     */
    RTI_SIZE_T head_pos;

    /*ci
     * \brief The current position of tail cursor in buffer
     */
    RTI_SIZE_T tail_pos;

    /*ci
     * \brief Temporary storage for the current head position in buffer
     *        so it can later be restored if a packet payload is traversed
     *        multiple times.
     */
    RTI_SIZE_T saved_head_pos;

    /*ci
     * \brief Temporary storage for the current tail position in buffer
     *        so it can later be restored if a packet payload is traversed
     *        multiple times.
     */
    RTI_SIZE_T saved_tail_pos;

    /*ci
     * \brief Meta data about the packet, refer to \ref NETIO_PacketInfo for
     *        details
     */
    struct NETIO_PacketInfo info;

    /*ci
     * \brief Sequence of destinations for single packet.  Memory owned by sender.
     */
    struct NETIO_AddressSeq *dests;

    /*ci
     * \brief Application level reference pointer to be able to retrieve
     *        content based on the NETIO_Packet.
     */
    void *ref;

    /*ci
     * \brief The first PacketBuffer in the NETIO_Packet.
     *
     * \details
     *  This buffer is mainly for data transmission which is a linked list of
     *  packet buffers.
     */
    struct NETIO_PacketBuffer *head_pbuf;

    /*ci
     * \brief The first PacketBuffer in the NETIO_Packet.
     */
    struct NETIO_PacketBuffer *tail_pbuf;

#if RTIME_UNITTEST_INCLUDE_SRCADDR
    /*ci
     * \
     */
    RTI_UINT32 udp_origin_addr;
#endif

};

/*e \dref_NETIO_Packet_T
 */
typedef struct NETIO_Packet NETIO_Packet_T;


#if RTIME_UNITTEST_INCLUDE_SRCADDR
#define NETIO_PacketInfo_udp_origin_addr_INITIALIZER ,0
#else
#define NETIO_PacketInfo_udp_origin_addr_INITIALIZER
#endif

/*ci
 * \brief Structure to save sufficient, but not complete, packet state
 *        needed to restore packets when sending.
 */
typedef struct NETIO_PacketState
{
    /*ci
     * \brief Shallow copy of the current state
     */
    NETIO_Packet_T shallow_copy;

    /*ci
     * \brief Copy of the content of the orignal pbuf_head. Needed to restore
     *        the original pbuf chain.
     */
    struct NETIO_PacketBuffer pbuf_head;

    /*ci
     * \brief Copy of the content of the orignal pbuf_tail. Needed to restore
     *        the original pbuf chain.
     */
    struct NETIO_PacketBuffer pbuf_tail;

} NETIO_PacketState_T;

/*ci
 * \brief Constant to initialize a \ref NETIO_PacketState
 */
#define NETIO_PacketState_INITIALIZER \
{\
    NETIO_Packet_INITIALIZER,\
    NETIO_PacketBuffer_INITIALIZER,\
    NETIO_PacketBuffer_INITIALIZER\
}

/*ci
 * \brief Every packet must have a unique ID. Since RTPS SN does not wrap around,
 *        it is safe to use as a unique ID per data-writer. It also avoids having
 *        to map a between packet id to a RTPS SN.
 */
typedef struct REDA_SequenceNumber NETIO_PacketId_T;

/*ci
 * \def NETIO_Packet_INITIALIZER
 * \brief Constant to initialize a \ref NETIO_Packet
 */
#define NETIO_Packet_INITIALIZER \
{\
    NETIO_Address_INITIALIZER,\
    NETIO_Address_INITIALIZER,\
    NULL, /* buffer */\
    0, /* max_length */\
    0, /* head_pos */\
    0, /* tail_pos */\
    0, /* saved_head_pos */\
    0, /* saved_tail_pos */\
    NETIO_PacketInfo_INITIALIZER,\
    NULL /* dests */,\
    NULL /* ref */,\
    NULL,\
    NULL\
    NETIO_PacketInfo_udp_origin_addr_INITIALIZER \
}

/*i
 * \brief Adjust the head cursor in the packet payload
 *
 * \details
 *
  * Adjust the head cursor in the packet payload. A positive value moves the
  * cursor forward, a negative value backwards.
  *
  * \param[in] packet Packet to adjust head cursor in
  * \param[in] delta  Adjustment to make
  *
  * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_set_head(NETIO_Packet_T *const packet,RTI_INT32 delta);

/*ci
 * \brief Adjust the tail cursor in the packet payload
 *
 * \details
 *
  * Adjust the tail cursor in the packet payload. A positive value moves the
  * cursor forward, a negative value backwards.
  *
  * \param[in] packet Packet to adjust tail cursor in
  * \param[in] delta  Adjustment to make
  *
  * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_set_tail(NETIO_Packet_T *const packet,RTI_INT32 delta);

/*e \dref_NETIO_Packet_get_head
 */
MUST_CHECK_RETURN NETIODllExport void*
NETIO_Packet_get_head(const NETIO_Packet_T *const packet);

/*ci
 * \brief Get a pointer to the tail head
 *
 * \details
 *
 * Get a pointer to the current tail. Note that it can be assumed that the
 * payload buffer is contiguous.
 *
 * \param[in] packet Packet to get the tail pointer for
 *
 * \return pointer to packet tail
 *
 * \sa \ref NETIO_Packet_set_tail
 */
MUST_CHECK_RETURN NETIODllExport void*
NETIO_Packet_get_tail(const NETIO_Packet_T *const packet);

/*ci
 * \brief Initialize a packet with a payload buffer of the given length
 *
 * \details
 *
 * NETIO_Packet only manipulates payloads by reference. The actual buffer
 * holding a payload must be managed outside of the packet structure. A
 * payload buffer is assigned to the packet with the length and an initialized
 * destination sequence.
 *
 * \param[in] packet         Packet structure to initialize
 * \param[in] init_buffer    Payload buffer
 * \param[in] init_length    Initial length in bytes of the buffer
 * \param[in] trailer_length The initial trail position in the payload
 * \param[in] dest_seq       Initialized sequence of destination address for
 *                           the packet
 *
 * \return RTI_TRUE on successful initialization, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_initialize(NETIO_Packet_T *const packet,
                       void *init_buffer,
                       RTI_SIZE_T init_length,
                       RTI_SIZE_T trailer_length,
                       struct NETIO_AddressSeq *dest_seq);

/*ci
 * \brief Set the payload buffer of a packet
 *
 * \details
 *
 * Set the payload buffer of a packet. The buffer is not copied, but only
 * referenced. The caller must ensure that the buffer is valid for the
 * lifetime of the packet.
 *
 * \param[in] packet          Packet structure to initialize
 * \param[in] buffer          The buffer to use
 * \param[in] max_buffer_length The maximum length of the buffer
 * \param[in] head_pos        Offset of first valid byte
 * \param[in] tail_pos        Offset of first unused byte. If head==tail
 *                            the buffer is empty.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
NETIODllExport RTI_BOOL
NETIO_Packet_set_buffer(NETIO_Packet_T *packet,
                        void *const buffer,
                        RTI_SIZE_T max_buffer_length,
                        RTI_SIZE_T head_pos,
                        RTI_SIZE_T tail_pos);

/*e \dref_NETIO_Packet_set_payload
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_set_payload(NETIO_Packet_T *const packet,
                                void *buffer,
                                RTI_INT32 buffer_length);

/*ci
 * \brief Initialize a packet with a payload buffer of the given length
 *        and state from an existing buffer
 *
 * \details
 *
 * \param[in] out_packet        Packet structure to initialize
 * \param[in] in_packet         Existing packet to copy state from
 * \param[in] buffer            The buffer to use
 * \param[in] max_buffer_length The maximum length of the buffer
 * \param[in] head_pos          Offset of first valid byte
 * \param[in] tail_pos          Offset of first unused byte. If head==tail
 *                              the buffer is empty.
 *
 * \return RTI_TRUE on successful initialization, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_initialize_from(NETIO_Packet_T *const out_packet,
                             const NETIO_Packet_T *const in_packet,
                             void *const buffer,
                             RTI_SIZE_T max_buffer_length,
                             RTI_SIZE_T head_pos,
                             RTI_SIZE_T tail_pos);

#ifndef RTI_CERT
/*ci
 * \brief Finalize a packet structure
 *
 * \details
 *
 * Finalize a packet structure
 *
 * \param[in] packet Packet structure to finalize
 *
 * \return RTI_TRUE on successful initialization, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_finalize(NETIO_Packet_T *const packet);
#endif /* !RTI_CERT */

/*ci
 * \brief Get pointer to the packet info
 * *
 * \param[in] packet Packet structure to get packet info for
 *
 * \return Pointer to packet info
 */
MUST_CHECK_RETURN NETIODllExport struct NETIO_PacketInfo*
NETIO_Packet_get_info(NETIO_Packet_T *const packet);

/*ci
 * \brief Set the source address of a packet
 *
 * \param[in] packet Packet to set source address in
 * \param[in] src    Source address of packet
 */
NETIODllExport void
NETIO_Packet_set_source(NETIO_Packet_T *const packet,
                        const struct NETIO_Address *const src);

/*e \dref_NETIO_Packet_get_payload_length
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_SIZE_T
NETIO_Packet_get_payload_length(const NETIO_Packet_T *const packet);

/*ci
 * \brief Save the current head and tail positions
 *
 * \details
 *
 * Save the current head and tail positions
 *
 * \param[in] packet Packet to save the head and tail positions for
 *
 *\sa \ref NETIO_Packet_restore_positions
 *
 */
NETIODllExport void
NETIO_Packet_save_positions(NETIO_Packet_T *const packet);

/*ci
 * \brief Restore the saved head and tail positions
 *
 * \details
 *
 * Restore the saved head and tail positions
 *
 * \param[in] packet Packet to restore the head and tail positions for
 *
 * \sa \ref NETIO_Packet_save_positions
 */
NETIODllExport void
NETIO_Packet_restore_positions(NETIO_Packet_T *const packet);

/*ci
 * \brief Save the saved head and tail positions to external variables
 *
 * \details
 *
 * Save the saved head and tail positions to external variables
 *
 * \param[in]  packet Packet to save the head and tail positions for
 * \param[out] head   Contains current head position for packet on return
 * \param[out] tail   Contains current tail position for packet on return
 *
 * \sa \ref NETIO_Packet_restore_positions_from
 */
NETIODllExport void
NETIO_Packet_save_positions_to(const NETIO_Packet_T *const packet,
                               RTI_SIZE_T *const head,
                               RTI_SIZE_T *const tail);

/*ci
 * \brief Restore the saved head and tail positions to external variables
 *
 * \details
 *
 * Restore the saved head and tail positions from external variables
 *
 * \param[in] packet Packet to restore the head and tail positions for
 * \param[in] head   Set head position for packet
 * \param[in] tail   Set tail position for packet
 *
 * \sa \ref NETIO_Packet_save_positions_to
 */
NETIODllExport void
NETIO_Packet_restore_positions_from(NETIO_Packet_T *const packet,
                                    RTI_SIZE_T head,
                                    RTI_SIZE_T tail);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Packet_set_pbuf(NETIO_Packet_T *packet,
                      struct NETIO_PacketBuffer *pbuf_head,
                      struct NETIO_PacketBuffer *pbuf_tail);

/*ci
 * \brief Save the saved packet state
 *
 * \details
 *
 * Save the saved head and tail positions to external variables
 *
 * \param[in]  packet Packet to save the head and tail positions for
 * \param[out] head   Contains current head position for packet on return
 * \param[out] tail   Contains current tail position for packet on return
 *
 * \sa \ref NETIO_Packet_restore_positions_from
 */
NETIODllExport void
NETIO_Packet_save_state(const NETIO_Packet_T *const packet,
                        NETIO_PacketState_T *state);

/*ci
 * \brief Restore the saved packet state
 *
 * \details
 *
 * Restore the packet with the saved state p
 *
 * \param[in] packet Packet to restore the head and tail positions for
 * \param[in] state  Packet state to restore
 *
 * \sa \ref NETIO_Packet_save_state
 */
NETIODllExport void
NETIO_Packet_restore_state(NETIO_Packet_T *packet,
                           const NETIO_PacketState_T *const state);


/*ci
 * \brief Get the buffer of a packet
 *
 * \details
 *
 * Get the buffer of a packet. The buffer is not copied, but only referenced.
 * The caller must ensure that the buffer is valid for the lifetime of the
 * packet.
 *
 * \param[in] packet Packet to get the buffer for
 *
 * \return pointer to packet buffer
 */
NETIODllExport void*
NETIO_Packet_get_buffer(NETIO_Packet_T *packet);

/*ci
 * \brief Return TRUE if the packet is a Ping message from Connext Pro or Micro
 *
 * \param[in] packet Packet to test
 *
 * \return TRUE if the packet is a Ping message, FALSE if not.
 */
NETIODllExport RTI_BOOL
NETIO_Packet_is_ndds_ping(const NETIO_Packet_T *packet);

/*ce @} */

/*e
 * \defgroup NETIO_InterfaceClass NETIO Interface API
 * \ingroup NETIO
 */
/*ci \addtogroup NETIO_InterfaceClass
 *   @{
 */

/*ci
 * \brief The required number of bytes to convert an component id and
 *        counter to a table name
 */
#define NETIO_TABLE_NAME_SIZE 17

/*ci
 *  \brief NETIO base-class property
 */
struct NETIO_InterfaceProperty
{
    struct RT_ComponentProperty _parent;

    /*ci \brief The maximum number of outgoing routes for the interface
     */
    RTI_UINT32 max_routes;

     /*ci \brief The maximum number of peer interfaces
     */
    RTI_UINT32 max_binds;

    /*ci \brief The address of the interface
     */
    struct NETIO_Address intf_addr;

    /*ci
     * \brief Mutex which can be used to independently protect
     *        network traffic from the upstream database.
     */
    struct OSAPI_Mutex *network_lock;

    /*ci \brief Shared packet pool
     */
    REDA_BufferPool_T packet_pool;
};

/*ci
 * \brief Constant to initialize \ref NETIO_InterfaceProperty
 */
#define NETIO_InterfaceProperty_INITIALIZER \
{\
    RT_ComponentProperty_INITIALIZER, \
    0, \
    0, \
    NETIO_Address_INITIALIZER, \
    NULL, \
    NULL \
}

/*ci
 * \brief NETIO Transport Properties
 */
struct NETIO_TransportProperty
{
    RTI_UINT32 send_size_max;
    RTI_UINT32 recv_size_max;
};

/*ci
 * \brief Constant to initialize \ref NETIO_TransportProperty
 */
#define NETIO_TransportProperty_INITIALIZER \
{\
    0,\
    0\
}

/*ci
 * \brief NETIO listener structure
 *
 * \details
 *
 * All NETIO interface are RT Components, and all NETIO layers derives
 * the listener class from the RT_ComponentListener.
 */
struct NETIO_InterfaceListener
{
    /*ci
     * \brief Derived from member
     */
    struct RT_ComponentListener _parent;
};

/*ci
 * \def   NETIO_InterfaceListener_INITIALIZE
 * \brief Constant to initialize \ref NETIO_InterfaceListener
 */
#define NETIO_InterfaceListener_INITIALIZE \
{\
    RT_ComponentListener_INITIALIZER \
}

/*ci
 * \brief Common NETIO Interface factory properties
 *
 * \details
 *
 * All NETIO layers are implemented as RT components. All NETIO layers
 * are created from NETIO factories, and all NETIO factories are RT
 * component factories that share common properties inherited from
 * this structure.
 */
struct NETIO_InterfaceFactoryProperty
{
    /*ci
     * \brief derived from member
     */
    struct RT_ComponentFactoryProperty _parent;
};

/*ci
 * \def NETIO_InterfaceFactoryProperty_INITIALIZER
 * \brief Constant to initialize a \ref NETIO_InterfaceFactoryProperty
 */
#define NETIO_InterfaceFactoryProperty_INITIALIZER \
{\
    RT_ComponentFactoryProperty_INITIALIZER \
}

/*ci
 * \brief Definitions of valid NETIO interface states
 */
typedef enum
{
    /*ci
     * \brief The interface has been created, but will not send/receive
     */
    NETIO_INTERFACESTATE_CREATED,

    /*ci
     * \brief The interface is enabled, can send/receive
     */
    NETIO_INTERFACESTATE_ENABLED
} NETIO_InterfaceState_T;

/*e \dref_NETIO_Interface
 * \brief Base-class definition for all NETIO interfaces
 *
 * \details
 *
 * All NETIO layers are RT components and are derived from RT components.
 * All NETIO layers also share a common base class, \ref NETIO_Interface
 * and share common properties.
 */
struct NETIO_Interface
{
    /*ci
     * \brief Derived member
     */
    struct RT_Component _parent;

    /*ci
     * \brief A route table with addresses to send to
     */
    DB_Table_T _rtable;

    /*ci
     * \brief A bind table with addresses to listen to
     */
    DB_Table_T _btable;

    /*ci
     * \brief The local address of the NETIO layer. The use is NETIO
     *        specific. It is not guaranteed to be unique outside of the
     *        stack the layer belongs to.
     */
    struct NETIO_Address local_address;

    /*ci
     * \brief The current NETIO interface state
     */
    NETIO_InterfaceState_T state;
};

/*ci
 * \brief Constant to initialize NETIO_Interface
 */
#define NETIO_Interface_INITIALIZER \
{ \
    RT_Component_INITIALIZER, /* _parent */ \
    NULL, /* _rtable */ \
    NULL, /* _btable */ \
    NETIO_Address_INITIALIZER, /* local_address */ \
    NETIO_INTERFACESTATE_CREATED /* state */ \
}

/*e \dref_NETIO_Interface_T
 */
typedef struct NETIO_Interface NETIO_Interface_T;

/*ci
 * \brief Common bind properties
 */
struct NETIOBindProperty
{
    /*ci
     * \brief The strength of the bind. Strength is a DDS concept and used to
     *        determine ownerships for instances.
     */
    RTI_INT32 strength;

    /*ci
     * \brief The lease duration of the bind. Lease duration is a DDS concept
     *        and used to determine when a writer can be considered as not
     *        alive.
     */
    struct OSAPI_SystemTime lease_duration;

    /*ci
     * \brief Data format used on this route
     */
    RTI_UINT16 data_format;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /*ci
     * \brief The liveliness kind of the bind. Liveliness kind is a DDS concept
     *        and toghether with the lease duration is used to determine when a
     *        writer can be considered as not alive.
     */
    RTI_UINT32 liveliness_kind;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
};

#if DDS_LIVELINESS_CHANNEL_ENABLED
#define NETIO_LivelinessBindProperty_INITIALIZER ,0
#else
#define NETIO_LivelinessBindProperty_INITIALIZER
#endif

/*ci
 * \def NETIOBindProperty_INITIALIZER
 * \brief Constant to initialize \ref NETIOBindProperty
 */
#define NETIOBindProperty_INITIALIZER \
{\
    0,\
    {\
        OSAPI_SYSTEM_TIME_SEC_MAX,\
        OSAPI_SYSTEM_TIME_NANO_MAX\
    },\
    0\
    NETIO_LivelinessBindProperty_INITIALIZER \
}

/*ci
 * \brief Common route properties
 */
struct NETIORouteProperty
{
    /*ci
     * \brief The maximum RTPS message that can be send on this route.
     */
    RTI_UINT32 send_size_max;

    /*ci
     * \brief If the route is reliable or best effort.
     */
    RTI_BOOL is_reliable;
};

/*ci
 * \def NETIORouteProperty_INITIALIZER
 * \brief Constant to initialize \ref NETIORouteProperty
 */
#define NETIORouteProperty_INITIALIZER \
{\
    0, \
    RTI_FALSE \
}

/*ci
 * \brief Generic bind key to listen to a source
 */
struct NETIOBindEntryKey
{
    /*ci
     * \brief The source address to listen to
     */
    struct NETIO_Address source;

    /*ci
     * \brief The destination address listening to the source
     */
    struct NETIO_Address destination;
};

/*ci
 * \brief Generic bind entry
 */
struct NETIOBindEntry
{
    /*ci
     * \brief The source address to listen to
     */
    struct NETIO_Address source;

    /*ci
     * \brief The destination address listening to the source
     */
    struct NETIO_Address destination;

    /*ci
     * \brief The interface to use to listen to the source
     */
    NETIO_Interface_T *intf;
};

/*ci
 * \brief The state of a route as determined by the protocol layer
 */
typedef enum
{
    /*ci
     * \brief The route is active, it can send data
     */
    NETIO_ROUTESTATE_ACTIVE,

    /*ci
     * \brief The route is inactive, it cannot send data
     */
    NETIO_ROUTESTATE_INACTIVE
} NETIO_RouteState_T;

/*ci
 * \brief Definition of generic route entry key. Typically all route entries
 *        inherit from this key
 */
struct NETIORouteEntryKey
{
    /*ci
     * \brief The address which an be reached on this route
     */
    struct NETIO_Address destination;

    /*ci
     * \brief The interface which can reach the destination
     */
    NETIO_Interface_T *intf;

    /*ci
     * \brief The address of the interface itself (source)
     */
    struct NETIO_Address intf_address;
};

/*ci
 * \brief Generic route entry, typically all routes inherit from this type
 */
struct NETIORouteEntry
{
   /*ci
    * \brief The destination address which can be reached
    */
    struct NETIO_Address destination;

    /*ci
     * \brief The interface which can reach the address
     */
    NETIO_Interface_T *intf;

    /*ci
     * \brief The address of the interface
     */
    struct NETIO_Address intf_address;

    /*ci
     * \brief State of the route entry
     */
    NETIO_RouteState_T state;
};

/*ci
 * \brief Discriminator for status changes detected at protocol level
 */
typedef enum
{
    /*ci
     * \brief A NETIO layer has determined that the peer is inactive
     */
    NETIO_EVENTKIND_INACTIVE_PEER,

    /*ci
     * \brief A NETIO layer has determined that the peer is active
     */
    NETIO_EVENTKIND_ACTIVE_PEER,

    /*ci
     * \brief A NETIO layer has freed up N number of resources
     */
    NETIO_EVENTKIND_RESOURCES_FREED,

    /*ci
     * \brief A NETIO layer has determined that a participant has lost liveliness
     */
    NETIO_EVENTKIND_PARTICIPANT_LIVELINESS_LOST,

    /*ci
     * \brief A NETIO packet by the specified sn has been queued
     */
    NETIO_EVENTKIND_PACKET_QUEUED,

    /*ci
     * \brief A NETIO packet by the specified sn is in progress of being sent
     */
    NETIO_EVENTKIND_PACKET_IN_PROGRESS,

    /*ci
     * \brief A NETIO packet by the specified sn has completed transmission
     */
    NETIO_EVENTKIND_PACKET_SEND_COMPLETED

} NETIO_EventKind_T;

/*ci
 * \brief Structure to signal protocol level status changes upstream/downstream
 */
struct NETIO_Event_PeerActivity
{
    /*ci
     * \brief Address of peer layer with the activity change
     */
    struct NETIO_Address peer_addr;

    /*ci
     * \brief Total number of times the peer has become inactive
     */
    RTI_INT32 inactive_total;

    /*ci
     * \brief Change in inactivity since last time signaled
     */
    RTI_INT32 inactive_change;

    /*ci
     * \brief Total number of times the peer has become active
     */
    RTI_INT32 active_total;

    /*ci
     * \brief Change in activity since last time signaled
     */
    RTI_INT32 active_change;
};

/*ci
 * \brief Structure to signal the number of resources released
 */
struct NETIO_Event_ResourcesFreed
{
    /*ci The ID of the entity freeing the resources
     */
    struct NETIO_Guid entity;

    /*ci The number of resources being freed
     */
    RTI_INT32 count;
};

/*ci
 * \brief Generic event structure to signal status changes detected at the
 *        protocol level
 */
struct NETIO_Event
{
    /*ci
     * \brief Event discriminator
     */
    NETIO_EventKind_T kind;

    /*ci
     * \brief Additional event information
     */
    union
    {
        struct NETIO_Event_PeerActivity peer_activity;
        struct NETIO_Event_ResourcesFreed resources_freed;
        struct REDA_SequenceNumber sn;
    } value;
};

/*e \dref_NETIO_InterfaceI
 */
struct NETIO_InterfaceI;

/*ci
 * \brief Create a table name with a consistent format for NETIO route
 *        and bind tables
 *
 * \param[out] tbl_name The resulting table name is placed here. It is assumed
 *                      that the buffer is long enough and is should be at least
 *                      NETIO_TABLE_NAME_SIZE bytes large.
 * \param[in]  id       The name of the factory owning the table
 * \param[in]  suffix   Single character identifying the type of table
 * \param[in]  instance The instance the table is created from
 *
 */
NETIODllExport void
NETIO_Interface_Table_name_from_id(char *tbl_name,
                                  union RT_ComponentFactoryId *id,
                                  char suffix,
                                  RTI_INT32 instance);

/*ci
 * \brief Initialize a NETIO_Interface base-class
 *
 * \param[in] netio      The netio interface to initialize
 * \param[in] netio_intf Pointer to the NETIO interface implementation
 * \param[in] property   The NETIO properties
 * \param[in] listener   The NETIO listener
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref NETIO_Interface_finalize
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Interface_initialize(struct NETIO_Interface *netio,
                          struct NETIO_InterfaceI *netio_intf,
                          const struct NETIO_InterfaceProperty *const property,
                          const struct NETIO_InterfaceListener *const listener);

#ifndef RTI_CERT
/*ci
 * \brief Finalize a NETIO_Interface base-class
 *
 * \param[in] netio The netio interface to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_initialize
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Interface_finalize(struct NETIO_Interface *netio);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of required database function to compare
 *        NETIO route records stored in a database
 *
 * \details
 *
 * This function is installed as the compare function for the
 * tables holding NETIO_Route records
 *
 * \param[in] flags Passed in from the database
 * \param[in] op1   Existing database record
 * \param[in] op2   New database record or key depending on flags
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN NETIODllExport RTI_INT32
NETIO_Interface_compare_route(RTI_INT32 flags,
                              const DB_Record_T op1, void *op2);

/*ci
 * \brief Implementation of required database function to compare
 *        NETIO route records stored in a database
 *
 * \details
 *
 * This function is installed as the compare function for the
 * tables holding NETIO_Bind records
 *
 * \param[in] flags Passed in from the database
 * \param[in] op1   Existing database record
 * \param[in] op2   New database record or key depending on flags
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN NETIODllExport RTI_INT32
NETIO_Interface_compare_bind(RTI_INT32 flags,
                             const DB_Record_T op1, void *op2);

/*ce \dref_NETIO_Interface_sendFunc
 * \brief Definition of the \idref_NETIO_InterfaceI send method
 *
 * \param[in] self        NETIO interface to send from
 * \param[in] source      The source interface for the packet
 * \param[in] destination The destination address for the packet
 * \param[in] packet      The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_sendFunc)(NETIO_Interface_T *self,
                            struct NETIO_Interface *source,
                            struct NETIO_Address *destination,
                            NETIO_Packet_T *packet)
)

/*e \dref_NETIO_Interface_receiveFunc
 * \brief Definition of the \idref_NETIO_InterfaceI receive method
 *
 * \param[in] netio_intf NETIO interface to receive in
 * \param[in] src_addr   The source address of the packet
 * \param[in] dst_addr   The destination address for the packet
 * \param[in] packet     The forwarded packet from downstream
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \idref_NETIO_Interface_receive, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_receiveFunc)(NETIO_Interface_T *netio_intf,
                              struct NETIO_Address *src_addr,
                              struct NETIO_Address *dst_addr,
                              NETIO_Packet_T *packet)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI acknack method
 *
 * \param[in] self      The NETIO interface receiving the acknack
 * \param[in] source    The peer source address of the acknack
 * \param[in] packet_id The packet_id/SN of the NETIO_Packet being acked/nacked
 * \param[in] nack      RTI_TRUE if this is a negative acknowledgment
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_acknack, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_acknackFunc)(NETIO_Interface_T *self,
                               struct NETIO_Address *source,
                               NETIO_PacketId_T *packet_id,
                               RTI_BOOL nack)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI request method
 *
 * \param[in] self             The NETIO interface receiving the request
 * \param[in] source           The peer source address of the request
 * \param[in] dest             The peer destination address of the request
 * \param[out] packet          The requested packet if it existed, NULL
 *                             otherwise
 * \param[in] packet_id        The packet_id/SN of the NETIO_Packet being
 *                             requested
 * \param[in] actual_packet_id The if packet_id is not available, the next
 *                             available SN/packet_id
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_return_loan, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_requestFunc)(NETIO_Interface_T *self,
                               struct NETIO_Address *source,
                               struct NETIO_Address *dest,
                               NETIO_Packet_T **packet,
                               NETIO_PacketId_T *packet_id,
                               NETIO_PacketId_T *actual_packet_id)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI return_loan method
 *
 * \param[in] self       The NETIO interface receiving the request
 * \param[in] source     The peer source address of the loan
 * \param[in] packet     The packet being returned
 * \param[in] packet_id  The packet_id/SN of the NETIO_Packet being returned
 *                       requested
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_request, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_return_loanFunc)(NETIO_Interface_T *self,
                                   struct NETIO_Address *source,
                                   NETIO_Packet_T *packet,
                                   NETIO_PacketId_T *packet_id)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI xmit_remove method
 *
 * \param[in] self        NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_xmit_remove, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_xmit_removeFunc)(NETIO_Interface_T *self,
                                   struct NETIO_Address *destination,
                                   NETIO_PacketId_T *packet_id)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI add_route method
 *
 * \param[in] self      NETIO interface to add the route to
 * \param[in] dst_addr  The destination address for the route
 * \param[in] via_intf  The downstream interface
 * \param[in] via_addr  The address to pass to the downstream interface
 * \param[in] property  The route property
 * \param[in] existed   Whether the route already existed
 *
 * \sa \ref NETIO_Interface_add_route, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_add_routeFunc)(NETIO_Interface_T *self,
                                 struct NETIO_Address *dst_addr,
                                 NETIO_Interface_T *via_intf,
                                 struct NETIO_Address *via_addr,
                                 struct NETIORouteProperty *property,
                                 RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI delete_route method
 *
 * \param[in]  self      NETIO interface to delete the route from
 * \param[in]  dst_addr  The destination address for the route
 * \param[in]  via_intf  The downstream interface
 * \param[in]  via_addr  The address to pass to the downstream interface
 * \param[out] existed   Whether the route existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that is it not
 *         considered a failure if the interface didn't exist.
 *
 * \sa \ref NETIO_Interface_delete_route, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_delete_routeFunc)(NETIO_Interface_T *self,
                                    struct NETIO_Address *dst_addr,
                                    NETIO_Interface_T *via_intf,
                                    struct NETIO_Address *via_addr,
                                    RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI bind method
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  property   The property to use for the bind
 * \param[out] existed    Whether a previous bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_Interface_bind, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_bindFunc)(NETIO_Interface_T *netio_intf,
                           struct NETIO_Address *src_addr,
                           struct NETIOBindProperty *property,
                           RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI unbind method
 *
 * \param[in]  netio_intf NETIO interface to unbind
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   Interface to unbind from
 * \param[out] existed    Whether a bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_Interface_unbind, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_unbindFunc)(NETIO_Interface_T *netio_intf,
                             struct NETIO_Address *src_addr,
                             NETIO_Interface_T *dst_intf,
                             RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI bind_external method
 *
 * \param[in]  src_intf   NETIO interface to bind to upstream interface
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to pass to the upstream interface
 * \param[in]  property   The properties for the bind
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_Interface_bind_external, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_bind_externalFunc)(NETIO_Interface_T *src_intf,
                                    struct NETIO_Address *src_addr,
                                    NETIO_Interface_T *dst_intf,
                                    struct NETIO_Address *dst_addr,
                                    struct NETIOBindProperty *property,
                                    RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI unbind_external method
 *
 * \param[in]  src_intf   NETIO interface to unbind from upstream interface
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to passed to the upstream interface
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_Interface_unbind_external, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_unbind_externalFunc)(NETIO_Interface_T *src_intf,
                                      struct NETIO_Address *src_addr,
                                      NETIO_Interface_T *dst_intf,
                                      struct NETIO_Address *dst_addr,
                                      RTI_BOOL *existed)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI get_external_interface method
 *
 * \param[in]   netio_intf   NETIO interface to get the external interface for
 * \param[in]   src_addr     The address to send to
 * \param[out]  dst_intf     The interface to use when sending to the address
 * \param[out]  dst_addr     The destination address to use when forwarding
 *                           packets to the destination interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_Interface_get_external_interface, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_get_external_interfaceFunc)(NETIO_Interface_T *netio_intf,
                                             struct NETIO_Address *src_addr,
                                             NETIO_Interface_T **dst_intf,
                                             struct NETIO_Address *dst_addr)
)

/*e \dref_NETIO_Interface_reserve_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_reserve_addressFunc)(NETIO_Interface_T *self,
                                       struct NETIO_AddressSeq *req_address,
                                       struct NETIO_AddressSeq *rsvd_address,
                                       struct NETIOBindProperty *property)
)

/*ce \dref_NETIO_Interface_release_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_release_addressFunc)(NETIO_Interface_T *self,
                                    struct NETIO_Address *src_addr)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI set_state method
 *
 * \param[in] netio_intf NETIO interface to set state on
 * \param[in] state      New state
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \dref_NETIO_Interface_set_state, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_set_stateFunc)(NETIO_Interface_T *netio_intf,
                                NETIO_InterfaceState_T state)
)

/*ce \dref_NETIO_Interface_resolve_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_resolve_addressFunc)(NETIO_Interface_T *netio_intf,
                                        const char *address_string,
                                        struct NETIO_Address *address_value,
                                        RTI_BOOL *invalid)
)

/*ce \dref_NETIO_Interface_get_route_tableFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_get_route_tableFunc)(NETIO_Interface_T *netio_intf,
                                        struct NETIO_AddressSeq *address,
                                        struct NETIO_NetmaskSeq *netmask)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI_post_event method
 *
 * \param[in] self     The datawriter interface the event occurred on
 * \param[in] src_intf The source of the event
 * \param[in] event    The NETIO event
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 *
 * \sa \ref NETIO_Interface_post_event, \idref_NETIO_InterfaceI
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_post_eventFunc)(NETIO_Interface_T *self,
                                  NETIO_Interface_T *src_intf,
                                  struct NETIO_Event *event)
)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI_lookup_route method
 *
 * \details
 *
 * Check if a NETIO interface has a route to a specific destination
 *
 * \param[in]  netio_intf   NETIO interface to get the external interface for
 * \param[in]  dst_reader   The address to send to
 * \param[in]  via_intf     The interface to use
 * \param[in]  via_address  The interface address to use
 * \param[out] route_exists RTI_TRUE if the route existed, RTI_FALSE if not
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_lookup_routeFunc)(struct NETIO_Interface *netio_intf,
                                    struct NETIO_Address *dst_reader,
                                    struct NETIO_Interface *via_intf,
                                    struct NETIO_Address *via_address,
                                    RTI_BOOL *route_exists)
)

/*ci \dref_NETIO_Interface_is_reachableFunc
 * \brief Definition of the \idref_NETIO_InterfaceI is_reachable method
 *
 * \details
 *
 * Check if an address is reachable
 *
 * \param[in]  netio_intf   NETIO interface to to use
 * \param[in]  address      Address to check if it can be reached
 * \param[out] is_reachable Set to RTI_TRUE if no error an address is reachable
 *
 * \return RTI_TRUE if successfull, RTI_FALSE if not
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_Interface_is_address_reachableFunc)(struct NETIO_Interface *netio_intf,
                                        const struct NETIO_AddressEx *const addr,
                                        RTI_BOOL *is_reachable)
)

#define NETIO_Interface_has_is_address_reachable(self_) \
    (((struct NETIO_InterfaceI*)((self_)->_parent._intf))->is_address_reachable != NULL)

typedef void
(*NETIO_Interface_get_transport_propertiesFunc)(
                                    struct NETIO_Interface *netio_intf,
                                    struct NETIO_TransportProperty *properties);
/*e \dref_NETIO_InterfaceI
 */
struct NETIO_InterfaceI
{
    /*ci
     * \brief Base-class interface
     */
    struct RT_ComponentI _parent;

    /*ci
     * \brief Method to send packets to one or more destinations
     */
    NETIO_Interface_sendFunc send;

    /*ci
     * \brief Method to ACK/NACK a packet upstream
     */
    NETIO_Interface_acknackFunc acknack;

    /*ci
     * \brief Method to request packet upstream
     */
    NETIO_Interface_requestFunc request;

    NETIO_Interface_return_loanFunc return_loan;

    /*ci
     * \brief Method to cancel a transmission downstream
     */
    NETIO_Interface_xmit_removeFunc xmit_remove;

    /*ci
     * \brief Method to add a destination route to an interface
     */
    NETIO_Interface_add_routeFunc add_route;

    /*ci
     * \brief Method to remove a route to a destination from an interface
     */
    NETIO_Interface_delete_routeFunc delete_route;

    /*ci
     * \brief Method to reserve addresses to listen to on an interface
     */
    NETIO_Interface_reserve_addressFunc reserve_address;

    /*ci
     * \brief Method to start listening for data from a peer interface
     */
    NETIO_Interface_bindFunc bind;

    /*ci
     * \brief Method to stop listening for data from a peer interface
     */
    NETIO_Interface_unbindFunc unbind;

    /*ci
     * \brief Method to forward a packet to an interface upstream
     */
    NETIO_Interface_receiveFunc receive;

    /*ci
     * \brief Method to get the interface to forward packets to from an
     *        downstream interface
     */
    NETIO_Interface_get_external_interfaceFunc get_external_interface;

    /*ci
     * \brief Method to add a path from an interface to an upstream interface
     */
    NETIO_Interface_bind_externalFunc bind_external;

    /*ci
     * \brief Method to remove a path from an interface to an upstream interface
     */
    NETIO_Interface_unbind_externalFunc unbind_external;

    /*ci
     * \brief Method to set the state of an interface
     */
    NETIO_Interface_set_stateFunc set_state;

    /*ci
     * \brief Method to release addresses to listen to on an interface
     */
    NETIO_Interface_release_addressFunc release_address;

    /*ci
     * \brief Method to query an interface if it can resolve an address or not
     */
    NETIO_Interface_resolve_addressFunc resolve_address;

    /*ci
     * \brief Method to query an interface for what addresses it can forward
     *        packets to
     */
    NETIO_Interface_get_route_tableFunc get_route_table;

    /*ci
     * \brief Method to post external events to an interface
     */
    NETIO_Interface_post_eventFunc post_event;

    /*ci
     * \brief Method to lookup if a route exists to a destination
     */
    NETIO_Interface_lookup_routeFunc lookup_route;

    /*ci
     * \brief Function to check if an address is reachable
     */
    NETIO_Interface_is_address_reachableFunc is_address_reachable;

    /*ci
     * \brief Function to get transport related properties
     */
    NETIO_Interface_get_transport_propertiesFunc get_properties;
};

/*ci
 * \brief Wrapper to call of the NETIO_InterfaceFactory->create_component
 *
 * \param[in] f_  The NETIO interface factory
 * \param[in] p_  The property
 * \param[in] l_  The listener
 */
#define NETIO_InterfaceFactory_create_component(f_,p_,l_) \
        (NETIO_Interface_T*)((f_)->intf->create_component(\
                (f_),p_,l_))

/*ci
 * \brief Wrapper to call of the NETIO_InterfaceFactory->delete_component
 *
 * \param[in] f_    The NETIO interface factory
 * \param[in] intf_ The NETIO interface to delete
 */
#define NETIO_InterfaceFactory_delete_component(f_,intf_) \
        (f_)->intf->delete_component(f_,(&(intf_)->_parent))

/*ci
 * \brief Wrapper to call of the NETIO_InterfaceFactory->get_property_component
 *
 * \param[in] f_    The NETIO interface factory
 * \param[in] prop_ The NETIO InterfaceFactoryProperty to get
 */
#define NETIO_InterfaceFactory_get_property(f_,prop_) \
        (f_)->intf->get_property(f_,(struct RT_ComponentFactoryProperty**)(prop_))

/*ci
 * \brief Wrapper to call the \ref NETIO_InterfaceI::send
 *
 * \details
 * Forward a packet from a source interface to a destination
 *
 * \param[in] self_      NETIO interface to send from
 * \param[in] src_intf_  The source interface for the packet
 * \param[in] dst_addr_  The destination address for the packet
 * \param[in] pkt_       The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_Interface_send(self_,src_intf_,dst_addr_,pkt_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->send(self_,src_intf_,dst_addr_,pkt_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_acknack
 *
 * \details
 * An acknack is processed for a peer. If all peers who should receive this
 * sample has acked it, then it can be ack'ed in the queue and is up for removal
 * A peer (source parameter) must acknack a sample at most once.
 *
 * \param[in] self_     The NETIO interface receiving the acknack
 * \param[in] src_intf_ The peer source address of the acknack
 * \param[in] sn_       The packet_id/SN of the NETIO_Packet being acked/nacked
 * \param[in] nack_     RTI_TRUE if this is a negative acknowledgment
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_Interface_acknack(self_,src_intf_,sn_,nack_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->acknack(self_,src_intf_,sn_,nack_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_request
 *
 * \details
 * Request a SN from source interface and destination address.
 * if SN  is not available nextpktid_ contains the next available
 * SN.
 *
 * \param[in] self_      The NETIO interface receiving the request
 * \param[in] src_       The peer source address of the request
 * \param[in] dst_       The peer destination address of the request
 * \param[in] pkt_       The requested packet if it existed, NULL
 *                       otherwise
 * \param[in] pktid_     The packet_id/SN of the NETIO_Packet being
 *                       requested
 * \param[in] nextpktid_ The if packet_id is not available, the next
 *                       available SN/packet_id
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_Interface_request(self_,src_,dst_,pkt_,pktid_,nextpktid_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->request(self_,src_,dst_,pkt_,pktid_,nextpktid_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_return_loan method
 *
 * \param[in] self_    The NETIO interface receiving the request
 * \param[in] src_     The peer source address of the loan
 * \param[in] pkt_     The packet being returned
 * \param[in] pkt_id_  The packet_id/SN of the NETIO_Packet being returned
 *                     requested
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_Interface_request, \idref_NETIO_InterfaceI
 */
#define NETIO_Interface_return_loan(self_,src_,pkt_,pkt_id_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->return_loan(self_,src_,pkt_,pkt_id_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_xmit_remove
 *
 * \details
 * Inform all downstream interfaces all attempts at delivering a packet
 * should be cancelled.
 *
 * \param[in] self_   NETIO interface to cancel a transmit on
 * \param[in] dst_    The destination address of the packet
 * \param[in] pkt_id_ The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_Interface_xmit_remove(self_,dst_,pkt_id_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->xmit_remove(self_,dst_,pkt_id_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_add_route
 *
 * \details
 * Add route to a matching peer to be reached via a downstream interface
 * using the specified downstream address.
 *
 * \param[in] self_      NETIO interface to add the route to
 * \param[in] dst_       The destination address for the route
 * \param[in] intf_      The downstream interface
 * \param[in] intf_addr_ The address to pass to the downstream interface
 * \param[in] prop_      The route property
 * \param[in] ex_        Whether the route already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_Interface_add_route(self_,dst_,intf_,intf_addr,prop_,ex_) \
    ((struct NETIO_InterfaceI*)(\
     (self_)->_parent._intf))->add_route(self_,dst_,intf_,intf_addr,prop_,ex_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_delete_route
 *
 * \details
 * Remove a route to a peer, either because it no longer matches or that the
 * peer has been deleted.
 *
 * \param[in]  self_      NETIO interface to add the route to
 * \param[in]  dst_       The destination address for the route
 * \param[in]  intf_      The downstream interface
 * \param[in]  intf_addr_ The address to pass to the downstream interface
 * \param[out] exist_     Whether the route existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that is it not
 *         considered a failure if the interface didn't exist.
 */
#define NETIO_Interface_delete_route(self_,dst_,intf_,intf_addr_,exist_) \
    ((struct NETIO_InterfaceI*)(\
     (self_)->_parent._intf))->delete_route(self_,dst_,intf_,intf_addr_,exist_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_reserve_address
 *
 * \details
 *  Reserve addresses on an interface to listen to using the requested addresses
 *  as the starting point. The resulting addresses may be fewer than requested,
 *  even 0 is legal.
 *
 * \param[in]    self_     NETIO interface to reserve addresses on
 * \param[in]    src_addr_ List of requested addresses
 * \param[inout] pub_addr_ List of addresses that are reserved
 * \param[in]    prop_     Properties to use to listen on the reserved addresses
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_reserve_address(self_,src_addr_,pub_addr_,prop_)\
    ((struct NETIO_InterfaceI*)(\
    (self_)->_parent._intf))->reserve_address(self_,src_addr_,pub_addr_,prop_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_release_address
 *
 * Release an address previously reserved. The address will no longer be
 * listened on by the caller.
 *
 * \param[in] self_     NETIO interface to release addresses on
 * \param[in] src_addr_ Address to release
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_release_address(self_,src_addr_) \
                    ((struct NETIO_InterfaceI*)(\
                    (self_)->_parent._intf))->release_address(self_,src_addr_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_bind
 *
 * \details
 * When an interface is matched with another interface (how is outside the
 * scope of this function) an entry in the bind-table for the interface is
 * created. The peer-to-peer state between the two interfaces should be
 * maintained based on this relationship until an unbind is performed.
 *
 * \param[in]  self_ NETIO interface to bind
 * \param[in]  src_  The address to bind to
 * \param[in]  prop_ The property to use for the bind
 * \param[out] ex_   Whether a previous bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_bind(self_,src_,prop_,ex_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->bind(self_,src_,prop_,ex_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_unbind
 *
 * \details
 * When an interface is unmatched from another interface (how is outside the
 * scope of this function) an entry in the bind-table for the interface can
 * safely be removed (although not required). No state is expected to be
 * maintained from this point on
 *
 * \param[in]  self_  NETIO interface to unbind
 * \param[in]  src_   The address to unbind from
 * \param[in]  dst_   Interface to unbind from
 * \param[out] exist_ Whether a bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_unbind(self_,src_,dst_,exist_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->unbind(self_,src_,dst_,exist_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_bind_external
 *
 * \details
 * When an upstream interface wants to listen to data from a local
 * downstream interface it binds to the downstream interface using the
 * external bind function. This typically means adding an interface to a
 * bind table in the downstream interface so the downstream interface is
 * able to forward data upstream.
 *
 * \param[in]  src_intf_ NETIO interface to bind to upstream interface
 * \param[in]  src_adr_  The address to bind to
 * \param[in]  dst_intf_ The upstream interface
 * \param[in]  dst_adr_  The address to pass to the upstream interface
 * \param[in]  p_        The properties for the bind
 * \param[out] e_        Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_bind_external(src_intf_,src_adr_,dst_intf_,dst_adr_,p_,e_) \
((struct NETIO_InterfaceI*)(\
 (src_intf_)->_parent._intf))->bind_external(src_intf_,src_adr_,dst_intf_,dst_adr_,p_,e_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_unbind_external
 *
 * \details
 * When an upstream interface no longer wants to listen to data from a local
 * downstream interface it unbinds from the downstream interface using the
 * external unbind function. This typically means removing an interface from a
 * bind table in the downstream interface so the downstream interface no longer
 * forwards data upstream to a non-existent interface.
 *
 * \param[in]  src_intf_ NETIO interface to unbind from upstream interface
 * \param[in]  src_adr_  The address to unbind from
 * \param[in]  dst_adr_  The upstream interface
 * \param[in]  dst_intf_ The address to passed to the upstream interface
 * \param[out] exist_    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_unbind_external(src_intf_,src_adr_,dst_adr_,dst_intf_,exist_) \
((struct NETIO_InterfaceI*)(\
 (src_intf_)->_parent._intf))->unbind_external(src_intf_,src_adr_,dst_adr_,dst_intf_,exist_)

/*e \dref_NETIO_Interface_receive
 */
#define NETIO_Interface_receive(self_,src_,dst_,p_) \
    ((struct NETIO_InterfaceI*)(\
            (self_)->_parent._intf))->receive(self_,src_,dst_,p_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_get_external_interface
 *
 * \details
 * When an interface is bound to a downstream interface it must provide
 * which interface and address the downstream interface should use when
 * forwarding a NETIO_Packet.
 *
 * \param[in]   self_   NETIO interface to get the external interface for
 * \param[in]   src_    The address to send to
 * \param[out]  ul_     The interface to use
 * \param[out]  dst_    The destination address to use
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_get_external_interface(self_,src_,ul_,dst_) \
    ((struct NETIO_InterfaceI*)(\
       (self_)->_parent._intf))->get_external_interface(self_,src_,ul_,dst_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_set_state
 *
 * \details
 * Set the state of an interface, the interface is not required to perform
 * any action as this is interface dependent.
 *
 * \param[in] self_  NETIO interface to set state on
 * \param[in] state_ New state
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_set_state(self_,state_) \
    ((struct NETIO_InterfaceI*)(\
       (self_)->_parent._intf))->set_state(self_,state_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_resolve_address
 *
 * \details
 * Request an interface to convert a string address to a \ref NETIO_Address.
 * If an interface successfully translated it, is_invalid_ is set to RTI_TRUE,
 * otherwise false and the content of the output is undefined.
 *
 * \param[out] self_       Interface requested to do the conversion
 * \param[in]  address_    Address to convert
 * \param[out] resolved_   Converted address on success
 * \param[out] is_invalid_ Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_resolve_address(self_,address_,resolved_,is_invalid_) \
((struct NETIO_InterfaceI*)(\
   (self_)->_parent._intf))->resolve_address(self_,address_,resolved_,is_invalid_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_get_route_table
 *
 * \details
 * Query an interface for what addresses it can forward packets to. Not
 * all interfaces provides a routing table, this is implementation dependent.
 *
 * \param[in]    self_    The NETIO interface
 * \param[inout] address_ Sequence of NETIO addresses this interface understands
 * \param[inout] netmask_ Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
#define NETIO_Interface_get_route_table(self_,address_,netmask_) \
    ((struct NETIO_InterfaceI*)(\
       (self_)->_parent._intf))->get_route_table(self_,address_,netmask_)

/*ci
 * \brief Wrapper to call \idref_NETIO_InterfaceI_post_event
 *
 * \details
 * Post an external event on the interface
 *
 * \param[in] self_     The interface interface the event occurred on
 * \param[in] src_intf_ The source of the event
 * \param[in] event_    The NETIO event
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
#define NETIO_Interface_post_event(self_,src_intf_,event_) \
    ((struct NETIO_InterfaceI*)(\
       (self_)->_parent._intf))->post_event(self_,src_intf_,event_)

/*ci
 * \brief Lookup if a route exists to a destination via an interface
 *
 * \param[in]  self_         The source interface
 * \param[in]  dst_reader_   The destination address
 * \param[in]  via_intf_     The downstream interface
 * \param[in]  via_address_  The address to pass to the downstream interface
 * \param[out] route_exists_ RTI_TRUE if the route existed, RTI_FALSE if not
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
#define NETIO_Interface_lookup_route(self_,dst_reader_,via_intf_,\
                                     via_address_,route_exists_)  \
 ((struct NETIO_InterfaceI*)(\
   (self_)->_parent._intf))->lookup_route(self_,dst_reader_,via_intf_,\
           via_address_,route_exists_)

/*ci
 * \brief Definition of the \idref_NETIO_InterfaceI_is_reachable method
 *
 * \details
 *
 * Check if an address is reachable
 *
 * \param[in]  netio_intf   NETIO interface to to use
 * \param[in]  address      Address to check if it can be reached
 * \param[out] is_reachable Set to RTI_TRUE if no error an address is reachable
 *
 * \return RTI_TRUE if the call was successful, RTI_FALSE if not
 */
#define NETIO_Interface_is_address_reachable(self_,address_,is_reachable_)  \
 ((struct NETIO_InterfaceI*)(\
   (self_)->_parent._intf))->is_address_reachable(self_,address_,is_reachable_)

/*ci
 * \brief Get the transport properties
 *
 * \param[in]     self_        The interface to query
 * \param[inout]  properties_  Filled in with properties on return.
 *
 * \return RTI_TRUE if the call was successful, RTI_FALSE if not
 */
#define NETIO_Interface_get_transport_properties(self_,properties_)  \
 ((struct NETIO_InterfaceI*)(\
   (self_)->_parent._intf))->get_properties(self_,properties_)

#ifdef __cplusplus
}
#endif

#endif /* netio_interface_h */

/*ci @} */
