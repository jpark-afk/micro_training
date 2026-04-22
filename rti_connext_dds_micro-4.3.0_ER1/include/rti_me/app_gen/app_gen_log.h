/*
 * FILE: appgen_log.h - App Generator error log messages
 *
 * (c) Copyright, Real-Time Innovations, 2017-2025.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */

/*e
 * \file
 * \defgroup APPGENLogCodesClass APPGEN 
 * \brief APPGEN. ModuleID = 13
 * \ingroup LoggingModule
 */
#ifndef appgen_log_h
#define appgen_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to allocate memory for an Application Generator interface
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_ALLOCATE_EC                    (APPGEN_LOG_BASE + 1)
#define APPGEN_LOG_ALLOCATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),APPGEN_LOG_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot copy a name into an entity name. Name too long.
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_COPY_ENTITY_NAME_EC            (APPGEN_LOG_BASE + 2)
#define APPGEN_LOG_COPY_ENTITY_NAME(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_COPY_ENTITY_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Cannot generate an entity name. Name too long.
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_CREATE_ENTITY_NAME_EC          (APPGEN_LOG_BASE + 3)
#define APPGEN_LOG_CREATE_ENTITY_NAME(level_,name_,multiplicity_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_CREATE_ENTITY_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("multiplicity",(multiplicity_),RTI_TRUE)

/*e
 * \brief Cannot create entity name because multiplicity is wrong (0)
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_MULTIPLICITY_EC                    (APPGEN_LOG_BASE + 4)
#define APPGEN_MULTIPLICITY(level_,multiplicity_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_MULTIPLICITY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("multiplicity",(multiplicity_),RTI_TRUE)

/*e
 * \brief Cannot register a factory
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_FACTORY_REGISTER_EC                (APPGEN_LOG_BASE + 5)
#define APPGEN_FACTORY_REGISTER(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_FACTORY_REGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Topic not found. Wrong topic name in writer configuration
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_FACTORY_TOPIC_NOT_FOUND_EC     (APPGEN_LOG_BASE + 6)
#define APPGEN_LOG_FACTORY_TOPIC_NOT_FOUND(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_FACTORY_TOPIC_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Failed to create/delete/lookup DDS entity
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_DDS_ENTITY_EC                  (APPGEN_LOG_BASE + 7)
#define APPGEN_LOG_DDS_ENTITY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),APPGEN_LOG_DDS_ENTITY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A call to a DDS api returned an error
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_API_ERROR_EC                    (APPGEN_LOG_BASE + 8)
#define APPGEN_LOG_API_ERROR(level_,error_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_API_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("error code",(error_),RTI_TRUE)

/*e
 * \brief Wrong application or participant name. Not a qualified name
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_APP_NAME_ERROR_EC               (APPGEN_LOG_BASE + 9)
#define APPGEN_LOG_APP_NAME_ERROR(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_APP_NAME_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Participant cannot be created because the library name is not
 * found in the configuration
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_LIB_NOT_FOUND_EC                (APPGEN_LOG_BASE + 10)
#define APPGEN_LOG_LIB_NOT_FOUND(level_,library_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_LIB_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("libname",(library_),RTI_TRUE)

/*e
 * \brief Participant cannot be created because the participant name is not
 * found in the configuration
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_APP_PARTICIPANT_NOT_FOUND_EC       (APPGEN_LOG_BASE + 11)
#define APPGEN_APP_PARTICIPANT_NOT_FOUND(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_APP_PARTICIPANT_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Application generation configuration is not correct or not consistent
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_APP_CONFIG_NOT_CORRECT_EC          (APPGEN_LOG_BASE + 12)
#define APPGEN_LOG_APP_CONFIG_NOT_CORRECT(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_APP_CONFIG_NOT_CORRECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

/*e
 * \brief Application generation configuration is not correct or not consistent
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT_EC  (APPGEN_LOG_BASE + 13)
#define APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(level_,name_,count_,pointer_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_LOG_APP_CONFIG_NOT_CORRECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("count",(count_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_POINTER("pointer",(pointer_),RTI_TRUE)

/*e
 * \brief Cannot unregister a factory
 * \ingroup APPGENLogCodesClass
 */
#define APPGEN_FACTORY_UNREGISTER_EC              (APPGEN_LOG_BASE + 14)
#define APPGEN_FACTORY_UNREGISTER(level_,name_) \
OSAPI_LOG_ENTRY_CREATE((level_),APPGEN_FACTORY_UNREGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_TRUE)

#endif /* appgen_log_h */

