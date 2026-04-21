/*
 * FILE: osapi_hash.h - Generic hash interface
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
 * 16mar2017,as  Created
 */

/*ci
 * \file
 * \brief Generic hash interface definition.
 */
#ifndef osapi_hash_h
#define osapi_hash_h


#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

/*ci
 * \brief Generic buffer to hold data, described by a pointer to the data, as
 *        well as the length of the data.
 */
struct OSAPI_Buffer
{
    /*ci
     * \brief Length of the buffer pointed to by pointer.
     */
    RTI_UINT32 length;

    /*ci
     * \brief Pre-allocated (by the caller) buffer.
     */
    char *pointer;
};

/*i \ingroup OSAPI_HashClass
 * \brief Compute the MD5 hash of a buffer.
 *
 * \param buffer[in] Buffer to hash.
 * \param buffer_size[in] Size of the buffer.
 * \param out[out] Buffer to store the hash.
 */
OSAPIDllExport void
OSAPI_Hash_compute_buffer_md5(
        const char*buffer, RTI_SIZE_T buffer_size, RTI_UINT8 * out);

/*i \ingroup OSAPI_HashClass
 * \brief Compute the MD5 hash of an array of buffers.
 *
 * \param bufs[in] Array of buffers to hash.
 * \param n_bufs[in] Number of buffers in the array.
 * \param out[out] Buffer to store the hash.
 */
OSAPIDllExport void
OSAPI_Hash_compute_buffer_scatter_md5(
    const struct OSAPI_Buffer *bufs,
    RTI_UINT32 n_bufs,
    RTI_UINT8 *out);

#define OSAPI_MD5_DIGEST_SIZE (16)

#define OSAPI_HASH_MAX_LENGTH   (16)
#define OSAPI_HASH_VALUE_NIL    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

typedef struct OSAPI_Hash
{
    RTI_UINT8 value[OSAPI_HASH_MAX_LENGTH];
    RTI_UINT8 length;
    RTI_UINT8 flags;
} OSAPI_Hash;


#define OSAPI_HASH_NIL \
{                           \
    OSAPI_HASH_VALUE_NIL, \
    0, \
    0 \
}

#define OSAPI_HASH_FLAG_VALID   ((RTI_UINT8)(0x01 << 0))

/*i \ingroup OSAPI_HashClass
  @brief initialize a OSAPI_Hash

  @param self \b In. The hash to initialize
*/
OSAPIDllExport
void
OSAPI_Hash_initialize(OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Checks if two OSAPI_Hash are equal

  @param a \b In. The first hash to compare
  @param b \b In. The second hash to compare
  @return RTI_TRUE if they are equal. RTI_FALSE otherwise
*/
OSAPIDllExport RTI_BOOL
OSAPI_Hash_equals(const OSAPI_Hash *a, const OSAPI_Hash *b);

/*i \ingroup OSAPI_HashClass
  @brief Compare two hashes

  @param a \b In. The first OSAPI_Hash to compare
  @param b \b In. The first OSAPI_Hash to compare
  @return Return 0 if they are equal.
*/
OSAPIDllExport int
OSAPI_Hash_compare(const OSAPI_Hash *a, const OSAPI_Hash *b);

/*i \ingroup OSAPI_HashClass
  @brief Copy a hash

  @param out \b Out. The OSAPI_Hash where to copy
  @param in \b In. The source OSAPI_Hash
*/
OSAPIDllExport void
OSAPI_Hash_copy(OSAPI_Hash *out, const OSAPI_Hash *in);

/*i \ingroup OSAPI_HashClass
  @brief Check if a OSAPI_Hash is valid

  @param self \b In. The OSAPI_Hash to check
*/
OSAPIDllExport RTI_BOOL
OSAPI_Hash_is_valid(const OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Returns the length in bytes of the OSAPI_Hash

  @param self \b In. The OSAPI_Hash to get the length
  @return the length in bytes of the hash
*/
OSAPIDllExport RTI_UINT8
OSAPI_Hash_length(const OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Returns a array containing the OSAPI_Hash value

  @param self \b In. The OSAPI_Hash to get the length
  @return A pointer to the octet buffer containing the hash
*/
OSAPIDllExport const RTI_UINT8*
OSAPI_Hash_value(const OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Marks a hash as invalid

  @param self \b In. The OSAPI_Hash to reset
*/
OSAPIDllExport void
OSAPI_Hash_reset(OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Print a RTISampleHash

  @param self \b In. The OSAPI_Hash to print
*/
OSAPIDllExport void
OSAPI_Hash_print(const OSAPI_Hash *self);

/*i \ingroup OSAPI_HashClass
  @brief Computes the MD5 of a buffer

  @param self \b Out. The output OSAPI_Hash
  @param in \b In. The buffer to calculate the hash value
  @param size \b In. The length of the buffer
*/
OSAPIDllExport void
OSAPI_Hash_compute_md5(
       OSAPI_Hash *self,
       const char *in,
       unsigned int size);

/*i \ingroup OSAPI_HashClass
  @brief Computes the MD5 of a set of buffers and store the result in
  the output OSAPI_Hash

  @param self \b Out. The output OSAPI_Hash
  @param bufs \b In. Array of buffers to calculate the hash value of
  @param n_bufs \b In. The number of buffers in the array
*/
OSAPIDllExport void
OSAPI_Hash_compute_scatter_md5(
       OSAPI_Hash *self,
       const struct OSAPI_Buffer *bufs,
       RTI_UINT32 n_bufs);

#endif /* osapi_hash_h */
