/*
 * FILE: wh_sm_log.h - Writer Log definitions
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
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
 * 16aug2011,tk Created
 */

/*e
 * \file 
 * \brief WH module log codes 
 */
#ifndef wh_sm_log_h
#define wh_sm_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup WHLogCodesClass WH 
 * \brief Writer History. ModuleID = 9 
 * \ingroup LoggingModule
 */

/*e
 * \brief KEEP_ALL History kind is not supported  
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED_EC           (WHSM_LOG_BASE + 1)
#define WHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSM_LOG_KEEP_ALL_HISTORY_NOT_SUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unlimited length resource limits unsupported 
 * \details Ensure that max_samples, max_samples_per_instance, and max_instances 
 * resource limits are all finite values 
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED_EC          (WHSM_LOG_BASE + 2)
#define WHSM_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSM_LOG_UNLIMITED_HISTORY_NOT_SUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DataWriterQos.resource_limits.max_samples set too small
 *  
 * \details DataWriterQos.resource_limits.max_samples must be 
 *        no less than max_instances * max_samples_per_instance 
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_MAX_SAMPLES_TOO_SMALL_EC                    (WHSM_LOG_BASE + 3)
#define WHSM_LOG_MAX_SAMPLES_TOO_SMALL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSM_LOG_MAX_SAMPLES_TOO_SMALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A history object
 */
#define WHSM_LOG_HISTORY_OBJECT                                         1

/*e
 * \brief A key object
 */
#define WHSM_LOG_KEY_OBJECT                                             2

/*e
 * \brief A sample object
 */
#define WHSM_LOG_SAMPLE_OBJECT                                          2

/*e
 * \brief A key-pool object
 */
#define WHSM_LOG_KEYPOOL_OBJECT                                         3

/*e
 * \brief A sample-pool object
 */
#define WHSM_LOG_SAMPLEPOOL_OBJECT                                      4

/*e
 * \brief A key index pool
 */
#define WHSM_LOG_KEYINDEX_OBJECT                                        5

/*e
 * \brief A key index pool
 */
#define WHSM_LOG_HISTORYINDEX_OBJECT                                    6

/*e
 * \brief A key index pool
 */
#define WHSM_LOG_SAMPLEINDEX_OBJECT                                     5

/*e
 * \brief Failed to allocate object of the specified kind
 *
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_OBJECT_ALLOCATE_EC                          (WHSM_LOG_BASE + 4)
#define WHSM_LOG_OBJECT_ALLOCATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),WHSM_LOG_OBJECT_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to delete object of the specified kind
 *
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_OBJECT_DELETE_EC                            (WHSM_LOG_BASE + 5)
#define WHSM_LOG_OBJECT_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),WHSM_LOG_OBJECT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to index an object of the specified kind
 *
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_OBJECT_INDEX_EC                             (WHSM_LOG_BASE + 6)
#define WHSM_LOG_OBJECT_INDEX(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),WHSM_LOG_OBJECT_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief No property when creating a writer history instance
 *
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_NO_PROPERTY_EC                              (WHSM_LOG_BASE + 7)
#define WHSM_LOG_NO_PROPERTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSM_LOG_NO_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief The sample removed from an index did not match sample's key
 *
 * \ingroup WHLogCodesClass
 */
#define WHSM_LOG_INVALID_INDEX_OBJECT_EC                     (WHSM_LOG_BASE + 8)
#define WHSM_LOG_INVALID_INDEX_OBJECT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),WHSM_LOG_INVALID_INDEX_OBJECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif

