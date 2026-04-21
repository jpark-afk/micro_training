/*
 * FILE: osapi_heap.h - Heap interface definition
 *
 * (c) Copyright, Real-Time Innovations, 2008-2024
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
 * 29apr2014,as MICRO-775 Remove unused OSAPI_Heap_reallocate_string
 * 28apr2014,as MICRO-228 (Verocel PR#1407) Document that allocation operations
 *              of OSAPI_Heap always expect a size greater than 0.
 * 22sep2008,tk Written
 */
/*ce
 * \file 
 * \brief Heap interface definition
 */
#ifndef osapi_heap_h
#define osapi_heap_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

OSAPIDllVariable extern const void *OSAPI_CC_NullPtr;

#define MACRO_TYPE RTI_UINT32

/*e \defgroup OSAPI_HeapClass OSAPI Heap
 *  \ingroup OSAPIModule
 */

/*ce \ingroup OSAPI_HeapClass
  @brief The OSAPIAlignment is the alignment in bytes; an address
  is aligned when it is a positive integer multiple of the alignment
*/
typedef RTI_INT32 OSAPI_Alignment_T;

/*e \ingroup OSAPI_HeapClass
  @brief Certain methods allow a default alignment: this should
  be an alignment that follows the "malloc" alignment of
  the architecture (aligned sufficiently to store any C-structure
  efficiently).
*/
#define OSAPI_ALIGNMENT_DEFAULT (-1)

/*e \ingroup OSAPI_HeapClass
 * \brief Allocates zero-initialized memory from the heap.
 *
 * \details
 *
 * The function allocates `count` elements of size `size`
 * and sets the memory region to 0.
 *
 * NOTE: This operation assumes the specified size to be > 0; this operation
 * is never used directly but it is only accessed by using one of the other
 * allocate operations of OSAPI_Heap (e.g. allocate_struct, allocate_array,
 * allocate_string...); these operations are implemented as macros and accept
 * a "type" argument which is always converted to a size > 0 using the sizeof
 * operator.
 *
 * \param[in] count The number of elements to allocate.
 *
 * \param[in] size  The size of the element; size is assumed to be always
 *                  greater than 0.
 *
 * \return. A pointer to a contiguous memory area guaranteed to be large
 * enough store `count` elements of size `size`.
 *
 * \sa \ref OSAPI_Heap_free
 */
OSAPIDllExport void*
OSAPI_Heap_allocate(RTI_SIZE_T count,RTI_SIZE_T size);

/*e \ingroup OSAPI_HeapClass
 * \brief Reallocate  memory from the heap.
 *
 * \param[in] ptr Currently allocated buffer
 *
 * \param[in] size The new desired size
 *
 * \return. A pointer to a contiguous memory area guaranteed to be large
 * enough store size bytes, NULL if the reallocation failed.
 *
 * \sa \ref OSAPI_Heap_free
 */
OSPSLDllExport void*
OSAPI_Heap_realloc(void *ptr,RTI_SIZE_T size);

#ifndef RTI_CERT
/*e \ingroup OSAPI_HeapClass
 *
 * \brief Frees memory allocated from the heap.
 *
 * \details
 *
 * The function frees memory previously allocated with \ref OSAPI_Heap_allocate
 * back to the heap.
 *
 * \param[in] ptr Pointer to region previous allocated
 *                 with \ref OSAPI_Heap_allocate
 *
 *
 * \sa \ref OSAPI_Heap_allocate
 */
OSAPIDllExport void
OSAPI_Heap_free(void *ptr);
#endif

/*i \ingroup OSAPI_HeapClass
    \brief Allocates space on the heap for a C structure and
    initializes the structure with 0's.

    Example:
    \code
    struct MyStruct *myStruct;
    OSAPI_Heap_allocate_struct(&myStruct, struct MyStruct);
    if (myStruct==NULL) {
        return;
    }
    \endcode

    The returned space must be freed with \ref OSAPI_Heap_free_struct.

    NOTE: This operation is implemented as a macro in order to support the
    specification of a "type" argument (converted to a size value using the
    sizeof operator).
*/
OSAPIDllExport void
OSAPI_Heap_allocate_struct(MACRO_TYPE **pointer, MACRO_TYPE);


#ifndef RTI_CERT
/*i \ingroup OSAPI_HeapClass
    \brief Returns previously allocated
    (with \ref OSAPI_Heap_allocate_struct) space to the heap.

    @param[in] pointer If NULL, no op.

    Example:
    \code
    char *c;
    OSAPI_Heap_allocate_struct(&c, char);
    OSAPI_Heap_free_struct(c);
    \endcode
 */
OSAPIDllExport void
OSAPI_Heap_free_struct(MACRO_TYPE *pointer);
#endif

/*i \ingroup OSAPI_HeapClass
    \brief A static method to allocate space on the heap for an array of C
    structures; the array is initialized with 0's.

    Example:
    \code
    struct MyElement *array;
    OSAPI_Heap_allocate_struct(&array, 7, struct MyElement);
    if (array==NULL) {
        return;
    }
    // elements of the array can be accessed as array[i]
    \endcode

    The returned space must be freed with \ref OSAPI_Heap_free_array.

    NOTE: This operation is implemented as a macro in order to support the
    specification of a "type" argument (converted to a size value using the
    sizeof operator).
*/
OSAPIDllExport void
OSAPI_Heap_allocate_array(MACRO_TYPE ** pointer, int count,MACRO_TYPE);


#ifndef RTI_CERT
/*i \ingroup OSAPI_HeapClass
    \brief A static method to return space (that was previously
    allocated with \ref OSAPI_Heap_allocate_array) to the heap.

    Example:
    \code
    char *c;
    OSAPI_Heap_allocate_array(&c, 1, char);
    OSAPI_Heap_free_array(c);
    \endcode

    @param[in] storage If NULL, no op.
 */
OSAPIDllExport void
OSAPI_Heap_free_array(MACRO_TYPE *storage);
#endif

/*i \ingroup OSAPI_HeapClass
    \brief A static method to allocate space on the heap for a string of up to
    a given size. The string is initialized with 0's.

    Example:
    \code
    char* myString;
    OSAPI_Heap_allocate_string(&myString, 128);
    \endcode

    @param[out] pointer  String buffer.
    @param[in]  size     Size in bytes to allocate; this length must not include
    the string terminator, which is automatically added to the specified value.

    The returned space must be freed with \ref OSAPI_Heap_free_string.
 */
OSAPIDllExport void
OSAPI_Heap_allocate_string(char **pointer,RTI_UINT32 size);

#ifndef RTI_CERT
/*i \ingroup OSAPI_HeapClass
    \brief A static method to return space string space to the heap.

    @param[in] pointer If NULL, no op.
 */
OSAPIDllExport void
OSAPI_Heap_free_string(char *pointer);
#endif

/*e \dref_OSAPI_Heap_allocate_buffer
*/
OSPSLDllExport void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment);

#ifndef RTI_CERT
/*e \ingroup OSAPI_HeapClass
  \brief A static method to return a block (that was previously
  allocated with \idref_OSAPI_Heap_allocate_buffer) to the heap.

  @param[in] buffer If NULL, no op.
*/
OSPSLDllExport void
OSAPI_Heap_free_buffer(void *buffer);
#endif /* !RTI_CERT */

/*e
 * \ingroup OSAPI_HeapClass
 * \brief Check if a particular address is aligned to an alignment
 */
#define OSAPI_Heap_is_address_aligned(location, alignment) \
  (((RTI_UINT32)((char*)(location) - (char*)OSAPI_CC_NullPtr) % (alignment)) == 0)

/*e
 * \ingroup OSAPI_HeapClass
 * \brief Aligns a particular address to an alignment
 * alignment
 */
#define OSAPI_Heap_align_size_up(size, alignment) \
        (((size) + ((alignment) - 1U)) & (~((alignment) - 1U)))

#ifdef __cplusplus
} /* extern "C" */
#endif


#include "osapi/osapi_heap_impl.h"

#endif /* osapi_heap_h */
