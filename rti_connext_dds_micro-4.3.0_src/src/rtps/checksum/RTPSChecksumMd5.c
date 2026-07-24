/*
 * FILE: RTPSChecksumMd5.c - Implementation of MD5
 *
 * Copyright 2020-2024 Real-Time Innovations, Inc.
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
 * - Added missing file section.
 * 04jan2021,tk
 *   MICRO-2783/PR#28523
 *     - Ignore the return values from the function calls in
 *       RTPS_BuiltinMD5_checksum_calculate() since neither function can
 *       fail.
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 */
/*ce
 * \file
 * \brief MD5 checksum calculation.
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_hash_h
#include "osapi/osapi_hash.h"
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

#include "RTPSChecksum.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Calculate MD5
 *
 * \details
 * Calculates MD5 checksum for buf.
 *
 * \param[in]   context
 * \param[in]   buf         The buffer to calculate the checksum from.
 * \param[in]   buf_length  The length of buf
 * \param[out]  checksum    The calculated MD5 checksum. Cannot be Null.
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_BOOL
RTPS_BuiltinMD5_checksum_calculate(void *context,
                                   const struct REDA_Buffer *buf,
                                   RTI_UINT32 buf_length,
                                   RTPS_Checksum_T *checksum)
{
    UNUSED_ARG(context);

    OSAPI_PRECONDITION((buf == NULL) || (checksum == NULL) || (buf_length < 1),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("buf",buf,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("buf_length",buf_length,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("checkusm",checksum,RTI_TRUE););

    OSAPI_Hash_compute_buffer_scatter_md5(buf, buf_length, checksum->checksum128);

    return RTI_TRUE;
}
