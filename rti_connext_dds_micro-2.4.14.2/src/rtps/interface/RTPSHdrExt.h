/*
 * FILE: RTPSHdrExt.h - Public RTPS functions to support RTPS header extensions
 *
 * Copyright 2020-2022 Real-Time Innovations, Inc.
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
 * - Changed rtps_msg_length to payload_length to be consistent with
 *   implementation for RTPS_Receiver_process_crc32 and
 *   RTPS_Receiver_process_header_ext
 * 03mar2021,tk MICRO-2866/PR#28697
 *   - Added msg_length to RTPS_Receive_is_msg_corrupted,
 *     RTPS_Receiver_process_crc32, and RTPS_Receiver_process_header_ext.
 * 04apr2021,tk MICRO-2974/PR.28968
 *  - Changed RTPS_Interface_set_header_extension() to return RTI_BOOL
 * 19oct2020,tk MICRO-2575/PR#28172 Removed support for custom checksums
 */
#ifndef RTPSHdrExt_h
#define RTPSHdrExt_h

#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
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

/*ci \brief Constant to indicate no checksum index has been specified
 */
#define RTPS_NO_CHECKSUM_INDEX (-1)

/*ci \brief The CRC32 function is at index 3 in the array of checksum functions
 */
#define RTPS_CRC32_INDEX (3)

extern RTI_BOOL
RTPS_Interface_add_header_extension(struct RTPS_Interface *intf,
                                    NETIO_Packet_T *packet);

extern RTI_BOOL
RTPS_Receive_is_msg_corrupted(struct RTPS_Interface *intf, /* self's external intf */
                              NETIO_Packet_T *packet,
                              union RTPS_MESSAGES *rtps_msg,
                              RTI_SIZE_T payload_length,
                              RTI_BOOL is_vendor_rti,
                              RTI_SIZE_T *msg_length);

extern RTI_BOOL
RTPS_Receiver_process_crc32(struct RTPS_Interface *intf,
                            NETIO_Packet_T *packet,
                            union RTPS_MESSAGES *rtps_msg,
                            RTI_SIZE_T payload_length,
                            struct RTPS_CRC32 *sub_msg,
                            RTI_BOOL byte_swap,
                            RTI_BOOL *dropped,
                            RTI_INT32 submsg_length,
                            RTI_SIZE_T *msg_length);

extern RTI_BOOL
RTPS_Receiver_process_header_ext(struct RTPS_Interface *intf,
                                 NETIO_Packet_T *packet,
                                 union RTPS_MESSAGES *rtps_msg,
                                 RTI_SIZE_T payload_length,
                                 struct RTPS_HEADER_EXT *sub_msg,
                                 RTI_BOOL byte_swap,
                                 RTI_BOOL *dropped,
                                 RTI_INT32 submsg_length,
                                 RTI_SIZE_T *msg_length);

extern RTI_BOOL
RTPS_Interface_set_header_extension(struct RTPS_Interface *intf,
                                    struct RTPS_HEADER_EXT *hdrext,
                                    NETIO_Packet_T *packet);

#endif
