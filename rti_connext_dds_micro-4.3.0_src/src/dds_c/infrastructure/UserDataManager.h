/*
 * FILE: UserDataManager.h - Blob manager implementation definitions
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
 * \brief Blob manager implementation definitions
 */
#ifndef ManagedString_h
#define ManagedString_h
#include "osapi/osapi_config.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "db/db_api.h"
#include "dds_c/dds_c_common.h"
#include "dds_c/dds_c_log.h"
#include "dds_c/dds_c_user_data_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_ManagedUserDataType
{
    DB_Table_T user_data_table;
    RTI_SIZE_T max_user_data_size;
};

struct DDS_UserDataManager
{
    DB_Database_T database;
    struct DDS_ManagedUserDataType *data_types[DDS_USER_DATA_TYPE_COUNT];
};

struct DDS_ManagedUserData
{
    DDS_Octet *value;
    RTI_SIZE_T length;
    RTI_SIZE_T ref_count;
};

#define DDS_ManagedUserData_INITIALIZER \
{\
    NULL,\
    0,\
    0\
}

#ifdef __cplusplus
}
#endif

#endif
