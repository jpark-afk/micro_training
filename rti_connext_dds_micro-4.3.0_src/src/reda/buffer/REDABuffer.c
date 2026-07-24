/*
 * FILE: REDABuffer.c - Buffer implementation
 *
 * Copyright 2012-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 25feb2015,tk Written, Refactored from reda_buffer.h
 */
/*ce
 * \file
 * \brief REDA Buffer implementation
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
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_buffer.h"
#endif

/*** SOURCE_BEGIN ***/

void
REDA_Buffer_set(struct REDA_Buffer *buffer,char *pointer,RTI_UINT32 length)
{
    buffer->length = length;
    buffer->pointer = pointer;
}

#ifndef RTI_CERT
void
REDA_Buffer_finalize(struct REDA_Buffer *buffer)
{
    if (buffer->pointer != NULL)
    {
        OSAPI_Heap_free(buffer->pointer);
        buffer->length = 0;
    }
}
#endif /*RTI_CERT*/
RTI_BOOL
REDA_Buffer_assert_buffer(struct REDA_Buffer *self, RTI_UINT32 new_length)
{
    RTI_BOOL result = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self, RTI_TRUE);)

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
        OSAPI_Heap_allocate_array(&self->pointer,new_length,char);
        if (self->pointer == NULL)
        {
            REDA_LOG_BUFFER_OUT_OF_MEMORY(OSAPI_LOGKIND_ERROR);
            goto done;
        }
        self->length = new_length;
    }
    else
    {
        /* If RTI_CERT is defined, it is an error to try to
           reallocate the buffer to a greater size. */
#if (defined(RTI_CERT) || OSAPI_DONT_HAVE_REALLOC)
        goto done;
#else
        self->pointer = (char*)OSAPI_Heap_realloc(self->pointer, new_length);
        if (self->pointer == NULL)
        {
            REDA_LOG_BUFFER_OUT_OF_MEMORY(OSAPI_LOGKIND_ERROR);
            goto done;
        }
        self->length = new_length;
#endif
    }

    result = RTI_TRUE;

done:

    return result;
}

