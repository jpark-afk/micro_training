
/*
 * FILE:Buffer.c - Buffer implementation
 *
 * Copyright 2019 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 25feb2015,am Written, Copied from RedaBuffer.c with appropriate changes for the length
 */
/*ce
 * \file
 * \brief DDS Buffer implementation
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef dds_c_log_h
#include "dds_c/dds_c_log.h"
#endif
#ifndef dds_c_buffer_h
#include "dds_c/dds_c_buffer.h"
#endif

/*** SOURCE_BEGIN ***/

void
DDS_Buffer_set(struct DDS_Buffer *buffer,char *pointer,DDS_Long length)
{
    OSAPI_PRECONDITION((buffer == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer, RTI_FALSE);
                       OSAPI_Log_entry_add_int("length", length, RTI_TRUE);)
    buffer->length = length;
    buffer->pointer = pointer;
}

#ifndef RTI_CERT
void
DDS_Buffer_finalize(struct DDS_Buffer *buffer)
{
    OSAPI_PRECONDITION((buffer == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer, RTI_TRUE);)
    if (buffer->pointer != NULL)
    {
        OSAPI_Heap_free(buffer->pointer);
        buffer->pointer = NULL;
        buffer->length = 0;
    }
}
#endif /*RTI_CERT*/
RTI_BOOL
DDS_Buffer_assert_buffer(struct DDS_Buffer *self, DDS_Long new_length)
{
    RTI_BOOL result = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL || new_length < 0),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self, RTI_FALSE);
                       OSAPI_Log_entry_add_int("new_length",new_length, RTI_TRUE);)

    /* Can only assert a buffer of length > 0 */
    if (new_length == 0)
    {
        return RTI_FALSE;
    }

    /* If current buffer is greater than requested length,
       do nothing and return success. */
    if (self->length >= new_length)
    {
        return RTI_TRUE;
    }

    /* First time allocation */
    if (self->pointer == NULL)
    {
        OSAPI_Heap_allocate_array(&self->pointer, (RTI_UINT32)new_length,char);
        if (self->pointer == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BUFFER_OBJECT)
            goto done;
        }
        self->length = new_length;
    }
    else
    {
        /* If RTI_CERT is defined, it is an error to try to
           reallocate the buffer to a greater size. */
#ifdef RTI_CERT
        goto done;
#else
        self->pointer = (char*)OSAPI_Heap_realloc(self->pointer, (RTI_UINT32)new_length);
        if (self->pointer == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BUFFER_OBJECT)
            goto done;
        }
        self->length = new_length;
#endif
    }

    result = RTI_TRUE;

done:

    return result;
}

