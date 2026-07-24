/*
 * FILE: NETIOZCOPYSharedQAdmin.c - Admin structure encapsulation -- implementation
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
 * \file
 * \brief A shared data structure administrating a NETIO_ZCOPY_SharedQ
 *        implementation
 *
 * \details
 * This file implements functionality to administrate location and state of all
 * Samples written into the NETIO_ZCOPY_SharedQ. All data types that have PI in
 * their names, for Position Independent, are data types to be stored in and
 * retrieved from shared memory.
 */
/*i
 * \addtogroup NETIO_ZCOPY_SharedQAdmin
 * @{
 */
/* Interface */
#include "NETIOZCOPYSharedQAdmin.h"

/* Implementation */
#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"

/* ------------------------
 * PI Sample administration
 * ------------------------
 */

/*i
 * \brief Admin element containing everything for a Sample
 *        that needs to be shared with the Readers
 */
struct SQ_PISampleAdmin_t
{
    /*i
     * \brief Relocatable reference to the Sample structure itself, as an index
     *        into the list of Samples, set at creation time and immutable after
     *        that.
     */
    SQ_Index index;

    /*i
     * \brief Samples that have been committed and not yet purged are valid.
     */
    RTI_BOOL is_valid;

    /*i
     * \brief Info is immutable during its lifetime but changes as the struct
     *        gets reused.
     */
    struct SQ_SampleInfo info;

    /*i
     * \brief Unique identifier for this Sample, immutable during its lifetime
     *        but changes as the struct gets reused.
     */
    SQ_SampleId_t sample_id;
};

/* -------------------------
 * PI SharedQ administration
 * -------------------------
 */

/*i
 * \brief Admin element containing everything for a SharedQ
 */
struct SQ_PISharedQAdmin_t
{
    /*i
     * \brief Total size of all SharedQ admin contents combined,
     *        set at creation time and immutable after that
     */
    SQ_Length total_sharedq_admin_size;

    /*i
     * \brief Any committing of a sample will update this field
     */
    SQ_SampleId_t latest_commit_id;

    /*i
     * \brief Any pulse will update this field, and it gets reset by a commit
     */
    SQ_SampleId_t pulses_since_latest_commit;

    /*i
     * \brief Any pulse will update this field
     */
    OSAPI_SystemTime latest_pulse_time;

    /*i
     * \brief Number of samples, set at creation time and immutable after that
     */
    SQ_Index sample_count;

    /*i
     * \brief A list of admins, one for each Sample
     */
    struct SQ_PISampleAdmin_t sample_admins[/*sample_count*/];
};

/* -------------------------
 * Local (heap) SharedQAdmin
 * -------------------------
 */

/*i
 * \brief Structure encapsulating the SharedQAdmin
 */
struct NETIO_ZCOPY_SharedQAdminImpl
{
    /*i
     * \brief Properties this SharedQAdmin was created with
     */
    SQ_Index sample_count;

    /*i
     * \brief Pointers into the flat memory containing the SharedQAdmin
     */
    struct SQ_PISharedQAdmin_t *pi_sharedq_admin;

    /*i
     * \brief Pointers into the flat memory containing the SampleAdmin
     */
    struct SQ_PISampleAdmin_t *pi_sharedq_sample_admins /*[sample_count]*/;
};

/* -------------------------------
 * Private functions and variables
 * -------------------------------
 */

RTI_PRIVATE const struct SQ_SampleInfo SQ_gv_SampleInfoEmpty =
{
        {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* .value */
        },                                                        /* .key */
        {
                0, /* .high */
                0, /* .low */
        },         /* .seq_nr */
        OSAPI_TIME_ZERO, /* .timestamp */
        SQ_SAMPLE_KIND_NONE, /* .kind */
};

/* ----------------
 * Class operations
 * ----------------
 */

/*i
 * \brief Calculation of sizes of structures/arrays, in bytes
 */
#define SQ_ALIGN_MAX (16)


/*** SOURCE_BEGIN ***/

/*i
 * \brief A helper function for alignment calculations
 *
 * \param[in] value The value to align
 *
 * \return The aligned value
 */
RTI_PRIVATE SQ_Length
pi_size_align_up(SQ_Length value)
{
    SQ_Length up = value + SQ_ALIGN_MAX - 1;
    return up - (up % SQ_ALIGN_MAX);
}


/*i
 * \brief Calculates the size of the memory in bytes needed to hold a PI shared
 *        data structure, containing a specific number of samples.
 *
 * \param[in] sample_count The max number of samples the structure must hold.
 * \param[out] length_out The length of the memory in bytes needed for the
 *             SharedQAdmin structure.
 *
 * \return RTI_TRUE if the memory size was calculated successfully,
 *         RTI_FALSE otherwise.
 *
 * \pre The length_out parameter must not be NULL.
 * \pre The sample_count parameter must be between 1 and 100000000 (inclusive).
 *
 * \post In case of success, the length_out parameter will be set to the
 *       calculated length of the memory needed.
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_get_mem_size(SQ_Index sample_count, SQ_Length *length_out)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            length_out == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("length_out", length_out, RTI_TRUE);)

    /* Limits for sample_count are the same as for max_samples*/
    if ((1 > sample_count) || (100000000 < sample_count))
    {
        goto done;
    }

    *length_out = pi_size_align_up(
            sizeof(struct SQ_PISharedQAdmin_t) +
            sample_count * sizeof(struct SQ_PISampleAdmin_t));
    result = RTI_TRUE;

done:
    return result;
}

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */
RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_create(
        SQ_Index sample_count,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory /*[flat_memory_length]*/,
        NETIO_ZCOPY_SharedQAdmin **self_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct NETIO_ZCOPY_SharedQAdminImpl *self = NULL;
    struct SQ_PISharedQAdmin_t *sharedq_admin = NULL;
    SQ_Length length_needed;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (flat_memory == NULL) || (self_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("flat_memory", flat_memory, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("self_out", self_out, RTI_TRUE);)

    /* Check if the memory block provided is big enough */
    if (!NETIO_ZCOPY_SharedQAdmin_get_mem_size(sample_count, &length_needed) ||
        (length_needed > flat_memory_length))
    {
        goto done;
    }

    /* Set and fill pointer into shared memory admin */
    sharedq_admin = (struct SQ_PISharedQAdmin_t *)flat_memory;
    *sharedq_admin = (struct SQ_PISharedQAdmin_t)
    {
            .total_sharedq_admin_size = length_needed,
            .latest_commit_id = 0,
            .pulses_since_latest_commit = 0,
            .latest_pulse_time = SQ_gv_SampleInfoEmpty.timestamp,
            .sample_count = sample_count,
    };

    /* SampleAdmins in Queue admin */
    for (SQ_Index i = 0; i < sample_count; i++)
    {
        struct SQ_PISampleAdmin_t *sample_admin = &sharedq_admin->sample_admins[i];
        *sample_admin = (struct SQ_PISampleAdmin_t)
        {
                .index = i,
                .is_valid = RTI_FALSE,
                .info = SQ_gv_SampleInfoEmpty,
                .sample_id = 0,
        };
    }

    /* Construct result */
    OSAPI_Heap_allocate_struct(&self, struct NETIO_ZCOPY_SharedQAdminImpl);
    if (self == NULL)
    {
        goto done;
    }
    *self = (struct NETIO_ZCOPY_SharedQAdminImpl)
    {
            .sample_count = sample_count,
            .pi_sharedq_admin = sharedq_admin,
            .pi_sharedq_sample_admins = &sharedq_admin->sample_admins[0],
    };

    *self_out = self;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_destroy(NETIO_ZCOPY_SharedQAdmin *self)
{
    if (self == NULL)
    {
        goto done;
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_struct(self);
#endif /* !RTI_CERT */

done:
    return RTI_TRUE;
}

void
NETIO_ZCOPY_SharedQAdmin_reset(NETIO_ZCOPY_SharedQAdmin *self)
{
    if (self != NULL)
    {
        self->sample_count = SQ_INDEX_NONE;
        self->pi_sharedq_admin = NULL;
        self->pi_sharedq_sample_admins = NULL;
    }
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_initialize(NETIO_ZCOPY_SharedQAdmin **self_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct NETIO_ZCOPY_SharedQAdminImpl *self;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self_out", self_out, RTI_TRUE);)

    /* Construct result */
    OSAPI_Heap_allocate_struct(&self, struct NETIO_ZCOPY_SharedQAdminImpl);
    if (self == NULL)
    {
        goto done;
    }

    /* Initialize the NETIO_ZCOPY_SharedQAdminImpl object */
    *self = (struct NETIO_ZCOPY_SharedQAdminImpl)
    {
        .sample_count = SQ_INDEX_NONE,
        .pi_sharedq_admin = NULL,
        .pi_sharedq_sample_admins = NULL,
    };

    /* Success */
    *self_out = self;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_open(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory /*[flat_memory_length]*/)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedQAdmin_t *sq_admin = (struct SQ_PISharedQAdmin_t *)flat_memory;

    OSAPI_PRECONDITION(
            (self == NULL) || (flat_memory == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
            OSAPI_Log_entry_add_pointer("flat_memory", flat_memory, RTI_FALSE);)

    /* Check if the memory block provided is big enough */
    /* Needs to be at least as big as the struct to even be able to get info */
    if ((flat_memory_length < sizeof(*sq_admin)) ||
        (flat_memory_length < sq_admin->total_sharedq_admin_size))
    {
        goto done;
    }

    /* Store values */
    self->sample_count = sq_admin->sample_count;
    self->pi_sharedq_admin = sq_admin;
    self->pi_sharedq_sample_admins = &sq_admin->sample_admins[0];

    /* Success */
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_close(NETIO_ZCOPY_SharedQAdmin *self)
{
    if (self == NULL)
    {
        goto done;
    }

done:
    return RTI_TRUE;
}

/*----------------------
 * Writer-side functions
 * ---------------------
 */
/* Functions expected to be used on the writer side of the SharedQ only,
 *   because the writer manages the contents and therefore knows what
 *   is needed to look up */

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_commit_sample(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index,
        const struct SQ_SampleInfo *sample_info)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedQAdmin_t *pi_admin;
    struct SQ_PISampleAdmin_t *pi_sample_admin;

    /* Preconditions*/
    OSAPI_PRECONDITION(
            (self == NULL) || (sample_info == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_info", sample_info, RTI_TRUE);)

    /* Check sample_admin's validity and commit it */
    if ((sample_index == SQ_INDEX_NONE) || (sample_index >= self->sample_count))
    {
        /* Index out of bounds */
        goto done;
    }
    pi_sample_admin = &self->pi_sharedq_sample_admins[sample_index];
    if (pi_sample_admin->is_valid)
    {
        /* Can not commit a sample that is already valid/committed */
        goto done;
    }
    pi_sample_admin->is_valid = RTI_TRUE;
    pi_sample_admin->info = *sample_info;

    /* Update info about latest activity */
    pi_admin = self->pi_sharedq_admin;
    pi_admin->latest_commit_id++;
    pi_admin->pulses_since_latest_commit = 0;

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_purge_sample(
        NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISampleAdmin_t *pi_sample_admin;

    /* Preconditions*/
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* Check sample_admin's validity and purge it */
    if ((sample_index == SQ_INDEX_NONE) || (sample_index >= self->sample_count))
    {
        /* Index out of bounds */
        goto done;
    }
    pi_sample_admin = &self->pi_sharedq_sample_admins[sample_index];
    pi_sample_admin->is_valid = RTI_FALSE;
    pi_sample_admin->info = SQ_gv_SampleInfoEmpty;

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_writer_pulse(
        NETIO_ZCOPY_SharedQAdmin *self,
        const OSAPI_SystemTime *pulse_timestamp)
{
    struct SQ_PISharedQAdmin_t *pi_admin;

    /* Preconditions*/
    OSAPI_PRECONDITION(
            (self == NULL) || (pulse_timestamp == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("pulse_timestamp", pulse_timestamp, RTI_TRUE);)

    /* Update timestamp of latest activity */
    pi_admin = self->pi_sharedq_admin;
    pi_admin->pulses_since_latest_commit++;
    pi_admin->latest_pulse_time = *pulse_timestamp;

    return RTI_TRUE;
}

/*----------------------
 * Reader-side functions
 * ---------------------
 */


RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_reader_get_sample_info(
        const NETIO_ZCOPY_SharedQAdmin *self,
        SQ_Index sample_index,
        struct SQ_SampleInfo *sample_info_out)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (sample_info_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_info_out", sample_info_out, RTI_TRUE);)

    if ((sample_index == SQ_INDEX_NONE) || (sample_index >= self->sample_count))
    {
        /* index is out of bounds */
        goto done;
    }
    if (!(self->pi_sharedq_sample_admins[sample_index].is_valid))
    {
        /* sample is not valid */
        goto done;
    }
    *sample_info_out = self->pi_sharedq_sample_admins[sample_index].info;
    result = RTI_TRUE;

done:
    return result;
}


RTI_BOOL
NETIO_ZCOPY_SharedQAdmin_reader_get_pulse_info(
        const NETIO_ZCOPY_SharedQAdmin *self,
        SQ_SampleId_t *pulses_since_latest_commit_out,
        struct SQ_SampleInfo *sample_info_out)
{
    /* Preconditions */
    OSAPI_PRECONDITION(
        (self == NULL) || (pulses_since_latest_commit_out == NULL) ||
            (sample_info_out == NULL),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
        OSAPI_Log_entry_add_pointer("pulses_since_latest_commit_out", pulses_since_latest_commit_out, RTI_FALSE);
        OSAPI_Log_entry_add_pointer("sample_info_out", sample_info_out, RTI_TRUE);
    )

    *pulses_since_latest_commit_out = self->pi_sharedq_admin->pulses_since_latest_commit;
    *sample_info_out = (struct SQ_SampleInfo) {
        .key = SQ_gv_SampleInfoEmpty.key,
        .seq_nr = SQ_gv_SampleInfoEmpty.seq_nr,
        .timestamp = self->pi_sharedq_admin->latest_pulse_time,
        .kind = SQ_SAMPLE_KIND_PULSE,
    };

    return RTI_TRUE;
}

/*i @} */
