/*
 * FILE: NETIOZCOPYSharedQAdmin.h - Admin structure encapsulation -- interface
 *
 * (c) Copyright 2022-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*i
 * \defgroup NETIO_ZCOPY_SharedQAdminClass NETIO_ZCOPY_SharedQAdmin
 * @{
 */

/*i
 * \file
 * \brief SharedQ Admin encapsulation -- interface
 */
#ifndef NETIOZCOPYSharedQAdmin_h
#define NETIOZCOPYSharedQAdmin_h

#include "netio_zcopy/netio_zcopy_sharedq_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ----------------
 * Type definitions
 * ----------------
 */

/*i
 * \brief SharedQ Admin structure
 */
typedef struct NETIO_ZCOPY_SharedQAdminImpl NETIO_ZCOPY_SharedQAdmin;

/*i
 * \brief For values uniquely identifying Samples
 */
typedef RTI_UINT64 SQ_SampleId_t;

/* ----------------
 * Class operations
 * ----------------
 */

/*i
 * \brief Get the size in bytes of the memory needed for a position independent
 *        (PI) shared data structure (SQ_PI structures), given the number of
 *        samples.
 *
 * \param[in] sample_count Max number of samples in the SharedQ
 * \param[out] length_out Length of the memory in bytes needed for the SharedQ
 *             Admin
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_get_mem_size(SQ_Index sample_count, SQ_Length *length_out);

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */

/*i
 * \brief Constructor for the encapsulated SharedQ Admin structure
 *
 * \param[in] sample_count Max number of samples in the SharedQ
 * \param[in] flat_memory_length Length of the memory in bytes for memory region
 *            pointed to by <flat_memory>
 * \param[in] flat_memory Pointer to memory region to hold the PI structure
 * \param[out] self_out Pointer to hold pointer to the resulting SharedQ Admin
 *             object
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \sa \ref NETIO_ZCOPY_SharedQAdmin_destroy
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_create(
        SQ_Index sample_count,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory /*[flat_memory_length]*/,
        NETIO_ZCOPY_SharedQAdmin **self_out);

/*i
 * \brief Destructor for the encapsulated SharedQ Admin structure
 *
 * This function destroys the heap memory allocated when the object was created.
 *
 * \param[in] self Pointer to the SharedQ Admin object to be destroyed
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \sa \ref NETIO_ZCOPY_SharedQAdmin_create
 *
 * \note Do nothing if <self> is NULL
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_destroy(NETIO_ZCOPY_SharedQAdmin *self);

/*i
 * \brief Constructor that creates the read-only NETIO_ZCOPY_SharedQAdmin object
 *        that can be used to encapsulate an existing PI data structure.
 *
 * \param[out] self_out Pointer to hold pointer to the resulting SharedQ Admin
 *             object
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \sa \ref NETIO_ZCOPY_SharedQAdmin_close
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_initialize(NETIO_ZCOPY_SharedQAdmin **self_out);

/*i
 * \brief Configure an existing NETIO_ZCOPY_SharedQAdmin object to encapsulate
 *        an existing PI data structure.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_SharedQAdmin object to be
 *            configured
 * \param[in] flat_memory_length Length of the memory in bytes for memory region
 *            pointed to by <flat_memory>
 * \param[in] flat_memory Pointer to memory region holding the existing PI
 *            structure
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \sa \ref NETIO_ZCOPY_SharedQAdmin_close
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_open(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory);

/*i
 * \brief Detach the encapsulated SharedQ Admin structure from existing PI data
 *        structure and destroy the object.
 *
 * \param[in] self Pointer to the SharedQ Admin object to be destroyed
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \sa \ref NETIO_ZCOPY_SharedQAdmin_open
 *
 * \note Do nothing if <self> is NULL
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_close(NETIO_ZCOPY_SharedQAdmin *self);

/* ---------------------
 * Writer-side functions
 * ---------------------
 */

/* Functions expected to be used on the writer side of the SharedQ only,
 *   because the writer manages the contents and therefore knows what
 *   is needed to look up */

/*i
 * \brief Associates <sample_info> with a sample at specified index and mark it
 *        as valid
 *
 * \param[in] self Pointer to the SharedQ Admin object
 * \param[in] sample_index Index of the sample in the list of all samples
 * \param[in] sample_info Pointer to the sample info (meta-data) to be
 *            associated with the sample
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_commit_sample(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index,
        const struct SQ_SampleInfo *sample_info);
/*i
 * \brief Mark the sample at the specified index as invalid.
 *
 * \param[in] self Pointer to the SharedQ Admin object
 * \param[in] sample_index Index of the sample in the list of all samples
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_purge_sample(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index);

/*i
 * \brief Update liveliness timestamp on PI sharedQ admin owned by the writer.
 *
 * This action will be visible as an update to its observers.
 *
 * \param[in] self Pointer to the SharedQ Admin object
 * \param[in] pulse_timestamp Pointer to the liveliness timestamp to update PI
 *            sharedQ admin
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_pulse(
        NETIO_ZCOPY_SharedQAdmin *self,
        const OSAPI_SystemTime *pulse_timestamp);

/* ---------------------
 * Reader-side functions
 * ---------------------
 */

/*i
 * \brief Get the sample info (meta-data) of the sample specified by
 *        <sample_index>
 *
 * \param[in] self Pointer to the SharedQ Admin object
 * \param[in] sample_index Index of the sample in the list of all samples
 * \param[out] sample_info_out Pointer to hold the sample info of the specified
 *             sample
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_reader_get_sample_info(
        const NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index,
        struct SQ_SampleInfo *sample_info_out);

/*i
 * \brief Get the number of pulses since the latest commit and the sample info
 *        of the pulse sample.
 *
 * \param[in] self Pointer to the SharedQ Admin object
 * \param[out] pulses_since_latest_commit_out Pointer to hold the number of
 *             pulses since the latest commit
 * \param[out] sample_info_out Pointer to hold the sample info of the pulse
 *             sample
 *
 * \return RTI_TRUE if the operation was successful, RTI_FALSE otherwise
 *
 * \pre This process must first acquire the lock associated with the shared
 *      memory pool.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_reader_get_pulse_info(
        const NETIO_ZCOPY_SharedQAdmin *self,
        SQ_SampleId_t *pulses_since_latest_commit_out,
        struct SQ_SampleInfo *sample_info_out);


/*i
 * \brief Reset the SharedQ Admin to its default values
 *
 * \param[in] self Pointer to the SharedQ Admin object
 *
 * \return void
 */
void
NETIO_ZCOPY_SharedQAdmin_reset(NETIO_ZCOPY_SharedQAdmin *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETIOZCOPYSharedQAdmin_h */

/*i @} */
