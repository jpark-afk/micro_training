/*
 * FILE: RTPSDataFrag.h - Support for RTPS Fragmentation
 *
 * (c) Copyright, Real-Time Innovations, 2018-2023
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef RTPSDataFrag_h

#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef db_log_h
#include "db/db_log.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif
#ifndef rtps_config_h
#include "rtps/rtps_config.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef rtps_log_h
#include "rtps/rtps_log.h"
#endif

#include "RTPSInterface.h"

#define MAX_OUTSTANDING_FRAGMENTS_PER_SAMPLE (256)

struct RTPS_TxFragmentRecord;

struct RTPS_TxFragmentPacketState
{
    /*ci
     * \brief The timestamp the packet was published with
     */
    struct OSAPI_SystemTime timestamp;

    /*ci
     * \brief Pointer to the packet's header. The content must not be modified.
     */
    struct NETIO_PacketBuffer *head_pbuf;

    /*ci
     * \brief Pointer to the packet's tail. The content must not be modified.
     */
    struct NETIO_PacketBuffer *tail_pbuf;

    /*ci
     * \brief The original packets rtps_flags
     */
    RTI_UINT32 rtps_flags;

    /*ci
     * \brief The actual length of the sample.
     */
    RTI_UINT32 length;

    /*ci \brief The last fragment available for this SN.
     *
     */
    RTI_UINT32 last_fragment;

    /*ci
     * \brief Packet reference received from upstream packets. Used in
     *        return_loan() so upstream can clean up resources.
     */
    void *packet_ref;

    /*ci
     * \brief Number of entries referring to this SN
     */
    RTI_UINT32 ref_count;

    /*ci
     * \brief Flag to indicate if this packet has been marked as in-progress
     */
    RTI_BOOL in_progress;

    /*ci
     * \brief The last entry for this SN that was sent. When tokens are
     *        distributed it start where is left off to prevent token
     *        starvation.
     */
    struct RTPS_TxFragmentRecord *next_route;

};

struct RTPS_TxFragmentRouteState
{
    /*ci
     * \brief The current offset of the first unsent octet. Note that the
     *        offset does _not_inlude the inline qos. However, the encapsulation
     *        header is considered part of the fragmented payload.
     */
    RTI_UINT32 offset;

    /*ci \brief Resend bitmap
     *
     * \details
     * Information needed to re-send one or more fragments. As fragments are
     * resent the bitmap is cleared. 1s indicate a resend and 0s indicates
     * unknown state, do not do anything.
     */
    struct RTPS_Bitmap bitmap;

    /*ci
     * \brief TRUE if a packet send was rescheduled to resend fragments
     */
    RTI_BOOL nack_frag_resend;

    /*ci
     * \brief The fragment last sent on this route. Used to determine the
     *        correct fragment number to include in the next HEARTBEAT_FRAG.
     */
    RTI_UINT32 fragment_last;

    /*ci
     * \brief Reference to the packet-state for quick access. The packet
     *        state is only removed when there are no more route state for
     *        a specific SN. Thus this reference is safe.
     */
    struct RTPS_TxFragmentPacketState *packet_state;

    /*ci Indicate that the entry is complete so it can be removed after the
     *   last fragment has been sent.
     */
    RTI_BOOL is_completed;
};

/*ci
 * \brief State types in the fragment table. The key is the same for all
 *        states.
 */
typedef enum
{
    /*ci
     * \brief State information about the {SN}
     */
    RTPS_TXFRAGMENT_STATE_PACKET_KIND,

    /*ci
     * \brief State information about the {SN,route}
     */
    RTPS_TXFRAGMENT_STATE_ROUTE_KIND
} RTPS_TxFragmentStateKind_T;

union RTPS_TxFragmentState
{
    /*ci
     * \brief State information for a {SN}
     */
    struct RTPS_TxFragmentPacketState packet;

    /*ci
     * \brief State information for a {SN,route}
     */
    struct RTPS_TxFragmentRouteState route;
};
/*ci \brief Flag to signify the gap_sn_start field is to be treated as NULL */
#define RTPS_QUEUED_PACKET_SN_START_NULL_FLAG           0x01U

/*ci \brief Flag to signify the gap_sn_end field is to be treated as NULL */
#define RTPS_QUEUED_PACKET_SN_END_NULL_FLAG             0x02U

/*ci \brief Flag to signify reader_entity field is to be treated as NULL */
#define RTPS_QUEUED_PACKET_RDR_ENTITY_NULL_FLAG         0x04U

/*ci \brief Flag to signify if the serialized payload has valid data */
#define RTPS_QUEUED_PACKET_VALID_DATA_FLAG              0x08U

/*ci \brief Flag to signify that this is a Liveliness packet */
#define RTPS_QUEUED_PACKET_LIVELINESS_FLAG         0x10U



/*ci
 * \brief A node in the list for queued packets.
 *
 * \details This node saves the function call's state to
 *          RTPS_Sender_route_packet when called with the send_mode set to
 *          RTPS_SENDMODE_QUEUE_PACKET. It preserves all the relevant information from
 *          the packet and parameters used during the call to RTPS_Sender_route_packet.
 *          This preservation enables the function, RTPS_Sender_route_packet to be
 *          called again asynchronously from a different thread.
 */
typedef struct RTPS_QueuedPacketNode
{
    /*ci \brief Parent node for the circular list */
    struct REDA_CircularListNode _parent;

    /*ci \brief Sequence of destinations for single packet */
    struct NETIO_AddressSeq *dests;

    /*ci \brief The first PacketBuffer in the NETIO_Packet */
    struct NETIO_PacketBuffer *head_pbuf;

    /*ci \brief The last PacketBuffer in the NETIO_Packet */
    struct NETIO_PacketBuffer *tail_pbuf;

    /*ci \brief Application level reference pointer to be able to retrieve
     *          content based on the NETIO_Packet.
     */
    void *ref;

    /*ci \brief The SN of the packet */
    struct REDA_SequenceNumber sn;

    /*ci \brief The time-stamp of the packet */
    struct OSAPI_SystemTime timestamp;

    /*ci \brief Peer entry for the packet. Can be NULL */
    struct RTPS_PeerEntry *peer_entry;

    /*ci \brief Starting sequence number if this is GAP packet */
    RTPS_SampleId_T gap_sn_start;

    /*ci \brief Ending sequence number if this is GAP packet */
    RTPS_SampleId_T gap_sn_end;

    /*ci \brief The maximum number of bytes buffer can hold */
    RTI_SIZE_T max_length;

    /*ci \brief The current position of head cursor in buffer */
    RTI_SIZE_T head_pos;

    /*ci \brief The current position of tail cursor in buffer */
    RTI_SIZE_T tail_pos;

    /*ci \brief Reader Entity for this packet */
    RTPS_Entity_T reader_entity;

    /*ci \brief Flags describing the contents of the payload */
    RTI_UINT32 rtps_flags;

    /*ci \brief Encapsulation of the payload */
    RTI_UINT16 encapsulation;

    /*ci \brief Send flags for the sned call */
    RTPS_SendFlags_T send_flags;

    /*ci \brief A bitmap to represent state of the parameters and valid_data
     *          when this node was created.
     */
    RTI_UINT8 node_bitmap;
}RTPS_QueuedPacketNode;

struct RTPS_TxFragmentRecord
{
    struct REDA_CircularListNode _parent;

    /*ci
     * \brief The sequence number being fragmented
     * key
     */
    struct REDA_SequenceNumber sn;

    /*ci
     * \brief The route to send to
     * key
     */
    struct RTPS_RouteEntry *route_entry;

    /*ci
     * \brief The peer to send to. Note that route_entry has a peer, however,
     *        locators may be shared by multiple readers in which case the
     *        peer in the route_entry is not valid. Instead this peer_entry
     *        is used.
     */
    struct RTPS_PeerEntry *peer_entry;

    /*ci
     * \brief The kind of entry
     *  - MASTER is the "maintenance" record with one per sequence and a
     *    route_entry = NULL.
     *  - ROUTE  is the per peer entry. A peer may have more than one route.
     */
    RTPS_TxFragmentStateKind_T kind;

    /*ci
     * \brief Entry specific state.
     *
     */
    union RTPS_TxFragmentState state;

    /*ci
     * \brief Encapsulation to use on route. The entity adding a route
     *        determines the encapsulation.
     */
    RTI_UINT16 encapsulation;
};

struct RTPS_TxFragmentRecordKey
{
    struct REDA_CircularListNode _parent;

    /*ci
     * \brief The sequence number being fragmented
     * key
     */
    struct REDA_SequenceNumber sn;

    /*ci
     * \brief The route to send to
     * key
     */
    struct RTPS_RouteEntry *route_entry;

    /*ci
     * \brief peer to send to
     */
    struct RTPS_PeerEntry *peer_entry;
};

extern RTI_BOOL
RTPS_Sender_is_mtu_exceeded(struct RTPS_RouteEntry *route_entry,
                               RTI_SIZE_T bytes);

extern RTI_BOOL
RTPS_Sender_queue_packet(struct RTPS_Interface *intf,
                         NETIO_Packet_T *packet,
                         struct RTPS_PeerEntry *peer_entry,
                         RTPS_SendFlags_T send_flags,
                         RTPS_Entity_T *reader_entity,
                         RTPS_SampleId_T *gap_sn_start,
                         RTPS_SampleId_T *gap_sn_end);

extern RTI_BOOL
RTPS_Sender_schedule_packet(struct RTPS_Interface *intf,
                               NETIO_Packet_T *packet,
                               struct RTPS_Bitmap *bitmap,
                               struct RTPS_RouteEntry *route_entry,
                               struct RTPS_PeerEntry *peer_entry,
                               RTI_BOOL schedule_flow);

extern NETIO_Packet_T*
RTPS_Sender_get_next_payload(struct RTPS_Interface *intf,
                            struct RTPS_TxFragmentRecord *tx_entry,
                            RTI_UINT32 max_bits,
                            union RTPS_MESSAGES *fragment,
                            RTI_BOOL *last_fragment);

extern RTI_INT32
RTPS_Sender_compare_tx_frag(RTI_INT32 flags,const DB_Record_T op1, void *key);

extern void
RTPS_Sender_delete_txtable_entry(struct RTPS_Interface *intf,
                                 struct RTPS_TxFragmentRecord *entry);

extern void
RTPS_Sender_deschedule_packet(struct RTPS_Interface *intf,
                              NETIO_PacketId_T *packet_id);

extern void
RTPS_Sender_delete_txtable_peer_entry(struct RTPS_Interface *intf,
                                      struct RTPS_PeerEntry *peer_entry,
                                      struct RTPS_RouteEntry *route_entry);

extern RTI_BOOL
RTPS_Sender_intialize_datafrag(struct RTPS_Interface *rtps_intf,
                       const struct RTPS_InterfaceProperty *const property);

extern void
RTPS_Sender_finalize_datafrag(struct RTPS_Interface *rtps_intf, RTI_BOOL with_nw_lock);

extern void
RTPS_Sender_process_nack_frag_ext(struct RTPS_Interface *intf,
                                  struct RTPS_PeerEntry *peer_entry,
                                  RTPS_SampleId_T *writer_sn,
                                  struct RTPS_Bitmap *bitmap);

extern void
RTPS_Sender_send_heartbeat_frag(struct RTPS_Interface *rtps_intf);

extern void
RTPS_Sender_add_heartbeat_frag_ext(struct RTPS_Interface *intf,
                                   RTPS_Entity_T reader_entity,
                                   RTPS_Entity_T writer_entity);

extern RTI_BOOL
RTPS_Sender_is_packet_scheduled(struct RTPS_Interface *intf,
                                RTPS_SampleId_T *sample_id);

extern RTI_UINT32
RTPS_Sender_send_fragments(struct RTPS_Interface *intf,RTI_UINT32 max_bits);

extern RTI_BOOL
RTPS_Sender_is_queue_empty(struct RTPS_Interface *intf);

struct RTPS_RxFragmentData
{
    struct NETIO_PacketBuffer pbuf;

    /*ci
     * \brief The first fragment number in this pbuf (in case multiple
     *        fragments are aggregated in the same sample.
     */
    RTI_UINT32 first_fn;

    /*ci
     * \brief The last fragment number in this pbuf.
     */
    RTI_UINT32 last_fn;
};

typedef REDA_CircularList_T RTPS_RxFragmentList;

#define RTPS_RxFragmentList_init(l_) \
                REDA_CircularList_init(&(l_))

#define RTPS_RxFragmentNode_init(n_) \
                REDA_CircularListNode_init(&(n_)->_node)

#define RTPS_RxFragmentList_link_node_after(a_,n_) \
                REDA_CircularList_link_node_after(&(a_)->_node,&(n_)->_node)

#define RTPS_RxFragmentList_unlink_node(n_) \
                REDA_CircularList_unlink_node(&(n_)->_node)

#define RTPS_RxFragmentList_append(l_,n_) \
                REDA_CircularList_append((l_),&(n_)->_node)

#define RTPS_RxFragmentNode_prepend(l_,n_) \
                REDA_CircularList_prepend((l_),&(n_)->_node)

#define RTPS_RxFragmentList_is_empty(l_) \
                REDA_CircularList_is_empty((l_))

#define RTPS_RxFragmentList_node_at_head(l_,n_) \
                REDA_CircularList_node_at_head((l_),&(n_)->_node)

#define RTPS_RxFragmentList_get_first(l_) \
        (struct RTPS_RxFragmentData*)REDA_CircularList_get_first((l_))

#define RTPS_RxFragmentList_get_last(l_) \
        (struct RTPS_RxFragmentData*)REDA_CircularList_get_last((l_))

#define RTPS_RxFragmentNode_as_pbuf(n_) (&(n_)->pbuf)

#define RTPS_RxFragmentNode_get_next(n_) \
    (struct RTPS_RxFragmentData*)((n_)->pbuf._next)

#define RTPS_RxFragmentNode_get_prev(n_) \
    (struct RTPS_RxFragmentData*)REDA_CircularListNode_get_prev(&(n_)->_node)

#define RTPS_RxFragment_get_distance(f_,l_) ((f_) - (l_) + 1)

#define RTPS_RxFragment_get_fn(f_,l_) ((f_) + (l_) - 1)

struct RTPS_RxFragmentRecord
{
    /*ci \brief The GUID of the remote writer publishing this RTPS sample
     */
    struct NETIO_Guid writer;

    /*ci \brief The sequence number for the RTPS sample being reassembled
     */
    struct REDA_SequenceNumber sn;

    struct REDA_SequenceNumber writer_lead;

    struct RTPS_RxFragmentData *head_frag;

    struct RTPS_RxFragmentData *tail_frag;

    RTI_UINT32 total_fragment_count;

    RTI_INT32 current_fragment_count;

    RTI_UINT32 flags;

    RTI_BOOL byte_swap;

    struct OSAPI_SystemTime timestamp;

    /*ci \brief State of up to 256 fragments
     */
    struct RTPS_Bitmap bitmap;

    RTI_UINT32 highest_sn;

    RTI_UINT32 ref_count;

    RTI_UINT32 reliable_count;

    RTI_INT32 reserved_pbuf_count;

    struct RTPS_RxFragmentRecord *zero_record;
};

MUST_CHECK_RETURN RTI_BOOL
RTPS_Receiver_process_data_frag(struct RTPS_Interface *intf,
                                struct RTPS_PeerEntry *peer_entry,
                                struct NETIO_Guid *writer,
                                NETIO_Packet_T *packet,
                                RTI_UINT8 flags,
                                RTI_UINT32 data_len,
                                struct RTPS_DATA_FRAG *fragment,
                                RTI_BOOL byte_swap);

extern RTI_BOOL
RTPS_Receiver_initialize_datafrag(struct RTPS_Interface *intf,
                        const struct RTPS_InterfaceProperty *const property);

extern void
RTPS_Receiver_finalize_datafrag(struct RTPS_Interface *intf);


extern void
RTPS_Receiver_update_acknack_from_rxtable(struct RTPS_Interface *intf,
                                          struct NETIO_Guid *writer,
                                          struct RTPS_ACKNACK *acknack);

extern void
RTPS_Receiver_process_heartbeat_ext(struct RTPS_Interface *intf,
                                    struct RTPS_Interface *local_intf,
                                    struct RTPS_PeerEntry *peer_entry,
                                    RTPS_SampleId_T *sn_first,
                                    RTPS_SampleId_T *sn_last);

extern void
RTPS_Receiver_process_heartbeat_frag_ext(struct RTPS_Interface *intf,
                                         struct RTPS_Interface *local_intf,
                                         struct RTPS_PeerEntry *peer_entry,
                                         RTPS_SampleId_T *writer_sn,
                                         RTI_UINT32 last_fn);

extern void
RTPS_Receiver_process_gap_ext(struct RTPS_Interface *intf,
                                      struct NETIO_Guid *writer,
                                      RTPS_SampleId_T *sn_start,
                                      RTPS_SampleId_T *sn_end,
                                      struct RTPS_Bitmap *bitmap);

extern void
RTPS_Receiver_get_nack_frags(struct RTPS_Interface *intf,
                             struct RTPS_PeerEntry *peer_entry,
                             union RTPS_MESSAGES *msg,
                             RTI_INT32 *msglen,
                             struct REDA_SequenceNumber *last_sn);

extern void
RTPS_Receiver_datafrag_writer_unbind(struct RTPS_Interface *intf,
                                     struct NETIO_Guid *guid,
                                     RTI_BOOL is_reliable);

extern void
RTPS_Receiver_datafrag_writer_bind(struct RTPS_Interface *intf,
                                   struct NETIO_Guid *guid,
                                   RTI_BOOL is_reliable);

#endif
