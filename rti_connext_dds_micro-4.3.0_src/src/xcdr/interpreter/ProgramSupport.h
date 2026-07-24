/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_interpreter.h"

/* When working with fix size flat data, the position of a member or the size
 * of a type changes depending on where a type is used. This is why flat data
 * requires up to 4 member offsets and 4 type sizes.
 *
 * When the language binding is not flat data we only need to work with one
 * member offset and one type size.
 *
 * This constant is used to represent the index of the offset or the type size
 * for non flat data language bindings
 */
#define NON_FLAT_DATA_INDEX 0

#define GotoDoneWithLine() \
    logLineNumber = __LINE__; \
    goto done

extern const char * RTIXCdrProgramKind_toStr(
        RTIXCdrTypeProgramKind programKind);

extern void RTIXCdrInstruction_copy(
        RTIXCdrInstruction *dst, 
        const RTIXCdrInstruction *src);

extern void RTIXCdrInstruction_getCommonParams(RTIXCdrInstruction *me);

extern void RTIXCdrInstruction_initialize(RTIXCdrInstruction * me);

extern const char * RTIXCdrInstruction_opCodeToStr(RTIXCdrInstruction * me);

extern RTIXCdrBoolean RTIXCdrInstruction_isPrimitiveOpcode(RTIXCdrOctet opcode);

extern RTIXCdrBoolean RTIXCdrInstruction_isPrimitiveSeqOpcode(RTIXCdrOctet opcode);

extern RTIXCdrBoolean RTIXCdrInstruction_isStringOpcode(RTIXCdrOctet opcode);

extern RTIXCdrBoolean RTIXCdrInstruction_isStringNonSeqOpcode(RTIXCdrOctet opcode);

extern RTIXCdrBoolean RTIXCdrInstruction_isStringSeqOpcode(RTIXCdrOctet opcode);

extern const char * RTIXCdrInstruction_getMemberName(
        const RTIXCdrInstruction * me,
        const RTIXCdrTypeCode *tc);

extern RTIXCdrBoolean RTIXCdrInstruction_isComplexOpcode(RTIXCdrOctet opcode);

extern RTIXCdrBoolean RTIXCdrInstruction_isHeaderOpcode(RTIXCdrOctet opcode);

extern void RTIXCdrProgram_print(RTIXCdrProgram *me, const char *title);

extern void RTIXCdrInterpreter_logProgramGenerationError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrTypeProgramKind programKind,
        RTIXCdrLogMessageId messageId,
        const char *functionName,
        RTIXCdrUnsignedLong line);

extern void RTIXCdrInterpreter_deleteProgram(RTIXCdrProgram *program);

extern RTIXCdrProgram *RTIXCdrInterpreter_newProgram(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList **dependentProgramList,
        RTIXCdrTypeProgramKind programKind,
        const struct RTIXCdrTypePluginProgramProperty *property);

extern void RTIXCdrInterpreter_isOptionalMemberValueSet(
        RTIXCdrBoolean *memberValueSet,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        void *programData);

/* @brief Gets a pointer to a member value.
 * 
 * The pointer to the member value is returned in memberValue.value.ptr
 */
extern void RTIXCdrInterpreter_getMemberValuePtr(
        RTIXCdrMemberValue memberValue,
        RTIXCdrUnsignedLong *elementCount,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        RTIXCdrUnsignedLong elementIndex,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrOctet refMemberKind,
        RTIXCdrBoolean useGetMemberValue,
        RTIXCdrUnsignedLong memberAllocSize,
        void *programData);

extern void RTIXCdrInterpreter_getPrimitiveMemberValuePtr(
        RTIXCdrMemberValue memberValue,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset);

extern char * RTIXCdrProgram_getExternalRefValuePtr(
        RTIXCdrProgram *program,
        void *externalRefPtr);

extern void RTIXCdrInterpreter_getStrValuePtr(
        RTIXCdrMemberValue memberValue,
        RTIXCdrLong charAlignment,
        RTIXCdrUnsignedLong elementCount,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        RTIXCdrUnsignedLong elementIndex,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        void *programData);

extern void RTIXCdrInterpreter_setMemberElementCount(
        RTIXCdrBoolean *failure,
        RTIXCdrMemberValue memberValue,
        RTIXCdrUnsignedLong elementCount,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrOctet refMemberKind,
        RTIXCdrUnsignedLong memberAllocSize,
        RTIXCdrBoolean trimToSize,
        RTIXCdrBoolean initializeElement,
        void *programData);

extern void RTIXCdrInterpreter_setStrElementCount(
        RTIXCdrBoolean *failure,
        RTIXCdrMemberValue memberValue,
        /* Always > 0 */
        RTIXCdrUnsignedLong elementCount,
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrOctet refMemberKind,
        RTIXCdrBoolean trimToSize,
        void *programData);

extern void RTIXCdrInterpreter_finalizeStr(
        void *sample,
        RTIXCdrUnsignedLongLong memberValueOffset,
        struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrBoolean deallocateReference,
        void *programData);

extern RTIXCdrBoolean RTIXCdrInterpreter_useMemberElementIndex(
        const RTIXCdrSampleAccessInfo *memberSampleAccessInfo);

extern void RTIXCdrInterpreter_getTcSingleMember(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrTypeCodeMember *member);

extern RTIXCdrBoolean RTIXCdrInterpreter_primitiveToLong(
        RTIXCdrLong *lValue,
        const RTIXCdrMemberValue *value,
        RTIXCdrBoolean useMemberIndexForAccessDiscValue,
        RTIXCdrTCKind primitiveKind);

extern RTIXCdrBoolean RTIXCdrInterpreter_longToPrimitive(
        RTIXCdrMemberValue *value,
        RTIXCdrLong lValue,
        RTIXCdrBoolean useMemberIndexForAccessDiscValue,
        RTIXCdrTCKind primitiveKind);

extern void RTIXCdrInterpreter_nextArrayElementPtr( 
        RTIXCdrMemberValue *value,
        RTIXCdrBoolean flatData,
        RTIXCdrUnsignedLong *typeSize,
        const RTIXCdrComplexInsParameters *complexParams,
        RTIXCdrOctet refMemberKind,
        RTIXCdrUnsignedShort externalRefSize);

extern RTIXCdrUnsignedLong RTIXCdrProgram_getFirstDataInstIndex(
        RTIXCdrProgram *self);

extern RTIXCdrUnsignedLong RTIXCdrProgram_getNextDataInstIndex(
        RTIXCdrProgram *self,
        RTIXCdrUnsignedLong currentInstIndex);

extern RTIXCdrBoolean RTIXCdrProgram_useGetMemberValueInMember(
        const struct RTIXCdrTypeCode * memberTc,
        RTIXCdrBoolean refMember);

extern void RTIXCdrProgram_deleteInstructions(RTIXCdrProgram *program);

struct RTIXCdrDependentProgramList;
typedef void RTIXCdrDependentProgramListNode;

/*
 * @brief This constant represents the threshold used to decide if a
 * dependent program list should be created based on an underlying SkipList or
 * InlineList.
 * 
 * When the number of expected aggregation (struct, valuetype, union) programs 
 * in the dependent list is above this threshold, the underlying list will be 
 * a SkipList.
 */
#define RTI_XCDR_DEPENDENT_PROGRAM_LIST_INLINE_THRESHOLD 512

/*
 * @brief Delete a list of dependent programs.
 *
 * This operation deletes both: the program nodes and the programs contained
 * in the list.
 * 
 * @param self InOut. The list. Cannot be NULL.
 */
extern void RTIXCdrDependentProgramList_delete(
        struct RTIXCdrDependentProgramList *self);

/*
 * @brief Creates a list of dependent programs for the top level tc.
 *
 * tc will be used to decide if the list of dependent programs will be
 * based on an underlying InlineList or SkipList. The decision is made based
 * on the number of aggregation (struct, valuetype, union) typecodes contained
 * in tc.
 * 
 * The programs will be indexed based on:
 * - program typecode pointer value
 * - program kind value
 * - program onlyKey value
 * 
 * @param tc In. Top-level typecode. Cannot be NULL.
 * 
 * @return The list or NULL if there is an error.
 */
extern struct RTIXCdrDependentProgramList * RTIXCdrDependentProgramList_newWithTc(
        const RTIXCdrTypeCode *tc);

/*
 * @brief Creates a list of dependent programs.
 *
 * expectedProgramCount will be used to decide if the list of dependent programs 
 * will be based on an underlying InlineList or SkipList.
 * 
 * The programs will be indexed based on:
 * - program typecode pointer value
 * - program kind value
 * - program onlyKey value
 *
 * @param expectedProgramCount In. Number of expected programs in the list.
 * 
 * @return The list or NULL if there is an error.
 */
extern struct RTIXCdrDependentProgramList * RTIXCdrDependentProgramList_new(
        RTIXCdrUnsignedLong expectedProgramCount);

/*
 * @brief Finds the first program in the program list with typecode equals
 * to tc, program kind equals to kind and program onlyKey attribute quals to
 * onlyKey.
 *
 * @param self In. The list. Cannot be NULL.
 * @param tc In. Expected program typecode. Cannot be NULL.
 * @param kind In. Expected program kind.
 * @param onlyKey In. Expected program onlyKey.
 *
 * @return Found program or NULL.
 */
extern RTIXCdrProgram * RTIXCdrDependentProgramList_findProgramWithKey(
        const struct RTIXCdrDependentProgramList *self,
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrTypeProgramKind kind,
        RTIXCdrBoolean onlyKey);

/*
 * @brief Adds a program to the input dependent list.
 *
 * @param self InOut. The list. Cannot be NULL.
 * @param program In. Program to be added. Cannot be NULL or part of the list.
 *
 * @return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
extern RTIXCdrBoolean RTIXCdrDependentProgramList_addProgram(
        struct RTIXCdrDependentProgramList *self,
        RTIXCdrProgram *program);

/*
 * @brief Gets a pointer to the program associated with a node.
 *
 * @param self \b In. The list. Cannot be NULL.
 * @param node \b In. The node. Cannot be NULL.
 * 
 * @return Pointer to the the program associated with a node.
 */
extern struct RTIXCdrProgram * RTIXCdrDependentProgramList_getNodeProgram(
        const struct RTIXCdrDependentProgramList *self,
        const RTIXCdrDependentProgramListNode *node);

/*
 * @brief Gets a pointer to the first node in the list.
 *
 * @param self \b In. The list. Cannot be NULL.
 * 
 * @return Pointer to the first node or NULL if the list is empty.
 */
extern const RTIXCdrDependentProgramListNode * RTIXCdrDependentProgramList_getFirstNode(
        const struct RTIXCdrDependentProgramList *self);

/*
 * @brief Gets a pointer to the next node in the list.
 *
 * @param self \b In. The list. Cannot be NULL.
 * @param prevNode \b In. Pointer to previous node. Cannot be NULL.
 * 
 * @return Pointer to the next node or NULL if prevNode is the last node.
 */
extern const RTIXCdrDependentProgramListNode * RTIXCdrDependentProgramList_getNextNode(
        const struct RTIXCdrDependentProgramList *self,
        const RTIXCdrDependentProgramListNode *prevNode);

/*
 * @brief Returns RTI_XCDR_TRUE if the list is using an underlying
 * inline list as opposed to an underlying skiplist.
 *
 * @param self \b In. The list. Cannot be NULL.
 * 
 * @return RTI_XCDR_TRUE if the list uses an underlying inline list. Otherwise,
 * RTI_XCDR_FALSE.
 */
extern RTIXCdrBoolean RTIXCdrDependentProgramList_isInlineList(
        const struct RTIXCdrDependentProgramList *self);

extern RTIXCdrUnsignedLong RTIXCdrInterpreter_getStrTypeSize(
        struct RTIXCdrSampleAccessInfo *sampleAccessInfo);

/* ------------------------------------------------------------------------- */
/* ---- Implementation ----------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrInstruction_copy(dst, src) *(dst) = *(src)

#define RTIXCdrInstruction_getCommonParams(inst) \
    (RTIXCdrCommonInsParameters *)&(inst)->params

#define RTIXCdrInstruction_isPrimitiveOpcode(opcode) ( \
    ((opcode) & 0xF8) == 0xC0 \
            || ((opcode) & 0xF8) == 0x40)

#define RTIXCdrInstruction_isPrimitiveSeqOpcode(opcode) ( \
    ((opcode) & 0xF8) == 0xC0)

#define RTIXCdrInstruction_isStringOpcode(opcode) ( \
    ((opcode) & 0xF8) == 0xA0 \
            || ((opcode) & 0xF8) == 0x20 \
            || ((opcode) & 0xF8) == 0x90 \
            || ((opcode) & 0xF8) == 0x10)

#define RTIXCdrInstruction_isStringNonSeqOpcode(opcode) \
    (((opcode) & 0xF8) == 0x20 \
            || ((opcode) & 0xF8) == 0x10)

#define RTIXCdrInstruction_isStringSeqOpcode(opcode) \
    (((opcode) & 0xF8) == 0xA0 \
            || ((opcode) & 0xF8) == 0x90)

#define RTIXCdrInstruction_isComplexOpcode(opcode) ( \
    ((opcode) & 0xF8) == 0x88 \
            || ((opcode) & 0xF8) == 0x08)

#define RTIXCdrInstruction_isHeaderOpcode(opcode) ( \
    ((opcode) & 0xF8) == 0x28            /* RTI_XCDR_..._MEMBER_HEADER_OPCODE */ \
            || ((opcode) & 0xF8) == 0x30 /* RTI_XCDR_..._SENTINEL_HEADER_OPCODE */ \
            || ((opcode) & 0xF8) == 0x18 /* RTI_XCDR_..._DHEADER_OPCODE */ \
            || ((opcode) & 0xF8) == 0x48 /* RTI_XCDR_..._COLLECTION_DHEADER_OPCODE */)

#define RTIXCdrInterpreter_allocateMemberValueIfNeeded( \
    memberValue, \
    sample, \
    memberValueOffset, \
    memberTc, \
    memberAllocSize, \
    allocatePointers__, \
    programData) \
{ \
    if ((memberValue).value.ptr == NULL && (memberAllocSize) > 0) { \
        (memberValue).value.ptr = RTIXCdrHeap_allocateWithAllocKind( \
                (memberAllocSize), \
                RTI_XCDR_STRUCT_ALLOC); \
        if ((memberValue).value.ptr != NULL) { \
            if ((memberTc)->_typePlugin != NULL) { \
                if ((memberTc)->_typePlugin->initializeSampleFnc != NULL) { \
                    if (!(memberTc)->_typePlugin->initializeSampleFnc( \
                            (memberValue).value.ptr, \
                            (allocatePointers__), \
                            RTI_XCDR_TRUE)) { \
                        RTIXCdrHeap_free((memberValue).value.ptr); \
                        (memberValue).value.ptr = NULL; \
                    } \
                } else if ((memberTc)->_typePlugin->initializeSampleWParamsFnc != NULL) { \
                    if (!(memberTc)->_typePlugin->initializeSampleWParamsFnc( \
                            (memberValue).value.ptr, \
                            (memberTc), \
                            NULL, /* unionInfo */ \
                            (programData), \
                            (memberTc)->_typePlugin->typePluginParam)) { \
                        RTIXCdrHeap_free((memberValue).value.ptr); \
                        (memberValue).value.ptr = NULL; \
                    } \
                } \
            } \
            *(char **)((void *)((char *)(sample) + (memberValueOffset))) = \
                (memberValue).value.ptr; \
        } \
    } \
}

#define RTIXCdrInterpreter_allocateStrValueIfNeeded( \
    memberValue, \
    memberTc, \
    sample, \
    memberValueOffset, \
    charCount, /* charCount is always > 0 */\
    trimToSize) \
{ \
    if (((memberValue).value.ptr == NULL) || \
            (trimToSize)) { \
        if (RTIXCdrTypeCode_getKind((memberTc)) == RTI_XCDR_TK_STRING) { \
            if ((trimToSize) && (memberValue).value.ptr != NULL) { \
                RTIXCdrHeap_freeString((memberValue).value.ptr); \
                (memberValue).value.ptr = NULL; \
            } \
            (memberValue).value.ptr = RTIXCdrHeap_allocateWithAllocKind( \
                    (charCount), \
                    RTI_XCDR_STRING_ALLOC); \
        } else { \
            if ((trimToSize) && (memberValue).value.ptr != NULL) { \
                RTIXCdrHeap_freeWString((memberValue).value.ptr); \
                (memberValue).value.ptr = NULL; \
            } \
            (memberValue).value.ptr = RTIXCdrHeap_allocateWithAllocKind( \
                    (RTIXCdrUnsignedLong)((charCount)*(sizeof(RTIXCdrWchar))), \
                    RTI_XCDR_ARRAY_ALLOC); \
        } \
        *(char **)(void *)((char *)(sample) + (memberValueOffset)) = \
            (memberValue).value.ptr; \
    } \
}

#define RTIXCdrInterpreter_getMemberValuePtr( \
        memberValue, \
        elementCount, \
        sample, \
        memberValueOffset, \
        elementIndex, \
        memberTc, \
        tcMemberInfo, \
        refMemberKind, \
        useGetMemberValue, \
        memberAllocSize, \
        programData) \
{ \
    if ((useGetMemberValue)) { \
        (memberValue) = (memberTc)->_sampleAccessInfo->getMemberValuePointerFcn( \
                (sample), \
                (elementCount), \
                (memberValueOffset), \
                (elementIndex), \
                (memberTc), \
                (tcMemberInfo), \
                (((refMemberKind) == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) && (memberAllocSize)>0), \
                (programData)); \
    } else { \
        if ((refMemberKind) != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) { \
            (memberValue).value.ptr = (char *)((char *)(sample) \
                    + (memberValueOffset)); \
        } else { \
            (memberValue).value.ptr = *(char **)((void *)((char *)(sample) \
                    + (memberValueOffset))); \
            RTIXCdrInterpreter_allocateMemberValueIfNeeded( \
                    (memberValue), \
                    (sample), \
                    (memberValueOffset), \
                    (memberTc), \
                    (memberAllocSize), \
                    RTI_XCDR_TRUE, /* allocatePointers */ \
                    (programData)); \
        } \
    } \
}

#define RTIXCdrInterpreter_getPrimitiveMemberValuePtr( \
        memberValue, \
        sample, \
        memberValueOffset) \
    (memberValue).value.ptr = (char *)((char *)(sample) \
        + (memberValueOffset)); \

#define RTIXCdrInterpreter_getStrValuePtr( \
        __memberValue, \
        __charAlignment, \
        __elementCount, \
        __sample, \
        __memberValueOffset, \
        __elementIndex, \
        __memberTc, \
        __tcMemberInfo, \
        __programData) \
{ \
    if ((__memberTc)->_sampleAccessInfo != NULL && \
            (__memberTc)->_sampleAccessInfo->getMemberValuePointerFcn != NULL) { \
        (__memberValue) = (__memberTc)->_sampleAccessInfo->getMemberValuePointerFcn( \
                (__sample), \
                (__elementCount), \
                (__memberValueOffset), \
                (__elementIndex), \
                (__memberTc), \
                (__tcMemberInfo), \
                RTI_XCDR_FALSE, \
                (__programData)); \
    } else { \
        (__memberValue).value.ptr = *(char **)((void *)((char *)(__sample) \
                + (__memberValueOffset))); \
        if ((void *) (__elementCount) != NULL) { \
            if ((__memberValue).value.ptr != NULL) { \
                if ((__charAlignment) == RTI_XCDR_ONE_BYTE_ALIGNMENT) { \
                    *(__elementCount) = (RTIXCdrString_getLengthWithMax( \
                            (__memberValue).value.ptr, \
                            (__memberTc)->_maximumLength) + 1); \
                } else { \
                    *(__elementCount) = (RTIXCdrWString_getLengthWithMax( \
                            (void *)(__memberValue).value.ptr, \
                            (__memberTc)->_maximumLength) + 1); \
                } \
            } else { \
                *(__elementCount) = 0; \
            } \
        } \
    } \
}

#define RTIXCdrInterpreter_isOptionalMemberValueSet( \
        memberValueSet, \
        sample, \
        memberValueOffset, \
        memberTc, \
        tcMemberInfo, \
        programData) \
{ \
    if ((memberTc)->_sampleAccessInfo != NULL && \
            (memberTc)->_sampleAccessInfo->getMemberValuePointerFcn != NULL) { \
        RTIXCdrMemberValue localMemberValue; \
        (localMemberValue) = (memberTc)->_sampleAccessInfo->getMemberValuePointerFcn( \
                (sample), \
                NULL, \
                (memberValueOffset), \
                0, \
                (memberTc), \
                (tcMemberInfo), \
                RTI_XCDR_FALSE, \
                (programData)); \
         *(memberValueSet) = (localMemberValue).isNull? \
                RTI_XCDR_FALSE:RTI_XCDR_TRUE; \
    } else { \
        char *ptr = *(char **)(void *)((char *)(sample) + (memberValueOffset)); \
        \
        if (ptr == NULL) { \
            *(memberValueSet) = RTI_XCDR_FALSE; \
        } else { \
            *(memberValueSet) = RTI_XCDR_TRUE; \
        } \
    } \
}

/* There is no a single language binding in which sequences are a pointer
 * to the buffer of elements. Because of that and to simplify the logic in
 * which setMemberElementCountFcn is not set is removed
 */
#define RTIXCdrInterpreter_setMemberElementCount( \
        failure, \
        memberValue, \
        elementCount, \
        sample, \
        memberValueOffset, \
        memberTc, \
        tcMemberInfo, \
        refMemberKind, \
        memberAllocSize, \
        trimToSize, \
        initializeElement, \
        programData) \
{ \
    /* Optional members are finalized everytime a sample is return to the \
     * sample pool on the reader queue. Because of this if memory has to \
     * be allocated we always do it to the exact size \
     */ \
    RTIXCdrBoolean __allocateMemoryIfNeeded = \
        (((refMemberKind) == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) && \
        (memberAllocSize)>0); \
    (memberValue) = (memberTc)->_sampleAccessInfo->setMemberElementCountFcn( \
        (failure), \
        (sample), \
        (elementCount), \
        (memberValueOffset), \
        (memberTc), \
        (tcMemberInfo), \
        __allocateMemoryIfNeeded, \
        __allocateMemoryIfNeeded?RTI_XCDR_TRUE:(trimToSize), \
        initializeElement, \
        (programData)); \
}

#define RTIXCdrInterpreter_setStrElementCount( \
        failure, \
        memberValue, \
        elementCount, /* For strings elementCount is always > 0 */\
        sample, \
        memberValueOffset, \
        memberTc, \
        tcMemberInfo, \
        refMemberKind, \
        trimToSize, \
        programData) \
{ \
    if ((memberTc)->_sampleAccessInfo != NULL && \
            (memberTc)->_sampleAccessInfo->setMemberElementCountFcn != NULL) { \
        (memberValue) =(memberTc)->_sampleAccessInfo->setMemberElementCountFcn( \
                (failure), \
                (sample), \
                (elementCount), \
                (memberValueOffset), \
                (memberTc), \
                (tcMemberInfo), \
                RTI_XCDR_TRUE, \
                ((trimToSize) || \
                    (refMemberKind) == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER), \
                RTI_XCDR_FALSE, \
                (programData)); \
    } else { \
        *(failure) = RTI_XCDR_FALSE; \
        (memberValue).value.ptr = *(char **)(void *)((char *)(sample) \
                + (memberValueOffset)); \
        RTIXCdrInterpreter_allocateStrValueIfNeeded( \
                (memberValue), \
                (memberTc), \
    		(sample), \
    		(memberValueOffset), \
    		(elementCount), \
                (trimToSize)); \
    	if ((memberValue).value.ptr == NULL) { \
    	    *(failure) = RTI_XCDR_TRUE; \
    	} \
    } \
}

#define RTIXCdrInterpreter_finalizeStr(                                      \
        sample,                                                              \
        offset,                                                              \
        memberSampleAccessInfo,                                              \
        memberTc,                                                            \
        tcMemberInfo,                                                        \
        deallocateReference,                                                 \
        programData)                                                         \
    {                                                                        \
        if (memberSampleAccessInfo != NULL                                   \
            && memberSampleAccessInfo->finalizeMemberValueFcn != NULL) {     \
            memberSampleAccessInfo->finalizeMemberValueFcn(                  \
                    (sample),                                                \
                    (offset),                                                \
                    (tcMemberInfo),                                          \
                    (deallocateReference),                                   \
                    (programData));                                          \
            offset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX]; \
        } else {                                                             \
            char **memberPtr__ =                                             \
                    (char **) (void *) ((char *) (sample) + (offset));       \
            if (*memberPtr__ != NULL) {                                       \
                if (RTIXCdrTypeCode_getKind((memberTc))                      \
                    == RTI_XCDR_TK_STRING) {                                 \
                    RTIXCdrHeap_freeString(*memberPtr__);                    \
                    offset += sizeof(char *);                                \
                } else {                                                     \
                    RTIXCdrHeap_freeWString(*memberPtr__);                   \
                    offset += sizeof(RTIXCdrWchar *);                        \
                }                                                            \
                *memberPtr__ = NULL;                                         \
            }                                                                \
        }                                                                    \
    }

#define RTIXCdrInterpreter_useMemberElementIndex(memberSampleAccessInfo) \
    (memberSampleAccessInfo != NULL &&  \
        memberSampleAccessInfo->setMemberElementValueFcn != NULL)


#define RTIXCdrInterpreter_nextArrayElementPtr( \
        memberValue, \
        flatData, \
        typeSize, \
        complexParams, \
        refMemberKind, \
        externalRefSize) \
    if (refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) { \
        (memberValue) += (externalRefSize); \
    } else if ((flatData)) { \
        (memberValue) += (typeSize)[( \
                RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong( \
                                              memberValue) \
                                      % RTI_XCDR_MAX_XCDR2_ALIGNMENT)]; \
        (memberValue) = (char *) RTIXCdrAlignment_alignAddressUp( \
                (void *) (memberValue), \
                (complexParams)->program->firstMemberAlignment); \
    } else { \
        (memberValue) += (typeSize)[0]; \
    }

#define RTIXCdrProgram_useGetMemberValueInMember(memberTc, refMember) \
(((memberTc)->_sampleAccessInfo != NULL && \
    (memberTc)->_sampleAccessInfo->getMemberValuePointerFcn != NULL && \
    (!(memberTc)->_sampleAccessInfo->useGetMemberValueOnlyWithRef || \
            (refMember)))?RTI_XCDR_TRUE:RTI_XCDR_FALSE)

#define RTIXCdrProgram_getExternalRefValuePtr(program, externalRefPtr) \
    ((program->getExternalRefPointerFcn != NULL)? \
        (program)->getExternalRefPointerFcn((externalRefPtr)): \
        *((char **)((void *)externalRefPtr)))

/* 
 * This is done mostly for DynamicData language binding which doesn't align
 * 8 byte primitives to 8 bytes, but 4. Directly accessing an 8 byte value from
 * a 4-byte aligned address on Solaris causes a bus error, so we must first
 * memcpy it and then assign the out value. 
 */ 
#if defined(RTI_SOL2)
#define RTIXCdrInterpreter_8ByteToLongMacro(longValue, prim8ByteValue, primType) \
{ \
    primType __primValueCopy; \
    RTIXCdrMemory_copy( \
            &__primValueCopy, \
            (void *)(prim8ByteValue), \
            sizeof(primType)); \
    (longValue) = (RTIXCdrLong) __primValueCopy; \
}
#else
#define RTIXCdrInterpreter_8ByteToLongMacro(longValue, prim8ByteValue, primType) \
    (longValue) = (RTIXCdrLong) *((primType *)(void *)(prim8ByteValue));
#endif

#define RTIXCdrInterpreter_primitiveToLongMacro( \
        lValue, \
        primValue, \
        primitiveKind, \
        failure) \
    (failure) = RTI_XCDR_FALSE; \
    switch ((primitiveKind)) { \
        case RTI_XCDR_TK_BOOLEAN: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrBoolean *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_CHAR: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrChar *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_OCTET: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrOctet *)(void *)(primValue).value.ptr);\
        } break; \
        case RTI_XCDR_TK_INT8: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrInt8 *)(void *)(primValue).value.ptr);\
        } break; \
        case RTI_XCDR_TK_UINT8: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrUInt8 *)(void *)(primValue).value.ptr);\
        } break; \
        case RTI_XCDR_TK_WCHAR: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrWchar *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_SHORT: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrShort *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_USHORT: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrUnsignedShort *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_ENUM: \
        case RTI_XCDR_TK_LONG: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrLong *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_ULONG: \
        { \
            (lValue) = (RTIXCdrLong) \
                *((RTIXCdrUnsignedLong *)(void *)(primValue).value.ptr); \
        } break; \
        case RTI_XCDR_TK_LONGLONG: \
        { \
            RTIXCdrInterpreter_8ByteToLongMacro( \
                    (lValue), \
                    (primValue).value.ptr, \
                    RTIXCdrLongLong); \
        } break; \
        case RTI_XCDR_TK_ULONGLONG: \
        { \
            RTIXCdrInterpreter_8ByteToLongMacro( \
                    (lValue), \
                    (primValue).value.ptr, \
                    RTIXCdrUnsignedLongLong); \
        } break; \
        default: \
        { \
            (lValue) = (RTIXCdrLong)RTI_XCDR_TYPECODE_INVALID_INDEX; \
            (failure) = RTI_XCDR_TRUE; \
        } \
    }

#define RTIXCdrInterpreter_primitiveToLongWIndexMacro( \
        lValue, \
        primValue, \
        primitiveKind, \
        failure) \
    (failure) = RTI_XCDR_FALSE; \
    switch ((primitiveKind)) { \
        case RTI_XCDR_TK_BOOLEAN: \
        { \
            (lValue) = (RTIXCdrLong)(primValue).value.bVal; \
        } break; \
        case RTI_XCDR_TK_WCHAR: \
        { \
            (lValue) = (RTIXCdrLong)(primValue).value.wVal; \
        } break; \
        case RTI_XCDR_TK_LONG: \
        { \
            (lValue) = (RTIXCdrLong)(primValue).value.lVal; \
        } break; \
        default: \
        { \
            (lValue) = (RTIXCdrLong)RTI_XCDR_TYPECODE_INVALID_INDEX; \
            (failure) = RTI_XCDR_TRUE; \
        } \
    }

#define RTIXCdrInterpreter_longToPrimitiveMacro( \
        primValue, \
        lValue, \
        primitiveKind, \
        failure) \
    (failure) = RTI_XCDR_FALSE; \
    switch ((primitiveKind)) { \
        case RTI_XCDR_TK_BOOLEAN: \
        { \
            *((RTIXCdrBoolean *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrBoolean)(lValue); \
        } break; \
        case RTI_XCDR_TK_CHAR: \
        { \
            *((RTIXCdrChar *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrChar)(lValue); \
        } break; \
        case RTI_XCDR_TK_OCTET: \
        { \
            *((RTIXCdrOctet *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrOctet)(lValue); \
        } break; \
        case RTI_XCDR_TK_INT8: \
        { \
            *((RTIXCdrInt8 *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrInt8)(lValue); \
        } break; \
        case RTI_XCDR_TK_UINT8: \
        { \
            *((RTIXCdrUInt8 *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrUInt8)(lValue); \
        } break; \
        case RTI_XCDR_TK_WCHAR: \
        { \
            *((RTIXCdrWchar *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrWchar)(lValue); \
        } break; \
        case RTI_XCDR_TK_SHORT: \
        { \
            *((RTIXCdrShort *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrShort)(lValue); \
        } break; \
        case RTI_XCDR_TK_USHORT: \
        { \
            *((RTIXCdrUnsignedShort *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrUnsignedShort)(lValue); \
        } break; \
        case RTI_XCDR_TK_ENUM: \
        case RTI_XCDR_TK_LONG: \
        { \
            *((RTIXCdrLong *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrLong)(lValue); \
        } break; \
        case RTI_XCDR_TK_ULONG: \
        { \
            *((RTIXCdrUnsignedLong *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrUnsignedLong)(lValue); \
        } break; \
        case RTI_XCDR_TK_LONGLONG: \
        { \
            *((RTIXCdrLongLong *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrLongLong)(lValue); \
        } break; \
        case RTI_XCDR_TK_ULONGLONG: \
        { \
            *((RTIXCdrUnsignedLongLong *)(void *)(primValue).value.ptr) = \
                    (RTIXCdrUnsignedLongLong)(lValue); \
        } break; \
        default: \
        { \
            (failure) = RTI_XCDR_TRUE; \
        } \
    }

#define RTIXCdrInterpreter_longToPrimitiveWIndexMacro( \
        primValue, \
        lValue, \
        primitiveKind, \
        failure) \
    (failure) = RTI_XCDR_FALSE; \
    switch ((primitiveKind)) { \
        case RTI_XCDR_TK_BOOLEAN: \
        { \
            (primValue).value.bVal = \
                    (RTIXCdrBoolean)(lValue); \
        } break; \
        case RTI_XCDR_TK_WCHAR: \
        { \
            (primValue).value.wVal = \
                    (RTIXCdrWchar)(lValue); \
        } break; \
        case RTI_XCDR_TK_LONG: \
        { \
            (primValue).value.lVal = \
                    (RTIXCdrLong)(lValue); \
        } break; \
        default: \
        { \
            (failure) = RTI_XCDR_TRUE; \
        } \
    }

#define RTIXCdrInterpreter_getStrTypeSize(strSampleAccessInfo__)               \
    ((strSampleAccessInfo__) != NULL                                           \
                     && (strSampleAccessInfo__)->typeSize[NON_FLAT_DATA_INDEX] \
                             != 0                                              \
             ? (strSampleAccessInfo__)->typeSize[NON_FLAT_DATA_INDEX]          \
             : ((RTIXCdrUnsignedLong) sizeof(char *)))

#define RTIXCdrInterpreter_deleteMember(sample__, elementOffset__) \
  { \
    char **memberPtr__ = \
            (char **) (void *) ((char *) (sample__) + (elementOffset__)); \
    RTIXCdrHeap_free(*memberPtr__); \
    *memberPtr__ = NULL; \
  }
