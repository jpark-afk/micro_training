/*
 * FILE: Property.c - CDR Property API
 *
 * (c) Copyright, Real-Time Innovations, 2017-2025.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ci @ingroup CDRModule
 * \file
 * \brief CDR Property API
 *
 * \details
 * Operations to manage a CDR Property and BinaryProperty
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef reda_sequence_h
#include "reda_sequence.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

/*** SOURCE_BEGIN ***/

RTI_BOOL
CDR_Property_initialize(struct CDR_Property* self)
{
    struct CDR_Property prop = CDR_Property_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

	*self = prop;

	return RTI_TRUE;
}

RTI_BOOL
CDR_Property_finalize(struct CDR_Property*  self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

#ifndef RTI_CERT
    /* When properties are removed from a sequence they are freed
     */
    if (self->name != NULL)
    {
        REDA_String_free(self->name);
    }

    if (self->value != NULL)
    {
        REDA_String_free(self->value);
    }
#endif

    return  RTI_TRUE;
}

RTI_BOOL
CDR_Property_copy(struct CDR_Property* dst,
                  const struct CDR_Property* src)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_UINT32 dst_name_len = 0,dst_value_len = 0;
    RTI_UINT32 src_name_len = 0,src_value_len = 0;

    OSAPI_PRECONDITION_ALWAYS(dst == NULL || src == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("dst",dst,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("src",src,RTI_TRUE);)

    dst_name_len = (dst->name != NULL)? REDA_String_length(dst->name) : 0U;
    dst_value_len = (dst->value != NULL)? REDA_String_length(dst->value) : 0U;
    src_name_len = (src->name != NULL)? REDA_String_length(src->name) : 0U;
    src_value_len = (src->value != NULL)? REDA_String_length(src->value) : 0U;

    if ((dst_name_len > 0) && (dst_name_len >= src_name_len))
    {
        if (!REDA_String_copy(dst->name,dst_name_len,src->name))
        {
            goto done;
        }
    }
    else if ((dst->name == NULL) && (src->name != NULL))
    {
        dst->name = REDA_String_dup(src->name);
        if (dst->name == NULL)
        {
            goto done;
        }
    }
    else if (src->name != NULL)
    {
        /* dst->name was set to a string of insufficient size */
        goto done;
    }

    if ((dst_value_len > 0) && (dst_value_len >= src_value_len))
    {
        if (!REDA_String_copy(dst->value,dst_value_len,src->value))
        {
            goto done;
        }
    }
    else if ((dst->value == NULL) && (src->value != NULL))
    {
        dst->value = REDA_String_dup(src->value);
        if (dst->value == NULL)
        {
            goto done;
        }
    }
    else if (src->value != NULL)
    {
        /* dst->value was set to a string of insufficient size */
        goto done;
    }

    dst->propagate = src->propagate;

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_BOOL
CDR_Property_is_consistent(const struct CDR_Property *self)
{
    RTI_BOOL retval = RTI_FALSE;
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->name == NULL || self->value == NULL)
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_INT32
CDR_Property_compare(const struct CDR_Property *prop1,
                     const struct CDR_Property *prop2)
{
    RTI_INT32 name_cmp = 0,value_cmp = 0;

    if (prop1 == NULL && prop2 == NULL)
    {
        return 0;
    }

    if (prop1 == NULL)
    {
        return -1;
    }

    if (prop2 == NULL)
    {
        return 1;
    }

    name_cmp = REDA_String_compare(prop1->name, prop2->name);

    if (name_cmp != 0)
    {
        return name_cmp;
    }

    value_cmp = REDA_String_compare(prop1->value, prop2->value);

    if (value_cmp != 0)
    {
        return value_cmp;
    }

    return prop1->propagate ?
                (prop2->propagate ? 0 : -1) :
                (prop2->propagate ? 1 : 0);
}


RTI_BOOL
CDR_Property_is_equal(const struct CDR_Property *prop1,
                     const struct CDR_Property *prop2)
{
    return (CDR_Property_compare(prop1,prop2) == 0);
}
