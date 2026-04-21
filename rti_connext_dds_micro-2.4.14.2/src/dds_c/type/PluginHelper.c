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
                                 void *param)
{
    RTI_UINT32 serializied_key_max_size;
#ifdef RTI_ENDIAN_LITTLE
    RTI_BOOL stream_need_byte_swap;
#endif

    /* validate parameters */
    if ((plugin == NULL) || (md5stream == NULL) ||
        (key_hash == NULL) || (instance == NULL))
    {
        return RTI_FALSE;
    }

    /* verify key support exists for this plugin */
    if ((plugin->key_kind == NDDS_TYPEPLUGIN_NO_KEY) ||
        (plugin->serialize_key == NULL) ||
        (plugin->get_serialized_key_max_size == NULL)) 
    {
        return RTI_FALSE;
    }

    /* insure sufficient room in stream for serialized key */
    serializied_key_max_size =
        plugin->get_serialized_key_max_size(plugin, 0, param);
    if (serializied_key_max_size > md5stream->length)
    {
        return RTI_FALSE;
    }

    /* need to zero stream buffer because any padding bytes won't be set by
       serialization, but they will be included in the hash */
    OSAPI_Memory_zero(md5stream->buffer, serializied_key_max_size);

    /* keyhash is always calculated from Big Endian serialization,
       so if we are little endian we need to set byte swap */
#ifdef RTI_ENDIAN_LITTLE
    stream_need_byte_swap = md5stream->need_byte_swap;
    md5stream->need_byte_swap = RTI_TRUE;
#endif
    CDR_Stream_reset(md5stream);
    if (!plugin->serialize_key(md5stream, instance, param)) 
    {
#ifdef RTI_ENDIAN_LITTLE
        md5stream->need_byte_swap = stream_need_byte_swap;
#endif
        return RTI_FALSE;
    }

    /* if the serialized key size is greater than the hash size,
       hash the serialized key, otherwise just use the serialized key */
    if (serializied_key_max_size > (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH)) 
    {
        NDDSCDR_Stream_compute_MD5(md5stream, key_hash->value);
    } 
    else 
    {
        /* sanity check to insure serialized key hasn't exceeded max size */
        if (CDR_Stream_get_current_position_offset(md5stream) >
            (unsigned int)(RTPS_KEY_HASH_MAX_LENGTH)) 
        {
#ifdef RTI_ENDIAN_LITTLE
            md5stream->need_byte_swap = stream_need_byte_swap;
#endif
            return RTI_FALSE;
        }
        OSAPI_Memory_zero(key_hash->value,RTPS_KEY_HASH_MAX_LENGTH);
        OSAPI_Memory_copy(key_hash->value, md5stream->buffer,
                         CDR_Stream_get_current_position_offset(md5stream));
    }
    key_hash->length = RTPS_KEY_HASH_MAX_LENGTH;
#ifdef RTI_ENDIAN_LITTLE
        md5stream->need_byte_swap = stream_need_byte_swap;
#endif
    return RTI_TRUE;
}

/*ci
 * \brief Get the key-kind for a type-plugin
 *
 * \param[in] plugin The type-plugin to return the key for
 * \param[in] param  Opaque pointer passed from the generated code
 *
 * \return The key-kind. If a NULL pointer is passed in for the plugin,
 *         NDDS_TYPEPLUGIN_NO_KEY is returned
 */
NDDS_TypePluginKeyKind
PluginHelper_get_key_kind(struct NDDS_Type_Plugin *plugin,void *param)
{
    UNUSED_ARG(param);

    if (plugin == NULL)
    {
        return NDDS_TYPEPLUGIN_NO_KEY;
    }

    return plugin->key_kind;
}

/*ci @} */

