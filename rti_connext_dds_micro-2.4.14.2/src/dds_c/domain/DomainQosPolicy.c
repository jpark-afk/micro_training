/*
 * FILE: DomainQosPolicy.c
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
 * 20sep2014,as Created
 */
/*ce
 * \file
 * \brief  Implementation of QosPolicy defined by domain module of DCPS
 *
 * \details
 * This file implements functions to mangage the life-cycle of QoS policies
 * which requires life-cycle methods that explicity calls other type specific
 * life-cycle methods, such as copy, initialize and finalize.
 *
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "DomainQosPolicy.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_DiscoveryQosPolicy structure
 *
 * \param[in] self DDS_DiscoveryQosPolicy structure to finalize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDS_ReturnCode_t
DDS_DiscoveryQosPolicy_finalize(struct DDS_DiscoveryQosPolicy* self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)


    if (!DDS_StringSeq_finalize(&self->enabled_transports) ||
        !DDS_StringSeq_finalize(&self->initial_peers))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Copy a DDS_DiscoveryQosPolicy structure
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 *
 * \param[inout] self The destination DDS_DiscoveryQosPolicy structure
 * \param[in]    from The source DDS_DiscoveryQosPolicy structure
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDS_ReturnCode_t
DDS_DiscoveryQosPolicy_copy(struct DDS_DiscoveryQosPolicy* self,
                            const struct DDS_DiscoveryQosPolicy* from)
{
    struct DDS_StringSeq *seq_copy_result = NULL;

    OSAPI_PRECONDITION(self == NULL || from == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("from",from,RTI_TRUE);)

    seq_copy_result =
        DDS_StringSeq_copy(&self->enabled_transports, &from->enabled_transports);

    if (seq_copy_result == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    seq_copy_result =
               DDS_StringSeq_copy(&self->initial_peers, &from->initial_peers);

    if (seq_copy_result == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    self->discovery = from->discovery;
    self->accept_unknown_peers = from->accept_unknown_peers;

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Compare two DDS_DiscoveryQosPolicy structures for equality
 *
 * \param[in] self  The left side of the comparison
 * \param[in] from  The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_DiscoveryQosPolicy_is_equal(const struct DDS_DiscoveryQosPolicy* self,
                                const struct DDS_DiscoveryQosPolicy* from)
{
    OSAPI_PRECONDITION(from == NULL || self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("from",from,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_StringSeq_is_equal(&self->enabled_transports,
                                &from->enabled_transports))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_StringSeq_is_equal(&self->initial_peers, &from->initial_peers))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (REDA_String_ncompare(RT_ComponentFactoryId_get_name(
                                                     &self->discovery.name),
                             RT_ComponentFactoryId_get_name(
                                                     &from->discovery.name),
                                                     RT_MAX_FACTORY_NAME) != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (OSAPI_Memory_compare(&self->discovery.property,
                             &from->discovery.property,
                             sizeof(self->discovery.property)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->accept_unknown_peers != from->accept_unknown_peers)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_UserTrafficQosPolicy_finalize structure
 *
 * \param[in] self DDS_UserTrafficQosPolicy_finalize structure to finalize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDS_ReturnCode_t
DDS_UserTrafficQosPolicy_finalize(struct DDS_UserTrafficQosPolicy* self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)


    if (!DDS_StringSeq_finalize(&self->enabled_transports))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Copy a DDS_UserTrafficQosPolicy structure
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 *
 * \param[inout] self The destination DDS_UserTrafficQosPolicy structure
 * \param[in]    from The source DDS_UserTrafficQosPolicy structure
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDS_ReturnCode_t
DDS_UserTrafficQosPolicy_copy(struct DDS_UserTrafficQosPolicy* self,
                              const struct DDS_UserTrafficQosPolicy* from)
{
    struct DDS_StringSeq *seq_copy_result = NULL;
    OSAPI_PRECONDITION(self == NULL || from == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("from",from,RTI_TRUE);)

    seq_copy_result =
      DDS_StringSeq_copy(&self->enabled_transports, &from->enabled_transports);

    if (seq_copy_result == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Compare two DDS_UserTrafficQosPolicy structures for equality
 *
 * \param[in] self  The left side of the comparison
 * \param[in] from  The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_UserTrafficQosPolicy_is_equal(const struct DDS_UserTrafficQosPolicy* self,
                                  const struct DDS_UserTrafficQosPolicy* from)
{
    OSAPI_PRECONDITION(self == NULL || from == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("from",from,RTI_TRUE);)

    if (!DDS_StringSeq_is_equal(
                        &self->enabled_transports, &from->enabled_transports))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci @} */
