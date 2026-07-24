/*
(c) Copyright, Real-Time Innovations, 2014-2025.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */
#include "../infrastructure/Infrastructure.h"
#include "../infrastructure/InlineList.h"
#include "../stream/Stream.h"
#include "../typeCode/TypeCode.h"
#include "ProgramSupport.h"
#include "InstructionIndex.h"

RTIXCdrOctet RTIXCdrInterpreter_g_primitiveInstOpCode[3] =
{
        RTI_XCDR_SER_PRIMITIVE_OPCODE,
        RTI_XCDR_DESER_PRIMITIVE_OPCODE,
        RTI_XCDR_SKIP_PRIMITIVE_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_stringInstOpCode[3] =
{
        RTI_XCDR_SER_STRING_OPCODE,
        RTI_XCDR_DESER_STRING_OPCODE,
        RTI_XCDR_SKIP_STRING_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_wstringInstOpCode[3] =
{
        RTI_XCDR_SER_WSTRING_OPCODE,
        RTI_XCDR_DESER_WSTRING_OPCODE,
        RTI_XCDR_SKIP_WSTRING_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_complexInstOpCode[3] =
{
        RTI_XCDR_SER_COMPLEX_OPCODE,
        RTI_XCDR_DESER_COMPLEX_OPCODE,
        RTI_XCDR_SKIP_COMPLEX_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_primitiveSeqInstOpCode[3] =
{
        RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE,
        RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE,
        RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_stringSeqInstOpCode[3] =
{
        RTI_XCDR_SER_STRING_SEQ_OPCODE,
        RTI_XCDR_DESER_STRING_SEQ_OPCODE,
        RTI_XCDR_SKIP_STRING_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_wstringSeqInstOpCode[3] =
{
        RTI_XCDR_SER_WSTRING_SEQ_OPCODE,
        RTI_XCDR_DESER_WSTRING_SEQ_OPCODE,
        RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE
};

RTIXCdrOctet RTIXCdrInterpreter_g_complexSeqInstOpCode[3] =
{
        RTI_XCDR_SER_COMPLEX_SEQ_OPCODE,
        RTI_XCDR_DESER_COMPLEX_SEQ_OPCODE,
        RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE
};

/**
 * The following global variable is configured by the environment variable
 * NDDS_XTYPES_COMPLIANCE_MASK or with the setter/getter functions
 * RTIXCdrInterpreter_getGlobalXtypeComplianceMask and
 * RTIXCdrInterpreter_setGlobalXtypeComplianceMask.
 */
RTIXCdrXTypesComplianceMask RTIXCdrInterpreter_g_XTypesComplianceMask =
        RTI_XCDR_XTYPES_COMPLIANCE_MASK_DEFAULT;

RTIXCdrXTypesComplianceMask RTIXCdrInterpreter_getGlobalXtypeComplianceMask(void)
{
    return RTIXCdrInterpreter_g_XTypesComplianceMask;
}

void RTIXCdrInterpreter_setGlobalXtypeComplianceMask(RTIXCdrXTypesComplianceMask mask)
{
    RTIXCdrInterpreter_g_XTypesComplianceMask = mask;
}

#ifndef RTI_XCDR_DISABLE_XTYPES_COMPLIANCE_MASK_FROM_ENV_VAR
RTIXCdrBoolean RTIXCdrInterpreter_loadGlobalXTypeComplianceMask(void)
{
    char envVarValue[RTI_XCDR_ENVIRONMENT_VARIABLE_MAX_SIZE] = { '\0' };
    char *value = NULL;
    RTIXCdrXTypesComplianceMask mask =
            RTIXCdrInterpreter_getGlobalXtypeComplianceMask();
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    value = RTIXCdrUtility_getEnvironmentVariable(
            NDDS_XTYPES_COMPLIANCE_MASK_ENV_VAR,
            envVarValue,
            RTI_XCDR_ENVIRONMENT_VARIABLE_MAX_SIZE);

    if (value != NULL) {
        if (!RTIXCdrUtility_strtoul(value, &mask)) {
            goto done;
        }
    }
    RTIXCdrInterpreter_setGlobalXtypeComplianceMask(mask);

    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}
#endif

RTIXCdrBoolean RTIXCdrXTypesComplianceMask_verifyGeneratedXTypesMask(
        RTIXCdrXTypesComplianceMask mask)
{
    RTIXCdrXTypesComplianceMask globalMask =
            RTIXCdrInterpreter_getGlobalXtypeComplianceMask();
    RTIXCdrBoolean globalMaskResult = RTI_XCDR_TRUE;
    RTIXCdrBoolean codegenMaskResult = RTI_XCDR_TRUE;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;

    globalMaskResult = RTIXCdrXTypesComplianceMask_isBitSet(
            globalMask,
            RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT);

    codegenMaskResult = RTIXCdrXTypesComplianceMask_isBitSet(
            mask,
            RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT);

    if (globalMaskResult != codegenMaskResult) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_BAD_PARAM_FAILURE_ID_s,
                "Inconsistent XTypes Compliance options for this type. (See "
                "https://community.rti.com/kb/xtypes-compliance-mismatch)");

        goto done;
    }

    result = RTI_XCDR_TRUE;
done:
    return result;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_generateInstructionOpcode(
        RTIXCdrInstruction * instruction,
        RTIXCdrOctet *opCodes,
        RTIXCdrBoolean *swapPrimitiveValues,
        RTIXCdrBoolean hasKey,
        RTIXCdrBoolean hasKeyBase,
        RTIXCdrBoolean skipDeserialization,
        const struct RTIXCdrTypeCodeMember *member,
        RTIXCdrTypeProgramKind programKind)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(swapPrimitiveValues == NULL, return RTI_XCDR_FALSE);
    /* member can be NULL for a union discriminator */
    
    switch (programKind) {
        case RTI_XCDR_SER_PROGRAM:
            instruction->opcode = opCodes[0];
            break;
        case RTI_XCDR_DESER_PROGRAM:
            if (!skipDeserialization) {
                instruction->opcode = opCodes[1];
            } else {
                *swapPrimitiveValues = RTI_XCDR_FALSE;
                instruction->opcode = opCodes[2];
            }
            break;
        case RTI_XCDR_GET_SER_SIZE_PROGRAM:
        case RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM:
        case RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM:
        case RTI_XCDR_SKIP_PROGRAM:
            instruction->opcode = opCodes[2];
            break;
        case RTI_XCDR_SER_TO_KEY_PROGRAM:
            if (!hasKey 
                    || (member == NULL && hasKeyBase) 
                    || (member != NULL 
                            && member->_memberFlags & RTI_XCDR_KEY_MEMBER)) {
                instruction->opcode = opCodes[1];
            } else {
                *swapPrimitiveValues = RTI_XCDR_FALSE;
                instruction->opcode = opCodes[2];
            }
            break;
        default:
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                    "stream program");
            goto done;
    }

    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

RTI_PRIVATE
RTIXCdrTCKind RTIXCdrInterpreter_alignmentToPrimitiveKind(
        RTIXCdrAlignment alignment) 
{
    switch (alignment) {
    case 1:
        return RTI_XCDR_TK_OCTET;
    case 2:
        return RTI_XCDR_TK_SHORT;
    case 4:
        return RTI_XCDR_TK_LONG;
    case 8:
        return RTI_XCDR_TK_LONGLONG;        
    default:
        return RTI_XCDR_TK_LONGDOUBLE;
    }
}

/* RTIXCdrTypeCode_isPrimitiveKind returns TRUE if memberTcKind
 * is an ENUM. However, when optimizeEnum is set to FALSE
 * the enum member cannot be treated as a primitive. For all purposes
 * is a complex member. This is why we need to wrap RTIXCdrTypeCode_isPrimitiveKind
 * into RTIXCdrInterpreter_isPrimitiveKind.
 */
#define RTIXCdrInterpreter_isPrimitiveKind(tc, memberTcKind, optimizeEnum) \
    ((RTIXCdrTypeCode_isPrimitiveKind(memberTcKind, RTI_XCDR_TRUE) && \
            (RTIXCdrTypeCode_getKind((tc)) == RTI_XCDR_TK_ENUM \
                    || (memberTcKind) != RTI_XCDR_TK_ENUM \
                    || (optimizeEnum)))?RTI_XCDR_TRUE:RTI_XCDR_FALSE)


RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_isInlineAllowed(
        RTIXCdrProgram *program,
        RTIXCdrBoolean baseInstruction,
        RTIXCdrInstruction * instruction,
        /* Indicates if the member for the instruction is a sequence */
        RTIXCdrBoolean isSeq,
        RTIXCdrBoolean swapPrimitiveValues,
        const struct RTIXCdrTypePluginProgramProperty *property)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrTCKind programTcKind;

    RTIXCdrLog_testPrecondition(program == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL,
            return RTI_XCDR_FALSE);

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);

    if (!property->inlineSequence) {
        /* 
         * We cannot inline sequence elements if the property does not allow it
         * The user must set this property to TRUE if the sequence uses 
         * discontiguous buffers.
         *
         * To figure out if we are processing a sequence we use the isSeq
         * parameter to indicate that the current instruction is a sequence
         * or programTcKind to indicate that the current program is for a 
         * sequence.
         */
        if (isSeq) {
            return RTI_XCDR_FALSE;
        }

        programTcKind = RTIXCdrTypeCode_getKind(program->typeCode);
        
        if (programTcKind == RTI_XCDR_TK_SEQUENCE) {
            return RTI_XCDR_FALSE;
        }
    }

    if (!property->inlineStruct) {
        return RTI_XCDR_FALSE;
    }

    if (property->onlyKey) {
        return RTI_XCDR_FALSE;
    }

    if (swapPrimitiveValues) {
        return RTI_XCDR_FALSE;
    }

    if (baseInstruction) {
        /* Base classes cannot be inlined */
        return RTI_XCDR_FALSE;
    }

    if (program->extKind != RTI_XCDR_FINAL_EXTENSIBILITY &&
            (program->kind == RTI_XCDR_DESER_PROGRAM ||
             program->kind == RTI_XCDR_SER_TO_KEY_PROGRAM)) {
        /* We cannot inline while deserializing because the origin type can be
         * different than the destination
         */
        return RTI_XCDR_FALSE;
    }

    if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
        /* Optional members cannot be inlined because their memory is not 
         * consecutive, as they're allocated on the heap
         */
        return RTI_XCDR_FALSE;
    }

    if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
        /* External members cannot be inlined because for external
         * arrays the ser/deser has to proceed one element at a time
         *
         * For example:
         *
         * Point {
         *     long m1;
         *     long m2;
         * };
         *
         * Point *m1[2];
         *
         * The previous member cannot be inlined because we cannot treat
         * the array as 16 bytes.
         */
        return RTI_XCDR_FALSE;
    }

    /* Structures in which we have to check min, max, and range for some members
     * cannot be inline
     */
    if (RTIXCdrTypeCode_hasNonDefaultRangeAnnotation(program->typeCode)) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_generateDHeaderInstruction(
        RTIXCdrInstruction * instruction,
        RTIXCdrTypeProgramKind programKind,
        RTIXCdrBoolean dHeaderForCollection)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    
    switch (programKind) {
        case RTI_XCDR_SER_PROGRAM:
            if (dHeaderForCollection) {
                instruction->opcode = RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE;
            } else {
                instruction->opcode = RTI_XCDR_SER_DHEADER_OPCODE;
            }
            break;
        case RTI_XCDR_SKIP_PROGRAM:
        case RTI_XCDR_SER_TO_KEY_PROGRAM:
        case RTI_XCDR_DESER_PROGRAM:
            if (dHeaderForCollection) {
                instruction->opcode = RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE;
            } else {
                instruction->opcode = RTI_XCDR_DESER_DHEADER_OPCODE;
            }
            break;
        case RTI_XCDR_GET_SER_SIZE_PROGRAM:
        case RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM:
        case RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM:
            if (dHeaderForCollection) {
                instruction->opcode = RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE;
            } else {
                instruction->opcode = RTI_XCDR_SKIP_DHEADER_OPCODE;
            }
            break;
        default:
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                    "stream program");
            goto done;
    }
    
    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_generateSentinelInstruction(
        RTIXCdrInstruction * instruction,
        RTIXCdrTypeProgramKind programKind)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    switch (programKind) {
    case RTI_XCDR_SER_PROGRAM:
        instruction->opcode = RTI_XCDR_SER_SENTINEL_HEADER_OPCODE;
        break;
    case RTI_XCDR_SKIP_PROGRAM:
    case RTI_XCDR_GET_SER_SIZE_PROGRAM:
    case RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM:
    case RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM:
        instruction->opcode = RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE;
        break;
    default:
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "stream program");
        goto done;
    }

    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_generateMemberHeaderInstruction(
        RTIXCdrProgram *program,
        RTIXCdrInstruction * instruction,
        const RTIXCdrTypeCodeMember *member,
        const RTIXCdrTypeCode *memberTc)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrTCKind memberKind = RTI_XCDR_TK_NULL;
    RTIXCdrAlignment alignment = 0;
    RTIXCdrOctet primitiveSize = 0;
    RTIBool dheaderInNonPrimitiveCollections = RTI_FALSE;
    RTIBool enumAsPrimitiveInCollections = RTI_FALSE;

    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);

    dheaderInNonPrimitiveCollections = RTIXCdrXTypesComplianceMask_isBitSet(
            program->xTypesComplianceMask,
            RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT);
    enumAsPrimitiveInCollections = RTIXCdrXTypesComplianceMask_isBitSet(
            program->xTypesComplianceMask,
            RTI_XCDR_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT);

    switch (program->kind) {
    case RTI_XCDR_SER_PROGRAM:
        instruction->opcode = RTI_XCDR_SER_MEMBER_HEADER_OPCODE;
        break;
    case RTI_XCDR_SER_TO_KEY_PROGRAM:
    case RTI_XCDR_DESER_PROGRAM:
        instruction->opcode = RTI_XCDR_DESER_MEMBER_HEADER_OPCODE;
        break;
    case RTI_XCDR_SKIP_PROGRAM:
    case RTI_XCDR_GET_SER_SIZE_PROGRAM:
    case RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM:
    case RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM:
        instruction->opcode = RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE;
        break;
    default:
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "stream program");
        goto done;
    }

    instruction->params.memberHeaderParams.hasV1NestedMemberHeaders =
            RTI_XCDR_FALSE;
    
    if (program->isCdr2) {
        /* Get the LC value for CDRv2 encapsulations */
        memberTc = RTIXCdrTypeCode_resolveAlias(memberTc);
        memberKind = RTIXCdrTypeCode_getKind(memberTc);

        instruction->params.memberHeaderParams.v2LC = 4;

        if (RTIXCdrTypeCode_isPrimitiveKind(memberKind, RTI_XCDR_TRUE)) {
            RTIXCdrTypeCode_getPrimitiveInfo(
                memberKind,
                RTI_XCDR_TRUE,
                &alignment,
                &primitiveSize);

            switch (primitiveSize) {
            case 1:
                instruction->params.memberHeaderParams.v2LC = 0;
                break;
            case 2:
                instruction->params.memberHeaderParams.v2LC = 1;
                break;
            case 4:
                instruction->params.memberHeaderParams.v2LC = 2;
                break;
            case 8:
                instruction->params.memberHeaderParams.v2LC = 3;
                break;
            default:
                /* 16-byte primitive uses LC equals to 4 */
                break;
            }
        } else if (memberKind == RTI_XCDR_TK_ARRAY 
                && dheaderInNonPrimitiveCollections) {
            const RTIXCdrTypeCode * arrElementTc =
                    RTIXCdrTypeCode_resolveAlias(memberTc->_typeCode);
            RTIXCdrTCKind arrMemberKind = RTIXCdrTypeCode_getKind(arrElementTc);

            if (!RTIXCdrTypeCode_isPrimitiveKind(
                    arrMemberKind, 
                    enumAsPrimitiveInCollections)) {
                /*
                * LC 5 because we can use the dheader serialized with the
                * array as the length (NEXTINT) of the member header for the 
                * mutable member.
                */
                instruction->params.memberHeaderParams.v2LC = 5;
            }
        } else if (memberKind == RTI_XCDR_TK_STRING 
                || memberKind == RTI_XCDR_TK_WSTRING) {
            instruction->params.memberHeaderParams.v2LC = 5;
        } else if (memberKind == RTI_XCDR_TK_SEQUENCE) {
            const RTIXCdrTypeCode * seqElementTc =
                    RTIXCdrTypeCode_resolveAlias(memberTc->_typeCode);
            RTIXCdrTCKind seqMemberKind = RTIXCdrTypeCode_getKind(seqElementTc);

            if (RTIXCdrTypeCode_isPrimitiveKind(
                        seqMemberKind, 
                        !dheaderInNonPrimitiveCollections || enumAsPrimitiveInCollections)) {
                RTIXCdrTypeCode_getPrimitiveInfo(
                    seqMemberKind,
                    RTI_XCDR_TRUE,
                    &alignment,
                    &primitiveSize);

                switch (primitiveSize) {
                case 1:
                    instruction->params.memberHeaderParams.v2LC = 5;
                    break;
                case 4:
                    instruction->params.memberHeaderParams.v2LC = 6;
                    break;
                case 8:
                    instruction->params.memberHeaderParams.v2LC = 7;
                    break;
                default:
                    /* 16-byte primitive uses LC equals to 4 */
                    break;
                }
            } else if (dheaderInNonPrimitiveCollections) {
                /* 
                 * LC 5 because we can use the dheader serialized with the
                 * sequence as the length (NEXTINT) of the member header for 
                 * the mutable member.
                 */
                instruction->params.memberHeaderParams.v2LC = 5;
            }
        } else if (memberKind == RTI_XCDR_TK_STRUCT ||
                    memberKind == RTI_XCDR_TK_UNION ||
                    memberKind == RTI_XCDR_TK_VALUE) {
            RTIXCdrExtensibilityKind extKind =
                    RTIXCdrTypeCode_getExtensibilityKind(memberTc);

            if (extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY ||
                    extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                instruction->params.memberHeaderParams.v2LC = 5;
            }
        }
    } else {
        if (member == NULL) {
            /* Union discriminator */
            instruction->params.memberHeaderParams.v1ExtendedId = RTI_XCDR_FALSE;
        } else {
            instruction->params.memberHeaderParams.v1ExtendedId = RTI_XCDR_MAYBE;
        
            if (member->_representation._pid > 16128) {
                instruction->params.memberHeaderParams.v1ExtendedId = RTI_XCDR_TRUE;
            } else {
                memberTc = RTIXCdrTypeCode_resolveAlias(memberTc);
                memberKind = RTIXCdrTypeCode_getKind(memberTc);

                if (RTIXCdrTypeCode_isPrimitiveKind(memberKind, RTI_XCDR_TRUE)) {
                    instruction->params.memberHeaderParams.v1ExtendedId = RTI_XCDR_FALSE;
                }
            }

            instruction->params.memberHeaderParams.hasV1NestedMemberHeaders = 
                    RTIXCdrTypeCode_hasMemberHeaders(memberTc);
        }
    }
    
    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

struct RTIXCdrInstructionState {
    RTIXCdrOctet opcode;
    RTIXCdrOctet refMemberKind;
    RTIXCdrBoolean useNonRefGetter;
    RTIXCdrUnsignedLongLong primitiveByteCount;
    RTIXCdrUnsignedLongLong strMaxCount;
    RTIXCdrAlignment primitiveAlignment;
    RTIXCdrOctet primitiveSize;
    RTIXCdrBoolean checkEnum;
    RTIXCdrBoolean memberDHeader;
};

#define RTIXCdrInstructionState_INITIALIZER { \
    RTI_XCDR_INVALID_OPCODE, \
    RTI_XCDR_FALSE, \
    RTI_XCDR_FALSE, \
    0, \
    0, \
    RTI_XCDR_INVALID_ALIGNMENT, \
    0, \
    RTI_XCDR_FALSE, \
    RTI_XCDR_FALSE \
}

RTI_PRIVATE
void RTIXCdrInstruction_storeState(
        struct RTIXCdrInstruction *self,
        struct RTIXCdrInstructionState *state,
        RTIXCdrBoolean countInBytes,
        RTIXCdrBoolean memberDHeader)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;
    
    commonParams = RTIXCdrInstruction_getCommonParams(self);
    
    state->opcode = self->opcode;
    state->primitiveAlignment = RTI_XCDR_INVALID_ALIGNMENT;
    state->primitiveByteCount = 0;
    state->strMaxCount = 0;
    state->useNonRefGetter = RTI_XCDR_FALSE;
    state->refMemberKind = commonParams->refMemberKind;
    state->checkEnum = RTI_XCDR_FALSE;
    state->memberDHeader = memberDHeader;

    if (commonParams->memberTc != NULL
            && commonParams->memberTc->_sampleAccessInfo != NULL
            && commonParams->memberTc->_sampleAccessInfo->getMemberValuePointerFcn != NULL
            && !commonParams->memberTc->_sampleAccessInfo->useGetMemberValueOnlyWithRef) {
        state->useNonRefGetter = RTI_XCDR_TRUE;
    }

    if (RTIXCdrInstruction_isPrimitiveOpcode(self->opcode)) {
        state->primitiveSize = self->params.primitiveParams.primitiveSize;

        if (countInBytes) {
            state->primitiveByteCount = commonParams->count;
        } else {
            state->primitiveByteCount = 
                    ((RTIXCdrUnsignedLongLong) commonParams->count) *
                            self->params.primitiveParams.primitiveSize;
        }

        state->primitiveAlignment = self->params.primitiveParams.primitiveAlignment;

        if (self->params.primitiveParams.enumTc != NULL) {
            state->checkEnum = RTI_XCDR_TRUE;
        }
    }
    
    if (RTIXCdrInstruction_isStringOpcode(self->opcode)) {    
        state->strMaxCount = self->params.strParams.charMaxCount;
    }
}

RTI_PRIVATE
void RTIXCdrInterpreter_assignBaseMemberValueOffset(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean *emptyTc,
        RTIXCdrInstruction *baseMemberInstruction)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;
    int i = 0;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(baseMemberInstruction == NULL, return);
    
    commonParams = RTIXCdrInstruction_getCommonParams(baseMemberInstruction);
    
    *emptyTc = RTI_XCDR_TRUE;
    
    for (i=0; i<4; i++) {
        commonParams->memberAccessInfo.bindingMemberValueOffset[i] = 0;
    }

    kind = RTIXCdrTypeCode_getKind(tc);
    
    if (kind == RTI_XCDR_TK_ALIAS) {
        tc = RTIXCdrTypeCode_resolveAlias(tc);
        kind = RTIXCdrTypeCode_getKind(tc);
    }

    if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE) {
        if (tc->_typeCode != NULL &&
                RTIXCdrTypeCode_getKind(tc->_typeCode) != RTI_XCDR_TK_NULL) {
            RTIXCdrInterpreter_assignBaseMemberValueOffset(
                    tc->_typeCode,
                    emptyTc,
                    baseMemberInstruction);

            if (!*emptyTc) {
                return;
            }
        }
    }

    if (tc->_memberCount > 0) {
        RTIBool isFlatData;

        isFlatData = RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
                tc->_sampleAccessInfo->languageBinding);

        *emptyTc =RTI_XCDR_FALSE;
        for (i=0; i<4; i++) {
            if (isFlatData) {
                commonParams->memberAccessInfo.bindingMemberValueOffset[i] =
                        tc->_sampleAccessInfo->memberAccessInfos[0].bindingMemberValueOffset[i];
            } else {
                /* 
                 * CODEGENII-1458: The base class for non flat data language 
                 * binding starts in offset 0
                 */
                commonParams->memberAccessInfo.bindingMemberValueOffset[i] = 0;
            }
        }
    }
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_isProgramMemberIdIndexRequired(
        RTIXCdrProgram *program)
{
    RTIXCdrBoolean generateIndex = RTI_XCDR_TRUE;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(program->typeCode);

    if ((kind == RTI_XCDR_TK_VALUE ||
            kind == RTI_XCDR_TK_STRUCT ||
            kind == RTI_XCDR_TK_UNION) &&
            program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY)
    {
        /* There are some cases in which the generation of indexes is
         * unnecessary. For example, upon serialization where we process
         * each member sequentially
         */
        if (program->kind != RTI_XCDR_DESER_PROGRAM &&
                program->kind != RTI_XCDR_SKIP_PROGRAM &&
                program->kind != RTI_XCDR_SER_TO_KEY_PROGRAM) {
            generateIndex = RTI_XCDR_FALSE;
        } else if (program->kind == RTI_XCDR_SKIP_PROGRAM &&
                program->isCdr2) {
            /* No need to generate index because skipping consists on
             * jumping length in DHEADER
             */
            generateIndex = RTI_XCDR_FALSE;
        }
    } else {
        generateIndex = RTI_XCDR_FALSE;
    }

    return generateIndex;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_isProgramLabelIndexRequired(
        RTIXCdrProgram *program)
{
    RTIXCdrBoolean generateIndex = RTI_XCDR_FALSE;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(program->typeCode);

    if (kind == RTI_XCDR_TK_UNION)
    {
        /* No need to generate index for skip program if there is DHEADER */
        if (!(program->kind == RTI_XCDR_SKIP_PROGRAM &&
                program->isCdr2 &&
                program->extKind != RTI_XCDR_FINAL_EXTENSIBILITY)) {
            if (program->kind == RTI_XCDR_SER_PROGRAM ||
                    program->kind == RTI_XCDR_GET_SER_SIZE_PROGRAM) {
                generateIndex = RTI_XCDR_TRUE;
            }

            if (program->kind == RTI_XCDR_DESER_PROGRAM ||
                    program->kind == RTI_XCDR_SKIP_PROGRAM ||
                    program->kind == RTI_XCDR_SER_TO_KEY_PROGRAM) {
                if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                    generateIndex = RTI_XCDR_TRUE;
                }
            }
        }
    }

    return generateIndex;
}

/**
 * @brief Add entries from the instruction index of a base type into this
 * this program's instruction index.
 *
 * This function will only merge the base type's instruction index if:
 *
 * - The program's type is mutable and has a complex base type.
 * - The program is a "deserialization", or "serialization to key" program.
 * - The base program actually has an instruction index.
 *
 * @pre The program must have a base type program.
 *
 * @param[in,out] program The program to merge the base type instruction index
 * into.
 * @param[in] baseTypeInstIndex The index of the instruction responsible for
 *            deserializing the base type. This value must be injected into the
 *            function because it is not cached inside the program.
 *
 * @return True if the base instruction index was merged (or not merged)
 * successfully, false otherwise.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_mergeBaseProgramMemberIdIndex(
        RTIXCdrProgram *program,
        const RTIXCdrUnsignedLong baseTypeInstIndex)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrInstruction *baseInstruction = NULL;
    RTIXCdrProgram *baseProgram = NULL;

    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(!program->hasBase, return RTI_XCDR_FALSE);

    if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY
        || (program->kind != RTI_XCDR_DESER_PROGRAM
            && program->kind != RTI_XCDR_SER_TO_KEY_PROGRAM
            && !(program->kind == RTI_XCDR_SKIP_PROGRAM && program->isCdr2))) {
        ok = RTI_XCDR_TRUE;
        goto done;
    }

    RTIXCdrLog_testPrecondition(
            baseTypeInstIndex >= program->instructionCount,
            goto done);
    baseInstruction = &program->instructions[baseTypeInstIndex];
    /* It is possible for the base instruction to not be a
     * DESER_COMPLEX, e.g.:
     * - Program is SER_TO_KEY, and the base type does not have
     *   key members, but the derived type does.
     */
    if (baseInstruction->opcode != RTI_XCDR_DESER_COMPLEX_OPCODE) {
        ok = RTI_XCDR_TRUE;
        goto done;
    }

    baseProgram = baseInstruction->params.complexParams.program;
    RTIXCdrLog_testPrecondition(baseProgram == NULL, goto done);
    if (baseProgram->instructionIndex == NULL) {
        ok = RTI_XCDR_TRUE;
        goto done;
    }

    if (!RTIXCdrInstructionIndex_addBaseTypeEntries(
                program->instructionIndex,
                baseProgram->instructionIndex)) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                "program base member ID index entries",
                program->typeCode->_name);
        goto done;
    }

    ok = RTI_XCDR_TRUE;
done:
    return ok;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_treatPrimitiveAsOctets(
        const RTIXCdrProgram *program,
        const RTIXCdrInstruction *instruction,
        RTIXCdrBoolean swapPrimitiveValues)
{
    struct RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);
    params = &instruction->params;
    
    if (swapPrimitiveValues) {
    	return RTI_XCDR_FALSE;
    }

    if (instruction->opcode == RTI_XCDR_SER_PRIMITIVE_OPCODE ||
        instruction->opcode == RTI_XCDR_DESER_PRIMITIVE_OPCODE) {
    
        if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
            /* External members cannot be trated as octets because for external
             * arrays the ser/deser has to go one element at a time
             * 
             * For example:
             * 
             * int *m1[2];
             * 
             * The previous member cannot be trated as 8 bytes
             */
             return RTI_XCDR_FALSE;
        }
        
        if (params->primitiveParams.enumTc != NULL) {
            /* The validity of an enum member has to be checked explicitly
             * This cannot be done if we treat the enum(s) as octets
             */
             return RTI_XCDR_FALSE;
        }
        
        if (params->primitiveParams.checkRange) {
            /* If we have to check ranges we cannot treat the primitive as
               octets */
            return RTI_XCDR_FALSE;
        }

        if (params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR &&
                !program->isCdr2) {
            /* The wire representation (4 bytes) of a V1 wchar is not compatible
             * with the in-memory representation (2 bytes)
             */
             return RTI_XCDR_FALSE;
        }
    }
    
    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInstruction_mustAlign(
        const RTIXCdrProgram *program,
        const RTIXCdrInstruction *instruction,
        const struct RTIXCdrInstructionState *prevInstState) 
{
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;

    /* 
     * When a precondition is not met we return TRUE because is more
     * conservative
     */
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_TRUE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_TRUE);
    RTIXCdrLog_testPrecondition(prevInstState == NULL, return RTI_XCDR_TRUE);

    kind = RTIXCdrTypeCode_getKind(program->typeCode);

    if (kind == RTI_XCDR_TK_UNION) {
        return RTI_XCDR_TRUE;
    }

    params = &instruction->params;

    if (prevInstState->primitiveAlignment 
            < params->primitiveParams.primitiveAlignment) {
        return RTI_TRUE;
    }

    if ((prevInstState->primitiveByteCount 
            % (RTIXCdrUnsignedLongLong)prevInstState->primitiveAlignment) != 0) {
        return RTI_TRUE;
    }

    if (prevInstState->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
        /* 
         * CORE-10254: We will always align members if the previous member
         * is optional
         */
        return RTI_XCDR_TRUE;
    }

    return RTI_FALSE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_isInstructionMergeable(
        const RTIXCdrProgram *program,
        const RTIXCdrInstruction *instruction,
        const struct RTIXCdrInstructionState *prevInstState,
        RTIXCdrBoolean swapPrimitiveValues,
        RTIXCdrUnsignedLong tcMemberIndex,
        RTIXCdrBoolean onlyKey,
        RTIXCdrBoolean memberDHeader)
{
    struct RTIXCdrCommonInsParameters *commonParams = NULL;
    const RTIXCdrInsParameters *params = NULL;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;

    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(prevInstState == NULL, return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(program->typeCode);

    if (memberDHeader
            || prevInstState->memberDHeader) {
        /*
         * If the member has to be serialized with Dheader (non primitive 
         * sequences and arrays in V2) then we cannot merge instructions.
         */
        return RTI_XCDR_FALSE;
    }

    if (onlyKey) {
        return RTI_XCDR_FALSE;
    }

    if (kind == RTI_XCDR_TK_UNION) {
        /*
         * The instructions for unions cannot be merged because in unions
         * only one of the instructions for the different members will be
         * executed.
         */
        return RTI_XCDR_FALSE;
    }

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);
    params = &instruction->params;

    /*
     * We can safely use tcMemberIndex instead of using memberIndex because
     * we cannot reach this point if we have a TK_UNION.
     */
    if (tcMemberIndex == 0) {
        /*
         * The instruction corresponding to the first member cannot be merged
         * with other instructions.
         *
         * Note that checking tcMemberIndex instead of memberIndex is fine: we
         * have already ruled out unions, and tcMemberIndex will be greater than
         * zero only if memberIndex > 0.
         */
        return RTI_XCDR_FALSE;
    }

    if (instruction->opcode != prevInstState->opcode) {
        return RTI_XCDR_FALSE;
    }

    if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
        /* Instructions of mutable types cannot be merged because the members
         * associated with the instruction will always have a header
         */
        return RTI_XCDR_FALSE;
    }

    if (program->extKind != RTI_XCDR_FINAL_EXTENSIBILITY
            && (program->kind == RTI_XCDR_DESER_PROGRAM ||
                    program->kind == RTI_XCDR_SER_TO_KEY_PROGRAM)) {
        /* For extensible types we cannot
         * merge members for deserialization related operations
         * because the input publisher type may not have the same members
         * than the subscriber type
         */
        return RTI_XCDR_FALSE;
    }

    if (commonParams->refMemberKind != RTI_XCDR_INTERPRETER_VALUE_MEMBER ||
            prevInstState->refMemberKind != RTI_XCDR_INTERPRETER_VALUE_MEMBER) {
        /* Instructions for reference members (optional or pointers) cannot
         * be merged
         */
        return RTI_XCDR_FALSE;
    }

    if (commonParams->memberTc != NULL
            && commonParams->memberTc->_sampleAccessInfo != NULL
            && (commonParams->memberTc->_sampleAccessInfo->getMemberValuePointerFcn != NULL &&
            !commonParams->memberTc->_sampleAccessInfo->useGetMemberValueOnlyWithRef)) {
        /* We cannot group together instructions if the location of the
         * member is provided via function
         */
        return RTI_XCDR_FALSE;
    }

    if (prevInstState->useNonRefGetter) {
        /* The previous instructions must not use a getter to access the
         * value of a non reference member */
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrInstruction_isPrimitiveOpcode(instruction->opcode)
            && program->typeCode->_sampleAccessInfo != NULL) {
        RTIXCdrUnsignedLongLong offsetDiff = 0;

        if (!program->isCdr2 
                && params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR) {
            /* 
             * For v1 encapsulation the cdr size of wchar is 4, but the
             * langauge bindings size may be smaller than this (in most cases
             * it will be 2). We cannot merge instructions if the language
             * binding layout does not macth the CDR layout and for this
             * specific case of wchar this may not be caught in the offsetDiff
             * check below due to padding bytes 
             */
            return RTI_XCDR_FALSE;
        }

        offsetDiff =
                program->typeCode->_sampleAccessInfo->memberAccessInfos[tcMemberIndex].
                        bindingMemberValueOffset[NON_FLAT_DATA_INDEX] -
                program->typeCode->_sampleAccessInfo->memberAccessInfos[tcMemberIndex - 1].
                        bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

        if (offsetDiff != prevInstState->primitiveByteCount) {
            /* We cannot merge primitive instruction if the language
             * binding layout does not macth the CDR layout
             */
            return RTI_XCDR_FALSE;
        }


        /* Merging a primitive instruction will enable doing a single memcpy
         * for multiple primitive members
         */
        if (swapPrimitiveValues) {
            /* Primitive instructions cannot be merged if we have to swap
             * bytes when serializing/deserializing
             */
            return RTI_XCDR_FALSE;
        }

        if (params->primitiveParams.enumTc != NULL ||
                prevInstState->checkEnum == RTI_XCDR_TRUE) {
            /* An enum instruction cannot be merged if we have to check the
             * values. For example, to detect that the user is writing a value
             * that is not allowed
             */
            return RTI_XCDR_FALSE;
        }

        if (params->primitiveParams.checkRange) {
            /* We cannot merge instructions if we have to check the range */
            return RTI_XCDR_FALSE;
        }

        if (params->primitiveParams.mustAlign) {
            /* We cannot merge primitive instructions if there could be padding
             * in the middle of the CDR stream
             */
            return RTI_XCDR_FALSE;
        }

        if (params->primitiveParams.primitiveSize !=
                prevInstState->primitiveSize) {
            /* We cannot merge primitive instructions if the primitive sizes
             * are different.
             *
             * Note that when we don't have to change endianness the primitiveSize
             * is set to one. Because of this the instructions for the members in
             * this IDL type can be merged if the program is on the native
             * endianness
             *
             * struct Point {
             *      long x;
             *      short y;
             * }
             */
            return RTI_XCDR_FALSE;
        }

        return RTI_XCDR_TRUE;
    }

    if (RTIXCdrInstruction_isStringNonSeqOpcode(instruction->opcode)) {
        /* Merging two string instructions will enable treating two
         * separate strings as a single string array. In this case we save
         * a few function calls
         */

        if (params->strParams.charMaxCount != prevInstState->strMaxCount) {
            return RTI_XCDR_FALSE;
        }

        return RTI_XCDR_TRUE;
    }

    return RTI_XCDR_FALSE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_needMutableHeader(
        RTIXCdrProgram *program,
        RTIXCdrBoolean onlyKeyForKeyhash) {
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrLog_testPrecondition(
            program->typeCode == NULL,
            return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(program->typeCode);

    if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
        if (kind == RTI_XCDR_TK_VALUE ||
                kind == RTI_XCDR_TK_STRUCT ||
                kind == RTI_XCDR_TK_UNION) {
            if (program->kind != RTI_XCDR_SER_TO_KEY_PROGRAM &&
                    program->kind != RTI_XCDR_DESER_PROGRAM &&
                    program->kind != RTI_XCDR_SKIP_PROGRAM) {

                if (!onlyKeyForKeyhash) {
                    return RTI_XCDR_TRUE;
                }
            }
        }
    }

    return RTI_XCDR_FALSE;
}

struct RTIXCdrMemberIndexEntry {
    RTIXCdrUnsignedLong memberId;
    RTIXCdrUnsignedLong memberIndex;
};

RTI_PRIVATE
int RTIXCdrMemberIndexEntry_compare(
        const void *entry1,
        const void *entry2)
{
    RTIXCdrUnsignedLong valLeft =
            ((const struct RTIXCdrMemberIndexEntry *)entry1)->memberId;
    RTIXCdrUnsignedLong valRight =
            ((const struct RTIXCdrMemberIndexEntry *)entry2)->memberId;


    if (valLeft > valRight) {
        return 1;
    } else if (valLeft < valRight) {
        return -1;
    }

    return 0;
}

/* The following function returns an array of member indexes
 *
 * When onlyKeyForKeyhash is set to TRUE, the members are ordered by
 * ascending member ID
 */
RTI_PRIVATE
struct RTIXCdrMemberIndexEntry *RTIXCdrInterpreter_createMemberIndexArray(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean onlyKeyForKeyhash)
{
    struct RTIXCdrMemberIndexEntry *result = NULL;
    RTIXCdrUnsignedLong i;
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    kind = RTIXCdrTypeCode_getKind(tc);

    RTIXCdrHeap_allocateArray(
            &result,
            tc->_memberCount,
            struct RTIXCdrMemberIndexEntry);

    if (result == NULL) {
        RTIXCdrLog_logTwoLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                1,
                sizeof(struct RTIXCdrMemberIndexEntry));
        return NULL;
    }

    for (i=0; i<tc->_memberCount; i++) {
        result[i].memberId = tc->_members[i]._representation._pid;
        result[i].memberIndex = i;
    }

    if (onlyKeyForKeyhash &&
            (kind == RTI_XCDR_TK_STRUCT ||
            kind == RTI_XCDR_TK_VALUE)) {
        qsort(result,
                tc->_memberCount,
                sizeof(struct RTIXCdrMemberIndexEntry),
                RTIXCdrMemberIndexEntry_compare);
    }

    return result;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrProgram_isFastSerializationSupported(
        RTIXCdrProgram *program)
{
    RTIXCdrUnsignedLong i;
    struct RTIXCdrInstruction *instruction;
    RTIXCdrCommonInsParameters *commonParams;

    if (program->isFlatDataProgram) {
        return RTI_XCDR_FALSE;
    }

    if (program->unionDiscKind != RTI_XCDR_TK_NULL) {
        /* Union types do not support fast ser */
        return RTI_XCDR_FALSE;
    }

    if (program->instructionIndex != NULL) {
        /* Mutable types do not support fast ser */
        return RTI_XCDR_FALSE;
    }

    for (i=0; i<program->instructionCount; i++) {
        instruction = &program->instructions[i];

        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        if (commonParams->addSeqDHeader) {
            /*
             * Fast serialization program not supported with arrays of sequences
             * of primitive enums when the global option
             * dheaderInNonPrimitiveCollections (not default) is enabled
             */
            return RTI_FALSE;
        }

        if (!RTIXCdrInstruction_isPrimitiveOpcode(instruction->opcode) &&
                instruction->opcode != RTI_XCDR_DESER_DHEADER_OPCODE &&
                instruction->opcode != RTI_XCDR_SER_DHEADER_OPCODE) {
            return RTI_XCDR_FALSE;
        }

        if (RTIXCdrInstruction_isPrimitiveOpcode(instruction->opcode)) {
            if (commonParams->refMemberKind != RTI_XCDR_INTERPRETER_VALUE_MEMBER) {
                return RTI_XCDR_FALSE;
            }

            if (RTIXCdrInstruction_isPrimitiveSeqOpcode(instruction->opcode)) {
                if (commonParams->memberTc != NULL
                        && RTIXCdrInterpreter_useMemberElementIndex(commonParams->memberTc->_sampleAccessInfo)) {
                    return RTI_XCDR_FALSE;
                }
                if (commonParams->seqElementTc != NULL
                        && commonParams->seqElementTc->_sampleAccessInfo != NULL) {
                    if (commonParams->seqElementTc->_sampleAccessInfo->getMemberValuePointerFcn != NULL &&
                            !commonParams->seqElementTc->_sampleAccessInfo->useGetMemberValueOnlyWithRef) {
                        return RTI_XCDR_FALSE;
                    }
                }
            } else {
                if (commonParams->memberTc != NULL
                        && commonParams->memberTc->_sampleAccessInfo != NULL) {
                    if (commonParams->memberTc->_sampleAccessInfo->getMemberValuePointerFcn != NULL &&
                            !commonParams->memberTc->_sampleAccessInfo->useGetMemberValueOnlyWithRef) {
                        return RTI_XCDR_FALSE;
                    }
                }
            }
        }
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_needPrimitiveRangeCheck(
        RTIXCdrTypeProgramKind programKind,
        struct RTIXCdrTypeCodeMember *tcMemberInfo)
{
    RTIXCdrLog_testPrecondition(tcMemberInfo == NULL, return RTI_XCDR_FALSE);

    if (!(programKind == RTI_XCDR_SER_PROGRAM ||
            programKind == RTI_XCDR_DESER_PROGRAM ||
            programKind == RTI_XCDR_SER_TO_KEY_PROGRAM)) {
        return RTI_XCDR_FALSE;
    }

    return RTIXCdrTypeCodeAnnotations_hasNonDefaultRange(
            &tcMemberInfo->_annotations);
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_optimizeMaxMinSerSizeProgram(
        RTIXCdrProgram *program,
        const struct RTIXCdrTypePluginProgramProperty *property)
{
    struct RTIXCdrTypePluginProgramContext context =
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    const RTIXCdrDependentProgramListNode *node = NULL;        
    RTIXCdrProgram *nextProgram;

    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (!program->listOwner) {
        return RTI_XCDR_TRUE;
    }

    if (program->kind != RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM &&
            program->kind != RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM) {
        return RTI_XCDR_TRUE;
    }

    /* We don't optimize when optimization is 2 because in order to do that
     * the program execution that follows next would require endpoint data
     * which we dont have.
     *
     * The endpoint data is defined in PRES
     */
    if (!property->optimizeEnum ||
            !property->resolveAlias ||
            !property->inlineStruct) {
        return RTI_XCDR_TRUE;
    }

    if (program->kind == RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM &&
            !property->v2Encapsulation) {
        /* We don't precalculate min size of V1 programs because
         * the calculation depends on useXcdr1ExtendedId and this depends
         * on having generated RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM which is not
         * guaranteed
         */
        return RTI_XCDR_TRUE;
    }

    node = RTIXCdrDependentProgramList_getFirstNode(
            program->dependentProgramList);

    while (node != NULL) {
        nextProgram = RTIXCdrDependentProgramList_getNodeProgram(
                program->dependentProgramList,
                node);
        context.program = nextProgram;
        context.onlyKey = (property->onlyKey || property->onlyKeyForKeyhash);
        context.typeCode = program->typeCode;
        context.overflow = RTI_XCDR_FALSE;
        context.inBaseClass = RTI_XCDR_FALSE;
        context.xcdrStream = NULL;

        /* Used by getSerSampleMaxSize and getSerSampleMinSize when optimization
         * is 2. In this case, after the program is generated we run it once,
         * we cache the result in serSize and we delete the instructions.
         *
         * The program execution will return serSize
         */
        if (program->kind == RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) {
            if (!RTIXCdrInterpreter_getSerSampleMaxSize(
                    &nextProgram->serSize,
                    nextProgram->typeCode,
                    nextProgram,
                    &context)) {
                return RTI_XCDR_FALSE;
            }
        } else {
            if (!RTIXCdrInterpreter_getSerSampleMinSize(
                    &nextProgram->serSize,
                    nextProgram->typeCode,
                    nextProgram,
                    &context)) {
                return RTI_XCDR_FALSE;
            }
        }

        node = RTIXCdrDependentProgramList_getNextNode(
                program->dependentProgramList,
                node);
    }

    node = RTIXCdrDependentProgramList_getFirstNode(
            program->dependentProgramList);

    while (node != NULL) {
        nextProgram = RTIXCdrDependentProgramList_getNodeProgram(
                program->dependentProgramList,
                node);
        RTIXCdrProgram_deleteInstructions(nextProgram);
        node = RTIXCdrDependentProgramList_getNextNode(
                program->dependentProgramList,
                node);
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrProgram *RTIXCdrInterpreter_generateTypePluginProgram(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList *dependentProgramList,
        RTIXCdrTypeProgramKind programKind,
        const struct RTIXCdrTypePluginProgramProperty *property)
{
    RTIXCdrProgram *program = NULL;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean swapPrimitiveValues = RTI_XCDR_FALSE;
    RTIXCdrBoolean globalSwapPrimitiveValues = RTI_XCDR_FALSE;
    struct RTIXCdrInstructionState prevInstState =
            RTIXCdrInstructionState_INITIALIZER;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
    RTIXCdrInstruction *instruction = NULL;
    RTIXCdrBoolean loggedError = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    struct RTIXCdrMemberIndexEntry *memberIndexArray = NULL;
    RTIXCdrUnsignedLong baseTypeInstIndex = 0;
    /* 
     * ROBUSTNESS-150: Declared at this scope because tc can be assigned to 
     * singleMemberTc below. 
     */
    struct RTIXCdrTypeCodeMember singleMember;
    struct RTIXCdrTypeCode singleMemberTc;
    RTIXCdrBoolean dheaderInNonPrimitiveCollections = RTI_XCDR_FALSE;
    RTIXCdrBoolean enumAsPrimitiveInCollections = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);
    RTIXCdrLog_testPrecondition(property == NULL, return NULL);

    if (RTIXCdrTypeCode_isCdrRepresentation(tc)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "typecode is in CDR representation");
        loggedError = RTI_XCDR_TRUE;
        GotoDoneWithLine();
    }
    
    if (property->onlyKeyForKeyhash) {
        if (property->littleEndianEncapsulation ||
                !property->v2Encapsulation ||
                !property->onlyKey ||
                (programKind != RTI_XCDR_SER_PROGRAM &&
                programKind != RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM)) {
            logMessageId =
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
            GotoDoneWithLine();
        }
    }

    /* 
     * First we look for the program to be created in the dependentProgramList
     * and if its there we return it
     */
    if (dependentProgramList != NULL) {
        program = RTIXCdrDependentProgramList_findProgramWithKey(
                dependentProgramList,
                tc,
                programKind,
                property->onlyKey);
        
        if (program != NULL) {
            return program;
        }
    }
    
    if (programKind != RTI_XCDR_SKIP_PROGRAM &&
            programKind != RTI_XCDR_GET_SER_SIZE_PROGRAM &&
            programKind != RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM &&
            programKind != RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM) {
      #ifdef RTI_ENDIAN_LITTLE
        if (!property->littleEndianEncapsulation) {
            globalSwapPrimitiveValues = RTI_XCDR_TRUE;
        }
      #else
        if (property->littleEndianEncapsulation) {
            globalSwapPrimitiveValues = RTI_XCDR_TRUE;
        }
      #endif
    }

    swapPrimitiveValues = globalSwapPrimitiveValues;

    program = RTIXCdrInterpreter_newProgram(
            tc,
            &dependentProgramList,
            programKind,
            property);
    if (program == NULL) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                "program", 
                tc->_name);
        loggedError = RTI_XCDR_TRUE;
        GotoDoneWithLine();
    }
    
    if (program->isUnbounded &&
            programKind == RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM &&
            (tc->_sampleAccessInfo == NULL ||
            tc->_sampleAccessInfo->languageBinding
                    != RTI_XCDR_TYPE_BINDING_DYN_DATA)) {
        /*
         * We dont need to generate any instruction because we know the type
         * is unbounded and its maximum size will be set to RTIXCdrLong_MAX. 
         *  
         * However, we do need to generate them for dynamic data because 
         * dynamic data relies on nested members having programs so that when 
         * users bind/get complex members they can treat those nested 
         * dynamic data objects the same as top-level objects. 
         *  
         * If tc->_sampleAccessInfo is NULL we know it's not DynamicData, so we 
         * skip generating the extra programs in that case as well 
         */
        ok = RTI_XCDR_TRUE;
        goto done;
    }

    dheaderInNonPrimitiveCollections = property->v2Encapsulation
            && RTIXCdrXTypesComplianceMask_isBitSet(program->xTypesComplianceMask,
                    RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT);
    enumAsPrimitiveInCollections = RTIXCdrXTypesComplianceMask_isBitSet(
            program->xTypesComplianceMask,
            RTI_XCDR_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT);

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ENUM:
        case RTI_XCDR_TK_ARRAY:
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ALIAS:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_UNION:
        {
            RTIXCdrBoolean hasKey = RTI_XCDR_FALSE;
            RTIXCdrBoolean hasBaseKey = RTI_XCDR_FALSE; 
            RTIXCdrUnsignedLong memberIndex = 0;
            RTIXCdrUnsignedLong instIndex = 0;
            RTIXCdrUnsignedLong memberCount = 0;
            RTIXCdrBoolean isUnion = RTI_XCDR_FALSE;
            RTIXCdrBoolean hasBase = RTI_XCDR_FALSE;            
            RTIXCdrUnsignedLong instructionCount = 0;
            RTIXCdrBoolean dheader = RTI_XCDR_FALSE;
            RTIXCdrBoolean sentinel = RTI_XCDR_FALSE;

            if (kind == RTI_XCDR_TK_STRUCT ||
                    kind == RTI_XCDR_TK_UNION) {
                /* 
                 * The min/max size serialized programs do not depend on sample
                 * accessors
                 */
                if (tc->_sampleAccessInfo == NULL
                        && programKind != RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM
                        && programKind != RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) {
                    RTIXCdrLog_logTwoStr(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_GET_FAILURE_ID_ss,
                        "sample access info",
                        tc->_name);
                    loggedError = RTI_XCDR_TRUE;
                    goto done;
                }
            }
            
            if (tc->_sampleAccessInfo != NULL &&
                    tc->_sampleAccessInfo->typeSize[0] >= RTIXCdrLong_MAX) {
                RTIXCdrLog_logTwoStr(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_GET_FAILURE_ID_ss,
                        "type size is too big",
                        tc->_name);
                loggedError = RTI_XCDR_TRUE;
                goto done;
            }
            
            if (property->v2Encapsulation &&
                    RTIXCdrTypeCode_isAggregationKind(kind)) {
                if ((program->extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY ||
                        program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY)) {
                    if (!property->onlyKeyForKeyhash) {
                        instructionCount++;
                        dheader = RTI_XCDR_TRUE;
                    }
                }
            }
            
            if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE ||
                    RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_STRUCT) {
                RTIXCdrTypeCode_getFirstMemberAlignment(
                        tc,
                        &program->firstMemberAlignment,
                        property->v2Encapsulation);
                
                if (program->firstMemberAlignment == -1) {
                    /* If the alignment is unknown because the type is empty
                     * we set it to 8 bytes because zero is not a valid
                     * alignment.
                     */
                    program->firstMemberAlignment = 8;
                }
            }
            
            if (!RTIXCdrTypeCode_isAggregationKind(kind)) {
                singleMemberTc = *tc;
                singleMemberTc._members = &singleMember;
                singleMemberTc._memberCount = 1;
                RTIXCdrInterpreter_getTcSingleMember(tc, &singleMember);
                tc = &singleMemberTc;
            }
            
            if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE) {
                if (tc->_typeCode != NULL) {
                    if (RTIXCdrTypeCode_getKind(tc->_typeCode) != RTI_XCDR_TK_NULL) {
                        memberCount++;
                        hasBase = RTI_XCDR_TRUE;
                        /**
                         * Coverity doesn't understand that hasBaseKey is not
                         * used to check for errors, but rather to determine if
                         * the base type has a key.
                         */
                        /* coverity[check_return : FALSE] */
                        hasBaseKey = RTIXCdrTypeCode_hasKey(tc->_typeCode);
                    }
                }
            }
            
            if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_UNION) {
                memberCount++;
                isUnion = RTI_XCDR_TRUE;

                if (property->v2Encapsulation) {
                    if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY ||
                            program->extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY) {
                        /* Skip the DHEADER */
                        if (!property->onlyKeyForKeyhash) {
                            program->unionInsIndex++;
                        }
                    }
                }

                if (RTIXCdrInterpreter_needMutableHeader(
                        program,
                        property->onlyKeyForKeyhash)) {
                    /* Skip the discriminator parameter header */
                    program->unionInsIndex++;
                }
            }
            
            memberCount += tc->_memberCount;
            instructionCount += memberCount;
            
            if (dheaderInNonPrimitiveCollections && kind != RTI_XCDR_TK_ENUM) {
                RTIXCdrUnsignedLong nonPrimitiveCollectionMemberCount =
                        RTIXCdrTypeCode_getNonPrimitiveCollectionMemberCount(
                                tc,
                                enumAsPrimitiveInCollections,
                                property->resolveAlias);
                /*
                 * Allocate potential space for DHEADER in sequence/array
                 * members.
                 */
                instructionCount += nonPrimitiveCollectionMemberCount;
            }

            if (RTIXCdrInterpreter_needMutableHeader(
                    program,
                    property->onlyKeyForKeyhash)) {
                /* We have to add memberCount because for mutable types
                 * each member has a header (one more instruction)
                 */
                instructionCount += memberCount;

                if (hasBase) {
                    /* For valuetype with base type we have to subtract
                     * one because the base member does not have header
                     */
                    instructionCount --;
                }

                if (!program->isCdr2) {
                    /* For the sentinel in V1 encapsulation */
                    instructionCount++;
                    sentinel = RTI_XCDR_TRUE;
                }
            } else {
                if (kind == RTI_XCDR_TK_VALUE || kind == RTI_XCDR_TK_STRUCT) {
                    /* We have to add one more instruction per optional member
                     * to process the header
                     */
                    instructionCount += RTIXCdrTypeCode_getOptionalMemberCount(tc);
                }
            }

            if (instructionCount == 0) {
                /* Empty structure */
                program->instructionCount = 0;
                break;
            }

            RTIXCdrHeap_allocateArray(
                    &program->instructions,
                    instructionCount,
                    RTIXCdrInstruction);

            if (program->instructions == NULL) {
                RTIXCdrLog_logTwoLong(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
                        1,
                        sizeof(RTIXCdrInstruction));
                loggedError = RTI_XCDR_TRUE;
                goto done;
            }
            
            for (instIndex=0; instIndex<instructionCount; instIndex++) {
                RTIXCdrInstruction_initialize(
                        &program->instructions[instIndex]);
            }

            hasKey = RTIXCdrTypeCode_hasKey(tc);
            instIndex = 0;
            
            if (dheader) {
                RTIXCdrInterpreter_generateDHeaderInstruction(
                        &program->instructions[0],
                        programKind,
                        RTI_XCDR_FALSE);
                instIndex++;
            }
            
            memberIndexArray = RTIXCdrInterpreter_createMemberIndexArray(
                    tc,
                    property->onlyKeyForKeyhash);
            if (memberIndexArray == NULL) {
                GotoDoneWithLine();
            }

            for (memberIndex=0; memberIndex<memberCount; memberIndex++) {
                const RTIXCdrTypeCode *memberTc = NULL;
                RTIXCdrTCKind memberTcKind = RTI_XCDR_TK_NULL;
                RTIXCdrOctet * opCodes = NULL;
                RTIXCdrBoolean tryMergeInstruction = RTI_XCDR_FALSE;
                RTIXCdrUnsignedLongLong byteCount = 0;
                RTIXCdrBoolean isSeq = RTI_XCDR_FALSE;
                RTIXCdrCommonInsParameters *commonParams = NULL;
                RTIXCdrInsParameters *params = NULL;
                struct RTIXCdrTypeCodeMember *member = NULL;
                RTIXCdrUnsignedLong tcMemberIndex = 0;
                RTIXCdrBoolean isPrimitiveMember = RTI_XCDR_FALSE;
                RTIXCdrUnsignedLongLong serSize = 0;
                RTIXCdrAlignment memberAlignment = 0;
                RTIXCdrBoolean countInBytes = RTI_XCDR_FALSE;
                RTIXCdrBoolean isOptional = RTI_XCDR_FALSE;
                RTIXCdrBoolean isPointer = RTI_XCDR_FALSE;
                RTIXCdrBoolean resolveAlias = RTI_XCDR_FALSE;
                const RTIXCdrTypeCode *aliasMemberTc = NULL;
                RTIXCdrTCKind aliasMemberTcKind = RTI_XCDR_TK_NULL;
                RTIXCdrBoolean isSeqSetMemberElementSet = RTI_XCDR_FALSE;
                RTIXCdrBoolean memberDHeader = RTI_XCDR_FALSE;
                RTIXCdrBoolean processingArray = RTI_XCDR_FALSE;
                RTIXCdrBoolean optimizeEnum = RTI_XCDR_FALSE;

                resolveAlias = property->resolveAlias;
                optimizeEnum = property->optimizeEnum;

                /* We have to initialize the instruction again because we
                 * may have merged this instruction in the previous iteration
                 */
                RTIXCdrInstruction_initialize(&program->instructions[instIndex]);
                
                if (isUnion || hasBase) {
                    if (memberIndex > 0) {
                        tcMemberIndex = memberIndexArray[memberIndex-1].memberIndex;
                    } else {
                        tcMemberIndex = RTI_XCDR_TYPECODE_INVALID_INDEX;
                    }
                } else {
                    tcMemberIndex = memberIndexArray[memberIndex].memberIndex;
                }

                if (tcMemberIndex != RTI_XCDR_TYPECODE_INVALID_INDEX
                        && !tc->_members[tcMemberIndex]._annotations._isSerializable) {
                    /* We have to ignore non serializable members */
                    /*
                     * Invalidate prevInstState to avoid merging instructions.
                     * We cannot merge two instructions if they do not
                     * correspond to two consecutive members
                     */
                    prevInstState.opcode = RTI_XCDR_INVALID_OPCODE;
                    continue;
                }
                
                if ((!isUnion && !hasBase) || memberIndex > 0) {
                    member = &tc->_members[tcMemberIndex];

                    isPointer = member->_representation._isPointer;

                    if ((!(member->_memberFlags & RTI_XCDR_KEY_MEMBER)
                            && property->onlyKey) && hasKey && !isUnion) {
                        continue;
                    }

                    if (kind == RTI_XCDR_TK_STRUCT ||
                            kind == RTI_XCDR_TK_VALUE) {
                        isOptional = RTIXCdrTypeCode_isOptionalMember(
                                        tc,
                                        tcMemberIndex);

                        if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                            /* The optional member header is not needed when
                             * working with mutable types
                             */
                            if (isOptional) {
                                RTIXCdrInterpreter_generateMemberHeaderInstruction(
                                        program,
                                        &program->instructions[instIndex],
                                        &tc->_members[tcMemberIndex],
                                        tc->_members[tcMemberIndex]._representation._typeCode);
                                RTIXCdrInstruction_storeState(
                                        &program->instructions[instIndex],
                                        &prevInstState,
                                        RTI_XCDR_FALSE, /* countInBytes */
                                        RTI_XCDR_FALSE /* memberDHeader */);
                                instIndex++;
                                RTIXCdrInstruction_initialize(
                                        &program->instructions[instIndex]);
                            }
                        }
                    }

                    memberTc = member->_representation._typeCode;
                } else {
                    /* With unions the first member is the discriminator
                     * With valuetypes with base types the first member is
                     * the base  type
                     */
                    member = NULL;
                    memberTc = tc->_typeCode;
                    if (isUnion) {
                        program->unionDiscKind =
                                RTIXCdrTypeCode_getKind(
                                        RTIXCdrTypeCode_resolveAlias(memberTc));

                        if (!RTIXCdrTypeCode_selectDefaultDiscriminator(
                                    program->typeCode,
                                    &program->defaultUnionDisc,
                                    RTIXCdrXTypesComplianceMask_isBitSet(
                                            RTIXCdrInterpreter_getGlobalXtypeComplianceMask(),
                                            RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT))) {
                            GotoDoneWithLine();
                        }

                        /* For unions, we always resolve the alias in a
                         * discriminator. This is done because the program
                         * execution does not work well with alias
                         * discriminators if the access to the underlying
                         * primitive type is based on index (this means
                         * the memory representation of the primitive type
                         * is different than the CDR representation)
                         */
                        resolveAlias = RTI_XCDR_TRUE;
                        /* Also for union disc, we always enable enum
                         * optimization. This will avoid that we end up with a
                         * complex instruction for the discriminator.
                         * The complex instruction would consider the value
                         * acceptUnknownEnumValue and would transform an
                         * unknown discriminator when what we really want is
                         * to leave the discriminator with its original value
                         * and take decisions based on the value of
                         * acceptUnknownUnionDiscriminator.
                         */
                        optimizeEnum = RTI_XCDR_TRUE;
                    }

                    if (hasKey && !hasBaseKey && property->onlyKey) {
                        continue;
                    }
                }

                memberTcKind = RTIXCdrTypeCode_getKind(memberTc);

                if (RTIXCdrInterpreter_needMutableHeader(
                        program,
                        property->onlyKeyForKeyhash)) {
                    if (!hasBase || memberIndex>0) {
                        if (isUnion && memberIndex == 0) {
                            RTIXCdrInterpreter_generateMemberHeaderInstruction(
                                    program,
                                    &program->instructions[instIndex],
                                    NULL,
                                    tc->_typeCode);
                        } else {
                            RTIXCdrInterpreter_generateMemberHeaderInstruction(
                                    program,
                                    &program->instructions[instIndex],
                                    &tc->_members[tcMemberIndex],
                                    tc->_members[tcMemberIndex]._representation._typeCode);
                        }
                        RTIXCdrInstruction_storeState(
                                &program->instructions[instIndex],
                                &prevInstState,
                                RTI_XCDR_FALSE, /* countInBytes */
                                RTI_XCDR_FALSE /* memberDHeader */);
                        instIndex++;
                        RTIXCdrInstruction_initialize(
                                &program->instructions[instIndex]);
                    }
                }

                if (resolveAlias) {
                    /* We resolve aliases until we find an alias that is a
                     * pointer. For example:
                     *
                     * typedef long * m1;
                     *
                     * In this case we cannot resolve that alias
                     */
                    aliasMemberTc =
                            RTIXCdrTypeCode_resolveAliasUntilPointer(memberTc);
                    memberTcKind = RTIXCdrTypeCode_getKind(aliasMemberTc);
                    
                    if (memberTcKind == RTI_XCDR_TK_ARRAY 
                            && (isOptional 
                                    || isPointer
                                    || dheaderInNonPrimitiveCollections)) {
                        /*
                         * Aliases cannot be resolved for optional members
                         * with an alias type that is an array.
                         *
                         * This is because optional array members are not
                         * currently supported in some languages such as
                         * C and C++.
                         *
                         * Also aliases cannot be resolved if we have to add
                         * DHEADER in non-primitive collection members. 
                         * For example:
                         *
                         * typedef string MyArr[2];
                         *
                         * struct MyType {
                         *     MyArr m1[2];
                         * }
                         *
                         * If we were going to resolve MyArr and replace it
                         * with string[2] the generated program would consider
                         * the member as an array of 4 strings instead and we
                         * would have a single DHEADER for the whole member.
                         */
                        resolveAlias = RTI_XCDR_FALSE;
                    } else {
                        memberTc = aliasMemberTc;
                    }
                }

                memberTcKind = RTIXCdrTypeCode_getKind(memberTc);

                /*
                 * CORE-12464: We need dheader for sequences/arrays of 
                 * non-primitive members if encapsulation is V2.
                 */
                if (dheaderInNonPrimitiveCollections) {
                    RTIBool useSampleAccessor = RTIXTypeCode_useSampleAccessor(
                            memberTc,
                            isOptional);

                    /* 
                     * The usage of useSampleAccessor below is to avoid adding
                     * a DHEADER for an array member twice.
                     *
                     * When arrays have a sample accessor we will treat the 
                     * member as a complex member and we will generate a 
                     * program for the array type. This is the program
                     * that should add the DHEADER for the ARRAY member.
                     */
                    if (kind == RTI_XCDR_TK_ARRAY
                            || kind == RTI_XCDR_TK_SEQUENCE
                            || (memberTcKind == RTI_XCDR_TK_ARRAY && !useSampleAccessor)
                            || memberTcKind == RTI_XCDR_TK_SEQUENCE) {
                        RTIXCdrTypeCode *elementTc;

                        if (kind == RTI_XCDR_TK_ARRAY 
                                || kind == RTI_XCDR_TK_SEQUENCE) {
                            elementTc = tc->_typeCode;
                        } else {
                            elementTc = memberTc->_typeCode;
                        }

                        aliasMemberTc = RTIXCdrTypeCode_resolveAlias(elementTc);
                        aliasMemberTcKind =
                                RTIXCdrTypeCode_getKind(aliasMemberTc);

                        if (!RTIXCdrTypeCode_isPrimitiveKind(
                                aliasMemberTcKind,
                                enumAsPrimitiveInCollections)) {
                            RTIXCdrInterpreter_generateDHeaderInstruction(
                                    &program->instructions[instIndex],
                                    programKind,
                                    RTI_XCDR_TRUE);
                            instIndex++;
                            memberDHeader = RTI_XCDR_TRUE;
                        }
                    }
                }

                opCodes = NULL;
                isSeq = RTI_XCDR_FALSE;
                isSeqSetMemberElementSet = RTI_XCDR_FALSE;
                serSize = 0;
                instruction = &program->instructions[instIndex];

                params = &instruction->params;
                commonParams = RTIXCdrInstruction_getCommonParams(instruction);

                if (kind == RTI_XCDR_TK_ARRAY 
                        || memberTcKind == RTI_XCDR_TK_ARRAY) {
                    processingArray = RTI_XCDR_TRUE;
                }
                
                if (isPointer) {
                    commonParams->refMemberKind =
                            RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER;
                } else if (isOptional) {
                    commonParams->refMemberKind =
                            RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER;
                } else {
                    commonParams->refMemberKind =
                            RTI_XCDR_INTERPRETER_VALUE_MEMBER;
                }
                
                /* 
                 * We may have a NULL sampleAccessInfo if we are generating the
                 * get_max/min_ser_size programs. We do not require the
                 * sampleAccessInfo or memberAccessInfo in these cases because
                 * these programs do not require accessing a sample of the
                 * type in order to calculate the size, they only require the
                 * typecode. The calculation of the max_min_ser_sizes is
                 * independent of any language binding specific sample
                 * access/offset information.
                 */
                if (tc->_sampleAccessInfo != NULL) {
                    if (hasBase && memberIndex == 0) {
                        RTIXCdrBoolean emptyTc;
                        
                        /* Assign offsets for the base member */
                        /* For non flat data language binding the offset will be 0
                         * For flat data the offsets will be the offsets of the
                         * first member of the base
                         */
                        RTIXCdrInterpreter_assignBaseMemberValueOffset(
                                tc,
                                &emptyTc,
                                instruction);
                        commonParams->memberAccessInfo.skipDeserialization = RTI_XCDR_FALSE;
                    } else {
                        if (hasBase) {
                            /* 
                             * There should always be memberAccessInfos for
                             * structs with a base. The only time we expect
                             * that there may not be memberAccessInfos is
                             * arrays
                             */
                            RTIXCdrLog_testPrecondition(
                                    tc->_sampleAccessInfo->memberAccessInfos == NULL,
                                    GotoDoneWithLine());
                            commonParams->memberAccessInfo = tc->_sampleAccessInfo->memberAccessInfos[tcMemberIndex];
                        } else {
                            if (tc->_sampleAccessInfo->memberAccessInfos != NULL) {
                                if (isUnion) {
                                    /* Union are ordered by member ID. This is why
                                     * we use directly memberIndex
                                     */
                                    commonParams->memberAccessInfo = tc->_sampleAccessInfo->memberAccessInfos[memberIndex];
                                } else {
                                    commonParams->memberAccessInfo = tc->_sampleAccessInfo->memberAccessInfos[tcMemberIndex];
                                }
                            }
                        }
                    }
                }
                
                if (RTIXCdrTypeCode_isAggregationKind(kind)) {
                    /* 
                     * Alias, sequences, array, and enums cannot provide
                     * tcMemberInfo because the member in the tc is artificial
                     * and lives on the stack. See singleMemberTc above
                     */
                    if ((!isUnion && !hasBase) || memberIndex > 0) {
                        commonParams->tcMemberInfo = &tc->_members[tcMemberIndex];
                    }
                }
                
                if (RTIXCdrTypeCode_isPrimitiveKind(memberTcKind, RTI_XCDR_TRUE)
                        && tcMemberIndex != RTI_XCDR_TYPECODE_INVALID_INDEX) {
                    /* We have to determine if we have to check the range */
                    params->primitiveParams.checkRange =
                            RTIXCdrInterpreter_needPrimitiveRangeCheck(
                                program->kind,
                                &tc->_members[tcMemberIndex]);
                }

                if (kind == RTI_XCDR_TK_ARRAY) {
                    commonParams->count = 
                        RTIXCdrTypeCode_getArrayElementCount(
                                tc);
                } else {
                    commonParams->count = 1;
                }

                /* 
                 * Arrays configuring the sample accessor are treated like 
                 * aliases/enums in that we generate a complex instruction and 
                 * program.
                 * 
                 * Therefore we keep the memberTc and memberTcKind as-is so that
                 * we will trigger generation of the complex instruction.
                 *
                 * Without sample accessor:
                 *
                 * 1) If we dont have have to serialize dhheaders in 
                 * non-primitive collections we will flatten out nested 
                 * arrays to look like a single array. For example:
                 *
                 * typedef Foo MyArray[2];
                 *
                 * struct {
                 *     MyArray m1[2];
                 * }
                 *
                 * The previous type will be treated as a single array of 4
                 * Foo.
                 *
                 * 2) However, if we have to serialize dhheaders in
                 * non-primitive collections we will not merge arrays because
                 * we would lose the information about the number of arrays and
                 * the ability to add one DHEADER per array.
                 */
                if (memberTcKind == RTI_XCDR_TK_ARRAY 
                        && !RTIXTypeCode_useSampleAccessor(
                                memberTc,
                                isOptional)) {
                    RTIXCdrUnsignedLongLong arrCount = commonParams->count;

                    if (!dheaderInNonPrimitiveCollections) {
                        /* 
                         * We merge arrays if we dont have to add DHEADER in
                         * non-primitive collections 
                         */
                        while (memberTcKind == RTI_XCDR_TK_ARRAY) {
                            arrCount *= RTIXCdrTypeCode_getArrayElementCount(
                                    memberTc);
                            
                            if (arrCount > RTIXCdrLong_MAX ||
                                    arrCount == 0) {
                                logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_ss;
                                GotoDoneWithLine();
                            }
                            
                            if (resolveAlias) {
                                memberTc =
                                        RTIXCdrTypeCode_resolveAliasUntilPointer(
                                                memberTc->_typeCode);
                            } else {
                                memberTc = memberTc->_typeCode;
                            }
                            
                            memberTcKind = RTIXCdrTypeCode_getKind(memberTc);
                        }
                    } else if (kind != RTI_XCDR_TK_ARRAY) {
                        /* 
                         * We cannot merge arrays because we would lose the
                         * information about the number of arrays and the 
                         * ability to add one DHEADER per array.
                         */
                        arrCount *= RTIXCdrTypeCode_getArrayElementCount(
                                memberTc);
                        memberTc = memberTc->_typeCode;
                        memberTcKind = RTIXCdrTypeCode_getKind(memberTc);
                    } 
                    /* } else {
                     *   If kind == RTI_XCDR_TK_ARRAY then we are working with 
                     *   a program for an array. This program has a single member 
                     *   whose type is memberTc. Since 
                     *   dheaderInNonPrimitiveCollections is set to TRUE we will
                     *   treat this member as a complex member and generate and
                     *   program for it.
                     * }
                     */

                    commonParams->count = (RTIXCdrUnsignedLong) arrCount;
                }

                commonParams->memberTc = (RTIXCdrTypeCode *) memberTc;
                commonParams->useGetMemberValue = RTIXCdrProgram_useGetMemberValueInMember(
                        memberTc,
                        commonParams->refMemberKind !=
                                RTI_XCDR_INTERPRETER_VALUE_MEMBER);
                countInBytes = RTI_XCDR_FALSE;

                if (memberTcKind == RTI_XCDR_TK_SEQUENCE) {
                    isSeq = RTI_XCDR_TRUE;

                    if ((memberTc->_sampleAccessInfo != NULL &&
                                memberTc->_sampleAccessInfo->setMemberElementValueFcn != NULL)) {
                        isSeqSetMemberElementSet = RTI_XCDR_TRUE;
                    }
                    
                    if (property->resolveAlias ||
                            /* AliasSeq ser/deser not supported when setting setMemberElementValueFcn
                             *
                             * We run into this problem with modern C++
                             */
                            isSeqSetMemberElementSet) {
                        aliasMemberTc = RTIXCdrTypeCode_resolveAliasUntilPointer(
                                memberTc->_typeCode);
                        memberTcKind = RTIXCdrTypeCode_getKind(aliasMemberTc);

                        if (memberTcKind == RTI_XCDR_TK_ARRAY) {
                            /* Aliases cannot be resolved for sequence elements
                             * that are arrays.
                             *
                             * Currently, we do not generate
                             * sample access info for these elements
                             *
                             * In general, aliases cannot be resolved for
                             * constructs that cannot be expressed in IDL
                             * without aliases.
                             *
                             * For example:
                             *
                             * typedef long MyArray[2]
                             *
                             * sequence<MyArray,2> member;
                             *
                             * In the previous example we cannot resolve
                             * MyArray because the following is not legal IDL
                             *
                             * sequence<long[2],2> member
                             */
                            memberTc = memberTc->_typeCode;
                        } else {
                            memberTc = aliasMemberTc;
                        }
                    } else {
                        memberTc = memberTc->_typeCode;
                    }

                    memberTcKind = RTIXCdrTypeCode_getKind(memberTc);
                    commonParams->seqElementTc = (RTIXCdrTypeCode *) memberTc;
                }

                if (RTIXCdrInterpreter_isInlineAllowed(
                        program,
                        (memberIndex==0 && hasBase),
                        instruction,
                        isSeq,
                        swapPrimitiveValues,
                        property))
                {
                    if (RTIXCdrTypeCode_hasCFriendlyCdrLayout(
                            memberTc,
                            &serSize,
                            &memberAlignment,
                            /* For sequences the array [] determines how many
                               sequences not how many sequence elements. This is
                               why we provide 1 as the size
                            */
                            isSeq?1:commonParams->count,
                            property->v2Encapsulation,
                            dheaderInNonPrimitiveCollections,
                            enumAsPrimitiveInCollections))
                    {
                        if (!isSeq
                                /* No gapping between sequence elements */
                                || ((serSize % (RTIXCdrUnsignedLongLong) memberAlignment) == 0
                                        && !isSeqSetMemberElementSet)) {
                            if (property->forceDependentPrograms) {
                                /*
                                * We are inlining the member within its enclosing
                                * type's program, but still need to generate the
                                * program for this member so that it can be found in
                                * the dependent program list
                                */
                                RTIXCdrProgram *tmpProgram = RTIXCdrInterpreter_generateTypePluginProgram(
                                        memberTc,
                                        dependentProgramList,
                                        programKind,
                                        property);
                                if (tmpProgram == NULL) {
                                    RTIXCdrLog_logTwoStr(
                                            RTI_XCDR_LOG_EXCEPTION,
                                            RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss,
                                            RTIXCdrProgramKind_toStr(programKind),
                                            memberTc->_name);
                                    loggedError = RTI_XCDR_TRUE;
                                    goto done;
                                }
                            }

                            memberTcKind = RTIXCdrInterpreter_alignmentToPrimitiveKind(
                                    memberAlignment);
                            countInBytes = RTI_XCDR_TRUE;

                            if (serSize > RTIXCdrLong_MAX) {
                                logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
                                GotoDoneWithLine();
                            }

                            if (!isSeq) {
                                commonParams->count = (RTIXCdrUnsignedLong)serSize;
                            }
                        }
                    }
                }

                if (RTIXCdrInterpreter_isPrimitiveKind(tc, 
                        memberTcKind, 
                        optimizeEnum)) {
                    isPrimitiveMember = RTI_XCDR_TRUE;
                } else {
                    isPrimitiveMember = RTI_XCDR_FALSE;
                }

                if (isSeq) {
                    if (isPrimitiveMember) {
                        opCodes = RTIXCdrInterpreter_g_primitiveSeqInstOpCode;
                    } else if (memberTcKind == RTI_XCDR_TK_STRING) {
                        opCodes = RTIXCdrInterpreter_g_stringSeqInstOpCode;
                    } else if (memberTcKind == RTI_XCDR_TK_WSTRING) {
                        opCodes = RTIXCdrInterpreter_g_wstringSeqInstOpCode;
                    } else {
                        opCodes = RTIXCdrInterpreter_g_complexSeqInstOpCode;
                    }

                    if (memberDHeader && processingArray) {
                        aliasMemberTc = RTIXCdrTypeCode_resolveAlias(
                                /* memberTc is the SEQUENCE element TC */
                                memberTc);
                        aliasMemberTcKind =
                                RTIXCdrTypeCode_getKind(aliasMemberTc);

                        if (!RTIXCdrTypeCode_isPrimitiveKind(
                                aliasMemberTcKind,
                                enumAsPrimitiveInCollections)) {
                            /*
                             * The DHEADER(s) for an array of sequences are
                             * added as follows:
                             * - For the array, the DHEADER is added using a
                             * COLLECTION_DHEADER instruction.
                             * - For the sequences we are setting the field
                             * addSeqDHeader to TRUE in commonParams. We will
                             * have to add one DHEADER per sequence during
                             * program execution.
                             *
                             * Note that this is needed because arrays do not
                             * get its own instruction. They just increase
                             * commonParams->count.
                             */
                            commonParams->addSeqDHeader = RTI_XCDR_TRUE;
                        }
                    }
                }
                
                if (isPrimitiveMember) {
                    if (opCodes == NULL) {
                        opCodes = RTIXCdrInterpreter_g_primitiveInstOpCode;
                        tryMergeInstruction = RTI_XCDR_TRUE;
                    }
                } else if (memberTcKind == RTI_XCDR_TK_STRING) {
                    if (opCodes == NULL) {
                        opCodes = RTIXCdrInterpreter_g_stringInstOpCode;
                    }

                    if (memberTc->_maximumLength >= program->unboundedSize) {
                        /* Unbounded */
                        params->strParams.charMaxCount = RTIXCdrLong_MAX;
                    } else {
                        params->strParams.charMaxCount = memberTc->_maximumLength+1;
                        
                        if (memberTc->_maximumLength >
                            (RTIXCdrLong_MAX - RTI_XCDR_FOUR_BYTE_SIZE - 1)) {
                            logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus;
                            GotoDoneWithLine();
                        }
                    }

                    params->strParams.charSize =
                            RTI_XCDR_ONE_BYTE_SIZE;
                    params->strParams.charAlignment =
                            RTI_XCDR_ONE_BYTE_ALIGNMENT;
                } else if (memberTcKind == RTI_XCDR_TK_WSTRING) {
                    if (opCodes == NULL) {
                        opCodes = RTIXCdrInterpreter_g_wstringInstOpCode;
                    }

                    if (property->v2Encapsulation) {
                        params->strParams.charAlignment =
                                RTI_XCDR_WCHAR_ALIGNMENT;
                        params->strParams.charSize =
                                RTI_XCDR_WCHAR_SIZE;
                    } else {
                        params->strParams.charAlignment =
                                RTI_XCDR_LEGACY_WCHAR_ALIGNMENT;
                        params->strParams.charSize =
                                RTI_XCDR_LEGACY_WCHAR_SIZE;
                    }

                    if (memberTc->_maximumLength >= program->unboundedSize) {
                        /* Unbounded */
                        params->strParams.charMaxCount = RTIXCdrLong_MAX;
                        byteCount = RTIXCdrLong_MAX;
                    } else {
                        params->strParams.charMaxCount = memberTc->_maximumLength + 1;
                        byteCount = ((RTIXCdrUnsignedLongLong)memberTc->_maximumLength+1) *
                            params->strParams.charSize;
                        if (byteCount > (RTIXCdrLong_MAX - RTI_XCDR_FOUR_BYTE_SIZE)) {
                            logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus;
                            GotoDoneWithLine();
                        }
                    }
                } else {
                    if (opCodes == NULL) {
                        opCodes = RTIXCdrInterpreter_g_complexInstOpCode;
                    }
                    
                    if (hasBase && memberIndex==0) {
                        params->complexParams.baseClass = RTI_XCDR_TRUE;
                        baseTypeInstIndex = instIndex;
                    } else {
                        params->complexParams.baseClass = RTI_XCDR_FALSE;                        
                    }
                    
                    if (property->inlineStruct) {
                        /* When inlineStruct is set to TRUE we assume that the
                         * user does not care about intercepting typePlugin
                         * calls. This is why we assign
                         * params->complexParams.typePlugin to NULL
                         *
                         * We dont do that for non aggregation types because
                         * inlineStruct only refers to struct/union
                         */
                        if (RTIXCdrTypeCode_isAggregationKind(memberTcKind)) {
                            params->complexParams.typePlugin = NULL;
                        } else if (program->kind == RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM ||
                                program->kind == RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM) {
                            /* For get max size serialize or get min size
                             * serialize we need to nullify the typePlugin
                             * because the program will be run at the end of
                             * this function and if a typePlugin was assigned
                             * running the program would require an endpoint
                             * data which we do not have as it is defined in
                             * PRES
                             */
                            params->complexParams.typePlugin = NULL;
                        } else {
                            params->complexParams.typePlugin =
                                    memberTc->_typePlugin;
                        }
                    } else {
                        params->complexParams.typePlugin = 
                                memberTc->_typePlugin;
                    }
                }

                result = RTIXCdrInterpreter_generateInstructionOpcode(
                        instruction,
                        opCodes,
                        &swapPrimitiveValues,
                        hasKey,
                        hasBaseKey,
                        commonParams->memberAccessInfo.skipDeserialization,
                        member,
                        programKind);

                if (!result) {
                    RTIXCdrLog_logStr(
                            RTI_XCDR_LOG_EXCEPTION,
                            RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                            "stream program");
                    loggedError = RTI_XCDR_TRUE;
                    goto done;
                }
                
                if (RTIXCdrInstruction_isComplexOpcode(instruction->opcode)) {
                    RTIXCdrTypePluginProgramProperty nestedProperty =
                            *property;

                    if (RTIXCdrTypeCode_isAggregationKind(kind)) {
                        if (property->onlyKey 
                                && !property->v2Encapsulation
                                && !hasKey) {
                            /*
                             * In V1, the serialization of the key for an
                             * unkeyed type (Type2 below) is equivalent to 
                             * serializing the whole type independently of 
                             * whether or not the nested types (Type1 below) 
                             * are keyed or unkeyed.
                             *
                             * For example:
                             *
                             *   struct Type1 {
                             *       @key long m1;
                             *       long m2;
                             *   };
                             *
                             *   struct Type2 {
                             *       Type1 m1;
                             *   };
                             *
                             *   struct Type3 {
                             *       @key Type2 m1;
                             *   };
                             *
                             * In the previous example the key for Type3 
                             * with XCDR1 data representation is: 
                             * {m1.m1.m1, m1.m1.m2}
                             *
                             * For XCDR2 the key is: {m1.m1.m1}
                             *
                             * For V1, if the current type does not have key 
                             * members we disable the onlyKey flag for the 
                             * nested members.
                             *
                             * Note that we only apply this optimization when
                             * the current type is a struct, union, or valuetype
                             * because other types such as ARRAYS or ALIAS 
                             * inherit must propagate their keyed attribute to
                             * their nexted types.
                             */
                            nestedProperty.onlyKey = RTI_XCDR_FALSE;
                        }
                    }

                    if (programKind == RTI_XCDR_SER_TO_KEY_PROGRAM ||
                            programKind == RTI_XCDR_DESER_PROGRAM) {
                        if (instruction->opcode == RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE ||
                                instruction->opcode == RTI_XCDR_SKIP_COMPLEX_OPCODE) {
                            params->complexParams.program =
                                    RTIXCdrInterpreter_generateTypePluginProgram(
                                            memberTc,
                                            dependentProgramList,
                                            RTI_XCDR_SKIP_PROGRAM,
                                            &nestedProperty);
                        } else {
                            params->complexParams.program =
                                    RTIXCdrInterpreter_generateTypePluginProgram(
                                            memberTc,
                                            dependentProgramList,
                                            programKind,
                                            &nestedProperty);
                        }
                    } else {
                        params->complexParams.program =
                                RTIXCdrInterpreter_generateTypePluginProgram(
                                        memberTc,
                                        dependentProgramList,
                                        programKind,
                                        &nestedProperty);
                    }

                    if (params->complexParams.program == NULL) {
                        RTIXCdrLog_logTwoStr(
                                RTI_XCDR_LOG_EXCEPTION,
                                RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss,
                                RTIXCdrProgramKind_toStr(programKind),
                                tc->_name);
                        loggedError = RTI_XCDR_TRUE;
                        goto done;
                    }
                }

                if (RTIXCdrInstruction_isPrimitiveOpcode(instruction->opcode)) {
                    params->primitiveParams.mustAlign = RTI_XCDR_TRUE;
                }

                if (isPrimitiveMember) {
                    params->primitiveParams.primitiveKind =
                            memberTcKind;
                    RTIXCdrTypeCode_getPrimitiveInfo(
                            memberTcKind,
                            property->v2Encapsulation,
                            &params->primitiveParams.primitiveAlignment,
                            &params->primitiveParams.primitiveSize);

                    if (!isSeq) {
                        if (countInBytes) {
                            byteCount = commonParams->count;
                        } else {
                            byteCount = commonParams->count
                                    * (RTIXCdrUnsignedLongLong) params
                                              ->primitiveParams.primitiveSize;
                        }

                        if (byteCount > RTIXCdrLong_MAX) {
                            logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss;
                            GotoDoneWithLine();
                        }
                        params->primitiveParams.primitiveByteCount =
                                (RTIXCdrUnsignedLong)byteCount;
                    } else {
                        if (serSize != 0) {
                            params->primitiveParams.primitiveByteCount =
                                (RTIXCdrUnsignedLong)serSize;
                        } else {
                            params->primitiveParams.primitiveByteCount =
                                (RTIXCdrUnsignedLong)
                                        params->primitiveParams.primitiveSize;
                        }
                    }

                    if (memberTcKind == RTI_XCDR_TK_ENUM)
                    {
                        RTIXCdrExtensibilityKind extensibilityKind =
                                RTIXCdrTypeCode_getExtensibilityKind(tc);

                        if (instruction->opcode == RTI_XCDR_DESER_PRIMITIVE_OPCODE
                                || instruction->opcode == RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE) {
                            if (extensibilityKind != RTI_XCDR_FINAL_EXTENSIBILITY) {
                                /* When we have to check the value of an enum it does
                                 * not make sense to deserialize together in a single
                                 * memcpy
                                 */
                                params->primitiveParams.enumTc =
                                        (RTIXCdrTypeCode *) memberTc;
                            }
                        } else if (instruction->opcode == RTI_XCDR_SER_PRIMITIVE_OPCODE
                                || instruction->opcode == RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE) {
                            params->primitiveParams.enumTc =
                                    (RTIXCdrTypeCode *) memberTc;
                        }
                    }

                    if (isSeq) {
                        if (commonParams->memberTc->_maximumLength
                                < program->unboundedSize) {
                            byteCount = commonParams->memberTc->_maximumLength *
                                (RTIXCdrUnsignedLongLong)
                                params->primitiveParams.primitiveSize;
                            if (byteCount > RTIXCdrLong_MAX) {
                                logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss;
                                GotoDoneWithLine();
                            }
                        }
                    }

                    params->primitiveParams.origPrimitiveSize =
                            params->primitiveParams.primitiveSize;
                }

                if (tryMergeInstruction) {
                    RTIXCdrOctet primitiveSize =
                            params->primitiveParams.primitiveSize;

                    if (RTIXCdrInterpreter_treatPrimitiveAsOctets(
                            program,
                            instruction,
                            swapPrimitiveValues)) {
                        if (countInBytes) {
                            byteCount = commonParams->count;
                        } else {
                            byteCount = (RTIXCdrUnsignedLongLong)primitiveSize *
                                    commonParams->count;
                        }

                        if (byteCount > RTIXCdrLong_MAX) {
                            logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss;
                            GotoDoneWithLine();
                        }
                        commonParams->count =
                                (RTIXCdrUnsignedLong) byteCount;
                        params->primitiveParams.primitiveSize =
                                RTI_XCDR_ONE_BYTE_SIZE;
                        params->primitiveParams.primitiveByteCount =
                                commonParams->count;
                    }

                    /* No need to align a primitive member if the previous
                     * primitive member has an alignment greater than the
                     * current member alignment.
                     *
                     * For unions, we will always align a primitive member
                     * because we can select only one member.
                     * 
                     * We will always align members if the previous member
                     * is optional
                     */
                    params->primitiveParams.mustAlign = 
                            RTIXCdrInstruction_mustAlign(
                                    program,
                                    instruction,
                                    &prevInstState);

                    if (RTIXCdrInterpreter_isInstructionMergeable(
                            program,
                            instruction,
                            &prevInstState,
                            swapPrimitiveValues,
                            tcMemberIndex,
                            property->onlyKey,
                            memberDHeader)) {
                        /* Reuse previous instruction and increase primitive
                         * count.
                         */
                        byteCount =
                                program->instructions[instIndex-1].params.primitiveParams.parent.count
                                + commonParams->count;

                        if (byteCount > RTIXCdrLong_MAX) {
                            logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss;
                            GotoDoneWithLine();
                        }

                        program->instructions[instIndex-1].params.primitiveParams.parent.count =
                                (RTIXCdrUnsignedLong)byteCount;
                        program->instructions[instIndex-1].params.primitiveParams.primitiveByteCount =
                                (RTIXCdrUnsignedLong)byteCount;
                    } else {
                        instIndex++;
                    }
                } else if (memberTcKind == RTI_XCDR_TK_STRING ||
                           memberTcKind == RTI_XCDR_TK_WSTRING ||
                           isSeq) {
                    if (!swapPrimitiveValues
                            && isPrimitiveMember &&
                            (memberTcKind != RTI_XCDR_TK_WCHAR ||
                                    program->isCdr2 ||
                                    (instruction->opcode != RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE &&
                                    instruction->opcode != RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE))) {
                        /* When we don't have to swap values we serialize
                         * primitive sequences as array of OCTETS.
                         */
                        params->primitiveParams.primitiveSize =
                                RTI_XCDR_ONE_BYTE_SIZE;
                    }

                    if (RTIXCdrInterpreter_isInstructionMergeable(
                            program,
                            instruction,
                            &prevInstState,
                            swapPrimitiveValues,
                            tcMemberIndex,
                            property->onlyKey,
                            memberDHeader)) {
                        program->instructions[instIndex-1].params.strParams.parent.count+=
                                commonParams->count;
                    } else {
                        instIndex++;
                    }
                } else {
                    instIndex++;
                }

                RTIXCdrInstruction_storeState(
                        instruction,
                        &prevInstState,
                        countInBytes,
                        memberDHeader);
                swapPrimitiveValues = globalSwapPrimitiveValues;
            }

            if (sentinel) {
                RTIXCdrInterpreter_generateSentinelInstruction(
                        &program->instructions[instIndex],
                        programKind);
                instIndex++;
            }

            program->instructionCount = instIndex;
        } break;
        default:
        goto done;
    }

    if (program->instructionCount > 0) {
        if (RTIXCdrInterpreter_isProgramMemberIdIndexRequired(program)) {
            program->instructionIndex = RTIXCdrInstructionIndex_new(
                    program,
                    RTI_XCDR_MEMBER_ID_INDEX_KIND);
            if (program->instructionIndex == NULL) {
                RTIXCdrLog_logTwoStr(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                        "program member ID index",
                        program->typeCode->_name);
                goto done;
            }

            if (program->hasBase) {
                if (!RTIXCdrInterpreter_mergeBaseProgramMemberIdIndex(
                            program,
                            baseTypeInstIndex)) {
                    RTIXCdrLog_logTwoStr(
                            RTI_XCDR_LOG_EXCEPTION,
                            RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                            "program base member ID index entries",
                            program->typeCode->_name);
                    goto done;
                }
            }
        }

        if (RTIXCdrInterpreter_isProgramLabelIndexRequired(program)) {
            program->instructionIndexByLabel = RTIXCdrInstructionIndex_new(
                    program,
                    RTI_XCDR_LABEL_INDEX_KIND);
            if (program->instructionIndexByLabel == NULL) {
                RTIXCdrLog_logTwoStr(
                        RTI_XCDR_LOG_EXCEPTION,
                        RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
                        "program label index",
                        program->typeCode->_name);
                goto done;
            }
        }
    }

    program->isFastSerializationSupported =
            RTIXCdrProgram_isFastSerializationSupported(program);

    if (!RTIXCdrInterpreter_optimizeMaxMinSerSizeProgram(
            program,
            property)) {
        logMessageId = RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss;
        GotoDoneWithLine();
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (memberIndexArray != NULL) {
        RTIXCdrHeap_freeArray(memberIndexArray);
    }

    if (!ok) {
        if (!loggedError) {
            RTIXCdrInterpreter_logProgramGenerationError(
                    tc,
                    instruction,
                    programKind,
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
