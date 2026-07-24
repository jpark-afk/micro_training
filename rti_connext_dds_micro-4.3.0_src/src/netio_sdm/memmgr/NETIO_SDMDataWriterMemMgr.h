/*
 * FILE: NETIO_SDMDataWriterMemMgr.h - DataWriter memory manager
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef NETIO_SDMDataWriterMemMgr_h
#define NETIO_SDMDataWriterMemMgr_h

#include "osapi/osapi_types.h"
#include "netio_sdm/netio_sdm.h"

#include "NETIO_SDMBitmap.h"
#include "NETIO_SDMDataWriterMemMgr.h"
#include "NETIO_SDMMemPool.h"

/******************************************************************************
 *                             NETIO_SDMUserData
 ******************************************************************************/
struct SDM_UserData
{
    struct SDM_Bitmap key_bitmap;
    RTI_INT32 ref_count;
};

struct SDM_UserData*
SDM_UserData_new(RTI_UINT32 num_segment_keys);

void
SDM_UserData_increment_ref_count(struct SDM_UserData *user_data);

void
SDM_UserData_decrement_ref_count(struct SDM_UserData *user_data);

RTI_INT32
SDM_UserData_get_ref_count(struct SDM_UserData *user_data);

struct SDM_Bitmap*
SDM_UserData_get_key_bitmap(struct SDM_UserData *user_data);

void
SDM_UserData_delete(struct SDM_UserData *user_data);

/******************************************************************************
 *                             SDM_MemSegment
 ******************************************************************************/
struct SDM_MemSegment
{
    /*
     * This structure will be added to a circular list so the first member
     * must be a circular list node
     */
    struct REDA_CircularListNode list_node;
    RTI_INT32 shm_key;
    void *buffer_start_address;
    void *buffer_end_address;
    RTI_UINT32 requested_size;
    RTI_UINT32 allocated_size;
    RTI_UINT32 buffer_state_size;
    RTI_INT32 ref_count;
    struct SDM_MemSegmentSharedSegmentEpoch shm_segment_epoch;
    struct NETIO_SharedMemorySegmentHandle shm_handle;
    struct SDM_MemSegmentShared *shared_hdr;
    RTIBool is_pending_detachment;
};

struct SDM_MemSegmentProperty
{
    RTI_UINT32 buffer_size;
    RTI_UINT32 buffer_count;
    RTI_UINT32 buffer_alignment;
};

#define SDM_MEMSEGMENT_DEFAULT_PROPERTY {  \
        0,                /*buffer size */ \
        0,               /*buffer count */ \
        8,           /*buffer_alignment */ \
}

/******************************************************************************
 *                             DataWriterShmMgr
 ******************************************************************************/

typedef enum
{
    SDM_DWSHMMGR_RETCODE_SUCCESS,

    SDM_DWSHMMGR_RETCODE_USER_TYPE_TOO_BIG,

    SDM_DWSHMMGR_RETCODE_BUFFER_NEEDED_TOO_BIG

} SDM_DWShmMgr_Retcode_T;

struct DataWriterShmMgrProperty
{
    RTI_INT32 base_key;
    RTI_INT32 domain_id;
    RTI_INT32 domain_gain;
    RTI_INT32 participant_index;
    RTI_INT32 participant_gain;

    /* Only relevant to MetpMempool within */
    struct NETIO_Guid datawriter_owner_guid;
    RTI_UINT32 max_user_sample_buffers;
    RTI_UINT32 user_sample_buffer_max_size;
    struct SDM_Bitmap *key_bitmap;
};

#define SDM_DataWriterShmMgrProp_INITALIZER {                                  \
        0x8000000,                                              /* Base Key */ \
        0,                                                     /* Domain Id */ \
        10000,                                               /* domain_gain */ \
        0,                                             /* participant_index */ \
        500,                                            /* participant_gain */ \
        NETIO_ADDRESS_GUID_UNKNOWN,                              /* dw guid */ \
        1,                                                /* sample buffers */ \
        0,                                              /* sammple_max_size */ \
        NULL                                                  /* key_bitmap */ \
}

struct SDM_MemBufferFreeNode
{
    struct REDA_CircularListNode _node;
    void *address;
};

struct DataWriterShmMgr
{
    RTI_INT32 base_shm_key;
    struct NETIO_Guid datawriter_guid;
    struct SDM_MemPool *mem_pool;
    struct SDM_Bitmap *key_bitmap;
};

RTI_BOOL
DataWriterShmMgr_initialize(
        struct DataWriterShmMgr *self,
        struct DataWriterShmMgrProperty *mem_admin_property);

struct SDM_MemSegment*
DataWriterShmMgr_new_segment(
        struct DataWriterShmMgr *self,
        SDM_DWShmMgr_Retcode_T *status_out,
        struct SDM_MemSegmentProperty *property);

RTI_BOOL
DataWriterShmMgr_finalize(struct DataWriterShmMgr *self);

NETIO_SDMDllExport RTI_BOOL
DataWriterShmMgr_decrement_reference(
        struct DataWriterShmMgr *self,
        void *address);

MUST_CHECK_RETURN struct SDM_MemSegment*
DataWriterShmMgr_is_buffer_from_here(
        struct DataWriterShmMgr *self,
        const void *buffer);

MUST_CHECK_RETURN NETIO_SDMDllExport void*
DataWriterShmMgr_allocate_buffer(struct DataWriterShmMgr *self);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
DataWriterShmMgr_move_buffer_from_free_list_to_in_use(
        struct DataWriterShmMgr *self,
        const void *address);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
DataWriterShmMgr_free_buffer(
        struct DataWriterShmMgr *self,
        const void *address,
        RTIBool removed);

MUST_CHECK_RETURN NETIO_SDMDllExport struct DataWriterShmMgr*
DataWriterShmMgr_new(struct DataWriterShmMgrProperty *property);

RTI_BOOL
DataWriterShmMgr_delete(struct DataWriterShmMgr *self);

RTIBool
DataWriterShmMgr_delete_segment(
        struct DataWriterShmMgr *self,
        struct SDM_MemSegment *segment);

#endif /* NETIO_SDMDataWriterMemMgr_h */
