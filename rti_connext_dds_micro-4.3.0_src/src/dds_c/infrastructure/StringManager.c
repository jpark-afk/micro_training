/*
 * FILE: StringManager.c - string Manager implementation
 *
 * (c) Copyright 2019 - 2024 Real-Time Innovations,
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
 * 22feb2024 - ad Added 3 modes of operation
 * 02may2019 - am Created
 */
/*ce
 * \file
 * \brief String Manager implementation
 *
 * \details
 * This file implements string manager to manipulate strings
 */

#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#include "StringManager.h"

/*---------------------------------------------------------------*/
/* common */
/*---------------------------------------------------------------*/
/*ci
 * \brief Compare entries in the table of string entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A string already in the database
 * \param[in] op2   A string or string to search for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_PRIVATE RTI_INT32
DDS_ManagedString_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_ManagedString *record_left = (struct DDS_ManagedString*)op1;
    const char *name_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        name_right = (const char*)op2;
    }
    else
    {
        name_right = ((struct DDS_ManagedString*)op2)->value;
    }

    return REDA_String_compare(record_left->value,name_right);
}

/*---------------------------------------------------------------*/
/* Dynamic - dynamic allocations*/
/*---------------------------------------------------------------*/

/*ci
 * \brief searches the list for a value
 *
 * \param[in]list  The list to search
 * \param[in]value The string to search
 *
 * \return The found node on success, NULL on failure.
 */
RTI_PRIVATE struct DDS_ManagedStringNode*
DDS_ManagedStringList_select(REDA_CircularList_T* list, const char *value)
{
    struct REDA_CircularListNode* tmp;

    tmp = REDA_CircularList_get_first(list);
    while(!REDA_CircularList_node_at_head(list,tmp))
    {
        if (DDS_String_cmp(((struct DDS_ManagedStringNode*)tmp)->value,
                                                                    value) == 0)
        {
            return (struct DDS_ManagedStringNode*)tmp;
        }

        tmp = REDA_CircularListNode_get_next(tmp);
    }

    return NULL;
}

/*ci
 * \brief Initialize a managed string
 *
 * \param[inout] managed_node The node to create
 * \param[in]    value        String value
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_initialize_dynamic(struct DDS_ManagedStringNode** managed_node,
                                    const char* value)
{

    OSAPI_PRECONDITION(( managed_node == NULL || (*managed_node) != NULL
        || value == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("managed_node",managed_node,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("(*managed_node)",(*managed_node),RTI_FALSE);
        OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&(*managed_node),struct DDS_ManagedStringNode);
    if ((*managed_node) == NULL)
    {
        return RTI_FALSE;
    }

    REDA_CircularListNode_init(&(*managed_node)->_node);
    (*managed_node)->value = REDA_String_dup(value);
    if ((*managed_node)->value == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(*managed_node);
#endif
        *managed_node = NULL;
        return RTI_FALSE;
    }
    (*managed_node)->ref_count = 1;
    return RTI_TRUE;
}

/*ci
 * \brief Finalize a managed string node
 *
 * \param[in] node The node to finalize
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_finalize_dynamic(struct DDS_ManagedStringNode *node)
{
    OSAPI_PRECONDITION(node == NULL,
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("item",node,RTI_TRUE);)

#ifndef RTI_CERT
    REDA_String_free(node->value);
#endif

    node->value = NULL;
    node->ref_count = 0;

    REDA_CircularList_unlink_node(&node->_node);

#ifndef RTI_CERT
    OSAPI_Heap_free_struct(node);
#endif

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete the string manager
 *
 * \param[in] string_manager The manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_dynamic(DDS_StringManager_T *string_manager)
{
    struct DDS_StringManager_dynamic *self =
                            (struct DDS_StringManager_dynamic*)string_manager;
    struct DDS_ManagedStringNode *list_item;

    OSAPI_PRECONDITION((string_manager == NULL), return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    while(!REDA_CircularList_is_empty(&self->string_list))
    {
        list_item = DDS_ManagedStringList_get_first(&self->string_list);
        DDS_StringManager_finalize_dynamic(list_item);
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_struct(self);
#endif

    return RTI_TRUE;
}
#endif /* RTI_CERT */

/*ci
 * \brief Return pointer to existing string or add a new string to the string
 *        manager.
 *
 * \param[in]string_manager The string manager
 * \param[in]value          String value
 *
 * \return pointer to the string on success, NULL on failure.
 */
RTI_PRIVATE char*
DDS_StringManager_assert_string_dynamic(DDS_StringManager_T* string_manager,
                                        const char* value)
{
    char *retval = NULL;
    struct DDS_StringManager_dynamic *self =
                            (struct DDS_StringManager_dynamic*) string_manager;
    struct DDS_ManagedStringNode* list_item = NULL;

    OSAPI_PRECONDITION(((value == NULL) || (string_manager == NULL)),
        return NULL,
        OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    list_item = DDS_ManagedStringList_select(&self->string_list,value);
    if (list_item == NULL)
    {
        if (!DDS_StringManager_initialize_dynamic(&list_item,value))
        {
            retval = NULL;
        }
        else
        {
            REDA_CircularList_append(&self->string_list,&list_item->_node);
            retval = list_item->value;
        }
    }
    else
    {
        list_item->ref_count++;
        retval = list_item->value;
    }

    return retval;
}

/*ci
 * \brief Delete a string from the string manager
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value          The string value to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_string_dynamic(DDS_StringManager_T *string_manager,
                                        char *value)
{
    struct DDS_StringManager_dynamic *self =
                            (struct DDS_StringManager_dynamic*) string_manager;
    struct DDS_ManagedStringNode* list_item = NULL;
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((value == NULL || string_manager == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    list_item = DDS_ManagedStringList_select(&self->string_list,value);
    if (list_item == NULL)
    {
        /* string not found */
        goto done;
    }
    else if (list_item->ref_count > 0)
    {
        list_item->ref_count--;
    }

    if (list_item->ref_count == 0)
    {
        DDS_StringManager_finalize_dynamic(list_item);
    }

    retval = RTI_TRUE;

done:

    return retval;
}

/*ci
 * \brief Create a dynamic string manager
 *
 * \details
 * Dynamically allocate strings and free string. Supports any string size,
 * and any number of strings. The manager does allocate memory during runtime.
 * Strings are stored in a list, allowing the creation of any number of strings.
 * When a string is no longer referenced the memory is freed.
 *
 * \param[in]property UNUSED_ARG
 * \param[in]name     UNUSED_ARG
 *
 * \return pointer to the manager on success, NULL on failure.
 */
RTI_PRIVATE DDS_StringManager_T*
DDS_StringManager_create_dynamic_inf(struct DDS_StringManagerProperty *property,
                                    const char *name)
{
    struct DDS_StringManager_dynamic *manager = NULL;
    UNUSED_ARG(property);
    UNUSED_ARG(name);

    OSAPI_PRECONDITION((property == NULL),return NULL,
            OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&manager,struct DDS_StringManager_dynamic);
    if (manager == NULL)
    {
        return NULL;
    }

#ifndef RTI_CERT
    manager->_parent.intf.delete = DDS_StringManager_delete_dynamic;
#endif /* RTI_CERT */
    manager->_parent.intf.assert_string =
                                        DDS_StringManager_assert_string_dynamic;
    manager->_parent.intf.delete_string =
                                        DDS_StringManager_delete_string_dynamic;
    REDA_CircularList_init(&manager->string_list);

    return &manager->_parent;
}

/*---------------------------------------------------------------*/
/* Fixed - buffer pools*/
/*---------------------------------------------------------------*/

/*ci
 * \brief Initialize a managed string
 *
 * \param[in] string_entry   String entry to initialize
 * \param[in] value          String value
 * \param[in] string_manager String Manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_initialize_fixed(DDS_ManagedString_T *string_entry,
                             const char* value,
                             DDS_StringManager_T* string_manager)
{
    struct DDS_StringManager_fixed *self =
                            (struct DDS_StringManager_fixed *)string_manager;
    RTI_SIZE_T value_length = 0;

    OSAPI_PRECONDITION((string_entry == NULL || value == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("string_entry",string_entry,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    value_length = REDA_String_length(value);
    if (value_length == RTI_SIZE_MAX)
    {
        return RTI_FALSE;
    }
    value_length++;

    if (value_length <= (RTI_SIZE_T)self->max_string_size)
    {
        if (string_entry->value == NULL)
        {
            return RTI_FALSE;
        }

        OSAPI_Memory_copy(string_entry->value, value, value_length);
    }
    else
    {
        return RTI_FALSE;
    }

    string_entry->ref_count = 1;
    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete the string manager
 *
 * \param[in] string_manager The manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_fixed(DDS_StringManager_T *string_manager)
{
#ifndef RTI_CERT
    struct DDS_StringManager_fixed *self =
                            (struct DDS_StringManager_fixed *)string_manager;
    DB_ReturnCode_T dbrc;

    if (self->string_table != NULL)
    {
        dbrc = DB_Database_delete_table(self->database, self->string_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR, self->string_table,dbrc)
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free_struct(self);

    self = NULL;
#else
    UNUSED_ARG(string_manager);
#endif

    return RTI_TRUE;
}
#endif /* RTI_CERT */

/*ci
 * \brief Return pointer to existing string or add a new string to the string
 *        manager.
 *
 * \param[in]string_manager The string manager
 * \param[in]value          String value
 *
 * \return pointer to the string on success, NULL on failure.
 */
RTI_PRIVATE char*
DDS_StringManager_assert_string_fixed(DDS_StringManager_T* string_manager,
                                        const char* value)
{
    struct DDS_StringManager_fixed *self =
                            (struct DDS_StringManager_fixed *)string_manager;
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    char *retval = NULL;

    OSAPI_PRECONDITION(((value == NULL) || (self == NULL) ||
        (self->string_table == NULL)),
        return NULL,
        OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("table", self->string_table,RTI_TRUE);)

    dbrc = DB_Table_select_match(self->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);


    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(self->string_table,
                                      (DB_Record_T*)&string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_PUBLICATION_RECORD,dbrc)
            retval = NULL;
            goto done;
        }
        if (!DDS_StringManager_initialize_fixed(string_entry,value,
                                                string_manager))
        {
            (void)DB_Table_delete_record(self->string_table,
                                                (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }

        dbrc = DB_Table_insert_record(self->string_table,string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_PUBLICATION_RECORD,dbrc)
            (void)DB_Table_delete_record(self->string_table,
                                        (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }

        retval = string_entry->value;
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        string_entry->ref_count++;
        retval = string_entry->value;
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
    }

done:
    return retval;
}

/*ci
 * \brief Delete a string from the string manager
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value          The string value to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_string_fixed(DDS_StringManager_T *string_manager,
                                        char *value)
{
    struct DDS_StringManager_fixed *self =
                            (struct DDS_StringManager_fixed *)string_manager;
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((value == NULL || string_manager == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager", string_manager,RTI_TRUE);)

    dbrc = DB_Table_select_match(self->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /*string not found*/
        goto done;
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        string_entry->ref_count--;
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }

    if (string_entry->ref_count == 0)
    {
        string_entry = NULL;
        dbrc = DB_Table_remove_record(self->string_table,
                                    (DB_Record_T*)&string_entry,value);

        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(self->string_table, string_entry);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_STRING_RECORD, dbrc)
                goto done;
            }
        }
        else
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_STRING_RECORD,dbrc)
            goto done;
        }
    }

    retval = RTI_TRUE;
done:

    return retval;
}

RTI_PRIVATE RTI_BOOL
DDS_StringManager_fixed_ctor(void *initialize_param, void *buffer)
{
    struct DDS_ManagedString *data = (struct DDS_ManagedString*)buffer;
    RTI_SIZE_T *size = (RTI_SIZE_T *)initialize_param;

    if (buffer == NULL)
    {
        return RTI_FALSE;
    }

    /* We are given the total string size including the nul.
     * REDA_String_alloc adds 1 for the nul so we need to pass in 1 less.
     */
    if (*size > 0)
    {
        data->value = REDA_String_alloc(*size - 1);
    }
    else
    {
        data->value = NULL;
    }

    data->ref_count = 0;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_StringManager_fixed_dtor(void *initialize_param, void *buffer)
{
    struct DDS_ManagedString *data = (struct DDS_ManagedString*)buffer;
    UNUSED_ARG(initialize_param);

#ifndef RTI_CERT
    REDA_String_free(data->value);
#endif

    data->value = NULL;
    data->ref_count = 0;

    return RTI_TRUE;
}
#endif /* RTI_CERT */

/*ci
 * \brief Create a dynamic string manager
 *
 * \details
 * Preallocate buffer with string reuse, fixed string size, fixed buffer size,
 * number of strings is (string size/allocation).
 *
 * During initialize, the system preallocates memory and frees it during
 * finalize. This approach supports a fixed string size and does not allocate
 * memory during runtime. When a string is no longer referenced the memory is
 * reused for future strings.
 *
 * \param[in]property Properties
 * \param[in]name     table name
 *
 * \return pointer to the manager on success, NULL on failure.
 */
RTI_PRIVATE DDS_StringManager_T*
DDS_StringManager_create_fixed_inf(struct DDS_StringManagerProperty *property,
                                    const char *name)
{
    DB_ReturnCode_T dbrc;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    struct DDS_StringManager_fixed *manager = NULL;
    RTI_SIZE_T length = 0;

    OSAPI_PRECONDITION((property == NULL),return NULL,
                    OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&manager,struct DDS_StringManager_fixed);
    if (manager == NULL)
    {
        return NULL;
    }

#ifdef RTI_CERT
#define  TABLE_FINALIZER NULL
#else
#define  TABLE_FINALIZER DDS_StringManager_fixed_dtor
#endif
    tbl_prop.max_records =
            (RTI_SIZE_T)(property->memory_allocation/property->max_string_size);
    length = (RTI_SIZE_T)(property->max_string_size);
    dbrc = DB_Database_create_table_w_ctor_and_dtor(&manager->string_table,
                                    property->database,
                                    name,
                                    sizeof(struct DDS_ManagedString),
                                    DDS_ManagedString_compare, &tbl_prop,
                                    DDS_StringManager_fixed_ctor,
                                    (void *)&length,
                                    TABLE_FINALIZER,
                                    NULL);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,name,dbrc)
        OSAPI_Heap_free_struct(manager);
        manager = NULL;
        return NULL;
    }
#undef TABLE_FINALIZER
    manager->database = property->database;
    manager->max_string_size = property->max_string_size;
    manager->memory_allocation = (RTI_SIZE_T)property->memory_allocation;
#ifndef RTI_CERT
    manager->_parent.intf.delete = DDS_StringManager_delete_fixed;
#endif /* RTI_CERT */
    manager->_parent.intf.assert_string = DDS_StringManager_assert_string_fixed;
    manager->_parent.intf.delete_string = DDS_StringManager_delete_string_fixed;
    return &manager->_parent;
}

/*---------------------------------------------------------------*/
/* Flexible - char buffer*/
/*---------------------------------------------------------------*/

/*ci
 * \brief Initialize a managed string
 *
 * \param[in] string_entry   String entry to initialize
 * \param[in] value          String value
 * \param[in] string_manager String manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_initialize_flexible(DDS_ManagedString_T *string_entry,
                                        const char* value,
                                        DDS_StringManager_T* string_manager)
{
    struct DDS_StringManager_flexible *self =
                        (struct DDS_StringManager_flexible *)string_manager;
    RTI_SIZE_T value_length = 0;
    RTI_SIZE_T remaining_space = 0;

    OSAPI_PRECONDITION((string_entry == NULL || value == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("string_entry",string_entry,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    value_length = REDA_String_length(value);
    if (value_length == RTI_SIZE_MAX)
    {
        return RTI_FALSE;
    }
    value_length++;

    remaining_space = (RTI_SIZE_T)self->memory_allocation - self->buffer_length;

    if (remaining_space >= value_length)
    {
        OSAPI_Memory_copy(self->buffer + self->buffer_length,
                            value,value_length);
        string_entry->value = self->buffer + self->buffer_length;
        self->buffer_length += value_length;
    }

    if (string_entry->value == NULL)
    {
        return RTI_FALSE;
    }

    string_entry->ref_count = 1;
    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete the string manager
 *
 * \param[in] string_manager The manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_flexible(DDS_StringManager_T *string_manager)
{
    struct DDS_StringManager_flexible *self =
                        (struct DDS_StringManager_flexible *)string_manager;
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    RTI_SIZE_T length = 0;
    char *currentString = self->buffer;

    while (self->buffer_length != 0)
    {
        string_entry = NULL;
        dbrc = DB_Table_select_match(self->string_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&string_entry,
                                (DB_Key_T)currentString);

        if (dbrc == DB_RETCODE_NO_DATA)
        {
            /*string not found*/
            return RTI_FALSE;
        }

        if (string_entry != NULL)
        {
            string_entry = NULL;
            dbrc = DB_Table_remove_record(self->string_table,
                                        (DB_Record_T*)&string_entry,
                                        currentString);

            if (dbrc == DB_RETCODE_OK)
            {
                dbrc = DB_Table_delete_record(self->string_table,
                                            string_entry);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                        DDSC_LOG_STRING_RECORD, dbrc)
                    return RTI_FALSE;
                }
            }
            else
            {
                DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                        DDSC_LOG_STRING_RECORD,dbrc)
                return RTI_FALSE;
            }
        }
        
        length = REDA_String_length(currentString);
        if (length == RTI_SIZE_MAX)
        {
            return RTI_FALSE;
        }
        length++;

        currentString += length;
        self->buffer_length -= length;
    }

#ifndef RTI_CERT
    if (self->string_table != NULL)
    {
        dbrc = DB_Database_delete_table(self->database, self->string_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR, self->string_table,dbrc)
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free_buffer(self->buffer);
    OSAPI_Heap_free_struct(self);

#endif

    self = NULL;

    return RTI_TRUE;
}
#endif /* RTI_CERT */

/*ci
 * \brief Return pointer to existing string or add a new string to the string
 *        manager.
 *
 * \param[in]string_manager The string manager
 * \param[in]value          String value
 *
 * \return pointer to the string on success, NULL on failure.
 */
RTI_PRIVATE char*
DDS_StringManager_assert_string_flexible(DDS_StringManager_T* string_manager,
                                        const char* value)
{
    struct DDS_StringManager_flexible *self =
                            (struct DDS_StringManager_flexible *)string_manager;
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    char *retval = NULL;

    OSAPI_PRECONDITION(((value == NULL) || (string_manager == NULL) ||
        (self->string_table == NULL)),
        return NULL,
        OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("table",self->string_table,RTI_TRUE);)

    dbrc = DB_Table_select_match(self->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(self->string_table,
                                      (DB_Record_T*)&string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_PUBLICATION_RECORD,dbrc)
            retval = NULL;
            goto done;
        }
        if (!DDS_StringManager_initialize_flexible(string_entry,value,
                                                    string_manager))
        {
            (void)DB_Table_delete_record(self->string_table,
                                                (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }

        dbrc = DB_Table_insert_record(self->string_table,string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_PUBLICATION_RECORD,dbrc)

            string_entry->ref_count = 0;
            (void)DB_Table_delete_record(self->string_table,
                                        (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }

        retval = string_entry->value;
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        string_entry->ref_count++;
        retval = string_entry->value;
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
    }

done:
    return retval;
}

/*ci
 * \brief Delete a string from the string manager
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value          The string value to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_StringManager_delete_string_flexible(DDS_StringManager_T *string_manager,
                                        char *value)
{
    struct DDS_StringManager_flexible *self =
                            (struct DDS_StringManager_flexible *)string_manager;
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((value == NULL || string_manager == NULL ||
                self->buffer == NULL),
                return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("string_manager",
                                            string_manager,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("buffer", self->buffer, RTI_TRUE);)

    dbrc = DB_Table_select_match(self->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);


    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /*string not found*/
        goto done;
    }
    else if ((dbrc == DB_RETCODE_OK) && (string_entry->ref_count > 0))
    {
        string_entry->ref_count--;
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }

    retval = RTI_TRUE;
done:

    return retval;
}

/*ci
 * \brief Create a dynamic string manager
 *
 * \details
 * Preallocate buffer and with no string reuse. Any string size, any buffer
 * size and any number of strings.
 *
 * During initialize, the system preallocates memory and frees it during
 * finalize. This approach supports any string size and does not allocate
 * memory during runtime. It stores strings in a list, allowing the creation
 * of any number of strings that fit within the preallocated memory.
 * When a string is no longer referenced the memory is not reused and the
 * string stays in memory and may be referenced again.
 *
 * \param[in]property Properties
 * \param[in]name     Table name
 *
 * \return pointer to the manager on success, NULL on failure.
 */
RTI_PRIVATE DDS_StringManager_T*
DDS_StringManager_create_flexible_inf(struct DDS_StringManagerProperty *property,
                                const char *name)
{
    DB_ReturnCode_T dbrc;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    struct DDS_StringManager_flexible *manager = NULL;

    OSAPI_PRECONDITION((property == NULL),return NULL,
                    OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&manager,struct DDS_StringManager_flexible);
    if (manager == NULL)
    {
        return NULL;
    }

    tbl_prop.max_records = (RTI_SIZE_T)((property->memory_allocation + 1) / 2);
    dbrc = DB_Database_create_table(&manager->string_table,
                                    property->database,
                                    name,
                                    sizeof(struct DDS_ManagedString),
                                    DDS_ManagedString_compare,&tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,name,dbrc)
        OSAPI_Heap_free_struct(manager);
        return NULL;
    }

    OSAPI_Heap_allocate_buffer(&manager->buffer,
                                (RTI_SIZE_T)property->memory_allocation,
                                OSAPI_ALIGNMENT_DEFAULT);
    if (manager->buffer == NULL)
    {
        return NULL;
    }

    manager->database = property->database;
    manager->max_string_size = (RTI_SIZE_T)property->max_string_size;
    manager->memory_allocation = (RTI_SIZE_T)property->memory_allocation;
    manager->buffer_length = 0;
#ifndef RTI_CERT
    manager->_parent.intf.delete = DDS_StringManager_delete_flexible;
#endif /* RTI_CERT */
    manager->_parent.intf.assert_string =
                                    DDS_StringManager_assert_string_flexible;
    manager->_parent.intf.delete_string =
                                    DDS_StringManager_delete_string_flexible;
    return &manager->_parent;
}

/*---------------------------------------------------------------*/
/* API */
/*---------------------------------------------------------------*/

/*ci
 * \brief Create a string manager
 *
 * \details
 * The string manager mode of operation is determined by property
 *
 * \param[inout] string_manager String manager
 * \param[in]    property       String manager properties
 * \param[in]    name           Name of the database
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL
DDS_StringManager_create(DDS_StringManager_T **string_manager,
                            struct DDS_StringManagerProperty *property,
                            const char *name)
{
    DDS_StringManager_T *manager = NULL;
    OSAPI_PRECONDITION_ALWAYS( string_manager == NULL || property == NULL ||
        name == NULL ,return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    if (property->memory_allocation == DDS_LENGTH_UNLIMITED)
    {
        manager = DDS_StringManager_create_dynamic_inf(property,name);
    }
    else if ((property->memory_allocation > 0) &&
            (property->max_string_size > 0))
    {
        manager = DDS_StringManager_create_fixed_inf(property,name);
    }
    else if ((property->memory_allocation > 0) &&
            (property->max_string_size == DDS_LENGTH_UNLIMITED))
    {
        manager = DDS_StringManager_create_flexible_inf(property,name);
    }

    if (manager == NULL)
    {
        return RTI_FALSE;
    }

    *string_manager = manager;
    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a string manager
 *
 * \details
 * The string manager mode of operation is determined by property
 *
 * \param[inout] string_manager String manager
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL
DDS_StringManager_delete(DDS_StringManager_T **string_manager)
{
    RTI_BOOL rtn = RTI_FALSE;
    OSAPI_PRECONDITION_ALWAYS( (string_manager == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    rtn = (*string_manager)->intf.delete(*string_manager);
    (*string_manager) = NULL;

    return rtn;
}
#endif /* RTI_CERT */

/*ci
 * \brief Return pointer to existing string or add a new string to the string
 *        manager.
 *
 * \param[in]string_manager The string manager
 * \param[in]value          String value
 *
 * \return pointer to the string on success, NULL on failure.
 */
char*
DDS_StringManager_assert_string(DDS_StringManager_T *string_manager,
                                const char* value)
{
    OSAPI_PRECONDITION_ALWAYS( (string_manager == NULL) || (value == NULL),
        return NULL,
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    return string_manager->intf.assert_string(string_manager,value);
}

/*ci
 * \brief Delete a string from the string manager
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value          The string value to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL
DDS_StringManager_delete_string(DDS_StringManager_T *string_manager,char *value)
{
    OSAPI_PRECONDITION_ALWAYS((string_manager == NULL) || (value == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    return string_manager->intf.delete_string(string_manager,value);
}
