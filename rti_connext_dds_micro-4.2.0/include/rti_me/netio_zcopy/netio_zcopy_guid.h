/*
 * FILE: netio_zcopy_guid.h
 *
 * Copyright 2022-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_zcopy_guid_h
#define netio_zcopy_guid_h

#include "osapi/osapi_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*i
 * \defgroup ZCOPY_GuidClass ZCOPY_Guid
 *
 * \brief GUID used to identify notifiers and notifiees
 */

#define ZCOPY_GUID_LENGTH (16)

/*i
 * \ingroup ZCOPY_GuidClass
 *
 * GUID used to identify notifiers and notifiees
 */
struct ZCOPY_Guid
{
    unsigned char value[ZCOPY_GUID_LENGTH];
};

#define ZCOPY_Guid_INITIALIZER \
{ \
    { 0 } \
}

/*i
 * \ingroup ZCOPY_GuidClass
 *
 * \brief Check if two guids are equal
 *
 * \param[in] a The first guid
 * \param[in] b The second guid
 *
 * \return RTI_TRUE if equal. Otherwise, RTI_FALSE.
 */
extern RTI_BOOL
ZCOPY_Guid_equals(const struct ZCOPY_Guid *a, const struct ZCOPY_Guid *b);

/*i
 * \ingroup ZCOPY_GuidClass
 *
 * \brief Convert a guid and port to a string
 *
 * \param[in] guid        Guid
 * \param[in] port        Port
 * \param[in] prefix      NULL terminated prefix to prepend to the output string
 * \param[out] buffer     Output buffer to write the string to
 * \param[in] buffer_len  Length of the output buffer
 *
 * \return RTI_TRUE on success. RTI_FALSE if the buffer is too small.
 */
extern RTI_BOOL
ZCOPY_Guid_to_string(
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        const char *prefix,
        char *buffer,
        RTI_SIZE_T buffer_len);

/*i
 * \ingroup ZCOPY_GuidClass
 *
 * \brief Convert raw bytes to a string which can be used as a path name
 *
 * \param[in] bytes       Bytes to convert
 * \param[in] bytes_len   Length of the bytes
 * \param[out] buffer     Output buffer to write the string to
 * \param[in] buffer_len  Length of the output buffer
 *
 * \return RTI_TRUE on success. RTI_FALSE if the buffer is too small.
 */
extern RTI_BOOL
ZCOPY_Guid_bytes_to_string(
        const unsigned char *bytes,
        RTI_SIZE_T bytes_len,
        char *buffer,
        RTI_SIZE_T buffer_len);

/*i
 * \ingroup ZCOPY_GuidClass
 *
 * An invalid or uninitialized notification guid
 */
#define ZCOPY_NOTIF_GUID_INVALID                      \
{                                                     \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_guid_h */
