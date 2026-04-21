/*
 * FILE: reda_circularlist.h - Circular list interface
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Changed reda_circularlist.c to reda_circularlist.h in FILE comment.
 * 05may2015,tk MICRO-1100/PR#14153 Added comments to is_linked() and
 *                                  unlink_node() for undefined behavior on an
 *                                  unlinked node
 * 01dec2014,tk MICRO-963/PR#12377 Fixed comment
 * 04jan2012,tk Written
 */
/*ci
 * \file
 *
 * \brief The REDA CircularList interface provides a platform independent API to
 *        manage a circular lists
 *
 * \defgroup REDACircularListClass REDA CircularList
 *
 * \ingroup REDAModule
 *
 * \details
 *
 * The REDA CircularList implements a circular list as a double linked list
 * where the head is an empty list-node.
 *
 */
/*ci \addtogroup REDACircularListClass
 *   @{
 */
#ifndef reda_circularlist_h
#define reda_circularlist_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief REDA_CircularListNode element
 *
 * \details
 *
 * Any node can be added to a circular list, and all APIs is assumed all nodes
 * includes this node as the first element:
 *
 * \code
 *
 * struct MyNode
 * {
 *     struct REDA_CircularListNode _node;
 *     ...
 * };
 * \endcode
 *
 * Note that REDA_CircularListNode does not contain a pointer to the node
 * data (MyNode) above.
 */
struct REDA_CircularListNode
{
    struct REDA_CircularListNode *_prev;
    struct REDA_CircularListNode *_next;
};

/*ci
 * \brief Abstract REDA_CircularList type
 */
typedef struct REDA_CircularListNode REDA_CircularList_T;

/*ci
 * \brief Abstract REDA_CircularListNode type
 */
typedef struct REDA_CircularListNode REDA_CircularListNode_T;

/*ci
 * \brief Initialize a circular list
 *
 * \details
 *
 * Initialize a circular list to an empty state. A variable of type
 * REDA_CircularList_T should be initialized exactly once.
 *
 * \param[in] list circular list to initialize
 */
REDADllExport void
REDA_CircularList_init(REDA_CircularList_T *list);

/*ci
 * \brief Initialize a circular list node
 *
 * \details
 *
 * Initialize a circular list node. An initialized node is not linked to
 * any list. It is the callers responsiblilty to ensure that
 * REDA_CircularListNode_init is <em> not </em> called on linked node.
 *
 * \param[in] node circular list node to initialize
 *
 */
REDADllExport void
REDA_CircularListNode_init(struct REDA_CircularListNode *node);

/*ci
 * \brief Link a circular list node after a node already in the list
 *
 * \details
 *
 * Link a circular list node after another node. Note that circular list
 * head is a circular list node itself.
 *
 * \param[in] after circular list node to link after
 * \param[in] node  circular list node to add to the list
 *
 * \sa \ref REDA_CircularList_link_node_after
 */
REDADllExport void
REDA_CircularList_link_node_after(struct REDA_CircularListNode *after,
                                  struct REDA_CircularListNode *node);

/*ci
 * \brief Unlink a circular list node from a circular list
 *
 * \details
 *
 * Remove a node from a circular list. No memory deallocation takes place
 * when a node is unlinked.  Note that this functions accepts a node where
 * both pointers already are NULL, but _does not_ perform any error checking.
 * If an invalid node is passed in the behavior is undefined.
 *
 * \param[in] node circular list node to remove from a list
 */
REDADllExport void
REDA_CircularList_unlink_node(struct REDA_CircularListNode *node);

/*ci
 * \brief Add a circular list node to the end of a circular list
 *
 * \details
 *
 * Append a circular list node to the end of a circular list.
 *
 * \param[in] list circular list node to append element to
 * \param[in] node circular list node to append
 *
 */
REDADllExport void
REDA_CircularList_append(REDA_CircularList_T *list,
                         REDA_CircularListNode_T *node);

#define REDA_CircularList_append(list_,node_) \
        REDA_CircularList_link_node_after((list_)->_prev,node_)

/*ci
 * \brief Add a circular list node to the beginning of the circular list
 *
 * \details
 *
 * Add a circular list node to the beginning of a circular list
 *
 * \param[in] list circular list node to prepend element to
 * \param[in] node circular list node to prepend
 *
 */
REDADllExport void
REDA_CircularList_prepend(REDA_CircularList_T *list,
                          REDA_CircularListNode_T *node);

#define REDA_CircularList_prepend(list_,node_) \
        REDA_CircularList_link_node_after((list_),node_)

/*ci
 * \brief Check if a circular list is empty
 *
 * \details
 *
 * Check if a circular list is empty
 *
 * \param[in] list circular list to check
 *
 * \return RTI_TRUE if circular list is empty, RTI_FALSE if not empty
 */
REDADllExport RTI_BOOL
REDA_CircularList_is_empty(REDA_CircularList_T *list);


/*ci
 * \brief Check if a node is at the head of the circular list head
 *
 * \details
 *
 * Check if a node is at the head of the circular list head
 *
 * \param[in] list circular list node to check
 * \param[in] node circular list node to check
 *
 * \return RTI_TRUE if node is at head, RTI_FALSE if not
 */
REDADllExport RTI_BOOL
REDA_CircularList_node_at_head(REDA_CircularList_T *list,
                               REDA_CircularListNode_T *node);

#define REDA_CircularList_node_at_head(c_list_,c_node_) \
                ((void*)(c_list_) == (void*)(c_node_))

/*ci
 * \brief Return the head of a circular list
 *
 * \details
 *
 * Return the head of a circular list
 *
 * \param[in] list circular list node to return head of
 *
 * \return pointer to first element in circular list
 */
REDADllExport REDA_CircularListNode_T*
REDA_CircularList_get_first(REDA_CircularList_T *list);

#define REDA_CircularList_get_first(c_list_) ((c_list_)->_next)

/*ci
 * \brief Return the last element of a circular list
 *
 * \details
 *
 * Return the last element of a circular list
 *
 * \param[in] list circular list node to return the last element of
 *
 * \return pointer to last element in circular list
 */
REDADllExport REDA_CircularListNode_T*
REDA_CircularList_get_last(REDA_CircularList_T *list);

#define REDA_CircularList_get_last(c_list_) ((c_list_)->_prev)

/*ci
 * \brief Return the next element after a node
 *
 * \details
 *
 * Given a node in the circular list node, return the element after the node
 *
 * \param[in] node circular list node to return the next element of
 *
 * \return pointer to element after the node
 */
REDADllExport REDA_CircularListNode_T*
REDA_CircularListNode_get_next(REDA_CircularListNode_T *node);

#define REDA_CircularListNode_get_next(c_node_) ((c_node_)->_next)

/*ci
 * \brief Return the previous element after a node
 *
 * \details
 *
 * Given a node in the circular list node, return the element before the node
 *
 * \param[in] node circular list node to return the previous element of
 *
 * \return pointer to element before the node
 */
REDADllExport REDA_CircularListNode_T*
REDA_CircularListNode_get_prev(REDA_CircularListNode_T *node);

#define REDA_CircularListNode_get_prev(c_node_) ((c_node_)->_prev)

/*ci
 * \brief Check if a node is linked into a circular list
 *
 * \details
 *
 * Check if a node is linked into a circular list. Note that this function
 * does only returns TRUE is both pointers are non-NULL. That is, this function
 * only works on a valid circular list-node where either both link pointers
 * are NULL or not NULL. If an invalid node is passed in the behavior is
 * undefined.
 *
 * \param[in] node circular list node to check
 *
 * \return RTI_TRUE if node is linked, RTI_FALSE if not
 */
REDADllExport RTI_BOOL
REDA_CircularListNode_is_linked(REDA_CircularListNode_T *node);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_circularlist_h */

/*ci @} */
