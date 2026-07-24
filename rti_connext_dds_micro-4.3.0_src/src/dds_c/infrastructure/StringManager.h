
/*
 * FILE: StringManager.h - String Manager implementation
 *
 * (c) Copyright 2024 Real-Time Innovations,
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
 * may022019 am created
 */
/*ce
 * \file
 * \brief String Manager implementation
 */
#ifndef ManagedString_h
#define ManagedString_h
#include "db/db_api.h"
#include "dds_c/dds_c_common.h"
#include "dds_c/dds_c_log.h"
#include "dds_c/dds_c_string_manager.h"
#include "reda/reda_bufferpool.h"
#include "reda/reda_circularlist.h"

#ifdef __cplusplus
    extern "C" {
#endif

/* Typedef for assert_string function pointer */
typedef char* 
(*DDS_StringManager_Assert)(DDS_StringManager_T* string_manager, const char* value);

/* Typedef for delete_string function pointer */
typedef RTI_BOOL 
(*DDS_StringManager_Delete_String)(DDS_StringManager_T* string_manager, char* value);

/* Typedef for delete function pointer */
typedef RTI_BOOL 
(*DDS_StringManager_Delete)(DDS_StringManager_T* string_manager);

/* Define the struct using the typedefs for function pointers */
typedef struct DDS_StringManagerI 
{
    /*ci
     * \brief Add a string or return an existing an string
     */
    DDS_StringManager_Assert assert_string;

    /*ci
     * \brief Delete a string
     */
    DDS_StringManager_Delete_String delete_string;

#ifndef RTI_CERT
    /*ci
     * \brief Delete the string manager
     */
    DDS_StringManager_Delete delete;

#endif /* RTI_CERT */

} DDS_StringManagerI;

struct DDS_StringManager
{
    /*ci
     * \brief String manager interface
     */
     DDS_StringManagerI intf;
};

struct  DDS_StringManager_dynamic
{
    DDS_StringManager_T _parent;

    /*ci
        * \brief List
        */
    REDA_CircularList_T string_list;
};

struct DDS_StringManager_fixed
{
    DDS_StringManager_T _parent;

    /*ci
     * \brief max size of a single string when in DDS_LENGTH_AUTO mode
     */
    RTI_INT32 max_string_size;

    /*ci
     * \brief size buffer/pool to preallocate/allocate
     */
    RTI_SIZE_T memory_allocation;

    /*ci
     * \brief the table of records
     */
    DB_Table_T string_table;

    /*ci
     * \brief database for the string_table
     */
    DB_Database_T database;
};

struct  DDS_StringManager_flexible
{
    DDS_StringManager_T _parent;

    /*ci
     * \brief max size of a single string when in DDS_LENGTH_AUTO mode
     */
    RTI_SIZE_T max_string_size;

    /*ci
     * \brief size buffer/pool to preallocate/allocate
     */
    RTI_SIZE_T memory_allocation;

    /*ci
     * \brief the table of records
     */
    DB_Table_T string_table;

    /*ci
     * \brief currently stored length
     */
    RTI_SIZE_T buffer_length;

    /*ci
     * \brief database for the string_table
     */
    DB_Database_T database;

    /*ci
     * \brief buffer
     */
    char *buffer;
};

struct DDS_ManagedString
{
    char *value;
    DDS_UnsignedLong ref_count;
};

#define DDS_ManagedString_INITIALIZER \
{\
    NULL,\
    0\
}

struct DDS_ManagedStringNode
{
    REDA_CircularListNode_T _node;
    char *value;
    DDS_UnsignedLong ref_count;
};

#define DDS_ManagedStringList_INITIALIZER \
{\
    NULL,\
    0,\
    REDA_CircularList_INITIALIZER\
}

#define DDS_ManagedStringList_get_first(l_) \
   (struct DDS_ManagedStringNode*)REDA_CircularList_get_first((l_))

#define DDS_ManagedStringList_get_next(c_) \
   (struct DDS_ManagedStringNode*)REDA_CircularListNode_get_next((c_))

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ManagedString_h */
