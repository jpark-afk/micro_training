/*
 * FILE: PluginHelper.c - Exported PluginHelper functions
 *
 * (c) Copyright, Real-Time Innovations, 2010-2015
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
 * 25Mar2015,as MICRO-908: Removed unused PluginHelper functions
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 05jun2012,kaj Created
 * 30oct2012,kaj Zero serialized key stream to insure zero padding on hash
 */
/*ce
 * \file
 * \brief  Exported PluginHelper functions
 *
 * \details
 * This function implements helper functions to interface with generated
 * type plugins.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_md5_h
#include "cdr/cdr_md5.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

#include "PluginHelper.h"

/*** SOURCE_BEGIN ***/
/*ci
 * \brief Generic function to calculate the keyhash for a type based on a
 *        sample
 *
 * \details
 *
 * All keyed types have a keyhash, the definition is part of the RTPS
 * specification. This help function is called by generated type-plugins
 * to calculate the keyhash.
 *
 * \param[in]  plugin    The type-plugin for the type
 * \param[in]  md5stream Stream to serialize a sample to
 * \param[out] key_hash  The resulting keyhash
 * \param[in]  instance  A sample with the key-fields filled with correct value
 * \param[in]  param     Opaque parameter passed from the generated code
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
PluginHelper_instance_to_keyhash(struct NDDS_Type_Plugin *plugin,
                                 struct CDR_Stream_t *md5stream,
                                 DDS_KeyHash_t *key_hash,
                                 const void *instance,
                                 DDS_EncapsulationId_t id)
{
    RTI_UINT32 serialized_key_max_size;
    RTI_BOOL retval = RTI_FALSE;
#ifdef RTI_ENDIAN_LITTLE
    RTI_BOOL saved_byte_swap;
    RTI_BOOL is_byte_swapped = RTI_FALSE;
#endif
    /* This function is only used for V1 keyhashes */
    UNUSED_ARG(id);

    /* validate parameters */
    if ((plugin == NULL) || (md5stream == NULL) ||
        (key_hash == NULL) || (instance == NULL))
    {
        goto done;
    }

    /* verify key support exists for this plugin */
    if (DDS_TypePlugin_get_key_kind(plugin) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        goto done;
    }

    /* insure sufficient room in stream for serialized key */
    serialized_key_max_size = DDS_TypePlugin_get_serialized_key_size(plugin,0);
    if (serialized_key_max_size > md5stream->length)
    {
        goto done;
    }

    /* need to zero stream buffer because any padding bytes won't be set by
       serialization, but they will be included in the hash */
    OSAPI_Memory_zero(md5stream->buffer, serialized_key_max_size);

    /* keyhash is always calculated from Big Endian serialization,
       so if we are little endian we need to set byte swap */
#ifdef RTI_ENDIAN_LITTLE
    saved_byte_swap = md5stream->need_byte_swap;
    md5stream->need_byte_swap = RTI_TRUE;
    is_byte_swapped = RTI_TRUE;
#endif

    CDR_Stream_reset(md5stream);
    if (!DDS_TypePlugin_serialize_key(plugin,md5stream, instance,NULL))
    {
        goto done;
    }

    /* if the serialized key size is greater than the hash size,
       hash the serialized key, otherwise just use the serialized key */
    if (serialized_key_max_size >
            (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH))
    {
        NDDSCDR_Stream_compute_MD5(md5stream, key_hash->value);
    }
    else
    {
        /* sanity check to insure serialized key hasn't exceeded max size */
        if (CDR_Stream_get_current_position_offset(md5stream) >
            (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH))
        {
            goto done;
        }
        OSAPI_Memory_zero(key_hash->value,RTPS_KEY_HASH_MAX_LENGTH);
        OSAPI_Memory_copy(key_hash->value, md5stream->buffer,
                         CDR_Stream_get_current_position_offset(md5stream));
    }
    key_hash->length = RTPS_KEY_HASH_MAX_LENGTH;

    retval = RTI_TRUE;
done:
#ifdef RTI_ENDIAN_LITTLE
    if (is_byte_swapped)
    {
        md5stream->need_byte_swap = saved_byte_swap;
    }
#endif
    return retval;
}

DDS_Boolean
PluginHelper_serialize_sequence(
                      struct DDS_TypePlugin *plugin,
                      struct CDR_Stream_t *cdrs,
                      const struct REDA_Sequence* in,
                      DDS_InstanceHandle_t *destination,
                      DDS_TypePlugin_serialize_sample_T  serialize_function)
{
    RTI_INT32 i, length;
    RTI_UINT32 ulength;
    void *buffer;

    length = REDA_Sequence_get_length(in);
    if (length < 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    ulength = (RTI_UINT32)length;
    if (!CDR_Stream_serialize_unsigned_long(cdrs, &ulength))
    {
        return DDS_BOOLEAN_FALSE;
    }

    for (i = 0; i < length; ++i)
    {
        buffer = REDA_Sequence_get_reference(in, i);
        if (buffer == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (!serialize_function(plugin, cdrs, buffer,destination))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;

}

DDS_Boolean
PluginHelper_deserialize_sequence(
                      struct DDS_TypePlugin *plugin,
                      struct REDA_Sequence *out,
                      struct CDR_Stream_t *cdrs,
                      DDS_InstanceHandle_t *source,
                      DDS_TypePlugin_deserialize_sample_T  deserialize_function)
{
    RTI_UINT32 length;
    RTI_INT32 i,slength;
    void *buffer;

    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (length > INT_MAX)
    {
        return DDS_BOOLEAN_FALSE;
    }

    slength = (RTI_INT32)length;
    if (slength > REDA_Sequence_get_maximum(out))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!REDA_Sequence_set_length(out, slength))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (slength == 0)
    {
        return DDS_BOOLEAN_TRUE;
    }

    for (i = 0; i < slength; ++i)
    {
        buffer = REDA_Sequence_get_reference(out, i);
        if (buffer == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }
        if (!deserialize_function(plugin,buffer,cdrs,source))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
PluginHelper_serialize_array(struct DDS_TypePlugin *plugin,
                             struct CDR_Stream_t *cdrs,
                             const void* in,
                             RTI_UINT32 length,
                             RTI_UINT32 element_size,
                             DDS_InstanceHandle_t *destination,
                             DDS_TypePlugin_serialize_sample_T serialize_function)
{
    RTI_UINT32 i;
    const char *array_element;

    array_element = (const char *)in;
    for (i = 0; i < length; ++i)
    {
        if (!serialize_function(plugin,cdrs, array_element,destination))
        {
            return DDS_BOOLEAN_FALSE;
        }
        array_element += element_size;
    }
    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
PluginHelper_deserialize_array(struct DDS_TypePlugin *plugin,
                               void* out,
                               struct CDR_Stream_t *cdrs,
                               RTI_UINT32 length,
                               RTI_UINT32 element_size,
                               DDS_InstanceHandle_t *source,
                               DDS_TypePlugin_deserialize_sample_T
                               deserialize_function)
{
    RTI_UINT32 i;
    char *array_element;

    array_element = (char *)out;

    for (i = 0; i < length; ++i)
    {
        if (!deserialize_function(plugin,array_element,cdrs,source))
        {
            return DDS_BOOLEAN_FALSE;
        }
        array_element += element_size;
    }
    return DDS_BOOLEAN_TRUE;
}

RTI_UINT32
PluginHelper_get_max_size_serialized_array(struct DDS_TypePlugin *plugin,
                                           RTI_UINT32 current_alignment,
                                           RTI_UINT32 ulength,
                                           DDS_TypePlugin_get_serialized_sample_size_T get_serialized_size_func)
{
    RTI_UINT32 add_size = 0;
    RTI_INT32 i = 0,samples_in_loop, loop_count;
    RTI_UINT32 loop_size;
    RTI_INT32 alignment, align[8];
    RTI_UINT32 size_align[8];
    RTI_INT32 length;

    /* Check for invalid length */
    /* for max size calculations ulength of zero is considered invalid */
    if ((ulength > INT_MAX) || (ulength == 0))
    {
        return 0;
    }

    length = (RTI_INT32)ulength;
    for (i=0; i<8; i++)
    {
        align[i] = -1;
        size_align[i] = 0;
    }

    i=0;
    alignment = (current_alignment % 8);

    while (align[alignment] < 0 && i < length)
    {
        RTI_UINT32 element_size;
        align[alignment] = i;
        size_align[alignment] = add_size;
        element_size = get_serialized_size_func(plugin,current_alignment + add_size);

        /* Check if nested call detected overflow (returns 0) */
        if (element_size == 0)
        {
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - element_size))
        {
            return 0;
        }

        add_size += element_size;
        i++;
        alignment = (RTI_INT32)((current_alignment + add_size)% 8);
    }

    if (i< length)
    {
        samples_in_loop = i - align[alignment];
        /* add_size >= size_align[alignment] due to monotonic growth */
        loop_size = add_size - size_align[alignment];
        loop_count = (length - i)/samples_in_loop;

        /* Check for overflow in multiplication */
        if ((loop_count != 0) && (loop_size > (UINT_MAX / (RTI_UINT32)loop_count)))
        {
            /* Overflow detected - return 0 to indicate error */
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - (loop_size * (RTI_UINT32)loop_count)))
        {
            return 0;
        }

        add_size += (loop_size * (RTI_UINT32)loop_count);
        i += samples_in_loop*loop_count;
    }

    for(;i < length; ++i)
    {
        RTI_UINT32 element_size;
        element_size = get_serialized_size_func(plugin,current_alignment + add_size);

        /* Check if nested call detected overflow (returns 0) */
        if (element_size == 0)
        {
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - element_size))
        {
            return 0;
        }

        add_size += element_size;
    }

    return add_size;
}

RTI_UINT32
PluginHelper_get_max_size_serialized_sequence(struct DDS_TypePlugin *plugin,
                                              RTI_UINT32 current_alignment,
                                              RTI_UINT32 length,
                                              DDS_TypePlugin_get_serialized_sample_size_T get_serialized_size_func)
{
    RTI_UINT32 add_size;
    RTI_UINT32 array_size;

    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);

    array_size = PluginHelper_get_max_size_serialized_array(plugin,
                                        current_alignment + add_size,
                                        length,get_serialized_size_func);

    /* Check if array size calculation detected overflow (returns 0) */
    if ((array_size == 0) && (length != 0))
    {
        return 0;
    }

    /* Check for overflow in addition */
    if (add_size > (UINT_MAX - array_size))
    {
        return 0;
    }

    add_size += array_size;

    return add_size;
}


/*ci @} */

