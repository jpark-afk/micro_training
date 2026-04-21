/*
 * FILE: BuiltinTopicKey.c - BuiltinTopicKey implemenation
 *
 * (c) Copyright 2008-2020 Real-Time Innovations,
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_BuiltinTopicKey_copy_prefix
 * 12dec2016,tk MICRO-1574 Added DDS_ObjectId_is_compatible()
 * 12mar2015,tk MICRO-1116/PR#14232 Return true only for standard RTPS
 *                                  built-ins (not vendor specific)
 *
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief BuiltinTopicKey implementation
 *
 * \details
 * This file implements function to manipulate the BuiltinTopicKey datatype.
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "BuiltinTopicKey.h"

/* exported in infrastructure.ifc */
const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_UNKNOWN =
    { {0, 0, 0, 0} };
const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_AUTO = { {0, 0, 0, 0} };
const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN = { {0, 0, 0} };
const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_PREFIX_AUTO = { {0, 0, 0,} };

/*** SOURCE_BEGIN ***/

DDS_Long
DDS_BuiltinTopicKey_compare(const DDS_BuiltinTopicKey_t *left,
                            const DDS_BuiltinTopicKey_t *right)
{
    DDS_Long i;

    for (i = 0; i < DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH; ++i)
    {
        if (left->value[i] > right->value[i])
        {
            return 1;
        }
        if (left->value[i] < right->value[i])
        {
            return -1;
        }
    }
    return 0;
}

/*ci
 * \brief Compare two DDS_BuiltinTopicKey_t structure for equality
 *
 * \param[in] a Left side of comparison
 * \param[in] b Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise
 *         DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_BuiltinTopicKey_equals(const DDS_BuiltinTopicKey_t *a,
                           const DDS_BuiltinTopicKey_t *b)
{
    OSAPI_PRECONDITION(a == NULL || b == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("a",a,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("b",b,RTI_TRUE);)

    return (RTPS_Guid_equals((struct RTPS_Guid*)a, (struct RTPS_Guid*)b) ? DDS_BOOLEAN_TRUE :  DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Compare two DDS_BuiltinTopicKey_t structure for prefix equality
 *
 * \details
 * The prefix of the DDS_BuiltinTopicKey_t consists of the first 12 bytes.
 * This function compares the first 12 bytes for equality.
 *
 * \param[in] a Left side of comparison
 * \param[in] b Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_BuiltinTopicKey_prefix_equals(const DDS_BuiltinTopicKey_t *a,
                                  const DDS_BuiltinTopicKey_t *b)
{
    OSAPI_PRECONDITION(a == NULL || b == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("a",a,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("b",b,RTI_TRUE);)

    return (RTPS_Guid_prefix_equals((struct RTPS_Guid *)a,(struct RTPS_Guid *)b) ? DDS_BOOLEAN_TRUE :  DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Compare two DDS_BuiltinTopicKey_t structure for suffic equality
 *
 * \details
 * The suffix of the DDS_BuiltinTopicKey_t consists of the last 4 bytes.
 * This function compares the last 4 bytes for equality.
 *
 * \param[in] a Left side of comparison
 * \param[in] b Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_BuiltinTopicKey_suffix_equals(const DDS_BuiltinTopicKey_t *a,
                                  const DDS_BuiltinTopicKey_t *b)
{
    OSAPI_PRECONDITION(a == NULL || b == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("a",a,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("b",b,RTI_TRUE);)

    return (RTPS_Guid_suffix_equals((struct RTPS_Guid *)a,(struct RTPS_Guid *)b) ? DDS_BOOLEAN_TRUE :  DDS_BOOLEAN_FALSE);
}

#ifndef RTI_CERT
/*ci
 * \brief Copy the prefix of a DDS_BuiltinTopicKey_t structure
 *
 * \details
 * The prefix of the DDS_BuiltinTopicKey_t consists of the first 12 bytes.
 * This function copies the first 12 bytes to the destination buffer.
 *
 * \param[in] a The destination DDS_BuiltinTopicKey_t structure
 * \param[in] b The source DDS_BuiltinTopicKey_t structure
 */
void
DDS_BuiltinTopicKey_copy_prefix(DDS_BuiltinTopicKey_t *a,
                                const DDS_BuiltinTopicKey_t *b)
{
    OSAPI_PRECONDITION(a == NULL || b == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("a",a,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("b",b,RTI_TRUE);)

    OSAPI_Memory_copy(a->value,b->value,
                      sizeof(DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN) -
                      sizeof(DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE));
}
#endif

/*ci
 * \brief Copy the suffix of a DDS_BuiltinTopicKey_t structure
 *
 * \details
 * The suffix of the DDS_BuiltinTopicKey_t consists of the last 4 bytes.
 * This function copies the last 4 bytes to the destination buffer.
 *
 * \param[in] a The destination DDS_BuiltinTopicKey_t structure
 * \param[in] b The source DDS_BuiltinTopicKey_t structure
 */
void
DDS_BuiltinTopicKey_copy_suffix(DDS_BuiltinTopicKey_t *a,
                                const DDS_BuiltinTopicKey_t *b)
{
    OSAPI_PRECONDITION(a == NULL || b == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("a",a,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("b",b,RTI_TRUE);)

    a->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = b->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID];
}


/*ci
 * \brief Convert a DDS_InstanceHandle_t to a DDS_BuiltinTopicKey_t
 *
 * \details
 * This function converts a DDS_InstanceHandle_t to a
 * DDS_BuiltinTopicKey_t and converts each field to host order to maintain
 * the correct byte ordering since the DDS_BuiltinTopicKey_t consists of
 * 4 integers.
 *
 * \param[in] out The destination DDS_BuiltinTopicKey_t structure
 * \param[in] in  The source DDS_InstanceHandle_t
 */
void
DDS_BuiltinTopicKey_from_guid(DDS_BuiltinTopicKey_t *out,
                              const DDS_InstanceHandle_t *in)
{
    OSAPI_Memory_copy(out->value,in->octet, 16);
#ifdef RTI_ENDIAN_LITTLE
    out->value[0] = NETIO_ntohl(out->value[0]);
    out->value[1] = NETIO_ntohl(out->value[1]);
    out->value[2] = NETIO_ntohl(out->value[2]);
    out->value[3] = NETIO_ntohl(out->value[3]);
#endif
}

/*ci
 * \brief Determine if an object id is a built-in topic or a user-defined topic
 *
 * \param[in] oid Object id to test
 *
 * \return DDS_BOOLEAN_TRUE if object id is a built-in topic,
 *         DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ObjectId_is_builtin(DDS_UnsignedLong oid)
{
    return ((oid & 0xc0) == 0xc0 ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Determine if a DDS_BuiltinTopicKey_t is a built-in topic or
 *        a user-defined topic
 *
 * \param[in] key DDS_BuiltinTopicKey_t to test
 *
 * \return DDS_BOOLEAN_TRUE if key is a built-in topic,
 *         DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_BuiltinTopicKey_is_builtin(const struct DDS_BuiltinTopicKey_t *const key)
{
    return ((key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] & 0xc0) == 0xc0 ?
                                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Determine if two DDS object-id's are compatible
 *
 * \details
 * Two DDS object-id's are compatible if they are both keyed
 * or both are unkeyed. They are not compatible if they one if keyed and the
 * other not. Note that the type of key, GUID or USER, is not relevant. Also,
 * it is not relevant if one is built-in and the other not.
 *
 * \param[in] dr_id The datareader object id
 * \param[in] dw_id The datawriter object id
 *
 * \return DDS_BOOLEAN_TRUE if objects are compatible,
 *         DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ObjectId_is_compatible(DDS_UnsignedLong dr_id,DDS_UnsignedLong dw_id)
{
    RTI_BOOL dr_is_keyed;
    RTI_BOOL dw_is_keyed;

    /* Only check the 4 LSB which indicates if the entity is keyed or not.
     * The 4 LSB are the same for keyed/non-keyed user/built-in end-points.
     */
    dr_is_keyed = ((dr_id & 0xf) == RTPS_OBJECT_NORMAL_USER_CST_READER) ? RTI_TRUE : RTI_FALSE;
    dw_is_keyed = ((dw_id & 0xf) == RTPS_OBJECT_NORMAL_USER_CST_WRITER) ? RTI_TRUE : RTI_FALSE;

    /* Could have used ^ here, but we those constructs for certification
     * reasons.
     */
    if (dr_is_keyed != dw_is_keyed)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci @} */

