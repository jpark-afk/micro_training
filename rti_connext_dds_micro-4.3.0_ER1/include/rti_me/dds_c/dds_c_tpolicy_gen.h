/*
 * FILE: dds_c_topic.h - DDS topic module
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 30apr2008  Created
 */
/*i @file
 * @ingroup DDSDomainModule
 */
#ifndef dds_c_tpolicy_gen_h
#define dds_c_tpolicy_gen_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef concatenate
#define concatenate(A, B)   A ## B
#endif

#ifndef T_is_equal
#define _is_equal(T) concatenate(T,_is_equal)
#define T_is_equal _is_equal(T)

DDS_Boolean
T_is_equal(const struct T *left, const struct T *right)
{
    if ((left == NULL) || (right == NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    return !OSAPI_Memory_compare(left, right, sizeof(struct T));
}

#undef T_is_equal

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#undef concatenate

#undef dds_c_tpolicy_gen_h


#endif /* dds_c_tpolicy_gen_h */
