/*
 * FILE: PublisherQos.c - Publisher Qos implementation
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
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_PublisherQos_is_equal
 * 24feb2021,tk MICRO-2913/PR#28851
 *   - Added always enable preconditions for DDS_PublisherQos_copy
 *     and DDS_PublisherQos_initialize.
 * 20feb2021,tk MICRO-2832/PR#28650
 *   - Updated function comment for DDS_PublisherQos_is_consistent()
 * 14dec2020,tk
 *     - MICRO-2680/PR.28249 Call RTI_ManagementQosPolicy_is_equal() in
 *       DDS_SubscriberQos_is_equal() instead of direct comparisons.
 *     - MICRO-2684/PR.28270 Return FALSE in is_consistent() if
 *       disable_unregister_dispose_for_unpublished_instance is TRUE
 * 28apr2014,as MICRO-371 (Verocel PR#1527) Removed use of OSAPI_Memory_compare
 *              in DDS_PublisherQos_is_equal in favor of direct comparison of
 *              qos structure's members.
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Publisher Qos implementation
 *
 * \details
 * This file implements functions to manage the PublisherQos structure
 *
 * \ingroup DDSDomainModule
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifndef dds_c_publication_h
  #include "dds_c/dds_c_publication.h"
#endif

const struct DDS_PublisherQos DDS_PUBLISHER_QOS_DEFAULT =
                                                DDS_PublisherQos_INITIALIZER;

#include "QosPolicy.h"
#include "PublisherQos.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_PublisherQos_copy(struct DDS_PublisherQos *out,
                      const struct DDS_PublisherQos *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)
    *out = *in;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_PublisherQos_initialize(struct DDS_PublisherQos *self)
{
    struct DDS_PublisherQos initVal = DDS_PublisherQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_PublisherQos_finalize(struct DDS_PublisherQos *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_RETCODE_OK;
}
#endif /*RTI_CERT*/


DDS_Boolean
DDS_PublisherQos_is_equal(const struct DDS_PublisherQos *left,
                          const struct DDS_PublisherQos *right)
{
    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->entity_factory.autoenable_created_entities ==
             right->entity_factory.autoenable_created_entities) &&
             RTI_ManagementQosPolicy_is_equal(&left->management,
                                              &right->management))
                ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a Publisher qos is consistent
 *
 * \param[in] self DDS_PublisherQos to check for consistency
 *
 * \return DDS_BOOLEAN_TRUE if the DDS_PublisherQos is consistent,
 *         DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_PublisherQos_is_consistent(const struct DDS_PublisherQos *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->management.disable_unregister_dispose_for_unpublished_instance)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
/*ci @} */


