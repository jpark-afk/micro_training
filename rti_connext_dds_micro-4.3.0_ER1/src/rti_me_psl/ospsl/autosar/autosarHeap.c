/*
 * FILE: autosarHeap.c - AutoSAR heap functionality
 *
 * Copyright 2012-2026 Real-Time Innovations, Inc.
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
 * 14sep2021,tk MICRO-3199/PR.29543
 * - Ensure a 0-sized allocation returns NULL in OSAPI_Heap_allocate_buffer
 * 21feb2021,tk MICRO-2583/PR.28144
 *   - Removed redundant assignment of NULL_PTR to buffer in
 *     OSAPI_Heap_allocate_buffer().
 * 10oct2020,fmt MICRO-2612/PR.28219
 *    - OSAPI_Heap_allocate_buffer() check if an allocation fits in the
 *      heap area using operator <= instead of only <.
 * 6oct2020,fmt MICRO-2585/PR.28155
 *    - Move consistency check of properties from
 *      OSAPI_AutosarHeap_is_initialized() to
 *      OSAPI_SystemAutosar_initialize()
 * 2oct2020,fmt MICRO-2584/PR.28152
 *    - Remove check for NULL pointer which are not needed
 * 1oct2020,fmt MICRO-2580/PR.28122
 *    - Use RTI_SIZE_T as the type of the size function parameter in
 *      OSAPI_Heap_allocate_buffer for consistency with the public API.
 * 1oct2020,fmt MICRO-2573/PR.28144
 *    - Set return buffer to NULL if OSAPI_Heap_allocate_buffer() fails
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 21may2020,fmt MICRO-2405/PR.27629 Simplify Autosar initialize() functions
 * 25aug2020,fmt MICRO-2443/PR.27634
 *    - Remove OSAPI_Heap_free_buffer() from autosarHeap.c for CERT builds
 * 13mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of OSAPI heap routines
 */

#include "rti_me_psl.h"

#include "autosarMutex.h"
#include "autosarHeap.h"

/*** SOURCE_BEGIN ***/
/*ci \brief Whether the Autosar heap have been initialized or not.
 */
RTI_PRIVATE RTI_BOOL OSAPI_AutosarHeap_fv_IsInitialized = RTI_FALSE;

/*ci \brief Current offset to different heap areas is allocated at the beginning
 * of the first heap area.
 */
RTI_PRIVATE P2VAR(uint32, AUTOMATIC, SOAD_APPL_DATA)
OSAPI_AutosarHeap_fv_HeapOffset = NULL_PTR;

/*ci \brief Mutex to use. Note that this pointer does not need to be freed since
 * is is either pointing to pre-allocated memory above or an internal
 * structure that should not be freed.
 */
RTI_PRIVATE P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA)
OSAPI_AutosarHeap_fv_Mutex = NULL_PTR;

RTI_SIZE_T OSAPI_gv_AllocatedByteCount = 0;

RTI_BOOL OSAPI_Heap_gv_disabled_alloc = RTI_FALSE;

/* OSAPI autosar module private functions */

/*ci \brief Ensures that Autosar heap module is initialized.
 *
 *  \return TRUE if module is initialized with no error or already initialized.
 *          Otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
OSAPI_AutosarHeap_is_initialized(void)
{
    uint32 initial_offset;
    uint32 i;
    
    if (OSAPI_AutosarHeap_fv_IsInitialized)
    {
        return TRUE;
    }

    /* Initialize current offsets for each heap area
     */
    initial_offset =
        ((char*)((((OSAPI_System_gv_PortProperty->heap_area[0] - ((char*)0)) +
         (sizeof(uint32) - 1)) & ~(sizeof(uint32) - 1)))) -
         OSAPI_System_gv_PortProperty->heap_area[0];

    OSAPI_AutosarHeap_fv_HeapOffset =
        (P2VAR(uint32, AUTOMATIC, SOAD_APPL_DATA))
            (OSAPI_System_gv_PortProperty->heap_area[0] +
             initial_offset);

    OSAPI_AutosarHeap_fv_HeapOffset[0] = initial_offset +
        (sizeof(uint32) * OSAPI_System_gv_PortProperty->number_of_heap_areas);
    for (i = 1; i < OSAPI_System_gv_PortProperty->number_of_heap_areas; i++)
    {
        OSAPI_AutosarHeap_fv_HeapOffset[i] = 0;
    }

    /* Only create mutex if thread-safe heap is enabled */
    if (OSAPI_System_gv_PortProperty->enable_thread_safe_heap)
    {
        OSAPI_AutosarHeap_fv_Mutex = OSAPI_Mutex_new();

        if (OSAPI_AutosarHeap_fv_Mutex == NULL_PTR)
        {
            return FALSE;
        }
    }

    OSAPI_AutosarHeap_fv_IsInitialized = RTI_TRUE;

    return TRUE;
}

/* OSAPI autosar module public functions */

#ifndef RTI_CERT
FUNC(void, SOAD_CODE)
OSAPI_AutosarHeap_finalize(void)
{
    OSAPI_AutosarHeap_fv_HeapOffset = NULL_PTR;
    OSAPI_AutosarHeap_fv_Mutex = NULL_PTR;
    OSAPI_AutosarHeap_fv_IsInitialized = FALSE;
}
#endif

/* OSAPI public functions */

/* ----------------------------------------------------------------- */
/* allocate the specified size of memory and align it as well */
FUNC(void, SOAD_CODE)
OSAPI_Heap_allocate_buffer(P2VAR(char*, AUTOMATIC, SOAD_APPL_DATA) buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment)
{
    RTI_BOOL ret_value;
    uint32 new_system_heap_offset;
    uint32 i;

    OSAPI_PRECONDITION((buffer == NULL_PTR),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    *buffer = NULL_PTR;

    if (!OSAPI_AutosarHeap_is_initialized())
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return;
    }

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return;
    }
#endif /* !RTI_CERT */

    if (size == 0)
    {
        return;
    }

    if (alignment == OSAPI_ALIGNMENT_DEFAULT)
    {
        alignment = sizeof(void*);
    }

    /* Only use mutex synchronization if thread-safe heap is enabled */
    if (OSAPI_System_gv_PortProperty->enable_thread_safe_heap)
    {
        ret_value = OSAPI_Mutex_take(OSAPI_AutosarHeap_fv_Mutex);
        if (ret_value != RTI_TRUE)
        {
            OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
            return;
        }
    }
    
    for (i = 0; i < OSAPI_System_gv_PortProperty->number_of_heap_areas; i++)
    {
        /* align correctly in case current heap start address is not correctly 
         * aligned
         */
        *buffer = (char*)((((OSAPI_System_gv_PortProperty->heap_area[i] -
                            ((char*)0)) + OSAPI_AutosarHeap_fv_HeapOffset[i]) +
                  (alignment - 1)) & ~(alignment - 1));
        new_system_heap_offset = (*buffer) -
                                 OSAPI_System_gv_PortProperty->heap_area[i];

        if ((new_system_heap_offset + size) <=
            OSAPI_System_gv_PortProperty->heap_area_size[i])
        {
            OSAPI_AutosarHeap_fv_HeapOffset[i] = new_system_heap_offset + size;

            OSAPI_Memory_zero(*buffer, size);

#ifndef RTI_CERT
            OSAPI_gv_AllocatedByteCount += size;
#endif /* !RTI_CERT */
            break;
        }
        else
        {
            /* It is necessary to reset buffer to a NULL_PTR in case it was
             * pointing to insufficient memory.
             */
            *buffer = NULL_PTR;
        }
    }

    /* Only release mutex if thread-safe heap is enabled */
    if (OSAPI_System_gv_PortProperty->enable_thread_safe_heap)
    {
        /* not much that we can do at this point if release resource fails */
        ret_value = OSAPI_Mutex_give(OSAPI_AutosarHeap_fv_Mutex);
        IGNORE_RETVAL(ret_value);
    }

#if OSAPI_ENABLE_LOG
    if ((*buffer) == NULL_PTR)
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
    }
#endif /* OSAPI_ENABLE_LOG */
}

/* ----------------------------------------------------------------- */
/* free the previously allocated memory */
#ifndef RTI_CERT
FUNC(void, SOAD_CODE)
OSAPI_Heap_free_buffer(P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) buffer)
{
    OSAPI_PRECONDITION(buffer == NULL_PTR,return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief
 * Disable memory allocation
 *
 * \details
 * This function disables memory allocation by setting a global flag.
 */
void
OSAPI_Heap_disable_alloc(void)
{
    OSAPI_Heap_gv_disabled_alloc = RTI_TRUE;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Return the number of bytes allocated through OSAPI_Heap functions
 *
 * \details
 * This function returns the number of bytes which have been allocated with the
 * OSAPI_Heap functions. The number is guaranteed to be no less than the sum
 * of all the sizes that have been allocated, but is not guaranteed to take
 * into account the overhead of the memory allocation itself.
 *
 * \return Number of bytes allocated
 */
RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count(void)
{

    return OSAPI_gv_AllocatedByteCount;
}
#endif /* !RTI_CERT */
