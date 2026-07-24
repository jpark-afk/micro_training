/*
 * FILE: REDAIndexer.c - Indexer implementation
 *
 * Copyright 2012-2015 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 02feb2015,tk MICRO-1044/PR#13561 Set correct error-level
 * 01dec2014,tk MICRO-962/PR#12376  Removed redundant code
 * 31jul2014,tk MICRO-172/PR#1064   Removed superfluous REDA_Indexer fields
 * 22apr2014,tk MICRO-173/PR#1065   Removed unused fields from binsearch
 * 12jun2013,tk MICRO-345: fixed
 * 25oct2012,tk Written
 */
/*ce
 * \file
 * \brief Implementation of the REDA Indexer API
 */
/*ci \addtogroup REDAIndexerClass
 * @{
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_index_h
#include "reda/reda_indexer.h"
#endif

#include "REDAIndexer.h"

/*ci
 * \brief Valid search options for \ref REDA_Indexer_find_entry_w_opcode
 */
typedef enum
{
    /*ci
     * \brief seach for something <=
     */
    REDA_INDEXER_OPCODE_EQ_OR_LT,

    /*ci
     * \brief seach for something >=
     */
    REDA_INDEXER_OPCODE_EQ_OR_GT
} REDA_IndexerOpCode_T;

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Return number of elements in the index
 *
 * \param[in] indexer Indexer to return number of elements for
 */
RTI_PRIVATE RTI_INT32
REDA_IndexerImpl_get_count(const struct REDA_Indexer *const indexer)
{
    return indexer->high_index + 1;
}

/*ci
 *
 * \brief Search in the REDA indexer
 *
 * \details
 *
 * Find a entry among the indexed elements based on the key
 *
 * \param[in] indexer       Index to search in
 * \param[in] key_is_record Whether the key parameter is record or key structure
 * \param[in] key_lower     key or record to compare against
 * \param[in] result        Index where the key would have been if it did exist
 *
 * \return RTI_TRUE is the entry exists, RTI_FALSE is not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
REDA_Indexer_binsearch(struct REDA_Indexer *const indexer,
                       RTI_BOOL key_is_record,
                       const void *const key_lower,
                       RTI_INT32 *const result)
{
    RTI_INT32 mid,cmp_result,cand_index;
    RTI_BOOL exists = RTI_FALSE;
    RTI_INT32 lower = 0;
    RTI_INT32 upper = indexer->high_index;

    cand_index = 0x7fffffffL;

    while (lower <= upper)
    {
        mid = lower + (upper-lower)/2;
        cmp_result = indexer->compare(indexer->elements[mid],key_is_record,key_lower);
        if (!cmp_result)
        {
            cand_index = mid;
            exists = RTI_TRUE;
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
        *result = lower;
    }
    else
    {
        *result = cand_index;
    }

    return exists;
}

/*******************************************************************************
 *                                  PUBLIC API
 ******************************************************************************/
REDA_Indexer_T*
REDA_Indexer_new(REDA_Indexer_compare_T compare,
                 struct REDA_IndexerProperty *property)
{
    struct REDA_Indexer *retval = NULL;
    struct REDA_Indexer *indexer = NULL;
    RTI_INT32 i;

    OSAPI_Heap_allocate_struct(&indexer,struct REDA_Indexer);
    if (indexer == NULL)
    {
        goto done;
    }

    indexer->property = *property;
    indexer->compare = compare;
    indexer->high_index = -1;
    indexer->iterator.current_index = -1;
    indexer->iterator.indexer = indexer;

     OSAPI_Heap_allocate_array(&indexer->elements,
                               (RTI_UINT32)indexer->property.max_entries,
                               void*);
     if (indexer->elements == NULL)
     {
         OSAPI_Heap_free_struct(indexer);
         goto done;
     }

     for (i = 0; i < indexer->property.max_entries; ++i)
     {
         indexer->elements[i] = NULL;
     }

     retval = indexer;

done:
    return retval;
}

#ifndef RTI_CERT
RTI_BOOL
REDA_Indexer_delete(REDA_Indexer_T *indexer)
{
    if (indexer->elements != NULL)
    {
        OSAPI_Heap_free_array(indexer->elements);
        indexer->elements = NULL;
    }

    OSAPI_Heap_free_struct(indexer);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
REDA_Indexer_add_entry(REDA_Indexer_T *indexer,void *entry)
{
    RTI_BOOL exists;
    RTI_INT32 new_index;

    if (indexer->high_index == -1)
    {
        indexer->elements[0] = entry;
        indexer->high_index = 0;
        return RTI_TRUE;
    }

    if (REDA_IndexerImpl_get_count(indexer) == indexer->property.max_entries)
    {
        REDA_LOG_INDEX_FULL(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    exists = REDA_Indexer_binsearch(indexer,RTI_TRUE,entry,&new_index);
    if (exists)
    {
        REDA_LOG_INDEX_ENTRY_EXISTS(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (new_index <= indexer->high_index)
    {
        RTI_SIZE_T count = (RTI_SIZE_T)sizeof(void*) *
                           (RTI_SIZE_T)(indexer->high_index - new_index + 1);

        OSAPI_Memory_move(&indexer->elements[new_index+1],
                          &indexer->elements[new_index],
                          count);
    }

    ++indexer->high_index;

    indexer->elements[new_index] = entry;

    return RTI_TRUE;
}

void*
REDA_Indexer_remove_entry(REDA_Indexer_T *indexer,const void *const key)
{
    RTI_BOOL exists;
    RTI_INT32 entry_index;
    void *entry;

    if (indexer->high_index == -1)
    {
        return NULL;
    }

    exists = REDA_Indexer_binsearch(indexer,RTI_FALSE,key,&entry_index);
    if (!exists)
    {
        return NULL;
    }

    entry = indexer->elements[entry_index];
    if (entry_index == indexer->high_index)
    {
        indexer->elements[entry_index] = NULL;
    }
    else
    {
        RTI_SIZE_T count = (RTI_SIZE_T)sizeof(void*) *
                           (RTI_SIZE_T)(indexer->high_index - entry_index);

        OSAPI_Memory_move(&indexer->elements[entry_index],
                          &indexer->elements[entry_index+1],
                          count);
    }

    --indexer->high_index;

    if (indexer->iterator.current_index == entry_index)
    {
        --indexer->iterator.current_index;
    }

    return entry;
}

/*ci
 *
 * \brief Search in the REDA indexer based on an operation
 *
 * \details
 *
 * Find a entry among the indexed elements based on the key
 *
 * \param[in] indexer  Index to search in
 * \param[in] key      Search key
 * \param[in] opcode   Seach condition
 *
 *
 * \return entry that matches or NULL if no matches exists
 */
RTI_PRIVATE void*
REDA_Indexer_find_entry_w_opcode(REDA_Indexer_T *indexer,const void *const key,
                                 REDA_IndexerOpCode_T opcode)
{
    RTI_INT32 mid,cmp_result,cand_index;
    RTI_INT32 lower = 0;
    RTI_INT32 upper = indexer->high_index;

    cand_index = -1;

    while (lower <= upper)
    {
        mid = lower + (upper-lower)/2;
        cmp_result = indexer->compare(indexer->elements[mid],RTI_FALSE,key);
        if (!cmp_result)
        {
            return indexer->elements[mid];
        }
        else if (cmp_result < 0)
        {
            if (opcode == REDA_INDEXER_OPCODE_EQ_OR_LT)
            {
                cand_index = mid;
            }
            lower = mid+1;
        }
        else
        {
            if (opcode == REDA_INDEXER_OPCODE_EQ_OR_GT)
            {
                cand_index = mid;
            }
            upper = mid-1;
        }
    }

    if (cand_index != -1)
    {
        return indexer->elements[cand_index];
    }
    else
    {
        return NULL;
    }
}

void*
REDA_Indexer_find_entry_eq_or_gt(REDA_Indexer_T *indexer,const void *const key)
{
    return REDA_Indexer_find_entry_w_opcode(
            indexer,key,REDA_INDEXER_OPCODE_EQ_OR_GT);
}

void*
REDA_Indexer_find_entry_eq_or_lt(REDA_Indexer_T *indexer,const void *const key)
{
    return REDA_Indexer_find_entry_w_opcode(
            indexer,key,REDA_INDEXER_OPCODE_EQ_OR_LT);
}

void*
REDA_Indexer_find_entry(REDA_Indexer_T *indexer,const void *const key)
{
    RTI_BOOL exists;
    RTI_INT32 entry_index;

    exists = REDA_Indexer_binsearch(indexer,RTI_FALSE,key,&entry_index);
    if (!exists)
    {
        return NULL;
    }

    return indexer->elements[entry_index];
}

RTI_INT32
REDA_Indexer_get_count(REDA_Indexer_T *indexer)
{
    if (indexer->high_index == -1)
    {
        return 0;
    }

    return REDA_IndexerImpl_get_count(indexer);
}

void*
REDA_Indexer_get_entry(REDA_Indexer_T *indexer,RTI_INT32 index)
{
    return indexer->elements[index];
}

void*
REDA_Indexer_get_first_entry(REDA_Indexer_T *indexer)
{
    if (indexer->high_index == -1)
    {
        return NULL;
    }

    return indexer->elements[0];
}

void*
REDA_Indexer_get_last_entry(REDA_Indexer_T *indexer)
{
    if (indexer->high_index == -1)
    {
        return NULL;
    }

    return indexer->elements[indexer->high_index];
}

REDA_IndexIterator_T*
REDA_Indexer_iterator_begin(REDA_Indexer_T *indexer)
{
    indexer->iterator.current_index = -1;

    return &indexer->iterator;
}

void*
REDA_Indexer_iterator_next(REDA_IndexIterator_T *iterator)
{
    ++iterator->current_index;

    if (iterator->current_index > iterator->indexer->high_index)
    {
        return NULL;
    }

    return iterator->indexer->elements[iterator->current_index];
}


/*ci @} */
