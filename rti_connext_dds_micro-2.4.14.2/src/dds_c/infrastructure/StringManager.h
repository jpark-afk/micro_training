
/*
 * FILE: StringManager.h - String Manager implementation
 *
 * (c) Copyright 2019 Real-Time Innovations,
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

#ifndef RTI_CERT

#ifndef ManagedString_h
#define ManagedString_h
#include "db/db_api.h"
#include "dds_c/dds_c_common.h"
#include "dds_c/dds_c_log.h"
#include "dds_c/dds_c_string_manager.h"

/*ci \brief The string manager
 */
struct DDS_StringManager
{
    /*ci \brief A table of strings
     */
    DB_Table_T string_table;

    /*ci \brief The database to store the table in
     */
    DB_Database_T database;
};

/*ci \brief A managed string
 */
struct DDS_ManagedString
{
    /*ci \brief The string value
     */
    char *value;

    /*ci \brief The number of references to this string
     */
    DDS_UnsignedLong ref_count;
};

/*ci \brief Memory initializer for a managed string
 */
#define DDS_ManagedString_INITIALIZER \
{\
    NULL,\
    0\
}


RTI_PRIVATE RTI_BOOL
DDS_ManagedString_initialize(DDS_ManagedString_T *string_entry, const char* const value);


RTI_PRIVATE void
DDS_ManagedString_finalize(DDS_ManagedString_T *string_entry);


#endif

#endif /* !RTI_CERT */
