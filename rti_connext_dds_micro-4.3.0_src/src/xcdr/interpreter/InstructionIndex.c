/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "../infrastructure/Infrastructure.h"
#include "../typeCode/TypeCode.h"
#include "../interpreter/ProgramSupport.h"
#include "InstructionIndex.h"

void RTIXCdrInstructionIndex_delete(
        struct RTIXCdrInstructionIndex * self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);

    if (self->entries != NULL) {
        RTIXCdrHeap_freeArray(self->entries);
    }

    RTIXCdrHeap_freeStruct(self);
}

RTI_PRIVATE
int RTIXCdrInstructionIndexEntry_compareLval(
        const void *entry1,
        const void *entry2)
{
    RTIXCdrLong valLeft =
            ((const struct RTIXCdrInstructionIndexEntry *)entry1)->indexValue.lVal;
    RTIXCdrLong valRight =
            ((const struct RTIXCdrInstructionIndexEntry *)entry2)->indexValue.lVal;


    if (valLeft > valRight) {
        return 1;
    } else if (valLeft < valRight) {
        return -1;
    }

    return 0;}

RTI_PRIVATE
int RTIXCdrInstructionIndexEntry_compareUlval(
        const void *entry1,
        const void *entry2)
{
    RTIXCdrUnsignedLong valLeft =
            ((const struct RTIXCdrInstructionIndexEntry *)entry1)->indexValue.ulVal;
    RTIXCdrUnsignedLong valRight =
            ((const struct RTIXCdrInstructionIndexEntry *)entry2)->indexValue.ulVal;


    if (valLeft > valRight) {
        return 1;
    } else if (valLeft < valRight) {
        return -1;
    }

    return 0;
}

/**
 * @brief Reallocate the index's entries buffer to the new size.
 *
 * This function will allocate a new buffer of the requested size, then
 * copy as many entries from the old buffer as can fit in the new buffer.
 *
 * The index's count field will not be modified.
 *
 * @pre newSize must be greater than 0.
 *
 * @param[in,out] self The instruction index to resize.
 * @param[in] newSize The new size of the entries buffer.
 *
 * @return RTI_XCDR_TRUE if the resize was successful, RTI_XCDR_FALSE otherwise.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInstructionIndex_resize(
        struct RTIXCdrInstructionIndex *self,
        RTIXCdrUnsignedLong newSize)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong oldSize = 0;
    struct RTIXCdrInstructionIndexEntry *oldEntries = NULL;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);
    /* We don't use this function to "free" the index's buffer */
    RTIXCdrLog_testPrecondition(newSize == 0, return RTI_XCDR_FALSE);

    oldEntries = self->entries;
    oldSize = self->count;

    RTIXCdrHeap_allocateArray(
            &self->entries,
            newSize,
            struct RTIXCdrInstructionIndexEntry);
    if (self->entries == NULL) {
        RTIXCdrLog_logTwoLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                sizeof(struct RTIXCdrInstructionIndexEntry),
                (RTIXCdrLong) newSize);
        goto done;
    }
    if (oldEntries != NULL && oldSize > 0) {
        const RTIXCdrUnsignedLong newLength =
                (newSize < oldSize) ? newSize : oldSize;
        RTIXCdrMemory_copy(
                self->entries,
                oldEntries,
                newLength
                        * (RTIXCdrUnsignedLong) sizeof(
                                struct RTIXCdrInstructionIndexEntry));
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        self->entries = oldEntries;
        self->count = oldSize;
    } else {
        if (oldEntries != NULL) {
            RTIXCdrHeap_freeArray(oldEntries);
        }
    }
    return result;
}

struct RTIXCdrInstructionIndex * RTIXCdrInstructionIndex_new(
        RTIXCdrProgram *program,
        RTIXCdrInstructionIndexKind indexKind)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    struct RTIXCdrInstructionIndex *me = NULL;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrTypeCode *tc = NULL;
    RTIXCdrUnsignedLong memberIndex = 0;
    RTIXCdrUnsignedLong entryIndex = 0;
    RTIXCdrUnsignedLong instIndex = 0;
    RTIXCdrUnsignedLong maxEntryCount = 0;
    RTIBool onlyKey = RTI_FALSE;

    RTIXCdrLog_testPrecondition(program == NULL, return NULL);

    tc = program->typeCode;
    kind = RTIXCdrTypeCode_getKind(tc);
    onlyKey = program->onlyKey;

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT &&
            kind != RTI_XCDR_TK_VALUE &&
            kind != RTI_XCDR_TK_UNION,
            return NULL);

    RTIXCdrLog_testPrecondition(
            program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY
                    && indexKind == RTI_XCDR_MEMBER_ID_INDEX_KIND,
            return NULL);

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_UNION &&
            indexKind == RTI_XCDR_LABEL_INDEX_KIND,
            return NULL);

    RTIXCdrHeap_allocateStruct(&me, struct RTIXCdrInstructionIndex);

    if (me == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
                sizeof(struct RTIXCdrInstructionIndex));
        return NULL;
    }

    me->program = (RTIXCdrProgram *)program;
    me->count = 0;
    me->entries = NULL;
    me->defaultInsIndex = RTI_XCDR_TYPECODE_INVALID_INDEX;

    if (indexKind == RTI_XCDR_LABEL_INDEX_KIND) {
        me->count = RTIXCdrTypeCode_getLabelCount(tc, RTI_XCDR_FALSE);
        maxEntryCount = me->count;
    } else {
        /* Member ID index */
        maxEntryCount = tc->_memberCount;
        me->count = 0;
        if (kind == RTI_XCDR_TK_UNION) {
            me->count++; /* Discriminator */
            maxEntryCount++;
        }
    }

    if (maxEntryCount > 0) {
        /*
         * CORE-15623: For unions that only have a default member maxEntryCount
         * will be 0, and we don't need to allocate memory for the entries.
         *
         * It is also the case for empty structures
         */
        RTIXCdrHeap_allocateArray(
                &me->entries,
                maxEntryCount,
                struct RTIXCdrInstructionIndexEntry);
        if (me->entries == NULL) {
            RTIXCdrLog_logTwoLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                    sizeof(struct RTIXCdrInstructionIndexEntry),
                    (RTIXCdrLong) maxEntryCount);
            goto done;
        }
    }

    instIndex = RTIXCdrProgram_getFirstDataInstIndex(program);

    if (indexKind == RTI_XCDR_MEMBER_ID_INDEX_KIND) {
        RTIBool hasKey = RTI_FALSE;

        if (kind == RTI_XCDR_TK_VALUE) {
            /* Skip base if it exists */
            if (program->hasBase) {
                instIndex = RTIXCdrProgram_getNextDataInstIndex(
                        program,
                        instIndex);
            }
        } else if (kind == RTI_XCDR_TK_UNION) {
            /* Discriminator */
            me->entries[entryIndex].indexValue.ulVal = 0;
            me->entries[entryIndex].instructionIndex = instIndex;
            me->entries[entryIndex].programTypeCode = tc;
            entryIndex++;
            instIndex = RTIXCdrProgram_getNextDataInstIndex(
                    program,
                    instIndex);
        }

        hasKey = RTIXCdrTypeCode_hasKey(tc);

        for (memberIndex=0; memberIndex<tc->_memberCount; memberIndex++) {
            /* 
             * All the members of types that do not have key members (@key)
             * are considered part of the key
             */ 
            if (!onlyKey 
                    || !hasKey 
                    || (tc->_members[memberIndex]._memberFlags & RTI_XCDR_KEY_MEMBER)) {
                me->entries[entryIndex].indexValue.ulVal =
                        tc->_members[memberIndex]._representation._pid;
                me->entries[entryIndex].instructionIndex = instIndex;
                me->entries[entryIndex].programTypeCode = tc;
                entryIndex++;
                me->count++;
                instIndex = RTIXCdrProgram_getNextDataInstIndex(
                        program,
                        instIndex);
            }
        }

        if (me->entries != NULL) {
            qsort(me->entries,
                  me->count,
                  sizeof(struct RTIXCdrInstructionIndexEntry),
                  RTIXCdrInstructionIndexEntry_compareUlval);
        }
    } else {
        /* Skip discriminator */
        instIndex = RTIXCdrProgram_getNextDataInstIndex(
                program,
                instIndex);

        for (memberIndex=0; memberIndex<tc->_memberCount; memberIndex++) {
            if ((RTIXCdrLong) memberIndex != tc->_default_index) {
                RTIXCdrLog_testPrecondition(me->entries == NULL, goto done);
                if (tc->_members[memberIndex]._labelsCount == 1) {
                    me->entries[entryIndex].indexValue.lVal =
                            tc->_members[memberIndex]._label;
                    me->entries[entryIndex].instructionIndex = instIndex;
                    entryIndex++;
                } else {
                    RTIXCdrUnsignedLong labelIndex = 0;
                    for (labelIndex=0; labelIndex<tc->_members[memberIndex]._labelsCount; labelIndex++) {
                        me->entries[entryIndex].indexValue.lVal =
                                tc->_members[memberIndex]._labels[labelIndex];
                        me->entries[entryIndex].instructionIndex = instIndex;
                        entryIndex++;
                    }
                }
            } else {
                me->defaultInsIndex = instIndex;
            }
            instIndex = RTIXCdrProgram_getNextDataInstIndex(
                    program,
                    instIndex);
        }

        if (me->entries != NULL) {
            qsort(me->entries,
                  me->count,
                  sizeof(struct RTIXCdrInstructionIndexEntry),
                  RTIXCdrInstructionIndexEntry_compareLval);
        }
    }

    if (me->count == 0) {
        /*
         * This may be the case, for example, for onlyKey programs for a
         * derived type where the only key fields are in the base type.
         *
         * It is also the case for a union that only has a default member.
         */
        if (me->entries != NULL) {
            RTIXCdrHeap_freeArray(me->entries);
            me->entries = NULL;
        }
    } else if (me->count != maxEntryCount) {
        /* 
         * There are cases, such as onlyKey programs, where the number of
         * required entries is much less than maxEntryCount. In these cases
         * we want to be more efficient with memory usage, so we reallocate
         * the entry array to the correct size
         */
        if (!RTIXCdrInstructionIndex_resize(me, me->count)) {
            RTIXCdrLog_logTwoLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                    sizeof(struct RTIXCdrInstructionIndexEntry),
                    (RTIXCdrLong) me->count);
            goto done;
        }
    }

    ok = RTI_XCDR_TRUE;
done:

    if (!ok) {
        RTIXCdrInstructionIndex_delete(me);
        me = NULL;
    }

    return me;
}

RTIXCdrBoolean RTIXCdrInstructionIndex_addBaseTypeEntries(
        struct RTIXCdrInstructionIndex *self,
        const struct RTIXCdrInstructionIndex *baseIndex)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLongLong newCount = 0;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(baseIndex == NULL, return RTI_XCDR_FALSE);

    /* If the base index is empty, there's nothing to do */
    if (baseIndex->count == 0) {
        result = RTI_XCDR_TRUE;
        goto done;
    }

    newCount = (RTIXCdrUnsignedLongLong) self->count + baseIndex->count;
    if ((newCount * sizeof(struct RTIXCdrInstructionIndexEntry))
                > RTIXCdrUnsignedLong_MAX
        || !RTIXCdrInstructionIndex_resize(
                self,
                (RTIXCdrUnsignedLong) newCount)) {
        RTIXCdrLog_logTwoLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                sizeof(struct RTIXCdrInstructionIndexEntry),
                (RTIXCdrLong) newCount);
        goto done;
    }
    /* The index must now have a buffer allocated */
    RTIXCdrLog_testPrecondition(self->entries == NULL, goto done);

    /* Append new entries to the end of the buffer */
    RTIXCdrMemory_copy(
            self->entries + self->count,
            baseIndex->entries,
            (RTIXCdrUnsignedLong) (baseIndex->count * sizeof(struct RTIXCdrInstructionIndexEntry)));
    self->count = (RTIXCdrUnsignedLong) newCount;

    qsort(self->entries,
          self->count,
          sizeof(struct RTIXCdrInstructionIndexEntry),
          RTIXCdrInstructionIndexEntry_compareUlval);

    result = RTI_XCDR_TRUE;
done:
    return result;
}

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrInstructionIndex_getInstructionIndexByMemberId(
        struct RTIXCdrInstructionIndex *self,
        RTIXCdrUnsignedLong *instructionIndex,
        RTIXCdrBoolean *knownMember,
        const struct RTIXCdrTypeCode **sourceTypeCode,
        const struct RTIXCdrTypeCode *programTypeCode,
        RTIXCdrUnsignedLong memberId)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(instructionIndex == NULL,
            return);
    RTIXCdrLog_testPrecondition(programTypeCode == NULL, return);
    RTIXCdrLog_testPrecondition(knownMember == NULL, return);

    RTIXCdrInstructionIndex_getInstructionIndexByMemberIdMacro(
            self,
            instructionIndex,
            knownMember,
            sourceTypeCode programTypeCode,
            memberId);
}
#endif

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrInstructionIndex_getInstructionIndexByLabel(
        struct RTIXCdrInstructionIndex *self,
        RTIXCdrUnsignedLong *instructionIndex,
        RTIXCdrLong label)
{
    RTIXCdrLog_testPrecondition(self == NULL,
            return);
    RTIXCdrLog_testPrecondition(instructionIndex == NULL,
            return);

    RTIXCdrInstructionIndex_getInstructionIndexByLabel(self,
            instructionIndex,
            label);
}
#endif
