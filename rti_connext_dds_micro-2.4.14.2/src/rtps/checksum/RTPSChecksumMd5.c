/*
 * FILE: RTPSChecksumMd5.c - Implementation of MD5
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
#ifndef cdr_md5_h
#include "cdr/cdr_md5.h"
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
 *                          If filled with 0, will calculate the MD5 checksum that
 *                          should be used to send the message. If different from 0,
 *                          will overwrite this buffer with the resulting division:
 *                          if the result is different from 0, the message is corrupted.
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_BOOL
RTPS_BuiltinMD5_checksum_calculate(void *context,
                                   const struct REDA_Buffer *buf,
                                   RTI_UINT32 buf_length,
                                   RTPS_Checksum_T *checksum)
{
    struct CDR_Stream_t stream;
    RTI_BOOL retval;

    UNUSED_ARG(buf_length);
    UNUSED_ARG(context);

    /* set_buffer cannot fail with these arguments */
    retval = CDR_Stream_set_buffer(&stream,buf[0].pointer,buf[0].length);
    IGNORE_RETVAL(retval);

    /* CDR_Stream_set_current_position_offset cannot fail when both stream
     * and length are valid arguments (same as above). Thus, ignore the return
     * values.
     */
    retval = CDR_Stream_set_current_position_offset(&stream,buf[0].length);
    IGNORE_RETVAL(retval);

    NDDSCDR_Stream_compute_MD5(&stream,checksum->checksum128);

    return RTI_TRUE;
}
