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
#include "InstructionIndex.h"
#include "xcdr/xcdr_interpreter.h"

/*
 * This macro returns both the next instruction index and the new
 * instructionCount.
 *
 * The instructionCount has to be updated to finish the for loop that iterates
 * through the instructions after the union active member is processed.
 *
 * The instIndex stores the index of the instruction for processing the data
 * member if a member exists that is identified by caseValue. This macro
 * adjusts that index backwards to account for any header
 * instructions that also need to be executed. If no member exists that is
 * identified by caseValue, then instIndex stores the index of the next
 * instruction to run, if there is one. For mutable XCDR1 unions, a sentinel
 * must be serialized. In all other cases, no more instructions should be run.
 *
 * The outcome will be that instructionCount number of instructions will
 * be executed, starting at index insIndex.
 */
#define RTIXCdrProgram_getUnionMemberInstructionIndex( \
        program, \
        insIndex, \
        instructionCount, \
        caseValueIdentifiesMember__, \
        caseValue) \
    { \
        RTIXCdrUnsignedLong instructionCount__ = 1; \
        RTIXCdrInstructionIndex_getInstructionIndexByLabel( \
                (program)->instructionIndexByLabel, \
                (insIndex), \
                (caseValue)); \
        if (*(insIndex) != RTI_XCDR_TYPECODE_INVALID_INDEX) { \
            /* \
             * Subtract up to two to account for collection dheader and \
             * mutable member header \
             */ \
            if (RTIXCdrInstruction_isHeaderOpcode( \
                        program->instructions[*(insIndex) -1].opcode)) { \
                (*(insIndex))--; \
                instructionCount__++; \
                if (RTIXCdrInstruction_isHeaderOpcode( \
                            program->instructions[*(insIndex) -1].opcode)) { \
                    (*(insIndex))--; \
                    instructionCount__++; \
                } \
            } \
            if ((program)->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) { \
                if (!(program)->isCdr2) { \
                    /* We add 1 to account for sentinel */ \
                    instructionCount__++; \
                } \
            } \
            *(instructionCount) = *(insIndex) + instructionCount__; \
            if ((caseValueIdentifiesMember__) != NULL) { \
                *(caseValueIdentifiesMember__) = RTI_TRUE; \
            } \
        } else { \
            /* \
             * The label was not found so there are no instructions to \
             * execute, unless we need to serialize the sentinel, which is the \
             * last instruction. \
             */ \
            if ((((program)->xTypesComplianceMask \
                  & RTI_XCDR_XTYPES_SENTINEL_IN_EMPTY_UNION_BIT) \
                 != 0) \
                && ((program)->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) \
                && !(program)->isCdr2) { \
                *(insIndex) = *(instructionCount) -1; \
            } else { \
                /* \
                 * Setting insIndex equal to instructionCount will prevent the \
                 * program for loop from executing any more iterations. \
                 */ \
                *(insIndex) = *(instructionCount); \
            } \
            if ((caseValueIdentifiesMember__) != NULL) { \
                *(caseValueIdentifiesMember__) = RTI_FALSE; \
            } \
        } \
    }

/*****************************************************************************/
/***** Serialize *************************************************************/
/*****************************************************************************/

RTI_PRIVATE
void RTIXCdrInterpreter_logSerializationError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(commonParams == NULL, return);

    switch (messageId) {
    case RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
    case RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
    {
        RTIXCdrLogParam param[4];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction, 
                tc);
        param[2].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[2].value.ulVal = runTimeParam->value.ulVal;
        param[3].kind = RTI_XCDR_LOG_ULONG_PARAM;

        if (messageId == RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu) {
            param[3].value.ulVal = instruction->params.strParams.charMaxCount-1;
        } else {
            param[3].value.ulVal = commonParams->memberTc->_maximumLength;
        }

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                4,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd:
    {
        RTIXCdrLogParam param[3];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction, 
                tc);
        param[2].kind = RTI_XCDR_LOG_LONG_PARAM;
        param[2].value.lVal = runTimeParam->value.lVal;

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                3,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss:
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
                messageId,
                2,
                param);
    } break;
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
                RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_serializeString(
        const struct RTIXCdrProgram *program,
        RTIXCdrStream *stream,
        const struct RTIXCdrInstruction *instruction,
        void *sample,
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i,j;
    RTIXCdrUnsignedLong stringCount;
    RTIXCdrUnsignedLong elementCount;
    RTIXCdrMemberValue memberValue;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    struct RTIXCdrTypeCode *strTc;
    const struct RTIXCdrSampleAccessInfo *strSampleAccessInfo;
    RTIXCdrUnsignedLongLong elOffset, strOffset, externalStrOffset;
    struct RTIXCdrTypeCodeMember *tcMember;
    RTIXCdrBoolean isSeq = RTI_XCDR_FALSE;
    char *strSample = (char *)sample;
    char *externalStrSample = (char *)sample;
    RTIXCdrCommonInsParameters *commonParams;
    const RTIXCdrInsParameters *params;
    RTIXCdrLong charAlignment;
    RTIXCdrOctet strRefMemberKind;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo;
    void *programData;
    RTIXCdrUnsignedLong strTypeSize;
    char *dHeaderSeqPosition = NULL;
    RTIBool isSeqDiscontiguous = RTI_FALSE;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    context->expectedSpaceError = RTI_XCDR_TRUE;
    programData = context->programData;

    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    params = &instruction->params;

    elementCount = commonParams->count;
    charAlignment = params->strParams.charAlignment;
    /* Strings are not supported with fix size flat data we use
     * offsetIndex 0 to select offset
     */
    elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
    strOffset = elOffset;
    externalStrOffset = elOffset;

    memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;

    if (instruction->opcode == RTI_XCDR_SER_WSTRING_SEQ_OPCODE ||
            instruction->opcode == RTI_XCDR_SER_STRING_SEQ_OPCODE) {
        strSampleAccessInfo = commonParams->seqElementTc->_sampleAccessInfo;
        strTc = commonParams->seqElementTc;
        strRefMemberKind = RTI_XCDR_INTERPRETER_VALUE_MEMBER;
        isSeq = RTI_XCDR_TRUE;
        stringCount = 0;
    } else {
        strSampleAccessInfo = memberSampleAccessInfo;
        strTc = commonParams->memberTc;
        strRefMemberKind = commonParams->refMemberKind;
        stringCount = 1;

        if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
            strOffset = 0;
        }
    }

    strTypeSize = RTIXCdrInterpreter_getStrTypeSize(strSampleAccessInfo);

    tcMember = commonParams->tcMemberInfo;
    
    for (i=0; i<elementCount; i++) {
        if (isSeq) {
            char *seqSample;

            if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                seqSample = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        ((char *)sample + elOffset));
                if (seqSample == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            } else {
                seqSample = (char *)sample;
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    &stringCount,
                    seqSample,
                    commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                            0: elOffset,
                    0,
                    commonParams->memberTc,
                    tcMember,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    programData);
            
            if (memberValue.value.ptr == NULL && stringCount > 0) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (stringCount > commonParams->memberTc->_maximumLength) {
                logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = stringCount;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (commonParams->addSeqDHeader) {
                dHeaderSeqPosition = RTIXCdrStream_serializeDHeader(stream);

                if (dHeaderSeqPosition == NULL) {
                    GotoDoneWithLine();
                }
            }

            if (!RTIXCdrStream_serialize4Byte(
                    stream,
                    &stringCount,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }

            if (commonParams->refMemberKind !=
                    RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            } else {
                elOffset += program->externalReferenceSize;
            }

            if (stringCount == 0) {
                if (dHeaderSeqPosition != NULL) {
                    if (!RTIXCdrStream_serializeDHeaderLength(
                            stream,
                            dHeaderSeqPosition)) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                    dHeaderSeqPosition = NULL;
                }
                continue;
            }

            /* Sequences of strings cannot have external strings */
            strSample = memberValue.value.ptr;
            strOffset = 0;
            isSeqDiscontiguous = memberValue.isDiscontiguous;
        }

        for (j=0; j<stringCount; j++) {
            RTIXCdrUnsignedLong charCount = 0;
            RTIXCdrUnsignedLong byteCount;
            RTIXCdrUnsignedLong serLength;
            char *strSampleVal;
            
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                strSample = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        (externalStrSample + externalStrOffset));
                if (strSample == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            }

            if (isSeqDiscontiguous) {
                /* 
                 * The element in the sequence is a pointer to a string
                 * (e.g, pointer to char* or DDS_Wchar* in C)
                 */
                strSampleVal = *RTIXCdrUtility_staticCast(char **, strSample);

                if (strSampleVal == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            } else {
                strSampleVal = strSample;
            }

            RTIXCdrInterpreter_getStrValuePtr(
                    memberValue,
                    charAlignment,
                    &charCount,
                    strSampleVal,
                    strOffset,
                    0,
                    strTc,
                    /* We do not provide tcMember because otherwise the
                     * accesor functions may thing that the string is optional
                     */
                    isSeq?NULL:tcMember,
                    programData);

            if (charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                byteCount = (RTIXCdrUnsignedLong)charCount;
                serLength = charCount;
            } else if (charAlignment == RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                byteCount = charCount << 2;
                serLength = charCount;
            } else {
                /* We do not serialize final NULL character */
                if (charCount != 0) {
                    byteCount = (charCount-1) << 1;
                } else {
                    byteCount = 0;
                }
                serLength = byteCount;
            }

            if (!RTIXCdrStream_serialize4Byte(
                    stream,
                    &serLength,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }

            if (charCount > 1) {
                if (charCount >= params->strParams.charMaxCount) {
                    RTIXCdrBoolean outOfBounds = RTI_XCDR_FALSE;

                    if (charCount > params->strParams.charMaxCount) {
                        outOfBounds = RTI_XCDR_TRUE;
                    } else if (charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                        if (memberValue.value.ptr[charCount - 1] != '\0') {
                            outOfBounds = RTI_XCDR_TRUE;
                        }
                    } else {
                        if (!RTIXCdrInterpreter_useMemberElementIndex(
                                strSampleAccessInfo)) {
                            if (((const RTIXCdrWchar *) (void *)
                                        memberValue.value.ptr)[charCount - 1]
                                != 0) {
                                outOfBounds = RTI_XCDR_TRUE;
                            }
                        } else {
                            memberValue =
                                    strSampleAccessInfo->getMemberValuePointerFcn(
                                            strSample,
                                            NULL,
                                            strOffset,
                                            charCount - 1,
                                            strTc,
                                            tcMember,
                                            RTI_XCDR_FALSE,
                                            programData);
                            if (memberValue.value.wVal != 0) {
                                outOfBounds = RTI_XCDR_TRUE;
                            }
                        }
                    }

                    if (outOfBounds) {
                        logMessageId =
                                RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu;
                        runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                        runTimeLogParam.value.ulVal = charCount;
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                }

                if (charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT
                        || (!stream->_needByteSwap
                                && charAlignment == RTI_XCDR_WCHAR_ALIGNMENT
                                && !RTIXCdrInterpreter_useMemberElementIndex(strSampleAccessInfo))) {
                    if (!RTIXCdrStream_serializeNByte(
                            stream,
                            memberValue.value.ptr,
                            charAlignment,
                            byteCount)) {
                        GotoDoneWithLine();
                    }
                } else {
                    RTIXCdrUnsignedLong h;

                    if (charAlignment == RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                        RTIXCdrLegacyWchar wc;
                    
                        if (!RTIXCdrInterpreter_useMemberElementIndex(
                                strSampleAccessInfo)) {
                            for (h=0; h<charCount; h++) {
                                wc = *(const RTIXCdrWchar *)
                                        (void *)memberValue.value.ptr;
                                if (!RTIXCdrStream_serialize4Byte(
                                        stream,
                                        &wc,
                                        RTI_XCDR_FALSE)) {
                                    GotoDoneWithLine();
                                }
                                memberValue.value.ptr += RTI_XCDR_WCHAR_SIZE;
                            }                            
                        } else {
                            for (h=0; h<charCount; h++) {
                                memberValue = strSampleAccessInfo->getMemberValuePointerFcn(
                                        strSample,
                                        NULL,
                                        strOffset,
                                        h,
                                        strTc,
                                        tcMember,
                                        RTI_XCDR_FALSE,
                                        programData);
                                wc = memberValue.value.wVal;
                                if (!RTIXCdrStream_serialize4Byte(
                                        stream,
                                        &wc,
                                        RTI_XCDR_FALSE)) {
                                    GotoDoneWithLine();
                                }
                            }
                        }
                    } else {
                        /* We do not serialized NULL characters for wstrings
                         * in V2
                         */
                        if (!RTIXCdrInterpreter_useMemberElementIndex(
                                strSampleAccessInfo)) {
                            for (h=0; h<(charCount-1); h++) {
                                if (!RTIXCdrStream_serialize2Byte(
                                        stream,
                                        (const RTIXCdrWchar *)
                                            (void *)memberValue.value.ptr,
                                        RTI_XCDR_FALSE)) {
                                    GotoDoneWithLine();
                                }
                                memberValue.value.ptr += RTI_XCDR_WCHAR_SIZE;
                            }
                        } else {
                            for (h=0; h<(charCount-1); h++) {
                                memberValue = strSampleAccessInfo->getMemberValuePointerFcn(
                                        strSample,
                                        NULL,
                                        strOffset,
                                        h,
                                        strTc,
                                        tcMember,
                                        RTI_XCDR_FALSE,
                                        programData);
                                if (!RTIXCdrStream_serialize2Byte(
                                        stream,
                                        &memberValue.value.wVal,
                                        RTI_XCDR_FALSE)) {
                                    GotoDoneWithLine();
                                }
                            }
                        }
                    }
                }
            } else if (charCount == 1) {
                /* In modern C++ an empty string may not have a buffer
                 * associated with it. Because of that, we need to deal with 
                 * this case separate
                 */
                /* The biggest character is the legacy wchar */
                RTIXCdrLegacyWchar wc = 0;
                
                /* With V2 encapsulation (RTI_XCDR_WCHAR_ALIGNMENT) we dont
                 * need to add NULL termination
                 */
                if (charAlignment != RTI_XCDR_WCHAR_ALIGNMENT) {
                    if (!RTIXCdrStream_serializeNByte(
                            stream,
                            &wc,
                            charAlignment,
                            byteCount)) {
                        GotoDoneWithLine();
                    }
                }
            }

            /* Goto next string */
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                externalStrOffset += program->externalReferenceSize;
            } else if (isSeqDiscontiguous) {
                /* 
                 * We are dealing with a discontiguous sequence of strings
                 * and we need to move the pointer to the next element of
                 * the sequence which is a pointer.
                 */
                strSample += sizeof(void *);
            } else {
                strOffset += strTypeSize;
            }
        }

        if (dHeaderSeqPosition != NULL) {
            if (!RTIXCdrStream_serializeDHeaderLength(
                    stream,
                    dHeaderSeqPosition)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }
            dHeaderSeqPosition = NULL;
        }
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok) {
        if (context->logAllErrorsButExpectedSpaceErrors && !context->expectedSpaceError) {
            RTIXCdrInterpreter_logSerializationError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return ok;
}

#define RTIXCdrInterpreter_serializePrimitiveArray( \
    stream, \
    valuePtr, \
    primitiveSize, \
    primitiveCount, \
    byteCount, \
    returnIfError) \
{ \
    RTIXCdrUnsignedLong __k; \
    \
    if (!RTIXCdrStream_checkSize((stream), (byteCount))) { \
        returnIfError; \
    } \
    \
    switch ((primitiveSize)) { \
    case RTI_XCDR_ONE_BYTE_SIZE: \
    { \
        /* When there is no byte swapping elementSize is \
         * RTI_XCDR_ONE_BYTE_SIZE \
         */ \
        RTIXCdrStream_serializeNByteFast( \
            (stream), \
            (valuePtr), \
            (byteCount)); \
    } break; \
    case RTI_XCDR_TWO_BYTE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            RTIXCdrStream_serialize2ByteFast((stream), (void *)(valuePtr)); \
            (valuePtr) += (primitiveSize); \
        } \
        (valuePtr) -= (primitiveSize); \
    } break; \
    case RTI_XCDR_FOUR_BYTE_SIZE: \
    { \
        if (params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR && \
                !program->isCdr2) { \
            RTIXCdrLegacyWchar __wc; \
            for (__k=0; __k<(primitiveCount); __k++) { \
                __wc = (RTIXCdrLegacyWchar)*(RTIXCdrWchar *)(void *)(valuePtr); \
                RTIXCdrStream_serialize4ByteFast((stream), &__wc); \
                (valuePtr) += sizeof(RTIXCdrWchar); \
            } \
            (valuePtr) -= sizeof(RTIXCdrWchar); \
        } else { \
            for (__k=0; __k<(primitiveCount); __k++) { \
                RTIXCdrStream_serialize4ByteFast((stream), (void *)(valuePtr)); \
                if (params->primitiveParams.enumTc) { \
                    RTIXCdrBoolean __isValid; \
                    RTIXCdrTypeCode_isValidEnumValue( \
                            params->primitiveParams.enumTc, \
                            &__isValid, \
                            *(RTIXCdrEnum *)(void *)valuePtr); \
                    if (!__isValid) { \
                        logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd; \
                        runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM; \
                        runTimeLogParam.value.lVal = *(RTIXCdrEnum *)(void *)valuePtr; \
                        context->expectedSpaceError = RTI_XCDR_FALSE; \
                        returnIfError; \
                    } \
                } \
                \
                (valuePtr) += (primitiveSize); \
            } \
            (valuePtr) -= (primitiveSize); \
        } \
    } break; \
    case RTI_XCDR_EIGHT_BYTE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            RTIXCdrStream_serialize8ByteFast((stream), (void *)(valuePtr)); \
            (valuePtr) += (primitiveSize); \
        } \
        (valuePtr) -= (primitiveSize); \
    } break; \
    case RTI_XCDR_LONG_DOUBLE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            RTIXCdrStream_serialize16ByteFast(stream, (void *)valuePtr); \
            (valuePtr) += (primitiveSize); \
        } \
        (valuePtr) -= (primitiveSize); \
    } break; \
    default: \
    { \
        context->expectedSpaceError = RTI_XCDR_FALSE; \
        returnIfError; \
    } break; \
    } \
}

#define RTIXCdrInterpreter_serializeOneBytePrimitiveArray( \
    stream, \
    valuePtr, \
    byteCount, \
    returnIfError) \
{ \
    if (!RTIXCdrStream_checkSize((stream), (byteCount))) { \
        returnIfError; \
    } \
    \
    RTIXCdrStream_serializeNByteFast( \
            (stream), \
            (valuePtr), \
            (byteCount)); \
}

#define RTIXCdrInterpreter_serializeExternalPrimitiveArray( \
    stream, \
    valuePtr, \
    primitiveSize, \
    primitiveCount, \
    byteCount, \
    returnIfError) \
{ \
    RTIXCdrUnsignedLong __k; \
    void *__externalValuePtr; \
    \
    if (!RTIXCdrStream_checkSize((stream), (byteCount))) { \
        returnIfError; \
    } \
    \
    switch ((primitiveSize)) { \
    case RTI_XCDR_ONE_BYTE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
            if (__externalValuePtr == NULL) { \
                context->expectedSpaceError = RTI_XCDR_FALSE; \
                logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                returnIfError; \
            } \
            RTIXCdrStream_serialize1ByteFast((stream), __externalValuePtr); \
            (valuePtr) += program->externalReferenceSize; \
        } \
    } break; \
    case RTI_XCDR_TWO_BYTE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
            if (__externalValuePtr == NULL) { \
                context->expectedSpaceError = RTI_XCDR_FALSE; \
                logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                returnIfError; \
            } \
            RTIXCdrStream_serialize2ByteFast((stream), __externalValuePtr); \
            (valuePtr) += program->externalReferenceSize; \
        } \
    } break; \
    case RTI_XCDR_FOUR_BYTE_SIZE: \
    { \
        if (params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR && \
                !program->isCdr2) { \
            RTIXCdrLegacyWchar __wc; \
            for (__k=0; __k<(primitiveCount); __k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                __wc = (RTIXCdrLegacyWchar)*(RTIXCdrWchar *)__externalValuePtr; \
                RTIXCdrStream_serialize4ByteFast((stream), &__wc); \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } else { \
            for (__k=0; __k<(primitiveCount); __k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                RTIXCdrStream_serialize4ByteFast( \
                        (stream), \
                        __externalValuePtr); \
                if (params->primitiveParams.enumTc) { \
                    RTIXCdrBoolean __isValid; \
                    RTIXCdrTypeCode_isValidEnumValue( \
                            params->primitiveParams.enumTc, \
                            &__isValid, \
                            *(RTIXCdrEnum *)__externalValuePtr); \
                    if (!__isValid) { \
                        logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd; \
                        runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM; \
                        runTimeLogParam.value.lVal = *(RTIXCdrEnum *)__externalValuePtr; \
                        context->expectedSpaceError = RTI_XCDR_FALSE; \
                        returnIfError; \
                    } \
                } \
                \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } \
    } break; \
    case RTI_XCDR_EIGHT_BYTE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
            if (__externalValuePtr == NULL) { \
                context->expectedSpaceError = RTI_XCDR_FALSE; \
                logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                returnIfError; \
            } \
            RTIXCdrStream_serialize8ByteFast( \
                    (stream), \
                    __externalValuePtr); \
            (valuePtr) += program->externalReferenceSize; \
        } \
    } break; \
    case RTI_XCDR_LONG_DOUBLE_SIZE: \
    { \
        for (__k=0; __k<(primitiveCount); __k++) { \
            __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
            if (__externalValuePtr == NULL) { \
                context->expectedSpaceError = RTI_XCDR_FALSE; \
                logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                returnIfError; \
            } \
            RTIXCdrStream_serialize16ByteFast( \
                    (stream), \
                    __externalValuePtr); \
            (valuePtr) += program->externalReferenceSize; \
        } \
    } break; \
    default: \
    { \
        context->expectedSpaceError = RTI_XCDR_FALSE; \
        returnIfError; \
    } break; \
    } \
}

#define RTIXCdrInterpreter_useExtendedV1Id(instruction, context) \
(((instruction)->params.memberHeaderParams.v1ExtendedId == RTI_XCDR_TRUE)? \
    RTI_XCDR_TRUE: \
    (((instruction)->params.memberHeaderParams.v1ExtendedId == RTI_XCDR_FALSE)? \
            RTI_XCDR_FALSE: context->useXcdr1ExtendedId))

#define RTIXCdrInterpreter_serializeComplex( \
    stream, \
    samplePtr, \
    serializeComplexFcn, \
    context, \
    returnIfError) \
if ((serializeComplexFcn) == NULL) { \
    if (!RTIXCdrInterpreter_serializeSample( \
            (stream), \
            (samplePtr), \
            (context)->typeCode, \
            (context)->program, \
            (context))) { \
        returnIfError; \
    } \
} else { \
    if (!serializeComplexFcn( \
            (context)->endpointPluginData, \
            (samplePtr), \
            (stream), \
            0, \
            (stream)->_encapsulationKind, \
            1, \
            (context)->endpointPluginQos)) { \
        returnIfError; \
    } \
}

RTI_PRIVATE
RTIXCdrEnum RTIXCdrInterpreter_getDefaultEnumValue(
        RTIXCdrCommonInsParameters *commonParams,
        struct RTIXCdrTypeCodeAnnotations *annotations)
{
    RTIXCdrLog_testPrecondition(commonParams == NULL, return 0);
    
    if (annotations == NULL 
            || annotations->_defaultValue._d == RTI_XCDR_TK_NULL) {
        if (commonParams->tcMemberInfo != NULL
                && commonParams->tcMemberInfo->_annotations._defaultValue._d 
                        != RTI_XCDR_TK_NULL) {
            annotations = &commonParams->tcMemberInfo->_annotations;
        } else {
            annotations = &commonParams->memberTc->_annotations;
        }
    }

    RTIXCdrLog_testPrecondition(
            annotations->_defaultValue._d != RTI_XCDR_TK_ENUM, 
            return 0);
    return annotations->_defaultValue._u.enumerated_value;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_checkPrimitiveRange(
        const void *value,
        RTIXCdrTCKind kind,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypeCodeMember *tcMemberInfo,
        struct RTIXCdrTypeCodeAnnotations *annotations,
        RTIXCdrBoolean externalRef,
        RTIXCdrBoolean serialization)
{
    const char *typeName;

    RTIXCdrLog_testPrecondition(value == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);

    typeName = program->typeCode->_name;

    if (externalRef) {
        /* External reference */
        value = (const char *)RTIXCdrProgram_getExternalRefValuePtr(
                program,
                (void *)value);

        if (value == NULL) {
            RTIXCdrLog_logTwoStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"");
            return RTI_XCDR_FALSE;
        }
    }

    /* 
     * annotations may not be NULL if a different annotation (not maxValue) is
     * set for this member. We only want to use the annotations value provided
     * to this function if the maxValue is initialized correctly
     */
    if (annotations == NULL || annotations->_maxValue._d == RTI_XCDR_TK_NULL) {
        if (tcMemberInfo != NULL) {
            annotations = &tcMemberInfo->_annotations;
        } else {
            annotations = &program->typeCode->_annotations;
        }
    }

    switch (kind) {
    case RTI_XCDR_TK_SHORT:
    {
        RTIXCdrShort *sVal = (RTIXCdrShort *)value;
        if (*sVal < annotations->_minValue._u.short_value  ||
                *sVal > annotations->_maxValue._u.short_value) {
            RTIXCdrLog_logTwoStrThreeLongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *sVal,
                    annotations->_minValue._u.short_value,
                    annotations->_maxValue._u.short_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_LONG:
    {
        RTIXCdrLong *lVal = (RTIXCdrLong *)value;
        if (*lVal < annotations->_minValue._u.long_value ||
                *lVal > annotations->_maxValue._u.long_value) {
            RTIXCdrLog_logTwoStrThreeLongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *lVal,
                    annotations->_minValue._u.long_value,
                    annotations->_maxValue._u.long_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_USHORT:
    {
        RTIXCdrUnsignedShort *usVal = (RTIXCdrUnsignedShort *)value;
        if (*usVal < annotations->_minValue._u.ushort_value ||
                *usVal > annotations->_maxValue._u.ushort_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *usVal,
                    annotations->_minValue._u.ushort_value,
                    annotations->_maxValue._u.ushort_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_ULONG:
    {
        RTIXCdrUnsignedLong *ulVal = (RTIXCdrUnsignedLong *)value;
        if (*ulVal < annotations->_minValue._u.ulong_value ||
                *ulVal > annotations->_maxValue._u.ulong_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *ulVal,
                    annotations->_minValue._u.ulong_value,
                    annotations->_maxValue._u.ulong_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_FLOAT:
    {
        RTIXCdrFloat *fVal = (RTIXCdrFloat *)value;
        if (RTIXCdrUtility_isnanf(*fVal)
                || RTIXCdrUtility_isinff(*fVal)
                || *fVal < annotations->_minValue._u.float_value
                || *fVal > annotations->_maxValue._u.float_value) {
            RTIXCdrLog_logTwoStrThreeDouble(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *fVal,
                    annotations->_minValue._u.float_value,
                    annotations->_maxValue._u.float_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_DOUBLE:
    {
        RTIXCdrDouble *dVal = (RTIXCdrDouble *)value;
        if (RTIXCdrUtility_isnan(*dVal)
                || RTIXCdrUtility_isinf(*dVal)
                || *dVal < annotations->_minValue._u.double_value
                || *dVal > annotations->_maxValue._u.double_value) {
            RTIXCdrLog_logTwoStrThreeDouble(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *dVal,
                    annotations->_minValue._u.double_value,
                    annotations->_maxValue._u.double_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_OCTET:
    {
        RTIXCdrOctet *oVal = (RTIXCdrOctet *)value;
        if (*oVal < annotations->_minValue._u.octet_value ||
                *oVal > annotations->_maxValue._u.octet_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *oVal,
                    annotations->_minValue._u.octet_value,
                    annotations->_maxValue._u.octet_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_INT8: {
        RTIXCdrInt8 *iVal = (RTIXCdrInt8 *) value;
        if (*iVal < annotations->_minValue._u.int8_value
                || *iVal > annotations->_maxValue._u.int8_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization
                            ? RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd
                            : RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
                    typeName ? typeName : "",
                    tcMemberInfo ? tcMemberInfo->_name : "",
                    *iVal,
                    annotations->_minValue._u.int8_value,
                    annotations->_maxValue._u.int8_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_UINT8: {
        RTIXCdrUInt8 *uVal = (RTIXCdrUInt8 *) value;
        if (*uVal < annotations->_minValue._u.uint8_value
                || *uVal > annotations->_maxValue._u.uint8_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization
                            ? RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu
                            : RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
                    typeName ? typeName : "",
                    tcMemberInfo ? tcMemberInfo->_name : "",
                    *uVal,
                    annotations->_minValue._u.uint8_value,
                    annotations->_maxValue._u.uint8_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_LONGLONG:
    {
        RTIXCdrLongLong *llVal = (RTIXCdrLongLong *)value;
        if (*llVal < annotations->_minValue._u.long_long_value ||
                *llVal > annotations->_maxValue._u.long_long_value) {
            RTIXCdrLog_logTwoStrThreeLongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *llVal,
                    annotations->_minValue._u.long_long_value,
                    annotations->_maxValue._u.long_long_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    case RTI_XCDR_TK_ULONGLONG:
    {
        RTIXCdrUnsignedLongLong *ullVal = (RTIXCdrUnsignedLongLong *)value;
        if (*ullVal < annotations->_minValue._u.ulong_long_value ||
                *ullVal > annotations->_maxValue._u.ulong_long_value) {
            RTIXCdrLog_logTwoStrThreeULongLong(
                    RTI_XCDR_LOG_EXCEPTION,
                    serialization?
                            RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
                            RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
                    typeName?typeName:"",
                    tcMemberInfo?tcMemberInfo->_name:"",
                    *ullVal,
                    annotations->_minValue._u.ulong_long_value,
                    annotations->_maxValue._u.ulong_long_value);
            return RTI_XCDR_FALSE;
        }
    } break;
    default:
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

#define RTIXCdrInterpreter_assignContextAnnotations( \
        context, \
        commonParams, \
        tc) \
if ((context)->annotations == NULL) { \
    if ((commonParams)->tcMemberInfo != NULL) { \
        if ((commonParams)->tcMemberInfo->_annotations._maxValue._d \
                != RTI_XCDR_TK_NULL \
                || (commonParams)->tcMemberInfo->_annotations._defaultValue._d \
                != RTI_XCDR_TK_NULL) { \
            (context)->annotations = \
                    &(commonParams)->tcMemberInfo->_annotations; \
        } \
    } else { \
        if ((tc)->_annotations._maxValue._d != RTI_XCDR_TK_NULL \
            || (tc)->_annotations._defaultValue._d != RTI_XCDR_TK_NULL) { \
            (context)->annotations = \
                    (struct RTIXCdrTypeCodeAnnotations *) \
                        &(tc)->_annotations; \
        } \
    } \
}

/* 
 * In some cases a sequence of complex elements can be considered a sequence
 * of primitives. For example:
 * 
 * struct Point {
 *     long x;
 *     long y;
 * }
 * 
 * struct Polygon {
 *     sequence<Point, 100> points;
 * }
 * 
 * The 'points' sequence can be treated as:
 * 
 * struct Polygon {
 *     sequence<long, 200> points;
 * }
 * 
 * This macro changes the sequence length so that it refers to primitive 
 * members and not complex members.
 * 
 * In the example above 100 would be converted into 200
 * 
 * The serialized size of the complex member is stored in 
 * params->primitiveParams.primitiveByteCount
 */
 #define RTIXCdrInterpreter_adjustSequenceElementCount( \
        seqElementCount__, \
        byteCount__, \
        params__) \
    if ((params__)->primitiveParams.primitiveByteCount != \
            (params__)->primitiveParams.origPrimitiveSize) { \
        (seqElementCount__) *= \
                ((params__)->primitiveParams.primitiveByteCount/ \
                        (params__)->primitiveParams.origPrimitiveSize); \
    } \
    (byteCount__) = \
            (seqElementCount__) * (params__)->primitiveParams.origPrimitiveSize


/*
 * @brief This function checks and adjusts the input seqElementCount.
 *
 * Checking seqElementCount means:
 * 1) That it should not exceed the maximum allowed.
 * 2) That there is space left in the stream to deserialize seqElementCount
 * elements.
 * 
 * Adjustment occurs in RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE instructions.
 * This adjustment is needed because the primitive sequence may have been the
 * result of optimizing a complex sequence as a sequence of primitives. See
 * the documentation of RTIXCdrInterpreter_adjustSequenceElementCount for
 * additional details.
 * 
 * @param primitiveSeqElementCount Out. For complex sequences optimized as a
 * primitive sequences, the value of primitiveSeqElementCount is adjusted to
 * represent the number of primitive elements instead of the number of
 * complex elements. For primitive sequences, primitiveSeqElementCount is set to
 * seqElementCount.
 * @param primitiveByteCount Out. For primitive sequences and complex sequences
 * optimized as a primitive sequences, the value of primitiveByteCount is set to
 * the total number of bytes of the serialized sequence.
 * @param seqElementCount In. The deserialized seqElementCount.
 * @param errorLogMessageId Out. If there is an error, this parameter is
 * initialized to the log message ID for the error.
 * @param stream In. XCDR stream.
 * @param instruction In. Instruction being processed.
 * 
 * @return RTI_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_checkAndAdjustSequenceElementCount(
        RTIXCdrUnsignedLong *primitiveSeqElementCount,
        RTIXCdrUnsignedLong *primitiveByteCount,
        RTIXCdrUnsignedLong *seqElementCount,
        RTIXCdrLogMessageId *errorLogMessageId,
        RTIXCdrStream *stream,
        struct RTIXCdrInstruction *instruction)
{
    RTIXCdrUnsignedLongLong longSeqElementCount;
    RTIXCdrUnsignedLongLong longPrimitiveByteCount;
    RTIXCdrCommonInsParameters *commonParams;

    RTIXCdrLog_testPrecondition(seqElementCount == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            errorLogMessageId == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);

    commonParams = RTIXCdrInstruction_getCommonParams(instruction);
    RTIXCdrLog_testPrecondition(commonParams == NULL, return RTI_XCDR_FALSE);

    if (*seqElementCount > commonParams->memberTc->_maximumLength) {
        *errorLogMessageId =
                RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
        return RTI_XCDR_FALSE;
    }

    if (instruction->opcode == RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE) {
        RTIXCdrLog_testPrecondition(
                primitiveByteCount == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                primitiveSeqElementCount == NULL,
                return RTI_XCDR_FALSE);

        longSeqElementCount = *seqElementCount;
        RTIXCdrInterpreter_adjustSequenceElementCount(
                longSeqElementCount,
                longPrimitiveByteCount,
                &instruction->params);
        *primitiveSeqElementCount = (RTIXCdrUnsignedLong) longSeqElementCount;
        *primitiveByteCount = (RTIXCdrUnsignedLong) longPrimitiveByteCount;
    } else if (instruction->opcode == RTI_XCDR_DESER_WSTRING_SEQ_OPCODE) {
        if (instruction->params.strParams.charAlignment
            == RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
            longPrimitiveByteCount =
                    ((RTIXCdrUnsignedLongLong) *seqElementCount)
                    * RTI_XCDR_LEGACY_WCHAR_SIZE;
        } else {
            longPrimitiveByteCount =
                    ((RTIXCdrUnsignedLongLong) *seqElementCount)
                    * RTI_XCDR_WCHAR_SIZE;
        }
    } else {
        longPrimitiveByteCount = *seqElementCount;
    }

    /* 
     * Note that for non-optimized complex sequences or string sequences, 
     * this checking is not accurate because it assumes that the sequence elements 
     * take 1 byte each (or 2/4 bytes for wstring sequences).
     * 
     * For complex sequences we could make it more precise by considering the 
     * minimum serialized size as the element size. However, this will add 
     * significant complexity to the logic as we will have to compute and 
     * cache this value.
     * 
     * This checking was added to detect situations in which the sequence
     * length is corrupted (or tampered) in order to avoid big memory 
     * allocations (see SEC-1338). Most of these situations will be correctly
     * detected with this conservative approach.
     */
    if (longPrimitiveByteCount > RTIXCdrStream_getRemainder(stream)) {
        *errorLogMessageId =
                RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_STREAM_FAILURE_ID_ssuu;
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

/*
 * When we have a collection dheader we have to skip the
 * collection dheader instruction to get the data instruction
 * associated with the mutable/optional member.
 */
#define  RTIXCdrInterpreter_getMemberDataInsIndex(program__, insIndex__) \
    (((program__)->isCdr2 && \
            RTIXCdrInstruction_isHeaderOpcode((program__)->instructions[(insIndex__) + 1].opcode)) \
                    ? (insIndex__) + 2 \
                    : (insIndex__) + 1)

/* 
 * With respect to error logging we do not log any error in this API related to 
 * running out of space. This is because this API is used by batching when 
 * max_data_bytes is set to detect situations in which we have to start 
 * new batch.
 */
RTIXCdrBoolean RTIXCdrInterpreter_fullSerializeSample(
        RTIXCdrStream *stream,
        void *sample,
        RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    /* Uninitialized variables */
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong instructionCount;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrBoolean isUnion;
    struct RTIXCdrStreamState memberStreamState;
    RTIXCdrBoolean flatData;
    RTIXCdrMemberValue memberValue;
    RTIXCdrUnsignedLong j;
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;
    char *valuePtr;
    RTIXCdrUnsignedLongLong elOffset;
    RTIXCdrUnsignedLong arrayCount;
    RTIXCdrBoolean getUnionInstIndex;

    /* Initialize variables */
    char *dHeaderPosition = NULL;
    char *dHeaderCollectionPosition = NULL;
    char *memberPosition = NULL;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
    RTIXCdrBoolean extended = RTI_XCDR_FALSE;
    RTIXCdrBoolean addPaddingToParameterLenght = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong offsetIndex = 0;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrBoolean useMemberIndex = RTI_XCDR_FALSE;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;
    void *programData;
    RTIXCdrInstruction *instruction = NULL;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    isUnion = program->unionDiscKind != RTI_XCDR_TK_NULL;
    getUnionInstIndex = isUnion;
    instructionCount = program->instructionCount;


    context->expectedSpaceError = RTI_XCDR_TRUE;
    programData = context->programData;

    flatData = program->isFlatDataProgram;

    if (flatData) {
        offsetIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               sample)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
    }

    addPaddingToParameterLenght = RTIXCdrXTypesComplianceMask_isBitSet(
            program->xTypesComplianceMask,
            RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT);

    for (i=0; i<instructionCount; ) {
        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);
        if (commonParams->memberTc != NULL) {
            memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
        }
        /*
         * memberTc can only be NULL if isHeaderOpcode is true, because header
         * operations are the only ones that do not require access to/knowledge
         * of the member's typecode that is being operated on
         */
        RTIXCdrLog_testPrecondition(
                !RTIXCdrInstruction_isHeaderOpcode(instruction->opcode)
                        && commonParams->memberTc == NULL,
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine());

        switch (instruction->opcode) {
        case RTI_XCDR_SER_PRIMITIVE_OPCODE:
        {
            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SER_PRIMITIVE_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            if (params->primitiveParams.mustAlign) {
                RTIXCdrStream_alignMacro(
                        stream,
                        params->primitiveParams.primitiveAlignment,
                        GotoDoneWithLine());
            }
            
            useMemberIndex = RTIXCdrInterpreter_useMemberElementIndex(
                    memberSampleAccessInfo);

            if (!useMemberIndex) 
            {
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        NULL,
                        sample,
                        commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);

                if (memberValue.value.ptr == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    if (params->primitiveParams.checkRange) {
                        if (!RTIXCdrInterpreter_checkPrimitiveRange(
                                memberValue.value.ptr,
                                params->primitiveParams.primitiveKind,
                                program,
                                commonParams->tcMemberInfo,
                                context->annotations,
                                RTI_XCDR_TRUE,
                                RTI_XCDR_TRUE)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                    }

                    RTIXCdrInterpreter_serializeExternalPrimitiveArray(
                        stream,
                        memberValue.value.ptr,
                        params->primitiveParams.primitiveSize,
                        commonParams->count,
                        params->primitiveParams.primitiveByteCount,
                        GotoDoneWithLine());
                } else {
                    if (params->primitiveParams.checkRange) {
                        if (!RTIXCdrInterpreter_checkPrimitiveRange(
                                memberValue.value.ptr,
                                params->primitiveParams.primitiveKind,
                                program,
                                commonParams->tcMemberInfo,
                                context->annotations,
                                RTI_XCDR_FALSE,
                                RTI_XCDR_TRUE)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                    }

                    RTIXCdrInterpreter_serializePrimitiveArray(
                        stream,
                        memberValue.value.ptr,
                        params->primitiveParams.primitiveSize,
                        commonParams->count,
                        params->primitiveParams.primitiveByteCount,
                        GotoDoneWithLine());
                }
            } else {
                RTIXCdrUnsignedLong primitiveCount;

                if (params->primitiveParams.primitiveSize != 
                        params->primitiveParams.origPrimitiveSize) {
                    primitiveCount = 
                            commonParams->count/params->primitiveParams.origPrimitiveSize;
                } else {
                    primitiveCount = commonParams->count;
                }
                
                if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    RTIXCdrUnsignedLong h;
                    char * externalRefPtr = NULL;
                    char * externalRefValuePtr = NULL;

                    externalRefPtr = (char *)sample +
                            commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex];

                    for (h=0; h<primitiveCount; h++) {
                        valuePtr = (char *)&memberValue.value;
                        externalRefValuePtr = RTIXCdrProgram_getExternalRefValuePtr(
                                program, externalRefPtr);

                        memberValue = memberSampleAccessInfo->getMemberValuePointerFcn(
                                externalRefValuePtr,
                                NULL,
                                0,
                                0,
                                commonParams->memberTc,
                                commonParams->tcMemberInfo,
                                RTI_XCDR_FALSE,
                                programData);
                        RTIXCdrInterpreter_serializePrimitiveArray(
                            stream,
                            valuePtr,
                            params->primitiveParams.primitiveSize,
                            1,
                            params->primitiveParams.origPrimitiveSize,
                            GotoDoneWithLine());

                        externalRefPtr += program->externalReferenceSize;
                    }
                } else {
                    for (j=0; j<primitiveCount; j++) {
                        valuePtr = (char *)&memberValue.value;
                        memberValue = memberSampleAccessInfo->getMemberValuePointerFcn(
                                sample,
                                NULL,
                                commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                                j,
                                commonParams->memberTc,
                                commonParams->tcMemberInfo,
                                RTI_XCDR_FALSE,
                                programData);

                        RTIXCdrInterpreter_serializePrimitiveArray(
                            stream,
                            valuePtr,
                            params->primitiveParams.primitiveSize,
                            1,
                            params->primitiveParams.origPrimitiveSize,
                            GotoDoneWithLine());
                    }
                }
            }
        } break;
        case RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong seqElementCount = 0;
            void *samplePtr = sample;
            RTIXCdrUnsignedLong byteCount;
            char *dHeaderSeqPosition = NULL;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            arrayCount = commonParams->count;
            /* Sequences are not supported with fix size flat data we use
             * offsetIndex 0 to select offset
             */
            elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

            for (j=0; j<arrayCount; j++) {
                RTIXCdrUnsignedLong h;

                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);

                if (seqElementCount > commonParams->memberTc->_maximumLength) {
                    logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (commonParams->addSeqDHeader) {
                    dHeaderSeqPosition = RTIXCdrStream_serializeDHeader(
                            stream);

                    if (dHeaderSeqPosition == NULL) {
                        GotoDoneWithLine();
                    }
                }

                if (!RTIXCdrStream_serialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (seqElementCount == 0) {
                    if (commonParams->refMemberKind !=
                            RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                        elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    } else {
                        elOffset += program->externalReferenceSize;
                    }

                    if (dHeaderSeqPosition != NULL) {
                        if (!RTIXCdrStream_serializeDHeaderLength(
                                stream, 
                                dHeaderSeqPosition)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                        dHeaderSeqPosition = NULL;
                    }
                    continue;
                }

                if (params->primitiveParams.primitiveAlignment >
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT) {
                    RTIXCdrStream_alignMacro(
                            stream,
                            params->primitiveParams.primitiveAlignment,
                            GotoDoneWithLine());
                }

                if (memberValue.isDiscontiguous) {
                    if (memberValue.value.ptr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }

                    /* 
                     * We are dealing with a discontiguous sequence where
                     * the buffer is a buffer of pointers to the actual data
                     * elements. 
                     *
                     * These sequences are only allowed in C and traditional C++
                     * where RTIXCdrInterpreter_useMemberElementIndex is false.
                     */
                    for (h=0; h<seqElementCount; h++) {
                        char **valuePtrPtr = RTIXCdrUtility_staticCast(
                                char **,
                                memberValue.value.ptr);
                        valuePtr = *(valuePtrPtr + h);

                        if (valuePtr == NULL) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }

                        RTIXCdrInterpreter_serializePrimitiveArray(
                            stream,
                            valuePtr,
                            params->primitiveParams.primitiveSize,
                            1,
                            params->primitiveParams.origPrimitiveSize,
                            GotoDoneWithLine());
                    }
                } else if (!RTIXCdrInterpreter_useMemberElementIndex(
                        memberSampleAccessInfo)) {
                    if (memberValue.value.ptr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }

                    RTIXCdrInterpreter_adjustSequenceElementCount(
                            seqElementCount, 
                            byteCount,
                            params);
    
                    RTIXCdrInterpreter_serializePrimitiveArray(
                        stream,
                        memberValue.value.ptr,
                        params->primitiveParams.primitiveSize,
                        seqElementCount,
                        byteCount,
                        GotoDoneWithLine());
                } else {
                    for (h=0; h<seqElementCount; h++) {
                        valuePtr = (char *)&memberValue.value;
                        memberValue =
                                memberSampleAccessInfo
                                        ->getMemberValuePointerFcn(
                                                sample,
                                                NULL,
                                                elOffset,
                                                h,
                                                commonParams->memberTc,
                                                commonParams->tcMemberInfo,
                                                RTI_XCDR_FALSE,
                                                programData);
                        RTIXCdrInterpreter_serializePrimitiveArray(
                            stream,
                            valuePtr,
                            params->primitiveParams.primitiveSize,
                            1,
                            params->primitiveParams.origPrimitiveSize,
                            GotoDoneWithLine());
                    }
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (dHeaderSeqPosition != NULL) {
                    if (!RTIXCdrStream_serializeDHeaderLength(
                            stream, 
                            dHeaderSeqPosition)) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                    dHeaderSeqPosition = NULL;
                }
            }
        } break;
        case RTI_XCDR_SER_WSTRING_SEQ_OPCODE:
        case RTI_XCDR_SER_WSTRING_OPCODE:
        case RTI_XCDR_SER_STRING_SEQ_OPCODE:
        case RTI_XCDR_SER_STRING_OPCODE:
        {
            if (!RTIXCdrInterpreter_serializeString(
                    program,
                    stream,
                    instruction,
                    sample,
                    tc,
                    context)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SER_COMPLEX_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginSerializeFunction serializeComplexFcn = NULL;
            char *samplePtr;
            struct RTIXCdrTypeCodeAnnotations *tmpAnnotations = NULL;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SER_COMPLEX_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            /*
             * memberSampleAccessInfo should not be NULL because all language
             * bindings have sample access info for complex members
             */
            RTIXCdrLog_testPrecondition(
                    memberSampleAccessInfo == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine());

            if (params->complexParams.typePlugin != NULL) {
                if (!context->onlyKey) {
                    serializeComplexFcn = (RTIXCdrTypePluginSerializeFunction)
                            params->complexParams.typePlugin->serializeFnc;
                } else {
                    serializeComplexFcn = (RTIXCdrTypePluginSerializeFunction)
                            params->complexParams.typePlugin->serializeKeyFnc;
                }
            }
            
            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    programData);

            arrayCount = commonParams->count;
            tmpInBaseClass = context->inBaseClass;
            tmpAnnotations = context->annotations;

            RTIXCdrInterpreter_assignContextAnnotations(
                    context,
                    commonParams,
                    tc);

            for (j=0; j<arrayCount; j++) {
                if (j != 0) {
                    RTIXCdrInterpreter_nextArrayElementPtr(
                            memberValue.value.ptr,
                            flatData,
                            memberSampleAccessInfo->typeSize,
                            &params->complexParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr = memberValue.value.ptr;
                } else {
                    samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program, memberValue.value.ptr);
                    if (samplePtr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                /* We have to assign this here because the variables
                 * can be overwritten by the nested calls
                 */
                context->program = params->complexParams.program;
                context->typeCode = commonParams->memberTc;
                context->inBaseClass = params->complexParams.baseClass;

                RTIXCdrInterpreter_serializeComplex(
                    stream,
                    samplePtr,
                    serializeComplexFcn,
                    context,
                    GotoDoneWithLine());
            }
            
            context->inBaseClass = tmpInBaseClass;
            context->annotations = tmpAnnotations;
        } break;
        case RTI_XCDR_SER_COMPLEX_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong seqElementCount = 0;
            RTIXCdrUnsignedLong h;
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginSerializeFunction serializeComplexFcn = NULL;
            void *samplePtr = sample;
            struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo =
                    commonParams->seqElementTc->_sampleAccessInfo;
            char *dHeaderSeqPosition = NULL;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SER_COMPLEX_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            if (params->complexParams.typePlugin != NULL) {
                if (!context->onlyKey) {
                    serializeComplexFcn = (RTIXCdrTypePluginSerializeFunction)
                            params->complexParams.typePlugin->serializeFnc;
                } else {
                    serializeComplexFcn = (RTIXCdrTypePluginSerializeFunction)
                            params->complexParams.typePlugin->serializeKeyFnc;
                }
            }

            arrayCount = commonParams->count;
            /* Sequences are not supported with fix size flat data we use
             * offsetIndex 0 to select offset
             */
            elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
            
            tmpInBaseClass = context->inBaseClass;

            for (j=0; j<arrayCount; j++) {
                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);

                if (seqElementCount > commonParams->memberTc->_maximumLength) {
                    logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (commonParams->addSeqDHeader) {
                    dHeaderSeqPosition = RTIXCdrStream_serializeDHeader(
                            stream);

                    if (dHeaderSeqPosition == NULL) {
                        GotoDoneWithLine();
                    }
                }
                
                if (!RTIXCdrStream_serialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (seqElementCount != 0) {
                    RTIXCdrUnsignedLongLong seqElementOffset;
                    char *seqElementPtr;

                    if (memberValue.isDiscontiguous) {
                        /*
                         * We are dealing with a discontiguous sequence where
                         * the buffer is a buffer of pointers to the actual data
                         * elements.
                         */
                        seqElementOffset = sizeof(char *);
                    } else {
                        seqElementOffset =
                                seqElementSampleAccessInfo
                                        ->typeSize[NON_FLAT_DATA_INDEX];
                    }

                    if (memberValue.value.ptr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }

                    for (h=0; h<seqElementCount; h++) {
                        if (memberValue.isDiscontiguous) {
                            char **seqElementPtrPtr = RTIXCdrUtility_staticCast(
                                    char **,
                                    memberValue.value.ptr);

                            seqElementPtr = *seqElementPtrPtr;

                            if (seqElementPtr == NULL) {
                                context->expectedSpaceError = RTI_XCDR_FALSE;
                                GotoDoneWithLine();
                            }
                        } else {
                            seqElementPtr = memberValue.value.ptr;
                        }

                        /* We have to assign this here because the variables
                        * can be overwritten by the nested calls
                        */
                        context->program = params->complexParams.program;
                        context->typeCode = params->complexParams.program->typeCode;
                        context->inBaseClass = params->complexParams.baseClass;

                        RTIXCdrInterpreter_serializeComplex(
                            stream,
                            seqElementPtr,
                            serializeComplexFcn,
                            context,
                            GotoDoneWithLine());

                        memberValue.value.ptr += seqElementOffset;
                    }
                }

                if (dHeaderSeqPosition != NULL) {
                    if (!RTIXCdrStream_serializeDHeaderLength(
                            stream, 
                            dHeaderSeqPosition)) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                    dHeaderSeqPosition = NULL;
                }
              
            }    
            
            context->inBaseClass = tmpInBaseClass;
        } break;
        case RTI_XCDR_SER_MEMBER_HEADER_OPCODE:
        {
            RTIXCdrBoolean memberValueSet = RTI_XCDR_TRUE;
            RTIXCdrCommonInsParameters *memberDataInstCommonParams;
            RTI_UINT32 dataInstIndex;

            dataInstIndex = RTIXCdrInterpreter_getMemberDataInsIndex(
                    program,
                    i);

            memberDataInstCommonParams = RTIXCdrInstruction_getCommonParams(
                    &program->instructions[dataInstIndex]);

            if (memberDataInstCommonParams->refMemberKind == RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                RTIXCdrInterpreter_isOptionalMemberValueSet(
                        &memberValueSet,
                        sample,
                        memberDataInstCommonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX],
                        memberDataInstCommonParams->memberTc,
                        memberDataInstCommonParams->tcMemberInfo,
                        programData);
            }
            
            if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY ||
                    memberValueSet) {
                RTIXCdrBoolean isUnionDiscriminator =
                        memberDataInstCommonParams->tcMemberInfo == NULL;
                RTIXCdrBoolean isKeyMember =
                        (!isUnionDiscriminator
                         && memberDataInstCommonParams->tcMemberInfo != NULL
                         && memberDataInstCommonParams->tcMemberInfo
                                         ->_memberFlags
                                 & 0x01 /* RTI_CDR_FLAG_KEY_MEMBER */)
                        == 0x01 /* RTI_CDR_FLAG_KEY_MEMBER */;
                RTIXCdrBoolean isMustUnderstandMember =
                        (!isKeyMember && !isUnionDiscriminator
                         && memberDataInstCommonParams->tcMemberInfo != NULL
                         && memberDataInstCommonParams->tcMemberInfo
                                    ->_annotations._isMustUnderstand);
                if (program->isCdr2) {
                    if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                        RTIXCdrBoolean failure;

                        memberPosition = RTIXCdrStream_serializeV2ParameterHeader(
                                stream,
                                &failure,
                                isUnionDiscriminator
                                        ? 0
                                        : memberDataInstCommonParams
                                                  ->tcMemberInfo
                                                  ->_representation._pid,
                                (/* mustUnderstand bit is always set for members
                                    annotated with @must_understand */
                                 isMustUnderstandMember ||
                                 /* mustUnderstand is only set for key members
                                    and union discriminators if the XTypes
                                    compliance mask allows it */
                                 (RTIXCdrXTypesComplianceMask_isBitSet(
                                          program->xTypesComplianceMask,
                                          RTI_XCDR_XTYPES_MUST_UNDERSTAND_IN_KEY_AND_DISCRIMINATOR_BIT)
                                  && (isUnionDiscriminator || isKeyMember)))
                                        ? RTI_XCDR_TRUE
                                        : RTI_XCDR_FALSE,
                                instruction->params.memberHeaderParams.v2LC);

                        if (failure) {
                            GotoDoneWithLine();
                        }
                    } else {
                        if (!RTIXCdrStream_serialize1Byte(
                                stream,
                                &memberValueSet)) {
                            GotoDoneWithLine();
                        }
                    }
                } else {
                    extended =
                            RTIXCdrInterpreter_useExtendedV1Id(
                                    instruction,
                                    context);
                    memberPosition = RTIXCdrStream_serializeV1ParameterHeader(
                            stream,
                            &memberStreamState,
                            extended,
                            isUnionDiscriminator
                                    ? 0
                                    : memberDataInstCommonParams->tcMemberInfo
                                              ->_representation._pid,
                            /* In XCDRv1, we only set mustUnderstand for
                               explicitly annotated members, and not for key
                               members or union discriminators, so not to break
                               backwards compatibility of the generated key
                               hashes.
                            */
                            isMustUnderstandMember ? RTI_XCDR_TRUE
                                                   : RTI_XCDR_FALSE);

                    if (memberPosition == NULL) {
                        GotoDoneWithLine();
                    }
                }
            }

            if (!memberValueSet) {
                /* Skip the member serialization instruction */
                i = dataInstIndex;
                if (!program->isCdr2 &&
                        program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                    if (!RTIXCdrStream_finishV1ParameterHeader(
                            stream,
                            &memberStreamState,
                            extended,
                            addPaddingToParameterLenght,
                            memberPosition)) {
                        GotoDoneWithLine();
                    }
                }
            } else {
                /* This variable will contain the index of the
                 * data instruction associated with this optional/mutable
                 * member.
                 *
                 * We use it below to not finalize the header until we process
                 * the data instruction
                 */
                memberWithHeaderIndex = dataInstIndex;
            }
        } break;
        case RTI_XCDR_SER_DHEADER_OPCODE: 
        {
            /* We don't serialize DHEADER for base class */
            if (!context->inBaseClass) {
                dHeaderPosition = RTIXCdrStream_serializeDHeader(stream);
                if (dHeaderPosition == NULL) {
                    GotoDoneWithLine();                
                }
            }
        } break;
        case RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE: 
        {
            dHeaderCollectionPosition = RTIXCdrStream_serializeDHeader(stream);
            if (dHeaderCollectionPosition == NULL) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SER_SENTINEL_HEADER_OPCODE:
        {
            /* We do not serialize sentinel for base class */
            if (!context->inBaseClass || program->serializeSentinelOnBase) {
                /* No need to finish parameter because we not provide state */
                if (RTIXCdrStream_serializeV1ParameterHeader(
                            stream,
                            NULL,
                            RTI_XCDR_FALSE,
                            RTI_XCDR_V1_PID_LIST_END,
                            program->disableMustUnderstandOnSentinel?
                                RTI_XCDR_FALSE:
                                RTI_XCDR_TRUE) == NULL) {
                    GotoDoneWithLine();
                }
            }
        } break;
        default:
            goto done;
        }

        if (dHeaderCollectionPosition != NULL
                && instruction->opcode != RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE) {
            if (!RTIXCdrStream_serializeDHeaderLength(
                    stream, 
                    dHeaderCollectionPosition)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }
            dHeaderCollectionPosition = NULL;
        }

        if (getUnionInstIndex && i == program->unionInsIndex) {
            /* 
             * Process union discriminator and select instruction
             * based on discriminator value
             */
            RTIXCdrLong caseValue;
            RTIXCdrBoolean failure;

            if (useMemberIndex) {
                RTIXCdrInterpreter_primitiveToLongWIndexMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind, 
                        failure);
            } else {
                RTIXCdrInterpreter_primitiveToLongMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind, 
                        failure);
            }
            if (failure) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            RTIXCdrProgram_getUnionMemberInstructionIndex(
                    program,
                    &i,
                    &instructionCount,
                    (RTIBool *) NULL, /* caseValueIdentifiesMember */
                    caseValue);

            if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                if (!program->isCdr2) {
                    /* Finalize discriminator parameter */
                    if (!RTIXCdrStream_finishV1ParameterHeader(
                            stream,
                            &memberStreamState,
                            extended,
                            addPaddingToParameterLenght,
                            memberPosition)) {
                        GotoDoneWithLine();
                    }
                }
                memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
            }

            getUnionInstIndex = RTI_XCDR_FALSE;
            continue;
        }
              
        if (memberWithHeaderIndex == i) {
            if (!program->isCdr2) {
                if (!RTIXCdrStream_finishV1ParameterHeader(
                        stream,
                        &memberStreamState,
                        extended,
                        addPaddingToParameterLenght,
                        memberPosition)) {
                    GotoDoneWithLine();
                }

                if (isUnion && i != program->unionInsIndex) {
                    /* We have to process the sentinel */
                    instructionCount = program->instructionCount;
                    /* We subtract 2 because 'i' will be increased at the end
                     * of the loop
                     */
                    i = instructionCount - 2;
                }
            } else if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                RTIXCdrStream_finishV2ParameterHeader(
                    stream,
                    memberPosition);
            }
            memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
        }

        i++;
    }
    
    if (dHeaderPosition != NULL) {
        if (!RTIXCdrStream_serializeDHeaderLength(
                stream, 
                dHeaderPosition)) {
            GotoDoneWithLine();
        }
    }

    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (context->logAllErrorsButExpectedSpaceErrors 
                && !context->expectedSpaceError
                && instruction != NULL) {
            RTIXCdrInterpreter_logSerializationError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }
    return result;
}

/* 
 * With respect to error logging, we do not log any error in this API related 
 * to running out of space. This is because this API is used by batching when 
 * max_data_bytes is set to detect situations in which we have to start new 
 * batch.
 */
RTIXCdrBoolean RTIXCdrInterpreter_fastSerializeSample(
        RTIXCdrStream *stream,
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    /* Uninitialized variables */
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong instructionCount;
    struct RTIXCdrInstruction *instruction;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrMemberValue memberValue;
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;

    /* Initialize variables */
    char *dHeaderPosition = NULL;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    char *dHeaderCollectionPosition = NULL;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    instructionCount = program->instructionCount;
    context->expectedSpaceError = RTI_XCDR_TRUE;

    for (i=0; i<instructionCount; i++) {
        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        switch (instruction->opcode) {
        case RTI_XCDR_SER_PRIMITIVE_OPCODE:
        {
            if (params->primitiveParams.mustAlign) {
                RTIXCdrStream_alignMacro(
                        stream,
                        params->primitiveParams.primitiveAlignment,
                        GotoDoneWithLine())
            }

            RTIXCdrInterpreter_getPrimitiveMemberValuePtr(
                    memberValue,
                    sample,
                    commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX]);

            if (params->primitiveParams.checkRange) {
                if (!RTIXCdrInterpreter_checkPrimitiveRange(
                        memberValue.value.ptr,
                        params->primitiveParams.primitiveKind,
                        program,
                        commonParams->tcMemberInfo,
                        context->annotations,
                        RTI_XCDR_FALSE,
                        RTI_XCDR_TRUE)) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
            }

            RTIXCdrInterpreter_serializePrimitiveArray(
                stream,
                memberValue.value.ptr,
                params->primitiveParams.primitiveSize,
                commonParams->count,
                params->primitiveParams.primitiveByteCount,
                GotoDoneWithLine());
        } break;
        case RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong seqElementCount = 0;
            void *samplePtr = sample;
            RTIXCdrUnsignedLongLong elOffset;
            RTIXCdrUnsignedLong j;
            RTIXCdrUnsignedLong arrayCount;
            RTIXCdrUnsignedLong byteCount;
            struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;

            memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
            arrayCount = commonParams->count;
            elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

            for (j=0; j<arrayCount; j++) {
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        samplePtr,
                        elOffset,
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        context->programData);

                if (seqElementCount > commonParams->memberTc->_maximumLength) {
                    logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (!RTIXCdrStream_serialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (seqElementCount == 0) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    continue;
                }

                if (params->primitiveParams.primitiveAlignment >
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT) {
                    RTIXCdrStream_alignMacro(
                            stream,
                            params->primitiveParams.primitiveAlignment,
                            GotoDoneWithLine());
                }

                if (memberValue.value.ptr == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (memberValue.isDiscontiguous) {
                    RTIXCdrUnsignedLong h;
                    char *valuePtr;

                    /* 
                     * We are dealing with a discontiguous sequence where
                     * the buffer is a buffer of pointers to the actual data
                     * elements. 
                     *
                     * These sequences are only allowed in C and traditional C++
                     * where RTIXCdrInterpreter_useMemberElementIndex is false.
                     */
                    for (h=0; h<seqElementCount; h++) {
                        char **valuePtrPtr = RTIXCdrUtility_staticCast(
                                char **,
                                memberValue.value.ptr);
                        valuePtr = *(valuePtrPtr + h);

                        if (valuePtr == NULL) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }

                        RTIXCdrInterpreter_serializePrimitiveArray(
                            stream,
                            valuePtr,
                            params->primitiveParams.primitiveSize,
                            1,
                            params->primitiveParams.origPrimitiveSize,
                            GotoDoneWithLine());
                    }
                } else {
                    RTIXCdrInterpreter_adjustSequenceElementCount(
                            seqElementCount,
                            byteCount, 
                            params);

                    RTIXCdrInterpreter_serializePrimitiveArray(
                        stream,
                        memberValue.value.ptr,
                        params->primitiveParams.primitiveSize,
                        seqElementCount,
                        byteCount,
                        GotoDoneWithLine());
                }

                elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            }
        } break;
        case RTI_XCDR_SER_DHEADER_OPCODE:
        {
            /* We don't serialize DHEADER for base class */
            if (!context->inBaseClass) {
                dHeaderPosition = RTIXCdrStream_serializeDHeader(stream);

                if (dHeaderPosition == NULL) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE: 
        {
            /* This is possible for arrays of sequences of primitives */
            dHeaderCollectionPosition = RTIXCdrStream_serializeDHeader(stream);

            if (dHeaderCollectionPosition == NULL) {
                GotoDoneWithLine();
            }
        } break;
        /* 
         * No need to process RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE because
         * types containing sequence of enums that must be serialized
         * with a dheader are not candidates for fast serialization.
         */
        default:
            goto done;
        }

        if (dHeaderCollectionPosition != NULL
                && instruction->opcode != RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE) {
            if (!RTIXCdrStream_serializeDHeaderLength(
                    stream, 
                    dHeaderCollectionPosition)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }
            dHeaderCollectionPosition = NULL;
        }
    }

    if (dHeaderPosition != NULL) {
        if (!RTIXCdrStream_serializeDHeaderLength(
                stream,
                dHeaderPosition)) {
            GotoDoneWithLine();
        }
    }

    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (context->logAllErrorsButExpectedSpaceErrors 
                && !context->expectedSpaceError
                && instruction != NULL) {
            RTIXCdrInterpreter_logSerializationError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }
    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleWithEncapsulationEx(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample, 
        struct RTIXCdrStream *stream,    
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrBoolean *expectedSpaceError)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean isLittleEndian = RTI_XCDR_FALSE;
    struct RTIXCdrTypePluginProgramContext context =
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    RTIXCdrBoolean isCdrV2;
    
    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);  
    
    isCdrV2 = RTIXCdrEncapsulationId_isCdrV2(encapsulationId);
    isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(
            encapsulationId);
    context.onlyKey = RTI_XCDR_FALSE;
    context.program = RTIXCdrInterpreterPrograms_getSerProgram(
            programs,
            isLittleEndian,
            isCdrV2,
            context.onlyKey);
    if (context.program == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GET_FAILURE_ID_s,
                "serialization program");
        return RTI_XCDR_FALSE;
    }

    context.typeCode = context.program->typeCode;
    context.inBaseClass = RTI_XCDR_FALSE;
        
    if (!isCdrV2) {
        RTIXCdrUnsignedLong maxSerSize;

        result = RTIXCdrInterpreter_getSerSampleMaxSizeWithEncapsulation(
                &maxSerSize,
                programs,
                encapsulationId);

        if (!result) {
            goto done;
        }

        if (maxSerSize > RTIXCdrUnsignedShort_MAX) {
            context.useXcdr1ExtendedId = RTI_XCDR_TRUE;
        } else {
            context.useXcdr1ExtendedId = RTI_XCDR_FALSE;
        }
    }

    RTIXCdrStream_serializeAndSetCdrEncapsulationWithEndiannessMacro(
            stream,
            context.program->encapsulationId,
            isLittleEndian,
            goto done);

    if (!RTIXCdrInterpreter_serializeSample(
            stream,
            sample,
            context.typeCode,
            context.program,
            &context)) {
        if (expectedSpaceError != NULL) {
            *expectedSpaceError = context.expectedSpaceError;
        }
        goto done;
    }

    result = RTI_XCDR_TRUE;
  done:   
    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleWithEncapsulation(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample, 
        struct RTIXCdrStream *stream,    
        RTIXCdrEncapsulationId encapsulationId)
{
    return RTIXCdrInterpreter_serializeSampleWithEncapsulationEx(
            programs,
            sample,
            stream,
            encapsulationId,
            NULL);
}

RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleToCdrBuffer(
        char *buffer,
        RTIXCdrUnsignedLong *length,
        const struct RTIXCdrInterpreterPrograms *programs,
        const void *sample,
        RTIXCdrEncapsulationId encapsulationId)
{
    struct RTIXCdrStream stream;

    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(length == NULL, return RTI_XCDR_FALSE);

    if (buffer == NULL) {
        if (!RTIXCdrInterpreter_getSerSampleSizeWithEncapsulation(
                length,
                (void *) sample,
                programs,
                encapsulationId)) {
            return RTI_XCDR_FALSE;
        }

        return RTI_XCDR_TRUE;
    }

    RTIXCdrLog_testPrecondition(buffer == NULL, return RTI_XCDR_FALSE);

    RTIXCdrStream_init(&stream);
    RTIXCdrStream_set(&stream, (char *) buffer, *length);

    if (!RTIXCdrInterpreter_serializeSampleWithEncapsulation(
            programs,
            (void *) sample, 
            &stream,    
            encapsulationId)) {
        return RTI_XCDR_FALSE;        
    }

    return RTI_XCDR_TRUE;
}

/*****************************************************************************/
/***** Deserialize & Skip ****************************************************/
/*****************************************************************************/

RTI_PRIVATE
void RTIXCdrInterpreter_logDeserializationError(
        const RTIXCdrTypeCode *tc,
        RTIXCdrStream *stream,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char *functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);

    switch (messageId) {
    case RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_ssd:
    case RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd:
    {
        RTIXCdrLogParam param[4];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);
        param[2].kind = RTI_XCDR_LOG_LONG_PARAM;
        param[2].value.lVal = runTimeParam->value.lVal;

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                3,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_STREAM_FAILURE_ID_ssuu:
    case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_STREAM_FAILURE_ID_ssuu:
    {
        RTIXCdrLogParam param[4];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);
        param[2].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[2].value.ulVal = runTimeParam->value.ulVal;
        param[3].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[3].value.ulVal = RTIXCdrStream_getRemainder(stream);

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                4,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
    case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
    {
        RTIXCdrLogParam param[4];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);
        param[2].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[2].value.ulVal = runTimeParam->value.ulVal;
        param[3].kind = RTI_XCDR_LOG_ULONG_PARAM;

        if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu) {
            param[3].value.ulVal = instruction->params.strParams.charMaxCount-1;
        } else {
            param[3].value.ulVal = commonParams->memberTc->_maximumLength;
        }

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                4,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss:
    case RTI_XCDR_LOG_CDR_DESERIALIZE_NOT_NULL_TERMINATED_STRING_FAILURE_ID_ss:
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
                messageId,
                2,
                param);
    } break;
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
                RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

RTI_PRIVATE
void RTIXCdrInterpreter_logSkipError(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInstruction *instruction,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLogParam *runTimeParam,
        const char * functionName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrCommonInsParameters *commonParams = NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(instruction == NULL, return);
    RTIXCdrLog_testPrecondition(functionName == NULL, return);

    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);

    switch (messageId) {
    case RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
    case RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
    {
        RTIXCdrLogParam param[4];

        RTIXCdrLog_testPrecondition(runTimeParam == NULL, return);

        param[0].kind = RTI_XCDR_LOG_STR_PARAM;
        param[0].value.strVal = tc->_name;
        param[1].kind = RTI_XCDR_LOG_STR_PARAM;
        param[1].value.strVal = RTIXCdrInstruction_getMemberName(
                instruction,
                tc);
        param[2].kind = RTI_XCDR_LOG_ULONG_PARAM;
        param[2].value.ulVal = runTimeParam->value.ulVal;
        param[3].kind = RTI_XCDR_LOG_ULONG_PARAM;

        if (messageId == RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu) {
            param[3].value.ulVal = instruction->params.strParams.charMaxCount-1;
        } else {
            param[3].value.ulVal = commonParams->memberTc->_maximumLength;
        }

        RTIXCdrLog_logWithParams(
                __FILE__,
                functionName,
                line,
                RTI_XCDR_LOG_EXCEPTION,
                messageId,
                4,
                param);
    } break;
    case RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss:
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
                messageId,
                2,
                param);
    } break;
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
                RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss,
                2,
                param);
    } break;
    }
}

/* 
 * With respect to error logging we do not log any error in this API related 
 * to running out of space 
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_skipString(
        const RTIXCdrTypeCode *tc,
        RTIXCdrStream *stream,
        struct RTIXCdrInstruction *instruction,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong charCount;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong stringCount;
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong elementCount;
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(
            commonParams == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    context->expectedSpaceError = RTI_XCDR_TRUE;

    elementCount = commonParams->count;

    for (i=0; i<elementCount; i++) {
        RTIXCdrUnsignedLong j;

        if (instruction->opcode == RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE ||
                instruction->opcode == RTI_XCDR_SKIP_STRING_SEQ_OPCODE) {
            if (commonParams->addSeqDHeader) {
                RTIXCdrBoolean corruptedHeader;

                if (!RTIXCdrStream_skipDHeader(
                        stream,
                        &corruptedHeader)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
                continue;
            }

            if (!RTIXCdrStream_deserialize4Byte(
                    stream,
                    &stringCount,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }

            if (stringCount > commonParams->memberTc->_maximumLength)
            {
                logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = stringCount;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }
        } else {
            stringCount = 1;
        }

        for (j=0; j<stringCount; j++) {
            if (!RTIXCdrStream_deserialize4Byte(
                    stream,
                    &charCount,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }
            
            if (params->strParams.charAlignment ==
                                RTI_XCDR_WCHAR_ALIGNMENT) {
                /* The serialized length for V2 strings is the number of
                 * bytes without including NULL terminated
                 */
                charCount = (charCount >> 1) + 1 /* NULL terminated */;
            }

            if (charCount > params->strParams.charMaxCount)
            {
                logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = charCount-1;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (charCount > 0) {
                RTIXCdrUnsignedLong byteCount;

                if (params->strParams.charAlignment ==
                        RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                    byteCount = charCount;
                } else if (params->strParams.charAlignment ==
                        RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                    byteCount = charCount << 2;
                } else {
                    byteCount = (charCount-1) << 1;
                }

                if (byteCount > 0) {
                    if (!RTIXCdrStream_skipNByte(
                            stream,
                            RTI_XCDR_ONE_BYTE_ALIGNMENT,
                            byteCount)) {
                        GotoDoneWithLine();
                    }
                }
            }
        }
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok 
            && context->logAllErrorsButExpectedSpaceErrors 
            && !context->expectedSpaceError) {
        RTIXCdrInterpreter_logSkipError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return ok;
}

/* 
 * With respect to error logging, we do not log any error in this API related 
 * to running out of space on the stream.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_skipPrimitiveSeq(
        const RTIXCdrTypeCode *tc,
        RTIXCdrStream *stream,
        struct RTIXCdrInstruction *instruction,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong seqCount;
    RTIXCdrUnsignedLong byteCount = 0;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong i;
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(
            commonParams == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    context->expectedSpaceError = RTI_XCDR_TRUE;
    seqCount = commonParams->count;

    for (i=0; i<seqCount; i++) {
        RTIXCdrUnsignedLong elementCount;

        if (commonParams->addSeqDHeader) {
            RTIXCdrBoolean corruptedHeader;

            if (!RTIXCdrStream_skipDHeader(
                    stream,
                    &corruptedHeader)) {
                if (corruptedHeader) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                }
                GotoDoneWithLine();
            }
            continue;
        }

        if (!RTIXCdrStream_deserialize4Byte(
                stream,
                &elementCount,
                RTI_XCDR_TRUE)) {
            GotoDoneWithLine();
        }

        if (elementCount == 0) {
            continue;
        }

        if (elementCount > commonParams->memberTc->_maximumLength) {
            logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
            runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
            runTimeLogParam.value.ulVal = elementCount;
            context->expectedSpaceError = RTI_XCDR_FALSE;
            GotoDoneWithLine();
        }

        RTIXCdrInterpreter_adjustSequenceElementCount(
                elementCount, 
                byteCount, 
                params);

        if (!RTIXCdrStream_skipNByte(
                stream,
                params->primitiveParams.primitiveAlignment,
                byteCount)) {
            GotoDoneWithLine();
        }
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok 
            && context->logAllErrorsButExpectedSpaceErrors 
            && !context->expectedSpaceError) {
        RTIXCdrInterpreter_logSkipError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return ok;
}

RTIXCdrBoolean RTIXCdrInterpreter_skipSample(
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_skipComplexSeq(
        const RTIXCdrTypeCode *tc,
        RTIXCdrStream *stream,
        struct RTIXCdrInstruction *instruction,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong seqCount;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong j;    
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;
    RTIXCdrTypePluginSkipFunction skipComplexFcn = NULL;
    RTIXCdrBoolean tmpInBaseClass;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(
            commonParams == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    context->expectedSpaceError = RTI_XCDR_TRUE;
    seqCount = commonParams->count;
    
    if (params->complexParams.typePlugin != NULL) {
        skipComplexFcn = params->complexParams.typePlugin->skipFnc;
    }

    tmpInBaseClass = context->inBaseClass;
        
    for (i=0; i<seqCount; i++) {
        RTIXCdrUnsignedLong elementCount;

        if (commonParams->addSeqDHeader) {
            RTIXCdrBoolean corruptedHeader;

            if (!RTIXCdrStream_skipDHeader(
                    stream,
                    &corruptedHeader)) {
                if (corruptedHeader) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                }
                GotoDoneWithLine();
            }
            continue;
        }

        if (!RTIXCdrStream_deserialize4Byte(
                stream,
                &elementCount,
                RTI_XCDR_TRUE)) {
            GotoDoneWithLine();
        }

        if (elementCount == 0) {
            continue;
        }

        if (elementCount > commonParams->memberTc->_maximumLength) {
            logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
            runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
            runTimeLogParam.value.ulVal = elementCount;
            context->expectedSpaceError = RTI_XCDR_FALSE;
            GotoDoneWithLine();
        }
        
        for (j=0; j<elementCount; j++) {
            /* We have to assign this here because the variables
             * can be overwritten by the nested calls
             */
            context->program = params->complexParams.program;
            context->typeCode = commonParams->memberTc;
            context->inBaseClass = params->complexParams.baseClass;

            if (skipComplexFcn == NULL) {
                if (!RTIXCdrInterpreter_skipSample(
                        stream,
                        context->typeCode,
                        context->program,
                        context)) {
                    GotoDoneWithLine();
                }
            } else {
                if (!skipComplexFcn(
                        context->endpointPluginData,
                        stream,
                        0,
                        1, 
                        context->endpointPluginQos)) {
                    GotoDoneWithLine();
                }
            }
        }
    }

    context->inBaseClass  = tmpInBaseClass;

    ok = RTI_XCDR_TRUE;
done:

    if (!ok
            && context->logAllErrorsButExpectedSpaceErrors 
            && !context->expectedSpaceError) {
        RTIXCdrInterpreter_logSkipError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return ok;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_deserializeString(
        void *sample,
        RTIXCdrStream *stream,
        const struct RTIXCdrProgram *program,
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrInstruction *instruction,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong stringCount;
    RTIXCdrBoolean isSkipError = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong charCount;
    RTIXCdrMemberValue memberValue = RTIXCdrMemberValue_INITIALIZER;
    struct RTIXCdrSampleAccessInfo * memberSampleAccessInfo;
    struct RTIXCdrSampleAccessInfo * strSampleAccessInfo;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong byteCount = 0;
    RTIXCdrUnsignedLong elementCount;
    RTIXCdrUnsignedLongLong elOffset, strOffset, externalStrOffset;
    RTIXCdrBoolean isSeq = RTI_XCDR_FALSE;
    /* Pointer to the sample containing the string */
    char *strSample = (char *)sample;
    /* Reference to the sample containing the string (double pointer) */
    char *strSampleRef = NULL;
    char *externalStrSample = (char *)sample;
    RTIXCdrCommonInsParameters *commonParams;
    RTIXCdrInsParameters *params;
    struct RTIXCdrTypeCodeMember *tcMember;
    struct RTIXCdrTypeCode *strTc;
    void *programData;
    RTIXCdrBoolean setNullChar = RTI_XCDR_FALSE;
    RTIXCdrOctet strRefMemberKind;
    RTIXCdrUnsignedLong strTypeSize;
    RTIXCdrUnsignedLong dheaderSeqSize;
    struct RTIXCdrStreamState streamSeqState;
    char *dheaderSeqPosition = NULL;
    RTIXCdrBoolean isDiscontiguous = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(
            commonParams == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
    tcMember = commonParams->tcMemberInfo;

    context->expectedSpaceError = RTI_XCDR_TRUE;
    programData =  context->programData;

    elementCount = commonParams->count;

    elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
    strOffset = elOffset;
    externalStrOffset = elOffset;

    if (instruction->opcode == RTI_XCDR_DESER_WSTRING_SEQ_OPCODE ||
            instruction->opcode == RTI_XCDR_DESER_STRING_SEQ_OPCODE) {
        isSeq = RTI_XCDR_TRUE;
        strSampleAccessInfo = commonParams->seqElementTc->_sampleAccessInfo;
        strTc = commonParams->seqElementTc;
        strRefMemberKind = RTI_XCDR_INTERPRETER_VALUE_MEMBER;
    } else {
        stringCount = 1;
        strSampleAccessInfo = memberSampleAccessInfo;
        strTc = commonParams->memberTc;
        strRefMemberKind = commonParams->refMemberKind;
    }

    strTypeSize = RTIXCdrInterpreter_getStrTypeSize(strSampleAccessInfo);

    if (params->strParams.charAlignment == RTI_XCDR_WCHAR_ALIGNMENT) {
        setNullChar = RTI_XCDR_TRUE;
    }

    for (i=0; i<elementCount; i++) {
        RTIXCdrUnsignedLong j;
        RTIXCdrBoolean failure;

        if (isSeq) {
            char *seqSample;

            if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                seqSample = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        ((char *)sample + elOffset));
                if (seqSample == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            } else {
                seqSample = (char *)sample;
            }
            
            if (commonParams->addSeqDHeader) {
                RTIXCdrBoolean corruptedHeader;

                if (!RTIXCdrStream_deserializeDHeader(
                        stream,
                        &corruptedHeader,
                        &dheaderSeqSize,
                        &dheaderSeqPosition,
                        &streamSeqState)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
            }

            if (!RTIXCdrStream_deserialize4Byte(
                    stream,
                    &stringCount,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }

            if (!RTIXCdrInterpreter_checkAndAdjustSequenceElementCount(
                    NULL,
                    NULL,
                    &stringCount,
                    &logMessageId,
                    stream,
                    instruction)) {
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = stringCount;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            RTIXCdrInterpreter_setMemberElementCount(
                    &failure,
                    memberValue,
                    stringCount,
                    seqSample,
                    commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                            0: elOffset,
                    commonParams->memberTc,
                    tcMember,
                    commonParams->refMemberKind,
                    memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                    (commonParams->memberTc->_maximumLength >= program->unboundedSize),
                    RTI_XCDR_TRUE,
                    programData);

            if (commonParams->refMemberKind !=
                    RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            } else {
                elOffset += program->externalReferenceSize;
            }

            isDiscontiguous = memberValue.isDiscontiguous;

            if (stringCount == 0) {
                if (dheaderSeqPosition != NULL) {
                    RTIXCdrStream_popState(stream, &streamSeqState);
                    RTIXCdrStream_setCurrentPosition(
                            stream,
                            dheaderSeqPosition);
                    RTIXCdrStream_increaseCurrentPosition(
                            stream,
                            dheaderSeqSize);
                    dheaderSeqPosition = NULL;
                }
                continue;
            }

            strSample = memberValue.value.ptr;
            strOffset = 0;

            if (failure) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (isDiscontiguous) {
                strSampleRef = strSample;
            }
        }

        for (j=0; j<stringCount; j++) {
            RTIXCdrBoolean zeroLength = RTI_XCDR_FALSE;
            
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                strSample = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        (externalStrSample + externalStrOffset));
                if (strSample == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
                /* Retrieving string from external: reset offset */
                strOffset = 0;
            } else if (isDiscontiguous) {
                if (j > 0) {
                    /*
                     * strSampleRef is a pointer to a pointer to a string
                     * which is a `char *` or a `RTIXCdrWchar *`
                     */

                    /* We advance to the next element in the sequence buffer */
                    strSampleRef += sizeof(void *);
                }

                /*
                 * From ser/deser purposes we are working assuming that is
                 * a sample with a single member that is a string or wstring
                 */
                strSample = (char *) (*(char ***) (void *) strSampleRef);
                strOffset = 0;
            }

            if (!RTIXCdrStream_deserialize4Byte(
                    stream,
                    &charCount,
                    RTI_XCDR_TRUE)) {
                GotoDoneWithLine();
            }
            
            if (params->strParams.charAlignment ==
                                RTI_XCDR_WCHAR_ALIGNMENT) {
                /* The serialized length for V2 strings is the number of
                 * bytes without including NULL terminated
                 */
                charCount = (charCount >> 1) + 1 /* NULL terminated */;
            } else if (charCount == 0) {
                /* On deserialization strings must contain at least the NULL
                 * character
                 */
                zeroLength = RTI_XCDR_TRUE;
                charCount++;
            }

            if (charCount > params->strParams.charMaxCount) {
                logMessageId = RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = charCount-1;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (params->strParams.charAlignment ==
                    RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                byteCount = charCount;
            } else if (params->strParams.charAlignment ==
                    RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                byteCount = charCount << 2;
            } else {
                /* Subtract NULL terminated */
                byteCount = (charCount-1) << 1;
            }

            if (!zeroLength 
                    && byteCount > RTIXCdrStream_getRemainder(stream)) {
                logMessageId = RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_STREAM_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = charCount-1;
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            RTIXCdrInterpreter_setStrElementCount(
                    &failure,
                    memberValue,
                    charCount,
                    strSample,
                    strOffset,
                    strTc,
                    /* We do not provide tcMember because otherwise the
                     * accesor functions may think that the string is optional
                     */
                    isSeq?NULL:tcMember,
                    strRefMemberKind,
                    (strTc->_maximumLength >= program->unboundedSize),
                    programData);

            if (failure) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            if (charCount == 1) {
                if (RTIXCdrInterpreter_useMemberElementIndex(strSampleAccessInfo)) {
                    memberValue.value.wVal = 0;
                    strSampleAccessInfo->setMemberElementValueFcn(
                            strSample,
                            strOffset,
                            0,
                            memberValue,
                            strTc,
                            tcMember,
                            RTI_XCDR_FALSE, /* string should be allocated at this point */
                            programData);
                } else if (memberValue.value.ptr != NULL) {
                    /* In modern C++ empty strings may not have a buffer 
                     * associated with them. This is why we check for NULL
                     */
                    RTIXCdrMemory_zero(
                            memberValue.value.ptr,
                            (params->strParams.charAlignment ==
                                    RTI_XCDR_ONE_BYTE_ALIGNMENT)?
                                sizeof(RTIXCdrChar):
                                sizeof(RTIXCdrWchar));
                }
                
                if (params->strParams.charAlignment !=
                        RTI_XCDR_WCHAR_ALIGNMENT &&
                        !zeroLength) {
                    if (!RTIXCdrStream_skipNByte(
                            stream,
                            RTI_XCDR_ONE_BYTE_ALIGNMENT,
                            params->strParams.charSize)) {
                        isSkipError = RTI_XCDR_TRUE;
                        GotoDoneWithLine();
                    }
                }

                /* Goto next string */
                strOffset += strTypeSize;
                continue;
            }

            if (params->strParams.charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT
                    || (!stream->_needByteSwap
                            && params->strParams.charAlignment == RTI_XCDR_WCHAR_ALIGNMENT
                            && !RTIXCdrInterpreter_useMemberElementIndex(strSampleAccessInfo))) {
                if (!RTIXCdrStream_deserializeNByte(
                        stream,
                        memberValue.value.ptr,
                        params->strParams.charAlignment,
                        byteCount)) {
                    GotoDoneWithLine();
                }

                if (params->strParams.charAlignment
                    == RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                    /*
                     * CORE-11203: Check that the string finalizes with NULL
                     * This is only necessary for string and not wstring
                     * because wstring in XCDR2 format
                     * (RTI_XCDR_WCHAR_ALIGNMENT) are not NULL-terminated on the
                     * wire.
                     */
                    if (memberValue.value.ptr[byteCount - 1] != 0) {
                        logMessageId =
                                RTI_XCDR_LOG_CDR_DESERIALIZE_NOT_NULL_TERMINATED_STRING_FAILURE_ID_ss;
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                }
            } else {
                RTIXCdrUnsignedLong h;
                
                if (!RTIXCdrStream_checkSize(stream, byteCount)) {
                    GotoDoneWithLine();
                }

                if (params->strParams.charAlignment ==
                        RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                    RTIXCdrLegacyWchar wc;

                    /* Here we are converting 4 bytes to 2 bytes.
                     * We are choosing to not check ranges for
                     * performance reasons and also considering that
                     * the wire encoding for wstrings has been
                     * standardized to UTF-16
                     */
                    if (!RTIXCdrInterpreter_useMemberElementIndex(
                            strSampleAccessInfo)) {
                        for (h=0; h<charCount; h++) {
                            RTIXCdrStream_deserialize4ByteFast(
                                    stream,
                                    &wc);
                            *((RTIXCdrWchar *)(void *)memberValue.value.ptr) =
                                (RTIXCdrWchar)wc;
                            memberValue.value.ptr += RTI_XCDR_WCHAR_SIZE;
                        }
                    } else {
                        for (h=0; h<charCount; h++) {
                            RTIXCdrStream_deserialize4ByteFast(
                                    stream,
                                    &wc);
                            memberValue.value.wVal = (RTIXCdrWchar)wc;
                            strSampleAccessInfo->setMemberElementValueFcn(
                                    strSample,
                                    strOffset,
                                    h,
                                    memberValue,
                                    strTc,
                                    tcMember,
                                    RTI_XCDR_FALSE, /* string should be allocated at this point */
                                    programData);
                        }
                    }

                    if (wc != 0) {
                        /*
                         * CORE-11203: Check that the wstring finalizes with
                         * NULL This is only necessary for legacy wstring
                         * (XCDR1) because wstring in XCDR2 format wstrings are
                         * not NULL-terminated on the wire
                         */
                        logMessageId =
                                RTI_XCDR_LOG_CDR_DESERIALIZE_NOT_NULL_TERMINATED_STRING_FAILURE_ID_ss;
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }
                } else {
                    /* wstring do not include NULL terminated char */
                    if (!RTIXCdrInterpreter_useMemberElementIndex(
                            strSampleAccessInfo)) {
                        RTIXCdrWchar *wc = (RTIXCdrWchar *)
                                (void *)memberValue.value.ptr;

                        for (h=0; h<(charCount-1); h++) {
                            RTIXCdrStream_deserialize2ByteFast(
                                    stream,
                                    wc);
                            wc++;
                        }
                    } else {
                        for (h=0; h<(charCount-1); h++) {
                            RTIXCdrStream_deserialize2ByteFast(
                                    stream,
                                    &memberValue.value.wVal);
                            strSampleAccessInfo->setMemberElementValueFcn(
                                    strSample,
                                    strOffset,
                                    h,
                                    memberValue,
                                    strTc,
                                    tcMember,
                                    RTI_XCDR_FALSE, /* string should be allocated at this point */
                                    programData);
                        }
                    }
                }
            }

            if (setNullChar) {
                /* The NULL terminated was not part of the stream
                 * and we have to set it separate
                 */
                if (RTIXCdrInterpreter_useMemberElementIndex(strSampleAccessInfo)) {
                    memberValue.value.wVal = 0;
                    strSampleAccessInfo->setMemberElementValueFcn(
                            strSample,
                            strOffset,
                            charCount-1,
                            memberValue,
                            strTc,
                            tcMember,
                            RTI_XCDR_FALSE, /* string should be allocated at this point */
                            programData);
                } else if (memberValue.value.ptr != NULL) {
                    /* In modern C++ empty strings may not have a buffer
                     * associated with them. This is why we check for NULL
                     */
                    RTIXCdrMemory_zero(
                            memberValue.value.ptr + byteCount,
                            sizeof(RTIXCdrWchar));
                }
            }

            /* Goto next string */
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                externalStrOffset += program->externalReferenceSize;
            } else {
                strOffset += strTypeSize;
            }
        }

        if (dheaderSeqPosition != NULL) {
            RTIXCdrStream_popState(stream, &streamSeqState);
            RTIXCdrStream_setCurrentPosition(stream, dheaderSeqPosition);
            RTIXCdrStream_increaseCurrentPosition(stream, dheaderSeqSize);
            dheaderSeqPosition = NULL;
        }
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok 
            && context->logAllErrorsButExpectedSpaceErrors 
            && !context->expectedSpaceError) {
        if (isSkipError) {
            RTIXCdrInterpreter_logSkipError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        } else {
            RTIXCdrInterpreter_logDeserializationError(
                    tc,
                    stream,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return ok;
}

/* isDisc is used to indicate that we are deserializing a discriminator. This
 * is important because when deserializing a discriminator we have to ignore
 * sampleAssignability->acceptUnknownEnumValue if the discriminator is an enum
 * or a typedef to an enum.
 *
 * Otherwise, changing a potential disc unknown value to the default enum
 * because acceptUnknownEnumValue is set to TRUE or rejecting it because
 * acceptUnknownEnumValue is set to FALSE would mask the value of
 * acceptUnknownUnionDiscriminator that is applied afterwards.
 */
#define RTIXCdrInterpreter_deserializePrimitiveArray( \
    stream, \
    valuePtr, \
    primitiveSize, \
    primitiveCount, \
    primitiveByteCount, \
    isDisc, \
    annotations, \
    returnIfError) \
{ \
    RTIXCdrUnsignedLong k; \
    \
    if (!RTIXCdrStream_checkSize((stream), (primitiveByteCount))) { \
        returnIfError; \
    } \
    \
    switch ((primitiveSize)) { \
        case RTI_XCDR_ONE_BYTE_SIZE: \
        { \
            RTIXCdrStream_deserializeNByteFast( \
                (stream), \
                (valuePtr), \
                (primitiveByteCount)); \
        } break; \
        case RTI_XCDR_TWO_BYTE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                RTIXCdrStream_deserialize2ByteFast( \
                    (stream), \
                    (void *)(valuePtr)); \
                (valuePtr) += (primitiveSize); \
            } \
            (valuePtr) -= (primitiveSize); \
        } break; \
        case RTI_XCDR_FOUR_BYTE_SIZE: \
        { \
            if (params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR && \
                    !program->isCdr2) { \
                RTIXCdrLegacyWchar __wc; \
                for (k=0; k<(primitiveCount); k++) { \
                    RTIXCdrStream_deserialize4ByteFast( \
                        (stream), \
                        &__wc); \
                    /* Here we are converting 4 bytes to 2 bytes. \
                     * We are choosing to not check ranges for \
                     * performance reasons and also considering that \
                     * the wire encoding for wstrings has been \
                     * standardized to UTF-16 \
                     */ \
                    *((RTIXCdrWchar *)(void *)(valuePtr)) = (RTIXCdrWchar)__wc; \
                    (valuePtr) += sizeof(RTIXCdrWchar); \
                } \
                (valuePtr) -= sizeof(RTIXCdrWchar); \
            } else { \
                for (k=0; k<(primitiveCount); k++) { \
                    RTIXCdrStream_deserialize4ByteFast( \
                        (stream), \
                        (void *)(valuePtr)); \
                    \
                    if (!(isDisc) && \
                            params->primitiveParams.enumTc != NULL) { \
                        RTIXCdrBoolean __isValid; \
                        \
                        RTIXCdrTypeCode_isValidEnumValue( \
                                params->primitiveParams.enumTc, \
                                &__isValid, \
                                *(RTIXCdrEnum *)(void *)valuePtr); \
                        \
                        if (!__isValid) { \
                            RTIXCdrEnum __defaultEnumValue; \
                            if (sampleAssignability == NULL || \
                                    !sampleAssignability->acceptUnknownEnumValue) { \
                                logMessageId = RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd; \
                                runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM; \
                                runTimeLogParam.value.lVal = *(RTIXCdrEnum *)(void *)valuePtr; \
                                context->expectedSpaceError = RTI_XCDR_FALSE; \
                                returnIfError; \
                            } \
                            __defaultEnumValue = \
                                    RTIXCdrInterpreter_getDefaultEnumValue( \
                                            &params->primitiveParams.parent, \
                                            (annotations));\
                            *(RTIXCdrEnum *)(void *)(valuePtr) = __defaultEnumValue; \
                        } \
                    } \
                    \
                    (valuePtr) += (primitiveSize); \
                } \
                (valuePtr) -= (primitiveSize); \
            } \
        } break; \
        case RTI_XCDR_EIGHT_BYTE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                RTIXCdrStream_deserialize8ByteFast( \
                    (stream), \
                    (void *)(valuePtr)); \
                (valuePtr) += (primitiveSize); \
            } \
            (valuePtr) -= (primitiveSize); \
        } break; \
        case RTI_XCDR_LONG_DOUBLE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                RTIXCdrStream_deserialize16ByteFast( \
                    (stream), \
                    (void *)(valuePtr)); \
                (valuePtr) += (primitiveSize); \
            } \
            (valuePtr) -= (primitiveSize); \
        } break; \
        default: \
        { \
            returnIfError; \
        } break; \
    } \
}

#define RTIXCdrInterpreter_deserializeOneBytePrimitiveArray( \
    stream, \
    valuePtr, \
    primitiveByteCount, \
    returnIfError) \
{ \
    if (!RTIXCdrStream_checkSize((stream), (primitiveByteCount))) { \
        returnIfError; \
    } \
    \
    RTIXCdrStream_deserializeNByteFast( \
            (stream), \
            (valuePtr), \
            (primitiveByteCount)); \
}

#define RTIXCdrInterpreter_deserializeExternalPrimitiveArray( \
    stream, \
    valuePtr, \
    primitiveSize, \
    primitiveCount, \
    primitiveByteCount, \
    annotations, \
    returnIfError) \
{ \
    RTIXCdrUnsignedLong k; \
    void *__externalValuePtr; \
    \
    if (!RTIXCdrStream_checkSize((stream), (primitiveByteCount))) { \
        returnIfError; \
    } \
    \
    switch ((primitiveSize)) { \
        case RTI_XCDR_ONE_BYTE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                RTIXCdrStream_deserialize1ByteFast( \
                    (stream), \
                    __externalValuePtr); \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } break; \
        case RTI_XCDR_TWO_BYTE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                RTIXCdrStream_deserialize2ByteFast( \
                    (stream), \
                    __externalValuePtr); \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } break; \
        case RTI_XCDR_FOUR_BYTE_SIZE: \
        { \
            if (params->primitiveParams.primitiveKind == RTI_XCDR_TK_WCHAR && \
                    !program->isCdr2) { \
                RTIXCdrLegacyWchar __wc; \
                for (k=0; k<(primitiveCount); k++) { \
                    __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                    if (__externalValuePtr == NULL) { \
                        context->expectedSpaceError = RTI_XCDR_FALSE; \
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                        returnIfError; \
                    } \
                    RTIXCdrStream_deserialize4ByteFast( \
                        (stream), \
                        &__wc); \
                    /* Here we are converting 4 bytes to 2 bytes. \
                     * We are choosing to not check ranges for \
                     * performance reasons and also considering that \
                     * the wire encoding for wstrings has been \
                     * standardized to UTF-16 \
                     */ \
                    *((RTIXCdrWchar *)(__externalValuePtr)) = (RTIXCdrWchar)__wc; \
                    (valuePtr) += program->externalReferenceSize; \
                } \
            } else { \
                for (k=0; k<(primitiveCount); k++) { \
                    __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                    if (__externalValuePtr == NULL) { \
                        context->expectedSpaceError = RTI_XCDR_FALSE; \
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                        returnIfError; \
                    } \
                    RTIXCdrStream_deserialize4ByteFast( \
                        (stream), \
                        __externalValuePtr); \
                    \
                    if (params->primitiveParams.enumTc != NULL) { \
                        RTIXCdrBoolean __isValid; \
                        \
                        RTIXCdrTypeCode_isValidEnumValue( \
                                params->primitiveParams.enumTc, \
                                &__isValid, \
                                *(RTIXCdrEnum *)__externalValuePtr); \
                        \
                        if (!__isValid) { \
                            RTIXCdrEnum __defaultEnumValue; \
                            if (sampleAssignability == NULL || \
                                    !sampleAssignability->acceptUnknownEnumValue) { \
                                logMessageId = RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd; \
                                runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM; \
                                runTimeLogParam.value.lVal = *(RTIXCdrEnum *)__externalValuePtr; \
                                context->expectedSpaceError = RTI_XCDR_FALSE; \
                                returnIfError; \
                            } \
                            \
                            __defaultEnumValue = \
                                    RTIXCdrInterpreter_getDefaultEnumValue( \
                                            &params->primitiveParams.parent, \
                                            (annotations)); \
                            *(RTIXCdrEnum *)__externalValuePtr = __defaultEnumValue; \
                        } \
                    } \
                    \
                    (valuePtr) += program->externalReferenceSize; \
                } \
            } \
        } break; \
        case RTI_XCDR_EIGHT_BYTE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                RTIXCdrStream_deserialize8ByteFast( \
                    (stream), \
                    __externalValuePtr); \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } break; \
        case RTI_XCDR_LONG_DOUBLE_SIZE: \
        { \
            for (k=0; k<(primitiveCount); k++) { \
                __externalValuePtr = RTIXCdrProgram_getExternalRefValuePtr(program, (valuePtr)); \
                if (__externalValuePtr == NULL) { \
                    context->expectedSpaceError = RTI_XCDR_FALSE; \
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss; \
                    returnIfError; \
                } \
                RTIXCdrStream_deserialize16ByteFast( \
                    (stream), \
                    __externalValuePtr); \
                (valuePtr) += program->externalReferenceSize; \
            } \
        } break; \
        default: \
        { \
            returnIfError; \
        } break; \
    } \
}

#define RTIXCdrInterpreter_deserializeComplex( \
    samplePtr, \
    stream, \
    deserializeFnc, \
    context, \
    sampleAssignability, \
    returnIfError) \
if ((deserializeFnc) == NULL) { \
    if (!RTIXCdrInterpreter_deserializeSample( \
            (samplePtr), \
            (stream), \
            (context)->typeCode, \
            (context)->program, \
            (sampleAssignability), \
            (context))) { \
        returnIfError; \
    } \
} else { \
    if (!deserializeFnc( \
            (context)->endpointPluginData, \
            (samplePtr), \
            (stream), \
            0, \
            1, \
            (context)->endpointPluginQos)) { \
        returnIfError; \
    } \
}

#define RTIXCdrInterpreter_isDiscInstIndex(i) \
    (processUnionDisc && i == program->unionInsIndex)

/* 
 * A space error is an error in which we run out of stream bytes.
 *
 * That is completely ok in cases where we deserialized a derived type
 * after receiving a base type. However, running out of stream will be
 * a real error in the following scenarios:
 * 
 * 1) When the type is final, because the publishing and subscribing types 
 * match exactly so the stream should have exactly the number of expected bytes. 
 * 2) When the number of bytes left in the stream is greater or equal than
 * the expected padding. This threshold is introduced to account for the padding 
 * bytes (up to 3) that will be added at the end of a sample serialization. 
 * Because CORE-9042 is not implemented yet, we cannot make an accurate decision 
 * if the remainder is less than this padding.
 * 
 * Unexpected space errors due to tampering string and sequence lengths
 * are not covered by this macro. They are detected and reported when the 
 * string or sequence length is deserialized.
 * 
 * Additional information can be found here CORE-11494.
 * 
 * This function should be inlined by compilers.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_isUnexpectedSpaceError(
        const RTIXCdrProgram *program,
        const RTIXCdrStream *stream)
{
    RTIXCdrUnsignedLong padding;

    if (program->extKind == RTI_XCDR_FINAL_EXTENSIBILITY) {
        return RTI_XCDR_TRUE;
    }

    padding = ((RTI_XCDR_RTPS_SUBMESSAGE_ALIGNMENT 
            - (RTIXCdrStream_getCurrentPositionOffset(stream) 
                    % RTI_XCDR_RTPS_SUBMESSAGE_ALIGNMENT))
            & (RTI_XCDR_RTPS_SUBMESSAGE_ALIGNMENT - 1));

    if (RTIXCdrStream_getRemainder(stream) > padding) {
        return RTI_XCDR_TRUE;
    }

    return RTI_XCDR_FALSE;
}

/**
 * @brief Sets the sample discriminator to default value.
 * 
 * @param memberValue. InOut. This parameter must point to the discriminator
 * in the sample if we are not accessing by index. Otherwise,
 * memberValue.ptr points to the local discriminator value in memberValue (bVal
 * , for example).
 * @param sample InOut. The sample.
 * @param program In. The union program.
 * @param instruction In. The discriminator instruction.
 * @param memberSampleAccessInfo In. Discriminator sample access info.
 * @param context In. Context.
 * 
 * @return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_setDefaultUnionDisc(
        RTIXCdrMemberValue *memberValue,
        void *sample,
        const RTIXCdrProgram *program,
        const struct RTIXCdrInstruction *instruction,
        const struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean failure;
    
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(memberValue == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL,
            return RTI_XCDR_FALSE);

    RTIXCdrInterpreter_longToPrimitiveMacro(
            *memberValue,
            program->defaultUnionDisc,
            program->unionDiscKind,
            failure);
    if (failure) {
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrInterpreter_useMemberElementIndex(
            memberSampleAccessInfo)) {
        const RTIXCdrCommonInsParameters *commonParams =
                RTIXCdrInstruction_getCommonParams(instruction);
        const RTIXCdrInsParameters *params =
                &instruction->params;

        RTIXCdrLog_testPrecondition(commonParams == NULL, return RTI_XCDR_FALSE);

        memberSampleAccessInfo->setMemberElementValueFcn(
                sample,
                commonParams->memberAccessInfo.bindingMemberValueOffset[0],
                0,
                *memberValue,
                commonParams->memberTc,
                commonParams->tcMemberInfo,
                params->primitiveParams.origPrimitiveSize,
                context->programData);
    }

    return RTI_XCDR_TRUE;
}

/**
 * The following function must be called to process an unknown discriminator
 * member value. Note the usage of the word member value.
 * Unknown discriminator member value is different than unknown discriminator
 * value. Unknown discriminator member value means that there is no case value
 * for the discriminator.
 * 
 * This function will set the sample discriminator to the default union
 * discriminator if acceptUnknownUnionDiscriminator is set to
 * RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR_AND_SELECT_DEFAULT.
 *
 * @param caseValue. Out. If there is an error caseValue is
 * initialized to the discriminator value.
 * @param memberValue. InOut. This parameter must point to the discriminator
 * in the language binding if we are not accessing by index. Otherwise,
 * memberValue.ptr points to the local discriminator value in memberValue (bVal
 * , for example).
 * @param sample InOut. The sample.
 * @param program In. The union program.
 * @param instruction In. The discriminator instruction.
 * @param memberSampleAccessInfo In. Discriminator sample access info.
 * @param sampleAssignability In. Sample assignability property.
 * @param context InOut. Context.
 * 
 * @return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_processUnknownDisc(
        RTIXCdrMemberValue *memberValue,
        void *sample,
        const RTIXCdrProgram *program,
        const struct RTIXCdrInstruction *instruction,
        const struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;

    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(program == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(memberValue == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(context == NULL,
            return RTI_XCDR_FALSE);

    if (sampleAssignability == NULL ||
            sampleAssignability->acceptUnknownUnionDiscriminator
                    == RTI_XCDR_REJECT_UNKNOWN_DISCRIMINATOR) {
        RTIXCdrLong caseValue = (RTIXCdrLong) RTI_XCDR_TYPECODE_INVALID_INDEX;
        RTIXCdrBoolean failure;

        /*
         * Receiving a discriminator without a member value (case) is not 
         * allowed and we fail.
         */
        context->expectedSpaceError = RTI_XCDR_FALSE;

        RTIXCdrInterpreter_primitiveToLongMacro(
                caseValue,
                *memberValue,
                program->unionDiscKind,
                failure);

        if (failure) {
            GotoDoneWithLine();
        }

        logMessageId = RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_ssd;
        runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM;
        runTimeLogParam.value.lVal = caseValue;
        GotoDoneWithLine();
    }

    if (sampleAssignability->acceptUnknownUnionDiscriminator
            == RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR) {
        /*
         * We just accept the discriminator value as it comes. This is the mode
         * that is compatible with the latest XTypes specification 1.3.
         * 
         * Note that if the discriminator is an enum and the value is not valid,
         * it will be changed by RTIXCdrInterpreter_validateUnionEnumDisc.
         */
        return RTI_XCDR_TRUE;
    }

    if (!RTIXCdrInterpreter_setDefaultUnionDisc(
            memberValue,
            sample,
            program,
            instruction,
            memberSampleAccessInfo,
            context)) {
        context->expectedSpaceError = RTI_XCDR_FALSE;
        GotoDoneWithLine();
    }

    result = RTI_XCDR_TRUE;
  done:

    if (context->logAllErrorsButExpectedSpaceErrors
              && !context->expectedSpaceError) {
          RTIXCdrInterpreter_logDeserializationError(
                  program->typeCode,
                  NULL,
                  instruction,
                  logMessageId,
                  &runTimeLogParam,
                  RTI_XCDR_FUNCTION_NAME,
                  logLineNumber);
    }

    return result;
}

/**
 * @brief The following function validates the value of an enum discriminator.
 * 
 * If the value pointed by memberValue is not valid and acceptUnknownEnumValue 
 * is set to RTI_XCDR_TRUE, this function changes it to the default
 * discriminator for the union and returns RTI_XCDR_TRUE. 
 * 
 * If acceptUnknownEnumValue is RTI_XCDR_FALSE, this function returns 
 * RTI_XCDR_FALSE.
 * 
 * @param memberValue. InOut. This parameter must point to the discriminator
 * in the language binding if we are not accessing by index. Otherwise,
 * memberValue.ptr points to the local discriminator value in memberValue (bVal
 * , for example).
 * @param sample InOut. The sample.
 * @param program In. The program.
 * @param instruction In. Discriminator instruction.
 * @param sampleAssignability In. Sample assignability property.
 * @param memberSampleAccessInfo In. Member sample access info.
 * @param context InOut. Context.
 * 
 * @return RTI_XCDR_TRUE if success. Otherwise, RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_validateUnionEnumDisc(
        RTIXCdrMemberValue *memberValue,
        void *sample,
        const RTIXCdrProgram *program,
        const struct RTIXCdrInstruction *instruction,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        const struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean isValidEnum;
    RTIXCdrEnum enumValue;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;

    RTIXCdrLog_testPrecondition(
            memberValue == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program->unionDiscKind != RTI_XCDR_TK_ENUM,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            instruction->params.primitiveParams.enumTc == NULL,
            return RTI_XCDR_FALSE);

    enumValue = *((RTIXCdrEnum *) (void *) memberValue->value.ptr);

    RTIXCdrTypeCode_isValidEnumValue(
            instruction->params.primitiveParams.enumTc,
            &isValidEnum,
            enumValue);

    if (!isValidEnum) {
        if (sampleAssignability == NULL
                || sampleAssignability->acceptUnknownUnionDiscriminator ==
                        RTI_XCDR_REJECT_UNKNOWN_DISCRIMINATOR) {
            logMessageId =
                    RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd;
            runTimeLogParam.kind = RTI_XCDR_LOG_LONG_PARAM;
            runTimeLogParam.value.lVal = enumValue;
            context->expectedSpaceError = RTI_XCDR_FALSE;
            GotoDoneWithLine();
        } else {
            if (!RTIXCdrInterpreter_setDefaultUnionDisc(
                    memberValue,
                    sample,
                    program,
                    instruction,
                    memberSampleAccessInfo,
                    context)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }
        }
    }

    result = RTI_XCDR_TRUE;
  done:

    if (context->logAllErrorsButExpectedSpaceErrors
              && !context->expectedSpaceError) {
          RTIXCdrInterpreter_logDeserializationError(
                  program->typeCode,
                  NULL,
                  instruction,
                  logMessageId,
                  &runTimeLogParam,
                  RTI_XCDR_FUNCTION_NAME,
                  logLineNumber);
    }

    return result;
}


RTIXCdrBoolean RTIXCdrInterpreter_fullDeserializeSample(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context)
{
    /* Uninitialized variables */
    RTIXCdrUnsignedLong instructionCount;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong dheaderSize;
    RTIXCdrUnsignedLong dheaderCollectionSize;
    RTIXCdrUnsignedLong dheaderSeqSize;
    struct RTIXCdrStreamState streamState;
    struct RTIXCdrStreamState streamCollectionState;
    struct RTIXCdrStreamState streamSeqState;
    struct RTIXCdrStreamState memberStreamState;
    RTIXCdrUnsignedLong memberLength;
    RTIXCdrBoolean flatData;
    RTIXCdrUnsignedLong seqElementCount;
    RTIXCdrMemberValue memberValue;
    RTIXCdrOctet primitiveSize;
    RTIXCdrUnsignedLong arrayCount;
    RTIXCdrUnsignedLong j;
    RTIXCdrBoolean failure;
    RTIXCdrUnsignedLongLong elOffset;
    RTIXCdrBoolean processUnionDisc;
    RTIXCdrUnionInitializeInfo unionInitInfo;
    RTIXCdrLong caseValue;
    void *programData;

    /* Initialized variables */
    RTIXCdrUnsignedLong primitiveByteCount = 0;
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrInsParameters *params = NULL;
    struct RTIXCdrInstruction *instruction = NULL;
    char *valuePtr = NULL;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean isSkipError = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    char *dheaderPosition = NULL;
    char *dheaderCollectionPosition = NULL;
    char *dheaderSeqPosition = NULL;
    RTIXCdrUnsignedLong optionalMemberIndex = RTIXCdrUnsignedLong_MAX;
    RTIXCdrUnsignedLong offsetIndex = 0;
    char *basePosition = NULL;
    RTIXCdrUnsignedLong baseInstIndex = 0;
    RTIXCdrBoolean initializeUnion = RTI_XCDR_FALSE;
    struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;
    RTIXCdrBoolean corruptedHeader = RTI_XCDR_TRUE;
    RTIBool deserializeNextMutableHeader = RTI_TRUE;
    struct RTIXCdrInstructionIndex *instructionIndex = NULL;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    
    processUnionDisc = program->unionDiscKind != RTI_XCDR_TK_NULL;
    instructionCount = program->instructionCount;

    context->expectedSpaceError = RTI_XCDR_TRUE;
    programData = context->programData;

    flatData = program->isFlatDataProgram;

    if (flatData) {
        offsetIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               sample)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
    }

    /* Check if the program for a derived type injected an instructionIndex.
     *
     * If provided, we expect it to contain member entries for this type too,
     * so we will use it for local lookups, and propagate it "up the hierarchy",
     * if this type also has a base type.
     */
    if (context->instructionIndex != NULL) {
        /* We only inject the instructionIndex when a mutable derived type
         * deserializes its base type, so let's do a sanity check.
         */
        if (program->instructionIndex == NULL) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
                    "unexpected instruction index injected "
                    "into non-mutable program");
            context->expectedSpaceError = RTI_XCDR_FALSE;
            GotoDoneWithLine();
        }
        instructionIndex = context->instructionIndex;
        /* We reset the injected context pointer here to make sure that it will
         * not be accidentally propagated to
         * RTIXCdrInterpreter_deserializeComplex(), e.g. if this type doesn't
         * have a base type but does have a complex sequence member.
         * Technically, we could avoid doing this if the program has a base
         * type, because the first instruction will call the base program, then
         * reset the pointer. Nonetheless, it is harmless to reset it here,
         * and it makes the code more robust to future changes (e.g. if we ever
         * decided to flatten and merge programs).
         */
        context->instructionIndex = NULL;
    } else {
        /* Use the local program's instructionIndex if available */
        instructionIndex = program->instructionIndex;
    }

    if (program->instructionIndex != NULL) {
        instructionCount = RTIXCdrUnsignedLong_MAX;
        if (program->isCdr2) {
            if (!context->inBaseClass) {
                if (!RTIXCdrStream_deserializeDHeader(
                            stream,
                            &corruptedHeader,
                            &dheaderSize,
                            &dheaderPosition,
                            &streamState)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
            }
            baseInstIndex++;
            i = 1;
        }
    }

    for (; i<instructionCount;) {
        if (program->instructionIndex != NULL && deserializeNextMutableHeader) {
            RTIXCdrUnsignedLong parameterId = 0;
            RTIXCdrBoolean extended;
            RTIXCdrBoolean mustUnderstand;
            RTIXCdrBoolean knownMember;
            RTIXCdrUnsignedLong instIndex;
            const RTIXCdrTypeCode *sourceTypeCode = NULL;

            if (i==baseInstIndex && program->hasBase) {
                /* If the program has a base
                 * (first instruction) we process base first
                 */
                basePosition = RTIXCdrStream_getCurrentPosition(stream);

                if (!program->serializeSentinelOnBase) {
                    RTIXCdrStream_pushState(
                            stream,
                            &memberStreamState,
                            RTI_XCDR_FALSE,
                            0);
                }
            } else {
                if (basePosition != NULL) {
                    if (!program->serializeSentinelOnBase) {
                        RTIXCdrStream_popState(stream, &memberStreamState);
                        RTIXCdrStream_setCurrentPosition(stream, basePosition);
                    }
                    basePosition = NULL;
                }

                if (!program->isCdr2) {
                    if (!RTIXCdrStream_deserializeV1ParameterHeader(
                            stream,
                            &memberStreamState,
                            &parameterId,
                            &memberLength,
                            &extended,
                            &mustUnderstand)) {
                        GotoDoneWithLine();
                    }

                    if (parameterId == RTI_XCDR_V1_PID_LIST_END) {
                        RTIXCdrStream_moveToNextParameterHeader(
                                stream,
                                &memberStreamState,
                                memberLength);
                        break;
                    }
                } else {
                    if (!RTIXCdrStream_deserializeV2ParameterHeader(
                            stream,
                            &memberStreamState,
                            &parameterId,
                            &memberLength,
                            &mustUnderstand)) {
                        GotoDoneWithLine();
                    }
                }

                /* Search for the member ID */
                RTIXCdrInstructionIndex_getInstructionIndexByMemberId(
                        instructionIndex,
                        &instIndex,
                        &knownMember,
                        &sourceTypeCode,
                        program->typeCode,
                        parameterId);

                if (instIndex == RTI_XCDR_TYPECODE_INVALID_INDEX) {
                    RTIXCdrStream_moveToNextParameterHeader(
                            stream,
                            &memberStreamState,
                            memberLength);

                    if (processUnionDisc) {
                        if (!RTIXCdrInterpreter_processUnknownDisc(
                                &memberValue,
                                sample,
                                program,
                                instruction,
                                memberSampleAccessInfo,
                                sampleAssignability,
                                context)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                    } else if (mustUnderstand && !knownMember) {
                        /* knowmMember indicates that the member belongs to
                         * another type in the hierarchy, so we should skip it
                         * instead of considering it an error.
                         */
                        RTIXCdrLog_logStrULong(
                                RTI_XCDR_LOG_EXCEPTION,
                                RTI_XCDR_LOG_CDR_DESERIALIZE_UNKNOWN_PARAMETER_ID_su,
                                tc->_name,
                                parameterId);
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }

                    continue;
                }

                if (processUnionDisc && instIndex != program->unionInsIndex) {
                    /* 
                     * If instIndex != discInsIndex then we have already
                     * processed the union's discriminator, so set this to
                     * false. For non-mutable types, this boolean gets set to
                     * false at the end of the for loop
                     */
                    processUnionDisc = RTI_XCDR_FALSE;
                }

                if (instIndex > 0
                    && program->instructions[instIndex - 1].opcode
                            == RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE) {
                    /* 
                     * Typically only the data instruction 
                     * (identified by instIndex) needs to be executed for each 
                     * member. However, when dheaderInNonPrimitiveCollections is
                     * set to true, we may have an additional instruction for
                     * non-primitive sequences and arrays and in such case
                     * we need to start with the dheader instruction.
                     */
                    instIndex--;
                }

                i = instIndex;
            }
        }

        deserializeNextMutableHeader = RTI_TRUE;
        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);
        if (commonParams->memberTc != NULL) {
            memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
        }
        /*
         * memberTc can only be NULL if isHeaderOpcode is true, because header
         * operations are the only ones that do not require access to/knowledge
         * of the member's typecode that is being operated on
         */
        RTIXCdrLog_testPrecondition(
                !RTIXCdrInstruction_isHeaderOpcode(instruction->opcode)
                        && commonParams->memberTc == NULL,
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine());

        /* The order of the case statement is important for performance reasons
         * We always favor primitives
         * 
         * Also, in this function, deserialization instructions should be
         * before than skip
         */
        switch (instruction->opcode) {
        case RTI_XCDR_DESER_PRIMITIVE_OPCODE:
        {
            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_DESER_PRIMITIVE_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            primitiveSize = params->primitiveParams.primitiveSize;

            if (params->primitiveParams.mustAlign)
            {
                RTIXCdrStream_alignMacro(
                        stream,
                        params->primitiveParams.primitiveAlignment,
                        GotoDoneWithLine());
            }

            if (!RTIXCdrInterpreter_useMemberElementIndex(
                    memberSampleAccessInfo)) {
                /* memberTc cannot be NULL at this point */
                /* coverity[var_deref_op : FALSE] */
                /* coverity[cert_exp34_c_violation : FALSE] */
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        NULL,
                        sample,
                        commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                        0,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        params->primitiveParams.primitiveByteCount,
                        programData);
    
                if (memberValue.value.ptr == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (processUnionDisc 
                        && tc->_typePlugin != NULL 
                        && tc->_typePlugin->initializeSampleWParamsFnc != NULL) {
                    RTIXCdrInterpreter_primitiveToLongMacro(
                            unionInitInfo.discValuePrev,
                            memberValue,
                            program->unionDiscKind,
                            failure);
                    if (failure) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        GotoDoneWithLine();
                    }

                    initializeUnion = RTI_XCDR_TRUE;
                }

                if (params->primitiveParams.checkRange) {
                    valuePtr = memberValue.value.ptr;
                }

                if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    RTIXCdrInterpreter_deserializeExternalPrimitiveArray(
                            stream,
                            memberValue.value.ptr,
                            params->primitiveParams.primitiveSize,
                            commonParams->count,
                            params->primitiveParams.primitiveByteCount,
                            context->annotations,
                            GotoDoneWithLine());
                    if (params->primitiveParams.checkRange) {
                        if (!RTIXCdrInterpreter_checkPrimitiveRange(
                                valuePtr,
                                params->primitiveParams.primitiveKind,
                                program,
                                commonParams->tcMemberInfo,
                                context->annotations,
                                RTI_XCDR_TRUE,
                                RTI_XCDR_FALSE)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                    }
                } else {
                    RTIXCdrInterpreter_deserializePrimitiveArray(
                            stream,
                            memberValue.value.ptr,
                            params->primitiveParams.primitiveSize,
                            commonParams->count,
                            params->primitiveParams.primitiveByteCount,
                            RTIXCdrInterpreter_isDiscInstIndex(i),
                            context->annotations,
                            GotoDoneWithLine());
                    if (params->primitiveParams.checkRange) {
                        if (!RTIXCdrInterpreter_checkPrimitiveRange(
                                valuePtr,
                                params->primitiveParams.primitiveKind,
                                program,
                                commonParams->tcMemberInfo,
                                context->annotations,
                                RTI_XCDR_FALSE,
                                RTI_XCDR_FALSE)) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine();
                        }
                    }
                }

            } else {
                RTIXCdrUnsignedLong desCount;
                RTIXCdrUnsignedLong primitiveCount;
                RTIXCdrUnsignedLong h;

                if (params->primitiveParams.origPrimitiveSize != primitiveSize &&
                        primitiveSize == 1) {
                    primitiveCount =
                            commonParams->count/params->primitiveParams.origPrimitiveSize;
                    desCount = params->primitiveParams.origPrimitiveSize;
                } else {
                    desCount = 1;
                    primitiveCount = commonParams->count;
                }

                primitiveByteCount = primitiveSize*desCount;

                if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    char *externalRefPtr = NULL;
                    char *externalRefValuePtr = NULL;

                    externalRefPtr = (char *)sample +
                            commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex];

                    for (h=0; h<primitiveCount; h++) {
                        valuePtr = (char *)&memberValue.value;
                        externalRefValuePtr = RTIXCdrProgram_getExternalRefValuePtr(
                                program, externalRefPtr);

                        RTIXCdrInterpreter_deserializePrimitiveArray(
                                stream,
                                valuePtr,
                                primitiveSize,
                                desCount,
                                primitiveByteCount,
                                RTIXCdrInterpreter_isDiscInstIndex(i),
                                context->annotations,
                                GotoDoneWithLine());
                        memberSampleAccessInfo->setMemberElementValueFcn(
                                externalRefValuePtr,
                                0,
                                0,
                                memberValue,
                                commonParams->memberTc,
                                commonParams->tcMemberInfo,
                                (commonParams->refMemberKind ==
                                        RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER),
                                programData);

                        externalRefPtr += program->externalReferenceSize;
                    }
                } else {
                    for (h=0; h<primitiveCount; h++) {
                        valuePtr = (char *)&memberValue.value;

                        RTIXCdrInterpreter_deserializePrimitiveArray(
                                stream,
                                valuePtr,
                                primitiveSize,
                                desCount,
                                primitiveByteCount,
                                RTIXCdrInterpreter_isDiscInstIndex(i),
                                context->annotations,
                                GotoDoneWithLine());
                        memberSampleAccessInfo->setMemberElementValueFcn(
                                sample,
                                commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                                h,
                                memberValue,
                                commonParams->memberTc,
                                commonParams->tcMemberInfo,
                                (commonParams->refMemberKind ==
                                        RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER),
                                programData);
                    }
                }
            }

            if (processUnionDisc && initializeUnion) {
                /* This code path is exercised only by Dyndata right now.
                 * This is why we dont consider the case in which values are 
                 * accessed by index
                 */
                RTIXCdrInterpreter_primitiveToLongMacro(
                        unionInitInfo.discValueNext,
                        memberValue,
                        program->unionDiscKind,
                        failure);
                if (failure) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (!tc->_typePlugin->initializeSampleWParamsFnc(
                        sample,
                        tc,
                        &unionInitInfo,
                        programData,
                        tc->_typePlugin->typePluginParam)) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrLong primitiveAlignment;
            char *samplePtr = (char *) sample;
            RTIXCdrUnsignedLong primitiveSeqElementCount = 0;

            /* 
             * memberSampleAccessInfo should not be NULL because all language
             * bindings have sample access info for complex members
             */
            RTIXCdrLog_testPrecondition(
                    memberSampleAccessInfo == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine());

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            arrayCount = commonParams->count;
            primitiveAlignment =
                    params->primitiveParams.primitiveAlignment;
            elOffset =
                    commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
            primitiveSize = params->primitiveParams.primitiveSize;

            for (j=0; j<arrayCount; j++) {
                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                if (commonParams->addSeqDHeader) {
                    if (!RTIXCdrStream_deserializeDHeader(
                            stream,
                            &corruptedHeader,
                            &dheaderSeqSize,
                            &dheaderSeqPosition,
                            &streamSeqState)) {
                        if (corruptedHeader) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                        }
                        GotoDoneWithLine();
                    }
                }
            
                if (!RTIXCdrStream_deserialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (seqElementCount > 0 &&
                        primitiveAlignment > RTI_XCDR_FOUR_BYTE_ALIGNMENT) {
                    /* We don't have to align the first element of the sequence
                     * if the alignment is greater than the length alignment
                     */
                    RTIXCdrStream_alignMacro(
                            stream,
                            primitiveAlignment,
                            GotoDoneWithLine());
                }

                if (!RTIXCdrInterpreter_checkAndAdjustSequenceElementCount(
                        &primitiveSeqElementCount,
                        &primitiveByteCount,
                        &seqElementCount,
                        &logMessageId,
                        stream,
                        instruction)) {
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                /* memberTc cannot be NULL at this point */
                /* coverity[var_deref_op : FALSE] */
                /* coverity[cert_exp34_c_violation : FALSE] */
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        commonParams->memberTc->_maximumLength >= program->unboundedSize,
                        RTI_XCDR_FALSE,
                        programData);
                
                if (failure) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (seqElementCount == 0) {
                    elOffset +=  memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    if (dheaderSeqPosition != NULL) {
                        RTIXCdrStream_popState(stream, &streamSeqState);
                        RTIXCdrStream_setCurrentPosition(stream, dheaderSeqPosition);
                        RTIXCdrStream_increaseCurrentPosition(
                                stream,
                                dheaderSeqSize);
                        dheaderSeqPosition = NULL;
                    }
                    continue;
                }

                if (memberValue.isDiscontiguous) {
                    RTIXCdrUnsignedLong desCount;
                    RTIXCdrUnsignedLong h;

                    if (params->primitiveParams.origPrimitiveSize
                                != primitiveSize
                        && primitiveSize == 1) {
                        desCount = params->primitiveParams.origPrimitiveSize;
                    } else {
                        desCount = 1;
                    }

                    primitiveByteCount = primitiveSize * desCount;

                    for (h = 0; h < seqElementCount; h++) {
                        valuePtr = *(((char **) (void *) memberValue.value.ptr) + h);

                        if (valuePtr == NULL) {
                            GotoDoneWithLine();
                        }

                        RTIXCdrInterpreter_deserializePrimitiveArray(
                                stream,
                                valuePtr,
                                primitiveSize,
                                desCount,
                                primitiveByteCount,
                                RTI_XCDR_FALSE,
                                context->annotations,
                                GotoDoneWithLine());
                    }
                } else if (!RTIXCdrInterpreter_useMemberElementIndex(
                                   memberSampleAccessInfo)) {
                    RTIXCdrInterpreter_deserializePrimitiveArray(
                        stream,
                        memberValue.value.ptr,
                        primitiveSize,
                        primitiveSeqElementCount,
                        primitiveByteCount,
                        RTI_XCDR_FALSE,
                        context->annotations,
                        GotoDoneWithLine());
                } else {
                    RTIXCdrUnsignedLong desCount;
                    RTIXCdrUnsignedLong h;

                    if (params->primitiveParams.origPrimitiveSize != primitiveSize &&
                            primitiveSize == 1) {
                        desCount = params->primitiveParams.origPrimitiveSize;
                    } else {
                        desCount = 1;
                    }

                    primitiveByteCount = primitiveSize * desCount;

                    for (h=0; h<seqElementCount; h++) {                        
                        valuePtr = (char *)&memberValue.value;
                        
                        RTIXCdrInterpreter_deserializePrimitiveArray(
                            stream,
                            valuePtr,
                            primitiveSize,
                            desCount,
                            primitiveByteCount,
                            RTI_XCDR_FALSE,
                            context->annotations,
                            GotoDoneWithLine());
                        memberSampleAccessInfo->setMemberElementValueFcn(
                                sample,
                                elOffset,
                                h,
                                memberValue,
                                commonParams->seqElementTc,
                                commonParams->tcMemberInfo,
                                RTI_XCDR_FALSE,
                                programData);
                    }
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (dheaderSeqPosition != NULL) {
                    RTIXCdrStream_popState(stream, &streamSeqState);
                    RTIXCdrStream_setCurrentPosition(
                            stream,
                            dheaderSeqPosition);
                    RTIXCdrStream_increaseCurrentPosition(
                            stream,
                            dheaderSeqSize);
                    dheaderSeqPosition = NULL;
                }
            }
        } break;
        case RTI_XCDR_DESER_WSTRING_OPCODE:
        case RTI_XCDR_DESER_STRING_OPCODE:
        case RTI_XCDR_DESER_WSTRING_SEQ_OPCODE:
        case RTI_XCDR_DESER_STRING_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_deserializeString(
                    sample,
                    stream,
                    program,
                    tc,
                    instruction,
                    context)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_DESER_COMPLEX_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginDeserializeFunction deserializeFnc = NULL;
            struct RTIXCdrTypeCodeAnnotations *tmpAnnotations;
            char *samplePtr;

            /* 
             * memberSampleAccessInfo should not be NULL because all language
             * bindings have sample access info for complex members
             */
            RTIXCdrLog_testPrecondition(
                    memberSampleAccessInfo == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine());

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_DESER_COMPLEX_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());
            
            if (params->complexParams.typePlugin != NULL) {
                if (!context->onlyKey) {
                    deserializeFnc = (RTIXCdrTypePluginDeserializeFunction)
                            params->complexParams.typePlugin->deserializeFnc;
                } else {
                    deserializeFnc = (RTIXCdrTypePluginDeserializeFunction)
                            params->complexParams.typePlugin->deserializeKeyFnc;
                }
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                    0,
                    commonParams->memberTc,
                    commonParams->tcMemberInfo,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                    programData);
            if (memberValue.value.ptr == NULL) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            tmpInBaseClass = context->inBaseClass;
            tmpAnnotations = context->annotations;

            /* If the instruction is deserializing a base type, inject
             * this program's instructionIndex into the base type's program.
             */
            if (params->complexParams.baseClass) {
                context->instructionIndex = instructionIndex;
            }

            RTIXCdrInterpreter_assignContextAnnotations(
                    context,
                    commonParams,
                    tc);

            for (j=0; j<commonParams->count; j++) {
                if (j != 0) {
                    /*
                     * For arrays, sample access info needs to be populated if
                     * we have a member that is not external.
                     */
                    RTIXCdrInterpreter_nextArrayElementPtr(
                            memberValue.value.ptr,
                            flatData,
                            memberSampleAccessInfo->typeSize,
                            &params->complexParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr = memberValue.value.ptr;
                } else {
                    samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program, memberValue.value.ptr);
                    if (samplePtr == NULL) {
                        if (program->getExternalRefPointerFcn == NULL) {
                            RTIXCdrMemberValue externalMemberValue;

                            externalMemberValue.value.ptr = NULL;

                            /* 
                             * Try to allocate the memory. This will help with
                             * use cases in which we create circular 
                             * dependencies using external members.
                             * 
                             * If program->getExternalRefPointerFcn != NULL
                             * it is responsibility of this method to allocate
                             * the memory.
                             */
                            RTIXCdrInterpreter_allocateMemberValueIfNeeded(
                                    externalMemberValue,
                                    memberValue.value.ptr,
                                    0,
                                    commonParams->memberTc,
                                    memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                                    RTI_XCDR_FALSE,
                                    programData);
                            samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                                    program, memberValue.value.ptr);
                        }

                        if (samplePtr == NULL) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                            logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                            GotoDoneWithLine();
                        }
                    }
                }

                /* We have to assign this here because the variables
                 * can be overwritten by the nested calls
                 */
                context->program = params->complexParams.program;
                context->typeCode = commonParams->memberTc;
                context->inBaseClass = params->complexParams.baseClass;

                RTIXCdrInterpreter_deserializeComplex(
                    samplePtr,
                    stream,
                    deserializeFnc,
                    context,
                    sampleAssignability,
                    GotoDoneWithLine());
            }

            context->inBaseClass = tmpInBaseClass;
            context->annotations = tmpAnnotations;
            context->instructionIndex = NULL;
        } break;
        case RTI_XCDR_DESER_COMPLEX_SEQ_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginDeserializeFunction deserializeComplexFcn = NULL;
            RTIXCdrUnsignedLong h;
            void *samplePtr = sample;
            struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo =
                    commonParams->seqElementTc->_sampleAccessInfo;

            /* 
             * memberSampleAccessInfo should not be NULL because all language
             * bindings have sample access info for sequences
             */
            RTIXCdrLog_testPrecondition(
                    memberSampleAccessInfo == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine());

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_DESER_COMPLEX_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            if (params->complexParams.typePlugin != NULL) {
                if (!context->onlyKey) {
                    deserializeComplexFcn = (RTIXCdrTypePluginDeserializeFunction)
                            params->complexParams.typePlugin->deserializeFnc;
                } else {
                    deserializeComplexFcn = (RTIXCdrTypePluginDeserializeFunction)
                            params->complexParams.typePlugin->deserializeKeyFnc;
                }
            }

            arrayCount = commonParams->count;
            elOffset =
                    commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
            
            tmpInBaseClass = context->inBaseClass;

            for (j=0; j<arrayCount; j++) {
                RTIXCdrBoolean isDiscontiguous;

                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                if (commonParams->addSeqDHeader) {
                    if (!RTIXCdrStream_deserializeDHeader(
                            stream,
                            &corruptedHeader,
                            &dheaderSeqSize,
                            &dheaderSeqPosition,
                            &streamSeqState)) {
                        if (corruptedHeader) {
                            context->expectedSpaceError = RTI_XCDR_FALSE;
                        }
                        GotoDoneWithLine();
                    }
                }
            
                if (!RTIXCdrStream_deserialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (!RTIXCdrInterpreter_checkAndAdjustSequenceElementCount(
                        NULL,
                        NULL,
                        &seqElementCount,
                        &logMessageId,
                        stream,
                        instruction)) {
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
                
                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        commonParams->memberTc->_maximumLength >= program->unboundedSize,
                        RTI_XCDR_TRUE,
                        programData);
                
                if (failure) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                isDiscontiguous = memberValue.isDiscontiguous;

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (seqElementCount == 0) {
                    if (dheaderSeqPosition != NULL) {
                        RTIXCdrStream_popState(stream, &streamSeqState);
                        RTIXCdrStream_setCurrentPosition(stream, dheaderSeqPosition);
                        RTIXCdrStream_increaseCurrentPosition(
                                stream,
                                dheaderSeqSize);
                        dheaderSeqPosition = NULL;
                    }
                    continue;
                }

                for (h = 0; h < seqElementCount; h++) {
                    char *valPtr;

                    /* We have to assign this here because the variables
                     * can be overwritten by the nested calls
                     */
                    context->program = params->complexParams.program;
                    context->typeCode = params->complexParams.program->typeCode;
                    context->inBaseClass = params->complexParams.baseClass;

                    if (isDiscontiguous) {
                        valPtr = *(((char **) (void *) memberValue.value.ptr));
                    } else {
                        valPtr = memberValue.value.ptr;
                    }

                    RTIXCdrInterpreter_deserializeComplex(
                            valPtr,
                            stream,
                            deserializeComplexFcn,
                            context,
                            sampleAssignability,
                            GotoDoneWithLine());

                    if (isDiscontiguous) {
                        /* In discontiguous case the elements are pointers */
                        memberValue.value.ptr += sizeof(void *);
                    } else {
                        memberValue.value.ptr +=
                                seqElementSampleAccessInfo
                                        ->typeSize[NON_FLAT_DATA_INDEX];
                    }
                }

                if (dheaderSeqPosition != NULL) {
                    RTIXCdrStream_popState(stream, &streamSeqState);
                    RTIXCdrStream_setCurrentPosition(stream, dheaderSeqPosition);
                    RTIXCdrStream_increaseCurrentPosition(
                            stream,
                            dheaderSeqSize);
                    dheaderSeqPosition = NULL;
                }
            }

            context->inBaseClass = tmpInBaseClass;
        } break;
        case RTI_XCDR_DESER_MEMBER_HEADER_OPCODE:
        {
            if (program->isCdr2) {
                RTIXCdrBoolean memberValueSet;

                if (!RTIXCdrStream_deserialize1Byte(
                        stream,
                        &memberValueSet)) {
                    GotoDoneWithLine();
                }

                if (!memberValueSet) {
                    i = RTIXCdrInterpreter_getMemberDataInsIndex(program, i);
                }
            } else {
                RTIXCdrUnsignedLong parameterId;
                RTIXCdrBoolean extended;
                RTIXCdrBoolean mustUnderstand;

                if (!RTIXCdrStream_deserializeV1ParameterHeader(
                        stream,
                        &memberStreamState,
                        &parameterId,
                        &memberLength,
                        &extended,
                        &mustUnderstand)) {
                    GotoDoneWithLine();
                }

                if (memberLength == 0) {
                    RTIXCdrStream_moveToNextParameterHeader(
                            stream,
                            &memberStreamState,
                            memberLength);
                    i++;
                } else {
                    optionalMemberIndex = i+1;
                }
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (!RTIXCdrStream_skipNByte(
                    stream,
                    params->primitiveParams.primitiveAlignment,
                    commonParams->count)) {
                isSkipError = RTI_XCDR_TRUE;
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        {
            if (dheaderCollectionPosition != NULL) {
                /* we do not have to skip the sequence if we have the DHEADER */
                RTIXCdrStream_popState(stream, &streamCollectionState);
                RTIXCdrStream_setCurrentPosition(
                        stream,
                        dheaderCollectionPosition);
                RTIXCdrStream_increaseCurrentPosition(
                        stream,
                        dheaderCollectionSize);
                dheaderCollectionPosition = NULL;
            } else {
                if (!RTIXCdrInterpreter_skipPrimitiveSeq(
                        tc,
                        stream,
                        instruction,
                        context)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_STRING_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_OPCODE:
        case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        {
            if (dheaderCollectionPosition != NULL) {
                /* we do not have to skip the sequence if we have the DHEADER */
                RTIXCdrStream_popState(stream, &streamCollectionState);
                RTIXCdrStream_setCurrentPosition(
                        stream,
                        dheaderCollectionPosition);
                RTIXCdrStream_increaseCurrentPosition(
                        stream,
                        dheaderCollectionSize);
                dheaderCollectionPosition = NULL;
            } else {
                if (!RTIXCdrInterpreter_skipString(
                        tc,
                        stream,
                        instruction,
                        context)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        {
            /* If the type is mutable there is no need to skip a base class
             * because it would be a NOOP operation. This is why we protect
             * that use case with an if
             */
            if (!params->complexParams.baseClass
                || program->instructionIndex == NULL) {
                RTIXCdrTypePluginSkipFunction skipComplexFcn = NULL;
                RTIXCdrBoolean tmpInBaseClass;

                /*
                 * Coverity is not smart enough to recognize that
                 * RTIXCdrInstruction_isHeaderOpcode must be false for
                 * RTI_XCDR_SKIP_COMPLEX_OPCODE. We add this precondition to
                 * appease Coverity.
                 */
                RTIXCdrLog_testPrecondition(
                        commonParams->memberTc == NULL,
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                                GotoDoneWithLine());

                if (params->complexParams.typePlugin != NULL) {
                    skipComplexFcn = params->complexParams.typePlugin->skipFnc;
                }

                tmpInBaseClass = context->inBaseClass;

                for (j=0; j<commonParams->count; j++) {
                    /* We have to assign this here because the variables
                     * can be overwritten by the nested calls
                     */
                    context->program = params->complexParams.program;
                    context->typeCode = commonParams->memberTc;
                    context->inBaseClass = params->complexParams.baseClass;

                    if (skipComplexFcn == NULL) {
                        if (!RTIXCdrInterpreter_skipSample(
                                stream,
                                context->typeCode,
                                context->program,
                                context)) {
                                isSkipError = RTI_XCDR_TRUE;
                                GotoDoneWithLine();
                        }
                    } else {
                        if (!skipComplexFcn(
                                context->endpointPluginData,
                                stream,
                                0,
                                1,
                                context->endpointPluginQos)) {
                            isSkipError = RTI_XCDR_TRUE;
                            GotoDoneWithLine();
                        }
                    }
                }

                context->inBaseClass = tmpInBaseClass;
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        {
            if (dheaderCollectionPosition != NULL) {
                /* we do not have to skip the sequence if we have the DHEADER */
                RTIXCdrStream_popState(stream, &streamCollectionState);
                RTIXCdrStream_setCurrentPosition(
                        stream,
                        dheaderCollectionPosition);
                RTIXCdrStream_increaseCurrentPosition(
                        stream,
                        dheaderCollectionSize);
                dheaderCollectionPosition = NULL;
            } else {
                if (!RTIXCdrInterpreter_skipComplexSeq(
                        tc,
                        stream,
                        instruction,
                        context)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        {            
            if (program->isCdr2) {
                RTIXCdrBoolean memberValueSet;

                if (!RTIXCdrStream_deserialize1Byte(
                        stream,
                        &memberValueSet)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }

                if (!memberValueSet) {
                    i = RTIXCdrInterpreter_getMemberDataInsIndex(program, i);
                }
            } else if (instruction->params.memberHeaderParams.hasV1NestedMemberHeaders) {
                RTIXCdrUnsignedLong parameterId;
                RTIXCdrBoolean extended;

                /* 
                 * CORE-13829: We cannot skip to the next value of this optional 
                 * member because the next value may contain other headers and 
                 * this would cause the stream to be out of sync as we would 
                 * miss alignment reset operations.
                 */
                if (!RTIXCdrStream_deserializeV1ParameterHeader(
                        stream,
                        &memberStreamState,
                        &parameterId,
                        &memberLength,
                        &extended,
                        NULL)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }

                if (memberLength == 0) {
                    RTIXCdrStream_moveToNextParameterHeader(
                            stream,
                            &memberStreamState,
                            memberLength);
                    i++;
                } else {
                    optionalMemberIndex = i+1;
                }
            } else {
                if (!RTIXCdrStream_deserializeAndSkipV1ParameterHeader(
                        stream,
                        NULL)) {
                    isSkipError = RTI_XCDR_TRUE;
                    GotoDoneWithLine();
                }
                i++;
            }
        } break;
        case RTI_XCDR_DESER_DHEADER_OPCODE: 
        {
            if (!context->inBaseClass) {
                /* 
                 * Even though this call is within the for loop, it can be 
                 * executed only once.
                 */
                if (!RTIXCdrStream_deserializeDHeader(
                        stream,
                        &corruptedHeader,
                        &dheaderSize,
                        &dheaderPosition,
                        &streamState)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE:
        {
            if (!RTIXCdrStream_deserializeDHeader(
                    stream,
                    &corruptedHeader,
                    &dheaderCollectionSize,
                    &dheaderCollectionPosition,
                    &streamCollectionState)) {
                if (corruptedHeader) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                }
                GotoDoneWithLine();
            }

            /* 
             * This member has two instructions. We need to skip the code at the
             * beginning of the for loop that process the member mutable headers
             */
            deserializeNextMutableHeader = RTI_FALSE;
        } break;
        default:
        {
            GotoDoneWithLine();
        } break;
        }

        if (dheaderCollectionPosition != NULL
                && instruction->opcode != RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE) {
            RTIXCdrStream_popState(stream, &streamCollectionState);
            RTIXCdrStream_setCurrentPosition(stream, dheaderCollectionPosition);
            RTIXCdrStream_increaseCurrentPosition(
                    stream,
                    dheaderCollectionSize);
            dheaderCollectionPosition = NULL;
        }

        if (processUnionDisc && i == program->unionInsIndex 
                && program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
            RTIBool caseValueIdentifiesMember;
            RTIBool *caseValueIdentifiesMemberPtr = &caseValueIdentifiesMember;

            /* 
             * Process union discriminator and select instruction
             * based on discriminator value
             */
            if (RTIXCdrInterpreter_useMemberElementIndex(
                    memberSampleAccessInfo)) {
                RTIXCdrInterpreter_primitiveToLongWIndexMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind,
                        failure);
            } else {
                RTIXCdrInterpreter_primitiveToLongMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind,
                        failure);
            }

            if (failure) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            RTIXCdrProgram_getUnionMemberInstructionIndex(
                    program,
                    &i,
                    &instructionCount,
                    caseValueIdentifiesMemberPtr,
                    caseValue);

            if (!caseValueIdentifiesMember) {
                /* We enter here if we did not find a case value (member value)
                 * for the received discriminator
                 */
                if (program->extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY) {
                    if (program->isCdr2) {
                        /* Two things are possible:
                         * 1) The received discriminator does not have a member
                         * value
                         * 2) The received discriminator has a member value but
                         * it is unknown
                         *
                         * In the first case we have to accept the
                         * discriminator as it comes
                         *
                         * In the second case we will only accept the
                         * discriminator if accept_unknown_union_discriminator
                         * is set to RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR or 
                         * RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR_AND_SELECT_DEFAULT.
                         *
                         * In reality, this property should be called
                         * accept_unknown_union_member_value instead of
                         * accept_unknown_union_discriminator.
                         */
                        if (dheaderSize !=
                                RTIXCdr_TCKind_g_primitiveCdrSizes[1][program->unionDiscKind]) {
                            /* We are in case 2 above. We receive unknown member
                             * value and we must reset the discriminator to its
                             * default value if accept_unknown_union_discriminator
                             * is set to RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR_AND_SELECT_DEFAULT.
                             * Otherwise, if accept_unknown_union_discriminator
                             * is set to RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR
                             * we just preserve the value.
                             *
                             * If dheaderSize was equal to
                             * RTIXCdr_TCKind_g_primitiveCdrSizes[1][program->unionDiscKind]
                             * we would be in case 1 in which there is no member
                             * value (or case) for the discriminator
                             */
                            if (!RTIXCdrInterpreter_processUnknownDisc(
                                    &memberValue,
                                    sample,
                                    program,
                                    instruction,
                                    memberSampleAccessInfo,
                                    sampleAssignability,
                                    context)) {
                                context->expectedSpaceError = RTI_XCDR_FALSE;
                                GotoDoneWithLine();
                            }
                        }
                    }/* else {*/
                        /* For V1, to avoid breaking existing users we have
                         * decided to preserve old behavior in which we
                         * keep the incoming value of the discriminator 
                         * untouched.
                         */
                    /*}*/
                }
            }

            if (program->unionDiscKind == RTI_XCDR_TK_ENUM
                    && program->extKind == RTI_XCDR_EXTENSIBLE_EXTENSIBILITY) {
                if (!RTIXCdrInterpreter_validateUnionEnumDisc(
                        &memberValue,
                        sample,
                        program,
                        instruction,
                        sampleAssignability,
                        memberSampleAccessInfo,
                        context)) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
            }

            processUnionDisc = RTI_XCDR_FALSE;
            continue;
        }

        if (processUnionDisc
                && program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
            if (program->unionDiscKind == RTI_XCDR_TK_ENUM) {
                if (!RTIXCdrInterpreter_validateUnionEnumDisc(
                        &memberValue,
                        sample,
                        program,
                        instruction,
                        sampleAssignability,
                        memberSampleAccessInfo,
                        context)) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
            }
        }

        /* 
         * If the instruction is RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE
         * there is still one more to go before moving to next member 
         */
        if (optionalMemberIndex == i
            || (program->instructionIndex != NULL
                && deserializeNextMutableHeader)) {
            if (basePosition == NULL) {
                RTIXCdrStream_moveToNextParameterHeader(
                        stream,
                        &memberStreamState,
                        memberLength);
            }
            optionalMemberIndex = RTIXCdrUnsignedLong_MAX;
        }

        i++;
    }
    
    result = RTI_XCDR_TRUE;
  done:    
    if (!result) {
        if (context->expectedSpaceError) {
            if (RTIXCdrInterpreter_isUnexpectedSpaceError(
                    program,
                    stream)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
            } else {
                result = RTI_XCDR_TRUE;
            }
        }

        if (context->logAllErrorsButExpectedSpaceErrors 
                && !context->expectedSpaceError
                && instruction != NULL) {
            if (isSkipError) {
                RTIXCdrInterpreter_logSkipError(
                        tc,
                        instruction,
                        logMessageId,
                        &runTimeLogParam,
                        RTI_XCDR_FUNCTION_NAME,
                        logLineNumber);
            } else {
                RTIXCdrInterpreter_logDeserializationError(
                        tc,
                        stream,
                        instruction,
                        logMessageId,
                        &runTimeLogParam,
                        RTI_XCDR_FUNCTION_NAME,
                        logLineNumber);
            }
        }
    }

    /* 
     * This has to be executed after RTIXCdrInterpreter_logDeserializationError
     * so that the stream position used to report a potential error is not 
     * changed
     */
    if (dheaderPosition != NULL) {
        RTIXCdrStream_popState(stream, &streamState);
        RTIXCdrStream_setCurrentPosition(stream, dheaderPosition);
        RTIXCdrStream_increaseCurrentPosition(stream, dheaderSize);
    }

    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_fastDeserializeSample(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context)
{
    /* Uninitialized variables */
    RTIXCdrUnsignedLong instructionCount;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrUnsignedLong dheaderSize;
    struct RTIXCdrStreamState streamState;
    RTIXCdrUnsignedLong dheaderCollectionSize;
    struct RTIXCdrStreamState streamCollectionState;
    RTIXCdrMemberValue memberValue;

    /* Initialized variables */
    RTIXCdrCommonInsParameters *commonParams = NULL;
    RTIXCdrInsParameters *params = NULL;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrBoolean isSkipError = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    char *dheaderPosition = NULL;
    char *dheaderCollectionPosition = NULL;
    RTIXCdrBoolean corruptedHeader = RTI_XCDR_TRUE;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    instructionCount = program->instructionCount;
    context->expectedSpaceError = RTI_XCDR_TRUE;

    for (; i<instructionCount; i++) {
        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        /* The order of the case statement is important for performance reasons
         * We always favor primitives
         *
         * Also, in this function, deserialization instructions should be
         * before than skip
         */
        switch (instruction->opcode) {
        case RTI_XCDR_DESER_PRIMITIVE_OPCODE:
        {
            char *valuePtr = NULL;

            if (params->primitiveParams.mustAlign)
            {
                RTIXCdrStream_alignMacro(
                        stream,
                        params->primitiveParams.primitiveAlignment,
                        GotoDoneWithLine());
            }

            RTIXCdrInterpreter_getPrimitiveMemberValuePtr(
                    memberValue,
                    sample,
                    commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX]);

            if (params->primitiveParams.checkRange) {
                valuePtr = memberValue.value.ptr;
            }

            RTIXCdrInterpreter_deserializePrimitiveArray(
                    stream,
                    memberValue.value.ptr,
                    params->primitiveParams.primitiveSize,
                    commonParams->count,
                    params->primitiveParams.primitiveByteCount,
                    RTI_XCDR_FALSE,
                    context->annotations,
                    GotoDoneWithLine());

            if (params->primitiveParams.checkRange) {
                if (!RTIXCdrInterpreter_checkPrimitiveRange(
                        valuePtr,
                        params->primitiveParams.primitiveKind,
                        program,
                        commonParams->tcMemberInfo,
                        context->annotations,
                        RTI_XCDR_FALSE,
                        RTI_XCDR_FALSE)) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrLong primitiveAlignment;
            RTIXCdrUnsignedLong seqElementCount;
            RTIXCdrOctet primitiveSize;
            RTIXCdrUnsignedLong arrayCount;
            RTIXCdrUnsignedLong j;
            struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo;
            RTIXCdrUnsignedLongLong elOffset;
            RTIXCdrBoolean failure;
            char *samplePtr = (char *)sample;
            RTIXCdrUnsignedLong primitiveSeqElementCount = 0;
            RTIXCdrUnsignedLong primitiveByteCount = 0;

            arrayCount = commonParams->count;
            primitiveAlignment =
                    params->primitiveParams.primitiveAlignment;
            elOffset =
                    commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
            memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;

            /* 
             * memberSampleAccessInfo should not be NULL because all language
             * bindings have sample access info for sequences
             */
            RTIXCdrLog_testPrecondition(
                    memberSampleAccessInfo == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine());

            primitiveSize = params->primitiveParams.primitiveSize;

            for (j=0; j<arrayCount; j++) {
                if (!RTIXCdrStream_deserialize4Byte(
                        stream,
                        &seqElementCount,
                        RTI_XCDR_TRUE)) {
                    GotoDoneWithLine();
                }

                if (!RTIXCdrInterpreter_checkAndAdjustSequenceElementCount(
                        &primitiveSeqElementCount,
                        &primitiveByteCount,
                        &seqElementCount,
                        &logMessageId,
                        stream,
                        instruction)) {
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (seqElementCount > 0 &&
                        primitiveAlignment > RTI_XCDR_FOUR_BYTE_ALIGNMENT) {
                    /* We don't have to align the first element of the sequence
                     * if the alignment is greater than the length alignment
                     */
                    RTIXCdrStream_alignMacro(
                            stream,
                            primitiveAlignment,
                            GotoDoneWithLine());
                }

                RTIXCdrInterpreter_setMemberElementCount(
                        &failure,
                        memberValue,
                        seqElementCount,
                        samplePtr,
                        elOffset,
                        commonParams->memberTc,
                        commonParams->tcMemberInfo,
                        commonParams->refMemberKind,
                        memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX],
                        commonParams->memberTc->_maximumLength >= program->unboundedSize,
                        RTI_XCDR_FALSE,
                        context->programData);

                if (failure) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }

                if (seqElementCount == 0) {
                    elOffset +=  memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                    continue;
                }

                if (memberValue.isDiscontiguous) {
                    RTIXCdrUnsignedLong desCount;
                    RTIXCdrUnsignedLong h;
                    char *valuePtr;

                    if (params->primitiveParams.origPrimitiveSize
                                != primitiveSize
                        && primitiveSize == 1) {
                        desCount = params->primitiveParams.origPrimitiveSize;
                    } else {
                        desCount = 1;
                    }

                    primitiveByteCount = primitiveSize * desCount;

                    for (h = 0; h < seqElementCount; h++) {
                        valuePtr = *(((char **) (void *) memberValue.value.ptr) + h);

                        if (valuePtr == NULL) {
                            GotoDoneWithLine();
                        }

                        RTIXCdrInterpreter_deserializePrimitiveArray(
                                stream,
                                valuePtr,
                                primitiveSize,
                                desCount,
                                primitiveByteCount,
                                RTI_XCDR_FALSE,
                                context->annotations,
                                GotoDoneWithLine());
                    }
                } else {
                    RTIXCdrInterpreter_deserializePrimitiveArray(
                            stream,
                            memberValue.value.ptr,
                            primitiveSize,
                            primitiveSeqElementCount,
                            primitiveByteCount,
                            RTI_XCDR_FALSE,
                            context->annotations,
                            GotoDoneWithLine());
                }

                elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (!RTIXCdrStream_skipNByte(
                    stream,
                    params->primitiveParams.primitiveAlignment,
                    commonParams->count)) {
                isSkipError = RTI_XCDR_TRUE;
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_skipPrimitiveSeq(
                    tc,
                    stream,
                    instruction,
                    context)) {
                isSkipError = RTI_XCDR_TRUE;
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_DESER_DHEADER_OPCODE:
        {
            if (!context->inBaseClass) {
                /* 
                 * Even though this call is within the for loop, it can be 
                 * executed only once.
                 */
                if (!RTIXCdrStream_deserializeDHeader(
                        stream,
                        &corruptedHeader,
                        &dheaderSize,
                        &dheaderPosition,
                        &streamState)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE:
        {
            if (!RTIXCdrStream_deserializeDHeader(
                    stream,
                    &corruptedHeader,
                    &dheaderCollectionSize,
                    &dheaderCollectionPosition,
                    &streamCollectionState)) {
                if (corruptedHeader) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                }
                GotoDoneWithLine();
            }
        } break;
        /* 
         * No need to process RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE because
         * types containing sequence of enums that must be deserialized
         * with a dheader are not candidates for fast deserialization.
         */
        default:
        {
            GotoDoneWithLine();
        } break;
        }

        if (dheaderCollectionPosition != NULL
                && instruction->opcode != RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE) {
            RTIXCdrStream_popState(stream, &streamCollectionState);
            RTIXCdrStream_setCurrentPosition(stream, dheaderCollectionPosition);
            RTIXCdrStream_increaseCurrentPosition(
                    stream,
                    dheaderCollectionSize);
            dheaderCollectionPosition = NULL;
        }
    }

    result = RTI_XCDR_TRUE;
  done:

    if (!result) {
        if (context->expectedSpaceError) {
            if (RTIXCdrInterpreter_isUnexpectedSpaceError(
                    program,
                    stream)) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
            } else {
                result = RTI_XCDR_TRUE;
            }
        }

        if (context->logAllErrorsButExpectedSpaceErrors 
                && !context->expectedSpaceError
                && instruction != NULL) {
            if (isSkipError) {
                RTIXCdrInterpreter_logSkipError(
                        tc,
                        instruction,
                        logMessageId,
                        &runTimeLogParam,
                        RTI_XCDR_FUNCTION_NAME,
                        logLineNumber);
            } else {
                RTIXCdrInterpreter_logDeserializationError(
                        tc,
                        stream,
                        instruction,
                        logMessageId,
                        &runTimeLogParam,
                        RTI_XCDR_FUNCTION_NAME,
                        logLineNumber);
            }
        }
    }

    /* 
     * This has to be executed after RTIXCdrInterpreter_logDeserializationError
     * so that the stream position used to report a potential error is not 
     * changed
     */
    if (dheaderPosition != NULL) {
        RTIXCdrStream_popState(stream, &streamState);
        RTIXCdrStream_setCurrentPosition(stream, dheaderPosition);
        RTIXCdrStream_increaseCurrentPosition(stream, dheaderSize);
    }

    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_deserializeSampleWithEncapsulation(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        struct RTIXCdrStream *stream,
        const struct RTIXCdrSampleAssignabilityProperty * ap)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    struct RTIXCdrTypePluginProgramContext context = 
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    RTIXCdrEncapsulationId encapsulationId;
    RTIXCdrBoolean isCdrV2;
    RTIXCdrBoolean isLittleEndian;
    
    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE); 
    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE); 
    
    RTIXCdrStream_deserializeAndSetCdrEncapsulationMacro(
            stream,
            goto done);

    encapsulationId = RTIXCdrStream_getEncapsulationId(stream);
    isCdrV2 = RTIXCdrEncapsulationId_isCdrV2(encapsulationId);
    isLittleEndian = RTIXCdrEncapsulationId_isLittleEndianCdr(encapsulationId);
    context.onlyKey = RTI_XCDR_FALSE;
    context.program = RTIXCdrInterpreterPrograms_getDeserProgram(
            programs,
            isLittleEndian?RTI_TRUE:RTI_FALSE,
            isCdrV2?RTI_TRUE:RTI_FALSE,
            context.onlyKey);
    if (context.program == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GET_FAILURE_ID_s,
                "deserialize program");
        return RTI_XCDR_FALSE;
    }

    context.typeCode = context.program->typeCode;
    context.inBaseClass = RTI_XCDR_FALSE;

    if (context.typeCode->_typePlugin != NULL) {
        if (RTIXCdrInterpreter_isInitializationNeededOnDeserialization(
                    context.program)) {
            if (context.typeCode->_typePlugin->initializeSampleFnc != NULL) {
                if (!context.typeCode->_typePlugin
                             ->initializeSampleFnc(sample, 0, 0)) {
                    goto done;
                }
            } else if (context.typeCode->_typePlugin->initializeSampleWParamsFnc
                    != NULL) {
                if (!context.typeCode->_typePlugin->initializeSampleWParamsFnc(
                            sample,
                            context.typeCode,
                            NULL,
                            context.programData,
                            context.typeCode->_typePlugin->typePluginParam)) {
                    goto done;
                }
            }
        }
    }

    if (!RTIXCdrInterpreter_deserializeSample(
            sample,
            stream,
            context.typeCode,
            context.program,
            ap,
            &context)) {
        goto done;
    }

    result = RTI_XCDR_TRUE;
  done: 

    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_deserializeFromCdrBuffer(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        const char *buffer,
        RTIXCdrUnsignedLong length)
{
    struct RTIXCdrStream stream;
    struct RTIXCdrSampleAssignabilityProperty ap;

    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(buffer == NULL, return RTI_XCDR_FALSE);

    RTIXCdrSampleAssignabilityProperty_setFromGlobalComplianceMask(&ap);

    RTIXCdrStream_init(&stream);
    RTIXCdrStream_set(&stream, (char *) buffer, length);

    if (!RTIXCdrInterpreter_deserializeSampleWithEncapsulation(
            programs,
            sample,
            &stream,
            &ap)) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}


/* CORE-9187: This type is defined to address warning
   "destination of memory copy is too small when deserializing a discriminator
   value.
*/
typedef union RTIXCdrInterpreterDiscriminatorValue {
    RTIXCdrLongLong ll;
    RTIXCdrLongDouble ld;
} RTIXCdrInterpreterDiscriminatorValue;

RTIXCdrBoolean RTIXCdrInterpreter_skipSample(
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrLogMessageId logMessageId = RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong instructionCount;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrBoolean processUnionDisc;
    RTIXCdrUnsignedLong optionalMemberIndex = RTIXCdrUnsignedLong_MAX;
    struct RTIXCdrStreamState memberStreamState;
    RTIXCdrUnsignedLong memberLength;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            stream == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    processUnionDisc = program->unionDiscKind != RTI_XCDR_TK_NULL;
    instructionCount = program->instructionCount;
    context->expectedSpaceError = RTI_XCDR_TRUE;

    if (program->instructionIndex != NULL) {
        RTIXCdrBoolean isSentinel = RTI_XCDR_FALSE;

        while (!isSentinel) {
            if (!RTIXCdrStream_deserializeAndSkipV1ParameterHeader(
                    stream,
                    &isSentinel)) {
                GotoDoneWithLine();
            }
        }

        result = RTI_TRUE;
        goto done;
    }

    for (i=0; i<instructionCount;) {
        RTIXCdrCommonInsParameters *commonParams;
        RTIXCdrInsParameters *params;

        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        /* 
         * The order of the case statement is important for performance reasons
         * We always favor primitives
         */
        switch (instruction->opcode) {
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (processUnionDisc) {
                if (params->primitiveParams.mustAlign) {
                    RTIXCdrStream_alignMacro(
                            stream,
                            params->primitiveParams.primitiveAlignment,
                            GotoDoneWithLine());
                }
            } else {
                if (!RTIXCdrStream_skipNByte(
                        stream,
                        params->primitiveParams.primitiveAlignment,
                        commonParams->count)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_skipPrimitiveSeq(
                    tc,
                    stream,
                    instruction,
                    context)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_STRING_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_OPCODE:
        case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_skipString(
                    tc,
                    stream,
                    instruction,
                    context)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        {
            RTIXCdrUnsignedLong j;
            RTIXCdrTypePluginSkipFunction skipComplexFcn = NULL;
            char *discPosition = NULL;
            RTIXCdrBoolean tmpInBaseClass;
            
            if (params->complexParams.typePlugin != NULL) {
                skipComplexFcn = params->complexParams.typePlugin->skipFnc;
            }
            
            tmpInBaseClass = context->inBaseClass;

            for (j=0; j<commonParams->count; j++) {
                if (processUnionDisc) {
                    RTIXCdrStream_alignMacro(
                            stream,
                            RTIXCdr_TCKind_g_primitiveCdrAlignments[program->isCdr2][program->unionDiscKind],
                            GotoDoneWithLine());

                    discPosition = RTIXCdrStream_getCurrentPosition(stream);
                }
                
                /* We have to assign this here because the variables
                 * can be overwritten by the nested calls
                 */
                context->program = params->complexParams.program;
                context->typeCode = commonParams->memberTc;
                context->inBaseClass = params->complexParams.baseClass;

                if (skipComplexFcn == NULL) {
                    if (!RTIXCdrInterpreter_skipSample(
                            stream,
                            context->typeCode,
                            context->program,
                            context)) {
                            GotoDoneWithLine();
                    }
                } else {
                    if (!skipComplexFcn(
                            context->endpointPluginData,
                            stream,
                            0,
                            1, 
                            context->endpointPluginQos)) {
                        GotoDoneWithLine();
                    }
                }
            }

            context->inBaseClass = tmpInBaseClass;

            if (processUnionDisc) {
                /* 
                 * Restore the position so that we can deserialize the disc
                 * value again below in order to look up the instruction index
                 */
                RTIXCdrStream_setCurrentPosition(stream, discPosition);
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_skipComplexSeq(
                    tc,
                    stream,
                    instruction,
                    context)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_DESER_DHEADER_OPCODE: 
        {
            if (!context->inBaseClass) {
                RTIXCdrBoolean corruptedHeader = RTI_XCDR_TRUE;

                /* 
                 * Even though this call is within the for loop, it can be 
                 * executed only once.
                 */
                if (!RTIXCdrStream_skipDHeader(
                        stream,
                        &corruptedHeader)) {
                    if (corruptedHeader) {
                        context->expectedSpaceError = RTI_XCDR_FALSE;
                    }
                    GotoDoneWithLine();
                }
                i = instructionCount;
            }
        } break;
        case RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE: 
        {
            RTIXCdrBoolean corruptedHeader = RTI_XCDR_TRUE;

            if (!RTIXCdrStream_skipDHeader(
                    stream,
                    &corruptedHeader)) {
                if (corruptedHeader) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                }
                GotoDoneWithLine();
            }

            /*
             * Increase instruction count by one to skip the skip instruction
             * following dheader.
             */
            i++;
        } break;
        case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        {            
            if (program->isCdr2) {
                RTIXCdrBoolean memberValueSet;

                if (!RTIXCdrStream_deserialize1Byte(
                        stream,
                        &memberValueSet)) {
                    GotoDoneWithLine();
                }

                if (!memberValueSet) {
                    i = RTIXCdrInterpreter_getMemberDataInsIndex(program, i);
                }
            } else if (
                    instruction->params.memberHeaderParams.hasV1NestedMemberHeaders) {
                RTIXCdrUnsignedLong parameterId;
                RTIXCdrBoolean extended;

                /* 
                 * CORE-13829: We cannot skip to the next value of this optional 
                 * member because its value contains other headers and 
                 * this would cause the stream to be out of sync as we would 
                 * miss alignment reset operations.
                 */
                if (!RTIXCdrStream_deserializeV1ParameterHeader(
                        stream,
                        &memberStreamState,
                        &parameterId,
                        &memberLength,
                        &extended,
                        NULL)) {
                    GotoDoneWithLine();
                }

                if (memberLength == 0) {
                    RTIXCdrStream_moveToNextParameterHeader(
                            stream,
                            &memberStreamState,
                            memberLength);
                    i++;
                } else {
                    optionalMemberIndex = i+1;
                }
            } else {
                if (!RTIXCdrStream_deserializeAndSkipV1ParameterHeader(
                        stream,
                        NULL)) {
                    GotoDoneWithLine();
                }
                i++;
            }
        } break;
        default:
            goto done;
        }

        if (RTIXCdrInterpreter_isDiscInstIndex(i)) {
            RTIXCdrLong caseValue;
            RTIXCdrInterpreterDiscriminatorValue val = { 0 };
            RTIXCdrMemberValue memberValue = RTIXCdrMemberValue_INITIALIZER;
            RTIXCdrBoolean failure;
            struct RTIXCdrSampleAssignabilityProperty *sampleAssignability 
                = NULL;

            memberValue.value.ptr = (char *)&val;
            /* CORE-9187: Although the discriminator cannot have type long
             * double, the following macro contains long double as part of
             * its switch statement. This is why val is declared as
             * RTIXCdrInterpreterDiscriminatorValue. If it was a "long long"
             * we will see the following compilation warning:
             *
             * warning C4789: destination of memory copy is too small
             */
            RTIXCdrInterpreter_deserializePrimitiveArray(
                stream,
                memberValue.value.ptr,
                RTIXCdr_TCKind_g_primitiveCdrSizes[program->isCdr2][program->unionDiscKind],
                1,
                RTIXCdr_TCKind_g_primitiveCdrSizes[program->isCdr2][program->unionDiscKind],
                RTI_XCDR_TRUE,
                context->annotations,
                GotoDoneWithLine());

            /* 
             * Process union discriminator and select instruction
             * based on discriminator value
             */
            RTIXCdrInterpreter_primitiveToLongMacro(
                    caseValue,
                    memberValue,
                    program->unionDiscKind,
                    failure);
            if (failure) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            RTIXCdrProgram_getUnionMemberInstructionIndex(
                    program,
                    &i,
                    &instructionCount,
                    (RTIBool *) NULL, /* caseValueIdentifiesMember */
                    caseValue);

            processUnionDisc = RTI_XCDR_FALSE;
            continue;
        } 

        if (optionalMemberIndex == i
                && optionalMemberIndex != RTIXCdrUnsignedLong_MAX) {
            RTIXCdrStream_moveToNextParameterHeader(
                    stream,
                    &memberStreamState,
                    memberLength);
            optionalMemberIndex = RTIXCdrUnsignedLong_MAX;
        }

        i++;
    }
    
    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (context->expectedSpaceError &&
                program->extKind != RTI_XCDR_FINAL_EXTENSIBILITY) {
            result = RTI_XCDR_TRUE;
        }
    }

    if (!result) {
        if (context->logAllErrorsButExpectedSpaceErrors 
                && !context->expectedSpaceError
                && instruction != NULL) {
            RTIXCdrInterpreter_logSkipError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return result;
}

/*****************************************************************************/
/***** GetSerSampleSize ******************************************************/
/*****************************************************************************/

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreter_skipStringFromSample(
        RTIXCdrStream *stream,
        const struct RTIXCdrProgram *program,
        const struct RTIXCdrInstruction *instruction,
        void *sample,
        const RTIXCdrTypeCode *tc,
        void *programData)
{
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i,j;
    RTIXCdrUnsignedLong stringCount;
    RTIXCdrUnsignedLong elementCount;
    RTIXCdrMemberValue memberValue;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrLogParam runTimeLogParam;
    const struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo;
    const struct RTIXCdrSampleAccessInfo *strSampleAccessInfo;
    RTIXCdrUnsignedLongLong elOffset, strOffset, externalStrOffset;
    struct RTIXCdrTypeCode *strTc;
    struct RTIXCdrTypeCodeMember *tcMember;
    RTIXCdrBoolean isSeq = RTI_XCDR_FALSE;
    char *strSample = (char *)sample;
    char *externalStrSample = (char *)sample;
    RTIXCdrCommonInsParameters *commonParams;
    const RTIXCdrInsParameters *params;
    RTIXCdrLong charAlignment;
    RTIXCdrOctet strRefMemberKind;
    RTIXCdrUnsignedLong strTypeSize;
    RTIBool isSeqDiscontiguous = RTI_FALSE;

    RTIXCdrLog_testPrecondition(stream == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(instruction == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    params = &instruction->params;
    commonParams = RTIXCdrInstruction_getCommonParams(
            instruction);
    RTIXCdrLog_testPrecondition(commonParams == NULL, return RTI_XCDR_FALSE);

    memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
    elementCount = commonParams->count;

    elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
    strOffset = elOffset;
    externalStrOffset = elOffset;

    if (instruction->opcode == RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE ||
            instruction->opcode == RTI_XCDR_SKIP_STRING_SEQ_OPCODE) {
        strSampleAccessInfo = commonParams->seqElementTc->_sampleAccessInfo;
        strTc = commonParams->seqElementTc;
        isSeq = RTI_XCDR_TRUE;
        strRefMemberKind = RTI_XCDR_INTERPRETER_VALUE_MEMBER;
        stringCount = 0;
    } else {
        strSampleAccessInfo = memberSampleAccessInfo;
        strTc = commonParams->memberTc;
        stringCount = 1;
        strRefMemberKind = commonParams->refMemberKind;

        if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
            strOffset = 0;
        }
    }

    strTypeSize = RTIXCdrInterpreter_getStrTypeSize(strSampleAccessInfo);

    tcMember = commonParams->tcMemberInfo;

    charAlignment = params->strParams.charAlignment;

    for (i=0; i<elementCount; i++) {
        if (isSeq) {
            char *seqSample;

            if (commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                seqSample  = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        ((char *)sample + elOffset));
                if (seqSample == NULL) {
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            } else {
                seqSample = (char *)sample;
            }
        
        
            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    &stringCount,
                    seqSample,
                    commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                            0: elOffset,
                    0,
                    commonParams->memberTc,
                    tcMember,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    programData);

            if (memberValue.value.ptr == NULL && stringCount > 0) {
                GotoDoneWithLine();
            }

            if (stringCount > commonParams->memberTc->_maximumLength) {
                logMessageId = RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                runTimeLogParam.value.ulVal = stringCount;
                GotoDoneWithLine();
            }

            if (commonParams->addSeqDHeader) {
                /*
                 * If we are processing an array of sequences of strings
                 * we have to skip the DHEADER for each sequence here.
                 */
                if (!RTIXCdrStream_skipNByte(
                        stream,
                        RTI_XCDR_DHEADER_ALIGNMENT,
                        RTI_XCDR_DHEADER_SIZE)) {
                    GotoDoneWithLine();
                }
            }

            if (!RTIXCdrStream_skipNByte(
                    stream,
                    RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                    RTI_XCDR_FOUR_BYTE_SIZE)) {
                GotoDoneWithLine();
            }

            if (commonParams->refMemberKind !=
                    RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
            } else {
                elOffset += program->externalReferenceSize;
            }

            if (stringCount == 0) {
                continue;
            }

            strSample = memberValue.value.ptr;
            strOffset = 0;
            isSeqDiscontiguous = memberValue.isDiscontiguous;
        }

        for (j=0; j<stringCount; j++) {
            RTIXCdrUnsignedLong byteCount;
            RTIXCdrUnsignedLong charCount = 0;
            char *strSampleVal;
            
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                strSample = RTIXCdrProgram_getExternalRefValuePtr(
                        program,
                        (externalStrSample + externalStrOffset));
                if (strSample == NULL) {
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            }

            if (isSeqDiscontiguous) {
                /* 
                 * The element in the sequence is a pointer to a string
                 * (e.g, pointer to char* or DDS_Wchar* in C)
                 */
                strSampleVal = *RTIXCdrUtility_staticCast(char **, strSample);

                if (strSampleVal == NULL) {
                    logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                    GotoDoneWithLine();
                }
            } else {
                strSampleVal = strSample;
            }

            RTIXCdrInterpreter_getStrValuePtr(
                    memberValue,
                    charAlignment,
                    &charCount,
                    strSampleVal,
                    strOffset,
                    0,
                    strTc,
                    /* We do not provide tcMember because otherwise the
                     * accesor functions may thing that the string is optional
                     */
                    isSeq?NULL:tcMember,
                    programData);

            if (charCount >= params->strParams.charMaxCount) {
                RTIXCdrBoolean outOfBounds = RTI_XCDR_FALSE;

                /**
                 * Coverity is complaining about charCount being
                 * 0, but this cannot happen because charMaxCount
                 * is always greater than 0.
                 */
                RTIXCdrLog_testPrecondition(charCount == 0, GotoDoneWithLine());

                if (charCount > params->strParams.charMaxCount) {
                    outOfBounds = RTI_XCDR_TRUE;
                } else if (charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                    RTIXCdrLog_testPrecondition(
                            memberValue.value.ptr == NULL,
                            GotoDoneWithLine());
                    /**
                     * Coverity is complaining about charCount being
                     * 0, but this cannot happen because charMaxCount
                     * is always greater than 0. The previous precondition
                     * is not working because this is being detected by
                     * the secure plan (Release).
                     */
                    /* coverity[overflow_const : FALSE] */
                    if (memberValue.value.ptr[charCount - 1] != '\0') {
                        outOfBounds = RTI_XCDR_TRUE;
                    }
                } else {
                    if (!RTIXCdrInterpreter_useMemberElementIndex(
                            strSampleAccessInfo)) {
                        RTIXCdrLog_testPrecondition(
                                memberValue.value.ptr == NULL,
                                GotoDoneWithLine());
                        if (((const RTIXCdrWchar *)
                                    (void *) memberValue.value.ptr)[charCount - 1]
                            != 0) {
                            outOfBounds = RTI_XCDR_TRUE;
                        }
                    } else {
                        memberValue =
                                strSampleAccessInfo->getMemberValuePointerFcn(
                                        strSample,
                                        NULL,
                                        strOffset,
                                        charCount - 1,
                                        strTc,
                                        tcMember,
                                        RTI_XCDR_FALSE,
                                        programData);
                        if (memberValue.value.wVal != 0) {
                            outOfBounds = RTI_XCDR_TRUE;
                        }
                    }
                }

                if (outOfBounds) {
                    logMessageId =
                            RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = charCount;
                    GotoDoneWithLine();
                }
            }

            byteCount = RTI_XCDR_FOUR_BYTE_SIZE;

            if (charAlignment == RTI_XCDR_ONE_BYTE_ALIGNMENT) {
                byteCount += (RTIXCdrUnsignedLong)charCount;
            } else if (charAlignment == RTI_XCDR_LEGACY_WCHAR_ALIGNMENT) {
                byteCount += charCount << 2;
            } else {
                byteCount += (charCount-1) << 1;
            }

            if (!RTIXCdrStream_skipNByte(
                    stream,
                    RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                    byteCount)) {
                GotoDoneWithLine();
            }

            /* Goto next string */
            if (strRefMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                externalStrOffset += program->externalReferenceSize;
            } else if (isSeqDiscontiguous) {
                /* 
                 * We are dealing with a discontiguous sequence of strings
                 * and we need to move the pointer to the next element of
                 * the sequence which is a pointer.
                 */
                strSample += sizeof(void *);
            } else {
                strOffset += strTypeSize;
            }
        }
    }

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok) {
        RTIXCdrInterpreter_logSkipError(
                tc,
                instruction,
                logMessageId,
                &runTimeLogParam,
                RTI_XCDR_FUNCTION_NAME,
                logLineNumber);
    }

    return ok;
}

/* 
 * Set the buffer for the stream used by interpreter functions that get sizes.
 * 
 * In these functions the RTIXCdrStream pointers are never dereferenced. 
 * The current position is used as an offset to compute the size of the
 * serialized sample by subtracting it from the buffer pointer which is also
 * treated as an offset.
 *
 * To avoid sanitizer errors such as this:
 *
 * runtime error: applying zero offset to null pointer
 *
 * we set the buffer to a non-null value (8).
 */
#define RTIXCdrInterpreter_setBufferForGetSize(me__) \
    RTIXCdrStream_set( \
            (me__), \
            RTIXCdrUtility_intToPointer(8), \
            RTIXCdrUnsignedLong_MAX)

RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleSize(
        RTIXCdrUnsignedLong *size,
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0;
    char *startPosition = NULL, * endPosition = NULL;
    RTIXCdrUnsignedLongLong longSize = 0;
    RTIXCdrStream xcdrStream;
    RTIXCdrStream *xcdrStreamPtr;
    struct RTIXCdrInstruction *instruction = NULL;
    void *programData;
    RTIXCdrLogParam runTimeLogParam;
    RTIXCdrLogMessageId logMessageId =
            RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrUnsignedLong instructionCount;
    RTIXCdrBoolean extended = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
    struct RTIXCdrStreamState memberStreamState;
    RTIXCdrBoolean flatData;
    RTIXCdrUnsignedLong offsetIndex = 0;
    RTIXCdrBoolean getUnionInstIndex;
    RTIXCdrBoolean primitiveDisc = RTI_XCDR_FALSE;
    RTIXCdrMemberValue memberValue = RTIXCdrMemberValue_INITIALIZER;

    /* GREEN-898 We need to do this assignment to a local variable in order to
     * prevent static analysis issues regarding the use of xcdrStream in
     * the context structure
     */
    struct RTIXCdrTypePluginProgramContext *localContext = context;
    const struct RTIXCdrSampleAccessInfo *memberSampleAccessInfo = NULL;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            size == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            sample == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    programData = context->programData;
    xcdrStreamPtr = localContext->xcdrStream;

    if (xcdrStreamPtr == NULL) {
        RTIXCdrStream_initialize(&xcdrStream);
        xcdrStreamPtr = &xcdrStream;
        localContext->xcdrStream = xcdrStreamPtr;
        RTIXCdrInterpreter_setBufferForGetSize(xcdrStreamPtr);
    }

    startPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
    getUnionInstIndex = program->unionDiscKind != RTI_XCDR_TK_NULL;
    instructionCount = program->instructionCount;
    flatData = program->isFlatDataProgram;

    if (flatData) {
        offsetIndex =
                (RTIXCdrUnsignedLong) (RTIXCdrUtility_pointerToUnsignedLongLong(
                                               sample)
                                       % RTI_XCDR_MAX_XCDR2_ALIGNMENT);
    }

    for (i=0; i<instructionCount; ) {
        RTIXCdrUnsignedLong byteCount;
        RTIXCdrUnsignedLong j;
        RTIXCdrUnsignedLong h;        
        RTIXCdrUnsignedLong seqElementCount = 0;
        RTIXCdrCommonInsParameters *commonParams;
        RTIXCdrInsParameters *params;
        struct RTIXCdrTypeCodeMember *tcMember;

        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);
        if (commonParams->memberTc != NULL) {
            memberSampleAccessInfo = commonParams->memberTc->_sampleAccessInfo;
        }
        /*
         * memberTc can only be NULL if isHeaderOpcode is true, because header
         * operations are the only ones that do not require access to/knowledge
         * of the member's typecode that is being operated on
         */
        RTIXCdrLog_testPrecondition(
                !RTIXCdrInstruction_isHeaderOpcode(instruction->opcode)
                        && commonParams->memberTc == NULL,
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine());

        tcMember = commonParams->tcMemberInfo;

        switch (instruction->opcode) {
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (getUnionInstIndex) {
                primitiveDisc = RTI_XCDR_TRUE;

                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        NULL,
                        sample,
                        commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                        0,
                        commonParams->memberTc,
                        tcMember,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);
            } else if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    params->primitiveParams.primitiveAlignment,
                    commonParams->count)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong elementCount;
            RTIXCdrUnsignedLongLong elOffset;
            char *samplePtr = (char *)sample;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());

            elementCount = commonParams->count;
            elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];

            for (j=0; j<elementCount; j++) {
                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        0,
                        commonParams->memberTc,
                        tcMember,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);
                /*
                 * For sequences, it is fine to get a null memberValue.value.ptr
                 * from the getMemberValuePtr call. This will be the case of
                 * zero-length sequences.
                 */

                if (seqElementCount > commonParams->memberTc->_maximumLength) {
                    logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    GotoDoneWithLine();
                }

                if (commonParams->addSeqDHeader) {
                    /*
                     * If we are processing an array of sequences
                     * we have to skip the DHEADER for each sequence here.
                     */
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_DHEADER_ALIGNMENT,
                            RTI_XCDR_DHEADER_SIZE)) {
                        GotoDoneWithLine();
                    }
                }

                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    GotoDoneWithLine();
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (seqElementCount == 0) {
                    continue;
                }

                RTIXCdrInterpreter_adjustSequenceElementCount(
                        seqElementCount, 
                        byteCount,
                        params);

                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        params->primitiveParams.primitiveAlignment,
                        byteCount)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_STRING_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_OPCODE:
        case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        {
            if (!RTIXCdrInterpreter_skipStringFromSample(
                    xcdrStreamPtr,
                    program,
                    instruction,
                    sample,
                    tc,
                    programData)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginGetSerializedSampleSizeFunction getSerSampleSizeFcn = NULL;
            char *samplePtr;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SKIP_COMPLEX_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());
            
            if (params->complexParams.typePlugin != NULL) {
                getSerSampleSizeFcn = params->complexParams.typePlugin->getSerSampleSizeFnc;
            }

            RTIXCdrInterpreter_getMemberValuePtr(
                    memberValue,
                    NULL,
                    sample,
                    commonParams->memberAccessInfo.bindingMemberValueOffset[offsetIndex],
                    0,
                    commonParams->memberTc,
                    tcMember,
                    commonParams->refMemberKind,
                    commonParams->useGetMemberValue,
                    0,
                    programData);
            if (memberValue.value.ptr == NULL) {
                context->expectedSpaceError = RTI_XCDR_FALSE;
                GotoDoneWithLine();
            }

            tmpInBaseClass = context->inBaseClass;

            for (j=0; j<commonParams->count; j++) {
                if (j != 0) {
                    RTIXCdrInterpreter_nextArrayElementPtr(
                            memberValue.value.ptr,
                            flatData,
                            memberSampleAccessInfo->typeSize,
                            &params->complexParams,
                            commonParams->refMemberKind,
                            program->externalReferenceSize);
                }

                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr = memberValue.value.ptr;
                } else {
                    samplePtr = RTIXCdrProgram_getExternalRefValuePtr(
                            program, memberValue.value.ptr);
                    if (samplePtr == NULL) {
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }

                /* 
                 * We have to assign this here because the variables
                 * can be overwritten by the nested calls
                 */
                context->program = params->complexParams.program;
                context->typeCode = commonParams->memberTc;
                context->inBaseClass = params->complexParams.baseClass;

                if (getSerSampleSizeFcn == NULL) {
                    /* Initialized to zero to avoid Purify error described in 
                     * CORE-9936
                     */
                    RTIXCdrUnsignedLong tmpSize = 0;
                    
                     if (!RTIXCdrInterpreter_getSerSampleSize(
                            &tmpSize,
                            samplePtr,
                            context->typeCode,
                            context->program,
                            context)) {
                         GotoDoneWithLine();                         
                     }
                } else {
                    RTIXCdrUtility_unusedReturnValue(
                            getSerSampleSizeFcn(
                                    context->endpointPluginData,
                                    0,
                                    context->encapsulationId,
                                    *size, /* Ignored */
                                    samplePtr), RTIXCdrUnsignedLong);
                }
                
                endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
                /* 
                 * endPosition and startPosition are assigned to the
                 * _currentPosition pointer in the xcdrStreamPtr.
                 *
                 * The xcdrStreamPtr always points to the same stream for the
                 * duration of the call to this function.
                 *
                 * Note that for this operation _currentPosition does not 
                 * point to a real memory location. It is used as an offset
                 * to compute the size of the serialized sample by subtracting
                 * it from the xcdrStreamPtr buffer pointer which is also 
                 * treated as an offset.
                 */
                /* coverity[cert_arr36_c_violation : FALSE] */
                longSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
                
                if (longSize > RTIXCdrLong_MAX) {
                    *size = RTIXCdrLong_MAX;
                    result = RTI_XCDR_TRUE;
                    goto done;
                }
            }

            context->inBaseClass = tmpInBaseClass;
        } break;
        case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong elementCount;
            RTIXCdrUnsignedLongLong elOffset;
            RTIXCdrTypePluginGetSerializedSampleSizeFunction getSerSampleSizeFcn = NULL;
            RTIXCdrBoolean tmpInBaseClass;
            void *samplePtr = sample;
            struct RTIXCdrSampleAccessInfo *seqElementSampleAccessInfo =
                    commonParams->seqElementTc->_sampleAccessInfo;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());
            
            if (params->complexParams.typePlugin != NULL) {
                getSerSampleSizeFcn = params->complexParams.typePlugin->getSerSampleSizeFnc;
            }

            elementCount = commonParams->count;
            elOffset = commonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX];
            
            tmpInBaseClass = context->inBaseClass;
            
            for (j=0; j<elementCount; j++) {
                RTIXCdrUnsignedLongLong seqElementOffset;
                char *seqElementPtr;

                if (commonParams->refMemberKind ==
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    samplePtr  = RTIXCdrProgram_getExternalRefValuePtr(
                            program,
                            ((char *)sample + elOffset));
                    if (samplePtr == NULL) {
                        logMessageId = RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss;
                        GotoDoneWithLine();
                    }
                }
            
                RTIXCdrInterpreter_getMemberValuePtr(
                        memberValue,
                        &seqElementCount,
                        samplePtr,
                        commonParams->refMemberKind == RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER?
                                0: elOffset,
                        0,
                        commonParams->memberTc,
                        tcMember,
                        commonParams->refMemberKind,
                        commonParams->useGetMemberValue,
                        0,
                        programData);
                /*
                 * For sequences, it is fine to get a null memberValue.value.ptr
                 * from the getMemberValuePtr call. This will be the case of
                 * zero-length sequences.
                 */

                if (seqElementCount > commonParams->memberTc->_maximumLength) {
                    logMessageId = RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu;
                    runTimeLogParam.kind = RTI_XCDR_LOG_ULONG_PARAM;
                    runTimeLogParam.value.ulVal = seqElementCount;
                    GotoDoneWithLine();
                }

                if (commonParams->addSeqDHeader) {
                    /*
                     * If we are processing an array of sequences
                     * we have to skip the DHEADER for each sequence here.
                     */
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_DHEADER_ALIGNMENT,
                            RTI_XCDR_DHEADER_SIZE)) {
                        GotoDoneWithLine();
                    }
                }
                
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    GotoDoneWithLine();
                }
                
                if (commonParams->refMemberKind !=
                        RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER) {
                    elOffset += memberSampleAccessInfo->typeSize[NON_FLAT_DATA_INDEX];
                } else {
                    elOffset += program->externalReferenceSize;
                }

                if (seqElementCount == 0) {
                    continue;
                }

                if (memberValue.isDiscontiguous) {
                    /*
                     * We are dealing with a discontiguous sequence where
                     * the buffer is a buffer of pointers to the actual data
                     * elements.
                     */
                    seqElementOffset = sizeof(char *);
                } else {
                    seqElementOffset =
                            seqElementSampleAccessInfo
                                    ->typeSize[NON_FLAT_DATA_INDEX];
                }

                if (memberValue.value.ptr == NULL) {
                    GotoDoneWithLine();
                }
                
                for (h=0; h<seqElementCount; h++) {
                    if (memberValue.isDiscontiguous) {
                        seqElementPtr = *RTIXCdrUtility_staticCast(
                                char **,
                                memberValue.value.ptr);

                        if (seqElementPtr == NULL) {
                            GotoDoneWithLine();
                        }
                    } else {
                        seqElementPtr = memberValue.value.ptr;
                    }

                    /* We have to assign this here because the variables
                     * can be overwritten by the nested calls
                     */
                    context->program = params->complexParams.program;
                    context->typeCode = params->complexParams.program->typeCode;
                    context->inBaseClass = params->complexParams.baseClass;

                    if (getSerSampleSizeFcn == NULL) {
                        /* Initialized to zero to avoid Purify error described in 
                        * CORE-9936
                        */
                        RTIXCdrUnsignedLong tmpSize = 0;
                        
                         if (!RTIXCdrInterpreter_getSerSampleSize(
                                &tmpSize,
                                seqElementPtr,
                                context->typeCode,
                                context->program,
                                context)) {
                             GotoDoneWithLine();
                         }
                    } else {
                        RTIXCdrUtility_unusedReturnValue(
                                getSerSampleSizeFcn(
                                        context->endpointPluginData,
                                        0,
                                        context->encapsulationId,
                                        *size, /* Ignored */
                                        seqElementPtr), RTIXCdrUnsignedLong);
                    }

                    endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
                    longSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
                    
                    if (longSize > RTIXCdrLong_MAX) {
                        *size = RTIXCdrLong_MAX;
                        result = RTI_XCDR_TRUE;
                        goto done;
                    }

                    memberValue.value.ptr += seqElementOffset;
                }
            }
            
            context->inBaseClass = tmpInBaseClass;
        } break;
        case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        {
            RTIXCdrBoolean memberValueSet = RTI_XCDR_TRUE;
            RTIXCdrCommonInsParameters *memberDataInstCommonParams;
            RTI_UINT32 dataInstIndex;

            dataInstIndex = RTIXCdrInterpreter_getMemberDataInsIndex(
                    program,
                    i);

            memberDataInstCommonParams = RTIXCdrInstruction_getCommonParams(
                    &program->instructions[dataInstIndex]);
            
            if (memberDataInstCommonParams->refMemberKind ==
                    RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                RTIXCdrInterpreter_isOptionalMemberValueSet(
                        &memberValueSet,
                        sample,
                        memberDataInstCommonParams->memberAccessInfo.bindingMemberValueOffset[NON_FLAT_DATA_INDEX],
                        memberDataInstCommonParams->memberTc,
                        memberDataInstCommonParams->tcMemberInfo,
                        programData);
            }
            
            if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY ||
                    memberValueSet) {
                if (program->isCdr2) {
                    if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                        if (!RTIXCdrStream_skipV2ParameterHeader(
                                xcdrStreamPtr,
                                instruction->params.memberHeaderParams.v2LC)) {
                            GotoDoneWithLine();
                        }
                    } else {
                        if (!RTIXCdrStream_skipNByte(
                                xcdrStreamPtr,
                                RTI_XCDR_ONE_BYTE_ALIGNMENT,
                                RTI_XCDR_OCTET_SIZE)) {
                            GotoDoneWithLine();
                        }
                    }
                } else {
                    extended = 
                            RTIXCdrInterpreter_useExtendedV1Id(
                                    instruction, 
                                    context);    
                    
                    if (!RTIXCdrStream_skipV1ParameterHeader(
                            xcdrStreamPtr,
                            &memberStreamState,
                            extended)) {
                        GotoDoneWithLine();
                    }
                }
                
                if (!memberValueSet) {
                    i = dataInstIndex;
                    if (!program->isCdr2 &&
                            program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                        if (!RTIXCdrStream_finishV1ParameterHeader(
                                xcdrStreamPtr,
                                &memberStreamState,
                                extended,
                                RTIXCdrXTypesComplianceMask_isBitSet(
                                        program->xTypesComplianceMask,
                                        RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT),
                                NULL)) {
                            GotoDoneWithLine();
                        }
                    }
                } else {
                    /* This variable will contain the index of the
                     * data instruction associated with this optional/mutable
                     * member.
                     *
                     * We use it below to not finalize the header until we process
                     * the data instruction
                     */
                    memberWithHeaderIndex = dataInstIndex;
                }
            } else if (!memberValueSet) {
                i = dataInstIndex;
            }
        } break;
        case RTI_XCDR_SKIP_DHEADER_OPCODE: 
        {
            if (!context->inBaseClass) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_DHEADER_ALIGNMENT,
                        RTI_XCDR_DHEADER_SIZE)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE: 
        {
            if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    RTI_XCDR_DHEADER_ALIGNMENT,
                    RTI_XCDR_DHEADER_SIZE)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE:
        {
            if (!context->inBaseClass) {
                if (!RTIXCdrStream_skipV1ParameterHeader(
                        xcdrStreamPtr,
                        NULL,
                        RTI_XCDR_FALSE)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        default:
            goto done;
        }

        if (getUnionInstIndex && i == program->unionInsIndex) {
            RTIXCdrBoolean failure;
            RTIXCdrLong caseValue;

            /* 
             * Process union discriminator and select instruction
             * based on discriminator value
             */
            if (RTIXCdrInterpreter_useMemberElementIndex(
                    memberSampleAccessInfo)) {
                RTIXCdrInterpreter_primitiveToLongWIndexMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind, 
                        failure);
            } else {
                /* The NULL check of memberValue.value.ptr only makes sense
                 * when the language binding mapping of the primitive type
                 * is equal (on size) to the wire mapping.
                 *
                 * For example, in QNX and modern C++
                 * the size of a boolean is not one byte, the wire size.
                 * Invoking RTIXCdrInterpreter_useMemberElementIndex
                 * tell us if the wire and binding sizes are the same
                 */
                if (memberValue.value.ptr == NULL) {
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                    GotoDoneWithLine();
                }
                RTIXCdrInterpreter_primitiveToLongMacro(
                        caseValue,
                        memberValue,
                        program->unionDiscKind, 
                        failure);
            }
            if (failure) {
                GotoDoneWithLine();
            }

            RTIXCdrProgram_getUnionMemberInstructionIndex(
                    program,
                    &i,
                    &instructionCount,
                    (RTIBool *) NULL, /* caseValueIdentifiesMember */
                    caseValue);

            if (primitiveDisc) {
                /* 
                 * Only skip here if we didn't already skip the discriminator
                 * above during the RTI_XCDR_SKIP_COMPLEX_OPCODE operation
                 * (!optimizeEnum or !resolveAlias)
                 */ 
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTIXCdr_TCKind_g_primitiveCdrAlignments[program->isCdr2][program->unionDiscKind],
                        RTIXCdr_TCKind_g_primitiveCdrSizes[program->isCdr2][program->unionDiscKind])) {
                    GotoDoneWithLine();
                }
            }

            if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                if (!program->isCdr2) {
                    /*
                     * Coverity reports that memberStreamState's buffer and
                     * bufferLenght can reach this code path uninitialized. This
                     * is not possible since, if the program is mutable, it
                     * should have an operation with code
                     * RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE and
                     * RTIXCdrStream_skipV1ParameterHeader should have been
                     * called at this point.
                     */
                    /* coverity[uninit_use_in_call : FALSE] */
                    if (!RTIXCdrStream_finishV1ParameterHeader(
                            xcdrStreamPtr,
                            &memberStreamState,
                            extended,
                            RTIXCdrXTypesComplianceMask_isBitSet(
                                    program->xTypesComplianceMask,
                                    RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT),
                            NULL)) {
                        GotoDoneWithLine();
                    }
                }
                memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
            }

            getUnionInstIndex = RTI_XCDR_FALSE;
            continue;
        }
   
        if (memberWithHeaderIndex == i) {
            if (!program->isCdr2) {
                if (!RTIXCdrStream_finishV1ParameterHeader(
                        xcdrStreamPtr,
                        &memberStreamState,
                        extended,
                        RTIXCdrXTypesComplianceMask_isBitSet(
                                program->xTypesComplianceMask,
                                RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT),
                        NULL)) {
                    GotoDoneWithLine();
                }
            }
            memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
        }

        i++;
    }

    endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
    /*
     * endPosition and startPosition are assigned to the _currentPosition 
     * pointer in the xcdrStreamPtr.
     *
     * The xcdrStreamPtr always points to the same stream for the duration of 
     * the call to this function.
     *
     * Note that for this operation _currentPosition does not point to a real 
     * memory location. It is used as an offset to compute the size of the 
     * serialized sample by subtracting it from the xcdrStreamPtr buffer pointer
     * which is also treated as an offset.
     */
    /* coverity[cert_arr36_c_violation : FALSE] */
    longSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);

    if (longSize > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        *size = RTI_XCDR_MAX_SERIALIZED_SIZE;
        context->overflow = RTI_XCDR_TRUE;
    } else {
        *size += (RTIXCdrUnsignedLong)longSize;
    }

    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (instruction != NULL) {
            RTIXCdrInterpreter_logSkipError(
                    tc,
                    instruction,
                    logMessageId,
                    &runTimeLogParam,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return result;
}

RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleSizeWithEncapsulation(
        RTIXCdrUnsignedLong *size,
        void *sample,
        const struct RTIXCdrInterpreterPrograms *programs,
        RTIXCdrEncapsulationId encapsulationId)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    struct RTIXCdrTypePluginProgramContext context =
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    RTIXCdrBoolean isCdrV2;

    RTIXCdrLog_testPrecondition(size == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(sample == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);

    isCdrV2 = RTIXCdrEncapsulationId_isCdrV2(encapsulationId);
    context.onlyKey = RTI_XCDR_FALSE;
    context.program = RTIXCdrInterpreterPrograms_getSerSizeProgram(
            programs,
            isCdrV2?RTI_TRUE:RTI_FALSE,
            context.onlyKey);
    if (context.program == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GET_FAILURE_ID_s,
                "serialize size program");
        return RTI_XCDR_FALSE;
    }

    context.typeCode = context.program->typeCode;
    context.overflow = RTI_XCDR_FALSE;
    context.encapsulationId = encapsulationId;
    context.xcdrStream = NULL;
    context.inBaseClass = RTI_XCDR_FALSE;

    *size = 0;

    if (!isCdrV2) {
        RTIXCdrUnsignedLong maxSerSize;

        result = RTIXCdrInterpreter_getSerSampleMaxSizeWithEncapsulation(
                &maxSerSize,
                programs,
                encapsulationId);

        if (!result) {
            goto done;
        }

        if (maxSerSize > RTIXCdrUnsignedShort_MAX) {
            context.useXcdr1ExtendedId = RTI_XCDR_TRUE;
        } else {
            context.useXcdr1ExtendedId = RTI_XCDR_FALSE;
        }
    }

    result = RTIXCdrInterpreter_getSerSampleSize(
            size,
            sample,
            context.typeCode,
            context.program,
            &context);
    if (!result) {
        goto done;
    }
    
    if(!context.overflow) {
        *size += RTI_XCDR_ENCAPSULATION_HEADER_SIZE;
    }
    
    result = RTI_XCDR_TRUE;
  done:

    return result;
}

/*****************************************************************************/
/***** GetSerSampleMaxSize ***************************************************/
/*****************************************************************************/

#define RTIXCdrInterpreter_max(n1,n2) ((n1>n2)?n1:n2)
#define RTIXCdrInterpreter_gotoDoneAfterOverflow() \
    context->overflow = RTI_XCDR_TRUE; \
    *size = RTI_XCDR_MAX_SERIALIZED_SIZE; \
    result = RTI_XCDR_TRUE; \
    GotoDoneWithLine()

RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMaxSize(
        RTIXCdrUnsignedLong *size,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i;
    char *startPosition = NULL, * endPosition = NULL;
    RTIXCdrUnsignedLongLong longSize = 0;
    RTIXCdrUnsignedLongLong discSize = 0;
    RTIXCdrStream xcdrStream;
    RTIXCdrStream *xcdrStreamPtr = NULL;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logMessageId =
            RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrBoolean isUnion;
    RTIXCdrUnsignedLong memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
    struct RTIXCdrStreamState memberStreamState;
    /* discStreamState is initialized to avoid warnings */
    struct RTIXCdrStreamAlignedPosition discAlignedPosition = RTIXCdrStreamAlignedPosition_INITIALIZER;
    struct RTIXCdrStreamAlignedPosition selectedMemberAlignedPosition =
            RTIXCdrStreamAlignedPosition_INITIALIZER;
    RTIXCdrBoolean unionSentinel = RTI_XCDR_FALSE;
    RTIXCdrBoolean isFirstUnionMember = RTI_XCDR_TRUE;

    /*
     * GREEN-898 We need to do this assignment to a local variable in order to
     * prevent static analysis issues regarding the use of xcdrStream in
     * the context structure
     */
    struct RTIXCdrTypePluginProgramContext *localContext = context;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            size == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    if (program->isUnbounded) {
        context->overflow = RTI_XCDR_TRUE;
        *size = RTI_XCDR_MAX_SERIALIZED_SIZE;
        return RTI_XCDR_TRUE;
    }

    if (program->instructions == NULL) {
        *size = program->serSize;
        return RTI_XCDR_TRUE;
    }

    xcdrStreamPtr = localContext->xcdrStream;
    
    if (xcdrStreamPtr == NULL) {
        RTIXCdrStream_initialize(&xcdrStream);
        xcdrStreamPtr = &xcdrStream;
        localContext->xcdrStream = xcdrStreamPtr;
        RTIXCdrInterpreter_setBufferForGetSize(xcdrStreamPtr);
    }

    isUnion = program->unionDiscKind != RTI_XCDR_TK_NULL;
    startPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);

    for (i=0; i<program->instructionCount; i++) {
        RTIXCdrUnsignedLong arrElementCount;
        RTIXCdrUnsignedLong byteCount;
        RTIXCdrCommonInsParameters *commonParams;
        RTIXCdrInsParameters *params;
        RTIXCdrUnsignedLong seqMaxElementCount;

        /*
         * Coverity complains because it detects that we could dereference
         * xcdrStreamPtr->_currentPosition (e.g. in RTIXCdrStream_skipNByte)
         * after setting it to a non-null fixed value (8) in
         * RTIXCdrInterpreter_setBufferForGetSize. This is not true because
         * xcdrStreamPtr->_zeroOnAlign is false. See the comment above
         * RTIXCdrInterpreter_setBufferForGetSize -- we don't access the xcdr
         * buffers in the get*Size functions.
         */
        RTIXCdrLog_testPrecondition(
                xcdrStreamPtr->_zeroOnAlign,
                return RTI_XCDR_FALSE);

        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        arrElementCount = commonParams->count;

        switch (instruction->opcode) {
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    params->primitiveParams.primitiveAlignment,
                    arrElementCount)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        {
            RTIXCdrUnsignedLong seqElementCount =
                commonParams->memberTc->_maximumLength;
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;

            RTIXCdrInterpreter_adjustSequenceElementCount(
                    seqElementCount, 
                    byteCount,
                    params);

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }

            while (!endArrIt) {
                if (commonParams->addSeqDHeader) {
                    /*
                     * If we are processing an array of sequences
                     * we have to skip the DHEADER for each sequence here.
                     */
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_DHEADER_ALIGNMENT,
                            RTI_XCDR_DHEADER_SIZE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }

                if (byteCount > 0) {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            params->primitiveParams.primitiveAlignment,
                            byteCount)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }
        } break;
        case RTI_XCDR_SKIP_STRING_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_OPCODE:
        {
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;
            byteCount = params->strParams.charMaxCount *
                    params->strParams.charSize;

            if (params->strParams.charAlignment ==
                    RTI_XCDR_WCHAR_ALIGNMENT &&
                    byteCount > 0) {
                /* NULL-terminated is not serialized */
                byteCount -= params->strParams.charSize;
            }

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }

            while (!endArrIt) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }

                if (byteCount > 0) {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            params->strParams.charAlignment,
                            byteCount)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }
        } break;
        case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        {
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;
            byteCount = params->strParams.charMaxCount *
                    params->strParams.charSize;

            if (params->strParams.charAlignment ==
                    RTI_XCDR_WCHAR_ALIGNMENT &&
                    byteCount > 0) {
                /* NULL-terminated is not serialized */
                byteCount -= params->strParams.charSize;
            }

            seqMaxElementCount = commonParams->memberTc->_maximumLength;

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }

            while (!endArrIt) {
                struct RTIXCdrArraySizeIterator seqSizeIt =
                        RTIXCdrArraySizeIterator_INITIALIZER;
                RTIXCdrBoolean endSeqIt;

                if (commonParams->addSeqDHeader) {
                    /*
                     * If we are processing an array of sequences of strings
                     * we have to skip the DHEADER for each sequence here.
                     */
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_DHEADER_ALIGNMENT,
                            RTI_XCDR_DHEADER_SIZE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }

                if (!RTIXCdrArraySizeIterator_first(
                        &seqSizeIt,
                        xcdrStreamPtr,
                        &endSeqIt,
                        seqMaxElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }

                while (!endSeqIt) {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                            RTI_XCDR_FOUR_BYTE_SIZE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }

                    if (byteCount > 0) {
                        if (!RTIXCdrStream_skipNByte(
                                xcdrStreamPtr,
                                params->strParams.charAlignment,
                                byteCount)) {
                            RTIXCdrInterpreter_gotoDoneAfterOverflow();
                        }
                    }

                    if (!RTIXCdrArraySizeIterator_next(
                            &seqSizeIt,
                            xcdrStreamPtr,
                            &endSeqIt,
                            seqMaxElementCount)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginGetSerializedSampleMaxSizeFunction 
                getSerSampleMaxSizeFnc = NULL;
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;

            /*
             * Coverity is not smart enough to recognize that
             * RTIXCdrInstruction_isHeaderOpcode must be false for
             * RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE. We add this precondition to
             * appease Coverity.
             */
            RTIXCdrLog_testPrecondition(
                    commonParams->memberTc == NULL,
                    context->expectedSpaceError = RTI_XCDR_FALSE;
                            GotoDoneWithLine());
            
            if (params->complexParams.typePlugin != NULL) {
                if (!context->onlyKey) {
                    getSerSampleMaxSizeFnc = (RTIXCdrTypePluginGetSerializedSampleMaxSizeFunction)
                            params->complexParams.typePlugin->getSerSampleMaxSizeFnc;
                } else {
                    getSerSampleMaxSizeFnc = (RTIXCdrTypePluginGetSerializedSampleMaxSizeFunction)
                            params->complexParams.typePlugin->getSerKeyMaxSizeFnc;                
                }
            }

            if (instruction->opcode == RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE) {
                seqMaxElementCount = commonParams->memberTc->_maximumLength;
            } else {
                seqMaxElementCount = 1;
            }
            
            tmpInBaseClass = context->inBaseClass;

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }

            while (!endArrIt) {
                struct RTIXCdrArraySizeIterator seqSizeIt =
                        RTIXCdrArraySizeIterator_INITIALIZER;
                RTIXCdrBoolean endSeqIt;

                if (instruction->opcode == RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE) {
                    if (commonParams->addSeqDHeader) {
                        /*
                        * If we are processing an array of sequences
                        * we have to skip the DHEADER for each sequence here.
                        */
                        if (!RTIXCdrStream_skipNByte(
                                xcdrStreamPtr,
                                RTI_XCDR_DHEADER_ALIGNMENT,
                                RTI_XCDR_DHEADER_SIZE)) {
                            RTIXCdrInterpreter_gotoDoneAfterOverflow();
                        }
                    }

                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                            RTI_XCDR_FOUR_BYTE_SIZE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrArraySizeIterator_first(
                        &seqSizeIt,
                        xcdrStreamPtr,
                        &endSeqIt,
                        seqMaxElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }

                while (!endSeqIt) {
                    /* We have to assign this here because the variables
                     * can be overwritten by the nested calls
                     */
                    context->program = params->complexParams.program;
                    context->typeCode = params->complexParams.program->typeCode;
                    context->inBaseClass = params->complexParams.baseClass;

                    if (getSerSampleMaxSizeFnc == NULL) {
                        /* Initialized to zero to avoid Purify error described in 
                        * CORE-9936
                        */
                        RTIXCdrUnsignedLong tmpSize = 0;
                        
                        if (!RTIXCdrInterpreter_getSerSampleMaxSize(
                                &tmpSize,
                                context->typeCode,
                                context->program,
                                context)) {
                            GotoDoneWithLine();                            
                        }
                    } else {
                        RTIXCdrUtility_unusedReturnValue(
                                getSerSampleMaxSizeFnc(
                                        context->endpointPluginData,
                                        &context->overflow,
                                        0,
                                        context->encapsulationId,
                                        (RTIXCdrUnsignedLong)0 /* Ignored */),
                                RTIXCdrUnsignedLong);
                    }

                    if (context->overflow) {
                        *size = RTI_XCDR_MAX_SERIALIZED_SIZE;
                        result = RTI_XCDR_TRUE;
                        goto done;
                    }

                    if (!RTIXCdrArraySizeIterator_next(
                            &seqSizeIt,
                            xcdrStreamPtr,
                            &endSeqIt,
                            seqMaxElementCount)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }

            context->inBaseClass = tmpInBaseClass;
        } break;
        case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        {
            if (program->isCdr2) {
                if (program->extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                    if (!RTIXCdrStream_skipV2ParameterHeader(
                            xcdrStreamPtr,
                            instruction->params.memberHeaderParams.v2LC)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                } else {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_ONE_BYTE_ALIGNMENT,
                            RTI_XCDR_OCTET_SIZE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }
            } else {
                if (!RTIXCdrStream_skipV1ParameterHeader(
                        xcdrStreamPtr,
                        &memberStreamState,
                        RTI_XCDR_TRUE)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }

            /* This variable will contain the index of the
             * data instruction associated with this optional/mutable
             * member.
             *
             * We use it below to not finalize the header until we process
             * the data instruction
             */
            memberWithHeaderIndex = RTIXCdrInterpreter_getMemberDataInsIndex(
                    program,
                    i);
        } break;
        case RTI_XCDR_SKIP_DHEADER_OPCODE: 
        {
            if (!context->inBaseClass) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_DHEADER_ALIGNMENT,
                        RTI_XCDR_DHEADER_SIZE)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE: 
        {
            if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    RTI_XCDR_DHEADER_ALIGNMENT,
                    RTI_XCDR_DHEADER_SIZE)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }
        } break;
        case RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE:
        {
            if (isUnion) {
                unionSentinel = RTI_XCDR_TRUE;
            } else {
                if (!context->inBaseClass || program->serializeSentinelOnBase) {
                    if (!RTIXCdrStream_skipV1ParameterHeader(
                            xcdrStreamPtr,
                            NULL,
                            RTI_XCDR_TRUE)) {
                        RTIXCdrInterpreter_gotoDoneAfterOverflow();
                    }
                }
            }
        } break;
        default:
            GotoDoneWithLine();
        }

        if (memberWithHeaderIndex == i) {
            if (!program->isCdr2) {
                if (!RTIXCdrStream_finishV1ParameterHeader(
                        xcdrStreamPtr,
                        &memberStreamState,
                        RTI_XCDR_TRUE,
                        RTIXCdrXTypesComplianceMask_isBitSet(
                                program->xTypesComplianceMask,
                                RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT),
                        NULL)) {
                    RTIXCdrInterpreter_gotoDoneAfterOverflow();
                }
            }
            memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
        }

        if (isUnion) {

            if (i == program->unionInsIndex) {
                endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
                /*
                * endPosition and startPosition are assigned to the 
                * _currentPosition pointer in the xcdrStreamPtr.
                *
                * The xcdrStreamPtr always points to the same stream for the 
                * duration of the call to this function.
                *
                * Note that for this operation _currentPosition does not point 
                * to a real memory location. It is used as an offset to compute 
                * the size of the serialized sample by subtracting it from the 
                * xcdrStreamPtr buffer pointer which is also treated as an 
                * offset.
                */
                /* coverity[cert_arr36_c_violation : FALSE] */
                discSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
                RTIXCdrStream_getAlignedPosition(
                        xcdrStreamPtr,
                        &discAlignedPosition);
            } else if (!RTIXCdrInstruction_isHeaderOpcode(instruction->opcode)) {
                RTIXCdrUnsignedLongLong maxLongSize;
                endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
 
                maxLongSize = RTIXCdrInterpreter_max(
                        (RTIXCdrUnsignedLongLong)(endPosition - discAlignedPosition.currentPosition),
                        longSize);

                if (maxLongSize > longSize || isFirstUnionMember) {
                    isFirstUnionMember = RTI_XCDR_FALSE;
                    RTIXCdrStream_getAlignedPosition(
                            xcdrStreamPtr,
                            &selectedMemberAlignedPosition);
                    longSize = maxLongSize;
                }

                RTIXCdrStream_restoreAlignedPosition(xcdrStreamPtr, &discAlignedPosition);
            }
        }
    }

    if (!isUnion) {
        endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
        longSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
    } else {
        longSize += discSize;
        RTIXCdrStream_restoreAlignedPosition(xcdrStreamPtr, &selectedMemberAlignedPosition);

        if (unionSentinel)
        {
            startPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
            if (!RTIXCdrStream_skipV1ParameterHeader(
                    xcdrStreamPtr,
                    NULL,
                    RTI_XCDR_TRUE)) {
                RTIXCdrInterpreter_gotoDoneAfterOverflow();
            }
            endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
            longSize += (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
        }
    }

    if (longSize > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        RTIXCdrInterpreter_gotoDoneAfterOverflow();
    }
    longSize += *size;
    if (longSize > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        RTIXCdrInterpreter_gotoDoneAfterOverflow();
    }
    *size = (RTIXCdrUnsignedLong) longSize;

    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (instruction != NULL) {
            RTIXCdrInterpreter_logSkipError(
                    tc,
                    instruction,
                    logMessageId,
                    NULL,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return result;
}

#undef RTIXCdrInterpreter_gotoDoneAfterOverflow

RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMaxSizeWithEncapsulation(
        RTIXCdrUnsignedLong *size,
        const struct RTIXCdrInterpreterPrograms *programs,
        RTIXCdrEncapsulationId encapsulationId)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    struct RTIXCdrTypePluginProgramContext context =
            RTIXCdrTypePluginProgramContext_INTIALIZER;
    RTIXCdrBoolean isCdrV2;

    RTIXCdrLog_testPrecondition(programs == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(size == NULL, return RTI_XCDR_FALSE);

    *size = 0;
    isCdrV2 = RTIXCdrEncapsulationId_isCdrV2(encapsulationId);
    context.onlyKey = RTI_XCDR_FALSE;
    context.program = RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(
            programs,
            isCdrV2?RTI_TRUE:RTI_FALSE,
            context.onlyKey);
    if (context.program == NULL) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GET_FAILURE_ID_s,
                "max serialized size program");
        return RTI_XCDR_FALSE;
    }

    context.typeCode = context.program->typeCode;
    context.overflow = RTI_XCDR_FALSE;
    context.inBaseClass = RTI_XCDR_FALSE;
    context.xcdrStream = NULL;
    
    result = RTIXCdrInterpreter_getSerSampleMaxSize(
            size,
            context.typeCode,
            context.program,
            &context);
    if (!result) {
        goto done;
    }

    if(!context.overflow) {
        RTIXCdrUnsignedLongLong longSize =
                (*size) + RTI_XCDR_ENCAPSULATION_HEADER_SIZE;
        if (longSize > RTI_XCDR_MAX_SERIALIZED_SIZE) {
            *size = RTI_XCDR_MAX_SERIALIZED_SIZE;
        } else {
            *size += RTI_XCDR_ENCAPSULATION_HEADER_SIZE;
        }
    }

    result = RTI_XCDR_TRUE;
  done:

    if (!result) {
        return 0;
    }
    return result;
  }

/*****************************************************************************/
/***** GetSerSampleMinSize ***************************************************/
/*****************************************************************************/

#define RTIXCdrInterpreter_min(n1,n2) ((n1<n2)?n1:n2)

RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMinSize(
        RTIXCdrUnsignedLong *size,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context)
{
    RTIXCdrBoolean result = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i;
    char *startPosition = NULL, * endPosition = NULL;
    RTIXCdrUnsignedLongLong longSize = RTIXCdrUnsignedLong_MAX;
    RTIXCdrUnsignedLongLong discSize = 0;
    RTIXCdrStream xcdrStream;
    RTIXCdrStream *xcdrStreamPtr = NULL;
    struct RTIXCdrInstruction *instruction = NULL;
    RTIXCdrUnsignedLong logMessageId =
            RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss;
    RTIXCdrUnsignedLong logLineNumber = 0;
    RTIXCdrBoolean isUnion;
    RTIXCdrUnsignedLong memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
    struct RTIXCdrStreamState memberStreamState;
    struct RTIXCdrStreamAlignedPosition discAlignedPosition = RTIXCdrStreamAlignedPosition_INITIALIZER;
    struct RTIXCdrStreamAlignedPosition selectedMemberAlignedPosition =
            RTIXCdrStreamAlignedPosition_INITIALIZER;
    /* GREEN-898 We need to do this assignment to a local variable in order to
     * prevent static analysis issues regarding the use of xcdrStream in
     * the context structure
     */
    struct RTIXCdrTypePluginProgramContext *localContext = context;
    RTIBool unionSentinel = RTI_XCDR_FALSE;
    RTIBool branchCircularPath;
    RTIBool circularPath = RTI_XCDR_FALSE;
    RTIXCdrBoolean isFirstUnionMember = RTI_XCDR_TRUE;

    RTIXCdrLog_testPrecondition(context == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            size == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            program == NULL,
            context->expectedSpaceError = RTI_XCDR_FALSE; return RTI_XCDR_FALSE);

    xcdrStreamPtr = localContext->xcdrStream;

    if (xcdrStreamPtr == NULL) {
        RTIXCdrStream_initialize(&xcdrStream);
        xcdrStreamPtr = &xcdrStream;
        localContext->xcdrStream = xcdrStreamPtr;
        localContext->parentVisitedNode = NULL;
        RTIXCdrInterpreter_setBufferForGetSize(xcdrStreamPtr);
    }

    if (program->instructions == NULL) {
        *size = program->serSize;
        return RTI_XCDR_TRUE;
    }

    startPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
    isUnion = program->unionDiscKind != RTI_XCDR_TK_NULL;

    for (i=0; i<program->instructionCount; i++) {
        RTIXCdrUnsignedLong arrElementCount;
        RTIXCdrInsParameters *params;
        RTIXCdrCommonInsParameters *commonParams;

        /*
         * Coverity complains because it detects that we could dereference
         * xcdrStreamPtr->_currentPosition (e.g. in RTIXCdrStream_skipNByte)
         * after setting it to a non-null fixed value (8) in
         * RTIXCdrInterpreter_setBufferForGetSize. This is not true because
         * xcdrStreamPtr->_zeroOnAlign is false. See the comment above
         * RTIXCdrInterpreter_setBufferForGetSize -- we don't access the xcdr
         * buffers in the get*Size functions.
         */
        RTIXCdrLog_testPrecondition(
                xcdrStreamPtr->_zeroOnAlign,
                return RTI_XCDR_FALSE);

        branchCircularPath = RTI_XCDR_FALSE;
        instruction = &program->instructions[i];
        params = &instruction->params;
        commonParams = RTIXCdrInstruction_getCommonParams(
                instruction);

        arrElementCount = commonParams->count;

        switch (instruction->opcode) {
        case RTI_XCDR_SKIP_PRIMITIVE_OPCODE:
        {
            if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    params->primitiveParams.primitiveAlignment,
                    arrElementCount)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_WSTRING_OPCODE:
        {
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                GotoDoneWithLine();
            }

            while (!endArrIt) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE)) {
                    GotoDoneWithLine();
                }

                if (params->strParams.charAlignment != 
                	RTI_XCDR_WCHAR_ALIGNMENT) {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            params->strParams.charAlignment,
                            params->strParams.charSize)) {
                        GotoDoneWithLine();
                    }
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_STRING_OPCODE:
        {
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                GotoDoneWithLine();
            }

            while (!endArrIt) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        RTI_XCDR_FOUR_BYTE_SIZE + 1)) {
                    GotoDoneWithLine();
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COMPLEX_OPCODE:
        {
            RTIXCdrBoolean tmpInBaseClass;
            RTIXCdrTypePluginGetSerializedSampleMinSizeFunction 
                getSerSampleMinSizeFnc = NULL;
            struct RTIXCdrArraySizeIterator arrSizeIt =
                    RTIXCdrArraySizeIterator_INITIALIZER;
            RTIXCdrBoolean endArrIt;
            RTIXCdrTypeCodeNode parentVisitedNode;
            const RTIXCdrTypeCodeNode *oldParentVisitedNode;

            parentVisitedNode.prev = context->parentVisitedNode;
            parentVisitedNode.tc = tc;

            if (RTIXCdrTypeCode_isTypeCodeVisited(
                        commonParams->memberTc,
                        &parentVisitedNode)) {
                branchCircularPath = RTI_XCDR_TRUE;
                circularPath = RTI_XCDR_TRUE;
                break;
            }

            oldParentVisitedNode = context->parentVisitedNode;
            localContext->parentVisitedNode = &parentVisitedNode;

            if (params->complexParams.typePlugin != NULL) {
                getSerSampleMinSizeFnc = 
                        (RTIXCdrTypePluginGetSerializedSampleMinSizeFunction)
                            params->complexParams.typePlugin->getSerSampleMinSizeFnc;
            }

            tmpInBaseClass = context->inBaseClass;

            if (!RTIXCdrArraySizeIterator_first(
                    &arrSizeIt,
                    xcdrStreamPtr,
                    &endArrIt,
                    arrElementCount)) {
                localContext->parentVisitedNode = oldParentVisitedNode;
                GotoDoneWithLine();
            }

            while (!endArrIt) {
                /* We have to assign this here because the variables
                 * can be overwritten by the nested calls
                 */
                context->program = params->complexParams.program;
                context->typeCode = commonParams->memberTc;
                context->inBaseClass = params->complexParams.baseClass;

                if (getSerSampleMinSizeFnc == NULL) {
                    /* Initialized to zero to avoid Purify error described in 
                     * CORE-9936
                     */
                    RTIXCdrUnsignedLong tmpSize = 0;

                    if (!RTIXCdrInterpreter_getSerSampleMinSize(
                            &tmpSize,
                            context->typeCode,
                            context->program,
                            context)) {
                        localContext->parentVisitedNode = oldParentVisitedNode;
                        GotoDoneWithLine();
                    }
                } else {
                    RTIXCdrUtility_unusedReturnValue(
                            getSerSampleMinSizeFnc(
                                    context->endpointPluginData,
                                    0,
                                    context->encapsulationId,
                                    0 /* Ignored */),
                            RTIXCdrUnsignedLong);
                }

                if (!RTIXCdrArraySizeIterator_next(
                        &arrSizeIt,
                        xcdrStreamPtr,
                        &endArrIt,
                        arrElementCount)) {
                    localContext->parentVisitedNode = oldParentVisitedNode;
                    GotoDoneWithLine();
                }
            }

            context->inBaseClass = tmpInBaseClass;
            localContext->parentVisitedNode = oldParentVisitedNode;
        } break;
        case RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE:
        case RTI_XCDR_SKIP_STRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE:
        case RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE:
        {
            /* Assume that the sequence has 0 elements */
            if (arrElementCount != 0) {
                const RTIXCdrUnsignedLong headerSize =
                        commonParams->addSeqDHeader ? RTI_XCDR_EIGHT_BYTE_SIZE
                                                    : RTI_XCDR_FOUR_BYTE_SIZE;
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                        headerSize)) {
                    GotoDoneWithLine();
                }

                if (arrElementCount != 1) {
                    if (!RTIXCdrStream_skipNByte(
                            xcdrStreamPtr,
                            RTI_XCDR_FOUR_BYTE_ALIGNMENT,
                            headerSize*
                                (arrElementCount-1))) {
                        GotoDoneWithLine();
                    }
                }
            }
        } break;
        case RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE:
        {
            RTI_UINT32 dataInstIndex;
            RTIXCdrCommonInsParameters *memberDataInstCommonParams;

            dataInstIndex = RTIXCdrInterpreter_getMemberDataInsIndex(
                    program,
                    i);

            memberDataInstCommonParams = RTIXCdrInstruction_getCommonParams(
                    &program->instructions[dataInstIndex]);

            if (memberDataInstCommonParams->refMemberKind !=
                    RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER) {
                if (program->isCdr2) {
                    if (!RTIXCdrStream_skipV2ParameterHeader(
                            xcdrStreamPtr,
                            instruction->params.memberHeaderParams.v2LC)) {
                        GotoDoneWithLine();
                    }
                } else {
                    RTIXCdrBoolean extended =
                            RTIXCdrInterpreter_useExtendedV1Id(
                                    instruction,
                                    context);

                    if (!RTIXCdrStream_skipV1ParameterHeader(
                            xcdrStreamPtr,
                            &memberStreamState,
                            extended)) {
                        GotoDoneWithLine();
                    }
                }

                memberWithHeaderIndex = dataInstIndex;
            } else {
                /* With mutable types we do not send optional members */
                if (program->extKind != RTI_XCDR_MUTABLE_EXTENSIBILITY) {
                    if (program->isCdr2) {
                        if (!RTIXCdrStream_skipNByte(
                                xcdrStreamPtr,
                                RTI_XCDR_ONE_BYTE_ALIGNMENT,
                                RTI_XCDR_OCTET_SIZE)) {
                            GotoDoneWithLine();
                        }
                    } else {
                        RTIXCdrBoolean extended =
                                RTIXCdrInterpreter_useExtendedV1Id(
                                        instruction,
                                        context);

                        if (!RTIXCdrStream_skipV1ParameterHeader(
                                xcdrStreamPtr,
                                NULL,
                                extended)) {
                            GotoDoneWithLine();
                        }
                    }
                }

                i = dataInstIndex;
            }
        } break;
        case RTI_XCDR_SKIP_DHEADER_OPCODE: 
        {
            if (!context->inBaseClass) {
                if (!RTIXCdrStream_skipNByte(
                        xcdrStreamPtr,
                        RTI_XCDR_DHEADER_ALIGNMENT,
                        RTI_XCDR_DHEADER_SIZE)) {
                    GotoDoneWithLine();
                }
            }
        } break;
        case RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE: 
        {
            if (!RTIXCdrStream_skipNByte(
                    xcdrStreamPtr,
                    RTI_XCDR_DHEADER_ALIGNMENT,
                    RTI_XCDR_DHEADER_SIZE)) {
                GotoDoneWithLine();
            }
        } break;
        case RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE:
        {
            if (isUnion) {
                unionSentinel = RTI_XCDR_TRUE;
            } else {
                if (!context->inBaseClass) {
                    if (!RTIXCdrStream_skipV1ParameterHeader(
                            xcdrStreamPtr,
                            NULL,
                            RTI_XCDR_FALSE)) {
                        GotoDoneWithLine();
                    }
                }
            }
        } break;
        default:
            GotoDoneWithLine();
        }
        
        if (memberWithHeaderIndex == i) {
            if (!program->isCdr2) {
                if (!RTIXCdrStream_finishV1ParameterHeader(
                        xcdrStreamPtr,
                        &memberStreamState,
                        RTI_XCDR_TRUE,
                        RTIXCdrXTypesComplianceMask_isBitSet(
                                program->xTypesComplianceMask,
                                RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT),
                        NULL)) {
                    GotoDoneWithLine();
                }
            }
            memberWithHeaderIndex = RTIXCdrUnsignedLong_MAX;
        }

        if (isUnion) {
            if (i == program->unionInsIndex) {
                endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
                /*
                * endPosition and startPosition are assigned to the 
                * _currentPosition pointer in the xcdrStreamPtr.
                *
                * The xcdrStreamPtr always points to the same stream for the 
                * duration of the call to this function.
                *
                * Note that for this operation _currentPosition does not point 
                * to a real memory location. It is used as an offset to compute 
                * the size of the serialized sample by subtracting it from the 
                * xcdrStreamPtr buffer pointer which is also treated as an 
                * offset.
                */
                /* coverity[cert_arr36_c_violation : FALSE] */
                discSize = (RTIXCdrUnsignedLongLong)(endPosition - startPosition);
                RTIXCdrStream_getAlignedPosition(
                        xcdrStreamPtr,
                        &discAlignedPosition);
            } else if (!RTIXCdrInstruction_isHeaderOpcode(
                               instruction->opcode)) {
                RTIXCdrUnsignedLongLong minLongSize;

                if (!branchCircularPath) {
                    endPosition =
                            RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);

                    minLongSize = RTIXCdrInterpreter_min(
                                (RTIXCdrUnsignedLongLong)(endPosition - discAlignedPosition.currentPosition),
                                longSize);

                    if (minLongSize < longSize || isFirstUnionMember) {
                        isFirstUnionMember = RTI_XCDR_FALSE;
                        RTIXCdrStream_getAlignedPosition(
                                xcdrStreamPtr,
                                &selectedMemberAlignedPosition);
                        longSize = minLongSize;
                    }
                }

                RTIXCdrStream_restoreAlignedPosition(
                        xcdrStreamPtr,
                        &discAlignedPosition);
            }
        } else if (circularPath) {
            break;
        }
    }

    if (!isUnion) {
        if (!circularPath) {
            endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
            longSize = (RTIXCdrUnsignedLongLong) (endPosition - startPosition);
        }
    } else if (longSize <= RTI_XCDR_MAX_SERIALIZED_SIZE) {
        longSize += discSize;
        RTIXCdrStream_restoreAlignedPosition(
                xcdrStreamPtr,
                &selectedMemberAlignedPosition);

        if (unionSentinel) {
            startPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
            if (!RTIXCdrStream_skipV1ParameterHeader(
                        xcdrStreamPtr,
                        NULL,
                        RTI_XCDR_FALSE)) {
                GotoDoneWithLine();
            }
            endPosition = RTIXCdrStream_getCurrentPosition(xcdrStreamPtr);
            longSize += (RTIXCdrUnsignedLongLong) (endPosition - startPosition);
        }
    }

    if (longSize > RTI_XCDR_MAX_SERIALIZED_SIZE) {
        *size = RTI_XCDR_MAX_SERIALIZED_SIZE;
        context->overflow = RTI_XCDR_TRUE;
    } else {
        *size += (RTIXCdrUnsignedLong)longSize;
    }

    result = RTI_XCDR_TRUE;
  done:
    if (!result) {
        if (instruction != NULL) {
            RTIXCdrInterpreter_logSkipError(
                    tc,
                    instruction,
                    logMessageId,
                    NULL,
                    RTI_XCDR_FUNCTION_NAME,
                    logLineNumber);
        }
    }

    return result;
}
