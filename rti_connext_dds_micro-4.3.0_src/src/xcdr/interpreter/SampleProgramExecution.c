/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "../infrastructure/Infrastructure.h"
#include "../stream/Stream.h"
#include "../typeCode/TypeCode.h"
#include "ProgramSupport.h"

#define RTIXCdrSampleInterpreter_populateMemberInfo(memberInfo, tc, context) \
    memberInfo._representation._typeCode = (RTIXCdrTypeCode *) tc; \
    memberInfo._memberFlags = \
    ((context->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) \
            ? RTI_XCDR_NONKEY_MEMBER \
            : RTI_XCDR_REQUIRED_MEMBER); \
    memberInfo._labelsCount = 0

#define RTIXCdrSampleInterpreter_getStringAllocSize(               \
        memberTc__,                                                \
        property__,                                                \
        program__,                                                 \
        actualSize__)                                              \
    ((property__)->allocateMaximumSize                             \
     && (memberTc__)->_maximumLength < (program__)->unboundedSize) \
            ? (memberTc__)->_maximumLength + 1                     \
            : (actualSize__);

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrSampleInterpreter_getUnionStartInstruction(
        RTIXCdrUnsignedLong *startInstruction,
        const struct RTIXCdrProgram *program,
        void *sample,
        RTIXCdrLong *discValueIn,
        RTIXCdrSampleProgramContext *context) 
{
    RTIXCdrMemberValue memberValue;
    RTIXCdrLong discValue;
    RTIXCdrBoolean failure;
    RTIXCdrUnsignedLong i = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLongLong elOffset = 0;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrBoolean found = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(
            startInstruction == NULL, 
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);

    if (discValueIn == NULL) {
        /* 
         * Get the discriminator value from the first instruction,
         * and then only execute the intruction that matches the member that
         * is currently set
         */

        /* The discriminator always has an instruction at position 0 */
        instruction = &program->instructions[0];
        RTIXCdrLog_testPrecondition(
                instruction->opcode != RTI_XCDR_FINALIZE_PRIMITIVE_OPCODE, 
                return RTI_XCDR_FALSE);

        commonParams = RTIXCdrInstruction_getCommonParams(instruction);
        params = &instruction->params;

        elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

        RTIXCdrInterpreter_getMemberValuePtr(
                memberValue,
                NULL,
                sample,
                elOffset,
                0,
                commonParams->memberTc,
                commonParams->tcMemberInfo,
                commonParams->refMemberKind,
                commonParams->useGetMemberValue,
                0,
                context->programData);
        if (memberValue.value.ptr == NULL) {
            /* The discriminator should always exist, so this is an error */
            return RTI_FALSE;
        }

        RTIXCdrInterpreter_primitiveToLongMacro(
                discValue,
                memberValue,
                params->primitiveSampleParams.primitiveKind,
                failure);
        if (failure) {
            return RTI_FALSE;
        }
    } else {
        discValue = *discValueIn;
    }

    /* 
     * Find the correct instruction for the discriminator.
     * If the set case value is a primitive then it will not have an
     * instruction in the program
     */
    for (i = 1; i < program->instructionCount; i++) {
        commonParams = RTIXCdrInstruction_getCommonParams(
                &program->instructions[i]);

        if (commonParams->tcMemberInfo->_labelsCount == 1) {
            if (commonParams->tcMemberInfo->_label != discValue) {
                continue;
            }

            *startInstruction = i;
            found = RTI_XCDR_TRUE;
            break;
        } else {
            RTIXCdrUnsignedLong k = 0;

            for (k = 0; 
                 k < commonParams->tcMemberInfo->_labelsCount; 
                 k++) {
                if (commonParams->tcMemberInfo->_labels[k] == discValue) {
                    *startInstruction = i;
                    found = RTI_XCDR_TRUE;
                    break;
                }
            }

            if (found) {
                break;
            }
        }
    }

    if (!found) {
        /*
         * We can reach here either because:
         * 1. the disc value is a valid case value in the union but there is
         *    no instruction for the member because it identifies a primitive
         *    member that does not require an instruction.
         * 2. the disc value is not a valid case value in the union and there
         *    is no default case label
         * 3. the disc value is not a valid case value in the union and there
         *    is a default case label
         *
         * We first assign startInstruction to instructionCount, indicating that
         * there is no instruction for this member.
         *
         * startInstruction will then be updated to a valid instruction if there
         * is a default case label
         */
        *startInstruction = program->instructionCount;

        if (program->typeCode->_default_index !=
                (RTIXCdrLong)RTI_XCDR_TYPECODE_INVALID_INDEX) {
            /*
             * There is a default case label, get the memberIndex that
             * identifies this member in the union.
             *
             * See CORE-9138 which was filed to address the performance impact
             * of this search.
             */
            RTIXCdrUnsignedLong memberIndex;
            RTIXCdrTypeCode_getUnionMemberIndex(
                    &memberIndex,
                    program->typeCode,
                    discValue);

            if (memberIndex != RTI_XCDR_TYPECODE_INVALID_INDEX
                    && memberIndex ==
                            (RTIXCdrUnsignedLong)program->typeCode->_default_index) {
                /*
                 * The unionDefaultCaseInsIndex identifies the default case
                 * label, as opposed to a different, valid case value in the
                 * union, so assign the startInstruction to
                 * unionDefaultCaseInsIndex, which is the index of the
                 * instruction for the union's default case label.
                 */
                if (program->unionDefaultCaseInsIndex
                    != RTI_XCDR_TYPECODE_INVALID_INDEX) {
                    *startInstruction = program->unionDefaultCaseInsIndex;
                }
            }
        }
    }

    return RTI_TRUE;
}

/*****************************************************************************/
/***** Initialize ************************************************************/
/*****************************************************************************/

RTI_PRIVATE
void RTIXCdrInterpreter_logInitializeError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    RTIXCdrUtility_unusedParameter(runTimeParam);

    switch (messageId) {
    case RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss:
    default:
    {
        RTIXCdrLogParam param[2];
        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeSampleWInstruction(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const RTIXCdrInitializeSampleProperty *property,
        RTIXCdrUnsignedLong startInstructionIn,
        RTIXCdrUnsignedLong instructionCountIn,
        RTIXCdrSampleProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLongLong elOffset = 0;
    RTIXCdrUnsignedLong startInstruction = 0;
    RTIXCdrUnsignedLong upperLoopBound = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong j = 0;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrUnsignedLong sizeIndex = 0;
    RTIXCdrUnsignedLong offsetIndex = 0;
    RTIXCdrBoolean flatData;
    RTIXCdrBoolean initializeToZero;
    RTIXCdrBoolean allocateOptionalMember = RTI_XCDR_FALSE;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(tc);
    initializeToZero = property->initializeToZero;

    if (tc->_sampleAccessInfo == NULL) {
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                context->languageBinding);
    } else {
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding);
        context->languageBinding = tc->_sampleAccessInfo->languageBinding;
    }

    if (flatData) {
        sizeIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               sample)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
        offsetIndex = sizeIndex;
    }

    /*
     * Set the entire sample buffer to zero and then the values that need
     * to be initialized to a non-zero value will be
     */
    if (initializeToZero && context->isTopLevel) {
        /*
         * All TopLevel types have a sample access info for dynamic data.
         * isTopLevel can be true only for complex types (also for arrays or
         * sequences of complex types).
         */
        RTIXCdrLog_testPrecondition(
                tc->_sampleAccessInfo == NULL,
                return RTI_XCDR_FALSE);

        RTIXCdrMemory_zero(sample, tc->_sampleAccessInfo->typeSize[sizeIndex]);
        /*
         * For the initialize program, only the very first type is zeroed,
         * there are no reference members that are initialized so we can set
         * this to false for all of the recursive calls to initialize
         */
        context->isTopLevel = RTI_XCDR_FALSE;
    }

    startInstruction = (startInstructionIn == RTI_XCDR_INSTRUCTION_INVALID
            ? 0 : startInstructionIn);

    if (instructionCountIn != RTI_XCDR_INSTRUCTION_INVALID) {
        if (instructionCountIn == 1
                && startInstructionIn != RTI_XCDR_INSTRUCTION_INVALID) {
            /*
             * This is true when we're explicitly initializing a struct
             * member; if this member is optional we want to allocate it
             */
            allocateOptionalMember = RTI_XCDR_TRUE;
        }
        upperLoopBound = instructionCountIn + startInstruction;
    } else {
        if (kind == RTI_XCDR_TK_UNION 
                && program->instructionCount >= 2
                && RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                        context->languageBinding)) {
            /*
             * A union may only have 1 instruction in a program generated for
             * SQL_TYPE_BINDING if none of the members require an instruction
             * (i.e. a primitive member with a 0 default value)  or if the
             * default discriminator does not identify any member in the union.
             *
             * Otheriwse, the first two instructions in a union program are
             * always for the discriminator and the default member, therefore
             * we just run these two instrctions for the initialize program.
             * There may be more than two instructions if there are cases in the
             * union that are complex members or primitive members with non-zero
             * default values.
             */
            if (program->unionInsIndex == RTI_XCDR_TYPECODE_INVALID_INDEX) {
                /*
                 * The union program does not have a default member, so we
                 * need to run the first instruction only (the discriminator)
                 */
                upperLoopBound = 1;
            } else {
                upperLoopBound = 2;
            }
        } else {
            upperLoopBound = program->instructionCount;
        }
    }

    RTIXCdrLog_testPrecondition(
            upperLoopBound > program->instructionCount,
            return RTI_XCDR_FALSE);

    for (i = startInstruction; i < upperLoopBound; i++) {
        RTIXCdrBoolean failure;
        RTIXCdrMemberValue memberValue;
        const RTIXCdrUnsignedLong *typeSizePtr = NULL;

        instruction = &program->instructions[i];

        commonParams = RTIXCdrInstruction_getCommonParams(instruction);
        RTIXCdrLog_testPrecondition(
                commonParams->memberTc == NULL, 
                return RTI_XCDR_FALSE);

        if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER
                && !allocateOptionalMember) {
            /*
             * Do not allocate: skip this member and move on to the next.
             * We assume the optional is already NULL because either of these
             * situations is true:
             *
             * a) This is a program for a contiguous-memory binding and it
             * always sets initializeToZero to true, so the optional is
             * zeroed-out above. For other bindings, the first time a sample
             * is initialized, initializeToZero must also be set to true.
             *
             * b) When a sample is "reinitialized" back to its default value,
             * and initializeToZero is false, we assume that the optional has
             * been deleted and nullified in the finalize function, which
             * supports finalizeOptionalsOnly.
             */
            continue;
        }

        elOffset = commonParams->memberAccessInfo
                .bindingMemberValueOffset[offsetIndex];
        memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;

        params = &instruction->params;

        switch (instruction->opcode) {
        case RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE:
        case RTI_XCDR_INITIALIZE_STRING_SEQ_OPCODE:
        case RTI_XCDR_INITIALIZE_WSTRING_SEQ_OPCODE:
        case RTI_XCDR_INITIALIZE_COMPLEX_SEQ_OPCODE:
        {
            /* 
             * If the enclosing type is a sequence then tcMemberInfo
             * is NULL. In this case, we must create a temporary memberInfo
             * with the correct type of the sequence so that it can be
             * initialized correctly.
             * 
             * Note, we only populate the fields that will be used and not
             * every member in the tcMember struct.
             *
             * IMPORTANT: the members we populate should be treated as
             * const members.
             */
            struct RTIXCdrTypeCodeMember tcMember;
            RTIXCdrSampleInterpreter_populateMemberInfo(tcMember, tc, context);

            /* 
             * Notice, there is no loop on commonParams->count here because
             * if there is an array of sequences, the array program will run
             * the sequence program count times. We don't perform the
             * initialization of an array of sequences inside the sequence prgm
             */
            RTIXCdrInterpreter_setMemberElementCount(
                    &failure,
                    memberValue,
                    0,
                    sample,
                    elOffset,
                    commonParams->memberTc,
                    &tcMember,
                    /* Use context (instead of member) because this instruction
                    can only be part of a sequence program; if a sequence member
                    is optional, it is indicated in the context */
                    context->refMemberKind,
                    memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                    RTI_XCDR_FALSE,
                    RTI_XCDR_TRUE,
                    context->programData);
            if (failure) {
                context->spaceError = RTI_XCDR_TRUE;
                GotoDoneWithLine();
            }
        }
        break;
        case RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE:
        {
            RTIXCdrOctet primitiveSize = 
                    params->primitiveSampleParams.primitiveSize;
            const void *primValue;

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    allocateOptionalMember
                            ? params->primitiveParams.primitiveByteCount
                            : 0,
                    context->programData);
            if (memberValue.value.ptr == NULL) {
                /* A value is always expected if the member was optional we
                 * either skipped it or getMemberValuePtr just allocated it,
                 * depending on the property
                 */
                GotoDoneWithLine();
            }

            if (kind == RTI_XCDR_TK_UNION && i == 0) {
                /* This is the union discriminator */
                RTIXCdrInterpreter_longToPrimitiveMacro(
                        memberValue,
                        program->defaultUnionDisc,
                        program->unionDiscKind,
                        failure);
                if (failure) {
                    GotoDoneWithLine();
                }
            } else {
                RTIXCdrBoolean setToZero = RTI_XCDR_FALSE;

                if (commonParams->tcMemberInfo == NULL) {
                    /*
                     * NULL for arrays of primitives because unlike an structure
                     * an array does not have typecode members which have
                     * associated tcMemberInfo.
                     */
                    if (RTIXCdrTypeCode_getKind(commonParams->memberTc)
                            == RTI_XCDR_TK_OCTET) {
                        /*
                         * We hit this codepath for an octet array instruction
                         * generated as a result of optimizing the
                         * initialization of an array of a fixed size type 
                         * where all members have default 0.
                         *
                         * This codepath only applies to non contiguous
                         * memory models because contiguous memory models
                         * do a memset before starting program execution.
                         */
                        primValue = &commonParams->memberTc->_annotations
                                             ._defaultValue._u.octet_value;
                        setToZero = RTI_XCDR_TRUE;
                    } else {
                        /*
                         * This codepath is only exercised for arrays of enums.
                         * Arrays of other primitives have 0 default values and:
                         * 
                         * 1) There will be optimized as an array
                         * octets hitting he codepath inside the if above.
                         * OR,
                         * 2) here will be optimized out
                         */
                        primValue = &commonParams->memberTc->_annotations
                                             ._defaultValue._u.long_value;
                    }
                } else {
                    primValue = &commonParams->tcMemberInfo->_annotations
                                         ._defaultValue._u;

                    if (!RTIXCdrAnnotationParameterValue_isNonZero(
                            &commonParams->tcMemberInfo->_annotations
                                     ._defaultValue)) {
                        setToZero = RTI_XCDR_TRUE;
                    }
                }

                if (commonParams->refMemberKind
                        != RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    if (setToZero) {
                        /*
                         * Zero initialization can be optimized with a single
                         * memset
                         */
                        RTIXCdrMemory_zero(
                                memberValue.value.ptr,
                                commonParams->count * primitiveSize);
                    } else {
                        for (j = 0; j < commonParams->count; j++) {
                            if (j != 0) {
                                memberValue.value.ptr += primitiveSize;
                            }
                            RTIXCdrMemory_copy(
                                    memberValue.value.ptr,
                                    primValue,
                                    primitiveSize);
                        }
                    }
                } else {
                    char *samplePtr;

                    for (j = 0; j < commonParams->count; j++) {
                        if (j != 0) {
                            memberValue.value.ptr += program->externalReferenceSize;
                        }

                        samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                                program,
                                memberValue.value.ptr);
                        if (samplePtr == NULL) {
                            GotoDoneWithLine();
                        }

                        RTIXCdrMemory_copy(samplePtr, primValue, primitiveSize);
                    }
                }
            }
        } break;
        case RTI_XCDR_INITIALIZE_ARRAY_OPCODE:
        case RTI_XCDR_INITIALIZE_COMPLEX_OPCODE:
        {
            char *samplePtr;

            /* The size of the member type if it needs to be allocated; zero
             * otherwise
             */
            RTIXCdrUnsignedLong memberAllocSize = 0;
            if (instruction->opcode == RTI_XCDR_INITIALIZE_ARRAY_OPCODE) {
                /* This is the size of each element */
                typeSizePtr = params->complexSampleParams.arrayElementTypeSize;
                if (allocateOptionalMember) {
                    /* This is the size of the full array; we need it to
                     * allocate the optional member
                     */
                    memberAllocSize =
                        params->complexSampleParams.arrayTypeTotalSize;
                }
            } else {
                typeSizePtr = memberSampleAccessInfo->typeSize;
                if (allocateOptionalMember) {
                    memberAllocSize = typeSizePtr[NON_FLAT_DATA_INDEX];
                }
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    memberAllocSize,
                    context->programData);
            if (memberValue.value.ptr == NULL) {
                GotoDoneWithLine();
            }

            for (j = 0; j < commonParams->count; j++) {
                if (j != 0) {
                    RTIXCdrInterpreter_nextArrayElementPtr(
                            memberValue.value.ptr,
                            flatData,
                            typeSizePtr,
                            &params->complexSampleParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr = memberValue.value.ptr;
                } else {
                    samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            memberValue.value.ptr);
                    if (samplePtr == NULL) {
                        GotoDoneWithLine();
                    }
                }

                if (!RTIXCdrSampleInterpreter_initializeSample(
                        samplePtr,
                        params->complexSampleParams.program->typeCode,
                        params->complexSampleParams.program,
                        property,
                        context)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_INITIALIZE_SEQ_OPCODE:
        {
            char *member = (char *)sample + elOffset;
            context->refMemberKind = commonParams->refMemberKind;

            for (j = 0; j < commonParams->count; j++) {
                char *memberPtr;

                if (j != 0) {
                    if (commonParams->refMemberKind !=
                            RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                        member += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    } else {
                        member += program->externalReferenceSize;
                    }
                }

                if (commonParams->refMemberKind
                        != RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    memberPtr = member;
                } else {
                    memberPtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            member);
                    if (memberPtr == NULL) {
                        GotoDoneWithLine();
                    }
                }

                if (!RTIXCdrSampleInterpreter_initializeSample(
                            memberPtr,
                            params->complexSampleParams.program->typeCode,
                            params->complexSampleParams.program,
                            property,
                            context)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_INITIALIZE_WSTRING_OPCODE:
        {
            /* 
             * tcMemberInfo will be NULL if this is an array of strings and
             * the default annotation wstring value may be NULL if the TypeCode
             * was not created through generated code
             */
            char *wstrSample = (char *) sample + elOffset;
            const RTIXCdrUnsignedLong stringSize =
                    (commonParams->tcMemberInfo == NULL 
                    || commonParams->tcMemberInfo->_annotations._defaultValue._u.wstring_value == NULL)
                            ? 1 
                            : RTIXCdrWString_getLength(
                                    commonParams->tcMemberInfo->_annotations._defaultValue._u.wstring_value) + 1;
            /* The size of the string type itself; if not specified by an
             * accessor, it is the size of a reference (e.g. in C, the size of a
             * DDS_Wchar* pointer)
             */
            const RTIXCdrUnsignedLong strTypeSize =
                    (commonParams->refMemberKind
                        == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER)
                    ? program->externalReferenceSize
                    : RTIXCdrInterpreter_getStrTypeSize(memberSampleAccessInfo);
            /* The number of characters to allocate, including the null
             * terminator
             */
            const RTIXCdrUnsignedLong stringAllocSize =
                    RTIXCdrSampleInterpreter_getStringAllocSize(
                            commonParams->memberTc,
                            property,
                            program,
                            stringSize);

            for (j = 0; j < commonParams->count; j++) {
                char *wstrSamplePtr;

                if (j != 0) {
                    wstrSample += strTypeSize;
                 }

                if (commonParams->refMemberKind
                        != RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    wstrSamplePtr = wstrSample;
                } else {
                    wstrSamplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            wstrSample);
                    if (wstrSamplePtr == NULL) {
                        GotoDoneWithLine();
                    }
                }

                RTIXCdrInterpreter_setStrElementCount(
                        &failure,
                        memberValue,
                        stringAllocSize,
                        wstrSamplePtr,
                        0, /* memberValueOffset */
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        RTI_XCDR_FALSE,
                        context->programData);
                if (failure) {
                    context->spaceError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }

                if (stringSize > 1) {
                    if (!RTIXCdrWString_copy(
                            (RTIXCdrWchar *)(void *)memberValue.value.ptr,
                            commonParams->tcMemberInfo->_annotations._defaultValue._u.wstring_value)) {
                        GotoDoneWithLine();
                    }
                } else {
                    RTIXCdrMemory_zero(
                            memberValue.value.ptr, 
                            sizeof(RTIXCdrWchar));
                }
            }
        } break;
        case RTI_XCDR_INITIALIZE_STRING_OPCODE:
        {
            /*
             * tcMemberInfo will be NULL if this is an array of strings and
             * the default annotation string value may be NULL if the TypeCode
             * was not created through generated code
             */
            char *strSample = (char *) sample + elOffset;
            const RTIXCdrUnsignedLong stringSize =
                    (commonParams->tcMemberInfo == NULL
                     || commonParams->tcMemberInfo->_annotations._defaultValue
                                     ._u.string_value
                             == NULL)
                    ? 1
                    : RTIXCdrString_getLength(
                              commonParams->tcMemberInfo->_annotations
                                      ._defaultValue._u.string_value)
                            + 1;
            /* The size of the string type itself; if not specified by an
             * accessor, it is the size of a reference (e.g. in C, the size of a
             * char* pointer)
             */
            const RTIXCdrUnsignedLong strTypeSize =
                    (commonParams->refMemberKind
                        == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER)
                    ? program->externalReferenceSize
                    : RTIXCdrInterpreter_getStrTypeSize(memberSampleAccessInfo);
            /* The number of characters to allocate, including the null
             * terminator
             */
            const RTIXCdrUnsignedLong stringAllocSize =
                    RTIXCdrSampleInterpreter_getStringAllocSize(
                            commonParams->memberTc,
                            property,
                            program,
                            stringSize);

            for (j = 0; j < commonParams->count; j++) {
                char *strSamplePtr;

                if (j != 0) {
                    strSample += strTypeSize;
                }

                if (commonParams->refMemberKind
                        != RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    strSamplePtr = strSample;
                } else {
                    strSamplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            strSample);
                    if (strSamplePtr == NULL) {
                        GotoDoneWithLine();
                    }
                }

                RTIXCdrInterpreter_setStrElementCount(
                        &failure,
                        memberValue,
                        stringAllocSize,
                        strSamplePtr,
                        0, /* memberValueOffset */
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        RTI_XCDR_FALSE,
                        context->programData);
                if (failure) {
                    context->spaceError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }

                if (stringSize > 1) {
                    RTIXCdrMemory_copy(
                            memberValue.value.ptr,
                            commonParams->tcMemberInfo->_annotations
                                    ._defaultValue._u.string_value,
                            stringAllocSize);
                } else {
                    RTIXCdrMemory_zero(
                            memberValue.value.ptr, 
                            sizeof(RTIXCdrChar));
                }
            }
        } break;
        default:
            goto done;
        }
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result && (property->logSpaceErrors || !context->spaceError)) {
        RTIXCdrInterpreter_logInitializeError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}

RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeUnion(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const RTIXCdrInitializeSampleProperty *property,
        RTIXCdrLong discValue,
        RTIXCdrSampleProgramContext *context)
{
    /* Uninitialized Variables */
    RTIXCdrMemberValue memberValue;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong startInstruction;
    RTIXCdrBoolean failure;

    /* Initialized Variables */
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLongLong elOffset = 0;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(tc) != RTI_XCDR_TK_UNION, 
            return RTI_XCDR_FALSE);

    if (!RTIXCdrSampleInterpreter_getUnionStartInstruction(
            &startInstruction, 
            program, 
            sample,
            &discValue,
            context)){
        GotoDoneWithLine();
    }

    if (!RTIXCdrSampleInterpreter_initializeSampleWInstruction(
            sample,
            tc,
            program,
            property,
            startInstruction,
            /*
             * startInstruction == program->instructionCount indicates that
             * there is no instruction for this member, therefore there is no
             * instruction to run and we pass in 0.
             * We call RTIXCdrSampleInterpreter_initializeSampleWInstruction
             * with 0 instructions because we still want to zero out the union's
             * memory and initialize the discriminator value
             */
            startInstruction == program->instructionCount ? 0 : 1,
            context)) {
        GotoDoneWithLine();
    }

    /* 
     * The first instruction in a union's program is for the discriminator,
     * which must be a primitive
     */
    instruction = &program->instructions[0];
    RTIXCdrLog_testPrecondition(
            instruction->opcode != RTI_XCDR_FINALIZE_PRIMITIVE_OPCODE 
            && instruction->opcode != RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE, 
            return RTI_XCDR_FALSE);

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);

    elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

    RTIXCdrInterpreter_getMemberValuePtr(
            memberValue,
            NULL,
            sample,
            elOffset,
            0,
            commonParams->memberTc,
            commonParams->tcMemberInfo,
            commonParams->refMemberKind,
            commonParams->useGetMemberValue,
            0,
            context->programData);
    if (memberValue.value.ptr == NULL) {
        /* The discriminator should always exist, so this is an error */
        GotoDoneWithLine();
    }

    RTIXCdrInterpreter_longToPrimitiveMacro(
            memberValue,
            discValue,
            program->unionDiscKind, 
            failure);
    if (failure) {
        GotoDoneWithLine();
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        RTIXCdrInterpreter_logInitializeError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}

RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeSequenceMembers(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrUnsignedLong indexBegin,
        RTIXCdrUnsignedLong indexEnd,
        RTIXCdrSampleProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong j = 0;
    RTIXCdrUnsignedLongLong memberSize = 0;
    RTIXCdrBoolean failure;
    struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(indexEnd < indexBegin, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(tc) != RTI_XCDR_TK_SEQUENCE, 
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program->instructionCount != 1, 
            return RTI_XCDR_FALSE);

    /* 
     * Sequences have 1 instruction with information about how to initialize
     * their member's type 
     */
    instruction = &program->instructions[0];

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);
    seqElementSampleAccessInfo =
            commonParams->seqElementTc->_sampleAccessInfo;
    params = &instruction->params;

    /* 
     * position seqMemberOffset at the first member in the sequence to
     * initialize
     */
    if (instruction->opcode == RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE) {
        memberSize =
                (params->primitiveSampleParams.primitiveSize
                        * ((RTIXCdrUnsignedLongLong) commonParams->count));
    } else {
        memberSize =
                (seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX] 
                        * ((RTIXCdrUnsignedLongLong) commonParams->count));
    }

    sample = (char *)sample + (indexBegin * memberSize);

    if (RTIXCdrTypeCode_getKind(commonParams->seqElementTc) 
            != RTI_XCDR_TK_ENUM) {

        /*
         * This should never happen, because the serialized size of a
         * type cannot be larger than RTI_XCDR_MAX_SERIALIZED_SIZE.
         * However, we add this precondition so it flags in case we add
         * a bug.
         */
        RTIXCdrLog_testPrecondition(
                memberSize * (indexEnd - indexBegin + 1)
                        > RTI_XCDR_MAX_SERIALIZED_SIZE,
                GotoDoneWithLine());

        /*
         * We don't need to initialize enum sequences to 0 because all of the
         * memory will be initialized directly after.
         * For non-enum primitives the default value is 0 anyway and for
         * strings and complex members we do need to do this initialization to
         * avoid interpreting junk as valid values
         */
        RTIXCdrMemory_zero(
                sample,
                (RTIXCdrUnsignedLong) (memberSize
                                       * (indexEnd - indexBegin + 1)));
    }

    /* 
     * We do the same for loop on i inside each case of the switch to avoid
     * performing the switch i number of times since the outcome will be the
     * same on each iteration.
     */
    switch (instruction->opcode) {
    case RTI_XCDR_INITIALIZE_COMPLEX_SEQ_OPCODE:
    {
        RTIXCdrBoolean isTopLevel = context->isTopLevel;


        for (i = 0; i < (indexEnd - indexBegin + 1); i++) {
            for (j = 0; j < commonParams->count; j++) {
                RTIXCdrInitializeSampleProperty property =
                        RTIXCdrInitializeSampleProperty_INITIALIZER;

                /* Preserve isTopLevel for each member in the sequence */
                context->isTopLevel = isTopLevel;
                property.initializeToZero = RTI_XCDR_FALSE;
                if (!RTIXCdrSampleInterpreter_initializeSample(
                        sample,
                        params->complexSampleParams.program->typeCode,
                        params->complexSampleParams.program,
                        &property,
                        context)) {
                    GotoDoneWithLine();
                }
                sample = (char *)sample + seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            }
        }
    } break;
    case RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE:
    {
        /* Primitive sequences cannot have default values */
        if (RTIXCdrTypeCode_getKind(commonParams->seqElementTc)
                == RTI_XCDR_TK_ENUM) {

            RTIXCdrOctet primitiveSize = 
                    params->primitiveSampleParams.primitiveSize;

            for (i = 0; i < (indexEnd - indexBegin + 1); i++) {
                /* 
                 * For arrays: commonParams->count = length_of_array
                 * for single primitive members: commonParams->count = 1
                 */
                for (j = 0; j < commonParams->count; j++) {
                    RTIXCdrMemory_copy(
                            sample, 
                            &commonParams->seqElementTc->_annotations._defaultValue._u.enumerated_value, 
                            primitiveSize);
                    sample = (char*)sample + primitiveSize;
                }
            }
        }
    } break;
    case RTI_XCDR_INITIALIZE_STRING_SEQ_OPCODE:
    case RTI_XCDR_INITIALIZE_WSTRING_SEQ_OPCODE:
    {
        for (i = 0; i < (indexEnd - indexBegin + 1); i++) {
            RTIXCdrMemberValue memberValue;
            struct RTIXCdrTypeCodeMember tcMember;
            RTIXCdrSampleInterpreter_populateMemberInfo(tcMember, tc, context);

            for (j = 0; j < commonParams->count; j++) {
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        1,
                        sample,
                        0,
                        commonParams->seqElementTc,
                        &tcMember,
                        RTI_XCDR_FALSE,
                        seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        RTI_XCDR_FALSE,
                        RTI_XCDR_TRUE,
                        context->programData);
                if (failure || memberValue.value.ptr == NULL) {
                    /* An empty string should have been allocated */
                    context->spaceError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }
                sample = (char *)sample + seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            }
        }

    } break;
    default:
        goto done;
    }
        
    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        RTIXCdrInterpreter_logInitializeError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}

RTIXCdrBoolean RTIXCdrSampleInterpreter_resizeSequenceMember(
        void *sample,
        const RTIXCdrProgram *program,
        RTIXCdrUnsignedLong index,
        RTIXCdrUnsignedLong newElementCount,
        RTIXCdrSampleProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong elOffset;
    RTIXCdrMemberValue memberValue;
    RTIXCdrBoolean failure;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            index >= program->instructionCount,
            return RTI_XCDR_FALSE);

    instruction = &program->instructions[index];
    commonParams = RTIXCdrInstruction_getCommonParams(instruction);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(commonParams->memberTc)
                    != RTI_XCDR_TK_SEQUENCE,
            return RTI_XCDR_FALSE);

    /*
     * Only individual sequences can be resized; arrays of sequences cannot be
     * resized.
     *
     * This function is currently only used by GenericTypePlugin.cxx, which is
     * used by the Python API to resize a C sequence in a C sample when
     * converting from a Python class. In the Python API, an array of sequences
     * is handled as an array of aliases to a sequence, which is an array of
     * classes/structs, and this function ends up being called for each element
     * of the array, not for the array as a whole.
     */
    RTIXCdrLog_testPrecondition(
            commonParams->count != 1,
            return RTI_XCDR_FALSE);

    if (RTIXCdrInterpreter_getUnboundedSize()
                    != commonParams->memberTc->_maximumLength
            && newElementCount > commonParams->memberTc->_maximumLength) {
        RTIXCdrLog_logTwoLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_dd,
                (RTIXCdrLong) newElementCount,
                (RTIXCdrLong) commonParams->memberTc->_maximumLength);
        goto done;
    }

    elOffset = commonParams->memberAccessInfo
            .bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

    RTIXCdrInterpreter_setMemberElementCount(
            &failure,
            memberValue,
            newElementCount,
            sample,
            elOffset,
            commonParams->memberTc,
            commonParams->tcMemberInfo,
            commonParams->refMemberKind,
            0, /* optional sequences must have been already allocated */
            RTI_XCDR_TRUE, /* trimToSize */
            RTI_XCDR_TRUE, /* initializeElement */
            context->programData);
    if (failure || memberValue.value.ptr == NULL) {
        GotoDoneWithLine();
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        RTIXCdrInterpreter_logInitializeError(
                program->typeCode,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}

/*****************************************************************************/
/***** Finalize **************************************************************/
/*****************************************************************************/

RTI_PRIVATE
void RTIXCdrInterpreter_logFinalizeError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    RTIXCdrUtility_unusedParameter(runTimeParam);

    switch (messageId) {
    case RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss:
    default:
    {
        RTIXCdrLogParam param[2];
        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

/* 
 * For unions, get the discriminator value from the first instruction, and
 * then search through each instruction in the program to see if there is an
 * instruction to finalize the currently set member.
 */
RTIXCdrBoolean RTIXCdrSampleInterpreter_finalizeSample(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrLong *discValue,
        const RTIXCdrFinalizeSampleProperty *property,
        RTIXCdrSampleProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong h = 0;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrUnsignedLong j = 0;
    RTIXCdrUnsignedLong startInstruction = 0;
    RTIXCdrUnsignedLong instructionCount = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLongLong elOffset = 0;
    RTIXCdrUnsignedLong seqElementCount = 0;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    char *member = NULL;
    RTIXCdrBoolean flatData;
    RTIXCdrUnsignedLong offsetIndex = 0;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);

    instructionCount = program->instructionCount;

    kind = RTIXCdrTypeCode_getKind(tc);

    if (kind == RTI_XCDR_TK_UNION) {
        /*
         * contiguous memory bindings only initialize one member at a time;
         * other bindings initialize all members. For contiguous memory bindings
         * allocating memory through a memory manager is efficient so, it's OK
         * to repeatedly allocate and finalize union members as the
         * discriminator changes. However, for non-contiguous bindings (e.g. the
         * C binding, which allocates memory with malloc) it is more efficient
         * to allocate samples to their maximum size, including all member of a
         * union.
         */
        if (RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                context->languageBinding)) {
            if (!RTIXCdrSampleInterpreter_getUnionStartInstruction(
                    &startInstruction,
                    program,
                    sample,
                    discValue,
                    context)) {
                GotoDoneWithLine();
            }

            /*
             * Set instructionCount so that we only execute the one instruction
             * below
             */
            if (startInstruction < instructionCount) {
                instructionCount = startInstruction + 1;
            }
        } else {
            /*
             * skip the discriminator
             */
            startInstruction++;
        }
    }

    if (tc->_sampleAccessInfo == NULL) {
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                context->languageBinding);
    } else {
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding);
        context->languageBinding = tc->_sampleAccessInfo->languageBinding;
    }

    if (flatData) {
        offsetIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               sample)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
    }

    for (i = startInstruction; i < instructionCount; i++) {
        RTIXCdrMemberValue memberValue;
        const RTIXCdrUnsignedLong *typeSizePtr = NULL;
        RTIXCdrBoolean mustFinalizeMember;

        instruction = &program->instructions[i];

        commonParams = RTIXCdrInstruction_getCommonParams(instruction);
        RTIXCdrLog_testPrecondition(
                commonParams->memberTc == NULL, 
                return RTI_XCDR_FALSE);
        memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
        mustFinalizeMember = !property->finalizeOptionalsOnly
                || commonParams->refMemberKind
                        == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER
                || context->refMemberKind
                        == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER;

        params = &instruction->params;

        elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex];

        switch (instruction->opcode) {
        case RTI_XCDR_FINALIZE_PRIMITIVE_SEQ_OPCODE:
        {
            /* 
             * Here we are freeing the sequence buffer, no need to free each
             * element as the only user of this program (dynamic data) does
             * not support pointer members. (so this is not a sequence of
             * primitive pointers)
             */
            struct RTIXCdrTypeCodeMember tcMember;

            if (!mustFinalizeMember) {
                continue;
            }

            RTIXCdrSampleInterpreter_populateMemberInfo(tcMember, tc, context);

            if (memberSampleAccessInfo->finalizeMemberValueFcn != NULL) {
                memberSampleAccessInfo->finalizeMemberValueFcn(
                        sample, 
                        elOffset, 
                        &tcMember,
                        RTI_XCDR_TRUE,
                        context->programData);
            }
        } break;
        case RTI_XCDR_FINALIZE_PRIMITIVE_OPCODE:
        {
            if (!mustFinalizeMember) {
                continue;
            }

            /* 
             * The only reason there is an instruction to finalize a primitive
             * member is because it's a pointer or an optional member,
             * so return the memory
             */ 
            /* 
             * Note, the finalize program does not support finalizing an
             * array of pointers. The only user of the finalize program is
             * DynamicData, and DynamicData does not take into account
             * pointers.
             * Therefore, there is no for loop here on the
             * commonParams->count because we are either finalizing a
             * single optional primitive or an optional array of primitives,
             * which only requires freeing the pointer to the array itself,
             * not each member individually
             */
            if (memberSampleAccessInfo != NULL &&
                    memberSampleAccessInfo->finalizeMemberValueFcn != NULL) {
                memberSampleAccessInfo->finalizeMemberValueFcn(
                        sample,
                        elOffset,
                        commonParams->tcMemberInfo,
                        RTI_XCDR_TRUE,
                        context->programData);
            } else {
                RTIXCdrInterpreter_deleteMember(sample, elOffset);
            }
        } break;
        case RTI_XCDR_FINALIZE_STRING_OPCODE:
        case RTI_XCDR_FINALIZE_WSTRING_OPCODE:
        {
            if (!mustFinalizeMember) {
                continue;
            }

            for (j = 0; j < commonParams->count; j++) {
                RTIXCdrInterpreter_finalizeStr(
                        sample,
                        elOffset,
                        memberSampleAccessInfo,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        RTI_XCDR_TRUE,
                        context->programData);
            }
        } break;
        case RTI_XCDR_FINALIZE_STRING_SEQ_OPCODE:
        case RTI_XCDR_FINALIZE_WSTRING_SEQ_OPCODE:
        {
            struct RTIXCdrTypeCodeMember tcMember;

            if (!mustFinalizeMember) {
                continue;
            }

            RTIXCdrSampleInterpreter_populateMemberInfo(tcMember, tc, context);
            if (!RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                        context->languageBinding)) {
                RTIXCdrBoolean failure;
                /* Set element count to 0 and trim: this forces the destruction
                 * of all elements. If we're only finalizing nested optionals,
                 * but not the whole sequence, we have to go element by element
                 * in the else block.
                 */
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        0,
                        sample,
                        elOffset,
                        commonParams->memberTc,
                        &tcMember,
                        commonParams->refMemberKind,
                        memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        RTI_XCDR_TRUE, /* trim to size */
                        RTI_XCDR_FALSE,
                        context->programData);
            } else {
                struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo;
                seqElementSampleAccessInfo =
                        commonParams->seqElementTc->_sampleAccessInfo;

                /* Get sequence buffer */
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        sample,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        &tcMember,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        context->programData);
                if (memberValue.value.ptr != NULL) {
                    RTIXCdrUnsignedLongLong strOffset = 0;
                    char *seq = memberValue.value.ptr;

                    /* finalize each string in this sequence */
                    for (h = 0; h < seqElementCount; h++) {
                        RTIXCdrInterpreter_finalizeStr(
                                seq,
                                strOffset,
                                seqElementSampleAccessInfo,
                                commonParams->seqElementTc,
                                NULL,
                                RTI_XCDR_TRUE,
                                context->programData);
                    }
                }
            }

            /* Free the sequence itself */
            memberSampleAccessInfo->finalizeMemberValueFcn(
                    sample,
                    elOffset,
                    &tcMember,
                    RTI_XCDR_TRUE,
                    context->programData);
        } break;
        case RTI_XCDR_FINALIZE_ARRAY_OPCODE:
        case RTI_XCDR_FINALIZE_COMPLEX_OPCODE:
        {
            /* If this member is optional, all of its nested members must be
             * finalized (not just nested optionals)
             */
            RTIXCdrFinalizeSampleProperty nestedProperty =
                    RTIXCdrFinalizeSampleProperty_INITIALIZER;
            nestedProperty.finalizeOptionalsOnly = !mustFinalizeMember;

            if (nestedProperty.finalizeOptionalsOnly
                    && !params->complexSampleParams.program->hasOptionals) {
                continue;
            }

            if (instruction->opcode == RTI_XCDR_FINALIZE_ARRAY_OPCODE) {
                typeSizePtr = params->complexSampleParams.arrayElementTypeSize;
            } else {
                typeSizePtr = memberSampleAccessInfo->typeSize;
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    context->programData);
            if (memberValue.value.ptr != NULL) {
                member = memberValue.value.ptr;

                for (j = 0; j < commonParams->count; j++) {
                    RTIXCdrInterpreter_nextArrayElementPtr( 
                            member,
                            flatData,
                            typeSizePtr,
                            &params->complexSampleParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);

                    /* 
                     * Finalize each member of a complex array (or just
                     * the complex member itself if not an array)
                     */
                    if (!RTIXCdrSampleInterpreter_finalizeSample(
                            memberValue.value.ptr,
                            commonParams->memberTc,
                            params->complexSampleParams.program,
                            NULL,
                            &nestedProperty,
                            context)) {
                        GotoDoneWithLine();
                    }

                    memberValue.value.ptr = member;
                }

                if (mustFinalizeMember) {
                    if (memberSampleAccessInfo != NULL
                        && memberSampleAccessInfo->finalizeMemberValueFcn
                                != NULL) {
                        memberSampleAccessInfo->finalizeMemberValueFcn(
                                sample,
                                elOffset,
                                commonParams->tcMemberInfo,
                                RTI_XCDR_TRUE,
                                context->programData);
                    } else if (
                            commonParams->refMemberKind
                            == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                        RTIXCdrInterpreter_deleteMember(sample, elOffset);
                    }
                }
            }
        } break;
        case RTI_XCDR_FINALIZE_SEQ_OPCODE:
        {
            RTIXCdrRefMemberKind previousRefMemberKind = context->refMemberKind;
            member = (char *) sample + elOffset;

            context->refMemberKind = commonParams->refMemberKind;
            for (j = 0; j < commonParams->count; j++) {
                if (!RTIXCdrSampleInterpreter_finalizeSample(
                        member,
                        commonParams->memberTc,
                        params->complexSampleParams.program,
                        NULL,
                        property,
                        context)) {
                    /* suppress Coverity MISSING_RESTORE issue */
                    context->refMemberKind = previousRefMemberKind;
                    GotoDoneWithLine();
                }

                member += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            }

            context->refMemberKind = previousRefMemberKind;
        } break;
        case RTI_XCDR_FINALIZE_COMPLEX_SEQ_OPCODE:
        {
            struct RTIXCdrTypeCodeMember tcMember;
            RTIXCdrBoolean failure;
            /* If this member is optional, all of its nested members must be
             * finalized (not just nested optionals)
             */
            RTIXCdrFinalizeSampleProperty nestedProperty =
                    RTIXCdrFinalizeSampleProperty_INITIALIZER;
            nestedProperty.finalizeOptionalsOnly = !mustFinalizeMember;

            RTIXCdrSampleInterpreter_populateMemberInfo(tcMember, tc, context);

            if (nestedProperty.finalizeOptionalsOnly
                    && !params->complexSampleParams.program->hasOptionals) {
                continue;
            }

            if (!RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                        context->languageBinding)
                    && !nestedProperty.finalizeOptionalsOnly) {
                /* Set element count to 0 and trim: this forces the destruction
                 * of all elements. If we're only finalizing nested optionals,
                 * but not the whole sequence, we have to go element by element
                 * in the else block.
                 */
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        0,
                        sample,
                        elOffset,
                        commonParams->memberTc,
                        &tcMember,
                        commonParams->refMemberKind,
                        memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        RTI_XCDR_TRUE, /* trim to size */
                        RTI_XCDR_FALSE,
                        context->programData);
            } else {
                struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo;
                seqElementSampleAccessInfo =
                        commonParams->seqElementTc->_sampleAccessInfo;

                /* Get sequence buffer */
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        sample,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        &tcMember,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        context->programData);
                if (memberValue.value.ptr != NULL) {
                    member = memberValue.value.ptr;

                    for (h = 0; h < seqElementCount; h++) {
                        /* finalize each member of this sequence */
                        member += seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];

                        if (!RTIXCdrSampleInterpreter_finalizeSample(
                                memberValue.value.ptr,
                                commonParams->seqElementTc,
                                params->complexSampleParams.program,
                                NULL,
                                &nestedProperty,
                                context)) {
                            GotoDoneWithLine();
                        }

                        memberValue.value.ptr = member;
                    }
                }
            }

            if (mustFinalizeMember) {
                /* Free the sequence itself */
                if (memberSampleAccessInfo->finalizeMemberValueFcn != NULL) {
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            sample,
                            elOffset,
                            &tcMember,
                            RTI_XCDR_TRUE,
                            context->programData);
                }
            }
        } break;
        default:
            GotoDoneWithLine();
        }
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        RTIXCdrInterpreter_logFinalizeError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}


/*****************************************************************************/
/***** Copy ******************************************************************/
/*****************************************************************************/

RTI_PRIVATE
void RTIXCdrInterpreter_logCopyError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    RTIXCdrUtility_unusedParameter(runTimeParam);

    switch (messageId) {
    case RTI_XCDR_LOG_COPY_FAILURE_ID_ss:
    default:
    {
        RTIXCdrLogParam param[2];
        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_COPY_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

/* 
 * Copying DynamicData objects requires replacing each reference within that
 * block with a new reference in the destination's memory manager block.
 * 
 * The current implementation assumes that the memory in destPt has been freed.
 * There is an initial memcpy of source into destPt which will leave all
 * references in destPt dangling if they have not been freed already.
 */ 
RTIXCdrBoolean RTIXCdrSampleInterpreter_copySample(
        void *destPt,
        void *source,
        RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrSampleProgramContext *sourceContext,
        RTIXCdrSampleProgramContext *destContext)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong h = 0;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrUnsignedLong j = 0;
    RTIXCdrUnsignedLong startInstruction = 0;
    RTIXCdrUnsignedLong instructionCount = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLongLong elOffset = 0;
    RTIXCdrUnsignedLong seqElementCount = 0;
    RTIXCdrUnsignedLong charCount = 0;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_COPY_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    char *sourceMember = NULL;
    char *destinationMember = NULL;
    RTIXCdrBoolean failure = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong typeSize = 0;
    RTIXCdrUnsignedLong offsetIndex = 0;
    RTIXCdrUnsignedLong sizeIndex = 0;
    RTIXCdrBoolean flatData;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;

    RTIXCdrLog_testPrecondition(destPt == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(source == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sourceContext == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(destContext == NULL, return RTI_XCDR_FALSE);
    
    kind = RTIXCdrTypeCode_getKind(tc);

    if (tc->_sampleAccessInfo == NULL) {
        RTIXCdrLog_testPrecondition(
                sourceContext->isTopLevel, 
                return RTI_XCDR_FALSE);
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                sourceContext->languageBinding);
    } else {
        flatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding);
        sourceContext->languageBinding = tc->_sampleAccessInfo->languageBinding;
        destContext->languageBinding = tc->_sampleAccessInfo->languageBinding;
    }

    if (flatData) {
        sizeIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               source)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
        offsetIndex = sizeIndex;
    }
    
    if (sourceContext->isTopLevel) {
        /*
         * Perform a memcpy from src to dst. Afterwards, only reference members
         * will be copied one-by-one. This code path is used when we are dealing
         * with a contiguous piece of memory (dynamic data and flat data use
         * cases)
         */
        RTIXCdrMemory_copy(
                destPt, 
                source, 
                tc->_sampleAccessInfo->typeSize[sizeIndex]);
    }

    instructionCount = program->instructionCount;
    
    if (kind == RTI_XCDR_TK_UNION) {
        if (!RTIXCdrSampleInterpreter_getUnionStartInstruction(
                &startInstruction,
                program, 
                source, 
                NULL,
                sourceContext)) {
            GotoDoneWithLine();
        }

        /* 
         * Set instructionCount so that we only execute the one instruction
         * below
         */
        if (startInstruction < instructionCount) {
            instructionCount = startInstruction + 1;
        }
    }
    
    for (i = startInstruction; i < instructionCount; i++) {
        RTIXCdrMemberValue sourceMemberValue;
        RTIXCdrMemberValue destinationMemberValue;
        const RTIXCdrUnsignedLong *typeSizePtr = NULL;
        RTIXCdrBoolean isSeq = RTI_XCDR_FALSE;

        instruction = &program->instructions[i];

        commonParams = RTIXCdrInstruction_getCommonParams(instruction);
        RTIXCdrLog_testPrecondition(
                commonParams->memberTc == NULL, 
                return RTI_XCDR_FALSE);
        memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
        params = &instruction->params;
        sourceMemberValue.isNull = RTI_XCDR_FALSE;

        elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex];

        switch (instruction->opcode) {
        case RTI_XCDR_COPY_PRIMITIVE_SEQ_OPCODE:
        {
            struct RTIXCdrTypeCodeMember tcMember;
            RTIXCdrSampleInterpreter_populateMemberInfo(
                    tcMember, 
                    tc, 
                    sourceContext);


            /* Get the source sequence buffer */
            RTIXCdrInterpreter_getMemberValuePtr(
                    sourceMemberValue,
                    &seqElementCount,
                    source,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    &tcMember,
                    sourceContext->refMemberKind,
                    RTIXCdrProgram_useGetMemberValueInMember(
                            commonParams->memberTc,
                            (sourceContext->refMemberKind
                                    != RTI_XCDR_INTERPRETER_VALUE_MEMBER)),
                    0,
                    sourceContext->programData);
            /* We don't need to process optional, unset sequences */
            if (((sourceContext->refMemberKind
                    == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)
                    && !sourceMemberValue.isNull)
                    || (sourceContext->refMemberKind
                            != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)) {
                if (sourceContext->refMemberKind
                        != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                    /* 
                     * Don't finalize optional sequences, they were finalized
                     * and re-allocated by the calling program
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt, 
                            elOffset, 
                            &tcMember,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                }

                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        destinationMemberValue,
                        seqElementCount,
                        destPt,
                        elOffset,
                        commonParams->memberTc,
                        &tcMember,
                        sourceContext->refMemberKind,
                        params->primitiveSampleParams.primitiveSize,
                        RTI_XCDR_FALSE,
                        RTI_XCDR_TRUE,
                        destContext->programData);
                if (failure) {
                    GotoDoneWithLine();
                }

                if (seqElementCount > 0) {
                    RTIXCdrLog_testPrecondition(
                            sourceMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);
                    RTIXCdrLog_testPrecondition(
                            destinationMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);

                    /* Copy the value into the allocated memory */
                    RTIXCdrMemory_copy(
                            destinationMemberValue.value.ptr,
                            sourceMemberValue.value.ptr,
                            params->primitiveSampleParams.primitiveSize * seqElementCount);
                }
            }
        } break;
        case RTI_XCDR_COPY_PRIMITIVE_OPCODE:
        {
            /* Get the source pointer */
            RTIXCdrInterpreter_getMemberValuePtr(
                    sourceMemberValue,
                    NULL,
                    source,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    sourceContext->programData);
            if (sourceMemberValue.value.ptr != NULL) {
                /* 
                 * Get/allocate the optional primitive in the destination
                 * sample
                 */
                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_VALUE_MEMBER) {
                    /* 
                     * This call to finalize will not deallocate any memory,
                     * just clear the reference so we can reallocate in the call
                     * to setMemberElementCount
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt, 
                            elOffset, 
                            commonParams->tcMemberInfo,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                }

                RTIXCdrInterpreter_getMemberValuePtr(
                        destinationMemberValue,
                        NULL,
                        destPt,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        params->primitiveSampleParams.primitiveSize,
                        destContext->programData);
                if (destinationMemberValue.value.ptr == NULL) {
                    GotoDoneWithLine();
                }

                /* Copy the value into the allocated memory */
                RTIXCdrMemory_copy(
                        destinationMemberValue.value.ptr,
                        sourceMemberValue.value.ptr,
                        params->primitiveSampleParams.primitiveSize
                                * commonParams->count);
            }
        } break;
        case RTI_XCDR_COPY_STRING_OPCODE:
        case RTI_XCDR_COPY_WSTRING_OPCODE:
        {
            typeSize = memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];

            for (j = 0; j < commonParams->count; j++) {
                RTIXCdrInterpreter_getMemberValuePtr(
                        sourceMemberValue,
                        &charCount,
                        source,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        sourceContext->programData);
                if (sourceMemberValue.value.ptr != NULL) {
                    /* 
                     * This call to finalize will not deallocate any memory,
                     * just clear the reference so we can reallocate in the
                     * call to setMemberElementCount
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt, 
                            elOffset, 
                            commonParams->tcMemberInfo,
                            RTI_XCDR_FALSE,
                            destContext->programData);

                    /* Allocate the required memory in the destination */
                    RTIXCdrInterpreter_setStrElementCount(
                            &failure,
                            destinationMemberValue,
                            charCount,
                            destPt,
                            elOffset,
                            commonParams->memberTc,
                            commonParams->tcMemberInfo,
                            RTI_XCDR_TRUE,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                    if (failure) {
                        GotoDoneWithLine();
                    }

                    RTIXCdrLog_testPrecondition(
                            destinationMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);

                    /* Copy the value into the allocated memory */
                    RTIXCdrMemory_copy(
                            destinationMemberValue.value.ptr,
                            sourceMemberValue.value.ptr,
                            params->strParams.charSize * charCount);
                }

                elOffset += typeSize;
            }
        } break;
        case RTI_XCDR_COPY_STRING_SEQ_OPCODE:
        case RTI_XCDR_COPY_WSTRING_SEQ_OPCODE:
        {
            char *srcSeq = NULL;
            char *dstSeq = NULL;
            RTIXCdrUnsignedLongLong strOffset = 0;
            struct RTIXCdrTypeCodeMember tcMember;
            struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo;

            RTIXCdrSampleInterpreter_populateMemberInfo(
                    tcMember, 
                    tc, 
                    sourceContext);

            seqElementSampleAccessInfo =
                    commonParams->seqElementTc->_sampleAccessInfo;

            /*
             * The copy function is currently only used for DynamicData and in
             * DynamicData the seqElementTc->_sampleAccessInfo is always
             * non-NULL.
             */
            RTIXCdrLog_testPrecondition(
                    seqElementSampleAccessInfo == NULL,
                    return RTI_XCDR_FALSE);

            /* Get src sequence buffer */
            RTIXCdrInterpreter_getMemberValuePtr(
                    sourceMemberValue,
                    &seqElementCount,
                    source,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    &tcMember,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    sourceContext->programData);
            /* We don't need to process optional, unset sequences */
            if (((sourceContext->refMemberKind
                    == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)
                    && !sourceMemberValue.isNull)
                    || (sourceContext->refMemberKind
                            != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)) {
                if (sourceContext->refMemberKind
                        != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                    /* 
                     * Don't finalize optional sequences, they were finalized
                     * and re-allocated by the calling program
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt,
                            elOffset, 
                            &tcMember,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                }

                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        destinationMemberValue,
                        seqElementCount,
                        destPt,
                        elOffset,
                        commonParams->memberTc,
                        &tcMember,
                        commonParams->refMemberKind,
                        seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        RTI_XCDR_FALSE,
                        RTI_XCDR_TRUE,
                        destContext->programData);
                if (failure) {
                    GotoDoneWithLine();
                }

                RTIXCdrLog_testPrecondition(
                        (seqElementCount > 0)
                                && (sourceMemberValue.value.ptr == NULL),
                        return RTI_XCDR_FALSE);

                srcSeq = sourceMemberValue.value.ptr;
                dstSeq = destinationMemberValue.value.ptr;

                /* copy each string in this sequence */
                for (h = 0; h < seqElementCount; h++) {
                    RTIXCdrInterpreter_getMemberValuePtr(
                            sourceMemberValue,
                            &charCount,
                            srcSeq,
                            strOffset,
                            0,
                            commonParams->seqElementTc,
                            NULL,
                            commonParams->refMemberKind,
                            RTIXCdrProgram_useGetMemberValueInMember(
                                    commonParams->seqElementTc,
                                    commonParams->refMemberKind),
                            0,
                            sourceContext->programData);
                    if (sourceMemberValue.value.ptr != NULL) {
                        /* 
                         * CORE-11187: 
                         * Finalize (clear) the string reference before
                         * allocating a new one because we don't initialize
                         * memory during the copy operation, the sequence
                         * elements that were allocated in setMemberElementCount
                         * above may contain junk references
                         */
                        seqElementSampleAccessInfo->finalizeMemberValueFcn(
                                dstSeq, 
                                strOffset, 
                                commonParams->tcMemberInfo,
                                RTI_XCDR_FALSE,
                                destContext->programData);

                        /* Allocate the required memory in the destination */
                        RTIXCdrInterpreter_setStrElementCount(
                                &failure,
                                destinationMemberValue,
                                charCount,
                                dstSeq,
                                strOffset,
                                commonParams->seqElementTc,
                                NULL,
                                RTI_XCDR_TRUE,
                                RTI_XCDR_FALSE,
                                destContext->programData);
                        if (failure) {
                            GotoDoneWithLine();
                        }

                        RTIXCdrLog_testPrecondition(
                                destinationMemberValue.value.ptr == NULL, 
                                return RTI_XCDR_FALSE);

                        /* Copy the value into the allocated memory */
                        RTIXCdrMemory_copy(
                                destinationMemberValue.value.ptr,
                                sourceMemberValue.value.ptr,
                                params->strParams.charSize * charCount);
                    }

                    strOffset += seqElementSampleAccessInfo
                                         ->typeSize[NON_FLAT_DATA_INDEX];
                }
            }
        } break;
        case RTI_XCDR_COPY_SEQ_OPCODE:
        case RTI_XCDR_COPY_ARRAY_OPCODE:
        case RTI_XCDR_COPY_COMPLEX_OPCODE:
        {
            RTIXCdrUnsignedLong complexOffsetIndex = 0;
            struct RTIXCdrTypeCodeMember *tcMemberPtr = NULL;
            struct RTIXCdrTypeCodeMember tcMember;

            if (instruction->opcode == RTI_XCDR_COPY_SEQ_OPCODE) {
                isSeq = RTI_XCDR_TRUE;
                typeSizePtr = memberSampleAccessInfo->typeSize;
            } else if (instruction->opcode == RTI_XCDR_COPY_ARRAY_OPCODE) {
                typeSizePtr = params->complexSampleParams.arrayElementTypeSize;
            } else {
                typeSizePtr = memberSampleAccessInfo->typeSize;
            }

            tcMemberPtr = commonParams->tcMemberInfo;
            if (commonParams->tcMemberInfo == NULL) {
                RTIXCdrSampleInterpreter_populateMemberInfo(
                        tcMember, 
                        tc, 
                        sourceContext);
                tcMemberPtr = &tcMember;
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    sourceMemberValue,
                    &seqElementCount,
                    source,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    tcMemberPtr,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    sourceContext->programData);
            /* If not a sequence
             *      Process set, optional members and non-optional members
             * Or if a sequence:
             *      Process set, optional sequences and non-optional sequences
             */
            if (!commonParams->refMemberKind
                    || (commonParams->refMemberKind && !sourceMemberValue.isNull)) {
                if (commonParams->refMemberKind) {
                    /* 
                     * This call to finalize will not deallocate any memory,
                     * just clear the reference so we can reallocate in the call
                     * to setStrElementCount
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt, 
                            elOffset,
                            tcMemberPtr,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                }

                /* 
                 * If the source is not NULL, then get the destination ptr.
                 * If needed, this call will allocate the member
                 */
                if (flatData) {
                    complexOffsetIndex =
                            (RTIXCdrUnsignedLong) ((RTIXCdrUtility_pointerToUnsignedLongLong(
                                                            destPt)
                                                    + elOffset)
                                                   % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
                }

                RTIXCdrInterpreter_getMemberValuePtr(
                        destinationMemberValue,
                        NULL,
                        destPt,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        tcMemberPtr,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        memberSampleAccessInfo->typeSize[complexOffsetIndex],
                        destContext->programData);
                
                if (isSeq) {
                    sourceMember = (char *)source + elOffset;
                    destinationMember = (char *)destPt + elOffset;
                } else {
                    RTIXCdrLog_testPrecondition(
                            sourceMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);
                    RTIXCdrLog_testPrecondition(
                            destinationMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);

                    sourceMember = sourceMemberValue.value.ptr;
                    destinationMember = destinationMemberValue.value.ptr;
                }

                for (j = 0; j < commonParams->count; j++) {
                    /* 
                     * Set the context in the loop because they may be
                     * overwritten  by nested calls
                     */
                    if (isSeq) {
                        sourceContext->isTopLevel = RTI_XCDR_FALSE;
                        destContext->isTopLevel = RTI_XCDR_FALSE;

                        sourceContext->refMemberKind = commonParams->refMemberKind;
                        destContext->refMemberKind = commonParams->refMemberKind;
                    } else {
                        sourceContext->isTopLevel =
                                (commonParams->refMemberKind
                                        != RTI_XCDR_INTERPRETER_VALUE_MEMBER);
                        destContext->isTopLevel =
                                (commonParams->refMemberKind
                                        != RTI_XCDR_INTERPRETER_VALUE_MEMBER);
                    }

                    if (!RTIXCdrSampleInterpreter_copySample(
                            destinationMember,
                            sourceMember,
                            commonParams->memberTc,
                            params->complexSampleParams.program,
                            sourceContext,
                            destContext)) {
                        GotoDoneWithLine();
                    }

                    RTIXCdrInterpreter_nextArrayElementPtr( 
                            sourceMember,
                            flatData,
                            typeSizePtr,
                            &params->complexSampleParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);

                    RTIXCdrInterpreter_nextArrayElementPtr( 
                            destinationMember,
                            flatData,
                            typeSizePtr,
                            &params->complexSampleParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);

                }
            }           
        } break;
        case RTI_XCDR_COPY_COMPLEX_SEQ_OPCODE:
        {
            struct RTIXCdrTypeCodeMember *tcMemberPtr = NULL;
            struct RTIXCdrTypeCodeMember tcMember;
            struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo;

            seqElementSampleAccessInfo =
                    commonParams->seqElementTc->_sampleAccessInfo;

            /* If there is a sequence of arrays */
            tcMemberPtr = commonParams->tcMemberInfo;
            if (commonParams->tcMemberInfo == NULL) {
                RTIXCdrSampleInterpreter_populateMemberInfo(
                        tcMember, 
                        tc, 
                        sourceContext);
                tcMemberPtr = &tcMember;
            }

            /* Get sequence buffer */
            RTIXCdrInterpreter_getMemberValuePtr(
                    sourceMemberValue,
                    &seqElementCount,
                    source,
                    elOffset,
                    0,
                    commonParams->memberTc,
                    tcMemberPtr,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    sourceContext->programData);
            /* We don't need to process optional, unset sequences */
            if (((sourceContext->refMemberKind
                    == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)
                    && !sourceMemberValue.isNull)
                    || (sourceContext->refMemberKind
                            != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER)) {
                if (sourceContext->refMemberKind
                        != RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                    /* 
                     * Don't finalize optional sequences, they were finalized
                     * and re-allocated by the calling program
                     */
                    memberSampleAccessInfo->finalizeMemberValueFcn(
                            destPt, 
                            elOffset, 
                            tcMemberPtr,
                            RTI_XCDR_FALSE,
                            destContext->programData);
                }

                /* Allocated the sequence's buffer */
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        destinationMemberValue,
                        seqElementCount,
                        destPt,
                        elOffset,
                        commonParams->memberTc,
                        tcMemberPtr,
                        commonParams->refMemberKind,
                        seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        RTI_XCDR_FALSE,
                        RTI_XCDR_TRUE,
                        destContext->programData);
                if (failure) {
                    GotoDoneWithLine();
                }

                if (seqElementCount > 0) {
                    RTIXCdrLog_testPrecondition(
                            sourceMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);
                    RTIXCdrLog_testPrecondition(
                            destinationMemberValue.value.ptr == NULL, 
                            return RTI_XCDR_FALSE);

                    RTIXCdrMemory_copy(
                            destinationMemberValue.value.ptr, 
                            sourceMemberValue.value.ptr,
                            seqElementCount * seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX]);
                }

                for (h = 0; h < seqElementCount; h++) {
                    /* 
                     * Set the context in the loop because they may be
                     * overwritten  by nested calls
                     */

                    /* 
                     * The copy was performed above, so we set this to false
                     * Note: this will have to be changed for sequences of
                     * pointers to complex members
                     */
                    sourceContext->isTopLevel = RTI_XCDR_FALSE;
                    destContext->isTopLevel = RTI_XCDR_FALSE;

                    /* 
                     * Members of a sequence cannot themselves be references, so
                     * set this to value member.
                     */
                    sourceContext->refMemberKind =
                            destContext->refMemberKind =
                                    RTI_XCDR_INTERPRETER_VALUE_MEMBER;

                    if (!RTIXCdrSampleInterpreter_copySample(
                            destinationMemberValue.value.ptr,
                            sourceMemberValue.value.ptr,
                            commonParams->memberTc->_typeCode,
                            params->complexSampleParams.program,
                            sourceContext,
                            destContext)) {
                        GotoDoneWithLine();
                    }

                    sourceMemberValue.value.ptr += 
                            seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    destinationMemberValue.value.ptr += 
                            seqElementSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                }
            }
        } break;
        default:
            GotoDoneWithLine();
        }
    }

    result = RTI_XCDR_TRUE;
done:
    if (!result) {
        RTIXCdrInterpreter_logCopyError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return result;
}
