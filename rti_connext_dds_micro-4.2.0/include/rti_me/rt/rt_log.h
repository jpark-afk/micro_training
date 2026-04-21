/*
 * FILE: rt_log.h - RT Log definitions
 *
 * Copyright 2012-2015 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 14oct2014,tk MICRO-943/PR#11893 Use correct ID for RT_Log_registry_inconsistent_cid
 * 04sep2012,tk Written
 */
 /*e
 * \file
 * \defgroup RTLogCodesClass RT
 * \brief RT. ModuleID = 3
 * \ingroup LoggingModule
 */
#ifndef rt_log_h
#define rt_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to set registry property due to registry already being enabled
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_SET_IMMUTABLE_PROPERTY_EC                       (RT_LOG_BASE + 1)
#define RT_LOG_SET_IMMUTABLE_PROPERTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RT_LOG_SET_IMMUTABLE_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize registry due to failed table creation
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_INIT_FAILURE_EC                        (RT_LOG_BASE + 2)
#define RT_LOG_REGISTRY_INIT_FAILURE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RT_LOG_REGISTRY_INIT_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to finalize registry due to failed table deletion
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_FINALIZE_EC                            (RT_LOG_BASE + 3)
#define RT_LOG_REGISTRY_FINALIZE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RT_LOG_REGISTRY_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief An attempt was made to register a factory that already existed
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_EXISTS_EC                              (RT_LOG_BASE + 4)
#define RT_LOG_REGISTRY_EXISTS(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),RT_LOG_REGISTRY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_))

/*e
 * \brief Error registering a component factory.  May have exceeded 
 *        RT_RegistryProperty.max_factories
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_REGISTER_EC                            (RT_LOG_BASE + 5)
#define RT_LOG_REGISTRY_REGISTER(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RT_LOG_REGISTRY_REGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief A registered factory failed to initialize
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_INIT_FACTORY_EC                        (RT_LOG_BASE + 7)
#define RT_LOG_REGISTRY_INIT_FACTORY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RT_LOG_REGISTRY_INIT_FACTORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Factory name longer than maximum length of 8
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_NAME_TOO_LONG_EC                       (RT_LOG_BASE + 8)
#define RT_LOG_REGISTRY_NAME_TOO_LONG(level_,name_,len_) \
OSAPI_LOG_ENTRY_CREATE((level_),RT_LOG_REGISTRY_NAME_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("len",(len_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("max",RT_MAX_FACTORY_NAME,RTI_TRUE)

/*e
 * \brief Factory name exists, but the class ID is not of the requested type
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_INCONSISTENT_CID_EC                    (RT_LOG_BASE + 9)
#define RT_LOG_REGISTRY_INCONSISTENT_CID(level_,name_,req_cid_,real_cid_) \
OSAPI_LOG_ENTRY_CREATE((level_),RT_LOG_REGISTRY_INCONSISTENT_CID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("r_cid",(req_cid_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("a_cid",(real_cid_),RTI_TRUE)

/*e
 * \brief An attempt was made to operate on an uninitialized registry
 * \ingroup RTLogCodesClass
 */
#define RT_LOG_REGISTRY_NOT_INITIALIZED_EC                    (RT_LOG_BASE + 10)
#define RT_LOG_REGISTRY_NOT_INITIALIZED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RT_LOG_REGISTRY_NOT_INITIALIZED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
#endif
