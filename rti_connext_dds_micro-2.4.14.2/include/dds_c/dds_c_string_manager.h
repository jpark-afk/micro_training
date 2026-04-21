
/*
 * FILE: dds_c_string_manager.h - DDS string Manager definitions
 *
 * (c) Copyright, Real-Time Innovations, 2019-2019
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief DDS string  Manager definitions
 */

#ifndef RTI_CERT

#ifndef dds_c_string_manager_h
#define dds_c_string_manager_h
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#include "db/db_api.h"

/* abstract type of string manager*/
struct DDS_StringManager;

typedef struct DDS_StringManager DDS_StringManager_T;

struct DDS_ManagedString;

typedef struct DDS_ManagedString DDS_ManagedString_T;

struct DDS_StringManagerProperty
{
    RTI_SIZE_T max_strings;
    
    DB_Database_T database;
    
};

#define DDS_StringManagerProperty_INITIALIZER \
{\
    0,\
    NULL\
}

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL 
DDS_StringManager_create(DDS_StringManager_T **string_manager, 
                                    struct DDS_StringManagerProperty *property);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_StringManager_delete(DDS_StringManager_T *string_manager);

MUST_CHECK_RETURN DDSCDllExport char* 
DDS_StringManager_assert_string(DDS_StringManager_T* string_manager, 
                                      const char* const value);

SHOULD_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_StringManager_delete_string(DDS_StringManager_T *string_manager, char *value);


#endif

#endif /* !RTI_CERT */
