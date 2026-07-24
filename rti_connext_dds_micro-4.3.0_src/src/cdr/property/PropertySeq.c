/*
 * FILE: PropertySeq.c - PropertySeq Helper Functions
 *
 * (c) Copyright, Real-Time Innovations, 2017-2024
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ci
 * \brief PropertySeq.c
 *
 * \details
 * This file implements helper functions for CDR Property and
 * CDR BinaryProperty Sequences
 */

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif

/*** SOURCE_BEGIN ***/

#define T struct CDR_Property
#define TSeq CDR_PropertySeq
#define REDA_SEQUENCE_USER_API
#define T_initialize CDR_Property_initialize
#define T_finalize   CDR_Property_finalize
#define T_copy       CDR_Property_copy
#define T_compare    CDR_Property_compare
#define TSeq_is_equal
#include "reda/reda_sequence_defn.h"
#undef T_initialize
#undef T_finalize
#undef T_copy
#undef T_compare

struct CDR_Property*
CDR_PropertySeq_lookup_property_w_prefix(
        const struct CDR_PropertySeq *self,
        const char *const user_prefix,
        const char *const name)
{
    RTI_INT32 i,length;
    RTI_SIZE_T user_prefix_len;
    struct CDR_Property *property = NULL;
    struct CDR_Property *retval = NULL;

    OSAPI_PRECONDITION_ALWAYS(((self == NULL)||(name == NULL) ||
            (user_prefix == NULL)),
            return NULL,
            OSAPI_Log_entry_add_pointer("props", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_prefix", user_prefix, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name", name, RTI_TRUE);)

    length = CDR_PropertySeq_get_length(self);
    user_prefix_len = REDA_String_length(user_prefix);

    if (REDA_String_length(name) + user_prefix_len > CDR_PROPERTY_NAME_MAX_LEN )
    {
        goto done;
    }

    for (i = 0; ((i < length) && (retval == NULL)); i++)
    {
        property = CDR_PropertySeq_get_reference(self, i);
        if (property == NULL)
        {
            goto done;
        }
        if (property->name == NULL)
        {
            goto done;
        }

        if (REDA_String_ncompare(
                property->name, user_prefix, user_prefix_len) != 0)
        {
            continue;
        }

        if(REDA_String_ncompare(
                property->name + user_prefix_len , ".", 1U) != 0)
        {
            continue;
        }

        if (!REDA_String_ncompare(property->name + user_prefix_len + 1 ,
                                name,  REDA_String_length(name)))
        {
            retval = property;
        }
    }

done:
    return retval;
}


struct CDR_Property*
CDR_PropertySeq_lookup_property(
        const struct CDR_PropertySeq *self, const char * const name)
{
    RTI_INT32 i,length;
    struct CDR_Property *property = NULL;
    struct CDR_Property *retval = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL)||(name == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("props", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name", name, RTI_TRUE);)

    length = CDR_PropertySeq_get_length(self);

    for (i = 0; i < length && retval == NULL; i++)
    {
        property = CDR_PropertySeq_get_reference(self, i);
        if (property == NULL)
        {
            goto done;
        }
        if (property->name == NULL)
        {
            goto done;
        }

        if (!REDA_String_ncompare(
                property->name, name, CDR_PROPERTY_NAME_MAX_LEN))
        {
            retval = property;
        }
    }
    done:
    return retval;
}


RTI_BOOL
CDR_PropertySeq_assert_property(struct CDR_PropertySeq *self,
                                const char * const name,
                                const char * const value,
                                CDR_Boolean propagate)
{

    struct CDR_Property *property = NULL;
    CDR_Long length = 0;
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 max_local = 0;

    OSAPI_PRECONDITION_ALWAYS(
            (self == NULL)||(name == NULL) || (value == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("value", value, RTI_TRUE);)

    length = CDR_PropertySeq_get_length(self);
    /* Search for the property */
    property = CDR_PropertySeq_lookup_property(self, name);
    if (property == NULL)
    {
        max_local = CDR_PropertySeq_get_maximum(self);
        /*
         * Check to see if CDR_PropertySeq instance maximum limit
         * has been reached - only then increment the max limit.
         */
        if (max_local < length+1)
        {
            if (!CDR_PropertySeq_set_maximum(self, max_local+1))
            {
                goto done;
            }
        }

        if (!CDR_PropertySeq_set_length(self, length+1))
        {
            goto done;
        }
        property = CDR_PropertySeq_get_reference(self, length);
        if (property == NULL)
        {
            goto done;
        }
        if (property->name != NULL)
        {
            /* this can happen if maximum > length */
            if (REDA_String_replace(&property->name, name) == NULL)
            {
                /* Property Name shouldn't be NULL */
                goto done;
            }
        }
        else
        {
            property->name = REDA_String_dup(name);
        }

        if (property->name == NULL)
        {
            goto done;
        }
    }

    if (property->value != NULL)
    {
        if (REDA_String_replace(&property->value, value) == NULL)
        {
            /* Property value NULL is not consistent behavior */
            goto done;
        }
    }
    else
    {
        property->value = REDA_String_dup(value);
    }

    if (property->value == NULL)
    {
        goto done;
    }
    property->propagate = propagate;
    retval = RTI_TRUE;

    done:
    if (!retval)
    {
        CDR_PropertySeq_set_length(self,length);
    }
    return retval;
}

RTI_BOOL
CDR_PropertySeq_is_consistent(const struct CDR_PropertySeq *self)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 i = 0,
              length = 0;
    struct CDR_Property *prop_ref = NULL;

    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    length = CDR_PropertySeq_get_length(self);
    for (i = 0; i < length; i++)
    {
        prop_ref = CDR_PropertySeq_get_reference(self, i);
        if (!CDR_Property_is_consistent(prop_ref))
        {
            goto done;
        }
    }
    retval = RTI_TRUE;

    done:
    return retval;
}
