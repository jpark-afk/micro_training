/*
 * FILE: db_log.h - DB Log definitions
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
 * 28may2012,tk Written
 */
/*e
 * \file
 * \brief DB module log codes
 *
 * \details
 * This file defines log codes for all messages logged by the database
 * modules. All the log codes in this file are publicly documented.
 */
#ifndef db_log_h
#define db_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup DBLogCodesClass DB
 * \brief Database. ModuleID = 2
 * \ingroup LoggingModule
 */

/*e
 * \brief Not sufficient memory to allocate sorted list for an index
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_SORTED_ALLOC_EC                                 (DB_LOG_BASE + 1)
#define DB_LOG_SORTED_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_SORTED_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief The specified database name is too long
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_NAME_TOO_LONG_EC                                (DB_LOG_BASE + 2)
#define DB_LOG_NAME_TOO_LONG(level_,len_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DB_LOG_NAME_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"length",(len_))

/*e
 * \brief Illegal table size specified
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ILLEGAL_TABLE_SIZE_EC                           (DB_LOG_BASE + 3)
#define DB_LOG_ILLEGAL_TABLE_SIZE(level_,size_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DB_LOG_ILLEGAL_TABLE_SIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"size",(size_))

/*e
 * \brief Illegal combination of lock mode and mutex given
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ILLEGAL_LOCK_MODE_EC                            (DB_LOG_BASE + 4)
#define DB_LOG_ILLEGAL_LOCK_MODE(level_,mode_,shared_) \
OSAPI_LOG_ENTRY_CREATE((level_),DB_LOG_ILLEGAL_LOCK_MODE_EC,\
                        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("mode",(mode_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_POINTER("shared",(shared_),RTI_TRUE)

/*e
 * \brief Failed to allocate database mutex
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_MUTEX_ALLOC_EC                                  (DB_LOG_BASE + 5)
#define DB_LOG_MUTEX_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_MUTEX_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate buffer pool
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ALLOC_TABLE_POOL_EC                             (DB_LOG_BASE + 6)
#define DB_LOG_ALLOC_TABLE_POOL(level_,size_,count_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DB_LOG_ALLOC_TABLE_POOL_EC,\
                   OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
                   "size",(RTI_INT32)(size_),"count",(RTI_INT32)(count_))

/*e
 * \brief Table is in use
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_TABLES_INUSE_EC                                 (DB_LOG_BASE + 7)
#define DB_LOG_TABLES_INUSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_TABLES_INUSE_EC,\
                        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Specified table name too long
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_TABLE_NAME_TOO_LONG_EC                          (DB_LOG_BASE + 8)
#define DB_LOG_TABLE_NAME_TOO_LONG(level_,table_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DB_LOG_TABLE_NAME_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"table",(table_))

/*e
 * \brief Illegal record count specified
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ILLEGAL_RECORD_COUNT_EC                         (DB_LOG_BASE + 9)
#define DB_LOG_ILLEGAL_RECORD_COUNT(level_,count_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DB_LOG_ILLEGAL_RECORD_COUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"count",(count_))

/*e
 * \brief Specified table already exists
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_TABLE_EXISTS_EC                                (DB_LOG_BASE + 10)
#define DB_LOG_TABLE_EXISTS(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DB_LOG_TABLE_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name=",(name_))

/*e
 * \brief Table resources exceeded
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_OUT_OF_TABLES_EC                               (DB_LOG_BASE + 11)
#define DB_LOG_OUT_OF_TABLES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_OUT_OF_TABLES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Table record resources exceeded
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_OUT_OF_RECORDS_EC                              (DB_LOG_BASE + 12)
#define DB_LOG_OUT_OF_RECORDS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_OUT_OF_RECORDS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Table index resources exceeded
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_OUT_OF_INDICES_EC                              (DB_LOG_BASE + 13)
#define DB_LOG_OUT_OF_INDICES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_OUT_OF_INDICES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Table cursor resources exceeded
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_OUT_OF_CURSORS_EC                              (DB_LOG_BASE + 14)
#define DB_LOG_OUT_OF_CURSORS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_OUT_OF_CURSORS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot delete table, records are still in table
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_RECORDS_INUSE_EC                               (DB_LOG_BASE + 15)
#define DB_LOG_RECORDS_INUSE(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING(level_,DB_LOG_RECORDS_INUSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"table",(name_))

/*e
 * \brief Cannot delete table, cursors are still in use
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_CURSORS_INUSE_EC                               (DB_LOG_BASE + 16)
#define DB_LOG_CURSORS_INUSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_CURSORS_INUSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot delete table, indices are still in use
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_INDEX_INUSE_EC                                 (DB_LOG_BASE + 17)
#define DB_LOG_INDEX_INUSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_INDEX_INUSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate cursor buffer pool
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ALLOC_CURSOR_POOL_EC                           (DB_LOG_BASE + 18)
#define DB_LOG_ALLOC_CURSOR_POOL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_ALLOC_CURSOR_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate index buffer pool
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ALLOC_INDEX_POOL_EC                            (DB_LOG_BASE + 19)
#define DB_LOG_ALLOC_INDEX_POOL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_ALLOC_INDEX_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate record buffer pool
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ALLOC_RECORD_POOL_EC                           (DB_LOG_BASE + 20)
#define DB_LOG_ALLOC_RECORD_POOL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_ALLOC_RECORD_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate database
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_ALLOC_DATABASE_EC                              (DB_LOG_BASE + 21)
#define DB_LOG_ALLOC_DATABASE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_ALLOC_DATABASE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief An attempt was made to delete a table not in use
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_TABLE_NOT_INUSE_EC                             (DB_LOG_BASE + 22)
#define DB_LOG_TABLE_NOT_INUSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_TABLE_NOT_INUSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief An attempt was made to insert an already existing record
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_RECORD_ALREADY_EXISTS_EC                       (DB_LOG_BASE + 23)
#define DB_LOG_RECORD_ALREADY_EXISTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_RECORD_ALREADY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief An attempt was made to delete a record that does not exist
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_RECORD_DOES_NOT_EXIST_EC                       (DB_LOG_BASE + 24)
#define DB_LOG_RECORD_DOES_NOT_EXIST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_RECORD_DOES_NOT_EXIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An attempt was made to use a cursor that has been invalidated
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_CURSOR_INVALIDATED_EC                          (DB_LOG_BASE + 25)
#define DB_LOG_CURSOR_INVALIDATED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_CURSOR_INVALIDATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to lock the database
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_LOCK_FAILURE_EC                                (DB_LOG_BASE + 26)
#define DB_LOG_LOCK_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_LOCK_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unlock the database
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_UNLOCK_FAILURE_EC                              (DB_LOG_BASE + 27)
#define DB_LOG_UNLOCK_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_UNLOCK_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Constructor/Destructor memory could not be allocated
 * \ingroup DBLogCodesClass
 */
#define DB_LOG_OUT_OF_CTORDTOR_EC                             (DB_LOG_BASE + 28)
#define DB_LOG_OUT_OF_CTORDTOR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DB_LOG_OUT_OF_CTORDTOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif /* db_log_h */
