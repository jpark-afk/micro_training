/*
 * FILE: NETIOZCOPYSharedMemPool.h - Relocatable memory pool -- interface
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
/*ce
 * \defgroup NETIOZCOPYSharedMemPoolClass NETIOZCOPYSharedMemPool
 * @{
 */

/*ce
 * \file
 * \brief Zero Copy Shared Memory Pool definitions
 */

#ifndef NETIOZCOPYSharedMemPool_h
#define NETIOZCOPYSharedMemPool_h

#include "netio_zcopy/netio_zcopy_sharedq_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NETIO_ZCOPY_SharedMemPoolImpl NETIO_ZCOPY_SharedMemPool;

/* Global values */

/*i \brief  Indicate auto maximum segment length of the MemPool */
#define NETIO_ZCOPY_SharedMemPool_max_segment_size_auto (0)

/*i
 * \brief Properties for a shared memory pool
 */
struct NETIO_ZCOPY_SharedMemPoolProperties
{
    /*i
     * \brief The maximum size of a segment in the pool
     */
    SQ_Length max_segment_size;

    /*i
     * \brief The maximum number of segments in the pool
     */
    SQ_Index max_segment_count;

    /*i
     * \brief The maximum number of consistent observers
     */
    SQ_Index max_consistent_obsrv_count;

    /*i
     * \brief Robust locking is used or not
     */
    RTI_BOOL robust_locking;
};

/*i
 * \brief Default properties for a shared memory pool
 */
extern const struct NETIO_ZCOPY_SharedMemPoolProperties
        NETIO_ZCOPY_SharedMemPoolProperties_DEFAULT;

/*i
 * \brief Generic version info struct for detecting compatibility when sharing
 */
 struct NETIO_ZCOPY_Version
{
    RTI_UINT8 major;
    RTI_UINT8 minor;
};

struct NETIO_ZCOPY_PIVersionInfo
{
    char id[2];
    struct NETIO_ZCOPY_Version version;
};


/*i
 * \brief Creates a new shared memory pool with the specified properties.
 *
 * \param[in] name The name of the shared memory pool.
 * \param[in] version_info The version information for the shared memory pool.
 * \param[in] admin_size The size of the admin area of the shared memory pool.
 * \param[in] elmt_count The number of elements in the shared memory pool.
 * \param[in] elmt_size The size of each element in the shared memory pool.
 * \param[in] properties The properties for the shared memory pool.
 * \param[out] mem_pool_out A pointer to the newly created shared memory pool.
 *
 * \return RTI_TRUE if the shared memory pool was successfully created,
 *         RTI_FALSE otherwise.
 *
 * \pre `name` must not be `NULL`.
 * \pre `version_info` must not be `NULL`.
 * \pre `elmt_count` must be greater than 0.
 * \pre `elmt_size` must be greater than 0.
 * \pre `mem_pool_out` must not be `NULL`.
 *
 * \post `*mem_pool_out` will be set to the newly created shared memory pool if
 *       the function returns RTI_TRUE.
 *
 * \note The caller is responsible for calling
 *       `NETIO_ZCOPY_SharedMemPool_destroy` to free the memory allocated for
 *       the shared memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_create(
        const char *name,
        const struct NETIO_ZCOPY_PIVersionInfo *version_info,
        SQ_Length admin_size,
        SQ_Index elmt_count,
        SQ_Length elmt_size,
        const struct NETIO_ZCOPY_SharedMemPoolProperties *properties,
        NETIO_ZCOPY_SharedMemPool **mem_pool_out);


/*i
 * \brief Destroys a shared memory pool.
 *
 * \param[in] self A pointer to the shared memory pool to be destroyed.
 *
 * \return RTI_TRUE if the shared memory pool was successfully destroyed,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 *
 * \note This function frees the memory allocated for the shared memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_destroy(NETIO_ZCOPY_SharedMemPool *self);

/*i
 * \brief Creates and initializes an object that can be used to attach to, and
 *        encapsulate, a previously created NETIO_ZCOPY_SharedMemPool object.
 *
 * \param[in] mode Level of consistency and robustness requested.
 * \param[out] mem_pool_out A pointer to hold a pointer to the resulting
 *             NETIO_ZCOPY_SharedMemPool object.
 *
 * \return RTI_TRUE if the shared memory pool was successfully initialized,
 *         RTI_FALSE otherwise.
 *
 * \pre `mem_pool_out` must not be `NULL`.
 *
 * \post `*mem_pool_out` will be set to the initialized shared memory pool if
 *       the function returns RTI_TRUE.
 *
 * \note The caller is responsible for calling `NETIO_ZCOPY_SharedMemPool_close`
 *       to close the shared memory pool.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_close
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_initialize(
        SQ_ConsistencyMode mode,
        NETIO_ZCOPY_SharedMemPool **mem_pool_out);

/*i
 * \brief Attaches to a previously created NETIO_ZCOPY_SharedMemPool object,
 *        by name.
 *
 * \param[in] self A pointer to this observer-side NETIO_ZCOPY_SharedMemPool
 *            object.
 * \param[in] name The name of the shared memory pool to which we will attach.
 * \param[out] is_connected_out A pointer to a boolean indicating whether
 *             attaching to shared memory succeeded
 * \param[out] elmt_count_out A pointer to return the number of elements this pool was
 *             created with.
 * \param[out] elmt_size_out A pointer to retun the size (in bytes) of each element in
 *             the shared memory pool.
 * \param[out] obsrv_count_out A Pointer to return the number of consistent
 *             observers this pool is able to have attached to it in strongly or
 *             robustly consistent mode.
 *
 * \return RTI_TRUE if the shared memory pool was successfully opened,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 * \pre `name` must not be `NULL`.
 * \pre `elmt_count_out` must not be `NULL`.
 * \pre `elmt_size_out` must not be `NULL`.
 * \pre `obsrv_count_out` must not be `NULL`.
 *
 * \post `*self` will be configured to the opened shared memory pool if the
 *       function returns RTI_TRUE.
 * \post `*is_connected_out` will be set to RTI_TRUE if attaching to the shared
 *       memory segment succeeded, and RTI_FALSE otherwise.
 * \post `*elmt_count_out` will be set to the number of elements in the shared
 *       memory pool (or 0 if is_connected is FALSE) if the function returns RTI_TRUE.
 * \post `*elmt_size_out` will be set to the size of each element in the shared
 *       memory pool (or 0 if is_connected is FALSE) if the function returns RTI_TRUE.
 * \post `*obsrv_count_out` will be set to the number of observers of the shared
 *       memory pool (or 0 if is_connected is FALSE) if the function returns RTI_TRUE.
 *
 * \note The caller is responsible for calling `NETIO_ZCOPY_SharedMemPool_close`
 *       to close the shared memory pool.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_close
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_open(
        NETIO_ZCOPY_SharedMemPool *self,
        const char *name,
        RTI_BOOL *is_connected_out,
        SQ_Index *elmt_count_out,
        SQ_Length *elmt_size_out,
        SQ_Index *obsrv_count_out);

/*i
 * \brief Detaches from the SharedMemPool data and destroys the encapsulating
 *        object.
 *
 * \details Detaches from all shared memory segments attached to during the
 *          open() function, clears the observer slot associated with this
 *          object if the consistency mode was STRONG or ROBUST and releases the
 *          heap memory associated with the NETIO_ZCOPY_SharedMemPool object.
 *
 * \param[in] self A pointer to NETIO_ZCOPY_SharedMemPool object to be
 *            destroyed.
 *
 * \return RTI_TRUE if the shared memory pool was successfully closed,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_open
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_close(NETIO_ZCOPY_SharedMemPool *self);


/*i
 * \brief Claims exclusive access to the admin shared memory region of a
 *        NETIO_ZCOPY_SharedMemPool.
 *
 * \details This function requests exclusive access to the memory region. If
 *          another process already has claimed access, then the calling thread
 *          will block until exclusive access is granted, either because the
 *          claim was released by the other process, or, in case of a
 *          NETIO_ZCOPY_SharedMemPool configured with robust locking, because
 *          the process owning the claim has died.
 *
 * \param[in] self A pointer to NETIO_ZCOPY_SharedMemPool object to which
 *            exclusive access is to be claimed.
 *
 * \return RTI_TRUE if a free element was successfully claimed,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_release
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_claim(NETIO_ZCOPY_SharedMemPool *self);

/*i
 * \brief Releases exclusive access to the admin shared memory region of a
 *        NETIO_ZCOPY_SharedMemPool.
 *
 * \param[in] self A pointer to NETIO_ZCOPY_SharedMemPool object to which
 *            exclusive access is to be released.
 *
 * \return RTI_TRUE if the admin shared memory region was successfully released,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 *
 * \note This function releases the exclusive access to the admin shared memory
 *       region of a NETIO_ZCOPY_SharedMemPool. The caller must have previously
 *       claimed a free element using `NETIO_ZCOPY_SharedMemPool_claim`.
 *
 * \sa \ref NETIO_ZCOPY_SharedMemPool_claim
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_release(NETIO_ZCOPY_SharedMemPool *self);

/* Getters */

/*i
 * \brief Getter function returns the administration-related properties for any
 *        NETIO_ZCOPY_SharedMemPool.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_SharedMemPool object from which
 *            to retrieve the admin data.
 * \param[out] user_admin_size_out A pointer to hold the size in bytes of the
 *             reserved administration space.
 * \param[out] user_admin_out A pointer to hold a pointer to the memory region
 *             reserved for administration.
 *
 * \return RTI_TRUE if the user-defined admin data was successfully retrieved,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 * \pre `user_admin_size_out` must not be `NULL`.
 * \pre `user_admin_out` must not be `NULL`.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_admin_info(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Length *user_admin_size_out,
        unsigned char **user_admin_out);

/*i
 * \brief Getter function returns the version information for a
 *        NETIO_ZCOPY_SharedMemPool.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_SharedMemPool object from which
 *            to retrieve the version information.
 * \param[out] version_info_out A pointer to a struct NETIO_ZCOPY_PIVersionInfo
 *             object to hold the version information.
 *
 * \return RTI_TRUE if the version information was successfully retrieved,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 * \pre `version_info_out` must not be `NULL`.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_version_info(
        NETIO_ZCOPY_SharedMemPool *self,
        struct NETIO_ZCOPY_PIVersionInfo *version_info_out);


/*i
 * \brief Getter function returns an element at a specified index from any
 *        NETIO_ZCOPY_SharedMemPool.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_SharedMemPool object from which
 *            to retrieve the element.
 * \param[in] elmt_index The index of the element to retrieve.
 * \param[out] elmt_size_out A pointer to hold the size in bytes of the
 *             retrieved element.
 * \param[out] elmt_out A pointer to hold a pointer to the memory region
 *             reserved for the element.
 *
 * \return RTI_TRUE if the element was successfully retrieved,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 * \pre `elmt_size_out` must not be `NULL`.
 * \pre `elmt_out` must not be `NULL`.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_element(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Index elmt_index,
        SQ_Length *elmt_size_out,
        unsigned char **elmt_out);


/*i
 * \brief Getter functions allows observers that are attached with consistency
 *        to query their position in the array of attached consistent observers.
 *
 * \param[in] self A pointer to a NETIO_ZCOPY_SharedMemPool object to which the
 *            observer is attached with consistency.
 * \param[out] obsrv_index_out A pointer to an SQ_Index object to hold the
 *             observer index.
 *
 * \return RTI_TRUE if the observer index was successfully retrieved,
 *         RTI_FALSE otherwise.
 *
 * \pre `self` must not be `NULL`.
 * \pre `obsrv_index_out` must not be `NULL`.
 */
RTI_BOOL
NETIO_ZCOPY_SharedMemPool_get_observer_index(
        NETIO_ZCOPY_SharedMemPool *self,
        SQ_Index *obsrv_index_out);

/*i
 * \brief Reset the value in the mempool structure to its default values.
 *
 * \param[in] self A pointer to a NETIO_ZCOPY_SharedMemPool
 * \return void
 */
void
NETIO_ZCOPY_SharedMemPool_reset(NETIO_ZCOPY_SharedMemPool *self);


/*i
 * \brief Gets the default value for shared memory pool properties.
 *
 * \param[in] self A pointer to a NETIO_ZCOPY_SharedMemPool. Expected to be a valid pointer (not NULL)
 */
void
NETIO_ZCOPY_SharedMemPool_get_default_properties(struct NETIO_ZCOPY_SharedMemPoolProperties* properties_out);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETIOZCOPYSharedMemPool_h */

/*i @} */
