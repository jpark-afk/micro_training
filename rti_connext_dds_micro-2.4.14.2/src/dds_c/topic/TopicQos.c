/*
 * FILE: TopicQos.c - DDS TopicQos implementation
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
 * 16sep2014,tk MICRO-872 Removed empty DDS_TopicQos_is_consistent
 * 20may2014,tk MICRO-792 Qos consistency checks
 * 06may2012,tk Major update
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief DDS TopicQos implementation
 *
 * \details
 * This file implements functions to manage the TopicQos policy
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif
#include "QosPolicy.h"
#include "TopicQos.h"

const struct DDS_TopicQos DDS_TOPIC_QOS_DEFAULT = DDS_TopicQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_TopicQos_copy(struct DDS_TopicQos *out, const struct DDS_TopicQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    *out = *in;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_TopicQos_initialize(struct DDS_TopicQos *self)
{
    struct DDS_TopicQos initVal = DDS_TopicQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_TopicQos_finalize(struct DDS_TopicQos * self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Check that the immutable part of two topic qos policies are valid
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_TopicQos_immutable_is_equal(const struct DDS_TopicQos *left,
                                const struct DDS_TopicQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!RTI_ManagementQosPolicy_is_equal(&left->management,&right->management))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_TopicQos_is_equal(const struct DDS_TopicQos *left,
                      const struct DDS_TopicQos *right)
{
    OSAPI_PRECONDITION_ALWAYS(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_TopicQos_immutable_is_equal(left,right);
}
#endif /* !RTI_CERT */

/*ci @} */
