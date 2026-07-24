/*
 * FILE: REDACircularList.c - Circular list implementation
 *
 * Copyright 2012-2015 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 12mar2015,tk MICRO-1100/PR#14153 Stricter test on is_linked and unlink_node
 * 30oct2012,tk Written
 */
/*ce
 * \file
 * \brief CircularList implementation
 *
 * \details
 * The file implements circular list functions. Note that the APIs are internal
 * and does note perform error-checking.
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
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_circular_h
#include "reda/reda_circularlist.h"
#endif

#include "REDACircularList.h"

/*** SOURCE_BEGIN ***/

void
REDA_CircularList_init(REDA_CircularList_T *list)
{
    list->_next = list;
    list->_prev = list;
}

void
REDA_CircularListNode_init(struct REDA_CircularListNode *node)
{
  node->_next = NULL;
  node->_prev = NULL;
}

void
REDA_CircularList_link_node_after(struct REDA_CircularListNode *after,
                                  struct REDA_CircularListNode *node)
{
    node->_next = after->_next;
    after->_next = node;
    node->_next->_prev = node;
    node->_prev = after;
}

void
REDA_CircularList_unlink_node(struct REDA_CircularListNode *node)
{
    if ((node->_prev == NULL) && (node->_next == NULL))
    {
        /* This is a valid use-case. If the node is already unlinked,
         * just return. It is up to the caller to add extra logic to
         * check if a node is linked before unlinking
         */
        return;
    }

    OSAPI_PRECONDITION_ALWAYS((node->_prev == NULL) || (node->_next == NULL),
                    return,
                    OSAPI_Log_entry_add_pointer("prev",node->_prev,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("next",node->_next,RTI_TRUE);)

    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_next = NULL;
    node->_prev = NULL;
}

RTI_BOOL
REDA_CircularList_is_empty(REDA_CircularList_T *list)
{
    return (list->_next == list ? RTI_TRUE : RTI_FALSE);
}

RTI_BOOL
REDA_CircularListNode_is_linked(REDA_CircularListNode_T *node)
{
    if ((node->_next != NULL) && (node->_prev != NULL))
    {
        return RTI_TRUE;
    }

    OSAPI_PRECONDITION((node->_prev != NULL) || (node->_next != NULL),
                       return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("prev",node->_prev,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("next",node->_next,RTI_TRUE);)

    return RTI_FALSE;
}

