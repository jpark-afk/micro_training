/*
(c) Copyright, Real-Time Innovations, 2014-2021.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"

typedef int (*RTIXCdrSkipListDataTypeCompareFunction)(
        const void *left,
        const void *right);

struct RTIXCdrSkipList;
struct RTIXCdrSkipListNode;

/*
 * @brief Asserts an element in the list.
 *
 * If the element is already part of the list this method set *alreadyExists
 * to RTI_XCDR_TRUE and returns RTI_XCDR_TRUE.
 *  
 * @param self \b InOut. The list. Cannot be NULL.
 * @param alreadyExists \b Out. If not NULL this parameter is initialized to 
 * RTI_XCDR_TRUE if the element already exists and RTI_XCDR_FALSE if the
 * element does not exists.
 * @param element \b In. Pointer to the element that the user wants to
 * assert. Cannot be NULL.
 * 
 * @return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
extern RTIXCdrBoolean RTIXCdrSkipList_assertElement(
        struct RTIXCdrSkipList *self,
        RTIXCdrBoolean *alreadyExists,
        void *element);

/*
 * @brief Finds an element in the list.
 *
 * This function will search through all elements in the list looking for 
 * the first element larger or equal to the provided searchElement.
 * 
 * @param self \b In. The list. Cannot be NULL.
 * @param preciseMatch \b Out. If not NULL this parameter is initialized to 
 * RTI_XCDR_TRUE if searchElement is found.
 * @param searchElement \b In. Pointer to the element that the user wants to
 * find. Only the members that are used by the list's compare function have
 * to be initialized.
 * 
 * @return Pointer to the first element larger or equal to the provided 
 * searchElement.
 */
extern void * RTIXCdrSkipList_findElement(
        const struct RTIXCdrSkipList *self,
        RTIXCdrBoolean *preciseMatch,
        void *searchElement);

/*
 * @brief Gets a pointer to the element associated with a node.
 *
 * @param self \b In. The node. Cannot be NULL.
 * 
 * @return Pointer to the the element associated with a node.
 */
extern void * RTIXCdrSkipListNode_getElement(
        const struct RTIXCdrSkipListNode *self);

/*
 * @brief Gets a pointer to the first node in the list.
 *
 * @param self \b In. The list. Cannot be NULL.
 * 
 * @return Pointer to the first node or NULL if the list is empty.
 */
extern const struct RTIXCdrSkipListNode * RTIXCdrSkipList_getFirstNode(
        const struct RTIXCdrSkipList *self);

/*
 * @brief Gets a pointer to the next node in the list.
 *
 * @param self \b In. The list. Cannot be NULL.
 * @param prevNode \b In. Pointer to previous node. Cannot be NULL.
 * 
 * @return Pointer to the next node or NULL if prevNode is the last node.
 */
extern const struct RTIXCdrSkipListNode * RTIXCdrSkipList_getNextNode(
        const struct RTIXCdrSkipList *self,
        const struct RTIXCdrSkipListNode *prevNode);

/*
 * @brief Deletes a list.
 *
 * @param self \b InOut. The list. Cannot be NULL.
 */
extern void RTIXCdrSkipList_delete(struct RTIXCdrSkipList *self);

/*
 * @brief Creates a new RTIXCdrSkipList.
 *
 * A RTIXCdrSkipList is a list of elements that provides fast addition, removal
 * and search.
 * 
 * @param compareFnc \b In. Compare function used to order elements in the list.
 * Cannot be NULL.
 * @param expectedElementCount \b In. The number of expected elements in the 
 * list. The performance of the list depends on this value being accurate.
 * 
 * @return A new list or NULL if there is an error.
 */
extern struct RTIXCdrSkipList * RTIXCdrSkipList_new(
        RTIXCdrSkipListDataTypeCompareFunction compareFnc,
        RTIXCdrUnsignedLong expectedElementCount);
