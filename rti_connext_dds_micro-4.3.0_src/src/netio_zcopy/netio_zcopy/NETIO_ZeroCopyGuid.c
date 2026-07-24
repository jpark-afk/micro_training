/*
 * FILE: NETIO_ZeroCopyGuid.c - ZCOPY_Guid implementation
 *
 * (c) Copyright 2023 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "netio_zcopy/netio_zcopy_guid.h"

#include "netio_zcopy/netio_zcopy_log.h"
#include "netio/netio_common.h"
#include "osapi/osapi_string.h"

/*** SOURCE_BEGIN ***/

RTI_BOOL
ZCOPY_Guid_equals(const struct ZCOPY_Guid *a, const struct ZCOPY_Guid *b)
{
    OSAPI_PRECONDITION((a == NULL) || (b == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("b", b, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("a", a, RTI_TRUE);)

    if (OSAPI_Memory_compare(&a->value, &b->value, sizeof(a->value)) != 0)
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_Guid_to_string(
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        const char *prefix,
        char *buffer,
        RTI_SIZE_T buffer_len)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_SIZE_T prefix_len;
    RTI_UINT16 short_port;
    unsigned char temp[sizeof(guid->value) + sizeof(short_port)];

    /* Validate port number */
    if (port > 0xFFFF)
    {
        ZCOPY_LOG_NOTIF_INVALID_PORT_NUMBER(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    short_port = (RTI_UINT16)port;
    short_port = NETIO_htons(short_port);

    /* Copy prefix into buffer */
    prefix_len = OSAPI_String_length_w_max(prefix, buffer_len);
    if (prefix_len == buffer_len)
    {
        ZCOPY_LOG_NOTIF_GUID_BUFFER_TOO_SMALL(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    OSAPI_Memory_copy(buffer, prefix, prefix_len + 1);

    /* Copy bytes into temp buffer */
    OSAPI_Memory_copy(temp, &guid->value, sizeof(guid->value));
    OSAPI_Memory_copy(
            temp + sizeof(guid->value),
            &short_port,
            sizeof(short_port));

    if (!ZCOPY_Guid_bytes_to_string(
                temp,
                sizeof(temp),
                buffer + prefix_len,
                buffer_len - prefix_len))
    {
        ZCOPY_LOG_NOTIF_GUID_BUFFER_TOO_SMALL(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    ok = RTI_TRUE;

done:
    return ok;
}

RTI_BOOL
ZCOPY_Guid_bytes_to_string(
        const unsigned char *bytes,
        RTI_SIZE_T bytes_len,
        char *buffer,
        RTI_SIZE_T buffer_len)
{
    /* Base64-ish alphabet to be a valid path name */
    const char *alphabet =
            "ABCDEFGHIJKLMNOP"
            "QRSTUVWXYZabcdef"
            "ghijklmnopqrstuv"
            "wxyz0123456789-_";
    RTI_BOOL ok = RTI_FALSE;
    RTI_SIZE_T cur_len = 0;
    RTI_SIZE_T i = 0;

    /* Validate buffer size */
    if (buffer_len == 0)
    {
        goto done;
    }

    /* Calculate number of characters required to represent bytes in base64 */
    RTI_SIZE_T converted_length = 4 * ((bytes_len + 2) / 3);
    if ((converted_length + 1) > buffer_len)
    {
        buffer[0] = '\0';
        goto done;
    }

    /* Convert 3 bytes at a time to 4 bytes of base64-ish.
     * Instead of padding with '=', zeros will be appended to <bytes> to make
     * its length a multiple of 3.
     */
    while (i < bytes_len)
    {
        RTI_UINT32 octet_a = (RTI_UINT32)bytes[i++];
        RTI_UINT32 octet_b = (RTI_UINT32)((i < bytes_len) ? bytes[i++] : 0);
        RTI_UINT32 octet_c = (RTI_UINT32)((i < bytes_len) ? bytes[i++] : 0);

        RTI_UINT32 triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        buffer[cur_len++] = alphabet[(triple >> 3 * 6) & 0x3F];
        buffer[cur_len++] = alphabet[(triple >> 2 * 6) & 0x3F];
        buffer[cur_len++] = alphabet[(triple >> 1 * 6) & 0x3F];
        buffer[cur_len++] = alphabet[(triple >> 0 * 6) & 0x3F];
    }

    buffer[cur_len] = '\0';
    ok = RTI_TRUE;

done:
    return ok;
}
