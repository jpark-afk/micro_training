/*
 * FILE: UserDataManager.c - UserDataManager implementation
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
 * \brief UserDataManager manager implementation
 */
#include "UserDataManager.h"
#include "osapi/osapi_string.h"

#define USER_DATA_TABLE_NAME_MAX_SIZE 16

#define DDS_UserDataType_is_valid(type_id) \
    ((type_id) < DDS_USER_DATA_UNKNOWN_TYPE)

#define DDS_UserDataManager_type_exists(manager, type_id) \
    ((manager)->data_types[(type_id)] != NULL)

/*** SOURCE_BEGIN ***/

RTI_PRIVATE const char *DDS_USER_DATA_TABLE_NAMES[DDS_USER_DATA_TYPE_COUNT] =
{
    "ud_participant",
    "ud_topic",
    "ud_publisher",
    "ud_subscriber",
    "ud_datawriter",
    "ud_datareader"
};

/*ci
 * \brief Compare entries in the table of user_data entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A user_data already in the database
 * \param[in] op2   A to search for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_PRIVATE RTI_INT32
DDS_ManagedUserData_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_ManagedUserData *record_left = (struct DDS_ManagedUserData*)op1;
    struct DDS_ManagedUserData *key_right = (struct DDS_ManagedUserData*)op2;

    UNUSED_ARG(flags);

    OSAPI_PRECONDITION((record_left == NULL || key_right == NULL),
                        return 0,
                        OSAPI_Log_entry_add_pointer("record_left",record_left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("key_right",key_right,RTI_TRUE);)

    if (record_left->length > key_right->length)
    {
        return 1;
    }
    else if (record_left->length < key_right->length)
    {
        return -1;
    }
    else
    {
        return OSAPI_Memory_compare(record_left->value,key_right->value,record_left->length);
    }
}

RTI_BOOL
DDS_UserDataManager_create(DDS_UserDataManager_T **user_data_manager,
                           DB_Database_T database)
{
    DDS_UserDataManager_T *manager = NULL;
    RTI_SIZE_T i;

    OSAPI_PRECONDITION((user_data_manager == NULL || database == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("database",database,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&manager, DDS_UserDataManager_T);
    if (manager == NULL)
    {
        return RTI_FALSE;
    }

    manager->database = database;
    for (i = 0; i < DDS_USER_DATA_TYPE_COUNT; ++i)
    {
        manager->data_types[i] = NULL;
    }

    *user_data_manager = manager;
    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
DDS_UserDataManager_delete(DDS_UserDataManager_T *user_data_manager)
{
    RTI_SIZE_T i;

    OSAPI_PRECONDITION((user_data_manager == NULL),return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_TRUE);)

    for (i = 0; i < DDS_USER_DATA_TYPE_COUNT; ++i)
    {
        if (DDS_UserDataManager_type_exists(user_data_manager, (DDS_UserDataType)i))
        {
            return RTI_FALSE;
        }
    }

    user_data_manager->database = NULL;

    OSAPI_Heap_free_struct(user_data_manager);

    return RTI_TRUE;
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_UserDataManager_entry_initialize(void *initialize_param, void *buffer)
{
    struct DDS_ManagedUserData *user_data = (struct DDS_ManagedUserData*)buffer;
    struct DDS_ManagedUserDataType *type = (struct DDS_ManagedUserDataType *)initialize_param;

    OSAPI_PRECONDITION((user_data == NULL) || (type == NULL),return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("user_data",user_data,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("type",type,RTI_TRUE);)

    *user_data = (struct DDS_ManagedUserData)DDS_ManagedUserData_INITIALIZER;

    OSAPI_Heap_allocate_array(&user_data->value, type->max_user_data_size, DDS_Octet);
    if (user_data->value == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_UserDataManager_entry_finalize(void *finalize_param, void *buffer)
{
    struct DDS_ManagedUserData *user_data = (struct DDS_ManagedUserData*)buffer;

    UNUSED_ARG(finalize_param);

    OSAPI_PRECONDITION((user_data == NULL),return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("user_data",user_data,RTI_TRUE);)

    if (user_data->ref_count != 0)
    {
        /* Blob still in use */
        return RTI_FALSE;
    }

    OSAPI_Heap_free_array(user_data->value);

    *user_data = (struct DDS_ManagedUserData)DDS_ManagedUserData_INITIALIZER;

    return RTI_TRUE;
}
#endif

RTI_BOOL
DDS_UserDataManager_register_type(DDS_UserDataManager_T *user_data_manager,
                                  DDS_UserDataType type_id,
                                  RTI_INT32 max_user_data_size,
                                  RTI_INT32 max_user_data_count)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_ManagedUserDataType *type = NULL;
    DB_ReturnCode_T dbrc;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;

    OSAPI_PRECONDITION((user_data_manager == NULL) || (max_user_data_size <= 0)
                    || (max_user_data_count <= 0) || !DDS_UserDataType_is_valid(type_id),
                    goto done,
                    OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_int("max_user_data_size",max_user_data_size,RTI_FALSE);
                    OSAPI_Log_entry_add_int("max_user_data_count",max_user_data_count,RTI_FALSE);
                    OSAPI_Log_entry_add_uint("type_id",type_id,RTI_TRUE);)

    if ((max_user_data_size <= 0) || (max_user_data_count <= 0)
            || !DDS_UserDataType_is_valid(type_id)
            || DDS_UserDataManager_type_exists(user_data_manager, type_id))
    {
        goto done;
    }

    /* Type does not exist, create it */
    OSAPI_Heap_allocate_struct(&type, struct DDS_ManagedUserDataType);
    if (type == NULL)
    {
        goto done;
    }
    type->max_user_data_size = (RTI_SIZE_T)max_user_data_size;

    tbl_prop.max_records = (RTI_SIZE_T)max_user_data_count;

#ifdef RTI_CERT
#define  TABLE_FINALIZER NULL
#else
#define  TABLE_FINALIZER DDS_UserDataManager_entry_finalize
#endif

    dbrc = DB_Database_create_table_w_ctor_and_dtor(&type->user_data_table,
                                    user_data_manager->database,
                                    DDS_USER_DATA_TABLE_NAMES[type_id],
                                    sizeof(struct DDS_ManagedUserData),
                                    DDS_ManagedUserData_compare, &tbl_prop,
                                    DDS_UserDataManager_entry_initialize,
                                    (void *)type,
                                    TABLE_FINALIZER,
                                    NULL);
#undef TABLE_FINALIZER

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_USER_DATA_TABLE_NAMES[type_id],dbrc)
        goto done;
    }

    user_data_manager->data_types[type_id] = type;
    retval = RTI_TRUE;

done:

#ifndef RTI_CERT
    if (!retval)
    {
        if (type != NULL)
        {
            OSAPI_Heap_free_struct(type);
        }
    }
#endif

    return retval;
}

#ifndef RTI_CERT
RTI_BOOL
DDS_UserDataManager_unregister_type(DDS_UserDataManager_T *user_data_manager,
                                    DDS_UserDataType type_id)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_ManagedUserDataType *type = NULL;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((user_data_manager == NULL) || !DDS_UserDataType_is_valid(type_id),
                    goto done,
                    OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_uint("type_id",type_id,RTI_TRUE);)

    if (!DDS_UserDataType_is_valid(type_id)
            || !DDS_UserDataManager_type_exists(user_data_manager, type_id))
    {
        goto done;
    }
    type = user_data_manager->data_types[type_id];

    dbrc = DB_Database_delete_table(user_data_manager->database,
                                        type->user_data_table);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }
    type->user_data_table = NULL;
    type->max_user_data_size = 0;

    OSAPI_Heap_free_struct(type);
    user_data_manager->data_types[type_id] = NULL;

    retval = RTI_TRUE;
done:
    return retval;
}
#endif

RTI_BOOL
DDS_UserDataManager_assert_user_data(DDS_UserDataManager_T* user_data_manager,
                                     DDS_UserDataType type_id,
                                     const struct DDS_OctetSeq *value,
                                     struct DDS_OctetSeq *user_data_out)
{
    RTI_SIZE_T value_length;
    struct DDS_ManagedUserDataType *type = NULL;
    struct DDS_ManagedUserData user_data_key = DDS_ManagedUserData_INITIALIZER;
    struct DDS_ManagedUserData *user_data_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((user_data_manager == NULL)
                    || (value == NULL) || (user_data_out == NULL)
                    || (!REDA_Sequence_has_ownership((struct REDA_Sequence *)user_data_out)),
                    goto done,
                    OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("user_data_out",user_data_out,RTI_FALSE);
                    OSAPI_Log_entry_add_int("has_ownership",
                            (user_data_out != NULL)?REDA_Sequence_has_ownership(
                                    (struct REDA_Sequence *)user_data_out):0,RTI_TRUE);)

    if (!DDS_UserDataType_is_valid(type_id))
    {
        goto done;
    }

    value_length = (RTI_SIZE_T)DDS_OctetSeq_get_length(value);
    if (value_length == 0)
    {
        /* Check for empty data before looking up type because this function
         * will still be called with empty data when the type is not registered.
         */
        if (DDS_OctetSeq_set_length(user_data_out, 0))
        {
            retval = RTI_TRUE;
        }
        goto done;
    }

    if (!DDS_UserDataManager_type_exists(user_data_manager, type_id)
            || (value_length > user_data_manager->data_types[type_id]->max_user_data_size))
    {
        goto done;
    }
    type = user_data_manager->data_types[type_id];

    user_data_key.value = value->_contiguous_buffer;
    user_data_key.length = value_length;

    dbrc = DB_Table_select_match(type->user_data_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&user_data_entry,
                                 (DB_Key_T)&user_data_key);
    if (dbrc == DB_RETCODE_OK)
    {
        /* Blob already exists */
        if (!REDA_Sequence_loan_contiguous(
                (struct REDA_Sequence *)user_data_out,
                user_data_entry->value,
                (RTI_INT32)user_data_entry->length,
                (RTI_INT32)type->max_user_data_size))
        {
            /* Error loaning user_data */
            goto done;
        }
        user_data_entry->ref_count++;
        retval = RTI_TRUE;
        goto done;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        /* Error looking up user_data */
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }

    /* Blob does not exist, create it */
    dbrc = DB_Table_create_record(type->user_data_table,
                                  (DB_Record_T*)&user_data_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD,dbrc)
        goto done;
    }

    OSAPI_Memory_copy(user_data_entry->value, value->_contiguous_buffer, value_length);
    user_data_entry->length = value_length;
    user_data_entry->ref_count = 1;

    dbrc = DB_Table_insert_record(type->user_data_table,user_data_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD,dbrc)
        dbrc = DB_Table_delete_record(type->user_data_table,
                                     (DB_Record_T)user_data_entry);
        IGNORE_RETVAL(dbrc);
        goto done;
    }

    if (!REDA_Sequence_loan_contiguous(
            (struct REDA_Sequence *)user_data_out,
            user_data_entry->value,
            (RTI_INT32)user_data_entry->length,
            (RTI_INT32)type->max_user_data_size))
    {
        /* Error loaning user_data */
        dbrc = DB_Table_delete_record(type->user_data_table,
                                      (DB_Record_T)user_data_entry);
        IGNORE_RETVAL(dbrc);
        goto done;
    }

    retval = RTI_TRUE;
done:
    return retval;
}

RTI_BOOL
DDS_UserDataManager_delete_user_data(DDS_UserDataManager_T *user_data_manager,
                                     DDS_UserDataType type_id,
                                     const struct DDS_OctetSeq *user_data)
{
    struct DDS_ManagedUserDataType *type = NULL;
    struct DDS_ManagedUserData user_data_key = DDS_ManagedUserData_INITIALIZER;
    struct DDS_ManagedUserData *user_data_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((user_data_manager == NULL) || (user_data == NULL),goto done,
                    OSAPI_Log_entry_add_pointer("user_data_manager",user_data_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("user_data",user_data,RTI_TRUE);)

    if (!DDS_UserDataType_is_valid(type_id)
            || !DDS_UserDataManager_type_exists(user_data_manager, type_id)
            || DDS_OctetSeq_get_length(user_data) == 0)
    {
        goto done;
    }

    user_data_key.value = user_data->_contiguous_buffer;
    user_data_key.length = (RTI_SIZE_T)user_data->_length;

    type = user_data_manager->data_types[type_id];
    dbrc = DB_Table_select_match(type->user_data_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&user_data_entry,
                                 (DB_Key_T)&user_data_key);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /* Blob does not exist */
        DDSC_LOG_RECORD_NOT_EXISTS(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        /* Error looking up user_data */
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }

    --user_data_entry->ref_count;
    if (user_data_entry->ref_count == 0)
    {
        user_data_entry = NULL;
        dbrc = DB_Table_remove_record(type->user_data_table,
                                      (DB_Record_T*)&user_data_entry,
                                      (DB_Key_T)&user_data_key);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD,dbrc)
            goto done;
        }

        user_data_entry->length = 0;

        dbrc = DB_Table_delete_record(type->user_data_table,
                                      (DB_Record_T)user_data_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_STRING_RECORD, dbrc)
            goto done;
        }
    }

    retval = RTI_TRUE;
done:
    return retval;
}
