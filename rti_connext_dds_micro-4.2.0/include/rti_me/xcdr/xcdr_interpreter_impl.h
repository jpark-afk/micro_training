/*
(c) Copyright, Real-Time Innovations, 2018-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_interpreter_impl_h
#define xcdr_interpreter_impl_h


#ifdef __cplusplus
    extern "C" {
#endif

#define RTIXCdrInterpreter_serializeSample( \
    stream, \
    sample, \
    tc, \
    program, \
    context) \
    (((program)->isFastSerializationSupported && !(stream)->_needByteSwap)? \
        RTIXCdrInterpreter_fastSerializeSample( \
                (stream), \
                (sample), \
                (tc), \
                (program), \
                (context)): \
        RTIXCdrInterpreter_fullSerializeSample( \
                (stream), \
                (sample), \
                (tc), \
                (program), \
                (context)))

#define RTIXCdrInterpreter_isInitializationNeededOnDeserialization(program__) \
    ((program__)->extKind != RTI_XCDR_FINAL_EXTENSIBILITY || \
            (program__)->typeCode->_sampleAccessInfo->languageBinding == \
                    RTI_XCDR_TYPE_BINDING_DYN_DATA || \
            (program__)->typeCode->_sampleAccessInfo->languageBinding == \
                    RTI_XCDR_TYPE_BINDING_SQL_FILTER)

#define RTIXCdrInterpreter_deserializeSample( \
    sample, \
    stream, \
    tc, \
    program, \
    sampleAssignability, \
    context) \
    (((program)->isFastSerializationSupported && !(stream)->_needByteSwap)? \
        RTIXCdrInterpreter_fastDeserializeSample( \
                (sample), \
                (stream), \
                (tc), \
                (program), \
                (sampleAssignability), \
                (context)): \
        RTIXCdrInterpreter_fullDeserializeSample( \
                (sample), \
                (stream), \
                (tc), \
                (program), \
                (sampleAssignability), \
                (context)))

#define RTIXCdrInterpreter_serializedSampleToKey \
    RTIXCdrInterpreter_deserializeSample

#define RTIXCdrInterpreter_fastSerializedSampleToKey \
    RTIXCdrInterpreter_fastDeserializeSample

#define RTIXCdrInterpreter_fullSerializedSampleToKey \
    RTIXCdrInterpreter_fullDeserializeSample

#define RTIXCdrInterpreterProgramsGenProperty_toTypePluginProperty( \
        self, \
        tpProperty) \
    (tpProperty)->resolveAlias = (self)->resolveAlias; \
    (tpProperty)->optimizeEnum = (self)->optimizeEnum; \
    (tpProperty)->inlineStruct = (self)->inlineStruct; \
    (tpProperty)->inlineSequence = (self)->inlineSequence; \
    (tpProperty)->forceDependentPrograms = (self)->forceDependentPrograms; \
    (tpProperty)->externalReferenceSize = (self)->externalReferenceSize; \
    (tpProperty)->getExternalRefPointerFcn = (self)->getExternalRefPointerFcn; \
    (tpProperty)->serializeSentinelOnBase = (self)->serializeSentinelOnBase; \
    (tpProperty)->disableMustUnderstandOnSentinel = (self)->disableMustUnderstandOnSentinel; \
    (tpProperty)->unboundedSize = (self)->unboundedSize

struct RTIXCdrInterpreterPrograms {
    /*
     * Indicates who is the top-level type of the program. Used by higher level
     * APIs.
     */
    RTIXCdrTypeCode *topLevelType;

    /*
     * Indicates the type of the program set.
     */
    RTIXCdrTypeCode *type;

    /*
     * Properties used for generating the programs. We store them so we can
     * assert programs incrementally (instead of setting up all of them from
     * the beginning).
     */
    RTIXCdrInterpreterProgramsGenProperty property;

    /*
     * Mask for the currently generated programs.
     */
    RTIXCdrProgramMask mask;

    /*
     * This is the owner of the programs and the responsible of cleaning-up
     * resources. If we are the owner of the programs, this will point to
     * ourselves.
     */
    struct RTIXCdrInterpreterPrograms *topLevelPrograms;

    /* First dimension: LE (1) or BE (0)
     * Second dimension: V2 (1) or V1 (0)
     * Third dimension: OnlyKey (1) AllFields (0)
     */
    RTIXCdrProgram *serializeProgram[2][2][2];
    RTIXCdrProgram *deserializeProgram[2][2][2];
    RTIXCdrProgram *skipProgram[2][2][2];

    /* First dimension: V2 (1) or V1 (0)
     * Second dimension: OnlyKey (1) AllFields (0)
     */
    RTIXCdrProgram *getSerSizeProgram[2][2];
    RTIXCdrProgram *getMaxSerSizeProgram[2][2];
    RTIXCdrProgram *getMinSerSizeProgram[2][2];

    /* First dimension: LE (1) or BE (0)
     * Second dimension: V2 (1) or V1 (0)
     */
    RTIXCdrProgram *serializeToKeyProgram[2][2];

    /* This program is created when: V2, and OnlyKey are set to TRUE and LE is
     * set to FALSE.
     *
     * The program is used to serialize a key for keyhash generation purposes
     * with V2 data representation.
     *
     * Values are serialized in the ascending order of their member IDs.
     * For mutable types, the key members are serialized without any parameter.
     * For appendable types the DHEADER is not included.
     */
    RTIXCdrProgram *serializeKeyForKeyhashProgram;

    /* This program is created when: V2, and OnlyKey are set to TRUE and LE is
     * set to FALSE.
     *
     * The program is used to get the maximum size of a serialized key 
     * for keyhash generation purposes with V2 data representation.
     */
    RTIXCdrProgram *getMaxKeySerSizeForKeyhashProgram;

    RTIXCdrProgram *initializeSampleProgram;
    RTIXCdrProgram *allocatedMembersSampleProgram;
};


#define RTIXCdrInterpreterPrograms_INITIALIZER \
{ \
    NULL, /* topLevelType */\
    NULL, /* type */\
    RTIXCdrInterpreterProgramsGenProperty_INITIALIZER, /* property */\
    RTI_XCDR_PROGRAM_MASK_NONE, /* mask */\
    NULL, /* topLevelPrograms */\
    {{{NULL, NULL}, {NULL, NULL}}, {{NULL, NULL}, {NULL, NULL}}}, /* serializeProgram */\
    {{{NULL, NULL}, {NULL, NULL}}, {{NULL, NULL}, {NULL, NULL}}}, /* deserializeProgram */\
    {{{NULL, NULL}, {NULL, NULL}}, {{NULL, NULL}, {NULL, NULL}}}, /* skipProgram */\
    {{NULL, NULL}, {NULL, NULL}}, /* getSerSizeProgram */\
    {{NULL, NULL}, {NULL, NULL}}, /* getMaxSerSizeProgram */\
    {{NULL, NULL}, {NULL, NULL}}, /* getMinSerSizeProgram */\
    {{NULL, NULL}, {NULL, NULL}}, /* serializeToKeyProgram */\
    NULL, /* serializeKeyForKeyhashProgram */\
    NULL, /* getMaxKeySerSizeForKeyhashProgram */ \
    NULL, /* initializeSampleProgram */\
    NULL  /* finalizeSampleProgram */\
}

#define RTIXCdrInterpreterPrograms_getSerProgram(me, \
        littleEndianEncapsulation, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->serializeProgram[(littleEndianEncapsulation)?1:0] \
        [(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])

#define RTIXCdrInterpreterPrograms_getSerProgramForKeyhash(me, \
        v2Encapsulation) \
    ((v2Encapsulation)? \
        (me)->serializeKeyForKeyhashProgram: \
        (me)->serializeProgram[0][0][1])

#define RTIXCdrInterpreterPrograms_getDeserProgram(me, \
        littleEndianEncapsulation, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->deserializeProgram[(littleEndianEncapsulation)?1:0] \
        [(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])

#define RTIXCdrInterpreterPrograms_getSerToKeyProgram(me, \
        littleEndianEncapsulation, \
        v2Encapsulation) \
    ((me)->serializeToKeyProgram[(littleEndianEncapsulation)?1:0] \
        [(v2Encapsulation)?1:0])

#define RTIXCdrInterpreterPrograms_getSkipProgram(me, \
        littleEndianEncapsulation, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->skipProgram[(littleEndianEncapsulation)?1:0] \
        [(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])

#define RTIXCdrInterpreterPrograms_getSerSizeProgram(me, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->getSerSizeProgram[(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])

#define RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(me, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->getMaxSerSizeProgram[(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])
        
#define RTIXCdrInterpreterPrograms_getMaxSerSizeProgramForKeyhash(me, \
        v2Encapsulation) \
    ((v2Encapsulation)? \
        (me)->getMaxKeySerSizeForKeyhashProgram: \
        (me)->getMaxSerSizeProgram[0][1])

#define RTIXCdrInterpreterPrograms_getMinSerSizeProgram(me, \
        v2Encapsulation, \
        onlyKey) \
    ((me)->getMinSerSizeProgram[(v2Encapsulation)?1:0] \
        [(onlyKey)?1:0])

#define RTIXCdrInterpreterPrograms_getInitializeSampleProgram(me) \
    ((me)->initializeSampleProgram)

#define RTIXCdrInterpreterPrograms_getAllocatedMembersSampleProgram(me) \
    ((me)->allocatedMembersSampleProgram)

/**
 * @brief Executes the initialize program on a given data sample, with the
 * option to run specific instructions.
 *
 * @param sample The data sample to initialize (fully or partially).
 * @param tc The type of the data sample.
 * @param program The initialization program.
 * @param property Properties that configure the initialization.
 * @param startInstructionIn The first instruction to execute
 * @param instructionCountIn  The number of instructions to execute. When this
 * number is 1, the function assumes that the member being initialized will be
 * allocated. For example, when > 1 optional members are set to NULL; when it is
 * 1 and the member is optional, this function allocates it.
 * @param context Context of the program execution
 * @return True if successful.
 */
extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrSampleInterpreter_initializeSampleWInstruction(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const RTIXCdrInitializeSampleProperty *property,
        RTIXCdrUnsignedLong startInstructionIn,
        RTIXCdrUnsignedLong instructionCountIn,
        RTIXCdrSampleProgramContext *context);

/**
 * @brief Executes the full initialize program on a given data sample.
 */
#define RTIXCdrSampleInterpreter_initializeSample( \
        sample, \
        tc, \
        program, \
        property, \
        context) \
    RTIXCdrSampleInterpreter_initializeSampleWInstruction( \
            (sample), \
            (tc), \
            (program), \
            (property), \
            RTI_XCDR_INSTRUCTION_INVALID, \
            RTI_XCDR_INSTRUCTION_INVALID, \
            (context))

#define RTIXCdrSampleAssignabilityProperty_setFromGlobalComplianceMask(property__) \
{ \
        RTIXCdrXTypesComplianceMask complianceMask__ = \
                RTIXCdrInterpreter_getGlobalXtypeComplianceMask(); \
        (property__)->acceptUnknownEnumValue = RTIXCdrXTypesComplianceMask_isBitSet( \
                complianceMask__, RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_ENUM_VALUE_BIT); \
        RTIXCdrXTypesComplianceMask_assignUnknownUnionDiscriminatorOption( \
                &(property__)->acceptUnknownUnionDiscriminator, complianceMask__); \
}

#define RTIXCdrXTypesComplianceMask_assignUnknownUnionDiscriminatorOption(value__, mask__) \
    if (RTIXCdrXTypesComplianceMask_isBitSet( \
            (mask__), RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_DISCRIMINATOR_BIT)) { \
        if (RTIXCdrXTypesComplianceMask_isBitSet( \
                (mask__), RTI_XCDR_XTYPES_SELECT_DEFAULT_DISCRIMINATOR_BIT)) { \
                *(value__) = RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR_AND_SELECT_DEFAULT; \
        } else { \
            *(value__) = RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR; \
        } \
    } else { \
        *(value__) = RTI_XCDR_REJECT_UNKNOWN_DISCRIMINATOR; \
    }

#define RTIXCdrXTypesComplianceMask_isBitSet(mask__, bit__) \
    (((mask__) & (bit__)) != 0)

#define RTIXCdrXTypesComplianceMask_setBit(mask__, bit__) \
    ((mask__) |= (bit__))

#define RTIXCdrXTypesComplianceMask_unsetBit(mask__, bit__) \
    ((mask__) &= ~((RTIXCdrXTypesComplianceMask) bit__))

#define RTIXcdr_static_cast(type_,in_ptr_) \
    (type_)((void*)(in_ptr_))

#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_interpreter_impl_h */
