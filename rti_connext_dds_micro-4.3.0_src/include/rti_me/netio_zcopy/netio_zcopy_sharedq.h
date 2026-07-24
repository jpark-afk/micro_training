/*
 * FILE: netio_zcopy_sharedq.h - Zero Copy sample queue -- interface
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

/*ci
 * \defgroup NETIO_ZCOPY_SharedQGroup NETIO_ZCOPY_SharedQ
 * @{
 */
/*ci
 * \file
 * \brief Zero Copy Shared queue API -- declarations
 * \details All types that live in shared memory should be Position Independent
 *          (PI). The set of functions below are used for mapping local types
 *          into shared memory (for the writer side) and reconstructing PI types
 *          from shared memory into local types.
 */
#ifndef NETIOZCOPYSharedQ_h
#define NETIOZCOPYSharedQ_h

#include "netio_zcopy/netio_zcopy_sharedq_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NETIO_ZCOPY_SharedQWriterImpl NETIO_ZCOPY_SharedQWriter;
typedef struct NETIO_ZCOPY_SharedQReaderImpl NETIO_ZCOPY_SharedQReader;

typedef RTI_UINT32 SQ_WriterSampleId;
typedef RTI_UINT32 SQ_ReaderSampleId;
typedef RTI_UINT64 SQ_SampleSeqNr;

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */

/*i
 * \brief Creates a new shared queue writer with the given parameters.
 *
 * \param[in] owner_key A pointer to a key identifying the shared queue.
 * \param[in] domain_id The ID for the DDS Domain in which this writer will
 *            operate.
 * \param[in] sample_count The number of samples in the shared queue.
 * \param[in] user_data_size The size of the user data in each sample.
 * \param[in] obsrv_count The maximum number of observers for the shared queue.
 * \param[in] protocol_version The version of the protocol to use, or
 *                SQ_VERSION_NUMBER_NONE for default.
 * \param[out] sharedq_writer_out A pointer to the created shared queue writer.
 *
 * \return RTI_TRUE if the shared queue writer was created successfully,
 *         RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQWriter_destroy
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_create(
        const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id,
        SQ_Index sample_count,
        SQ_Length user_data_size,
        SQ_Index obsrv_count,
        SQ_VersionNumber protocol_version,
        NETIO_ZCOPY_SharedQWriter **sharedq_writer_out);

/*i
 * \brief Creates a new shared queue writer with the given parameters and name.
 *
 * \param[in] name The name of the shared queue mem pool own by the writer.
 * \param[in] sample_count The number of samples in the shared queue.
 * \param[in] user_data_size The size of the user data in each sample.
 * \param[in] obsrv_count The number of observers for the shared queue.
 * \param[in] protocol_version The version of the protocol to use, or
 *                SQ_VERSION_NUMBER_NONE for default.
 * \param[out] sharedq_writer_out A pointer to a point to the created shared
 *             queue writer.
 *
 * \return RTI_TRUE if the shared queue writer was created successfully,
 *         RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQWriter_destroy
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_create_by_name(
        const char *name,
        SQ_Index sample_count,
        SQ_Length user_data_size,
        SQ_Index obsrv_count,
        SQ_VersionNumber protocol_version,
        NETIO_ZCOPY_SharedQWriter **sharedq_writer_out);


#ifndef RTI_CERT
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_destroy(NETIO_ZCOPY_SharedQWriter *self);
#endif /* !RTI_CERT */

/*i
 * \brief Helper function that returns the memory size of the
 *        NETIO_ZCOPY_SharedQReader object. Memory of that size can be used when
 *        initializing the object with the NETIO_ZCOPY_SharedQReader_initialize
 *        function.
 *
 * \return The size in bytes of a NETIO_ZCOPY_SharedQReader object.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_initialize
 */
RTI_SIZE_T
NETIO_ZCOPY_SharedQReader_get_size(void);

/*i
 * \brief Initializes a shared queue reader.
 *
 * \param[in] self The shared queue reader to initialize.
 * \param[in] mode The consistency mode to use.
 *
 * \return RTI_TRUE if the shared queue reader was initialized successfully,
 *         RTI_FALSE otherwise.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_initialize(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ConsistencyMode mode);

/*i
 * \brief Attaches this reader to an existing SharedQ data structure by owner
 *        key and domain ID.
 *
 * \param[in] self The shared queue reader to be opened.
 * \param[in] owner_key A guid uniquely identifying the SharedQ to which this
 *            reader will attach.
 * \param[in] domain_id The ID for the DDS Domain in which this reader will
 *            operate.
 *
 * \return RTI_TRUE if the shared queue reader was opened successfully,
 *         RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_open_by_name
 * \sa \ref NETIO_ZCOPY_SharedQReader_close
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_open(
        NETIO_ZCOPY_SharedQReader *self,
        const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id);

/*i
 * \brief Detached this reader from the sharedQ data structure.
 *
 * This function detaches the shared queue reader from sharedQ data structure,
 * including the consistent set, shared queue admin, and shared memory pool.
 *
 * \param[in] self Pointer to the shared queue reader to be closed.
 *
 * \return RTI_TRUE if the shared queue reader was successfully closed and
 *         resources were released, RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_open
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_close(NETIO_ZCOPY_SharedQReader *self);

#ifndef RTI_CERT

/*i
 * \brief Close and free this shared queue reader.
 *
 * \param[in] self The shared queue reader to finalize.
 *
 * \return RTI_TRUE if the shared queue reader was destroyed successfully,
 *         RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_open
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_finalize(NETIO_ZCOPY_SharedQReader *self);
#endif /* !RTI_CERT */

/* ---------------------
 * Writer-side functions
 * ---------------------
 */

/* Functions expected to be used on the writer side of the SharedQ only,
 * because the writer manages the contents and therefore knows what
 * is needed to look up
 */

/*i
 * \brief Request a pointer to a user data memory region from the SharedQ's pool
 *        of free Samples.
 *
 * \param[in] self The shared queue writer to get the loan from.
 * \param[out] user_data_size_out A pointer to hold the size (in bytes) of the
 *             loaned data region.
 * \param[out] user_data_out A pointer to hold a pointer to the loaned data
 *             region.
 * \param[out] sample_id_out A pointer to hold the unique ID of the sample being
 *             loaned.
 *
 * \return SQ_RETCODE_OK if the loan was successfully obtained,
 *         SQ_RETCODE_OUT_OF_RESOURCES if the loan could not be obtained,
 *         SQ_RETCODE_ERROR otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQWriter_discard_loan
 */
SQ_ReturnCode_t
NETIO_ZCOPY_SharedQWriter_get_loan(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_Length *user_data_size_out,
        SQ_MemPtr *user_data_out,
        SQ_WriterSampleId *sample_id_out);

/*i
 * \brief Return a user data memory region identified by <sample_id> to the free
 *        pool.
 *
 * Return a previously loaned user data memory region to the free pool managed
 * by this writer.
 *
 * \param[in] self The shared queue writer object.
 * \param[in] sample_id The unique ID of the sample to discard.
 *
 * \return RTI_TRUE if the loan was successfully discarded, RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQWriter_get_loan
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_discard_loan(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id);

/*i
 * \brief Lookup the unique ID of a sample given its user data memory region.
 *
 * \param[in] self The shared queue writer object.
 * \param[in] user_data The user data memory region to look up.
 * \param[out] sample_id_out A pointer to hold the unique ID of the sample.
 *
 * \return RTI_TRUE if the sample was found, RTI_FALSE otherwise.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_lookup_loaned_sample_id(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_MemPtr_Const user_data,
        SQ_WriterSampleId *sample_id_out);

/*i
 * \brief Commit a loaned user data region and its metadata to the SharedQ.
 *
 * \param[in] self The shared queue writer object.
 * \param[in] sample_id The ID of the sample to commit, as previously returned
 *            from NETIO_ZCOPY_SharedQWriter_get_loan.
 * \param[out] sample_info A pointer to a struct holding sample metadata to be
 *             associated with it in the SharedQ.
 *
 * \return RTI_TRUE if the sample was found, RTI_FALSE otherwise.
 *
 * \note This action makes the Sample visible to any observers of the SharedQ.
 *
 * \sa \ref NETIO_ZCOPY_SharedQWriter_discard_loan,
 *     \ref NETIO_ZCOPY_SharedQWriter_get_loan
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_commit(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id,
        const struct SQ_SampleInfo *sample_info);

/*i
 * \brief Remove a committed sample from SharedQ's valid sample list and return
 *        the memory region for eventual reuse.
 *
 * \param[in] self The shared queue writer object.
 * \param[in] sample_id The unique ID of the sample to purge.
 *
 * \return RTI_TRUE if the sample was purged, RTI_FALSE otherwise.
 *
 * \note This action makes the Sample invisible to any observers of the SharedQ.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_purge(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id);

/*i
 * \brief Assert the writer's liveliness without updating any Sample in the
 *        sharedQ.
 *
 * \param[in] self The shared queue writer object.
 * \param[in] pulse_timestamp The timestamp to associate with the pulse.
 *
 * \return RTI_TRUE if the pulse was successfully sent, RTI_FALSE otherwise.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_pulse(
        NETIO_ZCOPY_SharedQWriter *self,
        const OSAPI_SystemTime *pulse_timestamp);

/* ---------------------
 * Reader-side functions
 * --------------------- */

/*i
 * \brief Function prototype for visitor callback function for the peek_unseen
 *        functionality.
 */
typedef RTI_BOOL (*NETIO_ZCOPY_VisitReaderSampleFunc)(
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr sample_seq_nr,
        const struct SQ_SampleInfo *sample_info,
        void *visit_data);

/*i
 * \brief Walk over all samples that have been committed by the writer and have
 *        not been peeked before.
 *
 * \param[in] self The shared queue reader object.
 * \param[in] visit_func The visitor function to call on each unseen element in
 *            order that they were committed until visit_func returns RTI_FALSE.
 * \note When visit_func returns RTI_FALSE, the sample being iterated over is
 *       still considered unseen.
 * \param[in] visit_data The user-provided data to pass to the visitor function.
 *
 * \return RTI_TRUE if the peek was successful, RTI_FALSE otherwise.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_peek_unseen(
        NETIO_ZCOPY_SharedQReader *self,
        NETIO_ZCOPY_VisitReaderSampleFunc visit_func,
        void *visit_data);

/*i
 * \brief Get access to the user data associated with a sample ID.
 *
 * \param[in] self Pointer to the reader-side object encapsulating the SharedQ.
 * \param[in] sample_id The ID of the sample to loan.
 * \param[in] elmt_id Identifier of the element that will need to be protected.
 * \param[out] blob_length_out The length of the data memory region loaned.
 * \param[out] blob_out The pointer to the data memory region loaned.
 *
 * \return RTI_TRUE if the loan was successful, RTI_FALSE otherwise.
 *
 * \note If the SharedQ Reader was attached with consistency, the user data will
 *       be protected from reuse until the loan is returned.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_return_loan
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_get_loan(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr elmt_id,
        SQ_Length *blob_length_out,
        SQ_MemPtr *blob_out);

/*i
 * \brief Return a previously loaned sample and mark it as unused.
 *
 * \param[in] self The shared queue reader object.
 * \param[in] sample_id The ID of the sample to be returned.
 *
 * \return RTI_TRUE if the loan was successfully returned, RTI_FALSE otherwise.
 *
 * \sa \ref NETIO_ZCOPY_SharedQReader_get_loan
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_return_loan(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id);

/*i
 * \brief Check whether index/id mapping is still correct
 *
 * \param[in] self The shared queue reader object.
 * \param[in] sample_id The ID of the sample to be checked.
 * \param[in] elmt_id The ID of the element location believed by the caller
 *            to have data matching sample_id
 * \param[out] is_consistent_out A pointer to hold the result of the check.
 *
 * \return RTI_TRUE if the check was successful, RTI_FALSE otherwise.
 *
 * \note Intended for observers without strong consistency
 */
RTI_BOOL
NETIO_ZCOPY_SharedQReader_get_is_consistent(
        const NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr elmt_id,
        RTI_BOOL *is_consistent_out);


/*i
 * \brief Check whether the shared queue can be attached to
 *
 * \param[in] owner_key A pointer to a key identifying the shared queue.
 * \param[in] domain_id The ID for the DDS Domain in which this writer will
 *            operate.
 *
 * \return RTI_TRUE if we can attach to the Shared queue, otherwise return RTI_FALSE
*/
RTI_BOOL
NETIO_ZCOPY_SharedQ_can_attach(
	    const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id);


/*i
 * \brief Reset the shared queue reader to its default value
 *
 * \param[in] self A pointer to the shared queue reader
 *
 * \return void
*/
void
NETIO_ZCOPY_SharedQReader_reset( NETIO_ZCOPY_SharedQReader *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETIOZCOPYSharedQ_h */

/*ci @} */
