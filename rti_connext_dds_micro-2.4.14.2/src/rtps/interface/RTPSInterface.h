/*
 * FILE: RTPSInterface.h - Exported RTPS_Interface functions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 17aug2022,tk MICRO-4115/PR.30818
 * - Changed hb_count to RTI_UINT32 in RTPS_Writer for well defined
 *   behavior of rollover.
 * 26may2022,tk MICRO-3589/PR.30517
 * The following non-functional changes were made for clarity:
 * - Renamed RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_HOST to
 *   RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_NETWORK_ORDER
 * - Renamed RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_HOST to
 *   RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_NETWORK_ORDER
 * 15sep2021,tk MICRO-3251/PR.29568
 * - Only include batch_writer when RTPS_DATA_BATCH_ENABLED is TRUE.
 * 13sep2021,tk MICRO-3231/PR.29563
 * - Removed instance_counter which is no longer used to create table names.
 * 14oct2016,tk MICRO-1571 Handle re-discovery of endpoint discovery data
 * 06jun2016,tk MICRO-1543 Reference count shared resources for matched entities (needed
 *                         after changes to related to MICRO-1505
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 27jan2014,eh MICRO-714  Add in/active_reader_count, rcvd_first_hb
 */

/*ce @ingroup RTPSModule
 * \file
 * \brief Header file for an implementation of the NETIO interface for the 
 * RTPS protocol
 *  
 * \details 
 * - RTPS Message and Submessage defines and types
 */
#include "osapi/osapi_config.h"
#ifndef rtps_config_h
#include "rtps/rtps_config.h"
#endif

#ifndef RTPSInterface_pkg_h
#define RTPSInterface_pkg_h

/******************************************************************************/

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif

#include "../checksum/RTPSChecksum.h"

/*ci \brief Writer configuration flags */
typedef RTI_UINT32 RTPS_WriterFlags;

/*ci \brief Default empty writer flags */
#define RTPS_WRITER_FLAG_NONE            0x00U

/*ci \brief Reliable writer flag */
#define RTPS_WRITER_FLAG_RELIABLE        0x01U

/*ci \brief Active periodic HEARTBEAT event writer flag */
#define RTPS_WRITER_FLAG_HB_EVENT_ACTIVE 0x02U

/*ci \brief Flag indicating this is a built-in endpoint writer */
#define RTPS_WRITER_BUILTIN_WRITER       0x04U

/******************************************************************************/

/*ci \brief Reader configuration flags */
typedef RTI_UINT32 RTPS_ReaderFlags;

/*ci \brief Default empty reader flags */
#define RTPS_READER_FLAG_NONE                  0x00U

/*ci \brief Reliable reader flag */
#define RTPS_READER_FLAG_RELIABLE              0x01U

/*ci \brief Reliable reader ACKNACK active */
#define RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE  0x02U

/*ci \brief Flag indicating this is a built-in endpoint reader */
#define RTPS_READER_BUILTIN_READER             0x04U

/******************************************************************************/

/*ci \brief Offset used when deserializing received DATA submessage */
#define RTPS_RCV_DATA_SUBMSG_OFFSET       24

/*ci \brief Offset used when deserializing received DATA_BATCH submessage */
#define RTPS_RCV_DATA_BATCH_SUBMSG_OFFSET 44

/*ci \brief Offset used when deserializing received non-DATA submessage */
#define RTPS_RCV_NON_DATA_SUBMSG_OFFSET   12

/*ci \brief Size in bytes of RTPS Entity ID */
#define RTPS_ENTITY_ID_LENGTH 4

/*ci \brief Size in bytes of Length field in RTPS Header extension */
#define RTPS_HEADER_EXTN_LENGTH_SIZE 4

/*ci \brief Sample ID type */
typedef struct REDA_SequenceNumber RTPS_SampleId_T;

/******************************************************************************/

/*ci 
 * \brief Send window 
 *  
 * \details 
 * Used by reliable RTPS writer to track samples sent and
 * acknowledged by each remote reader. Each sent sample takes one entry in 
 * the window.  Window has a finite size and can become full. 
 */
struct RTPS_Window 
{
    /*ci \brief Count of in-use entries */
    RTI_UINT32 size;

    /*ci \brief Max count of entries */
    RTI_UINT32 max_size;

    /*ci \brief Bitmap representing sliding window */
    struct RTPS_Bitmap bitmap; 

    /*ci 
     * \brief 
     * Largest possible sequence number in window. 
     *  
     * \details 
     * Cached for quick determination of whether a sample is outside the window.
     */
    RTPS_SampleId_T tail;
};

/*ci 
 * \brief Remote reader peer record 
 *  
 * \details 
 * Writer keeps a RemoteReader record for each peer reader. Keeps state of 
 * reader's activity and sample acknowledgement. 
 */
struct RTPS_RemoteReader
{
    /*ci \brief RTI_TRUE for reliable remote reader */
    RTI_BOOL reliable;
     
    /* RTPS_RELIABILITY: Fields below apply only for reliable remote readers */
#if RTPS_RELIABILITY

    /*ci 
     * \brief Epoch of last received ACKNACK. 
     *  
     * \details 
     * Every ACKNACK contains an epoch that is increased with each new 
     * ACKNACK.  This is cached and used to ignore ACKNACKs with older epochs. 
     */ 
    REDA_Epoch_T epoch;

    /*ci 
     * \brief
     * Sequence number of last (newest) acknowledged sample.
     *
     * \details
     * Is one before the starting SN of send window (i.e. oldest unacknowledged
     * sample) to this remote reader. Is non-decreasing.
     */
    RTPS_SampleId_T last_acked_sn;

    /*ci 
     * \brief Flag preventing response to a non-progressing NACK.
     *  
     * \details 
     *  Set to RTI_TRUE at every periodic HEARTBEAT.
     *  Set to RTI_FALSE after done processing an ACKNACK.
     *  If RTI_FALSE, don't repair NACK whose bitmap lead has not advanced from
     *  the previous NACK of the same reader.
     */
    RTI_BOOL repair_stuck_nack;

    /*ci 
     * \brief 
     * Flag set when remote Reader is not actively responding to 
     * HEARTBEATs with ACKNACKs. 
     *  
     * \details 
     * Set RTI_TRUE when inactive_count counts down to zero. 
     * Set RTI_FALSE when ACKNACK is received from remote reader. 
     */
    RTI_BOOL is_inactive;

    /*ci 
     * \brief
     * Countdown for Reader becoming inactive.
     *
     * \details
     * Reset when ACKNACK is received.  Decremented every HEARTBEAT period.
     * Reader becomes inactive when count reaches zero.
     */ 
    RTI_INT32 inactive_count;

    /*ci 
     * \brief Send window of unacknowledged samples.
     *
     * \details
     * Used by reliable RTPS writer to track samples sent and
     * acknowledged by each remote reader. Each sent sample takes one entry in 
     * the window.  Window has a finite size and can become full.
     */
    struct RTPS_Window window; 

    /*ci
     * \brief Saved initial last acked sn from when the peer was first asserted
     */
    RTPS_SampleId_T initial_last_acked_sn;
#endif

};

/*ci 
 * \brief Remote writer peer record 
 *  
 * \details 
 * Reader keeps a RemoteWriter record for each peer writer. Keeps state on 
 * samples received from each writer. 
 */
struct RTPS_RemoteWriter
{
    /*ci \brief RTI_TRUE for reliable remote writer */
    RTI_BOOL reliable; 

    /*ci \brief Set RTI_TRUE after receiving first HEARTBEAT from writer */
    RTI_BOOL rcvd_first_hb;

#if RTPS_DATA_BATCH_ENABLED
    /*ci \brief Set RTI_TRUE after receiving HEARTBEAT_BATCH or
     * DATA_BATCH from writer
     */
    RTI_BOOL batch_writer;
#endif

    /*ci 
     * \brief Bitmap of samples received from writer 
     *  
     * \details 
     * Bitmap maintains invariant that its lead sequence number is the first 
     * SN that has not been received.  All SN less than lead SN have been 
     * received.  Bitmap functions as a receive window, as it as a finite 
     * size, and any received SN outside bitmap's range is dropped. Used for 
     * both reliable and best-effort writers 
     */
    struct RTPS_Bitmap bitmap; 

    /*ci
     * \brief Highest received sample virtual SN
     *
     * \details
     * For a writer which does not have batching enabled this field is not 
     * used. For a writer with batching enabled this SN contains the highest 
     * sample SN in the batch + 1 (not the batch sample SN but the SN of the 
     * sample inside the batch).
     */
    RTPS_SampleId_T first_unaccepted_virtual_sn;

    /*ci 
     * \brief The largest received SN that has been accepted upstream.
     *  
     * \details 
     * Used when determining reserved_count   
     */
    RTPS_SampleId_T highest_accepted_sn;

    /*ci 
     * \brief The largest received SN that has been accepted upstream.
     *
     * \details
     * Used when determining reserved_count
     */
    RTPS_SampleId_T highest_accepted_vsn;

    /*ci
     * \brief Number of entries reserved to fill holes created from receiving 
     * samples out of order. 
     *  
     * \details 
     * See RTPS_Reader.reserved_count for a description of reserved count. 
     * This reserved_count applies for this remote writer and is non-zero when 
     * a sample is received out of order from this remote writer. 
     */
    RTI_INT32 reserved_count;

#if RTPS_RELIABILITY

    /*ci 
     * \brief Epoch of last received HEARTBEAT. 
     *  
     * \details 
     * Every HEARTBEAT contains an epoch that is increased with each new 
     * HEARTBEAT. This is cached and used to ignore HEARTBEATs with older 
     * epochs. 
     */ 
    REDA_Epoch_T epoch; 
#endif

};

/*ci 
 * \brief Local writer record 
 *  
 * \details 
 * State and configuration for an RTPS writer. 
 */
struct RTPS_Writer
{
    /*ci 
     * \brief Reference to upstream NETIO interface (e.g. DDS DataWriter)
     *  
     * \details 
     * Cached for quick access to upstream interface 
     */
    NETIO_Interface_T *upstr_intf; 

    /*ci 
     * \brief Bitmask of assorted writer-specific flags
     *  
     * \details 
     * Includes flags for reliability, active periodic HEARTBEAT event. 
     */
    RTPS_WriterFlags flags;

    /*ci 
     * \brief Sequence number of first (oldest) sample in writer queue
     *  
     * \details 
     * Updated by upstream interface that keeps state of writer queue 
     */
    RTPS_SampleId_T first_sn;

    /*ci 
     * \brief Sequence number of last (newest) sample in writer queue
     *  
     * \details 
     * Updated by upstream interface that keeps state of writer queue 
     */
    RTPS_SampleId_T last_sn;

#if RTPS_RELIABILITY

    /*ci \brief Epoch for next sent HEARTBEAT */
    RTI_UINT32 hb_epoch;

    /*ci \brief Periodic HEARTBEAT Rate */
    struct OSAPI_NtpTime hb_period;

    /*ci \brief Piggyback HEARTBEAT rate */
    RTI_INT32 samples_per_hb;

    /*ci \brief Count for next piggyback HEARTBEAT 
     * 
     * \details 
     * Initialized to zero, counts up, and piggyback HEARTBEAT 
     * sent when reaches samples_per_hb. 
     */
    RTI_INT32 piggyback_sample_count;

    /*ci \brief Event for periodic HEARTBEATs */
    OSAPI_TimeoutHandle_T hb_event; 

    /*ci \brief Number of inactive matched readers */
    RTI_INT32 inactive_reliable_reader_count;

    /*ci \brief Number of active matched readers */
    RTI_INT32 active_reliable_reader_count;

#endif /* RTPS_RELIABILITY */
};

/*ci 
 * \brief Local reader record 
 *  
 * \details 
 * State and configuration for an RTPS reader. 
 */
struct RTPS_Reader
{
    /*ci 
     * \brief Bitmask of assorted reader-specific flags
     *  
     * \details 
     * Includes flags for reliability.
     */
    RTPS_ReaderFlags flags;

#if RTPS_RELIABILITY
    /*ci \brief Epoch for next sent ACKNACK */
    RTI_UINT32 ack_epoch;

    /*ci 
     * \brief Number of entries reserved to fill holes created from receiving 
     * samples out of order. 
     *  
     * \details 
     * Because samples are presented in-order by the upper DDS layer, receiving 
     * a sample out of sequence number order will create holes that must be 
     * filled before the samples can be presented. 
     * For example, receiving SN 2 before SN 1 will create one hole for SN 1. 
     * reserved_count represents the number of entries that need to 
     * be reserved for holes.  Without reserved_count, receiving out of order 
     * samples can consume all resources of the upper layer, preventing samples 
     * from ever being received in order and stopping all further reception. 
     * If samples are always received in order, reserve_count will always be 
     * zero. 
     * The reader is keeping track of a reserved_count for each remote writer 
     * (see RTPS_RemoteWriter.reserved_count), and this reserved_count is 
     * the sum across all remote writers. 
     */
    RTI_INT32 reserved_count;

    /*ci \brief Event for periodic ACKNACKs */
    OSAPI_TimeoutHandle_T acknack_event;

    /*ci \brief The rate of sending pre-emptive ACKNACKs, seconds part
     */
    RTI_INT32 acknack_sec;

    /*ci \brief The rate of sending pre-emptive ACKNACKs, nanosecond part
     */
    RTI_INT32 acknack_nanosec;
#endif


};

/*************************************/

/*ci 
 * \brief RTPS peer record 
 *  
 * \details 
 * An RTPS peer is a matched remote writer or reader.  Each local RTPS writer 
 * or reader keeps state of each individual peer by using this type. 
 */
struct RTPS_PeerEntry
{
    /*ci \brief Peer GUID address */
    struct NETIO_Guid addr;

    /*ci \brief State of peer, either writer or reader */
    union {
        struct RTPS_RemoteReader reader;
        struct RTPS_RemoteWriter writer;
    } info;
};

/*ci 
 * \brief Context for processing a received message
 *  
 * \details 
 * Processing a message into its constituent RTPS submessages requires 
 * keeping some context, which this type stores. 
 */
struct RTPS_ReceiveContext
{
    /*ci \brief Current source GUID of submessages */
    RTPS_GUID src;

    /*ci \brief Current destination GUID of submessages */
    RTPS_GUID dst;

    /*ci \brief Number of DATA submessages in this message rejected by 
      upstream receive operation */
    RTI_UINT32 sample_rejected_count;
};

/*************************************/

/*ci 
 * \brief Representation of an RTPS endpoint or receiver
 *  
 * \details 
 * Implements the NETIO interface, and is an RT component. 
 */

struct RTPS_Interface
{
    /*ci \brief Base NETIO interface */
    struct NETIO_Interface _parent;

    /*ci \brief Interface property */
    struct RTPS_InterfaceProperty property;

    /*ci \brief Factory of this interface */
    struct RTPS_InterfaceFactory *factory;

    /*ci \brief Index of base NETIO route table, keyed on peer GUIDs */
    DB_Index_T rtable_peer_index;

    /*ci \brief Indexer of peers of this interface's endpoint */
    REDA_Indexer_T *peers_index;

    /*ci \brief Pool of peer entries */
    struct REDA_BufferPool *peers_pool; 

    /*ci \brief Context for processing received message */
    struct RTPS_ReceiveContext context; 

    /*ci \brief Packet for sending message created by this interface */
    NETIO_Packet_T *packet;

    /*ci \brief Buffer for this interface's packet */
    void *packet_buf; 
    
    /*ci \brief Local endpoint state */
    union
    {
        /*ci \brief Local RTPS writer state */
        struct RTPS_Writer writer;

        /*ci \brief Local RTPS reader state */
        struct RTPS_Reader reader;
    } endpoint;

    /*ci \brief Sequence of destinations to use for sending local packet */
    struct NETIO_AddressSeq local_dest_seq;

    /*ci
     * \brief An index sorting routes to a peer by priority. Only the highest
     *        priority route is used.
     */
    DB_Index_T direct_route;

    /*ci
     * \brief An index sorting routes to a group by priority. Only the highest
     *        priority routes are used.
     */
    DB_Index_T group_route;

    /*ci \brief Index of all the selected routes in a group (live data)
     */
    DB_Index_T selected_group_route_index;

    /*ci \brief If >= 0, index into checksum table for checksum function to use
     */
    RTI_INT32 checksum_index;
};

/*************************************/

/*ci 
 * \brief RT Component Factory for RTPS interfaces
 */
struct RTPS_InterfaceFactory
{
    /*ci \brief Base RT Component Factory */
    struct RT_ComponentFactory _parent;

    /*ci \brief System clock */
    struct OSAPI_System *clock;

    /*ci \brief Factory initialized flag, RTI_TRUE upon initialization */
    RTI_BOOL _initialized;

    /*ci \brief Indexer of RTPS external interfaces 
     * 
     * \details 
     * External interfaces map to DDS DomainParticipants, so RTPS writer and 
     * reader endpoints created by DDS entities belonging to the same 
     * DomainParticipant also share the same RTPS external interface. 
     * This indexer provides quick access to the external interface 
     * corresponding to an RTPS writer or reader. 
     */
    REDA_Indexer_T *ext_intf_index;

    /*ci \brief Available checksum functions. The +1 is the the CRC32 used
     *          by Core and is hidden.
     */
    RTPS_ChecksumClass_T checksum[RTPS_CHECKSUM_CLASS_MAX + 1];

    /*ci \brief Pointer to current RTPS factory property to use.
     */
    const struct RTPS_InterfaceFactoryProperty *property;
};

/******************************************************************************/

/*ci \brief Priority value assigned to a route */
typedef RTI_UINT8 RTPS_RoutePriority_T;

/*ci \brief Route entry for sending a packet to a peer 
 * 
 * \details 
 * As an NETIO interface, RTPS uses the NETIO route table to store its routes
 * to take when sending a message from itself to its peers. This is the type 
 * of each RTPS route entry. 
 */
struct RTPS_RouteEntry
{
    /*ci
     * \brief The address which can be reached. RTPS only cares about the GUID
     */
     struct NETIO_Guid destination;

     /*ci
      * \brief The downstream interface which can reach the destination GUID
      */
     NETIO_Interface_T *intf;

     /*ci
      * \brief The address the downstream interface should use the reach the GUID
      */
     struct NETIO_Address intf_address;

    /* ptr to peer state entry */
    struct RTPS_PeerEntry *peer_ref;

    /*ci
     * \brief Group route priority.  Higher priorities get routed to first
     */
    RTPS_RoutePriority_T group_priority;

    /*ci
     * \brief Group route priority.  Higher priorities get routed to first
     */
    RTPS_RoutePriority_T direct_priority;

    /*ci
     * \brief Whether this route is part of the broadcast group or not
     */
    RTI_BOOL selected_group_route;
};

/* Higher Value = Higher Priority */
#define RTPS_ROUTE_PRIORITY_UNDEFINED   0
#define RTPS_ROUTE_PRIORITY_UNICAST     1
#define RTPS_ROUTE_PRIORITY_MULTICAST   2

/******************************************************************************/
/*ci \brief Bind entry for receiving and forwarding packet  
 * 
 * \details 
 * As an NETIO interface, RTPS uses the NETIO bind table to store entries that
 * bind the RTPS interface with other NETIO interfaces that are its peers.
 */
struct RTPS_ExtBindEntry
{
    /*ci
     * \brief The source address of the peer session sending the packet
     */
    struct NETIO_Guid source;

    /*ci
     * \brief The entity address of the local session receiving the packet
     *        All RTPS session created from the same session have the same
     *        GUID prefix
     */
    struct NETIO_GuidEntity destination;

    /*ci
     * \brief The local RTPS interface with the entity id destination
     */
    struct RTPS_Interface *_rtps_intf;

    /*ci \brief RTPS peer reference
     *
     * \details
     * An RTPS bind operation between an RTPS external intf and RTPS reader
     * or reliable writer has a src_addr this is the GUID of the peer writer
     * or reader. This peer_ref is the cached pointer to the peer entry
     * of the src_addr, and is used for quick access to the peer when
     * processing submessages received from it.
     */
    struct RTPS_PeerEntry *peer_ref;
};

/*ci \brief Bind entry for receiving and forwarding packet
 *
 * \details
 * As an NETIO inteface, RTPS uses the NETIO bind table to store entries that
 * bind the RTPS interface with other NETIO interfaces, above or below in the
 * NETIO stack.  An RTPS interface will receive packets from below
 * interfaces binded to it, and it will forward received packets upstream
 * to binded interfaces above it.
 */
struct RTPS_IntBindEntry
{
    /*ci
     * \brief upstream/downstream interface to pass NETIO packets.
     */
    NETIO_Interface_T *intf;

    /*ci
     * \brief Reference counter when a writer is matched with multiple
     *        readers
     */
    RTI_INT32 ref_count;
};

/******************************************************************************/
/*ci \brief Max size of header for locally created packet */
#define RTPS_DOWNSTREAM_HEADER_MAX_SIZE (128)

/*ci \brief Max number of submessages in locally created packet.
 *  Currently 4: INFO_TS, INFO_DST, GAP, HEARTBEAT
 */
#define RTPS_LOCAL_PACKET_SUBMSG_MAX_COUNT  (4)

/*ci \brief Max size of locally created packet */
#define RTPS_LOCAL_PACKET_SIZE \
(RTPS_DOWNSTREAM_HEADER_MAX_SIZE + sizeof(struct RTPS_Header) + \
 (RTPS_LOCAL_PACKET_SUBMSG_MAX_COUNT * sizeof(union RTPS_MESSAGES)))

extern const RTI_UINT32 RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_NETWORK_ORDER;

extern const RTI_UINT32 RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_NETWORK_ORDER;

/******************************************************************************/
/*ci \brief Entity ID is Simple Discovery Protocol Participant Writer */
#define RTPSInterface_addr_is_SDP_Participant_sender(addr_) \
    (OSAPI_Memory_compare(&((addr_)->entity),\
                         &RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_NETWORK_ORDER,\
                         sizeof(RTI_UINT32)) == 0);


/*ci \brief Entity ID is Simple Discovery Protocol Participant Reader */
#define RTPSInterface_addr_is_SDP_Participant_receiver(addr_) \
        (OSAPI_Memory_compare(&((addr_)->entity),\
                             &RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_NETWORK_ORDER,\
                             sizeof(RTI_UINT32)) == 0);

/*ci \brief Byte swap based on RTPS submessage header's endian flag */
#ifdef RTI_ENDIAN_LITTLE
#define RTPSInterface_byte_swap(flags_) \
    ((flags_ & RTPS_ENDIAN_FLAG) == 0)
#else
#define RTPSInterface_byte_swap(flags_) \
    ((flags_ & RTPS_ENDIAN_FLAG) != 0)    
#endif

/******************************************************************************/
/*ci \brief Flags when sending packet */
typedef RTI_UINT8 RTPS_SendFlags_T; 

/*ci \brief Send DATA submesage */
#define RTPS_SEND_DATA_FLAG         0x01

/*ci \brief Send GAP submesage */
#define RTPS_SEND_GAP_FLAG          0x02

/*ci \brief Send HEARTBEAT submesage */
#define RTPS_SEND_HB_FLAG           0x04

/*ci \brief Send liveliness HEARTBEAT submesage */
#define RTPS_SEND_LIVE_HB_FLAG      0x08

/*ci \brief Enable pull-mode for historical samples 
 * 
 * \details 
 * Enabling pull mode does not send samples over transport, instead relies on 
 * Reader peer to NACK for resend. 
 */
#define RTPS_SEND_PULL_MODE_FLAG    0x10

/******************************************************************************/
/*ci \brief Context when writer is resending samples */
struct RTPS_ResendContext
{
    /*ci \brief Flags for sending packet */
    RTPS_SendFlags_T send_flags;

    /*ci \brief Writer entity ID */
    RTPS_Entity_T writer_entity;

    /*ci \brief Reader entity ID */
    RTPS_Entity_T reader_entity;

    /*ci \brief SN of sample being requested for resend */
    RTPS_SampleId_T req_sn;

    /*ci \brief SN of sample returned by request operation */
    RTPS_SampleId_T actual_sn;

    /*ci \brief Flag indicating a GAP submessage is being created */
    RTI_BOOL gap_in_progress;

    /*ci \brief Starting SN of a GAP submessage */
    RTPS_SampleId_T gap_start;

    /*ci \brief Ending SN of a GAP submessage */
    RTPS_SampleId_T gap_end;

    /*ci \brief RTI_TRUE when pull mode is enabled */
    RTI_BOOL pull_mode;

    /*ci \brief Flag whether a sample was resent */
    RTI_BOOL resent;
};

/*ci \brief Default-value initializer for RTPS_ResendContext */
#define RTPS_ResentContext_INITIALIZER \
{ \
    0, /* send_flags */ \
    RTPS_ENTITY_UNKNOWN, /* writer_entity */ \
    RTPS_ENTITY_UNKNOWN, /* reader_entity */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* req_sn */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* actual_sn */ \
    RTI_FALSE, /* gap_in_progress */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* gap_start */ \
    REDA_SEQUENCE_NUMBER_ZERO, /* gap_end */ \
    RTI_FALSE, /* pull_mode */ \
    RTI_FALSE /* resent */ \
}

/*ci \brief Maximum number of entries for RTPS external interfaces index */
#define RTPS_EXT_INTF_MAX_ENTRIES  16

/*ci \brief Maximum length for RTPS table name strings */
#define RTPS_TABLE_NAME_MAX_LEN    16

#define RTPS_Interface_checksum_calculate(intf_,c_index_,buf_,buf_s_,chksum_)\
         intf_->factory->checksum[c_index_].checksum_calculate(\
                intf_->factory->checksum[c_index_].context,\
                buf_,buf_s_,chksum_)
    

extern void
RTPS_Interface_set_submessage_header(struct RTPS_SubmsgHdr *header,
                                     RTI_UINT8 kind,
                                     RTI_UINT8 flags,
                                     RTI_SIZE_T length);

#endif /* RTPSInterface_pkg_h */
