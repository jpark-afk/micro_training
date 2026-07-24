/*
 * FILE: LocatorSeq.c - InstanceHandle sequence implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 24oct2008,tk Created
 */
/*ce
 * \file
 * \brief Locator sequence implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif

/*** SOURCE_BEGIN ***/

#define T struct RTPS_Locator
#define TSeq DDS_LocatorSeq
#define TSeq_is_equal
#define TSeq_loan_contiguous
#define TSeq_unloan
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/*ci \brief Determine if the sequence is an extended locator or not.
 *
 * \details
 * Functions that take a DDS_LocatorSeq and a DDS_LocatorExSeq must check
 * which sequence type it is before accessing the elements. Due to the different
 * element sizes, if the sequence is a DDS_LocatorSeq then it must be
 * accessed with the DDS_LocatorSeq API, and if it is a DDS_LocatorExSeq then
 * it must be accessed with the DDS_LocatorExSeq API.
 *
 * \param [in] seq The sequence to check
 *
 * \return RTI_TRUE if the sequence is an extended locator sequence,
 * otherwise RTI_FALSE.
 */
RTI_BOOL
DDS_LocatorSeq_is_extended(const struct DDS_LocatorSeq *seq)
{
    if (seq->_element_size == sizeof(struct DDS_LocatorEx))
    {
        return RTI_TRUE;
    }
    return RTI_FALSE;
}

const struct DDS_LocatorSeq DDS_NullLocatorSeq =
        REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(NULL,0,0,struct DDS_Locator);

#define T struct DDS_LocatorEx
#define TSeq DDS_LocatorExSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#define TSeq_loan_contiguous
#define TSeq_unloan
#include "reda/reda_sequence_defn.h"

/*ci \brief check if a locator sequence is equal to an extended locator
 * sequence by ignoring the extended fields in the extended locator sequence.
 *
 * \param[in] left The locator sequence to compare
 * \param[in] right The extended locator sequence to compare
 *
 * \return RTI_TRUE if the sequences are equal, RTI_FALSE otherwise.
 */
RTI_BOOL
DDS_LocatorSeq_is_equal_to(const struct DDS_LocatorSeq *left,
                           const struct DDS_LocatorExSeq *right)
{
    RTI_INT32 i;

    if (DDS_LocatorSeq_get_length(left)
        != DDS_LocatorExSeq_get_length(right))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < DDS_LocatorSeq_get_length(left); ++i)
    {
        const struct DDS_Locator *left_locator =
                                DDS_LocatorSeq_get_reference(left, i);
        const struct DDS_LocatorEx *right_locator =
                                DDS_LocatorExSeq_get_reference(right, i);

        if (left_locator->kind != right_locator->kind)
        {
            return RTI_FALSE;
        }

        if (left_locator->port != right_locator->port)
        {
            return RTI_FALSE;
        }

        if (OSAPI_Memory_compare(left_locator->address,
                                 right_locator->address,
                                 sizeof(right_locator->address)) != 0)
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci \brief copy a DDS_LocatorExSeq to a DDS_LocatorSeq by ignoring the
 *    extended fields in the extended locator sequence.
 *
 * \param[out] dst_seq The sequence to copy into
 * \param[in] src_seq The sequence to copy from
 *
 * \return RTI_TRUE if the copy was successful, RTI_FALSE otherwise.
 */
RTI_BOOL
DDS_LocatorSeq_copy_from(struct DDS_LocatorSeq *dst_seq,
                         const struct DDS_LocatorExSeq *src_seq)
{
        RTI_INT32 index,length;

        length = DDS_LocatorExSeq_get_length(src_seq);
        if (length > DDS_LocatorSeq_get_maximum(dst_seq))
        {
            if (!DDS_LocatorSeq_set_maximum(dst_seq,length))
            {
                return RTI_FALSE;
            }
        }

        if (!DDS_LocatorSeq_set_length(dst_seq,length))
        {
            return RTI_FALSE;
        }

        for (index = 0; index < length; index++)
        {
            struct DDS_Locator *dst_locator;
            const struct DDS_LocatorEx *src_locator;

            src_locator = DDS_LocatorExSeq_get_reference(src_seq, index);
            dst_locator = DDS_LocatorSeq_get_reference(dst_seq, index);

            dst_locator->kind = src_locator->kind;
            dst_locator->port = src_locator->port;
            OSAPI_Memory_copy(dst_locator->address,
                            src_locator->address,
                            sizeof(dst_locator->address));
        }

        return RTI_TRUE;
}

/*ci \brief copy a DDS_LocatorSeq to a DDS_LocatorExSeq and clear the
 *    extended fields in the extended locator sequence.
 *
 * \param[out] dst_seq The sequence to copy into
 * \param[in] src_seq The sequence to copy from
 *
 * \return RTI_TRUE if the copy was successful, RTI_FALSE otherwise.
 */
RTI_BOOL
DDS_LocatorExSeq_copy_from(struct DDS_LocatorExSeq *dst_seq,
                           const struct DDS_LocatorSeq *src_seq)
{
        RTI_INT32 index,length;

        length = DDS_LocatorSeq_get_length(src_seq);
        if (length > DDS_LocatorExSeq_get_maximum(dst_seq))
        {
            if (!DDS_LocatorExSeq_set_maximum(dst_seq,length))
            {
                return RTI_FALSE;
            }
        }

        if (!DDS_LocatorExSeq_set_length(dst_seq,length))
        {
            return RTI_FALSE;
        }

        for (index = 0; index < length; index++)
        {
            const struct DDS_Locator *src_locator;
            struct DDS_LocatorEx *dst_locator;

            src_locator = DDS_LocatorSeq_get_reference(src_seq, index);
            dst_locator = DDS_LocatorExSeq_get_reference(dst_seq, index);

            dst_locator->kind = src_locator->kind;
            dst_locator->port = src_locator->port;
            OSAPI_Memory_copy(dst_locator->address,
                            src_locator->address,
                            sizeof(dst_locator->address));
            dst_locator->length = 0;
            OSAPI_Memory_zero(dst_locator->encapsulations,
                            sizeof(dst_locator->encapsulations));
        }

        return RTI_TRUE;
}

/*ci @} */
