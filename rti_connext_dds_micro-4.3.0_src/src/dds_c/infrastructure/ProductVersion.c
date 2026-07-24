/*
 * FILE: ProductVersion.c - ProductVersion implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 20sep2014,as Removed use of deprecated header dds_c_tpolicy_gen.h
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief ProductVersion implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare two DDS_ProductVersion structures
 *
 * @pre Both arguments must be non-NULL.
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return less than, equal to, or greater than  0, according to whether left is
 *         logically less than, equal to, or greater than right.
 */
DDS_Long
DDS_ProductVersion_compare(const struct DDS_ProductVersion* left,
                           const struct DDS_ProductVersion* right)
{
    DDS_Long diff;

    diff = left->major - right->major;
    if (diff != 0)
    {
        goto done;
    }

    diff = left->minor - right->minor;
    if (diff != 0)
    {
        goto done;
    }

    diff = left->release - right->release;
    if (diff != 0)
    {
        goto done;
    }

    diff = left->revision - right->revision;

done:
    return diff;
}

/*ci @} */
