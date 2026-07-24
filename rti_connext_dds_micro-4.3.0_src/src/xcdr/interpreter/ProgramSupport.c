/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "../infrastructure/Infrastructure.h"
#include "../infrastructure/InlineList.h"
#include "../infrastructure/SkipList.h"
#include "../typeCode/TypeCode.h"
#include "../stream/Stream.h"
#include "ProgramSupport.h"
#include "InstructionIndex.h"

/*
 * @brief Data structure used to store a list of dependent programs for a top
 * level TypeCode.
 *
 * A top level program generated from a top level TypeCode can contain
 * complex instructions that reference other programs.
 *
 * A RTIXCdrDependentProgramList contains the list of programs that can be
 * referenced by a top level program.
 *
 * The sole purpose of the list is to avoid the creation of multiple
 * programs for the same type.
 * 
 * The dependent programs can be stored in an underlying InlineList or SkipList
 * depending on the number of expected elements in RTIXCdrDependentProgramList.
 * 
 * This decision is made at creation time based on a threshold.
 * 
 * @see RTIXCdrDependentProgramList_new
 */
struct RTIXCdrDependentProgramList {
    struct RTIXCdrInlineList *inlineList;
    struct RTIXCdrSkipList *skipList;
};

/* 
* The following global variables configure aspects of the program generation
* that are global to the process.
*/

/*
 * CORE-10578
 * @brief Allow the user to indicate the unbounded support value by using the
 * function RTIXCdrInterpreter_setUnboundedSize.
 */
RTIXCdrUnsignedLong RTIXCdrInterpreter_g_unboundedSize = RTIXCdrLong_MAX;

void RTIXCdrInterpreter_setUnboundedSize(RTIXCdrUnsignedLong unboundedSize) {
    RTIXCdrInterpreter_g_unboundedSize = unboundedSize;
}

RTIXCdrUnsignedLong RTIXCdrInterpreter_getUnboundedSize(void) {
    return RTIXCdrInterpreter_g_unboundedSize;
}

void RTIXCdrInstruction_initialize(RTIXCdrInstruction * me) {
    RTIXCdrLog_testPrecondition(me == NULL, return);

    RTIXCdrMemory_zero(
            &me->params,
            sizeof(RTIXCdrInsParameters));
}

const char * RTIXCdrInstruction_opCodeToStr(RTIXCdrInstruction * me) {
    RTIXCdrLog_testPrecondition(me == NULL, return NULL);

    switch(me->opcode) {
    
    case RTI_XCDR_SER_PRIMITIVE_OPCODE:
        return "SER_PRIMITIVE";
    case RTI_XCDR_DESER_PRIMITIVE_OPCODE:
        return "DESER_PRIMITIVE";
    case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        return "SKIP_PRIMITIVE";
    case RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE:
        return "INITIALIZE_PRIMITIVE";
    case RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE:
        return "ALLOCATED_PRIMITIVE";

    case RTI_XCDR_SER_STRING_OPCODE:
        return "SER_STRING";
    case RTI_XCDR_DESER_STRING_OPCODE:
        return "DESER_STRING";
    case RTI_XCDR_SKIP_STRING_OPCODE:
        return "SKIP_STRING";
    case RTI_XCDR_INITIALIZE_STRING_OPCODE:
        return "INITIALIZE_STRING";
    case RTI_XCDR_ALLOCATED_STRING_OPCODE:
        return "ALLOCATED_STRING";

    case RTI_XCDR_SER_WSTRING_OPCODE:
        return "SER_WSTRING";
    case RTI_XCDR_DESER_WSTRING_OPCODE:
        return "DESER_WSTRING";
    case RTI_XCDR_SKIP_WSTRING_OPCODE:
        return "SKIP_WSTRING";
    case RTI_XCDR_INITIALIZE_WSTRING_OPCODE:
        return "INITIALIZE_WSTRING";
    case RTI_XCDR_ALLOCATED_WSTRING_OPCODE:
        return "ALLOCATED_WSTRING";
    
    case RTI_XCDR_SER_COMPLEX_OPCODE:
        return "SER_COMPLEX";        
    case RTI_XCDR_DESER_COMPLEX_OPCODE:
        return "DESER_COMPLEX";
    case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        return "SKIP_COMPLEX";
    case RTI_XCDR_INITIALIZE_COMPLEX_OPCODE:
        return "INITIALIZE_COMPLEX";
    case RTI_XCDR_ALLOCATED_COMPLEX_OPCODE:
        return "ALLOCATED_COMPLEX";
        
    case RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE:
        return "SER_PRIMITIVE_SEQ";
    case RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE:
        return "DESER_PRIMITIVE_SEQ";
    case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        return "SKIP_PRIMITIVE_SEQ";
    case RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE:
        return "INITIALIZE_PRIMITIVE_SEQ";
    case RTI_XCDR_ALLOCATED_PRIMITIVE_SEQ_OPCODE:
        return "ALLOCATED_PRIMITIVE_SEQ";
        
    case RTI_XCDR_SER_STRING_SEQ_OPCODE:
        return "SER_STRING_SEQ";
    case RTI_XCDR_DESER_STRING_SEQ_OPCODE:
        return "DESER_STRING_SEQ";
    case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        return "SKIP_STRING_SEQ";
    case RTI_XCDR_INITIALIZE_STRING_SEQ_OPCODE:
        return "INITIALIZE_STRING_SEQ";
    case RTI_XCDR_ALLOCATED_STRING_SEQ_OPCODE:
        return "ALLOCATED_STRING_SEQ";

    case RTI_XCDR_SER_WSTRING_SEQ_OPCODE:
        return "SER_WSTRING_SEQ";
    case RTI_XCDR_DESER_WSTRING_SEQ_OPCODE:
        return "DESER_WSTRING_SEQ";
    case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        return "SKIP_WSTRING_SEQ";
    case RTI_XCDR_INITIALIZE_WSTRING_SEQ_OPCODE:
        return "INITIALIZE_WSTRING_SEQ";
    case RTI_XCDR_ALLOCATED_WSTRING_SEQ_OPCODE:
        return "ALLOCATED_WSTRING_SEQ";
    
    case RTI_XCDR_SER_COMPLEX_SEQ_OPCODE:
        return "SER_COMPLEX_SEQ";
    case RTI_XCDR_DESER_COMPLEX_SEQ_OPCODE:
        return "DESER_COMPLEX_SEQ";
    case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        return "SKIP_COMPLEX_SEQ";
    case RTI_XCDR_INITIALIZE_COMPLEX_SEQ_OPCODE:
        return "INITIALIZE_COMPLEX_SEQ";
    case RTI_XCDR_ALLOCATED_COMPLEX_SEQ_OPCODE:
        return "ALLOCATED_COMPLEX_SEQ";

    case RTI_XCDR_SER_DHEADER_OPCODE:
        return "SER_DHEADER";
    case RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE:
        return "SER_COLLECTION_DHEADER";
    case RTI_XCDR_DESER_DHEADER_OPCODE:
        return "DESER_DHEADER";
    case RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE:
        return "DESER_COLLECTION_DHEADER";
    case RTI_XCDR_SKIP_DHEADER_OPCODE:
        return "SKIP_DHEADER";    
    case RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE:
        return "SKIP_COLLECTION_DHEADER";    

    case RTI_XCDR_SER_SENTINEL_HEADER_OPCODE:
        return "SER_SENTINEL_HEADER";
    case RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE:
        return "SKIP_SENTINEL_HEADER";
                
    case RTI_XCDR_SER_MEMBER_HEADER_OPCODE:
        return "SER_MEMBER_HEADER";
    case RTI_XCDR_DESER_MEMBER_HEADER_OPCODE:
        return "DESER_MEMBER_HEADER";
    case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        return "SKIP_MEMBER_HEADER";

    case RTI_XCDR_INITIALIZE_SEQ_OPCODE:
        return "INITIALIZE_SEQ";        
    case RTI_XCDR_ALLOCATED_SEQ_OPCODE:
        return "ALLOCATED_SEQ";    

    case RTI_XCDR_INITIALIZE_ARRAY_OPCODE:
        return "INITIALIZE_ARRAY";        
    case RTI_XCDR_ALLOCATED_ARRAY_OPCODE:
        return "ALLOCATED_ARRAY";    
    default:
        return "INVALID";
    }
}

/* Micro does not support printf */
#if defined(RTI_XCDR_HAVE_PRINTF)
RTI_PRIVATE
void RTIXCdrProgram_printIndent(RTIXCdrUnsignedLong count) {
    RTIXCdrUnsignedLong indentCount = 0;
    for (indentCount=0; indentCount<count; indentCount++) {
        printf("\t");
    } 
}

void RTIXCdrProgram_print(RTIXCdrProgram *me, const char * title) {
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrUnsignedLong indent = 0;

    RTIXCdrLog_testPrecondition(me == NULL, return);

    if (title != NULL) {
        printf("%s:\n", title);
        indent++;
    }

    if (me->instructions == NULL) {
        return;
    }

    for (i=0; i<me->instructionCount; i++) {
        RTIXCdrInstruction *instruction = NULL;
        RTIXCdrCommonInsParameters *commonParams = NULL;

        instruction = &me->instructions[i];
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        RTIXCdrProgram_printIndent(indent);
        printf("%s:\n", RTIXCdrInstruction_opCodeToStr(instruction));

        if (RTIXCdrInstruction_isHeaderOpcode(instruction->opcode)) {
            continue;
        }

        RTIXCdrProgram_printIndent(indent);
        printf("\tMemberAccessInfo: %u %d\n",
                commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX],
                commonParams->memberAccessInfo.skipDeserialization);

        if (commonParams->memberTc != NULL &&
                commonParams->memberTc->_sampleAccessInfo != NULL) {
            RTIXCdrProgram_printIndent(indent);
            printf("\tMemberSampleAccessInfo: %d %u\n",
                    commonParams->memberTc->_sampleAccessInfo->languageBinding,
                    commonParams->memberTc->_sampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX]);
        }

        RTIXCdrProgram_printIndent(indent);
        printf("\tArrayElementCount: %u\n", commonParams->count);

        if (commonParams->seqElementTc != NULL &&
                commonParams->seqElementTc->_sampleAccessInfo != NULL) {
            RTIXCdrProgram_printIndent(indent);
            printf("\tSeqElementSampleAccessInfo: %d %u\n",
                    commonParams->seqElementTc->_sampleAccessInfo->languageBinding,
                    commonParams->seqElementTc->_sampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX]);
        }

        if (RTIXCdrInstruction_isStringOpcode(instruction->opcode)) {
            RTIXCdrProgram_printIndent(indent);
            printf("\tCharSize: %d\n",
                    (RTIXCdrLong) instruction->params.strParams.charSize);
            RTIXCdrProgram_printIndent(indent);
            printf("\tCharAlignment: %d\n",
                    (RTIXCdrLong) instruction->params.strParams.charAlignment);
            RTIXCdrProgram_printIndent(indent);
            printf("\tCharMaxCount: %u\n",
                    (RTIXCdrLong) instruction->params.strParams.charMaxCount);
        }

        if (RTIXCdrInstruction_isPrimitiveOpcode(instruction->opcode)) {
            RTIXCdrProgram_printIndent(indent);
            printf("\tPrimitiveSize: %d\n",
                    (RTIXCdrLong) instruction->params.primitiveParams.primitiveSize);
            RTIXCdrProgram_printIndent(indent);
            printf("\tOrigPrimitiveSize: %d\n",
                    (RTIXCdrLong) instruction->params.primitiveParams.origPrimitiveSize);
            RTIXCdrProgram_printIndent(indent);
            printf("\tPrimitiveAlignment: %d\n",
                    (RTIXCdrLong) instruction->params.primitiveParams.primitiveAlignment);
            RTIXCdrProgram_printIndent(indent);
            printf("\tMustAlign: %d\n",
                    (RTIXCdrLong) instruction->params.primitiveParams.mustAlign);
            RTIXCdrProgram_printIndent(indent);
            printf("\tCheckEnum: %d\n",
                    (RTIXCdrLong) (instruction->params.primitiveParams.enumTc != NULL));
        }
    }
}
#endif

int RTIXCdrInterpreterProgramsGenProperty_compare(
        const RTIXCdrInterpreterProgramsGenProperty *left,
        const RTIXCdrInterpreterProgramsGenProperty *right)
{
    int result = 0;

    result = left->generateV1Encapsulation - right->generateV1Encapsulation;
    if (result != 0) {
        return result;
    }

    result = left->generateV2Encapsulation - right->generateV2Encapsulation;
    if (result != 0) {
        return result;
    }
    result = left->generateLittleEndian - right->generateLittleEndian;
    if (result != 0) {
        return result;
    }
    result = left->generateBigEndian - right->generateBigEndian;
    if (result != 0) {
        return result;
    }
    result = left->generateWithAllFields - right->generateWithAllFields;
    if (result != 0) {
        return result;
    }
    result = left->generateWithOnlyKeyFields - right->generateWithOnlyKeyFields;
    if (result != 0) {
        return result;
    }
    result = left->resolveAlias - right->resolveAlias;
    if (result != 0) {
        return result;
    }
    result = left->optimizeEnum - right->optimizeEnum;
    if (result != 0) {
        return result;
    }
    result = left->inlineStruct - right->inlineStruct;
    if (result != 0) {
        return result;
    }
    result = left->inlineSequence - right->inlineSequence;
    if (result != 0) {
        return result;
    }
    result = left->forceDependentPrograms - right->forceDependentPrograms;
    if (result != 0) {
        return result;
    }
    result = RTIXCdrUtility_compareNumericValue(
            left->externalReferenceSize,
            right->externalReferenceSize);
    if (result != 0) {
        return result;
    }

    result = RTIXCdrUtility_compareFunctionPointer(
            left->getExternalRefPointerFcn,
            right->getExternalRefPointerFcn);
    if (result != 0) {
        return result;
    }

    result = (int) (left->xTypesComplianceMask - right->xTypesComplianceMask);
    if (result != 0) {
        return result;
    }

    return result;
}

const char * RTIXCdrProgramKind_toStr(
        RTIXCdrTypeProgramKind programKind)
{
    switch (programKind) {
    case RTI_XCDR_SER_PROGRAM:
        return "serialize";
    case RTI_XCDR_DESER_PROGRAM:
        return "deserialize";
    case RTI_XCDR_SKIP_PROGRAM:
        return "skip";
    case RTI_XCDR_GET_SER_SIZE_PROGRAM:
        return "get_serialized_size";
    case RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM:
        return "get_max_serialized_size";
    case RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM:
        return "get_min_serialized_size";
    case RTI_XCDR_SER_TO_KEY_PROGRAM:
        return "serialized_sample_to_key";
    case RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM:
        return "initialize_sample";
    case RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM:
        return "allocated_members_sample";        
    default:
        return "unknown";
    }
}

RTIXCdrBoolean RTIXCdrInterpreter_primitiveToLong(
        RTIXCdrLong *lValue,
        const RTIXCdrMemberValue *value,
        RTIXCdrBoolean useMemberIndexForAccessDiscValue,
        RTIXCdrTCKind primitiveKind)
{
    RTIXCdrBoolean failure;

    RTIXCdrLog_testPrecondition(lValue == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(value == NULL, return RTI_XCDR_FALSE);

    if (!useMemberIndexForAccessDiscValue) {
        RTIXCdrInterpreter_primitiveToLongMacro(
                *lValue,
                *value,
                primitiveKind,
                failure);
    } else {
        RTIXCdrInterpreter_primitiveToLongWIndexMacro(
                *lValue,
                *value,
                primitiveKind,
                failure);
    }

    return !failure;
}

RTIXCdrBoolean RTIXCdrInterpreter_longToPrimitive(
        RTIXCdrMemberValue *value,
        RTIXCdrLong lValue,
        RTIXCdrBoolean useMemberIndexForAccessDiscValue,
        RTIXCdrTCKind primitiveKind)
{
    RTIXCdrBoolean failure;

    RTIXCdrLog_testPrecondition(value == NULL, return RTI_XCDR_FALSE);

    if (!useMemberIndexForAccessDiscValue) {
        RTIXCdrInterpreter_longToPrimitiveMacro(
                *value,
                lValue,
                primitiveKind,
                failure);
    } else {
        RTIXCdrInterpreter_longToPrimitiveWIndexMacro(
                *value,
                lValue,
                primitiveKind,
                failure);
    }

    return !failure;
}

void RTIXCdrProgram_deleteInstructions(RTIXCdrProgram *program) {
    RTIXCdrLog_testPrecondition(program == NULL, return);

    if (program->instructionIndex != NULL) {
        RTIXCdrInstructionIndex_delete(
                program->instructionIndex);
        program->instructionIndex = NULL;
    }

    if (program->instructionIndexByLabel != NULL) {
        RTIXCdrInstructionIndex_delete(
                program->instructionIndexByLabel);
        program->instructionIndexByLabel = NULL;
    }

    if (program->instructions != NULL) {
        RTIXCdrHeap_freeArray(program->instructions);
        program->instructions = NULL;
    }
}

RTI_PRIVATE
void RTIXCdrInterpreter_deleteProgramWoDependencies(RTIXCdrProgram *program)
{
    RTIXCdrLog_testPrecondition(program == NULL, return);

    RTIXCdrProgram_deleteInstructions(program);
    RTIXCdrHeap_freeStruct(program);

}

void RTIXCdrInterpreter_deleteProgram(RTIXCdrProgram *program)
{
    RTIXCdrLog_testPrecondition(program == NULL, return);

    if (program->dependentProgramList != NULL &&
            program->listOwner) {
        /* 
         * Delete the dependentProgramList takes care of the program itself
         * as well because it is the first element of the list
         */
        RTIXCdrDependentProgramList_delete(program->dependentProgramList);
    } else if (program->dependentProgramList == NULL) {
        RTIXCdrInterpreter_deleteProgramWoDependencies(
                program);
    }
}

RTIXCdrProgram *RTIXCdrInterpreter_newProgram(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList **dependentProgramList,
        RTIXCdrTypeProgramKind programKind,
        const struct RTIXCdrTypePluginProgramProperty *property)
{
    RTIXCdrProgram *program = NULL;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);
    RTIXCdrLog_testPrecondition(dependentProgramList == NULL, return NULL);

    RTIXCdrHeap_allocateStruct(&program, RTIXCdrProgram);
    if (program == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
                sizeof(RTIXCdrProgram));
        goto done;
    }
    
    RTIXCdrMemory_zero(program, sizeof(RTIXCdrProgram));
    RTIXCdrInlineListNode_initialize(&program->node);
    program->kind = programKind;
     
    if (*dependentProgramList == NULL) {
        /* Create a new list */
        program->dependentProgramList = RTIXCdrDependentProgramList_newWithTc(
                tc);
        
        if (program->dependentProgramList == NULL) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
                    "dependent program list");
            goto done;
        }
        
        program->listOwner = RTI_XCDR_TRUE;
        *dependentProgramList = program->dependentProgramList;
    } else {
        program->dependentProgramList = *dependentProgramList;
        program->listOwner = RTI_XCDR_FALSE;
    }
    
    program->externalReferenceSize = property->externalReferenceSize;
    program->getExternalRefPointerFcn = property->getExternalRefPointerFcn;

    if (!property->v2Encapsulation) {
        if (property->onlyKey || property->onlyKeyForKeyhash) {
            program->serializeSentinelOnBase =
                    property->serializeSentinelOnBase;
        }
        program->disableMustUnderstandOnSentinel =
                property->disableMustUnderstandOnSentinel;
    }

    program->typeCode = (struct RTIXCdrTypeCode *)tc;
    program->extKind = RTIXCdrTypeCode_getExtensibilityKind(tc);

    if (!RTIXCdrDependentProgramList_addProgram(
            program->dependentProgramList,
            program)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ADD_FAILURE_ID_s,
                "program");
        goto done;  
    }

    program->hasBase = RTIXCdrTypeCode_hasBase(tc);

    /* 
     * CORE-11378: The hasBase property must be set to FALSE if the program is 
     * generated for onlyKey fields and the base does not need to be serialized
     */
    if (program->hasBase && property->onlyKey) {
        RTIXCdrBoolean hasKey;
        RTIXCdrBoolean hasBaseKey;

        hasKey = RTIXCdrTypeCode_hasKey(tc);
        hasBaseKey = RTIXCdrTypeCode_hasKey(tc->_typeCode);

        if (hasKey && !hasBaseKey) {
            program->hasBase = RTI_XCDR_FALSE;
        }
    }

    if (tc->_sampleAccessInfo != NULL) {
        program->isFlatDataProgram = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding);
    } else {
        program->isFlatDataProgram = RTI_XCDR_FALSE;
    }

    program->unionDiscKind = RTI_XCDR_TK_NULL;

    /*
     * CORE-10578: If the user is not setting the rtiddsgen command line option
     * "-VtcUnboundedSize", use the global variable
     * "RTIXCdrInterpreter_g_unboundedSize".
     */
    if (property->unboundedSize == RTIXCdrLong_MAX) {
        program->unboundedSize = RTIXCdrInterpreter_g_unboundedSize;
    } else {
        program->unboundedSize = property->unboundedSize;
    }

    if (property->xTypesComplianceMask != 0) {
        program->xTypesComplianceMask = property->xTypesComplianceMask;
    } else {
        program->xTypesComplianceMask =
                RTIXCdrInterpreter_getGlobalXtypeComplianceMask();
    }

    program->isUnbounded = RTIXCdrTypeCode_isUnbounded(
            tc,
            property->onlyKey,
            program->unboundedSize);

    program->hasOptionals = RTIXCdrTypeCode_hasOptionals(tc);

    if (property->v2Encapsulation) {
        program->isCdr2 = RTI_XCDR_TRUE;
    } else {
        program->isCdr2 = RTI_XCDR_FALSE;
    }

    program->encapsulationId = RTIXCdrEncapsulation_getEncapsulationId(
            property->littleEndianEncapsulation,
            program->isCdr2,
            program->extKind);

    program->onlyKey = property->onlyKey;

    ok = RTI_XCDR_TRUE;
done:
    if (!ok) {
        if (program != NULL) {
            RTIXCdrInterpreter_deleteProgram(program);
            program = NULL;
        }
    }

    return program;
}

/*
 * The following function populates a RTIXCdrTypeCodeMember structure
 * for an ALIAS, ARRAY, SEQUENCE, or ENUM typecode so we can treat them
 * as an structure with a single member.
 */
void RTIXCdrInterpreter_getTcSingleMember(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrTypeCodeMember *member)
{
    RTIXCdrTCKind tcKind;
    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(member == NULL, return);

    tcKind = RTIXCdrTypeCode_getKind(tc);

    /* ALIAS, ARRAY, ENUM:
     * Member is a pointer if owner is marked as pointer.
     *
     * SEQUENCE: Member represents the sequence itself.
     */
    member->_representation._isPointer = tc->_isPointer;

    if (tc->_typeCode == NULL || tcKind == RTI_XCDR_TK_SEQUENCE) {
        /* ENUM and SEQUENCE */
        member->_representation._typeCode = (RTIXCdrTypeCode *) tc;
    } else {
        /* ALIAS and ARRAY */
        member->_representation._typeCode = tc->_typeCode;
    }

    member->_representation._bits = 0;
    member->_representation._pid = 0;

    member->_memberFlags = 0;

    member->_annotations = tc->_annotations;
}

const char * RTIXCdrInstruction_getMemberName(
        const RTIXCdrInstruction * me,
        const RTIXCdrTypeCode *tc)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrTCKind kind;
    
    RTIXCdrLog_testPrecondition(me == NULL, return NULL);
    RTIXCdrLog_testPrecondition(tc == NULL, return NULL); 
    
    commonParams = RTIXCdrInstruction_getCommonParams(me);
    RTIXCdrLog_testPrecondition(commonParams == NULL, return NULL); 

    if (commonParams->tcMemberInfo != NULL) {
        return commonParams->tcMemberInfo->_name;
    } else {
        kind = RTIXCdrTypeCode_getKind(tc);
        
        if (kind == RTI_XCDR_TK_ALIAS) {
            return "alias";
        } else if (kind == RTI_XCDR_TK_UNION) {
            return "disc";
        } else {
            return "unknown";
        }
    }
}

void RTIXCdrInterpreter_logProgramGenerationError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrTypeProgramKind programKind,
        RTIXCdrLogMessageId messageId,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    switch (messageId) {
    case RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus:
    {
        RTIXCdrLogParam param[4];

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = RTIXCdrProgramKind_toStr(
                programKind);
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;

        if (tc->_name == NULL) {
            param[1].value.strVal = "NULL";
        } else {
            param[1].value.strVal = tc->_name;
        }

        param[2].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[2].value.ulVal = instruction->params.strParams.charMaxCount-1;
        param[3].kind = RTI_XCDR_LOG_STR_PARAM;
        param[3].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction, 
                tc);

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                4,
                param);
    } break;
    case RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss:
    case RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss:
    {
        RTIXCdrLogParam param[2];
        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = RTIXCdrProgramKind_toStr(
                programKind);
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;

        if (tc->_name == NULL) {
            param[1].value.strVal = "NULL";
        } else {
            param[1].value.strVal = tc->_name;
        }

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                2,
                param);
    } break;
    default: {
        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNKNOWN_FAILURE_ID,
                0,
                NULL);
    } break;
    }
}

RTIXCdrUnsignedLong RTIXCdrProgram_getFirstDataInstIndex(
        RTIXCdrProgram *self)
{
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrInstruction *inst = NULL;
    RTIXCdrLog_testPrecondition(
            self == NULL,
            return RTI_XCDR_TYPECODE_INVALID_INDEX);

    for (i = 0; i<self->instructionCount; i++) {
        inst = &(self->instructions[i]);

        if (!RTIXCdrInstruction_isHeaderOpcode(inst->opcode)) {
            return i;
        }
    }

    return RTI_XCDR_TYPECODE_INVALID_INDEX;
}

RTIXCdrUnsignedLong RTIXCdrProgram_getNextDataInstIndex(
        RTIXCdrProgram *self,
        RTIXCdrUnsignedLong currentInstIndex)
{
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrInstruction *inst = NULL;

    RTIXCdrLog_testPrecondition(
            self == NULL,
            return RTI_XCDR_TYPECODE_INVALID_INDEX);

    for (i = (currentInstIndex+1); i<self->instructionCount; i++) {
        inst = &(self->instructions[i]);

        if (!RTIXCdrInstruction_isHeaderOpcode(inst->opcode)) {
            return i;
        }
    }

    return RTI_XCDR_TYPECODE_INVALID_INDEX;
}

RTI_PRIVATE
RTIXCdrLong RTIXCdrDependentProgramList_compare(
    const void *left,
    const void *right)
{
    const struct RTIXCdrProgram *entry1 =
        (const struct RTIXCdrProgram *)left;
    const struct RTIXCdrProgram *entry2 =
        (const struct RTIXCdrProgram *)right;

    RTIXCdrLog_testPrecondition(entry1 == NULL, return 0);
    RTIXCdrLog_testPrecondition(entry2 == NULL, return 0);

    if (entry1->typeCode < entry2->typeCode) {
        return -1;
    } else if (entry1->typeCode > entry2->typeCode) {
        return 1;
    }

    if (entry1->kind < entry2->kind) {
        return -1;
    } else if (entry1->kind > entry2->kind) {
        return 1;
    }

    if (entry1->onlyKey < entry2->onlyKey) {
        return -1;
    } else if (entry1->onlyKey > entry2->onlyKey) {
        return 1;
    }

    return 0;
}

void RTIXCdrDependentProgramList_delete(
        struct RTIXCdrDependentProgramList *self)
{
    RTIXCdrProgram *dependentProgram = NULL;

    RTIXCdrLog_testPrecondition(self == NULL, return);

    if (self->inlineList != NULL) {
        while (self->inlineList->first != NULL) {
            dependentProgram = (RTIXCdrProgram *)
                    self->inlineList->first;
            RTIXCdrInlineList_removeNode(
                    self->inlineList, 
                    self->inlineList->first);           
            RTIXCdrInterpreter_deleteProgramWoDependencies(
                    dependentProgram);
        }

        RTIXCdrInlineList_delete(self->inlineList);
    }

    if (self->skipList != NULL) {
        const struct RTIXCdrSkipListNode *node;

        node = RTIXCdrSkipList_getFirstNode(self->skipList);

        while (node != NULL) {
            dependentProgram = RTIXCdrSkipListNode_getElement(node);
            RTIXCdrLog_testPrecondition(dependentProgram == NULL, return);

            RTIXCdrInterpreter_deleteProgramWoDependencies(dependentProgram);
            node = RTIXCdrSkipList_getNextNode(self->skipList, node);
        }

        RTIXCdrSkipList_delete(self->skipList);
    }

    RTIXCdrHeap_freeStruct(self);
}

struct RTIXCdrDependentProgramList * RTIXCdrDependentProgramList_new(
        RTIXCdrUnsignedLong expectedProgramCount)
{
    struct RTIXCdrDependentProgramList *self = NULL;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrHeap_allocateStruct(&self, struct RTIXCdrDependentProgramList);

    if (self == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
                sizeof(struct RTIXCdrDependentProgramList));
        goto done;
    }

    /*
     * Here, we have to take the decision of creating an inlineList or a 
     * skipList.
     * 
     * skipList will provide more efficient search but it will require more
     * space. 
     * 
     * In the first implementation of the interpreter we were only using
     * an inlineList. However, this impl led to bugs like the one described here
     * CORE-12179 where a CFT was taking a long time to be created.
     * 
     * We are picking a threshold of inlineListThreshold to make this decision.
     */
    if (expectedProgramCount <= RTI_XCDR_DEPENDENT_PROGRAM_LIST_INLINE_THRESHOLD) {
        self->inlineList = RTIXCdrInlineList_new();
        
        if (self->inlineList == NULL) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
                    "dependent programs inline list");
            goto done;
        }
    } else {
        self->skipList = RTIXCdrSkipList_new(
                RTIXCdrDependentProgramList_compare,
                expectedProgramCount);

        if (self->skipList == NULL) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
                    "dependent programs skip list");
            goto done;
        }
    }

    ok = RTI_XCDR_TRUE;
  done:

    if (!ok && self != NULL) {
        RTIXCdrDependentProgramList_delete(self);
        self = NULL;
    }

    return self;
}

struct RTIXCdrDependentProgramList * RTIXCdrDependentProgramList_newWithTc(
        const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrDependentProgramList *self = NULL;
    RTIXCdrUnsignedLong tcCount;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    tcCount = RTIXCdrTypeCode_getAggregationTypeCodeCount(tc);
    self = RTIXCdrDependentProgramList_new(tcCount);

    if (self == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
                "dependent programs list");
    }

    return self;
}

RTIXCdrProgram * RTIXCdrDependentProgramList_findProgram(
        const struct RTIXCdrDependentProgramList *self,
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrTypeProgramKind kind)
{    
    RTIXCdrProgram *dependentProgram = NULL;
    RTIXCdrProgram searchProgram;

    RTIXCdrLog_testPrecondition(self == NULL, return NULL);
    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    if (self->inlineList != NULL) {
        struct RTIXCdrInlineListNode *dependentProgramNode = NULL;

        dependentProgramNode = self->inlineList->first;
        
        while (dependentProgramNode != NULL) {
            dependentProgram = (RTIXCdrProgram *)dependentProgramNode;
            
            if (dependentProgram->typeCode == tc 
                    && dependentProgram->kind == kind) {
                return dependentProgram;
            }
            dependentProgramNode = dependentProgramNode->next;
        }

        return NULL;
    }

    searchProgram.typeCode = (struct RTIXCdrTypeCode *) tc;
    searchProgram.kind = kind;

    /*
     * We do not care about the key value. Therefore, we set this value to
     * RTI_XCDR_FALSE which is the smallest boolean value (0).
     */
    searchProgram.onlyKey = RTI_XCDR_FALSE;

    dependentProgram =
            RTIXCdrSkipList_findElement(self->skipList, NULL, &searchProgram);

    if (dependentProgram != NULL) {
        if (dependentProgram->typeCode != tc
            || dependentProgram->kind != kind) {
            return NULL;
        }
    }

    return dependentProgram;
}

/* This method considers if the program is for only key fields or for all the
 * fields when doing the search.
 *
 * This will be needed when working with key programs in V1 as we may need
 * to have the two versions on the list of dependent programs for the top level
 * program.
 */
RTIXCdrProgram *RTIXCdrDependentProgramList_findProgramWithKey(
        const struct RTIXCdrDependentProgramList *self,
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrTypeProgramKind kind,
        RTIXCdrBoolean onlyKey)
{
    RTIXCdrProgram *dependentProgram = NULL;
    RTIXCdrProgram searchProgram;
    RTIXCdrBoolean preciseMatch;

    RTIXCdrLog_testPrecondition(self == NULL, return NULL);
    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    if (self->inlineList != NULL) {
        struct RTIXCdrInlineListNode *dependentProgramNode = NULL;

        dependentProgramNode = self->inlineList->first;
        
        while (dependentProgramNode != NULL) {
            dependentProgram = (RTIXCdrProgram *)dependentProgramNode;
            
            if (dependentProgram->typeCode == tc 
                    && dependentProgram->kind == kind
                    && dependentProgram->onlyKey == onlyKey) {
                return dependentProgram;
            }
            dependentProgramNode = dependentProgramNode->next;
        }

        return NULL;
    }

    searchProgram.typeCode = (struct RTIXCdrTypeCode *) tc;
    searchProgram.kind = kind;
    searchProgram.onlyKey = onlyKey;

    dependentProgram = RTIXCdrSkipList_findElement(
            self->skipList,
            &preciseMatch,
            &searchProgram);

    if (dependentProgram == NULL || !preciseMatch) {
        return NULL;
    }

    return dependentProgram;
}

RTIXCdrBoolean RTIXCdrDependentProgramList_addProgram(
        struct RTIXCdrDependentProgramList *self,
        RTIXCdrProgram *program)
{    
    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);

    if (self->inlineList != NULL) {
        RTIXCdrLog_testPrecondition(
                program->node.next != NULL || program->node.prev != NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrInlineList_addNodeToBack(
                self->inlineList,
                &program->node);
    } else {
        RTIXCdrBoolean alreadyExists;

        if (!RTIXCdrSkipList_assertElement(
                self->skipList, 
                &alreadyExists, 
                program)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_ASSERT_FAILURE_ID_s,
                    "program");
            return RTI_XCDR_FALSE;
        }

        RTIXCdrLog_testPrecondition(alreadyExists, return RTI_XCDR_FALSE);
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrProgram * RTIXCdrDependentProgramList_getNodeProgram(
        const struct RTIXCdrDependentProgramList *self,
        const RTIXCdrDependentProgramListNode *node)
{
    RTIXCdrLog_testPrecondition(self == NULL, return NULL);
    RTIXCdrLog_testPrecondition(node == NULL, return NULL);

    if (self->inlineList != NULL) {
        return (RTIXCdrProgram *) node;
    }

    return RTIXCdrSkipListNode_getElement(node);
}

const RTIXCdrDependentProgramListNode * RTIXCdrDependentProgramList_getFirstNode(
        const struct RTIXCdrDependentProgramList *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return NULL);

    if (self->inlineList != NULL) {
        return self->inlineList->first;
    }

    return (const RTIXCdrDependentProgramListNode *)
            RTIXCdrSkipList_getFirstNode(self->skipList);
}

const RTIXCdrDependentProgramListNode * RTIXCdrDependentProgramList_getNextNode(
        const struct RTIXCdrDependentProgramList *self,
        const RTIXCdrDependentProgramListNode *prevNode)
{
    RTIXCdrLog_testPrecondition(self == NULL, return NULL);
    RTIXCdrLog_testPrecondition(prevNode == NULL, return NULL);

    if (self->inlineList != NULL) {
        return ((const struct RTIXCdrInlineListNode *) prevNode)->next;
    }

    return (const RTIXCdrDependentProgramListNode *)
            RTIXCdrSkipList_getNextNode(self->skipList, prevNode);
}

RTIXCdrBoolean RTIXCdrDependentProgramList_isInlineList(
        const struct RTIXCdrDependentProgramList *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);

    return (self->inlineList != NULL) ? RTI_XCDR_TRUE : RTI_XCDR_FALSE;
}
