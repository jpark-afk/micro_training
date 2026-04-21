/*
 * FILE: xcdr_heapmgr.h
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef xcdr_heapmanager_h
#define xcdr_heapmanager_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif
#ifndef reda_buffer_pool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*XCDR_HeapMgr_gen_init)(void *buffer_to_initialize)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*XCDR_HeapMgr_gen_finalize)(void *buffer_to_finalize)
)

/* Forward declare XCDR_HeapMgr. Its internals will be unknown
 * outside of this class
 */
struct XCDR_HeapMgr;

struct XCDR_HeapMgrProperty
{
    RTI_SIZE_T buffer_size;
    RTI_SIZE_T max_buffers;
    XCDR_HeapMgr_gen_init initialize_func;
    XCDR_HeapMgr_gen_init finalize_func;
};

REDADllExport void*
XCDR_HeapMgr_allocate_buffer(struct XCDR_HeapMgr *mngr);

REDADllExport RTI_BOOL
XCDR_HeapMgr_return_buffer(struct XCDR_HeapMgr *mngr, void *sample);

REDADllExport struct XCDR_HeapMgr*
XCDR_HeapMgr_new(struct XCDR_HeapMgrProperty *mngr_prop);

REDADllExport RTI_BOOL
XCDR_HeapMgr_is_buffer_in_use(
        struct XCDR_HeapMgr *mngr,
        const void* sample,
        RTI_BOOL *in_use);

REDADllExport RTI_BOOL
XCDR_HeapMgr_revert_sample_state(struct XCDR_HeapMgr *mngr, const void* sample);

REDADllExport RTI_BOOL
XCDR_HeapMgr_set_buffer_in_use(struct XCDR_HeapMgr *mngr, const void* sample);

REDADllExport RTI_BOOL
XCDR_HeapMgr_set_buffer_not_in_use(struct XCDR_HeapMgr *mngr, const void* sample);

REDADllExport RTI_BOOL
XCDR_HeapMgr_delete(struct XCDR_HeapMgr *mngr_prop);

REDADllExport RTI_BOOL
XCDR_HeapMgr_is_owner(struct XCDR_HeapMgr *mngr, const void *sample);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* xcdr_heapmanager_h */
