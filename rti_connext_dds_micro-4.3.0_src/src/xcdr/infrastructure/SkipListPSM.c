/*
 * FILE: SkipListPSM.c - XCDR SkipList implementation
 *
 * Copyright 2023-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "reda/reda_indexer.h"
#include "reda/reda_bufferpool.h"
#include "reda/reda_circularlist.h"
#include "Infrastructure.h"
#include "SkipList.h"

/*ci \brief Implementation of the abstract skiplist used by XCDR
 */
struct RTIXCdrSkipList
{
    /*ci \brief indexer to sort the content
     */
    REDA_Indexer_T *indexer;

    /*ci \brief pool of skiplist nodes
     */
    REDA_BufferPool_T pool;

    /*ci \brief list to traverse the skiplist back and forth
     */
    REDA_CircularListNode_T list;

    /*ci \brief User supplied sort function
     */
    RTIXCdrSkipListDataTypeCompareFunction cmp_func;
};

/*ci \brief Abstract skiplist node
 */
struct RTIXCdrSkipListNode
{
    /*ci \brief list node
     */
    REDA_CircularListNode_T _node;

    /*ci \brief Reference to which the skiplist node belongs
     */
    const struct RTIXCdrSkipList *skip_list;

    /*ci \brief The user data compared by the skiplist's cmp_func function
     */
    void *user_data;
};

/*ci \brief Indirect compare function to compare user-data in the Skiplist.
 *
 * \param[in] record - An existing node, left side
 * \param[in] key_is_record - TRUE if the key parameter is a full record
 * \param[in] key - The key to compare the record against
 *
 * \return 1 if left > right, 0 is left == right, -1 if left < right
 */
RTI_PRIVATE RTI_INT32
RTIXCdrSkipList_indexer_compare(const void *const record,
                                RTI_BOOL key_is_record,
                                const void *const key)
{
    struct RTIXCdrSkipListNode *left = (struct RTIXCdrSkipListNode*)record;
    struct RTIXCdrSkipListNode *right = (struct RTIXCdrSkipListNode*)key;
    UNUSED_ARG(key_is_record);

    return left->skip_list->cmp_func(left->user_data,right->user_data);
}

/*ci \brief Return the user data of a skiplist node
 *
 * \param[in] self - Skiplist node to return user data from. Must not be NULL.
 *
 * \return User-data pointer in skiplist node
 */
void*
RTIXCdrSkipListNode_getElement(const struct RTIXCdrSkipListNode *self)
{
    return self->user_data;
}


/*ci \brief Return the first node in a skiplist, NULL if the list is empty.
 *
 * \param[in] self - Skiplist to first node from. Must not be NULL.
 *
 * \return First node in skiplist node, NULL if the list is empty
 */
const struct RTIXCdrSkipListNode*
RTIXCdrSkipList_getFirstNode(const struct RTIXCdrSkipList *self)
{
    if (self->list._next == self->list._prev)
    {
        return NULL;
    }

    return (const struct RTIXCdrSkipListNode*)self->list._next;
}

/*ci \brief Given a skiplist node, return the next node or NULL if the node is
 *          the last node.
 *
 * \param[in] self - The skiplist. Must not be NULL.
 * \param[in] prev_node - The node to return the next from. Must not be NULL.
 *
 * \return The node after prev_node, NULL if prev_node is the last node.
 */
const struct RTIXCdrSkipListNode*
RTIXCdrSkipList_getNextNode(const struct RTIXCdrSkipList *self,
                            const struct RTIXCdrSkipListNode *prev_node)
{
    if (prev_node->_node._next == &self->list)
    {
        return NULL;
    }

    return (struct RTIXCdrSkipListNode*)prev_node->_node._next;
}

/*ci \brief Add a user element to the skiplist if it does not exist.
 *
 * \param[in] self - The skiplist to add the element to. Must not be NULL.
 * \param[out] alreadyExists - If not NULL, set to TRUE if the node already
 *                             existed.
 * \param[in] element - The element to add. Must not be NULL.
 *
 * \return TRUE on sucess, FALSE on failure. If the element already already
 *         existed TRUE is returned and if alreadyExists is not NULL it is set
 *         to TRUE.
 */
RTIXCdrBoolean
RTIXCdrSkipList_assertElement(struct RTIXCdrSkipList *self,
                              RTIXCdrBoolean *alreadyExists,
                              void *element)
{
    struct RTIXCdrSkipListNode key;
    struct RTIXCdrSkipListNode *node;
    struct RTIXCdrSkipListNode *next_node;

    if (alreadyExists != NULL)
    {
        *alreadyExists = RTI_XCDR_FALSE;
    }

    key.user_data = element;
    key.skip_list = self;
    REDA_CircularListNode_init(&key._node);

    next_node = REDA_Indexer_find_entry_eq_or_gt(self->indexer, &key);

    if ((next_node != NULL) &&
        !self->cmp_func(key.user_data,next_node->user_data))
    {
        if (alreadyExists != NULL)
        {
            *alreadyExists = RTI_XCDR_TRUE;
        }
        return RTI_XCDR_TRUE;
    }

    node = (struct RTIXCdrSkipListNode*)REDA_BufferPool_get_buffer(self->pool);
    if (node == NULL)
    {
        return RTI_XCDR_FALSE;
    }

    *node = key;

    if (!REDA_Indexer_add_entry(self->indexer, node))
    {
        REDA_BufferPool_return_buffer(self->pool, node);
        return RTI_XCDR_FALSE;
    }

    if (next_node != NULL)
    {
        REDA_CircularList_link_node_after((REDA_CircularListNode_T*)node,
                                          (REDA_CircularListNode_T*)next_node);
    }
    else
    {
        REDA_CircularList_append(&self->list,(REDA_CircularListNode_T*)node);
    }

    return RTI_XCDR_TRUE;
}

/*ci \brief Find an element, or the next higher, in a skiplist.
 *
 * \param[in] self - The skiplist to add the element to. Must not be NULL.
 * \param[out] preciseMatch - If not NULL, set to TRUE if the node is an exact
 *                            match.
 * \param[in] searchElement - The element to find. Must not be NULL.
 *
 * \return If an exact match is found, return the element and if preciseMatch
 *         is not NULL set it to TRUE. If an exact match is not found, but an
 *         greater element is found return return the element and if preciseMatch
 *         is not NULL set it to FALSE. Otherwise return NULL.
 */
void*
RTIXCdrSkipList_findElement(const struct RTIXCdrSkipList *self,
                            RTIXCdrBoolean *preciseMatch,
                            void *searchElement)
{
    struct RTIXCdrSkipListNode key;
    struct RTIXCdrSkipListNode *next_node;

    if (preciseMatch != NULL)
    {
        *preciseMatch = RTI_XCDR_FALSE;
    }

    key.user_data = searchElement;
    key.skip_list = self;
    REDA_CircularListNode_init(&key._node);

    next_node = REDA_Indexer_find_entry_eq_or_gt(self->indexer, &key);

    if (next_node == NULL)
    {
        return  NULL;
    }

    if (!self->cmp_func(key.user_data,next_node->user_data))
    {
        if (preciseMatch != NULL)
        {
            *preciseMatch = RTI_XCDR_TRUE;
        }
    }

    return next_node->user_data;
}

/*ci \brief Delete the skiplist
 *
 * \param[in] self - The skiplist to delete. Must not be NULL.
 */
void
RTIXCdrSkipList_delete(struct RTIXCdrSkipList *self)
{
    const struct RTIXCdrSkipListNode *sl_node;
    const struct RTIXCdrSkipListNode *next_node;

    sl_node = RTIXCdrSkipList_getFirstNode(self);
    while (sl_node != NULL)
    {
        next_node = RTIXCdrSkipList_getNextNode(self,sl_node);
        REDA_BufferPool_return_buffer(self->pool,
                                    (struct RTIXCdrSkipListNode*)sl_node);
        sl_node = next_node;
    }

#ifndef RTI_CERT
    if (!REDA_Indexer_delete(self->indexer))
    {
        return;
    }

    if (!REDA_BufferPool_delete(self->pool))
    {
        return;
    }

    OSAPI_Heap_free(self);
#endif
}

/*ci \brief Create a skiplist capable of holding a finite number of elements
 *
 * \param[in] compareFnc - A compare function determining the order of the
 *                         nodes in the skiplist. The function receives a left
 *                         node and a right node and must return 1 if left > right,
 *                         0 if left == right, -1 if left < right.
 *
 * \param[in] expectedElementCount - The maximum number of elements in the list
 *
 * \return An empty skiplist on success, NULL on failure.
 */
extern struct RTIXCdrSkipList*
RTIXCdrSkipList_new(RTIXCdrSkipListDataTypeCompareFunction compareFnc,
                    RTIXCdrUnsignedLong expectedElementCount)
{
    struct REDA_IndexerProperty idx_property = REDA_IndexerProperty_INITIALIZER;
    struct REDA_BufferPoolProperty bp_property = REDA_BufferPoolProperty_INITIALIZER;
    struct RTIXCdrSkipList *a_skiplist = NULL;

    OSAPI_Heap_allocate_struct(&a_skiplist, struct RTIXCdrSkipList);
    if (a_skiplist == NULL)
    {
        return NULL;
    }

    idx_property.max_entries = (RTI_INT32)expectedElementCount;
    a_skiplist->indexer = REDA_Indexer_new(RTIXCdrSkipList_indexer_compare,
                                           &idx_property);
    if (a_skiplist->indexer == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free(a_skiplist);
#endif
        return NULL;
    }

    bp_property.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
    bp_property.buffer_size = RTI_SIZEOF(struct RTIXCdrSkipListNode);
    bp_property.max_buffers = expectedElementCount;

    a_skiplist->pool = REDA_BufferPool_new("skiplist", &bp_property,
                                           NULL, NULL, NULL, NULL);

    if (a_skiplist->pool == NULL)
    {
#ifndef RTI_CERT
        if (!REDA_Indexer_delete(a_skiplist->indexer))
        {
            return NULL;
        }
        OSAPI_Heap_free(a_skiplist);
#endif
        return NULL;
    }

    REDA_CircularList_init(&a_skiplist->list);
    a_skiplist->cmp_func = compareFnc;

    return a_skiplist;
}

