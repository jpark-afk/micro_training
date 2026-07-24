/*
 * FILE: UserDataQosPolicy.h - User Data QoS API definitions
 *
 * (c) Copyright, Real-Time Innovations, 2023-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief User Data QoS API definitions
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef UserDataQosPolicy_h
#define UserDataQosPolicy_h

#include "dds_c/dds_c_user_data_qos.h"
#include "dds_c/dds_c_discovery.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef RTI_CERT
/*ci
 *
 * \brief Finalize the DDS_UserDataQosPolicy
 *
 * \param[in]  policy DDS_UserDataQosPolicy to be finalized
 *
 * \return RTI_TRUE if DDS_UserDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_UserDataQosPolicy_finalize(struct DDS_UserDataQosPolicy *policy);
#endif /* !RTI_CERT */

/*ci
 * \brief Finalize the DDS_UserDataQosPolicy without deallocating
 *
 * \param[in]  policy DDS_UserDataQosPolicy to be finalized.
 *
 * \return RTI_TRUE if DDS_UserDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_UserDataQosPolicy_finalize_no_dealloc(
        struct DDS_UserDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type);

/*ci
 * \brief Initialize the DDS_UserDataQosPolicy
 *
 * \param[in]  policy DDS_UserDataQosPolicy to be initialized
 *
 * \return RTI_TRUE if the DDS_UserDataQosPolicy instance is initialized
 *         successfully, RTI_FALSE otherwise
 */
DDS_ReturnCode_t
DDS_UserDataQosPolicy_initialize(struct DDS_UserDataQosPolicy *policy);

/*ci
 * \brief Make a copy of the DDS_UserDataQosPolicy instance.
 *
 * \param[in]   from_policy Pointer to DDS_UserDataQosPolicy instance to be copied
 * \param[out]  to_policy Pointer to DDS_UserDataQosPolicy which will be a copy
 *              of the from_policy.
 *
 * \return RTI_TRUE if the DDS_UserDataQosPolicy instance is copied successfully,
 *         RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_UserDataQosPolicy_copy(struct DDS_UserDataQosPolicy *to_policy,
                            const struct DDS_UserDataQosPolicy *from_policy);

/*ci
 * \brief Check if the DDS_UserDataQosPolicy has been populated consistently.
 *
 * \brief A DDS_UserDataQosPolicy is consistent if the length of its octet
 *        sequence is less than or equal to max_length
 *
 * \param[in]  policy DDS_UserDataQosPolicy that has to be checked for consistency
 *
 * \return RTI_TRUE if the policy is consistent, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_UserDataQosPolicy_is_consistent(const struct DDS_UserDataQosPolicy *policy,
                                    DDS_Long max_length);

/*ci
 * \brief Check if two DDS_UserDataQosPolicy instances are equal
 *
 * \param[in]  left  Left side of comparison
 * \param[in]  right Right side of comparison
 * \return RTI_TRUE if left = right, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_UserDataQosPolicy_is_equal(const struct DDS_UserDataQosPolicy *left,
        const struct DDS_UserDataQosPolicy *right);

#ifndef RTI_CERT
/*ci
 * \brief Finalize the DDS_GroupDataQosPolicy
 *
 * \param[in]  policy DDS_GroupDataQosPolicy to be finalized
 *
 * \return RTI_TRUE if DDS_GroupDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_GroupDataQosPolicy_finalize(struct DDS_GroupDataQosPolicy *policy);
#endif /* !RTI_CERT */

/*ci
 * \brief Finalize the DDS_GroupDataQosPolicy without deallocating
 *
 * \param[in]  policy DDS_GroupDataQosPolicy to be finalized.
 *
 * \return RTI_TRUE if DDS_GroupDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_GroupDataQosPolicy_finalize_no_dealloc(
        struct DDS_GroupDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type);

DDS_ReturnCode_t
DDS_GroupDataQosPolicy_initialize(struct DDS_GroupDataQosPolicy *policy);

/*ci
 * \brief Make a copy of the DDS_GroupDataQosPolicy instance.
 *
 * \param[in]   from_policy Pointer to DDS_GroupDataQosPolicy instance to be copied
 * \param[out]  to_policy Pointer to DDS_GroupDataQosPolicy which will be a copy
 *              of the from_policy.
 *
 * \return RTI_TRUE if the DDS_GroupDataQosPolicy instance is copied successfully,
 *         RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_GroupDataQosPolicy_copy(struct DDS_GroupDataQosPolicy *to_policy,
                            const struct DDS_GroupDataQosPolicy *from_policy);

/*ci
 * \brief Check if the DDS_GroupDataQosPolicy has been populated consistently.
 *
 * \brief A DDS_GroupDataQosPolicy is consistent if the length of its octet
 *        sequence is less than or equal to max_length
 *
 * \param[in]  policy DDS_GroupDataQosPolicy that has to be checked for consistency
 *
 * \return RTI_TRUE if the policy is consistent, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_GroupDataQosPolicy_is_consistent(const struct DDS_GroupDataQosPolicy *policy,
                                    DDS_Long max_length);

/*ci
 * \brief Check if two DDS_GroupDataQosPolicy instances are equal
 *
 * \param[in]  left  Left side of comparison
 * \param[in]  right Right side of comparison
 * \return RTI_TRUE if left = right, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_GroupDataQosPolicy_is_equal(const struct DDS_GroupDataQosPolicy *left,
        const struct DDS_GroupDataQosPolicy *right);

#ifndef RTI_CERT
/*ci
 * \brief Finalize the DDS_TopicDataQosPolicy
 *
 * \param[in]  policy DDS_TopicDataQosPolicy to be finalized
 *
 * \return RTI_TRUE if DDS_TopicDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_TopicDataQosPolicy_finalize(struct DDS_TopicDataQosPolicy *policy);
#endif /* !RTI_CERT */

/*ci
 * \brief Finalize the DDS_TopicDataQosPolicy without deallocating
 *
 * \param[in]  policy DDS_TopicDataQosPolicy to be finalized.
 *
 * \return RTI_TRUE if DDS_TopicDataQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDS_ReturnCode_t
DDS_TopicDataQosPolicy_finalize_no_dealloc(
        struct DDS_TopicDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type);

DDS_ReturnCode_t
DDS_TopicDataQosPolicy_initialize(struct DDS_TopicDataQosPolicy *policy);

/*ci
 * \brief Make a copy of the DDS_TopicDataQosPolicy instance.
 *
 * \param[in]   from_policy Pointer to DDS_TopicDataQosPolicy instance to be copied
 * \param[out]  to_policy Pointer to DDS_TopicDataQosPolicy which will be a copy
 *              of the from_policy.
 *
 * \return RTI_TRUE if the DDS_TopicDataQosPolicy instance is copied successfully,
 *         RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_TopicDataQosPolicy_copy(struct DDS_TopicDataQosPolicy *to_policy,
                            const struct DDS_TopicDataQosPolicy *from_policy);

/*ci
 * \brief Check if the DDS_TopicDataQosPolicy has been populated consistently.
 *
 * \brief A DDS_TopicDataQosPolicy is consistent if the length of its octet
 *        sequence is less than or equal to max_length
 *
 * \param[in]  policy DDS_TopicDataQosPolicy that has to be checked for consistency
 *
 * \return RTI_TRUE if the policy is consistent, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_TopicDataQosPolicy_is_consistent(const struct DDS_TopicDataQosPolicy *policy,
                                    DDS_Long max_length);

/*ci
 * \brief Check if two DDS_TopicDataQosPolicy instances are equal
 *
 * \param[in]  left  Left side of comparison
 * \param[in]  right Right side of comparison
 * \return RTI_TRUE if left = right, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN extern RTI_BOOL
DDS_TopicDataQosPolicy_is_equal(const struct DDS_TopicDataQosPolicy *left,
        const struct DDS_TopicDataQosPolicy *right);

#ifdef __cplusplus
}
#endif

#endif /* UserDataQosPolicy_h*/

/*ci @} */
