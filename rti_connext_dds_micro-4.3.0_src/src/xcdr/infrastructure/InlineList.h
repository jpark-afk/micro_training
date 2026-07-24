/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"

#define RTIXCdrInlineListNode_INITIALIZER {NULL, NULL};

struct RTIXCdrInlineList {
    struct RTIXCdrInlineListNode *first;
    struct RTIXCdrInlineListNode *last;
};

#define RTIXCdrInlineList_INITIALIZER {NULL, NULL};

extern void RTIXCdrInlineListNode_initialize(struct RTIXCdrInlineListNode *self);

extern void RTIXCdrInlineList_removeNode(
        struct RTIXCdrInlineList *self,
        struct RTIXCdrInlineListNode *node);

extern void RTIXCdrInlineList_addNodeToBack(
        struct RTIXCdrInlineList *self,
        struct RTIXCdrInlineListNode *node);

extern void RTIXCdrInlineList_initialize(struct RTIXCdrInlineList *self);

extern void RTIXCdrInlineList_finalize(struct RTIXCdrInlineList *self);

extern void RTIXCdrInlineList_delete(struct RTIXCdrInlineList *self);

extern struct RTIXCdrInlineList * RTIXCdrInlineList_new(void);
