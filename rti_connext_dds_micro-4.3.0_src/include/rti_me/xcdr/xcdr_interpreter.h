/*
(c) Copyright, Real-Time Innovations, 2017-2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_interpreter_h
#define xcdr_interpreter_h


#include "xcdr/xcdr_dll.h"
#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_stream.h"
#include "xcdr/xcdr_typeCode.h"


#ifdef __cplusplus
    extern "C" {
#endif

typedef RTIXCdrOctet RTIXCdrRefMemberKind;
#define RTI_XCDR_INTERPRETER_VALUE_MEMBER 0
#define RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER 1
#define RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER 2

/**
 * @brief Compliance mask for the XTypes specification.
 *
 * The possible bits are:
 * - RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT
 * - RTI_XCDR_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT
 * - RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT
 * - RTI_XCDR_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT
 * - RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_ENUM_VALUE_BIT
 * - RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_DISCRIMINATOR_BIT
 * - RTI_XCDR_XTYPES_SELECT_DEFAULT_DISCRIMINATOR_BIT
 * - RTI_XCDR_XTYPES_SENTINEL_IN_EMPTY_UNION_BIT
 * - RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT
 * - RTI_XCDR_XTYPES_JAVA_KEYHASH_750_BIT
 * - RTI_XCDR_XTYPES_PYTHON_INT8_730_BIT
 * - RTI_XCDR_XTYPES_MUST_UNDERSTAND_IN_KEY_AND_DISCRIMINATOR_BIT
 */
typedef RTIXCdrUnsignedLong RTIXCdrXTypesComplianceMask;

/* How the parameters are organized in the following structures is important
 * due to resource consumption reasons.
 *
 * Try to not create padding between fields.
 */
typedef struct RTIXCdrCommonInsParameters {
    struct RTIXCdrTypeCode *memberTc;
    struct RTIXCdrTypeCode *seqElementTc;
    struct RTIXCdrTypeCodeMember *tcMemberInfo;
    RTIXCdrUnsignedLong count;
    struct RTIXCdrMemberAccessInfo memberAccessInfo;
    /* Indicates if the member is
     * optional: RTI_XCDR_INTERPRETER_OPTIONAL_MEMBER
     * external: RTI_XCDR_INTERPRETER_EXTERNAL_MEMBER
     * value: RTI_XCDR_INTERPRETER_VALUE_MEMBER
     */
    RTIXCdrRefMemberKind refMemberKind;
    RTIXCdrBoolean useGetMemberValue;
    /*
     * Set to RTI_XCDR_TRUE only when the instruction processes
     * an array of sequences of non-primitive elements.
     */
    RTIXCdrBoolean addSeqDHeader;
} RTIXCdrCommonInsParameters;

typedef struct RTIXCdrStringInsParameters {
    RTIXCdrCommonInsParameters parent;
    RTIXCdrUnsignedLong charMaxCount;
    RTIXCdrLong charAlignment;
    RTIXCdrOctet charSize;
} RTIXCdrStringInsParameters;

typedef struct RTIXCdrPrimitiveInsParameters {
    RTIXCdrCommonInsParameters parent;
    /* When set the value of an enum must be check on ser/deser,
     * enumTc contains the enumTc
     */
    struct RTIXCdrTypeCode *enumTc;
    RTIXCdrUnsignedLong primitiveByteCount;
    RTIXCdrTCKind primitiveKind;
    RTIXCdrLong primitiveAlignment;
    RTIXCdrBoolean checkRange;
    RTIXCdrBoolean mustAlign;
    RTIXCdrOctet primitiveSize;
    RTIXCdrOctet origPrimitiveSize;
} RTIXCdrPrimitiveInsParameters;

struct RTIXCdrProgram;

typedef struct RTIXCdrComplexInsParameters {
    RTIXCdrCommonInsParameters parent;
    RTIXCdrTypePlugin *typePlugin;
    struct RTIXCdrProgram *program;
    RTIXCdrBoolean baseClass;
} RTIXCdrComplexInsParameters;

typedef struct RTIXCdrMemberHeaderInsParameters {
    RTIXCdrCommonInsParameters parent;
    RTIXCdrExtendedBoolean v1ExtendedId;
    /* This field will contain the LC (length code) value for a member in a
     * mutable type
     */
    RTIXCdrOctet v2LC;
    /* This field is set to RTI_XCDR_TRUE if the V1 optional member value 
     * associated with this header has nested member values that contain 
     * headers.
     *
     * For example:
     *
     *   struct MyNestedStructWithOptional {
     *       @optional long m1;
     *   };
     *
     *   struct MyNestedStruct {
     *       long m1;
     *   };
     *
     *   @mutable
     *   struct MyStructMutable {
             long m1;
     *   };
     *
     *   struct MyStruct {
     *       @optional MyNestedStruct m1;
     *       @optional MyNestedStructWithOptional m2;
     *       @optional MyStructMutable m3;
     *   };
     *
     * In this case:
     * - The header for m1 in MyStruct will have this field set to
     * RTI_XCDR_FALSE because the member value associated with MyStruct.m1 
     * does not have any header.
     * - The header for m2 in MyStruct will have this field set to
     * RTI_XCDR_TRUE because the member value associated with MyStruct.m2
     * has a header (MyNestedStructWithOptional.m1).
     * - The header for m3 in MyStruct will have this field set to
     * RTI_XCDR_TRUE because the member value associated with MyStruct.m3
     * has a header (MyStructMutable.m1 because MyStructMutable is mutable).
     */
    RTIXCdrBoolean hasV1NestedMemberHeaders;
} RTIXCdrMemberHeaderInsParameters;

typedef struct RTIXCdrPrimitiveSampleInsParameters {
    RTIXCdrCommonInsParameters parent;
    RTIXCdrTCKind primitiveKind;
    RTIXCdrLong primitiveAlignment;
    /* The size of the primtive member */
    RTIXCdrOctet primitiveSize;
} RTIXCdrPrimitiveSampleInsParameters;

typedef struct RTIXCdrComplexSampleInsParameters {
    RTIXCdrCommonInsParameters parent;
    struct RTIXCdrProgram *program;
    /*
     * The possible different sizes of the array element type.
     *
     * Note: this field has a value if this instruction is part of an array
     * program.
     */
    RTIXCdrUnsignedLong arrayElementTypeSize[RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES];

    /*
     * The total size of the array.
     *
     * Note: this field has a value if this instruction has opcode
     * RTI_XCDR_INITIALIZE_ARRAY_OPCODE and is part of a struct or union program
     * for a non-contiguous memory binding (such as C). It is used to allocate
     * optional arrays when needed.
     */
    RTIXCdrUnsignedLong arrayTypeTotalSize;
} RTIXCdrComplexSampleInsParameters;

typedef union RTIXCdrInsParameters {
    RTIXCdrStringInsParameters strParams;
    RTIXCdrPrimitiveInsParameters primitiveParams;
    RTIXCdrComplexInsParameters complexParams;
    RTIXCdrMemberHeaderInsParameters memberHeaderParams;
    /* Used for the Sample Interpreter */
    RTIXCdrPrimitiveSampleInsParameters primitiveSampleParams;
    RTIXCdrComplexSampleInsParameters complexSampleParams;
} RTIXCdrInsParameters;

/* 
 * Opcode structure:
 * 
 * Bit 8-4 (object to which instruction applies):
 * PrimSeq: 0xC0
 * StringSeq: 0xA0
 * WStringSeq: 0x90
 * ComplexSeq: 0x88
 * Prim: 0x40
 * String: 0x20
 * WString: 0x10
 * Complex: 0x08
 * DHEADER: 0x18
 * COLLECTION_DHEADER: 0x48
 * Member Header: 0x28
 * SENTINEL: 0x30
 * Sequences: 0x80 
 * Arrays: 0x58 
 * 
 * Bit 3-1 (operation):
 * Ser: 0x00
 * Deser: 0x01
 * Skip: 0x02
 * Init: 0x03
 * Allocated Members: 0x04
 * FinishSer: 0x05
 *
 */

#define RTI_XCDR_OPCODE_COUNT 55
#define RTI_XCDR_INVALID_OPCODE 0

#define RTI_XCDR_SER_PRIMITIVE_OPCODE 0x40
#define RTI_XCDR_DESER_PRIMITIVE_OPCODE 0x41
#define RTI_XCDR_SKIP_PRIMITIVE_OPCODE 0x42
#define RTI_XCDR_INITIALIZE_PRIMITIVE_OPCODE 0x43
#define RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE 0x44
#define RTI_XCDR_FINALIZE_PRIMITIVE_OPCODE RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE
#define RTI_XCDR_COPY_PRIMITIVE_OPCODE RTI_XCDR_ALLOCATED_PRIMITIVE_OPCODE

#define RTI_XCDR_SER_STRING_OPCODE 0x20
#define RTI_XCDR_DESER_STRING_OPCODE 0x21
#define RTI_XCDR_SKIP_STRING_OPCODE 0x22
#define RTI_XCDR_INITIALIZE_STRING_OPCODE 0x23
#define RTI_XCDR_ALLOCATED_STRING_OPCODE 0x24
#define RTI_XCDR_FINALIZE_STRING_OPCODE RTI_XCDR_ALLOCATED_STRING_OPCODE
#define RTI_XCDR_COPY_STRING_OPCODE RTI_XCDR_ALLOCATED_STRING_OPCODE

#define RTI_XCDR_SER_WSTRING_OPCODE 0x10
#define RTI_XCDR_DESER_WSTRING_OPCODE 0x11
#define RTI_XCDR_SKIP_WSTRING_OPCODE 0x12
#define RTI_XCDR_INITIALIZE_WSTRING_OPCODE 0x13
#define RTI_XCDR_ALLOCATED_WSTRING_OPCODE 0x14
#define RTI_XCDR_FINALIZE_WSTRING_OPCODE RTI_XCDR_ALLOCATED_WSTRING_OPCODE
#define RTI_XCDR_COPY_WSTRING_OPCODE RTI_XCDR_ALLOCATED_WSTRING_OPCODE

#define RTI_XCDR_SER_COMPLEX_OPCODE 0x08
#define RTI_XCDR_DESER_COMPLEX_OPCODE 0x09
#define RTI_XCDR_SKIP_COMPLEX_OPCODE 0x0A
#define RTI_XCDR_INITIALIZE_COMPLEX_OPCODE 0x0B
#define RTI_XCDR_ALLOCATED_COMPLEX_OPCODE 0x0C
#define RTI_XCDR_FINALIZE_COMPLEX_OPCODE RTI_XCDR_ALLOCATED_COMPLEX_OPCODE
#define RTI_XCDR_COPY_COMPLEX_OPCODE RTI_XCDR_ALLOCATED_COMPLEX_OPCODE

#define RTI_XCDR_SER_PRIMITIVE_SEQ_OPCODE 0xC0
#define RTI_XCDR_DESER_PRIMITIVE_SEQ_OPCODE 0xC1
#define RTI_XCDR_SKIP_PRIMITIVE_SEQ_OPCODE 0xC2
#define RTI_XCDR_INITIALIZE_PRIMITIVE_SEQ_OPCODE 0xC3
#define RTI_XCDR_ALLOCATED_PRIMITIVE_SEQ_OPCODE 0xC4
#define RTI_XCDR_FINALIZE_PRIMITIVE_SEQ_OPCODE RTI_XCDR_ALLOCATED_PRIMITIVE_SEQ_OPCODE
#define RTI_XCDR_COPY_PRIMITIVE_SEQ_OPCODE RTI_XCDR_ALLOCATED_PRIMITIVE_SEQ_OPCODE

#define RTI_XCDR_SER_STRING_SEQ_OPCODE 0xA0
#define RTI_XCDR_DESER_STRING_SEQ_OPCODE 0xA1
#define RTI_XCDR_SKIP_STRING_SEQ_OPCODE 0xA2
#define RTI_XCDR_INITIALIZE_STRING_SEQ_OPCODE 0xA3
#define RTI_XCDR_ALLOCATED_STRING_SEQ_OPCODE 0xA4
#define RTI_XCDR_FINALIZE_STRING_SEQ_OPCODE RTI_XCDR_ALLOCATED_STRING_SEQ_OPCODE
#define RTI_XCDR_COPY_STRING_SEQ_OPCODE RTI_XCDR_ALLOCATED_STRING_SEQ_OPCODE

#define RTI_XCDR_SER_WSTRING_SEQ_OPCODE 0x90
#define RTI_XCDR_DESER_WSTRING_SEQ_OPCODE 0x91
#define RTI_XCDR_SKIP_WSTRING_SEQ_OPCODE 0x92
#define RTI_XCDR_INITIALIZE_WSTRING_SEQ_OPCODE 0x93
#define RTI_XCDR_ALLOCATED_WSTRING_SEQ_OPCODE 0x94
#define RTI_XCDR_FINALIZE_WSTRING_SEQ_OPCODE RTI_XCDR_ALLOCATED_WSTRING_SEQ_OPCODE
#define RTI_XCDR_COPY_WSTRING_SEQ_OPCODE RTI_XCDR_ALLOCATED_WSTRING_SEQ_OPCODE

#define RTI_XCDR_SER_COMPLEX_SEQ_OPCODE 0x88
#define RTI_XCDR_DESER_COMPLEX_SEQ_OPCODE 0x89
#define RTI_XCDR_SKIP_COMPLEX_SEQ_OPCODE 0x8A
#define RTI_XCDR_INITIALIZE_COMPLEX_SEQ_OPCODE 0x8B
#define RTI_XCDR_ALLOCATED_COMPLEX_SEQ_OPCODE 0x8C
#define RTI_XCDR_FINALIZE_COMPLEX_SEQ_OPCODE RTI_XCDR_ALLOCATED_COMPLEX_SEQ_OPCODE
#define RTI_XCDR_COPY_COMPLEX_SEQ_OPCODE RTI_XCDR_ALLOCATED_COMPLEX_SEQ_OPCODE

#define RTI_XCDR_SER_DHEADER_OPCODE 0x18
#define RTI_XCDR_DESER_DHEADER_OPCODE 0x19
#define RTI_XCDR_SKIP_DHEADER_OPCODE 0x1A

#define RTI_XCDR_SER_SENTINEL_HEADER_OPCODE 0x30
#define RTI_XCDR_SKIP_SENTINEL_HEADER_OPCODE 0x32

#define RTI_XCDR_SER_MEMBER_HEADER_OPCODE 0x28
#define RTI_XCDR_DESER_MEMBER_HEADER_OPCODE 0x29
#define RTI_XCDR_SKIP_MEMBER_HEADER_OPCODE 0x2A

#define RTI_XCDR_SER_COLLECTION_DHEADER_OPCODE 0x48
#define RTI_XCDR_DESER_COLLECTION_DHEADER_OPCODE 0x49
#define RTI_XCDR_SKIP_COLLECTION_DHEADER_OPCODE 0x4A

#define RTI_XCDR_INITIALIZE_SEQ_OPCODE 0x83
#define RTI_XCDR_ALLOCATED_SEQ_OPCODE 0x84
#define RTI_XCDR_FINALIZE_SEQ_OPCODE RTI_XCDR_ALLOCATED_SEQ_OPCODE
#define RTI_XCDR_COPY_SEQ_OPCODE RTI_XCDR_ALLOCATED_SEQ_OPCODE

#define RTI_XCDR_INITIALIZE_ARRAY_OPCODE 0x5B
#define RTI_XCDR_ALLOCATED_ARRAY_OPCODE 0x5C
#define RTI_XCDR_FINALIZE_ARRAY_OPCODE RTI_XCDR_ALLOCATED_ARRAY_OPCODE
#define RTI_XCDR_COPY_ARRAY_OPCODE RTI_XCDR_ALLOCATED_ARRAY_OPCODE

#define RTI_XCDR_INSTRUCTION_INVALID RTIXCdrUnsignedLong_MAX

typedef struct RTIXCdrInstruction {
    RTIXCdrOctet opcode;
    RTIXCdrInsParameters params;
} RTIXCdrInstruction;


struct RTIXCdrInlineList;


typedef enum RTIXCdrTypeProgramKind {
    RTI_XCDR_SER_PROGRAM = 0x00000001 << 0,
    RTI_XCDR_DESER_PROGRAM = 0x00000001 << 1,
    RTI_XCDR_SKIP_PROGRAM = 0x00000001 << 2,
    RTI_XCDR_GET_SER_SIZE_PROGRAM = 0x00000001 << 3,
    RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM = 0x00000001 << 4,
    RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM = 0x00000001 << 5,
    RTI_XCDR_SER_TO_KEY_PROGRAM = 0x00000001 << 6,
    RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM = 0x00000001 << 7,
    RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM = 0x00000001 << 8
} RTIXCdrTypeProgramKind;

struct RTIXCdrInstructionIndex;

/* Gets the pointer to the value pointed by an external reference.
 * For example, in modern C++ an external reference has the type:
 * dds::core::external
 *
 * This function must return the pointer associated with the value associated
 * with the external reference
 *
 * For example:
 *
 * @external int a;
 *
 * The call to RTIXCdrGetExternalRefPointerFcn will return a pointer to a
 */
typedef char * (*RTIXCdrGetExternalRefPointerFcn)(void *ref);

struct RTIXCdrDependentProgramList;

/*
 * Members are ordered in this structure to minimize memory usage.
 */
typedef struct RTIXCdrProgram {
    /* A program can be part of a dependentProgramList */
    struct RTIXCdrInlineListNode node;
    /* TypeCode associated with the program */
    struct RTIXCdrTypeCode *typeCode;
    /* A program can contain complex instructions that reference other programs
     * The following list contains the list of programs that can be referenced
     * by this program.
     *
     * Usually this list is created by the top level program and provided
     * to the nested programs so that they can use it.
     *
     * The sole purpose of the list is to avoid the creation of multiple
     * programs for the same type.
     */
    struct RTIXCdrDependentProgramList *dependentProgramList;
    RTIXCdrInstruction *instructions;

    /* Instruction index by member ID */
    struct RTIXCdrInstructionIndex *instructionIndex;
    /* Instruction index by label for unions */
    struct RTIXCdrInstructionIndex *instructionIndexByLabel;

    /* member used to manipulate external refs */
    RTIXCdrGetExternalRefPointerFcn getExternalRefPointerFcn;
    RTIXCdrTypeProgramKind kind;

    /*
     * RTI_XCDR_TK_NULL if the program is not for a union. The unaliased kind
     * of the discriminator otherwise
     */
    RTIXCdrTCKind unionDiscKind;

    /**
     * The default discriminator value of a union. This value stores the
     * default value of the discriminator type, which may be different
     * depending whether or not the
     * RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT bit is set
     * in the xtypes compliance mask.
     */
    RTIXCdrLong defaultUnionDisc;

    /**
     * The default case discriminator value of a union.
     * The DynamicData API allows a user to set a member by name without
     * providing a value to set the discriminator to. In that case, if
     * they set the default case, we need to have a value to set the
     * discriminator to. This value is stored here.
     * This value might be different from the defaultUnionDisc value,
     * the default discriminator value of a union doesn't have to be
     * the same as the default discriminator of the default case member.
     */
    RTIXCdrLong defaultCaseDisc;

    /**
     * Unbounded size.
     * 
     * A sequence or string with a maximum size greater or equal than this 
     * number is considered unbounded.
     * 
     * There is an internal feature that allows the user to specify what value 
     * is used here to determine if members are unbounded. If this feature is 
     * not used, this value is RTIXCDRLong_MAX (See CORE-10578).
     */
    RTIXCdrUnsignedLong unboundedSize;

   /**
     * XTypes compliance mask.
     *
     * It is used to configure the level of compliance with the XTypes
     * specification.
     *
     * The default value is RTI_XCDR_XTYPES_COMPLIANCE_MASK_DEFAULT
     *
     * We need the mask here because the interpreter needs it in both
     * TypePlugin program generation and execution. The mask is
     * cached during the program generation for the program execution.
     */
    RTIXCdrXTypesComplianceMask xTypesComplianceMask;

    RTIXCdrExtensibilityKind extKind;
    RTIXCdrEncapsulationId encapsulationId;

    /*
     * When used in a TypePlugin program:
     * The instruction index of the instruction that processes the
     * discriminator in union
     *
     * When used in a Sample program:
     * The instruction of the member identified by the
     * default discriminator.
     */
    RTIXCdrUnsignedLong unionInsIndex;

    /*
     * Only used in a Sample program:
     * The instruction of the default case member.
     */
    RTIXCdrUnsignedLong unionDefaultCaseInsIndex;

    /* For TK_STRUCT and TK_VALUE this field conatisn the alignment of the
     * first member
     */
    RTIXCdrAlignment firstMemberAlignment;

    RTIXCdrUnsignedLong instructionCount;

    /* Used by getSerSampleMaxSize and getSerSampleMinSize when optimization
     * is 2. In this case, after the program is generated we run it once,
     * we cache the result here and we delete the instructions.
     *
     * The program execution will return this number
     */
    RTIXCdrUnsignedLong serSize;

    RTIXCdrUnsignedShort externalReferenceSize;

    /* Indicates if a program is only for key fields */
    RTIXCdrBoolean onlyKey;
    RTIXCdrBoolean isFlatDataProgram;
    RTIXCdrBoolean isFastSerializationSupported;

    RTIXCdrBoolean serializeSentinelOnBase;
    RTIXCdrBoolean disableMustUnderstandOnSentinel;
    RTIXCdrBoolean isCdr2;
    RTIXCdrBoolean hasBase;
    RTIXCdrBoolean isUnbounded;

    /* Indicates if the type associated with the program has any optional
     * members at any level of nesting.
     */
    RTIXCdrBoolean hasOptionals;

    /*
     * Indicates if this program is the owner of the dependentProgramList. The
     * list will be deleted only by its owner when RTIXCdrInterpreter_deleteProgram
     * is invoked.
     */
    RTIXCdrBoolean listOwner;
} RTIXCdrProgram;

extern RTIXCdrDllExport
void RTIXCdrInterpreter_deleteProgram(RTIXCdrProgram * me);


#define RTI_XCDR_PROGRAM_COUNT 9


typedef RTIXCdrLong RTIXCdrProgramMask;


#define RTI_XCDR_PROGRAM_MASK_NONE ((RTIXCdrProgramMask) 0)

#define RTI_XCDR_PROGRAM_MASK_TYPEPLUGIN \
( \
    RTI_XCDR_SER_PROGRAM | \
    RTI_XCDR_DESER_PROGRAM | \
    RTI_XCDR_SKIP_PROGRAM | \
    RTI_XCDR_GET_SER_SIZE_PROGRAM | \
    RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM | \
    RTI_XCDR_GET_MIN_SER_SIZE_PROGRAM | \
    RTI_XCDR_SER_TO_KEY_PROGRAM \
)

#define RTI_XCDR_PROGRAM_MASK_SAMPLE \
( \
    RTI_XCDR_INITIALIZE_SAMPLE_PROGRAM | \
    RTI_XCDR_ALLOCATED_SAMPLE_PROGRAM \
)

#define RTI_XCDR_PROGRAM_MASK_ALL  ((RTIXCdrProgramMask)0xFFFFFFFF)

#define RTI_XCDR_REJECT_UNKNOWN_DISCRIMINATOR 0
#define RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR_AND_SELECT_DEFAULT 1
#define RTI_XCDR_ACCEPT_UNKNOWN_DISCRIMINATOR 2

struct RTIXCdrSampleAssignabilityProperty {
    /* 
     * Whether or not to accept samples with unknown enumeration values.
     *
     * If the value is set to RTI_XCDR_TRUE, then the unknown enum value is 
     * converted to its default value.
     */
    RTIXCdrBoolean acceptUnknownEnumValue;
    /* 
     * Whether or not to accept samples with a discriminator that does
     * not select any member in the union (unknown discriminator).
     * 0: Samples with unknown discriminator are not accepted.
     * 1: Samples with unknown discriminator are accepted and deserialized
     * as a union sample initialize to its default value.
     * 2: Samples with unknown discriminator are accepted and the discriminator
     * keeps the unknown value. No member is selected. This is the behavior
     * specified in the XTypes specification.
     */
    RTIXCdrOctet acceptUnknownUnionDiscriminator;
};

#define RTIXCdrSampleAssignabilityProperty_INITIALIZER { \
    RTI_XCDR_FALSE, /* acceptUnknownEnumValue */ \
    RTI_XCDR_REJECT_UNKNOWN_DISCRIMINATOR /* acceptUnknownUnionDiscriminator */ \
}

/* @brief Generates a type plugin program that manipulates a CDR stream
 *
 * @param tc \b In. TypeCode.
 * 
 * @param programKind \b Program kind.
 * 
 * @param littleEndianEncapsulation \b In. Indicates if the program is for
 * a little endian encapsulation or not.
 * 
 * @param v2Encapsulation \b In. Indicates if the program is for a V2
 * (XTypes 1.2) encapsulation or a V1.
 * 
 * @param onlyKey \b In. Indicates if the program must be generated for only
 * key fields.
 * 
 * @return The stream program if success. Otherwise, NULL.
 */
typedef struct RTIXCdrTypePluginProgramProperty {
    RTIXCdrBoolean littleEndianEncapsulation;
    RTIXCdrBoolean v2Encapsulation;
    RTIXCdrBoolean onlyKey;
    /* This parameter is used only when littleEndianEncapsulation is FALSE,
     * v2Encapsulation is TRUE, and onlyKey is TRUE.
     *
     * The value of this parameter determines if the program will serialize
     * the key fir keyhash generation purposes or not.
     */
    RTIXCdrBoolean onlyKeyForKeyhash;
    /* Indicates if aliases can be resolved to their most primitive type
     * for optimization purposes. If the user does not need to customize
     * the TypePlugin code for an alias, this parameter must be
     * set to RTI_XCDR_TRUE to optimize performance.
     * 
     * When this parameter is set to TRUE the following structure can be
     * serialized with a single instruction:
     * 
     * typedef LongType x;
     * 
     * struct Point {
     *     LongType x;
     *     LongType y;
     * };
     */
    RTIXCdrBoolean resolveAlias;
    /* Indicates if the processing of enumeration values can be optimized
     * by not calling the TypePlugin functions associated with the enum
     * 
     * For example:
     * 
     * enum VehicleKind {
     *     CAR,
     *     PLANE
     * };
     * 
     * struct Vehicle {
     *     VehicleKind kind;
     * };
     * 
     * With optimizing enums, the TypePlugin functions for Vehicle will call
     * the TypePlugin functions for VehicleKind to operate with enums. 
     * 
     * If the user does not require changing the TypePlugin code for the enum,
     * the program execution for Vehicle can be optimized by operating 
     * directly on the enum value without calling the enum's TyePlugin
     * functions.
     * 
     * For example, the serialization of kind with be done with a single
     * SER_PRIMITIVE instruction.
     */
    RTIXCdrBoolean optimizeEnum;
    /* Indicates if the interpreter can inline nested structs within the
     * parent struct. For example:
     * 
     * struct Point {
     *     long x;
     *     long y;
     * };
     * 
     * struct Rectangle {
     *     Point p1;
     *     Point p2;
     * };
     * 
     * With inlineStruct enabled, a Rectangle sample can be serialized with a
     * single SER_PRIMITIVE instruction for 4 longs.
     *
     * This optimization also applies to the elements of a sequence. For 
     * example:
     *
     * struct RGBPixel {
     *     long r;
     *     long g;
     *     long b;
     * };
     * 
     * struct Image {
     *   sequence<RGBPixel, 1048576> pixels;
     * };
     *
     * With inlineStruct enabled, an Image sample can be serialized with a
     * single memcpy operation.
     */
    RTIXCdrBoolean inlineStruct;
    /*
     * When inlineStruct is set to RTI_XCDR_TRUE, sequence elements may be 
     * expanded inline and serialized in a single memcpy operation. 
     * However, if you intend to use sequences with discontiguous buffers 
     * in the DataWriter samples for C and C++98, you must disable this 
     * inline expansion by setting this property to RTI_XCDR_FALSE.
     *
     * By disabling inline expansion, the ability to serialize a sequence 
     * buffer with a single memcpy call is disabled. Instead, the sequence 
     * elements will be serialized one by one. However, the serialization 
     * of each element will still be optimized by the interpreter.
     */
    RTIXCdrBoolean inlineSequence;
    /*
     * Used to force the program generation for every complex type, even when a
     * struct is inlined. This does NOT mean that structs are not inlined, but
     * that if a user requires retrieving the programs for nested complex types,
     * they will be available for lookup in the dependentPrograms list.
     * 
     * DynamicData sets this property to TRUE because member programs are
     * retrieved during the bind operation
     */
    RTIXCdrBoolean forceDependentPrograms;

    /* Indicates if the V1 sentinel for mutable types must set the
     * must understand bit
     */
    RTIXCdrBoolean disableMustUnderstandOnSentinel;
    /* Indicates if the V1 sentinel must be serialized for a base class for
     * the purposes of key serialization and keyhash generation
     */
    RTIXCdrBoolean serializeSentinelOnBase;

    /* Members required to manipulate external references */
    RTIXCdrUnsignedShort externalReferenceSize;
    RTIXCdrGetExternalRefPointerFcn getExternalRefPointerFcn;

    /**
     * Unbounded size.
     * 
     * A sequence or string with a maximum size greater or equal than this 
     * number is considered unbounded.
     * 
     * Default value is RTIXCdrLong_MAX
     * 
     * At code generation time this default value can be overwitten using
     * the non-public command-line option -VtcUnboundedSize (See CORE-10578).
     */
    RTIXCdrUnsignedLong unboundedSize;
    /*
     * Overrides settings in NDDS_Config_XTypesComplianceMask when generating
     * type programs. Necessary to force spec compliance for XCDR2 on builtin
     * channels. This will only take effect when non-zero.
     */
    RTIXCdrXTypesComplianceMask xTypesComplianceMask;
} RTIXCdrTypePluginProgramProperty;

#define RTIXCdrTypePluginProgramProperty_INITIALIZER \
    { \
        RTI_XCDR_TRUE,           /* littleEndianEncapsulation */ \
                RTI_XCDR_TRUE,   /* v2Encapsulation */ \
                RTI_XCDR_FALSE,  /* onlyKey */ \
                RTI_XCDR_FALSE,  /* onlyKeyForKeyhash */ \
                RTI_XCDR_TRUE,   /* resolveAlias */ \
                RTI_XCDR_TRUE,   /* optimizeEnum */ \
                RTI_XCDR_TRUE,   /* inlineStruct */ \
                RTI_XCDR_TRUE,   /* inlineSequence */ \
                RTI_XCDR_FALSE,  /* forceDependentPrograms */ \
                RTI_XCDR_FALSE,  /* disableMustUnderstandOnSentinel */ \
                RTI_XCDR_FALSE,  /* serializeSentinelOnBase */ \
                sizeof(char *),  /* externalReferenceSize */ \
                NULL,            /* getExternalRefPointerFcn */ \
                RTIXCdrLong_MAX, /* unboundedSize */ \
                0                /* xTypesComplianceMask */ \
    }

/*
 * Context of the execution of a program
 */
struct RTIXCdrTypePluginProgramContext {
    /* This parameter is specific to a language binding an it is provided
     * to the RTIXCdrSampleAccessInfo accessor methods
     */
    void *programData;
    /* This is the endpointPluginData parameter in the TypePlugin functions 
     * This value is opaque to the interpreter. It is needed because it needs
     * to be propagated to the TypePlugin calls for complex nested members
     */
    void *endpointPluginData;
    /* This is the endpointPluginQos parameter in the TypePlugin functions 
     * This value is opaque to the interpreter. It is needed because it needs
     * to be propagated to the TypePlugin calls for complex nested members
     */    
    void *endpointPluginQos;
    /*
     * Indicates if the Interpreter functions must log errors that are non
     * expected space errors.
     *
     * For example:
     * - Failures due to range checking
     * - Failures getting a member value in a data sample
     * - Failures serializing strings and sequences with a length greater than
     * the maximum allowed.
     * - etc.
     *
     * See CORE-14102 for more information about when log messages should and
     * should not be printed in the type plugin.
     */
    RTIXCdrBoolean logAllErrorsButExpectedSpaceErrors;
    /* 
     * Indicates if there was an expected failure due to lack of space on 
     * XCDR stream.
     * 
     * The definition of expectedSpace error is different for serialization and 
     * deserialization:
     * 
     * - On serialization, we treat all the errors due to running out of space
     * in the XCDR stream as expected. This is done because with batching we use 
     * serialization space failures to detect situations in which we have to 
     * start a new batch because max_data_bytes is exceeded.
     * 
     * - On deserialization, it is ok to run out of space. This can occur
     * if a DR subscribing to a derived type receive a base type sample.
     * However, not all space errors are considered expected (unlike 
     * serialization). See RTIXCdrInterpreter_isUnexpectedSpaceError 
     * documentation for additional info.
     */
    RTIXCdrBoolean expectedSpaceError;
    
    /* Pointer to the program that will have to be executed when calling
     * a TypePlugin function on a nested complex member.
     * 
     * The TopLevel type will get its program by calling:
     * RTIXCdrInterpreterPrograms_getXXXProgram
     * 
     * For complex nested members the program will be obtained from 
     * this variable.
     * 
     * This variable will be assigned by the interpreter's TypePlugin 
     * functions (for example RTIXCdrInterpreter_serializeSample) before 
     * calling TypePlugin functions for nested complex members.
     */
    RTIXCdrProgram *program;
    /* Pointer to he typeCode associated with the program above */
    struct RTIXCdrTypeCode *typeCode; 
    
    /* Used for getSerSampleSizeFnc */
    RTIXCdrEncapsulationId encapsulationId;
    /* Used by functions that can work with onlyKey members:
     * serialize
     * deserialize
     * getMaxSizeSerialized
     */
    RTIXCdrBoolean onlyKey;
    /* Used to indicate if there is an overflow when invoking the TypePlugin
     * getMaxSizeSerialized functions
     */
    RTIXCdrLong overflow;
    
    /* For the getSerSampleSize, getSerSampleMaxSize and getSerSampleMinSize
     * this pointer will be used to propagate the stream that is used to 
     * calculate the sizes downstream, as the stream is not a parameter of
     * the corresponding TypePlugin operations
     */
    RTIXCdrStream *xcdrStream;
    
    /* Indicates if we are processing a base class
     * This field is needed to not include the DHEADER field or the SENTINEL
     * when working with V1 mutable types
     */
    RTIXCdrBoolean inBaseClass;
    
    /* Indicates whether or not to use extended ID header for 
     * non-primitive fields when the encapsulation is XCDR1
     */
    RTIXCdrBoolean useXcdr1ExtendedId;

    /* Used to change range in alias of primitive values */
    struct RTIXCdrTypeCodeAnnotations *annotations;

    /* 
     * To handle recursion, this node points to the parent node, which is part
     * of the stack of the calling program.
     */
    const struct RTIXCdrTypeCodeNode *parentVisitedNode;

    /*
     * When deserializing mutable types with one or more base types,
     * we must inject the instruction index from the most-derived type so
     * that each program in the hierarchy can detect known member IDs, and
     * avoid accidentally erroring if a member ID (from another type) is
     * marked as "must understand".
     */
    struct RTIXCdrInstructionIndex *instructionIndex;
};

#define RTIXCdrTypePluginProgramContext_INTIALIZER \
    { \
        NULL,                  /* programData */ \
                NULL,          /* endpointPluginData */ \
                NULL,          /* endpointPluginQos */ \
                RTI_XCDR_TRUE, /* logAllErrorsButExpectedSpaceErrors */ \
                RTI_XCDR_TRUE, /* expectedSpaceError */ \
                NULL,          /* program */ \
                NULL,          /* typeCode */ \
                RTI_XCDR_ENCAPSULATION_ID_CDR_LE, /* encapsulationId */ \
                RTI_XCDR_FALSE,                   /* onlyKey */ \
                RTI_XCDR_FALSE,                   /* overflow */ \
                NULL,                             /* xcdrStream */ \
                RTI_XCDR_FALSE,                   /* inBaseClass */ \
                RTI_XCDR_FALSE,                   /* useXcdr1ExtendedId */ \
                NULL,                             /* annotations */ \
                NULL,                             /* parentVisitedNode */ \
                NULL                              /* instructionIndex */ \
    }

/* @brief Generates a type plugin program that manipulates a CDR stream
 *
 * @param tc \b In. TypeCode. The TypeCode should not be deleted for the 
 * duration of the program
 * 
 * @param dependentProgramList \b InOut. List of dependent programs. If this
 * parameter is NULL the program will generate a new list that will be used
 * to store dependent programs.
 * 
 * @param programKind \b In. Program kind.
 * 
 * @return The program if success. Otherwise, NULL.
 */

extern RTIXCdrDllExport
RTIXCdrProgram *RTIXCdrInterpreter_generateTypePluginProgram(
        /* The TypeCode should not be deleted for the duration of the program */
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList *dependentProgramList,
        RTIXCdrTypeProgramKind programKind,
        const struct RTIXCdrTypePluginProgramProperty *property);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_serializeSample(
        struct RTIXCdrStream *me,
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleWithEncapsulationEx(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        struct RTIXCdrStream *stream,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrBoolean *expectedSpaceError);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleWithEncapsulation(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        struct RTIXCdrStream *stream,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_serializeSampleToCdrBuffer(
        char *buffer,
        RTIXCdrUnsignedLong *length,
        const struct RTIXCdrInterpreterPrograms *programs,
        const void *sample,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fullSerializeSample(
        RTIXCdrStream *stream,
        void *sample,
        RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fastSerializeSample(
        RTIXCdrStream *stream,
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_deserializeSample(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fullDeserializeSample(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fastDeserializeSample(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

/* 
 * For C, C++, and modern C++ the deserialization operation does not need to
 * initialize the target sample because all fields will be populated.
 *
 * For DynData and SqlFilter the situation is different:
 * - For SqlFilter we depend on the initialization being done by the deserialize
 * operation as the legacy initialization before deserializing is not invoked.
 * CORE-9123
 *
 * - For DynData, reference members (strings, optional members) must be
 * re-initialized to default values. When a sample is returned to the reader
 * pool the memory manager is reset, invalidating all reference values within
 * the dynamicdata buffer. These must be zeroed before deserializing to avoid
 * trying to dereference an invalid reference within the memory manager.
 * CORE-9121
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_isInitializationNeededOnDeserialization(
        const struct RTIXCdrProgram *program);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_deserializeSampleWithEncapsulation(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        struct RTIXCdrStream *stream,
        const struct RTIXCdrSampleAssignabilityProperty *ap);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_deserializeFromCdrBuffer(
        const struct RTIXCdrInterpreterPrograms *programs,
        void *sample,
        const char *buffer,
        RTIXCdrUnsignedLong length);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_serializedSampleToKey(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fullSerializedSampleToKey(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_fastSerializedSampleToKey(
        void *sample,
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const struct RTIXCdrSampleAssignabilityProperty *sampleAssignability,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_skipSample(
        RTIXCdrStream *stream,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleSize(
        RTIXCdrUnsignedLong *size,
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrInterpreter_getSerSampleSizeWithEncapsulation(
        RTIXCdrUnsignedLong *size,
        void *sample,
        const struct RTIXCdrInterpreterPrograms *programs,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMaxSize(
        RTIXCdrUnsignedLong *size,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMaxSizeWithEncapsulation(
        RTIXCdrUnsignedLong *size,
        const struct RTIXCdrInterpreterPrograms *programs,
        RTIXCdrEncapsulationId encapsulationId);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_getSerSampleMinSize(
        RTIXCdrUnsignedLong *size,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        struct RTIXCdrTypePluginProgramContext *context);

extern RTIXCdrDllExport
void RTIXCdrProgram_print(RTIXCdrProgram *me, const char * title);


typedef struct RTIXCdrInterpreterProgramsGenProperty {
    RTIXCdrBoolean generateV1Encapsulation;
    RTIXCdrBoolean generateV2Encapsulation;
    RTIXCdrBoolean generateLittleEndian;
    RTIXCdrBoolean generateBigEndian; 
    RTIXCdrBoolean generateWithAllFields;
    RTIXCdrBoolean generateWithOnlyKeyFields;    
    RTIXCdrBoolean resolveAlias;
    RTIXCdrBoolean optimizeEnum;    
    RTIXCdrBoolean inlineStruct;
    RTIXCdrBoolean inlineSequence;
    RTIXCdrBoolean forceDependentPrograms;
    /* Indicates if the V1 sentinel for mutable types must set the
     * must understand bit
     */
    RTIXCdrBoolean disableMustUnderstandOnSentinel;
    /* Indicates if the V1 sentinel must be serialized for a base class for
     * the purposes of key serialization and keyhash generation
     */
    RTIXCdrBoolean serializeSentinelOnBase;
    /* Members required to manipulate external references */
    RTIXCdrUnsignedShort externalReferenceSize;
    RTIXCdrGetExternalRefPointerFcn getExternalRefPointerFcn;

    /*
     * Unbounded size.
     *
     * A sequence or string with a maximum size greater or equal than this
     * number is considered unbounded.
     *
     * Default value is RTIXCdrLong_MAX
     *
     * At code generation time this default value can be overwitten using
     * the command-line option -VtcUnboundedSize (See CORE-10578).
     */
    RTIXCdrUnsignedLong unboundedSize;
    /*
     * Overrides settings in NDDS_Config_XTypesComplianceMask when generating
     * type programs. Necessary to force spec compliance for XCDR2 on builtin
     * channels. This will only take effect when non-zero.
     */
    RTIXCdrXTypesComplianceMask xTypesComplianceMask;
} RTIXCdrInterpreterProgramsGenProperty;

#define RTIXCdrInterpreterProgramsGenProperty_INITIALIZER \
    { \
        RTI_XCDR_TRUE,           /* generateV1Encapsulation */ \
                RTI_XCDR_TRUE,   /* generateV2Encapsulation */ \
                RTI_XCDR_TRUE,   /* generateLittleEndian */ \
                RTI_XCDR_TRUE,   /* generateBigEndian */ \
                RTI_XCDR_TRUE,   /* generateWithAllFields */ \
                RTI_XCDR_TRUE,   /* generateWithOnlyKeyFields */ \
                RTI_XCDR_TRUE,   /* resolveAlias */ \
                RTI_XCDR_TRUE,   /* optimizeEnum */ \
                RTI_XCDR_TRUE,   /* inlineStruct */ \
                RTI_XCDR_TRUE,   /* inlineSequence */ \
                RTI_XCDR_FALSE,  /* forceDependentPrograms */ \
                RTI_XCDR_FALSE,  /* disableMustUnderstandOnSentinel */ \
                RTI_XCDR_FALSE,  /* serializeSentinelOnBase */ \
                sizeof(char **), /* externalReferenceSize */ \
                NULL,            /* getExternalRefPointerFcn */ \
                RTIXCdrLong_MAX, /* unboundedSize */ \
                0                /* xTypesComplianceMask */ \
    }


extern RTIXCdrDllExport
int RTIXCdrInterpreterProgramsGenProperty_compare(
        const RTIXCdrInterpreterProgramsGenProperty *left,
        const RTIXCdrInterpreterProgramsGenProperty *right);

extern RTIXCdrDllExport
void RTIXCdrInterpreterProgramsGenProperty_toTypePluginProperty(
        const RTIXCdrInterpreterProgramsGenProperty *self,
        RTIXCdrTypePluginProgramProperty *tpProperty);

struct RTIXCdrInterpreterPrograms;

/* @brief Creates a program set. This program set is the owner of the programs,
 * and the responsible of creating/destroying them.
 * 
 * A program set is a set of related programs associated with an input
 * TypeCode.
 * 
 * The kind of programs contained in a program set is determined
 * by the input mask.
 * 
 * Given a program kind, the user can generate multiple versions of the 
 * program for different endianness, encapsulations, and so on. The generated
 * versions are configured using the input generation property.
 * 
 * Why do we have to generate multiple versions of a program kind?
 * 
 * For example, in the case of deserialization, this is needed because the input
 * sample maybe serialized in LITTLE endian or BIG endian. This information is
 * not known at program generation time. In this case, when the sample is
 * received, the application should pick the right version of the program by
 * looking at the encapsulation header and using the program set API: 
 * RTIXCdrInterpreterPrograms_getSerProgram
 * 
 * @param type \b In. TypeCode associated with the program set. This TypeCode
 * should have _sampleAccessInfo set for the language binding for which the
 * program are generated.
 * 
 * @param property \b In. Determine what programs to generate for a given
 * program kind. It also configures the generation optimization parameters.
 *
 * @param mask \b In. Program kinds contained in the program set.
 *
 * @return Program set if success. Otherwise, NULL.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreterPrograms_initialize(
        struct RTIXCdrInterpreterPrograms *me,
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterProgramsGenProperty *property,
        RTIXCdrProgramMask mask);

/* @brief Creates a program set from an existing program set. The programs will
 * be loaned from the top-level programs associated to the passed parent
 * programs.
 *
 * @param tc \b In. TypeCode associated with the program set. This TypeCode
 * should have _sampleAccessInfo set for the language binding for which the
 * program are generated.
 *
 * @param parentPrograms \b In. Program set from which we will retrieve the
 * properties and the top-level programs (the owner of the programs).
 *
 * @param mask \b In. Program kinds contained in the generated program set.
 *
 * @return Program set if success. Otherwise, NULL.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreterPrograms_initializeFromPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterPrograms *parentPrograms,
        RTIXCdrProgramMask mask);

/* @brief If the program set is the owner of the programs, releases the
 * resources associated to the programs. If the program set is not the owner,
 * this function is a noop.
 */
extern RTIXCdrDllExport
void RTIXCdrInterpreterPrograms_finalize(struct RTIXCdrInterpreterPrograms *me);

extern RTIXCdrDllExport
struct RTIXCdrInterpreterPrograms *RTIXCdrInterpreterPrograms_new(
        const RTIXCdrTypeCode *type,
        const struct RTIXCdrInterpreterProgramsGenProperty *property,
        RTIXCdrProgramMask mask);

extern RTIXCdrDllExport
void RTIXCdrInterpreterPrograms_delete(struct RTIXCdrInterpreterPrograms *me);

/*
 * @brief Assert the programs for a given program set. Programs will be either
 * retrieved from the top-level program set (potentially triggering its
 * generation as part of the top-level program set) or generated if me is
 * the owner of the programs.
 *
 * @param me Program set where we will assert the programs.
 *
 * @param mask The program kind mask. Only program kinds in the mask will be
 *        asserted.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreterPrograms_assertPrograms(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrProgramMask mask);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getSerProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean littleEndianEncapsulation,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getSerProgramForKeyhash(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean v2Encapsulation);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getDeserProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean littleEndianEncapsulation,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getSerToKeyProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean littleEndianEncapsulation,
        RTIXCdrBoolean v2Encapsulation);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getSkipProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean littleEndianEncapsulation,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getSerSizeProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getMaxSerSizeProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getMaxSerSizeProgramForKeyhash(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean v2Encapsulation);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getMinSerSizeProgram(
        struct RTIXCdrInterpreterPrograms *me,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean onlyKey);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getInitializeSampleProgram(
        struct RTIXCdrInterpreterPrograms *me);

extern RTIXCdrDllExport
struct RTIXCdrProgram * RTIXCdrInterpreterPrograms_getAllocatedMembersSampleProgram(
        struct RTIXCdrInterpreterPrograms *me);

typedef struct RTIXCdrSampleProgramContext {
    /* 
     * This parameter is specific to a language binding an it is provided
     * to the RTIXCdrSampleAccessInfo accessor methods
     */
    void *programData;

    /* 
     * Members do not always a tcMemberInfo, for example, when a sequence or
     * array is a top-level type. In these cases, we need to create a
     * tcMemberInfo. This field allows us to always create the tcMemberInfo with
     * the correct value for optionality/pointers  
     */
    RTIXCdrRefMemberKind refMemberKind;

    /* 
     * Indicates whether the program needs to treat this member as though it has
     * not been processed at all yet.
     * 
     * This is used in the initialization program to indicate whether to set the
     * sample to 0 or not. It is not necessary to set a nested complex member
     * to 0 if the enclosing type has already done this work.
     * 
     * In the copy program, the top level type is copied, any nested members
     * do not need to be copied unless they are referred to through a reference
     * from the enclosing type. 
     */
    RTIXCdrBoolean isTopLevel;

    /* 
     * The sample interpreter has a program for every array. At the beginning of
     * a program we are required to check if the language binding is flat data
     * in order to get the correct offsets and sizes. Arrays, however, are not
     * required to have sample access infos. In this case, we cache the
     * language in the context so that we can still determine this information.
     * 
     * It is not required for callers of the sample interpreter programs to
     * set this value correctly. The program will set it based on the first tc's
     * sampleAccessInfo
     */ 
    RTIXCdrLanguageBinding languageBinding;

    /*
     * Indicates if there was a space error while initializing a sample.
     */
    RTIXCdrBoolean spaceError;
} RTIXCdrSampleProgramContext;


#define RTIXCdrSampleProgramContext_INITIALIZER_W_BINDING(binding__) { \
    NULL, /* programData */ \
    RTI_XCDR_INTERPRETER_VALUE_MEMBER, /* refMemberKind */ \
    RTI_XCDR_TRUE, /* isTopLevel */ \
    binding__, /* languageBinding */ \
    RTI_XCDR_FALSE /* spaceError */ \
}

#define RTIXCdrSampleProgramContext_INITIALIZER \
  RTIXCdrSampleProgramContext_INITIALIZER_W_BINDING( \
          RTI_XCDR_TYPE_BINDING_INVALID)

/*
 * @brief Given a typecode, generate a program that will act on a user-level
 *        sample of that type.
 *
 * @param tc The typecode describing the type of the sample for which to
 *        generate the program
 * @param dependentProgramList \b InOut. List of dependent programs. If this
 *        parameter is NULL the program will generate a new list that will be
 *        used to store dependent programs.
 * @param programKind The kind of the program to generate. Currently support
 *        programs:
 *        - initialize
 *        - allocated
 * @param property Properties that may alter how a given programKind is
 *        generated
 */
extern RTIXCdrDllExport
RTIXCdrProgram *RTIXCdrInterpreter_generateSampleProgram(
        const RTIXCdrTypeCode *tc,
        struct RTIXCdrDependentProgramList *dependentProgramList,
        RTIXCdrTypeProgramKind programKind, 
        const struct RTIXCdrTypePluginProgramProperty *property);


/*
 * Arguments to RTIXCdrSampleInterpreter_initializeSample controlling
 * initialization and memory allocation.
 */
typedef struct RTIXCdrInitializeSampleProperty {
    /*
     * Whether or not to initialize the sample to all zeroes before then going
     * through and setting the members with non-zero defaults. This is set to
     * false for flat data as flat data will set the sample contents to zero
     * before calling the initialize program.
     */
    RTIXCdrBoolean initializeToZero;

    /*
     * Whether bounded strings are allocated with enough capacity to hold their
     * maximum size as indicated in the TypeCode. If false, only enough memory
     * to hold each member's default value is allocated. Note that for sequences
     * this behavior is implemented in the accessors, not in the interpreter
     * itself.
     */
    RTIXCdrBoolean allocateMaximumSize;

    /*
     * Indicates if space errors must be logged. This parameter is generally
     * true except in the case of SqlFilter, in which the deserialization
     * (that calls initialize first) can fail if there is not enough space in
     * the deserialization buffer.
     *
     * The filter code is prepared to reallocate the deserialize buffer and
     * try again.
     */
    RTIXCdrBoolean logSpaceErrors;
} RTIXCdrInitializeSampleProperty;

#define RTIXCdrInitializeSampleProperty_INITIALIZER { \
    RTI_XCDR_TRUE, /* initializeToZero */ \
    RTI_XCDR_FALSE, /* allocateMaximumSize */ \
    RTI_XCDR_TRUE, /* logSpaceErrors */ \
}

/*
 * @brief Initialize a sample to its default values.
 *
 * No memory currently allocated to this sample will be freed
 * (i.e. for pointer members). Finalize must be called first if the sample
 * has not just been created on the stack or the heap.
 *
 * @param sample the sample to initialize
 * @param tc the Typecode of the sample that is being initialized
 * @param program the initialize program for the sample that was generated in
 *        RTIXCdrSampleInterpreter_generateProgram
 * @param property Controls some aspects of the initialization and memory
 *        allocation.
 * @param context The program context. Contains the program data
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeSample(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const RTIXCdrInitializeSampleProperty *property,
        RTIXCdrSampleProgramContext *context);

/*
 * @brief Initialize a range of indexes in a sequence. This method is used when
 *        resizing a sequence. This method assumes that the sequence buffer has
 *        already been allocated
 *
 * @param sample the sequence to initialize
 * @param program The sequence's program
 * @param indexBegin The index of the first member to initialize
 * @param indexEnd The index of the last member to initialize
 * @param context The program context
 *
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeSequenceMembers(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrUnsignedLong indexBegin,
        RTIXCdrUnsignedLong indexEnd,
        RTIXCdrSampleProgramContext *context);

/**
 * @brief Resize a sequence inside a struct or union.
 *
 * Using the initialize program for a struct or union, this function resizes
 * a sequence member of any element type.
 *
 * The member must be a sequence. It cannot be an array of sequences.
 *
 * @param sample The data sample
 * @param program The initialize program for the type that contains the sequence
 * @param index The instruction index that initializes the sequence
 * @param newElementCount The new size of the sequence
 * @param context The program context
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_resizeSequenceMember(
        void *sample,
        const RTIXCdrProgram *program,
        RTIXCdrUnsignedLong index,
        RTIXCdrUnsignedLong newElementCount,
        RTIXCdrSampleProgramContext *context);

/*
 * @brief Initialize a union to the specified discriminator
 * 
 * This method allows the initialization of a union to a case value that is not
 * the default case value.
 * 
 * @param sample the union to initialize
 * @param tc the union's typecode
 * @param program The unions's program
 * @param initializeToZero whether or not to initialize the sample to all zeroes
 *        before then going through and setting the members with non-zero
 *        defaults.
 * @param discValue The discriminator value to initialize
 * @param context The program context
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_initializeUnion(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        const RTIXCdrInitializeSampleProperty *property,
        RTIXCdrLong discValue,
        RTIXCdrSampleProgramContext *context);

/*
 * Arguments to RTIXCdrSampleInterpreter_finalizeSample controlling
 * finalization and memory deletion.
 */
typedef struct RTIXCdrFinalizeSampleProperty {
    /*
     * When true, the finalize function will finalize and delete optional
     * members only, at all levels of nestedness.
     */
    RTIXCdrBoolean finalizeOptionalsOnly;
} RTIXCdrFinalizeSampleProperty;

#define RTIXCdrFinalizeSampleProperty_INITIALIZER { \
    RTI_XCDR_FALSE, /* finalizeOptionalsOnly */ \
}

/*
 * @brief Finalize a sample. All memory will be released, but memory will not
 * be set to default values. the initialize program must be used for that.
 *
 * @param sample the sample to finalize
 * @param tc the Typecode of the sample that is being finalized
 * @param program the finalize program for the sample that was generated in
 *        RTIXCdrSampleInterpreter_generateProgram
 * @param discValue A specific discriminator value to finalise. This
 *        functionality is used when we're deserializing a union and we need to
 *        finalize the previously initialized union member
 * @param programData an opaque pointer that may be used in different language
 *        binding to provide language-specific program data to the program that
 *        is being run
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_finalizeSample(
        void *sample,
        const RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrLong *discValue,
        const RTIXCdrFinalizeSampleProperty *property,
        RTIXCdrSampleProgramContext *context);

/*
 * @brief Copy a sample.
 * For flat data and dynamic data, the program copies the source to the
 * destination as a single memcpy and then traverses through the nested members,
 * allocating members in the destination sample where necessary (sequences/
 * strings/optional/reference members).
 *
 *
 * @param destPt The destination sample
 * @param source The source sample
 * @param tc the Typecode of the samples that are being copied
 * @param program The copy program for this type
 * @param sourceContext The context for the source sample
 * @param destContext The context for the destination sample
 *
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleInterpreter_copySample(
        void *destPt,
        void *source,
        RTIXCdrTypeCode *tc,
        const RTIXCdrProgram *program,
        RTIXCdrSampleProgramContext *sourceContext,
        RTIXCdrSampleProgramContext *destContext);

/*
 * CORE-10578
 * @brief Allow the user to indicate the unbounded support value.
 */
extern RTIXCdrDllExport void
RTIXCdrInterpreter_setUnboundedSize(RTIXCdrUnsignedLong unboundedSize);

/*
 * CORE-10578
 * @brief Allow the user to get the unbounded support value.
 */
extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrInterpreter_getUnboundedSize(void);

/**
 * @brief When this bit is set the serialization of sequences
 * and arrays with non primitive members includes a DHEADER.
 *
 * To be compatible with the XTypes specification this bit must be set.
 *
 * Only applies to V2 encapsulation.
 */
#define RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT (0x00000001U)

/**
 * @brief When this bit is set, enums are considered primitive
 * types in collection types.
 *
 * This means that a DHEADER will not be added to collections of enums if
 * RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT is set.
 *
 * To be compatible with the XTypes specification this bit must be unset.
 *
 * Only applies to V2 encapsulation.
 */
#define RTI_XCDR_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT (0x00000002U)

/**
 * @brief When this bit is set the length of a member header in a
 * mutable type will take into account the added padding that may
 * follow the serialized member.
 *
 * To be compatible with the XTypes specification this bit must be unset.
 *
 * Only applies to V1 encapsulation.
 */
#define RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT (0x00000004U)

/**
 * @brief When this bit is set the options in the encapsulation header will
 * include the padding bits.
 *
 * Connext will set the least significant two bits in the second byte of the 
 * options field to a value that encodes the number of padding bytes needed 
 * after the end of the serialized payload in order to reach the next 4-byte 
 * aligned offset.
 */
#define RTI_XCDR_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT (0x00000008U)

/**
 * @brief When this bit is set, samples containing an unknown enumerator
 * can be successfully deserialized to the default enumeration value, and
 * the sample wont be dropped.
 *
 * To be compatible with the XTypes specification this bit must be unset.
 */
#define RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_ENUM_VALUE_BIT (0x00000010U)

/**
 * @brief When this bit is set, samples containing an unknown discriminator
 * can be successfully deserialized, and the sample wont be dropped.
 *
 * To be compatible with the XTypes specification this bit must be set.
 */
#define RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_DISCRIMINATOR_BIT (0x00000020U)

/**
 * @brief When this bit is set, samples containing an unknown discriminator
 * will be assigned the default discriminator value, if the bit is not set
 * the sample will preserve the discriminator value.
 *
 * To be compatible with the XTypes specification this bit must be unset.
 *
 * This bit will be ignored if the bit RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_DISCRIMINATOR_BIT
 * is not set.
 */
#define RTI_XCDR_XTYPES_SELECT_DEFAULT_DISCRIMINATOR_BIT (0x00000040U)

/**
 * @brief When and only when this bit is set, samples containing
 * an empty union (with only a discriminator and no selected
 * member) will be serialized with a sentinel header.
 *
 * To be compatible with the XTypes specification, this bit must
 * be set.
 */
#define RTI_XCDR_XTYPES_SENTINEL_IN_EMPTY_UNION_BIT (0x00000080U)

/**
 * @brief When this is set the discriminator of a union will be initialized
 * to the default value of the discriminator type. If this bit is not set
 * the discriminator will be initialized to smallest value of the discriminator
 * present in the union.
 *
 * To be compatible with the XTypes specification this bit must be set.
 */
#define RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT (0x00000100U)

/**
 * @brief When this bit is set, the KeyHash calculation for a Java instance
 * will match the one produced by the Java API in Connext 7.5.0.
 * However, this behavior is not compliant with the XTypes specification.
 *
 * This affects KeyHash calculation only when using XCDR encoding and
 * the Java language API for mutable types, and either of the following
 * conditions is met:
 *   - The key is not a primitive type, and the maximum serialized size
 *     of a sample for the type exceeds 65KB.
 *   - At least one key member has a member ID greater than 16128.
 *
 * To be compatible with the XTypes specification, this bit must
 * be unset.
 */
#define RTI_XCDR_XTYPES_JAVA_KEYHASH_750_BIT (0x00000200U)

/**
 * @brief When this bit is set, the TypeCode generation for a Python type
 * that contains an Int8 member will match the one produced by the Python
 * API in Connext 7.3.0.
 * However, this behavior is not compliant with the XTypes specification,
 * and Python types with Int8 members won't interoperate with other APIs.
 *
 * To be compatible with the XTypes specification, this bit must be unset.
 */
#define RTI_XCDR_XTYPES_PYTHON_INT8_730_BIT (0x00000400U)


/**
 * @brief When this bit is set, the parameter id for members that are part of
 * the key, or for the discriminator of a union, will have the MUST_UNDERSTAND
 * bit set when serialized using XCDR2.
 *
 * To be compatible with the XTypes specification, this bit must
 * be set.
 *
 * This bit only applies to V2 encapsulation, in order to preserve backwards
 * compatibility with existing applications using XCDR1. The MUST_UNDERSTAND
 * bit will never be set for key members and discriminators when using V1
 * encapsulation, because doing so would change the key hash calculation.
 */
#define RTI_XCDR_XTYPES_MUST_UNDERSTAND_IN_KEY_AND_DISCRIMINATOR_BIT \
    (0x00000800U)

/**
 * @brief The XTypes vendor compliance mask.
 *
 * This value is fully aligned with the XTypes specification.
 * If this value is updated you should evaluate the need to update
 * `programProperties->xTypesComplianceMask` in
 * RTIXCdrTypeObjectV2_getTypeObjectV2ProgramProperties
 */
#define RTI_XCDR_XTYPES_COMPLIANCE_MASK_VENDOR \
    (RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT \
     | RTI_XCDR_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT \
     | RTI_XCDR_XTYPES_ACCEPT_UNKNOWN_DISCRIMINATOR_BIT \
     | RTI_XCDR_XTYPES_SENTINEL_IN_EMPTY_UNION_BIT \
     | RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT \
     | RTI_XCDR_XTYPES_MUST_UNDERSTAND_IN_KEY_AND_DISCRIMINATOR_BIT)

/**
 * @brief The default XTypes compliance mask.
 *
 * This value is not fully aligned with the XTypes specification.
 *
 * If you require full compatibility with the specification use the
 * RTI_XCDR_XTYPES_COMPLIANCE_MASK_VENDOR
 */
#define RTI_XCDR_XTYPES_COMPLIANCE_MASK_DEFAULT \
    (RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT \
     | RTI_XCDR_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT \
     | RTI_XCDR_XTYPES_SENTINEL_IN_EMPTY_UNION_BIT \
     | RTI_XCDR_XTYPES_INITIALIZE_DISCRIMINATOR_TO_DEFAULT_BIT)

/**
 * @brief Loads into the sample assignability property the corresponding values
 * from the global XTypes compliance mask.
 *
 * @param property The sample assignability property to set.
 */
extern RTIXCdrDllExport
void RTIXCdrSampleAssignabilityProperty_setFromGlobalComplianceMask(
        struct RTIXCdrSampleAssignabilityProperty *property);

/**
 * @brief Assigns the unknown union discriminator option from
 * the XTypes compliance mask.
 *
 * @param mask The bitmask where the bit needs to be unset.
 * @param value The unknown union discriminator return value.
 */
extern RTIXCdrDllExport
void RTIXCdrXTypesComplianceMask_assignUnknownUnionDiscriminatorOption(
        RTIXCdrUnsignedLong *value,
        RTIXCdrXTypesComplianceMask mask);

/**
 * @brief Checks if a specific bit is set in a bitmask.
 *
 * @param mask__ The bitmask to check for the set status of the specified
 * bit.
 * @param bit__ The bit to check.
 */
extern RTIXCdrDllExport
void RTIXCdrXTypesComplianceMask_isBitSet(
        RTIXCdrXTypesComplianceMask mask,
        RTIXCdrUnsignedLong bit);

/**
 * @brief Sets a specific bit in a bitmask.
 *
 * @param mask__ The bitmask where the bit needs to be set.
 * @param bit__ The bit to set.
 */
extern RTIXCdrDllExport
void RTIXCdrXTypesComplianceMask_setBit(
        RTIXCdrXTypesComplianceMask mask,
        RTIXCdrUnsignedLong bit);

/**
 * @brief Unsets (clears) a specific bit in a bitmask.
 *
 * @param mask__ The bitmask where the bit needs to be unset.
 * @param bit__ The bit to unset.
 */
extern RTIXCdrDllExport
void RTIXCdrXTypesComplianceMask_unsetBit(
        RTIXCdrXTypesComplianceMask mask,
        RTIXCdrUnsignedLong bit);

/**
 * @brief Environment variable name for the XTypes compliance mask.
 *
 * If this environment variable is set, the value will be the one used
 * to initialize the global XTypes compliance mask.
 */
#define NDDS_XTYPES_COMPLIANCE_MASK_ENV_VAR \
    "NDDS_XTYPES_COMPLIANCE_MASK"

/**
 * @brief Gets the global XTypes compliance mask.
 *
 * @return The global XTypes compliance mask.
 */
extern RTIXCdrDllExport
RTIXCdrXTypesComplianceMask RTIXCdrInterpreter_getGlobalXtypeComplianceMask(void);

/**
 * @brief Sets the global XTypes compliance mask.
 *
 * @param mask The new value for the global XTypes compliance mask.
 */
extern RTIXCdrDllExport
void RTIXCdrInterpreter_setGlobalXtypeComplianceMask(RTIXCdrXTypesComplianceMask mask);

/**
 * @brief Load the value of the environment variable NDDS_XTYPES_COMPLIANCE_MASK
 * in the global XTypes compliance mask, if it exists.
 *
 * The format of the must is an integer in HEX or decimal notation.
 *
 * @return RTI_XCDR_TRUE if there were no errors, RTI_XCDR_FALSE if
 * there is an error parsing the value. If the var is not defined the
 * method returns RTI_XCDR_TRUE and leaves the global XTypes compliance
 * mask unchanged.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrInterpreter_loadGlobalXTypeComplianceMask(void);

extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrXTypesComplianceMask_verifyGeneratedXTypesMask(
        RTIXCdrXTypesComplianceMask mask);

/*
 * @brief Finds the first program in the program list with typecode equals
 * to tc and program kind equals to kind.
 *
 * @param self In. The list. Cannot be NULL.
 * @param tc In. Expected program typecode. Cannot be NULL.
 * @param kind In. Expected program kind.
 *
 * @return Found program or NULL.
 */
extern RTIXCdrDllExport RTIXCdrProgram *RTIXCdrDependentProgramList_findProgram(
        const struct RTIXCdrDependentProgramList *self,
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrTypeProgramKind kind);

#ifdef __cplusplus
    }	/* extern "C" */
#endif


#include "xcdr/xcdr_interpreter_impl.h"

#endif /* xcdr_interpreter_h */
