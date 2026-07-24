/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_typeCode.h"

typedef union RTIXCdrInstructionIndexValue {
    RTIXCdrUnsignedLong ulVal;
    RTIXCdrLong lVal;
} RTIXCdrInstructionIndexValue;

struct RTIXCdrInstructionIndexEntry {
    /* For member ID indexes the indexValue represents the member ID
     * For label indexes the indexValue represents the label value
     */
    RTIXCdrInstructionIndexValue indexValue;
    /* 
     * The instruction index corresponds to the index of the data, as opposed 
     * to header, instruction corresponding to the member identified by 
     * indexValue. For example, a mutable member will have a header instruction 
     * (index 0) followed by an instruction for how to serialize that member 
     * (index 1). The instructionIndex in this case is 1, not 0.
     *
     * Note: The instruction index does not include the headers (member header,
     * collection dheader) instructions and it is up to the execution engine to
     * process the headers instructions
     */
    RTIXCdrUnsignedLong instructionIndex;
    /* For member ID indexes, we keep track of the TypeCode that the member
     * belongs to. This is used to determine if any member ID should be known
     * to the program's TypeCode, because defined by another type in the
     * hierarchy, or if its actually unknown, but also to prevent the program
     * from trying to deserialize a member that doesn't belong to it.
     *
     * For label indexes, this filed is not used and should be set to NULL.
     */
    const struct RTIXCdrTypeCode *programTypeCode;
};

typedef enum RTIXCdrInstructionIndexKind {
    RTI_XCDR_MEMBER_ID_INDEX_KIND,
    RTI_XCDR_LABEL_INDEX_KIND
} RTIXCdrInstructionIndexKind;

/* This structure is used to create instruction indexes.
 * Instruction indexes are created for mutable struct and valuetypes as well
 * as unions to find the right instruction to execute upon deserialization.
 */
struct RTIXCdrInstructionIndex
{
    RTIXCdrProgram *program;
    RTIXCdrUnsignedLong count;
    struct RTIXCdrInstructionIndexEntry *entries;
    /* For unions this is the index of the instruction corresponding
     * to the default member if it exists */
    RTIXCdrUnsignedLong defaultInsIndex;
};

extern void RTIXCdrInstructionIndex_delete(
        struct RTIXCdrInstructionIndex * self);

extern struct RTIXCdrInstructionIndex * RTIXCdrInstructionIndex_new(
        RTIXCdrProgram *program,
        RTIXCdrInstructionIndexKind indexKind);

/**
 * @brief Add entries from a base instruction index into this index.
 *
 * This function is used to merge the instruction index of a (base)
 * type into another type's index. We use this to correctly handle
 * derived mutable types, which have either a key or must_understand
 * member by declaring it themselves, or by inheriting from a base
 * type.
 *
 * Instruction will be copied "as is" from the base index, specifically
 * preserving the programTypeCode field in each entry. This allows
 * the lookup function to correctly identify members that are known
 * to the program's TypeCode, but belong to another type.
 *
 * @param[in,out] self The instruction index to which to add the base entries.
 * @param[in] baseIndex The base instruction index from which to copy entries.
 *
 * @return RTI_XCDR_TRUE if the operation was successful, RTI_XCDR_FALSE
 * otherwise.
 */
extern RTIXCdrBoolean RTIXCdrInstructionIndex_addBaseTypeEntries(
        struct RTIXCdrInstructionIndex *self,
        const struct RTIXCdrInstructionIndex *baseIndex);

/**
 * @brief Query the index for the instruction corresponding to the
 * specified member ID.
 *
 * Since the index may contain entries for members that do not belong
 * to the program's TypeCode (for example, members defined in a base
 * type), this function requires the program's TypeCode to correctly
 * identify whether the member ID is:
 *
 * - known and should be handled by the invoking program.
 * - known, but not handled by this program.
 * - unknown.
 *
 * The nuance in the lookup is required to correctly handle any member
 * that may appear in a serialized buffer with the "must understand"
 * bit set in its parameter ID (i.e. @key and @must_understand members).
 *
 * @param[in] self The instruction index to query.
 * @param[out] instructionIndex The index of the instruction corresponding
 *        to the specified member ID. If the member ID is unknown, this
 *       will be set to RTI_XCDR_TYPECODE_INVALID_INDEX.
 * @param[out] knownMember This will be set to RTI_XCDR_TRUE if the
 *        member ID belongs to programTypeCode or to any other type in
 *        the type hierarchy where programTypeCode appears, RTI_XCDR_FALSE
 *        otherwise.
 * @param[in] programTypeCode The TypeCode of the program for which
 *        we are querying the instruction index.
 * @param[in] memberId The member ID for which to query the instruction index.
 */
extern void RTIXCdrInstructionIndex_getInstructionIndexByMemberId(
        struct RTIXCdrInstructionIndex *self,
        RTIXCdrUnsignedLong *instructionIndex,
        RTIXCdrBoolean *knownMember,
        const struct RTIXCdrTypeCode **sourceTypeCode,
        const struct RTIXCdrTypeCode *programTypeCode,
        RTIXCdrUnsignedLong memberId);

extern void RTIXCdrInstructionIndex_getInstructionIndexByLabel(
        struct RTIXCdrInstructionIndex *self,
        RTIXCdrUnsignedLong *instructionIndex,
        RTIXCdrLong label);

/* ------------------------------------------------------------------------- */
/* ---- Implementation ----------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrInstructionIndex_getInstructionIndexByMemberIdMacro( \
        self__, \
        instIndex__, \
        knownMember__, \
        sourceTypeCode__, \
        programTypeCode__, \
        memberId__) \
    *(instIndex__) = RTI_XCDR_TYPECODE_INVALID_INDEX; \
    *(knownMember__) = RTI_XCDR_FALSE; \
\
    if ((self__)->count != 0) { \
        RTIXCdrLongLong mid; \
        RTIXCdrUnsignedLong val; \
        RTIXCdrLongLong high; \
        RTIXCdrLongLong low = 0; \
\
        high = (self__)->count - 1; \
        while (low <= high) { \
            mid = low + ((high - low) >> 1); \
            val = (self__)->entries[mid].indexValue.ulVal; \
            if (val < (memberId__)) { \
                low = mid + 1; \
            } else if (val > (memberId__)) { \
                high = mid - 1; \
            } else if ( \
                    (self__)->entries[mid].programTypeCode \
                    == (programTypeCode__)) { \
                *(instIndex__) = (self__)->entries[mid].instructionIndex; \
                *(knownMember__) = RTI_XCDR_TRUE; \
                *(sourceTypeCode__) = (self__)->entries[mid].programTypeCode; \
                break; \
            } else { \
                *(instIndex__) = RTI_XCDR_TYPECODE_INVALID_INDEX; \
                *(knownMember__) = RTI_XCDR_TRUE; \
                *(sourceTypeCode__) = (self__)->entries[mid].programTypeCode; \
                break; \
            } \
        } \
    }

#define RTIXCdrInstructionIndex_BINARY_SEARCH_THRESHOLD 10

#define RTIXCdrInstructionIndex_getInstructionIndexByLabelMacro(self, \
    instIndex, \
    label) \
if ((self)->count > RTIXCdrInstructionIndex_BINARY_SEARCH_THRESHOLD) { \
    RTIXCdrLongLong __mid; \
    RTIXCdrLong __val; \
    RTIXCdrLongLong __high; \
    RTIXCdrLongLong __low = 0; \
 \
    __high = (self)->count - 1; \
    *(instIndex) = (self)->defaultInsIndex; \
 \
    while (__low <= __high) { \
        __mid = __low + ((__high-__low) >> 1); \
        __val = (self)->entries[__mid].indexValue.lVal; \
        if (__val < (label)) { \
            __low = __mid + 1; \
        } else if (__val > (label)) { \
            __high = __mid - 1; \
        } else { \
            *(instIndex) = (self)->entries[__mid].instructionIndex; \
            break; \
        } \
    } \
} else { \
    RTIXCdrUnsignedLong __entryIndex; \
    RTIXCdrLong __val; \
    \
    *(instIndex) = (self)->defaultInsIndex; \
    for (__entryIndex = 0; __entryIndex < (self)->count; __entryIndex++) { \
        __val = (self)->entries[__entryIndex].indexValue.lVal; \
        if (__val == (label)) { \
            *(instIndex) = (self)->entries[__entryIndex].instructionIndex; \
            break; \
        } else if (__val > (label)) { \
            break; \
        } \
    } \
}

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrInstructionIndex_getInstructionIndexByMemberId \
        RTIXCdrInstructionIndex_getInstructionIndexByMemberIdMacro
#endif

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrInstructionIndex_getInstructionIndexByLabel \
        RTIXCdrInstructionIndex_getInstructionIndexByLabelMacro
#endif
