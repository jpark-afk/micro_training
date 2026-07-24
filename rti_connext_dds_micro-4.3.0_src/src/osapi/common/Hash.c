/*
 * FILE: Hash.c - Generic hash interface
 *
 * (c) Copyright, Real-Time Innovations, 2018-2018
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
 * 16mar2018,as  Created
 *
 */

/*ci
 * \file
 * \brief Generic hash interface implementation
 * \defgroup OSAPIHashClass OSAPI Hash functions
 * \ingroup OSAPIModule
 *
 *  \details
 *  This provides platform independent functions to compute and manipulate
 *  the hash of a data stream.
 *  .
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_hash.h"
#include "osapi/osapi_string.h"

/*** SOURCE_BEGIN ***/

void
OSAPI_Hash_initialize(OSAPI_Hash *self)
{
    self->flags = 0;
    self->length = 0;
    OSAPI_Memory_zero(self->value, OSAPI_HASH_MAX_LENGTH);
}

/*e \ingroup OSAPIHashClass
  @brief Checks if two OSAPI_Hash are equal

  @param a \b In. The first hash to compare
  @param b \b In. The second hash to compare
  @return RTI_TRUE if they are equal. RTI_FALSE otherwise
*/
RTI_BOOL
OSAPI_Hash_equals(const OSAPI_Hash *a, const OSAPI_Hash *b)
{
    if (OSAPI_Hash_is_valid(a) != OSAPI_Hash_is_valid(b))
    {
        return RTI_FALSE;
    }
    else if (OSAPI_Hash_length(a) != OSAPI_Hash_length(b))
    {
        return RTI_FALSE;
    }
    else
    {
        return (OSAPI_Memory_compare(
                        OSAPI_Hash_value(a),
                        OSAPI_Hash_value(b),
                        OSAPI_Hash_length(a)) == 0);
    }
}

/*e \ingroup OSAPIHashClass
  @brief Compare two hashes

  @param a \b In. The first OSAPI_Hash to compare
  @param a \b In. The first OSAPI_Hash to compare
  @return Return 0 if they are equal.
*/
int
OSAPI_Hash_compare(const OSAPI_Hash *a, const OSAPI_Hash *b)
{
    if (OSAPI_Hash_is_valid(a) != OSAPI_Hash_is_valid(b))
    {
        if (OSAPI_Hash_is_valid(a))
        {
            return 1;
        }
        else
        {
            return -1;
        }
    } else if ((OSAPI_Hash_length(a) - OSAPI_Hash_length(b)) != 0)
    {
        return (OSAPI_Hash_length(a) - OSAPI_Hash_length(b));
    }
    else
    {
        return (OSAPI_Memory_compare(
                        OSAPI_Hash_value(a),
                        OSAPI_Hash_value(b),
                        OSAPI_Hash_length(a)));
    }
}

/*e \ingroup OSAPIHashClass
  @brief Copy a hash

  @param out \b Out. The OSAPI_Hash where to copy
  @param in \b In. The source OSAPI_Hash
*/
void
OSAPI_Hash_copy(OSAPI_Hash *out, const OSAPI_Hash *in)
{
    out->length = in->length;
    OSAPI_Memory_copy((void *)out->value,(void *)in->value,out->length);
    out->flags = in->flags;
}

/*e \ingroup OSAPIHashClass
  @brief Check if a OSAPI_Hash is valid

  @param self \b In. The OSAPI_Hash to check
*/
RTI_BOOL
OSAPI_Hash_is_valid(const OSAPI_Hash *self)
{
    return (self->flags & OSAPI_HASH_FLAG_VALID) ?  RTI_TRUE : RTI_FALSE;
}

/*e \ingroup OSAPIHashClass
  @brief Returns the length in bytes of the OSAPI_Hash

  @param self \b In. The OSAPI_Hash to get the length
  @return the length in bytes of the hash
*/
RTI_UINT8
OSAPI_Hash_length(const OSAPI_Hash *self)
{
    return self->length;
}

/*e \ingroup OSAPIHashClass
  @brief Returns a array containing the OSAPI_Hash value

  @param self \b In. The OSAPI_Hash to get the length
  @return A pointer to the octet buffer containing the hash
*/
const RTI_UINT8 *
OSAPI_Hash_value(const OSAPI_Hash *self)
{
    return self->value;
}

/*e \ingroup OSAPIHashClass
  @brief Marks a hash as invalid

  @param self \b In. The OSAPI_Hash to reset
*/
void
OSAPI_Hash_reset(OSAPI_Hash *self)
{
    (self)->flags &= (RTI_UINT8)~OSAPI_HASH_FLAG_VALID;
}

/*e \ingroup OSAPIHashClass
  @brief Print a RTISampleHash

  @param self \b In. The OSAPI_Hash to print
*/
void
OSAPI_Hash_print(const OSAPI_Hash *self)
{
    UNUSED_ARG(self);
}

/*e \ingroup OSAPIHashClass
  @brief Computes the MD5 of a buffer

  @param self \b Out. The output OSAPI_Hash
  @param in \b In. The buffer to calculate the hash value
  @param size \b In. The length of the buffer
*/
void
OSAPI_Hash_compute_md5(
       OSAPI_Hash *self,
       const char *in,
       unsigned int size)
{

    OSAPI_Hash_compute_buffer_md5(in, size, self->value);
    self->flags |= OSAPI_HASH_FLAG_VALID;
    self->length = OSAPI_MD5_DIGEST_SIZE;
}

/*e \ingroup OSAPIHashClass
  @brief Computes the MD5 of a set of buffers and store the result in
  the output OSAPI_Hash

  @param self \b Out. The output OSAPI_Hash
  @param in \b In. Array of buffers that will be used to calculate the hash
  value
  @param size \b In. The number of elements in the array of buffers
  @param element_size \b In. An array containing the size of the buffers in the
  in parameter
*/
void
OSAPI_Hash_compute_scatter_md5(
       OSAPI_Hash *self,
       const struct OSAPI_Buffer *bufs,
       RTI_UINT32 n_bufs)
{
    OSAPI_Hash_compute_buffer_scatter_md5(bufs, n_bufs, self->value);
    self->flags |= OSAPI_HASH_FLAG_VALID;
    self->length = OSAPI_MD5_DIGEST_SIZE;
}
