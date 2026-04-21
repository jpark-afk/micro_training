/*
 * FILE: DBUrtdb.c - DB Implementation
 *
 * (c) Copyright, Real-Time Innovations, 2012-2020.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 06apr2021,tk MICRO-3002/PR.29029
 *   - Removed redundant variable 'j' in DB_Table_create_index
 * 06apr2021,tk MICRO-3000/PR.29024
 *   - Use RTI_TRUE/RTI_FALSE instead of 1/0 in calls to
 *     URTDB_Table_update_indices.
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 05may2015,tk MICRO-1179/PR#14680 Removed redundant code in if test
 * 10mar2015,tk MICRO-1114          Fixed index update
 * 17feb2015,tk MICRO-1071/PR#13967 Refactored and simplified search code
 * 02feb2015,tk MICRO-1039/PR#13557 Use correct log-message for cursor
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous parameters in URTDB_RecordHeader
 *                                - Removed unused search_array
 *                                - Removed unused hash function
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 30may2014,eh MICRO-204 (Verocel PR#1092): Add explanation of debug-only
 *              precondition checks
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 09may2014,eh MICRO-195 (Verocel PR#1088): remove URTDB_Index_finalize() from
 *              Cert
 * 07may2014,tk MICRO-203 (Verocel PR#1091): Added explanation of memory usage
 *              MICRO-205 (Verocel PR#1093): Added explanation of linked cursors
 *              MICRO-209 (Verocel PR#1095): Added explanation of output pointers
 * 06may2014,tk MICRO-227 (Verocel PR#1406): Refactored get_count() to better
 *              handle error conditions
 * 29apr2014,as MICRO-197 (Verocel PR#1090): Remove redundant code from
 *              URTDB_Table_select
 * 02aug2013,tk MICRO-365 (Verocel PR#1548): Use REDA_String_compare to
 *              compare strings.
 * 02aug2013,tk MICRO-214: Return error if DB_Database_create cannot
 *              allocate tables
 * 28may2012,tk Written
 */
/*ce
 * \file
 * \defgroup DBModule DB
 *
 * \details
 * This file implements the database API.
 */
/*ci \addtogroup DBModule
 *  @{
 */
#include "osapi/osapi_config.h"

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef db_log_h
#include "db/db_log.h"
#endif

#include "DBUrtdb.h"

/*ci
 *
 * \brief Convert from a database record to a user record
 *
 * \details
 *
 * A record is contiguous, fixed size block of memory structure as
 * follows:
 *
 * \verbatim
 * +------------+ <= &record[0]
 * |   header   | struct URTDB_RecordHeader
 * |            |
 * +------------+ <= &record[1] (When memory is viewed as an array of
 * |            |                type struct URTDB_RecordHeader)
 * |  user-part | void*
 * |            |
 * |            |
 * |            |
 * +------------+
 * \endverbatim
 *
 * Thus:
 * - &record[0] - Beginning of database record
 * - &record[1] - Beginning of user-part of data-base record. The user-part
 *                is allocated when a record is created with
 *                DB_Table_create_record()
 */
#define URTDB_RECORD_TO_USER_RECORD(r_) &r_[1]

/*ci
 * \brief This constant is added to the user defined size for a record
 */
#define URTDB_RECORDER_HEADER_SIZE (sizeof(struct URTDB_RecordHeader))

/*ci
 * \brief This constant is used to calculate the size of index arrays
 */
RTI_PRIVATE const RTI_SIZE_T URTDB_IndexEntry_fv_Size = sizeof(struct URTDB_IndexEntry);

/*ci
  * \brief Mask for cursor state, used to indicate if a cursor is allocated or
  *        not. 1 - free, 0 = allocated
  */
#define DB_CURSOR_STATE (0x00000001U)

 /*ci
   * \brief Mask for whether a cursor is currently used or not.
   *        1 - in use, 0 - not in use
   */
#define DB_CURSOR_INUSE (0x00000100U)

 /*ci
   * \brief Mask for index state, used to indicate if an index is allocated or
   *        not. 1 - free, 0 = allocated
   */
#define DB_INDEX_STATE  (0x00010000U)

 /*ci
   * \brief Mask for whether an index is currently in use or not.
   *        1 - in use, 0 - not in use
   */
#define DB_INDEX_INUSE  (0x01000000U)

/*ci
 * \brief The maximum number of indices which can be allocated.
 */
#define DB_INDEX_MAX     8U

 /*ci
  * \brief The maximum number of cursors which can be allocated.
  */
#define DB_CURSOR_MAX    8U

/*** SOURCE_BEGIN ***/

/*******************************************************************************
 *                                  Internal API
 *
 * - Internal APIs do not check inputs. Debug-only precondition checks are
 *   defensive only, as callers are internal and ensure parameters are valid.
 * - Internal APIs are never to be exported
 ******************************************************************************/

/*ci
 *
 * \brief Initialize an index
 *
 * \param[in] index        Index to initialize
 * \param[in] compare_func Compare function used to sort this index
 * \param[in] property     Index properties
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref URTDB_Index_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
URTDB_Index_initialize(struct DB_Index *index,
                       DB_IndexCompare_T compare_func,
                       struct DB_IndexProperty *property)
{
    RTI_SIZE_T i;

    index->compare = compare_func;
    index->sorted_array = NULL;
    index->record_count = 0;
    index->last_changed_index = 0;
#if OSAPI_ENABLE_PRECONDITION
    index->property = *property;
#endif

    OSAPI_Heap_allocate_array(&index->sorted_array,
                            property->max_entries,
                            struct URTDB_IndexEntry);
    if (index->sorted_array == NULL)
    {
        DB_LOG_SORTED_ALLOC(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < property->max_entries; ++i)
    {
        index->sorted_array[i].record = NULL;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Finalize an index
 *
 * \details
 *
 * Free up all resource used by an index
 *
 * \param[in] index Index to finalize
 *
 * \sa \ref URTDB_Index_initialize
 */
RTI_PRIVATE void
URTDB_Index_finalize(struct DB_Index *index)
{
    index->compare = NULL;

    if (index->sorted_array)
    {
        OSAPI_Heap_free_array(index->sorted_array);
        index->sorted_array = NULL;
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Search for a record matching an index
 *
 * \param[in]  index         The sorted index to search in
 * \param[in]  lower         The lowest array index to start the search in
 * \param[in]  upper         The upper array index to finish the search in
 * \param[in]  key           The key or record to search for based on the flags
 * \param[in]  opcode_flags  Whether the key is a full record or just the key
 * \param[out] result        If the record exists it is the array index where
 *                           the record is located, otherwise it is the array
 *                           index where the record would have have been
 *                           if it did exist
 *
 * \return DB_RETCODE_EXISTS when a matching record was found
 *         DB_RETCODE_NO_DATA when no matching record was found
 * \sa
 */
MUST_CHECK_RETURN RTI_PRIVATE DB_ReturnCode_T
URTDB_Table_find_ge(struct DB_Index *index,
                    RTI_INT32 lower,RTI_INT32 upper,
                    void *op2,RTI_INT32 opcode_flags,
                    RTI_INT32 *result)
{
    RTI_INT32 mid,cmp_result;
    DB_ReturnCode_T retcode = DB_RETCODE_NO_DATA;

    while (lower <= upper)
    {
        mid = lower + (upper-lower)/2;

        /* The user only have access to the user visible part */
        cmp_result = index->compare(opcode_flags,
              URTDB_RECORD_TO_USER_RECORD(index->sorted_array[mid].record),op2);

        if (!cmp_result)
        {
            *result = mid;
            retcode = DB_RETCODE_EXISTS;
            break;
        }
        else if (cmp_result < 0)
        {
            lower = mid+1;
        }
        else
        {
            upper = mid-1;
        }
    }

    if (lower > upper)
    {
        /* This only happens if the record is not found, in which case
         * retcode = DB_RETCODE_NO_DATA
         */
        *result = lower;
    }

    return retcode;
}

/*ci
 * \brief Update an index after a change in the table
 *
 * \param[in] urtdb_table   Table with changes
 * \param[in] index         Index to update
 * \param[in] new_record    Newly added/removed record
 * \param[in] remove_record True if record was removed
 *
 */
RTI_PRIVATE void
URTDB_Table_update_index(struct DB_Table *urtdb_table,
                         struct DB_Index *index,
                         struct URTDB_RecordHeader *new_record,
                         RTI_BOOL remove_record)
{
    RTI_INT32 insert_index,i;
    DB_ReturnCode_T retcode = DB_RETCODE_NO_DATA;
    RTI_INT32 record_count = 0;
    RTI_INT32 lower_index = 0;

    /* record_count == count _before_ new_record has been added/deleted
     */

    /* If the index is empty and a record is added then just add the record
     * to the index.
     */
    if ((index->record_count == 0) && !remove_record)
    {
        index->sorted_array[0].record = new_record;
        index->last_changed_index = 0;
        ++index->record_count;
        return;
    }

    /* If there are no more records in the default index, where all the
     * records in the database exists, then no more records can exist in the
     * index either. Note that it is not possible to remove a record from an
     * index if there is 1 record in the index and it is the record being
     * removed because there may be other records in the index that may
     * take its place.
     */
    if (urtdb_table->default_index.record_count == 0)
    {
        index->sorted_array[0].record = NULL;
        index->last_changed_index = 0;
        --index->record_count;
        return;
    }

    /* Check if the record exists in the index. It may not exist
     * if another record with a matching index already exists in the index
     */
    retcode = URTDB_Table_find_ge(index,0,index->record_count-1,
                                  URTDB_RECORD_TO_USER_RECORD(new_record),0,
                                  &insert_index);

    record_count = index->record_count - insert_index;

    if (remove_record && (retcode == DB_RETCODE_EXISTS) &&
            (new_record == index->sorted_array[insert_index].record))
    {
        /* The removed record exists in the index. Look for another
         * record to replace it in the index.
         */
        retcode = DB_RETCODE_NO_DATA;

        /* The default index is based on the primary key and can never
         * have a replacement record.
         */
        if (index != &urtdb_table->default_index)
        {
            /* This is slow for large tables. An alternative is to keep
             * a list of alternative records. However, the justification
             * is that it is not anticipated that there will be a high
             * frequency of deletion of records, only during shutdown and
             * liveliness losses. Note that the record being removed has
             * already been removed from the default index, thus it will
             * not find itself.
             */
            for (i = 0; i < urtdb_table->default_index.record_count; ++i)
            {
                if (!index->compare(0,
                        URTDB_RECORD_TO_USER_RECORD(urtdb_table->default_index.sorted_array[i].record),
                        URTDB_RECORD_TO_USER_RECORD(new_record)))
                {
                    retcode = DB_RETCODE_EXISTS;
                    lower_index = i;
                    break;
                }
            }
        }

        if (retcode == DB_RETCODE_EXISTS)
        {
            /* If a replacement record exists, substitute it for the old one
             */
            index->sorted_array[insert_index] = urtdb_table->default_index.sorted_array[lower_index];
            index->last_changed_index = insert_index;
        }
        else
        {
            /* Otherwise remove the entry. Note that it is not possible for
             * an index to have < 2 entry and get here. If there was only
             * 1 entry in the index then the entire table could only have
             * 1 entry, but that entry would have already been removed so
             * this code-path would never be entered.
             */
            OSAPI_Memory_move(&index->sorted_array[insert_index],
                              &index->sorted_array[insert_index+1],
                              URTDB_IndexEntry_fv_Size*((RTI_SIZE_T)record_count-1u));
            index->sorted_array[index->record_count-1].record = NULL;
            index->last_changed_index = insert_index;
            --index->record_count;
        }
    }
    else if (!remove_record && (retcode == DB_RETCODE_NO_DATA))
    {
        /* urtdb_table->record_count will be the _last_ index */
        if (insert_index != (index->record_count))
        {
            OSAPI_Memory_move(&index->sorted_array[insert_index+1],
                              &index->sorted_array[insert_index],
                              URTDB_IndexEntry_fv_Size*((RTI_SIZE_T)record_count));
        }
        index->sorted_array[insert_index].record = new_record;
        index->last_changed_index = insert_index;
        ++index->record_count;
    }
}

/*ci
 * \brief Update all indices for a table
 *
 * \details
 * This function is used to update all the current indices on a table. First
 * the default index is updated. This is important if a record is removed,
 * because the record must be removed to ensure it is not used by an index
 * as a replacement record. By removing the record from the default index
 * first, the default index is in a clean state and can safely be used to
 * find a replacement record.
 *
 * \param[in] urtdb_table   Table with changes
 * \param[in] new_record    Newly added/removed record
 * \param[in] remove_record True if record was removed
 *
 * \sa \ref URTDB_Table_update_index
 */
RTI_PRIVATE void
URTDB_Table_update_indices(struct DB_Table *urtdb_table,
                           struct URTDB_RecordHeader *new_record,
                           RTI_BOOL remove_record)
{
    RTI_UINT32 i;

    /* IMPORTANT: Always remove from the default record first to ensure that
     * the indices doesn't reuse that record for a substitute record.
     */
    URTDB_Table_update_index(urtdb_table,&urtdb_table->default_index,
                             new_record,remove_record);


    for (i = 0; i < DB_INDEX_MAX; ++i)
    {
        if (((DB_INDEX_INUSE << i) & urtdb_table->resource_state))
        {
            URTDB_Table_update_index(urtdb_table,
                                     &urtdb_table->indices[i],
                                     new_record,remove_record);
        }
    }
}

/*ci
 * \brief Update a cursor
 *
 * \details
 *
 * Update a cursor after an index has changed. If index update interferred
 * with the cursor's result set the cursor is invalidated.
 *
 * \param[in] cursor        Cursor to update
 * \param[in] index_removed True if an index was removed
 */
RTI_PRIVATE void
URTDB_Cursor_update(struct DB_Cursor *cursor,
                    RTI_BOOL index_removed)
{
    RTI_INT32 changed_index;

    if (cursor->index == NULL)
    {
        return;
    }

    changed_index = cursor->index->last_changed_index;

    if (changed_index > cursor->last_index)
    {
        return;
    }

    if (index_removed)
    {
        --cursor->last_index;
        if (changed_index < cursor->first_index)
        {
            --cursor->first_index;
        }
        if (changed_index <= cursor->current_index)
        {
            --cursor->current_index;
        }
    }
    else
    {
        if (changed_index < cursor->first_index)
        {
            /* If the inserted index is below the first record in the result set
             * it is guaranteed to not be part of the selection, so move cursors
             */
            ++cursor->first_index;
            ++cursor->current_index;
            ++cursor->last_index;
        }
        else if (changed_index < cursor->current_index)
        {
            /* If an inserted record modifies part of the selection that
             * has already been iterated over, invalidate the cursor
             */
            cursor->index = NULL;
        }
        else
        {
            ++cursor->last_index;
        }
    }
}

/*ci
 * \brief Update all open cursors on a table
 *
 * Update all open cursors on a table when an index changes
 *
 * \param[in] urtdb_table    Table with cursors to update
 * \param[in] index_removed  True if an index was removed
 */
RTI_PRIVATE void
URTDB_Table_update_cursors(struct DB_Table *urtdb_table,
                           RTI_BOOL index_removed)
{
    RTI_UINT32 i;

    for (i = 0; i < DB_CURSOR_MAX; ++i)
    {
        if (((DB_CURSOR_INUSE << i) & urtdb_table->resource_state))
        {
            URTDB_Cursor_update(&urtdb_table->cursors[i],index_removed);
        }
    }
}

/*ci
 *
 * \brief Select records from a table based on a search criteria
 *
 * \details
 * This function searches for records matching the search criteria and
 * returns a cursor on success. A cursor is required to interrogate the
 * results, such as the number of records, or iterate over the records.
 *
 * A successful call returns a cursor. All cursors currently in-use is linked
 * to a cursor list. When a change in a table occurs this list is used to
 * determine if a cursor is still valid or not.
 *
 * \param[in]    table   Table to search
 * \param[out]   index   Index to use for searching
 * \param[out]   eh      Cursor with result set.
 * \param[in]    opcode  Which select operation to apply
 * \param[in]    arg0    lower limit or exact match
 * \param[in]    arg1    upper limit or NULL
 *
 * \return One of the following return code are returned:
 *         DB_RETCODE_OK      - A cursor with 0 or more records are
 *                              returned. The cursor must be returned
 *                              when done.
 *         DB_RETCODE_OUT_OF_RESOURCES - No cursors are available
 *
 * \sa DB_Cursor_get_count, DB_Cursor_get_next, DB_Cursor_finish
 */
MUST_CHECK_RETURN RTI_PRIVATE DB_ReturnCode_T
URTDB_Table_select(struct DB_Table *table,
                   DB_Index_T index,
                   DB_Cursor_T *eh,
                   DB_Select_T opcode,
                   void *arg0,void *arg1)
{
    struct DB_Cursor *cursor = NULL;
    DB_ReturnCode_T retcode;
    RTI_INT32 lower_index = 0;
    RTI_INT32 upper_index = 0;
    RTI_UINT32 i;

    *eh = NULL;

    for (i = 0; i < DB_CURSOR_MAX; ++i)
    {
        if ((DB_CURSOR_STATE << i) & table->resource_state)
        {
            cursor = &table->cursors[i];
            table->resource_state &= ~(DB_CURSOR_STATE << i);
            table->resource_state |= (DB_CURSOR_INUSE << i);
            break;
        }
    }

    if (i == DB_CURSOR_MAX)
    {
        DB_LOG_OUT_OF_CURSORS(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_OUT_OF_RESOURCES;
    }

    *eh = cursor;

    /* The index to select from */
    cursor->index = index;

    /* These indices are all invalid and the initial state for the cursor,
     * and also ensures that last - first + 1 = 0 for get_count()
     */
    cursor->first_index = -1;
    cursor->current_index = -1;
    cursor->last_index = cursor->first_index - 1;

    if (index->record_count == 0)
    {
        /* There are no records, but that is OK */
        return DB_RETCODE_OK;
    }
    else if (opcode == DB_SELECTOPCODE_ALL)
    {
        /* At least one record exists, update indices */
        cursor->first_index = 0;
        cursor->current_index = 0;

        /* The last index is set to one less than the record count
         * (0 based index) and guarantees that the calculation
         * (last - first + 1) always return 0 in case
         * there are no records or > 0 if there are records.
         */
        cursor->last_index = index->record_count - 1;
    }
    else if (opcode == DB_SELECTOPCODE_BETWEEN)
    {
        /* Look for records within a range. The range must be from
         * low to high as defined by the compare function. Start by
         * searching for the lowest record within the range.
         */
        retcode = URTDB_Table_find_ge(index,0,index->record_count-1,
                                      arg0,DB_SELECT_OP2_AS_KEY,&lower_index);

        if (lower_index >= index->record_count)
        {
            /* if lower_index is >= the highest index the lowest range was
             * not found. Thus, no data exists in the range [low,high]
             */
            return DB_RETCODE_OK;
        }

        /* At least one index matched the criteria, update indices */
        cursor->first_index = lower_index;
        cursor->current_index = lower_index;

        if ((lower_index == index->record_count-1) &&
            (retcode == DB_RETCODE_EXISTS))
        {
            /* If the found record is the only record and was an exact match
             * it it not necessary to search for the upper index.
             */
            cursor->last_index = lower_index;
        }
        else
        {
            /* The lower bound was found, next find the highest index <= to the
             * upper bound. Since we start the search at the lower bound we
             * are guaranteed to find at least one record inside the range.
             * Note that the search starts at the lower index found. This
             * ensures that in the case where the range spans 1 record, both
             * lower and upper can test for equality.
             */
            retcode = URTDB_Table_find_ge(index,lower_index,
                                      index->record_count-1,
                                      arg1,DB_SELECT_OP2_AS_KEY,&upper_index);

            if (retcode == DB_RETCODE_NO_DATA)
            {
                if (upper_index == lower_index)
                {
                    /* A special case is when both the lower bound and the upper
                     * bound is less then the smallest record. In this case
                     * no records exist so the cursor must be reset to an
                     * empty set.
                     */
                    cursor->first_index = -1;
                    cursor->current_index = -1;
                }
                else
                {
                    /* If the upper bound did not exist the index returned
                     * is where the record should have been, which is 1 higher
                     * then the record which should be included in the range
                     */
                    cursor->last_index = upper_index - 1;
                }
            }
            else
            {
                /* An exact match was found at upper index  */
                cursor->last_index = upper_index;
            }
        }
    }

    return DB_RETCODE_OK;
}

/*******************************************************************************
 *                                  Public API
 *
 * All public APIs are documented in the public header files.
 ******************************************************************************/
DB_ReturnCode_T
DB_Database_create(DB_Database_T *db,
                   const char *name,
                   struct DB_DatabaseProperty *db_property,
                   struct OSAPI_Mutex *const shared_lock)
{
    DB_ReturnCode_T retcode = DB_RETCODE_OK;
    struct DB_Database *urtdb = NULL;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;

    OSAPI_PRECONDITION((db == NULL) || (*db != NULL) ||
                            (db_property == NULL) || (name == NULL),
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("db",db,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("*dp",
                                        (db != NULL ? *db : NULL),RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("property",db_property,RTI_TRUE);)

    *db = NULL;
    if (REDA_String_length(name) > (URTDB_DATABASE_NAME_MAX_LENGTH - 1))
    {
        DB_LOG_NAME_TOO_LONG(OSAPI_LOGKIND_ERROR,REDA_String_length(name))
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (db_property->max_tables == 0)
    {
        DB_LOG_ILLEGAL_TABLE_SIZE(OSAPI_LOGKIND_ERROR,db_property->max_tables)
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (((db_property->lock_mode == DB_LOCK_LEVEL_SHARED) &&
            shared_lock == NULL) ||
            ((db_property->lock_mode != DB_LOCK_LEVEL_SHARED) &&
                        shared_lock != NULL))
    {
        DB_LOG_ILLEGAL_LOCK_MODE(OSAPI_LOGKIND_ERROR,
                                 db_property->lock_mode,shared_lock)
        return DB_RETCODE_BAD_PARAMETER;
    }

    OSAPI_Heap_allocate_struct(&urtdb,struct DB_Database);
    if (!urtdb)
    {
        DB_LOG_ALLOC_DATABASE(OSAPI_LOGKIND_ERROR)
        retcode = DB_RETCODE_ERROR;
        goto done;
    }

    urtdb->property = *db_property;
    urtdb->tables = NULL;
    urtdb->lock = NULL;
    REDA_CircularList_init(&urtdb->tables_in_use);

    OSAPI_Memory_copy((void *)urtdb->name,(const void *)name,
                      REDA_String_length(name) + 1);

    if (db_property->lock_mode > DB_LOCK_LEVEL_SHARED)
    {
        urtdb->lock = OSAPI_Mutex_new();
        if (urtdb->lock == NULL)
        {
            DB_LOG_MUTEX_ALLOC(OSAPI_LOGKIND_ERROR)
            retcode = DB_RETCODE_ERROR;
            goto done;
        }
    }
    else if (db_property->lock_mode == DB_LOCK_LEVEL_SHARED)
    {
        urtdb->lock = shared_lock;
    }

    pool_property.buffer_size = sizeof(struct DB_Table);
    pool_property.max_buffers = urtdb->property.max_tables;
    urtdb->tables = REDA_BufferPool_new(name,&pool_property,NULL,urtdb,NULL,urtdb);
    if (urtdb->tables == NULL)
    {
        DB_LOG_ALLOC_TABLE_POOL(OSAPI_LOGKIND_ERROR,
                                pool_property.buffer_size,
                                pool_property.max_buffers)
        retcode = DB_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    retcode = DB_RETCODE_OK;
    *db = urtdb;

done:

    return retcode;
}

#ifndef RTI_CERT
DB_ReturnCode_T
DB_Database_delete(DB_Database_T db)
{
    struct DB_Table *table;
    struct DB_Table *table_next;
    DB_ReturnCode_T retcode;

    OSAPI_PRECONDITION(db == NULL,
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("db",db,RTI_TRUE);)

    table = (struct DB_Table*)REDA_CircularList_get_first(&db->tables_in_use);

    while (!REDA_CircularList_node_at_head(&db->tables_in_use,&table->node))
    {
        table_next = (struct DB_Table*)REDA_CircularListNode_get_next(
                                                                &table->node);
        retcode = DB_Database_delete_table(db,table);
        if (retcode != DB_RETCODE_OK)
        {
            return DB_RETCODE_EXISTS;
        }
        table = table_next;
    }

    if (!REDA_CircularList_is_empty(&db->tables_in_use))
    {
        DB_LOG_TABLES_INUSE(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_EXISTS;
    }

    if (db->tables != NULL)
    {
        if (!REDA_BufferPool_delete(db->tables))
        {
            return DB_RETCODE_EXISTS;
        }
    }

    if ((db->property.lock_mode != DB_LOCK_LEVEL_SHARED) && (db->lock != NULL))
    {
        if (!OSAPI_Mutex_delete(db->lock))
        {
            return DB_RETCODE_ERROR;
        }
    }

    OSAPI_Heap_free_struct(db);

    return DB_RETCODE_OK;
}
#endif /* !RTI_CERT */

#if OSAPI_MUTEX_TRACE_ENABLED
DB_ReturnCode_T
DB_Database_lock_(DB_Database_T db,char *file,RTI_INT32 lineno)
#else
DB_ReturnCode_T
DB_Database_lock(DB_Database_T db)
#endif
{
    OSAPI_PRECONDITION(db == NULL,
                           return DB_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("db",db,RTI_TRUE);)

#if OSAPI_MUTEX_TRACE_ENABLED
    if (db->lock && !OSAPI_Mutex_take_(db->lock,file,lineno))
#else
    if (db->lock && !OSAPI_Mutex_take(db->lock))
#endif
    {
        DB_LOG_LOCK_FAILURE(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_ERROR;
    }

    return DB_RETCODE_OK;
}

#if OSAPI_MUTEX_TRACE_ENABLED
DB_ReturnCode_T
DB_Database_unlock_(DB_Database_T db,char *file,RTI_INT32 lineno)
#else
DB_ReturnCode_T
DB_Database_unlock(DB_Database_T db)
#endif
{
    OSAPI_PRECONDITION(db == NULL,
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("db",db,RTI_TRUE);)

#if OSAPI_MUTEX_TRACE_ENABLED
    if (db->lock && !OSAPI_Mutex_give_(db->lock,file,lineno))
#else
    if (db->lock && !OSAPI_Mutex_give(db->lock))
#endif
    {
        DB_LOG_UNLOCK_FAILURE(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_ERROR;
    }

    return DB_RETCODE_OK;
}

#ifndef RTI_CERT
/*ci
 * \brief Forwarder for Database table constructors
 *
 * \param[in] initialize_param Table parameters
 * \param[in] buffer           Database entry to initialize 
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataBase_forward_ctor(void *initialize_param, void *buffer)
{
    struct DB_TableRowCtor *user_ctor =
                            (struct DB_TableRowCtor*)initialize_param;

    if (user_ctor->ctor == NULL)
    {
        return RTI_TRUE;
    }
    
    return user_ctor->ctor(user_ctor->user_param,
                           (char*)buffer + URTDB_RECORDER_HEADER_SIZE);
}

/*ci
 * \brief Forwarder for Database table destructors
 *
 * \param[in] initialize_param Table parameters
 * \param[in] buffer           Database entry to finalize 
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_Database_forward_dtor(void* initialize_param, void *buffer)
{
    struct DB_TableRowDtor *user_dtor =
                        (struct DB_TableRowDtor*)initialize_param;

    if (user_dtor->dtor == NULL)
    {
        return RTI_TRUE;
    }
    
    return user_dtor->dtor(user_dtor->user_param,
                           (char*)buffer + URTDB_RECORDER_HEADER_SIZE);
}
#endif /* ! RTI_CERT */

#ifndef RTI_CERT
DB_ReturnCode_T
DB_Database_create_table_w_ctor_and_dtor(DB_Table_T *table,
                                         DB_Database_T db,
                                         const char *name,
                                         RTI_SIZE_T record_size,
                                         DB_IndexCompare_T compare_func,
                                         struct DB_TableProperty *property,
                                         DB_Table_initializeFunc_T ctor,
                                         void* ctor_param,
                                         DB_Table_finalizeFunc_T dtor,
                                         void* dtor_param)
#else
DB_ReturnCode_T
DB_Database_create_table(DB_Table_T *table,
                         DB_Database_T db,
                         const char *name,
                         RTI_SIZE_T record_size,
                         DB_IndexCompare_T compare_func,
                         struct DB_TableProperty *property)
#endif /* !RTI_CERT */
{
    DB_ReturnCode_T retcode = DB_RETCODE_OK;
    struct DB_Table *out_table = NULL;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct DB_IndexProperty index_property = DB_IndexProperty_INITIALIZER;
    RTI_SIZE_T i;

    OSAPI_PRECONDITION(((table == NULL) || (*table != NULL) ||
                            (db == NULL) || (property == NULL) ||
                            (name == NULL) || (compare_func == NULL)),
                            return DB_RETCODE_BAD_PARAMETER,
                OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("*table",(table != NULL ? *table : NULL),RTI_FALSE);
                OSAPI_Log_entry_add_pointer("db",db,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("compare",compare_func != NULL ? (void*)0x1 : NULL,RTI_TRUE);)

    if (REDA_String_length(name) > URTDB_TABLE_NAME_MAX_LENGTH - 1)
    {
        DB_LOG_TABLE_NAME_TOO_LONG(OSAPI_LOGKIND_ERROR,name)
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (property->max_records == 0)
    {
        DB_LOG_ILLEGAL_RECORD_COUNT(OSAPI_LOGKIND_ERROR,property->max_records)
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (property->max_indices > DB_INDEX_MAX)
    {
        DB_LOG_ILLEGAL_RECORD_COUNT(OSAPI_LOGKIND_ERROR,property->max_indices)
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (property->max_cursors > DB_CURSOR_MAX)
    {
        DB_LOG_ILLEGAL_RECORD_COUNT(OSAPI_LOGKIND_ERROR,property->max_cursors)
        retcode = DB_RETCODE_BAD_PARAMETER;
        goto done;
    }

#if OSAPI_ENABLE_PRECONDITION
    out_table = (struct DB_Table*)REDA_CircularList_get_first(&db->tables_in_use);

    while (!REDA_CircularList_node_at_head(&db->tables_in_use,&out_table->node))
    {
        if (!REDA_String_ncompare(out_table->name,name,
                                  URTDB_DATABASE_NAME_MAX_LENGTH - 1))
        {
            break;
        }
        out_table = (struct DB_Table*)REDA_CircularListNode_get_next(&out_table->node);
    }

    if (!REDA_CircularList_node_at_head(&db->tables_in_use,&out_table->node))
    {
        DB_LOG_TABLE_EXISTS(OSAPI_LOGKIND_ERROR,out_table->name)
        retcode = DB_RETCODE_EXISTS;
        goto done;
    }
#endif

    out_table = (struct DB_Table*)REDA_BufferPool_get_buffer(db->tables);
    if (out_table == NULL)
    {
        DB_LOG_OUT_OF_TABLES(OSAPI_LOGKIND_ERROR)
        retcode = DB_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

#if OSAPI_ENABLE_PRECONDITION
    out_table->property = *property;
#else
    out_table->max_records = property->max_records;
#endif

#if OSAPI_ENABLE_PRECONDITION
    OSAPI_Memory_copy((void *)out_table->name, (const void *)name,
                       REDA_String_length(name) + 1);
#endif
#ifndef RTI_CERT
    out_table->user_ctor.ctor = ctor;
    out_table->user_ctor.user_param = ctor_param;
    out_table->user_dtor.dtor = dtor;
    out_table->user_dtor.user_param = dtor_param;
#endif /* !RTI_CERT */
    pool_property.buffer_size = (RTI_SIZE_T)(URTDB_RECORDER_HEADER_SIZE + record_size);
    pool_property.max_buffers = property->max_records;
#ifndef RTI_CERT
    out_table->records = REDA_BufferPool_new(
                            "records",&pool_property,
                            DDS_DataBase_forward_ctor,(void*)&out_table->user_ctor,
                            DDS_Database_forward_dtor,(void*)&out_table->user_dtor);
#else
    out_table->records = REDA_BufferPool_new("records",&pool_property,NULL,NULL,NULL,NULL);
#endif /* !RTI_CERT */
    if (out_table->records == NULL)
    {
        DB_LOG_ALLOC_RECORD_POOL(OSAPI_LOGKIND_ERROR)
        retcode = DB_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    out_table->indices = NULL;
    if (property->max_indices > 0)
    {
        OSAPI_Heap_allocate_array(&out_table->indices,property->max_indices,struct DB_Index);
        if (out_table->indices == NULL)
        {
            DB_LOG_ALLOC_INDEX_POOL(OSAPI_LOGKIND_ERROR)
            retcode = DB_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }

    out_table->cursors = NULL;
    if (property->max_cursors > 0)
    {
        OSAPI_Heap_allocate_array(&out_table->cursors,property->max_cursors,struct DB_Cursor);
        if (out_table->cursors == NULL)
        {
            DB_LOG_ALLOC_CURSOR_POOL(OSAPI_LOGKIND_ERROR)
            retcode = DB_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }

    REDA_CircularList_init(&out_table->records_in_use);

    out_table->resource_state = 0;
    for (i = 0; i < property->max_indices; ++i)
    {
        out_table->resource_state |= DB_INDEX_STATE << i;
    }

    for (i = 0; i < property->max_cursors; ++i)
    {
        out_table->resource_state |= DB_CURSOR_STATE << i;
    }

    REDA_CircularListNode_init(&out_table->node);

    index_property.max_entries = property->max_records;

    if (!URTDB_Index_initialize(&out_table->default_index,
                                compare_func,&index_property))
    {
        retcode = DB_RETCODE_ERROR;
        goto done;
    }

    REDA_CircularList_link_node_after(&db->tables_in_use,&out_table->node);

    *table = (DB_Table_T) out_table;

    retcode = DB_RETCODE_OK;

done:

    return retcode;
}

#ifndef RTI_CERT
DB_ReturnCode_T
DB_Database_create_table(DB_Table_T *table,
                         DB_Database_T db,
                         const char *name,
                         RTI_SIZE_T record_size,
                         DB_IndexCompare_T compare_func,
                         struct DB_TableProperty *property)
{
    return DB_Database_create_table_w_ctor_and_dtor(table,
                                            db,
                                            name,
                                            record_size,
                                            compare_func,
                                            property,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
DBDllExport DB_ReturnCode_T
DB_Database_delete_table(DB_Database_T db,DB_Table_T table)
{
    DB_ReturnCode_T retcode = DB_RETCODE_NO_DATA;
#if OSAPI_ENABLE_PRECONDITION
    RTI_SIZE_T i,mask;
#endif

    OSAPI_PRECONDITION(((table == NULL) || (db == NULL)),
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("db",db,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

    if (!REDA_CircularListNode_is_linked(&table->node))
    {
        DB_LOG_TABLE_NOT_INUSE(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_BAD_PARAMETER;
    }

    if (!REDA_CircularList_is_empty(&table->records_in_use))
    {
        DB_LOG_RECORDS_INUSE(OSAPI_LOGKIND_ERROR,DB_Table_get_name(table))
        return DB_RETCODE_EXISTS;
    }

#if OSAPI_ENABLE_PRECONDITION
    mask = 0;
    for (i = 0; i < table->property.max_indices; ++i)
    {
        mask |= DB_INDEX_STATE << i;
    }

    for (i = 0; i < table->property.max_cursors; ++i)
    {
        mask |= DB_CURSOR_STATE << i;
    }

    if (table->resource_state != mask)
    {
        DB_LOG_INDEX_INUSE(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_EXISTS;
    }
#endif

    if ((table->records) && !REDA_BufferPool_delete(table->records))
    {
        DB_LOG_RECORDS_INUSE(OSAPI_LOGKIND_ERROR,DB_Table_get_name(table))
        retcode = DB_RETCODE_EXISTS;
        goto done;
    }

    if (table->indices)
    {
        OSAPI_Heap_free_array(table->indices);
    }

    if (table->cursors)
    {
        OSAPI_Heap_free_array(table->cursors);
    }

    URTDB_Index_finalize(&table->default_index);

    REDA_CircularList_unlink_node(&table->node);

    REDA_BufferPool_return_buffer(db->tables,table);

    retcode = DB_RETCODE_OK;

done:

    return retcode;
}
#endif /* !RTI_CERT */

/*************************** Table Functions **********************************/
#if OSAPI_ENABLE_LOG
const char*
DB_Table_get_name(DB_Table_T table)
{
    OSAPI_PRECONDITION(table == NULL,return NULL,
                        OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

#if OSAPI_ENABLE_PRECONDITION
    return table->name;
#else
    UNUSED_ARG(table);
    return "<not set>";
#endif
}
#endif

DB_ReturnCode_T
DB_Table_create_record(DB_Table_T table,DB_Record_T *record)
{
    DB_ReturnCode_T retcode = DB_RETCODE_ERROR;
    struct URTDB_RecordHeader *internal_record;

    OSAPI_PRECONDITION(((table == NULL) || (record == NULL) || (*record != NULL)),
                            return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("record",record,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("*record",
                                (record != NULL ? *record : NULL),RTI_TRUE);)

    internal_record = (struct URTDB_RecordHeader*)REDA_BufferPool_get_buffer(
                                                            table->records);
    if (internal_record == NULL)
    {
        DB_LOG_OUT_OF_RECORDS(OSAPI_LOGKIND_ERROR)
        retcode = DB_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    REDA_CircularListNode_init(&internal_record->node);
    *record = (DB_Record_T)URTDB_RECORD_TO_USER_RECORD(internal_record);

    retcode = DB_RETCODE_OK;

done:
    return retcode;
}

DB_ReturnCode_T
DB_Table_insert_record(DB_Table_T table, DB_Record_T record)
{
    struct URTDB_RecordHeader *urdb_record = (struct URTDB_RecordHeader *)record;
    DB_ReturnCode_T retcode = DB_RETCODE_ERROR;
    RTI_INT32 v_index;

    OSAPI_PRECONDITION(((table == NULL) || (record == NULL)),
                        return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("record",record,RTI_TRUE);)

    --urdb_record;
    retcode = URTDB_Table_find_ge(&table->default_index,
                          0,table->default_index.record_count-1,
                          URTDB_RECORD_TO_USER_RECORD(urdb_record),0,&v_index);

    if (retcode != DB_RETCODE_NO_DATA)
    {
        DB_LOG_RECORD_ALREADY_EXISTS(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    REDA_CircularList_link_node_after(&table->records_in_use,
                                      &urdb_record->node);
    URTDB_Table_update_indices(table,urdb_record,RTI_FALSE);
    URTDB_Table_update_cursors(table,RTI_FALSE);

    retcode = DB_RETCODE_OK;

done:
    return retcode;
}

void
DB_Table_reindex_index(DB_Table_T table, struct DB_Index *index)
{
    RTI_INT32 si;

    OSAPI_PRECONDITION(((table == NULL) || (index == NULL)),
                       return,
                       OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("index",index,RTI_TRUE);)

    index->record_count = 0;
    index->last_changed_index = 0;

    for (si = 0; si < table->default_index.record_count; si++)
    {
        URTDB_Table_update_index(table,index,
                                table->default_index.sorted_array[si].record,
                                RTI_FALSE);
    }
}

DB_ReturnCode_T
DB_Table_delete_record_w_dtor(DB_Table_T table,DB_Record_T record,
                              DB_Table_RecordDtor_T dtor)
{
    DB_ReturnCode_T retcode = DB_RETCODE_ERROR;
    struct URTDB_RecordHeader *urdb_record = (struct URTDB_RecordHeader *)record;
    RTI_INT32 v_index;

    OSAPI_PRECONDITION(((table == NULL) || (record == NULL)),
                        return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("record",record,RTI_TRUE);)

    --urdb_record;
    if (REDA_CircularListNode_is_linked(&urdb_record->node))
    {
        retcode = URTDB_Table_find_ge(&table->default_index,
                          0,table->default_index.record_count-1,
                          URTDB_RECORD_TO_USER_RECORD(urdb_record),0,&v_index);

        if (retcode != DB_RETCODE_EXISTS)
        {
            DB_LOG_RECORD_DOES_NOT_EXIST(OSAPI_LOGKIND_ERROR)
            retcode = DB_RETCODE_NO_DATA;
            goto done;
        }

        URTDB_Table_update_indices(table,urdb_record,RTI_TRUE);
        URTDB_Table_update_cursors(table,RTI_TRUE);
        REDA_CircularList_unlink_node(&urdb_record->node);
    }

    if (dtor)
    {
        if (!dtor(record))
        {
            return DB_RETCODE_ERROR;
        }
    }

    REDA_BufferPool_return_buffer(table->records,urdb_record);

    retcode = DB_RETCODE_OK;

done:
    return retcode;
}

DB_ReturnCode_T
DB_Table_delete_record(DB_Table_T table,DB_Record_T record)
{
    return DB_Table_delete_record_w_dtor(table,record,NULL);
}

DB_ReturnCode_T
DB_Table_remove_record(DB_Table_T table,DB_Record_T *record,DB_Key_T key)
{
    DB_ReturnCode_T retcode = DB_RETCODE_OK;
    struct URTDB_RecordHeader *existing_record;
    RTI_INT32 v_index;

    OSAPI_PRECONDITION(((table == NULL) || (record == NULL) ||
                            (key == NULL) || (*record != NULL)),
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("record",record,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("key",key,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("*record",
                                 (record != NULL ? *record : NULL),RTI_TRUE);)

    retcode = URTDB_Table_find_ge(&table->default_index,
                                  0,table->default_index.record_count-1,key,
                                  DB_SELECT_OP2_AS_KEY,&v_index);

    if (retcode != DB_RETCODE_EXISTS)
    {
        retcode = DB_RETCODE_NO_DATA;
        goto done;
    }

    existing_record = table->default_index.sorted_array[v_index].record;
    URTDB_Table_update_indices(table,existing_record,RTI_TRUE);
    URTDB_Table_update_cursors(table,RTI_TRUE);
    REDA_CircularList_unlink_node(&existing_record->node);
    *record = URTDB_RECORD_TO_USER_RECORD(existing_record);
    retcode = DB_RETCODE_OK;

done:
    return retcode;
}

DB_ReturnCode_T
DB_Table_create_index(DB_Table_T table,
                      DB_Index_T *index,
                      DB_IndexCompare_T compare_func,
                      const struct DB_IndexProperty *const property)
{
    struct DB_Index *a_index = NULL;
    struct DB_IndexProperty real_property;
    RTI_SIZE_T i;
    RTI_INT32 si;

    OSAPI_PRECONDITION((index == NULL) || (property == NULL) ||
                            (compare_func == NULL) || (table == NULL),
                    return DB_RETCODE_BAD_PARAMETER,
                    OSAPI_Log_entry_add_pointer("index",index,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("*compare",compare_func != NULL ? (void*)0x1 : NULL,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

    for (i = 0; i < DB_INDEX_MAX; ++i)
    {
        if ((DB_INDEX_STATE << i) & table->resource_state)
        {
            a_index = &table->indices[i];
            table->resource_state &= ~(DB_INDEX_STATE << i);
            break;
        }
    }

    if (i == DB_INDEX_MAX)
    {
        DB_LOG_OUT_OF_INDICES(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_OUT_OF_RESOURCES;
    }

    real_property = *property;
    if (real_property.max_entries == 0)
    {
#if OSAPI_ENABLE_PRECONDITION
        real_property.max_entries = table->property.max_records;
#else
        real_property.max_entries = table->max_records;
#endif
    }

    if (!URTDB_Index_initialize(a_index,compare_func,&real_property))
    {
        table->resource_state |= (DB_INDEX_STATE << i);
        return DB_RETCODE_OUT_OF_RESOURCES;
    }

    /* Add each new record to the new index
     */
    for (si = 0; si < table->default_index.record_count; si++)
    {
        URTDB_Table_update_index(table,a_index,
                                table->default_index.sorted_array[si].record,
                                RTI_FALSE);
    }

    table->resource_state |= (DB_INDEX_INUSE << i);
    *index = a_index;

    return DB_RETCODE_OK;
}

#ifndef RTI_CERT
DB_ReturnCode_T
DB_Table_delete_index(DB_Table_T table,DB_Index_T index)
{
    RTI_SIZE_T i;

    OSAPI_PRECONDITION((index == NULL) || (table == NULL),
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("index",index,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

    URTDB_Index_finalize(index);

    for (i = 0; i < DB_INDEX_MAX; ++i)
    {
        if ((index == &table->indices[i]) && !(table->resource_state & (DB_INDEX_STATE << i)))
        {
            table->resource_state |= (DB_INDEX_STATE << i);
            table->resource_state &= ~(DB_INDEX_INUSE << i);
            break;
        }
    }

    return DB_RETCODE_OK;
}
#endif /* !RTI_CERT */

DB_ReturnCode_T
DB_Table_select_all(DB_Table_T table,DB_Index_T index,DB_Cursor_T *eh)
{
    OSAPI_PRECONDITION((eh == NULL) || (table == NULL),
                        return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("eh",eh,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

    if (index == DB_TABLE_DEFAULT_INDEX)
    {
        index = &table->default_index;
    }

    return URTDB_Table_select(table,index,eh,
                              DB_SELECTOPCODE_ALL,NULL,NULL);
}

DB_ReturnCode_T
DB_Table_select_match(DB_Table_T table,
                      DB_Index_T index,
                      DB_Record_T *record,
                      DB_Key_T key)
{
    DB_ReturnCode_T retcode = DB_RETCODE_ERROR;
    RTI_INT32 v_index;

    OSAPI_PRECONDITION(((table == NULL) || (record == NULL) ||
                            (key == NULL)),
                        return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("record",record,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("key",key,RTI_TRUE);)

    if (index == DB_TABLE_DEFAULT_INDEX)
    {
        index = &table->default_index;
    }

    retcode = URTDB_Table_find_ge(index,0,index->record_count-1,
                                  key,DB_SELECT_OP2_AS_KEY,&v_index);

    if (retcode != DB_RETCODE_EXISTS)
    {
        goto done;
    }

    *record = URTDB_RECORD_TO_USER_RECORD(index->sorted_array[v_index].record);

    retcode = DB_RETCODE_OK;

done:
    return retcode;
}

DB_ReturnCode_T
DB_Table_select_range(DB_Table_T table,
                      DB_Index_T index,
                      DB_Cursor_T *eh,
                      DB_Key_T lower,
                      DB_Key_T upper)
{
    OSAPI_PRECONDITION((table==NULL) || (eh==NULL) ||
                           (lower==NULL) || (upper==NULL),
                            return DB_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("table",table,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("eh",eh,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("lower",lower,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("upper",upper,RTI_TRUE);)

    if (index == DB_TABLE_DEFAULT_INDEX)
    {
        index = &table->default_index;
    }

    return URTDB_Table_select(table,index,eh,
                              DB_SELECTOPCODE_BETWEEN,lower,upper);
}

#ifndef RTI_CERT
DB_ReturnCode_T
DB_Cursor_get_count(DB_Cursor_T cursor,RTI_INT32 *count)
{
    /* DB_Cursor_get_count() is called only after DB_Table_select_* 
     *  has returned a valid cursor.
     *  Thus, the precondition check is defensive/debug only.
     */
    OSAPI_PRECONDITION((cursor == NULL) || (count == NULL),
                           return DB_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("cursor",cursor,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("count",count,RTI_TRUE);)

    /* A cursor may be invalidated if a record is inserted inside the
     * range of records currently selected by this cursor.
     */
    if (cursor->index == NULL)
    {
        DB_LOG_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return DB_RETCODE_INVALIDATED_CURSOR;
    }

    /* A cursor is guaranteed to span 0 or more records if valid. Note that
     * the range is index based hence the + 1. Also note that last_index is
     * always the last index of a valid record (minimum is 0), or 1 less
     * than first_index if there are no records. Hence, this calculation
     * is guaranteed to return either 0 if there are no records or the
     * actual number of records the cursor spans.
     */
    *count = cursor->last_index - cursor->first_index + 1;

    return DB_RETCODE_OK;
}
#endif

DB_ReturnCode_T
DB_Cursor_get_next(DB_Cursor_T cursor, DB_Record_T *record)
{
    /* DB_Cursor_get_next() is called only after DB_Table_select_* 
     *  has returned a valid cursor.
     *  Thus, the precondition check is defensive/debug only.
     */
    OSAPI_PRECONDITION((cursor==NULL || record==NULL),
                        return DB_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("cursor",cursor,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("record",record,RTI_TRUE);)

    if (cursor->index == NULL)
    {
        return DB_RETCODE_INVALIDATED_CURSOR;
    }

    if (cursor->current_index > cursor->last_index)
    {
        return DB_RETCODE_NO_DATA;
    }

    *record = URTDB_RECORD_TO_USER_RECORD(cursor->index->sorted_array[cursor->current_index].record);
    ++cursor->current_index;

    return DB_RETCODE_OK;
}

/*
 * This function returns a cursor previously returned by one of the
 * DB_Table_select_all or DB_Table_select_range calls to the free cursor pool.
 * A valid cursor is _always_ linked. Thus, if a cursor is linked it must
 * be unlinked and return to the pool of free cursors when done. If it is
 * not linked it is not linked it is not a valid cursor and it cannot
 * be safely returned. There is no mechanism in place to catch this and
 * it is the responsibility of the caller to ensure that DB_Cursor_finish is
 * only called on valid cursors.
 */
void
DB_Cursor_finish(DB_Table_T table,DB_Cursor_T cursor)
{
    RTI_UINT32 i;

    /*  DB_Cursor_finish() is called only after DB_Table_select_*
     *  has returned a valid cursor.
     *  Thus, the precondition check is defensive/debug only.
     */
    OSAPI_PRECONDITION((cursor==NULL || table==NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("cursor",cursor,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("table",table,RTI_TRUE);)

    for (i = 0; i < DB_CURSOR_MAX; ++i)
    {
        if ((cursor == &table->cursors[i]) && !(table->resource_state & (DB_CURSOR_STATE << i)))
        {
            cursor->index = NULL;
            table->resource_state |= (DB_CURSOR_STATE << i);
            table->resource_state &= ~(DB_CURSOR_INUSE << i);
            break;
        }
    }
}

/*ci @} */
