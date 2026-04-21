/*
 * FILE: QosPolicy.h - QoSPolicy implementation
 *
 * (c) Copyright 2008-2020 Real-Time Innovations,
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
 * 12mar2015,tk MICRO-1097 Removed unused DDS_TransportQosPolicy_is_consistent
 *                         prototype
 * 20may2014,tk MICRO-792: Qos consistency checks
 * 19jul2013,as Added support for C++ (functions for all QosPolicy types)
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief QoSPolicy implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef QosPolicy_h
#define QosPolicy_h

#ifdef __cplusplus
extern "C"
{
#endif


/****************************
    DDS_DurabilityQosPolicy
*****************************/

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DurabilityQosPolicy_is_equal(
        const struct DDS_DurabilityQosPolicy *left,
        const struct DDS_DurabilityQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DurabilityQosPolicy_is_consistent(const struct DDS_DurabilityQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DurabilityQosPolicy_is_compatible(const struct DDS_DurabilityQosPolicy *request,
                                      const struct DDS_DurabilityQosPolicy *offered);

/****************************
    DDS_DestinationOrderQosPolicy
*****************************/

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DestinationOrderQosPolicy_is_equal(
                    const struct DDS_DestinationOrderQosPolicy *left,
                    const struct DDS_DestinationOrderQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DestinationOrderQosPolicy_is_consistent(
                    const struct DDS_DestinationOrderQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DestinationOrderQosPolicy_is_compatible(
                    const struct DDS_DestinationOrderQosPolicy *request,
                    const struct DDS_DestinationOrderQosPolicy *offered);

/*************************
    DDS_DeadlineQosPolicy
*************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DeadlineQosPolicy_is_equal(const struct DDS_DeadlineQosPolicy *left,
                                 const struct DDS_DeadlineQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DeadlineQosPolicy_is_consistent(const struct DDS_DeadlineQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DeadlineQosPolicy_is_compatible(const struct DDS_DeadlineQosPolicy *request,
                                    const struct DDS_DeadlineQosPolicy *offered);

/*************************
    DDS_PresentationQosPolicy
*************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_PresentationQosPolicy_is_compatible(const struct DDS_PresentationQosPolicy *request,
                                        const struct DDS_PresentationQosPolicy *offered);

/*************************
    DDS_HistoryQosPolicy
*************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_HistoryQosPolicy_is_equal(const struct DDS_HistoryQosPolicy *left,
                                 const struct DDS_HistoryQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_HistoryQosPolicy_is_consistent(const struct DDS_HistoryQosPolicy *self);

/******************************************
    DDS_SystemResourceLimitsQosPolicy
******************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_SystemResourceLimitsQosPolicy_immutable_is_equal(
                    const struct DDS_SystemResourceLimitsQosPolicy *left,
                    const struct DDS_SystemResourceLimitsQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SystemResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_SystemResourceLimitsQosPolicy *self);

/**************************************************
    DDS_ResourceLimitsQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_ResourceLimitsQosPolicy_is_equal(
        const struct DDS_ResourceLimitsQosPolicy *left,
        const struct DDS_ResourceLimitsQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_ResourceLimitsQosPolicy_is_consistent(
                                const struct DDS_ResourceLimitsQosPolicy *self);

/**************************************************
    DDS_OwnershipQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_OwnershipQosPolicy_is_equal(
        const struct DDS_OwnershipQosPolicy *left,
        const struct DDS_OwnershipQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_OwnershipQosPolicy_is_consistent(const struct DDS_OwnershipQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_OwnershipQosPolicy_is_compatible(const struct DDS_OwnershipQosPolicy *request,
                                     const struct DDS_OwnershipQosPolicy *offered);

/**************************************************
    DDS_OwnershipStrengthQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_OwnershipStrengthQosPolicy_is_equal(
        const struct DDS_OwnershipStrengthQosPolicy *left,
        const struct DDS_OwnershipStrengthQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_OwnershipStrengthQosPolicy_is_consistent(
                            const struct DDS_OwnershipStrengthQosPolicy *self);

/**************************************************
    DDS_LivelinessQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_LivelinessQosPolicy_is_equal(const struct DDS_LivelinessQosPolicy *left,
                                 const struct DDS_LivelinessQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_LivelinessQosPolicy_is_consistent(
                                    const struct DDS_LivelinessQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_LivelinessQosPolicy_is_compatible(const struct DDS_LivelinessQosPolicy *request,
                                      const struct DDS_LivelinessQosPolicy *offered);

/**************************************************
    DDS_ReliabilityQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_ReliabilityQosPolicy_is_equal(
        const struct DDS_ReliabilityQosPolicy *left,
        const struct DDS_ReliabilityQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_ReliabilityQosPolicy_is_consistent(
                                   const struct DDS_ReliabilityQosPolicy *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_ReliabilityQosPolicy_is_compatible(const struct DDS_ReliabilityQosPolicy *reader,
                                       const struct DDS_ReliabilityQosPolicy *writer);

/**************************************************
    DDS_TypeSupportQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_TransportQosPolicy_copy(struct DDS_TransportQosPolicy *left,
                            const struct DDS_TransportQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeSupportQosPolicy_is_equal(
        const struct DDS_TypeSupportQosPolicy *left,
        const struct DDS_TypeSupportQosPolicy *right);

#if 0
MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeSupportQosPolicy_is_consistent(
                                    const struct DDS_TypeSupportQosPolicy *self);
#endif

/**************************************************
    DDS_DataWriterProtocolQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterProtocolQosPolicy_is_equal(
        const struct DDS_DataWriterProtocolQosPolicy *left,
        const struct DDS_DataWriterProtocolQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterProtocolQosPolicy_is_consistent(
                            const struct DDS_DataWriterProtocolQosPolicy *self);

/**************************************************
    DDS_DataReaderResourceLimitsQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderResourceLimitsQosPolicy_is_equal(
        const struct DDS_DataReaderResourceLimitsQosPolicy *left,
        const struct DDS_DataReaderResourceLimitsQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_DataReaderResourceLimitsQosPolicy *self);


/**************************************************
    DDS_TransportQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_TransportQosPolicy_is_equal(const struct DDS_TransportQosPolicy *left,
                                const struct DDS_TransportQosPolicy *right);

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_TransportQosPolicy_finalize(struct DDS_TransportQosPolicy *self);
#endif /* !RTI_CERT */

/**************************************************
    DDS_RtpsReliableWriterProtocol_t
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_RtpsReliableWriterProtocol_t_is_equal(
        const struct DDS_RtpsReliableWriterProtocol_t *left,
        const struct DDS_RtpsReliableWriterProtocol_t *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_RtpsReliableWriterProtocol_is_consistent(
                          const struct DDS_RtpsReliableWriterProtocol_t *self);

/**************************************************
    DDS_RtpsReliableReaderProtocol_t
 **************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_RtpsReliableReaderProtocol_is_equal(
        const struct DDS_RtpsReliableReaderProtocol_t *left,
        const struct DDS_RtpsReliableReaderProtocol_t *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_RtpsReliableReaderProtocol_is_consistent(
                          const struct DDS_RtpsReliableReaderProtocol_t *self);

/**************************************************
    DDS_EntityNameQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_EntityNameQosPolicy_is_equal(const struct DDS_EntityNameQosPolicy *left,
                                 const struct DDS_EntityNameQosPolicy *right);

/**************************************************
    DDS_DataReaderProtocolQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderProtocolQosPolicy_is_equal(
        const struct DDS_DataReaderProtocolQosPolicy *left,
        const struct DDS_DataReaderProtocolQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderProtocolQosPolicy_is_consistent(
        const struct DDS_DataReaderProtocolQosPolicy *self);

/**************************************************
    DDS_WireProtocolQosPolicy
**************************************************/

MUST_CHECK_RETURN extern DDS_Boolean
DDS_WireProtocolQosPolicy_is_equal(const struct DDS_WireProtocolQosPolicy *left,
                                 const struct DDS_WireProtocolQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_WireProtocolQosPolicy_is_consistent(const struct DDS_WireProtocolQosPolicy *self);

/**************************************************
    DDS_DataWriterResourceLimitsQosPolicy
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
RTI_ManagementQosPolicy_is_equal(const struct RTI_ManagementQosPolicy *left,
                                 const struct RTI_ManagementQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterResourceLimitsQosPolicy_is_equal(
        const struct DDS_DataWriterResourceLimitsQosPolicy *left,
        const struct DDS_DataWriterResourceLimitsQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_DataWriterResourceLimitsQosPolicy *self);


/**************************************************
    DDS_DomainParticipantResourceLimitsQosPolicy
**************************************************/

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantResourceLimitsQosPolicy_is_equal(
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *left,
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantResourceLimitsQosPolicy_is_consistent(
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *self);

/**************************************************
    DDS_ChecksumProperty
**************************************************/
MUST_CHECK_RETURN extern DDS_Boolean
DDS_ChecksumProperty_is_equal(const struct DDS_ChecksumProperty *left,
                              const struct DDS_ChecksumProperty *right);

#ifdef __cplusplus
}
#endif


#endif /* QosPolicy_pkg_h */

/*ci @} */
