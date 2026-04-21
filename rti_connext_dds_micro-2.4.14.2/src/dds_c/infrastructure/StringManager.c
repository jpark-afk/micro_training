
/*
 * FILE: StringManager.c - string Manager implementation
 *
 * (c) Copyright 2019 - 2019 Real-Time Innovations,
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
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * may022019,am created
 */
/*ce
 * \file
 * \brief String Manager implementation
 *
 * \details
 * This file implements string manager to manipulate strings
 */

#ifdef RTI_CERT
/*** SOURCE_BEGIN ***/
#endif

#ifndef RTI_CERT

#include "dds_c/dds_c_string_manager.h"
#include "StringManager.h"
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

/*** SOURCE_BEGIN ***/

RTI_PRIVATE const char *const DDS_MANAGEDSTRING_TABLE_NAME = "sm_table";

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

/*ci
 * \brief Create a new StringManager
 *
 * \param[out] string_manager On success, pointer to new string manager.
 * \param[in] property String mananger properties
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL 
DDS_StringManager_create(DDS_StringManager_T **string_manager, 
                         struct DDS_StringManagerProperty *property)
{
    DB_ReturnCode_T dbrc;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    DDS_StringManager_T *manager = NULL;
    
    OSAPI_PRECONDITION((string_manager == NULL || property == NULL),return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)
    
    OSAPI_Heap_allocate_struct(&manager, DDS_StringManager_T);
    tbl_prop.max_records = property->max_strings;
    dbrc = DB_Database_create_table(&manager->string_table,
                                    property->database,DDS_MANAGEDSTRING_TABLE_NAME,
                                    sizeof(struct DDS_ManagedString),
                                    DDS_ManagedString_compare,&tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_MANAGEDSTRING_TABLE_NAME,dbrc)
        OSAPI_Heap_free_struct(manager);
        return RTI_FALSE;
    }
    
    manager->database = property->database;
    *string_manager = manager;
    return RTI_TRUE;
    
}

/*ci
 * \brief Delete a StringManager
 *
 * \param[in] string_manager String manager to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL
DDS_StringManager_delete(DDS_StringManager_T *string_manager)

{
    DB_ReturnCode_T dbrc;
    if (string_manager->string_table != NULL)
    {
        dbrc = DB_Database_delete_table(string_manager->database,
                                            string_manager->string_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                  string_manager->string_table,dbrc)
                return RTI_FALSE;
        }
    }
    
    OSAPI_Heap_free_struct(string_manager);
    return RTI_TRUE;
    
}

/*ci
 * \brief Initialize a managed string
 *
 * \param[in] string_entry String entry to initialize
 * \param[in] value String value
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_ManagedString_initialize(DDS_ManagedString_T *string_entry,
                             const char* const value)
{
    OSAPI_PRECONDITION((string_entry == NULL || value == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("string_entry",string_entry,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)
    string_entry->value = REDA_String_dup(value);
    if (string_entry->value == NULL)
    {
        return RTI_FALSE;
    }
    string_entry->ref_count = 1;
    return RTI_TRUE;
}

/*ci
 * \brief Finalize a managed string
 *
 * \param[in] string_entry String entry to finalize
 *
 */
RTI_PRIVATE void
DDS_ManagedString_finalize(DDS_ManagedString_T *string_entry)
{
    OSAPI_PRECONDITION((string_entry == NULL ||
                    string_entry->value == NULL ||
                    string_entry->ref_count != 0),
                    return,
                    OSAPI_Log_entry_add_pointer("string_entry",string_entry,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("value", string_entry->value,RTI_FALSE);
                    OSAPI_Log_entry_add_uint("ref_count",string_entry->ref_count,RTI_TRUE);)
    
    REDA_String_free(string_entry->value);
    string_entry->value = NULL;
    string_entry->ref_count = 0;
}

/*ci
 * \brief Return pointer to existing string or add a new string to the string
 *        manager.
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value The string value to manager
 *
 * \return Pointer to string
 */
char* 
DDS_StringManager_assert_string(DDS_StringManager_T* string_manager, 
                                const char* const value)
{
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    char *retval = NULL;
    
    OSAPI_PRECONDITION(((value == NULL) || 
                    (string_manager == NULL) ||
                    (string_manager->string_table == NULL)),
                    return NULL,
                    OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("string_table",
                                                string_manager->string_table,RTI_TRUE);)

    dbrc = DB_Table_select_match(string_manager->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);
    
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /* we allocate memory for the managed string and the value inside here.
         * This should only happen when a local enitity is created
         * if we are avoiding a run time memory allocation
         */
        dbrc = DB_Table_create_record(string_manager->string_table,
                                      (DB_Record_T*)&string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            retval = NULL;
            goto done;
        }
        if (!DDS_ManagedString_initialize(string_entry,value))
        {
            (void)DB_Table_delete_record(string_manager->string_table,
                                                (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }
        dbrc = DB_Table_insert_record(string_manager->string_table,string_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            DDS_ManagedString_finalize(string_entry);
            (void)DB_Table_delete_record(string_manager->string_table,
                                        (DB_Record_T)string_entry);
            retval = NULL;
            goto done;
        }
        retval = string_entry->value;
        goto done;
        
    }
    else if (dbrc == DB_RETCODE_OK)
    {  
        string_entry->ref_count++;
        retval = string_entry->value;
        goto done;
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD)
        goto done;
    }
done:
    return retval;
        
}

/*ci
 * \brief Delete a string from the string manager
 *
 * \param[in] string_manager The string manager to use
 * \param[in] value The string value to delete
 *
 * \return TRUE on success, FALSE on failure.
 */
RTI_BOOL 
DDS_StringManager_delete_string(DDS_StringManager_T *string_manager,char *value)
{
    DB_ReturnCode_T dbrc;
    DDS_ManagedString_T *string_entry = NULL;
    RTI_BOOL retval = RTI_FALSE;
    
    OSAPI_PRECONDITION((value == NULL || 
                    string_manager == NULL ||
                    string_manager->string_table == NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("value",value,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("string_manager",
                                                string_manager,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("string_table",
                                                string_manager->string_table,
                                                RTI_TRUE);)
    
    dbrc = DB_Table_select_match(string_manager->string_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&string_entry,
                                 (DB_Key_T)value);
          
    if (dbrc == DB_RETCODE_OK)
    {
        string_entry->ref_count--;
    }
    else
    {
        /* string not found or another error */
        goto done;
          
    }

    if (string_entry->ref_count == 0)
    {
        string_entry = NULL;
        dbrc = DB_Table_remove_record(string_manager->string_table,            
                                      (DB_Record_T*)&string_entry,value);
        
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(string_manager->string_table, 
                                          string_entry);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_STRING_RECORD, dbrc)
                goto done;
            }

            DDS_ManagedString_finalize(string_entry);
        }
        else
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRING_RECORD,dbrc)
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    return retval;
}

#endif /* !RTI_CERT */
