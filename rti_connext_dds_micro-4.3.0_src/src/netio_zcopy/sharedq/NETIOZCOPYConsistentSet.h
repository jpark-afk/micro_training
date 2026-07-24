/*
 * FILE: NETIOZCOPYConsistentSet.h - Shared Robust Consistent Collection -- interface
 *
 * (c) Copyright 2022-2026 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*i
 * \defgroup NETIOZCOPY_ConsistentSetClass NETIOZCOPY_ConsistentSet
 * @{
 */
/*i
 * \file
 * \brief Shared Robust Consistent Collection -- definition
 */
#ifndef NETIOZCOPYConsistentSet_h
#define NETIOZCOPYConsistentSet_h

#include "netio_zcopy/netio_zcopy_sharedq_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ----------------
 * Type definitions
 * ----------------
 */

typedef struct NETIO_ZCOPY_ConsistentSetOwnedImpl NETIO_ZCOPY_ConsistentSetOwned;
typedef struct NETIO_ZCOPY_ConsistentSetObservedImpl NETIO_ZCOPY_ConsistentSetObserved;

/* For values uniquely identifying an element */
typedef RTI_UINT64 SQ_ElmtId;
#define SQ_ELMT_ID_MAX ((SQ_ElmtId)-1)

/* ----------------
 * Class operations
 * ----------------
 */

/*ci
 * \brief Gets the memory size needed for a NETIO_ZCOPY_ConsistentSetOwned
 *        object.
 *
 * This function calculates the memory size needed for a
 * NETIO_ZCOPY_ConsistentSetOwned object based on the provided element count and
 * observer count.
 *
 * \param[in] elmt_count The number of elements in the consistent set.
 * \param[in] obsrv_count The number of observers in the consistent set.
 * \param[out] length_out A pointer to the length of the memory needed for the
 *             consistent set.
 *
 * \return RTI_TRUE if the function succeeded, RTI_FALSE otherwise.
 *
 * \pre The element count and observer count must be within the range of 1 to
 *      100000000.
 *
 * \post In case of success, the length_out parameter will be set to the
 *       calculated memory size needed for the consistent set.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_create
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSet_get_mem_size(
        SQ_Index elmt_count,
        SQ_Index obsrv_count,
        SQ_Length *length_out);

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */

/*ci
 * \brief The element data initializer function gets invoked for every element,
 *   to associate it with (opaque) data
 *
 * \param[in] elmt_index The index of the element being initialized
 * \param[out] elmt_data_length_out The length of the element data
 * \param[out] elmt_data_out The element data
 * \param[in] initializer_param The parameter passed to the initializer function
 */
typedef void (*NETIO_ZCOPY_ElementDataInitializerFunc)(
        SQ_Index elmt_index,
        SQ_Length *elmt_data_length_out,
        SQ_MemPtr *elmt_data_out,
        void *initializer_param);

/*ci
 * \brief Creates a NETIO_ZCOPY_ConsistentSetOwned object.
 *
 * This function creates a NETIO_ZCOPY_ConsistentSetOwned object based on the
 * provided element count and observer count. It also initializes the elements
 * of the consistent set using the provided initializer function.
 *
 * \param[in] elmt_count The number of elements in the consistent set.
 * \param[in] obsrv_count The number of observers in the consistent set.
 * \param[in] flat_mem_len The length of the flat memory provided for the
 *            consistent set.
 * \param[in] flat_mem A pointer to the flat memory provided for the consistent
 *            set.
 * \param[in] initializer A pointer to the function used to initialize the
 *            elements of the consistent set.
 * \param[in] initializer_param A pointer to the parameter used by the
 *            initializer function.
 * \param[out] self_out A pointer to the NETIO_ZCOPY_ConsistentSetOwned object.
 *
 * \return RTI_TRUE if the function succeeded, RTI_FALSE otherwise.
 *
 * \pre The element count and observer count must be within the range of 1 to
 *      100000000.
 * \pre The flat memory length must be greater than or equal to the memory size
 *      needed for the consistent set.
 * \pre The flat memory must be aligned to the size of a pointer.
 * \pre The initializer function must not be NULL.
 *
 * \post In case of success, the self_out parameter will be set to the created
 *       NETIO_ZCOPY_ConsistentSetOwned object.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_get_mem_size
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_create(
        SQ_Index elmt_count,
        SQ_Index obsrv_count,
        SQ_Length flat_mem_len,
        SQ_MemPtr flat_mem /*[flat_mem_len]*/,
        RTI_BOOL use_timestamp_as_id,
        NETIO_ZCOPY_ElementDataInitializerFunc initializer,
        void *initializer_param,
        NETIO_ZCOPY_ConsistentSetOwned **self_out);

/*ci
 * \brief Destroy a NETIO_ZCOPY_ConsistentSetOwned object.
 *
 * \param[in] self The object to destroy.
 *
 * \return RTI_TRUE if the object was destroyed successfully, RTI_FALSE
 *         otherwise.
 *
 * \post In case of success, the self parameter will be set to NULL.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_create
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_destroy(NETIO_ZCOPY_ConsistentSetOwned *self);

/*ci
 * \brief Create a NETIO_ZCOPY_ConsistentSetObserved object.
 *
 * This function creates a NETIO_ZCOPY_ConsistentSetObserved object.
 *
 * \param[out] self_out A pointer to the created
 *             NETIO_ZCOPY_ConsistentSetObserved object.
 *
 * \return RTI_TRUE if the object was created successfully, RTI_FALSE otherwise.
 *
 * \post In case of success, the self_out parameter will be set to the created
 *       NETIO_ZCOPY_ConsistentSetObserved object.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_open
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_close
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_initialize(
        NETIO_ZCOPY_ConsistentSetObserved **self_out);

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_destroy(
        NETIO_ZCOPY_ConsistentSetObserved *self);

/*ci
 * \brief Configure an existing NETIO_ZCOPY_ConsistentSetObserved object.
 *
 * This function configures an existing NETIO_ZCOPY_ConsistentSetObserved object
 * and initializes it with the provided parameters.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetObserved object.
 * \param[in] with_consistency A boolean value indicating whether consistency is
 *            requested.
 * \param[in] obsrv_index The index of the observer.
 * \param[in] flat_memory_length The length of the flat memory.
 * \param[in] flat_memory A pointer to the flat memory.
 * \param[in] initial A boolean value indicating whether this is the initial
 *                    configuration.
 *
 * \return RTI_TRUE if the object was created successfully, RTI_FALSE otherwise.
 *
 * \pre The flat_memory parameter must be large enough to inspect the required
 *      size.
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \post In case of success, the NETIO_ZCOPY_ConsistentSetObserved object
 *       pointed to by self will be configured.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_initialize
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_close
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_open(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        RTI_BOOL with_consistency,
        SQ_Index obsrv_index,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory,
        RTI_BOOL initial);

/*ci
 * \brief Detach the encapsulated PIConsistentSet from an observer and destroy
 *        the encapsulating object.
 *
 * \param[in] self The encapsulating object to be destroyed.
 *
 * \return RTI_TRUE if the object was destroyed successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \post In case of success, the self parameter will be set to NULL.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_open
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_close(NETIO_ZCOPY_ConsistentSetObserved *self);

/* --------------------
 * Owner-side functions
 * --------------------
 */

/* Functions expected to be used on the owner side of the collection only */

/*ci
 * \brief Reserve an element in a consistent set.
 *
 * This function reserves an element in a consistent set.
 *
 * \param[in] self The consistent set in which to reserve an element.
 * \param[out] elmt_index_out The index of the reserved element.
 * \param[out] elmt_data_length_out The length of the data in the reserved
 *             element.
 * \param[out] elmt_data_out The data in the reserved element.
 *
 * \return RTI_TRUE if an element was reserved successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \post In case of success, the element will be removed from the free list
 *       and prepend it onto the list of reserved elements
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_element_unreserve
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_reserve(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index *elmt_index_out,
        SQ_Length *elmt_data_length_out, /* optional */
        SQ_MemPtr *elmt_data_out /* optional*/);

/*ci
 * \brief Unreserve an element in a consistent set.
 *
 * This function unreserves an element in a consistent set.
 *
 * \param[in] self The consistent set in which to unreserve an element.
 * \param[in] elmt_index The index of the element to unreserve.
 *
 * \return RTI_TRUE if an element was unreserved successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \post In case of success, the element will be added to the free list.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_element_reserve
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_unreserve(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index);

/*ci
 * \brief Commit an element in a consistent set.
 *
 * This function commits an element locally in a consistent set,
 * and finally commit it in the shared region
 *
 * \param[in] self The consistent set in which to commit an element.
 * \param[in] elmt_index The index of the element to commit.
 *
 * \return RTI_TRUE if an element was committed successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \pre The element must not be committed already.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_element_purge
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_commit(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index);

/*ci
 * \brief Purge an element in a consistent set locally.
 *
 * \param[in] self The consistent set in which to purge an element.
 * \param[in] elmt_index The index of the element to purge.
 *
 * \return RTI_TRUE if an element was uncommitted successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \pre The element must already be committed.
 * \post In case of success, the element will be appended to the free list.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_element_commit
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_purge(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index);

/*ci
 * \brief Evict an observer from the consistent set.
 *
 * \param[in] self The consistent set from which to evict the observer.
 * \param[in] obsrv_index The index of the observer to evict.
 *
 * \return RTI_TRUE if the observer was evicted successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 * \pre The observer must exist in the consistent set.
 *
 * \post In case of success, the observer will be removed from the consistent
 *       set.
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_observer_evict(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index obsrv_index);

/*ci
 * \brief Lookup index of reserved element by its user data value.
 *
 * This function is used to lookup a reserved element in a consistent set by its
 * data.
 *
 * \param[in] self The consistent set.
 * \param[in] user_data The user data to compare against the element's data.
 * \param[out] elmt_index_out The index of the element if found.
 *
 * \return RTI_TRUE if the element was found, RTI_FALSE otherwise.
 *
 * \sa \ref lookup_reserved_elmt_id_predicate
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_lookup_reserved_elmt_by_data(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_MemPtr_Const user_data,
        SQ_Index* elmt_index_out);


/* -----------------------
 * Observer-side functions
 * -----------------------
 */


/*ci
 * \brief The element visitor function gets invoked for every element in the
 *   consistent set.
 *
 * \param[in] elmt_id The unique identifier of the element being visited
 * \param[in] elmt_index The index of the element being visited
 * \param[in] visit_data The parameter passed to the visitor function
 *
 * \return RTI_TRUE to continue the iteration, RTI_FALSE to terminate the
 *         iteration prematurely
 */
typedef RTI_BOOL (*NETIO_ZCOPY_VisitElementFunc)(
        SQ_ElmtId elmt_id,
        SQ_Index elmt_index,
        void *visit_data);

/*ci
 * \brief Iterates over the elements in a consistent set and applies visit
 *        function to them.
 *
 * This function iterates over the elements in a consistent set,
 * starting from the element with the specified ID and index.
 * It calls the specified visit function for each element visited,
 * passing the element's ID, index, and the specified visit data as
 * arguments. If the visit function returns false, the iteration is terminated.
 *
 * \param[in] self The consistent set observed by the current process.
 * \param[in] starting_elmt_id The ID of the element to start iterating from.
 * \param[in] starting_elmt_index The index of the element to start iterating
 *            from.
 * \param[in] visit_func The function to call for each element visited.
 * \param[in] visit_data The data to pass to the visit function.
 *
 * \return RTI_TRUE if the iteration completed successfully, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_elements_iterate(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId starting_elmt_id,
        SQ_Index starting_elmt_index, /* for faster access */
        NETIO_ZCOPY_VisitElementFunc visit_func,
        void *visit_data);

/*ci
 * \brief Protects an element in the consistent set observed.
 *
 * This function protects an element in the consistent set observed.
 * If consistency is required, it marks the element as locked.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetObserved object.
 * \param[in] elmt_id The ID of the element to protect.
 * \param[in] elmt_index The index of the element to protect.
 * \param[out] is_valid_out A pointer to a boolean value that will be set to
 *             true if the element is still valid, false otherwise.
 *
 * \return RTI_TRUE if the element was successfully protected, RTI_FALSE
 *         otherwise.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_element_unprotect
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_element_protect_if_valid(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId elmt_id,
        SQ_Index elmt_index,
        RTI_BOOL *is_valid_out);

/*ci
 * \brief Unprotects an element in the consistent set observed.
 *
 * This function unprotects an element in the consistent set observed.
 * If consistency is required, it marks the element as unlocked.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetObserved object.
 * \param[in] elmt_index The index of the element to unprotect.
 *
 * \return RTI_TRUE if the element was successfully unprotected, RTI_FALSE
 *         otherwise.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_element_protect
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_element_unprotect(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_Index elmt_index);


/*ci
 * \brief Check whether index/id mapping is still correct.
 *
 * This function returns whether an element in the consistent set observed is
 * consistent.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetObserved object.
 * \param[in] elmt_id The ID of the element to check.
 * \param[in] elmt_index The index of the element to check.
 * \param[out] is_elmt_consistent_out A pointer to a boolean value that will be
 *             set to true if the element is consistent, false otherwise.
 *
 * \return RTI_TRUE if the element was successfully checked, RTI_FALSE
 *         otherwise.
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetObserved_element_protect
 *
 * \note Intended for observers without strong consistency
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_get_element_is_consistent(
        const NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId elmt_id,
        SQ_Index elmt_index,
        RTI_BOOL *is_elmt_consistent_out);

/*i
 * \brief Reset the NETIO_ZCOPY_ConsistentSetObserved to its default values
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetObserved object.
 * \return void
 */
void
NETIO_ZCOPY_ConsistentSetObserved_reset(
        NETIO_ZCOPY_ConsistentSetObserved *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETIOZCOPYConsistentSet_h */

/*i @} */
