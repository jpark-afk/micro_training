/*
 * FILE: NETIOZCOPYConsistentSet.c - Shared Robust Consistent Collection -- implementation
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
 * \addtogroup NETIOZCOPY_ConsistentSetClass
 * @{
 */
/*i
 * \file
 * \brief Shared Robust Consistent Collection -- implementation
 */
/* Interface */
#include "NETIOZCOPYConsistentSet.h"

/* Implementation */
#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_system.h"

/* Position Independent (PI) data types to be stored in shared memory */

/* -------------------------------------
 * PI SharedConsistentSet administration
 * -------------------------------------
 */

/*i
 * \brief Structure containing everything for a shared consistent set
 */
struct SQ_PISharedConsistentSet
{
    /*i
     * \brief Total size of all SharedConsistentSet admin contents combined,
     *        set at creation time and immutable after that.
     */
    SQ_Length set_size;

    /*i
     * \brief Whenever an Element is committed, this field will be updated
     */
    SQ_ElmtId newest_id;

    /*i
     * \brief Tail of committed (valid) Elements list
     */
    SQ_Index newest_index;

    /*i
     * \brief Total number of elements, set at creation time and immutable
     *        after that
     */
    SQ_Index elmt_count;

    /*i
     * \brief Size of a element, set at creation time and immutable after that
     */
    SQ_Length elmt_size;

    /*i
     * \brief Element starting offset, set at creation time and immutable
     *        after that
     */
    SQ_Length elmt_offset;

    /*i
     * \brief Total number of observers, set at creation time and immutable
     *        after that
     */
    SQ_Index obsrv_count;

    /*i
     * \brief Size of a observer, set at creation time and immutable after that
     */
    SQ_Length obsrv_size;

    /*i
     * \brief Observer starting offset, set at creation time and immutable
     *        after that.
     */
    SQ_Length obsrv_offset;
};

/* ------------------------------
 * ModificationId types and logic
 * ------------------------------
 */

/*ci
 * \brief 64-bit sequence number used to register every modification to a
 *        ConsistentSet. The layout of this struct is the same as the
 *        OSAPI_SystemTime struct, so that we can use the same type for
 *        both purposes, for backward compatibility reasons.
 */
struct SQ_ModificationSeqNr
{
    /*i \brief Acutal id as a sequence number
     */
    RTI_UINT64 id;

    /*e \brief Value containing the version number while at the same time
     *         being an invalid nanosecond value when interpreted as a timestamp,
     *         so that we can use the same type
     */
    RTI_UINT32 version;
};

/*ci
 * \brief Union representing a modification identifier, which can be interpreted
 *        either as a timestamp or as a sequence number.
 */
typedef union
{
    OSAPI_SystemTime timestamp;
    struct SQ_ModificationSeqNr seq_nr;
} SQ_ModificationId;

/*ci
 * \brief Value representing the current version number of this protocol
 */
#define SQ_MODIFICATION_ID_V2_0 (0x40200000UL)

RTI_PRIVATE RTI_BOOL
SQ_ModificationId_from_elmt_id(
    SQ_ModificationId *self,
    SQ_ElmtId elmt_id,
    RTI_BOOL use_timestamp_as_id)
{
    RTI_BOOL result = RTI_TRUE;
    if (use_timestamp_as_id)
    {
        result = OSAPI_System_get_time(&self->timestamp);
    }
    else
    {
        self->seq_nr.id = elmt_id;
        self->seq_nr.version = SQ_MODIFICATION_ID_V2_0;
    }
    return result;
}

RTI_PRIVATE void
SQ_ModificationId_minimize(
    SQ_ModificationId *self,
    RTI_BOOL use_timestamp_as_id)
{
    *self = (SQ_ModificationId){0};
    if (!use_timestamp_as_id)
    {
        self->seq_nr.version = SQ_MODIFICATION_ID_V2_0;
    }
}

RTI_PRIVATE void
SQ_ModificationId_maximize(
    SQ_ModificationId *self,
    RTI_BOOL use_timestamp_as_id)
{
    if (use_timestamp_as_id)
    {
        self->timestamp = (OSAPI_SystemTime)OSAPI_TIME_MAX;
    }
    else
    {
        self->seq_nr.id = SQ_ELMT_ID_MAX;
        self->seq_nr.version = SQ_MODIFICATION_ID_V2_0;
    }
}

RTI_PRIVATE RTI_BOOL
SQ_ModificationId_is_seq_nr(const SQ_ModificationId *mod_id)
{
    /* The first two bytes need to match, other bytes might be used for
     * future extensions */
    return (mod_id->seq_nr.version & 0xFFFF0000UL) == SQ_MODIFICATION_ID_V2_0;
}

/* -------------------------
 * PI Element administration
 * -------------------------
 */

/*i
 * \brief Node in a doubly linked, circular, Position Independent (PI) list.
 *
 * This struct serves as the "pointers" to other nodes in a list. This design
 * uses no administrative/controlling class-- the list can be used simply by
 * traversing through the list from any node. The notion of a "head" or "tail"
 * can be maintained by the owner of the list.
 */
struct SQ_PIListNode
{
    /*i
     * \brief Index to the next node in the list
     */
    SQ_Index next_index;
    /*i
     * \brief Index to the previous node in the list
     */
    SQ_Index prev_index;
};

/*i
 * \brief Initializing value for a PIListNode that is not in a list
 */
static const struct SQ_PIListNode SQ_gv_PIListNodeNil =
{
        .next_index = SQ_INDEX_NONE,
        .prev_index = SQ_INDEX_NONE,
};

/*i
 * \brief Structure containing everything for an Element that
 *        needs to be shared with the Observers
 */
struct SQ_PIElement
{
    /*i
     * \brief Unique identifier for this Element, immutable during
     *        its lifetime but changes as the struct gets reused.
     */
    SQ_ElmtId id;
    /*i
     * \brief Immutable, relocatable reference to the Element structure itself,
     *        as an index into the list of Elements
     */
    SQ_Index index;
    /*i
     * \brief Only Elements that have been committed, and possibly purged, but
     *        not yet reused, are valid
     */
    RTI_BOOL is_valid;
    /*i
     * \brief Modification id value when this element got committed
     */
    SQ_ModificationId modification_id_committed;
    /*i
     * \brief Modification id value when this element got purged,
     *        or SQ_MODIFICATION_ID_MAX if it was not purged (yet)
     */
    SQ_ModificationId modification_id_purged;
    /*i
     * \brief Valid elements live in an "inline list" in order they were
     *        committed
     */
    struct SQ_PIListNode valid_list_node;
    /*i
     * \brief Number of Observers currently keeping this Element protected
     */
    SQ_Index ref_count;
};

/* -------------------------
 * PI Observer locking admin
 * -------------------------
 */

/* Bitset operations */

#define SQ_BITMASK(b_)      (RTI_UINT8)(1 << ((b_) % CHAR_BIT))
#define SQ_BITSLOT(b_)      ((b_) / CHAR_BIT)
#define SQ_BITSET(a_, b_)   ((a_)[SQ_BITSLOT(b_)] |= SQ_BITMASK(b_))
#define SQ_BITCLEAR(a_, b_) ((a_)[SQ_BITSLOT(b_)] &= (RTI_UINT8)~SQ_BITMASK(b_))
#define SQ_BITTEST(a_, b_)  ((a_)[SQ_BITSLOT(b_)] & SQ_BITMASK(b_))
#define SQ_BITNSLOTS(nb_)   (((nb_) + CHAR_BIT - 1) / CHAR_BIT)

/*i
 * \brief Locking structure containing a bitmap representing
          elements in the consistent set
 */
struct SQ_PIObserver
{
    /*i
     * \brief Byte count of locking bitmap.
     */
    SQ_Index bitmap_byte_count;
    /*i
     * \brief Each bit in the following bitmap corresponds to an individual
     *        element in the consistent set
     */
    unsigned char locking_bitmap[/*bitmap_byte_count*/];
};

/* -----------------
 * Private functions
 * -----------------
 */

/* Calculation of sizes of structures/arrays, in bytes */
#define SQ_ALIGN_MAX (16)
#define SQ_ALIGNOF(test_type_) offsetof(struct {char c; test_type_ member;}, member)

/*i
 * \brief Calculate the aligned size of a value based on a given boundary size.
 *        The boundary size must be a power of 2.
 *
 * \param[in] value The value to align.
 * \param[in] boundary_size The boundary size to align the value to.
 *
 * \return The aligned size of the value.
 */
RTI_PRIVATE SQ_Length
pi_size_align_up(SQ_Length value, SQ_Index boundary_size)
{
    SQ_Length up = value + boundary_size - 1;
    return up - (up % boundary_size);
}

/*i
 * \brief Calculate the aligned size of a value based on the alignment of a
 *        given type.
 *
 * \param[in] value_ The value to align.
 * \param[in] type_ The type to align the value to.
 *
 * \return The aligned size of the value.
 */
#define PI_SIZE_ALIGN_UP_FOR_TYPE(value_, type_) \
    pi_size_align_up((value_), SQ_ALIGNOF(type_))

/*i
 * \brief Calculate the aligned size of the maximum value.
 *
 * \param[in] value_ The value to align.
 *
 * \return The maximum aligned size.
 */
#define PI_SIZE_ALIGN_UP_MAX(value_) \
    pi_size_align_up((value_), SQ_ALIGN_MAX)


/*** SOURCE_BEGIN ***/

/*i
 * \brief Calculate the aligned size of the base structure of the shared
 *        consistent set.
 *
 * \return The aligned size of the base structure of the shared consistent set.
 */
RTI_PRIVATE SQ_Length
pi_size_base(void)
{
    return PI_SIZE_ALIGN_UP_MAX(sizeof(struct SQ_PISharedConsistentSet));
}

/*i
 * \brief Calculate the aligned size of a SQ_PIElement.
 *
 * \return The aligned size of a SQ_PIElement.
 */
RTI_PRIVATE SQ_Length
pi_size_element(void)
{
    return sizeof(struct SQ_PIElement);
}

/*i
 * \brief Calculate the aligned size of an observer structure based on the
 *        number of elements in the consistent set.
 *
 * \param[in] elmt_count The number of elements in the consistent set.
 *
 * \return The aligned size of the observer structure.
 */
RTI_PRIVATE SQ_Length
pi_size_observer(SQ_Index elmt_count)
{
    /* SQ_Index is passed to PI_SIZE_ALIGN_UP_FOR_TYPE() as the type because
     * struct SQ_PIObserver ends in a flexible array, which is not supported
     * by offsetof()
     */
    return PI_SIZE_ALIGN_UP_FOR_TYPE(
            sizeof(struct SQ_PIObserver) + SQ_BITNSLOTS(elmt_count),
            SQ_Index);
}

/*i
 * \brief Calculate the aligned size of the list of elements in the consistent
 *        set.
 *
 * \param[in] elmt_count The number of elements in the consistent set.
 *
 * \return The aligned size of the list of elements in the consistent set.
 */
RTI_PRIVATE SQ_Length
pi_size_element_list(SQ_Index elmt_count)
{
    return PI_SIZE_ALIGN_UP_MAX(elmt_count * pi_size_element());
}

/*i
 * \brief Calculate the aligned size of the list of observers in the consistent
 *        set.
 *
 * \param[in] elmt_count The number of elements in the consistent set.
 * \param[in] obsrv_count The number of observers in the consistent set.
 *
 * \return The aligned size of the list of observers in the consistent set.
 */
RTI_PRIVATE SQ_Length
pi_size_observer_list(SQ_Index elmt_count, SQ_Index obsrv_count)
{
    return PI_SIZE_ALIGN_UP_MAX(obsrv_count * pi_size_observer(elmt_count));
}

/* Convenience functions to get element or observer pointer */

/*i
 * \brief Get a pointer to the element at the given index in the consistent set.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] index The index of the element to get.
 *
 * \return A pointer to the element at the given index in the consistent set.
 */
RTI_PRIVATE struct SQ_PIElement *
NETIO_ZCOPY_ConsistentSet_get_element(const struct SQ_PISharedConsistentSet *consistent_set, SQ_Index index)
{
    /* For the element we want to retrieve, offset is the distance, in bytes, to
     * its position in the region of memory occupied by consistent_set
     */
    SQ_Length offset = consistent_set->elmt_offset + index * consistent_set->elmt_size;

    /* By casting consistent_set to (unsigned char *), we can use offset, as
     * calculated above, to move to the starting address of the SQ_PIElement of
     * interest
     */
    return (struct SQ_PIElement *)&(((unsigned char *)consistent_set)[offset]);
}

/*i
 * \brief Get a pointer to the observer at the given index in the consistent
 *        set.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] index The index of the observer to get.
 *
 * \return A pointer to the observer at the given index in the consistent set.
 */
RTI_PRIVATE struct SQ_PIObserver *
NETIO_ZCOPY_ConsistentSet_get_observer(const struct SQ_PISharedConsistentSet *consistent_set, SQ_Index index)
{
    /* For the observer we want to retrieve, offset is the distance, in bytes,
     * to its position in the region of memory occupied by consistent_set
     */
    SQ_Length offset = consistent_set->obsrv_offset + index * consistent_set->obsrv_size;

    /* By casting consistent_set to (unsigned char *), we can use offset, as
     * calculated above, to move to the starting address of the SQ_PIObserver of
     * interest
     */
    return (struct SQ_PIObserver *)&(((unsigned char *)consistent_set)[offset]);
}

/* -----------------
 * Locking functions
 * -----------------
 */

/* Locking of elements by strongly consistent observers. Each observer maintains
 * a bitmap for the elements it locks. A reference count is used for determining
 * the presence of locking more efficiently, but this is not sufficient for
 * recovering from observer deaths.
 */

/*i
 * \brief Acquire a read lock on an element for an observer.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt_index The index of the element to lock.
 * \param[in] obsrv_index The index of the observer to lock the element for.
 *
 * \pre The element is not already locked for the observer.
 */
RTI_PRIVATE void
pi_elmt_lock_acquire_read(
        struct SQ_PISharedConsistentSet *consistent_set,
        SQ_Index elmt_index,
        SQ_Index obsrv_index)
{
    /* The following should be true, but is not tested for performance reasons:
     *
     * !SQ_BITTEST(
     *      NETIO_ZCOPY_ConsistentSet_get_observer(consistent_set, obsrv_index)->locking_bitmap,
     *      elmt_index))
     * and
     * NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index)->ref_count <=
     *      consistent_set->obsrv_count
     */

    /* Set locking bit for this observer for this element */
    SQ_BITSET(NETIO_ZCOPY_ConsistentSet_get_observer(consistent_set, obsrv_index)->locking_bitmap, elmt_index);

    /* Reference count is basically a cache for the number of bits set */
    NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index)->ref_count++;
}

/*i
 * \brief Release a read lock on an element for an observer.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt_index The index of the element to unlock.
 * \param[in] obsrv_index The index of the observer to unlock the element for.
 *
 * \pre The element is locked for the observer.
 */
RTI_PRIVATE void
pi_elmt_lock_release_read(
        struct SQ_PISharedConsistentSet *consistent_set,
        SQ_Index elmt_index,
        SQ_Index obsrv_index)
{
    /* The following should be true, but is not tested for performance reasons:
     *
     * SQ_BITTEST(NETIO_ZCOPY_ConsistentSet_get_observer(consistent_set, obsrv_index)->locking_bitmap,
     *      elmt_index))
     *
     * and
     *
     * NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index)->ref_count > 0
     */

    /* Clear locking bit for this observer for this element */
    SQ_BITCLEAR(NETIO_ZCOPY_ConsistentSet_get_observer(consistent_set, obsrv_index)->locking_bitmap, elmt_index);

    /* Reference count is basically a cache for the number of bits set */
    NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index)->ref_count--;
}

/*i
 * \brief Release a lock on an element if it is locked.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt_index The index of the element to unlock.
 * \param[in] obsrv_index The index of the observer to unlock the element for.
 *
 */
RTI_PRIVATE void
pi_elmt_lock_release_read_if_locked(
        struct SQ_PISharedConsistentSet *consistent_set,
        SQ_Index elmt_index,
        SQ_Index obsrv_index)
{
    if (SQ_BITTEST(NETIO_ZCOPY_ConsistentSet_get_observer(consistent_set, obsrv_index)->locking_bitmap, elmt_index))
    {
        pi_elmt_lock_release_read(consistent_set, elmt_index, obsrv_index);
    }
}

/*i
 * \brief Peek to see if the write lock on an element can be acquired.
 *
 * \param[in] consistent_set The Position Independent consistent set.
 * \param[in] elmt_index The index of the element to peek.
 *
 * \return RTI_TRUE if the write lock is available, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
pi_elmt_lock_probe_acquire_write(struct SQ_PISharedConsistentSet *consistent_set, SQ_Index elmt_index)
{
    /* Get the sample's admin's refcount */
    return (0 == NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index)->ref_count);
}

/*i
 * \brief Remove an observer from the consistent set and clears any outstanding
 *        locks.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] obsrv_index The index of the observer to remove.
 *
 * \pre The observer exists in the consistent set.
 */
RTI_PRIVATE void
pi_obsrv_remove(struct SQ_PISharedConsistentSet *consistent_set, SQ_Index obsrv_index)
{
    /* Clear any outstanding locks */
    for (SQ_Index elmt_index = 0; elmt_index < consistent_set->elmt_count; elmt_index++)
    {
        pi_elmt_lock_release_read_if_locked(consistent_set, elmt_index, obsrv_index);
    }
}

/*i
 * \brief Commit an element, making it valid.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt The element to commit.
 * \param[in] use_timestamp_as_id Whether to use timestamp or sequence number as modification id
 *
 */
RTI_PRIVATE void
pi_elmt_validate(
    struct SQ_PISharedConsistentSet *consistent_set,
    struct SQ_PIElement *elmt,
    RTI_BOOL use_timestamp_as_id)
{
    SQ_Index elmt_newest_index;
    RTI_BOOL retcode;

    /* Get the admin of the element to be committed and update it */
    elmt_newest_index = consistent_set->newest_index;
    if (elmt_newest_index == SQ_INDEX_NONE)
    {
        /* Special case, only element in the list points to itself */
        elmt->valid_list_node.prev_index = elmt->index;
        elmt->valid_list_node.next_index = elmt->index;
    }
    else
    {
        /* Regular case, insert element circularly */
        struct SQ_PIElement *elmt_newest;
        struct SQ_PIElement *elmt_oldest;
        SQ_Index elmt_oldest_index;

        /* Start with the latest element */
        elmt_newest = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_newest_index);

        /* Oldest comes after newest */
        elmt_oldest_index = elmt_newest->valid_list_node.next_index;
        elmt_oldest = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_oldest_index);

        /* Do the insertion */
        elmt->valid_list_node.next_index = elmt_oldest_index;
        elmt->valid_list_node.prev_index = elmt_newest_index;
        elmt_newest->valid_list_node.next_index = elmt->index;
        elmt_oldest->valid_list_node.prev_index = elmt->index;
    }

    /* This is a new element so a new id */
    elmt->id = ++consistent_set->newest_id;
    /* Mark the element as valid */
    elmt->is_valid = RTI_TRUE;

    /* This value is unused, so set it to the element's id */
    retcode = SQ_ModificationId_from_elmt_id(&elmt->modification_id_committed,
        elmt->id, RTI_FALSE);
    IGNORE_RETVAL(retcode);

    /* The element has not been purged yet, so set its purged id to the maximum */
    SQ_ModificationId_maximize(&elmt->modification_id_purged, use_timestamp_as_id);

    /* Update the newest index in the consistent set */
    consistent_set->newest_index = elmt->index;
}


/*i
 * \brief Mark an element as purged. Note that it remains valid to the outside
 *        worlds until it gets reused/invalidated
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt The element to mark as purged.
 * \param[in] use_timestamp_as_id Whether to use timestamp or sequence number as modification id
 *
 */
RTI_PRIVATE void
pi_elmt_purge(
    struct SQ_PISharedConsistentSet *consistent_set,
    struct SQ_PIElement *elmt,
    RTI_BOOL use_timestamp_as_id)
{
    /* Update the modification id counter for the consistent set
     * and assign its current value to the element's modification_id_purged
     */
    RTI_BOOL retcode;

    /* Increase modification id to reflect this change */
    ++consistent_set->newest_id;

    /* Update the element's modification_id_purged */
    retcode = SQ_ModificationId_from_elmt_id(
        &elmt->modification_id_purged,
        consistent_set->newest_id,
        use_timestamp_as_id);
    IGNORE_RETVAL(retcode);
}


/*i
 * \brief Invalidate an element and remove it from the list.
 *
 * \param[in] consistent_set The consistent set.
 * \param[in] elmt The element to invalidate.
 * \param[in] use_timestamp_as_id Whether to use timestamp or sequence number as modification id
 *
 */
RTI_PRIVATE void
pi_elmt_invalidate(
    struct SQ_PISharedConsistentSet *consistent_set,
    struct SQ_PIElement *elmt,
    RTI_BOOL use_timestamp_as_id)
{
    /* Before doing anything else, mark the element as invalid */
    elmt->is_valid = RTI_FALSE;
    SQ_ModificationId_minimize(&elmt->modification_id_committed, use_timestamp_as_id);
    SQ_ModificationId_minimize(&elmt->modification_id_purged, use_timestamp_as_id);

    /* Remove elmt from the list of valid elements */
    if (elmt->valid_list_node.next_index == elmt->index)
    {
        /* Points to itself so this is the last one to be removed */
        consistent_set->newest_index = SQ_INDEX_NONE;
    }
    else
    {
        /* Normal case */
        SQ_Index prev_index = elmt->valid_list_node.prev_index;
        SQ_Index next_index = elmt->valid_list_node.next_index;
        NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, prev_index)->valid_list_node.next_index =
                next_index;
        NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, next_index)->valid_list_node.prev_index =
                prev_index;
        /* In case we happen to have removed the newest element */
        if (consistent_set->newest_index == elmt->index)
        {
            /* Update the anchor so it points to the "previously newest"
            * sample
            */
            consistent_set->newest_index = prev_index;
        }
    }
    /* For checking purposes, erase the links in the removed element */
    elmt->valid_list_node = SQ_gv_PIListNodeNil;

    return;
}

/*i
 * \brief Find the element with the smallest ID that is greater than
 *        elmt_id_min.
 *
 * This function searches for the element in the consistent set whose ID is
 * greater than elmt_id_min, but is smallest among all candidate elements.
 * The search starts at the newest element in the set and proceeds backwards
 * through the list until an element with the smallest ID is found whose ID is
 * still greater than elmt_id_min.
 *
 * \param[in] consistent_set The consistent set to search.
 * \param[in] elmt_id_min The minimum ID to search against.
 * \param[in] elmt_index_cached The index of an element that may be a good
 *            candidate for the search. If this element has an ID equal to
 *            elmt_id_min, the elmt_index_out will be the next element in
 *            the list.
 * \param[out] elmt_index_out A pointer to a variable that will receive the
 *             index of the first element whose ID is greater than elmt_id_min.
 *
 * \return RTI_TRUE if an element was found, RTI_FALSE otherwise.
 *
 * \note the list is sorted in order of ID, which is the order of committing.
 *       The largest ID is the newest element, and the smallest ID is the
 *       oldest.
 */
RTI_PRIVATE void
pi_elmt_find_after(
        struct SQ_PISharedConsistentSet *consistent_set,
        SQ_ElmtId elmt_id_min,
        SQ_Index elmt_index_cached,
        SQ_Index *elmt_index_out)
{
    SQ_Index elmt_newest_index;
    SQ_Index elmt_oldest_index;
    SQ_Index index_found = SQ_INDEX_NONE;
    struct SQ_PIElement *elmt_newest;
    struct SQ_PIElement *elmt_oldest;

    /* Jump onto the list at the latest sample */
    elmt_newest_index = consistent_set->newest_index;
    elmt_newest = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_newest_index);
    elmt_oldest_index = elmt_newest->valid_list_node.next_index;
    elmt_oldest = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_oldest_index);

    /* Special case: the oldest is newer than requested; in other words, all
     * elements in the set meet our search criteria. We want the oldest one.
     * The oldest index is right after the newest (recall that the list is
     * circular)
     */
    if (elmt_oldest->id > elmt_id_min)
    {
        index_found = elmt_oldest->index;
    }
    else
    {
        if (elmt_index_cached != SQ_INDEX_NONE)
        {
            /* Still not found, try via the cached index value */
            struct SQ_PIElement *elmt_cached;
            elmt_cached = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index_cached);
            if (elmt_cached->id == elmt_id_min)
            {
                /* Cache hit:
                 * Hurray! Get the next index to get to the first element id
                 * that is larger
                 */
                index_found = elmt_cached->valid_list_node.next_index;
            }
        }
        if (index_found == SQ_INDEX_NONE)
        {
            /* No cache available, or cache miss:
             * Did not find it for any special cases, brute force it and find
             * the number by walking the list in reverse (since we expect users
             * to typically read samples that are newer).
             */
            struct SQ_PIElement *elmt = elmt_newest;
            struct SQ_PIElement *elmt_prev = NULL;
            do
            {
                /* Traverse backwards, so next is prev */
                SQ_Index index_next = elmt->valid_list_node.prev_index;
                /* We never should go around entirely, that would have been
                 * one of the special cases above
                 */
                elmt_prev = elmt;
                elmt = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, index_next);
            } while (elmt->id > elmt_id_min);
            index_found = elmt_prev->index;
        }
    }
    /* If we did not find it here, we have a bug */

    *elmt_index_out = index_found;
    return;
}

/* ---------------------------------------
 * Local (heap) SharedConsistentSet admins
 * ---------------------------------------
 */

/* Owner side maintains a different administration than Observer side */

/*i
 * \brief Enumeration of the possible states of an element in a consistent set.
 */
typedef enum
{
    SQ_ELMT_STATE_UNKNOWN = 0,
    SQ_ELMT_STATE_FREE = 1,
    SQ_ELMT_STATE_RESERVED = 2,
    SQ_ELMT_STATE_COMMITTED = 3,
    SQ_ELMT_STATE_FREEING = 4,
} SQ_ElementState;

/*i
 * \brief Represent an element in a consistent set that is owned by the owner
 *        side.
 */
struct SQ_ElementOwned
{
    /*i
     * \brief The index of the element in the list of elements.
     */
    SQ_Index index;

    /*i
     * \brief The next entry in the list that this sample is a member of.
     *
     * This could be a list of free entries, or a list of reserved entries, or
     * no list at all.
     */
    struct SQ_ElementOwned *next;

    /*i
     * \brief The length of the data associated with this element.
     */
    SQ_Length data_length;

    /*i
     * \brief A pointer to the data associated with this element.
     *
     * This data is not interpreted by this class.
     */
    SQ_MemPtr data;

    /*i
     * \brief The current state describing what is happening with the element.
     */
    SQ_ElementState state;
};

/*i
 * \brief Implementation of a consistent set owned by the owner side.
 */
struct NETIO_ZCOPY_ConsistentSetOwnedImpl
{
    /*i
     * \brief Pointer to the flat (Position Independent) memory.
     */
    struct SQ_PISharedConsistentSet *pi_consistent_set;

    /*i
     * \brief Flag indicating whether to use timestamp as the element ID.
     */
    RTI_BOOL use_timestamp_as_id;

    /*i
     * \brief Number of elements and observers this SharedConsistentSet was
     *        created with.
     */
    SQ_Index elmt_count;
    SQ_Index obsrv_count;

    /*i
     * \brief End nodes of list containing Elements currently ready to be
     *        (re)used.
     */
    struct SQ_ElementOwned *free_list_head;
    struct SQ_ElementOwned *free_list_tail;

    /*i
     * \brief Start node of list containing reserved Elements.
     */
    struct SQ_ElementOwned *reserved_list_head;

    /*i
     * \brief Array of all elements.
     */
    struct SQ_ElementOwned *elmts /*[elmt_count]*/;
};




/*i
 * \brief Represent an observed consistent set.
 *
 * This struct contains information about an observed consistent set,
 * including a pointer to the flat memory, the number of elements in the set,
 * and the latest element seen by the observer.
 */
struct NETIO_ZCOPY_ConsistentSetObservedImpl
{
    /*i
     * \brief Pointer into the flat (Position Independent) memory.
     *
     * This is a pointer to the flat memory where the consistent set is stored.
     */
    struct SQ_PISharedConsistentSet *pi_consistent_set;

    /*i
     * \brief Number of elements this SharedConsistentSet was created with.
     *
     * This is the number of elements that the consistent set was created with.
     */
    SQ_Index elmt_count;

    /*i
     * \brief Indicate if this is a consistent observer.
     *
     * This is a boolean value that indicates if this observer is a consistent
     * observer.
     */
    RTI_BOOL with_consistency;

    /*i
     * \brief Latest element seen by this Observer (when it was iterated over).
     *
     * This is the index of the latest element seen by this observer when it was
     * iterated over.
     */
    SQ_Index obsrv_index;

    /*i
     * \brief Any element committed before this modification id
     *        is considered historical -- id version
     */
    SQ_ModificationId starting_mod_seq_id;

    /*i
     * \brief Any element committed before this modification id
     *        is considered historical -- timestamp version
     */
    SQ_ModificationId starting_mod_timestamp_id;
};


/* Managing the list of free ((re)usable) elements living on the heap */


/*i
 * \brief Push an Element onto the free list, at the beginning.
 *
 * This function adds an element to the beginning of the free list in the
 * NETIO_ZCOPY_ConsistentSetOwned struct.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] elmt Pointer to the SQ_ElementOwned struct to be added to the
 *            free list.
 */
RTI_PRIVATE void
free_list_prepend(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        struct SQ_ElementOwned *elmt)
{
    elmt->next = self->free_list_head;
    self->free_list_head = elmt;
    if (self->free_list_tail == NULL)
    {
        /* This was the first element, update tail as well */
        self->free_list_tail = elmt;
    }
}

/*i
 * \brief Push an Element onto the free list, at the end.
 *
 * This function adds an element to the end of the free list in the
 * NETIO_ZCOPY_ConsistentSetOwned struct.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] elmt Pointer to the SQ_ElementOwned struct to be added to the
 *            free list.
 */
RTI_PRIVATE void
free_list_append(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        struct SQ_ElementOwned *elmt)
{
    if (self->free_list_tail != NULL)
    {
        self->free_list_tail->next = elmt;
    }
    self->free_list_tail = elmt;
    if (self->free_list_head == NULL)
    {
        /* This was the first element, update head as well */
        self->free_list_head = elmt;
    }
    elmt->next = NULL;
}

/*i
 * \brief Push an Element onto the reserved list, at the beginning.
 *
 * This function adds an element to the beginning of the reserved list in the
 * NETIO_ZCOPY_ConsistentSetOwned struct.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] elmt Pointer to the SQ_ElementOwned struct to be added to the
 *            reserved list.
 */
RTI_PRIVATE void
reserved_list_prepend(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        struct SQ_ElementOwned *elmt)
{
    elmt->next = self->reserved_list_head;
    self->reserved_list_head = elmt;
}

/* Callback function when popping with condition. Return RTI_TRUE for the
 * Element that needs to be popped
 */
typedef enum {
    SQ_ACTION_UNDEFINED,
    SQ_ACTION_NONE,
    SQ_ACTION_POP,
    SQ_ACTION_CANCEL,
} SQ_PredicateAction;

/*i
 * \brief Function pointer type for a walk action function.
 *
 * This function pointer type is used to define a function that can be passed
 * as a parameter to the free_list_walk_w_action function. The function is
 * called for each element in the free list of a NETIO_ZCOPY_ConsistentSetOwned
 * struct, and is used to determine whether to perform an action on the element.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] elmt Pointer to the SQ_ElementOwned struct being evaluated.
 * \param[in] user_data Pointer to user-defined data that can be used by the
 *                      walk action function.
 * \param[out] action_out Pointer to an SQ_PredicateAction enum that specifies
 *                        the action to be taken on the element.
 */
typedef void (*SQ_WalkActionFunc)(
        const NETIO_ZCOPY_ConsistentSetOwned *self,
        const struct SQ_ElementOwned *elmt,
        SQ_MemPtr_Const user_data,
        SQ_PredicateAction *action_out);

/*i
 * \brief Walk the free list and perform an action on the first element that
 *        meets the predicate.
 *
 * This function walks the free list of a NETIO_ZCOPY_ConsistentSetOwned struct
 * and performs an action on the first element that meets the predicate. The
 * walk starts at the head and proceeds towards the tail.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] predicate Pointer to a function that is used to determine whether
 *            an element meets the predicate. If the predicate is NULL, it is
 *            assumed to be always true, with a POP action.
 * \param[in] user_data Pointer to user-defined data that can be used by the
 *            predicate function.
 * \param[out] elmt_out Pointer to a pointer to the SQ_ElementOwned struct
 *             that meets the predicate. This is set to NULL if no element meets
 *             the predicate.
 *
 * \return RTI_TRUE if an element meets the predicate and the action has been
 *         performed on it. RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
free_list_walk_w_action(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_WalkActionFunc predicate,
        SQ_MemPtr_Const user_data,
        struct SQ_ElementOwned **elmt_out)
{
    RTI_BOOL result = RTI_FALSE;
    SQ_PredicateAction action = SQ_ACTION_NONE;
    struct SQ_ElementOwned *prev_elmt;
    struct SQ_ElementOwned *cur_elmt;

    prev_elmt = NULL;
    /* Always start at the head */
    cur_elmt = self->free_list_head;

    /* Keep walking as long as no action is indicated */
    while (cur_elmt != NULL)
    {
        predicate(self, cur_elmt, user_data, &action);
        /* Break out if the predicate indicated some action */
        if (action != SQ_ACTION_NONE)
        {
            break;
        }
        /* Move on if the predicate succeeded but indicated 'no action' */
        prev_elmt = cur_elmt;
        cur_elmt = cur_elmt->next;
    }

    switch (action)
    {
        case SQ_ACTION_POP:
            /* Action: pop the Element we found */
            /* Are we removing the head? */
            if (self->free_list_head == cur_elmt)
            {
                /* Remove Element that is the first in the list */
                self->free_list_head = cur_elmt->next;
            }
            /* Are we removing the tail? */
            if (self->free_list_tail == cur_elmt)
            {
                /* Remove Element that is the last in the list */
                self->free_list_tail = prev_elmt;
            }
            /* Re-link the list if needed */
            if (prev_elmt != NULL)
            {
                /* Remove the Element in the middle */
                prev_elmt->next = cur_elmt->next;
            }
            /* Reset pointer to next */
            cur_elmt->next = NULL;
            break;
        case SQ_ACTION_CANCEL:
            /* Action: stop walking */
            break;
        default:
            /* Error -- no action was indicated for any of the Elements */
            goto done;
    }

    /* Export the Element we found */
    *elmt_out = cur_elmt;
    result = RTI_TRUE;

done:
    return result;
}

/*i
 * \brief Walk the reserved list and perform an action on the first element that
 *        meets the predicate.
 *
 * This function walks the reserved list of a NETIO_ZCOPY_ConsistentSetOwned
 * struct and performs an action on the first element that meets the predicate.
 * The walk starts at the head and proceeds towards the tail.
 *
 * \param[in] self Pointer to the NETIO_ZCOPY_ConsistentSetOwned struct.
 * \param[in] predicate Pointer to a function that is used to determine whether
 *            an element meets the predicate. If the predicate is NULL, it is
 *            assumed to be always true, with a POP action.
 * \param[in] user_data Pointer to user-defined data that can be used by the
 *            predicate function.
 * \param[out] elmt_out Pointer to a pointer to the SQ_ElementOwned struct
 *             that meets the predicate. This is set to NULL if no element meets
 *             the predicate.
 *
 * \return RTI_TRUE if an element meets the predicate and the action has been
 *         performed on it. RTI_FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
reserved_list_walk_w_action(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_WalkActionFunc predicate,
        SQ_MemPtr_Const user_data,
        struct SQ_ElementOwned **elmt_out)
{
    RTI_BOOL result = RTI_FALSE;
    SQ_PredicateAction action = SQ_ACTION_NONE;
    struct SQ_ElementOwned *prev_elmt;
    struct SQ_ElementOwned *cur_elmt;

    prev_elmt = NULL;
    /* Always start at the head */
    cur_elmt = self->reserved_list_head;

    /* Keep walking as long as no action is indicated */
    while (cur_elmt != NULL)
    {
        predicate(self, cur_elmt, user_data, &action);
        /* Break out if the predicate indicated some action */
        if (action != SQ_ACTION_NONE)
        {
            break;
        }
        /* Move on if the predicate succeeded but indicated 'no action' */
        prev_elmt = cur_elmt;
        cur_elmt = cur_elmt->next;
    }

    switch (action)
    {
        case SQ_ACTION_POP:
            /* Action: pop the Element we found */
            /* Are we removing the head? */
            if (self->reserved_list_head == cur_elmt)
            {
                /* Remove Element that is the first in the list */
                self->reserved_list_head = cur_elmt->next;
            }
            /* Re-link the list if needed */
            if (prev_elmt != NULL)
            {
                /* Remove the Element in the middle */
                prev_elmt->next = cur_elmt->next;
            }
            /* Reset pointer to next */
            cur_elmt->next = NULL;
            break;
        case SQ_ACTION_CANCEL:
            /* Action: stop walking */
            break;
        default:
            /* Error -- no action was indicated for any of the Elements */
            goto done;
    }

    /* Export the Element we found */
    *elmt_out = cur_elmt;
    result = RTI_TRUE;

done:
    return result;
}


/*i
 * \brief Predicate function used to unreserve an element in the reserved list.
 *
 * This function is used to unreserve an element in the reserved list of a
 * NETIO_ZCOPY_ConsistentSetOwned object. It is called by the
 * reserved_list_walk_w_action function.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetOwned object.
 * \param[in] elmt A pointer to the SQ_ElementOwned object to be checked.
 * \param[in] user_data A pointer to the user data passed to
 *            reserved_list_walk_w_action.
 * \param[out] action_out A pointer to the SQ_PredicateAction to be performed.
 *
 * \sa \ref reserved_list_walk_w_action
 */
RTI_PRIVATE void
unreserve_predicate(
        const NETIO_ZCOPY_ConsistentSetOwned *self,
        const struct SQ_ElementOwned *elmt,
        SQ_MemPtr_Const user_data,
        SQ_PredicateAction *action_out)
{
    UNUSED_ARG(self);

    if (elmt->index == *((const SQ_Index *)user_data))
    {
        /* We found the element, pop it off the reserved list */
        *action_out = SQ_ACTION_POP;
    } else {
        /* This is not the one, do nothing */
        *action_out = SQ_ACTION_NONE;
    }

    return;
}

/*i
 * \brief Commits an element locally.
 *
 * This function is used to commit an element locally in a
 * NETIO_ZCOPY_ConsistentSetOwned object. It changes the state of the element
 * from SQ_ELMT_STATE_RESERVED to SQ_ELMT_STATE_COMMITTED.
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetOwned object.
 * \param[in] elmt_index The index of the element to be committed.
 *
 * \return RTI_TRUE if the function succeeded, RTI_FALSE otherwise.
 *
 * \sa \ref elmt_uncommit_locally
 */
RTI_PRIVATE RTI_BOOL
elmt_commit_locally(NETIO_ZCOPY_ConsistentSetOwned *self, SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_ElementOwned *elmt;

    elmt = &self->elmts[elmt_index];
    if (elmt->state != SQ_ELMT_STATE_RESERVED)
    {
        goto done;
    }

    /* Remove it from the list of reserved elements */
    if (!reserved_list_walk_w_action(self, unreserve_predicate,
            (SQ_MemPtr_Const)&elmt_index, &elmt))
    {
        goto done;
    }

    /* Error if not found */
    if (elmt == NULL)
    {
        goto done;
    }

    /* Change the state */
    elmt->state = SQ_ELMT_STATE_COMMITTED;
    result = RTI_TRUE;

done:
    return result;
}

/*i
 * \brief Uncommits an element locally.
 *
 * This function is used to uncommit an element locally in a
 * NETIO_ZCOPY_ConsistentSetOwned object. It changes the state of the element
 * from SQ_ELMT_STATE_COMMITTED to SQ_ELMT_STATE_FREEING
 *
 * \param[in] self A pointer to the NETIO_ZCOPY_ConsistentSetOwned object.
 * \param[in] elmt_index The index of the element to be uncommitted.
 *
 * \return RTI_TRUE if the function succeeded, RTI_FALSE otherwise.
 *
 * \pre The element must be in the SQ_ELMT_STATE_COMMITTED state.
 *
 * \sa \ref elmt_commit_locally
 */
RTI_PRIVATE RTI_BOOL
elmt_uncommit_locally(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_ElementOwned *elmt;

    elmt = &self->elmts[elmt_index];
    if (elmt->state != SQ_ELMT_STATE_COMMITTED)
    {
        /* Can not uncommit a non-committed element */
        goto done;
    }

    /* All uncommited elements transition to FREEING, indicating that they
     *   are still available, but will be considered for reuse when needed */
    elmt->state = SQ_ELMT_STATE_FREEING;
    /* Append so already available elements will be reused first */
    free_list_append(self, elmt);

    result = RTI_TRUE;

done:
    return result;
}

/* ----------------
 * Class operations
 * ----------------
 */

RTI_BOOL
NETIO_ZCOPY_ConsistentSet_get_mem_size(
        SQ_Index elmt_count,
        SQ_Index obsrv_count,
        SQ_Length *length_out)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            length_out == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("length_out", length_out, RTI_TRUE);
    )

    /* Limits for elmt_count are the same as for max_samples.
     * Limits for obrv_count are the same as for max_remote_readers.
     */
    if ((1 > elmt_count) || (100000000 < elmt_count) || (1 > obsrv_count) ||
        (100000000 < obsrv_count))
    {
        goto done;
    }

    *length_out = pi_size_base() + pi_size_element_list(elmt_count) +
                  pi_size_observer_list(elmt_count, obsrv_count);

    result = RTI_TRUE;

done:
    return result;
}

/* ----------------------------
 * Constructors and destructors
 * ----------------------------
 */
RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_create(
        SQ_Index elmt_count,
        SQ_Index obsrv_count,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory /*[flat_memory_length]*/,
        RTI_BOOL use_timestamp_as_id,
        NETIO_ZCOPY_ElementDataInitializerFunc initializer,
        void *initializer_param,
        NETIO_ZCOPY_ConsistentSetOwned **self_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct NETIO_ZCOPY_ConsistentSetOwnedImpl *self = NULL;
    struct SQ_ElementOwned *elmts = NULL;
    SQ_Length length_needed;
    struct SQ_PISharedConsistentSet *pi_consistent_set;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (flat_memory == NULL) || (self_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("flat_memory", flat_memory, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("self_out", self_out, RTI_TRUE);
    )

    if (!NETIO_ZCOPY_ConsistentSet_get_mem_size(
                elmt_count,
                obsrv_count,
                &length_needed) ||
        (flat_memory_length < length_needed))
    {
        goto done;
    }

    /* Construct result */
    OSAPI_Heap_allocate_struct(&self, struct NETIO_ZCOPY_ConsistentSetOwnedImpl);
    if (self == NULL)
    {
        goto done;
    }
    OSAPI_Heap_allocate_array(&elmts, elmt_count, struct SQ_ElementOwned);
    if (elmts == NULL)
    {
        goto done;
    }

    /* Set pointer into flat memory and initialize its fields */
    pi_consistent_set = (struct SQ_PISharedConsistentSet *)flat_memory;
    *pi_consistent_set = (struct SQ_PISharedConsistentSet)
    {
            .set_size = length_needed,
            .newest_id = 0,
            .newest_index = SQ_INDEX_NONE,
            .elmt_count = elmt_count,
            .elmt_offset = pi_size_base(),
            .elmt_size = pi_size_element(),
            .obsrv_count = obsrv_count,
            .obsrv_offset = pi_size_base() + pi_size_element_list(elmt_count),
            .obsrv_size = pi_size_observer(elmt_count)
    };

    *self = (struct NETIO_ZCOPY_ConsistentSetOwnedImpl)
    {
            .pi_consistent_set = pi_consistent_set,
            .use_timestamp_as_id = use_timestamp_as_id,
            .elmt_count = elmt_count,
            .obsrv_count = obsrv_count,
            .free_list_head = NULL,
            .free_list_tail = NULL,
            .reserved_list_head = NULL,
            .elmts = elmts,
    };

    /* Initialization of elements in list */
    for (SQ_Index i = 0; i < elmt_count; i++)
    {
        SQ_Length data_length = 0;
        SQ_MemPtr data = NULL;

        /* Position Independent attributes */
        *NETIO_ZCOPY_ConsistentSet_get_element(pi_consistent_set, i) = (struct SQ_PIElement)
        {
                .index = i,
                .ref_count = 0,
                .is_valid = RTI_FALSE,
                .modification_id_committed = {{0}},
                .modification_id_purged = {{0}},
                .valid_list_node = SQ_gv_PIListNodeNil,
                .id = 0,
        };
        /* Local (heap) attributes */
        if (initializer != NULL)
        {
            initializer(i, &data_length, &data, initializer_param);
        }
        elmts[i] = (struct SQ_ElementOwned)
        {
                .index = i,
                .next = NULL,
                .data_length = data_length,
                .data = data,
                .state = SQ_ELMT_STATE_FREE,
        };
        /* Starting off, all elements are free to use */
        free_list_append(self, &elmts[i]);
    }

    /* Observers in list, Process Independent only */
    for (SQ_Index i = 0; i < obsrv_count; i++)
    {
        struct SQ_PIObserver *obsrv = NETIO_ZCOPY_ConsistentSet_get_observer(pi_consistent_set, i);
        obsrv->bitmap_byte_count = SQ_BITNSLOTS(elmt_count);
        OSAPI_Memory_zero(&obsrv->locking_bitmap[0], obsrv->bitmap_byte_count);
    }

    /* Output and result */
    *self_out = self;
    result = RTI_TRUE;

done:
    if (!result)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_array(elmts);
        OSAPI_Heap_free_struct(self);
#endif /* !RTI_CERT */
    }
    /* The memory for elmts and self is leaked here in the error case only when
     * compiling with RTI_CERT because memory is intentionally never freed
     * for cert.
     */
    /* coverity[leaked_storage] */
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_destroy(NETIO_ZCOPY_ConsistentSetOwned *self)
{
    if (self == NULL)
    {
        goto done;
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_array(self->elmts);
    OSAPI_Heap_free(self);
#endif /* !RTI_CERT */

done:
    return RTI_TRUE;
}

void
NETIO_ZCOPY_ConsistentSetObserved_reset(
        NETIO_ZCOPY_ConsistentSetObserved *self)
{
    if (self != NULL)
    {
        self->pi_consistent_set = NULL;
        self->elmt_count = SQ_INDEX_NONE;
        self->with_consistency = RTI_TRUE;
        self->obsrv_index = SQ_INDEX_NONE;
    }
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_destroy(
            NETIO_ZCOPY_ConsistentSetObserved *self)
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

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_initialize(
        NETIO_ZCOPY_ConsistentSetObserved **self_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct NETIO_ZCOPY_ConsistentSetObservedImpl *self = NULL;

    OSAPI_PRECONDITION(
            (self_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self_out", self_out, RTI_TRUE);
    )

    /* Construct result */
    OSAPI_Heap_allocate_struct(&self, struct NETIO_ZCOPY_ConsistentSetObservedImpl);
    if (self == NULL)
    {
        goto done;
    }


    *self = (struct NETIO_ZCOPY_ConsistentSetObservedImpl)
    {
        .pi_consistent_set = NULL,
        .elmt_count = SQ_INDEX_NONE,
        .with_consistency = RTI_TRUE,
        .obsrv_index = SQ_INDEX_NONE,
        .starting_mod_seq_id = {{0}},
        .starting_mod_timestamp_id = {{0}},
    };

    *self_out = self;
    result = RTI_TRUE;

done:
    if (!result)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(self);
#endif /* !RTI_CERT */
    }
    /* The memory for self is leaked here in the error case only when
     * compiling with RTI_CERT because memory is intentionally never freed in
     * the Connext Cert runtime.
     */
    /* coverity[leaked_storage] */
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_open(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        RTI_BOOL with_consistency,
        SQ_Index obsrv_index,
        SQ_Length flat_memory_length,
        SQ_MemPtr flat_memory,
        RTI_BOOL initial)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedConsistentSet *pi_consistent_set;
    SQ_Index my_index = SQ_INDEX_NONE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (flat_memory == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
            OSAPI_Log_entry_add_pointer("flat_memory", flat_memory, RTI_FALSE);)

    /* Point to the structure in the memory */
    pi_consistent_set = (struct SQ_PISharedConsistentSet *)flat_memory;

    /* Memory size has to be large enough to even inspect the required size */
    if ((flat_memory_length < pi_size_base()) ||
            (flat_memory_length < pi_consistent_set->set_size))
    {
        goto done;
    }

    /* If consistency is requested, find an open slot */
    if (with_consistency)
    {
        if (obsrv_index >= pi_consistent_set->obsrv_count)
        {
            goto done;
        }
        my_index = obsrv_index;
    }

    self->pi_consistent_set = pi_consistent_set;
    self->elmt_count = pi_consistent_set->elmt_count;
    self->with_consistency = with_consistency;
    self->obsrv_index = my_index;

    /* For an initial attachment, we consider everything that is already committed
     * to be historical, so we start observing at the current newest id / timestamp.
     * For a subsequent, observation start at 0 meaning that nothing will be
     * considered historical.
     */
    result = RTI_TRUE;
    if (initial)
    {
        result = result && SQ_ModificationId_from_elmt_id(&self->starting_mod_seq_id,
            pi_consistent_set->newest_id, RTI_FALSE);
        result = result && SQ_ModificationId_from_elmt_id(&self->starting_mod_timestamp_id,
            pi_consistent_set->newest_id, RTI_TRUE);
    }
    else
    {
        SQ_ModificationId_minimize(&self->starting_mod_seq_id, RTI_FALSE);
        SQ_ModificationId_minimize(&self->starting_mod_timestamp_id, RTI_TRUE);
    }

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_close(NETIO_ZCOPY_ConsistentSetObserved *self)
{
    if (self == NULL)
    {
        goto done;
    }

    /* Any call to NETIO_ZCOPY_ConsistentSetObserved_close for a
     * NETIO_ZCOPY_ConsistentSetObserved that has not yet been opened would be a
     * user error, but we check for a NULL pi_consistent_set here just in the
     * interest of defensive programming.
     */
    if ((self->with_consistency) && (self->pi_consistent_set != NULL))
    {
        /* Remove myself from the observer list in flat memory */
        pi_obsrv_remove(self->pi_consistent_set, self->obsrv_index);
    }

done:
    return RTI_TRUE;
}

/*----------------------
 * Owner-side functions
 * ---------------------
 */

/* Functions to be used on the owner side of the collection only, because the
 * owner manages the contents and therefore knows what is needed to look up etc.
 */

/*i
 * \brief Predicate function to determine if an element can be reserved.
 *
 * This function is used to determine if an element in a consistent set can be
 * reserved. It checks if the element is in the process of being freed and if it
 * is no longer loaned by any reader. If the element can be reserved, it is
 * popped off the free list.
 *
 * \param[in] self The consistent set to which the element belongs.
 * \param[in] elmt The element to check.
 * \param[in] user_data Unused user data.
 * \param[out] action_out The action to take on the element if it can be
 *             reserved.
 */
RTI_PRIVATE void
can_be_reserved_predicate(
        const NETIO_ZCOPY_ConsistentSetOwned *self,
        const struct SQ_ElementOwned *elmt,
        SQ_MemPtr_Const user_data,
        SQ_PredicateAction *action_out)
{
    RTI_BOOL can_be_reserved = RTI_TRUE;
    UNUSED_ARG(user_data);

    /* Free-ing elements can only be reserved if they are no longer
     *     loaned by any reader */
    if (elmt->state == SQ_ELMT_STATE_FREEING)
    {
        can_be_reserved = pi_elmt_lock_probe_acquire_write(
            self->pi_consistent_set, elmt->index);
    }

    /* If it can be reserved indeed, we want to pop it off the free list */
    if (can_be_reserved)
    {
        *action_out = SQ_ACTION_POP;
    }

    return;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_reserve(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index *elmt_index_out,
        SQ_Length *elmt_data_length_out,
        SQ_MemPtr *elmt_data_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_ElementOwned *elmt = NULL;
    struct SQ_PIElement *pi_elmt;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (elmt_index_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("elmt_index_out", elmt_index_out, RTI_TRUE);
    )

    /* Take the first free element (if available) */
    if (!free_list_walk_w_action(self, can_be_reserved_predicate, NULL, &elmt))
    {
        /* No free element available */
        goto done;
    }

    /* Now that the element is reserved, it is no longer valid to read */
    pi_elmt = NETIO_ZCOPY_ConsistentSet_get_element(
        self->pi_consistent_set, elmt->index);
    if (pi_elmt->is_valid)
    {
        pi_elmt_invalidate(self->pi_consistent_set, pi_elmt, self->use_timestamp_as_id);
    }

    /* Prepend the popped element onto the list of reserved elements */
    elmt->state = SQ_ELMT_STATE_RESERVED;
    reserved_list_prepend(self, elmt);

    *elmt_index_out = elmt->index;
    *elmt_data_length_out = elmt->data_length;
    *elmt_data_out = elmt->data;

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_unreserve(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_ElementOwned *elmt;

    /* Preconditions */
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
    )

    /* Bounds checking */
    if (elmt_index >= self->elmt_count)
    {
        goto done;
    }

    /* Nothing has been committed yet so this action is entirely local, no
     * actions in the shared consistent set itself required
     */
    elmt = &self->elmts[elmt_index];
    if (elmt->state != SQ_ELMT_STATE_RESERVED)
    {
        goto done;
    }

    /* Remove it from the list of reserved elements */
    if (!reserved_list_walk_w_action(self, unreserve_predicate,
        (SQ_MemPtr_Const)&elmt_index, &elmt))
    {
        goto done;
    }

    /* Error if not found */
    if (elmt == NULL)
    {
        goto done;
    }

    /* Prepend so this will be the first element to be reused */
    free_list_prepend(self, elmt);
    elmt->state = SQ_ELMT_STATE_FREE;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_commit(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedConsistentSet *pi_consistent_set;
    struct SQ_PIElement *pi_elmt;

    /* Preconditions*/
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
    )

    /* Bounds checking */
    if (elmt_index >= self->elmt_count)
    {
        goto done;
    };

    /* Get the element we want to commit */
    pi_consistent_set = self->pi_consistent_set;
    pi_elmt = NETIO_ZCOPY_ConsistentSet_get_element(pi_consistent_set, elmt_index);
    /* It can not be already valid/committed before */
    if (pi_elmt->is_valid)
    {
        goto done;
    }

    /* Commit elmt locally */
    if (!elmt_commit_locally(self, elmt_index))
    {
        goto done;
    }

    /* Finally commit elmt in shared region, so it becomes valid for all */
    pi_elmt_validate(pi_consistent_set, pi_elmt, self->use_timestamp_as_id);

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_element_purge(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedConsistentSet *pi_consistent_set;
    struct SQ_PIElement *pi_elmt;

    /* Preconditions*/
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
    )

    /* Bounds checking */
    if (elmt_index >= self->elmt_count)
    {
        goto done;
    }

    /* Uncommit locally only */
    if (!elmt_uncommit_locally(self, elmt_index))
    {
        goto done;
    }

    pi_consistent_set = self->pi_consistent_set;

    /* Mark the element as purged */
    pi_elmt = NETIO_ZCOPY_ConsistentSet_get_element(pi_consistent_set, elmt_index);
    pi_elmt_purge(pi_consistent_set, pi_elmt, self->use_timestamp_as_id);

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_observer_evict(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_Index obsrv_index)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
    )

    /* Bounds checking */
    if (obsrv_index >= self->obsrv_count)
    {
        goto done;
    }

    pi_obsrv_remove(self->pi_consistent_set, obsrv_index);
    result = RTI_TRUE;

done:
    return result;
}


/*i
 * \brief Predicate function used to lookup a reserved element by its data.
 *
 * \param[in] self The consistent set.
 * \param[in] elmt The element to check.
 * \param[in] user_data The user data to compare against the element's data.
 * \param[out] action_out The action to take based on the comparison.
 *
 * \sa \ref NETIO_ZCOPY_ConsistentSetOwned_lookup_reserved_elmt_by_data
 */
RTI_PRIVATE void
lookup_reserved_elmt_id_predicate(
        const NETIO_ZCOPY_ConsistentSetOwned *self,
        const struct SQ_ElementOwned *elmt,
        SQ_MemPtr_Const user_data,
        SQ_PredicateAction *action_out)
{
    UNUSED_ARG(self);

    if (elmt->data == user_data) {
        /* Found it, cancel the walk */
        *action_out = SQ_ACTION_CANCEL;
    } else {
        /* This is not it, keep walking */
        *action_out = SQ_ACTION_NONE;
    }

    return;
}


RTI_BOOL
NETIO_ZCOPY_ConsistentSetOwned_lookup_reserved_elmt_by_data(
        NETIO_ZCOPY_ConsistentSetOwned *self,
        SQ_MemPtr_Const user_data,
        SQ_Index* elmt_index_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_ElementOwned *elmt_found;

    if (!reserved_list_walk_w_action(self, lookup_reserved_elmt_id_predicate,
            user_data, &elmt_found))
    {
        goto done;
    }

    *elmt_index_out = elmt_found->index;
    result = RTI_TRUE;

done:
    return result;
}


/*----------------------
 * Observer-side functions
 * ---------------------
 */
RTI_PRIVATE RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_is_mod_after_started(
        const NETIO_ZCOPY_ConsistentSetObserved *self,
        const SQ_ModificationId *mod_id)
{
    RTI_BOOL result;

    if (SQ_ModificationId_is_seq_nr(mod_id))
    {
        /* It's a sequence number, compare as such */
        result = self->starting_mod_seq_id.seq_nr.id < mod_id->seq_nr.id;
    }
    else
    {
        /* It's a timestamp, compare as such */
        result = OSAPI_SystemTime_compare(
            &self->starting_mod_timestamp_id.timestamp, &mod_id->timestamp) <= 0;
    }

    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_elements_iterate(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId starting_elmt_id,
        SQ_Index starting_elmt_index, /* for faster access */
        NETIO_ZCOPY_VisitElementFunc visit_func,
        void *visit_data)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedConsistentSet *consistent_set;
    SQ_Index newest_index;
    SQ_ElmtId newest_id;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (visit_func == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("visit_func", (const void *)&visit_func, RTI_TRUE);
    )

    /* Bounds checking */
    if ((starting_elmt_index != SQ_INDEX_NONE) &&
            (starting_elmt_index >= self->elmt_count))
    {
        goto done;
    };

    /* Get info about the latest stuff happening */
    consistent_set = self->pi_consistent_set;
    newest_index = consistent_set->newest_index;
    if (newest_index != SQ_INDEX_NONE)
    {
        newest_id = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, newest_index)->id;
    }
    else
    {
        /* the assumption here is that an element with sequence number == 0
         * can not exist
         */
        newest_id = 0;
    }

    /* Are we starting before the end? */
    if (starting_elmt_id < newest_id)
    {
        RTI_BOOL terminated = RTI_FALSE;
        SQ_Index elmt_index;
        RTI_BOOL visits_completed = RTI_FALSE;
        struct SQ_PIElement *pi_elmt = NULL;

        pi_elmt_find_after(consistent_set,
            starting_elmt_id, starting_elmt_index, &elmt_index);

        /* Start walking until the end (or until termination is requested) */
        while (!visits_completed)
        {
            pi_elmt = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index);
            /* Do not walk over elements purged before the starting modification id */
            if (NETIO_ZCOPY_ConsistentSetObserved_is_mod_after_started(self,
                &pi_elmt->modification_id_purged))
            {
                terminated = !visit_func(pi_elmt->id, pi_elmt->index, visit_data);
            }
            if ((elmt_index != newest_index) && !terminated)
            {
                elmt_index = pi_elmt->valid_list_node.next_index;
            }
            else
            {
                visits_completed = RTI_TRUE;
            }
        }
    }

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_element_protect_if_valid(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId elmt_id,
        SQ_Index elmt_index,
        RTI_BOOL *is_valid_out)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL is_elmt_consistent = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (is_valid_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("is_valid_out", is_valid_out, RTI_TRUE);
    )

    /* Check if this is the element that we expect it to be and that it is still
     * valid (ie not freeing yet)
     */
    if (!NETIO_ZCOPY_ConsistentSetObserved_get_element_is_consistent(
                self,
                elmt_id,
                elmt_index,
                &is_elmt_consistent))
    {
        goto done;
    }

    if (is_elmt_consistent && self->with_consistency)
    {
        /* If consistency is required, mark it as locked. */
        pi_elmt_lock_acquire_read(self->pi_consistent_set,
                elmt_index, self->obsrv_index);
    }

    *is_valid_out = is_elmt_consistent;
    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_element_unprotect(
        NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_Index elmt_index)
{
    RTI_BOOL result = RTI_FALSE;

    /* Preconditions */
    OSAPI_PRECONDITION(
            self == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);
    )

    /* Bounds checking */
    if (elmt_index >= self->elmt_count)
    {
        goto done;
    }

    /* Actual action only required if consistency is needed */
    if (self->with_consistency)
    {
        /* Returning the loan is nothing but releasing sample's lock */
        pi_elmt_lock_release_read(self->pi_consistent_set,
                elmt_index, self->obsrv_index);
    }

    result = RTI_TRUE;

done:
    return result;
}

RTI_BOOL
NETIO_ZCOPY_ConsistentSetObserved_get_element_is_consistent(
        const NETIO_ZCOPY_ConsistentSetObserved *self,
        SQ_ElmtId elmt_id,
        SQ_Index elmt_index,
        RTI_BOOL *is_elmt_consistent_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct SQ_PISharedConsistentSet *consistent_set;
    struct SQ_PIElement *elmt;

    /* Preconditions */
    OSAPI_PRECONDITION(
            (self == NULL) || (is_elmt_consistent_out == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("self", is_elmt_consistent_out, RTI_TRUE);
    )
    /* Bounds checking */
    if (elmt_index >= self->elmt_count)
    {
        goto done;
    }

    /* Check if this is the element that we expect it to be and that it is still
     * valid (i.e. not freed yet)
     */
    consistent_set = self->pi_consistent_set;
    elmt = NETIO_ZCOPY_ConsistentSet_get_element(consistent_set, elmt_index);
    *is_elmt_consistent_out = (elmt->id == elmt_id) && (elmt->is_valid);
    result = RTI_TRUE;

done:
    return result;
}

/*i @} */
