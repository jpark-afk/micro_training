/*
 * FILE: RTPSHdrExt.c -  Implementation of RTPS functions to support RTPS
 *                       header extensions
 *
 * Copyright 2020-2021 Real-Time Innovations, Inc.
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Added documentation for missing parameter byte_swap in
 *   RTPS_Interface_is_checksum_valid
 * 13sep2021,tk MICRO-3071/PR.29186
 * - Ensure pid_ptr is set correctly in RTPS_Receiver_process_header_ext
 *   if P-bit is set regardless of which optional fields precedes the
 *   parameter list by setting it relative to the current packet header
 *   instead of msg_ptr.
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 *  - Added suppression of cert_str31_c_violation in RTPS_Interface_is_checksum_valid
 * 15jun2021,tk MICRO-3083/PR.29225
 * - Only discard a message if the senders checksum is not supported and
 *   check_crc is TRUE.
 * 15jun2021,tk MICRO-3071/PR.29186
 * - Ensure msg_ptr is set correctly if the length is not present, but a
 *   parameter list is in RTPS_Receiver_process_header_ext().
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 03mar2021,tk MICRO-2866/PR#28697
 *   - Return the message length from process_header_ext(),
 *     process_crc32(), and is_msg_corrupted() if available, even if it may
 *     be invalid.
 * 04apr2021,tk MICRO-2974/PR.28968
 *   - Return TRUE/FALSE in RTPS_Interface_set_header_extension()
 *   - Check return value in RTPS_Interface_is_checksum_valid() in
 *     RTPS_Interface_set_header_extension()
 * 04apr2021,tk MICRO-2845/PR.28682
 *   - Removed empty block in RTPS_Receive_is_msg_corrupted()
 * 14dec2020,tk
 *   MICRO-2779/PR.28507
 *     - Validate the HeaderExtension by fully parsing it, including the P-bit
 *     - Drop messages with a mandatory PID in the HeaderExtension
 *   MICRO-2765/PR.28486
 *     - Do not move head of packet in case of failure of processing the
 *       flags in RTPS_Receiver_process_crc32().
 *   MICRO-2764/PR.28470
 *     - Initialize dropped to RTI_FALSE in RTPS_Receiver_process_crc32()
 *   MICRO-2768/PR.28489
 *     - Corrected description of return value for RTPS_Receive_is_msg_corrupted
 *     - Removed test for is_vendor_rti  for RTPS_HEADER_EXTN_KIND since this is
 *       now standardized.
 *     - Check that that payload is at least (length + sizeof(struct RTPS_SubmsgHdr)
 *       in RTPS_Receive_is_msg_corrupted().
 *     - Drop message if a checksum is required but RTPS message is too short to
 *       include it.
 *   MICRO-2738/PR.28448
 *     - Corrected check for minimum length of payload in
 *       RTPS_Receiver_process_header_ext().
 *   MICRO-2741/PR.28351
 *     - Changed intf to in in RTPS_Interface_set_header_extension()
 *       parameter description.
 *     - Clarified meaning of the submsg_length to
 *       RTPS_Receiver_process_header_ext().
 *     - Clarified meaning of the submsg_length to
 *       RTPS_Receiver_process_crc32().
 *     - Corrected description of return value for
 *       RTPS_Receive_is_msg_corrupted()
 *   MICRO-2739/PR.28455
 *     - Removed UNUSED_ARG(byte_swap) in RTPS_Receiver_process_crc32()
 *
 * 19oct2020,tk MICRO-2575/PR#28172 Updated parsing of header extension to be
 *                                  RTPS 2.5 compliant.
 * 07may2020,tk MICRO-2364/PR#27267 Remove compiler warning with GCC on QNX
 */
#include "RTPSHdrExt.h"

/*ci \brief Array of sizes for the different checksum kinds. Note that this array
 *    is zero based, thus C bits = 1 is index 0 etc.
 */
RTI_PRIVATE const RTI_SIZE_T RTPS_fv_ChecksumSize[4] =
{
    4,   /* 0 - 32-bit builtin */
    8,   /* 1 - 64-bit builtin */
    16,  /* 2 - 128-bit builtin */
    4    /* 3 - 32-bit builtin Core checksum  */
};

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Get checksum size in bytes
 *
 * \param[in] checksum_index
 *
 * \return Number of bytes in checksum_index
 */
RTI_PRIVATE RTI_SIZE_T
RTPS_Interface_get_checksum_size(RTI_INT32 checksum_index)
{
    return RTPS_fv_ChecksumSize[checksum_index];
}

/*ci \brief Validate a checksum in a buffer received by RTPS
 *
 * \param[in] intf - The RTPS interface
 * \param[in] payload_start - The beginning of the buffer
 * \param[in] payload_length - The number of bytes to calculate the checksum over
 * \param[in] received_checksum - Pointer to the received checksum to compare
 *                                against
 * \param[in] checksum_index - The checksum function to use
 * \param[in] byte_swap - RTI_TRUE if byteswapping should be performed when
 *                        deserializing, RTI_FALSE if not.
 *
 * \return TRUE if the checksum is valid, FALSE if the checksum is not valid
 */
RTI_PRIVATE RTI_BOOL
RTPS_Interface_is_checksum_valid(struct RTPS_Interface *intf,
                                 void *payload_start,
                                 RTI_SIZE_T payload_length,
                                 RTPS_Checksum_T *received_checksum,
                                 RTI_INT32 checksum_index,
                                 RTI_BOOL byte_swap)
{
    struct REDA_Buffer buf[1];
    RTPS_Checksum_T saved_checksum;
    RTPS_Checksum_T calculated_checksum;
    char *ptr;
    RTI_BOOL retval = RTI_FALSE;
    RTI_SIZE_T checksum_size;

    checksum_size = RTPS_Interface_get_checksum_size(checksum_index);

    /* Zero the received checksum and calculated checksum, save
     * the received checksum so it can be restored.
     */
    OSAPI_Memory_copy(&saved_checksum,received_checksum,checksum_size);
    OSAPI_Memory_zero(received_checksum, checksum_size);
    OSAPI_Memory_zero(&calculated_checksum, checksum_size);

    REDA_Buffer_set(&buf[0], payload_start, payload_length);

    /* checksum_index is 0-3
     */
    /* coverity[cert_str31_c_violation] */
    if (!RTPS_Interface_checksum_calculate(intf,checksum_index,buf,1,&calculated_checksum))
    {
        return RTI_FALSE;
    }

    ptr = (char*)&saved_checksum;

    if (checksum_size == CDR_UNSIGNED_LONG_SIZE)
    {
        if (checksum_index == RTPS_CRC32_INDEX)
        {
            /* The Pro 32-bit CRC is sent in native endianess */
            CDR_deserialize_unsigned_long(
                            &ptr,&received_checksum->checksum32,byte_swap);
        }
        else
        {
            CDR_deserialize_unsigned_long_from_big_endian(&ptr,
                                      &received_checksum->checksum32);
        }
    }
    else if (checksum_size == CDR_UNSIGNED_LONG_LONG_SIZE)
    {
        CDR_deserialize_unsigned_long_long_from_big_endian(&ptr,
                                      &received_checksum->checksum64);
    }
    else
    {
        OSAPI_Memory_copy(received_checksum,&saved_checksum,checksum_size);
    }

    retval = OSAPI_Memory_compare(&calculated_checksum,received_checksum,checksum_size);

    /* Restore original checksum */
    OSAPI_Memory_copy(received_checksum,&saved_checksum,checksum_size);

    return retval == 0;
}

/*ci \brief Add space for a header extension
 *
 * \param[in] intf - The RTPS interface
 * \param[in] packet - The packet to add space for the header extension
 *
 * \return TRUE if space was added, FALSE if not
 */
RTI_BOOL
RTPS_Interface_add_header_extension(struct RTPS_Interface *intf,
                                    NETIO_Packet_T *packet)
{
    RTI_SIZE_T checksum_size = 0;
    RTI_SIZE_T hdr_size = 0;

    checksum_size = RTPS_Interface_get_checksum_size(intf->checksum_index);

    hdr_size = RTI_SIZEOF(struct RTPS_SubmsgHdr)
                   + RTPS_HEADER_EXTN_LENGTH_SIZE
                   + checksum_size;

    if (hdr_size > INT_MAX)
    {
        OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,(RTI_INT32)hdr_size,INT_MAX)
        return RTI_FALSE;
    }

    /* move the head back the size required for the header extnsion*/
    if (!NETIO_Packet_set_head(packet, 0 - (RTI_INT32)(hdr_size)))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Assign fields of Header Extension
 *
 *
 * \details
 *
 * 0...2...........8...............16..............24..............32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    RTPS_HE    |P|C|C|W|U|T|L|E|       octetsToNextHeader      |
 * +---------------+---------------+---------------+---------------+
 * |      MessageLength  messageLength        [only if L == 1]     |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * +      Timestamp      rtpsSendTimestamp    [only if T == 1]     +
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |      UExtension4    uExtension4          [only if U == 1]     |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * +      WExtension8    wExtension8          [only if W == 1]     +
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * ~      Checksum       messageChecksum      [only if CC != 00]   ~
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * ~      ParameterList  parameters           [only if P != 0]     ~
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 *
 *
 * \param[in]  intf The RTPS interface
 * \param[inout] hdrext The header extension to fill in
 * \param[in]    packet The packet to set the header extension in
 *
 * \return returns RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
RTPS_Interface_set_header_extension(struct RTPS_Interface *intf,
                                    struct RTPS_HEADER_EXT *hdrext,
                                    NETIO_Packet_T *packet)
{
    RTI_UINT8 flags = 0;
    RTI_SIZE_T submsg_size = 0;
    RTI_SIZE_T checksum_size;
    RTI_SIZE_T rtps_message_length;
    struct REDA_Buffer buf[1];
    RTPS_Checksum_T checksum;
    char *msg_ptr = (char *)hdrext;
    RTI_UINT8 submsg_kind = 0;

    if (intf->checksum_index < 0)
    {
        /* This function is never called unless a checksum is added, extra
         * safe-guard.
         */
        return RTI_FALSE;
    }

    if (intf->factory->property->checksum.checksum_tx_mode == RTPS_CHECKSUM_TXMODE_RTICRC32)
    {
        submsg_kind = RTPS_RTI_CRC32;
        flags = 0;
    }
    else
    {
        submsg_kind = RTPS_HEADER_EXTN_KIND;
        /* Add 1 to index since internally the first checksum starts at
         * index 0, not 1.
         */
        flags = RTPS_HEADER_EXTFLAG_L |
                (RTI_UINT8)(((RTI_UINT32)intf->checksum_index + 1U) << RTPS_HEADER_EXTFLAG_C_POS);
    }

    checksum_size = RTPS_Interface_get_checksum_size(intf->checksum_index);

    submsg_size = RTI_SIZEOF(struct RTPS_SubmsgHdr)
                    + RTPS_HEADER_EXTN_LENGTH_SIZE
                    + checksum_size;

    RTPS_Interface_set_submessage_header(&hdrext->hdr,
                                         submsg_kind,
                                         flags,
                                         submsg_size);

    /* Set the length field
     */
    msg_ptr += RTI_SIZEOF(struct RTPS_SubmsgHdr);
    rtps_message_length = NETIO_Packet_get_payload_length(packet);
    CDR_serialize_unsigned_long(&msg_ptr,&rtps_message_length,0);

    /* Checksum follows length, clear checksum field first
     */
    OSAPI_Memory_zero(msg_ptr,checksum_size);
    OSAPI_Memory_zero(&checksum,checksum_size);

    REDA_Buffer_set(&buf[0],NETIO_Packet_get_head(packet),rtps_message_length);

    if (!RTPS_Interface_checksum_calculate(intf,intf->checksum_index,buf,1,&checksum))
    {
        RTPS_LOG_CHECKSUM_CHECKSUM_CALCULATION_FAILED(OSAPI_LOGKIND_ERROR,
                                                      intf->checksum_index)
        return RTI_FALSE;
    }

    if (intf->factory->property->checksum.checksum_tx_mode == RTPS_CHECKSUM_TXMODE_RTICRC32)
    {
        /* CRC32 to Core is still sent with host endianess using the E-bit */
        OSAPI_Memory_copy(msg_ptr,&checksum,checksum_size);
    }
    else
    {
        /* HE checksums are always sent with big-endian
         * For 32 and 64-bit checksum host endianess calculations are assumed
         * since that is very common for CRCs. 128-bit is always an octet
         * array.
         */
        if (checksum_size == CDR_UNSIGNED_LONG_SIZE)
        {
            CDR_serialize_unsigned_long_to_big_endian(&msg_ptr,&checksum.checksum32);
        }
        else if (checksum_size == CDR_UNSIGNED_LONG_LONG_SIZE)
        {
            CDR_serialize_unsigned_long_long_to_big_endian(&msg_ptr,&checksum.checksum64);
        }
        else
        {
            OSAPI_Memory_copy(msg_ptr,&checksum,checksum_size);
        }
    }

    return RTI_TRUE;
}

/*ci \brief Process a received header extension
 *
 * \details
 *
 * This is the HeaderExtension submessage structure in RTPS 2.5
 *
 * 0...2...........8...............16..............24..............32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    RTPS_HE    |P|C|C|W|U|T|L|E|       octetsToNextHeader      |
 * +---------------+---------------+---------------+---------------+
 * |      MessageLength  messageLength        [only if L == 1]     |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * +      Timestamp      rtpsSendTimestamp    [only if T == 1]     +
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |      UExtension4    uExtension4          [only if U == 1]     |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * +      WExtension8    wExtension8          [only if W == 1]     +
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * ~      Checksum       messageChecksum      [only if CC != 00]   ~
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 * |                                                               |
 * ~      ParameterList  parameters           [only if P != 0]     ~
 * |                                                               |
 * +---------------+---------------+---------------+---------------+
 *
 * MessageLength is required to be able to calculate the checksum
 * Timestamp is ignored if present
 * UExtension4 is ignored if present
 * WExtension8 is ignored if present
 * Checksum is used if needed
 *
 * The rest of the submessage is ignored using octetsToNextHeader to
 * jump to the next submessage. That is, the P-bit is not even checked.
 *
 * \param[in] intf The RTPS interface the packet was received on
 * \param[in] packet The packet itself
 * \param[in] rtps_msg Pointer to the RTPS msg (RTPS header)
 * \param[in] payload_length The length of the payload with the RTPS message.
 *                           There may be more than one, rtps_msg points to the
 *                           first one.
 * \param[in] sub_msg Pointer to the header extension submsg
 * \param[in] byte_swap Whether byte-swap is needed or not
 * \param[out] dropped TRUE if the entire msg should be dropped, FALSE otherwise
 * \param[in] submsg_length The length of the HeaderExtension Submessage,
 *                          excluding the length of Submessage header itself.
 * \param[out] msg_length   The decoded message-length if available, 0 otherwise.
 *                          Cannot be NULL. Note that the message-length may
 *                          not be a valid length, e.g. if the checksum was
 *                          invalid or the HeaderExtension itself was invalid.
 *                          However, if the current message is invalid
 *                          it may be used to jump to the next possible RTPS
 *                          message if it is within the bounds of the payload.
 *
 * \return TRUE on successful processing, FALSE on failure. If TRUE then
 *         dropped indicates if the packet should be dropped or not.
 */
RTI_BOOL
RTPS_Receiver_process_header_ext(struct RTPS_Interface *intf,
                                 NETIO_Packet_T *packet,
                                 union RTPS_MESSAGES *rtps_msg,
                                 RTI_SIZE_T payload_length,
                                 struct RTPS_HEADER_EXT *sub_msg,
                                 RTI_BOOL byte_swap,
                                 RTI_BOOL *dropped,
                                 RTI_INT32 submsg_length,
                                 RTI_SIZE_T *msg_length)
{
    RTI_UINT16 c_bits = 0x00;
    char *msg_ptr = NULL;
    RTI_INT32 checksum_index = 0;
    RTI_SIZE_T hdrext_msglen = 0;
    RTI_SIZE_T checksum_size = 0;
    RTPS_Checksum_T *chksum_ptr;
    RTI_UINT32 skip_bytes = 0;

    *dropped = RTI_FALSE;
    *msg_length = 0;

    /* 2 bits are used for the checksum, subtract 1 since internally checksums
     * start at index 0.
     */
    c_bits = (RTI_UINT16)((sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_C) >> RTPS_HEADER_EXTFLAG_C_POS);

    if ((c_bits > 0) && !(sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_L))
    {
        /* C bits set, but no length, inconsistent submessage */
        RTPS_LOG_CHECKSUM_MISSING_CHECKSUM_LENGTH(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (NETIO_Packet_get_payload_length(packet) < RTI_SIZEOF(struct RTPS_SubmsgHdr))
    {
        RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Always start right after the header */
    if (!NETIO_Packet_set_head(packet, RTI_SIZEOF(struct RTPS_SubmsgHdr)))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* It is possible that a HeaderExtension is received with a message
     * length, but no checksum is required. This information is not
     * currently used, but still validated.
     */
    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_L)
    {
        /* Check that the submessage is at least the minimum required length
         * to deserialize the messageLength field.
         */
        if (NETIO_Packet_get_payload_length(packet) < RTI_SIZEOF(CDR_Long))
        {
            RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }

        /* Deserialize HeaderExtension.length */
        msg_ptr = (char*)NETIO_Packet_get_head(packet);

        CDR_deserialize_unsigned_long(&msg_ptr, &hdrext_msglen, byte_swap);

        /* This length may or may not be valid. */
        *msg_length = hdrext_msglen;

        skip_bytes += RTI_SIZEOF(CDR_Long);
    }

    /* calculate the number of bytes that must be skipped to get to the
     * checksum, including the message length.
     */
    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_T)
    {
        /* A timestamp is 2 32-bit integers */
        skip_bytes += 2 * RTI_SIZEOF(CDR_Long);
    }

    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_U)
    {
        /* The U extension is 4 octets */
        skip_bytes += 4 * RTI_SIZEOF(CDR_Octet);
    }

    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_W)
    {
        /* The W extension is 8 octets */
        skip_bytes += 8 * RTI_SIZEOF(CDR_Octet);
    }

    if (c_bits != 0)
    {
        /* A potential checksum follows the W-bit if present,
         * determine if it can be accepted. If not, drop the entire
         * message.
         */
        checksum_index = c_bits - 1;
        checksum_size = RTPS_Interface_get_checksum_size(checksum_index);

        /* Reject messages that are not in the allowed_crc_mask
         */
        if (intf->property.check_crc && !(((1U << checksum_index) & intf->property.allowed_crc_mask)))
        {
            /* checksum present, but not understood, drop message */
            RTPS_LOG_CHECKSUM_UNSUPPORTED(OSAPI_LOGKIND_ERROR,c_bits,
                                          intf->property.allowed_crc_mask)
            *dropped = RTI_TRUE;
            return RTI_TRUE;
        }

        /* Check that the submessage is at least the minimum required length
         * for skipped bytes, the length and the checksum. At this point
         * the length has already been deserialized _and_ the packet header
         * has been moved.
         */
        if (NETIO_Packet_get_payload_length(packet) < (skip_bytes + checksum_size))
        {
            /* Not enough bytes left for the checksum and bytes to skip.
             * Since the submessage is invalid return false and do not skip
             * the submessage header.
             */
            RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }

        /* Save the ptr to the checksum. Note that the packet header position
         * is still just after the submessage header. skip_bytes does
         * include the size of the messageLength, so subtract it since it
         * must already have been deserialized.
         */
        msg_ptr += skip_bytes - RTI_SIZEOF(CDR_Long);
    }

    if ((RTI_SIZE_T)submsg_length < (skip_bytes + checksum_size))
    {
        /* With or without the L-bit, this is wrong */
        RTPS_LOG_INCONSISTENT_HDR_EXT_LENGTH_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Check for parameter IDs */
    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_P)
    {
        RTI_SIZE_T param_length;
        char *pid_ptr;
        RTI_UINT16 pid_length = 0;
        RTI_UINT16 pid_id = 0;

        /* Start after the optional L, T,U,W and checksum fields.
         * - skip_bytes is the length of all optional fields, including L
         * - checksum_size is the size of the checksum if present
         */
        pid_ptr = (char*)NETIO_Packet_get_head(packet) + skip_bytes + checksum_size;

        /* The remaining bytes in the HeaderExtension must be parameter
         * IDs.
         *
         * The estimated length of all PIDs is the total submsg length,
         * which starts after the submsg header, minus the bytes to skip
         * minus the messageLength (included in the skip-bytes), minus the
         * checksum size (not included in the skip-bytes).
         */
        param_length = (RTI_SIZE_T)submsg_length - checksum_size - skip_bytes;

        if (param_length > NETIO_Packet_get_payload_length(packet))
        {
            /* The P-bit is set, but there are not enough bytes for the
             * parameters the message is invalid.
             */
            RTPS_LOG_INVALID_HDR_EXT_PID_LIST(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }

        while (param_length >= RTPS_HEADER_EXTPID_HDR_SIZE)
        {
            CDR_deserialize_unsigned_short(&pid_ptr,&pid_id, byte_swap);

            /* If the PID is standard & mandatory, the HeaderExtension is
             * invalid since none are supported. Drop the entire message.
             */
            if (pid_id & RTPS_PID_INCOMPATIBLE_MASK)
            {
                RTPS_LOG_UNSUPPORTED_MANDTORY_HDR_EXT_PID(OSAPI_LOGKIND_ERROR,pid_id)
                return RTI_FALSE;
            }

            CDR_deserialize_unsigned_short(&pid_ptr,&pid_length, byte_swap);

            if (pid_length & 0x3)
            {
                /* round up to 4 bytes as per the RTPS spec. If the message is
                 * too short then parsing will fail.
                 */
                pid_length = (RTI_UINT16)((pid_length + 3) & ~0x3);
            }

            if (param_length < (RTI_SIZE_T)(pid_length + (RTI_SIZE_T)RTPS_HEADER_EXTPID_HDR_SIZE))
            {
                /* prevent underflow */
                break;
            }
            pid_ptr += pid_length;
            param_length -= (RTI_SIZE_T)(pid_length + (RTI_SIZE_T)RTPS_HEADER_EXTPID_HDR_SIZE);
        }

        if (param_length != 0)
        {
            /* The parameter IDs could not be decoded correctly, the
             * last PID was too big or there are not enough for a new PID.
             */
            RTPS_LOG_INVALID_HDR_EXT_PID_ERROR(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    /* The complete HeaderExtension has been parsed. Determine if the lengths
     * are consistent, that is the parsed length and the encoded length is
     * valid.
     */
    if (sub_msg->hdr.flags & RTPS_HEADER_EXTFLAG_L)
    {
        RTI_SIZE_T min_msg_length = (RTI_SIZE_T)submsg_length
                                    + RTI_SIZEOF(struct RTPS_Header)
                                    + RTI_SIZEOF(struct RTPS_SubmsgHdr);

        /* The submsg_length held all the information based on the flags,
         * determine if the
         * 1) The length in the HeaderExtension is less than the minimum
         *    calculated
         * 2) Or the payload is too short.
         */
        if ((hdrext_msglen < min_msg_length) || (hdrext_msglen > payload_length))
        {
            RTPS_LOG_INCONSISTENT_HDR_EXT_LENGTH_ERROR(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    /* The HeaderExtension is valid. If a checksum is present, determine if it
     * the entire message is valid if a checksum is present and should be
     * checked.
     */

    packet->info.checksum_info = CDR_STREAM_CHECKSUM_HDREXT;

    /* If the checksum is not present determine what to do */
    if (c_bits == 0)
    {
        if (intf->property.require_crc)
        {
            /* checksum required, but not present */
            *dropped = RTI_TRUE;
        }
        else
        {
            /* checksum not required and not present/unsupported, skip the
             * rest of the submessage.
             */
            if (!NETIO_Packet_set_head(packet, submsg_length))
            {
                RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }
        }

        /* Did not fail to parse the submessage */
        return RTI_TRUE;
    }


    if (intf->property.check_crc)
    {
        OSAPI_Compiler_reinterpret_cast(chksum_ptr,msg_ptr,ptr_ptr);

        if (!RTPS_Interface_is_checksum_valid(intf,
                                              rtps_msg,
                                              hdrext_msglen,
                                              chksum_ptr,
                                              checksum_index,
                                              byte_swap))
        {
            RTPS_LOG_CHECKSUM_CHECKSUM_ERROR(OSAPI_LOGKIND_ERROR)
            *dropped = RTI_TRUE;
        }
        else
        {
            packet->info.checksum_info |= c_bits;
        }
    }

    /* HeaderExtension was ok, move past the HeaderExtension to the first
     * actual submessage.
     */
    if (!NETIO_Packet_set_head(packet,submsg_length))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Process a received CRC32 submsg
 *
 * \param[in] intf The RTPS interface the packet was received on
 * \param[in] packet The packet itself
 * \param[in] rtps_msg Pointer to the RTPS msg (RTPS header)
 * \param[in] payload_length The length of the RTPS message
 * \param[in] sub_msg Pointer to the header extension submsg
 * \param[in] byte_swap Whether byte-swap is needed or not
 * \param[out] dropped TRUE if the entire msg should be dropped, FALSE otherwise
 * \param[in] submsg_length The length of the RTI_CRC32 Submessage,
 *                          excluding the length of Submessage header itself.
 * \param[out] msg_length   The decoded message-length if available, 0 otherwise.
 *                          Cannot be NULL. Note that the message-length may
 *                          not be a valid length, e.g. if the checksum was
 *                          invalid, however if the current message is invalid
 *                          it may be used to jump to the next potential RTPS
 *                          message if it is within the bounds of the payload.
 *
 * \return TRUE on successful processing, FALSE on failure. If TRUE then
 *         dropped indicates if the packet should be dropped or not.
 */
RTI_BOOL
RTPS_Receiver_process_crc32(struct RTPS_Interface *intf,
                            NETIO_Packet_T *packet,
                            union RTPS_MESSAGES *rtps_msg,
                            RTI_SIZE_T payload_length,
                            struct RTPS_CRC32 *sub_msg,
                            RTI_BOOL byte_swap,
                            RTI_BOOL *dropped,
                            RTI_INT32 submsg_length,
                            RTI_SIZE_T *msg_length)
{
    RTI_UINT32 msglength;
    char *msg_ptr = (char *)sub_msg;
    RTPS_Checksum_T *chksum_ptr;

    *dropped = RTI_FALSE;
    *msg_length = 0;

    if (NETIO_Packet_get_payload_length(packet) <
        (RTI_SIZEOF(struct RTPS_SubmsgHdr) + CDR_UNSIGNED_LONG_SIZE))
    {
        /* Not enough bytes left for the message length */
        RTPS_LOG_CHECKSUM_CHECKSUM_DESERIALIZE_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!NETIO_Packet_set_head(packet, RTI_SIZEOF(struct RTPS_SubmsgHdr)))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Deserialize length. Even if the remainder of the submessage is invalid,
     * the length may still be of use to find the next potential RTPS message
     * in the payload.
     */
    msg_ptr = (char*)NETIO_Packet_get_head(packet);
    CDR_deserialize_unsigned_long(&msg_ptr, &msglength, byte_swap);

    if (payload_length < msglength)
    {
        /* Mismatch in length, don't bother checking the CRC. Note that
         * there may be more than 1 RTPS message in a payload, so this
         * only checks that the deserialized message length does not
         * exceed the remaining payload.
         */
        RTPS_LOG_CHECKSUM_INCONSISTENT_LENGTH(OSAPI_LOGKIND_ERROR,
                                         payload_length,
                                         msglength)
        return RTI_FALSE;
    }

    *msg_length = msglength;

    if (sub_msg->hdr.flags & ~RTPS_SUBMSG_FLAG_E)
    {
        /* Only allow the endianness flags to be set since it is unknown
         * how the unsupported flags may impact the checksum. The message
         * is considered invalid and the rest of the message should be
         * dropped.
         */
        RTPS_LOG_CHECKSUM_INVALID_CRC32_FLAGS(OSAPI_LOGKIND_ERROR,sub_msg->hdr.flags)
        return RTI_FALSE;
    }

    if ((NETIO_Packet_get_payload_length(packet) <  RTPS_RTI_CRC32_LENGTH) ||
        (submsg_length != RTPS_RTI_CRC32_LENGTH))
    {
        /* Not enough bytes left for the CRC length */
        RTPS_LOG_CHECKSUM_CHECKSUM_DESERIALIZE_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    packet->info.checksum_info = CDR_STREAM_CHECKSUM_CRC32;

    if (intf->property.check_crc)
    {
        /* msg_ptr points to the received checksum */
        OSAPI_Compiler_reinterpret_cast(chksum_ptr,msg_ptr,ptr_ptr);
        if (!RTPS_Interface_is_checksum_valid(intf,
                                              rtps_msg,
                                              msglength,
                                              chksum_ptr,
                                              RTPS_CRC32_INDEX,
                                              byte_swap))
        {
            RTPS_LOG_CHECKSUM_CHECKSUM_ERROR(OSAPI_LOGKIND_ERROR)
            *dropped = RTI_TRUE;
        }
        else
        {
            packet->info.checksum_info |= CDR_STREAM_CHECKSUM_BUILTIN32;
        }
    }

    /* Make sure the packet head is at the next potential submessage.
     */
    if (!NETIO_Packet_set_head(packet, submsg_length))
    {
        RTPS_LOG_NETIO_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Determine if a RTPS msg has a valid checksum if present
 *
 * \param[in] intf The interface the packet was received on
 * \param[in] packet The packet
 * \param[in] rtps_msg The RTPS message
 * \param[in] payload_length The total length of the packet
 * \param[in] is_vendor_rti TRUE if this packet was received from a RTI product
 * \param[out] msg_length   The decoded message-length if available, 0 otherwise.
 *                          Cannot be NULL. Note that the message-length may
 *                          not be a valid length, e.g. if the checksum was
 *                          invalid, however if the current message is invalid
 *                          it may be used to jump to the next potential RTPS
 *                          message if it is within the bounds of the payload.
 *
 * \return TRUE if the RTPS message is corrupted, the first submessage is
 *         too short, or a required checksum is missing, FALSE otherwise.
 */
RTI_BOOL
RTPS_Receive_is_msg_corrupted(struct RTPS_Interface *intf, /* self's external intf */
                              NETIO_Packet_T *packet,
                              union RTPS_MESSAGES *rtps_msg,
                              RTI_SIZE_T payload_length,
                              RTI_BOOL is_vendor_rti,
                              RTI_SIZE_T *msg_length)
{
    union RTPS_MESSAGES *sub_msg = NULL;
    char *msg_ptr = NULL;
    RTI_BOOL byte_swap;
    RTI_BOOL drop_message = RTI_FALSE;
    RTI_UINT16 length;

    /* A valid RTPS message header has been found, but no state
     * transitions have been performed. Check if checksum is required and
     * if it is present. A checksum _must_ follow the RTPS header in the
     * form of either a header extension or a Core CRC32 submessage. If
     * neither is found and a checksum is required, drop the message. If
     * it is found then validate if enabled, otherwise skip it.
     */
    msg_ptr = NETIO_Packet_get_head(packet);
    *msg_length = 0;

    OSAPI_Compiler_reinterpret_cast(sub_msg,msg_ptr,ptr_ptr);

    if (NETIO_Packet_get_payload_length(packet) < RTI_SIZEOF(struct RTPS_SubmsgHdr))
    {
        /* Since the remaining message is too short for one submessage, this
         * function is called after the RTPS header has been found, stop
         * processing. If a CRC is required, then drop_message = TRUE,
         * otherwise let the caller decide what to do since technically the
         * message is not corrupted, just short.
         */
        if (intf->property.require_crc)
        {
            drop_message = RTI_TRUE;
        }
        goto done;
    }

    byte_swap = RTPSInterface_byte_swap(sub_msg->submsg.flags);

    /* Skip the flags and kind */
    msg_ptr += 2;

    CDR_deserialize_unsigned_short(&msg_ptr,&length,byte_swap);

    if (NETIO_Packet_get_payload_length(packet) < (length + RTI_SIZEOF(struct RTPS_SubmsgHdr)))
    {
        drop_message = RTI_TRUE;
        goto done;
    }

    if (sub_msg->submsg.kind == RTPS_HEADER_EXTN_KIND)
    {
        if (!RTPS_Receiver_process_header_ext(intf,
                                              packet,
                                              rtps_msg,
                                              payload_length,
                                              &sub_msg->header_ext,
                                              byte_swap,
                                              &drop_message,
                                              length,msg_length))
        {
            drop_message = RTI_TRUE;
        }
    }
    else if (is_vendor_rti && (sub_msg->submsg.kind == RTPS_RTI_CRC32))
    {
        if (!RTPS_Receiver_process_crc32(intf,packet,
                                         rtps_msg,
                                         payload_length,
                                         &sub_msg->crc32,
                                         byte_swap,
                                         &drop_message,
                                         length,msg_length))
        {
            drop_message = RTI_TRUE;
        }
    }
    else if (intf->property.require_crc)
    {
        drop_message = RTI_TRUE;
    }
    else
    {
        /* A RTPS header was not found and a checksum is not required, return
         * RTI_FALSE and let processing of submessages continue.
         */
        return RTI_FALSE;
    }

done:

    return drop_message;
}
