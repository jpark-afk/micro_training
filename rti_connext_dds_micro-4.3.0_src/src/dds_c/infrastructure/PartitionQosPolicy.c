/*
 * FILE: PartitionQosPolicy.c - PropertyQosPolicy Helper Functions
 *
 * (c) Copyright, Real-Time Innovations, 2024
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
 * 01Dec2023,ad Created.
 */

/*ci
 * \brief PartitionQosPolicy.c
 */

#include "PartitionQosPolicy.h"
#include "reda/reda_string.h"
#include "reda/reda_regex.h"
#include "osapi/osapi_log.h"

const char * DDS_PartitionQosPolicy_fv_empty_sequence_array[1] =
{
    ""
};

const struct DDS_StringSeq DDS_PartitionQosPolicy_fv_empty_sequence =
        REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(
                DDS_PartitionQosPolicy_fv_empty_sequence_array,
                1,
                1,
                char *);

/*ci
 * \brief Initialize a DDS_PartitionQosPolicy
 *
 * \param[in] self  The DDS_PartitionQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_PartitionQosPolicy_initialize(struct DDS_PartitionQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                return DDS_RETCODE_ERROR,
                OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_StringSeq_initialize(&self->name))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Test that two DDS_PartitionQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDS_Boolean
DDS_PartitionQosPolicy_is_equal(const struct DDS_PartitionQosPolicy *left,
                                const struct DDS_PartitionQosPolicy *right)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_StringSeq_is_equal(&left->name, &right->name))
    {
        goto done;
    }
        retval = DDS_BOOLEAN_TRUE;

done:
    return retval;
}

/*ci
 * \brief Check that a DDS_PartitionQosPolicy policy has consistent and
*   legal values with the participants resource limits
 *
 * \param[in] self         The Partition Qos Policy
 * \param[in] participant  The participant to use for the check
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_PartitionQosPolicy_is_consistent_w_limits(
        const struct DDS_PartitionQosPolicy *self,
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *dp_qos)
{
    RTI_INT32 partition_cumulative_characters = 0;
    RTI_INT32 length = 0;
    RTI_INT16 i = 0;

    OSAPI_PRECONDITION(self == NULL || dp_qos == NULL,
                return DDS_BOOLEAN_FALSE,
                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    partition_cumulative_characters =
                                    dp_qos->max_partition_cumulative_characters;

    if (DDS_StringSeq_get_length(&self->name) > dp_qos->max_partitions)
    {
        return DDS_BOOLEAN_FALSE;
    }

    for (i = 0; i < DDS_StringSeq_get_length(&self->name); i++)
    {
        if (*DDS_StringSeq_get_reference(&self->name,i) != NULL)
        {
            length = (RTI_INT32)(REDA_String_length(
                                *DDS_StringSeq_get_reference(&self->name,i)));
            if (length == INT_MAX)
            {
                return DDS_BOOLEAN_FALSE;
            }
            length++;
        }
        else
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (( length > dp_qos->max_partition_string_size) &&
            (dp_qos->max_partition_string_size != DDS_LENGTH_UNLIMITED))
        {
            return DDS_BOOLEAN_FALSE;
        }

        /* If we are using fixed string sizes then the length is always
         * max_partition_string_size
         */
        if (dp_qos->max_partition_string_size != DDS_LENGTH_UNLIMITED)
        {
            length = dp_qos->max_partition_string_size;
        }

        if (length > partition_cumulative_characters)
        {
            return DDS_BOOLEAN_FALSE;
        }

        partition_cumulative_characters -= length;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief DDS_PartitionQosPolicy_copy will allocate memory if the
 * left has no memory allocated. If the user does not want memory to be
 * allocated then left must have enough memory for all of the partitions.
 *
 * \param[in] left  The destination for the DDS_PartitionQosPolicy
 * \param[in] right The source of the DDS_PartitionQosPolicy
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PartitionQosPolicy_copy(struct DDS_PartitionQosPolicy *left,
                            const struct DDS_PartitionQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_StringSeq_copy(&left->name, &right->name))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Set the left partition qos policy string references
 *        with the right qos policy using the string manager assert
 *
 * \param[out] left           The destination for the DDS_PartitionQosPolicy
 * \param[in]  right          The source of the DDS_PartitionQosPolicy
 * \param[in]  string_manager The string manager for copying the partition seq
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PartitionQosPolicy_set_from(struct DDS_PartitionQosPolicy *left,
                                const struct DDS_PartitionQosPolicy *right,
                                DDS_StringManager_T *string_manager)
{
    RTI_INT16 i = 0;
    RTI_INT32 right_length = 0;

    OSAPI_PRECONDITION(left == NULL || right == NULL ||
    ( (string_manager == NULL) && (DDS_StringSeq_get_length(&right->name) > 0) ),
        return DDS_BOOLEAN_FALSE,
        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("right",right,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    right_length = DDS_StringSeq_get_length(&right->name);

    if (right_length == 0)
    {
        if (!DDS_StringSeq_set_length(&left->name,0))
        {
            return DDS_BOOLEAN_FALSE;
        }

        return DDS_BOOLEAN_TRUE;
    }

    if (!DDS_StringSeq_set_length(&left->name,right_length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    for (i = 0; i < right_length; i++)
    {
        if (*DDS_StringSeq_get_reference(&left->name,i) != NULL)
        {
            /* unexpected error */
            return DDS_BOOLEAN_FALSE;
        }
        *DDS_StringSeq_get_reference(&left->name,i) =
            DDS_StringManager_assert_string(string_manager,
            *DDS_StringSeq_get_reference(&right->name,i));

        if (*DDS_StringSeq_get_reference(&left->name,i) == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Checks if a string contains a regular expression.
 *
 * \param[in] name The string seq to check
 *
 * \return DDS_BOOLEAN_TRUE if the seq of strings only has strings with regex
 *         DDS_BOOLEAN_FALSE if the seq of strings has atleast one concrete
 *          string
 */
RTI_PRIVATE DDS_Boolean
DDS_PartitionQosPolicy_is_regex(char *name)
{
    const char regex_chars[] = REDA_REGEX_SUPPORTED_REGEX_CHARACTERS;
    char *slash_tmp = NULL;
    RTI_INT16 regex_length = 0;
    RTI_INT16 j = 0;
    RTI_INT16 k = 0;
    RTI_INT16 numSlashes = 0;

    regex_length = RTI_SIZEOF(regex_chars) - 2;

    for (j = 0; name[j] != '\0'; j++)
    {
        /* Check if the current character is a regex character */
        for (k = 0; k <= regex_length; k++)
        {
            if (name[j] == regex_chars[k])
            {
                /* Escaped characters turn the special character into a
                 * concrete character.
                 */
                slash_tmp = &name[j];
                numSlashes = 0;
                while ( (slash_tmp > name) && (slash_tmp[-1] == '\\') )
                {
                    ++numSlashes;
                    --slash_tmp;
                }
                if ( (numSlashes % 2) == 0)
                {
                    return DDS_BOOLEAN_TRUE;
                }
            }
        } /* regex check */
    } /* for loop */

    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Checks if a string sequence contains only regular expressions.
 *
 * \param[in] name The string seq to check
 *
 * \return DDS_BOOLEAN_TRUE if the seq of strings only has strings with regex.
 *         DDS_BOOLEAN_FALSE if the seq of strings has atleast one concrete
 *          string.
 */
RTI_PRIVATE DDS_Boolean
DDS_PartitionQosPolicy_seq_only_regex(const struct DDS_StringSeq *name)
{
    RTI_INT32 length;
    RTI_INT16 i = 0;

    length = DDS_StringSeq_get_length(name);
    if (length == 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    for (i = 0; i < length; i++)
    {
        if (!DDS_PartitionQosPolicy_is_regex(
                                        *DDS_StringSeq_get_reference(name, i)))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief checks if two partition strings are compatible
 *
 * \param[in] request The requested DDS_PartitionQosPolicy
 * \param[in] offered The offered DDS_PartitionQosPolicy
 *
 * \return DDS_BOOLEAN_TRUE if compatible, DDS_BOOLEAN_FALSE if not compatible
 */
DDS_Boolean
DDS_PartitionQosPolicy_is_compatible(
        const struct DDS_StringSeq *request,
        const struct DDS_StringSeq *offered)
{
    DDS_Boolean offered_use_empty_string = DDS_BOOLEAN_TRUE;
    DDS_Boolean request_use_empty_string = DDS_BOOLEAN_TRUE;
    RTI_INT32 off_len;
    RTI_INT32 req_len;
    RTI_INT32 off_index;
    RTI_INT32 req_index;
    const struct DDS_StringSeq *reqPtr = NULL;
    const struct DDS_StringSeq *offPtr = NULL;

    OSAPI_PRECONDITION(request == NULL || offered == NULL,
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("request",request,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("offered",offered,RTI_TRUE);)

    off_len = DDS_StringSeq_get_length(offered);
    req_len = DDS_StringSeq_get_length(request);

    /* If length is 0 then the partition is the empty string  "" */
    if ((off_len == 0) && (req_len == 0))
    {
        return DDS_BOOLEAN_TRUE;
    }

    reqPtr  = (req_len == 0) ? &DDS_PartitionQosPolicy_fv_empty_sequence : request;
    offPtr  = (off_len == 0) ? &DDS_PartitionQosPolicy_fv_empty_sequence : offered;
    req_len = (req_len == 0) ? 1 : req_len;
    off_len = (off_len == 0) ? 1 : off_len;

    /* If the partition only contains regex then the partition belongs
     * to the empty string.
     */
    offered_use_empty_string = DDS_PartitionQosPolicy_seq_only_regex(offPtr);
    request_use_empty_string = DDS_PartitionQosPolicy_seq_only_regex(reqPtr);

    /* Two empty strings always match */
    if (offered_use_empty_string && request_use_empty_string)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* Check the offered and requested string sequences against each other. */
    for (off_index = 0; (off_index < off_len); ++off_index)
    {
        char* off_str = *DDS_StringSeq_get_reference(offPtr,off_index);

        /* Check the empty sting if requested is regex only. */
        if (request_use_empty_string &&
            REDA_String_fnmatch(off_str,"",0) == REDA_REGEX_FNM_MATCH)
        {
            return DDS_BOOLEAN_TRUE;
        }

        for (req_index = 0; req_index < req_len; ++req_index)
        {
            char* req_str = *DDS_StringSeq_get_reference(reqPtr,req_index);

            RTI_BOOL req_is_regex = DDS_PartitionQosPolicy_is_regex(req_str);
            RTI_BOOL off_is_regex = DDS_PartitionQosPolicy_is_regex(off_str);

            /* Check the empty string if offered is regex only.
             * If both are concrete strings then compare them.
             * Otherwise compare the pattern against the concrete.
             */
            if ((offered_use_empty_string && (REDA_String_fnmatch(
                    req_str, "", 0) == REDA_REGEX_FNM_MATCH)) ||
                (!req_is_regex && !off_is_regex && (REDA_String_compare(
                    off_str, req_str) == 0)) ||
                (!req_is_regex && off_is_regex && (REDA_String_fnmatch(
                    off_str, req_str, 0) == REDA_REGEX_FNM_MATCH)) ||
                (req_is_regex && !off_is_regex && (REDA_String_fnmatch(
                    req_str, off_str, 0) == REDA_REGEX_FNM_MATCH)))
            {
                return DDS_BOOLEAN_TRUE;
            }
        }/* request loop*/
    }/* offered loop */

    return DDS_BOOLEAN_FALSE;
}


#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_PartitionQosPolicy
 *
 * \param[in] self  The DDS_PartitionQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_PartitionQosPolicy_finalize(struct DDS_PartitionQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
            return DDS_RETCODE_ERROR,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!REDA_StringSeq_finalize(&self->name))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Finalize a DDS_PartitionQosPolicy and do not free resources.
 *
 * \param[in] self            The DDS_PartitionQosPolicy
 * \param[in] string_manager  The manager to use for clearing strings
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_PartitionQosPolicy_finalize_no_dealloc(struct DDS_PartitionQosPolicy *self,
                                    DDS_StringManager_T *string_manager)
{
    OSAPI_PRECONDITION(self == NULL ||
        ((string_manager == NULL) && (DDS_StringSeq_get_length(&self->name) > 0)),
        return DDS_RETCODE_ERROR,
        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    /* When max_partitions is 0 then the string manager isn't created */
    if (string_manager == NULL)
    {
        return DDS_RETCODE_OK;
    }

    if (!DDS_PartitionQosPolicy_clear_strings(string_manager, self))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Finalize a DDS_PartitionQosPolicy and do not free resources.
 *
 * \param[in] string_manager  The manager to use to clear strings
 * \param[in] partition       The Partition QoS Policy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
RTI_BOOL
DDS_PartitionQosPolicy_clear_strings(DDS_StringManager_T *string_manager,
                                    struct DDS_PartitionQosPolicy *partition)
{
    RTI_INT16 i = 0;
    RTI_BOOL ok = DDS_BOOLEAN_TRUE;

    OSAPI_PRECONDITION(partition == NULL || string_manager == NULL,
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("partition",partition,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("string_manager",string_manager,RTI_TRUE);)

    if (DDS_StringSeq_get_length(&partition->name) == 0)
    {
        return DDS_BOOLEAN_TRUE;
    }

    for (i = 0; i < DDS_StringSeq_get_length(&partition->name); i++)
    {
        if (*DDS_StringSeq_get_reference(&partition->name,i) != NULL)
        {
            /* keep attempting to delete strings even when one fails */
            ok = ok && DDS_StringManager_delete_string(string_manager,
                *DDS_StringSeq_get_reference(&partition->name,i));

            *DDS_StringSeq_get_reference(&partition->name,i) = NULL;
        }
    }

    if (ok)
    {
        DDS_StringSeq_set_length(&partition->name,0);
    }

    return ok;
}

/*ci
 * \brief Set the max and allocate memory for a partition qos based on the
 *  domain participants limits.
 *
 * \param[in] partition The Partition QoS Policy
 * \param[in] dp_qos    The DomainParticipant qos
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PartitionQosPolicy_set_maximum_w_max(struct DDS_PartitionQosPolicy *partition,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    OSAPI_PRECONDITION((partition == NULL) || (dp_qos == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("partition",partition,RTI_TRUE);)

    if (dp_qos->resource_limits.max_partition_string_size == DDS_LENGTH_UNLIMITED)
    {
        /* if max_partition_string_size is unlimited then we don't know what size
         * the string may be. Go ahead and allocate the max amount of memory.
         */
        if (!DDS_StringSeq_set_maximum_w_max(&partition->name,
                DDS_PARTITIONQOSPOLICY_MAX_PARTITIONS,
                DDS_PARTITIONQOSPOLICY_MAX_PARTITION_CHARACTERS))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    else if (dp_qos->resource_limits.max_partition_string_size > 0)
    {
        if (!DDS_StringSeq_set_maximum_w_max(&partition->name,
                dp_qos->resource_limits.max_partitions,
                (RTI_UINT32)dp_qos->resource_limits.max_partition_string_size))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    else
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
