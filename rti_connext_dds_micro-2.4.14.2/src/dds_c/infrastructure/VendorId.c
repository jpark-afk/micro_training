/*
 * FILE: VendorId.c - VendorId implementation
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
 * \brief VendorId implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Test if two DDS_VendorId structures are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_VendorId_is_equal(const struct DDS_VendorId *left,
                      const struct DDS_VendorId *right)
{
    if ((left == NULL) || (right == NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    
    return OSAPI_Memory_compare(left, right, sizeof(struct DDS_VendorId)) == 0 ? 
                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */

