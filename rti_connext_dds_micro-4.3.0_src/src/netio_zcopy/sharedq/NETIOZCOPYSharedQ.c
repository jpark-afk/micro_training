/*
 * FILE: NETIOZCOPYSharedQ.c - Zero Copy sample queue -- implementation
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
 * \addtogroup NETIO_ZCOPY_SharedQGroup
 * @{
 */

/*i
 * \file
 * \brief Zero Copy Shared queue API -- implementation
 */
/* Interface */
#include "netio_zcopy/netio_zcopy_sharedq.h"

/* Implementation */
#include "NETIOZCOPYSharedMemPool.h"
#include "NETIOZCOPYConsistentSet.h"
#include "NETIOZCOPYSharedQAdmin.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_system.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#if OSAPI_ENABLE_LOG
#include "dds_c/dds_c_log.h"
#endif

/* --------------------------------------------
 * Basic Position Independent (PI) SharedQ info
 * --------------------------------------------
 */
 

/*i \brief For checking compatibility of an SharedQ in shmem */
RTI_PRIVATE
const struct NETIO_ZCOPY_PIVersionInfo SQ_fv_MyVersion =
{
    .id = {'S', 'Q'},
    .version =
    {
        .major = 2,
        .minor = 0,
    },
};

/* --------------------
 * Local (heap) SharedQ
 * --------------------
 */
/*i
 * \brief Struct implementing SharedQ Writer side
 */
struct NETIO_ZCOPY_SharedQWriterImpl
{
    /*i \brief For managing the pool of shared memory elements, which is own */
    NETIO_ZCOPY_SharedMemPool *mem_pool;
    /*i \brief Object holding the sample administration elements */
    NETIO_ZCOPY_SharedQAdmin *qadmin;
    /*i
     * \brief Object for robustly sharing a consistent collection of elements,
     *        owner side
     */
    NETIO_ZCOPY_ConsistentSetOwned *consistent_set;
};
/*i
 * \brief SharedQ sample mapping
 */
struct SQ_ElmtSampleMapping
{
    SQ_Index index_in_consistent_set;
    SQ_ElmtId elmt_id;
};

/*i
 * \brief Struct implementing SharedQ Reader side
 */
struct NETIO_ZCOPY_SharedQReaderImpl
{
    /*i
     * \brief For attaching to the pool of shared memory elements
     */
    NETIO_ZCOPY_SharedMemPool *mem_pool;

    /*i
     * \brief Object holding the sample administration elements
     */
    NETIO_ZCOPY_SharedQAdmin *qadmin;

    /*i
     * \brief Object for robustly sharing a consistent collection of elements,
     *        observer side
     */
    NETIO_ZCOPY_ConsistentSetObserved *consistent_set;

    /*i
     * \brief Keep track of latest element ID seen by this reader (when it was
     *        iterated over)
     */
    SQ_ElmtId latest_elmt_id;

    /*i
     * \brief Keep track of pulses seen by this reader
     */
    SQ_ElmtId pulses_seen_since_last_commit;

    /*i
     * \brief Keep track of latest element Index seen by this reader (when it
     *        was iterated over)
     */
    SQ_Index latest_elmt_index;

    /*i
     * \brief Consistency mode for this reader
     */
    SQ_ConsistencyMode mode;

    /*i
     * \brief Hold on to the index-id mappings to track if element got updated
     *        in the meantime
     */
    SQ_Index elmt_count;

    /*i
     * \brief A boolean to track whether this reader has been able to set up
     *        a connection (attachment) to its writer
     */
    RTI_BOOL is_open;

    /*i
     * \brief A boolean to indicate that we gave up on opening the SharedQ
     *        because it was expected to be available but we did not succeed
     *        in opening it and trying again would be futile but expensive.
     */
    RTI_BOOL give_up_on_open;

    /*i
     * \brief Storage space for the name of the shared memory segment,
     *        in case we need to retry attaching
     */
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
};

/*i
 * \brief Constant for the base name of the sharedQueue.
 */
#define SQ_BASE_NAME "SQ"

/*i
 * \brief Helper type for visitor function
 */
struct SQ_VisitElmtArg
{
    NETIO_ZCOPY_SharedQReader *self;
    NETIO_ZCOPY_VisitReaderSampleFunc visit_func;
    void *visit_data;
    RTI_BOOL terminated;
};


/*** SOURCE_BEGIN ***/

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */

/*i
 * \brief Initializes a shared memory element with data from a existing shared
 *        memory object at specific index.
 *
 * \param[in] elmt_index The index of the shared memory element from which to
 *            get data.
 * \param[out] elmt_data_length_out A pointer to the length of the initializing
 *             data.
 * \param[out] elmt_data_out A pointer to the data to be used to initialize.
 * \param[out] initializer_param A pointer to the initialized shared memory pool
 *             if successful.
 */
RTI_PRIVATE void
element_initializer(
        SQ_Index elmt_index,
        SQ_Length *elmt_data_length_out,
        SQ_MemPtr *elmt_data_out,
        void *initializer_param)
{
    NETIO_ZCOPY_SharedMemPool *mem_pool =
        (NETIO_ZCOPY_SharedMemPool *)initializer_param;
    SQ_Length data_length;
    SQ_MemPtr data;

    if (NETIO_ZCOPY_SharedMemPool_get_element(
            mem_pool, elmt_index, &data_length, &data))
    {
        *elmt_data_length_out = data_length;
        *elmt_data_out = data;
    }
}

RTI_BOOL
NETIO_ZCOPY_SharedQWriter_create(
        const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id,
        SQ_Index sample_count,
        SQ_Length user_data_size,
        SQ_Index obsrv_count,
        SQ_VersionNumber protocol_version,
        NETIO_ZCOPY_SharedQWriter **sharedq_writer_out)
{
    RTI_BOOL result = RTI_FALSE;
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    OSAPI_PRECONDITION_ALWAYS(
            (sharedq_writer_out == NULL) || (owner_key == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("sharedq_writer_out", sharedq_writer_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("owner_key", owner_key, RTI_TRUE);)

    /* Construct name of SharedQ */
    if (!ZCOPY_Guid_to_string(
                owner_key,
                domain_id,
                SQ_BASE_NAME,
                name,
                sizeof(name)))
    {
        goto done;
    }

    result = NETIO_ZCOPY_SharedQWriter_create_by_name(name, sample_count,
            user_data_size, obsrv_count, protocol_version, sharedq_writer_out);

done:
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQWriter_create_by_name(
        const char *name,
        SQ_Index sample_count,
        SQ_Length user_data_size,
        SQ_Index obsrv_count,
        SQ_VersionNumber protocol_version,
        NETIO_ZCOPY_SharedQWriter **sharedq_writer_out)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL retval;
    NETIO_ZCOPY_SharedQWriter *writer = NULL;
    NETIO_ZCOPY_SharedMemPool *mem_pool = NULL;
    NETIO_ZCOPY_ConsistentSetOwned *consistent_set = NULL;
    SQ_Length consistent_set_length;
    NETIO_ZCOPY_SharedQAdmin *qadmin = NULL;
    SQ_Length qadmin_length;
    struct NETIO_ZCOPY_SharedMemPoolProperties props;
    unsigned char *user_admin;
    SQ_Length user_admin_size;
    RTI_BOOL use_timestamp_as_id = RTI_FALSE;
    struct NETIO_ZCOPY_PIVersionInfo version_info = SQ_fv_MyVersion;

    RTI_PRIVATE const struct NETIO_ZCOPY_Version NETIO_ZCOPY_TSVERSION[] =
    {
        {.major = 0, .minor = 1},
        {.major = 1, .minor = 0},
    };
    RTI_PRIVATE SQ_VersionNumber NETIO_ZCOPY_TSVCOUNT =
        sizeof(NETIO_ZCOPY_TSVERSION) /
        sizeof(NETIO_ZCOPY_TSVERSION[0]);

    if (protocol_version < NETIO_ZCOPY_TSVCOUNT)
    {
        version_info.version = NETIO_ZCOPY_TSVERSION[protocol_version];
        use_timestamp_as_id = RTI_TRUE;
    }

#if OSAPI_ENABLE_LOG
    if (protocol_version > NETIO_ZCOPY_TSVCOUNT)
    {
        /* At this point, this is a warning only and the latest known
         * protocol version will be used.
         */
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_DATAWRITER_ENTITY)
    }
#endif

    NETIO_ZCOPY_SharedMemPool_get_default_properties(&props);

    if (!NETIO_ZCOPY_SharedQAdmin_get_mem_size(sample_count, &qadmin_length) ||
        !NETIO_ZCOPY_ConsistentSet_get_mem_size(
                sample_count,
                obsrv_count,
                &consistent_set_length))
    {
        goto done;
    }

    /* configure properties with the caller-supplied observer count */
    props.max_consistent_obsrv_count = obsrv_count;

    if (!NETIO_ZCOPY_SharedMemPool_create(
                name,
                &version_info,
                consistent_set_length + qadmin_length,
                sample_count,
                user_data_size,
                &props,
                &mem_pool))
    {
        goto done;
    }

    if (!NETIO_ZCOPY_SharedMemPool_get_admin_info(
                mem_pool,
                &user_admin_size,
                &user_admin))
    {
        goto done;
    }

    if (!NETIO_ZCOPY_SharedQAdmin_create(
                sample_count,
                qadmin_length,
                &user_admin[0],
                &qadmin))
    {
        goto done;
    }

    if (!NETIO_ZCOPY_ConsistentSetOwned_create(
                sample_count,
                obsrv_count,
                consistent_set_length,
                &user_admin[qadmin_length],
                use_timestamp_as_id,
                element_initializer,
                mem_pool,
                &consistent_set))
    {
        goto done;
    }

    OSAPI_Heap_allocate_struct(&writer, NETIO_ZCOPY_SharedQWriter);
    if (writer == NULL)
    {
        goto done;
    }
    *writer = (NETIO_ZCOPY_SharedQWriter)
    {
        .mem_pool = mem_pool,
        .qadmin = qadmin,
        .consistent_set = consistent_set,
    };

    *sharedq_writer_out = writer;
    result = RTI_TRUE;

done:
    if (!result)
    {
        /* return values are ignored here because our presence in this block
         * indicates a fatal error
         */

        retval = NETIO_ZCOPY_ConsistentSetOwned_destroy(consistent_set);
        IGNORE_RETVAL(retval);
        retval = NETIO_ZCOPY_SharedQAdmin_destroy(qadmin);
        IGNORE_RETVAL(retval);
        retval = NETIO_ZCOPY_SharedMemPool_destroy(mem_pool);
        IGNORE_RETVAL(retval);
    }
    /* In the error case, various memory is leaked here only when compiling with
     * RTI_CERT because memory is intentionally not freed for cert.
     */
    /* coverity[leaked_storage] */
    return result;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_ZCOPY_SharedQWriter_destroy(NETIO_ZCOPY_SharedQWriter *self)
{
    RTI_BOOL result = RTI_TRUE;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            result = RTI_FALSE; goto done;,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* capture an intermediate return of RTI_FALSE, but continue to attempt to
     * release resources
     */
    result &= NETIO_ZCOPY_ConsistentSetOwned_destroy(self->consistent_set);
    result &= NETIO_ZCOPY_SharedQAdmin_destroy(self->qadmin);
    result &= NETIO_ZCOPY_SharedMemPool_destroy(self->mem_pool);

    OSAPI_Heap_free_struct(self);

done:
    return result;
}
#endif /* !RTI_CERT */

void
NETIO_ZCOPY_SharedQReader_reset( NETIO_ZCOPY_SharedQReader *self)
{
    OSAPI_PRECONDITION(
            (self == NULL),
            return,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)
    NETIO_ZCOPY_SharedMemPool_reset(self->mem_pool);
    NETIO_ZCOPY_SharedQAdmin_reset(self->qadmin);
    NETIO_ZCOPY_ConsistentSetObserved_reset(self->consistent_set);
    self->latest_elmt_id = 0;
    self->latest_elmt_index = SQ_INDEX_NONE;
    self->pulses_seen_since_last_commit = 0;
    self->elmt_count = 0;
    self->is_open = RTI_FALSE;
    self->give_up_on_open = RTI_FALSE;
}

RTI_BOOL
NETIO_ZCOPY_SharedQReader_initialize(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ConsistencyMode mode)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL retval;
    NETIO_ZCOPY_SharedMemPool *mem_pool = NULL;
    NETIO_ZCOPY_SharedQAdmin *sq_admin = NULL;
    NETIO_ZCOPY_ConsistentSetObserved *consistent_set = NULL;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if(!NETIO_ZCOPY_SharedMemPool_initialize(mode, &mem_pool))
    {
        goto done;
    }

    if(!NETIO_ZCOPY_SharedQAdmin_initialize(&sq_admin))
    {
        goto done;
    }

    if(!NETIO_ZCOPY_ConsistentSetObserved_initialize(&consistent_set))
    {
        goto done;
    }

    *self = (NETIO_ZCOPY_SharedQReader)
    {
        .mem_pool = mem_pool,
        .qadmin = sq_admin,
        .consistent_set = consistent_set,
        .mode = mode,
        .latest_elmt_id = 0,
        .latest_elmt_index = SQ_INDEX_NONE,
        .pulses_seen_since_last_commit = 0,
        .elmt_count = 0,
        .is_open = RTI_FALSE,
        .give_up_on_open = RTI_FALSE,
    };

    result = RTI_TRUE;

done:
    if (!result)
    {
        /* return values are ignored here because our presence in this block
         * indicates a fatal error
         */
        retval = NETIO_ZCOPY_SharedMemPool_destroy(mem_pool);
        UNUSED_ARG(retval);
        retval = NETIO_ZCOPY_SharedQAdmin_destroy(sq_admin);
        UNUSED_ARG(retval);
        retval = NETIO_ZCOPY_ConsistentSetObserved_destroy(consistent_set);
        UNUSED_ARG(retval);
    }
    /* In the error case, various memory is leaked here only when compiling with
     * RTI_CERT because memory is intentionally not freed for cert.
     */

    /* coverity[leaked_storage] */

    return result;
}


RTI_PRIVATE RTI_BOOL
NETIO_ZCOPY_SharedQReader_open_by_name(
        NETIO_ZCOPY_SharedQReader *self,
        const char *name,
        RTI_BOOL initial)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL retval;
    struct NETIO_ZCOPY_PIVersionInfo version;
    SQ_Length consistent_set_length;
    SQ_Length qadmin_length;
    unsigned char *user_admin;
    SQ_Length user_admin_size;
    RTI_BOOL is_connected;
    SQ_Index sample_count;
    SQ_Length user_data_length;
    SQ_Index obsrv_count;
    SQ_Index obsrv_index;

    if (!NETIO_ZCOPY_SharedMemPool_open(
                self->mem_pool,
                name,
                &is_connected,
                &sample_count,
                &user_data_length,
                &obsrv_count))
    {
        goto done;
    }

    /* Not being able to connect is not an error, but will skip the rest
     *   of the function */
    if (is_connected)
    {
        /* Check if format version matches. We can connect to major versions older than mine */
        if (!NETIO_ZCOPY_SharedMemPool_get_version_info(self->mem_pool, &version) ||
            OSAPI_Memory_compare(&SQ_fv_MyVersion.id, &version.id, sizeof(version.id)) ||
            SQ_fv_MyVersion.version.major < version.version.major)
        {
            goto done;
        }

        if (!NETIO_ZCOPY_SharedQAdmin_get_mem_size(sample_count, &qadmin_length) ||
            !NETIO_ZCOPY_ConsistentSet_get_mem_size(
                    sample_count,
                    obsrv_count,
                    &consistent_set_length))
        {
            goto done;
        }

        if (!NETIO_ZCOPY_SharedMemPool_get_admin_info(
                    self->mem_pool,
                    &user_admin_size,
                    &user_admin))
        {
            goto done;
        }

        if (!NETIO_ZCOPY_SharedQAdmin_open(self->qadmin, qadmin_length, &user_admin[0]))
        {
            goto done;
        }

        if (self->mode >= SQ_CSTY_MODE_STRONG)
        {
            if (!NETIO_ZCOPY_SharedMemPool_get_observer_index(self->mem_pool, &obsrv_index))
            {
                goto done;
            }
        }
        else
        {
            obsrv_index = SQ_INDEX_NONE;
        }
        if (!NETIO_ZCOPY_ConsistentSetObserved_open(
                    self->consistent_set,
                    (self->mode >= SQ_CSTY_MODE_STRONG),
                    obsrv_index,
                    consistent_set_length,
                    &user_admin[qadmin_length],
                    initial))
        {
            goto done;
        }

        /* The identifiers will be initialized as we walk over the consistent set
        * elements
        */

        self->elmt_count = sample_count;
        self->is_open = RTI_TRUE;
    }

    result = RTI_TRUE;

done:
    if (!result)
    {
        /* return values are ignored here because our presence in this block
         * indicates a fatal error
         */
        retval = NETIO_ZCOPY_ConsistentSetObserved_close(self->consistent_set);
        IGNORE_RETVAL(retval);
        retval = NETIO_ZCOPY_SharedQAdmin_close(self->qadmin);
        IGNORE_RETVAL(retval);
        /* coverity[tainted_scalar] */
        retval = NETIO_ZCOPY_SharedMemPool_close(self->mem_pool);
        IGNORE_RETVAL(retval);
    }
    /* MICRO-5234 */
    /* coverity[leaked_storage] */
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQReader_open(
        NETIO_ZCOPY_SharedQReader *self,
	    const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id)
{
    RTI_BOOL result = RTI_FALSE;
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (owner_key == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("owner_key", owner_key, RTI_TRUE);)

    /* Construct name of SharedQ */
    if (!ZCOPY_Guid_to_string(
                owner_key,
                domain_id,
                SQ_BASE_NAME,
                name,
                sizeof(name)))
    {
        goto done;
    }

    result = NETIO_ZCOPY_SharedQReader_open_by_name(self, name, RTI_TRUE);

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQReader_close(NETIO_ZCOPY_SharedQReader *self)
{
    RTI_BOOL result = RTI_TRUE;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            result = RTI_FALSE; goto done;,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (self->is_open != RTI_TRUE)
    {
        /* There is no valid reason to call this function on a SharedQReader
         * that is not open. But if that occurs (due to user error), do nothing
         * and exit with RTI_TRUE
         */
        goto done;
    }

    /* capture an intermediate return of RTI_FALSE, but continue to attempt to
     * release resources
     */
    result &= NETIO_ZCOPY_ConsistentSetObserved_close(self->consistent_set);
    result &= NETIO_ZCOPY_SharedQAdmin_close(self->qadmin);
    result &= NETIO_ZCOPY_SharedMemPool_close(self->mem_pool);

    /* Set is_open to RTI_FALSE in ALL CASES, even if there were intermediate
     * failures in the *_close() calls above
     */
    self->is_open = RTI_FALSE;


done:
    /* Note that regardless of the returned result (RTI_TRUE or RTI_FALSE) this
     * close function may only be called once. Failures-- although reported to
     * the caller-- are unexpected and non-recoverable.
     */
    return result;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_ZCOPY_SharedQReader_finalize(NETIO_ZCOPY_SharedQReader *self)
{
    RTI_BOOL result = RTI_TRUE;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            result = RTI_FALSE; goto done;,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    result &= NETIO_ZCOPY_SharedQReader_close(self);
    result &= NETIO_ZCOPY_SharedMemPool_destroy(self->mem_pool);
    result &= NETIO_ZCOPY_SharedQAdmin_destroy(self->qadmin);
    result &= NETIO_ZCOPY_ConsistentSetObserved_destroy(self->consistent_set);

done:
    return result;
}
#endif /* !RTI_CERT */

/* ---------------------
 * Writer-side functions
 * ---------------------
 */

/* Functions expected to be used on the writer side of the SharedQ only,
 *   because the writer manages the contents and therefore knows what
 *   is needed to look up
 */


SQ_ReturnCode_t
NETIO_ZCOPY_SharedQWriter_get_loan(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_Length *user_data_size_out,
        SQ_MemPtr *user_data_out,
        SQ_WriterSampleId *sample_id_out)
{
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;
    SQ_Length elmt_data_length;
    SQ_MemPtr elmt_data;
    SQ_ReturnCode_t return_code = SQ_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) ||
            (user_data_size_out == NULL) ||
            (user_data_out == NULL) ||
            (sample_id_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_data_size_out", user_data_size_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_data_out", user_data_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_id_out", sample_id_out, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    if (!NETIO_ZCOPY_ConsistentSetOwned_element_reserve(self->consistent_set,
            &elmt_index, &elmt_data_length, &elmt_data))
    {
        return_code = SQ_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    /* Now that the Sample has been reserved, it is no longer accessible,
     *   so invalidate it. */
    if (!NETIO_ZCOPY_SharedQAdmin_writer_purge_sample(self->qadmin, elmt_index))
    {
        goto done;
    }

    /* It's an index to me, but an ID to the outside world */
    *sample_id_out = elmt_index;
    *user_data_size_out = elmt_data_length;
    *user_data_out = elmt_data;
    return_code = SQ_RETCODE_OK;

done:
    if (mem_pool_claimed)
    {
        if (!NETIO_ZCOPY_SharedMemPool_release(self->mem_pool))
        {
            /* If we are here, releasing the lock on the shared memory pool has
             * failed for some reason. If that happens, we set the return_code
             * to SQ_RETCODE_ERROR even if return_code was previously set to
             * SQ_RETCODE_OUT_OF_RESOURCES on this function call-- the ERROR
             * case is viewed as more serious and fatal.
             */
            return_code = SQ_RETCODE_ERROR;
        }
    }
    return return_code;
}

RTI_BOOL
NETIO_ZCOPY_SharedQWriter_discard_loan(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    /* It's an ID to the outside world, but an index to me */
    elmt_index = sample_id;
    if (!NETIO_ZCOPY_ConsistentSetOwned_element_unreserve(self->consistent_set, elmt_index))
    {
        goto done;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQWriter_lookup_loaned_sample_id(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_MemPtr_Const user_data,
        SQ_WriterSampleId *sample_id_out)
{
    RTI_BOOL result = RTI_FALSE;
    SQ_Index elmt_index;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (sample_id_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_id_out", sample_id_out, RTI_TRUE);)

    if (!NETIO_ZCOPY_ConsistentSetOwned_lookup_reserved_elmt_by_data(
            self->consistent_set, user_data, &elmt_index))
    {
        goto done;
    }

    /* The Sample ID for the outside world is just its index for me */
    *sample_id_out = elmt_index;
    result = RTI_TRUE;

done:
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQWriter_commit(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id,
        const struct SQ_SampleInfo *sample_info)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (sample_info == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_info", sample_info, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    /* It's an ID to the outside world, but an index to me */
    elmt_index = sample_id;
    if (!NETIO_ZCOPY_ConsistentSetOwned_element_commit(self->consistent_set, elmt_index))
    {
        goto done;
    }

    if (!NETIO_ZCOPY_SharedQAdmin_writer_commit_sample(
                self->qadmin,
                elmt_index,
                sample_info))
    {
        goto done;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQWriter_purge(
        NETIO_ZCOPY_SharedQWriter *self,
        SQ_WriterSampleId sample_id)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    /* It's an ID to the outside world, but an index to me */
    elmt_index = sample_id;
    if (!NETIO_ZCOPY_ConsistentSetOwned_element_purge(self->consistent_set, elmt_index))
    {
        goto done;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQWriter_pulse(
        NETIO_ZCOPY_SharedQWriter *self,
        const OSAPI_SystemTime *pulse_timestamp)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    if (!NETIO_ZCOPY_SharedQAdmin_writer_pulse(self->qadmin, pulse_timestamp))
    {
        goto done;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}

/* ---------------------
 * Reader-side functions
 * --------------------- */

RTI_SIZE_T
NETIO_ZCOPY_SharedQReader_get_size(void)
{
    return sizeof(NETIO_ZCOPY_SharedQReader);
}

/*i
 * \brief Visitor function when peeking elements (samples)
 *
 * \param[in] elmt_id The unique ID of the element (sample) being visited.
 * \param[in] elmt_index The index of the element (sample) being visited.
 * \param[in] visit_data The visit data passed to the visitor function.
 *
 * \return RTI_TRUE if the visitor function should continue, RTI_FALSE
 *         otherwise.
 */
RTI_PRIVATE RTI_BOOL
visit_unseen_element(SQ_ElmtId elmt_id, SQ_Index elmt_index, void *visit_data)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_VisitElmtArg *arg = (struct SQ_VisitElmtArg *)visit_data;
    NETIO_ZCOPY_SharedQReader *self = arg->self;
    struct SQ_SampleInfo sample_info;

    /* Get SampleInfo associated with this index, to forward to user */
    if (!NETIO_ZCOPY_SharedQAdmin_reader_get_sample_info(
                self->qadmin,
                elmt_index,
                &sample_info))
    {
        goto done;
    }

    /* Forward to the visitor function of the user first (and honor termination
     * if requested). Note that my index is the outside world's id.
     */
    if (!arg->visit_func(elmt_index,elmt_id,&sample_info, arg->visit_data))
    {
        arg->terminated = RTI_TRUE;
        goto done;
    }

    /* Keep track of latest index and id seen */
    self->latest_elmt_id = elmt_id;
    self->latest_elmt_index = elmt_index;
    /* The latest we have seen is not a pulse */
    self->pulses_seen_since_last_commit = 0;

    result = RTI_TRUE;
done:
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQReader_peek_unseen(
        NETIO_ZCOPY_SharedQReader *self,
        NETIO_ZCOPY_VisitReaderSampleFunc visit_func,
        void *visit_data)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    struct SQ_VisitElmtArg elmt_visit_data;
    SQ_SampleId_t pulses_since_latest_commit;
    struct SQ_SampleInfo sample_info;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (visit_func == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_int("visit_func == NULL", (visit_func == NULL), RTI_TRUE);)

    /* Make sure that the reader is connected to the writer */
    if (!self->is_open)
    {
        if (self->give_up_on_open)
        {
            /* We have already tried to open the SharedQ and failed, and we are
             * in peek_unseen() which means that the writer is active, so we
             * expected the SharedQ to be available. Since it was not, there is
             * no point in trying again. */
            goto done;
        }
        if (!NETIO_ZCOPY_SharedQReader_open_by_name(self, self->name, RTI_FALSE) ||
            (!self->is_open))
        {
            /* At this point, we do expect the attachment to succeed. The fact
             *   that we are in peek_unseen() means that the originating writer
             *   has its communication path set up, which means that it
             *   should be possible to attach to it. */
            self->give_up_on_open = RTI_TRUE;
            goto done;
        }
    }

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    /* Fill my own visit data */
    elmt_visit_data = (struct SQ_VisitElmtArg)
    {
        .self = self,
        .visit_func = visit_func,
        .visit_data = visit_data,
        .terminated = RTI_FALSE,
    };

    /* Walk over the unseen elements in the consistent set */
    if (!NETIO_ZCOPY_ConsistentSetObserved_elements_iterate(
                self->consistent_set,
                self->latest_elmt_id,
                self->latest_elmt_index,
                visit_unseen_element,
                &elmt_visit_data))
    {
        goto done;
    }

    /* Only continue if the callback function has not requested termination */
    if (!elmt_visit_data.terminated)
    {
        /* There may be one final element in the form of a pulse */
        if (!NETIO_ZCOPY_SharedQAdmin_reader_get_pulse_info(
                    self->qadmin,
                    &pulses_since_latest_commit,
                    &sample_info))
        {
            goto done;
        }
        /* Any pulse(s) observed after the last write? */
        if (self->pulses_seen_since_last_commit < pulses_since_latest_commit)
        {
            /* Yes, then iterate that too, only for the last pulse */
            if (visit_func(SQ_INDEX_NONE, 0, &sample_info, visit_data))
            {
                /* Only update if the pulse was processed */
                self->pulses_seen_since_last_commit = pulses_since_latest_commit;
            }
        }
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQReader_get_loan(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr elmt_id,
        SQ_Length *blob_length_out,
        SQ_MemPtr *blob_out)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;
    RTI_BOOL is_still_valid;
    SQ_Length blob_length;
    SQ_MemPtr blob;

    /* The outside world's id is my index */
    elmt_index = sample_id;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (elmt_index >= self->elmt_count) ||
                (blob_length_out == NULL) || (blob_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_uint("elmt_index", elmt_index, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("blob_length_out", blob_length_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("blob_out", blob_out, RTI_TRUE);)

    if (self->is_open != RTI_TRUE)
    {
        result = RTI_TRUE;
        *blob_out = NULL;
        blob_length_out = 0;
        return result;
    }

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    if (!NETIO_ZCOPY_ConsistentSetObserved_element_protect_if_valid(
                self->consistent_set,
                elmt_id,
                elmt_index,
                &is_still_valid))
    {
        goto done;
    }

    if (is_still_valid)
    {
        if (!NETIO_ZCOPY_SharedMemPool_get_element(
                    self->mem_pool,
                    elmt_index,
                    &blob_length,
                    &blob))
        {
            goto done;
        }
        *blob_length_out = blob_length;
        *blob_out = blob;
    }
    else
    {
        /* Index/Id combination is no longer valid.
         * This is a legal case, so no failure but just 0/NULL result.
         */
        *blob_length_out = 0;
        *blob_out = NULL;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQReader_return_loan(
        NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;

    /* The outside world's id is my index */
    elmt_index = sample_id;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (elmt_index >= self->elmt_count),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_uint("elmt_index", elmt_index, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    /* In case of strong consistency, the element needs to be unprotected */
    if (!NETIO_ZCOPY_ConsistentSetObserved_element_unprotect(
                self->consistent_set,
                elmt_index))
    {
        goto done;
    }

    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQReader_get_is_consistent(
        const NETIO_ZCOPY_SharedQReader *self,
        SQ_ReaderSampleId sample_id,
        SQ_SampleSeqNr elmt_id,
        RTI_BOOL *is_consistent_out)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL mem_pool_claimed = RTI_FALSE;
    SQ_Index elmt_index;

    RTI_BOOL is_elmt_consistent;

    /* The outside world's id is my index */
    elmt_index = sample_id;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL) || (elmt_index >= self->elmt_count) || (is_consistent_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_uint("elmt_index", elmt_index, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("is_consistent_out", is_consistent_out, RTI_TRUE);)

    if (!NETIO_ZCOPY_SharedMemPool_claim(self->mem_pool))
    {
        goto done;
    }
    mem_pool_claimed = RTI_TRUE;

    if (!NETIO_ZCOPY_ConsistentSetObserved_get_element_is_consistent(
                self->consistent_set,
                elmt_id,
                elmt_index,
                &is_elmt_consistent))
    {
        goto done;
    }

    *is_consistent_out = is_elmt_consistent;
    result = RTI_TRUE;

done:
    if (mem_pool_claimed)
    {
        result &= NETIO_ZCOPY_SharedMemPool_release(self->mem_pool);
    }
    return result;
}

/*ci
* \brief Called from the reader to check if it can attach to the Shared Queue.
*/
RTI_BOOL
NETIO_ZCOPY_SharedQ_can_attach(
	    const struct ZCOPY_Guid *owner_key,
        SQ_Index domain_id)
{
    RTI_BOOL result = RTI_FALSE;
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    /* Construct name of SharedQ */
    if (!ZCOPY_Guid_to_string(
                owner_key,
                domain_id,
                SQ_BASE_NAME,
                name,
                sizeof(name)))
    {
        goto done;
    }

    result = OSAPI_SharedMemorySegment_exists(name);

done:
    return result;
}

/*i @} */
