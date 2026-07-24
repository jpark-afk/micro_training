/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "../infrastructure/Infrastructure.h"
#include "../typeCode/TypeCode.h"
#include "ProgramSupport.h"

void RTIXCdrInterpreterPrograms_finalize(struct RTIXCdrInterpreterPrograms *me)
{
    RTIXCdrLong i = 0, j = 0, h = 0;
    
    RTIXCdrLog_testPrecondition(me == NULL, return);

    if (me != me->topLevelPrograms) {
        /* Only the top-level program owns the programs and can delete them. */
        return;
    }

    for (i=0; i<2; i++) {
        for (j=0; j<2; j++) {
            for (h=0; h<2; h++) {
                if (me->serializeProgram[i][j][h] != NULL) {
                    RTIXCdrInterpreter_deleteProgram(
                            me->serializeProgram[i][j][h]);
                    me->serializeProgram[i][j][h] = NULL;
                }
                if (me->deserializeProgram[i][j][h] != NULL) {
                    RTIXCdrInterpreter_deleteProgram(
                            me->deserializeProgram[i][j][h]);
                    me->deserializeProgram[i][j][h] = NULL;
                }
                if (me->skipProgram[i][j][h] != NULL) {
                    RTIXCdrInterpreter_deleteProgram(
                            me->skipProgram[i][j][h]);
                    me->skipProgram[i][j][h] = NULL;
                }
            }
            if (me->getSerSizeProgram[i][j] != NULL) {
                RTIXCdrInterpreter_deleteProgram(
                        me->getSerSizeProgram[i][j]);
                me->getSerSizeProgram[i][j] = NULL;
            }
            if (me->getMaxSerSizeProgram[i][j] != NULL) {
                RTIXCdrInterpreter_deleteProgram(
                        me->getMaxSerSizeProgram[i][j]);
                me->getMaxSerSizeProgram[i][j] = NULL;
            }
            if (me->getMinSerSizeProgram[i][j] != NULL) {
                RTIXCdrInterpreter_deleteProgram(
                        me->getMinSerSizeProgram[i][j]);
                me->getMinSerSizeProgram[i][j] = NULL;
            }
            if (me->serializeToKeyProgram[i][j] != NULL) {
                RTIXCdrInterpreter_deleteProgram(
                        me->serializeToKeyProgram[i][j]);
                me->serializeToKeyProgram[i][j] = NULL;
            }
        }
    }

    if (me->initializeSampleProgram != NULL) {
        RTIXCdrInterpreter_deleteProgram(
                me->initializeSampleProgram);
        me->initializeSampleProgram = NULL;
    }

    if (me->allocatedMembersSampleProgram != NULL) {
        RTIXCdrInterpreter_deleteProgram(
                me->allocatedMembersSampleProgram); 
        me->allocatedMembersSampleProgram = NULL;       
    }

    if (me->serializeKeyForKeyhashProgram != NULL) {
        RTIXCdrInterpreter_deleteProgram(
                me->serializeKeyForKeyhashProgram);
        me->serializeKeyForKeyhashProgram = NULL;
    }

    if (me->getMaxKeySerSizeForKeyhashProgram != NULL) {
        RTIXCdrInterpreter_deleteProgram(
                me->getMaxKeySerSizeForKeyhashProgram);
        me->getMaxKeySerSizeForKeyhashProgram = NULL;
    }
}

/*
 * Generates a set of programs defined by a mask if they don't exist yet. If
 * the programs already exist, this function has no effect.
 *
 * @param me Program set whose programs we generate.
 * @param mask Set of programs to generate.
 * @param tc Typecode to use for generating the programs. Note use this
 * parameter instead of using me->type. This is needed to be able to populate
 * programs for types that do not support them but whose children do.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreterPrograms_generate(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrProgramMask mask,
        const RTIXCdrTypeCode *tc)
{
    int i = 0, j = 0, h = 0;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrTypeProgramKind programKind;
    struct RTIXCdrTypePluginProgramProperty programProperty =
            RTIXCdrTypePluginProgramProperty_INITIALIZER;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isCdrRepresentation(tc)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "typecode is in CDR representation");
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrTypeCode_isCollectionKind(RTIXCdrTypeCode_getKind(tc))
            && ((mask & ~RTI_XCDR_PROGRAM_MASK_SAMPLE)
                    != RTI_XCDR_PROGRAM_MASK_NONE)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "non-sample programs on collection");
        return RTI_XCDR_FALSE;
    }

    RTIXCdrInterpreterProgramsGenProperty_toTypePluginProperty(
            &me->property,
            &programProperty);

    /* First dimension: LE (1) or BE (0)
     * Second dimension: V2 (1) or V1 (0)
     * Third dimension: OnlyKey (1) AllFields (0)
     */
    for (i=0; i<2; i++) {
        programProperty.littleEndianEncapsulation = 
                (i == 0)?RTI_XCDR_FALSE:RTI_XCDR_TRUE;

        if (!me->property.generateLittleEndian &&
                programProperty.littleEndianEncapsulation) {
            continue;
        }

        if (!me->property.generateBigEndian &&
                !programProperty.littleEndianEncapsulation) {
            continue;
        }

        for (j=0; j<2; j++) {
            programProperty.v2Encapsulation = 
                    (j == 0)?RTI_XCDR_FALSE:RTI_XCDR_TRUE;
            
            if (!me->property.generateV2Encapsulation &&
                    programProperty.v2Encapsulation) {
                continue;
            }

            if (!me->property.generateV1Encapsulation &&
                    !programProperty.v2Encapsulation) {
                continue;
            }

            for (h=0; h<2; h++) {
                programProperty.onlyKey = 
                        (h == 0)?RTI_XCDR_FALSE:RTI_XCDR_TRUE;

                if (!me->property.generateWithAllFields &&
                        !programProperty.onlyKey) {
                    continue;
                }

                if (!me->property.generateWithOnlyKeyFields &&
                        programProperty.onlyKey) {
                    continue;
                }

                if (mask & RTI_XCDR_SER_PROGRAM) {
                    if (me->serializeProgram[i][j][h] == NULL) {
                        me->serializeProgram[i][j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_SER_PROGRAM,
                                &programProperty);

                        if (me->serializeProgram[i][j][h] == NULL) {
                            programKind = RTI_XCDR_SER_PROGRAM;
                            goto done;
                        }
                    }
                }

                if (mask & RTI_XCDR_DESER_PROGRAM) {
                    if (me->deserializeProgram[i][j][h] == NULL) {
                        me->deserializeProgram[i][j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_DESER_PROGRAM,
                                &programProperty);

                        if (me->deserializeProgram[i][j][h] == NULL) {
                            programKind = RTI_XCDR_DESER_PROGRAM;
                            goto done;
                        }
                    }
                }

                if (mask & RTI_XCDR_SKIP_PROGRAM) {
                    if (me->skipProgram[i][j][h] == NULL) {
                        me->skipProgram[i][j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_SKIP_PROGRAM,
                                &programProperty);

                        if (me->skipProgram[i][j][h] == NULL) {
                            programKind = RTI_XCDR_SKIP_PROGRAM;
                            goto done;
                        }
                    }
                }

                if (mask & RTI_XCDR_GET_SER_SIZE_PROGRAM) {                    
                    if (me->getSerSizeProgram[j][h] == NULL) {
                        me->getSerSizeProgram[j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_GET_SER_SIZE_PROGRAM,
                                &programProperty);
    
                        if (me->getSerSizeProgram[j][h] == NULL) {
                            programKind = RTI_XCDR_GET_SER_SIZE_PROGRAM;
                            goto done;
                        }
                    }
                }

                if (mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) {
                    if (me->getMaxSerSizeProgram[j][h] == NULL) {
                        me->getMaxSerSizeProgram[j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM,
                                &programProperty);
    
                        if (me->getMaxSerSizeProgram[j][h] == NULL) {
                            programKind = RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM;
                            goto done;
                        }
                    }
                }

                if (mask & RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM) {
                    if (me->getMinSerSizeProgram[j][h] == NULL) {
                        me->getMinSerSizeProgram[j][h] =
                            RTIXCdrInterpreter_generateTypePluginProgram(
                                tc,
                                NULL,
                                RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM,
                                &programProperty);
    
                        if (me->getMinSerSizeProgram[j][h] == NULL) {
                            programKind = RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM;
                            goto done;
                        }
                    }
                }
            }
            
            if (me->property.generateWithAllFields) {
                programProperty.onlyKey = RTI_XCDR_FALSE;

                if (mask & RTI_XCDR_SER_TO_KEY_PROGRAM) {
                    if (me->serializeToKeyProgram[i][j] == NULL) {
                        me->serializeToKeyProgram[i][j] =
                                RTIXCdrInterpreter_generateTypePluginProgram(
                                        tc,
                                        NULL,
                                        RTI_XCDR_SER_TO_KEY_PROGRAM,
                                        &programProperty);

                        if (me->serializeToKeyProgram[i][j] == NULL) {
                            programKind = RTI_XCDR_SER_TO_KEY_PROGRAM;
                            goto done;
                        }
                    }
                }
            }
        }
    }

    if ((mask & RTI_XCDR_SER_PROGRAM)
            || (mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM)) {
        if (me->property.generateWithOnlyKeyFields
                && me->property.generateV2Encapsulation) {
            programProperty.onlyKey = RTI_XCDR_TRUE;
            programProperty.v2Encapsulation = RTI_XCDR_TRUE;
            programProperty.littleEndianEncapsulation = RTI_XCDR_FALSE;
            programProperty.onlyKeyForKeyhash = RTI_XCDR_TRUE;

            /* The user cannot intercept this program. Enable all
             * optimizations
             */
            programProperty.optimizeEnum = RTI_XCDR_TRUE;
            programProperty.resolveAlias = RTI_XCDR_TRUE;
            programProperty.inlineStruct = RTI_XCDR_TRUE;
            /*
             * Do not set inlineSequence to true, because the user maybe using
             * sequences with discontiguous memory layout. We leave the decision
             * to the user.
             */

            
            if ((mask & RTI_XCDR_SER_PROGRAM) && 
                    me->serializeKeyForKeyhashProgram == NULL) {
                me->serializeKeyForKeyhashProgram = RTIXCdrInterpreter_generateTypePluginProgram(
                        tc,
                        NULL,
                        RTI_XCDR_SER_PROGRAM,
                        &programProperty);
                if (me->serializeKeyForKeyhashProgram == NULL) {
                    programKind = RTI_XCDR_SER_PROGRAM;
                    goto done;
                }
            }

            if ((mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) && 
                    me->getMaxKeySerSizeForKeyhashProgram == NULL) {
                me->getMaxKeySerSizeForKeyhashProgram = RTIXCdrInterpreter_generateTypePluginProgram(
                        tc,
                        NULL,
                        RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM,
                        &programProperty);
                if (me->getMaxKeySerSizeForKeyhashProgram == NULL) {
                    programKind = RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM;
                    goto done;
                }
            }
        }
    }

    /* Sample Interpreter Programs */
    if (mask & RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM) {
        if (me->initializeSampleProgram == NULL) {
            me->initializeSampleProgram = RTIXCdrInterpreter_generateSampleProgram(
                    tc,
                    NULL, /* dependentProgramList */
                    RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM,
                    &programProperty);
            if (me->initializeSampleProgram == NULL) {
                programKind = RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM;
                goto done;
            }
        }
    }

    if (mask & RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM) {
        if (me->allocatedMembersSampleProgram == NULL) {
            me->allocatedMembersSampleProgram =
                    RTIXCdrInterpreter_generateSampleProgram(
                            tc,
                            NULL, /* dependentProgramList */
                            RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM,
                            &programProperty);
            if (me->allocatedMembersSampleProgram == NULL) {
                programKind = RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM;
                goto done;
            }
        }
    }

    me->mask |= mask;

    ok = RTI_XCDR_TRUE;
  done:
    if (!ok) {
        RTIXCdrLog_logTwoStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss,
                RTIXCdrProgramKind_toStr(programKind),
                (tc->_name == NULL ? "anonymous" : tc->_name));
        RTIXCdrInterpreterPrograms_finalize(me);
    }

    return ok;
}

/*
 * Generates a set of programs defined by a mask if they don't exist yet. If
 * the programs already exist, this function has no effect.
 *
 * @param me Program set whose programs we generate.
 * @param mask Set of programs to generate.
 * @param tc Typecode to use for generating the programs. Note use this
 * parameter instead of using me->type. This is needed to be able to populate
 * programs for types that do not support them but whose children do.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreterPrograms_generateTopLevelPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrProgramMask mask,
        const RTIXCdrTypeCode *tc)
{
    RTIXCdrTCKind tcKind;
    RTIXCdrProgramMask adjustedMask = mask;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(me == NULL, goto done);
    RTIXCdrLog_testPrecondition(tc == NULL, goto done);

    tcKind = RTIXCdrTypeCode_getKind(tc);

    if (RTIXCdrTypeCode_isCollectionKind(tcKind)) {
        /*
         * Because serialization programs for collections are not
         * yet supported, if the typecode is a collection
         * we do not calculate the serialization programs for the
         * collection type, instead we calculate the programs for
         * the collection member type if that supports these programs.
         *
         * Serialization operations on collections are not allowed
         * but they are allowed in dynamic data objects bound to
         * members or sub-members of a collection.
         *
         * We now compute two masks:
         *   1) nonSampleProgramsMask: Mask with bits not supported
         *      by collections, we will assert these programs using
         *      the content's type.
         *   2) adjustedMask: Mask with sample-only bits, this are
         *      supported by collections.
         */
        RTIXCdrProgramMask nonSampleProgramsMask =
                (mask & ~RTI_XCDR_PROGRAM_MASK_SAMPLE);

        adjustedMask &= RTI_XCDR_PROGRAM_MASK_SAMPLE;

        if (nonSampleProgramsMask != RTI_XCDR_PROGRAM_MASK_NONE) {
            /*
             * We need to assert non-sample programs: use top-level
             * content type.
             */
            const RTIXCdrTypeCode *contentTypeCode;
            const RTIXCdrTypeCode *seqTypeCode = tc;

            while (RTIXCdrTypeCode_isCollectionKind(tcKind)) {
                contentTypeCode = RTIXCdrTypeCode_getContentType(seqTypeCode);

                contentTypeCode = RTIXCdrTypeCode_resolveAlias(contentTypeCode);
                tcKind = RTIXCdrTypeCode_getKind(contentTypeCode);
                seqTypeCode = contentTypeCode;
            }
            
            /* 
             * Only generate these programs for types that support
             * serialization programs (not primitives/strings/collections)
             */
            if (RTIXCdrTypeCode_isAggregationKind(tcKind)) {
                if (!RTIXCdrInterpreterPrograms_generate(
                        me,
                        nonSampleProgramsMask,
                        contentTypeCode)) {
                    RTIXCdrLog_logStr(
                            RTI_XCDR_LOG_EXCEPTION,
                            RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                            (contentTypeCode->_name == NULL
                                    ? "anonymous"
                                    : contentTypeCode->_name));
                    goto done;
                }
            }
        }
    }

    if (adjustedMask != RTI_XCDR_PROGRAM_MASK_NONE) {
        if (!RTIXCdrInterpreterPrograms_generate(me, adjustedMask, tc)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (tc->_name == NULL ? "anonymous" : tc->_name));
            goto done;
        }
    }

    ok = RTI_XCDR_TRUE;
done:

    return ok;
}

RTI_PRIVATE
RTIXCdrBoolean  RTIXCdrInterpreterPrograms_generateFromTopLevelPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrProgramMask mask)
{
    struct RTIXCdrInterpreterPrograms *topLevelPrograms = NULL;
    const RTIXCdrTypeCode *tc = NULL;
    int i = 0, j = 0, k = 0;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrProgramMask missingTopLevelProgramsMask; 

    RTIXCdrLog_testPrecondition(me == NULL, goto done);
    topLevelPrograms = me->topLevelPrograms;
    RTIXCdrLog_testPrecondition(topLevelPrograms == NULL, goto done);
    tc = me->type;
    RTIXCdrLog_testPrecondition(tc == NULL, goto done);

    missingTopLevelProgramsMask = ((topLevelPrograms->mask ^ mask) & mask); 

    if (missingTopLevelProgramsMask != RTI_XCDR_PROGRAM_MASK_NONE) {

        /*
         * Top-level programs' mask does not contain all of the requested
         * programs: we generate them.
         */
        if (!RTIXCdrInterpreterPrograms_generateTopLevelPrograms(
                topLevelPrograms,
                missingTopLevelProgramsMask,
                topLevelPrograms->type)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (topLevelPrograms->type->_name == NULL
                            ? "anonymous"
                            : topLevelPrograms->type->_name));
            goto done;
        }
    }

    /*
     * At this point, the top-level programs contains all of the requested
     * programs: we can now retrieve the dependent programs for our program
     * set.
     */
    if (mask & RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM) {
        me->initializeSampleProgram =
                RTIXCdrDependentProgramList_findProgram(
                        topLevelPrograms->initializeSampleProgram->dependentProgramList,
                        tc,
                        RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM);
    }

    if (mask & RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM) {
        me->allocatedMembersSampleProgram =
                RTIXCdrDependentProgramList_findProgram(
                        topLevelPrograms->allocatedMembersSampleProgram->dependentProgramList,
                        tc,
                        RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM);
    }

    if ((mask & RTI_XCDR_SER_PROGRAM) || (mask & RTI_XCDR_DESER_PROGRAM)) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                for (k = 0; k < 2; k++) {
                    if (mask & RTI_XCDR_SER_PROGRAM) {
                        if (topLevelPrograms->serializeProgram[i][j][k] != NULL) {
                            me->serializeProgram[i][j][k] =
                                    RTIXCdrDependentProgramList_findProgram(
                                            topLevelPrograms->serializeProgram[i][j][k]->dependentProgramList,
                                            tc,
                                            RTI_XCDR_SER_PROGRAM);
                        }
                    }

                    if (mask & RTI_XCDR_DESER_PROGRAM) {
                        if (topLevelPrograms->deserializeProgram[i][j][k] != NULL) {
                            me->deserializeProgram[i][j][k] =
                                    RTIXCdrDependentProgramList_findProgram(
                                            topLevelPrograms->deserializeProgram[i][j][k]->dependentProgramList,
                                            tc,
                                            RTI_XCDR_DESER_PROGRAM);
                        }
                    }
                }
            }
        }
    }

    if ((mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM)
            || (mask & RTI_XCDR_GET_SER_SIZE_PROGRAM)) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                if (mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) {
                    if (topLevelPrograms->getMaxSerSizeProgram[i][j] != NULL) {
                        me->getMaxSerSizeProgram[i][j] =
                                RTIXCdrDependentProgramList_findProgram(
                                        topLevelPrograms->getMaxSerSizeProgram[i][j]->dependentProgramList,
                                        tc,
                                        RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM);
                    }
                }

                if (mask & RTI_XCDR_GET_SER_SIZE_PROGRAM) {
                    if (topLevelPrograms->getSerSizeProgram[i][j] != NULL) {
                        me->getSerSizeProgram[i][j] =
                                RTIXCdrDependentProgramList_findProgram(
                                        topLevelPrograms->getSerSizeProgram[i][j]->dependentProgramList,
                                        tc,
                                        RTI_XCDR_GET_SER_SIZE_PROGRAM);
                    }
                }
            }
        }
    }

    if (mask & RTI_XCDR_SER_PROGRAM) {
        if (topLevelPrograms->serializeKeyForKeyhashProgram != NULL) {
            me->serializeKeyForKeyhashProgram =
                RTIXCdrDependentProgramList_findProgram(
                        topLevelPrograms->serializeKeyForKeyhashProgram->dependentProgramList,
                        tc,
                        RTI_XCDR_SER_PROGRAM);
        }
    }

    if (mask & RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM) {
        if (topLevelPrograms->getMaxKeySerSizeForKeyhashProgram != NULL) {
            me->getMaxKeySerSizeForKeyhashProgram =
                RTIXCdrDependentProgramList_findProgram(
                        topLevelPrograms->getMaxKeySerSizeForKeyhashProgram->dependentProgramList,
                        tc,
                        RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM);
        }
    }

    me->mask |= mask;
    ok = RTI_XCDR_TRUE;
done:

    return ok;
}

void RTIXCdrInterpreterPrograms_delete(struct RTIXCdrInterpreterPrograms *me)
{
    RTIXCdrLog_testPrecondition(me == NULL, return);

    RTIXCdrInterpreterPrograms_finalize(me);
    RTIXCdrHeap_freeStruct(me);
}

RTIXCdrBoolean RTIXCdrInterpreterPrograms_assertPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrProgramMask mask)
{
    const RTIXCdrTypeCode *tc = NULL;
    RTIXCdrProgramMask missingProgramsMask;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);

    missingProgramsMask = ((me->mask ^ mask) & mask); 

    if (missingProgramsMask == RTI_XCDR_PROGRAM_MASK_NONE) {
        /* We already have all of the requested programs. */
        return RTI_XCDR_TRUE;
    }

    tc = me->type;

    if (me == me->topLevelPrograms) {
        if (!RTIXCdrInterpreterPrograms_generateTopLevelPrograms(
                me, 
                missingProgramsMask, 
                tc)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (tc->_name == NULL ? "anonymous" : tc->_name));
            return RTI_XCDR_FALSE;
        }
    } else {
        if (!RTIXCdrInterpreterPrograms_generateFromTopLevelPrograms(
                me,
                missingProgramsMask)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (tc->_name == NULL ? "anonymous" : tc->_name));
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrInterpreterPrograms_initializeWithParams(
        struct RTIXCdrInterpreterPrograms *me,
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterPrograms *topLevelPrograms,
        const RTIXCdrTypeCode *topLevelType,
        const struct RTIXCdrInterpreterProgramsGenProperty *property,
        RTIXCdrProgramMask mask)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(type == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(topLevelPrograms == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(topLevelType == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    RTIXCdrMemory_zero(me, sizeof(struct RTIXCdrInterpreterPrograms));
    me->topLevelType = (RTIXCdrTypeCode *) topLevelType;
    me->type = (RTIXCdrTypeCode *) type;
    me->property = *property;
    me->topLevelPrograms = (struct RTIXCdrInterpreterPrograms *) topLevelPrograms;

    if (property->xTypesComplianceMask != 0) {
        me->globalXTypesComplianceMask = property->xTypesComplianceMask;
    } else {
        me->globalXTypesComplianceMask =
                RTIXCdrInterpreter_getGlobalXtypeComplianceMask();
    }

    if (me == topLevelPrograms) {
        /* This program set is the owner of the programs: generate them. */
        if (!RTIXCdrInterpreterPrograms_generateTopLevelPrograms(
                me, 
                mask, 
                type)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (type->_name == NULL ? "anonymous" : type->_name));
            return RTI_XCDR_FALSE;
        }
    } else {
        /* This program set is now the owner of the programs: retrieve them from
         * the top-level program set.
         */
        if (!RTIXCdrInterpreterPrograms_generateFromTopLevelPrograms(
                me, 
                mask)) {
            RTIXCdrLog_logStr(
                    RTI_XCDR_LOG_EXCEPTION,
                    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
                    (type->_name == NULL ? "anonymous" : type->_name));
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrInterpreterPrograms_initialize(
        struct RTIXCdrInterpreterPrograms *me,
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterProgramsGenProperty *property,
        RTIXCdrProgramMask mask)
{
    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(type == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isCdrRepresentation(type)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
                "typecode is in CDR representation");
        return RTI_XCDR_FALSE;
    }

    return RTIXCdrInterpreterPrograms_initializeWithParams(
            me,
            type, /* type */
            me, /* topLevelPrograms */
            type, /* topLevelType */
            property,
            mask);
}

RTIXCdrBoolean RTIXCdrInterpreterPrograms_initializeFromPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterPrograms *parentPrograms,
        RTIXCdrProgramMask mask)
{
    struct RTIXCdrInterpreterPrograms *topLevelPrograms = NULL;

    RTIXCdrLog_testPrecondition(me == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(type == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(parentPrograms == NULL, return RTI_XCDR_FALSE);

    topLevelPrograms = parentPrograms->topLevelPrograms;
    RTIXCdrLog_testPrecondition(topLevelPrograms == NULL, return RTI_XCDR_FALSE);

    return RTIXCdrInterpreterPrograms_initializeWithParams(
            me,
            type,
            topLevelPrograms,
            topLevelPrograms->type,
            &topLevelPrograms->property,
            mask);
}

struct RTIXCdrInterpreterPrograms *RTIXCdrInterpreterPrograms_new(
        const RTIXCdrTypeCode *tc,
        const struct RTIXCdrInterpreterProgramsGenProperty *property,
        RTIXCdrProgramMask mask)
{
    struct RTIXCdrInterpreterPrograms *me = NULL, *retMe = NULL;

    RTIXCdrLog_testPrecondition(tc == NULL, goto done);
    RTIXCdrLog_testPrecondition(property == NULL, goto done);

    RTIXCdrHeap_allocateStruct(&me, struct RTIXCdrInterpreterPrograms);
    if (me == NULL) {
        RTIXCdrLog_logLong(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
                sizeof(struct RTIXCdrInterpreterPrograms));
        goto done;
    }

    if (!RTIXCdrInterpreterPrograms_initialize(
            me,
            tc,
            property,
            mask)) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
                "programs");
        goto done;
    }

    /* Only set the mask if we have actually generated the programs. */
    me->mask = mask;

    retMe = me;
done:
    if (retMe != me) {
        RTIXCdrInterpreterPrograms_delete(me);
    }

    return retMe;
}
