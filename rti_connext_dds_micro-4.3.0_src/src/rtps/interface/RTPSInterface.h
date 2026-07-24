/*
 * FILE: RTPSInterface.h - Exported RTPS_Interface functions
 *
 * Copyright (c) 2008-2026, Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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
#ifndef netio_flowcontroller_h
#include "netio/netio_flowcontroller.h"
#endif

#include "../checksum/RTPSChecksum.h"

/******************************************************************************/

/*ci \brief Offset used when deserializing received DATA submessage */
#define RTPS_RCV_DATA_SUBMSG_OFFSET       24U

/*ci \brief Offset used when deserializing received DATA_BATCH submessage */
#define RTPS_RCV_DATA_BATCH_SUBMSG_OFFSET 44U

/*ci \brief Offset used when deserializing received DATA submessage */
#define RTPS_RCV_DATA_FRAG_SUBMSG_OFFSET  36U

/*ci \brief Offset used when deserializing received non-DATA submessage */
#define RTPS_RCV_NON_DATA_SUBMSG_OFFSET   12U

/*ci \brief Size in bytes of RTPS Entity ID */
#define RTPS_ENTITY_ID_LENGTH 4U

/*ci \brief Size in bytes of Length field in RTPS Header extension */
#define RTPS_HEADER_EXTN_LENGTH_SIZE 4

/*ci \brief Sample ID type */
typedef struct REDA_SequenceNumber RTPS_SampleId_T;

/******************************************************************************/

/*ci \brief Minimum length of unknown RTPS submessage */
#define RTPS_SUBMSG_MIN_LEN_DEFAULT     0xff

/*ci \brief Minimum length of RTPS PAD submessage */
#define RTPS_SUBMSG_MIN_LEN_PAD         0

/*ci \brief Minimum length of RTPS ACKNACK submessage */
#define RTPS_SUBMSG_MIN_LEN_ACKNACK     24

/*ci \brief Minimum length of RTPS HEARTBEAT submessage */
#define RTPS_SUBMSG_MIN_LEN_HB          28

 /*ci \brief Minimum length of RTPS HEARTBEAT_FRAG submessage */
#define RTPS_SUBMSG_MIN_LEN_HB_FRAG     24

/*ci \brief Minimum length of RTPS HEARTBEAT submessage */
#define RTPS_SUBMSG_MIN_LEN_HB_BATCH    44

/*ci \brief Minimum length of RTPS GAP submessage */
#define RTPS_SUBMSG_MIN_LEN_GAP         28

/*ci \brief Minimum length of RTPS INFO_SRC submessage */
#define RTPS_SUBMSG_MIN_LEN_INFO_TS     0

/*ci \brief Minimum length of RTPS INFO_SRC submessage */
#define RTPS_SUBMSG_MIN_LEN_INFO_SRC    20

/*ci \brief Minimum length of RTPS INFO_DST submessage */
#define RTPS_SUBMSG_MIN_LEN_INFO_DST    12

/*ci \brief Minimum length of RTPS INFO_REPLY submessage */
#define RTPS_SUBMSG_MIN_LEN_INFO_REPLY  12

/*ci \brief Minimum length of RTPS DATA submessage */
#define RTPS_SUBMSG_MIN_LEN_DATA        20

/*ci \brief Minimum length of RTPS DATA submessage */
#define RTPS_SUBMSG_MIN_LEN_DATA_FRAG   32

/*ci \brief Minimum length of RTPS NACK_FRAG submessage */
#define RTPS_SUBMSG_MIN_LEN_NACK_FRAG   28

/*ci \brief Minimum length of RTPS DATA submessage */
#define RTPS_SUBMSG_MIN_LEN_DATA_BATCH  40

/*ci \brief Largest value of supported submessage kind */
#define RTPS_SUBMSG_KIND_HIGH           26

/********************************************************************************/

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
    RTI_INT32 size;

    /*ci \brief Max count of entries */
    RTI_INT32 max_size;

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

void
RTPS_Window_remove(struct RTPS_Window *window,
                   RTI_BOOL *removed,
                   RTPS_SampleId_T *sn);

#define RTPS_REMOTEENDPOINT_MASK_IS_RELIABLE         0x1u
#define RTPS_REMOTEENDPOINT_MASK_IS_BACTHED          0x2u
#define RTPS_REMOTEENDPOINT_MASK_IS_READER           0x4u

#define RTPS_REMOTEREADER_MASK_REPAIR_STUCK_NACK     0x8u
#define RTPS_REMOTEREADER_MASK_INACTIVE              0x10u

#define RTPS_RemoteEndpoint_set_reader(re_) \
((re_)->parent.status |= RTPS_REMOTEENDPOINT_MASK_IS_READER)

#define RTPS_RemoteWriter_set_reliable(rw_) \
((rw_)->parent.status |= RTPS_REMOTEENDPOINT_MASK_IS_RELIABLE)

#define RTPS_RemoteWriter_set_batched(rw_) \
((rw_)->parent.status |= RTPS_REMOTEENDPOINT_MASK_IS_BACTHED)

#define RTPS_RemoteEndpoint_is_reader(re_) \
((re_)->status & RTPS_REMOTEENDPOINT_MASK_IS_READER)

#define RTPS_RemoteWriter_is_reliable(rw_) \
((rw_)->parent.status & RTPS_REMOTEENDPOINT_MASK_IS_RELIABLE)

#define RTPS_RemoteWriter_is_batched(rw_) \
((rw_)->parent.status & RTPS_REMOTEENDPOINT_MASK_IS_BACTHED)

#define RTPS_RemoteReader_is_reliable(rr_) \
((rr_)->parent.status & RTPS_REMOTEENDPOINT_MASK_IS_RELIABLE)

#define RTPS_RemoteReader_set_reliable(rr_) \
((rr_)->parent.status |= RTPS_REMOTEENDPOINT_MASK_IS_RELIABLE)

#define RTPS_RemoteReader_set_repair_stuck_nack(rr_) \
((rr_)->parent.status |= RTPS_REMOTEREADER_MASK_REPAIR_STUCK_NACK)

#define RTPS_RemoteReader_clear_repair_stuck_nack(rr_) \
((rr_)->parent.status &= ~RTPS_REMOTEREADER_MASK_REPAIR_STUCK_NACK)

#define RTPS_RemoteReader_is_repair_stuck_nack(rr_) \
((rr_)->parent.status & RTPS_REMOTEREADER_MASK_REPAIR_STUCK_NACK)

#define RTPS_RemoteReader_set_inactive(rr_) \
((rr_)->parent.status |= RTPS_REMOTEREADER_MASK_INACTIVE)

#define RTPS_RemoteReader_clear_inactive(rr_) \
((rr_)->parent.status &= ~RTPS_REMOTEREADER_MASK_INACTIVE)

#define RTPS_RemoteReader_is_inactive(rr_) \
((rr_)->parent.status & RTPS_REMOTEREADER_MASK_INACTIVE)

struct RTPS_RemoteEndpoint
{
    RTI_UINT32 status;
};

struct RTPS_RemoteReaderReliableState
{
    /*ci
     * \brief Epoch of last received ACKNACK.
     *
     * \details
     * Every ACKNACK contains an epoch that is increased with each new
     * ACKNACK.  This is cached and used to ignore ACKNACKs with older epochs.
     */
    #define RTPS_RemoteReaderReliable_epoch(rr_) \
        (rr_)->reliable_state->epoch
    REDA_Epoch_T epoch;

    /*ci
     * \brief
     * Sequence number of last (newest) acknowledged sample.
     *
     * \details
     * Is one before the starting SN of send window (i.e. oldest unacknowledged
     * sample) to this remote reader. Is non-decreasing.
     */
    #define RTPS_RemoteReaderReliable_last_acked_sn(rr_) \
        (rr_)->reliable_state->last_acked_sn
    RTPS_SampleId_T last_acked_sn;

#if DDS_FILTERING_ENABLED
    /*ci
     * \brief If the writer is performing filtering, this is the last sequence
     *        number that was sent to the reader.
     */
    #define RTPS_RemoteReaderReliable_last_sent_sn(rr_) \
        (rr_)->reliable_state->last_sent_sn
    RTPS_SampleId_T last_sent_sn;

#endif /* DDS_FILTERING_ENABLED */

    /*ci
     * \brief
     * Countdown for Reader becoming inactive.
     *
     * \details
     * Reset when ACKNACK is received.  Decremented every HEARTBEAT period.
     * Reader becomes inactive when count reaches zero.
     */
    #define RTPS_RemoteReaderReliable_inactive_count(rr_) \
        (rr_)->reliable_state->inactive_count
    RTI_INT32 inactive_count;

    /*ci
     * \brief Send window of unacknowledged samples.
     *
     * \details
     * Used by reliable RTPS writer to track samples sent and
     * acknowledged by each remote reader. Each sent sample takes one entry in
     * the window.  Window has a finite size and can become full.
     */
    #define RTPS_RemoteReaderReliable_window(rr_) (rr_)->reliable_state->window
    struct RTPS_Window window;

    /*ci
     * \brief Saved initial last acked sn from when the peer was first asserted
     */
    #define RTPS_RemoteReaderReliable_initial_last_acked_sn(rr_) \
        (rr_)->reliable_state->initial_last_acked_sn
    RTPS_SampleId_T initial_last_acked_sn;

    /*ci
     * \brief Epoch of last received NACK_FRAG.
     *
     * \details
     * Every NACK_FRAG contains an epoch that is increased with each new
     * NACK_FRAG.  This is cached and used to ignore NACK_FRAG with older
     * epochs.
     */
    REDA_Epoch_T nack_frag_epoch;
    #define RTPS_RemoteReaderReliable_nack_frag_epoch(rr_) \
        (rr_)->reliable_state->nack_frag_epoch
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
    /*ci \brief Base-class
     */
    struct RTPS_RemoteEndpoint parent;

    /*ci \brief state for a reliable RemoteReader
     */
    struct RTPS_RemoteReaderReliableState *reliable_state;
};

struct RTPS_RemoteWriterReliableState
{

    /*ci \brief Set RTI_TRUE after receiving first HEARTBEAT from writer */
    #define RTPS_RemoteWriterReliable_rcvd_first_hb(rw_) \
        (rw_)->reliable_state->rcvd_first_hb
    RTI_BOOL rcvd_first_hb;

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
    #define RTPS_RemoteWriterReliable_bitmap(rw_) \
        (rw_)->reliable_state->bitmap
    struct RTPS_Bitmap bitmap;

    /*ci
     * \brief Number of entries reserved to fill holes created from receiving
     * samples out of order.
     *
     * \details
     * See RTPS_Reader.reserved_count for a description of reserved count.
     * This reserved_count applies for this remote writer and is non-zero when
     * a sample is received out of order from this remote writer.
     */
    #define RTPS_RemoteWriterReliable_reserved_count(rw_) \
        (rw_)->reliable_state->reserved_count
    RTI_INT32 reserved_count;

    /*ci
     * \brief Epoch of last received HEARTBEAT.
     *
     * \details
     * Every HEARTBEAT contains an epoch that is increased with each new
     * HEARTBEAT. This is cached and used to ignore HEARTBEATs with older
     * epochs.
     */
    #define RTPS_RemoteWriterReliable_epoch(rw_) \
        (rw_)->reliable_state->epoch
    REDA_Epoch_T epoch;

    /*ci
     * \brief Epoch of last received HEARTBEAT_FRAG.
     *
     * \details
     * Every HEARTBEAT contains an epoch that is increased with each new
     * HEARTBEAT. This is cached and used to ignore HEARTBEATs with older
     * epochs.
     */
    #define RTPS_RemoteWriterReliable_epoch_hb_frag(rw_) \
        (rw_)->reliable_state->epoch_hb_frag
    REDA_Epoch_T epoch_hb_frag;
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
    /*ci \brief base-class
     */
    struct RTPS_RemoteEndpoint parent;

    /*ci \brief State for a reliable remote writer
     */
    struct RTPS_RemoteWriterReliableState *reliable_state;

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
};

/*\ci \brief Shared state for local endpoints
 */
struct RTPS_Endpoint
{
    /*ci \brief Index of base NETIO route table, keyed on peer GUIDs
     */
    DB_Index_T rtable_peer_index;

    /*ci \brief Indexer of peers of this interface's endpoint
     */
    REDA_Indexer_T *peers_index;

    /*ci \brief Pool of peer entries */
    struct REDA_BufferPool *peers_pool;

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

    /*ci \brief Reference count for upstr_intf
     */
    RTI_UINT32 upstr_intf_ref_count;

    /*ci \brief the endpoint priority
     */
    RTI_INT32 transport_priority;
};

struct RTPS_WriterReliableState
{
    /*ci \brief Epoch for next sent HEARTBEAT
     */
    #define RTPS_WriterReliable_hb_epoch(wr_) \
        (wr_)->reliable_state->hb_epoch
    RTI_UINT32 hb_epoch;

    /*ci \brief Periodic HEARTBEAT Rate
     */
    #define RTPS_WriterReliable_hb_period(wr_) \
        (wr_)->reliable_state->hb_period
    struct OSAPI_SystemTime hb_period;

    /*ci \brief Piggyback HEARTBEAT rate
     */
    #define RTPS_WriterReliable_samples_per_hb(wr_) \
        (wr_)->reliable_state->samples_per_hb
    RTI_INT32 samples_per_hb;

    /*ci \brief Event for periodic HEARTBEATs
     */
    #define RTPS_WriterReliable_hb_event(wr_) \
        (wr_)->reliable_state->hb_event
    OSAPI_TimeoutHandle_T hb_event;

    /*ci \brief Number of inactive matched readers
     */
    #define RTPS_WriterReliable_inactive_reliable_reader_count(wr_) \
                        (wr_)->reliable_state->inactive_reliable_reader_count
    RTI_INT32 inactive_reliable_reader_count;

    /*ci \brief Number of active matched readers
     */
    #define RTPS_WriterReliable_active_reliable_reader_count(wr_) \
                        (wr_)->reliable_state->active_reliable_reader_count
    RTI_INT32 active_reliable_reader_count;

    /*ci \brief Maximum number of HB retries before a remote reader is inactive
     */
    #define RTPS_WriterReliable_max_hb_retries(wr_) \
        (wr_)->reliable_state->max_hb_retries
    RTI_INT32 max_hb_retries;

    /*ci \brief The maximum writer window size
     */
    #define RTPS_WriterReliable_max_window_size(wr_) \
        (wr_)->reliable_state->max_window_size
    RTI_INT32 max_window_size;
};

struct RTPS_WriterFragmentState
{

    /*ci
     * \brief Table of all currently fragmented samples
     */
    #define RTPS_WriterFragment_tx_frag_table(wr_) \
        (wr_)->fragment_state->tx_frag_table
    DB_Table_T tx_frag_table;

    /*ci \brief A pool of packets which can be queued
     */
    #define RTPS_WriterFragment_packet_queue_pool(wr_) \
        (wr_)->fragment_state->packet_queue_pool
    struct REDA_BufferPool *packet_queue_pool;

    /*ci \brief The flow-controller used
     */
    #define RTPS_WriterFragment_netio_fc(wr_) \
        (wr_)->fragment_state->netio_fc
    NETIO_FlowController *netio_fc;

    /*ci \brief The numnber of bytes in a fragmented sample
     */
    #define RTPS_WriterFragment_fragment_size_bytes(wr_) \
        (wr_)->fragment_state->fragment_size_bytes
    RTI_UINT16 fragment_size_bytes;

    /*ci \brief Temporary pbufs to link together fragmented samples
     */
    #define RTPS_WriterFragment_frag_pbuf(wr_) \
        (wr_)->fragment_state->frag_pbuf
    struct NETIO_PacketBuffer frag_pbuf[2];

    /*ci \brief List of samples to be flow-controlled and/or fragmented
     */
    #define RTPS_WriterFragment_tx_list(wr_) \
        (wr_)->fragment_state->tx_list
    REDA_CircularList_T tx_list;

    /*ci \brief The current sample being sent from the fragmentation table
     */
    #define RTPS_WriterFragment_tx_entry_cur(wr_) \
        (wr_)->fragment_state->tx_entry_cur
    struct RTPS_TxFragmentRecord *tx_entry_cur;

    /*ci \brief A queue of packet to be sent
     */
    #define RTPS_WriterFragment_packet_queue(wr_) \
        (wr_)->fragment_state->packet_queue
    REDA_CircularList_T packet_queue;

    /*ci \brief  Lock to protect the packe_queue
     */
    #define RTPS_WriterFragment_packet_queue_lock(wr_) \
        (wr_)->fragment_state->packet_queue_lock
    struct OSAPI_Mutex *packet_queue_lock;

    /*ci \brief Scheduling handle for this interface, only used by sender
     */
    #define RTPS_WriterFragment_sched_handle(wr_) \
        (wr_)->fragment_state->sched_handle
    NETIO_FlowControllerFlowHandle sched_handle;
};

/*ci
 * \brief Local writer record
 *
 * \details
 * State and configuration for an RTPS writer.
 */
struct RTPS_Writer
{
    struct RTPS_Endpoint _parent;

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

    /*ci  \brief The last completed SN
     */
    #define RTPS_Writer_last_completed_sn(wr_) (wr_)->last_completed_sn
    struct REDA_SequenceNumber last_completed_sn;

    struct RTPS_WriterReliableState *reliable_state;

    /*ci \brief Optional fragmentation state if the RTPS writer supports
     *   fragmented and flow-control
     */
    struct RTPS_WriterFragmentState *fragment_state;

#if DDS_FILTERING_ENABLED
    /*ci \brief Optional writer-filter state if the RTPS writer is filtered
     */
    struct RTPS_FilterPluginWriterFilter *writer_filter;

    /*ci \brief Optional filter plugin if the RTPS writer is filtered
     */
    struct RTPS_FilterPlugin *filter_plugin;
#endif
};

/*ci
 * \brief Reliable state for a local Writer
 */
struct RTPS_ReaderReliableState
{
    /*ci \brief Epoch for next sent ACKNACK
     */
    #define RTPS_ReaderReliable_ack_epoch(r_) \
        (r_)->reliable_state->ack_epoch
    RTI_UINT32 ack_epoch;

    /*ci \brief Epoch for next sent NACK_FRAG
     */
    #define RTPS_ReaderReliable_nack_frag_epoch(r_) \
        (r_)->reliable_state->nack_frag_epoch
    RTI_INT32 nack_frag_epoch;

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
    #define RTPS_ReaderReliable_reserved_count(r_) \
        (r_)->reliable_state->reserved_count
    RTI_INT32 reserved_count;

    /*ci \brief Event for periodic ACKNACKs
     */
    #define RTPS_ReaderReliable_acknack_event(r_) \
        (r_)->reliable_state->acknack_event
    OSAPI_TimeoutHandle_T acknack_event;

    /*ci \brief The rate of sending pre-emptive ACKNACKs, seconds part
     */
    #define RTPS_ReaderReliable_acknack_sec(r_) (\
            r_)->reliable_state->acknack_sec
    RTI_INT32 acknack_sec;

    /*ci \brief The rate of sending pre-emptive ACKNACKs, nanosecond part
     */
    #define RTPS_ReaderReliable_acknack_nanosec(r_) \
        (r_)->reliable_state->acknack_nanosec
    RTI_INT32 acknack_nanosec;

    /*ci \brief The NACK period for preemptive NACKs
     */
    #define RTPS_ReaderReliable_nack_period(r_) \
        (r_)->reliable_state->nack_period
    struct OSAPI_SystemTime nack_period;

    /*ci \brief The maximum receive window
     */
    #define RTPS_ReaderReliable_max_window_size(r_) \
        (r_)->reliable_state->max_window_size
    RTI_INT32 max_window_size;

    /* ci \brief The maximum number of outstanding samples
     */
    #define RTPS_ReaderReliable_max_samples(r_) \
        (r_)->reliable_state->max_samples
    RTI_INT32 max_samples;
};

/*ci
 * \brief Fragmentation state for a local Writer
 */
struct RTPS_ReaderFragmentState
{
    /*ci \brief The maximum number of samples a writer can have outstanding
     */
    #define RTPS_ReaderFragment_max_fragmented_samples_per_remote_writer(r_) \
            (r_)->fragment_state->max_fragmented_samples_per_remote_writer
    RTI_INT32 max_fragmented_samples_per_remote_writer;

    /*ci \brief The total number of fragmented bytes
     */
    #define RTPS_ReaderFragment_rcv_fragment_size_bytes(r_) \
            (r_)->fragment_state->rcv_fragment_size_bytes
    RTI_UINT32 rcv_fragment_size_bytes;

    /*ci \brief The maximum number of fragmented samples that can be received
     */
    #define RTPS_ReaderFragment_max_fragmented_samples(r_) \
            (r_)->fragment_state->max_fragmented_samples
    RTI_INT32 max_fragmented_samples;

    /*ci \brief Table of currently fragmented samples
     */
    #define RTPS_ReaderFragment_data_frag_table(r_) \
            (r_)->fragment_state->data_frag_table
    DB_Table_T data_frag_table;

    /*ci \brief Pool of fragmented samples
     */
    #define RTPS_ReaderFragment_pbuf_pool(r_) \
        (r_)->fragment_state->pbuf_pool
    REDA_BufferPool_T pbuf_pool;

    /*ci \brief Number of fragmented samples
     */
    #define RTPS_ReaderFragment_pbuf_count(r_) \
        (r_)->fragment_state->pbuf_count
    RTI_INT32 pbuf_count;
};

/*ci
 * \brief Local reader record
 *
 * \details
 * State and configuration for an RTPS reader.
 */
struct RTPS_Reader
{
    /*ci \brief base-class
     */
    struct RTPS_Endpoint _parent;

    /*ci \brief Optional reliable state for a reliable reader
     */
    struct RTPS_ReaderReliableState *reliable_state;

    /*ci \brief Optional fragmentation state for a fragmented reader
     */
    struct RTPS_ReaderFragmentState *fragment_state;
};

/*ci \brief A peer entry as a remote reader
 */
#define RTPS_PeerEntry_as_remote_reader(pe_) \
(pe_)->info.reader

/*ci \brief A peer entry as a remote writer
 */
#define RTPS_PeerEntry_as_remote_writer(pe_) \
(pe_)->info.writer

/*ci
 * \brief RTPS peer record
 *
 * \details
 * An RTPS peer is a matched remote writer or reader.  Each local RTPS writer
 * or reader keeps state of each individual peer by using this type.
 */
struct RTPS_PeerEntry
{
    /*ci \brief Peer GUID address
     */
    struct NETIO_Guid addr;

    /*ci \brief State of peer, either writer or reader
     */
    union
    {
        struct RTPS_RemoteReader *reader;
        struct RTPS_RemoteWriter *writer;
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
};

/*ci \brief The default configuration
 */
#define RTPS_INTERFACE_MODE_DEFAULT             0u

/*ci \brief Mask that determines the interface kind
 * - 0 Invalid
 * - 1 External
 * - 2 Reader
 * - 3 Writer
 */
#define RTPS_INTERFACE_MODE_MASK_KIND           0x3u

/*ci \brief mode bit set if the interface is anonymous
 */
#define RTPS_INTERFACE_MODE_MASK_ANONYMOUS      0x4u

/*ci \brief mode bit set if the interface is reliable
 */
#define RTPS_INTERFACE_MODE_MASK_RELIABLE       0x8u

/*ci \brief mode bit set if the interface is fragmented
 */
#define RTPS_INTERFACE_MODE_MASK_FRAGMENTED     0x10u

/*ci \brief mode bit set if the interface is builtin
 */
#define RTPS_INTERFACE_MODE_MASK_BUILTIN        0x20u

/*ci \brief mode bit set if the interface shall check the CRC on incoming
 *   messages
 */
#define RTPS_INTERFACE_MODE_MASK_CHECK_CRC      0x40u

/*ci \brief mode bit set if the interface requires CRC
 */
#define RTPS_INTERFACE_MODE_MASK_REQUIRE_CRC    0x80u

/*ci \brief mode bit set if the interface has trust enabled
 */
#define RTPS_INTERFACE_MODE_MASK_TRUST_ENABLED  0x100u

/*ci \brief mode bit set if the interface has AAD enabled
 */
#define RTPS_INTERFACE_MODE_MASK_AAD_ENABLED    0x200u

/* Flags that overlap between reader and writer
 * because they are never valid at the same time
 */

/*ci \brief mode bit set if reader ACKNACK timer is enabled
 */
#define RTPS_INTERFACE_MODE_MASK_ACKNACK_EVENT_ENABLED  0x400u

/*ci \brief mode bit set if writer HB timer is enabled
 */
#define RTPS_INTERFACE_MODE_MASK_HB_EVENT_ENABLED       0x400u

/* The constants below define the bit posistion for corresponding mode bits
 */
#define RTPS_INTERFACE_MODE_KIND_UNDEFINED    0
#define RTPS_INTERFACE_MODE_KIND_EXTERNAL     1
#define RTPS_INTERFACE_MODE_KIND_READER       2
#define RTPS_INTERFACE_MODE_KIND_WRITER       3

#define RTPS_INTERFACE_MODE_ANONYMOUS_FALSE   0
#define RTPS_INTERFACE_MODE_ANONYMOUS_TRUE    0x4u

#define RTPS_INTERFACE_MODE_RELIABLE_FALSE     0
#define RTPS_INTERFACE_MODE_RELIABLE_TRUE      0x8u

#define RTPS_INTERFACE_MODE_FRAGMENTED_FALSE    0
#define RTPS_INTERFACE_MODE_FRAGMENTED_TRUE     0x10u

#define RTPS_INTERFACE_MODE_BUILTIN_FALSE    0
#define RTPS_INTERFACE_MODE_BUILTIN_TRUE     0x20u

#define RTPS_INTERFACE_MODE_CHECK_CRC_FALSE    0
#define RTPS_INTERFACE_MODE_CHECK_CRC_TRUE     0x40u

#define RTPS_INTERFACE_MODE_REQUIRE_CRC_FALSE    0
#define RTPS_INTERFACE_MODE_REQUIRE_CRC_TRUE     0x80u

#define RTPS_INTERFACE_MODE_TRUST_ENABLED_FALSE    0
#define RTPS_INTERFACE_MODE_TRUST_ENABLED_TRUE     0x100u

#define RTPS_INTERFACE_MODE_AAD_ENABLED_FALSE    0
#define RTPS_INTERFACE_MODE_AAD_ENABLED_TRUE     0x200u

/*ci \brief Check if fragmentation is enabled
 */
#define RTPS_Interface_is_async_pub_enabled(intf_) \
    RTPS_Interface_is_fragmented((intf_))

/*ci \brief Set the interface as external
 */
#define RTPS_Interface_set_external(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_KIND) \
    | RTPS_INTERFACE_MODE_KIND_EXTERNAL)

/*ci \brief Set the interface as RTPS Reader
 */
#define RTPS_Interface_set_reader(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_KIND) \
    | RTPS_INTERFACE_MODE_KIND_READER)

/*ci \brief Set the interface as RTPS Writer
 */
#define RTPS_Interface_set_writer(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_KIND) \
    | RTPS_INTERFACE_MODE_KIND_WRITER)

/*ci \brief Set the interface as an anonymous interface
 */
#define RTPS_Interface_set_anonymous(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_ANONYMOUS) \
    | RTPS_INTERFACE_MODE_ANONYMOUS_TRUE)

/*ci \brief Set the interface as a reliable interface
 */
#define RTPS_Interface_set_reliable(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_RELIABLE) \
    | RTPS_INTERFACE_MODE_RELIABLE_TRUE)

/*ci \brief Set the interface as an interface that supports fragmentation
 */
#define RTPS_Interface_set_fragmented(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_FRAGMENTED) \
    | RTPS_INTERFACE_MODE_FRAGMENTED_TRUE)

/*ci \brief Clear the interface as an interface that supports fragmentation
 */
#define RTPS_Interface_clear_fragmented(intf_) \
(intf_)->mode = ((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_FRAGMENTED)

/*ci \brief Set the interface as a builin interface
 */
#define RTPS_Interface_set_builtin(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_BUILTIN) \
    | RTPS_INTERFACE_MODE_BUILTIN_TRUE)

/*ci \brief Configure the interface to check CRC
 */
#define RTPS_Interface_set_check_crc(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_CHECK_CRC) \
    | RTPS_INTERFACE_MODE_CHECK_CRC_TRUE)

/*ci \brief Configure the interface to require CRC
 */
#define RTPS_Interface_set_require_crc(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_REQUIRE_CRC) \
    | RTPS_INTERFACE_MODE_REQUIRE_CRC_TRUE)

/*ci \brief Configure the interface to enable trust
 */
#define RTPS_Interface_set_trust_enabled(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_TRUST_ENABLED) \
    | RTPS_INTERFACE_MODE_TRUST_ENABLED_TRUE)

/*ci \brief Enable AAD trust on the interface
 */
#define RTPS_Interface_set_aad_enabled(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_AAD_ENABLED) \
    | RTPS_INTERFACE_MODE_AAD_ENABLED_TRUE)

/*ci \brief Enable the ACKNACK timer on the interface
 */
#define RTPS_Interface_set_acknack_event_enabled(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_ACKNACK_EVENT_ENABLED) \
    | RTPS_INTERFACE_MODE_MASK_ACKNACK_EVENT_ENABLED)

/*ci \brief Disable the ACKNACK timer on the interface
 */
#define RTPS_Interface_clear_acknack_event_enabled(intf_) \
((intf_)->mode = (intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_ACKNACK_EVENT_ENABLED)

/*ci \brief Check if the ACKNACK timer is enabled the interface
 */
#define RTPS_Interface_is_acknack_event_enabled(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_ACKNACK_EVENT_ENABLED)

/*ci \brief Enable the HB timer on the interface
 */
#define RTPS_Interface_set_hb_event_enabled(intf_) \
(intf_)->mode = (((intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_HB_EVENT_ENABLED) \
    | RTPS_INTERFACE_MODE_MASK_HB_EVENT_ENABLED)

/*ci \brief Disable the HB timer on the interface
 */
#define RTPS_Interface_clear_hb_event_enabled(intf_) \
((intf_)->mode = (intf_)->mode & ~RTPS_INTERFACE_MODE_MASK_HB_EVENT_ENABLED)

/*ci \brief Check if the HB timer is enabled the interface
 */
#define RTPS_Interface_is_hb_event_enabled(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_HB_EVENT_ENABLED)

/*ci \brief Check if the interface is an external
 */
#define RTPS_Interface_is_external(intf_) \
(((intf_)->mode & RTPS_INTERFACE_MODE_MASK_KIND) \
    == RTPS_INTERFACE_MODE_KIND_EXTERNAL)

/*ci \brief Check if the interface is a RTPS reader
 */
#define RTPS_Interface_is_reader(intf_) \
(((intf_)->mode & RTPS_INTERFACE_MODE_MASK_KIND) \
    == RTPS_INTERFACE_MODE_KIND_READER)

/*ci \brief Check if the interface is a RTPS writer
 */
#define RTPS_Interface_is_writer(intf_) \
(((intf_)->mode & RTPS_INTERFACE_MODE_MASK_KIND) \
    == RTPS_INTERFACE_MODE_KIND_WRITER)

/*ci \brief Check if the interface is a RTPS Endpoint (reader or writer)
 */
#define RTPS_Interface_is_endpoint(intf_) \
    (RTPS_Interface_is_reader(intf_) || RTPS_Interface_is_writer(intf_))

/*ci \brief Check if the interface is an anonymous interface
 */
#define RTPS_Interface_is_anonymous(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_ANONYMOUS)

/*ci \brief Check if the interface is reliable
 */
#define RTPS_Interface_is_reliable(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_RELIABLE)

/*ci \brief Check if the interface supports fragmentation
 */
#define RTPS_Interface_is_fragmented(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_FRAGMENTED)

/*ci \brief Check if the interface is a builtin endpoint
 */
#define RTPS_Interface_is_builtin(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_BUILTIN)

/*ci \brief Check if checking CRC is enabled in the interface
 */
#define RTPS_Interface_is_check_crc(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_CHECK_CRC)

/*ci \brief Check if the interface requires CRC
 */
#define RTPS_Interface_is_require_crc(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_REQUIRE_CRC)

/*ci \brief Check if trust is enabled on the interface
 */
#define RTPS_Interface_is_trust_enabled(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_TRUST_ENABLED)

/*ci \brief Check if AAD is enabled on the interface
 */
#define RTPS_Interface_is_aad_enabled(intf_) \
((intf_)->mode & RTPS_INTERFACE_MODE_MASK_AAD_ENABLED)

/*ci \brief The interface as an endpoint
 */
#define  RTPS_Interface_as_endpoint(intf_) \
((struct RTPS_Endpoint*)(intf_)->endpoint_data.endpoint)

/*ci \brief The interface as a writer
 */
#define  RTPS_Interface_as_writer(intf_) \
((intf_)->endpoint_data.writer)

/*ci \brief The interface as a reader
 */
#define  RTPS_Interface_as_reader(intf_) \
((intf_)->endpoint_data.reader)

/*ci \brief The interface as an external interface
 */
#define  RTPS_Interface_as_external(intf_) \
((intf_)->endpoint_data.external)

/*ci \brief The endpoint's routing table peer index
 */
#define RTPS_Interface_rtable_peer_index(intf_) \
RTPS_Interface_as_endpoint(intf_)->rtable_peer_index

/*ci \brief The endpoint's peer indexer
 */
#define RTPS_Interface_peers_index(intf_) \
RTPS_Interface_as_endpoint(intf_)->peers_index

/*ci \brief The endpoint's peer pool
 */
#define RTPS_Interface_peers_pool(intf_) \
RTPS_Interface_as_endpoint(intf_)->peers_pool

/*ci \brief The endpoint's direct routing index
 */
#define RTPS_Interface_direct_route(intf_) \
RTPS_Interface_as_endpoint(intf_)->direct_route

/*ci \brief The endpoint's group routing index
 */
#define RTPS_Interface_group_route(intf_) \
RTPS_Interface_as_endpoint(intf_)->group_route

/*ci \brief The endpoints selected group index
 */
#define RTPS_Interface_selected_group_route_index(intf_) \
RTPS_Interface_as_endpoint(intf_)->selected_group_route_index

/*ci \brief State maintained by the external interface
 */
struct RTPS_External
{
    /*ci \brief Context for processing received message
     */
    struct RTPS_ReceiveContext context;
};

/*ci \brief State required when PSK is enabled
 */
struct RTPS_TransformState
{
    /*ci \brief The Trust transform buffer size
     */
    #define RTPS_InterfaceTransform_transform_buffer_size(intf_) \
        (intf_)->transform_state->transform_buffer_size

    RTI_SIZE_T transform_buffer_size;

    /*ci \brief The Trust service plugin
     */
    #define RTPS_InterfaceTransform_service_plugin(intf_) \
        (intf_)->transform_state->service_plugin
     RTPS_TrustPlugin *service_plugin;

    /*ci
    * \brief A buffer pool for the buffers to be used for transform operations
    * for outgoing and incoming messages
    */
    #define RTPS_InterfaceTransform_transform_buf_pool(intf_) \
        (intf_)->transform_state->transform_buf_pool
    REDA_BufferPool_T transform_buf_pool;
};

/*ci
 * \brief Representation of an RTPS endpoint or external interface
 *
 * \details
 * Implements the NETIO interface, and is an RT component.
 */
struct RTPS_Interface
{
    /*ci \brief Base NETIO interface
     */
    struct NETIO_Interface _parent;

    /*ci \brief Endpoint configuration
     */
    RTI_UINT32 mode;

    /*ci \brief The database to use to create tables
     */
    DB_Database_T db;

    /*ci \brief The timer to use to create RTPS timers
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief Reference to upstream NETIO interface (e.g. DDS DataWriter)
     */
    NETIO_Interface_T *upstr_intf;

    /*ci \brief The allowed CRC for incoming RTPS messages
     */
    RTI_UINT16 allowed_crc_mask;

    /*ci \brief Factory of this interface
     */
    struct RTPS_InterfaceFactory *factory;

    /*ci \brief Packet for sending message created by this interface
     */
    NETIO_Packet_T *packet;

    /*ci \brief Buffer for this interface's packet
     */
    char *packet_buf;

    /*ci \brief Head pbuf for an RTPS packet
     */
    struct NETIO_PacketBuffer head_pbuf;

    /*ci \brief Tail pbuf for an RTPS packet
     */
    struct NETIO_PacketBuffer tail_pbuf;

    /*ci \brief Sequence of destinations to use for sending local packet
     */
    struct NETIO_AddressSeq local_dest_seq;

    /*ci \brief The external interface for the endpoint
     */
    struct RTPS_Interface *ext_intf;

    /*ci \brief If >= 0, index into checksum table for checksum function to use
     */
    RTI_INT32 checksum_index;

    /*ci \brief Lock to protect sending and receiving
     *          network traffic independently of the database.
     */
    struct OSAPI_Mutex *network_lock;

    /*ci \brief Transform resources is PSK is enabled
     */
    struct RTPS_TransformState *transform_state;

#if OSAPI_ENABLE_TRACE
    const char *session_name;
#endif

    /*ci \brief The different operating modes
     */
    union
    {
        /*ci \brief Local RTPS writer state */
        struct RTPS_Writer *writer;

        /*ci \brief Local RTPS reader state */
        struct RTPS_Reader *reader;

        /*ci \brief Local RTPS endpoint state */
        struct RTPS_Endpoint *endpoint;

        /*ci \brief external state */
        struct RTPS_External *external;
    } endpoint_data;
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
typedef RTI_UINT32 RTPS_RoutePriority_T;

/*ci \brief NETIO Address used by the RTPS Interface routing table.
 */
struct RTPS_RouteEntryAddress
{
    /*ci \brief Address kind */
    RTI_INT32 kind;

    /*ci \brief Address port */
    RTI_UINT32 port;

    /*ci \brief Address */
    union NETIO_AddressValue value;
};

/*ci \brief RTPS_RouteEntryAddress initializer
 */
#define RTPS_RouteEntryAddress_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_RESERVED, /* kind */ \
    0, /* port */ \
    {{0,0,0,0}}, /* value */ \
}

/* Higher Value = Higher Priority */
#define RTPS_ROUTE_PRIORITY_UNDEFINED   0
#define RTPS_ROUTE_PRIORITY_MIN         1u
#define RTPS_ROUTE_PRIORITY_UNICAST     1u
#define RTPS_ROUTE_PRIORITY_MULTICAST   2u
#define RTPS_ROUTE_PRIORITY_MAX         3u

/* Mask for the different route entry options
 */
#define RTPS_ROUTEENTRY_ENCAPSULATION_MASK   (0xffffu)
#define RTPS_ROUTEENTRY_DIRECT_PRIORITY_MASK (0x30000u)
#define RTPS_ROUTEENTRY_GROUP_PRIORITY_MASK  (0xc0000u)
#define RTPS_ROUTEENTRY_SELECTED_GROUP_MASK  (0x100000u)

#define RTPS_RouteEntry_set_encapsulation(r_,e_) \
(r_)->status = ((r_)->status & ~RTPS_ROUTEENTRY_ENCAPSULATION_MASK) | (e_)

#define RTPS_RouteEntry_set_direct_priority(r_,p_) \
(r_)->status = ((r_)->status & ~RTPS_ROUTEENTRY_DIRECT_PRIORITY_MASK) | ((p_)<< 16u)

#define RTPS_RouteEntry_set_group_priority(r_,p_) \
(r_)->status = ((r_)->status & ~RTPS_ROUTEENTRY_GROUP_PRIORITY_MASK) | ((p_)<< 18u)

#define RTPS_RouteEntry_set_selected_group(r_,t_) \
(r_)->status = ((r_)->status & ~RTPS_ROUTEENTRY_SELECTED_GROUP_MASK) | ((t_)<< 20u)

#define RTPS_RouteEntry_get_encapsulation(r_) \
((r_)->status & RTPS_ROUTEENTRY_ENCAPSULATION_MASK)

#define RTPS_RouteEntry_get_direct_priority(r_) \
(((r_)->status & RTPS_ROUTEENTRY_DIRECT_PRIORITY_MASK) >> 16u)

#define RTPS_RouteEntry_get_group_priority(r_) \
(((r_)->status & RTPS_ROUTEENTRY_GROUP_PRIORITY_MASK) >> 18u)

#define RTPS_RouteEntry_get_selected_group(r_) \
(((r_)->status & RTPS_ROUTEENTRY_SELECTED_GROUP_MASK) >> 20u)

#define RTPS_RouteEntry_is_selected_group(r_) \
((r_)->status & RTPS_ROUTEENTRY_SELECTED_GROUP_MASK)

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
     * key
     */
     struct NETIO_Guid destination;

     /*ci
      * \brief The downstream interface which can reach the destination GUID
      * key
      */
     NETIO_Interface_T *intf;

     /*ci
      * \brief The address the downstream interface should use the reach the GUID
      * key
      */
    struct RTPS_RouteEntryAddress intf_address;

    /*ci
     * \brief ptr to peer state entry
     */
    struct RTPS_PeerEntry *peer_ref;

    /*ci \brief Route entry options
     *
     * \details
     *
     * selected_group_route [20]
     * group_priority       [18-19]
     * direct_priority      [16-17]
     * encapsulation        [0-15]
     */
    RTI_UINT32 status;

    /*ci
     * \brief The MTU for this interface
     */
    RTI_SIZE_T mtu;

    /*ci
     * \brief The number of time this route has been asserted. Must be
     *         deleted an equal number of times.
     */
    RTI_UINT32 ref_count;

    /*ci \brief Count for next piggyback HEARTBEAT
     *
     * \details
     * Initialized to zero, counts up, and piggyback HEARTBEAT
     * sent when reaches samples_per_hb.
     */
    RTI_INT32 piggyback_sample_count;

    /*ci \brief Count for next piggyback HEARTBEAT_FRAG
     *
     * \details
     * Initialized to zero, counts up, and piggyback HEARTBEAT_FRAG
     * sent when reaches samples_per_hb.
     */
    RTI_INT32 piggyback_fragment_count;
};

#define RTPS_ROUTE_ENTRY_INITIALIZER \
{                                     \
    NETIO_GUID_INITIALIZER,        /* destination */ \
    NULL,                          /* intf */ \
    RTPS_RouteEntryAddress_INITIALIZER,     /* intf_address */ \
    NULL,                          /* peer_ref */ \
    0,\
    0,          /* mtu */ \
    0,         /* ref_count */ \
    0,         /* piggyback_sample_count */ \
    0         /* piggyback_fragment_count */ \
}

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

    /*ci
     * \brief Reference count for this bind entry. Entry is deleted only when
     *        the count reaches 0.
     */
    RTI_UINT32 ref_count;
};

#define RTPS_EXT_BIND_ENTRY_INITIALIZER \
{\
    NETIO_GUID_INITIALIZER, /* source */ \
    NETIO_GUID_ENTITY_INITIALIZER, /* destination */ \
    NULL, /* _rtps_intf */ \
    NULL, /* peer_ref */ \
    0 /* ref_count */ \
}
/******************************************************************************/
/*ci \brief Max size of header for locally created packet */
#define RTPS_DOWNSTREAM_HEADER_MAX_SIZE (128)

/*ci \brief Max number of submessages in locally created packet.
 *  Currently 4: INFO_TS, INFO_DST, GAP, HEARTBEAT
 */
#define RTPS_LOCAL_PACKET_SUBMSG_MAX_COUNT  (8)

/*ci \brief Max size of locally created packet */
#define RTPS_LOCAL_PACKET_SIZE \
(RTPS_DOWNSTREAM_HEADER_MAX_SIZE + sizeof(struct RTPS_Header) + \
 (RTPS_LOCAL_PACKET_SUBMSG_MAX_COUNT * sizeof(union RTPS_MESSAGES)))

/* ci \brief Max size of transformed packet
 * max size of a downstream interface.Since this buffer is maintained
 * per participant we cannot calculate a conservative size for the this
 * buffer based on the number of peers (varies per writer/reader).
 *  Thus, we go with the large size
 */
#define RTPS_TRANSFORM_PACKET_SIZE (65535)

/******************************************************************************/
/*ci \brief Entity ID is anonymous Writer */
#define RTPSInterface_addr_is_SDP_Participant_sender(addr_) \
    (*OSAPI_Compiler_reinterpret_cast(RTI_UINT32*,&((addr_)->entity)) == \
        NETIO_ntohl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT))

/*ci \brief Entity ID is Simple Discovery Protocol Participant Reader */
#define RTPSInterface_addr_is_SDP_Participant_receiver(addr_) \
    (*OSAPI_Compiler_reinterpret_cast(RTI_UINT32*,&((addr_)->entity)) == \
        NETIO_ntohl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT))

/******************************************************************************/
/*ci \brief Flags when sending packet */
typedef RTI_UINT16 RTPS_SendFlags_T;

/*ci \brief Send DATA submesage */
#define RTPS_SEND_DATA_FLAG         0x01U

/*ci \brief Send GAP submesage */
#define RTPS_SEND_GAP_FLAG          0x02U

/*ci \brief Send HEARTBEAT submesage */
#define RTPS_SEND_HB_FLAG           0x04U

/*ci \brief Send liveliness HEARTBEAT submesage */
#define RTPS_SEND_LIVE_HB_FLAG      0x08U

/*ci \brief Enable pull-mode for historical samples
 *
 * \details
 * Enabling pull mode does not send samples over transport, instead relies on
 * Reader peer to NACK for resend.
 */
#define RTPS_SEND_PULL_MODE_FLAG       0x10U

#define RTPS_SEND_ACKNACKFLAG_FLAG     0x20U

#define RTPS_SEND_RESEND_DATA_FLAG     0x40U

/*ci \brief Send DATA_FRAG submesage */
#define RTPS_SEND_DATA_FRAG_FLAG         0x80U

#define RTPS_SEND_NACK_FRAG_FLAG        0x100U

#define RTPS_SEND_LAST_DATA_FRAG_FLAG   0x200U

#define RTPS_SEND_DATA_NON_FRAG_FLAG    0x400U

#define RTPS_SEND_HB_FRAG_FLAG          0x800U

/*ci
 *\brief Enum to represent different modes the RTPS Sender
 *       could be called in.
 */
typedef enum RTPS_SendMode
{
    /*ci \brief Only queue and don't send */
    RTPS_SENDMODE_QUEUE_PACKET,

    /*ci \brief Add to the tx list after calculating the routes and peers */
    RTPS_SENDMODE_SCHEDULE_PACKET,

    /*ci \brief Wake-up the task thread to process the tx list */
    RTPS_SENDMODE_RESCHEDULE_PACKET,

    /*ci \brief Send to the routes and peers*/
    RTPS_SENDMODE_SEND_PACKET
} RTPS_SendMode;


/*ci
 *\brief Enum to represent different modes the RTPS periodic
 *       HB can be called.
 */
typedef enum RTPS_PeriodicHBMode
{
    /*ci \brief Called from the timer expiring */
    RTPS_PERIODIC_HB_TIMER,
    /*ci \brief Called from the Scheduler */
    RTPS_PERIODIC_HB_SCHEDULED
} RTPS_PeriodicHBMode;

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

/*ci \brief Maximum length for RTPS table name strings */
#define RTPS_TABLE_NAME_MAX_LEN    16

#define RTPS_Interface_checksum_calculate(intf_,c_index_,buf_,buf_s_,chksum_)\
         intf_->factory->checksum[c_index_].checksum_calculate(\
                intf_->factory->checksum[c_index_].context,\
                buf_,buf_s_,chksum_)

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Interface_create_rtps_msg(struct RTPS_Interface *intf, /* RTPS self */
                               struct NETIO_Address *dest, /* RTPS peer */
                               struct RTPS_RouteEntry *route_entry,
                               struct RTPS_PeerEntry *peer_entry,
                               NETIO_Packet_T *packet,
                               RTI_UINT32 send_flags,
                               RTPS_Entity_T *in_reader_entity,
                               RTPS_SampleId_T *gap_sn_start,
                               RTPS_SampleId_T *gap_sn_end,
                               union RTPS_MESSAGES *acknack,
                               RTI_INT32 acknack_length,
                               union RTPS_MESSAGES *data_frag);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Interface_initialize_packet(struct RTPS_Interface *intf);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Sender_send_pdu(struct RTPS_Interface *self,
                     struct RTPS_RouteEntry *route_entry,
                     NETIO_Packet_T *packet);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Receiver_process_data(struct RTPS_Interface *intf,
                           struct RTPS_PeerEntry *peer_entry,
                           NETIO_Packet_T *packet,
                           RTI_UINT8 flags,
                           RTI_UINT32 data_len,
                           RTI_BOOL byte_swap);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Sender_route_packet(struct RTPS_Interface *intf,
                         NETIO_Packet_T *packet,
                         struct RTPS_PeerEntry *peer_entry,
                         RTPS_SendFlags_T send_flags,
                         RTPS_Entity_T *reader_entity,
                         RTPS_SampleId_T *gap_sn_start,
                         RTPS_SampleId_T *gap_sn_end,
                         union RTPS_MESSAGES *acknak,
                         RTI_INT32 acknack_length,
                         struct RTPS_Bitmap *fragments,
                         RTPS_SendMode send_mode);

extern void
RTPS_Interface_set_submessage_header(struct RTPS_SubmsgHdr *header,
                                     RTI_UINT8 kind,
                                     RTI_UINT8 flags,
                                     RTI_SIZE_T length);

extern RTI_UINT32
RTPS_Interface_get_bitmap_int_count(RTI_INT32 bit_count);

extern  RTI_SIZE_T
RTPS_Interface_get_nack_frag_size(RTI_INT32 bit_count);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Writer_request_and_resend_packet(struct RTPS_Interface *intf,
                                      struct RTPS_Writer *writer,
                                      struct RTPS_RemoteReader *reader,
                                      struct RTPS_PeerEntry *peer,
                                      NETIO_Packet_T **packet,
                                      struct RTPS_ResendContext *ctxt,
                                      struct RTPS_Bitmap *samples,
                                      struct RTPS_Bitmap *fragments);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Writer_direct_send(struct RTPS_Interface *intf,
                        RTPS_SendFlags_T send_flags,
                        NETIO_Packet_T *packet,
                        struct RTPS_PeerEntry *peer_entry,
                        RTPS_Entity_T *reader_entity,
                        RTPS_SampleId_T *gap_sn_start,
                        RTPS_SampleId_T *gap_sn_end,
                        struct RTPS_Bitmap *bitmap);

MUST_CHECK_RETURN extern RTI_INT32
RTPS_Interface_compare_guid(struct NETIO_Guid *left,struct NETIO_Guid *right);

MUST_CHECK_RETURN extern RTI_INT32
RTPS_Interface_compare_guid_prefix(struct NETIO_GuidPrefix *left,struct NETIO_GuidPrefix *right);

extern RTI_BOOL
RTPS_Receiver_valid_submessage(union RTPS_MESSAGES *msg,
                               RTI_BOOL *supported_submsg,
                               NETIO_Packet_T *packet);

MUST_CHECK_RETURN extern RTI_BOOL
RTPS_Interface_send_liveliness(struct RTPS_Interface *intf);


extern void
RTPS_Interface_restore_packet(
        struct RTPS_Interface *rtps_intf,
        NETIO_Packet_T *packet,
        NETIO_PacketState_T *saved_packet_state);

extern void
RTPS_RouteEntryAddress_to(struct NETIO_Address *to,
                          struct RTPS_RouteEntryAddress *from);

extern RTI_INT32
RTPS_RouteEntryAddress_compare(struct RTPS_RouteEntryAddress *left,
                               struct RTPS_RouteEntryAddress *right);

#endif /* RTPSInterface_pkg_h */
