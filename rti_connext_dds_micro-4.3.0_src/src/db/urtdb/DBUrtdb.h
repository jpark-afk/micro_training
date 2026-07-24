/*
 * FILE: DBUrtdb.h - DB Implementation
 *
 * (c) Copyright, Real-Time Innovations, 2012-2025.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 28may2012,tk Written
 */

/*ci \addtogroup DBModule
 *  @{
 */

#ifndef DBUrtdb_h
#define DBUrtdb_h

#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif

/*ci \brief A record is a contiguous, fixed size block of memory structure:
 *
 * \verbatim
 * +------------+
 * |   header   | struct URTDB_RecordHeader
 * +------------+
 * |            |
 * |  user-part | void*
 * |            |
 * |            |
 * |            |
 * +------------+
 * \endverbatim
 */
typedef struct URTDB_RecordHeader
{
    /* ptr ensure that the header is aligned to a ptr which is a assumed
     * by all higher level users of a database record.
     */
    union
    {
        void *ptr;
        RTI_UINT32 state;
    } header;
} URTDB_RecordHeader;

/*ci \brief A record is not used
 */
#define URTDB_RECORD_STATE_ALLOCATED 1

/*ci \brief A record is inserted
 */
#define URTDB_RECORD_STATE_INSERTED  2

/*ci \brief A record is removed
 */
#define URTDB_RECORD_STATE_REMOVED   3

/*ci \brief ReturnTRUE if the record is allocated
 */
#define  URTDB_Record_is_allocated(r_) \
    ((r_)->header.state == URTDB_RECORD_STATE_ALLOCATED)

/*ci \brief ReturnTRUE if the record is removed
 */
#define  URTDB_Record_is_removed(r_) \
    ((r_)->header.state == URTDB_RECORD_STATE_REMOVED)

/*ci \brief ReturnTRUE if the record is inserted
 */
#define  URTDB_Record_is_inserted(r_) \
    ((r_)->header.state == URTDB_RECORD_STATE_INSERTED)

/*ci \brief Set the record state to inserted
 */
#define  URTDB_Record_set_inserted(r_) \
    (r_)->header.state = URTDB_RECORD_STATE_INSERTED

/*ci \brief Set the record state to allocated
 */
#define  URTDB_Record_set_allocated(r_) \
    (r_)->header.state = URTDB_RECORD_STATE_ALLOCATED

/*ci \brief Set the record state to removed
 */
#define  URTDB_Record_set_removed(r_) \
    (r_)->header.state = URTDB_RECORD_STATE_REMOVED

/*ci
 * \brief Concrete implementation of the abstract DB_Cursor type
 */
struct DB_Cursor
{
    /*ci
     * \brief Index of the first record in the result set
     */
    RTI_INT32 first_index;

    /*ci
     * \brief The index of the current record the cursor is pointing to
     */
    RTI_INT32 current_index;

    /*ci
     * \brief Index of the last record in the result set
     */
    RTI_INT32 last_index;

    /*ci
     * \brief The index the cursor was created from
     */
    struct DB_Index *index;
};

/*ci
 * \brief An element in an index holding a pointer to a user-record
 */
struct URTDB_IndexEntry
{
    /*ci
     * \brief Reference to a record header
     */
    URTDB_RecordHeader *record;
};

/*ci
 * \brief Concrete implementation of the DB_Index type
 */
struct DB_Index
{
    /*ci
     * \brief Compare function for this index
     */
    DB_IndexCompare_T compare;

    /*ci
     * \brief Sorted array of references to table records
     */
    struct URTDB_IndexEntry *sorted_array;

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     * \brief The index property passed in the \ref DB_Table_create_index call
     */
    struct DB_IndexProperty property;
#endif

    /*ci
     * \brief The number of records currently in this index
     */
    RTI_INT32 record_count;

    /*ci
     * \brief The index into the index array of the last changed index. Used
     * to determine if a cursor is invalidated if an index is updated while
     * the cursor is open.
     */
    RTI_INT32 last_changed_index;
};


/*ci
 * \brief Concrete Implementation of the user constructor saved by the DB_table
 */
struct DB_TableRowCtor
{
    DB_Table_initializeFunc_T ctor;
    void *user_param;
};

/*ci
 * \brief Concrete Implementation of the user destructor saved by the DB_table
 */
struct DB_TableRowDtor
{
    DB_Table_finalizeFunc_T dtor;
    void *user_param;
};

struct DB_TableRowCtorDtor
{
    /*ci
     *\brief Place to save the constructor and constructor params provided
     * provided by the user
     */
    struct DB_TableRowCtor user_ctor;

    /*ci
     *\brief PLace to save the destructor and destructor params provided
     * provided by the user
     */
    struct DB_TableRowDtor user_dtor;
};

/*ci
 * \brief Concrete implementation of the DB_Table type
 */
struct DB_Table
{
    /*ci
     *\brief Used to link together tables in use
     */
    REDA_CircularListNode_T node;

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     *\brief Unique name of the table within the database
     */
    char *name;
#endif

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     *\brief The properties passed to \ref DB_Database_create_table
     */
    struct DB_TableProperty property;
#else
    RTI_SIZE_T max_records;
#endif

    /*ci
     *\brief Pool of fixed sized records
     */
    REDA_BufferPool_T records;

    /*ci
     *\brief List of records currently in use
     */
    RTI_UINT32 records_in_use;

    /*ci
     * \brief Resource state
     *
     * \details
     * Max 8 indices, 8 cursors
     * 31-16 - Indices 1 bit allocation 1 bit for state
     * 15-0  - Cursors
     */
    RTI_UINT32 resource_state;

    /*ci
     * \brief Array of indices for the table
     */
    struct DB_Index *indices;

    /*ci
     * \brief Array of cursors for the table
     */
    struct DB_Cursor *cursors;

    /*ci
     *\brief Default index created based on the compare function passed to
     *       \ref DB_Database_create_table
     */
    struct DB_Index default_index;

    /*ci
     * \brief Optional storage for constructors/destructors
     */
    struct DB_TableRowCtorDtor *ctordtor;
};

/*ci
 * \brief Concrete implementation of DB_Database type
 */
struct DB_Database
{
    REDA_CircularList_T tables;

    /*ci
     *\brief Database lock based on \ref DB_LockLevel_T
     */
    OSAPI_Mutex_T *lock;

    /*ci
     * \brief The locking mode used for the database
     */
    DB_LockLevel_T lock_mode;
};

#endif

/*ci @} */
