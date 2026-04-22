/*
 * FILE: netio_zcopy_whsq_log.h -Shared Queue Data History Log definitions
 *
 * (c) Copyright 2022-2025 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*e
 * \file 
 * \brief WHSQ module log codes 
 */
#ifndef netio_zcopy_whsq_log_h
#define netio_zcopy_whsq_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*e
 * \defgroup WHSQLogCodesClass WHSQ
 * \brief Writer History Shared Queue. ModuleID = 22 
 * \ingroup LoggingModule
 */

#define WHSQ_LOG_BASE                                        (22 << 16)

/*e
 * \brief KEEP_ALL History kind is not supported  
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED_EC           (WHSQ_LOG_BASE + 1)
#define WHSQ_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unlimited length resource limits unsupported 
 * \details Ensure that max_samples, max_samples_per_instance, and max_instances 
 * resource limits are all finite values 
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED_EC          (WHSQ_LOG_BASE + 2)
#define WHSQ_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DataWriterQos.resource_limits.max_samples set too small
 *  
 * \details DataWriterQos.resource_limits.max_samples must be 
 *        no less than max_instances * max_samples_per_instance 
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_MAX_SAMPLES_TOO_SMALL_EC                    (WHSQ_LOG_BASE + 3)
#define WHSQ_LOG_MAX_SAMPLES_TOO_SMALL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_MAX_SAMPLES_TOO_SMALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A history object
 */
#define WHSQ_LOG_HISTORY_OBJECT                                         1

/*e
 * \brief Failed to allocate object of the specified kind
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_OBJECT_ALLOCATE_EC                          (WHSQ_LOG_BASE + 4)
#define WHSQ_LOG_OBJECT_ALLOCATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),WHSQ_LOG_OBJECT_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))


/*e
 * \brief Failed to purge a sample from the shared queue
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_PURGE_FAILED_EC                          (WHSQ_LOG_BASE + 5)
#define WHSQ_LOG_PURGE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_PURGE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create the underlying writer history
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_WH_CREATE_FAILED_EC                          (WHSQ_LOG_BASE + 6)
#define WHSQ_LOG_WH_CREATE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_WH_CREATE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete object of the specified kind
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_OBJECT_DELETE_EC                            (WHSQ_LOG_BASE + 7)
#define WHSQ_LOG_OBJECT_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),WHSQ_LOG_OBJECT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))


/*e
 * \brief Listener or Property not configured when creating the component
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_CONFIGURATION_EC                              (WHSQ_LOG_BASE + 8)
#define WHSQ_LOG_CONFIGURATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_CONFIGURATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief No factory for forwarding when creating a writer history instance
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_NO_DDS_FACTORY_EC                           (WHSQ_LOG_BASE + 9)
#define WHSQ_LOG_NO_DDS_FACTORY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_NO_DDS_FACTORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Committing a Sample failed
 *
 * \ingroup WHSQLogCodesClass
 */
#define WHSQ_LOG_COMMIT_FAILED_EC                               (WHSQ_LOG_BASE + 10)
#define WHSQ_LOG_COMMIT_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSQ_LOG_COMMIT_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*netio_zcopy_whsq_log_h*/
