/*
 * FILE: md5.c - MD5 calculation API
 *
 * Copyright (c) 2008-2024 Real-Time Innovations, All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "cdr/cdr_stream.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_hash.h"

#ifndef cdr_md5_h
#include "cdr/cdr_md5.h"
#endif

/*ci 
 *\brief 
 * Compute the MD5 hash of a stream's buffer 
 * 
 *\details 
 * This function is used to compute the hash for keyed IDL data-types.
 * The use of the MD5 algorithm is mandated by document
 * "Real-time Publish-Subscribe Wire Protocol DDS Interoperability
 * Wire Protocol" (OMG, formal/2009-01-05).
 *
 * This operation computes the MD5 of a CDR_Stream_t and stores the result in
 * the specified output buffer.
 * Arguments are assumed to be valid; verification of their validity is left up
 * to the invoking context.
 * The input CDR_Stream_t must have an allocated buffer (me->buffer != NULL)
 * with some data to be hashed (as determined by a return value > 0 from
 * CDR_Stream_GetCurrentPositionOffset).
 * The specified output must point to a buffer of at least 16 bytes, in order
 * to be able to store the result of the hashing.
 *
 * NOTE: this function is used only by PluginHelper_instance_to_keyhash; this
 * is the default implementation of a FooTypePlugin's instance_to_keyhash;
 * instance_to_keyhash is invoked only by the DataWriter and DataReader
 * implementations, which use a dedicated internal CDR_Stream_t (md5_stream),
 * always reset before being passed to instance_to_keyhash, and a stack-
 * allocated DDS_KeyHash_t to store the output; PluginHelper_instance_to_keyhash
 * first writes the serialized key to the CDR_Stream_t and then possibly uses
 * NDDSCDR_Stream_computeMD5 to store its hash if the length exceeds
 * RTPS_KEY_HASH_MAX_LENGTH (16 bytes)
 * 
 * \param[in] me Stream
 * \param[out] out Computed MD5 hash
 *
 */
void
NDDSCDR_Stream_compute_MD5(struct CDR_Stream_t *me, RTI_UINT8 * out)
{

    OSAPI_Hash_compute_buffer_md5(
            me->buffer,
            CDR_Stream_get_current_position_offset(me),
            out);
}


