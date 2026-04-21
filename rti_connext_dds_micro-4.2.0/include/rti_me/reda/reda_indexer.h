/*
 * FILE: reda_indexer.h - Indexer interface
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
 * 31jul2014,tk MICRO-172/PR#1064 - Removed superfluous REDA_Indexer fields
 * 25oct2012,tk Written
 */
/*ci
 * \file
 *
 * \brief The REDA Indexer interface provides a implementation independent API
 *        for searching and finding elements.
 *
 * \defgroup REDAIndexerClass REDA Indexer
 * \ingroup REDAModule
 *
 * \details
 *
 * The REDA Indexer indexes elements based on a compare function and
 * provides function to search and iterate over the elements in the index.
 * The API is defined so that the underlying data-structures are not exposed.
 */
#ifndef reda_indexer_h
#define reda_indexer_h
#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \addtogroup REDAIndexerClass
 * @{
 */
struct REDA_Indexer;

struct REDA_IndexIterator;

typedef struct REDA_IndexIterator REDA_IndexIterator_T;

/*ci
 * \brief Abstract Indexer type
 */
typedef struct REDA_Indexer REDA_Indexer_T;

/*ci
 * \brief The properties an index can be created with.
 */
struct REDA_IndexerProperty
{
    /*ci
     * \brief The maximum number of elements the index can handle
     */
    RTI_INT32 max_entries;
};

/*ci
 * \def REDA_IndexerProperty_INITIALIZER
 * \brief \ref REDA_IndexerProperty initializer
 */
#define REDA_IndexerProperty_INITIALIZER \
{\
    1\
}

/*ci
 * \brief A generic record compare function
 *
 * \details
 *
 * The compare function must maintain an ordered relationship.
 *
 * \param[in] record        Already indexed element to compare against
 * \param[in] key_is_record Whether the key is a complete record or the user
 *                          defined key
 * \param[in] key           Key to compare against
 *
 * \return The function must return zero if record == key,
 *         a negative integer if record < key,
 *         and a positive integer if record > key
 */
FUNCTION_MUST_TYPEDEF(
RTI_INT32
(*REDA_Indexer_compare_T)(const void *const record, RTI_BOOL key_is_record,
                          const void *const key)
)

/*ci
 * \brief Create a new indexer
 *
 * \param[in] compare Compare function that determines the ordering of the
 *                    indexed elements as defined for
 *                    \ref REDA_Indexer_compare_T
 *
 * \param[in] property Indexer property. Refer to \ref
 *                     REDA_IndexerProperty for details.
 *
 * \return Pointer to new indexer on success, NULL on failure
 *
 * \sa REDA_Indexer_delete
 */
MUST_CHECK_RETURN REDADllExport REDA_Indexer_T*
REDA_Indexer_new(REDA_Indexer_compare_T compare,
                 struct REDA_IndexerProperty *property);

#ifndef RTI_CERT
/*ci
 * \brief Delete an indexer
 *
 * \details
 *
 * Delete an index. It is legal to delete an index which contains elements.
 * It is the callers responsibility to make sure that the elements in the
 * index can be properly deleted.
 *
 * \param[in] indexer Indexer to delete
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa REDA_Indexer_new
 */
SHOULD_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Indexer_delete(REDA_Indexer_T *indexer);
#endif /* !RTI_CERT */

/*ci
 * \brief Add an element to an index
 *
 * \details
 *
 * Add an element to the indexer. The indexer is not responsible for managing
 * the memory owner the entry.
 *
 * \param[in] indexer Indexer to add the element to
 * \param[in] entry   Entry to add to indexer
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa REDA_Indexer_remove_entry
 */
MUST_CHECK_RETURN REDADllExport RTI_BOOL
REDA_Indexer_add_entry(REDA_Indexer_T *indexer,void *entry);

/*ci
 * \brief Remove an element from an index
 *
 * \details
 *
 * Remove an element from an indexer based on the key. The indexer is not
 * responsible for managing the memory used by the removed entry.
 *
 * \param[in] indexer Indexer to add the element to
 * \param[in] key     Key to remove
 *
 * \return pointer to removed entry if it existed, NULL otherwise.
 *
 * \sa REDA_Indexer_add_entry
 */
SHOULD_CHECK_RETURN REDADllExport void*
REDA_Indexer_remove_entry(REDA_Indexer_T *indexer,const void *const key);

/*ci
 * \brief Find an entry based on the key
 *
 * \details
 *
 * Search for an entry based on the key.
 *
 * \param[in] indexer Indexer to search for element in
 * \param[in] key     Key to search for
 *
 * \return pointer to entry if it existed, NULL otherwise.
 *
 * \sa REDA_Indexer_find_entry_eq_or_gt
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_find_entry(REDA_Indexer_T *indexer,const void *const key);

/*ci
 * \brief Find an entry with a key greater or equal to the key
 *
 * \details
 *
 * Find an entry with a key greater or equal to the key.
 *
 * \param[in] indexer Indexer to search for element in
 * \param[in] key     Key to search for
 *
 * \return pointer to entry if it existed, NULL otherwise.
 *
 * \sa REDA_Indexer_find_entry
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_find_entry_eq_or_gt(REDA_Indexer_T *indexer,const void *const key);

/*ci
 * \brief Find an entry with a key less than or equal to the key
 *
 * \details
 *
 * Find an entry with a key greater or equal to the key.
 *
 * \param[in] indexer Indexer to search for element in
 * \param[in] key     Key to search for
 *
 * \return pointer to entry if it existed, NULL otherwise.
 *
 * \sa REDA_Indexer_find_entry
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_find_entry_eq_or_lt(REDA_Indexer_T *indexer,const void *const key);

/*ci
 * \brief Return the number of elements in the index
 *
 * \details
 *
 * Return the number of elements in the index
 *
 * \param[in] indexer Indexer to return count for. Indexer cannot be NULL,
 *                    and it is the callers responsibility to ensure it is not.
 *
 * \return Number of elements in the index.
 *
 * \sa REDA_Indexer_get_entry
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_Indexer_get_count(REDA_Indexer_T *indexer);

/*ci
 * \brief Return the index'th element in the index
 *
 * \details
 *
 * Return the index'th element in the index.
 *
 * \param[in] indexer Indexer to return the element from
 * \param[in] index   Which element to return, starting at 0
 *
 * \return The index'th element in the indexer
 *
 * \sa REDA_Indexer_get_count
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_get_entry(REDA_Indexer_T *indexer,RTI_INT32 index);

/*ci
 * \brief Return the first element in the index
 *
 * \details
 *
 * Return the first element in the index
 *
 * \param[in] indexer Indexer to return the the first element from
 *
 * \return The first element in the index, NULL if the index is empty.
 *
 * \sa
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_get_first_entry(REDA_Indexer_T *indexer);

/*ci
 * \brief Return the last element in the index
 *
 * \details
 *
 * Return the last element in the index
 *
 * \param[in] indexer Indexer to return the the last element from
 *
 * \return The last element in the index, NULL if the index is empty.
 *
 * \sa
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_get_last_entry(REDA_Indexer_T *indexer);

/*ci
 * \brief Start an iterator for the indexer
 *
 * \details
 *
 * Each indexer has exactly one -1- iterator. This function initializes the
 * iterator for the indexer and clears any previous initialization. Thus,
 * it is the callers responsibility to ensure that the iterator is used
 * at most once at a time.
 *
 * \param[in] indexer Indexer to iterate over
 *
 * \return An initialized iterator
 */
REDADllExport REDA_IndexIterator_T*
REDA_Indexer_iterator_begin(REDA_Indexer_T *indexer);

/*ci
 * \brief Return the the next element in the iterator
 *
 * \details
 *
 * Each call to this function returns the next element in the indexer. The
 * next element is the element that is larger than the current
 * element as defined by the indexer's compare function.
 *
 * NOTE: If an element is removed the iterator is updated so that no elements
 *       are missed. However, if an element is added the iterator may
 *       include the new element. Thus, a current iteration must be able
 *       to handle the addition of a new element as well as not rely on
 *       the additon of a new element.
 *
 * \param[in] iterator Iterator to return the the last element from
 *
 *
 * \return The next element or NULL if there are no more elements
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_Indexer_iterator_next(REDA_IndexIterator_T *iterator);

#ifdef __cplusplus
}         /* extern "C" */
#endif

#endif /* reda_indexer_h */

/*ci @} */
