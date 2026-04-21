/*
 * FILE: SubscriberQos.c - Subscriber Qos implementation
 *
 * (c) Copyright 2008-2021 Real-Time Innovations,
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
 * 18may2022,tk MICRO-3572/PR.30496
 * - Return DDS_RETCODE_BAD_PARAMETER instead of
 *   DDS_RETCODE_PRECONDITION_NOT_MET from:
 *   - DDS_SubscriberQos_initialize
 *   - DDS_SubscriberQos_copy
 *   - DDS_SubscriberQos_finalize (not relevant for Cert)
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Removed UNUSED_ARG(self) in DDS_SubscriberQos_is_consistent
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_SubscriberQos_is_equal
 * 24feb2021,tk MICRO-2913/PR#28851
 *   - Added always enable preconditions for DDS_SubscriberQos_copy
 *     and DDS_SubscriberQos_initialize.
 * 20feb2021,tk MICRO-2832/PR#28650
 *   - Updated function comment for DDS_SubscriberQos_is_consistent()
 * 14dec2020,tk
 *     - MICRO-2680/PR.28249 Call RTI_ManagementQosPolicy_is_equal() in
 *       DDS_SubscriberQos_is_equal() instead of direct comparisons.
 *     - MICRO-2684/PR.28270 Return FALSE in is_consistent() if
 *       disable_unregister_dispose_for_unpublished_instance is TRUE
 * 16sep2014,tk MICRO-897/PR#10783 Fixed return value in precondition test
 * 28apr2014,as MICRO-371 (Verocel PR#1527) Removed use of OSAPI_Memory_compare
 *              in DDS_SubscriberQos_is_equal in favor of direct comparison of
 *              qos structure's members.
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber Qos implementation
 *
 * \details
 * This file implements functions to manage the SubscriberQos structure
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "QosPolicy.h"
#include "SubscriberQos.h"

const struct DDS_SubscriberQos DDS_SUBSCRIBER_QOS_DEFAULT =
                                                DDS_SubscriberQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Check that a Subscriber qos is consistent
 *
 * \param[in] self DDS_SubscriberQos to check
 *
 * \return DDS_BOOLEAN_TRUE if the DDS_SubscriberQos
 *         is consistent, DDS_BOOLEAN_FALSE otherwise.
 */
DDS_Boolean
DDS_SubscriberQos_is_consistent(const struct DDS_SubscriberQos *self)
{
    if (self->management.disable_unregister_dispose_for_unpublished_instance)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_ReturnCode_t
DDS_SubscriberQos_copy(struct DDS_SubscriberQos *out,
                       const struct DDS_SubscriberQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    *out = *in;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_SubscriberQos_initialize(struct DDS_SubscriberQos *self)
{

    struct DDS_SubscriberQos initVal = DDS_SubscriberQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    *self = initVal;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_SubscriberQos_finalize(struct DDS_SubscriberQos *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)


    return DDS_RETCODE_OK;
}
#endif /*RTI_CERT*/

DDS_Boolean
DDS_SubscriberQos_is_equal(const struct DDS_SubscriberQos *left,
                           const struct DDS_SubscriberQos *right)
{
    OSAPI_PRECONDITION_ALWAYS(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->entity_factory.autoenable_created_entities ==
             right->entity_factory.autoenable_created_entities) &&
            RTI_ManagementQosPolicy_is_equal(&left->management,
                                             &right->management))
                    ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */

