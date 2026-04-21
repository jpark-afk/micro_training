/*
 * FILE: RTPSChecksum.c - Common checksum utilities
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   RTPS_ChecksumClass_is_valid
 * 04jan2021,tk
 *   MICRO-2782/PR#28522
 *     - Updated description of return value for RTPS_ChecksumClass_is_equal()
 *     - Return RTI_TRUE or RTI_FALSE.
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 */
/*ce
 * \file
 * \brief Checksum calculation utilities.
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
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

#ifndef RTI_CERT
/*ci \brief Check if a checksum class is valid
 *
 * \details
 * A valid checksum class must either have id and checksum function set to NULL,
 * or specify a negative class id and a checksum function different from
 * NULL.
 *
 * \param[in] function The class to check.
 *
 * \return TRUE if the class is valid, FALSE if not.
 */
RTI_BOOL
RTPS_ChecksumClass_is_valid(const struct RTPS_ChecksumClass *function)
{

    if (((function->class_id == 0) && (function->checksum_calculate == NULL)) ||
        ((function->class_id < 0) && (function->checksum_calculate != NULL)))
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}
#endif

/*ci \brief Check if a checksum class is supported
 *
 * \param[in] function The class to check.
 *
 * \return TRUE if the class is supported, FALSE if not.
 */
RTI_BOOL
RTPS_ChecksumClass_is_supported(const struct RTPS_ChecksumClass *function)
{

    if ((function->class_id != 0) &&
        (function->checksum_calculate != NULL))
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci \brief Test if two checksum classes are equal
 *
 * \param[in] left Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return TRUE if left and right are equal, FALSE otherwise.
 */
RTI_BOOL
RTPS_ChecksumClass_is_equal(const struct RTPS_ChecksumClass *left,
                            const struct RTPS_ChecksumClass *right)
{
    return ((left->class_id == right->class_id) &&
            (left->context == right->context) &&
            (left->checksum_calculate == right->checksum_calculate)) ?
                    RTI_TRUE : RTI_FALSE;
}
