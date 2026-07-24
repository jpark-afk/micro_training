/*
 * FILE: DomainFactoryQos.c - DomainFactoryQos implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2026.
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
 * 13jul2015,tk MICRO-1421/PR#15309 Added max_components to is_equal()
 * 29jul2014,tk MICRO-854/PR#10209: Use correct boolean type
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DomainParticipantFactoryQos implementation
 *
 * \details
 * This file implements functions to manage the life-cycle of the participant
 * factory QoS policy.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "Entity.h"
#include "QosPolicy.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DomainFactoryQos.h"

const struct DDS_DomainParticipantFactoryQos DDS_PARTICIPANT_FACTORY_QOS_DEFAULT
                                 = DDS_DomainParticipantFactoryQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_copy(
                        struct DDS_DomainParticipantFactoryQos *out,
                        const struct DDS_DomainParticipantFactoryQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    *out = *in;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_initialize(
                        struct DDS_DomainParticipantFactoryQos *self)
{
    struct DDS_DomainParticipantFactoryQos initVal =
                            DDS_DomainParticipantFactoryQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_finalize(
                        struct DDS_DomainParticipantFactoryQos* self)
{
    PRECOND_ARG(self)
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

DDS_Boolean
DDS_DomainParticipantFactoryQos_immutable_is_equal(
                     const struct DDS_DomainParticipantFactoryQos *left,
                     const struct DDS_DomainParticipantFactoryQos *right)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    retval = DDS_SystemResourceLimitsQosPolicy_immutable_is_equal(
                            &left->resource_limits, &right->resource_limits);

    return retval;
}

DDS_Boolean
DDS_DomainParticipantFactoryQos_is_equal(
                        const struct DDS_DomainParticipantFactoryQos *left,
                        const struct DDS_DomainParticipantFactoryQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

   return ((left->entity_factory.autoenable_created_entities ==
            right->entity_factory.autoenable_created_entities) &&
           (left->resource_limits.max_participants ==
            right->resource_limits.max_participants) &&
            (left->resource_limits.max_components ==
             right->resource_limits.max_components));
}

DDS_Boolean
DDS_DomainParticipantFactoryQos_is_consistent(
                          const struct DDS_DomainParticipantFactoryQos *self)
{

    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return
        DDS_SystemResourceLimitsQosPolicy_is_consistent(&self->resource_limits);
}

/*ci @} */

