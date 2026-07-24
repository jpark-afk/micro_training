/*
(c) Copyright, Real-Time Innovations, 2014-2025.
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
#include "xcdr/xcdr_rtiddsgen_infrastructure.h"

#define RTI_XCDR_SAMPLE_PROGRAM_COUNT 2

RTIXCdrOctet RTIXCdrSampleInterpreter_g_primitiveOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE,
    RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_primitiveSeqOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE,
    RTI_XCDR_ALLOCATED_PRIMITIVE_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_stringOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_STRING_OPCODE,
    RTI_XCDR_ALLOCATED_STRING_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_wstringOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_WSTRING_OPCODE,
    RTI_XCDR_ALLOCATED_WSTRING_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_complexOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_COMPLEX_OPCODE,
    RTI_XCDR_ALLOCATED_COMPLEX_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_complexSeqOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_COMPLEX_SEQ_OPCODE,
    RTI_XCDR_ALLOCATED_COMPLEX_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_stringSeqOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_STRING_SEQ_OPCODE,
    RTI_XCDR_ALLOCATED_STRING_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_seqOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_SEQ_OPCODE,
    RTI_XCDR_ALLOCATED_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrSampleInterpreter_g_arrayOpCode[RTI_XCDR_SAMPLE_PROGRAM_COUNT] =
{
    RTI_XCDR_INITIALIZE_ARRAY_OPCODE,
    RTI_XCDR_ALLOCATED_ARRAY_OPCODE
};

/*****************************************************************************/
/***** Program Generation ****************************************************/
/*****************************************************************************/

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrSampleInterpreter_generateInstructionOpcode(
        RTIXCdrInstruction *instruction,
        RTIXCdrOctet *opCodes,
        RTIXCdrTypeProgramKind programKind)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);

    switch (programKind) {
        case RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM:
            instruction->opcode = opCodes[0];
            break;
        case RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM:
            instruction->opcode = opCodes[1];
            break;
        default:
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                    "sample program");
            goto done;
    }

    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

/**
 * This function calculates the size of the arrayTc elements. The elements
 * themselves can be arrays.
 *
 * @param[out] arrayParams The array parameters to populate.
 * @param[in] arrayTc Input array.
 * @param[in] memberTc The input array member typecode. It may itself be an
 * array.
 * @param[in] isContiguousMemoryBinding. Indicates if the language binding is
 * contiguous or not.
 * @param[in] externalReferenceSize. Size of an external reference.
 * 
 * The size calculated in this function is used for two purposes:
 * 
 * 1) To allocate the buffer for the elements of an optional array in non
 * contiguous memory bindings.
 * 2) To move from one element to the next during program execution.
 * 
 * If the element is an external reference, the size is calculated considering
 * externalReferenceSize instead of the element type size.
 */
RTI_PRIVATE
void RTIXCdrSampleInterpreter_calculateArraySizes(
        RTIXCdrComplexSampleInsParameters *arrayParams,
        const RTIXCdrTypeCode *arrayTc,
        const RTIXCdrTypeCode *memberTc,
        RTIXCdrBoolean isContiguousMemoryBinding,
        RTIXCdrUnsignedShort externalReferenceSize)
{
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrUnsignedLong typeSizes[RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES] = {0};
    RTIXCdrUnsignedLong count = 1;
    RTIXCdrAlignment alignment = 0;
    RTIXCdrUnsignedLong i = 0;

    RTIXCdrLog_testPrecondition(arrayTc == NULL, return);
    RTIXCdrLog_testPrecondition(memberTc == NULL, return);
    RTIXCdrLog_testPrecondition(arrayParams == NULL, return);

    /* 
     * If the array or array member already has a sampleAccessInfo, just use
     * the sizes from that.
     */
    if (memberTc->_sampleAccessInfo != NULL) {
        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            arrayParams->arrayElementTypeSize[i] =
                    memberTc->_sampleAccessInfo->typeSize[i];
        }

        return;
    } 
    
    if (arrayTc->_sampleAccessInfo != NULL) {
        count = RTIXCdrTypeCode_getArrayElementCount(arrayTc);
        RTIXCdrLog_testPrecondition(count == 0, return);

        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            arrayParams->arrayElementTypeSize[i] =
                    arrayTc->_sampleAccessInfo->typeSize[i] / count;
        }

        return;
    }

    /*
     * If the member is an array, we calculate the number of elements
     * in the flatten member array stopping the first time we encounter a
     * non-array element or an external reference.
     * 
     * For example:
     * 
     * typedef long MyLongArr[2]; 
     * typedef MyLongArr MyLongArrArr[3];
     * 
     * In the previous example assuming that memberTc is MyLongArrArr
     * count would be 6.
     * 
     * typedef long *MyLongArr[2]; 
     * typedef MyLongArr MyLongArrArr[3];
     * 
     * In the previous example assuming that memberTc is MyLongArrArr
     * count would be 3.
     *
     */
    kind = RTIXCdrTypeCode_getKind(memberTc);

    while (kind == RTI_XCDR_TK_ARRAY) {
        count *= RTIXCdrTypeCode_getArrayElementCount(memberTc);
        RTIXCdrLog_testPrecondition(count == 0, return);

        memberTc = RTIXCdrTypeCode_resolveAliasUntilPointer(memberTc->_typeCode);
        kind = RTIXCdrTypeCode_getKind(memberTc);
    }

    /*
     * At this point count contains the number of flatten out elements in the
     * arrayTc's member type.
     */
    if (memberTc->_isPointer) {
        /*
         * If the array member is a reference the we calculate the sizes using
         * externalReferenceSize.
         */
        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            arrayParams->arrayElementTypeSize[i] =
                   externalReferenceSize * count;
        }

        return;
    }

    /* At this point we know that the array element is not a reference */
    if (memberTc->_sampleAccessInfo != NULL) {
        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            typeSizes[i] = memberTc->_sampleAccessInfo->typeSize[i];
        }
    } else if (RTIXCdrTypeCode_isPrimitiveKind(kind, RTI_XCDR_TRUE)) {
        RTIXCdrUnsignedLong size;

        size = RTIXCdr_TCKind_g_primitiveCdrSizes[1][kind];
        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            typeSizes[i] = size;
        }
    }

    if (!isContiguousMemoryBinding) {
        /*
         * For non-contiguous memory bindings (e.g, C) the array's
         * elementTypeSize can be calculated with a simple product because each
         * element's size is always the same regardless of the relative
         * position of the array in the sample memory buffer.
         */
        for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
            arrayParams->arrayElementTypeSize[i] =
                    typeSizes[i] * count;
        }

        return;
    }

    /* This code path should be exercised only by FlatData */

    /* 
     * Note: we are not handling strings here because only flat data should get
     * to this point and flat data does not support strings
     */

    RTIXCdrTypeCode_getFirstMemberAlignment(
            memberTc,
            &alignment,
            RTI_XCDR_TRUE);

    /**
     * Alignment could be -1 if there is an array of empty structs, but this
     * code path is only exercised by FlatData, and FlatData does not support
     * empty structs. (Codegen will fail if there is an empty struct).
     */
    RTIXCdrLog_testPrecondition(alignment <= 0, return);

    for (i = 0; i < RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES; i++) {
        /* 
         * Assume an initial alignment of i, use that index in typeSizes as the
         * size of the first member of the array. See where the next member
         * will start, and then get the size of that member (and consequently
         * all of the rest of the members in the array)
         */ 
        RTIXCdrUnsignedLong memberSizes = 0;
        RTIXCdrUnsignedLong firstMemberSize = 0;
        RTIXCdrUnsignedLong secondMemberLocation = 0;

        /* 
         * Align the first member's size up to the alignment of the next member
         * in the array
         */
        /**
         * Coverity is complaining about alignment being 0 or -1, but
         * this cannot happen, alignment will always be greater than 0.
         */
        /* coverity[overflow_const : FALSE] */
        firstMemberSize = RTIXCdrAlignment_alignSizeUp(typeSizes[i], alignment);

        /* The second member will start at (i + aligned first member size) */
        secondMemberLocation = i + firstMemberSize;

        memberSizes =
                typeSizes[secondMemberLocation % RTI_XCDR_MAX_XCDR2_ALIGNMENT];
        memberSizes = RTIXCdrAlignment_alignSizeUp(memberSizes, alignment);

        arrayParams->arrayElementTypeSize[i] = 
                firstMemberSize + (memberSizes * (count - 1));
    }
}

/*
 * Context during the generation of a program.
 *
 * The context contains parameters that are shared during the whole series of
 * function calls involved in the generation of one top-level program.
 */
typedef struct RTIXCdrSampleProgramGenerationContext {
    /*
     * The language binding for which this program is being generated.
     *
     * Note that the language binding is also part of the sampleAccessInfo, but
     * sampleAccessInfo is not always available during the program generation
     * if a nested type doesn't have it.
     */
    RTIXCdrLanguageBinding languageBinding;

    /*
     * Optional list of dependent programs
     */
    struct RTIXCdrDependentProgramList *dependentProgramList;

    /*
     * The kind of program being generated.
     */
    RTIXCdrTypeProgramKind programKind;
} RTIXCdrSampleProgramGenerationContext;

#define RTIXCdrSampleProgramGenerationContext_INITIALIZER \
{ \
    RTI_XCDR_TYPE_BINDING_INVALID, \
    NULL, \
    0 \
}

/**
 * @brief returns RTI_XCDR_TRUE if a sample with type tc can be manipulated as
 * if it was an array of octets.
 * 
 * @param[in] tc. Typecode. 
 * @param isContiguousMemoryBinding. Indicates if the memory representation
 * is contiguous. 
 * @param programKind. Program kind.
 * @return RTI_XCDR_TRUE if a sample with type tc can be manipulated as
 * if it was an array of octets. Otherwise, RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrSampleInterpreter_treatTypeAsOctetArray(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean isContiguousMemoryBinding,
        RTIXCdrTypeProgramKind programKind)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    if (isContiguousMemoryBinding) {
        return RTI_XCDR_FALSE;
    }

    if (programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM) {
        if (RTIXCdrTypeCode_isFixedSizeWithZeroDefault(tc)) {
            return RTI_XCDR_TRUE;
        }
    } else {
        if (RTIXCdrTypeCode_isFixedSize(tc)) {
            return RTI_XCDR_TRUE;
        }
    }

    return RTI_XCDR_FALSE;
}

/**
 * @brief Initializes an instruction to manipulate an array of octets.
 * 
 * @param[out] instruction. The instruction. 
 * @param[in] count. Number of octets in the array.
 * @param[in] programKind. Program kind used to determine the opcode.
 */
RTI_PRIVATE
void RTIXCdrSampleInterpreter_populateInitializeOctetArrayInstruction(
        RTIXCdrInstruction *instruction,
        RTIXCdrUnsignedLong count,
        RTIXCdrTypeProgramKind programKind)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrInsParameters *params = NULL;

    RTIXCdrLog_testPrecondition(instruction == NULL, return);

    RTIXCdrInstruction_initialize(instruction);

    if (programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM) {
        instruction->opcode = RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE;
    } else {
        instruction->opcode = RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE;
    }

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(instruction);
    commonParams->count = count;
    commonParams->memberTc = (struct RTIXCdrTypeCode *)&RTIXCdr_g_tc_octet;
    params->primitiveSampleParams.primitiveKind = RTI_XCDR_TK_OCTET;
    params->primitiveSampleParams.primitiveAlignment =
            RTI_XCDR_ONE_BYTE_ALIGNMENT;
    params->primitiveSampleParams.primitiveSize = RTI_XCDR_OCTET_SIZE;
}

RTI_PRIVATE
RTIXCdrProgram *RTIXCdrInterpreter_generateSampleProgramI(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean tcIsPointer,
        const struct RTIXCdrTypePluginProgramProperty *property,
        RTIXCdrSampleProgramGenerationContext *context);

/*
 * @brief Populate params for an instruction. This includes information like
 *        the default values, the number of members in a collection type, the
 *        size of members, etc.
 *
 * @param instruction The instruction to populate
 * @param newInstruction Whether or not a new instruction needs to be added to
 *        the program. For example, a new instruction does not need to be added
 *        to the initialize program for primitive members with a default value
 *        of 0
 * @param tc The enclosing typecode. This typecode will have kind STRUCT, VALUE,
 *        UNION, ARRAY, or SEQUENCE.
 * @param memberTc The type of the member within the enclosing type that the
 *        instruction is being generated for. It may be NULL if the
 *        generateProgram is being called on an ARRAY or a SEQUENCE
 * @param memberAccessInfoIndex The index of the member within the
 *        memberAccessInfos of the tc. This index will be different than the
 *        index of the member itself in the tc in the case of a union because
 *        index 0 is reserved for the discriminator
 * @param programKind The kind of the program that the instruction is being
 *        populated for
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrSampleInterpreter_populateInstruction(
        RTIXCdrProgram *program,
        RTIXCdrInstruction *instruction,
        RTIXCdrBoolean *newInstruction,
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean tcIsArrayOfPointers,
        const RTIXCdrTypeCode *memberTcIn,
        RTIXCdrUnsignedLong memberAccessInfoIndex,
        const struct RTIXCdrTypePluginProgramProperty *property,
        struct RTIXCdrSampleProgramGenerationContext *context)
{
    struct RTIXCdrTypeCodeMember singleMember;
    struct RTIXCdrTypeCode singleMemberTc;
    const RTIXCdrTypeCode *memberTc = NULL;
    RTIXCdrTCKind memberTcKind = RTI_XCDR_TK_NULL;
    RTIXCdrTCKind tcKind = RTI_XCDR_TK_NULL;
    RTIXCdrOctet *opCodes = NULL;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrInsParameters *params = NULL;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
    RTIXCdrBoolean logError = RTI_XCDR_TRUE;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong memberTcIndex = 0;
    RTIXCdrBoolean isMemberOptional = RTI_XCDR_FALSE;
    RTIXCdrBoolean isMemberPointer = RTI_XCDR_FALSE;
    RTIXCdrBoolean isMemberArrayOfArrays = RTI_XCDR_FALSE;
    RTIXCdrBoolean isContiguousMemoryBinding;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(memberTcIn == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(newInstruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    RTIXCdrInstruction_initialize(instruction);

    tcKind = RTIXCdrTypeCode_getKind(tc);

    /* This code path is not prepared to handle enums. */
    RTIXCdrLog_testPrecondition(
            tcKind == RTI_XCDR_TK_ENUM,
            return RTI_XCDR_FALSE);

    isContiguousMemoryBinding =
            RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                    context->languageBinding);

    if (tcKind == RTI_XCDR_TK_ARRAY) {
        if (RTIXCdrSampleInterpreter_treatTypeAsOctetArray(
                tc,
                isContiguousMemoryBinding,
                context->programKind)) {
            RTIXCdrUnsignedLongLong totalSize;
            RTIXCdrUnsignedLong elementSize;

            if (context->programKind == RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM) {
                /* No need to generate instruction for finalize/copy */
                *newInstruction = RTI_XCDR_FALSE;
                return RTI_XCDR_TRUE;
            }

            RTIXCdrLog_testPrecondition(
                    RTIXCdrTypeCode_getKind(tc->_typeCode) == RTI_XCDR_TK_STRING
                            || RTIXCdrTypeCode_getKind(tc->_typeCode)
                                    == RTI_XCDR_TK_WSTRING,
                    GotoDoneWithLine());

            /*
             * To optimize the array manipulation time, we generate a single
             * instruction to manipulate the array of fixed size
             * elements as if it was an array of octets.
             */
            RTIXCdrSampleInterpreter_calculateArraySizes(
                    &instruction->params.complexSampleParams,
                    tc,
                    memberTcIn,
                    isContiguousMemoryBinding,
                    program->externalReferenceSize);

            elementSize = instruction->params.complexSampleParams
                                  .arrayElementTypeSize[NON_FLAT_DATA_INDEX];
            totalSize = (RTIXCdrUnsignedLongLong) elementSize
                    * RTIXCdrTypeCode_getArrayElementCount(tc);

            if (totalSize > RTIXCdrUnsignedLong_MAX) {
                logMessageId =
                        RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss;
                GotoDoneWithLine();
            }

            RTIXCdrSampleInterpreter_populateInitializeOctetArrayInstruction(
                    instruction,
                    (RTIXCdrUnsignedLong) totalSize,
                    context->programKind);
            *newInstruction = RTI_XCDR_TRUE;
            return RTI_XCDR_TRUE;
        }
    }

    if (tcKind == RTI_XCDR_TK_ALIAS
            || tcKind == RTI_XCDR_TK_ARRAY
            || tcKind == RTI_XCDR_TK_SEQUENCE) {
        singleMemberTc = *tc;
        singleMemberTc._members = &singleMember;
        singleMemberTc._memberCount = 1;
        RTIXCdrInterpreter_getTcSingleMember(tc, &singleMember);
        tc = &singleMemberTc;
    }

    memberTc = (RTIXCdrTypeCode *) memberTcIn;

    if (tcKind != RTI_XCDR_TK_UNION || memberAccessInfoIndex != 0) {
        memberTcIndex = memberAccessInfoIndex;
        if (tcKind == RTI_XCDR_TK_UNION) {
            memberTcIndex = memberAccessInfoIndex - 1;
        }

        /*
         * We only resolve alias if it is not pointer alias. This could be
         * marked in two different ways: parent's member representation or
         * memberTc. We check first for the parent information, then we use
         * resolveAliasWithPointer to check for the memberTc information.
         */
        if (!tc->_members[memberTcIndex]._representation._isPointer) {
            memberTc = RTIXCdrTypeCode_resolveAliasUntilPointer(memberTcIn);
        }
    }
    memberTcKind = RTIXCdrTypeCode_getKind(memberTc);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(instruction);

    /* Populate the commonParams */
    commonParams->count = 1;
    commonParams->memberTc = (RTIXCdrTypeCode *) memberTc;

    if (tc->_sampleAccessInfo != NULL
            && (context->languageBinding
                    == RTI_XCDR_TYPE_BINDING_SQL_FILTER)
            && tc->_sampleAccessInfo->memberAccessInfos != NULL
            && tc->_sampleAccessInfo->memberAccessInfos[memberAccessInfoIndex]
                       .skipDeserialization) {
        /*
         * We don't need to initialize members that we are going to skip
         * deserializing.
         */
        *newInstruction = RTI_XCDR_FALSE;
        return RTI_XCDR_TRUE;
    }

    /* 
     * Sequence and array programs have a single instruction for the
     * type that they contain
     */
    if (tcKind == RTI_XCDR_TK_ARRAY) {
        commonParams->count = RTIXCdrTypeCode_getArrayElementCount(tc);

        RTIXCdrSampleInterpreter_calculateArraySizes(
                &params->complexSampleParams,
                tc,
                memberTc,
                isContiguousMemoryBinding,
                program->externalReferenceSize);

        isMemberPointer = tcIsArrayOfPointers;
    } else if (tcKind == RTI_XCDR_TK_SEQUENCE) {
        /* 
         * For sequences we use the memberTc for the sequence's type and we use
         * the seqElementTc for the type of the members in the sequence.
         */

        /*
         * At this point, tc is a pointer to this function's stack. We retrieve
         * the original tc through the first and only member of our single
         * member tc.
         */
        commonParams->memberTc = singleMember._representation._typeCode;
        commonParams->seqElementTc = (RTIXCdrTypeCode *) memberTc;
        isMemberPointer = tc->_isPointer;

        /* Sequences have specific opcodes */
        if (RTIXCdrTypeCode_isPrimitiveKind(memberTcKind, RTI_XCDR_TRUE)) {
            opCodes = RTIXCdrSampleInterpreter_g_primitiveSeqOpCode;
        } else if (memberTcKind == RTI_XCDR_TK_STRING
                || memberTcKind == RTI_XCDR_TK_WSTRING) {
            /*
             * TODO JLV ask if is ok to not have wstring in his own opcode
             * (as we do for type plugin programs).
             */
            opCodes = RTIXCdrSampleInterpreter_g_stringSeqOpCode;
        } else {
            opCodes = RTIXCdrSampleInterpreter_g_complexSeqOpCode;
        }

        /* Sequences always need a new instruction */
        *newInstruction = RTI_XCDR_TRUE;
    } else if (tcKind != RTI_XCDR_TK_UNION || memberAccessInfoIndex != 0) {
        /*
         * index = 0 for unions means the discriminator, which does not have a
         * tcMemberInfo
         */
        memberTcIndex = memberAccessInfoIndex;
        if (tcKind == RTI_XCDR_TK_UNION) {
            memberTcIndex = memberAccessInfoIndex - 1;
        }

        isMemberPointer =
                tc->_members[memberTcIndex]._representation._isPointer;

        /*
         * ALIAS, ARRAY, and ENUM cannot provide tcMemberInfo because the
         * member in the tc is artificial and lives on the stack. See
         * singleMemberTc above.
         *
         * Note that from those, only ALIAS can reach this code path.
         */
        if (tcKind != RTI_XCDR_TK_ALIAS) {
            /* tc->_sampleAccessInfo should only be NULL for flat data arrays */
            RTIXCdrLog_testPrecondition(
                    tc->_sampleAccessInfo == NULL,
                    GotoDoneWithLine());

            commonParams->tcMemberInfo = &tc->_members[memberTcIndex];
            commonParams->memberAccessInfo =
                    tc->_sampleAccessInfo
                            ->memberAccessInfos[memberAccessInfoIndex];
        }

        if (tcKind == RTI_XCDR_TK_STRUCT
                || tcKind == RTI_XCDR_TK_VALUE) {
            isMemberOptional =
                    RTIXCdrTypeCode_isOptionalMember(tc, memberTcIndex);
        }
    }

    /*
     * We now check if the member is an ARRAY of ARRAYs (after resolving all the
     * non-pointer aliases).
     */
    if ((memberTcKind == RTI_XCDR_TK_ARRAY)
        && (RTIXCdrTypeCode_getKind(RTIXCdrTypeCode_resolveAliasUntilPointer(
                    memberTc->_typeCode))
            == RTI_XCDR_TK_ARRAY)) {
        isMemberArrayOfArrays = RTI_XCDR_TRUE;
    }

    /*
     * If we have an ARRAY, the operation handling the array should not resolve
     * the external reference, we will do it as we process the N members within
     * the array.
     */
    if (isMemberPointer
            && ((memberTcKind != RTI_XCDR_TK_ARRAY)
                    /*
                     * There is one exception: an ARRAY of ARRAYs. In this case,
                     * the pointer qualifier means we have a pointer to an
                     * ARRAY of ARRAYs.
                     */
                    || isMemberArrayOfArrays)) {
        commonParams->refMemberKind =
                RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER;
    } else if (isMemberOptional) {
        commonParams->refMemberKind =
                RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER;
    } else {
        commonParams->refMemberKind =
                RTI_XCDR_INTERPRETER_VALUE_MEMBER;
    }

    if ((commonParams->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER
            && (context->programKind == RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM))
        || (!isContiguousMemoryBinding
            && context->programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM)) {
        /*
         * All optional members require an instruction in the
         * finalize/copy programs; in non-contiguous bindings such as the C
         * binding we need to initialize all members because a memset may not
         * be possible if the type has a non-contiguous memory layout. Note that
         * when the type is fixed-size and all its members' default values are
         * zero, we generate a single instruction that memsets the entire
         * sample.
         */
        *newInstruction = RTI_XCDR_TRUE;
    }

    commonParams->useGetMemberValue = RTIXCdrProgram_useGetMemberValueInMember(
            commonParams->memberTc,
            commonParams->refMemberKind != RTI_XCDR_INTERPRETER_VALUE_MEMBER);

    /* Populate type-specific params */
    switch (memberTcKind) {
        case RTI_XCDR_TK_SHORT:
        case RTI_XCDR_TK_LONG: 
        case RTI_XCDR_TK_USHORT:
        case RTI_XCDR_TK_ULONG:
        case RTI_XCDR_TK_FLOAT:
        case RTI_XCDR_TK_DOUBLE:
        case RTI_XCDR_TK_BOOLEAN:
        case RTI_XCDR_TK_CHAR: 
        case RTI_XCDR_TK_OCTET:
        case RTI_XCDR_TK_INT8:
        case RTI_XCDR_TK_UINT8:
        case RTI_XCDR_TK_ENUM:
        case RTI_XCDR_TK_LONGLONG:
        case RTI_XCDR_TK_ULONGLONG:
        case RTI_XCDR_TK_LONGDOUBLE:
        case RTI_XCDR_TK_WCHAR:
        {
            if (opCodes == NULL) {
                opCodes = RTIXCdrSampleInterpreter_g_primitiveOpCode;
            }

            params->primitiveSampleParams.primitiveKind = memberTcKind;

            RTIXCdrTypeCode_getPrimitiveInfo(
                    params->primitiveSampleParams.primitiveKind, 
                    RTI_XCDR_TRUE, 
                    &params->primitiveSampleParams.primitiveAlignment, 
                    &params->primitiveSampleParams.primitiveSize);

            /* 
             * Arrays can only have non-zero default values if they are arrays
             * of enums. However, in this case, there is no tcMemberInfo, so
             * we must leave the check for enums separately rather than
             * relying on hasNonZeroDefault API  
             */
            if (context->programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM
                    && ((commonParams->tcMemberInfo != NULL && 
                            RTIXCdrTypeCodeMember_hasNonZeroDefault(
                                    commonParams->tcMemberInfo)) 
                    || (memberTcKind == RTI_XCDR_TK_ENUM 
                            && memberTc->_annotations._defaultValue._u.long_value != 0)
                    || !isContiguousMemoryBinding)) {
                /*
                 * primitive members only require instructions in
                 * the finalize/copy programs if they are optional,
                 * and that has been taken care of above.
                 *
                 * The C binding is an exception and always requires an
                 * instruction per member for the initialize program, because
                 * it may contain pointers and a memset is not always valid.
                 */
                *newInstruction = RTI_XCDR_TRUE;
            }
        }
        break;
        case RTI_XCDR_TK_STRING:
        {
            if (opCodes == NULL) {
                opCodes = RTIXCdrSampleInterpreter_g_stringOpCode;
            }

            if (memberTc->_maximumLength > program->unboundedSize) {
                logMessageId = 
                        RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus;
                GotoDoneWithLine();
            }

            params->strParams.charMaxCount = memberTc->_maximumLength + 1;
            params->strParams.charSize = RTI_XCDR_ONE_BYTE_SIZE;
            params->strParams.charAlignment = RTI_XCDR_ONE_BYTE_ALIGNMENT;

            *newInstruction = RTI_XCDR_TRUE;
        } break;
        case RTI_XCDR_TK_WSTRING:
        {
            if (opCodes == NULL) {
                opCodes = RTIXCdrSampleInterpreter_g_wstringOpCode;
            }

            params->strParams.charAlignment = RTI_XCDR_WCHAR_ALIGNMENT;
            params->strParams.charSize = RTI_XCDR_WCHAR_SIZE;

            if (memberTc->_maximumLength > program->unboundedSize) {
                logMessageId = 
                        RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus;
                GotoDoneWithLine();
            }

            params->strParams.charMaxCount = memberTc->_maximumLength + 1;

            *newInstruction = RTI_XCDR_TRUE;
        } break;
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ARRAY:
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_UNION:
        case RTI_XCDR_TK_ALIAS:
        {
            if (opCodes == NULL) {
                if (memberTcKind == RTI_XCDR_TK_SEQUENCE) {
                    opCodes = RTIXCdrSampleInterpreter_g_seqOpCode;
                } else if (memberTcKind == RTI_XCDR_TK_ARRAY) {
                    opCodes = RTIXCdrSampleInterpreter_g_arrayOpCode;
                } else {
                    opCodes = RTIXCdrSampleInterpreter_g_complexOpCode;
                }
            }

            params->complexSampleParams.program =
                    RTIXCdrInterpreter_generateSampleProgramI(
                            memberTc,
                            isMemberPointer, /* tcIsPointer */
                            property,
                            context);
            if (params->complexSampleParams.program == NULL) {
                RTIXCdrLog_logTwoStr(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss,
                        RTIXCdrProgramKind_toStr(context->programKind),
                        tc->_name);
                logError = RTI_XCDR_FALSE;
                goto done;
            }

            /* Calculate arrayTypeTotalSize. This is only needed for
             * non-contiguous memory bindings (e.g. C) in order to allocate
             * optional arrays
             */
            if (RTIXCdrTypeCode_getKind(memberTc) == RTI_XCDR_TK_ARRAY
                    && context->programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM
                    && !isContiguousMemoryBinding) {
                const RTIXCdrInstruction *arrayInstruction;
                RTIXCdrUnsignedLong elementTypeSize;
                RTIXCdrUnsignedLong elementCount;
                RTIXCdrTCKind elementKind =
                        RTIXCdrTypeCode_getKind(memberTc->_typeCode);

                /*
                 * For non-contiguous bindings, generateSampleProgramI
                 * above will always generate 1 instruction for the
                 * array program. For other bindings, such as
                 * DynamicData the program may have zero instructions.
                 */
                RTIXCdrLog_testPrecondition(
                        params->complexSampleParams.program->instructionCount
                                != 1,
                        goto done);

                arrayInstruction =
                        &params->complexSampleParams.program->instructions[0];


                if (elementKind == DDS_TK_STRING
                        || elementKind == DDS_TK_WSTRING) {
                    if (memberTc->_typeCode->_sampleAccessInfo != NULL) {
                        elementTypeSize = memberTc->_typeCode->_sampleAccessInfo
                                                  ->typeSize[0];
                    } else {
                        elementTypeSize = sizeof(char *);
                    }
                } else {
                    elementTypeSize =
                            arrayInstruction->params.complexSampleParams
                                    .arrayElementTypeSize[0];
                }

                elementCount =
                        ((const RTIXCdrCommonInsParameters *) &arrayInstruction
                                        ->params)
                                ->count;
                params->complexSampleParams.arrayTypeTotalSize =
                        elementCount * elementTypeSize;
            }

            *newInstruction = RTI_XCDR_TRUE;
        } break;
        default:
            GotoDoneWithLine();
    }
    
    result = RTIXCdrSampleInterpreter_generateInstructionOpcode(
            instruction,
            opCodes,
            context->programKind);
    if (!result) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "stream program");
        logError = RTI_XCDR_FALSE;
        goto done;
    }

    ok = RTI_XCDR_TRUE;
done:
    if (!ok) {
        if (logError) {
            RTIXCdrInterpreter_logProgramGenerationError(
                    tc,
                    instruction,
                    context->programKind,
                    logMessageId,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return ok;
}

RTI_PRIVATE
RTIXCdrProgram *RTIXCdrInterpreter_generateSampleProgramI(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean tcIsPointer,
        const struct RTIXCdrTypePluginProgramProperty *property,
        RTIXCdrSampleProgramGenerationContext *context)
{
    RTIXCdrProgram *program = NULL;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrBoolean reallocateArray = RTI_XCDR_FALSE;
    RTIXCdrBoolean loggedError = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrBoolean newInstruction = RTI_XCDR_FALSE;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
    RTIXCdrUnsignedLong arrayCount = 0;
    const RTIXCdrTypeCode *unaliasedTc = NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);
    RTIXCdrLog_testPrecondition(context == NULL, return NULL);
    RTIXCdrLog_testPrecondition(property == NULL, return NULL);

    /*
     * We only resolve alias if it is not pointer alias. This could be
     * marked in two different ways: parent's member representation or
     * memberTc. We check first for the parent information, then we use
     * resolveAliasWithPointer to check for the memberTc information.
     */
    if (!tcIsPointer) {
        unaliasedTc = RTIXCdrTypeCode_resolveAliasUntilPointer(tc);
    } else {
        unaliasedTc = tc;
    }

    if (tc->_sampleAccessInfo != NULL
        && tc->_sampleAccessInfo->languageBinding != RTI_XCDR_TYPE_BINDING_C
        && !RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding)
        && tc->_sampleAccessInfo->languageBinding
                != RTI_XCDR_TYPE_BINDING_DYN_DATA
        && tc->_sampleAccessInfo->languageBinding
                != RTI_XCDR_TYPE_BINDING_SQL_FILTER) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "the sample programs are only supported for C, flat data, "
                "dynamic data, and sql filter language bindings");
        loggedError = RTI_XCDR_TRUE;
        goto done;
    }

    /* 
     * First we look for the program to be created in the dependentProgramList
     * and if its there we return it
     */
    if (context->dependentProgramList != NULL) {
        program = RTIXCdrDependentProgramList_findProgram(
                context->dependentProgramList,
                unaliasedTc,
                context->programKind);
        if (program != NULL) {
            return program;
        }
    }

    program = RTIXCdrInterpreter_newProgram(
            unaliasedTc,
            &context->dependentProgramList,
            context->programKind,
            property);
    if (program == NULL) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                "program", 
                unaliasedTc->_name);
        loggedError = RTI_XCDR_TRUE;
        goto done;
    }

    RTIXCdrTypeCode_getFirstMemberAlignment(
            unaliasedTc,
            &program->firstMemberAlignment,
            RTI_XCDR_TRUE);
    if (program->firstMemberAlignment == -1) {
        /* If the alignment is unknown because the type is empty
         * we set it to 8 bytes because zero is not a valid alignment.
         */
        program->firstMemberAlignment = 8;
    }
    
    switch (RTIXCdrTypeCode_getKind(unaliasedTc)) {
    case RTI_XCDR_TK_VALUE:
    case RTI_XCDR_TK_STRUCT:
    {
        /* 
         * For VALUE types, flatten out the base types and have a
         * program instruction per member in each of the
         * base + derived types combined
         */
        RTIXCdrUnsignedLong i = 0;
        RTIXCdrUnsignedLong instructionCount = 0;
        RTIXCdrUnsignedLong memberCount = 0;
        RTIXCdrBoolean isContiguousMemoryBinding =
            RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
                    context->languageBinding);

        if (RTIXCdrSampleInterpreter_treatTypeAsOctetArray(
                unaliasedTc,
                isContiguousMemoryBinding,
                context->programKind)) {

            if (context->programKind == RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM) {
                /* No need to generate instruction for finalize/copy */
                break;
            }
            /*
             * To optimize sample manipulation time,
             * we generate a single instruction to process a fixed size
             * structure type as if it was an array of octets.
             */
            RTIXCdrHeap_allocateArray(
                    &program->instructions,
                    1,
                    RTIXCdrInstruction);
            if (program->instructions == NULL) {
                arrayCount = memberCount;
                GotoDoneWithLine();
            }

            RTIXCdrSampleInterpreter_populateInitializeOctetArrayInstruction(
                    &program->instructions[instructionCount],
                    unaliasedTc->_sampleAccessInfo
                            ->typeSize[NON_FLAT_DATA_INDEX],
                    context->programKind);
            program->instructionCount = 1;
            break;
        }

        /* This function takes into account base types */
        RTIXCdrTypeCode_getMemberCount(&memberCount, unaliasedTc);

        RTIXCdrHeap_allocateArray(
                &program->instructions,
                memberCount,
                RTIXCdrInstruction);
        if (program->instructions == NULL) {
            arrayCount = memberCount;
            GotoDoneWithLine();
        }

        for (i = 0; i < memberCount;) {
            RTIXCdrUnsignedLong k = 0;
            const RTIXCdrTypeCode *typeCode = NULL;

            RTIXCdrTypeCode_getTypeContainingIndex(
                    &typeCode, 
                    unaliasedTc, 
                    memberCount, 
                    i);
            if (typeCode == NULL) {
                GotoDoneWithLine();
            }

            for (k = 0; k < typeCode->_memberCount; k++) {
                newInstruction = RTI_XCDR_FALSE;

                if (!RTIXCdrSampleInterpreter_populateInstruction(
                        program,
                        &program->instructions[instructionCount],
                        &newInstruction,
                        typeCode,
                        RTI_FALSE, /* tcIsArrayOfPointers */
                        typeCode->_members[k]._representation._typeCode,
                        k,
                        property,
                        context)) {
                    GotoDoneWithLine();
                }

                if (newInstruction) {
                    instructionCount++;
                }

                i++;
            }
        }

        /* The initialize program for C samples must have one instruction per
         * member, and given an index i, instruction[i] corresponds to member[i]
         */
        RTIXCdrLog_testPrecondition(
                tc->_sampleAccessInfo->languageBinding
                                == RTI_XCDR_TYPE_BINDING_C
                        && context->programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM
                        && instructionCount != memberCount,
                GotoDoneWithLine());


        program->instructionCount = instructionCount;
        if (instructionCount != memberCount) {
            reallocateArray = RTI_XCDR_TRUE;
        }
    } break;
    case RTI_XCDR_TK_UNION:
    {
        RTIXCdrUnsignedLong i = 0;
        RTIXCdrLong discValue = 0;
        RTIXCdrLong discIndex = 0;
        RTIXCdrUnsignedLong instructionCount = 0;
        RTIXCdrBoolean specComplianceDiscriminator =
                RTIXCdrXTypesComplianceMask_isBitSet(
                        RTIXCdrInterpreter_getGlobalXtypeComplianceMask(),
                        RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT);

        /* 
         * A union needs an operation for
         * (1 (the discriminator) + memberCount)
         */
        instructionCount = unaliasedTc->_memberCount + 1;

        RTIXCdrHeap_allocateArray(
                &program->instructions,
                instructionCount,
                RTIXCdrInstruction);
        if (program->instructions == NULL) {
            arrayCount = instructionCount;
            GotoDoneWithLine();
        }

        /*
         * Reset instructionCount so it can keep track of how many
         * instructions are actually needed for this type
         */
        instructionCount = 0;
        program->unionDefaultCaseInsIndex = RTI_XCDR_TYPECODE_INVALID_INDEX;

        /* Generate the instruction for the discriminator */
        if (!RTIXCdrSampleInterpreter_populateInstruction(
                program,
                &program->instructions[instructionCount],
                &newInstruction, 
                unaliasedTc,
                RTI_FALSE, /* tcIsArrayOfPointers */
                RTIXCdrTypeCode_resolveAlias(unaliasedTc->_typeCode),
                0, 
                property,
                context)) {
            GotoDoneWithLine();
        }

        /*
         * The first instruction for a union, the discriminator instruction
         * that was added above, is always kept regardless of if they are
         * zeroes.
         */
        instructionCount++;

        /* Find the default discriminator for the union */
        RTIXCdrTypeCode_selectDefaultDiscriminator(
                unaliasedTc,
                &discValue,
                specComplianceDiscriminator);

        RTIXCdrTypeCode_getUnionMemberIndex(
                (RTIXCdrUnsignedLong *) &discIndex,
                unaliasedTc,
                discValue);

        program->unionDiscKind = RTIXCdrTypeCode_getKind(
                unaliasedTc->_typeCode);
        program->defaultUnionDisc = discValue;

        if (unaliasedTc->_default_index != -1) {
            program->defaultCaseDisc = discValue;

            /* We also need to get a valid discriminator value for the default
             * case for use in DynamicData. The DynamicData API allows a user
             * to set a member by name without providing a value to set the
             * discriminator to. In that case, if they set the default case,
             * we need to have a value to set the discriminator to. We only need
             * to get this separate value we when all these conditions are met:
             * 1. RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT is
             *    set.
             * 2. The default discriminator value does not fall within the
             *    default case. In that case defaultCaseDisc is discValue.
             */
            if (specComplianceDiscriminator
                && discIndex != unaliasedTc->_default_index) {
                RTIXCdrTypeCode_selectDefaultDiscriminator(
                        unaliasedTc,
                        &program->defaultCaseDisc,
                        RTI_FALSE);
            }
        } else {
            program->defaultCaseDisc = (RTIXCdrLong) RTI_XCDR_TYPECODE_INVALID_INDEX;
        }

        /*
         * If the default discriminator value selects a member in the union
         * (discIndex != -1), add the instruction for that member as the
         * second instruction.
         */
        if (discIndex != -1) {
            if (!RTIXCdrSampleInterpreter_populateInstruction(
                        program,
                        &program->instructions[instructionCount],
                        &newInstruction,
                        unaliasedTc,
                        RTI_FALSE, /* tcIsArrayOfPointers */
                        unaliasedTc->_members[discIndex]
                                ._representation._typeCode,
                        (RTIXCdrUnsignedLong) discIndex + 1,
                        property,
                        context)) {
                GotoDoneWithLine();
            }

            /*
             * The second instruction is also always kept but only for the
             * initialize program. This helps us to avoid having to recalculate
             * the default discriminator and index during initialization.
             *
             * There is one exception: that we are in SQL FILTER language
             * binding, and that we are skipping that member for
             * deserialization: in that case, we don't need the initialization
             * instruction for that member.
             */
            if (newInstruction
                || ((context->programKind == RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM)
                    && (tc->_sampleAccessInfo->languageBinding
                        != RTI_XCDR_TYPE_BINDING_SQL_FILTER))) {
                program->unionInsIndex = instructionCount;

                if (discIndex == unaliasedTc->_default_index) {
                    program->unionDefaultCaseInsIndex = instructionCount;
                }

                instructionCount++;
            } else {
                /*
                 * If there is no instruction for the discriminator, initialize
                 * the discInsIndex to a value that will prevent any
                 * instructions getting executed
                 */
                program->unionInsIndex = RTI_XCDR_TYPECODE_INVALID_INDEX;
            }
        } else {
            /**
             * The default discriminator does not match any of the members, so
             * we set the unionInsIndex to an invalid value. This will prevent
             * any memory allocation for a supposed default member in the Sample
             * program execution.
             */
            program->unionInsIndex = RTI_XCDR_TYPECODE_INVALID_INDEX;
        }

        /* Generate the instructions for each union member */
        for (i = 0; i < unaliasedTc->_memberCount; i++) {
            /* discIndex >= 0 at this point, so it is ok to cast to unsigned */
            if (i == (RTIXCdrUnsignedLong)discIndex) {
                /* There was already an instruction generated for this index */
                continue;
            }

            newInstruction = RTI_XCDR_FALSE;

            if (!RTIXCdrSampleInterpreter_populateInstruction(
                    program,
                    &program->instructions[instructionCount],
                    &newInstruction,
                    unaliasedTc,
                    RTI_FALSE, /* tcIsArrayOfPointers */
                    unaliasedTc->_members[i]._representation._typeCode,
                    i + 1,
                    property,
                    context)) {
                GotoDoneWithLine();
            }

            if (newInstruction) {
                if ((RTIXCdrLong) i == unaliasedTc->_default_index) {
                    program->unionDefaultCaseInsIndex = instructionCount;
                }

                instructionCount++;
            }
        }

        if (instructionCount != (unaliasedTc->_memberCount + 1)) {
            reallocateArray = RTI_XCDR_TRUE;
        }

        program->instructionCount = instructionCount;
    } break;
    case RTI_XCDR_TK_ALIAS:
    case RTI_XCDR_TK_SEQUENCE:
    case RTI_XCDR_TK_ARRAY:
    {
        const RTIXCdrTypeCode *unaliasedMemberTc = NULL;
        RTIXCdrBoolean tcIsArrayOfPointers = RTI_XCDR_FALSE;

        /* 
         * Sequences/arrays have one instruction encoding how to handle the
         * type of their members. 
         */
        program->instructionCount = 1;
        RTIXCdrHeap_allocateArray(
                &program->instructions,
                program->instructionCount,
                RTIXCdrInstruction);
        if (program->instructions == NULL) {
            arrayCount = program->instructionCount;
            GotoDoneWithLine();
        }

        unaliasedMemberTc =
                RTIXCdrTypeCode_resolveAliasUntilPointer(unaliasedTc->_typeCode);

        if ((RTIXCdrTypeCode_getKind(unaliasedTc) == RTI_XCDR_TK_ARRAY)
                && (RTIXCdrTypeCode_getKind(unaliasedMemberTc)
                        != RTI_XCDR_TK_ARRAY)) {
            tcIsArrayOfPointers = tcIsPointer;
        }

        if (!RTIXCdrSampleInterpreter_populateInstruction(
                program,
                &program->instructions[0],
                &newInstruction,
                unaliasedTc,
                tcIsArrayOfPointers, /* tcIsArrayOfPointers */
                unaliasedMemberTc,
                0,
                property,
                context)) {
            GotoDoneWithLine();
        }

        if (!newInstruction) {
            program->instructionCount = 0;
        }
    } break;
    default:
        GotoDoneWithLine();
    }

    if (program->instructionCount == 0) {
        /* 
         * If there are no instructions then the instruction array can be deleted.
         * The program will still be returned because the programs still need to
         * do some processing regardless of whether there are instrcutions for
         * individual members. For example, in initialize and finalize, the sample
         * must be set to 0
         */
        RTIXCdrHeap_freeArray(program->instructions);
        program->instructions = NULL;
    } else if (reallocateArray) {
        /* 
         * In many cases, the number of instructions is far fewer than the
         * number of members in the type. For this reason, we reallocate the
         * array to the exact size so as not to waste so much memory
         */
        RTIXCdrInstruction *tmpArray = NULL;
        RTIXCdrUnsignedLong j = 0;

        RTIXCdrHeap_allocateArray(
                &tmpArray,
                program->instructionCount,
                RTIXCdrInstruction);
        if (tmpArray == NULL) {
            arrayCount = program->instructionCount;
            /* 
             * If this allocation fails we can still continue. The instructions
             * have already been generated successfully. Therefore set ok to
             * true
             */
            ok = RTI_XCDR_TRUE;
            GotoDoneWithLine();
        }

        /* 
         * Shallow copy to the smaller array and the deallocate the larger
         * array and replace with the new, smaller array
         */
        for (j = 0; j < program->instructionCount; j++) {
            tmpArray[j] = program->instructions[j];
        }

        RTIXCdrHeap_freeArray(program->instructions);
        program->instructions = tmpArray;
    }

    ok = RTI_XCDR_TRUE;
done:

    if (arrayCount != 0) {
        RTIXCdrLogParam param[2];
        /* Initializing param completely to avoid Coverity false positives */
        RTIXCdrMemory_zero(param, sizeof(RTIXCdrLogParam) * 2);

        param[0].kind = RTI_XCDR_LOG_LONG_PARAM;
        param[0].value.lVal = (RTIXCdrLong)arrayCount;
        param[1].kind = RTI_XCDR_LOG_LONG_PARAM;
        param[1].value.lVal = sizeof(RTIXCdrInstruction);

        RTIXCdrLog_logWithParams(
                __FILE__,
                RTI_FUNCTION_NAME,
                __LINE__,
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                2,
                param);
    }

    if (!ok) {
         if (!loggedError) {
            RTIXCdrInterpreter_logProgramGenerationError(
                    unaliasedTc,
                    NULL,
                    context->programKind,
                    logMessageId,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }

        if (program != NULL) {
            RTIXCdrInterpreter_deleteProgram(program);
            program = NULL;
        }
    }

    return program;
}

/*
 * @brief Given a typecode, generate a program that will act on a user-level
 *        sample of that type.
 *
 * @param tc The typecode describing the type of the sample for which to
 *        generate the program
 * @param dependentProgramList \b InOut. List of dependent programs. If this
 *        parameter is NULL the program will generate a new list that will be used
 *        to store dependent programs.
 * @param programKind The kind of the program to generate. Currently support
 *        programs:
 *        - initialize
 *        - allocated members (i.e. finalize/copy)
 * @param property Properties that may alter how a given programKind is
 *        generated
 */
RTIXCdrProgram *RTIXCdrInterpreter_generateSampleProgram(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList *dependentProgramList,
        RTIXCdrTypeProgramKind programKind,
        const struct RTIXCdrTypePluginProgramProperty *property)
{
    RTIXCdrSampleProgramGenerationContext context =
            RTIXCdrSampleProgramGenerationContext_INITIALIZER;

    RTIXCdrLog_testPrecondition(tc->_sampleAccessInfo == NULL, return NULL);

    context.dependentProgramList = dependentProgramList;
    context.programKind = programKind;
    context.languageBinding = tc->_sampleAccessInfo->languageBinding;

    return RTIXCdrInterpreter_generateSampleProgramI(
            tc,
            tc->_isPointer,
            property,
            &context);
}
