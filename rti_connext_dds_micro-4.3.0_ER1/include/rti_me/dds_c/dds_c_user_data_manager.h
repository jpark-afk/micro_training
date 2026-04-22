
/*
 * FILE: dds_c_user_data_manager.h - Blob manager definitions
 *
 * (c) Copyright, Real-Time Innovations, 2023-2023
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
 * \brief Blob manager definitions
 */

#ifndef dds_c_user_data_manager_h
#define dds_c_user_data_manager_h
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum DDS_UserDataType
{
    DDS_USER_DATA_PARTICIPANT_TYPE,
    DDS_USER_DATA_TOPIC_TYPE,
    DDS_USER_DATA_PUBLISHER_TYPE,
    DDS_USER_DATA_SUBSCRIBER_TYPE,
    DDS_USER_DATA_DATAWRITER_TYPE,
    DDS_USER_DATA_DATAREADER_TYPE,
    DDS_USER_DATA_UNKNOWN_TYPE
} DDS_UserDataType;

#define DDS_USER_DATA_TYPE_COUNT DDS_USER_DATA_UNKNOWN_TYPE

/* abstract type of user_data manager*/
struct DDS_UserDataManager;

typedef struct DDS_UserDataManager DDS_UserDataManager_T;

/*i
 * \brief Create a UserDataManager using the given property
 *
 * \param[out] user_data_manager The created UserDataManager
 * \param[in] property The property to use for the creation
 *
 * \return RTI_TRUE if the creation was successful, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_UserDataManager_create(DDS_UserDataManager_T **user_data_manager,
                           DB_Database_T database);

#ifndef RTI_CERT
/*i
 * \brief Delete a UserDataManager
 *
 * \details This function will free all the memory allocated by the
 *          UserDataManager. It will fail if there are still some
 *          types registered to the UserDataManager.
 *
 * \param[in] user_data_manager The UserDataManager to delete
 *
 * \return RTI_TRUE if the deletion was successful, RTI_FALSE otherwise
 */
DDSCDllExport RTI_BOOL
DDS_UserDataManager_delete(DDS_UserDataManager_T *user_data_manager);
#endif

/*i
 * \brief Register a type to the UserDataManager
 *
 * \param[in] user_data_manager The UserDataManager to use
 * \param[in] type_id The id of the type to register
 * \param[in] max_user_data_size The maximum size of user data of this type
 * \param[in] max_user_data_count The maximum number of unique data of this type
 *
 * \return RTI_TRUE if the registration was successful, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_UserDataManager_register_type(DDS_UserDataManager_T *user_data_manager,
                                  DDS_UserDataType type_id,
                                  RTI_INT32 max_user_data_size,
                                  RTI_INT32 max_user_data_count);

/*i
 * \brief Unregister a type from the UserDataManager
 *
 * \param[in] user_data_manager The UserDataManager to use
 * \param[in] type_id The id of the type to unregister
 *
 * \return RTI_TRUE if the unregistration was successful, RTI_FALSE otherwise
 */
DDSCDllExport RTI_BOOL
DDS_UserDataManager_unregister_type(DDS_UserDataManager_T *user_data_manager,
                                    DDS_UserDataType type_id);

/*i
 * \brief Assert that a user data is present in the UserDataManager and
 *        get a reference to the managed data.
 *
 * \param[in] user_data_manager The UserDataManager to use
 * \param[in] type_id The id of the type of the user data
 * \param[in] value The value of the user data
 * \param[out] user_data_out The managed user data
 *
 * \return RTI_TRUE if the assertion was successful, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_UserDataManager_assert_user_data(DDS_UserDataManager_T* user_data_manager,
                                     DDS_UserDataType type_id,
                                     const struct DDS_OctetSeq *value,
                                     struct DDS_OctetSeq *user_data_out);

/*i
 * \brief Delete a reference to a previously asserted user data.
 *
 * \details Decrement the reference count of a given user data. If the
 *          reference count reaches 0, the user data removed from the manager.
 *
 * \param[in] user_data_manager The UserDataManager to use
 * \param[in] type_id The id of the type of the user data
 * \param[in] user_data The user data to delete
 *
 * \return RTI_TRUE if the deletion was successful, RTI_FALSE otherwise
 */
DDSCDllExport RTI_BOOL
DDS_UserDataManager_delete_user_data(DDS_UserDataManager_T *user_data_manager,
                                     DDS_UserDataType type_id,
                                     const struct DDS_OctetSeq *user_data);

#ifdef __cplusplus
}
#endif

#endif
