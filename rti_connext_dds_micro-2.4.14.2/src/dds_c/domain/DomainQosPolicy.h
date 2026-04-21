/*
 * FILE: DomainQosPolicy.h
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
 * 20sep2014,as  Created
 */
/*ce
 * \file
 * \brief  Implementation of QosPolicy defined by domain module of DCPS
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainQosPolicy_h
#define DomainQosPolicy_h

/*i \dref_UserTrafficQosPolicy_is_equal
 */
MUST_CHECK_RETURN DDS_Boolean
DDS_UserTrafficQosPolicy_is_equal(
            const struct DDS_UserTrafficQosPolicy* self,
            const struct DDS_UserTrafficQosPolicy* from);

/*i \dref_UserTrafficQosPolicy_copy
 */
DDS_ReturnCode_t
DDS_UserTrafficQosPolicy_copy(
        struct DDS_UserTrafficQosPolicy* self,
        const struct DDS_UserTrafficQosPolicy* from);

#ifndef RTI_CERT
/*i \dref_UserTrafficQosPolicy_finalize
 */
MUST_CHECK_RETURN DDS_ReturnCode_t
DDS_UserTrafficQosPolicy_finalize(struct DDS_UserTrafficQosPolicy* self);
#endif /* !RTI_CERT */


#endif

/*ci @} */
