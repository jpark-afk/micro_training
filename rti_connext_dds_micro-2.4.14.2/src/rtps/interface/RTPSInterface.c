/*
 * FILE: RTPSInterface.c - NETIO interface for RTPS send and receive
 *
 * Copyright 2008-2022 Real-Time Innovations, Inc.
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
 * 08sep2022,tk MICRO-3367/PR.30121
 * - RTPS_Interface_compare_ext_bind
 *  - Place MUST_CHECK_RETURN on line after comment
 * 06sep2022,tk MICRO-4151/PR.30884
 * - RTPS_Interface_set_info_ts
 *   - Corrected function parameter to info_ts
 * 17aug2022,tk MICRO-4115/PR.30818
 * - Changed count to RTI_UINT32 in RTPS_Interface_set_heartbeat
 * 09aug2022,tk MICRO-4081/PR.30742
 * - Fixed issue when the first_write_sequence_number for the DataWriter is
 *   different from 1. If the intiial SN sent by RTPS is different from 1,
 *   a GAP submessage is added before the first DATA submessage to avoid
 *   the RTPS reader sending an unnecessary ACKNACK for the DATA samples with
 *   a SN <= than the first SN.
 * 08aug2022,tk MICRO-3367/PR.30121
 * - Placed { on its own line in:
 *   - RTPS_Writer_on_periodic_heartbeat
 * 27may2022,am MICRO-3537/PR.30409
 * - Removed NETIO_Address_is_udpv4_multicast and replaced with
 *   NETIO_Address_is_multicast 
 * 26may2022,tk MICRO-3589/PR.30517
 * The following non-functional changes were made for clarity:
 * - Renamed RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_HOST to
 *   RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_NETWORK_ORDER
 * - Renamed RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_HOST to
 *   RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_NETWORK_ORDER
 * - Use NETIO_htonl instead of NETIO_ntohl
 * 26may2022,tk MICRO-3590/PR.30529
 * - Removed redundant else branch in RTPS_Receiver_process_data
 *   after if (peer_entry != NULL) since peer_addr is only used if
 *   peer_entry is different from NULL.
 * - Unrelated to this issue, but addressed as part of opening this file:
 *   Initialize rtps_msg_head and rtps_msg_tail to 0 in RTPS_Interface_receive
 *   to silence Xcode static analyzer that an uninitialized variable 'may'
 *   be used. The path leading to using uninitialized variables is not
 *   possible and is also not reported by Coverity.
 * 18may,2022,tk MICRO-3573/PR.30518
 *  - Use RTI_SIZEOF(rtps_msg->header.rtps) instead of sizeof(VALID_RTPS_HEADER)
 *    in RTPS_Interface_receive for clarity and to cast to RTI_SIZE_T
 * 18feb2022,jh MICRO-3455
 * - Added cast to RTI_UINT8 to fix compiler warning in
 *   RTPS_Interface_set_submessage_header
 * 15sep2021,tk MICRO-3273/PR.29656
 * - Fixed RTPS_InterfaceFactory_initialize to fail if not
 *   both function and context is equal when reusing a builtin checksum
 *   implemention and allow_override is TRUE.
 * 13sep2021,tk MICRO-3231/PR.29563
 * - Use the entity ID as the unique ID in table names. The entity ID is
 *   unique for each RTPS interface within a DomainParticipant and each
 *   DomainParticipant has its own database.
 * - Removed factory->instance_counter.
 * 13sep2021,tk MICRO-3195/PR.29515
 * - Removed redundant test for DB_RETCODE_NO_DATA as return code from
 *   DB_Table_select_all and DB_Table_select_range in RTPS_Sender_route_packet
 *   and RTPS_Interface_select_reader_routes.
 * 13sep2021,tk MICRO-3255/PR.29646
 * - Removed redundant switch-case for RTPS_INFO_REPLY_KIND,
 *   RTPS_INFO_REPLY_IP4_KIND,and RTPS_INFO_SRC_KIND in
 *   RTPS_Interface_receive().
 * - Removed redundant test tmp_msg.submsg.kind == DATA_BATCH_KIND
 *   in RTPS_Interface_receive.
 * 13sep2021,tk MICRO-3251/PR.29568
 * - Only include batch processing code for HEARTBEAT_BATCH
 *   and DATA_BATCH when RTPS_DATA_BATCH_ENABLED is TRUE.
 * 13sep2021,tk MICRO-2866/PR.28697
 * - Removed redundant test in RTPS_Interface_receive()
 *   where a test for sufficient packet length to test for
 *   VALID_RTPS_HEADER is tested twice. Removed the test
 *   that is always FALSE.
 * 22jul2021,tk MICRO-1497/PR.16267
 * - If the payload is larger than 64KB, ensure that the DATA submessage is
 *   the last submessage in the RTPS message and that the submessage header
 *   length is 0.
 * 16apr2021,tk MICRO-2845/PR.28682
 * - Removed empty statements (LOG statements ending in ;)
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Made documentation for RTPS_InterfaceFactory_get_interface public.
 * 04apr2021,tk MICRO-2974/PR.28968
 *  - Check return value from Return TRUE/FALSE in
 *    RTPS_Interface_set_header_extension() in RTPS_Writer_direct_send()
 *  - Check return value from RTPS_Interface_set_header_extension()
 *    in RTPS_Reader_send_acknack()
 *  - Check return value from RTPS_Interface_set_header_extension()
 *    in RTPS_Interface_send()
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in RTPS_Receiver_process_gap()
 *  - Removed empty blocks in RTPS_Receiver_process_heartbeat()
 *  - Removed empty blocks in RTPS_Receiver_process_data()
 *  - Removed empty blocks in RTPS_Receiver_process_data_batch()
 *  - Removed empty blocks in RTPS_Interface_post_event()
 *  - Removed empty block in RTPS_Interface_send
 *  - Removed empty block in RTPS_Interface_clear_window
 * 03mar2021,tk MICRO-2866/PR.28697
 * - Added test that message has sufficient bytes to test for VALID_RTPS_HEADER
 *    in RTPS_Interface_receive()
 * 03mar2021,tk MICRO-2866/PR#28697
 * - Ensure that 'RTPS' is validated on a 32-bit aligned address in
 *   RTPS_Interface_receive().
 * - Changed RTPS_Interface_receive() to only check for 'RTPS' at the
 *   beginning of the payload and by jumping to the next message if the
 *   current is invalid and the length is available.
 * 25feb2021,tk MICRO-2768/PR#28489
 *   - Removed HEADER_EXTN_KIND from RTPS_Receiver_valid_submessage since it
 *     is only valid as part of the header.
 * 25feb2021,tk MICRO-2902/PR#28816
 *   - Added check that interface mode is RTPS_INTERFACEMODE_READER in
 *     RTPS_Interface_set_state() before checking reader specific flags.
 * 25feb2021,tk MICRO-2902/PR#28746
 *   - Updated description of RTPS_Sender_route_packet()
 *   - Updated return value for RTPS_Receiver_forward_upstream()
 *   - Updated the description of the property argument in
 *     RTPS_InterfaceFactory_initialize().
 * 20feb2021,tk MICRO-2866/PR#28697
 *   - Ensure that 'RTPS' in validated on a 32-bit aligned address in
 *     RTPS_Interface_receive().
 * 20feb2021,tk MICRO-2885/PR#28784
 *   - Removed redundant code in RTPS_Receiver_process_acknack() when an
 *     ACKNACK with a lead of zero is received.
 * 20feb2021,tk MICRO-2848/PR#28693
 *   - Fixed incorrect function header comments.
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 19oct2020,tk MICRO-2575/PR#28172 Removed support for custom checksums
 * 05jun2017,eh MICRO-1614 Ensure enough buffer for msg header and submsg
 * 14oct2016,tk MICRO-1571 Handle re-discovery of endpoint discovery data
 * 06jun2016,tk MICRO-1543 Reference count shared resources for matched entities (needed
 *                         after changes to related to MICRO-1505
 * 17dec2015,eh MICRO-1511: accept DATA with K-flag
 * 16dec2015,tk MICRO-1503 Removed unnecessary max_message_buffer_count property
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 03aug2015,eh MICRO-1482 Update check of ACKNACK and HEARTBEAT counts to drop
 *              only identical counts
 * 25jul2015,eh MICRO-1461/PR#15579 With first sample sent, advance all
 *              peers' send windows and last_unacked_sn
 * 20jul2015,tk MICRO-1445/PR#15492 The maximum number of records for external
 *                                  interface's bind table is max_routes + max_binds.
 * 18jul2015,eh MICRO-1443 Fix HEARTBEAT response to preemptive ACKNACK
 * 18jul2015,eh MICRO-1443 Fix update of reader's last_acked_sn
 * 09jul2015,eh MICRO-1400/PR#15272 Fix comment header
 * 02jul2015,eh MICRO-1348 Remove magic numbers
 * 30jun2015,eh MICRO-1370/PR#15206 Fix reliable reader to always send
 *              preemptive ACKNACK
 * 29jun2015,eh MICRO-1357/PR#15147 Remove redundant check in process_data
 * 24jun2015,eh MICRO-1326/PR#15041 Fix validity check for zero-length
 *              submessage
 * 24jun2015,eh MICRO-1327/PR#15042 Fix ignoring of unsupported submessage
 * 15jun2015,eh MICRO-1260/PR#14860 Invalid known submessage invalidates rest
 *              of containing message
 * 12jun2015,eh MICRO-1259/PR#14859 Fix RTPS_RELIABILITY guard in process_data
 * 12jun2015,eh MICRO-1271/PR#14876 Fix comment in compare_ext_intf
 * 12jun2015,eh MICRO-1277/PR#14902 Check valid Heartbeat firstSN, lastSN
 * 12jun2015,eh MICRO-1278/PR#14905 Fix comment of int_to_entityid
 * 12jun2015,eh MICRO-1282/PR#14916 Check for NULL upstream interface
 * 12jun2015,eh MICRO-1289/PR#14947 Invalidate timestamp
 * 12jun2015,eh MICRO-1234/PR#14803 INFO_DST length check
 * 12jun2015,eh MICRO-1232/PR#14801 DATA submessage validity checks
 * 10jun2015,eh MICRO-1274/PR#14890 Fixed RTPS_Window_insert comment
 * 10jun2015,eh MICRO-1275/PR#14896 Remove redundant SN compare for last_sn
 * 08jun2015,eh MICRO-1297/PR#14965 Consistent return false on
 *              NETIO_Packet_set_head failure
 * 14may2015,tk Use intf->_parent.local_address instead of
 *              intf->property.intf_address
 * 14may2015,eh MICRO-1146/PR#14594 Fix RTPS_InterfaceFactory_fv_Intf and
 *              RTPS_InterfaceFactory_fv_Factory to be singleton
 * 12may2015,eh MICRO-1170/PR#14655 Remove checks for DB_RETCODE_EXISTS
 * 11may2015,eh MICRO-1197/PR#14754 Truncate ACKNACK bitmap after shift
 * 02apr2015,eh MICRO-964/PR#12378 Remove from Cert unused locator defines
 * 12mar2015,eh MICRO-1111/PR#14195 Remove redundant assignment of existed
 * 11mar2015,eh MICRO-1095/PR#14121 Fix loop of all_peers_list of routes
 * 05mar2015,eh MICRO-1101/PR#14161 Fix redundancies in process_gap()
 * 05mar2015,eh MICRO-1103/PR#14165 Fix return value in process_gap()
 * 04mar2015,eh MICRO-1091/PR#14101 Remove invalid comment
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh MICRO-1075: remove and replace macros
 * 26jan2015,tk MICRO-1028/PR#13473 Removed magic number 0xc0
 * 20jan2015,eh MICRO-944/PR#11897 Remove Locator.c
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 10oct2014,eh MICRO-929: valid INFO_DST check
 * 10oct2014,eh MICRO-924: valid ACKNACK check
 * 10oct2014,eh MICRO-930: valid INFO_TS check
 * 10oct2014,eh MICRO-926: valid DATA check
 * 10oct2014,eh MICRO-932: check submsg length vs packet's remaining length
 * 10oct2014,eh MICRO-928: valid GAP check
 * 10oct2014,eh MICRO-927: valid HEARTBEAT check
 * 18aug2014,eh MICRO-867: max_window not greater than 256
 * 14aug2014,eh MICRO-884: Add peers index and pool, remove peer ref_count
 * 10aug2014,eh MICRO-868: Fix xmit_remove()
 * 31jul2014,tk MICRO-172/PR#1064 - Removed superfluous REDA_Indexer fields
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 20may2014,eh MICRO-85: Change from CDR to RTPS deserialize SN
 * 15may2014,eh MICRO-790: Add reserved count for reader + remote writers
 * 05may2014,eh MICRO-782: route_packet() skip_send
 * 19apr2014,eh MICRO-287: DB retcode in log msgs
 * 27mar2014,eh MICRO-747: prevent HB-ACK ping-pong upon sample rejection and
 *              non-progressing ACKNACK
 * 20feb2015 ,eh MICRO-714: request() in assert_peer()
 * 03feb2014,eh Fixed MICRO-714: use acknack() to nack for late-joining reader
 * 07jan2013,eh Fixed MICRO-714: support per-peer ack state, in/active readers,
 *              volatile durability writer
 * 13jun2013,tk Fixed MICRO-275: Clear dst_addr in get_external_interface
 * 03jun2013,eh Fixed MICRO-657: set source_timestamp on INFO_TS reception
 * 20may2013,eh Fixed MICRO-379: packet set_head/tail
 * 30may2013,eh Fixed MICRO-640: dispose
 * 17may2013,eh Fixed MICRO-385: remove unused fns/fields of packet
 * 17may2013,eh Merge fix for MICRO-377 (CR-135) from 2.2.x_BOE49
 * 13mar2013,eh Fixed MICRO-351 (liveliness HB)
 * 13mar2013,eh Fixed MICRO-301 (log msg)
 * 14feb2013,eh Fixed MICRO-216 (liveliness HB)
 * 04feb2013,eh Fixed MICRO-232, MICRO-262
 * 27apr2012,tk Written
 */

/*ce @ingroup RTPSModule
 * \file
 * \brief An implementation of the NETIO interface for the RTPS protocol
 *
 * \details
 * - Creation and deletion of RTPS interfaces
 * - Binding and routing of packets between RTPS and upstream and downstream
 *   NETIO interfaces
 * - Sending and receiving of RTPS messages and submessages by RTPS endpoints
 */
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
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
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
#include "RTPSHdrExt.h"

/*ci \brief Singleton RTPS implementation of NETIO interface */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI RTPSInterface_fv_Intf; /* fwd decl */


/*ci \brief Minimum length of unknown RTPS submessage */
#define RTPS_SUBMSG_MIN_LEN_DEFAULT     0xff

/*ci \brief Minimum length of RTPS PAD submessage */
#define RTPS_SUBMSG_MIN_LEN_PAD         0

/*ci \brief Minimum length of RTPS ACKNACK submessage */
#define RTPS_SUBMSG_MIN_LEN_ACKNACK     24

/*ci \brief Minimum length of RTPS HEARTBEAT submessage */
#define RTPS_SUBMSG_MIN_LEN_HB          28

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
#define RTPS_SUBMSG_MIN_LEN_DATA_BATCH  40

/*ci \brief Largest value of supported submessage kind */
#define RTPS_SUBMSG_KIND_HIGH           26

/*ci \brief Lookup table of minimum lengths of each submessage
 *
 * \details Array index is equal to Submessage ID.  For example, the RTPS
 * submessage ID for an ACKNACK submessage is 0x06, so the value of the element
 * with index 6 is the minimum length (specifically, the minimum value of
 * the octetsToNextHeader field) of an ACKNACK.
 * Unsupported or invalid indices are set with maximum length (0xff).
 */
RTI_PRIVATE const RTI_UINT16 RTPS_fv_SubMsgMinLen[RTPS_SUBMSG_KIND_HIGH] =
{
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_PAD, /* PAD (0x01) */
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_ACKNACK, /* ACKNACK (0x06) */
    RTPS_SUBMSG_MIN_LEN_HB, /* HEARTBEAT (0x07)*/
    RTPS_SUBMSG_MIN_LEN_GAP, /* GAP (0x8) */
    RTPS_SUBMSG_MIN_LEN_INFO_TS, /* INFO_TS (0x9) */
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_INFO_SRC, /* INFO_SRC (0x0c) */
    RTPS_SUBMSG_MIN_LEN_DEFAULT, /* INFO_REPLY_IP4(0x0d) */
    RTPS_SUBMSG_MIN_LEN_INFO_DST, /* INFO_DST (0x0e) */
    RTPS_SUBMSG_MIN_LEN_INFO_REPLY, /* INFO_REPLY (0x0f) */
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DATA, /* DATA (0x15) */
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DEFAULT,
    RTPS_SUBMSG_MIN_LEN_DATA_BATCH, /* DATA_BATCH (0x18) */
    RTPS_SUBMSG_MIN_LEN_HB_BATCH /* HEARTBEAT_BATCH (0x19) */
};

/*ci \brief The maximum DATA submessage length
 */
#define RTPS_MAX_DATA_LENGTH USHRT_MAX

/*ci \brief Length in bytes of RTPS submessage header */
#define RTPS_SUBMSG_HEADER_LEN  4

/*ci \brief Maximum length in bytes of RTPS ACKNACK submessage */
#define RTPS_ACKNACK_SUBMSG_MAX_LEN 56

/*ci \brief Maximum length in bytes of RTPS GAP submessage */
#define RTPS_GAP_SUBMSG_MAX_LEN 60

/*ci \brief Maximum length in bytes of RTPS GAP submessage */
#define RTPS_INFO_TS_SUBMSG_MAX_LEN 8

#ifndef RTI_CERT
/*ci \brief Invalid Locator Kind */
const RTI_INT32 RTPS_LOCATOR_KIND_INVALID = -1;

/*ci \brief Invalid Locator Port */
const RTI_UINT32 RTPS_LOCATOR_PORT_INVALID = 0;

/*ci \brief Invalid Locator Address */
const unsigned char
    RTPS_LOCATOR_ADDRESS_INVALID[RTPS_LOCATOR_ADDRESS_LENGTH_MAX] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif /* !RTI_CERT */

/*ci \brief Invalid Locator */
const struct RTPS_Locator_t RTPS_LOCATOR_INVALID = {
    -1,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

/*ci \brief Reserved Locator Kind */
const RTI_INT32 RTPS_LOCATOR_KIND_RESERVED = 0;

/*ci \brief DUPv4 Locator Kind */
const RTI_INT32 RTPS_LOCATOR_KIND_UDPv4 = 1;

/*ci \brief DUPv6 Locator Kind */
const RTI_INT32 RTPS_LOCATOR_KIND_UDPv6 = 2;

/*ci \brief Shared Memory Locator Kind */
const RTI_INT32 RTPS_LOCATOR_KIND_SHMEM = 3;

/*ci \brief Maximum number of indexes in an RTPS Interface's route table */
#define RTPS_INTERFACE_ROUTE_TABLE_MAX_INDEXES  5

/*ci \brief Index of entity kind suffix of entity ID */
#define RTPS_ENTITY_KIND_INDEX  3

/*ci \brief Conversion factor from milliseconds to nanoseconds */
#define RTPS_MS_TO_NS 1000000

/*** SOURCE_BEGIN ***/

/*ci \brief Constant host order entity id for the participant writer
 */
const RTI_UINT32 RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT_NETWORK_ORDER =
                            NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT);

/*ci \brief Constant host order entity id for the participant reader
 */
const RTI_UINT32 RTPS_OBJECT_ID_READER_SDP_PARTICIPANT_NETWORK_ORDER =
                            NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

/*ci
 *
 * \brief Checks equality of prefixes of two GUIDs
 *
 * \param[in] a First GUID
 * \param[in] b Second GUID
 *
 * \return RTI_TRUE iff both GUIDs have all prefix fields equal, RTI_FALSE
 * otherwise.
 */
RTI_BOOL
RTPS_Guid_prefix_equals(struct RTPS_Guid* a,
                        struct RTPS_Guid* b)
{
    return ((a->prefix.host_id == b->prefix.host_id) &&
            (a->prefix.app_id  == b->prefix.app_id ) &&
            (a->prefix.instance_id  == b->prefix.instance_id ));
}

/*ci
 *
 * \brief Checks equality of suffixes of two GUIDs
 *
 * \param[in] a First GUID
 * \param[in] b Second GUID
 *
 * \return RTI_TRUE iff both GUIDs have suffix fields equal, RTI_FALSE
 * otherwise.
 */
RTI_BOOL
RTPS_Guid_suffix_equals(struct RTPS_Guid* a,
                        struct RTPS_Guid* b)
{
    return (a->object_id == b->object_id);
}

/*ci
 *
 * \brief Checks equality of two GUIDs
 *
 * \param[in] a First GUID
 * \param[in] b Second GUID
 *
 * \return RTI_TRUE iff both GUIDs have all fields equal, RTI_FALSE otherwise.
 */
RTI_BOOL
RTPS_Guid_equals(struct RTPS_Guid* a,
                        struct RTPS_Guid* b)
{
    return (RTPS_Guid_prefix_equals(a, b) && RTPS_Guid_suffix_equals(a, b));
}

/*ci
 * \brief
 * Assign fields of RTPS message header
 *
 * \param[inout] header RTPS message header to set
 * \param[in] prefix Sender's GUID prefix
 */
RTI_PRIVATE void
RTPS_Interface_set_message_header(struct RTPS_Header *header,
                                  RTPS_GuidPrefix_T *prefix)
{
    header->rtps = VALID_RTPS_HEADER;
    header->protocol_version.major = RTPS_PROTOCOL_VERSION_MAJOR;
    header->protocol_version.minor = RTPS_PROTOCOL_VERSION_MINOR;
    header->vendor_id.value[0] = RTPS_VENDOR_ID_MAJOR;
    header->vendor_id.value[1] = RTPS_VENDOR_ID_MINOR;
    header->guid_prefix = *prefix;
}

/*ci
 * \brief
 * Assign fields of RTPS submessage header
 *
 * \param[inout] header  RTPS submessage header to set
 * \param[in] kind Submessage kind
 * \param[in] flags Submessage flags
 * \param[in] length Submessage length
 *
 */
void
RTPS_Interface_set_submessage_header(struct RTPS_SubmsgHdr *header,
                                     RTI_UINT8 kind,
                                     RTI_UINT8 flags,
                                     RTI_SIZE_T length)
{
    header->kind = kind;
    header->length = (RTI_UINT16)(length - RTPS_SUBMSG_HEADER_LEN);

    /* Set submessage's little-endian flag only for little-endian platform */
#ifdef RTI_ENDIAN_LITTLE
    header->flags = flags | RTPS_SUBMSG_FLAG_E;
#else
    header->flags = flags & (RTI_UINT8)~RTPS_SUBMSG_FLAG_E;
#endif
}

/*ci
 * \brief Get the number of 32-bit integers to fit a given bit count in a bitmap.
 *
 * \param[in] bit_count Bit count
 *
 * \return Number of integers to contain the input bit_count in a bitmap.
 */
RTI_PRIVATE RTI_UINT32
RTPS_Interface_get_bitmap_int_count(RTI_INT32 bit_count)
{
    return (RTI_UINT32)((bit_count + 31)/32);
}

/*ci
 * \brief Get length of serialized GAP submessage
 *
 * \param[in] bit_count Bit count of GAP's bitmap
 *
 * \return Number of bytes of serialized GAP submessage
 */
RTI_PRIVATE RTI_SIZE_T
RTPS_Interface_get_gap_size(RTI_INT32 bit_count)
{
    /* Bitmap has variable length, depending on bit count */
    return (RTI_SIZE_T)((sizeof(struct RTPS_GAP)) - (sizeof(struct RTPS_Bitmap)) +
        (sizeof(struct REDA_SequenceNumber)) +
        (sizeof(RTI_INT32)) +
        (sizeof(RTI_INT32) * RTPS_Interface_get_bitmap_int_count(bit_count)));
}

/*ci
 * \brief Get length of serialized ACKNACK submessage
 *
 * \param[in] bit_count Bit count of ACKNACK's bitmap
 *
 * \return Number of bytes of serialized ACKNACK submessage
 */
RTI_PRIVATE RTI_SIZE_T
RTPS_Interface_get_acknack_size(RTI_INT32 bit_count)
{
    /* Bitmap has variable length, depending on bit count */
    return (RTI_SIZE_T)((sizeof(struct RTPS_ACKNACK) - sizeof(struct RTPS_Bitmap)) +
        (sizeof(struct REDA_SequenceNumber)) +
        (sizeof(RTI_INT32))+
        (sizeof(RTI_INT32) * RTPS_Interface_get_bitmap_int_count(bit_count)));
}

/*ci
 * \brief Get length of serialized DATA submessage
 *
 * \param[in] payload_size Size in bytes of DATA payload
 *
 * \return Number of bytes of serialized DATA submessage
 */
RTI_PRIVATE RTI_SIZE_T
RTPS_Interface_get_data_size(RTI_SIZE_T payload_size)
{
    return (RTI_SIZE_T)((sizeof(struct RTPS_DATA) + payload_size));
}

/*ci
 * \brief Assign fields of ACKNACK
 *
 * \param[inout] ack ACKNACK submessage to set
 * \param[in] flags ACKNACK flags
 * \param[in] length Submessage length
 * \param[in] reader Reader entity ID
 * \param[in] writer Writer entity ID
 * \param[in] bitmap Bitmap
 * \param[in] count Epoch count
 */
RTI_PRIVATE void
RTPS_Interface_set_acknack(struct RTPS_ACKNACK *ack,
                           RTI_UINT8 flags,
                           RTI_SIZE_T length,
                           RTPS_Entity_T reader,
                           RTPS_Entity_T writer,
                           struct RTPS_Bitmap *bitmap,
                           RTI_UINT32 count)
{
    RTI_UINT32 i, int_count, *epoch_ref = NULL;

    /* Epoch may start at the bitmap's first element.  Will end up there if
     *  bitmap has bitcount of zero.
     */
    epoch_ref = &(ack->bitmap.bits[0]);
    RTPS_Interface_set_submessage_header(
       &ack->hdr, RTPS_ACKNACK_KIND, flags, length);
    ack->reader = reader;
    ack->writer = writer;
    ack->bitmap.lead = bitmap->lead;
    ack->bitmap.bit_count = bitmap->bit_count;
    int_count = RTPS_Interface_get_bitmap_int_count(bitmap->bit_count);
    for (i = 0; i < int_count; ++i)
    {
        ack->bitmap.bits[i] = bitmap->bits[i];
        epoch_ref = &(ack->bitmap.bits[i+1]);
    }
    *epoch_ref = count;
}

/*ci \brief Assign fields of DATA submessage
 *
 * \param[inout] data DATA submessage to set
 * \param[in] flags Submessage flags
 * \param[in] length Submessage length
 * \param[in] data_flags  DATA flags
 * \param[in] qos_offset Inline Qos offset
 * \param[in] reader Reader entity ID
 * \param[in] writer Writer entity ID
 * \param[in] sn Sequence number
 */
RTI_PRIVATE void
RTPS_Interface_set_data(struct RTPS_DATA *data,
                        RTI_UINT8 flags,
                        RTI_SIZE_T length,
                        RTI_UINT8 data_flags,
                        RTI_UINT16 qos_offset,
                        RTPS_Entity_T reader,
                        RTPS_Entity_T writer,
                        struct REDA_SequenceNumber *sn)
{
    RTPS_Interface_set_submessage_header(
       &data->hdr, RTPS_DATA_KIND, flags, length);
    data->flags = data_flags;
    data->qos_offset = qos_offset;
    data->reader = reader;
    data->writer = writer;
    data->sn = *sn;
}


/*ci \brief Assign fields of INFO_TS
 *
 * \param[inout] info_ts INFO_TS to set
 * \param[in] ts Timestamp
 */
RTI_PRIVATE void
RTPS_Interface_set_info_ts(struct RTPS_INFO_TS *info_ts,
                           struct OSAPI_NtpTime *ts)
{
    RTPS_Interface_set_submessage_header(
       &info_ts->hdr, RTPS_INFO_TS_KIND, 0, sizeof(struct RTPS_INFO_TS));
    info_ts->timestamp.seconds = ts->sec;
    info_ts->timestamp.fractions = ts->frac;
}


/*ci
 * \brief
 * Checks whether submessage kind is an info submessage
 *
 * \param[in] kind Submessage kind
 *
 * \return RTI_TRUE iff kind is an INFO_* submessage, otherwise RTI_FALSE.
 */
RTI_PRIVATE RTI_BOOL
RTPS_Interface_submessage_is_info(RTI_UINT8 kind)
{
    return ((kind == RTPS_INFO_DST_KIND) || (kind == RTPS_INFO_TS_KIND) ||
            (kind == RTPS_INFO_SRC_KIND) || (kind == RTPS_INFO_REPLY_IP4_KIND) ||
            (kind == RTPS_INFO_REPLY_KIND));
}


/*ci
 * \brief
 * Checks whether submessage is kind sent by a writer
 *
 * \param[in] kind Submessage kind
 *
 * \return RTI_TRUE iff kind is a submessage sent by writer, otherwise RTI_FALSE.
 */
RTI_PRIVATE RTI_BOOL
RTPS_Interface_submessage_is_from_writer(RTI_UINT8 kind)
{
    return  ((kind == RTPS_GAP_KIND) || (kind == RTPS_DATA_KIND)
#if RTPS_DATA_BATCH_ENABLED
              || (kind == RTPS_DATA_BATCH_KIND)
              || (kind == RTPS_HEARTBEAT_BATCH_KIND)
#endif
              || (kind == RTPS_HEARTBEAT_KIND));
}

/*ci \brief Assign fields of GAP submessage
 *
 * \param[inout] gap GAP submessage to set
 * \param[in] length Submessage length
 * \param[in] reader Reader entity ID
 * \param[in] writer Writer entity ID
 * \param[in] sn_start GAP's starting sequence number
 * \param[in] bitmap GAP's bitmap
 */
RTI_PRIVATE void
RTPS_Interface_set_gap(struct RTPS_GAP *gap,
                       RTI_SIZE_T length,
                       RTPS_Entity_T reader,
                       RTPS_Entity_T writer,
                       RTPS_SampleId_T *sn_start,
                       struct RTPS_Bitmap *bitmap)
{
    RTI_UINT32 i, int_count;

    RTPS_Interface_set_submessage_header(&gap->hdr, RTPS_GAP_KIND, 0, length);
    gap->reader = reader;
    gap->writer = writer;
    gap->sn_start = *sn_start;
    gap->bitmap.lead = bitmap->lead;
    gap->bitmap.bit_count = bitmap->bit_count;
    int_count = RTPS_Interface_get_bitmap_int_count(bitmap->bit_count);

    /* Copy only valid bits of input bitmap */
    for (i = 0; i < int_count; ++i)
    {
        gap->bitmap.bits[i] = bitmap->bits[i];
    }
}

/*ci \brief Assign fields of HEARTBEAT
 *
 * \param[inout] hb HEARTBEAT to set
 * \param[in] flags Submessage flags
 * \param[in] reader Reader entity ID
 * \param[in] writer Writer entity ID
 * \param[in] sn_first First sequence number
 * \param[in] sn_last Last sequence number
 * \param[in] count Epoch count
 */
RTI_PRIVATE void
RTPS_Interface_set_heartbeat(struct RTPS_HEARTBEAT *hb,
                             RTI_UINT8 flags,
                             RTPS_Entity_T reader,
                             RTPS_Entity_T writer,
                             struct REDA_SequenceNumber *sn_first,
                             struct REDA_SequenceNumber *sn_last,
                             RTI_UINT32 count)
{
    RTPS_Interface_set_submessage_header(
       &hb->hdr, RTPS_HEARTBEAT_KIND, flags, sizeof(struct RTPS_HEARTBEAT));
    hb->reader = reader;
    hb->writer = writer;
    hb->sn_first = *sn_first;
    hb->sn_last = *sn_last;
    hb->count = count;
}

/*ci \brief Assign fields of INFO_DST
 *
 * \param[inout] info INFO_DST to set
 * \param[in] dst_prefix Destination GUID prefix
 */
RTI_PRIVATE void
RTPS_Interface_set_info_dst(struct RTPS_INFO_DST *info,
                            RTPS_GuidPrefix_T *dst_prefix)
{
    RTPS_Interface_set_submessage_header(
       &info->hdr, RTPS_INFO_DST_KIND, 0, (sizeof(struct RTPS_INFO_DST)));
    info->guid_prefix = *dst_prefix;
}

/*ci \brief Returns whether entity ID is unknown
 *
 * \param[in] entity Entity ID to check
 *
 * \return RTI_TRUE if entity is unknown, RTI_FALSE otherwise.
  */
RTI_PRIVATE RTI_BOOL
RTPS_Interface_is_unknown_entity(RTPS_Entity_T *entity)
{
    return ((entity->value[0] == 0) && (entity->value[1] == 0) &&
            (entity->value[2] == 0) && (entity->value[3] == 0));
}

/*ci
 * \brief
 * Initializes local packet of interface
 *
 * \param[in] intf  self
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_initialize_packet(struct RTPS_Interface *intf)
{
    if (!NETIO_Packet_initialize(intf->packet, intf->packet_buf,
                                 RTPS_LOCAL_PACKET_SIZE,
                                 sizeof(union RTPS_MESSAGES),
                                 &intf->local_dest_seq))
    {
        return RTI_FALSE;
    }

    intf->packet->source = intf->_parent.local_address;
    intf->packet->dests = &intf->local_dest_seq;

    return RTI_TRUE;
}

/*ci
 * \brief
 * Finalize resources of interface
 *
 * \details
 * Called by delete.
 * Not implemented for Cert, as no freeing of resources allowed.
 *
 * \param[in] rtps_intf  self
 */
#ifndef RTI_CERT
RTI_PRIVATE void
RTPS_Interface_finalize(struct RTPS_Interface *rtps_intf)
{
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor;
    struct RTPS_RouteEntry *route = NULL;
    struct RTPS_BindEntry *bind = NULL;
    RTI_INT32 count, i;
    struct RTPS_PeerEntry *peer = NULL;
    RTI_BOOL bretval;
    struct OSAPI_TimeoutHandle TIMEOUT_INIT = OSAPI_TimeoutHandle_INITIALIZER;

#if RTPS_RELIABILITY
    if (rtps_intf->property.mode == RTPS_INTERFACEMODE_WRITER)
    {
        if (rtps_intf->endpoint.writer.flags & RTPS_WRITER_FLAG_HB_EVENT_ACTIVE)
        {
            bretval = OSAPI_Timer_delete_timeout(
                                    rtps_intf->property._parent._parent.timer,
                                    &rtps_intf->endpoint.writer.hb_event);
#if OSAPI_ENABLE_LOG
            if (!bretval)
            {
                RTPS_LOG_TIMER_DELETE_TIMEOUT(OSAPI_LOGKIND_WARNING)
            }
#else
            IGNORE_RETVAL(bretval);
#endif
        }
        rtps_intf->endpoint.writer.hb_event = TIMEOUT_INIT;
    }
    else if (rtps_intf->property.mode == RTPS_INTERFACEMODE_READER)
    {
        if (rtps_intf->endpoint.reader.flags & RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE)
        {
            bretval = OSAPI_Timer_delete_timeout(
                                    rtps_intf->property._parent._parent.timer,
                                    &rtps_intf->endpoint.reader.acknack_event);
#if OSAPI_ENABLE_LOG
            if (!bretval)
            {
                RTPS_LOG_TIMER_DELETE_TIMEOUT(OSAPI_LOGKIND_WARNING)
            }
#else
            IGNORE_RETVAL(bretval);
#endif
        }
        rtps_intf->endpoint.reader.acknack_event = TIMEOUT_INIT;
    }
#endif

    bretval = NETIO_AddressSeq_finalize(&rtps_intf->local_dest_seq);
    IGNORE_RETVAL(bretval);

    if (rtps_intf->peers_index != NULL)
    {
        if (rtps_intf->peers_pool != NULL)
        {
            count = REDA_Indexer_get_count(rtps_intf->peers_index);
            for (i = 0; i < count; ++i)
            {
                peer = (struct RTPS_PeerEntry *)
                    REDA_Indexer_get_entry(rtps_intf->peers_index, i);
                REDA_BufferPool_return_buffer(rtps_intf->peers_pool, peer);
            }
#if OSAPI_ENABLE_LOG
            if (!REDA_BufferPool_delete(rtps_intf->peers_pool))
            {
                RTPS_LOG_DELETE_BUFFER_POOL(OSAPI_LOGKIND_ERROR)
            }
#else
            REDA_BufferPool_delete(rtps_intf->peers_pool);
#endif
            rtps_intf->peers_pool = NULL;
        }

#if OSAPI_ENABLE_LOG
        if (!REDA_Indexer_delete(rtps_intf->peers_index))
        {
            RTPS_LOG_DELETE_INDEXER(OSAPI_LOGKIND_ERROR)
        }
#else
        REDA_Indexer_delete(rtps_intf->peers_index);
#endif
        rtps_intf->peers_index = NULL;
    }


    if (rtps_intf->property.mode == RTPS_INTERFACEMODE_EXTERNAL_RECEIVER)
    {
        if (rtps_intf->packet_buf != NULL)
        {
            OSAPI_Heap_free(rtps_intf->packet_buf);
        }
        if (rtps_intf->packet != NULL)
        {
            OSAPI_Heap_free_struct(rtps_intf->packet);
        }
    }

    if (rtps_intf->direct_route != NULL)
    {
        dbrc = DB_Table_delete_index(rtps_intf->_parent._rtable,
                                     rtps_intf->direct_route);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

    if (rtps_intf->group_route != NULL)
    {
        dbrc = DB_Table_delete_index(rtps_intf->_parent._rtable,
                                     rtps_intf->group_route);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

    /* Delete route table and index */
    if (rtps_intf->_parent._rtable != NULL)
    {
        dbrc = DB_Table_delete_index(rtps_intf->_parent._rtable,
                                     rtps_intf->rtable_peer_index);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif

        dbrc = DB_Table_delete_index(rtps_intf->_parent._rtable,
                                     rtps_intf->selected_group_route_index);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif

        dbrc = DB_Table_select_all_default(rtps_intf->_parent._rtable,&cursor);
        if (dbrc == DB_RETCODE_OK)
        {
            do
            {
                dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route);
                if (dbrc == DB_RETCODE_OK)
                {
                    dbrc = DB_Table_delete_record(rtps_intf->_parent._rtable,
                            (DB_Record_T)route);
                }
            } while (dbrc == DB_RETCODE_OK);
            DB_Cursor_finish(rtps_intf->_parent._rtable, cursor);
        }

        dbrc = DB_Database_delete_table(rtps_intf->property._parent._parent.db,
                                        rtps_intf->_parent._rtable);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

    /* Delete bind table */
    if (rtps_intf->_parent._btable != NULL)
    {
        dbrc = DB_Table_select_all_default(rtps_intf->_parent._btable,&cursor);
        if (dbrc == DB_RETCODE_OK)
        {
            do
            {
                dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind);
                if (dbrc == DB_RETCODE_OK)
                {
                    dbrc = DB_Table_delete_record(rtps_intf->_parent._btable,
                            (DB_Record_T)bind);
                }
            } while (dbrc == DB_RETCODE_OK);
            DB_Cursor_finish(rtps_intf->_parent._btable, cursor);
        }

        dbrc = DB_Database_delete_table(rtps_intf->property._parent._parent.db,
                                        rtps_intf->_parent._btable);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            RTPS_LOG_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

#if OSAPI_ENABLE_LOG
    if (!NETIO_Interface_finalize(&rtps_intf->_parent))
    {
        RTPS_LOG_FINALIZE_INTERFACE(OSAPI_LOGKIND_ERROR)
    }
#else
    NETIO_Interface_finalize(&rtps_intf->_parent);
#endif
}
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Delete an interface
 *
 * \details
 * Not implemented for Cert, as no freeing of resources allowed.
 *
 * \param[in] rtps_intf  self
 */
#ifndef RTI_CERT
RTI_PRIVATE void
RTPS_Interface_delete(struct RTPS_Interface *rtps_intf)
{
    struct RTPS_Interface *ext_intf = NULL;

    if (rtps_intf != NULL)
    {
        /* remove external interfaces into factory's ext_intf index */
        if (rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
            NETIO_htonl(RTPS_OBJECT_ID_PARTICIPANT))
        {
            ext_intf = (struct RTPS_Interface *)
                REDA_Indexer_remove_entry(rtps_intf->factory->ext_intf_index,
                                          &rtps_intf->_parent.local_address);
#if OSAPI_ENABLE_LOG
            if (ext_intf == NULL)
            {
                RTPS_LOG_INDEXER_REMOVE_ENTRY(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(ext_intf);
#endif
        }
        RTPS_Interface_finalize(rtps_intf);
        OSAPI_Heap_free_struct(rtps_intf);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Compare function for bind table. Conforms to DB_IndexCompare_T
 *
 * \param[in] left left-side NETIO_Guid entry
 * \param[in] right right-side NETIO_Guid entry
 *
 * \return Comparing left-side NETIO_Guid  and right NETIO_Guid entry:
 *         positive integer if left is greater than right,
 *         negative integer if left is less than right,
 *         zero if left is equal to right
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_guid(struct NETIO_Guid *left,struct NETIO_Guid *right)
{
    RTI_INT32 i;

    for (i = 0; i < 12; ++i)
    {
        if (left->prefix.value[i] > right->prefix.value[i])
        {
            return 1;
        }

        if (left->prefix.value[i] < right->prefix.value[i])
        {
            return -1;
        }
    }

    for (i = 0; i < 4; ++i)
    {
        if (left->entity.value[i] > right->entity.value[i])
        {
            return 1;
        }

        if (left->entity.value[i] < right->entity.value[i])
        {
            return -1;
        }
    }

    return 0;
}

/*ci
 * \brief
 * Compare function for route table's peer index. Conforms to DB_IndexCompare_T
 *
 * \param[in] flags Information about the operands
 * \param[in] op1 Left RTPS_RouteEntry record to compare
 * \param[in] op2 Right NETIO_Address record to compare
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_route_peer(RTI_INT32 flags,
                                 const DB_Record_T op1,
                                 void *op2)
{
    struct RTPS_RouteEntry *left_record = (struct RTPS_RouteEntry *)op1;
    struct NETIO_Guid *right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        right = (struct NETIO_Guid*)op2;
    }
    else
    {
        right = &((struct RTPS_RouteEntry *)op2)->destination;
    }

    return RTPS_Interface_compare_guid(&left_record->destination, right);
}



/*ci
 * \brief
 * Compare function for peer index. Conforms to REDA_Indexer_compare_T
 *
 * \param[in] record Peer entry already in index
 * \param[in] key_is_record Whether key is peer entry record
 * \param[in] key Key to compare
 *
 * \return Given the left record's address and the right key address, return 0
 * \return positive integer if record is greater than key,
 *         negative integer if record is less than key,
 *         zero if record is equal to key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_peer(const void *const record,
                            RTI_BOOL key_is_record,
                            const void *const key)
{
    struct NETIO_Guid *addr_right = (struct NETIO_Guid*)key;

    if (key_is_record)
    {
        addr_right = &((struct RTPS_PeerEntry *)key)->addr;
    }

    return RTPS_Interface_compare_guid(&((struct RTPS_PeerEntry *)record)->addr,
                                 addr_right);
}

/*ci
 * \brief
 * Compare function for internal bind table. Conforms to DB_IndexCompare_T
 *
 * \param[in] flags Unused
 * \param[in] op1 Internal bind entry
 * \param[in] op2 Internal bind entry
 *
 * \return Comparing left bind entry's and right bind entry's interface:
 *         positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_int_bind(RTI_INT32 flags,
                            const DB_Record_T op1, void *op2)
{
    struct RTPS_IntBindEntry *left_record = (struct RTPS_IntBindEntry*)op1;
    struct RTPS_IntBindEntry *right_record = (struct RTPS_IntBindEntry*)op2;
    UNUSED_ARG(flags);

    if (left_record->intf > right_record->intf)
    {
        return 1;
    }

    if (left_record->intf < right_record->intf)
    {
        return -1;
    }

    return 0;
}

/*ci
 * \brief
 * Compare function for external bind table. Conforms to DB_IndexCompare_T
 *
 * \param[in] flags Unused
 * \param[in] op1 External bind entry
 * \param[in] op2 External bind entry
 *
 * \return Comparing left bind entry's and right bind entry's addresses
 *         positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_ext_bind(RTI_INT32 flags,
                            const DB_Record_T op1, void *op2)
{
    struct RTPS_ExtBindEntry *left_record = (struct RTPS_ExtBindEntry*)op1;
    struct RTPS_ExtBindEntry *right_record = (struct RTPS_ExtBindEntry*)op2;
    RTI_INT32 diff;
    UNUSED_ARG(flags);

    diff = RTPS_Interface_compare_guid(&left_record->source,&right_record->source);
    if (diff != 0)
    {
        return diff;
    }

    return OSAPI_Memory_compare(&left_record->destination,
                                &right_record->destination,
                                sizeof(struct NETIO_GuidEntity));
}

/*ci
 * \brief Compare function for direct route index table
 *
 * \param[in] flags Unused
 * \param[in] op1 Route entry
 * \param[in] op2 Route entry
 *
 * \return Comparing left route and right route entry
 *         positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_direct_route(RTI_INT32 flags,
                                    const DB_Record_T op1,void *op2)
{
    struct RTPS_RouteEntry *left = (struct RTPS_RouteEntry *)op1;
    struct RTPS_RouteEntry *right = (struct RTPS_RouteEntry *)op2;
    RTI_INT32 diff;
    UNUSED_ARG(flags);

    diff = RTPS_Interface_compare_guid(&left->destination,&right->destination);
    if (diff != 0)
    {
        return diff;
    }

    /* Make sure higher priority appears first by returning < if left is
     * numerically higher than right
     */
    if (left->direct_priority > right->direct_priority)
    {
        return -1;
    }

    if (left->direct_priority < right->direct_priority)
    {
        return 1;
    }

    diff = NETIO_Address_compare(&left->intf_address,&right->intf_address);
    if (diff != 0)
    {
        return diff;
    }

    return 0;
}

/*ci
 * \brief Compare function for group route index table
 *
 * \param[in] flags Unused
 * \param[in] op1 Route entry
 * \param[in] op2 Route entry
 *
 * \return Comparing left route and right route entry
 *         positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_group_route(RTI_INT32 flags,
                                    const DB_Record_T op1,void *op2)
{
    struct RTPS_RouteEntry *left = (struct RTPS_RouteEntry *)op1;
    struct RTPS_RouteEntry *right = (struct RTPS_RouteEntry *)op2;
    RTI_INT32 diff;
    UNUSED_ARG(flags);

    diff = RTPS_Interface_compare_guid(&left->destination,&right->destination);
    if (diff != 0)
    {
        return diff;
    }

    /* Make sure higher priority appears first by returning < if left is
     * numerically higher than right
     */
    if (left->group_priority > right->group_priority)
    {
        return -1;
    }

    if (left->group_priority < right->group_priority)
    {
        return 1;
    }

    diff = NETIO_Address_compare(&left->intf_address,&right->intf_address);
    if (diff != 0)
    {
        return diff;
    }

    return 0;
}

/*ci
 * \brief Compare function for selected group route
 *
 * \param[in] flags Unused
 * \param[in] op1 route entry
 * \param[in] op2 route entry
 *
 * \return Comparing left route and right route entry
 *         positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_selected_group_route(RTI_INT32 flags,
                                            const DB_Record_T op1,void *op2)
{
    struct RTPS_RouteEntry *left = (struct RTPS_RouteEntry *)op1;
    struct RTPS_RouteEntry *right = (struct RTPS_RouteEntry *)op2;
    UNUSED_ARG(flags);

    /* Make sure selected routes (1) appears before unselected routes (0)
     */
    if (left->selected_group_route < right->selected_group_route)
    {
        return 1;
    }

    if (left->selected_group_route > right->selected_group_route)
    {
        return -1;
    }

    return NETIO_Address_compare(&left->intf_address,&right->intf_address);
}

/*ci
 * \brief Compare entries in the database of route entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A RTPS_RouteEntry already in the database
 * \param[in] op2   Either a RTPS_RouteEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_route(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    RTI_INT32 diff;
    struct RTPS_RouteEntry *lkey = (struct RTPS_RouteEntry*)op1;
    struct RTPS_RouteEntry *rkey = (struct RTPS_RouteEntry*)op2;
    UNUSED_ARG(flags);

    diff = RTPS_Interface_compare_guid(&lkey->destination,&rkey->destination);
    if (diff != 0)
    {
        return diff;
    }

    if (lkey->intf > rkey->intf)
    {
        return 1;
    }

    if (lkey->intf < rkey->intf)
    {
        return -1;
    }

    return  NETIO_Address_compare(&lkey->intf_address,&rkey->intf_address);
}

/*ci
 * \brief
 * Initialize a created interface
 *
 * \param[in] rtps_intf self
 * \param[in] factory RTPS interface factory
 * \param[in] property RTPS interface property
 * \param[in] listener NETIO listener
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_initialize(struct RTPS_Interface *rtps_intf,
                          struct RTPS_InterfaceFactory *factory,
                          const struct RTPS_InterfaceProperty *const property,
                          const struct NETIO_InterfaceListener *const listener)
{
    DB_ReturnCode_T db_rc;
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[RTPS_TABLE_NAME_MAX_LEN];
    union RT_ComponentFactoryId id;
    struct DB_IndexProperty idx_property = DB_IndexProperty_INITIALIZER;
    struct OSAPI_TimeoutHandle TIMEOUT_INIT = OSAPI_TimeoutHandle_INITIALIZER;
    struct REDA_BufferPoolProperty pool_property =
        REDA_BufferPoolProperty_INITIALIZER;
    struct REDA_IndexerProperty index_prop = REDA_IndexerProperty_INITIALIZER;
    struct RTPS_Interface *ext_intf;

    if (!NETIO_Interface_initialize(&rtps_intf->_parent,
                                   &RTPSInterface_fv_Intf,
                                   &property->_parent,
                                   (listener ? listener : NULL)))
    {
        RTPS_LOG_INITIALIZE_INTERFACE(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    if (property->mode == RTPS_INTERFACEMODE_UNDEFINED)
    {
        RTPS_LOG_INTF_MODE_UNDEF(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    /* Route table */
    id._value = factory->_parent._id._value;

    /* Use the entity ID as the unique ID in table names. The entity ID is
     * unique for each RTPS interface within a DomainParticipant and each
     * DomainParticipant has its own database.
     */
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'r',
                   (RTI_INT32)property->intf_address.value.rtps_guid.object_id);

    tbl_property.max_records = property->_parent.max_routes;
    tbl_property.max_indices = RTPS_INTERFACE_ROUTE_TABLE_MAX_INDEXES;
    tbl_property.max_cursors = 2;
    
    if (property->mode != RTPS_INTERFACEMODE_EXTERNAL_RECEIVER)
    {
        ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
                            factory->ext_intf_index, &property->intf_address);
        if (ext_intf == NULL)
        {
            RTPS_LOG_INITIALIZE_INTERFACE(OSAPI_LOGKIND_ERROR)
            goto cleanup;
        }

        db_rc = DB_Database_create_table(&rtps_intf->_parent._rtable,
                property->_parent._parent.db,
                tbl_name,
                sizeof(struct RTPS_RouteEntry),
                RTPS_Interface_compare_route,
                &tbl_property);
        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_CREATE_ROUTE_TABLE(OSAPI_LOGKIND_ERROR, db_rc)
            goto cleanup;
        }

        /* Index to lookup route entry based on peer address */
        db_rc = DB_Table_create_index(rtps_intf->_parent._rtable,
                &rtps_intf->rtable_peer_index,
                RTPS_Interface_compare_route_peer,
                &idx_property);
        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_DB_CREATE_ROUTE_PEER_INDEX(OSAPI_LOGKIND_ERROR, db_rc)
            goto cleanup;
        }

        db_rc = DB_Table_create_index(rtps_intf->_parent._rtable,
                                      &rtps_intf->direct_route,
                                      RTPS_Interface_compare_direct_route,
                                      &idx_property);

        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_DB_CREATE_DIRECT_INDEX(OSAPI_LOGKIND_ERROR,db_rc)
            goto cleanup;
        }

        db_rc = DB_Table_create_index(rtps_intf->_parent._rtable,
                                      &rtps_intf->group_route,
                                      RTPS_Interface_compare_group_route,
                                      &idx_property);

        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_DB_CREATE_GROUP_INDEX(OSAPI_LOGKIND_ERROR,db_rc)
            goto cleanup;
        }

        rtps_intf->packet_buf = ext_intf->packet_buf;
        rtps_intf->packet = ext_intf->packet;
        rtps_intf->selected_group_route_index = NULL;
        rtps_intf->checksum_index = ext_intf->checksum_index;

        db_rc = DB_Table_create_index(rtps_intf->_parent._rtable,
                                      &rtps_intf->selected_group_route_index,
                                      RTPS_Interface_compare_selected_group_route,
                                      &idx_property);

        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_DB_CREATE_SELECTED_GROUP_INDEX(OSAPI_LOGKIND_ERROR,db_rc)
            goto cleanup;
        }
    }
    else
    {
        /* Allocate resources shared by all interfaces sharing the same
         * external interface. The assumption is that the external interface
         * has already been created.
         */
        /* Packet for message originated by RTPS */
        rtps_intf->packet_buf = OSAPI_Heap_allocate(1,RTPS_LOCAL_PACKET_SIZE);
        if (rtps_intf->packet_buf == NULL)
        {
            RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
            goto cleanup;
        }

        OSAPI_Heap_allocate_struct(&rtps_intf->packet,NETIO_Packet_T);
        if (rtps_intf->packet == NULL)
        {
            RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
            goto cleanup;
        }

        rtps_intf->checksum_index = RTPS_NO_CHECKSUM_INDEX;

        /* Check which function to use for sending checksum, if so */
        if (property->computed_crc_kind)
        {
            if (factory->property->checksum.checksum_tx_mode == RTPS_CHECKSUM_TXMODE_RTICRC32)
            {
                rtps_intf->checksum_index = RTPS_CRC32_INDEX;
            }
            else
            {
                RTI_UINT16 computed_crc_kind = property->computed_crc_kind;

               /* Determine the function to use for sending data. An index
                * < 0 means that no checksum is added. This selects the highest index
                * assuming that the properties are valid and that only 1 bit is set.
                */
                while (computed_crc_kind)
                {
                    rtps_intf->checksum_index++;
                    computed_crc_kind = computed_crc_kind >> 1U;
                }
            }
        }
    }

    /* Bind table */
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
               (RTI_INT32)property->intf_address.value.rtps_guid.object_id);

    tbl_property.max_records =  property->_parent.max_binds;

    if (property->mode == RTPS_INTERFACEMODE_EXTERNAL_RECEIVER)
    {
        /* Compare fnc for external receiver considers both source and dest */
        db_rc = DB_Database_create_table(&rtps_intf->_parent._btable,
                                         property->_parent._parent.db,
                                         tbl_name,
                                         sizeof(struct RTPS_ExtBindEntry),
                                         RTPS_Interface_compare_ext_bind,
                                         &tbl_property);
    }
    else
    {
        /* Compare fnc for non external receiver considers only source */
        db_rc = DB_Database_create_table(&rtps_intf->_parent._btable,
                                         property->_parent._parent.db,
                                         tbl_name,
                                         sizeof(struct RTPS_IntBindEntry),
                                         RTPS_Interface_compare_int_bind,
                                         &tbl_property);
    }

    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_CREATE_BIND_TABLE(OSAPI_LOGKIND_ERROR, db_rc)
        goto cleanup;
    }

    rtps_intf->property = *property;

    /* Window size is limited by bitmap implementation */
    if (rtps_intf->property.max_window_size > RTPS_BITMAP_SIZE_MAX)
    {
        rtps_intf->property.max_window_size = RTPS_BITMAP_SIZE_MAX;
    }

    rtps_intf->factory = factory;
    rtps_intf->_parent.state = NETIO_INTERFACESTATE_CREATED;
    rtps_intf->_parent.local_address = property->intf_address;

    /* Create buffer pool of peers */
    pool_property.buffer_size = sizeof(struct RTPS_PeerEntry);
    pool_property.max_buffers = (RTI_SIZE_T)property->max_peer_count;
    pool_property.flags |= REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
    rtps_intf->peers_pool = REDA_BufferPool_new("peers_pool",&pool_property,
                                                NULL, NULL, NULL, NULL);

    if (rtps_intf->peers_pool == NULL)
    {
        RTPS_LOG_CREATE_BUFFER_POOL(OSAPI_LOGKIND_ERROR,
                                    pool_property.buffer_size,
                                    pool_property.max_buffers)
        goto cleanup;
    }

    /* Index for lookup of active peers */
    index_prop.max_entries = property->max_peer_count;
    rtps_intf->peers_index = REDA_Indexer_new(
       RTPS_Interface_compare_peer, &index_prop);
    if (rtps_intf->peers_index == NULL)
    {
        RTPS_LOG_CREATE_PEERS_INDEX(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    /* Sequence used for locally generated sends */
    if (!NETIO_AddressSeq_initialize(&rtps_intf->local_dest_seq))
    {
        RTPS_LOG_SEQ_INIT(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    if (!NETIO_AddressSeq_set_maximum(&rtps_intf->local_dest_seq, 1))
    {
        RTPS_LOG_SEQ_MAX(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    if (!NETIO_AddressSeq_set_length(&rtps_intf->local_dest_seq, 1))
    {
        RTPS_LOG_SEQ_LEN(OSAPI_LOGKIND_ERROR)
        goto cleanup;
    }

    /* Initialize Writer */
    if (property->mode == RTPS_INTERFACEMODE_WRITER)
    {
        struct RTPS_Writer *writer = &rtps_intf->endpoint.writer;

        REDA_SequenceNumber_set_zero(&writer->first_sn);
        REDA_SequenceNumber_set_zero(&writer->last_sn);

        writer->upstr_intf = NULL;
        writer->flags = RTPS_WRITER_FLAG_NONE;

        if (property->reliable)
        {
            writer->flags |= RTPS_WRITER_FLAG_RELIABLE;
        }
        else
        {
            /* best effort writer has no send window */
            rtps_intf->property.max_window_size = 0;
        }

#if RTPS_RELIABILITY
        writer->hb_epoch = 0;
        writer->hb_event = TIMEOUT_INIT;
        writer->piggyback_sample_count = 0;
        writer->hb_period = property->hb_period;
        writer->samples_per_hb = (RTI_INT32)property->samples_per_hb;
        writer->inactive_reliable_reader_count = 0;
        writer->active_reliable_reader_count = 0;
#endif

        if ((rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
            (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION)) ||
            (rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
            (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION)))
        {
            rtps_intf->endpoint.writer.flags |= RTPS_WRITER_BUILTIN_WRITER;
        }

    }
    /* Initialize Reader */
    else if (property->mode == RTPS_INTERFACEMODE_READER)
    {
        rtps_intf->endpoint.reader.flags = RTPS_READER_FLAG_NONE;
        if (property->reliable)
        {
            rtps_intf->endpoint.reader.flags |= RTPS_READER_FLAG_RELIABLE;
        }

        if ((rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
            (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PUBLICATION)) ||
            (rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
            (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION)))
        {
            rtps_intf->endpoint.reader.flags |= RTPS_READER_BUILTIN_READER;
        }

#if RTPS_RELIABILITY
        rtps_intf->endpoint.reader.ack_epoch = 0;
#endif

        OSAPI_NtpTime_to_nanosec(&rtps_intf->endpoint.reader.acknack_sec,
                                 (RTI_UINT32*)&rtps_intf->endpoint.reader.acknack_nanosec,
                                 &property->nack_period);

        rtps_intf->endpoint.reader.acknack_event = TIMEOUT_INIT;

        rtps_intf->endpoint.reader.reserved_count = 0;
    }

    return RTI_TRUE;

cleanup:
#ifndef RTI_CERT
    RTPS_Interface_finalize(rtps_intf);
#endif
    return RTI_FALSE;
}

/*ci
 * \brief
 * Compare function for external interface index. Conforms to
 * REDA_Indexer_compare_T
 *
 * \details
 * Compares the GUID Prefix portion of the GUID that corresponds to the
 * DomainParticipant, and the port which has been set to the domain ID
 *
 * \param[in] record RTPS interface already in index
 * \param[in] key_is_record Whether key is RTPS_Interface
 * \param[in] key Key to compare
 *
 * \return positive integer if record is greater than key,
 *         negative integer if record is less than key,
 *         zero if record is equal to key
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RTPS_Interface_compare_ext_intf(const void *const record,
                                RTI_BOOL key_is_record,
                                const void *const key)
{
    struct RTPS_Interface *intf_left = (struct RTPS_Interface *)record;
    struct NETIO_Address *addr_right = (struct NETIO_Address *)key;
    RTI_INT32 result;

    if (key_is_record)
    {
        addr_right = &((struct RTPS_Interface *)key)->_parent.local_address;
    }
    else
    {
        addr_right = (struct NETIO_Address *)key;
    }

    /* compare participant guid-prefix */
    result = OSAPI_Memory_compare(&intf_left->_parent.local_address.value,
                                  &addr_right->value,
                                  sizeof(struct NETIO_GuidPrefix));
    if (result != 0)
    {
        return result;
    }
    else if (intf_left->_parent.local_address.port < addr_right->port)
    {
        return -1;
    }
    else if (intf_left->_parent.local_address.port > addr_right->port)
    {
        return 1;
    }

    return 0;
}


/*ci
 * \brief
 * Create a new RTPS interface
 *
 * \param[in] factory RTPS interface factory
 * \param[in] property RTPS interface property
 * \param[in] listener NETIO listener
 *
 * \return pointer to RTPS interface on success, otherwise NULL
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RTPS_Interface*
RTPS_Interface_create(struct RTPS_InterfaceFactory *factory,
                      const struct RTPS_InterfaceProperty *const property,
                      const struct NETIO_InterfaceListener *const listener)
{
    struct RTPS_Interface *rtps_intf = NULL;
    struct RTPS_Interface *ext_intf = NULL;
    struct REDA_IndexerProperty index_prop = REDA_IndexerProperty_INITIALIZER;

    if (!factory->_initialized)
    {
        /* Initialized factory will have external intf index created */
        index_prop.max_entries = RTPS_EXT_INTF_MAX_ENTRIES;
        factory->ext_intf_index = REDA_Indexer_new(
           RTPS_Interface_compare_ext_intf,&index_prop);
        if (factory->ext_intf_index == NULL)
        {
            RTPS_LOG_INITIALIZE_INTERFACE(OSAPI_LOGKIND_ERROR)
            return NULL;
        }

        factory->_initialized = RTI_TRUE;
    }

    OSAPI_Heap_allocate_struct(&rtps_intf, struct RTPS_Interface);
    if (rtps_intf == NULL)
    {
        RTPS_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (!RTPS_Interface_initialize(rtps_intf, factory, property, listener))
    {
        RTPS_LOG_INITIALIZE_INTERFACE(OSAPI_LOGKIND_ERROR)
#ifndef RTI_CERT
        RTPS_Interface_delete(rtps_intf);
#endif
        return NULL;
    }

    /* Assert external interfaces into factory's ext_intf index */
    /* An RTPS external intf maps directly to a DDS DomainParticipant. */
    if (rtps_intf->_parent.local_address.value.rtps_guid.object_id ==
        NETIO_htonl(RTPS_OBJECT_ID_PARTICIPANT))
    {
        ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
           factory->ext_intf_index, &rtps_intf->_parent.local_address);
        if (ext_intf == NULL)
        {
            /* new external interface -> add intf to index */
            if (!REDA_Indexer_add_entry(factory->ext_intf_index, rtps_intf))
            {
                RTPS_LOG_INDEX_ADD_ENTRY(OSAPI_LOGKIND_ERROR)
#ifndef RTI_CERT
                RTPS_Interface_delete(rtps_intf);
#endif
                return NULL;
            }
        }
    }

    return rtps_intf;
}

/*ci
 * \brief
 * Reset window to be empty starting at specified seq num
 *
 * \param[in] window The window to reset
 * \param[in] max_window_size The max window size
 * \param[in] sn Starting sequence number
 *
 */
RTI_PRIVATE void
RTPS_Window_reset(struct RTPS_Window *window,
                  RTI_UINT32 max_window_size,
                  RTPS_SampleId_T *sn)
{
    RTPS_SampleId_T tmp = REDA_SEQUENCE_NUMBER_ZERO;

    window->size = 0;
    window->max_size = max_window_size;
    RTPS_Bitmap_reset(&window->bitmap, sn, (RTI_INT32)max_window_size);

    tmp.low = max_window_size - 1;
    REDA_SequenceNumber_add(&window->tail, sn, &tmp);
}

/*ci
 * \brief
 * Checks whether a sequence number is present in window
 *
 * \param[in] window The window to check
 * \param[in] sn Sequence number to check
 *
 * \return RTI_TRUE if sn is present in window, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Window_sn_within_window(struct RTPS_Window *window,
                             RTPS_SampleId_T *sn)
{
    if ((REDA_SequenceNumber_compare(sn, &window->bitmap.lead) < 0) ||
        (REDA_SequenceNumber_compare(sn, &window->tail) > 0))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}


/*ci
 * \brief
 * Insert a sequence number into window
 *
 * \param[in] window The window to insert into
 * \param[in] sn Sequence number to insert
 *
 * \return RTI_TRUE if sn was successfully inserted, RTI_FALSE otherwise due
 * to out-of-range sn.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Window_insert(struct RTPS_Window *window,
                   RTPS_SampleId_T *sn)
{
    RTI_BOOL exists = RTI_FALSE;

    if (!RTPS_Bitmap_set_bit(&window->bitmap, &exists, sn, RTI_TRUE))
    {
        /* out of range of window */
        RTPS_LOG_FULL_SEND_WINDOW(OSAPI_LOGKIND_INFO)
        return RTI_FALSE;
    }

    if (!exists)
    {
        ++window->size;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Remove a sequence number from window
 *
 * \param[in] window  The window to remove from
 * \param[out] removed RTI_TRUE if sn was removed, RTI_FALSE otherwise.
 *                     May be NULL.
 * \param[in] sn Sequence number to remove
 *
 */
RTI_PRIVATE void
RTPS_Window_remove(struct RTPS_Window *window,
                   RTI_BOOL *removed,
                   RTPS_SampleId_T *sn)
{
    RTI_BOOL unchanged = RTI_TRUE;

    if (removed != NULL)
    {
        *removed = RTI_FALSE;
    }

    if (window->size == 0)
    {
        return;
    }

    if (RTPS_Bitmap_set_bit(&window->bitmap, &unchanged, sn, RTI_FALSE) &&
        !unchanged)
    {
        --window->size;
        if (removed != NULL)
        {
            *removed = RTI_TRUE;
        }
    }
}

/*ci
 * \brief
 * Advance starting sequence number of window
 *
 * \param[in] window The window to advance in
 * \param[in] sn New starting sequence number of window
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Window_advance(struct RTPS_Window *window,
                    RTPS_SampleId_T *sn)
{
    RTPS_SampleId_T lowest_sn, tmp = REDA_SEQUENCE_NUMBER_ZERO;

    /* Must not advance beyond lowest unacked SN */
    if (RTPS_Bitmap_get_first_bit(&window->bitmap, &lowest_sn, RTI_TRUE))
    {
        if (REDA_SequenceNumber_compare(sn, &lowest_sn) > 0)
        {
            RTPS_LOG_WINDOW_ADVANCE_UNACKED(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    if (!RTPS_Bitmap_shift(&window->bitmap, sn))
    {
        RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    tmp.low = window->max_size - 1;
    REDA_SequenceNumber_add(&window->tail, &window->bitmap.lead, &tmp);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Send a packet to a specific peer or all peers.
 *
 * \param[in] intf The interface to send from
 * \param[in] packet The packet to send
 * \param[in] peer_entry Peer entry to send to. If peer_entry is NULL,
 *                       the message is sent to all matched peers, otherwise
 *                       the message it sent to the specified peer.
 * \param[in] send_flags Flags indicating kind of payload in packet
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Sender_route_packet(struct RTPS_Interface *intf,
                         NETIO_Packet_T *packet,
                         struct RTPS_PeerEntry *peer_entry,
                         RTPS_SendFlags_T send_flags)
{
    RTI_BOOL ok = RTI_TRUE;
    struct RTPS_RouteEntry *route_entry = NULL;
    RTI_SIZE_T pkt_head, pkt_tail;
    struct RTPS_RouteEntry *last_route = NULL;
    struct NETIO_AddressSeq *upstr_dest_seq = NULL;
    struct NETIO_Guid guid_max = {{{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                   0xff,0xff,0xff,0xff}},{{0xff,0xff,0xff,0xff}}};
    RTI_UINT8 min_pri = 1;
    RTI_UINT8 max_pri = 255;
    RTI_UINT32 max_port = 0xffffffff;
    struct RTPS_RouteEntry route_key_low;
    struct RTPS_RouteEntry route_key_high;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_Address dest_addr;
    RTI_BOOL is_reliable_writer;

    upstr_dest_seq = packet->dests;
    packet->dests = &intf->local_dest_seq;

    is_reliable_writer = (intf->property.mode == RTPS_INTERFACEMODE_WRITER) &&
                          intf->property.reliable;

    if (peer_entry != NULL)
    {
        if (is_reliable_writer && (send_flags & RTPS_SEND_DATA_FLAG))
        {
            if (peer_entry->info.reader.reliable &&
                !peer_entry->info.reader.is_inactive &&
                !RTPS_Window_insert(&peer_entry->info.reader.window,
                                    &packet->info.sn))
            {
                goto done;
            }
        }

        route_key_low.destination = peer_entry->addr;
        NETIO_Address_init(&route_key_low.intf_address,0);

        /* 255 is logically lower than 1 when comparing priorities */
        route_key_low.direct_priority = max_pri;

        /* 1 is lowest priority, appears last in the index. Because not all
         * fields in RTPS_RouteEntry are used in the comparison in the
         * direct_route index, the Coverity [uninit_use] event as marked as
         * 'Intentional'.
         */
        /* coverity[uninit_use] */
        route_key_high = route_key_low;
        route_key_high.direct_priority = min_pri;

        NETIO_Address_set_guid(&route_key_high.intf_address,max_port,&guid_max);

        dbrc = DB_Table_select_range(intf->_parent._rtable,intf->direct_route,
                                       &cursor,&route_key_low,&route_key_high);
    }
    else
    {
        /* Only update the window if there is data being sent (only data
         * is sequenced).
         */
        if (is_reliable_writer && (send_flags & RTPS_SEND_DATA_FLAG))
        {
            RTI_INT32 peers_count,i;
            struct RTPS_PeerEntry *peer;

            peers_count = REDA_Indexer_get_count(intf->peers_index);
            for (i = 0; i < peers_count; ++i)
            {
                peer = (struct RTPS_PeerEntry*)
                                REDA_Indexer_get_entry(intf->peers_index,i);

                if (peer->info.reader.reliable &&
                        !peer->info.reader.is_inactive &&
                        !RTPS_Window_insert(&peer->info.reader.window,
                                            &packet->info.sn))
                {
                    break;
                }
            }

            if (i < peers_count)
            {
                /* Failed to update a reader. Revert window update on previous
                 * readers and don't send any data. Since the i-th update
                 * failed, start at the index before.
                 */
                for (i = i - 1; i >= 0; --i)
                {
                    peer = (struct RTPS_PeerEntry*)
                                REDA_Indexer_get_entry(intf->peers_index,i);

                    if (peer->info.reader.reliable &&
                            !peer->info.reader.is_inactive)
                    {
                        RTPS_Window_remove(&peer->info.reader.window,
                                           NULL,&packet->info.sn);
                    }
                }

                goto done;
            }
        }

        dbrc = DB_Table_select_all(intf->_parent._rtable,
                                   intf->selected_group_route_index,
                                   &cursor);
    }

    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)(&route_entry));
    while (dbrc == DB_RETCODE_OK)
    {
        /* For participant announcements, all locators are used */
        if (!intf->property.anonymous)
        {
            if (peer_entry == NULL)
            {
                /* For groups sends send to all the routes that has been
                 * selected. All selected routes comes first, thus it is
                 * sufficient to test if the route has been selected.
                 */
                if (!route_entry->selected_group_route)
                {
                    /* Last selected route has been reached, stop */
                    break;
                }
                /* If route_entry->selected_group_route is TRUE, continue */
            }
            else
            {
                /* This is a direct send */
                if ((last_route != NULL) &&
                    (last_route->direct_priority != route_entry->direct_priority))
                {
                    /* Stop sending if a different priority is reached.
                     * We only send to one priority level for either type.
                     */
                    break;
                }
            }
        }

        last_route = route_entry;

        *NETIO_AddressSeq_get_reference(packet->dests, 0) =
                                                    route_entry->intf_address;

        NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);

        NETIO_Address_set_guid(&dest_addr,0,&route_entry->destination);

        if (!NETIO_Interface_send(route_entry->intf,&intf->_parent,
                                  &dest_addr,packet))
        {
            /* Fail on exit, but continue sending to other routes */
            RTPS_LOG_SEND(OSAPI_LOGKIND_ERROR)
            ok = RTI_FALSE;
        }

        NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)(&route_entry));
    }

    DB_Cursor_finish(intf->_parent._rtable,cursor);

done:

    packet->dests = upstr_dest_seq;

    return ok;
}

/*ci
 * \brief
 * Set a GAP submessage in a packet
 *
 * \param[in] packet The packet to update
 * \param[in] gap_sn_start First SN of GAP
 * \param[in] gap_sn_end SN that is one greater than the last SN of GAP
 * \param[in] writer_entity Entity ID of source writer
 * \param[in] reader_entity Entity ID of destination reader
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Writer_set_gap(NETIO_Packet_T *packet,
                   RTPS_SampleId_T *gap_sn_start,
                   RTPS_SampleId_T *gap_sn_end,
                   RTPS_Entity_T *writer_entity,
                   RTPS_Entity_T *reader_entity)
{
    union RTPS_MESSAGES *msg = NULL;
    struct RTPS_Bitmap resend_bitmap;
    RTI_SIZE_T payload_length;

    /* PRECONDITION: gap_sn_start < gap_sn_end */
    if (REDA_SequenceNumber_compare(gap_sn_start, gap_sn_end) >= 0)
    {
        return RTI_FALSE;
    }

    /* Prepended to the packet's buffer, where space for GAP is made
     *  by decreasing the packet's head position.
     */
    RTPS_Bitmap_reset(&resend_bitmap, gap_sn_end, 0);
    payload_length = RTPS_Interface_get_gap_size(resend_bitmap.bit_count);

    if (!NETIO_Packet_set_head(packet, 0 - (RTI_INT32)payload_length))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    RTPS_Interface_set_gap(&msg->gap, payload_length, *reader_entity,
                           *writer_entity, gap_sn_start, &resend_bitmap);
    return RTI_TRUE;
}

/*ci
 * \brief
 * Writer sends a packet directly to a reader, setting RTPS submessages within
 * the packet's buffer
 *
 * \param[in] intf The interface to send from
 * \param[in] send_flags Submessage-specific flags
 * \param[in] packet The packet to send
 * \param[in] peer_entry Destination reader's peer entry
 * \param[in] writer_entity Entity ID of source writer
 * \param[in] reader_entity Entity ID of destination reader
 * \param[in] gap_sn_start First SN of GAP, if needed
 * \param[in] gap_sn_end SN that is one greater than the last SN of GAP, if
 * needed
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Writer_direct_send(struct RTPS_Interface *intf,
                        RTPS_SendFlags_T send_flags,
                        NETIO_Packet_T *packet,
                        struct RTPS_PeerEntry *peer_entry,
                        RTPS_Entity_T *writer_entity,
                        RTPS_Entity_T *reader_entity,
                        RTPS_SampleId_T *gap_sn_start,
                        RTPS_SampleId_T *gap_sn_end)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_UINT8 data_flags = RTPS_FLAGS_NONE;
    RTI_UINT8 hb_flags = RTPS_FLAGS_NONE;
    RTI_SIZE_T payload_length, actual_payload_length;
    RTI_INT32 tail_adj;
    union RTPS_MESSAGES *msg = NULL;
    struct RTPS_HEADER_EXT *hdrext = NULL;
    RTI_BOOL is_data_last = RTI_FALSE;

    OSAPI_TRACE_NET("direct send",RTI_FALSE)
    OSAPI_TRACE_GUID("writer",&writer_entity->value,RTI_FALSE)
    OSAPI_TRACE_GUID("reader",&reader_entity->value,RTI_FALSE)
    OSAPI_TRACE_INT32("flags",send_flags,RTI_FALSE)
    OSAPI_TRACE_INT32("flags",send_flags,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)

    /* Packet may be sent multiple times, so must cache its state before send,
     * and restore state after send
     */
    NETIO_Packet_save_positions(packet);

    payload_length = RTPS_Interface_get_data_size(
                                    NETIO_Packet_get_payload_length(packet));

    actual_payload_length = payload_length;

    /* If the actual_payload_length exceeds an unsigned 16-bit integer, do not
     * add a piggyback HB and let the DATA submessage extend to the end of the
     * UDP payload.
     *
     * NOTE: This does not mean that the message can be sent by the
     *       downstream transport, _only_ that a DATA submessage > 64KB is
     *       correctly formatted.
     */
    if ((send_flags & RTPS_SEND_DATA_FLAG)
        && (actual_payload_length > RTPS_MAX_DATA_LENGTH))
    {
        is_data_last = RTI_TRUE;
    }

    /* HEARTBEAT */
    /* Appended to the packet's buffer, where space for HEARTBEAT is made
     * by advancing the packet's tail
     */
    if (!is_data_last &&
        (send_flags & (RTPS_SEND_HB_FLAG | RTPS_SEND_LIVE_HB_FLAG)))
    {
        /* If a piggyback HB is added, make sure the payload_length
         * is divisible by 4, such that the tail's start is 4-byte aligned
         * relative to the packet's head
         */
        actual_payload_length = (actual_payload_length + 3U) & ~0x3U;

        /* Make sure the current-tail is aligned to 4 */
        tail_adj = (RTI_INT32)(actual_payload_length - payload_length);

        if (!NETIO_Packet_set_tail(packet, tail_adj))
        {
            RTPS_LOG_NETIO_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        /* Now set the HB */
        msg = (union RTPS_MESSAGES *)NETIO_Packet_get_tail(packet);
        if (!NETIO_Packet_set_tail(packet, (sizeof(struct RTPS_HEARTBEAT))))
        {
            RTPS_LOG_NETIO_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if (send_flags & RTPS_SEND_LIVE_HB_FLAG)
        {
            hb_flags = RTPS_HBFLAGS_L | RTPS_HBFLAGS_F;
        }

        /* MICRO-1579: While the writer keeps track of what is available
         * on a per remote reader basis, always annouce the full HB. This
         * is necessary in case a remote reader resets the link by sending
         * an ACKANCK with a bitmap lead of 0. In a crosssing HBs is for the
         * previous session then the reader may think that some samples are
         * not relevant and not NACK them. This will cause discovery to fail.
         * When the full HB is sent, a subsequent NACK may instead result in a
         * GAP being sent.
         */
        intf->endpoint.writer.hb_epoch++;
        RTPS_Interface_set_heartbeat(&msg->hb, hb_flags,
                                     *reader_entity, *writer_entity,
                                     &intf->endpoint.writer.first_sn,
                                     &intf->endpoint.writer.last_sn,
                                     intf->endpoint.writer.hb_epoch);
        /* Ensure HB's first SN > 0 */
        if (REDA_SequenceNumber_is_zero(&msg->hb.sn_first))
        {
            REDA_SequenceNumber_plusplus(&msg->hb.sn_first);
        }
    }

    /* DATA */
    /* Prepended to the packet's buffer, where space for DATA is made
     *  by decreasing the packet's head position.
     */
    if (send_flags & RTPS_SEND_DATA_FLAG)
    {
        if (is_data_last)
        {
            /* Use RTPS_SUBMSG_HEADER_LEN because RTPS_Interface_set_data
             * subtracts it.
             */
            actual_payload_length = RTPS_SUBMSG_HEADER_LEN;
        }

        if (!NETIO_Packet_set_head(packet, 0 - (RTI_INT32)RTPS_Interface_get_data_size(0)))
        {
            RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        msg = (union RTPS_MESSAGES*)NETIO_Packet_get_head(packet);

        if ((packet->info.rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS) != 0)
        {
            data_flags |= RTPS_DATAFLAGS_Q;
        }
        if (packet->info.valid_data)
        {
            data_flags |= RTPS_DATAFLAGS_D;
        }

        RTPS_Interface_set_data(&msg->data, data_flags, actual_payload_length,
                                RTPS_FLAGS_NONE, RTPS_DATA_INLINEQOS_OFFSET,
                                *reader_entity, *writer_entity, &packet->info.sn);
    }

    /* GAP */
    if (send_flags & RTPS_SEND_GAP_FLAG)
    {
        if (!RTPS_Writer_set_gap(packet, gap_sn_start, gap_sn_end,
                                 writer_entity, reader_entity))
        {
            RTPS_LOG_SET_GAP(OSAPI_LOGKIND_ERROR,
                             gap_sn_start->low, gap_sn_end->low,
                             writer_entity->value[0])
            goto done;
        }
    }


    /* INFO_DST */
    /* Prepended to the packet's buffer */
    if (!NETIO_Packet_set_head(packet, 0L-(RTI_INT32)(sizeof(struct RTPS_INFO_DST))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    RTPS_Interface_set_info_dst(&msg->info_dst,
                                &peer_entry->addr.prefix);

    if (send_flags & RTPS_SEND_DATA_FLAG)
    {
        /* INFO_TS */
        if (!NETIO_Packet_set_head(packet, 0L-(RTI_INT32)(sizeof(struct RTPS_INFO_TS))))
        {
            RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
                goto done;
        }

        msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
        RTPS_Interface_set_info_ts(&msg->info_ts, &packet->info.timestamp);
    }
    
    if (intf->checksum_index > RTPS_NO_CHECKSUM_INDEX)
    {
        /* This adds sufficient space for the header extension */
        if (!RTPS_Interface_add_header_extension(intf,packet))
        {
            goto done;
        }

        /* Save the location of the header extension as the entire RTPS
         * message must be built before it can be filled in.
         */
        hdrext = (struct RTPS_HEADER_EXT*)NETIO_Packet_get_head(packet);
    }

    /* Prepend RTPS Header to the packet's buffer
     */
    if (!NETIO_Packet_set_head(packet, 0L-(RTI_INT32)(sizeof(struct RTPS_Header))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    RTPS_Interface_set_message_header(&msg->header,
                                &intf->_parent.local_address.value.guid.prefix);

    /* Send directly to peer with its list of routes */
    packet->source = intf->_parent.local_address;

    if (hdrext != NULL)
    {
        /* Update the header extension if it was added */
        if (!RTPS_Interface_set_header_extension(intf,hdrext,packet))
        {
            goto done;
        }
    }

    if (!RTPS_Sender_route_packet(intf, packet,peer_entry,send_flags))
    {
        RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    ok = RTI_TRUE;
done:
    NETIO_Packet_restore_positions(packet);
    return ok;
}

/*ci
 * \brief
 * Request a sample from upstream writer queue to resend directly to
 * a reader
 *
 * \details
 * A sample is requested from the writer queue for resend when the writer
 * receives an ACKNACK from a reader, or when a reader peer is initially
 * asserted and thus the writer pushes its historical queued samples to it.
 *
 * \param[in] intf The local RTPS interface
 * \param[in] writer Local writer doing resend
 * \param[in] reader Destination reader's state entry
 * \param[in] peer Destination reader's peer entry
 * \param[inout] packet Packet containing requested sample
 * \param[in] ctxt Context keeping state of resend
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Writer_request_and_resend_packet(struct RTPS_Interface *intf,
                                      struct RTPS_Writer *writer,
                                      struct RTPS_RemoteReader *reader,
                                      struct RTPS_PeerEntry *peer,
                                      NETIO_Packet_T **packet,
                                      struct RTPS_ResendContext *ctxt)
{
    RTI_BOOL bretval;
    struct NETIO_Address peer_addr;

    /* Context's send flags apply per packet, so reset before each send */
    ctxt->send_flags = 0;

    NETIO_Address_set_guid(&peer_addr,0,&peer->addr);

    if (!NETIO_Interface_request(writer->upstr_intf,
                                 &peer_addr,
                                 &intf->_parent.local_address,
                                 packet,
                                (NETIO_PacketId_T*)&ctxt->req_sn,
                                (NETIO_PacketId_T*)&ctxt->actual_sn))
    {
        RTPS_LOG_REQUEST(OSAPI_LOGKIND_ERROR)

        /* error not fatal, get sample after req sn */
        ctxt->actual_sn = ctxt->req_sn;
        return RTI_TRUE;
    }

    /* Don't send sample if pull mode, instead just insert in window */
    if (ctxt->pull_mode)
    {
        if (*packet != NULL)
        {
            bretval = RTPS_Window_insert(&reader->window, &ctxt->actual_sn);
#if OSAPI_ENABLE_LOG
            if (!bretval)
            {
                RTPS_LOG_WINDOW_INSERT(OSAPI_LOGKIND_INFO)
            }
#else
            IGNORE_RETVAL(bretval);
#endif
            bretval = NETIO_Interface_return_loan(writer->upstr_intf,
                                          &peer_addr,*packet,
                                          (NETIO_PacketId_T*)&ctxt->actual_sn);
            IGNORE_RETVAL(bretval);
        }
        return RTI_TRUE;
    }

    /* By here, requested sample will be sent (not in pull mode) */

    /* After the request, the returned packet may be NULL, which means
     * no valid data is available for an SN equal to or greater than the
     * requested SN.
     *
     * If the returned packet is not NULL, it contains either valid or invalid
     * data.
     *
     * The returned actual_sn is the smallest SN greater than or equal to
     * req_sn that has valid data.  This means if the returned packet
     * contains valid data, actual_sn is equal to req_sn. If the packet does
     * not contain valid data, actual_sn is the smallest SN larger than
     * req_sn that has valid data.
     *
     * For all SN in the range of req_sn to actual_sn, inclusive, the
     * RTPS writer must send a GAP for all SN of invalid data,
     * and DATA for SN of valid data.
     *
     * To optimize bandwidth usage, GAPs are combined for consecutive requests
     * that return only invalid data.  That is, if this request returned only
     * SN of invalid data, its GAP can be combined with either a GAP from the
     * previous request, or a GAP of the next request.  Once a request returns
     * valid data, any GAP in process of being combined will be sent (preprended
     * to the DATA in a single RTPS message) and a next invalid data will start
     * a new GAP.
     *
     */

    /* If req_sn != actual_sn, create GAP from req_sn to actual_sn */
    if ((*packet == NULL) ||
        (REDA_SequenceNumber_compare(&ctxt->req_sn, &ctxt->actual_sn) != 0))
    {
        if (!ctxt->gap_in_progress)
        {
            ctxt->gap_start = ctxt->req_sn;
            ctxt->gap_in_progress = RTI_TRUE;
        }

        /* The GAP is from req_sn to actual_sn, exclusive.

           If the request returned a NULL packet, actual_sn was not set. We
           must GAP up through the last sample written. Thus, we must set
           actual_sn to one greater than the writer's last SN.
        */
        if (*packet == NULL)
        {
            ctxt->actual_sn = writer->last_sn;
            REDA_SequenceNumber_plusplus(&ctxt->actual_sn);
        }
        ctxt->gap_end = ctxt->actual_sn;
    }

    if (ctxt->gap_in_progress)
    {
        ctxt->send_flags |= RTPS_SEND_GAP_FLAG;
    }

    /* Resend DATA only if fits within send window */
    if ((*packet != NULL) &&
        RTPS_Window_sn_within_window(&reader->window, &ctxt->actual_sn))
    {
        ctxt->send_flags |= RTPS_SEND_DATA_FLAG;
    }
    else
    {
        if (*packet != NULL)
        {
            bretval = NETIO_Interface_return_loan(writer->upstr_intf,
                                          &peer_addr,*packet,
                                          (NETIO_PacketId_T*)&ctxt->actual_sn);
            IGNORE_RETVAL(bretval);
        }

        /* If not resending DATA, nullify packet */
        *packet = NULL;
    }

    /* Resend GAP and/or DATA */
    if (ctxt->send_flags)
    {
        /* A NULL packet here means just a GAP is being sent */
        if (*packet == NULL)
        {
            if (!RTPS_Interface_initialize_packet(intf))
            {
                RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }
            *packet = intf->packet;
        }
        if (!RTPS_Writer_direct_send(intf, ctxt->send_flags, *packet, peer,
                                     &ctxt->writer_entity, &ctxt->reader_entity,
                                     &ctxt->gap_start, &ctxt->gap_end))
        {
            RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
            if (*packet != intf->packet)
            {
                bretval = NETIO_Interface_return_loan(writer->upstr_intf,
                                      &peer_addr,*packet,
                                      (NETIO_PacketId_T*)&ctxt->actual_sn);
                IGNORE_RETVAL(bretval);
            }
            return RTI_FALSE;
        }
        ctxt->gap_in_progress = RTI_FALSE;
        ctxt->resent = RTI_TRUE;
    }

    if ((*packet != NULL) && (*packet != intf->packet))
    {
        bretval = NETIO_Interface_return_loan(writer->upstr_intf,
                                      &peer_addr,*packet,
                                      (NETIO_PacketId_T*)&ctxt->actual_sn);
        IGNORE_RETVAL(bretval);
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Writer requests historical samples, if any, to send to a reader
 *
 * \details
 * Called upon asserting a new peer reader.  Writer requests historical samples
 * until all historical samples are requested or until its send window is full.
 *
 * \param[in] intf Self (writer) interface
 * \param[in] peer Destination reader's peer entry
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Writer_request_history(struct RTPS_Interface *intf,
                            struct RTPS_PeerEntry *peer)
{
    RTPS_SampleId_T SN_ONE = {0,1};
    NETIO_Packet_T *packet = NULL;
    struct RTPS_ResendContext ctxt = RTPS_ResentContext_INITIALIZER;

    struct RTPS_Writer *writer = &intf->endpoint.writer;
    struct RTPS_RemoteReader *reader = &peer->info.reader;

    /* Request and send historical samples */
    /* No historical samples if last_acked_sn is zero */
    if (!REDA_SequenceNumber_is_zero(&reader->last_acked_sn))
    {
        /* Push history over-the-wire only if valid ACKNACK received from reader,
           meaning an ACKNACK with non-zero epoch has been received */
        ctxt.pull_mode = (reader->epoch == 0 ? RTI_TRUE : RTI_FALSE);
        ctxt.writer_entity = intf->_parent.local_address.value.guid.entity;
        ctxt.reader_entity = peer->addr.entity;

         /* [PUSH_MODE]: Send a GAP from SN 1 through last_acked_sn */
        if (!ctxt.pull_mode)
        {
            if (!RTPS_Interface_initialize_packet(intf))
            {
                RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }

            ctxt.gap_end = reader->last_acked_sn;
            REDA_SequenceNumber_plusplus(&ctxt.gap_end);

            if (!RTPS_Writer_direct_send(intf,RTPS_SEND_GAP_FLAG,intf->packet,
                                peer, &ctxt.writer_entity, &ctxt.reader_entity,
                                &SN_ONE, &ctxt.gap_end))
            {
                RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }
        }

        /* Request historical samples until
           1) requested up through last SN written, or
           2) send window is full
        */
        ctxt.req_sn = reader->last_acked_sn;
        REDA_SequenceNumber_plusplus(&ctxt.req_sn);

        while (RTPS_Window_sn_within_window(&reader->window, &ctxt.req_sn) &&
             (REDA_SequenceNumber_compare(&ctxt.req_sn, &writer->last_sn) <= 0))
        {
            if (!RTPS_Writer_request_and_resend_packet(
               intf, writer, reader, peer, &packet, &ctxt))
            {
                return RTI_FALSE;
            }

            /* Request next sample */
            ctxt.req_sn = ctxt.actual_sn;
            REDA_SequenceNumber_plusplus(&ctxt.req_sn);
        }

        /* Always send trailing HEARTBEAT */
        if (!RTPS_Interface_initialize_packet(intf))
        {
            RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
        if (!RTPS_Writer_direct_send(intf, RTPS_SEND_HB_FLAG, intf->packet,
                    peer, &ctxt.writer_entity, &ctxt.reader_entity, NULL, NULL))
        {
            RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }
    return RTI_TRUE;
}

/*ci
 * \brief Set the entity id from a long
 *
 * \param[inout] entity Entity whose ID is being set
 * \param[in] eid The entity id to set
 */
RTI_PRIVATE void
RTPS_Interface_int_to_entityid(RTPS_Entity_T *entity,RTI_UINT32 eid)
{
    eid = NETIO_htonl(eid);
    OSAPI_Memory_copy((void*)&entity->value,&eid,sizeof(RTI_UINT32));
}

/*ci
 * \brief
 * Writer sends new packet
 *
 * \details
 * Sends new DATA or liveliness HEARTBEAT.
 *
 * \param[in] netio_intf Self (writer) interface
 * \param[in] source Upstream NETIO interface calling this send
 * \param[in] dest Destination address
 * \param[in] packet Packet containing data to send
 *
 * \return RTI_TRUE on success, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_send(NETIO_Interface_T *netio_intf, /* RTPS self */
                    NETIO_Interface_T *source, /* upstream */
                    struct NETIO_Address *dest, /* RTPS peer */
                    NETIO_Packet_T *packet)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    RTI_BOOL ok = RTI_FALSE;
    union RTPS_MESSAGES *msg = NULL;
    RTPS_Entity_T reader_entity = RTPS_ENTITY_UNKNOWN;
    RTPS_Entity_T writer_entity = RTPS_ENTITY_UNKNOWN;
    RTI_SIZE_T payload_length, actual_payload_length;
    RTI_INT32 tail_adj;
    struct RTPS_Writer *writer = NULL;
    RTI_UINT8 data_flags = 0;
    RTPS_SendFlags_T send_flags = 0;
    struct NETIO_Address UNKNOWN_ADDR = NETIO_Address_INITIALIZER;
    struct RTPS_PeerEntry *peer = NULL;
    RTI_INT32 i;
    struct NETIO_Address *pkt_dst = NULL;
    RTI_INT32 peers_count = 0;
    RTI_BOOL is_first_write = RTI_FALSE;
    struct RTPS_HEADER_EXT *hdrext = NULL;
    RTI_BOOL is_data_last = RTI_FALSE;
    struct REDA_SequenceNumber sn_one = {0,1};

    UNUSED_ARG(dest);

    /* Check for bad parameters */
    if (netio_intf == NULL || source == NULL || packet == NULL ||
        packet->dests == NULL || NETIO_AddressSeq_get_length(packet->dests) == 0)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Only enabled interfaces may send */
    if (intf->_parent.state != NETIO_INTERFACESTATE_ENABLED)
    {
        RTPS_LOG_NOT_ENABLED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* RTPS reader don't currently send() */
    if (intf->property.mode != RTPS_INTERFACEMODE_WRITER)
    {
        RTPS_LOG_READER_UNSUPPORTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    writer = &intf->endpoint.writer;

    /* Cache the upstream (DDS) interface, as need to request samples from it
       upon processing ACKNACKs */
    if (writer->upstr_intf == NULL)
    {
        writer->upstr_intf = source;
    }
    else if (writer->upstr_intf != source)
    {
        RTPS_LOG_INTERFACE_MISMATCH(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    writer_entity = intf->_parent.local_address.value.guid.entity;

    /* Liveliness assertion sends only liveliness HEARTBEAT */
    if (packet->info.rtps_flags & NETIO_RTPS_FLAGS_LIVELINESS)
    {
        /* Liveliness sends to NULL packet dests, meaning all peers should
           receive the liveliness HB */
        if (!RTPS_Interface_initialize_packet(intf))
        {
            RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        intf->packet->info = packet->info;

        peers_count = REDA_Indexer_get_count(intf->peers_index);
        for (i = 0; i < peers_count; ++i)
        {
            peer = (struct RTPS_PeerEntry *)
                REDA_Indexer_get_entry(intf->peers_index, i);
            reader_entity = peer->addr.entity;

            if (!RTPS_Writer_direct_send(intf, RTPS_SEND_LIVE_HB_FLAG,
                intf->packet, peer,&writer_entity, &reader_entity, NULL, NULL))
            {
                RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
        /* Done sending liveliness, return */
        ok = RTI_TRUE;
        goto done;
    }

    is_first_write = REDA_SequenceNumber_is_zero(&writer->first_sn);

    /* update first, last seq numbers */
    REDA_SequenceNumber_max(&writer->first_sn,
                           &writer->first_sn, &packet->info.first_available_sn);


    /* With very first written sample, update starting point of send windows */
    if (is_first_write)
    {
        peers_count = REDA_Indexer_get_count(intf->peers_index);
        for (i = 0; i < peers_count; ++i)
        {
            peer = (struct RTPS_PeerEntry *)
                REDA_Indexer_get_entry(intf->peers_index, i);

            RTPS_Window_reset(&peer->info.reader.window,
                              intf->property.max_window_size, &writer->first_sn);

            peer->info.reader.last_acked_sn = writer->first_sn;
            if (!REDA_SequenceNumber_is_zero(&peer->info.reader.last_acked_sn))
            {
                REDA_SequenceNumber_minusminus(&peer->info.reader.last_acked_sn);
            }
        }

    }

    REDA_SequenceNumber_max(&writer->last_sn,
                            &writer->last_sn, &packet->info.sn);

    /* Create PDU
     *
     * Note that the PDU is built backwards, from tail to head:
     * RTPS_HEADER + INFO_TS + DATA/GAP
     */

    /* Built-in discovery writers direct messages to built-in discovery
     * readers
     */
    if (intf->_parent.local_address.value.rtps_guid.object_id ==
        (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION))
    {
        RTPS_Interface_int_to_entityid(&reader_entity,
                                       RTPS_OBJECT_ID_READER_SDP_PUBLICATION);
    }
    else if (intf->_parent.local_address.value.rtps_guid.object_id ==
        (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION))
    {
        RTPS_Interface_int_to_entityid(&reader_entity,
                                       RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION);
    }
    else if (intf->_parent.local_address.value.rtps_guid.object_id ==
        (RTI_UINT32)NETIO_htonl(RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT))
    {
        RTPS_Interface_int_to_entityid(&reader_entity,
                                       RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);
    }

    packet->source = intf->_parent.local_address;

    /* Get original payload length */
    payload_length = RTPS_Interface_get_data_size(
                                NETIO_Packet_get_payload_length(packet));
    actual_payload_length = payload_length;


    /* If the actual_payload_length exceeds an unsigned 16-bit integer, do not
     * add a piggyback HB and let the DATA submessage extend to the end of the
     * UDP payload.
     *
     * NOTE: This does not mean that the message can be sent by the
     *       downstream transport, _only_ that a DATA submessage > 64KB is
     *       correctly formatted.
     */
    if (payload_length > RTPS_MAX_DATA_LENGTH)
    {
        is_data_last = RTI_TRUE;
        /* Use RTPS_SUBMSG_HEADER_LEN because RTPS_Interface_set_data
         * subtracts it.
         */
        actual_payload_length = RTPS_SUBMSG_HEADER_LEN;
    }

#if RTPS_RELIABILITY
    /* Reliable writer appends piggyback heartbeat, unless the DATA submessage
     * exceeds an unsigned 16-bit integer
     */
    if (!is_data_last && (intf->endpoint.writer.flags & RTPS_WRITER_FLAG_RELIABLE) &&
        (intf->endpoint.writer.samples_per_hb > 0))
    {
        intf->endpoint.writer.piggyback_sample_count++;

        if (intf->endpoint.writer.piggyback_sample_count ==
            intf->endpoint.writer.samples_per_hb)
        {
            /* If a piggyback HB is added make sure the payload_length
             * is divisible by 4 so that the next hext header is aligned
             * properly.
             */
            actual_payload_length = (actual_payload_length + 3U) & ~0x3U;

            /* Make sure the current-tail is aligned to 4 */
            tail_adj = (RTI_INT32)(actual_payload_length - payload_length);

            if (!NETIO_Packet_set_tail(packet, tail_adj))
            {
                RTPS_LOG_NETIO_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            msg = (union RTPS_MESSAGES *)NETIO_Packet_get_tail(packet);
            if (msg == NULL)
            {
                RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            if (!NETIO_Packet_set_tail(packet, (sizeof(struct RTPS_HEARTBEAT))))
            {
                RTPS_LOG_NETIO_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            intf->endpoint.writer.hb_epoch++;

            RTPS_Interface_set_heartbeat(&msg->hb, RTPS_FLAGS_NONE,
                                         reader_entity, writer_entity,
                                         &intf->endpoint.writer.first_sn,
                                         &intf->endpoint.writer.last_sn,
                                         intf->endpoint.writer.hb_epoch);

            intf->endpoint.writer.piggyback_sample_count = 0;
        }
    }
#endif

    /* Only DATA submessage from this point on */
    send_flags = RTPS_SEND_DATA_FLAG;
    packet->info.protocol_id = NETIO_PROTOCOL_RTPS;

    /* DATA submessage, for inline qos and/or data */
    if (!NETIO_Packet_set_head(packet, 0 - (RTI_INT32)RTPS_Interface_get_data_size(0)))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if ((packet->info.rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS) != 0)
    {
        data_flags |= RTPS_DATAFLAGS_Q;
    }

    if (packet->info.valid_data)
    {
        data_flags |= RTPS_DATAFLAGS_D;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    if (msg == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    RTPS_Interface_set_data(&msg->data, data_flags, actual_payload_length,
                            RTPS_FLAGS_NONE, RTPS_DATA_INLINEQOS_OFFSET,
                            reader_entity, writer_entity, &packet->info.sn);

    /* [GAP]
     * A RTPS writer always sends a HB[1,0] if no samples have been written,
     * such as in response to a preemptive ACKNACK. However, if the first
     * SN written > 1, a reader will not process SN > 1 until a HB is received
     * indicating the available range. To avoid this situation, if the
     * first SN written > 1, add a GAP message to indicate that all SN
     * before the first SN is irrelevant.
     */
    if (is_first_write &&
        (REDA_SequenceNumber_compare(&writer->first_sn,&sn_one) > 0))
    {
        if (!RTPS_Writer_set_gap(packet,
                                 &sn_one,&intf->endpoint.writer.first_sn,
                                 &writer_entity,&reader_entity))
        {
            RTPS_LOG_SET_GAP(OSAPI_LOGKIND_ERROR,
                            sn_one.low,intf->endpoint.writer.first_sn.low,
                            writer_entity.value[0])
            goto done;
        }
    }

    /* INFO_TS */
    if (!NETIO_Packet_set_head(packet,0L-(RTI_INT32)(sizeof(struct RTPS_INFO_TS))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    if (msg == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    RTPS_Interface_set_info_ts(&msg->info_ts, &packet->info.timestamp);
    
    if (intf->checksum_index > RTPS_NO_CHECKSUM_INDEX)
    {
        /* This adds sufficient space for the header extension */
        if (!RTPS_Interface_add_header_extension(intf,packet))
        {
            goto done;
        }

        /* Save the location of the header extension as the entire RTPS
         * message must be built before it can be filled in.
         */
        hdrext = (struct RTPS_HEADER_EXT*)NETIO_Packet_get_head(packet);
    }

    /* RTPS Header */
    if (!NETIO_Packet_set_head(packet, 0L-(RTI_INT32)(sizeof(struct RTPS_Header))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
    if (msg == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    RTPS_Interface_set_message_header(&msg->header,
                                &intf->_parent.local_address.value.guid.prefix);

    if (hdrext != NULL)
    {
        /* Update the header extension if it was added */
        if (!RTPS_Interface_set_header_extension(intf,hdrext,packet))
        {
            goto done;
        }
    }

    for (i = 0; i < NETIO_AddressSeq_get_length(packet->dests); ++i)
    {
        pkt_dst = NETIO_AddressSeq_get_reference(packet->dests, i);

        if (NETIO_Address_compare(&UNKNOWN_ADDR, pkt_dst) == 0)
        {
            /* broadcast routes */
            peer = NULL;
        }
        else
        {
            /* peer routes */
            peer = (struct RTPS_PeerEntry *)
                REDA_Indexer_find_entry(intf->peers_index, &pkt_dst->value.guid);
            if (peer == NULL)
            {
                RTPS_LOG_LOOKUP_PEER(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }

        if (!RTPS_Sender_route_packet(intf, packet, peer, send_flags))
        {
            RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (intf->property.reliable && (REDA_Indexer_get_count(intf->peers_index) == 0))
    {
        RTI_BOOL brc;

        brc = NETIO_Interface_acknack(writer->upstr_intf,
                                      NULL,
                                      &packet->info.sn,
                                      RTI_FALSE);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            RTPS_LOG_ACK(OSAPI_LOGKIND_ERROR);
        }
#else
        IGNORE_RETVAL(brc);
#endif
    }

    ok = RTI_TRUE;

done:

    return ok;
}

/*ci
 * \brief
 * Unsupported NETIO interface operation acknack
 *
 * \param [in] netio_intf Unused
 * \param [in] source Unused
 * \param [in] packet_id Unused
 * \param [in] nack Unused
 *
 * \return RTI_FALSE always, because is unsupported
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_acknack(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *source,
                      NETIO_PacketId_T *packet_id,
                      RTI_BOOL nack)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(packet_id);
    UNUSED_ARG(nack);

    /* RTPS samples do not require acknowledgement */
    RTPS_LOG_FUNC_UNSUPPORTED(OSAPI_LOGKIND_ERROR)
    return RTI_FALSE;
}

/*ci
 * \brief
 * Unsupported NETIO interface operation request
 *
 * \param [in] netio_intf Unused
 * \param [in] source Unused
 * \param [in] dest Unused
 * \param [in] packet Unused
 * \param [in] packet_id Unused
 * \param [inout] next_packet_id Unused
 *
 * \return RTI_FALSE always, because is unsupported
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_request(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *source,
                      struct NETIO_Address *dest,
                      NETIO_Packet_T **packet,
                      NETIO_PacketId_T *packet_id,
                      NETIO_PacketId_T *next_packet_id)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(dest);
    UNUSED_ARG(packet);
    UNUSED_ARG(packet_id);
    UNUSED_ARG(next_packet_id);

    /* RTPS does not respond to requests */
    RTPS_LOG_FUNC_UNSUPPORTED(OSAPI_LOGKIND_ERROR)
    return RTI_FALSE;
}

/*ci
 * \brief
 * Unsupported NETIO interface operation return loan
 *
 * \param [in] netio_intf Unused
 * \param [in] source Unused
 * \param [in] packet Unused
 * \param [in] packet_id Unused
 *
 * \return RTI_FALSE always, because is unsupported
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_return_loan(NETIO_Interface_T *netio_intf,
                          struct NETIO_Address *source,
                          NETIO_Packet_T *packet,
                          NETIO_PacketId_T *packet_id)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(packet);
    UNUSED_ARG(packet_id);

    /* RTPS does not loan samples */
    RTPS_LOG_FUNC_UNSUPPORTED(OSAPI_LOGKIND_ERROR)
    return RTI_FALSE;
}


/*ci
 * \brief
 * Remove a sample from the writer's send window
 *
 * \param[in] netio_intf Self (writer) interface
 * \param[in] destination Unused
 * \param[in] packet_id Sequence number of sample to remove
 *
 * \return RTI_TRUE on successful window update or noop, otherwise RTI_FALSE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_xmit_remove(NETIO_Interface_T *netio_intf,
                          struct NETIO_Address *destination,
                          NETIO_PacketId_T *packet_id)
{
    RTI_BOOL ok = RTI_FALSE;
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    RTI_INT32 i = 0;
    RTPS_SampleId_T first_unacked_sn;
    RTI_INT32 peers_count = 0;
    struct RTPS_PeerEntry *peer = NULL;
    UNUSED_ARG(destination);

    if (packet_id == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Remove sample only for Writer, and only if SN is within valid range */
    if (intf->property.mode == RTPS_INTERFACEMODE_WRITER &&
        (REDA_SequenceNumber_compare(packet_id,
                                     &intf->endpoint.writer.first_sn) >= 0))
    {
        /* Update last_acked_sn for Readers and Writer */
        peers_count = REDA_Indexer_get_count(intf->peers_index);

        for (i = 0; i < peers_count; ++i)
        {
            peer = (struct RTPS_PeerEntry *)
                REDA_Indexer_get_entry(intf->peers_index, i);

            RTPS_Window_remove(&peer->info.reader.window, NULL, packet_id);

            /* Advance peer reader's last_acked_sn if discarded SN is
               next (first unacknowledged) SN */
            first_unacked_sn = peer->info.reader.last_acked_sn;
            REDA_SequenceNumber_plusplus(&first_unacked_sn);

            if (REDA_SequenceNumber_compare(packet_id, &first_unacked_sn) == 0)
            {
                /* Advance window to new unacked SN */
                REDA_SequenceNumber_plusplus(&first_unacked_sn);
                if (!RTPS_Window_advance(&peer->info.reader.window,
                                         &first_unacked_sn))
                {
                    RTPS_LOG_WINDOW_ADVANCE(OSAPI_LOGKIND_ERROR)
                    goto done;
                }
                REDA_SequenceNumber_plusplus(&peer->info.reader.last_acked_sn);
            }
        }
    }

    ok = RTI_TRUE;
done:

    return ok;
}

/*ci
 * \brief
 * Assert a peer RTPS writer or reader
 *
 * \details
 * A peer is a remote writer or reader with which the self reader or writer
 * communicates.  This function asserts the peer entry that keeps state of
 * a peer, such as send and receive windows.
 *
 * \param[in] intf Self interface
 * \param[out] existed Flag set true if peer entry already exists.
 * \param[in] peer_addr Address of peer
 * \param[in] property Input property upon creating route to peer
 *
 * \return On success, pointer to asserted peer entry.  Otherwise, NULL.
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RTPS_PeerEntry*
RTPS_Interface_assert_peer(struct RTPS_Interface *intf,
                           RTI_BOOL *existed,
                           struct NETIO_Address *peer_addr,
                           struct RTPS_RouteProperty *property)
{
    struct RTPS_PeerEntry *peer = NULL;
    RTPS_SampleId_T SN_ONE = {0,1};

    /* Precondition: existed != NULL */
    *existed = RTI_FALSE;
    peer = (struct RTPS_PeerEntry*)
                         REDA_Indexer_find_entry(intf->peers_index,
                                                 &peer_addr->value.guid);
    if (peer != NULL)
    {
        /* Found existing peer */
        *existed = RTI_TRUE;
        return peer;
    }

    peer = (struct RTPS_PeerEntry*)REDA_BufferPool_get_buffer(intf->peers_pool);
    if (peer == NULL)
    {
        RTPS_LOG_GET_PEER_BUFFER(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    /* Begin initializing new peer */
    peer->addr = peer_addr->value.guid;

    if (!REDA_Indexer_add_entry(intf->peers_index, peer))
    {
        RTPS_LOG_INDEX_ADD_ENTRY(OSAPI_LOGKIND_ERROR)
        REDA_BufferPool_return_buffer(intf->peers_pool, peer);
        return NULL;
    }

    /* Writer initializes Remote Reader peer */
    if (intf->property.mode == RTPS_INTERFACEMODE_WRITER)
    {
        struct RTPS_Writer *writer = &intf->endpoint.writer;
        struct RTPS_RemoteReader *reader = &peer->info.reader;

        reader->reliable = (property == NULL ? RTI_FALSE : property->reliable);

#if RTPS_RELIABILITY
        reader->is_inactive = RTI_FALSE;
        reader->inactive_count = intf->property.max_hb_retries;
        reader->epoch = 0;
        reader->repair_stuck_nack = RTI_TRUE;

        if (property == NULL)
        {
            reader->last_acked_sn = writer->last_sn;
        }
        else if (REDA_SequenceNumber_is_unknown(&property->last_acked_sn))
        {
            REDA_SequenceNumber_set_zero(&reader->last_acked_sn);
        }
        else
        {
            reader->last_acked_sn = property->last_acked_sn;
        }

        /* Save these values so the receive window can be reset later */
        reader->initial_last_acked_sn = reader->last_acked_sn;
        reader->window.max_size = 0;

        /* Initialize send window for remote reader */
        if (intf->property.max_window_size > 0)
        {
            RTPS_SampleId_T bitmaplead = reader->last_acked_sn;
            REDA_SequenceNumber_plusplus(&bitmaplead);
            RTPS_Window_reset(&reader->window,
                              intf->property.max_window_size, &bitmaplead);
        }

        if (intf->property.reliable && reader->reliable)
        {
            ++writer->active_reliable_reader_count;
        }
#endif
    }
    /* Reader initializes Remote Writer peer */
    else if (intf->property.mode == RTPS_INTERFACEMODE_READER)
    {
        struct RTPS_RemoteWriter *writer = &peer->info.writer;

        writer->reliable = (property == NULL ? RTI_FALSE : property->reliable);
        writer->rcvd_first_hb = RTI_FALSE;
#if RTPS_DATA_BATCH_ENABLED
        writer->batch_writer = RTI_FALSE;
#endif
        RTPS_Bitmap_reset(&writer->bitmap, &SN_ONE,
                            (RTI_INT32)intf->property.max_window_size);

        writer->first_unaccepted_virtual_sn = SN_ONE;

#if RTPS_RELIABILITY
        writer->epoch = 0;
        REDA_SequenceNumber_set_zero(&writer->highest_accepted_sn);
        REDA_SequenceNumber_set_zero(&writer->highest_accepted_vsn);
        writer->reserved_count = 0;
#endif
    }

    return peer;
}

/*ci
 * \brief
 * Reader sends to a writer a packet with an ACKNACK submessage
 *
 * \param[in] intf Self (reader) interface
 * \param[in] bitmap ACKNACK bitmap to send
 * \param[in] reader Self reader's endpoint state
 * \param[in] peer_entry Destination writer's peer entry
 * \param[in] final_ack Flag set RTI_TRUE when ACKNACK to send is fully
 * acknowledged.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Reader_send_acknack(struct RTPS_Interface *intf,
                         struct RTPS_Bitmap *bitmap,
                         struct RTPS_Reader *reader,
                         struct RTPS_PeerEntry *peer_entry,
                         RTI_BOOL final_ack)
{
    RTI_SIZE_T payload_length;
    union RTPS_MESSAGES *msg = NULL;
    struct RTPS_HEADER_EXT *hdrext = NULL;

    payload_length = RTPS_Interface_get_acknack_size(bitmap->bit_count);

    if (!RTPS_Interface_initialize_packet(intf))
    {
        RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!NETIO_Packet_set_head(intf->packet, 0 - (RTI_INT32)payload_length))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    msg = (union RTPS_MESSAGES*)NETIO_Packet_get_head(intf->packet);
    reader->ack_epoch++;
    RTPS_Interface_set_acknack(
       &msg->acknack, (RTI_UINT8)(final_ack ? RTPS_ACKNACKFLAGS_F : 0),
       payload_length,
       intf->_parent.local_address.value.guid.entity,
       peer_entry->addr.entity,
       bitmap, reader->ack_epoch);

    /* INFO_DST */
    if (!NETIO_Packet_set_head(intf->packet, 0L-(RTI_INT32)(sizeof(struct RTPS_INFO_DST))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(intf->packet);
    RTPS_Interface_set_info_dst(&msg->info_dst,
                                &peer_entry->addr.prefix);

    if (intf->checksum_index > RTPS_NO_CHECKSUM_INDEX)
    {
        /* This adds sufficient space for the header extension */
        if (!RTPS_Interface_add_header_extension(intf,intf->packet))
        {
            return RTI_FALSE;
        }

        /* Save the location of the header extension as the entire RTPS
         * message must be built before it can be filled in.
         */
        hdrext = (struct RTPS_HEADER_EXT*)NETIO_Packet_get_head(intf->packet);
    }

    /* RTPS Header */
    if (!NETIO_Packet_set_head(intf->packet, 0L-(RTI_INT32)(sizeof(struct RTPS_Header))))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(intf->packet);
    RTPS_Interface_set_message_header(&msg->header,
                                &intf->_parent.local_address.value.guid.prefix);

    if (hdrext != NULL)
    {
        /* Update the header extension if it was added */
        if (!RTPS_Interface_set_header_extension(intf,hdrext,intf->packet))
        {
            return RTI_FALSE;
        }
    }

    if (!RTPS_Sender_route_packet(intf, intf->packet,peer_entry, 0))
    {
        RTPS_LOG_ROUTE_PACKET(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Return a priority value of a route given its address
 *
 * \details
 * Routes can be prioritized when sending packets.  This function assigns a
 * priority based on the route's address.
 *
 * \param[in] via_addr Route address
 * \param[in] group TRUE if this is a group, FALSE otherwise.
 *
 * \return Priority value for the route.  Valid values are
 * RTPS_ROUTE_PRIORITY_UNICAST or RTPS_ROUTE_PRIORITY_MULTICAST.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTPS_RoutePriority_T
RTPS_Interface_compute_route_priority(struct NETIO_Address *via_addr,
                                      RTI_BOOL group)
{
    RTPS_RoutePriority_T result;

    if (group)
    {
        if (NETIO_Address_is_multicast(via_addr))
        {
            result = RTPS_ROUTE_PRIORITY_MULTICAST;
        }
        else
        {
            result = RTPS_ROUTE_PRIORITY_UNICAST;
        }
    }
    else
    {
        if (!NETIO_Address_is_multicast(via_addr))
        {
            result = RTPS_ROUTE_PRIORITY_MULTICAST;
        }
        else
        {
            result = RTPS_ROUTE_PRIORITY_UNICAST;
        }
    }

    return result;
}

/*ci
 * \brief When the route table changes based on addition/removal of a peer,
 *        call this function to select the preferred routes.
 *
 * \param[in] intf RTPS interface
 * \param[in] peer_entry The peer to select routes for
 */
RTI_PRIVATE void
RTPS_Interface_select_reader_routes(struct RTPS_Interface *intf,
                                    struct RTPS_PeerEntry *peer_entry)
{
    RTI_UINT8 min_pri = 1;
    RTI_UINT8 max_pri = 255;
    RTI_UINT32 max_port = 0xffffffff;
    struct RTPS_RouteEntry route_key_low;
    struct RTPS_RouteEntry route_key_high;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct RTPS_RouteEntry *route_entry = NULL;
    struct RTPS_RouteEntry *last_entry = NULL;
    struct NETIO_Guid guid_max = {{{0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff}},
                                  {{0xff,0xff,0xff,0xff}}};

    route_key_low.destination = peer_entry->addr;
    NETIO_Address_init(&route_key_low.intf_address,0);

    /* 255 is logically lower than 1 when comparing priorities */
    route_key_low.group_priority = max_pri;

    /* 1 is highest priority, appears first in the index */
    /* Not all fields in route_key_low are used in a comparison. Thus,
     * the Coverity event [uninit_use] is marked as 'Intentional'.
     */
    /* coverity[uninit_use] */
    route_key_high = route_key_low;
    route_key_high.group_priority = min_pri;

    NETIO_Address_set_guid(&route_key_high.intf_address,max_port,&guid_max);

    dbrc = DB_Table_select_range(intf->_parent._rtable,intf->group_route,
                                 &cursor,&route_key_low,&route_key_high);

    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)(&route_entry));
    while (dbrc == DB_RETCODE_OK)
    {
        /* If a lower priority is found, make sure it is unselected in case
         * it was linked before.
         */
        if ((last_entry != NULL) &&
            (last_entry->group_priority != route_entry->group_priority))
        {
            route_entry->selected_group_route = RTI_FALSE;
        }
        else
        {
            /* The first entry is always part of the selected routes */
            last_entry = route_entry;
            route_entry->selected_group_route = RTI_TRUE;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)(&route_entry));
    }

    DB_Cursor_finish(intf->_parent._rtable,cursor);

    DB_Table_reindex_index(intf->_parent._rtable,
                           intf->selected_group_route_index);
}

/*ci
 * \brief
 * Adds a route to send packet from source to peer via a transport
 *
 * \details
 * Implementation of NETIO add_route operation.
 * Adds a route to an RTPS peer (remote writer or remote reader) via a
 * specific downstream transport interface and address.
 *
 *
 * \param[in] src_intf  Self
 * \param[in] dst_addr  RTPS peer GUID
 * \param[in] via_intf  Downstream transport interface
 * \param[in] via_addr  Downstream transport address
 * \param[in] property  Route property. May be NULL.
 * \param[out] existed  Flag of whether route already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_add_route(NETIO_Interface_T *src_intf, /* RTPS self */
                        struct NETIO_Address *dst_addr, /* RTPS peer */
                        NETIO_Interface_T *via_intf, /* xport intf */
                        struct NETIO_Address *via_addr, /* xport addr (uni/multicast) */
                        struct NETIORouteProperty *property,
                        RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)src_intf;
    struct RTPS_Interface *ext_intf = NULL;
    struct RTPS_PeerEntry *peer_entry = NULL;
    struct RTPS_RouteEntry *route_entry = NULL;
    struct RTPS_RouteProperty *route_property = NULL;
    struct RTPS_RouteEntry route_key;
    struct RTPS_ExtBindEntry *bind_entry = NULL;
    RTI_BOOL is_reliable_peer = RTI_FALSE;
    DB_ReturnCode_T db_rc;
    struct RTPS_ExtBindEntry bind_key;
    RTI_BOOL existing_peer;
    RTI_BOOL bretval;

    /* bad param check */
    if ((src_intf == NULL) || (dst_addr == NULL) ||
        (via_intf == NULL) || (via_addr == NULL))
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("add route",RTI_FALSE)
    OSAPI_TRACE_INT32("intf.port",intf->_parent.local_address.port,RTI_FALSE)
    OSAPI_TRACE_GUID("intf.address",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst_addr.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst_addr.address",&dst_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("via_addr.port",via_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("via_addr.address",&via_addr->value.rtps_guid,RTI_TRUE)

    /* A reliable peer must provide a valid route property */
    if (property != NULL)
    {
        route_property = (struct RTPS_RouteProperty*)property;
        is_reliable_peer = route_property->reliable;
    }
    else
    {
        /* If self is reliable reader, this route being added must be to a
         * reliable writer
         */
        if (intf->property.mode == RTPS_INTERFACEMODE_READER)
        {
            is_reliable_peer = RTI_TRUE;
        }
    }

    /* Must have a peer entry to which to route */
    peer_entry = RTPS_Interface_assert_peer(intf, &existing_peer, dst_addr,
                                            route_property);
    if (peer_entry == NULL)
    {
        RTPS_LOG_ASSERT_PEER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* check for existing record */
    route_key.destination = dst_addr->value.guid; /* RTPS peer */
    route_key.intf = via_intf; /* xport intf */
    route_key.intf_address = *via_addr; /* xport addr */

    db_rc = DB_Table_select_match(intf->_parent._rtable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&route_entry,
                                 (DB_Key_T)&route_key);

    /* Need to update existed out-param before returning */
    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if ((db_rc != DB_RETCODE_OK) && (db_rc != DB_RETCODE_NO_DATA))
    {
        RTPS_LOG_DB_SELECT_MATCH(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    if (db_rc == DB_RETCODE_OK)
    {
        return RTI_TRUE;
    }

    /* Assert route entry */
    db_rc = DB_Table_create_record(intf->_parent._rtable,
                                  (DB_Record_T *)&route_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_CREATE_ROUTE_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    /* Initialize route entry */
    route_entry->intf = via_intf;
    route_entry->intf_address = *via_addr;
    route_entry->destination = dst_addr->value.guid;

    /* Compute and set route priority.
       Current priority order: multicast > unicast.
    */
    route_entry->group_priority = RTPS_Interface_compute_route_priority(
                                                            via_addr,RTI_TRUE);

    route_entry->direct_priority = RTPS_Interface_compute_route_priority(
                                                            via_addr,RTI_FALSE);

    route_entry->peer_ref = peer_entry;

    OSAPI_TRACE_NET("STATE[first,last]",RTI_FALSE)
    OSAPI_TRACE_INT32("first.high",intf->endpoint.writer.first_sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("first.low",intf->endpoint.writer.first_sn.low,RTI_FALSE)
    OSAPI_TRACE_INT32("last.high",intf->endpoint.writer.last_sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("last.low",intf->endpoint.writer.last_sn.low,RTI_TRUE)

    /* Given this is a writer, upstream queue can update sequencen numbers
       of its available samples */
    if ((intf->property.mode == RTPS_INTERFACEMODE_WRITER) &&
        (route_property != NULL))
    {
        intf->endpoint.writer.first_sn = route_property->first_sn;
        intf->endpoint.writer.last_sn = route_property->last_sn;
    }

    db_rc = DB_Table_insert_record(intf->_parent._rtable,
                                   (DB_Record_T)route_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_INSERT_ROUTE_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        (void)DB_Table_delete_record(intf->_parent._rtable,
                                     (DB_Record_T)route_entry);
        return RTI_FALSE;
    }

    /* A new route entry has been added for the reader, determine the
     * routes to use for this reader
     */
    RTPS_Interface_select_reader_routes(intf,peer_entry);

    /* After route entry was successfully inserted, check that its peer entry
     * is the same one referenced by the bind entry keyed on the peer's
     * address.
     *
     * First, get the matching external interface.  It is the one keyed with
     * the GUID Prefix portion of this interface's GUID address.
     */
    ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
                                        intf->factory->ext_intf_index,
                                        &intf->_parent.local_address);
    if (ext_intf == NULL)
    {
        RTPS_LOG_FIND_EXTERNAL_INTERFACE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* For non-anonymous interfaces, check in the external interface's bind
     * table for an existing bind entry keyed on this interface's GUID and
     * the peer's address.
     */
    if (!intf->property.anonymous)
    {
        /* Note that bind key's dst and src are reverse of route key */
        bind_key.source = dst_addr->value.guid; /* RTPS peer address */
        bind_key.destination = intf->_parent.local_address.value.guid.entity;

        db_rc = DB_Table_select_match(ext_intf->_parent._btable,
                                      DB_TABLE_DEFAULT_INDEX,
                                      (DB_Record_T *)&bind_entry,
                                      &bind_key);

        if ((db_rc != DB_RETCODE_OK) && (db_rc != DB_RETCODE_NO_DATA))
        {
            RTPS_LOG_DB_SELECT_MATCH(OSAPI_LOGKIND_ERROR, db_rc)
            return RTI_FALSE;
        }

        if (db_rc == DB_RETCODE_OK)
        {
            /* Found a matching bind entry whose peer reference is NULL,
             * because bind() for the peer was called before this add_route().
             * Update bind entry's peer reference to this peer entry.
             */
            if (bind_entry->peer_ref == NULL)
            {
                bind_entry->peer_ref = peer_entry;
            }
        }
    }

#if RTPS_RELIABILITY
    /* Send preemptive HEARTBEAT or ACKNACK between reliable interfaces.
     * "Preemptive" in this case means before a sample has been sent between
     * peers.
     * A preemptive HEARTBEAT from writer to reader expedites the
     * reader's first ACKNACK and jumpstarts its request for any unreceived
     * samples.
     * A preremptive ACKNACK from reader to writer will have zero values for
     * its lead SN and bitcount, and it jumpstarts the writer to send the reader
     * a valid HEARTBEAT.
     */

    /* Stop if don't need to send preemptive HB or ACKNACK */
    if (existing_peer || !intf->property.reliable || !is_reliable_peer)
    {
        return RTI_TRUE;
    }

    /* Writer requests history and/or sends preemptive HEARTBEAT */
    if (intf->property.mode == RTPS_INTERFACEMODE_WRITER)
    {
        bretval = RTPS_Writer_request_history(intf, peer_entry);
#if OSAPI_ENABLE_LOG
        /* Don't return failure on failed preemptive HEARTBEAT */
        if (!bretval)
        {
            RTPS_LOG_WRITER_REQUEST_SEND_HISTORY(OSAPI_LOGKIND_ERROR)
        }
#else
        /* Don't return failure on failed preemptive HEARTBEAT */
        IGNORE_RETVAL(bretval);
#endif
    }
    else /* Reader sends preemptive ACKNACK */
    {
        struct RTPS_Bitmap bitmap;
        RTPS_SampleId_T SN_ZERO = REDA_SEQUENCE_NUMBER_ZERO;

        RTPS_Bitmap_reset(&bitmap, &SN_ZERO, 0);

        bretval = RTPS_Reader_send_acknack(intf, &bitmap,
                                &intf->endpoint.reader,peer_entry, RTI_FALSE);
#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            RTPS_LOG_READER_SEND_ACKNACK(OSAPI_LOGKIND_ERROR)
            /* Don't return failure on failed preemptive ACKNACK */
        }
#else
        /* Don't return failure on failed preemptive HEARTBEAT */
        IGNORE_RETVAL(bretval);
#endif

        if ((intf->endpoint.reader.flags &
                RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE) &&
            !OSAPI_Timer_update_timeout(intf->property._parent._parent.timer,
                &intf->endpoint.reader.acknack_event,
                intf->endpoint.reader.acknack_sec,
                intf->endpoint.reader.acknack_nanosec))
        {
            RTPS_LOG_CREATE_HB_EVENT(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

#endif /* RTPS_RELIABILITY */
    return RTI_TRUE;
}

/*ci
 * \brief
 * Clear a remote readers send window
 *
 * \details
 * This function calls the upstream interface with any ack'ed/nack'ed
 * samples in the send window when the remote reader is removed. It is
 * important that samples are not accounted for more than once.
 *
 * \param[in] writer   RTPS writer
 * \param[in] reader   The peer reader to clear the window for
 *
 */
RTI_PRIVATE void
RTPS_Interface_clear_window(struct RTPS_Writer *writer,
                            struct RTPS_RemoteReader *reader)
{
    RTPS_SampleId_T reader_first_unacked_sn;
    RTI_BOOL bit_value;
    RTI_BOOL brc;

    if (!(writer->flags & RTPS_WRITER_FLAG_RELIABLE))
    {
        return;
    }

    if (writer->upstr_intf == NULL)
    {
        return;
    }

    /* Go through all the SNs written by the writer. The first assumption is
     * that all the samples between [first,last] has been written by the
     * writer and either an ACK or NACK is expected. The second assumption is
     * that a SN is either ACKed or NACKed at most one time.
     */
    reader_first_unacked_sn = reader->initial_last_acked_sn;
    REDA_SequenceNumber_plusplus(&reader_first_unacked_sn);

    if (REDA_SequenceNumber_compare(&reader_first_unacked_sn,&writer->first_sn) < 0)
    {
        reader_first_unacked_sn = writer->first_sn;
    }

    while (REDA_SequenceNumber_compare(&reader_first_unacked_sn,&writer->last_sn) <= 0)
    {
        if (!RTPS_Bitmap_get_bit(&reader->window.bitmap,
                                 &bit_value,
                                 &reader_first_unacked_sn))
        {
            /* This SN does not exist in the readers window. The reader's
             * window may have been full and thus not added to the reader's
             * window. But upstream does not know that, thus NACK it.
             */
            bit_value = RTI_TRUE;
        }

        /* If the sample didn't exist or is already NACK'ed bit_value will
         * be 1 here. If bit_value is 0, it was in the window and has been
         * ACK'ed.
         */
        brc = NETIO_Interface_acknack(writer->upstr_intf,
                                      NULL,
                                      &reader_first_unacked_sn,
                                      bit_value);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            RTPS_LOG_ACK(OSAPI_LOGKIND_ERROR);
        }
#else
        IGNORE_RETVAL(brc);
#endif

        REDA_SequenceNumber_plusplus(&reader_first_unacked_sn);
    }
}

/*ci
 * \brief
 * Deletes a previously created route
 *
 * \details
 * Implementation of NETIO delete_route operation.
 *
 * \param[in] netio_intf  Self
 * \param[in] dst_addr  RTPS peer GUID
 * \param[in] via_intf  Downstream transport interface
 * \param[in] via_addr  Downstream transport address
 * \param[out] existed  Flag of whether route already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_delete_route(NETIO_Interface_T *netio_intf, /* RTPS self */
                          struct NETIO_Address *dst_addr, /* RTPS peer */
                          NETIO_Interface_T *via_intf, /* xport intf */
                          struct NETIO_Address *via_addr, /* xport addr */
                          RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_RouteEntry *route_entry = NULL;
    struct RTPS_RouteEntry key;
    DB_ReturnCode_T db_rc;

    if (netio_intf == NULL || dst_addr == NULL ||
        via_intf == NULL || via_addr == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    key.destination = dst_addr->value.guid;
    key.intf = via_intf;
    key.intf_address = *via_addr;

    db_rc = DB_Table_remove_record(intf->_parent._rtable,
                                  (DB_Record_T *)&route_entry,
                                  &key);
    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if (db_rc != DB_RETCODE_OK)
    {
        if (db_rc == DB_RETCODE_NO_DATA)
        {
            RTPS_LOG_NONEXISTENT_ROUTE(OSAPI_LOGKIND_WARNING)
            return RTI_TRUE;
        }
        RTPS_LOG_DB_REMOVE_ROUTE_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    /* A route entry has been deleted for the reader, updated the selected
     * routes.
     */
    RTPS_Interface_select_reader_routes(intf,route_entry->peer_ref);

    /* If peer existed for this route entry, return its resources */
    if (!intf->property.anonymous &&
        !((intf->property.mode == RTPS_INTERFACEMODE_READER) &&
          !intf->property.reliable))
    {
        if  (intf->property.mode == RTPS_INTERFACEMODE_WRITER)
        {
            RTPS_Interface_clear_window(&intf->endpoint.writer,
                                        &route_entry->peer_ref->info.reader);
        }

        if (REDA_Indexer_remove_entry(intf->peers_index, &dst_addr->value.guid) != NULL)
        {
            REDA_BufferPool_return_buffer(intf->peers_pool,
                                          route_entry->peer_ref);
        }
    }

    route_entry->peer_ref = NULL;

    db_rc = DB_Table_delete_record(intf->_parent._rtable, route_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_DELETE_ROUTE_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Binds an RTPS interface to an RTPS external interface to enable its reception
 * of packets.
 *
 * \details
 * Implementation of NETIO bind operation.
 * An RTPS external interface corresponding to this writer/reader interface
 * receives packets from downstream.  It passes each packet to RTPS interfaces
 * asserted in its bind table.  This operation asserts those entries in the
 * external interface's bind table.
 *
 * \param[in] netio_intf  Self interface
 * \param[in] src_addr  RTPS peer address/GUID
 * \param[in] property NETIO bind property
 * \param[out] existed  Flag of whether bind entry already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_bind(NETIO_Interface_T *netio_intf, /* RTPS self */
                    struct NETIO_Address *src_addr, /* RTPS peer */
                    struct NETIOBindProperty *property,
                    RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_Interface *ext_intf = NULL;
    struct RTPS_RouteEntry *route_entry = NULL;
    struct RTPS_ExtBindEntry *bind_entry = NULL;
    struct RTPS_PeerEntry *peer_entry = NULL;
    DB_ReturnCode_T db_rc;
    struct RTPS_ExtBindEntry bind_key;
    UNUSED_ARG(property);

    /* bad param check */
    if (netio_intf == NULL || src_addr == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("bind:",RTI_FALSE)
    OSAPI_TRACE_INT32("source.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("source.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("intf.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("intf.address",&src_addr->value.rtps_guid,RTI_TRUE)

    /* Only non-anonymous interfaces maintain peers */
    if (!intf->property.anonymous)
    {
        db_rc = DB_Table_select_match(intf->_parent._rtable,
                                     intf->rtable_peer_index,
                                     (DB_Record_T *)&route_entry,
                                     (DB_Key_T)&src_addr->value.guid);

        if ((db_rc != DB_RETCODE_OK) && (db_rc != DB_RETCODE_NO_DATA))
        {
            RTPS_LOG_DB_SELECT_MATCH(OSAPI_LOGKIND_ERROR, db_rc)
            return RTI_FALSE;
        }

        /* Found preexisting peer */
        if (db_rc == DB_RETCODE_OK)
        {
            peer_entry = route_entry->peer_ref;
        }
    }

    bind_key.source = src_addr->value.guid; /* peer addr */
    bind_key.destination = intf->_parent.local_address.value.guid.entity; /* intf addr */

    /* Get the external interface that corresponds to this interface and assert
     * a bind entry.
     *
     * Note, an RTPS interface has a GUID as its address, where the GUID Prefix
     * portion identifies the DDS DomainParticipant that created its
     * DDS DataWriter or DataReader.  The external interface index is keyed on
     * the GUID Prefix.  So, the corresponding external interface is the one
     * with the same GUID Prefix.
     */
    ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
       intf->factory->ext_intf_index, &intf->_parent.local_address);
    if (ext_intf == NULL)
    {
        RTPS_LOG_FIND_EXTERNAL_INTERFACE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    db_rc = DB_Table_select_match(
       ext_intf->_parent._btable, DB_TABLE_DEFAULT_INDEX,
       (DB_Record_T *)&bind_entry, (DB_Key_T)&bind_key);

    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if ((db_rc != DB_RETCODE_OK) && (db_rc != DB_RETCODE_NO_DATA))
    {
        RTPS_LOG_DB_SELECT_MATCH(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    if (db_rc == DB_RETCODE_OK)
    {
        return RTI_TRUE;
    }

    db_rc = DB_Table_create_record(ext_intf->_parent._btable,
                                  (DB_Record_T *)&bind_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_CREATE_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(bind_entry,sizeof(struct RTPS_ExtBindEntry));
    bind_entry->source = bind_key.source;
    bind_entry->destination = bind_key.destination;
    bind_entry->_rtps_intf = intf;
    bind_entry->peer_ref = peer_entry; /* NULL peer for anon endpoint */

    db_rc = DB_Table_insert_record(ext_intf->_parent._btable,
                                   (DB_Record_T)bind_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_INSERT_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        (void)DB_Table_delete_record(ext_intf->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Unbinds an RTPS interface from an RTPS external interface
 *
 * \param[in] netio_intf  Self interface
 * \param[in] src_addr  RTPS peer address/GUID
 * \param[in] dst_intf Self interface
 * \param[out] existed  Flag of whether bind entry already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_unbind(NETIO_Interface_T *netio_intf, /* RTPS self */
                    struct NETIO_Address *src_addr, /* RTPS peer */
                    NETIO_Interface_T *dst_intf, /* RTPS self */
                    RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_Interface *ext_intf = NULL;
    struct RTPS_ExtBindEntry *bind_entry = NULL;
    DB_ReturnCode_T db_rc;
    struct RTPS_ExtBindEntry bind_key;
    UNUSED_ARG(dst_intf);

    /* bad param check */
    if (netio_intf == NULL || src_addr == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    bind_key.source = src_addr->value.guid;
    bind_key.destination = intf->_parent.local_address.value.guid.entity;

    ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
       intf->factory->ext_intf_index, &intf->_parent.local_address);
    if (ext_intf == NULL)
    {
        RTPS_LOG_FIND_EXTERNAL_INTERFACE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    db_rc = DB_Table_remove_record(ext_intf->_parent._btable,
                                   (DB_Record_T *)&bind_entry, &bind_key);
    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if (db_rc != DB_RETCODE_OK)
    {
        if (db_rc == DB_RETCODE_NO_DATA)
        {
            RTPS_LOG_NONEXISTENT_BIND(OSAPI_LOGKIND_WARNING)
            return RTI_TRUE;
        }
        RTPS_LOG_DB_REMOVE_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    if ((intf->property.mode == RTPS_INTERFACEMODE_WRITER) &&
        (bind_entry->peer_ref != NULL))
    {
        RTPS_Interface_clear_window(&intf->endpoint.writer,
                                    &bind_entry->peer_ref->info.reader);
    }

    /* For best-effort reader, decrease peer's refCount */
    if (!intf->property.anonymous &&
        (intf->property.mode == RTPS_INTERFACEMODE_READER) &&
        !intf->property.reliable &&
        (bind_entry->peer_ref != NULL))
    {
        if (REDA_Indexer_remove_entry(intf->peers_index,
                                  &bind_entry->peer_ref->addr) != NULL)
        {
            REDA_BufferPool_return_buffer(intf->peers_pool,
                                          bind_entry->peer_ref);
        }
    }

    bind_entry->_rtps_intf = NULL;
    bind_entry->peer_ref = NULL;

    if (DB_Table_delete_record(ext_intf->_parent._btable,
                               (DB_Record_T)bind_entry) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Binds RTPS interface with an upstream interface
 *
 * \details
 * Implementation of NETIO interface's bind_external operation.
 * Adds bind entry to local bind table, enabling interface to pass received
 * samples upstream or request samples from upstream.
 *
 * \param[in] src_intf Self RTPS interface
 * \param[in] src_addr Self address
 * \param[in] dst_intf Upstream interface
 * \param[in] dst_addr Upstream interface's address
 * \param[in] property Bind property
 * \param[out] existed Flag of whether bind entry for this and upstream interface
 *                     already existed.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_bind_external(NETIO_Interface_T *src_intf, /* intf intf */
                             struct NETIO_Address *src_addr, /* peer addr */
                             NETIO_Interface_T *dst_intf, /* upstream intf */
                             struct NETIO_Address *dst_addr, /* upstream addr */
                             struct NETIOBindProperty *property,
                             RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)src_intf;
    struct RTPS_IntBindEntry *bind_entry = NULL;
    DB_ReturnCode_T db_rc;
    struct RTPS_IntBindEntry bind_key;
    UNUSED_ARG(property);

    /* bad param check */
    if (src_intf == NULL || src_addr == NULL ||
        dst_intf == NULL || dst_addr == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("bind external:",RTI_FALSE)
    OSAPI_TRACE_GUID("source",&src_addr->value.as_int32.value,RTI_FALSE)
    OSAPI_TRACE_GUID("intf",&dst_addr->value.as_int32.value,RTI_TRUE)

    bind_key.intf = dst_intf;

    /* assert in own bind table .. */
    db_rc = DB_Table_select_match(intf->_parent._btable,
                                      DB_TABLE_DEFAULT_INDEX,
                                      (DB_Record_T *)&bind_entry,
                                      &bind_key);
    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if ((db_rc != DB_RETCODE_OK) && (db_rc != DB_RETCODE_NO_DATA))
    {
        RTPS_LOG_LOOKUP_EXT_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    if (db_rc == DB_RETCODE_OK)
    {
        ++bind_entry->ref_count;
        return RTI_TRUE;
    }

    if (DB_Table_create_record(intf->_parent._btable,
                              (DB_Record_T *)&bind_entry) != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_CREATE_EXT_BIND_ENTRY(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(bind_entry, sizeof(struct RTPS_IntBindEntry));
    bind_entry->intf = dst_intf;
    bind_entry->ref_count = 1;

    db_rc = DB_Table_insert_record(intf->_parent._btable,
                                   (DB_Record_T)bind_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_INSERT_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        (void)DB_Table_delete_record(intf->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    if ((intf->property.mode == RTPS_INTERFACEMODE_WRITER) &&
        intf->property.reliable)
    {
        if (intf->endpoint.writer.upstr_intf == NULL)
        {
            intf->endpoint.writer.upstr_intf = dst_intf;
        }
        else if (intf->endpoint.writer.upstr_intf != dst_intf)
        {
            RTPS_LOG_INTERFACE_MISMATCH(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Unbind RTPS interface from upstream interface
 *
 * \details
 * Remove bind entry from local bind table, stopping interface from passing or
 * requesting samples upstream.
 *
 * \param[in] src_intf Self RTPS interface
 * \param[in] src_addr Self address
 * \param[in] dst_intf Upstream interface
 * \param[in] dst_addr Upstream interface's address
 * \param[out] existed Flag of whether bind entry for this and upstream interface
 *                     already existed.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_unbind_external(NETIO_Interface_T *src_intf, /* self intf */
                               struct NETIO_Address *src_addr, /* self addr */
                               NETIO_Interface_T *dst_intf, /* upstream intf */
                               struct NETIO_Address *dst_addr, /* upstream addr */
                               RTI_BOOL *existed)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)src_intf;
    struct RTPS_IntBindEntry *bind_entry = NULL;
    DB_ReturnCode_T db_rc;
    struct RTPS_IntBindEntry bind_key;

    /* bad param check */
    if (src_intf == NULL || src_addr == NULL ||
        dst_intf == NULL || dst_addr == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("unbind external:",RTI_FALSE)
    OSAPI_TRACE_GUID("source",&src_addr->value.as_int32.value,RTI_FALSE)
    OSAPI_TRACE_GUID("intf",&dst_addr->value.as_int32.value,RTI_TRUE)

    bind_key.intf = dst_intf;

    db_rc = DB_Table_select_match(intf->_parent._btable,
                                  DB_TABLE_DEFAULT_INDEX,
                                  (DB_Record_T *)&bind_entry,&bind_key);

    if (existed != NULL)
    {
        *existed = (db_rc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE;
    }

    if (db_rc == DB_RETCODE_NO_DATA)
    {
        RTPS_LOG_NONEXISTENT_EXTERNAL_BIND(OSAPI_LOGKIND_WARNING)
        return RTI_TRUE;
    }

    --bind_entry->ref_count;

    if (bind_entry->ref_count > 0)
    {
        return RTI_TRUE;
    }

    bind_entry = NULL;
    db_rc = DB_Table_remove_record(intf->_parent._btable,
                                   (DB_Record_T *)&bind_entry,
                                    &bind_key);

    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_REMOVE_EXTERNAL_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    db_rc = DB_Table_delete_record(intf->_parent._btable,
                                       (DB_Record_T)bind_entry);
    if (db_rc != DB_RETCODE_OK)
    {
        RTPS_LOG_DB_DELETE_EXTERNAL_BIND_ENTRY(OSAPI_LOGKIND_ERROR, db_rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Get the RTPS external interface for a given RTPS interface
 *
 * \details
 * Returns the RTPS external interface that has the same GUID Prefix as
 * the input RTPS interface's GUID.
 *
 * \param[in] netio_intf Self RTPS interface
 * \param[in] src_addr Unused
 * \param[out] dst_intf Pointer to external interface
 * \param[out] dst_addr Address of external interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_get_external_interface(NETIO_Interface_T *netio_intf,
                                      struct NETIO_Address *src_addr,
                                      NETIO_Interface_T **dst_intf, /* out */
                                      struct NETIO_Address *dst_addr /* out */)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_Interface *ext_intf = NULL;
    UNUSED_ARG(src_addr);

    ext_intf = (struct RTPS_Interface*)REDA_Indexer_find_entry(
                intf->factory->ext_intf_index, &intf->_parent.local_address);

    if (ext_intf == NULL)
    {
        RTPS_LOG_FIND_EXTERNAL_INTERFACE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    *dst_intf = &ext_intf->_parent;

    /* The RTPS external interface parses source address from the RTPS message
     * itself. Thus, it is not necessary to know the destination address passed
     * in. By using the same destination address for all messages the number
     * of entries in the layer RTPS is bound too is reduced (fewer keys). For
     * consistency we use the GUID prefix and 0 for Entity id.
     */
    *dst_addr = intf->_parent.local_address;
    dst_addr->value.rtps_guid.object_id = 0;

    return RTI_TRUE;
}

/*ci
 * \brief
 * Forward a received packet to upstream interfaces
 *
 * \param[in] intf Self RTPS interface
 * \param[out] all_received Flag indicating whether packet was successfully
 *                          received by all upstream interfaces.
 * \param[in] packet Received packet
 * \param[in] src_addr Address (GUID) of sending RTPS interface
 *
 * \return RTI_TRUE if the packet is forwarded successfully upstream,
 *         RTI_FALSE if not.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_forward_upstream(struct RTPS_Interface *intf,
                               RTI_BOOL *all_received,
                               NETIO_Packet_T *packet,
                               struct NETIO_Address *src_addr)
{
    DB_ReturnCode_T db_rc;
    struct RTPS_IntBindEntry *bind_entry;
    RTI_SIZE_T pkt_head, pkt_tail;
    struct NETIO_Address real_src;
    struct NETIO_Address saved_src;
    DB_Cursor_T cursor;

    packet->info.protocol_id = NETIO_PROTOCOL_RTPS;
    NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);
    saved_src = packet->source;

    /* NOTE: anon interfaces does not have an entry (because they have not
     * been discovered yet. RTPS does not require that state is maintained
     * for these entities. The only anon entities are built-in participant
     * readers/writers.
     */
    if (src_addr == NULL)
    {
        real_src = packet->source;
        real_src.value.as_int32.value[0] = 0;
        real_src.value.as_int32.value[1] = 0;
        real_src.value.as_int32.value[2] = 0;
    }
    else
    {
        real_src = *src_addr;
    }

    if (all_received != NULL)
    {
        *all_received = RTI_TRUE;
    }

    cursor = NULL;
    db_rc = DB_Table_select_all(intf->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                &cursor);
    if (db_rc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    db_rc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
    while (db_rc == DB_RETCODE_OK)
    {
        packet->source = real_src;
        packet->local_source = intf->_parent.local_address;

        if (!NETIO_Interface_receive(bind_entry->intf,&real_src,
                                     &intf->_parent.local_address,
                                     packet))
        {
            RTPS_LOG_RECEIVE(OSAPI_LOGKIND_ERROR)

            /* Failed upstream reception is counted as a rejected sample */
            ++intf->context.sample_rejected_count;

            if (all_received != NULL)
            {
                *all_received = RTI_FALSE;
            }
        }
        db_rc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
    }
    DB_Cursor_finish(intf->_parent._btable,cursor);

    NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);
    packet->source = saved_src;

    return RTI_TRUE;
}

/*ci
 * \brief
 * Process a received RTPS GAP submessage
 *
 * \param[in] intf Self RTPS reader interface
 * \param[in] peer_entry Source writer's peer entry
 * \param[in] packet Received packet containing submessage
 * \param[in] stream Pointer to submessage for deserialization
 * \param[in] submsg_len Submessage length
 * \param[in] byte_swap Flag indicating endianness byte-swap needed to
 *                      deserialize submessage.
 *
 * \return RTI_FALSE on invalid submessage, otherwise RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_process_gap(struct RTPS_Interface *intf,
                          struct RTPS_PeerEntry *peer_entry,
                          NETIO_Packet_T *packet,
                          char *stream,
                          RTI_UINT32 submsg_len,
                          RTI_BOOL byte_swap)
{
    RTPS_SampleId_T oldlead, highest_accepted_sn, sn_start, sn_end, zero_sn;
    struct RTPS_Bitmap bitmap, final_bitmap, writer_bitmap_before_gap;
    struct RTPS_Reader *reader;
    struct RTPS_RemoteWriter *writer;
    RTPS_SampleId_T SN_ONE = {0,1};
    RTPS_SampleId_T SN_ZERO = {0,0};
    struct NETIO_Address peer_addr;
    RTI_INT32 unreserved_count = 0;

    if ((intf == NULL) || (peer_entry == NULL))
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);

    /* Only unanonymous readers process GAPs.
     * (anonymous writer only talks with anonymous readers,
     * and being best-effort and stateless, anonymous writers
     * never send GAPs
     */
    if (intf->property.mode != RTPS_INTERFACEMODE_READER ||
        intf->property.anonymous)
    {
        return RTI_TRUE;
    }

    reader = &intf->endpoint.reader;
    writer = &peer_entry->info.writer;

    oldlead = writer->bitmap.lead;

    /* Precondition: stream pointing to GAP starting sn */
    RTPS_SequenceNumber_deserialize(&stream, &sn_start, byte_swap);

    if (!RTPS_Bitmap_deserialize(&bitmap, (const char**)&stream,
                             (submsg_len - RTPS_SUBMSG_MIN_LEN_GAP), byte_swap))
    {
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("received GAP:",RTI_FALSE)
    OSAPI_TRACE_INT32("sn_start.high",sn_start.high,RTI_FALSE)
    OSAPI_TRACE_INT32("sn_start.low",sn_start.low,RTI_FALSE)
    OSAPI_TRACE_INT32("bitmap.lead.high",bitmap.lead.high,RTI_FALSE)
    OSAPI_TRACE_INT32("bitmap.lead.low",bitmap.lead.low,RTI_TRUE)

    /* The range of sequence numbers in a GAP is from its sn_start to the last
     * bit of its bitmap.  All SNs from sn_start to the bitmap's lead are
     * committable, and all 1's in the bitmap are also committable.
     *
     * To process the GAP, all of its committable SNs must be merged with the
     * peer writer's bitmap of received/committed SNs.
     *
     */

    /* Invalid GAP with invalid sn_start */
    if (REDA_SequenceNumber_compare(&sn_start, &SN_ONE) == -1)
    {
        return RTI_FALSE;
    }

    /* Save writer's bitmap before merging with GAP.  Will be used later to
     * determine unreserved count.
     */
    writer_bitmap_before_gap = writer->bitmap;

    /* The distance between the GAP's sn_start and its bitmap's lead may be 
     * larger than the maximum size of a bitmap. The peer writer's 
     * bitmap may need to be shifted to the end of that range.  This is 
     * necessary when sn_start to the GAP bitmap's lead straddles the 
     * peer writer bitmap's lead. 
     */
    if ((REDA_SequenceNumber_compare(&sn_start, &writer->bitmap.lead) <= 0) &&
        (REDA_SequenceNumber_compare(&bitmap.lead, &writer->bitmap.lead) > 0))
    {
        if (!RTPS_Bitmap_shift(&writer->bitmap, &bitmap.lead))
        {
            RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }
    }

    /* Create a temp bitmap, covering the range of the writer's bitmap, with
     * bits set according to the GAP.  Will then be merged with the writer's
     * bitmap.
     */
    RTPS_Bitmap_reset(&final_bitmap, &writer->bitmap.lead,
                      RTPS_BITMAP_SIZE_MAX);

    /* Fill the bits of the temp bitmap, from the GAP's sn_start up to
     * its bitmap's lead, exclusive.
     * This is necessary when sn_start > writer bitmap's lead (as the writer's
     * bitmap has not been shifted above), and is otherwise a noop.
     */
    sn_end = bitmap.lead;
    REDA_SequenceNumber_minusminus(&sn_end);

    /* Only fill the range if start <= end, otherwise proceed to
     * handling the bitmap. While the GAP range may be invalid, indicating
     * zero SNs in the range, the bitmap should still to be processed.
     */
    if (REDA_SequenceNumber_compare(&sn_start, &sn_end) <= 0)
    {
        if (!RTPS_Bitmap_fill(&final_bitmap, &sn_start, &sn_end, RTI_TRUE))
        {
            return RTI_TRUE;
        }
    }

    RTPS_Bitmap_merge(&final_bitmap, &bitmap);
    RTPS_Bitmap_merge(&writer->bitmap, &final_bitmap);

    /* update data-received bitmap */
    if (RTPS_Bitmap_get_first_bit(&writer->bitmap, &zero_sn, RTI_FALSE))
    {
        if (!RTPS_Bitmap_shift(&writer->bitmap, &zero_sn))
        {
            RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }
    }
    else
    {
        zero_sn.high = 0;
        zero_sn.low = (RTI_UINT32)writer->bitmap.bit_count;
        REDA_SequenceNumber_increment(&zero_sn, &writer->bitmap.lead);
        RTPS_Bitmap_reset(&writer->bitmap, &zero_sn,
                          (RTI_INT32)intf->property.max_window_size);
    }

    if (REDA_SequenceNumber_compare(&oldlead, &writer->bitmap.lead) >= 0)
    {
        /* no samples to commit */
        return RTI_TRUE;
    }


    /* Unreserve all samples, up to either the writer bitmap's new lead or
     * the original highest accepted sequence number, that
     * were previously unreceived but now GAP'd.
     */
    REDA_SequenceNumber_min(&highest_accepted_sn,
                            &writer->highest_accepted_sn, &writer->bitmap.lead);

    unreserved_count = RTPS_Bitmap_get_unreserved_count(&writer_bitmap_before_gap,
                                                        &highest_accepted_sn);

    /* Do not manipulate reservation counts for batched writer since
     * it is unknown what the virtual SNs was reserved for. Instead
     * wait for a HEARTBEAT_BATCH. Note that a GAP that moved the lead
     * beyond the highest accepted SN could be viewed as a HEARTBEAT_BATCH
     * with a first_virtual > lead. However, it is unknown what the first
     * virtual lead is.
     */
#if RTPS_DATA_BATCH_ENABLED
    if (!writer->batch_writer &&
        REDA_SequenceNumber_compare(&writer->highest_accepted_sn,&SN_ZERO) > 0)
#else
    if (REDA_SequenceNumber_compare(&writer->highest_accepted_sn,&SN_ZERO) > 0)
#endif
    {
        reader->reserved_count -= unreserved_count;
        writer->reserved_count -= unreserved_count;

#if OSAPI_ENABLE_LOG
        if (reader->reserved_count < 0)
        {
            RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
        }
#endif
    }

    /* Commit (invalid) samples upstream. */
    /* Set invalid data flag, first uncommittable sn */
    packet->info.valid_data = 0;
    packet->info.committable_sn = writer->bitmap.lead;
    packet->info.rtps_flags |= NETIO_RTPS_FLAGS_COMMIT_DATA;

    /* Always commit by RTPS for batch data */
    REDA_SequenceNumber_set_zero(&packet->info.last_committed_virtual_sn);
    if (!RTPS_Receiver_forward_upstream(intf, NULL, packet, &peer_addr))
    {
        RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    /* Because samples were committed, redetermine reserved count */

    /* Look for the last bit set in writer' bitmap, as that will be the highest
     * accepted SN.  If not found, highest accepted SN is the first SN before
     * the bitmap's lead SN.
     */
    if (!RTPS_Bitmap_get_last_bit(&writer->bitmap, &writer->highest_accepted_sn,
                                  RTI_TRUE))
    {
        /* Bitmap is empty, meaning bitmap was shifted and
         *  reserved count is now zero
         */
        writer->highest_accepted_sn = writer->bitmap.lead;
        REDA_SequenceNumber_minusminus(&writer->highest_accepted_sn);
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Process a received RTPS ACKNACK submessage
 *
 * \param[in] intf Self RTPS writer interface
 * \param[in] peer_entry Source reader's peer entry
 * \param[in] stream Pointer to packet's buffer to deserialize submessage
 * \param[in] flags Submessage flags
 * \param[in] submsg_len Submessage length
 * \param[in] byte_swap Flag indicating endianness byte-swap needed to
 *                      deserialize submessage
 *
 * \return RTI_FALSE on invalid submessage, otherwise RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_process_acknack(struct RTPS_Interface *intf,
                              struct RTPS_PeerEntry *peer_entry,
                              char *stream,
                              RTI_UINT8 flags,
                              RTI_UINT32 submsg_len,
                              RTI_BOOL byte_swap)
{
    RTI_BOOL ok = RTI_TRUE;
#if RTPS_RELIABILITY   /* returns false if !defined(RTPS_RELIABILITY) */
    struct RTPS_Writer *writer = NULL;
    struct RTPS_RemoteReader *reader = NULL;
    struct RTPS_Bitmap bitmap;
    struct REDA_SequenceNumber bitmap_last_sn = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber a_sn;
    REDA_Epoch_T epoch;
    struct RTPS_ResendContext ctxt = RTPS_ResentContext_INITIALIZER;
    NETIO_Packet_T *packet = NULL;
    RTPS_SampleId_T reader_first_unacked_sn;
    struct NETIO_Event peer_event;
    RTI_BOOL removed;
    struct NETIO_Address peer_addr;
    RTI_BOOL window_reset = RTI_FALSE;

    if ((intf == NULL) || (peer_entry == NULL))
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);

    /* only unanonymous writers process ACKNACKs */
    if (intf->property.mode != RTPS_INTERFACEMODE_WRITER ||
        intf->property.anonymous)
    {
        return RTI_TRUE;
    }

    writer = &intf->endpoint.writer;
    reader = &peer_entry->info.reader;

    /* dont' process if writer is not reliable */
    if (!(writer->flags & RTPS_WRITER_FLAG_RELIABLE) || (!reader->reliable))
    {
        return RTI_TRUE;
    }

    if (!RTPS_Bitmap_deserialize(&bitmap, (const char**)&stream,
                         (submsg_len - RTPS_SUBMSG_MIN_LEN_ACKNACK), byte_swap))
    {
        /* invalid on failed bitmap deserialization */
        return RTI_FALSE;
    }

    RTPS_Epoch_deserialize(&epoch, &stream, byte_swap);

    /* ignore same ACKNACK */
    if (epoch == reader->epoch)
    {
        OSAPI_TRACE_NET("dropped stale ACKNACK:",RTI_FALSE)
        OSAPI_TRACE_INT32("bitmap.lead.high",bitmap.lead.high,RTI_FALSE)
        OSAPI_TRACE_INT32("bitmap.lead.low",bitmap.lead.low,RTI_FALSE)
        OSAPI_TRACE_INT32("bitmap.bit_count",bitmap.bit_count,RTI_FALSE)
        OSAPI_TRACE_INT32("epoch",epoch,RTI_TRUE)

        return RTI_TRUE;
    }

    reader->epoch = epoch;

    OSAPI_TRACE_NET("received ACKNACK:",RTI_FALSE)
    OSAPI_TRACE_INT32("bitmap.lead.high",bitmap.lead.high,RTI_FALSE)
    OSAPI_TRACE_INT32("bitmap.lead.low",bitmap.lead.low,RTI_FALSE)
    OSAPI_TRACE_INT32("bitmap.bit_count",bitmap.bit_count,RTI_FALSE)
    OSAPI_TRACE_INT32("epoch",epoch,RTI_TRUE)

    /* Verify upstream interface is available for processing request */
    if (writer->upstr_intf == NULL)
    {
        RTPS_LOG_INTERFACE_MISMATCH(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    /* Receiving this ACKNACK means this Reader is active */
    if (reader->is_inactive)
    {
        reader->is_inactive = RTI_FALSE;
        reader->inactive_count = intf->property.max_hb_retries;

        ++writer->active_reliable_reader_count;
        --writer->inactive_reliable_reader_count;

        peer_event.kind = NETIO_EVENTKIND_ACTIVE_PEER;
        NETIO_Address_set_guid(&peer_event.value.peer_activity.peer_addr,0,
                               &peer_entry->addr);
        peer_event.value.peer_activity.active_change = 1;
        peer_event.value.peer_activity.active_total =
            writer->active_reliable_reader_count;
        peer_event.value.peer_activity.inactive_change = -1;
        peer_event.value.peer_activity.inactive_total =
            writer->inactive_reliable_reader_count;

        if (!NETIO_Interface_post_event(writer->upstr_intf,
                                        &intf->_parent,
                                        &peer_event))
        {
            RTPS_LOG_STATUS_CHANGE(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

    }
    reader->inactive_count = intf->property.max_hb_retries;

    ctxt.writer_entity = intf->_parent.local_address.value.guid.entity;
    ctxt.reader_entity = peer_entry->addr.entity;

    /* If preemptive ACKNACK (zero lead SN and bit count) there are two
     * cases:
     * 1) For a user reader, send a HB with the currently available samples
     * 2) For a builtin reader, reset the send-window for the reader and
     *    send a heartbeat as if the reader has not received anything.
     */
    if (!(writer->flags & RTPS_WRITER_BUILTIN_WRITER) &&
         (bitmap.bit_count == 0) && REDA_SequenceNumber_is_zero(&bitmap.lead))
    {
        if (!RTPS_Interface_initialize_packet(intf))
        {
            RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        ctxt.send_flags = RTPS_SEND_HB_FLAG;

        if (!RTPS_Writer_direct_send(intf, ctxt.send_flags,intf->packet,
                         peer_entry, &ctxt.writer_entity, &ctxt.reader_entity,
                         &ctxt.gap_start, &ctxt.gap_end))
        {
            RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        return RTI_TRUE;
    }

    /* MICR-1571:
     * For built-in writers it is necessary to support a reset of the
     * remote reader's state if the remote reader looses liveliness with
     * this writer, but the writer does not loose liveliness with the
     * reader.
     *
     * For user readers, it is NOT supported that a reader can request
     * previously sent samples.
     *
     * When a reader sends an ACKNACK the writer determines the state based on
     * the bitmap lead and the initial lead. If the received lead is 0 the
     * remote reader window it reset. Note that the code is written to handle
     * a broader range of resets, but only an ACKNACK with a bitmap lead of 0
     * triggers the code.
     *
     * By computing the current state of the reader, the writer can
     * re-send previously sent samples, even if they were ACKed.
     *
     * This requires that upstream correctly updates the expected number of
     * ACKnacks.
     */
    if ((writer->flags & RTPS_WRITER_BUILTIN_WRITER) &&
        REDA_SequenceNumber_is_zero(&bitmap.lead))
    {
        /* This ACKNACK is moving backwards, re-synch */
        a_sn = reader->initial_last_acked_sn;

        /* a_sn is the last_acked_SN SN for this reader. Create a new reader
         * window based on [a_sn,writer->last_sn] and the new ACKNACK bitmap.
         * NOTE: The previous reader state is erased. That is, do not take the
         * union of the previous state and the new bitmap.
         *
         * Clear the current reader window (ACKNACK upstream the current
         * window). This is necessary in case an ACKNACK was lost before
         * the window is reset. Note that this is ok for the built-in endpoints
         * because history is KEEP_LAST of 1, TRANSIENT_LOCAL. If a session
         * is reset, the reader will get what's available regardless. Thus,
         * if a sample was NACKED upstream and the sample is removed, it was
         * removed because of either an unregister (delete) or create call
         * for a new instance. In the general case it may be necessary to
         * increment the upstream acknack count when a SN is resent due to a
         * link reset. However, in that case it may be better to not clear
         * the window upstream. However, if it is a link_reset flag could
         * be passed upstream to indicate that this is a new request for a
         * previously acked sample.
         */
        RTPS_Interface_clear_window(writer,reader);
        window_reset = RTI_TRUE;
        reader->is_inactive = RTI_FALSE;


        /* The writer sends out what is available. Thus is there is a crossing
         * of HB and an ACKNACK with a bitmap lead of 0 then, the HB has the
         * absolute truth across all readers. However, the writer does keep
         * track of what is relevant for a reader and may respond with a GAP
         * instead of DATA.
         */
        reader->epoch = 0;
        reader->repair_stuck_nack = RTI_TRUE;
        reader->last_acked_sn = a_sn;

        if (reader->window.max_size > 0)
        {
            RTPS_SampleId_T bitmaplead = reader->last_acked_sn;
            REDA_SequenceNumber_plusplus(&bitmaplead);
            RTPS_Window_reset(&reader->window,
                              reader->window.max_size, &bitmaplead);

            /* This reset ensures that the processed acknack matches the
             * readers receive window. Any ACKed samples are ignored until
             * a new ACKNACK is received.
             */
             RTPS_Bitmap_reset(&bitmap,&bitmaplead,0);
        }
    }

    /* Update reader's send window */
    reader_first_unacked_sn = reader->last_acked_sn;
    REDA_SequenceNumber_plusplus(&reader_first_unacked_sn);

    if (REDA_SequenceNumber_compare(&bitmap.lead,&reader_first_unacked_sn) > 0)
    {
        do
        {
            RTPS_Window_remove(&reader->window,
                               &removed,&reader_first_unacked_sn);
            if (removed || window_reset)
            {
                /* Call ack for all SN's, not only samples removed
                 * from window, as SN's may have been GAP'd and not in the
                 * window.
                 *
                 * NOTE: MICRO-1571 fixes the problem with asymmetric
                 * liveliness loss. However, the fix also resets the
                 * readers window, causing ACKNACKing discovery samples
                 * to fail because the sample is not in the receive window of
                 * the reader. Thus, if the window has been reset, also
                 * ackack upstream.
                 */
                if (!NETIO_Interface_acknack(writer->upstr_intf,
                                             &peer_addr,
                                             &reader_first_unacked_sn,
                                             RTI_FALSE /* nack */))
                {
                    RTPS_LOG_ACK(OSAPI_LOGKIND_ERROR)
                    return RTI_TRUE;
                }
            }

            REDA_SequenceNumber_plusplus(&reader_first_unacked_sn);
            if (!RTPS_Window_advance(&reader->window, &reader_first_unacked_sn))
            {
                RTPS_LOG_WINDOW_ADVANCE(OSAPI_LOGKIND_ERROR)
                return RTI_TRUE;
            }
        }
        while (REDA_SequenceNumber_compare(&bitmap.lead, &reader_first_unacked_sn) > 0);

        reader->last_acked_sn = bitmap.lead;
        REDA_SequenceNumber_minusminus(&reader->last_acked_sn);
    }
    else if (!reader->repair_stuck_nack)
    {
        /* ACKNACK's bitmap lead has not advanced from previous ACKNACK.
         * To prevent resending to a stuck reader that will keep rejecting and
         * NACKing the same samples, stop this non-progressing repair, and
         * instead wait until the next periodic HEARTBEAT.
         */
        return RTI_TRUE;
    }

    /* No resend for pure ACK */
    if ((bitmap.bit_count == 0) && ((flags & RTPS_ACKNACKFLAGS_F) != 0))
    {
        /* MICRO-5183:
         * If everything up to, but not including, the bitmap lead is ACKED,
         * and no response is needed (F=1), it means the reader is up to date
         * and no response is required. In this case reset
         * reader->repair_stuck_nack in case was it was set, but
         * everything was repaired before the next periodic HB is sent, such as
         * when using piggybacked HBs.
         */
        reader->repair_stuck_nack = RTI_TRUE;
        return RTI_TRUE;
    }

    /* Begin servicing NACK requests ... */
    RTPS_Bitmap_invert(&bitmap); /* 0's are now missing samples */
    RTPS_Bitmap_truncate(&bitmap, &writer->last_sn);

    /* Save SN of last valid bit of bitmap */
    bitmap_last_sn.low = (RTI_UINT32)(bitmap.bit_count - 1);
    REDA_SequenceNumber_increment(&bitmap_last_sn, &bitmap.lead);


    /* An ACKNACK may request for samples less than what was previously
     * acknowledged.  This is not normal but possible, like when a Reader
     * asymmetrically unmatches (and rematches with) a Writer.
     *
     * When this happens, the sample SNs less than first_unack_sn are resent
     * in a GAP.
     *
     */
    if (RTPS_Bitmap_get_first_bit(&bitmap, &ctxt.req_sn, RTI_FALSE) &&
       (REDA_SequenceNumber_compare(&reader->last_acked_sn, &ctxt.req_sn) >= 0))
    {
        ctxt.gap_in_progress = RTI_TRUE;
        ctxt.gap_start = ctxt.req_sn;
        ctxt.gap_end = reader->last_acked_sn;
        REDA_SequenceNumber_plusplus(&ctxt.gap_end);
        ctxt.req_sn = ctxt.gap_end;

        if (!RTPS_Bitmap_shift(&bitmap, &ctxt.req_sn))
        {
            RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }
        RTPS_Bitmap_truncate(&bitmap, &bitmap_last_sn);
    }

    /* push all requested samples */
    while ((RTPS_Bitmap_get_first_bit(&bitmap, &ctxt.req_sn, RTI_FALSE) &&
            RTPS_Window_sn_within_window(&reader->window, &ctxt.req_sn)))
    {
        if (!RTPS_Writer_request_and_resend_packet(
                            intf, writer, reader, peer_entry, &packet, &ctxt))
        {
            return RTI_TRUE;
        }

        /* Next SN to request */
        REDA_SequenceNumber_plusplus(&ctxt.actual_sn);
        if (!RTPS_Bitmap_shift(&bitmap, &ctxt.actual_sn))
        {
            RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        RTPS_Bitmap_truncate(&bitmap, &bitmap_last_sn);
    }

    /* Send GAP and/or piggyback HEARTBEAT */
    ctxt.send_flags = 0;
    if (ctxt.gap_in_progress)
    {
        ctxt.send_flags |= RTPS_SEND_GAP_FLAG;
    }

    /* Always respond with a HB to an ACKNACK */
    ctxt.send_flags |= RTPS_SEND_HB_FLAG;

    if (ctxt.resent)
    {
        /* Repaired at least one sample, so until the next periodic HB,
         *  don't repair NACK's that have not advanced.
         */
        reader->repair_stuck_nack = RTI_FALSE;
    }

    if (!RTPS_Interface_initialize_packet(intf))
    {
        RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    if (!RTPS_Writer_direct_send(intf, ctxt.send_flags,intf->packet,
                     peer_entry, &ctxt.writer_entity, &ctxt.reader_entity,
                     &ctxt.gap_start, &ctxt.gap_end))
    {
        RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

#endif /* RTPS_RELIABILITY */

    return ok;
}

/*ci
 * \brief
 * Process a received RTPS HEARTBEAT submessage
 *
 * \param[in] intf Self RTPS reader interface
 * \param[in] peer_entry Source writer's peer entry
 * \param[in] stream Pointer to packet's buffer to deserialize submessage
 * \param[in] kind Heartbeat kind (only when RTPS_DATA_BATCH_ENABLED is TRUE)
 * \param[in] flags Submessage flags
 * \param[in] byte_swap Flag indicating endianness byte-swap needed to
 * deserialize submessage
 *
 * \return RTI_FALSE on invalid submessage, otherwise RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_process_heartbeat(struct RTPS_Interface *intf,
                                struct RTPS_PeerEntry *peer_entry,
                                char *stream,
#if RTPS_DATA_BATCH_ENABLED
                                RTI_UINT8 kind,
#endif
                                RTI_UINT8 flags,
                                RTI_BOOL byte_swap)
{
    struct RTPS_Reader *reader;
    struct RTPS_RemoteWriter *writer;
    RTPS_SampleId_T sn_first, sn_last;
#if RTPS_DATA_BATCH_ENABLED
    RTPS_SampleId_T virtual_sn_first, virtual_sn_last;
#endif
    RTPS_SampleId_T sn_last_plus_one;
    REDA_Epoch_T epoch;
    struct RTPS_Bitmap bitmap;
    RTI_BOOL final_ack = RTI_FALSE;
    struct REDA_SequenceNumber SN_ONE = {0,1};
    struct NETIO_Address peer_addr;
    RTI_INT32 unreserved_count = 0;
    RTPS_SampleId_T diff;
    RTI_BOOL brc;

    if ((intf == NULL) || (peer_entry == NULL))
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);

    /* only unanonymous readers process HEARTBEATs */
    if ((intf->property.mode != RTPS_INTERFACEMODE_READER) ||
        intf->property.anonymous)
    {
        return RTI_TRUE;
    }

    reader = &intf->endpoint.reader;
    writer = &peer_entry->info.writer;

    /* liveliness assertion */
    if ((flags & RTPS_HBFLAGS_L) != 0)
    {
        /* invalid data, only asserting liveliness */
        intf->packet->info.valid_data = 0;
        intf->packet->info.rtps_flags = NETIO_RTPS_FLAGS_LIVELINESS;

        brc = RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                             &peer_addr);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(brc);
#endif
        return RTI_TRUE;
    }

#if RTPS_RELIABILITY
    /* dont' process further if reader is not reliable */
    if ((reader->flags & RTPS_READER_FLAG_RELIABLE) == 0)
    {
        return RTI_TRUE;
    }

    RTPS_SequenceNumber_deserialize(&stream, &sn_first, byte_swap);
    RTPS_SequenceNumber_deserialize(&stream, &sn_last, byte_swap);

#if RTPS_DATA_BATCH_ENABLED
    if (kind == RTPS_HEARTBEAT_BATCH_KIND)
    {
        RTPS_SequenceNumber_deserialize(&stream, &virtual_sn_first, byte_swap);
        RTPS_SequenceNumber_deserialize(&stream, &virtual_sn_last, byte_swap);

        writer->batch_writer = RTI_TRUE;
    }
    else
    {
        virtual_sn_first = sn_first;
        virtual_sn_last = sn_last;
    }
#endif

    RTPS_Epoch_deserialize(&epoch, &stream, byte_swap);

    /* ignore same heartbeat */
    if (epoch == writer->epoch)
    {
        OSAPI_TRACE_NET("dropped stale HEARTBEAT:",RTI_FALSE)
        OSAPI_TRACE_INT32("first.high",sn_first.high,RTI_FALSE)
        OSAPI_TRACE_INT32("first.low",sn_first.low,RTI_FALSE)
        OSAPI_TRACE_INT32("last.high",sn_last.high,RTI_FALSE)
        OSAPI_TRACE_INT32("last.low",sn_last.low,RTI_FALSE)
#if RTPS_DATA_BATCH_ENABLED
#if OSAPI_ENABLE_TRACE
        /* following if has only trace instructions, so no need to add 
         * it in case traces are not enabled at compile time
         */
        if (kind == RTPS_HEARTBEAT_BATCH_KIND)
        {
            OSAPI_TRACE_INT32("firstbatch.high",virtual_sn_first.high,RTI_FALSE)
            OSAPI_TRACE_INT32("firstbatch.low",virtual_sn_first.low,RTI_FALSE)
            OSAPI_TRACE_INT32("lastbatch.high",virtual_sn_last.high,RTI_FALSE)
            OSAPI_TRACE_INT32("lastbatch.low",virtual_sn_last.low,RTI_FALSE)
        }
#endif /* OSAPI_ENABLE_TRACE */
#endif
        OSAPI_TRACE_INT32("epoch",epoch,RTI_TRUE)
        return RTI_TRUE;
    }

    writer->epoch = epoch;

    OSAPI_TRACE_NET("received HEARTBEAT:",RTI_FALSE)
    OSAPI_TRACE_INT32("first.high",sn_first.high,RTI_FALSE)
    OSAPI_TRACE_INT32("first.low",sn_first.low,RTI_FALSE)
    OSAPI_TRACE_INT32("last.high",sn_last.high,RTI_FALSE)
    OSAPI_TRACE_INT32("last.low",sn_last.low,RTI_FALSE)
#if RTPS_DATA_BATCH_ENABLED
#if OSAPI_ENABLE_TRACE
    /* following if has only trace instructions, so no need to add 
     * it in case traces are not enabled at compile time
     */
    if (kind == RTPS_HEARTBEAT_BATCH_KIND)
    {
        OSAPI_TRACE_INT32("firstbatch.high",virtual_sn_first.high,RTI_FALSE)
        OSAPI_TRACE_INT32("firstbatch.low",virtual_sn_first.low,RTI_FALSE)
        OSAPI_TRACE_INT32("lastbatch.high",virtual_sn_last.high,RTI_FALSE)
        OSAPI_TRACE_INT32("lastbatch.low",virtual_sn_last.low,RTI_FALSE)
    }
#endif /* OSAPI_ENABLE_TRACE */
#endif
    OSAPI_TRACE_INT32("epoch",epoch,RTI_TRUE)

    /* Validity check: non-negative sn_first */
    if (REDA_SequenceNumber_compare(&sn_first, &SN_ONE) == -1)
    {
        return RTI_FALSE;
    }

#if RTPS_DATA_BATCH_ENABLED
    if (kind == RTPS_HEARTBEAT_BATCH_KIND)
    {
        if (REDA_SequenceNumber_compare(&virtual_sn_first, &SN_ONE) == -1)
        {
            return RTI_FALSE;
        }
    }
#endif

    /* Validity check: sn_first <= sn_last + 1 */
    sn_last_plus_one = sn_last;
    REDA_SequenceNumber_plusplus(&sn_last_plus_one);

    if (REDA_SequenceNumber_compare(&sn_first, &sn_last_plus_one) > 0)
    {
        return RTI_FALSE;
    }

#if RTPS_DATA_BATCH_ENABLED
    if (kind == RTPS_HEARTBEAT_BATCH_KIND)
    {
        sn_last_plus_one = virtual_sn_last;
        REDA_SequenceNumber_plusplus(&sn_last_plus_one);

        if (REDA_SequenceNumber_compare(&virtual_sn_first, &sn_last_plus_one) > 0)
        {
            return RTI_FALSE;
        }
    }
#endif

    /* The HB has the maximum range of sequence numbers available to any data
     * reader. Thus, always advance the window based on the HB's first sequence
     * number.
     */
    if (REDA_SequenceNumber_compare(&sn_first, &writer->bitmap.lead) > 0)
    {
        if (writer->rcvd_first_hb)
        {
            /* Reader missed some samples --> notify upstream of lost data */
#if RTPS_DATA_BATCH_ENABLED
            if (kind == RTPS_HEARTBEAT_BATCH_KIND)
            {
                /* This is not a correct number because there may be samples
                 * between first_unaccepeted and first_virtual that will be
                 * committed. In the case of
                 */
                REDA_SequenceNumber_subtract(&diff, &virtual_sn_first,
                                     &writer->first_unaccepted_virtual_sn);

                intf->packet->info.lost_sample_sn =
                                            writer->first_unaccepted_virtual_sn;
            }
            else
#endif
            {
                REDA_SequenceNumber_subtract(&diff, &sn_first,
                                             &writer->bitmap.lead);

                intf->packet->info.lost_sample_sn = writer->bitmap.lead;
            }

            intf->packet->info.lost_sample_count = (RTI_INT32)diff.low;
            intf->packet->info.valid_data = 0;
            intf->packet->info.rtps_flags |= NETIO_RTPS_FLAGS_LOST_DATA;

            if (!RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                                &peer_addr))
            {
                RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
                return RTI_TRUE;
            }
        }

        /* Count the number of unreserved samples in the bitmap up to
         * the new temporary lead.
         */
        if (REDA_SequenceNumber_compare(&sn_first,&writer->highest_accepted_sn) > 0)
        {
            unreserved_count = RTPS_Bitmap_get_unreserved_count(
                                                &writer->bitmap,
                                                &writer->highest_accepted_sn);
        }
        else
        {
            unreserved_count = RTPS_Bitmap_get_unreserved_count(&writer->bitmap,
                                                                &sn_first);
        }

        if (!RTPS_Bitmap_shift(&writer->bitmap, &sn_first))
        {
            RTPS_LOG_SHIFT_BITMAP(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        if (!RTPS_Bitmap_get_first_bit(&writer->bitmap, &diff, RTI_FALSE))
        {
            return RTI_TRUE;
        }

        sn_first = writer->bitmap.lead;
        REDA_SequenceNumber_increment(&sn_first,&diff);

        if (!RTPS_Bitmap_shift(&writer->bitmap, &diff))
        {
            RTPS_LOG_BITMAP_SHIFT(OSAPI_LOGKIND_ERROR)
            return RTI_TRUE;
        }

        /* Update the highest_accepted_sn to be the maximum of the
         * current highest_accepted_sn and the new lead. The function
         * updates the highest_accepted_sn (output) to the new lead if
         * no accepted SN is found in the new bitmap.
         */
        if (!RTPS_Bitmap_get_last_bit(&writer->bitmap,
                                      &writer->highest_accepted_sn,
                                      RTI_TRUE))
        {
            /* Bitmap is empty, meaning bitmap was shifted and
             *  reserved count is now zero
             */
            writer->highest_accepted_sn = writer->bitmap.lead;
            REDA_SequenceNumber_minusminus(&writer->highest_accepted_sn);
        }

#if RTPS_DATA_BATCH_ENABLED
        if (kind == RTPS_HEARTBEAT_KIND)
        {
#endif
            reader->reserved_count -= unreserved_count;
            writer->reserved_count -= unreserved_count;

#if OSAPI_ENABLE_LOG
            if (reader->reserved_count < 0)
            {
                RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
            }
#endif
#if RTPS_DATA_BATCH_ENABLED
        }
        else
        {
            writer->first_unaccepted_virtual_sn = virtual_sn_first;
        }
#endif
    }

    /* Reader cannot commit samples until it receives a HEARTBEAT, so it knows
     * what samples are available from the writer.
     * Thus, for the GAPs and DATAs received before the first HEARTBEAT, ensure
     * that they are committed.
     */
    if (!writer->rcvd_first_hb &&
        (REDA_SequenceNumber_compare(&writer->bitmap.lead, &SN_ONE) > 0))
    {
        intf->packet->info.valid_data = 0;
        intf->packet->info.rtps_flags = NETIO_RTPS_FLAGS_COMMIT_DATA;
        intf->packet->info.committable_sn = writer->bitmap.lead;
        intf->packet->info.lost_sample_count = 0;

#if RTPS_DATA_BATCH_ENABLED
        if (kind == RTPS_HEARTBEAT_KIND)
        {
#endif
            /* Commit by RTPS SN */
            REDA_SequenceNumber_set_zero(&intf->packet->info.last_committed_virtual_sn);
            if (!RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                                &peer_addr))
            {
                RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
                return RTI_TRUE;
            }
#if RTPS_DATA_BATCH_ENABLED
        }
        else
        {
            /* Commit by Virtual SN. If the virtual SN is less or equal than
             * highest accepted, commit everything up to highest_accepted + 1.
             * If virtual_sn_first > writer->highest_accepted_vsn,
             * first commit up to the highest_accepted + 1, then the diff
             * between the highest and virtual_first. But, ignore the
             * lost count since those samples were never reserved.
             */
            if (REDA_SequenceNumber_compare(&virtual_sn_first,
                                            &writer->highest_accepted_vsn) > 0)
            {
                intf->packet->info.last_committed_virtual_sn = writer->highest_accepted_vsn;
                REDA_SequenceNumber_plusplus(&intf->packet->info.last_committed_virtual_sn);

                if (!RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                                    &peer_addr))
                {
                    RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
                    return RTI_TRUE;
                }

                reader->reserved_count -= intf->packet->info.lost_sample_count;
                writer->reserved_count -= intf->packet->info.lost_sample_count;

#if OSAPI_ENABLE_LOG
                if (reader->reserved_count < 0)
                {
                    RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
                }
#endif
                intf->packet->info.last_committed_virtual_sn = virtual_sn_first;

                if (!RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                                    &peer_addr))
                {
                    RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
                    return RTI_TRUE;
                }
            }
            else
            {
                intf->packet->info.last_committed_virtual_sn = virtual_sn_first;

                if (!RTPS_Receiver_forward_upstream(intf, NULL,intf->packet,
                                                    &peer_addr))
                {
                    RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
                    return RTI_TRUE;
                }

                reader->reserved_count -= intf->packet->info.lost_sample_count;
                writer->reserved_count -= intf->packet->info.lost_sample_count;

#if OSAPI_ENABLE_LOG
                if (reader->reserved_count < 0)
                {
                    RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
                }
#endif
            }
        }
#endif /* RTPS_DATA_BATCH_ENABLED */
    }

    writer->rcvd_first_hb = RTI_TRUE;

    bitmap = writer->bitmap;
    RTPS_Bitmap_truncate(&bitmap, &sn_last);

    /* Reader is fully acknowledged if it has no missing (zero) samples
       in its bitmap up to the last written sample */
    final_ack = (RTPS_Bitmap_get_first_bit(&bitmap, NULL, RTI_FALSE)) ?
                                                         RTI_FALSE : RTI_TRUE;

    /* Don't send ACKNACK if HEARTBEAT is final and reader is fully
       acknowledged */
    if (((flags & RTPS_HBFLAGS_F) != 0) && final_ack)
    {
        RTPS_LOG_FULLY_ACKED_READER(OSAPI_LOGKIND_INFO)
        return RTI_TRUE;
    }

    /* Send ACKNACK */
    RTPS_Bitmap_invert(&bitmap); /* over-wire format */

    brc = RTPS_Reader_send_acknack(intf, &bitmap, reader, peer_entry, final_ack);
#if OSAPI_ENABLE_LOG
    if (!brc)
    {
        RTPS_LOG_READER_SEND_ACKNACK(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(brc);
#endif

#endif /* RTPS_RELIABILITY */
    return RTI_TRUE;
}

/*ci
 * \brief
 * Process a received RTPS DATA submessage
 *
 * \param[in] intf Self RTPS reader interface
 * \param[in] peer_entry Source writer's peer entry
 * \param[in] packet Packet containing received DATA submessage
 * \param[in] flags DATA submessage flags
 * \param[in] data_len Length of payload of DATA submessage
 * \param[in] byte_swap Flag indicating endianness byte-swap needed to
 * deserialize DATA submessage
 *
 * \return RTI_FALSE on invalid submessage, otherwise RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_process_data(struct RTPS_Interface *intf,
                           struct RTPS_PeerEntry *peer_entry,
                           NETIO_Packet_T *packet,
                           RTI_UINT8 flags,
                           RTI_UINT32 data_len,
                           RTI_BOOL byte_swap)
{
    RTPS_SampleId_T first_unaccepted_sn;
    RTPS_SampleId_T new_highest_accepted_sn;
    RTPS_SampleId_T new_rsvd_count = REDA_SEQUENCE_NUMBER_ZERO;
    RTPS_SampleId_T origlead;
    RTPS_SampleId_T diff = REDA_SEQUENCE_NUMBER_ZERO;
    RTI_BOOL received;
    struct RTPS_Reader *reader;
    struct RTPS_RemoteWriter *writer;
    struct REDA_Buffer data_buf = REDA_BUFFER_INVALID;
    RTI_BOOL forwarded = RTI_FALSE;
    RTPS_SampleId_T SN_ZERO = REDA_SEQUENCE_NUMBER_ZERO;
    struct NETIO_Address peer_addr = NETIO_Address_INITIALIZER;
    RTI_BOOL reliable_reader = RTI_FALSE;
    RTI_BOOL brc;

    UNUSED_ARG(byte_swap);

    if (intf == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    /* only readers process DATA */
    if (intf->property.mode != RTPS_INTERFACEMODE_READER)
    {
        return RTI_TRUE;
    }

    /* Validity checks: SN must be positive and known.
     * Note, an unknown SN has value less than zero, so it is covered by the
     * check below.
     */
    if (REDA_SequenceNumber_compare(&packet->info.sn, &SN_ZERO) <= 0)
    {
        return RTI_FALSE;
    }

    reader = &intf->endpoint.reader;

    /* unanonymous reader needs a valid peer_entry */
    if (!intf->property.anonymous && (peer_entry == NULL))
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_WARNING)
        return RTI_TRUE;
    }

    writer = (!intf->property.anonymous ? &peer_entry->info.writer : NULL);

    OSAPI_TRACE_NET("received DATA:",RTI_FALSE)
    OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)

    reliable_reader = (!intf->property.anonymous) &&
        ((reader->flags & RTPS_READER_FLAG_RELIABLE) != 0);

    /* stream header has not been deserialized yet */
    packet->info.protocol_data.rtps_data.stream_need_byte_swap = 
                                                          CDR_BYTESWAP_INVALID;
    
    if (peer_entry != NULL)
    {
        NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);
    }
    
#if RTPS_RELIABILITY
    if (reliable_reader)
    {
        /* Assert this SN in the writer's bitmap of received SN's.
           Don't accept out-of-range or already received SN's */
        if (!RTPS_Bitmap_set_bit(&writer->bitmap, &received, &packet->info.sn,
                                 RTI_TRUE))
        {
            OSAPI_TRACE_NET("rejected out-of-range DATA:",RTI_FALSE)
            OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)
            return RTI_TRUE;
        }

        if (received)
        {
            RTPS_LOG_DATA_ALREADY_RECEIVED(
                    OSAPI_LOGKIND_INFO, 
                    packet->info.sn.high, 
                    packet->info.sn.low,
                    writer->bitmap.lead.high,
                    writer->bitmap.lead.low,
                    peer_addr.value.rtps_guid.host_id,
                    peer_addr.value.rtps_guid.app_id,
                    peer_addr.value.rtps_guid.instance_id,
                    peer_addr.value.rtps_guid.object_id)
            OSAPI_TRACE_NET("rejected already received DATA:",RTI_FALSE)
            OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)
            return RTI_TRUE;
        }

        /* Get first unreceived SN.
         *
         *  Impl note: checking return value of RTPS_Bitmap_get_first_bit()
         *  is unnecessary. If retval is false, it means all bits of the
         *  bitmap are 1', and its postcondition sets first_unaccepted_sn to the
         *  first SN after the largest SN of the bitmap, which is the correct
         *  first unreceived SN.
         */
        (void)RTPS_Bitmap_get_first_bit(&writer->bitmap,
                                        &first_unaccepted_sn, RTI_FALSE);

        /* Ensure there's enough space to accept sample.
         *
         * If any previous SN have not been received, check that there is
         * enough space to reserve from the highest received SN to the lowest
         * unreceived/uncommittable SN
         *
         * Either this packet will be the new highest accepted SN, or a
         * prior out-of-order sample is still the highest accepted SN
         */
        REDA_SequenceNumber_max(&new_highest_accepted_sn,
                                &writer->highest_accepted_sn,
                                &packet->info.sn);

        /* Reserve space if any holes in bitmap, due to out-of-order reception.
         *
         * There are out-of-order samples only if the highest committed SN is
         * greater than the first uncommittable SN, meaning a sample SN less
         * than the highest committed SN has not been received.
         *
         * More precisely:
         *       first_uncommittable_sn = new_highest_committed_sn + 1
         */
        if ((REDA_SequenceNumber_compare(&writer->highest_accepted_sn,
                                         &new_highest_accepted_sn) < 0))
        {
            REDA_SequenceNumber_subtract(&new_rsvd_count,
                                         &new_highest_accepted_sn,
                                         &writer->highest_accepted_sn);

            /* Continue sample reception only if the new reserved count
             * (equal to subtracting the writer's previous reserved count and
             * adding the writer's updated reserved count) does
             * not exceed the max available reserve count
             */
            if (intf->property.max_samples <
                (reader->reserved_count + (RTI_INT32)new_rsvd_count.low))
            {
                OSAPI_TRACE_NET("rejected uncommittable DATA:",RTI_FALSE)
                OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_FALSE)
                OSAPI_TRACE_INT32("writer->highest_accepted_sn.high",
                                  writer->highest_accepted_sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("writer->highest_accepted_sn.low",
                                  writer->highest_accepted_sn.low,RTI_FALSE)
                OSAPI_TRACE_INT32("new_highest_accepted_sn.high",
                                  new_highest_accepted_sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("new_highest_accepted_sn.low",
                                  new_highest_accepted_sn.low,RTI_FALSE)
                OSAPI_TRACE_INT32("new_rsvd_count.low",
                                  new_rsvd_count.low,RTI_TRUE)


                /* Revert bitmap */
                brc = RTPS_Bitmap_set_bit(&writer->bitmap, NULL, &packet->info.sn,
                                          RTI_FALSE);
#if OSAPI_ENABLE_LOG
                if (!brc)
                {
                    RTPS_LOG_BITMAP_SET_BIT(OSAPI_LOGKIND_INFO)
                }
#else
                IGNORE_RETVAL(brc);
#endif
                return RTI_TRUE;
            }
        }

#if OSAPI_ENABLE_LOG
        if (REDA_SequenceNumber_compare(
                &packet->info.sn,
                &writer->bitmap.lead) > 0)
        {
            /* Sample Received Out of Order */
            RTPS_LOG_DATA_SN_OUT_OF_ORDER(
                    OSAPI_LOGKIND_INFO, 
                    packet->info.sn.high, 
                    packet->info.sn.low,
                    writer->bitmap.lead.high,
                    writer->bitmap.lead.low,
                    peer_addr.value.rtps_guid.host_id,
                    peer_addr.value.rtps_guid.app_id,
                    peer_addr.value.rtps_guid.instance_id,
                    peer_addr.value.rtps_guid.object_id)
        }
#endif
    }
    else /* Best-Effort */
#endif /* RTPS_RELIABILITY */
    {
        first_unaccepted_sn = packet->info.sn;
        REDA_SequenceNumber_plusplus(&first_unaccepted_sn);

        if (!intf->property.anonymous)/* best-effort, not anonymous */
        {
            if ((REDA_SequenceNumber_compare(
               &packet->info.sn,&writer->bitmap.lead) < 0))
            {
                RTPS_LOG_DATA_ALREADY_RECEIVED(
                        OSAPI_LOGKIND_INFO, 
                        packet->info.sn.high, 
                        packet->info.sn.low,
                        writer->bitmap.lead.high,
                        writer->bitmap.lead.low,
                        peer_addr.value.rtps_guid.host_id,
                        peer_addr.value.rtps_guid.app_id,
                        peer_addr.value.rtps_guid.instance_id,
                        peer_addr.value.rtps_guid.object_id)
                OSAPI_TRACE_NET("rejected already received DATA:",RTI_FALSE)
                OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_TRUE)

                return RTI_TRUE;
            }

            origlead = writer->bitmap.lead;
            RTPS_Bitmap_reset(&writer->bitmap, &first_unaccepted_sn, 0);

            if (REDA_SequenceNumber_compare(&packet->info.sn, &origlead) > 0)
            {
                /* lost some sample(s) */
                REDA_SequenceNumber_subtract(&diff, &packet->info.sn, &origlead);
                packet->info.lost_sample_count = (RTI_INT32)diff.low;
                packet->info.lost_sample_sn = origlead;
                packet->info.rtps_flags |= NETIO_RTPS_FLAGS_LOST_DATA;
            }
        }
    }

    OSAPI_TRACE_NET("received DATA: GUID:",RTI_FALSE)
    OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)

    /*  Payload is either valid data, valid key, or invalid */
    if ((flags & RTPS_DATAFLAGS_D) && !(flags & RTPS_DATAFLAGS_K))
    {
        packet->info.valid_data = 1;
        packet->info.valid_key = 0;
    }
    else if (!(flags & RTPS_DATAFLAGS_D) && (flags & RTPS_DATAFLAGS_K))
    {
        packet->info.valid_data = 0;
        packet->info.valid_key = 1;
    }
    else
    {
        packet->info.valid_data = 0;
        packet->info.valid_key = 0;
    }

    /* committable_sn is actually first unaccepted sn */
    packet->info.committable_sn = first_unaccepted_sn;
    if ((flags & RTPS_DATAFLAGS_Q) != 0)
    {
        packet->info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
    }

    REDA_Buffer_set(&data_buf, NETIO_Packet_get_head(packet), data_len);
    packet->info.protocol_data.rtps_data.inline_data = &data_buf;
    packet->info.protocol_id = NETIO_PROTOCOL_RTPS;
    packet->info.rtps_flags |= NETIO_RTPS_FLAGS_DATA;

    /* If new_rsvd_count > 0, it is in addition to previously reserved samples.
     */
    if (new_rsvd_count.low > 0)
    {
        reader->reserved_count += (RTI_INT32)new_rsvd_count.low;
        writer->reserved_count += (RTI_INT32)new_rsvd_count.low;
    }

    /* Always commit by RTPS for data */
    REDA_SequenceNumber_set_zero(&packet->info.last_committed_virtual_sn);

    /* forward upstream, to modules expecting msgs from src */
    if (peer_entry == NULL)
    {
        forwarded = RTPS_Receiver_forward_upstream(intf,&received,packet,NULL);
    }
    else
    {
        forwarded = RTPS_Receiver_forward_upstream(intf,&received,
                                                   packet,&peer_addr);
    }

    /* Check reliable sample was accepted upstream */
    if (reliable_reader)
    {
#if RTPS_RELIABILITY
        if (!forwarded || !received)
        {
            if (new_rsvd_count.low > 0)
            {
                reader->reserved_count -= (RTI_INT32)new_rsvd_count.low;
                writer->reserved_count -= (RTI_INT32)new_rsvd_count.low;
            }

#if OSAPI_ENABLE_LOG
            if (reader->reserved_count < 0)
            {
                RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
            }
#endif
            /* Revert bitmap on failed upstream reception */
            if (!RTPS_Bitmap_set_bit(&writer->bitmap, NULL, &packet->info.sn,
                                     RTI_FALSE))
            {
                RTPS_LOG_BITMAP_SET_BIT(OSAPI_LOGKIND_INFO)
                return RTI_TRUE;
            }
        }
        else
#endif /* RTPS_RELIABILITY */
        {
            /* Upstream reception was successful. Call bitmap shift(), which
             *  will advance bitmap as necessary if first unaccepted SN has
             *  advanced.
             */
            if (!RTPS_Bitmap_shift(&writer->bitmap, &first_unaccepted_sn))
            {
                RTPS_LOG_SHIFT_BITMAP(OSAPI_LOGKIND_ERROR)
                return RTI_TRUE;
            }

            /* Update accepted SN and reserved counts */
            REDA_SequenceNumber_max(&writer->highest_accepted_sn,
                                    &writer->highest_accepted_sn,
                                    &packet->info.sn);
        }
    }

#if OSAPI_ENABLE_LOG
    if (!forwarded)
    {
        RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
    }
#endif

    return RTI_TRUE;
}

#if RTPS_DATA_BATCH_ENABLED
/*ci
 * \brief
 * Skips inline qos in a packet
 *  
 * \param[in] packet Packet containing received DATA_BATCH submessage
 * \param[in] byte_swap Indicates how to deserialize data in the packet
 * \param[out] inline_qos_len Number of bytes in the inline QoS
 * 
 * \return RTI_FALSE on invalid inline qos
 */
RTI_PRIVATE RTI_UINT32
RTPS_Receiver_process_skip_inline_qos(NETIO_Packet_T *packet,
                                      RTI_BOOL byte_swap,
                                      RTI_UINT32 *inline_qos_len)
{
    char *msg_ptr = NULL;
    RTI_UINT16 pid = 0;
    RTI_UINT16 pid_length = 0;

    *inline_qos_len = 0;

    do
    {
        /* read pid */
        msg_ptr = (char*)NETIO_Packet_get_head(packet);
        if (!NETIO_Packet_set_head(packet, (RTI_UINT32)sizeof(RTI_UINT16)))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            return RTI_FALSE;
        }
        CDR_deserialize_unsigned_short(&msg_ptr, &pid, byte_swap);
        (*inline_qos_len) += (RTI_UINT32)sizeof(RTI_UINT16);

        /* read length */
        msg_ptr = (char*)NETIO_Packet_get_head(packet);
        if (!NETIO_Packet_set_head(packet, (RTI_UINT32)sizeof(RTI_UINT16)))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            return RTI_FALSE;
        }
        CDR_deserialize_unsigned_short(&msg_ptr, &pid_length, byte_swap);
        (*inline_qos_len) += (RTI_UINT32)sizeof(RTI_UINT16);

        /* skip length bytes */
        if (!NETIO_Packet_set_head(packet, pid_length))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            return RTI_FALSE;
        }
        (*inline_qos_len) += pid_length;
    }
    while(pid != RTPS_PID_SENTINEL);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Process a received RTPS DATA_BATCH submessage
 *  
 * \param[in] intf Self RTPS reader interface 
 * \param[in] peer_entry Source writer's peer entry 
 * \param[in] packet Packet containing received DATA submessage
 * \param[in] msg DATA BATCH submessage
 * \param[in] data_len Length of payload of DATA_BATCH submessage 
 * \param[in] byte_swap Flag indicating endianness byte-swap needed to 
 * deserialize DATA_BATCH submessage
 * 
 * \return RTI_FALSE on invalid submessage, otherwise RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Receiver_process_data_batch(struct RTPS_Interface *intf,
                                 struct RTPS_PeerEntry *peer_entry,
                                 NETIO_Packet_T *packet,
                                 const union RTPS_MESSAGES *msg,
                                 RTI_UINT32 data_len,
                                 RTI_BOOL byte_swap)
{
#define RTPS_DATA_BATCH_SAMPLEINFO_HEADER_LENGTH ((RTI_UINT32)sizeof(RTI_UINT16) + \
                                                  (RTI_UINT32)sizeof(RTI_UINT16) + \
                                                  (RTI_UINT32)sizeof(RTI_UINT32))
#define RTPS_TIMESTAMP_LENGTH  ((RTI_UINT32)sizeof(RTI_INT32) + (RTI_UINT32)sizeof(RTI_UINT32))

    RTPS_SampleId_T first_unaccepted_sn;
    RTPS_SampleId_T new_highest_accepted_sn;
    RTPS_SampleId_T new_rsvd_count = REDA_SEQUENCE_NUMBER_ZERO;
    RTPS_SampleId_T origlead;
    RTPS_SampleId_T diff = REDA_SEQUENCE_NUMBER_ZERO;
    RTI_BOOL received;
    struct RTPS_Reader *reader;
    struct RTPS_RemoteWriter *writer;
    struct REDA_Buffer data_buf = REDA_BUFFER_INVALID;
    struct REDA_Buffer serialized_batched_data_buf = REDA_BUFFER_INVALID;
    RTI_BOOL forwarded = RTI_FALSE;
    RTPS_SampleId_T SN_ZERO = REDA_SEQUENCE_NUMBER_ZERO;
    struct NETIO_Address peer_addr;
    RTI_BOOL reliable_reader = RTI_FALSE;
    RTI_UINT32 rtps_flags;
    RTI_UINT16 flags;
    RTI_UINT16 qos_offset;
    RTI_UINT32 serialized_data_len = 0;
    RTI_UINT32 total_serialized_data_len = 0;
    char *encapsulation_id_pointer = NULL;
    struct REDA_SequenceNumber sn_offset;
    struct REDA_SequenceNumber last_sample_sn;
    char *msg_ptr = NULL;
    RTI_UINT32 forwarded_samples = 0;

    if (intf == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_TRUE;
    }

    /* only readers process DATA_BATCH */
    if (intf->property.mode != RTPS_INTERFACEMODE_READER)
    {
        return RTI_TRUE;
    }

    /* Validity checks: SN must be positive and known.
     * Note, an unknown SN has value less than zero, so it is covered by the 
     * check below. 
     */
    if (REDA_SequenceNumber_compare(&msg->data_batch.batch_sn, &SN_ZERO) <= 0)
    {
        return RTI_FALSE;
    }

    reader = &intf->endpoint.reader;

    if ((peer_entry == NULL) || intf->property.anonymous)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_WARNING)
        return RTI_TRUE;
    }

    writer = &peer_entry->info.writer;

    OSAPI_TRACE_NET("received DATA_BATCH:",RTI_FALSE)
    OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.high",msg->data_batch.batch_sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("sn.low",msg->data_batch.batch_sn.low,RTI_FALSE)
    OSAPI_TRACE_INT32("first_sample.high",msg->data_batch.first_sample_sn.high,RTI_FALSE)
    OSAPI_TRACE_INT32("first_sample.low",msg->data_batch.first_sample_sn.low,RTI_FALSE)
    OSAPI_TRACE_INT32("offset to last SN",msg->data_batch.offset_last_sn,RTI_FALSE)
    OSAPI_TRACE_INT32("first_sample.low",msg->data_batch.first_sample_sn.low,RTI_FALSE)
    OSAPI_TRACE_INT32("batchSampleCount",msg->data_batch.batch_sample_count,RTI_TRUE)

    reliable_reader = ((reader->flags & RTPS_READER_FLAG_RELIABLE) != 0);

    /* keep a copy of the rtps flags as we might need to 
     * modify them for each sample
     */
    rtps_flags = packet->info.rtps_flags;

    writer->batch_writer = RTI_TRUE;

    /* Calculate the last sample virtual SN. This is needed to calculate
     * reservation count in case of out-of-order packets and to know
     * when the last sample in the batch is proccessed.
     * The offset is limited to 32 bit unsigned, so the high int can always 
     * be set to 0.
     */
    sn_offset.high = 0;
    sn_offset.low = msg->data_batch.offset_last_sn;
    REDA_SequenceNumber_add(&last_sample_sn,
                            &msg->data_batch.first_sample_sn,
                            &sn_offset);

    NETIO_Address_set_guid(&peer_addr,0,&peer_entry->addr);

#if RTPS_RELIABILITY
    if (reliable_reader)
    {
        /* Assert this SN in the writer's bitmap of received SN's.
            Don't accept out-of-range or already received SN's */
        if (!RTPS_Bitmap_set_bit(&writer->bitmap, &received, 
                                 &msg->data_batch.batch_sn, 
                                 RTI_TRUE))
        {
            OSAPI_TRACE_NET("rejected out-of-range DATA_BATCH:",RTI_FALSE)
            OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.high",msg->data_batch.batch_sn.high,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.low",msg->data_batch.batch_sn.low,RTI_TRUE)
            return RTI_TRUE;
        }

        if (received)
        {
            RTPS_LOG_DATA_BATCH_ALREADY_RECEIVED(
                    OSAPI_LOGKIND_INFO, 
                    msg->data_batch.batch_sn.high, 
                    msg->data_batch.batch_sn.low,
                    writer->bitmap.lead.high,
                    writer->bitmap.lead.low,
                    peer_addr.value.rtps_guid.host_id,
                    peer_addr.value.rtps_guid.app_id,
                    peer_addr.value.rtps_guid.instance_id,
                    peer_addr.value.rtps_guid.object_id)
            OSAPI_TRACE_NET("rejected already received DATA_BATCH:",RTI_FALSE)
            OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.high",msg->data_batch.batch_sn.high,RTI_FALSE)
            OSAPI_TRACE_INT32("sn.low",msg->data_batch.batch_sn.low,RTI_TRUE)
            return RTI_TRUE;
        }
        
        /* Get first unreceived SN.
         *
         *  Impl note: checking return value of RTPS_Bitmap_get_first_bit()
         *  is unnecessary. If retval is false, it means all bits of the
         *  bitmap are 1', and its postcondition sets first_unaccepted_sn to the
         *  first SN after the largest SN of the bitmap, which is the correct
         *  first unreceived SN.
         */
        (void)RTPS_Bitmap_get_first_bit(&writer->bitmap,
                                        &first_unaccepted_sn, RTI_FALSE);

        /* Ensure there's enough space to accept sample.
         *
         * If any previous SN have not been received, check that there is
         * enough space to reserve from the highest received SN to the lowest
         * unreceived/uncommittable SN
         *
         * Either this batch will be the new highest accepted SN, or a
         * prior out-of-order sample is still the highest accepted SN
         */
        REDA_SequenceNumber_max(&new_highest_accepted_sn,
                                &writer->highest_accepted_sn,
                                &msg->data_batch.batch_sn);

        /* Reserve space if any holes in bitmap, due to out-of-order reception.
         *
         * There are out-of-order samples only if the highest committed SN is
         * greater than the first uncommittable SN, meaning a sample SN less
         * than the highest committed SN has not been received.
         *
         * More precisely:
         *       first_uncommittable_sn = new_highest_committed_sn + 1
         */
        if ((REDA_SequenceNumber_compare(&writer->highest_accepted_sn,
                                         &new_highest_accepted_sn) < 0))
        {
            /* Check that there are available entries to reserve for
             * receiving all out-of-order samples. The last_sample_sn
             * for this batch SN must be higher than the previous
             * last highest vsn (because the batch sn is). Note that
             * there may be holes, but at this point this is not known.
             * If there are holes or invalid SN they will be reclaimed
             * later.
             */
            REDA_SequenceNumber_subtract(&new_rsvd_count,
                                         &last_sample_sn,
                                         &writer->highest_accepted_vsn);

            /* Continue sample reception only if there are sufficient
             * samples for the entire batch. A special case is made for
             * the case when this sample is committable, that is equal
             * to first_unaccepted_sn - 1. In this we don't perform the check
             * in order to allow a batch > max_samples to proceed. Other
             * wise the batch may always be rejected and communication will
             * stop.
             *
             * Temporarily decrement to check if first_unaccepted_sn is
             * equal to the current highest. If so then the window is fully
             * acked and the sample can be accepted. It is restored after
             * the comparison.
             */
            REDA_SequenceNumber_minusminus(&first_unaccepted_sn);

            if ((intf->property.max_samples <
                (reader->reserved_count + (RTI_INT32)new_rsvd_count.low)) &&
                REDA_SequenceNumber_compare(&new_highest_accepted_sn,
                                            &first_unaccepted_sn) > 0)
            {
                OSAPI_TRACE_NET("rejected uncommittable DATA_BATCH:",RTI_FALSE)
                OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.high",msg->data_batch.batch_sn.high,RTI_FALSE)
                OSAPI_TRACE_INT32("sn.low",msg->data_batch.batch_sn.low,RTI_TRUE)

                /* Revert bitmap */
                if (!RTPS_Bitmap_set_bit(&writer->bitmap, NULL, 
                                         &msg->data_batch.batch_sn, 
                                         RTI_FALSE))
                {
                    RTPS_LOG_BITMAP_SET_BIT(OSAPI_LOGKIND_INFO)
                }

                REDA_SequenceNumber_plusplus(&first_unaccepted_sn);
                return RTI_TRUE;
            }

            REDA_SequenceNumber_plusplus(&first_unaccepted_sn);
            writer->highest_accepted_vsn = last_sample_sn;
        }

        if (REDA_SequenceNumber_compare(
                &msg->data_batch.batch_sn,
                &writer->bitmap.lead) > 0)
        {
            /* Sample Received Out of Order */
            RTPS_LOG_DATA_BATCH_SN_OUT_OF_ORDER(
                    OSAPI_LOGKIND_INFO, 
                    msg->data_batch.batch_sn.high, 
                    msg->data_batch.batch_sn.low,
                    writer->bitmap.lead.high,
                    writer->bitmap.lead.low,
                    peer_addr.value.rtps_guid.host_id,
                    peer_addr.value.rtps_guid.app_id,
                    peer_addr.value.rtps_guid.instance_id,
                    peer_addr.value.rtps_guid.object_id)
        }
    }
    else /* Best-Effort */
#endif /* RTPS_RELIABILITY */
    {
        first_unaccepted_sn = msg->data_batch.batch_sn;
        REDA_SequenceNumber_plusplus(&first_unaccepted_sn);

        if ((REDA_SequenceNumber_compare(&msg->data_batch.batch_sn,
                                         &writer->bitmap.lead) < 0))
        {
            RTPS_LOG_DATA_BATCH_ALREADY_RECEIVED(
                    OSAPI_LOGKIND_INFO,
                    msg->data_batch.batch_sn.high,
                    msg->data_batch.batch_sn.low,
                    writer->bitmap.lead.high,
                    writer->bitmap.lead.low,
                    peer_addr.value.rtps_guid.host_id,
                    peer_addr.value.rtps_guid.app_id,
                    peer_addr.value.rtps_guid.instance_id,
                    peer_addr.value.rtps_guid.object_id)
            OSAPI_TRACE_NET("rejected already received DATA_BATCH:",RTI_FALSE)
            OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_TRUE)
            return RTI_TRUE;
        }

        origlead = writer->bitmap.lead;
        RTPS_Bitmap_reset(&writer->bitmap, &first_unaccepted_sn, 0);

        if (REDA_SequenceNumber_compare(&msg->data_batch.batch_sn,
                                        &origlead) > 0)
        {
            /* lost some sample(s) */
            REDA_SequenceNumber_subtract(&diff,
                                         &msg->data_batch.first_sample_sn,
                                         &writer->first_unaccepted_virtual_sn);
            packet->info.lost_sample_count = (RTI_INT32)diff.low;
            packet->info.lost_sample_sn = writer->first_unaccepted_virtual_sn;
            packet->info.rtps_flags |= NETIO_RTPS_FLAGS_LOST_DATA;
        }

        /* The current first unaccepted SN matches the first sample in the BATCH
         */
        writer->first_unaccepted_virtual_sn = msg->data_batch.first_sample_sn;
    }

    packet->info.sn = msg->data_batch.batch_sn;
    packet->info.virtual_sn = msg->data_batch.first_sample_sn;

    /* get the pointer to the encapsulation id */
    NETIO_Packet_save_positions(packet);
    if (!NETIO_Packet_set_head(packet, 
                               (RTI_INT32)msg->data_batch.encapsulation_offset))
    {
        RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
        return RTI_TRUE;
    }
    encapsulation_id_pointer = NETIO_Packet_get_head(packet);
    NETIO_Packet_restore_positions(packet);

    /* stream header has not be deserialized yet */
    packet->info.protocol_data.rtps_data.stream_need_byte_swap = 
                                                          CDR_BYTESWAP_INVALID;
    /* If new_rsvd_count > 0, it is in addition to previously reserved samples.
     */
    if (new_rsvd_count.low > 0)
    {
        reader->reserved_count += (RTI_INT32)new_rsvd_count.low;
        writer->reserved_count += (RTI_INT32)new_rsvd_count.low;
    }

    /* When a DATA_BATCH is committed is based on the frist unaccepted
     * bitmap because RTPS does no longer know which VSN is corresponds
     * to.
     */
    packet->info.committable_sn = first_unaccepted_sn;

    do
    {
        /* Deserialize sampleInfo up to inlineQoS
         *
         * First, deserialize flags, octets to inline Qos
         * and serialized data length
         */
        msg_ptr = (char*)NETIO_Packet_get_head(packet);
        if (!NETIO_Packet_set_head(packet, 
                                   RTPS_DATA_BATCH_SAMPLEINFO_HEADER_LENGTH))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            break;
        }

        data_len -= RTPS_DATA_BATCH_SAMPLEINFO_HEADER_LENGTH;
        CDR_deserialize_unsigned_short_from_big_endian(&msg_ptr, &flags);
        CDR_deserialize_unsigned_short(&msg_ptr, &qos_offset, byte_swap);
        CDR_deserialize_unsigned_long(&msg_ptr, &serialized_data_len, byte_swap);

        /* read timestamp if present */
        if (flags & RTPS_DATABATCHFLAGS_T)
        {
            if (!NETIO_Packet_set_head(packet, RTPS_TIMESTAMP_LENGTH))
            {
                RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
                break;
            }
            data_len -= RTPS_TIMESTAMP_LENGTH;
            CDR_deserialize_long(&msg_ptr, 
                                 &packet->info.timestamp.sec, 
                                 byte_swap);
            CDR_deserialize_unsigned_long(&msg_ptr, 
                                          &packet->info.timestamp.frac, 
                                          byte_swap);
        }

        /* read offset SN if present. if not present the offset to next sn is 1 */
        if (flags & RTPS_DATABATCHFLAGS_O)
        {
            if (!NETIO_Packet_set_head(packet, sizeof(RTI_UINT32)))
            {
                RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
                break;
            }
            data_len -= (RTI_UINT32)sizeof(RTI_UINT32);
            CDR_deserialize_unsigned_long(&msg_ptr, &sn_offset.low, byte_swap);
        }
        else
        {
            sn_offset.low = 1;
        }

        /* The offset is limited to 32 bit unsigned, so the 
         * high int can always be set to 0.
         */
        sn_offset.high = 0;

        /* If the sample is invalid data has been lost; treat invalid sample
         * as lost sample(s). It could also happen that the offset SN is greater
         * than 1, in which case there will be also lost data
         */
        if (flags & RTPS_DATABATCHFLAGS_I)
        {
            /* If a sample is invalid, also check if the diff in SN for this
             * SN and the previous > 1.
             */
            origlead = writer->first_unaccepted_virtual_sn;

            REDA_SequenceNumber_subtract(&diff,
                                         &packet->info.virtual_sn, &origlead);

            /* If data lost was already detected before start processing the
             * samples in the BATCH (before this while() loop), means that also
             * the first sample in the BATCH is invalid, so we just need to add
             * 1 to the number of lost samples.
             */
            if (packet->info.rtps_flags & NETIO_RTPS_FLAGS_LOST_DATA)
            {
                packet->info.lost_sample_count++;
            }
            else
            {
                packet->info.lost_sample_count = (RTI_INT32)diff.low + 1;
                packet->info.lost_sample_sn = origlead;
                packet->info.rtps_flags |= NETIO_RTPS_FLAGS_LOST_DATA;
            }
            writer->first_unaccepted_virtual_sn = packet->info.virtual_sn;
        }

        OSAPI_TRACE_NET("received DATA_BATCH: GUID:",RTI_FALSE)
        OSAPI_TRACE_GUID("source",&intf->_parent.local_address.value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("sn.high",packet->info.sn.high,RTI_FALSE)
        OSAPI_TRACE_INT32("sn.low",packet->info.sn.low,RTI_TRUE)

        /*  Payload is either valid data, valid key, or invalid */
        if ((flags & RTPS_DATABATCHFLAGS_D)
             && !(flags & RTPS_DATABATCHFLAGS_K)
             && !(flags & RTPS_DATABATCHFLAGS_I))
        {
            packet->info.valid_data = 1;
            packet->info.valid_key = 0;
        }
        else if (!(flags & RTPS_DATABATCHFLAGS_D)
                && (flags & RTPS_DATABATCHFLAGS_K)
                && !(flags & RTPS_DATABATCHFLAGS_I))
        {
            packet->info.valid_data = 0;
            packet->info.valid_key = 1;
        }
        else
        {
            packet->info.valid_data = 0;
            packet->info.valid_key = 0;
        }

        /* committable_sn is actually first unaccepted sn */
        if ((flags & RTPS_DATABATCHFLAGS_Q) != 0)
        {
            packet->info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
        }
        else
        {
            packet->info.rtps_flags &= ~NETIO_RTPS_FLAGS_INLINEQOS;
        }
        REDA_Buffer_set(&data_buf, NETIO_Packet_get_head(packet), data_len);

        /* Create the buffer to serialized data. The first sample the buffer
         * points to is the encapsulation id and it contains encapsulation id
         * and serialized data. For the second and following sample the
         * encapsulation id is not needed as it was already deserialized
         * so the buffer points to the beginning of serialized data.
         * Note than the serialized data length for the first sample
         * can be 0, but adding RTI_CDR_ENCAPSULATION_HEADER_SIZE
         * prevents total_serialized_data_len from being 0 after the
         * first sample was read.
         */
        if (total_serialized_data_len == 0)
        {
            REDA_Buffer_set(&serialized_batched_data_buf, 
                            encapsulation_id_pointer,
                            serialized_data_len + 
                            RTI_CDR_ENCAPSULATION_HEADER_SIZE);
            total_serialized_data_len = RTI_CDR_ENCAPSULATION_HEADER_SIZE;
        }
        else
        {
            REDA_Buffer_set(&serialized_batched_data_buf, 
                            encapsulation_id_pointer + 
                            total_serialized_data_len,
                            serialized_data_len);
        }

        packet->info.protocol_data.rtps_data.inline_data = &data_buf;
        packet->info.protocol_data.rtps_data.serialized_data_batch = 
                                               &serialized_batched_data_buf;

        packet->info.protocol_id = NETIO_PROTOCOL_RTPS;
        packet->info.rtps_flags |= NETIO_RTPS_FLAGS_DATA_BATCH;

        /* Always commit by RTPS for batch data */
        REDA_SequenceNumber_set_zero(&packet->info.last_committed_virtual_sn);

        forwarded = RTPS_Receiver_forward_upstream(
                                        intf, &received, packet,&peer_addr);

        if (forwarded)
        {
            /* jump all inline_qos read by the upstream function. this will 
             * leave the next sample_info ready to read
             */
            if (!NETIO_Packet_set_head(packet,
                                       (RTI_INT32)(data_len - data_buf.length)))
            {
                RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
                break;
            }
            data_len -= (data_len - data_buf.length);
        }
#if OSAPI_ENABLE_LOG
        else
        {
            RTPS_LOG_FORWARD_UPSTREAM(OSAPI_LOGKIND_ERROR)
        }
#endif

        if (forwarded && received)
        {
            /* Update the number of forwarded samples if reception was
             * successful. If this was only sample lost then don't reduce it
             * again.
             */
            forwarded_samples++;
        }

        if ((reliable_reader) &&
            (packet->info.rtps_flags & NETIO_RTPS_FLAGS_LOST_DATA))
        {
            /* Samples that are lost. This happens when there are holes
             * in the virtual sn range that can committed. It is always
             * set to 0 by the reader cache. The adjustment is needed because
             * non-existent samples cannot be reclaimed via the event
             * from upstream.
             */
            reader->reserved_count -= packet->info.lost_sample_count;
            writer->reserved_count -= packet->info.lost_sample_count;

#if OSAPI_ENABLE_LOG
            if (reader->reserved_count < 0)
            {
                RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
            }
#endif
        }

        /* restore rtps flags. changes in flags regarding 
         * the last sample need to be reverted
         */
        packet->info.rtps_flags = rtps_flags;

        packet->info.lost_sample_count = 0;

        /* update the total length of serialized data */
        total_serialized_data_len += serialized_data_len;

        /* Calculate the sn for the next sample in the batch */
        REDA_SequenceNumber_increment(&packet->info.virtual_sn, &sn_offset);

        /* Calculate the sn for the next unaccepted sample
         */
        REDA_SequenceNumber_plusplus(&writer->first_unaccepted_virtual_sn);
    } while (REDA_SequenceNumber_compare(&last_sample_sn,
                                         &packet->info.virtual_sn) >= 0);

    if (reliable_reader)
    {
#if RTPS_RELIABILITY
        if (!forwarded_samples)
        {
            /* Revert bitmap on failed upstream reception. In this case
             * all reserved samples have been reclaimed.
             */
            if (!RTPS_Bitmap_set_bit(&writer->bitmap, NULL,
                                     &msg->data_batch.batch_sn,
                                     RTI_FALSE))
            {
                RTPS_LOG_BITMAP_SET_BIT(OSAPI_LOGKIND_INFO)
                return RTI_TRUE;
            }
        }
        else
#endif /* RTPS_RELIABILITY */
        {
            /* Upstream reception was successful. Call bitmap shift(), which
             *  will advance bitmap as necessary if first unaccepted SN has
             *  advanced.
             */
            if (!RTPS_Bitmap_shift(&writer->bitmap, &first_unaccepted_sn))
            {
                RTPS_LOG_SHIFT_BITMAP(OSAPI_LOGKIND_ERROR)
            }

            /* Update accepted SN */
            REDA_SequenceNumber_max(&writer->highest_accepted_sn,
                                    &writer->highest_accepted_sn,
                                    &msg->data_batch.batch_sn);
        }
    }

    return RTI_TRUE;

#undef RTPS_DATA_BATCH_SAMPLEINFO_HEADER_LENGTH
#undef RTPS_TIMESTAMP_LENGTH
}
#endif /* RTPS_DATA_BATCH_ENABLED */

/*ci
 * \brief
 * Verify validity of received submessage
 *
 * \param[in] msg Received submessage
 * \param[inout] supported_submsg Whether submessage is supported.
 * \param[in] packet Packet containing received submessage
 *
 * \return RTI_TRUE on received submessage being verified as valid, otherwise
 * RTI_FALSE.  known_submsg set RTI_TRUE if msg->submsg.kind is a supported
 * submessage, otherwise set RTI_FALSE.
 */
RTI_PRIVATE RTI_BOOL
RTPS_Receiver_valid_submessage(union RTPS_MESSAGES *msg,
                               RTI_BOOL *supported_submsg,
                               NETIO_Packet_T *packet)
{
#if RTPS_DATA_BATCH_ENABLED
#define RTPS_BATCH_IS_VALID_SUBMSG \
    || \
    msg->submsg.kind == RTPS_HEARTBEAT_BATCH_KIND || \
    msg->submsg.kind == RTPS_DATA_BATCH_KIND
#else
#define RTPS_BATCH_IS_VALID_SUBMSG
#endif

    /* Function never called with supported_submsg being NULL */
    if (msg->submsg.kind == RTPS_PAD_KIND ||
        msg->submsg.kind == RTPS_ACKNACK_KIND || 
        msg->submsg.kind == RTPS_HEARTBEAT_KIND ||
        msg->submsg.kind == RTPS_GAP_KIND ||
        msg->submsg.kind == RTPS_INFO_TS_KIND ||
        msg->submsg.kind == RTPS_INFO_DST_KIND ||
        msg->submsg.kind == RTPS_DATA_KIND
        RTPS_BATCH_IS_VALID_SUBMSG )
    {
        *supported_submsg = RTI_TRUE;
    }
    else
    {   
        *supported_submsg = RTI_FALSE;
        return RTI_FALSE;
    }

    /* Submsg length + submsg header (4 bytes) must fit in rest of packet */
    if (((RTI_SIZE_T)(msg->submsg.length + RTPS_SUBMSG_HEADER_LEN)) > 
        NETIO_Packet_get_payload_length(packet))
    {
        return RTI_FALSE;
    }

    /* Positive submsg length must meet minimum, check for standard messages
     * Note: 0-length submessage is allowed
     */
    if (!RTPS_Submsg_is_kind_vendor(msg->submsg.kind) &&
        (msg->submsg.length < RTPS_fv_SubMsgMinLen[msg->submsg.kind]))
    {
        return RTI_FALSE;
    }

    /* ACKNACK submsg must not exceed maximum length */
    if ((msg->submsg.kind == RTPS_ACKNACK_KIND) && 
        (msg->submsg.length > RTPS_ACKNACK_SUBMSG_MAX_LEN))
    {
        return RTI_FALSE;
    }

    /* GAP submsg must not exceed maximum length */
    if ((msg->submsg.kind == RTPS_GAP_KIND) && 
        (msg->submsg.length > RTPS_GAP_SUBMSG_MAX_LEN))
    {
        return RTI_FALSE;
    }

    /* INFO_TS submsg with valid timestamp must be exact length */
    if ((msg->submsg.kind == RTPS_INFO_TS_KIND) && 
        ((msg->submsg.flags & RTPS_INFO_TSFLAGS_I) == 0) &&
        (msg->submsg.length != RTPS_INFO_TS_SUBMSG_MAX_LEN))
    {
        return RTI_FALSE;
    }

     /* INFO_DST submsg must not exceed exact length */
    if ((msg->submsg.kind == RTPS_INFO_DST_KIND) && 
        (msg->submsg.length != RTPS_SUBMSG_MIN_LEN_INFO_DST))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


/*ci
 * \brief
 * Process an RTPS message, and its constituent submessages, received from 
 * a downstream transport
 *  
 * \details 
 * This function is called for an RTPS external interface when a downstream 
 * transport has passed up a received message.  The function validates the 
 * message as RTPS, and parses and processes each individual RTPS submessage, 
 * passes each submessage to the appropriate destination readers or reliable 
 * writers.  
 *  
 * \param[in] netio_intf Self RTPS external interface 
 * \param[in] source Downstream transport's address 
 * \param[in] dst Destination RTPS interface's address 
 * \param[in] packet Received packet containing RTPS message
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_receive(NETIO_Interface_T *netio_intf, /* self's external intf */
                       struct NETIO_Address *source, /* downstream addr */
                       struct NETIO_Address *dst, /* self addr  */
                       NETIO_Packet_T *packet)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    RTI_BOOL ok = RTI_FALSE;
    struct RTPS_ExtBindEntry *bind_entry = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T db_rc;
    RTI_BOOL byte_swap;
    union RTPS_MESSAGES *rtps_msg = NULL;
    union RTPS_MESSAGES tmp_msg;
    char *msg_ptr = NULL; 
    struct RTPS_ExtBindEntry bind_key_low, bind_key_high;
    struct NETIO_GuidEntity OID_MAX = {{0xff,0xff,0xff,0xff}};
    struct NETIO_GuidEntity OID_ZERO = {{0,0,0,0}};
    RTI_BOOL is_SDPP_sender, is_SDPP_rcver; /* SDPP: Simple Discovery Protocol Participant */
    RTI_SIZE_T submsg_offset, submsg_length, orig_submsg_length;
    RTI_SIZE_T pkt_head, pkt_tail;
    RTI_SIZE_T rtps_msg_head = 0, rtps_msg_tail = 0;
    RTI_BOOL valid_msg = RTI_FALSE;
    OSAPI_NtpTime TS_ZERO = OSAPI_NTP_TIME_ZERO;
    RTI_BOOL supported_submsg = RTI_FALSE;
    RTI_BOOL msg_dst_is_self = RTI_TRUE;
    RTI_SIZE_T payload_length = 0; 
    RTI_BOOL is_vendor_rti = RTI_FALSE;
    RTI_SIZE_T msg_length = 0;
    RTI_SIZE_T unaligned_submsg_length = 0;

    OSAPI_TRACE_ONLY_VARIABLE(source);
    OSAPI_TRACE_ONLY_VARIABLE(dst);

    OSAPI_TRACE_NET("forward:",RTI_FALSE)
    OSAPI_TRACE_INT32("length",NETIO_Packet_get_payload_length(packet),RTI_FALSE)
    OSAPI_TRACE_INT32("intf.port",source->port,RTI_FALSE)
    OSAPI_TRACE_GUID("intf.address",&source->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst_addr.port",dst->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst_addr.address",&dst->value.rtps_guid,RTI_TRUE)

    /* bad param check */
    if (netio_intf == NULL || packet == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* receive only over external intf */
    if (intf->property.mode != RTPS_INTERFACEMODE_EXTERNAL_RECEIVER)
    {
        /* Defensive check, should not happen */
        RTPS_LOG_UNSUPPORTED_INTERFACE(OSAPI_LOGKIND_WARNING)
        ok = RTI_TRUE; /* dropped msg is not fatal */
        goto done;
    }

    /* intf must be enabled to receive */
    if (intf->_parent.state != NETIO_INTERFACESTATE_ENABLED)
    {
        RTPS_LOG_INTERFACE_NOT_ENABLED(OSAPI_LOGKIND_WARNING)
        ok = RTI_TRUE; /* dropped msg is not fatal */
        goto done;
    }

    packet->info.protocol_id = NETIO_PROTOCOL_RTPS;
    packet->info.timestamp = TS_ZERO;
    packet->info.checksum_info = CDR_STREAM_CHECKSUM_NONE;

    /* Process each message and its submessages in packet */
    while (NETIO_Packet_get_payload_length(packet) > 0)
    {
        OSAPI_TRACE_NET("processing payload length:",RTI_FALSE)
        OSAPI_TRACE_INT32("length",NETIO_Packet_get_payload_length(packet),RTI_TRUE)

        if (!valid_msg && (msg_length != 0))
        {
            /* Because the current message was invalidated, jump to the next
             * potential message or stop processing if that is not possible.
             * If valid_msg is false and msg_length == 0, valid_msg will remain
             * false ,unless a 'RTPS' happened to be at the current location,
             * and processing will stop before the next submessage is processed.
             */
            msg_length = OSAPI_Compiler_align_unsigned(msg_length,
                                           RTPS_SUBMESSAGE_LENGTH_BYTE_ALIGN);

            NETIO_Packet_restore_positions_from(
                    packet,rtps_msg_head,rtps_msg_tail);

            if (!NETIO_Packet_set_head(packet, (RTI_INT32)msg_length))
            {
                /* This is not an error since there may not be any
                 * more RTPS messages in the payload, this is ok.
                 */
                goto done;
            }
            msg_length = 0;
        }

        /* A check for 'RTPS' is only done at expected locations in the
         * payload, starting at the beginning of the payload. A new RTPS
         * may start after processing a submessage, thus always check
         * at the beginning of the iteration.
         */
        rtps_msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
        if ((NETIO_Packet_get_payload_length(packet) >= RTI_SIZEOF(rtps_msg->header.rtps)) &&
            rtps_msg->header.rtps == VALID_RTPS_HEADER)
        {
            msg_length = 0;
            valid_msg = RTI_FALSE;

            payload_length = NETIO_Packet_get_payload_length(packet);

            if ((payload_length >= sizeof(struct RTPS_Header)) &&
                    (rtps_msg->header.protocol_version.major == RTPS_PROTOCOL_VERSION_MAJOR) &&
                    (rtps_msg->header.protocol_version.minor >= RTPS_PROTOCOL_VERSION_MINOR))
            {
                NETIO_Packet_save_positions_to(packet,&rtps_msg_head,&rtps_msg_tail);

                /* initialize receive context */
                intf->context.src.prefix = rtps_msg->header.guid_prefix;
                intf->context.sample_rejected_count = 0;

                /* context.dst defaults to reader's participant's GUID prefix */
                OSAPI_Memory_zero(&intf->context.dst.entity,
                                  sizeof(struct NETIO_GuidEntity));
                OSAPI_Memory_copy(&intf->context.dst.prefix,
                                  &intf->_parent.local_address.value.guid.prefix,
                                  sizeof(struct NETIO_GuidPrefix));
                msg_dst_is_self = RTI_TRUE;

                packet->info.protocol_data.rtps_data.guid_prefix =
                        rtps_msg->header.guid_prefix;

                /* Micro is 1.10, Core is 1.1
                 */
                is_vendor_rti = (rtps_msg->header.vendor_id.value[0] == RTPS_VENDOR_ID_MAJOR) &&
                        ((rtps_msg->header.vendor_id.value[1] == 1) ||
                                (rtps_msg->header.vendor_id.value[1] == RTPS_VENDOR_ID_MINOR));

                packet->info.vendor_major_id = rtps_msg->header.vendor_id.value[0];
                packet->info.vendor_minor_id = rtps_msg->header.vendor_id.value[1];
                packet->info.checksum_info = CDR_STREAM_CHECKSUM_NONE;

                /* RTPS header is fixed size, it is safe to move past it
                 * regardless whether the message was corrupted or not.
                 */
                if (!NETIO_Packet_set_head(packet, sizeof(struct RTPS_Header)))
                {
                    RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
                    goto done;
                }

                /* A valid RTPS message header has been found, but no state
                 * transitions has been performed.  Check if checksum is
                 * required and if it is present. A checksum _must_ follow
                 * the RTPS header in the form of either a header extension
                 * or a Core CRC submessage. If neither is found, but a
                 * checksum is required drop the message. If it is found
                 * then validate if enabled, otherwise skip it.
                 */
                if (RTPS_Receive_is_msg_corrupted(intf,packet,rtps_msg,
                                                  payload_length,
                                                  is_vendor_rti,&msg_length))
                {
                    RTPS_LOG_CHECKSUM_CHECKSUM_ERROR(OSAPI_LOGKIND_WARNING)
                    /* The message was dropped due to a missing checksum,
                     * invalid checksum, or invalid header. Search for
                     * next RTPS message. If the message length is known,
                     * search for the next message on the next iteration,
                     * otherwise drop the remainder of the payload.
                     */
                    if (msg_length != 0)
                    {
                        continue;
                    }
                    /* If the length is not available, do not look for
                     * more RTPS messages.
                     */
                    goto done;
                }

                /* Valid RTPS message */
                valid_msg = RTI_TRUE;

                /* If the code gets to this point it is assumed that the
                 * packet is starting at the next valid submsg. If there is no
                 * submsg following, then the next test for submsg will fail.
                 */
                rtps_msg = (union RTPS_MESSAGES *)NETIO_Packet_get_head(packet);
            }
        }

        if (!valid_msg)
        {
            /* The beginning of a new RTPS message has not been found or the
             * message being processed is not a valid RTPS message, so stop
             * processing.
             */
            goto done;
        }

        /* A valid RTPS header and a checksum check (if performed) has validated
         * the message. Continue processing as normal and
         * verify enough buffer for submessage header. Note that the current
         * offset is not checked againts the message length since it may not
         * be available.
         */
        if (NETIO_Packet_get_payload_length(packet) < sizeof(struct RTPS_SubmsgHdr))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            goto done;
        }

        /* parse submessage header: kind, flags, length */
        tmp_msg.submsg.kind = rtps_msg->submsg.kind;
        tmp_msg.submsg.flags = rtps_msg->submsg.flags;
        byte_swap = RTPSInterface_byte_swap(tmp_msg.submsg.flags);

        /* It is important to reset the rtps_flags. In case that an RTPS message
         * contains several submessages we do not want to use the flags from the
         * previous message
         */
        packet->info.rtps_flags = NETIO_RTPS_FLAGS_DEFAULT;

        if ((tmp_msg.submsg.flags & RTPS_ENDIAN_FLAG) != 0)
        {
            packet->info.rtps_flags |= NETIO_RTPS_FLAGS_LITTLE_ENDIAN;
        }

        /* Set pointer to submessage's length field, and starting from here,
         * use it to deserialize submessage, field by field
         */
        msg_ptr = (char *)&rtps_msg->submsg.length;

        CDR_deserialize_unsigned_short(&msg_ptr, &tmp_msg.submsg.length,
                                      byte_swap);
        orig_submsg_length = (RTI_SIZE_T)tmp_msg.submsg.length;
        submsg_length = (RTI_SIZE_T)tmp_msg.submsg.length;

        if (submsg_length > NETIO_Packet_get_payload_length(packet))
        {
            RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(OSAPI_LOGKIND_WARNING)
            goto done;
        }

        /* Handle zero-length submessages */
        if ((submsg_length == 0) &&
            ((tmp_msg.submsg.kind != RTPS_INFO_TS_KIND) &&
             (tmp_msg.submsg.kind != RTPS_PAD_KIND)))
        {
            /* length is the remaining bytes in the payload. It is assumed
             * the will not be truncated.
             */
            submsg_length = NETIO_Packet_get_payload_length(packet);
            unaligned_submsg_length = submsg_length;

            /* Set the submessage length to the minimum required to be considered
             * valid. tmp_msg.submsg.length is not used later.
             */
            tmp_msg.submsg.length = RTPS_fv_SubMsgMinLen[tmp_msg.submsg.kind];
        }
        else
        {
            /* account for submsg header */
            submsg_length += RTPS_SUBMESSAGE_HEADER_LENGTH;
            unaligned_submsg_length = submsg_length;

            if (submsg_length % RTPS_SUBMESSAGE_LENGTH_BYTE_ALIGN)
            {
                RTI_SIZE_T aligned_submsg_length;

                /* The RTPS spec. allows the last submessage to have a length
                 * that is not a multiple of 4. However, since the actual
                 * UDP payload length may be different from the effective payload
                 * length, e.g. multiple RTPS messages are sent in the same UDP
                 * payload, it is not always possible to determine if this is the
                 * last submessage or not. Thus, align up to the next 4 bytes
                 * if possible, otherwise leave submsg_length as is since a
                 * new RTPS message cannot possibly follow.
                 */
                aligned_submsg_length = OSAPI_Compiler_align_unsigned(submsg_length,
                                                 RTPS_SUBMESSAGE_LENGTH_BYTE_ALIGN);

                if (NETIO_Packet_get_payload_length(packet) >= aligned_submsg_length)
                {
                    submsg_length = aligned_submsg_length;
                }
            }
        }

        /* Check valid deserialized submsg length and kind */
        if (!RTPS_Receiver_valid_submessage(&tmp_msg, &supported_submsg, packet))
        {
            if (supported_submsg)
            {
                /* An invalid known submessage invalidates the rest of the message
                 * that contains it.  Ignore following submessages until end of
                 * packet or start of next message.
                 */
                valid_msg = RTI_FALSE;
                continue;
            }
        }

        if ((msg_length != 0) &&
            ((packet->head_pos - rtps_msg_head + unaligned_submsg_length) > msg_length))
        {
            /* The submessage is outside the RTPS message boundary as determined
             * by the RTPS message length. Invalidate the rest of the
             * RTPS message.
             */
            RTPS_LOG_MESSAGE_EXCEED_LENGTH_ERROR(OSAPI_LOGKIND_WARNING)
            valid_msg = RTI_FALSE;
            continue;
        }

        /* skip PAD and unsupported submessages. Note that if the message
         * is invalid (supported, but invalid submessage, the processing
         * does not reach here.
         */
        if (!supported_submsg || (tmp_msg.submsg.kind == RTPS_PAD_KIND))
        {
            if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_length))
            {
                RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
                goto done;
            }
            continue;
        }

        /* Handle INFO submsgs */
        if (RTPS_Interface_submessage_is_info(tmp_msg.submsg.kind))
        {
            switch (tmp_msg.submsg.kind)
            {
             case RTPS_INFO_DST_KIND:

                OSAPI_Memory_copy(&intf->context.dst.prefix,
                                  &rtps_msg->info_dst.guid_prefix,
                                  sizeof(struct NETIO_GuidPrefix));

                msg_dst_is_self =
                    (OSAPI_Memory_compare(intf->context.dst.prefix.value,
                        intf->_parent.local_address.value.guid.prefix.value,
                        sizeof(struct NETIO_GuidPrefix)) == 0) ? RTI_TRUE : RTI_FALSE;

                break;

             case RTPS_INFO_TS_KIND:

                if ((tmp_msg.submsg.flags & RTPS_INFO_TSFLAGS_I) == 0)
                {
                    CDR_deserialize_long(
                       &msg_ptr, &packet->info.timestamp.sec, byte_swap);
                    CDR_deserialize_unsigned_long(
                       &msg_ptr, &packet->info.timestamp.frac, byte_swap);
                }
                else
                {
                    /* Invalidate flag = 1 means context's timestamp
                     * must be invalidated (set to zero)
                     */
                    packet->info.timestamp = TS_ZERO;
                }
                break;

             /*  No default case needed, all accounted for by
                 RTPSSubmessage_is_info() */
            }
            if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_length))
            {
                RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
                goto done;
            }
            continue;
        }

        /* Skip submessages not destined for self (changed by INFO_DST) */
        if (!msg_dst_is_self)
        {
            if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_length))
            {
                RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
                goto done;
            }
            continue;
        }

        /* Get DATA inlineQos, extraFlags, seq num */
#if RTPS_DATA_BATCH_ENABLED
        /* A warning about extraneous parentheses is given if
         * (tmp_msg.submsg.kind == RTPS_DATA_BATCH_KIND) is excluded,
         * thus two different if tests.
         */
        if ((tmp_msg.submsg.kind == RTPS_DATA_KIND)
            || (tmp_msg.submsg.kind == RTPS_DATA_BATCH_KIND))
#else
        if (tmp_msg.submsg.kind == RTPS_DATA_KIND)
#endif
        {
            /* flags and qos_offset have the same location in both
             * data and data_batch messsages. this is why we can
             * use the data submessage to store for both data
             * and data_batach submessages
             */
            CDR_deserialize_unsigned_short(
               &msg_ptr, &tmp_msg.data.flags, byte_swap);
            CDR_deserialize_unsigned_short(
               &msg_ptr, &tmp_msg.data.qos_offset, byte_swap);
        }

        /* Update context src/dst from submsg */
        if (RTPS_Interface_submessage_is_from_writer(tmp_msg.submsg.kind))
        {
            /* msg_ptr points to the payload buffer and checks are
             * performed to make sure it is within bounds before it is
             * accessed. Thus, the Coverity event [overrun-buffer-arg] is
             * marked as 'Intentional'.
             */
            /* coverity[overrun-buffer-arg] */
            OSAPI_Memory_copy(&intf->context.dst.entity, msg_ptr,
                              RTPS_ENTITY_ID_LENGTH);
            msg_ptr += RTPS_ENTITY_ID_LENGTH;
            OSAPI_Memory_copy(&intf->context.src.entity, msg_ptr,
                              RTPS_ENTITY_ID_LENGTH);
            msg_ptr += RTPS_ENTITY_ID_LENGTH;
        }
        else
        {
            OSAPI_Memory_copy(&intf->context.src.entity, msg_ptr,
                              RTPS_ENTITY_ID_LENGTH);
            msg_ptr += RTPS_ENTITY_ID_LENGTH;
            OSAPI_Memory_copy(&intf->context.dst.entity, msg_ptr,
                              RTPS_ENTITY_ID_LENGTH);
            msg_ptr += RTPS_ENTITY_ID_LENGTH;
        }

        packet->source.value.guid = intf->context.src;
        packet->source.kind = NETIO_ADDRESS_KIND_INTRA;
        packet->source.port = 0;

        if (tmp_msg.submsg.kind == RTPS_DATA_KIND)
        {
            RTPS_SequenceNumber_deserialize(&msg_ptr,&packet->info.sn,byte_swap);
            packet->info.virtual_sn = packet->info.sn;

            submsg_offset = RTPS_RCV_DATA_SUBMSG_OFFSET;
        }
#if RTPS_DATA_BATCH_ENABLED
        else if (tmp_msg.submsg.kind == RTPS_DATA_BATCH_KIND)
        {
            RTPS_SequenceNumber_deserialize(
                &msg_ptr,&tmp_msg.data_batch.batch_sn,byte_swap);
            RTPS_SequenceNumber_deserialize(
                &msg_ptr, &tmp_msg.data_batch.first_sample_sn, byte_swap);
            CDR_deserialize_unsigned_long(
                &msg_ptr, &tmp_msg.data_batch.offset_last_sn, byte_swap);
            CDR_deserialize_unsigned_long(
                &msg_ptr, &tmp_msg.data_batch.batch_sample_count, byte_swap);
            /* In case there are inline qos in the batch skip them.
             * We only support key hash, status info and sentinel,
             * and those are on the inline_qos in the sampleInfo,
             * so we can just skip any inline QoS here.
             */
           if (packet->info.rtps_flags & RTPS_DATAFLAGS_Q)
           {
                if (!RTPS_Receiver_process_skip_inline_qos(packet, 
                                                           byte_swap, 
                                                           &submsg_offset))
                {
                    goto done;
                }
           }
           else
           {
               submsg_offset = 0;
           }
           CDR_deserialize_unsigned_long(
                &msg_ptr, &tmp_msg.data_batch.encapsulation_offset, byte_swap);

           submsg_offset += RTPS_RCV_DATA_BATCH_SUBMSG_OFFSET;
        }
#endif /* RTPS_DATA_BATCH_ENABLED */
        else
        {
            /* The number of bytes deserialized thus far for this submessage is
             * the same (submessage header + writer entityId + reader entityId)
             * except for a DATA and DATA_BATCH submessages that has additional
             * fields.
             */
            submsg_offset = RTPS_RCV_NON_DATA_SUBMSG_OFFSET;
        }

        /* save positions after deserializing submsg headers, entity IDs; for
         * DATA, also deserialized extra flags, inlineQoSOffset, and seq num.
         */
        if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_offset))
        {
            RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);
        submsg_length -= submsg_offset;

        /* Given this msg is destined for a builtin participant discovery
         * reader, ignore participant discovery messages sent from myself
         */
        if ((intf->context.dst.entity.value[RTPS_ENTITY_KIND_INDEX] &
             RTPS_OBJECT_RESERVED_META_UNKNOWN) &&
             !OSAPI_Memory_compare(intf->context.src.prefix.value,
                            intf->_parent.local_address.value.guid.prefix.value,
                            sizeof(struct NETIO_GuidPrefix)))
        {
            /* Quietly drop rest of message */
            ok = RTI_TRUE;
            goto done;
        }

        is_SDPP_sender =
            RTPSInterface_addr_is_SDP_Participant_sender(&intf->context.src);
        if (is_SDPP_sender)
        {
            /* from stateless writer --> received by all stateless readers of
             * corresponding entity ID
             */
            db_rc = DB_Table_select_all_default(intf->_parent._btable, &cursor);
        }
        else
        {
            bind_key_low.source = intf->context.src;
            bind_key_low.destination = intf->context.dst.entity;

            /* Not all fields in bind_key_low are used in a comparison. Thus,
             * the Coverity event [uninit_use] is marked as 'Intentional'.
             */
            /* coverity[uninit_use] */
            bind_key_high = bind_key_low;

            /* Search full range if dest id is unknown */
            if (RTPS_Interface_is_unknown_entity(&intf->context.dst.entity))
            {
                /* Full range of destination addresses starts from low key of
                 * all zero ...
                 */
                bind_key_low.destination = OID_ZERO;

                /* ... to high key of all max. */
                bind_key_high.destination = OID_MAX;
            }

            db_rc = DB_Table_select_range(
               intf->_parent._btable, DB_TABLE_DEFAULT_INDEX, &cursor,
               (DB_Key_T)&bind_key_low, (DB_Key_T)&bind_key_high);
        }

        if (db_rc != DB_RETCODE_OK)
        {
            RTPS_LOG_DB_SELECT_RANGE(OSAPI_LOGKIND_INFO, db_rc)

            if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_length))
            {
                RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
                goto done;
            }
            continue;
        }

        /* Forward submessage to each selected destination */
        db_rc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
        while (db_rc == DB_RETCODE_OK)
        {
            if (bind_entry->_rtps_intf == NULL)
            {
                /* already deleted, skip */
                goto nextEntry;
            }

            is_SDPP_rcver = RTPSInterface_addr_is_SDP_Participant_receiver(
               &bind_entry->_rtps_intf->_parent.local_address.value.guid);

            /* skip this entry if:
             *  - interface is not enabled
             *   - stateless is talking with non-stateless
             */
            if ((bind_entry->_rtps_intf->_parent.state !=
                 NETIO_INTERFACESTATE_ENABLED) ||
                (is_SDPP_sender ^ is_SDPP_rcver))
            {
                goto nextEntry;
            }

            /* Each invalid submessage below will invalidate rest of its
             * containing message.
             */
            switch (tmp_msg.submsg.kind)
            {
             case RTPS_ACKNACK_KIND:

                if (!RTPS_Receiver_process_acknack(bind_entry->_rtps_intf,
                            bind_entry->peer_ref,msg_ptr,tmp_msg.submsg.flags,
                            orig_submsg_length,byte_swap))
                {
                    RTPS_LOG_PROCESS_ACKNACK(OSAPI_LOGKIND_ERROR)
                    valid_msg = RTI_FALSE;
                }
                break;

             case RTPS_DATA_KIND:

                 if (!RTPS_Receiver_process_data(bind_entry->_rtps_intf,
                        bind_entry->peer_ref,packet,tmp_msg.submsg.flags,
                        submsg_length,byte_swap))
                 {
                     RTPS_LOG_PROCESS_DATA(OSAPI_LOGKIND_ERROR)
                     valid_msg = RTI_FALSE;
                 }
                break;

#if RTPS_DATA_BATCH_ENABLED
             case RTPS_DATA_BATCH_KIND:

                 if (!RTPS_Receiver_process_data_batch(bind_entry->_rtps_intf,
                        bind_entry->peer_ref,packet,&tmp_msg,
                        submsg_length,byte_swap))
                 {
                     RTPS_LOG_PROCESS_DATA_BATCH(OSAPI_LOGKIND_ERROR)
                     valid_msg = RTI_FALSE;
                 }
                break;
#endif /* RTPS_DATA_BATCH_ENABLED */

             case RTPS_GAP_KIND:

                if (!RTPS_Receiver_process_gap(bind_entry->_rtps_intf,
                                        bind_entry->peer_ref,packet,msg_ptr,
                                        orig_submsg_length,byte_swap))
                {
                    RTPS_LOG_PROCESS_GAP(OSAPI_LOGKIND_ERROR)
                    valid_msg = RTI_FALSE;
                }
                break;

             case RTPS_HEARTBEAT_KIND:

                if (!RTPS_Receiver_process_heartbeat(bind_entry->_rtps_intf,
                        bind_entry->peer_ref,msg_ptr,
#if RTPS_DATA_BATCH_ENABLED
                        tmp_msg.submsg.kind,
#endif
                        tmp_msg.submsg.flags,byte_swap))
                {
                    RTPS_LOG_PROCESS_HEARTBEAT(OSAPI_LOGKIND_ERROR)
                    valid_msg = RTI_FALSE;
                }
                break;

#if RTPS_DATA_BATCH_ENABLED
             case RTPS_HEARTBEAT_BATCH_KIND:

                if (!RTPS_Receiver_process_heartbeat(bind_entry->_rtps_intf,
                        bind_entry->peer_ref,msg_ptr,tmp_msg.submsg.kind,
                        tmp_msg.submsg.flags,byte_swap))
                {
                    RTPS_LOG_PROCESS_HEARTBEAT_BATCH(OSAPI_LOGKIND_ERROR)
                    valid_msg = RTI_FALSE;
                }
                break;
#endif /* RTPS_DATA_BATCH_ENABLED */

             default:
                /* ignore and warn about unknown submessages */
                RTPS_LOG_UNKNOWN_SUBMESSAGE(OSAPI_LOGKIND_WARNING)
                break;
            }
 nextEntry:
            NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);
            db_rc = DB_Cursor_get_next(cursor, (DB_Record_T*)&bind_entry);
        }
        DB_Cursor_finish(intf->_parent._btable, cursor);

        if (!NETIO_Packet_set_head(packet, (RTI_INT32)submsg_length))
        {
            RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_WARNING)
            goto done;
        }
    }
    ok = RTI_TRUE;

done:

    return ok;
}

/*ci
 * \brief
 * Periodic event that sends periodic pre-emptive ACKNACK submessages
 *
 * \details
 * A reliable reader send a pre-emptive ACKNACK to trigger a writer
 * to send a HB with the initial state of the reader.
 *
 * \param[in] storage Event storage containing self pointer
 *
 * \return OSAPI_TIMEOUT_OP_AUTOMATIC upon periodic rescheduling of
 * event, OSAPI_TIMEOUT_OP_MANUAL upon stopping periodic rescheduling
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
RTPS_Reader_on_periodic_acknack(struct OSAPI_TimeoutUserData *storage)
{
#if RTPS_RELIABILITY
    OSAPI_TimeoutOp_t result = OSAPI_TIMEOUT_OP_AUTOMATIC;
    struct RTPS_Reader *reader = NULL;
    struct RTPS_Interface *intf = (struct RTPS_Interface *)storage->field[0];
    RTI_INT32 i;
    RTI_INT32 peers_count = 0;
    struct RTPS_PeerEntry *peer = NULL;
    struct RTPS_Bitmap bitmap;
    RTPS_SampleId_T SN_ZERO = REDA_SEQUENCE_NUMBER_ZERO;
    RTI_BOOL bretval;
    RTI_INT32 writer_count = 0;

    reader = &intf->endpoint.reader;

    if (intf->_parent.state != NETIO_INTERFACESTATE_ENABLED)
    {
        reader->flags &= ~RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE;
        result =  OSAPI_TIMEOUT_OP_MANUAL;
        goto done;
    }

    if (intf->peers_index == NULL)
    {
        goto done;
    }

    /* Check for writers the reader had not yet received a valid
     * HB from. When the reader is up to date with all remote writers
     * the ACKNACK timer is stopped.
     */
    peers_count = REDA_Indexer_get_count(intf->peers_index);
    RTPS_Bitmap_reset(&bitmap, &SN_ZERO, 0);

    for (i = 0; i < peers_count; ++i)
    {
        peer = (struct RTPS_PeerEntry*)
                        REDA_Indexer_get_entry(intf->peers_index, i);

        if (!peer->info.writer.rcvd_first_hb)
        {
            bretval = RTPS_Reader_send_acknack(intf, &bitmap,
                                &intf->endpoint.reader,peer, RTI_FALSE);
#if OSAPI_ENABLE_LOG
            if (!bretval)
            {
                RTPS_LOG_READER_SEND_ACKNACK(OSAPI_LOGKIND_ERROR)
                /* Don't return failure on failed preemptive ACKNACK */
            }
#else
            /* Don't return failure on failed preemptive HEARTBEAT */
            IGNORE_RETVAL(bretval);
#endif
            ++writer_count;
        }
    }

    if (writer_count == 0)
    {
        /* Do not reschedule the timeout if a valid HB has been received from
         * all writers
         */
        result = OSAPI_TIMEOUT_OP_MANUAL;
    }

done:

    return result;

#else
    UNUSED_ARG(storage);
    /* event should not be running, stop */
    return OSAPI_TIMEOUT_OP_MANUAL;
#endif
}

/*ci
 * \brief
 * Periodic event that sends HEARTBEAT submessages
 *
 * \details
 * A reliable writer is configured with a periodic HEARTBEAT duration.  This
 * is the event triggered once per period to send a HEARTBEAT to each
 * peer reader.
 *
 * \param[in] storage Event storage containing self pointer
 *
 * \return OSAPI_TIMEOUT_OP_AUTOMATIC upon periodic rescheduling of
 * event, OSAPI_TIMEOUT_OP_MANUAL upon stopping periodic rescheduling
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
RTPS_Writer_on_periodic_heartbeat(struct OSAPI_TimeoutUserData *storage)
{
#if RTPS_RELIABILITY
    OSAPI_TimeoutOp_t result = OSAPI_TIMEOUT_OP_AUTOMATIC;
    struct RTPS_Writer *writer = NULL;
    struct RTPS_Interface *intf = (struct RTPS_Interface *)storage->field[0];
    RTPS_Entity_T writer_entity = RTPS_ENTITY_UNKNOWN;
    RTPS_Entity_T reader_entity = RTPS_ENTITY_UNKNOWN;
    RTI_INT32 i;
    struct NETIO_Event peer_event;
    RTPS_SampleId_T tmp_sn;
    RTI_BOOL removed;
    RTI_INT32 peers_count = 0;
    struct RTPS_PeerEntry *peer = NULL;
    struct NETIO_Address peer_addr;

    writer = &intf->endpoint.writer;

    if (intf->_parent.state != NETIO_INTERFACESTATE_ENABLED)
    {
        writer->flags &= ~RTPS_WRITER_FLAG_HB_EVENT_ACTIVE;
        result =  OSAPI_TIMEOUT_OP_MANUAL; /* don't reschedule */
        goto done;
    }

    /* don't send HB if queue is empty */
    if (REDA_SequenceNumber_is_zero(&writer->first_sn))
    {
        goto done;
    }

    writer_entity = intf->_parent.local_address.value.guid.entity;

    /* Check for inactive Readers */
    peers_count = REDA_Indexer_get_count(intf->peers_index);

    for (i = 0; i < peers_count; ++i)
    {
        peer = (struct RTPS_PeerEntry *)
            REDA_Indexer_get_entry(intf->peers_index, i);

        /* Always reset repair_stuck_nack flag */
        peer->info.reader.repair_stuck_nack = RTI_TRUE;
        NETIO_Address_set_guid(&peer_addr,0,&peer->addr);

        /* Skip deleted or fully acknowledged readers */
        if (REDA_SequenceNumber_compare(
           &peer->info.reader.last_acked_sn, &writer->last_sn) >= 0)
        {
            continue;
        }

        if (intf->property.max_hb_retries > 0)
        {
            if ((peer->info.reader.inactive_count == 0) &&
                !peer->info.reader.is_inactive)
            {
                /* By not responding with ACKNACKs to periodic HEARTBEATs,
                   this remote reader has gone Inactive.

                   All of its outstanding samples must be acknowledged, and then
                   upstream needs to be notified of its change to inactivity
                */

                /* NACK upstream each sample in window */
                tmp_sn = peer->info.reader.last_acked_sn;
                REDA_SequenceNumber_plusplus(&tmp_sn);

                while ((peer->info.reader.window.size > 0) &&
                      (REDA_SequenceNumber_compare(&writer->last_sn,
                                                   &tmp_sn) >= 0))
                {
                    RTPS_Window_remove(&peer->info.reader.window,
                                       &removed, &tmp_sn);
                    if (removed)
                    {
                        if (!NETIO_Interface_acknack(writer->upstr_intf,
                                                &peer_addr, &tmp_sn, RTI_TRUE))
                        {
                            RTPS_LOG_ACK(OSAPI_LOGKIND_ERROR)
                            goto done;
                        }
                    }
                    REDA_SequenceNumber_plusplus(&tmp_sn);
                }

                /* Notify upstream of newly inactivated reader */
                peer->info.reader.is_inactive = RTI_TRUE;

                --writer->active_reliable_reader_count;
                ++writer->inactive_reliable_reader_count;

                peer_event.kind = NETIO_EVENTKIND_INACTIVE_PEER;

                NETIO_Address_set_guid(&peer_event.value.peer_activity.peer_addr,0,
                                       &peer->addr);

                peer_event.value.peer_activity.active_change = -1;
                peer_event.value.peer_activity.active_total =
                    writer->active_reliable_reader_count;
                peer_event.value.peer_activity.inactive_change = 1;
                peer_event.value.peer_activity.inactive_total =
                    writer->inactive_reliable_reader_count;

                if (!NETIO_Interface_post_event(writer->upstr_intf,
                                                &intf->_parent,
                                                &peer_event))
                {
                    RTPS_LOG_STATUS_CHANGE(OSAPI_LOGKIND_ERROR)
                    goto done;
                }
            }
            else
            {
                --peer->info.reader.inactive_count;
            }
        }

        /* Send direct HEARTBEAT */
        if (!RTPS_Interface_initialize_packet(intf))
        {
            RTPS_LOG_PACKET_INITIALIZE(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        reader_entity = peer->addr.entity;

        if (!RTPS_Writer_direct_send(intf, RTPS_SEND_HB_FLAG,intf->packet,
                              peer, &writer_entity, &reader_entity, NULL, NULL))
        {
            RTPS_LOG_DIRECT_SEND(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

done:

    return result;

#else
    /* event should not be running, stop */
    return OSAPI_TIMEOUT_OP_MANUAL;
#endif
}

/*ci
 * \brief
 * Set state of RTPS Interface
 *
 * \details
 * Only valid state for RTPS interface to change to is
 * NETIO_INTERFACESTATE_ENABLED.
 *
 * \param[in] netio_intf Self interface
 * \param[in] state New state to transition
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_set_state(NETIO_Interface_T *netio_intf,
                         NETIO_InterfaceState_T state)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_Interface *ext_intf = NULL;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    RTI_INT32 sec = 0;
    RTI_UINT32 ms = 0;

    /* bad param check */
    if (netio_intf == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    intf->_parent.state = state;

    if (state == NETIO_INTERFACESTATE_ENABLED)
    {
        /* enable external interface */
        ext_intf = (struct RTPS_Interface *)REDA_Indexer_find_entry(
           intf->factory->ext_intf_index, &intf->_parent.local_address);
        if (ext_intf == NULL)
        {
            RTPS_LOG_FIND_EXTERNAL_INTERFACE(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
        ext_intf->_parent.state = NETIO_INTERFACESTATE_ENABLED;

#if RTPS_RELIABILITY
        /* activate periodic HB if necessary */
        if ((intf->property.mode == RTPS_INTERFACEMODE_WRITER) &&
            (intf->endpoint.writer.flags & RTPS_WRITER_FLAG_RELIABLE) &&
            !(intf->endpoint.writer.flags & RTPS_WRITER_FLAG_HB_EVENT_ACTIVE))
        {
            OSAPI_NtpTime_to_millisec(
                    &sec, &ms, &intf->endpoint.writer.hb_period);
            storage.field[0] = (void *)intf;

            if (!OSAPI_Timer_create_timeout(intf->property._parent._parent.timer,
                                           &intf->endpoint.writer.hb_event,
                                           sec,(RTI_INT32)(ms * RTPS_MS_TO_NS),
                                           OSAPI_TIMER_PERIODIC,
                                           RTPS_Writer_on_periodic_heartbeat,
                                           &storage))
            {
                RTPS_LOG_CREATE_HB_EVENT(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }
            intf->endpoint.writer.flags |= RTPS_WRITER_FLAG_HB_EVENT_ACTIVE;
        }
        else if ((intf->property.mode == RTPS_INTERFACEMODE_READER) &&
                 (intf->endpoint.reader.flags & RTPS_READER_FLAG_RELIABLE) &&
                 !(intf->endpoint.reader.flags & RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE))
        {
            storage.field[0] = (void *)intf;
            if (!OSAPI_Timer_create_timeout(intf->property._parent._parent.timer,
                    &intf->endpoint.reader.acknack_event,
                    intf->endpoint.reader.acknack_sec,
                    intf->endpoint.reader.acknack_nanosec,
                    OSAPI_TIMER_PERIODIC,
                    RTPS_Reader_on_periodic_acknack,
                    &storage))
            {
                RTPS_LOG_CREATE_HB_EVENT(OSAPI_LOGKIND_ERROR)
                                return RTI_FALSE;
            }
            intf->endpoint.reader.flags |= RTPS_READER_FLAG_ACKNACK_EVENT_ACTIVE;
        }
#endif
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_lookup_route
 *
 * \details
 *
 * Check if a NETIO interface has a route to a specific destination
 *
 * \param[in]  netio_intf   The source interface
 * \param[in]  dst_reader   The destination address
 * \param[in]  via_intf     The downstream interface
 * \param[in]  via_address  The address to pass to the downstream interface
 * \param[out] route_exists RTI_TRUE if the route existed, RTI_FALSE if not
 *
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_lookup_route(struct NETIO_Interface *netio_intf,
                            struct NETIO_Address *dst_reader,
                            struct NETIO_Interface *via_intf,
                            struct NETIO_Address *via_address,
                            RTI_BOOL *route_exists)
{
    struct RTPS_Interface *rtps_intf =
                                (struct RTPS_Interface *)netio_intf;
    struct RTPS_RouteEntry *route_entry = NULL;
    struct RTPS_RouteEntry route_key;
    DB_ReturnCode_T dbrc;

    route_key.destination = dst_reader->value.guid;
    route_key.intf = via_intf;
    route_key.intf_address = *via_address;

    *route_exists = RTI_FALSE;

    dbrc = DB_Table_select_match(rtps_intf->_parent._rtable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&route_entry,
                                 (DB_Key_T)&route_key);

    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        return RTI_FALSE;
    }

    if (dbrc == DB_RETCODE_OK)
    {
        *route_exists = RTI_TRUE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_post_event method
 *
 * \param[in] netio_intf The interface receiving the event
 * \param[in] src_intf   The interface sending the event
 * \param[in] evt        The event
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_Interface_post_event(NETIO_Interface_T *netio_intf,
                          NETIO_Interface_T *src_intf,
                          struct NETIO_Event *evt)
{
    struct RTPS_Interface *rtps_intf = (struct RTPS_Interface *)netio_intf;
    struct RTPS_PeerEntry *peer = NULL;
    UNUSED_ARG(src_intf);

    switch (evt->kind)
    {
    case NETIO_EVENTKIND_RESOURCES_FREED:
        peer = (struct RTPS_PeerEntry*)REDA_Indexer_find_entry(
                                            rtps_intf->peers_index,
                                            &evt->value.resources_freed.entity);
        if (peer != NULL)
        {
            peer->info.writer.reserved_count -= evt->value.resources_freed.count;
        }

        rtps_intf->endpoint.reader.reserved_count -= evt->value.resources_freed.count;

#if OSAPI_ENABLE_LOG
        if ((rtps_intf->endpoint.reader.reserved_count < 0) &&
                ((!rtps_intf->property.anonymous) &&
                 ((rtps_intf->endpoint.reader.flags & RTPS_READER_FLAG_RELIABLE) != 0)))
        {
            RTPS_LOG_NEGATIVE_RESERVATION_COUNT(OSAPI_LOGKIND_ERROR)
        }
#endif

        break;
    default:
        break;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *
 * RTPS Component Interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI RTPSInterface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    RTPS_Interface_send,                  /* send */
    RTPS_Interface_acknack,               /* acknack */
    RTPS_Interface_request,               /* request */
    RTPS_Interface_return_loan,           /* return_loan */
    RTPS_Interface_xmit_remove,           /* xmit_remove */
    RTPS_Interface_add_route,             /* add_route */
    RTPS_Interface_delete_route,          /* delete_route */
    NULL,                                 /* reserve_public_address */
    RTPS_Interface_bind,                  /* bind */
    RTPS_Interface_unbind,                /* unbind */
    RTPS_Interface_receive,               /* receive */
    RTPS_Interface_get_external_interface,/* get_external_interface */
    RTPS_Interface_bind_external,         /* bind_external */
    RTPS_Interface_unbind_external,       /* unbind_external */
    RTPS_Interface_set_state,             /* set_state */
    NULL,                                 /* release_address */
    NULL,                                 /* resolve_address */
    NULL,                                 /* get_route_table */
    RTPS_Interface_post_event,            /* post_event */
    RTPS_Interface_lookup_route
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief
 * Create a new RTPS interface
 *
 * \param[in] factory RTPS interface factory
 * \param[in] property
 * \param[in] listener
 *
 * \return On success, pointer to created RTPS interface.  Otherwise, NULL.
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
RTPS_InterfaceFactory_create_component(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentProperty *property,
        struct RT_ComponentListener *listener)
{
    struct RTPS_Interface *retval = NULL;

    if (factory == NULL || property == NULL)
    {
        RTPS_LOG_BAD_PARAMETER(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    retval = RTPS_Interface_create(
            (struct RTPS_InterfaceFactory*)factory,
            (const struct RTPS_InterfaceProperty *)property,
            (const struct NETIO_InterfaceListener *)listener);

    return (retval == NULL ? NULL : &retval->_parent._parent);
}

#ifndef RTI_CERT
/*ci
 * \brief
 * Delete an RTPS interface
 *
 * \param[in] factory RTPS interface factory
 * \param[in] component RTPS interface to delete
 *
 */
RTI_PRIVATE void
RTPS_InterfaceFactory_delete_component(struct RT_ComponentFactory *factory,
                                     RT_Component_T *component)
{
    struct RTPS_Interface *intf = (struct RTPS_Interface*)component;

    if (factory != NULL && component != NULL)
    {
        RTPS_Interface_delete(intf);
    }
}
#endif /* !RTI_CERT */

/*ci \brief Implementation of the NETIO_InterfaceI_get_property() function
 *
 * \param[in] factory The RTPS interface factory
 * \param[inout] property The property the RTPS factory was initialized with
 */
RTI_PRIVATE void
RTPS_InterfaceFactory_get_property(struct RT_ComponentFactory *factory,
                                   struct RT_ComponentFactoryProperty **property)
{
    struct RTPS_InterfaceFactory *rtps_factory = (struct RTPS_InterfaceFactory*)factory;

    if ((factory != NULL) && (property != NULL))
    {
        *property = (struct RT_ComponentFactoryProperty*)rtps_factory->property;
    }
}

/* Forward declaration */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
RTPS_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
/* Forward declaration */
RTI_PRIVATE void
RTPS_InterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */


LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI RTPS_InterfaceFactory_fv_Intf =
{
    RT_COMPONENT_FACTORY_ID_DEFAULT,
    RTPS_InterfaceFactory_initialize,
#ifndef RTI_CERT
    RTPS_InterfaceFactory_finalize,
#else
    NULL, /* RTPS_InterfaceFactory_finalize, */
#endif
    RTPS_InterfaceFactory_create_component,
#ifndef RTI_CERT
    RTPS_InterfaceFactory_delete_component,
#else
    NULL, /* RTPS_InterfaceFactory_delete_component, */
#endif
    NULL,
    RTPS_InterfaceFactory_get_property
};


/*ci
 * \brief RTPS NETIO interface factory
 *
 * \details
 * The RTPS NETIO interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct RTPS_InterfaceFactory RTPS_InterfaceFactory_fv_Factory =
{
  {
     &RTPS_InterfaceFactory_fv_Intf,
     NULL,
     {{{0,0}}}
  },
  NULL, /* clock */
  RTI_FALSE, /* _initialized */
  NULL, /* ext_intf_index */
  {
      {
          1,NULL,RTPS_BuiltinCrc32_checksum_calculate
      },
      {
          2,NULL,RTPS_BuiltinCrc64_checksum_calculate
      },
      {
          3,NULL,RTPS_BuiltinMD5_checksum_calculate
      },
      {
          4,NULL,RTPS_BuiltinCrc32Pro_checksum_calculate
      }
  },
  NULL
};

const struct RTPS_InterfaceFactoryProperty RTPS_INTERFACE_FACTORY_DEFAULT =
{
        NETIO_InterfaceFactoryProperty_INITIALIZER,
    {
        {
            1,NULL,RTPS_BuiltinCrc32_checksum_calculate
        },
        {
            2,NULL,RTPS_BuiltinCrc64_checksum_calculate
        },
        {
            3,NULL,RTPS_BuiltinMD5_checksum_calculate
        },
        RTPS_CHECKSUM_TXMODE_OMG,
        RTI_FALSE
    }
};

/*ci
 * \brief
 * Initialize the RTPS interface factory
 *
 * \param[in] property The property the RTPS factory is registered with.
 *                     If the property is NULL, the default RTPS property
 *                     is used.
 * \param[in] listener Unused
 *
 * \return Initialized RTPS Interface component factory
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
RTPS_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                 struct RT_ComponentFactoryListener *listener)
{
    struct RTPS_InterfaceFactory *factory = &RTPS_InterfaceFactory_fv_Factory;
    const struct RTPS_InterfaceFactoryProperty *rtps_property = (struct RTPS_InterfaceFactoryProperty*)property;

    UNUSED_ARG(listener);

    if (rtps_property == NULL)
    {
        rtps_property = &RTPS_INTERFACE_FACTORY_DEFAULT;
    }

    if (!rtps_property->checksum.allow_builtin_override)
    {
        if (!RTPS_ChecksumClass_is_equal(&rtps_property->checksum.builtin_checksum32_class,
                                    &RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class))
        {
            RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM32(OSAPI_LOGKIND_ERROR)
            return NULL;
        }
        if (!RTPS_ChecksumClass_is_equal(&rtps_property->checksum.builtin_checksum64_class,
                                    &RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class))
        {
            RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM64(OSAPI_LOGKIND_ERROR)
            return NULL;
        }
        if (!RTPS_ChecksumClass_is_equal(&rtps_property->checksum.builtin_checksum128_class,
                                    &RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class))
        {
            RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM128(OSAPI_LOGKIND_ERROR)
            return NULL;
        }
    }
    else
    {
        /* A built-in function can be reused for the same class-id if it is
         * identical, but it is not legal to reuse a function for a different
         * checksum.
         */
        if ((rtps_property->checksum.builtin_checksum32_class.class_id != RTPS_CHECKSUM_CLASSID_BUILTIN32) ||
            (rtps_property->checksum.builtin_checksum32_class.checksum_calculate == NULL) ||
            (rtps_property->checksum.builtin_checksum32_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.checksum_calculate) ||
            (rtps_property->checksum.builtin_checksum32_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.checksum_calculate) ||
             ((rtps_property->checksum.builtin_checksum32_class.checksum_calculate ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.checksum_calculate) &&
              (rtps_property->checksum.builtin_checksum32_class.context !=
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.context)) ||
             ((rtps_property->checksum.builtin_checksum32_class.checksum_calculate !=
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.checksum_calculate) &&
              (rtps_property->checksum.builtin_checksum32_class.context != NULL) &&
              ((rtps_property->checksum.builtin_checksum32_class.context ==
                RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.context) ||
               (rtps_property->checksum.builtin_checksum32_class.context ==
                RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.context) ||
               (rtps_property->checksum.builtin_checksum32_class.context ==
                RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.context))))
        {
            RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN32(OSAPI_LOGKIND_ERROR)
            return NULL;
        }

        if ((rtps_property->checksum.builtin_checksum64_class.class_id != RTPS_CHECKSUM_CLASSID_BUILTIN64) ||
            (rtps_property->checksum.builtin_checksum64_class.checksum_calculate == NULL) ||
            (rtps_property->checksum.builtin_checksum64_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.checksum_calculate) ||
            (rtps_property->checksum.builtin_checksum64_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.checksum_calculate) ||
            ((rtps_property->checksum.builtin_checksum64_class.checksum_calculate ==
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.checksum_calculate) &&
             (rtps_property->checksum.builtin_checksum64_class.context !=
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.context)) ||
            ((rtps_property->checksum.builtin_checksum64_class.checksum_calculate !=
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.checksum_calculate) &&
             (rtps_property->checksum.builtin_checksum64_class.context != NULL) &&
             ((rtps_property->checksum.builtin_checksum64_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.context) ||
              (rtps_property->checksum.builtin_checksum64_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.context) ||
              (rtps_property->checksum.builtin_checksum64_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.context))))
        {
            RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN64(OSAPI_LOGKIND_ERROR)
            return NULL;
        }

        if ((rtps_property->checksum.builtin_checksum128_class.class_id != RTPS_CHECKSUM_CLASSID_BUILTIN128) ||
            (rtps_property->checksum.builtin_checksum128_class.checksum_calculate == NULL) ||
            (rtps_property->checksum.builtin_checksum128_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.checksum_calculate) ||
            (rtps_property->checksum.builtin_checksum128_class.checksum_calculate ==
             RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.checksum_calculate) ||
            ((rtps_property->checksum.builtin_checksum128_class.checksum_calculate ==
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.checksum_calculate) &&
             (rtps_property->checksum.builtin_checksum128_class.context !=
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.context)) ||
            ((rtps_property->checksum.builtin_checksum128_class.checksum_calculate !=
              RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.checksum_calculate) &&
             (rtps_property->checksum.builtin_checksum128_class.context != NULL) &&
             ((rtps_property->checksum.builtin_checksum128_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum32_class.context) ||
              (rtps_property->checksum.builtin_checksum128_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum64_class.context) ||
              (rtps_property->checksum.builtin_checksum128_class.context ==
               RTPS_INTERFACE_FACTORY_DEFAULT.checksum.builtin_checksum128_class.context))))
        {
            RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN128(OSAPI_LOGKIND_ERROR)
            return NULL;
        }
    }

    factory->property = rtps_property;
    factory->checksum[0] = rtps_property->checksum.builtin_checksum32_class;
    factory->checksum[1] = rtps_property->checksum.builtin_checksum64_class;
    factory->checksum[2] = rtps_property->checksum.builtin_checksum128_class;

    factory->_initialized = RTI_FALSE;
    RTPS_InterfaceFactory_fv_Factory._parent._factory = &factory->_parent;

    return &RTPS_InterfaceFactory_fv_Factory._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize the RTPS interface factory
 *
 * \param[in] factory RTPS Interface factory
 * \param[in] property Unused
 * \param[in] listener Unused
 *
 */
RTI_PRIVATE void
RTPS_InterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    struct RTPS_InterfaceFactory *rtps_factory = &RTPS_InterfaceFactory_fv_Factory;
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    if (factory == &rtps_factory->_parent)
    {
        if (rtps_factory->_initialized)
        {
            (void)REDA_Indexer_delete(rtps_factory->ext_intf_index);
            rtps_factory->_initialized = RTI_FALSE;
        }

        if ((property != NULL) &&
            (rtps_factory->property != &RTPS_INTERFACE_FACTORY_DEFAULT))
        {
            *property = (struct RT_ComponentFactoryProperty*)rtps_factory->property;
        }
        else if (property != NULL)
        {
            /* Never return internal property */
            *property = NULL;
        }
    }
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
RTPS_InterfaceFactory_get_interface(void)
{
    return &RTPS_InterfaceFactory_fv_Intf;
}
