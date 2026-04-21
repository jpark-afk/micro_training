/*
 * FILE: db_api.h - DB API definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 06may2014,tk MICRO-227 (Verocel PR#1406): Refactored get_count() to better
 *              handle error conditions
 * 28may2012,tk Written
 */
/*ci
 * \file
 * \defgroup DBModule DB
 *
 * \brief Public DB API
 *
 * \details
 * This file defines the public API for the database. It contains functions
 * to create/delete databases, create/delete tables, create/delete records
 * in tables and functions to traverse and index database tables based on keys.
 * The database API is not a general purpose API, it is only designed to support
 * features in Connext Micro.
 */

/*ci \addtogroup DBModule
 *  @{
 */
#ifndef db_api_h
#define db_api_h

#ifndef db_dll_h
#include "db/db_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \ingroup DBModule
 */

/*ci
 * \brief Abstract database declaration
 * \ingroup DBModule
 */
struct DB_Database;

/*ci
 * \brief Abstract database type
 */
typedef struct DB_Database* DB_Database_T;

/*ci
 * \brief Abstract database table
 */
struct DB_Table;

/*ci
 * \brief Abstract database table type
 */
typedef struct DB_Table *DB_Table_T;

/*ci
 * \brief Abstract database table record
 */
struct DB_Record;

/*ci
 * \brief Abstract database table record type
 */
typedef void *DB_Record_T;

/*ci
 * \brief Abstract database key
 */
typedef void *DB_Key_T;

/*ci
 * \brief Abstract database cursor
 */
struct DB_Cursor;

/*ci
 * \brief Abstract database cursor type
 */
typedef struct DB_Cursor *DB_Cursor_T;

/*ci
 * \brief Abstract database index
 */
struct DB_Index;

/*ci
 * \brief Abstract database index type
 */
typedef struct DB_Index *DB_Index_T;

/****************/

/*ci
 * \brief Maximum length of a database name in ASCIIZ format, NULL included
 */
#define URTDB_DATABASE_NAME_MAX_LENGTH 16

/*ci
 * \brief Maximum length of a table name in ASCIIZ format, NULL included
 */
#define URTDB_TABLE_NAME_MAX_LENGTH    16

/*ci
 * \brief The value for the default index. The default index is created
 * based on the compare function passed to the \ref DB_Database_create_table
 * function.
 */
#define DB_TABLE_DEFAULT_INDEX 0

/*ci
 * \brief Return code for database function
 */
typedef enum
{
    /*ci
     * \brief Function call was successful
     */
    DB_RETCODE_OK = 0,

    /*ci
     * \brief Invalid arguments to function
     */
    DB_RETCODE_BAD_PARAMETER,

    /*ci
     * \brief One or more records still exist in the database
     */
    DB_RETCODE_EXISTS,

    /*ci
     * \brief An cursor contains no more data, or a search did not yield any
     * results
     */
    DB_RETCODE_NO_DATA,

    /*ci
     * \brief The database is out of one or more pre-allocated resources
     */
    DB_RETCODE_OUT_OF_RESOURCES,

    /*ci
     * \brief A cursor is no longer valid due to the database being updated
     * while the cursor was in use
     */
    DB_RETCODE_INVALIDATED_CURSOR,

    /*ci
     * \brief Generic catch all return code in case of failure
     */
    DB_RETCODE_ERROR
} DB_ReturnCode_T;

/*ci
 * \brief Database locking granularity
 */
typedef enum
{
    /*ci
     * \brief The database is never locked
     */
    DB_LOCK_LEVEL_NONE   = 0,

    /*ci
     * \brief The database is using a shared lock (outside of the database)
     */
    DB_LOCK_LEVEL_SHARED = 1,

    /*ci
     * \brief The database is using a global lock created internally
     */
    DB_LOCK_LEVEL_GLOBAL = 2
} DB_LockLevel_T;

/*ci
 * \brief Record selection criteria
 */
typedef enum
{
    /*ci
     * \brief Select record from a table based on exact match
     */
    DB_SELECTOPCODE_EQUAL   = 1,

    /*ci
     * \brief Select records from a table that has a key within a range
     */
    DB_SELECTOPCODE_BETWEEN = 2,

    /*ci
     * \brief Select all records from a table
     */
    DB_SELECTOPCODE_ALL     = 3
} DB_Select_T;

/*ci
 * \brief Database properties
 *
 * \details
 *
 * A new database can be created with the following properties. Note that
 * the properties are immutable.
 */
struct DB_DatabaseProperty
{
    /*ci
     * \brief The maximum number of tables the database can contain
     */
    RTI_SIZE_T max_tables;

    /*ci
     * \brief The locking mode used for the database
     */
    DB_LockLevel_T lock_mode;
};

/*ci
 * \brief Initialize a \ref DB_DatabaseProperty to a default value
 */
#define DB_DatabaseProperty_INITIALIZER \
{\
    1,\
    DB_LOCK_LEVEL_NONE\
}

/*ci
 * \brief Operand 2 in the compare function is a key
 */
#define DB_SELECT_OP2_AS_KEY (0x1)

/*ci
 * \brief Test if operand 2 in the compare function is a key
 */
#define DB_SELECT_OP2_IS_KEY(flags) (flags & DB_SELECT_OP2_AS_KEY)

/*ci
 * \brief A generic record compare function
 *
 * \details
 *
 * A table contains records sorted by a key. It is not possible to
 * create a table with no compare function. The compare function must
 * maintain an ordered relationship.
 *
 * \param[in] flags Information about the operands
 * \param[in] op1   Always DB_Record_T
 * \param[in] key   DB_Record_T or DB_Key_T (passed transparently from the user)
 *
 *  \return positive integer if op1 is greater than key,
 *          negative integer if op1 is less than key,
 *          zero if left is equal to key
 *
 * \sa \ref DB_SELECT_OP2_IS_KEY
 */
FUNCTION_MUST_TYPEDEF(
RTI_INT32
(*DB_IndexCompare_T)(RTI_INT32 flags,const DB_Record_T op1, void *key)
)

/*ci
 * \brief A record destructor function type
 *
 * \details
 *
 * When a record is deleted from a table the database still needs the primary
 * key of the table. However, if the key is dynamically allocated it cannot
 * be free'd before \ref DB_Table_delete_record is called. To solve this a
 * record destructor can be passed to DB_Table_delete_record_w_dtor. The
 * destructor is called when the DB is done with the record and can be used
 * to clean up any resources.
 *
 * \param[in] record Record to destruct/finalize
 *
 * \return A record destructor function must return RTI_TRUE on success and
 *         RTI_FALSE on failure. NOTE: Even if the dtor returns failure the
 *         record is considered deleted.
 *
 * \sa DB_Table_delete_record_w_dtor
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*DB_Table_RecordDtor_T)(DB_Record_T record)
)

/*ci
 * \brief Table properties
 *
 * \details
 *
 * A table can be created with the following properties. Note that
 * the properties are immutable.
 */
struct DB_TableProperty
{
    /*ci
     * \brief The maximum number of records the table can hold
     */
    RTI_SIZE_T max_records;

    /*ci
     * \brief The maximum number of indices which can be created in the table
     */
    RTI_SIZE_T max_indices;

    /*ci
     * \brief The maximum number of cursors which can be open on the table
     *        at any given time
     */
    RTI_SIZE_T max_cursors;
};

/*ci
 * \brief Initialize a \ref DB_TableProperty to a default value
 */
#define DB_TableProperty_INITIALIZER \
{\
    0,\
    0,\
    1\
}

/*ci
 * \brief Index properties
 *
 * \details
 *
 * A index can be created with the following properties. Note that
 * the properties are immutable.
 */
struct DB_IndexProperty
{
    /*ci
     * \brief The maximum number of entries the index can hold
     */
    RTI_SIZE_T max_entries;
};

/*ci
 * \brief Initialize a \ref DB_IndexProperty to a default value
 */
#define DB_IndexProperty_INITIALIZER \
{\
    0 \
}

#ifndef RTI_CERT
/*ci
 *
 * \brief DB_Table  initialization function type.
 *
 * \details
 *
 * For each record is allocated from system memory
 * DB_Table buffer calls this function exactly once.
 *
 * \param[in] initialize_param Parameter passed to \ref REDA_BufferPool_new
 * \param[in] buffer           Buffer to initialize
 *
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DB_Table_initializeFunc_T)(void *initialize_param, void *buffer)
)

/*ci
 *
 * \brief DB_Table finalization type
 *
 * \details
 * For each record that is returned to system memory DB_Table buffer calls
 * this function exactly once
 *
 * \param[in] finalize_param Parameter passed to \ref REDA_BufferPool_new
 * \param[in] buffer         Buffer to finalize
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*DB_Table_finalizeFunc_T)(void *finalize_param, void *buffer)
)
#endif /* !RTI_CERT */

/**************************** Database Functions ******************************/

/*ci
 * \brief Create an empty database
 *
 * \param[inout] db          Handle to the newly created database. *db must be NULL.
 * \param[in]    name        The name of the database, cannot be NULL. It is OK
 *                           to create multiple databases with the same name
 * \param[in]    property    The database properties. Cannot be NULL.
 * \param[in]    shared_lock If the lock-level is \ref DB_LOCK_LEVEL_SHARED this
 *                           argument must point to a valid mutex.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         db holds a reference to the new database.
 *
 * \sa \ref DB_Database_delete
 *
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_create(DB_Database_T *db,
                   const char *name,
                   struct DB_DatabaseProperty *property,
                   OSAPI_Mutex_T *const shared_lock);

#ifndef RTI_CERT
/*ci
 * \brief Delete a database
 *
 * \details
 *
 * This function deletes a database and returns any used memory to the system.
 * It is illegal to delete a database that has resources in use. For example,
 * it is illegal to delete a database where a cursor is open or a table contains
 * records. This restriction exists to prevent an application from deleting
 * resources that may be in use and thus could cause a memory access violation.
 *
 * \param[in] db Handle to db being deleted
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the database was deleted and db is no longer a valid handle. Note
 *         the handle is not cleared.
 *
 * \sa \ref DB_Database_create
 *
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_delete(DB_Database_T db);
#endif /* !RTI_CERT */

/*ci
 * \brief Lock database to prevent modifications
 *
 * \details
 *
 * This function locks a database, preventing any modification to it.
 * If the database was created with a lock-level of \ref DB_LOCK_LEVEL_SHARED
 * or \ref DB_LOCK_LEVEL_GLOBAL the mutex is taken before returning. Otherwise
 * this call always succeeds. Note that there is no timeout on the mutex.
 * A locked database must be unlocked with \ref DB_Database_unlock. It is
 * legal to lock the database multiple times from the same thread, and a
 * database must be unlocked as many times as it was locked.
 *
 * \param[in] db Handle to db to lock
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the database was successfully locked.
 *
 * \sa \ref DB_Database_unlock, \ref DB_LockLevel_T
 *
 */
#if OSAPI_MUTEX_TRACE_ENABLED
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_lock_(DB_Database_T db,char *file,RTI_INT32 lineno);

#define DB_Database_lock(db_) DB_Database_lock_(db_,__FILE__,__LINE__)
#else
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_lock(DB_Database_T db);
#endif
/*ci
 * \brief Unlock a database previously locked with \ref DB_Database_lock
 *
 * \details
 *
 * This function unlocks a database, allowing modifications to it. Note that if
 * a database was locked multiple time, an equal number of unlocks is required.
 *
 * \param[in] db Handle to db to unlock
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the database was successfully unlocked.
 *
 * \sa \ref DB_Database_lock, \ref DB_LockLevel_T
 *
 */
#if OSAPI_MUTEX_TRACE_ENABLED
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_unlock_(DB_Database_T db,char *file,RTI_INT32 lineno);

#define DB_Database_unlock(db_) DB_Database_unlock_(db_,__FILE__,__LINE__)
#else
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_unlock(DB_Database_T db);
#endif

/*ci
 * \brief Create a database table
 *
 * \details
 *
 * This function creates an empty database table. Every table in the same
 * database must have a unique name. A table contains fixed size records.
 * Any memory allocation required by a record must be done outside of the
 * database. For example, any record that contains pointers or sequences
 * must be initialized outside of the table itself.
 *
 * \param[inout] table        Handle to new table. *table must be NULL on input.
 * \param[in]    db           Database where the table shall be created.
 *                            Cannot be NULL.
 * \param[in]    name         Name of the table. Must be unique in the database.
 *                            Cannot be NULL.
 * \param[in]    record_size  Size in bytes of a record. Must be > 0.
 * \param[in]    compare_func The compare function. Cannot be NULL.
 * \param[in]    property     Table properties. Cannot be NULL.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the table was successfully created and table contains a handle to
 *         the newly created table.
 *
 * \sa \ref DB_Database_delete_table
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_create_table(DB_Table_T *table,
                         DB_Database_T db,
                         const char *name,
                         RTI_SIZE_T record_size,
                         DB_IndexCompare_T compare_func,
                         struct DB_TableProperty *property);

#ifndef RTI_CERT
/*ci
 * \brief Create a database table with record constructors and destructors
 *
 * \details
 *
 * This function creates an empty database table. Every table in the same
 * database must have a unique name. A table contains fixed size records.
 * Any memory allocation required by a record must be done outside of the
 * database. For example, any record that contains pointers or sequences
 * must be initialized outside of the table itself.
 *
 * \param[inout] table        Handle to new table. *table must be NULL on input.
 * \param[in]    db           Database where the table shall be created.
 *                            Cannot be NULL.
 * \param[in]    name         Name of the table. Must be unique in the database.
 *                            Cannot be NULL.
 * \param[in]    record_size  Size in bytes of a record. Must be > 0.
 * \param[in]    compare_func The compare function. Cannot be NULL.
 * \param[in]    property     Table properties. Cannot be NULL.
 * \param[in]    ctor         Record constructor. Can be NULL.
 * \param[in]    ctor_param   Parameter to pass to record constructor.
 * \param[in]    dtor         Record constructor. Can be NULL.
 * \param[in]    dtor_param   Parameter to pass to record decstructor.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the table was successfully created and table contains a handle to
 *         the newly created table.
 *
 * \sa \ref DB_Database_delete_table
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_create_table_w_ctor_and_dtor(DB_Table_T *table,
                         DB_Database_T db,
                         const char *name,
                         RTI_SIZE_T record_size,
                         DB_IndexCompare_T compare_func,
                         struct DB_TableProperty *property,
                         DB_Table_initializeFunc_T ctor,
                         void* ctor_param,
                         DB_Table_finalizeFunc_T dtor,
                         void* dtor_param);
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Delete a database table
 *
 * \details
 *
 * Delete a database table. A database table can only be deleted if it
 * is empty and there are no outstanding cursors.
 *
 * \param[in]  db           Database to delete table from.
 * \param[in]  table        Table to delete.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the table was successfully deleted.
 *
 * \sa \ref DB_Database_create_table
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Database_delete_table(DB_Database_T db,DB_Table_T table);
#endif /* !RTI_CERT */

/*************************** Table Functions **********************************/

/*ci
 * \brief Create record in a database table
 *
 * \details
 *
 * A table can only manage records created from the table. This function
 * returns an unused record. Note that the record is not initialized. It is
 * illegal to delete a table with records in use. That is, all records must be
 * deleted from a table before the table can be deleted.
 *
 * \param[in]    table   Table to create record in.
 * \param[inout] record  Created record. *record must be NULL on input.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the record was successfully created and a handle is returned in
 *         record.
 *
 * \sa \ref DB_Table_delete_record
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_create_record(DB_Table_T table,DB_Record_T *record);

/*ci
 * \brief Insert a record into the table
 *
 * \details
 *
 * A record becomes available to users of a table only after it is inserted.
 * If a record is inserted into a cursor selection the cursor is invalidated.
 *
 * \param[in] table   Table to insert record into.
 * \param[in] record  Record to insert.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the record was successfully inserted.
 *
 * \sa \ref DB_Table_delete_record
 *
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_insert_record(DB_Table_T table, DB_Record_T record);

/*ci
 * \brief Remove a record from a table
 *
 * \details
 *
 * Remove a record from the table based on the key. The record is no longer
 * search-able and it not reusable until deleted with
 * \ref DB_Table_delete_record.
 *
 * \param[in]  table  Table to remove record from.
 * \param[out] record Removed record.
 * \param[in]  key    The key to search for.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the record was removed.
 *
 * \sa \ref DB_Table_insert_record
 *
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_remove_record(DB_Table_T table,DB_Record_T *record,DB_Key_T key);

/*ci
 * \brief Delete a record from a table
 *
 * \details
 *
 * Delete the record from the table. A deleted record will be marked as free
 * for reuse and may be returned by \ref DB_Table_create_record.
 *
 * \param[in] table   Table to delete record from.
 * \param[in] record  Record to delete.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the record was deleted.
 *
 * \sa \ref DB_Table_create_record
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_delete_record(DB_Table_T table,DB_Record_T record);

/*ci
 * \brief Delete a record from a table with with a record destructor
 *
 * \details
 *
 * Delete the record from the table. A deleted record will be marked as free
 * for reuse and may be returned by \ref DB_Table_create_record. The provided
 * dtor, if not NULL, is called to finalize the record before it is returned to
 * for reuse. This is useful for records that use dynamically allocated
 * keys that are either the primary key or a part of a key for an index.
 *
 * \param[in] table   Table to delete record from.
 * \param[in] record  Record to delete.
 * \paran[in] dtor    Function pointer to call with record to finalize the
 *                    content. See \ref DB_Table_RecordDtor_T for details.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the record was deleted.
 *
 * \sa \ref DB_Table_create_record
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_delete_record_w_dtor(DB_Table_T table,DB_Record_T record,
                              DB_Table_RecordDtor_T dtor);

/*ci
 * \brief Create a new index for a table
 *
 * \details
 *
 * A table always has a default index based on the compare function passed
 * to \ref DB_Database_create_table function. It is however possible to create
 * a subset of the table or sort the records in a different order by creating
 * a new index. Note that it is illegal to delete a table with indices
 * (except for the default).
 *
 * \param[in]    table        Table to create index for.
 * \param[inout] index        Handle to index, *index must be NULL on input.
 * \param[in]    compare_func Compare function.
 * \param[in]    property     The index property.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the index was successfully created and a handle is available in the
 *         index output parameter.
 *
 * \sa \ref DB_Table_delete_index
 *
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_create_index(DB_Table_T table,
                      DB_Index_T *index,
                      DB_IndexCompare_T compare_func,
                      const struct DB_IndexProperty *const property);

/*ci
 * \brief Reindex an index
 *
 * \details
 *
 * When a record's secondary fields used in an index changes values it is
 * necessary to reindex since the sorting order may change.
 *
 * \param[in]    table        Table to create index for.
 * \param[inout] index        Handle to index to reindex.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the index was successfully reindexed.
 *
 * \sa \ref DB_Table_delete_index
 *
 */
DBDllExport void
DB_Table_reindex_index(DB_Table_T table, struct DB_Index *index);

/*ci
 * \brief Delete an index
 *
 * \details
 *
 * Delete an index previously created with \ref DB_Table_create_index
 *
 * \param[in] table        Table to delete index from.
 * \param[in] index        Handle to index.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         the index was successfully deleted.
 *
 * \sa \ref DB_Table_delete_index
 *
 */
#ifndef RTI_CERT
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_delete_index(DB_Table_T table,DB_Index_T index);
#endif /* !RTI_CERT */

/*ci
 * \brief Select all the records from an index
 *
 * \details
 *
 * Create cursor to iterate over all the records in an index. The
 * DB_Table_select_all_default function uses the default index which will return
 * all the records in the table. The records are ordered based on the compare
 * function for the index used for the selection.
 *
 * Note that the database should be locked while iterating over a cursor.
 *
 * \param[in]    table   Table to select records from.
 * \param[in]    index   Index to return records from.
 * \param[out]   cursor  Cursor to iterate over result set.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         a cursor was successfully created and a handle is available in the
 *         cursor output parameter.
 *
 * \sa \ref DB_Table_select_match, \ref DB_Table_select_range
 *
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_select_all(DB_Table_T table,DB_Index_T index,DB_Cursor_T *cursor);


/*ci
 * \brief Select all the records from the default index
 *
 * \details
 *
 * Create cursor to iterate over all the records in the default index.
 * Note that the database should be locked while iterating over a cursor.
 *
 * \param[in]    table   Table to iterate over.
 * \param[out]   cursor  Cursor to iterate over result set.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         a cursor was successfully created and a handle available in the
 *         cursor output parameter.
 *
 * \sa \ref DB_Table_select_match, \ref DB_Table_select_range
 *
 */
#define DB_Table_select_all_default(table_,eh_) \
        DB_Table_select_all(table_,DB_TABLE_DEFAULT_INDEX,eh_)

/*ci
 * \brief Search for a record in an index
 *
 * \details
 *
 * Search for a record with a matching key in an index. If a match is found the
 * record is returned in the record parameter. If none are found
 * DB_RETCODE_NO_DATA is returned. NOTE: The output parameter is only updated
 * on success. If no record is found the output argument is _not_ updated.
 *
 * \param[in]  table  Table to search in.
 * \param[in]  index  Index to return record from.
 * \param[out] record Updated with matching record if it is found.
 * \param[in]  key    Key to search for.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         record points to the matching record.
 *
 * \sa \ref DB_Table_select_all, \ref DB_Table_select_range
 *
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_select_match(DB_Table_T table,DB_Index_T index,
                     DB_Record_T *record,DB_Key_T key);

/*ci
 * \brief Select all the records from an index that lies within a range
 *
 * \details
 *
 * Select all records between [lower,upper] keys.
 *
 * \param[in]  table  Table to delete index from
 * \param[in]  index  Index to return records from
 * \param[out] cursor Cursor to the result set
 * \param[in]  lower  Lower key to search for
 * \param[in]  upper  Upper key to search for
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         cursor holds records which lies within the [lower,upper] range of
 *         keys.
 *
 * \sa \ref DB_Table_select_all, \ref DB_Table_select_match
 *
 */
SHOULD_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Table_select_range(DB_Table_T table,DB_Index_T index,
                     DB_Cursor_T *cursor,
                     DB_Key_T lower,DB_Key_T upper);

#if OSAPI_ENABLE_LOG
/*ci
 * \brief Return the name of a table
 *
 * \details
 *
 * Return the name of a table
 *
 * \param[in] table     Table to return name for, cannot be NULL.
 *
 * \return pointer to name of table in ASCIIZ format
 *
 * \sa
 */
DBDllExport const char*
DB_Table_get_name(DB_Table_T table);
#endif

#ifndef RTI_CERT
/*ci
 * \brief Return the number of records in a result set for a cursor
 *
 * \param[in]  cursor Cursor to result set.
 * \param[out] count  Number of records the cursor can iterate over.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         count contains the number of records.
 *
 * \sa \ref DB_Table_select_range, \ref DB_Table_select_all
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Cursor_get_count(DB_Cursor_T cursor,RTI_INT32 *count);
#endif

/*ci
 * \brief Return the next record the cursor is pointing to
 *
 * \details
 *
 * This function returns the next record the cursor is pointing to in its
 * result set. When a cursor is first created it does not point to a record,
 * but upon the first call to \ref DB_Cursor_get_next it points to the first
 * record (if any). Note that a cursor cannot be iterated over again, a new
 * cursor must be created.
 *
 * \param[in]  handle Cursor with a result set.
 * \param[out] record Pointer to the next record.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         record points to the next record.
 *
 * \sa \ref DB_Table_select_range, \ref DB_Table_select_all,
 *     \ref DB_Cursor_finish
 */
MUST_CHECK_RETURN DBDllExport DB_ReturnCode_T
DB_Cursor_get_next(DB_Cursor_T handle, DB_Record_T *record);

/*ci
 * \brief Close a cursor
 *
 * \details
 *
 * This function closes a cursor and returns it to the table. Note that it
 * is illegal to delete a table if there are open cursors.
 *
 * \param[in] table Table the cursor should be returned to. Must be the
 *                  same table the cursor was created from. Note that the
 *                  caller is responsible to make sure this is the case.
 *
 * \param[in] cursor Cursor to close.
 *
 * \return One of \ref DB_ReturnCode_T is returned. If DB_RETCODE_OK is returned
 *         record point to the next record.
 *
 * \sa \ref DB_Table_select_range, \ref DB_Table_select_all
 */
DBDllExport void
DB_Cursor_finish(DB_Table_T table,DB_Cursor_T cursor);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* db_api_h */

/*ci @} */
