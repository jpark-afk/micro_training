/* $Id$

 (c) Copyright, Real-Time Innovations, 2002-2017.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
=========================================================================*/

#include "Infrastructure.h"
#include "InlineList.h"

void RTIXCdrInlineListNode_initialize(struct RTIXCdrInlineListNode *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);

    self->next = NULL;
    self->prev = NULL;
}

void RTIXCdrInlineList_removeNode(
        struct RTIXCdrInlineList *self,
        struct RTIXCdrInlineListNode *node)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(node == NULL, return);

    if (node->next != NULL || node->prev != NULL) {
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }

        if (node->next != NULL) {
            node->next->prev = node->prev;
        }

        if (self->last == node) {
            self->last = node->prev;
        }

        if (self->first == node) {
            self->first = node->next;
        }

        node->next = NULL;
        node->prev = NULL;
    } else {
        /* This is the last element */
        self->last = NULL;
        self->first = NULL;
    }
}

void RTIXCdrInlineList_addNodeToBack(
        struct RTIXCdrInlineList *self,
        struct RTIXCdrInlineListNode *node)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(node == NULL, return);

    if (self->last != NULL) {
        node->prev = self->last;
        self->last->next = node;
        self->last = node;
    } else {
        self->last = node;
        self->first = node;
        node->prev = NULL;
    }

    node->next = NULL;
}

void RTIXCdrInlineList_finalize(struct RTIXCdrInlineList *self) {
    RTIXCdrLog_testPrecondition(self == NULL, return);
    
    while (self->first != NULL) {
        RTIXCdrInlineList_removeNode(self, self->first);
    }
}

void RTIXCdrInlineList_initialize(struct RTIXCdrInlineList *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);

    self->first = NULL;
    self->last = NULL;
}


void RTIXCdrInlineList_delete(struct RTIXCdrInlineList *self) {
    RTIXCdrLog_testPrecondition(self == NULL, return);
    
    RTIXCdrInlineList_finalize(self);
    RTIXCdrHeap_freeStruct(self);
}

struct RTIXCdrInlineList * RTIXCdrInlineList_new(void)
{
    struct RTIXCdrInlineList *self = NULL;
    
    RTIXCdrHeap_allocateStruct(&self, struct RTIXCdrInlineList);

    if (self == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
                sizeof(struct RTIXCdrInlineList));
        return NULL;
    }    
    
    RTIXCdrInlineList_initialize(self);
    return self;
}

