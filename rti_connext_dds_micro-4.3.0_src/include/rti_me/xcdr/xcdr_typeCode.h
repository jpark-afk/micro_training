/*
(c) Copyright, Real-Time Innovations, 2017-2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_typeCode_h
#define xcdr_typeCode_h


#include "xcdr/xcdr_infrastructure_psm.h"
#ifdef __cplusplus
    extern "C" {
#endif


#include "xcdr/xcdr_stream.h"


/* Value assigned to the label DEFAULT in a union.
 * This value should not be used to determine if a label is the default label
 * or not. Use the function RTIXCdrTypeCode_get_default_index instead.
 */
#define RTI_XCDR_TYPE_CODE_UNION_DEFAULT_LABEL 0x40000001

    typedef enum {
        RTI_XCDR_TK_NULL,
        RTI_XCDR_TK_SHORT,
        RTI_XCDR_TK_LONG,
        RTI_XCDR_TK_USHORT,
        RTI_XCDR_TK_ULONG,
        RTI_XCDR_TK_FLOAT,
        RTI_XCDR_TK_DOUBLE,
        RTI_XCDR_TK_BOOLEAN,
        RTI_XCDR_TK_CHAR,
        RTI_XCDR_TK_OCTET,
        RTI_XCDR_TK_STRUCT,
        RTI_XCDR_TK_UNION,
        RTI_XCDR_TK_ENUM,
        RTI_XCDR_TK_STRING,
        RTI_XCDR_TK_SEQUENCE,
        RTI_XCDR_TK_ARRAY,
        RTI_XCDR_TK_ALIAS,
        RTI_XCDR_TK_LONGLONG,
        RTI_XCDR_TK_ULONGLONG,
        RTI_XCDR_TK_LONGDOUBLE,
        RTI_XCDR_TK_WCHAR,
        RTI_XCDR_TK_WSTRING,
        RTI_XCDR_TK_VALUE,
        RTI_XCDR_TK_INT8,
        RTI_XCDR_TK_UINT8,

        RTI_XCDR_TK_RAW_BYTES = 0x7e,
        RTI_XCDR_TK_RAW_BYTES_KEYED = 0x7f,

        /*
         * Important! When adding new flags, make sure that the value does not
         * conflict with the values used to indicate that a typecode is in cdr
         * representation, see RTIXCdrTypeCode_isCdrRepresentation
         */

        /* only used for local representation, OR with above */
        RTI_XCDR_TK_FLAG_IS_INDEXED = 0x8000,

        /* only used to represent xtypes annotations */
        RTI_XCDR_TK_FLAGS_IS_FINAL = 0x4000,
        RTI_XCDR_TK_FLAGS_IS_MUTABLE = 0x2000,

        /*
         * !!!! DO NOT ADD NEW ANNOTATIONS HERE !!!
         * Add them to RTIXCdrTypeCodeAnnotations
         */
        /* only used to represent RTI-specific annotations */
        RTI_XCDR_TK_FLAGS_IS_FLAT_DATA = 0x10000,
        RTI_XCDR_TK_FLAGS_IS_SHMEM_REF = 0x20000,

        RTI_XCDR_TK_FLAGS_ALL = 0xfff00

    } RTIXCdrTCKind;

    typedef enum {
        RTI_XCDR_FINAL_EXTENSIBILITY,
        RTI_XCDR_EXTENSIBLE_EXTENSIBILITY,
        RTI_XCDR_MUTABLE_EXTENSIBILITY
    } RTIXCdrExtensibilityKind;

    typedef short RTIXCdrValueModifier;


#define RTI_XCDR_VM_NONE 0
#define RTI_XCDR_VM_CUSTOM 1
#define RTI_XCDR_VM_ABSTRACT 2
#define RTI_XCDR_VM_TRUNCATABLE 3
#define RTI_XCDR_VM_MUTABLE 4

typedef short RTIXCdrVisibility;


#define RTI_XCDR_PRIVATE_MEMBER 0
#define RTI_XCDR_PUBLIC_MEMBER 1

typedef RTIXCdrOctet RTIXCdrMemberFlags;


#define RTI_XCDR_NONKEY_MEMBER 0
#define RTI_XCDR_KEY_MEMBER 1
#define RTI_XCDR_REQUIRED_MEMBER 2 /* for STRUCT AND VALUE */

/* bit flags: [... required | key ] */
#define RTI_XCDR_FLAG_KEY_MEMBER 1
#define RTI_XCDR_FLAG_REQUIRED_MEMBER 2

struct RTIXCdrTypeCode;

struct RTIXCdrTypeCodeMember;

typedef RTIXCdrOctet RTIXCdrLanguageBinding;

#define RTI_XCDR_TYPE_BINDING_INVALID       0x00
#define RTI_XCDR_TYPE_BINDING_C             0x01
#define RTI_XCDR_TYPE_BINDING_CPP           0x02
#define RTI_XCDR_TYPE_BINDING_CPP_03        0x03
#define RTI_XCDR_TYPE_BINDING_CPP_03_STL    0x04
#define RTI_XCDR_TYPE_BINDING_CPP_11        0x05
#define RTI_XCDR_TYPE_BINDING_CPP_11_STL    0x06
#define RTI_XCDR_TYPE_BINDING_DYN_DATA      0x07
#define RTI_XCDR_TYPE_BINDING_SQL_FILTER    0x08

#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_C           0x11
#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_CPP         0x12
#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_CPP_03      0x13
#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_CPP_11      0x14
#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_CPP_03_STL  0x15
#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_CPP_11_STL  0x16

#define RTI_XCDR_TYPE_BINDING_FLAT_DATA_MASK 0x10

/* We created this structure to support use cases in which the language binding
 * implementation cannot return a pointer to a member value with the expected
 * format.
 *
 * For example, in modern C++, a vector of booleans does not store an array
 * of elements where each byte represent a boolean. In fact, it does store
 * booleans in a compact form (booleans)
 *
 * Because of this language binding implementation, the call to
 * getMemberValuePointerFcn cannot return a pointer to a buffer containing
 * the boolean values of the vector
 *
 * The interpreter will have to get/set one boolean at a time and the language
 * binding will not return a pointer to the value but the actual value
 * as part of this union.
 *
 * At this point only bVal, wVal, and lVal are used by language bindings.
 * However, we have also added RTIXCdrLongLong to avoid a potential static code
 * analysis issues. RTIXCdrLongDouble cannot be added because the underlying
 * representation of that type has changed in GCC 4.4 and it broke the ABI of
 * passing union. Instead we will add a 16-byte array member.
 *
 * The sequence buffers the isDiscontiguous flag is used to indicate that the
 * sequence is discontiguous. Instead of returning a pointer an array of
 * elements, the language binding will return a pointer to a buffer containing
 * the pointers to the elements of the sequence.
 */
typedef struct RTIXCdrMemberValue {
    RTIXCdrBoolean isNull;
    RTIXCdrBoolean isDiscontiguous; /* Only used for sequences */
    union RTIXCdrMemberValue_v {
        char *ptr;
        RTIXCdrBoolean bVal;
        RTIXCdrWchar wVal;
        RTIXCdrLong lVal;
        /* To avoid static code analysis issues in ROBUSTNESS-147 */
        RTIXCdrLongLong llVal;
        /* To avoid compilation warning in CORE-15861 */
        char ldVal[16];
    } value;
} RTIXCdrMemberValue;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

  #define RTIXCdrMemberValue_INITIALIZER \
      { \
          RTI_XCDR_FALSE,         /* isNull */ \
                  RTI_XCDR_FALSE, /* isDiscontiguous */ \
          { \
              /* Initializing the largest member in the */ \
              /* union ensures that all members get     */ \
              /* initialized to 0 (PLATFORMS-2155)      */ \
              .llVal = 0ll \
          } \
      }

#else

    #define RTIXCdrMemberValue_INITIALIZER        \
        {                                         \
            RTI_XCDR_FALSE, /* isNull */          \
            RTI_XCDR_FALSE, /* isDiscontiguous */ \
            {                                     \
                0                                 \
            }                                     \
        }

#endif

extern RTIXCdrDllVariable
const RTIXCdrMemberValue RTI_XCDR_MEMBER_VALUE_INVALID;

extern RTIXCdrDllVariable
const RTIXCdrMemberValue RTI_XCDR_MEMBER_VALUE_NIL;

/*
 * If the member is an optional member or a pointer, isNull must be set in the
 * returned value. Otherwise, setting isNull is not mandatory.
 *
 * This output elementCount value must be set for sequences and strings.For
 * strings, the elementCount shall include the null terminator.
 */
typedef RTIXCdrMemberValue (*RTIXCdrGetMemberValuePointerFcn)(
        void *sample,
        RTIXCdrUnsignedLong *elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        /* The elementIndex is only used when setMemberElementValueFcn is set
         */
        RTIXCdrUnsignedLong elementIndex,
        const struct RTIXCdrTypeCode *memberTc,
        /* In some cases this parameter could be NULL. Do not assume it cannot
         * be NULL
         *
         * For example, if the interpreter calls the function to get a pointer
         * for the element in a sequence. Technically in this case
         * there is no memberInfo because the memberInfo corresponds
         * to the sequence
         */
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        void *programData);

/*
 * @brief Sets elementCount in sequences and strings
 *
 * @param trimToSize. Used for unbounded members. When set to true the
 * implementation must reallocate sequences and strings to have a capacity
 * of elementCount elements.
 * 
 * @param initializeElement. Indicates if the elements of the sequence up to
 * elementCount must be initialized to their default value. For example, 
 * this parameter will be set to FALSE for final fix size types if  
 * optimizations are enabled. 
 *
 * For sequences, elementCount is the sequence length
 * For strings, elementCount is the string length including the NULL-terminated
 * character. elementCount should never be zero.
 *
 * The function must set failure to RTI_XCDR_TRUE if there is an error.
 */
typedef RTIXCdrMemberValue (*RTIXCdrSetMemberElementCountFcn)(
        RTIXCdrBoolean *failure,
        void *sample,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *tcMemberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        RTIXCdrBoolean trimToSize,
        RTIXCdrBoolean initializeElement,
        void *programData);

/*
 * @brief Sets the value for the element with index elementIndex in a
 * sequence or string.
 *
 * @precondition For optional members, the member should be set before this
 * function is invoked.
 *
 * For sequences, elementCount is the sequence length
 * For strings, elementCount is the string length including the NULL-terminated
 * character. elementCount should never be zero.
 */
typedef void (*RTIXCdrSetMemberElementValueFcn)(
        void *sample,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        /* The elementIndex is only used when setMemberElementValueFcn is set
         */
        RTIXCdrUnsignedLong elementIndex,
        RTIXCdrMemberValue val,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        void *programData);

/*
 * @brief Finalize a member
 *
 * @param sample The sample containing the member to finalize
 *
 * @param bindingMemberValueOffset The offset of the member to finalize within
 *        sample
 *
 * @param memberInfo the memberInfo of the member to finalize
 *
 * @param deallocateReference Whether to deallocate memory associated with the
 *        member being finalized or to just clear the reference to allocated
 *        memory. This is true when called from the finalize program, but
 *        set to false when called from the copy program because we have copied
 *        reference from the source to the destination and we simply want to
 *        clear the reference and then reallocate memory to copy into
 *
 * @param programData The language binding-specific programData
 */
typedef void (*RTIXCdrFinalizeMemberValueFcn) (
        void *sample,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean deallocateReference,
        void *programData);


#define RTIXCdrMemberAccessInfo_OFFSET_COUNT 4

typedef struct RTIXCdrMemberAccessInfo {
    /* A member can have multiple offsets for flat data language binding
     * in which a member offset varies depending on the address in which
     * the sample containing the member starts
     * 
     * For the rest of the languages only the first offset is used
     * 
     * Note that the bindingMemberValueOffset type is RTIXCdrUnsignedLong
     * versus RTIXCdrUnsignedLongLong. We chose to do this to save space
     * because our implementation restricts the maximum size of a sample to
     * 2^31 for other reasons
     */
    RTIXCdrUnsignedLong bindingMemberValueOffset[RTIXCdrMemberAccessInfo_OFFSET_COUNT];
    /* When this is true, the deserialize program will skip this member instead.
     * Additionally, the initialize program will not be generated for the
     * member.
     */
    RTIXCdrBoolean skipDeserialization;
} RTIXCdrMemberAccessInfo;


#define RTIXCdrMemberAccessInfo_INITIALIZER {{0,0,0,0}, RTI_XCDR_FALSE}


/* 
 * The boolean type in the TypePlugin functions is RTIXCdrLong to match
 * RTIBool in higher layers
 */

typedef RTIXCdrLong (*RTIXCdrTypePluginSerializeFunction)(
        void *endpointPluginData,
        const void *sample, 
        struct RTIXCdrStream *stream, 
        RTIXCdrLong serializeEncapsulation,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrLong serializeSample, 
        void *endpointPluginQos);

typedef RTIXCdrLong (*RTIXCdrTypePluginSerializeKeyFunction)(
        void *endpointPluginData,
        const void *sample, 
        struct RTIXCdrStream *stream, 
        RTIXCdrLong serializeEncapsulation,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrLong serializeKey, 
        void *endpointPluginQos);

typedef RTIXCdrLong (*RTIXCdrTypePluginDeserializeFunction)(
        void *endpointData,
        void *sample,
        struct RTIXCdrStream *stream,
        RTIXCdrLong deserializeEncapsulation,
        RTIXCdrLong deserializeSample, 
        void *endpointPluginQos);

typedef RTIXCdrLong (*RTIXCdrTypePluginDeserializeKeyFunction)(
        void *endpointData,
        void *sample,
        struct RTIXCdrStream *stream,
        RTIXCdrLong deserializeEncapsulation,
        RTIXCdrLong deserializeKey, 
        void *endpointPluginQos);

typedef RTIXCdrLong (*RTIXCdrTypePluginSkipFunction)(
        void *endpointData,
        struct RTIXCdrStream *stream,   
        RTIXCdrLong skipEncapsulation,
        RTIXCdrLong skipSample, 
        void *endpointPluginQos);
/*
 * The size parameter (the parameter without name) is not used and it is here 
 * only for legacy purposes. This is done so that this function definition is 
 * compatible with the definition expected by the TypePlugin.
 *
 * The size is obtained from the RTIXCdrStream provided as part of the
 * endpointData.
 */
typedef RTIXCdrUnsignedLong (*RTIXCdrTypePluginGetSerializedSampleSizeFunction)(
        void *endpointData,
        RTIXCdrLong includeEncapsulation,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrUnsignedLong /* size */,
        const void *sample);

/*
 * The size parameter (the parameter without name) is not used and it is here 
 * only for legacy purposes. This is done so that this function definition is 
 * compatible with the definition expected by the TypePlugin.
 * 
 * The size is obtained from the RTIXCdrStream provided as part of the
 * endpointData.
 */
typedef RTIXCdrUnsignedLong (*RTIXCdrTypePluginGetSerializedSampleMaxSizeFunction)(
        void *endpointData,
        RTIXCdrLong *overflow,
        RTIXCdrLong includeEncapsulation,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrUnsignedLong /* size */);
 
/*
 * The size parameter (the parameter without name) is not used and it is here 
 * only for legacy purposes. This is done so that this function definition is 
 * compatible with the definition expected by the TypePlugin.
 * 
 * The size is obtained from the RTIXCdrStream provided as part of the
 * endpointData.
 */
typedef RTIXCdrUnsignedLong (*RTIXCdrTypePluginGetSerializedSampleMinSizeFunction)(
        void *endpointData,
        RTIXCdrLong includeEncapsulation,
        RTIXCdrEncapsulationId encapsulationId,
        RTIXCdrUnsignedLong /* size */);

/*
 * The size parameter (the parameter without name) is not used and it is here 
 * only for legacy purposes. This is done so that this function definition is 
 * compatible with the definition expected by the TypePlugin.
 * 
 * The size is obtained from the RTIXCdrStream provided as part of the
 * endpointData.
 */
typedef RTIXCdrUnsignedLong (*RTIXCdrTypePluginGetSerializedKeyMaxSizeFunction)(
        void *endpointData,
        RTIXCdrLong *overflow,
        RTIXCdrLong includeEncapsulation,
        RTIXCdrEncapsulationId encapsulation_id,
        RTIXCdrUnsignedLong /* size */);

typedef RTIXCdrLong (*RTIXCdrTypePluginSerializedSampleToKeyFunction)(
        void *endpointData,
        void *sample,
        struct RTIXCdrStream *stream, 
        RTIXCdrLong deserializeEncapsulation,  
        RTIXCdrLong deserializeKey, 
        void *endpointPluginQos);

typedef RTIXCdrLong (*RTIXCdrTypePluginInitializeSampleFunction)(
        void *sample,
        RTIXCdrLong allocatePointers, 
        RTIXCdrLong allocateMemory);

/* 
 * A type used when deserializing unions in order to indicate to the
 * initializeWParam callback the previous discriminator value that was set and
 * the one that was received.
 * 
 * This allows the callback implementation to finalize and reinitialize the
 * member if needed. 
 */
typedef struct RTIXCdrUnionInitializeInfo {
    RTIXCdrLong discValuePrev;
    RTIXCdrLong discValueNext;
} RTIXCdrUnionInitializeInfo;

#define RTIXCdrUnionInitializeInfo_INTIALIZER { \
    0, /* discValuePrev */ \
    0 /* discValueNext */ \
}

/* 
 * Used for language bindings that do no provide the generated initialize
 * methods. Currently the only user is the DynamicData language binding.
 * 
 * @param sample The member getting initiaized
 * @param typeCode The typecode of the member getting initialized
 * @param unionInfo If this method is called for a union, the union info will
 *        be set indicating that the previous union member should be finalized
 *        and the new member should be initialized
 * @param programData The programData from the calling program
 * @param param A language binding specific param that is provided in the
 *        RTIXCdrTypePlugin when this initialize method is set
 */ 
typedef RTIXCdrLong (*RTIXCdrTypePluginInitializeSampleWParamsFunction)(
        void *sample,
        const struct RTIXCdrTypeCode *typeCode,
        RTIXCdrUnionInitializeInfo *unionInfo,
        void *programData,
        void *param);

typedef RTIXCdrLong (*RTIXCdrTypePluginFinalizeSampleFunction)(
        void *sample);

typedef RTIXCdrLong (*RTIXCdrTypePluginFinalizeSampleWParamsFunction)(
        void *sample,
        const struct RTIXCdrTypeCode *typeCode,
        void *programData,
        void *param);

typedef struct RTIXCdrTypePlugin {
    RTIXCdrTypePluginSerializeFunction serializeFnc;
    RTIXCdrTypePluginSerializeKeyFunction serializeKeyFnc;
    RTIXCdrTypePluginDeserializeFunction deserializeFnc;
    RTIXCdrTypePluginDeserializeKeyFunction deserializeKeyFnc;
    RTIXCdrTypePluginSkipFunction skipFnc;
    RTIXCdrTypePluginGetSerializedSampleSizeFunction getSerSampleSizeFnc;
    RTIXCdrTypePluginGetSerializedSampleMaxSizeFunction getSerSampleMaxSizeFnc;
    RTIXCdrTypePluginGetSerializedKeyMaxSizeFunction getSerKeyMaxSizeFnc;
    RTIXCdrTypePluginGetSerializedSampleMinSizeFunction getSerSampleMinSizeFnc;
    RTIXCdrTypePluginSerializedSampleToKeyFunction serializedSampleToKeyFnc;
    RTIXCdrTypePluginInitializeSampleFunction initializeSampleFnc;
    RTIXCdrTypePluginInitializeSampleWParamsFunction initializeSampleWParamsFnc;
    RTIXCdrTypePluginFinalizeSampleFunction finalizeSampleFnc;
    RTIXCdrTypePluginFinalizeSampleWParamsFunction finalizeSampleWParamsFnc;
    /*
     * Can be used by a language binding to pass in language binding specific
     * information to any RTIXCdrTypePlugin *WParamsFnc
     */
    void *typePluginParam;
} RTIXCdrTypePlugin;

#define RTIXCdrTypePlugin_INITIALIZER { \
    NULL, /* serializeFnc */ \
    NULL, /* serializeKeyFnc */ \
    NULL, /* deserializeFnc */ \
    NULL, /* deserializeKeyFnc */ \
    NULL, /* skipFnc */ \
    NULL, /* getSerSampleSizeFnc */ \
    NULL, /* getSerSampleMaxSizeFnc */ \
    NULL, /* getSerKeyMaxSizeFnc */ \
    NULL, /* getSerSampleMinSizeFnc */ \
    NULL, /* serializedSampleToKeyFnc */ \
    NULL, /* initializeSampleFnc */ \
    NULL, /* initializeSampleWParamsFnc */ \
    NULL, /* finalizeSampleFnc */ \
    NULL, /* finalizeSampleWParamsFnc */ \
    NULL /* typePluginParam */ \
}


#define RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES 4

typedef struct RTIXCdrSampleAccessInfo {
    RTIXCdrLanguageBinding languageBinding;
    /* A type can have multiple sizes for flat data language binding
     * in which the type size varies depending on the address in which
     * a member of that type starts
     * 
     * For the rest of the languages only the type is used
     */
    RTIXCdrUnsignedLong typeSize[RTIXCdrMemberAccessInfo_NUM_TYPE_SIZES];
    /* Set to TRUE if the user wants to call getMemberValuePointerFcn only
     * for optional or external members
     */
    RTIXCdrBoolean useGetMemberValueOnlyWithRef;
    RTIXCdrGetMemberValuePointerFcn getMemberValuePointerFcn;
    RTIXCdrSetMemberElementCountFcn setMemberElementCountFcn;
    RTIXCdrSetMemberElementValueFcn setMemberElementValueFcn;
    RTIXCdrFinalizeMemberValueFcn finalizeMemberValueFcn;
    struct RTIXCdrMemberAccessInfo *memberAccessInfos;
} RTIXCdrSampleAccessInfo;

extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(
        RTIXCdrLanguageBinding languageBinding);

/*
 * Contiguous-memory bindings are those where memory allocations are performed
 * using a memory manager and the memory of a data sample is always contiguous
 * in memory. The C binding for example is not contiguous because variable-sized
 * members are pointers.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(
        RTIXCdrLanguageBinding languageBinding);

/*
 * Sets isValid to true if value is a valid enum value in tc, otherwise 
 * isValid is set to false.
 */
extern void RTIXCdrTypeCode_isValidEnumValue(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrBoolean *isValid,
        RTIXCdrEnum value);

#define RTIXCdrSampleAccessInfo_INITIALIZER { \
    RTI_XCDR_TYPE_BINDING_C, \
    {0,0,0,0}, \
    RTI_XCDR_FALSE, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL \
}

typedef struct RTIXCdrTypeCodeRepresentation {
    /*
     * struct, union, value: _pid is the member ID of the member. It
     * can be set by users with the @ID annotation in IDL.
     */
    RTIXCdrUnsignedLong _pid;
    RTIXCdrBoolean _isPointer; /* struct, union, value */
    RTIXCdrShort _bits; /* struct, value */
    struct RTIXCdrTypeCode *_typeCode; /* struct, union, value */
} RTIXCdrTypeCodeRepresentation;

typedef enum {
    RTI_XCDR_XCDR = (0x00000001 << 0),
    RTI_XCDR_XML = (0x00000001 << 1),
    RTI_XCDR_XCDR2 = (0x00000001 << 2)
} RTIXCdrDataRepresentationMaskBits;

typedef RTIXCdrLong RTIXCdrDataRepresentationMask;

#define RTI_XCDR_DATA_REPRESENTATION_MASK_DEFAULT (RTI_XCDR_XCDR|RTI_XCDR_XCDR2)

/*
 * Literal value of an annotation member: either the default value in its
 * definition or the value applied in its usage.
 */
typedef struct RTIXCdrAnnotationParameterValue {

    RTIXCdrTCKind _d;
    union RTIXCdrAnnotationParameterValue_u {
        RTIXCdrBoolean boolean_value;
        RTIXCdrOctet octet_value;
        RTIXCdrInt8 int8_value;
        RTIXCdrUInt8 uint8_value;
        RTIXCdrShort short_value;
        RTIXCdrUnsignedShort ushort_value;
        RTIXCdrLong long_value;
        RTIXCdrUnsignedLong ulong_value;
        RTIXCdrLongLong long_long_value;
        RTIXCdrUnsignedLongLong ulong_long_value;
        RTIXCdrFloat float_value;
        RTIXCdrDouble double_value;
        /* RTIXCdrLongDouble long_double_value; Not supported yet */
        RTIXCdrChar char_value;
        RTIXCdrWchar wchar_value;
        RTIXCdrLong enumerated_value;
        RTIXCdrChar *string_value;
        RTIXCdrWchar *wstring_value;
    } _u;

} RTIXCdrAnnotationParameterValue;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

  #define RTIXCdrAnnotationParameterValue_INITIALIZER \
      { \
          RTI_XCDR_TK_NULL, \
          { \
              /* Initializing the largest member in the */ \
              /* union ensures that all members get     */ \
              /* initialized to 0 (PLATFORMS-2155)      */ \
              .long_long_value = 0ll \
          } \
      }

#else

    #define RTIXCdrAnnotationParameterValue_INITIALIZER \
        {                                               \
            RTI_XCDR_TK_NULL,                           \
            {                                           \
                0                                       \
            }                                           \
        }

#endif

typedef enum RTIXCdrObservableDistributionKind {
    RTI_XCDR_OBSERVABLE_PERIODIC_DISTRIBUTION_KIND,
    RTI_XCDR_OBSERVABLE_ON_CHANGE_DISTRIBUTION_KIND,
    RTI_XCDR_OBSERVABLE_UNSPECIFIED_DISTRIBUTION_KIND
} RTIXCdrObservableDistributionKind;


typedef struct RTIXCdrObservableAnnotation {
    RTIXCdrObservableDistributionKind distributionKind;
    RTIXCdrBoolean isSet;
} RTIXCdrObservableAnnotation;

#define RTIXCdrObservableAnnotation_INITIALIZER { \
    RTI_XCDR_OBSERVABLE_UNSPECIFIED_DISTRIBUTION_KIND, \
    RTI_XCDR_FALSE \
}

typedef struct RTIXCdrResourceAnnotation {
    char *className;
    char *owner;
    RTIXCdrBoolean isRoot;
    RTIXCdrBoolean isSet;
} RTIXCdrResourceAnnotation;

#define RTIXCdrResourceAnnotation_INITIALIZER { \
    NULL, \
    NULL, \
    RTI_XCDR_FALSE, \
    RTI_XCDR_FALSE \
}

typedef struct RTIXCdrTypeCodeAnnotations {
    struct RTIXCdrAnnotationParameterValue _defaultValue;
    struct RTIXCdrAnnotationParameterValue _minValue;
    struct RTIXCdrAnnotationParameterValue _maxValue;
    RTIXCdrChar *_unitValue;
    RTIXCdrDataRepresentationMask _allowedDataRepresentationMask;
    /*
     * @observable_group for aggregate types or
     * @observable for members
     */
    RTIXCdrObservableAnnotation _observable;
    /* @resource for aggregate types */
    RTIXCdrResourceAnnotation _resource;
    /* @non_serialized */
    RTIBool _isSerializable;
    /* @nested */
    RTIBool _isNested;
    /* @autoid(HASH) */
    RTIBool _isAutoIdHash;
    /* @must_understand
     *
     * This flag is only set for members that are explicitly annotated with
     * @must_understand. Key members and union discriminators will not have
     * this flag set. When converting between TypeObbject (V1, and V2) to
     * TypeCode we will make sure to normalize the value and to enforce this
     * convention, even if the user manually changed the flag.
     *
     * This flag is stored inside TypeCodeAnnotations because adding it to
     * RTIXcdrTypeCodeMember would cause too much disruption. For this reason
     * the literal value of the member is not checked during comparisons of
     * RTIXcdrTypeCodeAnnotations. A member's "must understand" property will be
     * checked during comparison of the enclosing RTIXCdrTypeCodeMember by
     * querying the value with the is_member_must_understand() function.
     * The getter function will ignore the value stored in this field if a
     * member is a key and always return true for it, improving the robustness
     * of the comparison.
     */
    RTIBool _isMustUnderstand;
} RTIXCdrTypeCodeAnnotations;

#define RTIXCdrTypeCodeAnnotations_INITIALIZER \
    { \
        RTIXCdrAnnotationParameterValue_INITIALIZER, /* _defaultValue */ \
                RTIXCdrAnnotationParameterValue_INITIALIZER, /* _minValue */ \
                RTIXCdrAnnotationParameterValue_INITIALIZER, /* _maxValue */ \
                NULL,                                        /* _unitValue */ \
                RTI_XCDR_DATA_REPRESENTATION_MASK_DEFAULT, /* _allowedDataRepresentationMask \
                                                            */ \
                RTIXCdrObservableAnnotation_INITIALIZER, /* _observable */ \
                RTIXCdrResourceAnnotation_INITIALIZER,   /* _resource */ \
                RTI_XCDR_TRUE,                           /* _isSerializable */ \
                RTI_FALSE,                               /* _isNested */ \
                RTI_FALSE,                               /* _isAutoIdHash */ \
                RTI_FALSE /* _isMustUnderstand */ \
    }

extern RTIXCdrDllExport 
void RTIXCdrTypeCodeAnnotations_initialize(
        struct RTIXCdrTypeCodeAnnotations *annotations);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_copy(
        struct RTIXCdrTypeCodeAnnotations *out,
        const struct RTIXCdrTypeCodeAnnotations *in);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrAnnotationParameterValue_equals(
        const RTIXCdrAnnotationParameterValue *first,
        const RTIXCdrAnnotationParameterValue *second);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_equals(
        const struct RTIXCdrTypeCodeAnnotations *first,
        const struct RTIXCdrTypeCodeAnnotations *second);

extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrTypeCodeAnnotations_isDefaultAndRangeConsistent(
        const struct RTIXCdrTypeCodeAnnotations *self,
        RTIXCdrBoolean isRequiredMember,
        RTIXCdrBoolean isPointerMember);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_hasNonDefaultRange(
        const struct RTIXCdrTypeCodeAnnotations *annotations);

/**
 * @brief Retrieves the value for the unit annotation.
 *
 * @param annotations The annotations struct to query.
 * @return A pointer to the unit string if set and non-empty, NULL otherwise.
 */
extern RTIXCdrDllExport RTIXCdrChar *RTIXCdrTypeCodeAnnotations_getUnit(
        const struct RTIXCdrTypeCodeAnnotations *annotations);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_hasMemberNonDefaultDefault(
        const struct RTIXCdrTypeCode *self,
        const struct RTIXCdrTypeCodeMember *member,
        const RTIXCdrUnsignedLong memberIndex);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_hasNonDefaultDefault(
        const struct RTIXCdrTypeCode *self,
        const RTIXCdrTypeCodeAnnotations *annotations);

/*
 * @brief Checks if 2 discriminator values, labelLeft and labelRight, identify 
 *        the same member in a union, tc. There are 3 categories of label values
 *        in a union:
 *    1. Explicit case label values -- these are explicitly listed cases in
 *       the union
 *    2. Case label values that identify the default case -- if the union
 *       contains a 'default' case label, then any non-explicit label values
 *       fall into this category.
 *    3. Unknown case label values -- if the union does NOT contain a
 *       'default' case label, then any non-explicit label values are
 *       considered unknown values
 * -----------------------------------------------------------------------------
 * When a union contains a 'default' case label, this function returns TRUE if 
 * both of the provided labels identify the same member. 
 *  
 * For example in this union: 
 *  
 * union switch (long) { 
 *     case 1:
 *     case 2:
 *         memberA;
 *     case 3:
 *     default:
 *         memberB;
 * }; 
 *  
 * These combinations will return true: 
 *     - 1,2 (both explicit labels that identify the same member) 
 *     - 3,5 (one explicit and one 'default' value that identify the same member)
 *     - 6,7 (two 'default' values that identify the same member)
 *  
 * These combinations will return false because one value identifies memberA and 
 * the other identifies memberB: 
 *     - 1,3 
 *     - 2,3 
 *     - 1,5 
 *     - 2,5 
 * -----------------------------------------------------------------------------
 * When a union does NOT contian a default case label, this function will return 
 * FALSE if either of the provided labels is an unknown label value. 
 * For example in this union: 
 *  
 * union switch (long) { 
 *     case 1:
 *     case 2:
 *         memberA;
 *     case 3:
 *         memberB;
 * }; 
 *  
 * The only combination that will return TRUE is 1,2. All other combinations 
 * will return FALSE.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_discValuesSelectSameMember(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrLong labelLeft,
        RTIXCdrLong labelRight);

extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_hasDefaultLabel(
        const struct RTIXCdrTypeCode *tc);

extern RTIXCdrDllExport void RTIXCdrAnnotationParameterValue_finalize(
        struct RTIXCdrAnnotationParameterValue *in);

extern RTIXCdrDllExport
void RTIXCdrTypeCodeAnnotations_finalize(
        struct RTIXCdrTypeCodeAnnotations *annotations);

typedef struct RTIXCdrTypeCodeMember {
    char *_name; /* struct, union, enum, value */
    struct RTIXCdrTypeCodeRepresentation _representation; /* struct, union, value */
    RTIXCdrLong _ordinal; /* enum */
    RTIXCdrUnsignedLong _labelsCount; /* union */
    RTIXCdrLong _label; /* union */
    RTIXCdrLong *_labels; /* union */
    RTIXCdrMemberFlags _memberFlags; /* struct, value */
    RTIXCdrVisibility _visibility; /* value: public or private */
    struct RTIXCdrTypeCodeAnnotations _annotations; /* default, min, max */
} RTIXCdrTypeCodeMember;

#define RTIXCdrTypeCodeMember_initialize(self__)                    \
    {                                                               \
        struct RTIXCdrTypeCodeAnnotations annotationsInitialValue = \
                RTIXCdrTypeCodeAnnotations_INITIALIZER;             \
        (self__)->_name = NULL;                                     \
        (self__)->_representation._pid = 0;                         \
        (self__)->_representation._isPointer = RTI_XCDR_FALSE;      \
        (self__)->_representation._bits = -1;                       \
        (self__)->_representation._typeCode = NULL;                 \
        (self__)->_ordinal = 0;                                     \
        (self__)->_labelsCount = 0;                                 \
        (self__)->_label = 0;                                       \
        (self__)->_labels = NULL;                                   \
        (self__)->_memberFlags = RTI_XCDR_FALSE;                    \
        (self__)->_visibility = (short) 0;                          \
        (self__)->_annotations = annotationsInitialValue;           \
    }

struct RTIXCdrInterpreterPrograms;

typedef struct RTIXCdrTypeCode {
    RTIXCdrLong _kind; /* All types, flags are or'd into this, use RTIXCdrTypeCode_getKind to access */
    RTIXCdrBoolean _isPointer; /* alias */
    RTIXCdrLong _default_index; /* union */
    char *_name; /* struct, union, enum, alias, value */
    struct RTIXCdrTypeCode *_typeCode; /* alias, sequence, array, union, value, struct */
    RTIXCdrUnsignedLong _maximumLength; /* string, wstring, sequence, array */
    RTIXCdrUnsignedLong _dimensionsCount; /* array */
    RTIXCdrUnsignedLong *_dimensions; /* array */
    RTIXCdrUnsignedLong _memberCount; /* struct, union, enum, value */
    struct RTIXCdrTypeCodeMember *_members; /* struct, union, enum, value */
    RTIXCdrValueModifier _typeModifier; /* value */
    struct RTIXCdrTypeCodeAnnotations _annotations; /* default, min, max, only for alias and enum at top level */
    /* 
     * To be used by the middleware. Indicates if a TypeCode is copyable
     * or not (constant). Non copyable typecodes are not copied and/or deleted
     * because they are consider and treated as constants.
     * You just can use them as references
     */
    RTIXCdrBoolean _isCopyable;

    /* Information used by the interpreter */
    /* This information is specific to a language binding */
    RTIXCdrSampleAccessInfo *_sampleAccessInfo;
    RTIXCdrTypePlugin *_typePlugin;
} RTIXCdrTypeCode;

#define RTIXCdrTypeCode_initialize(self__)                          \
    {                                                               \
        struct RTIXCdrTypeCodeAnnotations annotationsInitialValue = \
                RTIXCdrTypeCodeAnnotations_INITIALIZER;             \
        (self__)->_kind = RTI_XCDR_TK_NULL;                         \
        (self__)->_isPointer = RTI_XCDR_FALSE;                      \
        (self__)->_default_index = -1;                              \
        (self__)->_name = NULL;                                     \
        (self__)->_typeCode = NULL;                                 \
        (self__)->_maximumLength = 0;                               \
        (self__)->_dimensionsCount = 0;                             \
        (self__)->_dimensions = NULL;                               \
        (self__)->_memberCount = 0;                                 \
        (self__)->_members = NULL;                                  \
        (self__)->_typeModifier = RTI_XCDR_VM_NONE;                 \
        (self__)->_annotations = annotationsInitialValue;           \
        (self__)->_isCopyable = RTI_XCDR_TRUE;                      \
        (self__)->_sampleAccessInfo = NULL;                         \
        (self__)->_typePlugin = NULL;                               \
    }

/*
 * @brief The following function return RTI_XCDR_TRUE if the input TypeCode is a
 * struct/valuetype (or an alias to struct/valuetype) and its serialized
 * samples have a format that can potentially be directly copied into a C sample
 * for the type with a memcpy operation
 *
 * This property is interesting because it will allow to serialize/deserialize
 * structures/valuetypes or arrays/sequences of structures/valuetypes
 * with a single memcpy.
 *
 * For example:
 *
 * struct Point {
 *     long x;
 *     long y;
 * };
 *
 * The previous struct has a CDR layout friendly to C
 *
 * A struct/valuetype 'MyStruct' has a C friendly CDR layout when:
 * 1) MyStruct is marked as @final or @appendable when the data representation
 * is XCDR v1. Mutable structures are not C friendly.
 * 2) MyStruct does not have a base type
 * 3) MyStruct only contain primitive members or complex members composed only
 * of primitive members. A primitive member is a member with any of the
 * following types: int16, int32, int64, uint16, uint32, uint64, float, double,
 * octet, and char. The following primitive types are not currently supported
 * for inlining purposes: long double, wchar, boolean, enum.
 * 4) With any initial alignment (1, 2, 4, 8) greater than the maximum alignment 
 * across all the members of the struct, there is no padding between the members
 * that are part of MyStruct. To apply this rule consider the V1 alignments and
 * size for primitive types.
 * 5) With any initial alignment (1, 2, 4, 8) greater than the maximum alignment 
 * across all the members of the struct, there is no padding between the elements 
 * of an array of MyStruct.
 *
 * struct InlineStruct {
 *     char m1;
 *     long m2;
 * }; --> Not C friendly by 4)
 *
 * struct InlineStruct {
 *     long m1;
 *     short m2;
 * }; --> Not C friendly by 5)
 *
 * struct InlineStruct {
 *     long m1;
 *     short m2;
 *     short m3;
 * }; --> C frienly
 *
 * @param tc \b In. TypeCode
 *
 * @param elementCount \b In. Number of elements in an array of tc
 *
 * @param serSize \b Out. Serialized size of the inlinable struct
 *
 * @param alignment \b Out. Alignment required by the first member of the
 * struct/valuetype
 *
 * @param v2Encapsulation \b In. Indicates if the serialization format is
 * V1 or V2
 * 
 * @param dHeaderInNonPrimitiveCollection \b In. Indicates if non primitive
 * collections in V2 format must be serialized with a dheader. This is 
 * incompatible with a C friendly layout.
 *
 * @param enumAsPrimitiveInCollection \b In. Indicates if enums in collections
 * must be treated as primitives. This is incompatible with a C friendly layout.
 * This parameter is ignored when dHeaderInNonPrimitiveCollection is set to
 * RTI_XCDR_FALSE.
 *
 * @return RTI_XCDR_TRUE if the input TypeCode is a struct/valuetype (or an
 * alias to struct/valuetype) and its serialized samples have a format that can
 * potentially be directly copied into a C sample for the type with a memcpy
 * operation.
 */
extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrTypeCode_hasCFriendlyCdrLayout(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrUnsignedLongLong *serSize,
        RTIXCdrAlignment *alignment,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean dHeaderInNonPrimitiveCollection,
        RTIXCdrBoolean enumAsPrimitiveInCollection);

/*
 * @brief Indicates if the inout member is optional
 */
extern RTIXCdrDllExport 
RTIXCdrBoolean RTIXCdrTypeCodeMember_isOptional(
        const struct RTIXCdrTypeCodeMember *member);

/*
 * @brief Indicates if the specified member is must_understand
 */
extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrTypeCodeMember_isMustUnderstand(const RTIXCdrTypeCodeMember *self);

extern RTIXCdrDllExport 
RTIXCdrTCKind RTIXCdrTypeCode_getKind(const struct RTIXCdrTypeCode *tc);

extern RTIXCdrDllExport 
RTIXCdrUnsignedLong RTIXCdrTypeCode_getArrayElementCount(const struct RTIXCdrTypeCode *tc);

/**
 * @brief Return RTI_XCDR_TRUE if any of the TypeCode samples may require 
 * padding bytes at the end of the serialized data.
 *
 * For mutable types this function return RTI_XCDR_FALSE because the
 * samples of mutable types never require padding bytes at the end of the
 * serialized data.
 *
 * For all other types, this function will only return RTI_XCDR_FALSE if
 * none of the samples require padding bytes at the end of the serialized data 
 * For example:
 *
 *    struct Foo1 { <-- Samples for this type will always require padding 
 *       long m1;
 *       char m2;
 *    }
 *
 *    struct Foo2 { <-- Samples of this type will never require padding 
 *       long m1;
 *       double m2;
 *    }
 * 
 *   struct Foo3 { <-- Samples of this type may require padding
 *       long m1;
 *       sequence<octet> m2;
 *   }
 *
 * For Foo1, and Foo3 the function will return RTI_XCDR_TRUE while for Foo2
 * the function will return RTI_XCDR_FALSE.
 *
 * @param[in] tc TypeCode.
 * @param[in] v2Encapsulation Indicates if the serialization format is
 * V1 or V2.
 *
 * @return RTI_XCDR_TRUE if any of the TypeCode samples may require padding
 * bytes at the end of the serialized data. RTI_XCDR_FALSE otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_sampleMayRequirePadding(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrBoolean v2Encapsulation);

#define RTI_XCDR_INITIALIZE_STRING_TYPECODE(maximum__) \
    { \
        RTI_XCDR_TK_STRING,                     /* _kind */ \
        RTI_XCDR_FALSE,                         /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        NULL,                                   /* _typeCode */ \
        maximum__,                              /* _maximumLength */ \
        0,                                      /* _dimensionsCount */ \
        NULL,                                   /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        RTI_XCDR_TRUE,                          /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

#define RTI_XCDR_INITIALIZE_WSTRING_TYPECODE(maximum__) \
    { \
        RTI_XCDR_TK_WSTRING,                    /* _kind */ \
        RTI_XCDR_FALSE,                         /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        NULL,                                   /* _typeCode */ \
        maximum__,                              /* _maximumLength */ \
        0,                                      /* _dimensionsCount */ \
        NULL,                                   /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        RTI_XCDR_TRUE,                          /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

#define RTI_XCDR_INITIALIZE_SEQUENCE_TYPECODE(maximum__, typecode__) \
    { \
        RTI_XCDR_TK_SEQUENCE,                   /* _kind */ \
        RTI_XCDR_FALSE,                         /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        typecode__,                             /* _typeCode */ \
        maximum__,                              /* _maximumLength */ \
        0,                                      /* _dimensionsCount */ \
        NULL,                                   /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        RTI_XCDR_TRUE,                          /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

#define RTI_XCDR_INITIALIZE_ARRAY_TYPECODE( \
        dimensionsCount__, \
        dimension_1__, \
        dimensions__, \
        typecode__) \
    { \
        RTI_XCDR_TK_ARRAY,                      /* _kind */ \
        RTI_XCDR_FALSE,                         /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        typecode__,                             /* _typeCode */ \
        dimension_1__,                          /* _maximumLength */ \
        dimensionsCount__,                      /* _dimensionsCount */ \
        dimensions__,                           /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        RTI_XCDR_TRUE,                          /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

#define RTI_XCDR_INITIALIZE_ALIAS_TYPECODE(dimension__, typecode__, pointer__) \
    { \
        RTI_XCDR_TK_ALIAS,                      /* _kind */ \
        pointer__,                              /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        typecode__,                             /* _typeCode */ \
        dimension__,                            /* _maximumLength */ \
        0,                                      /* _dimensionsCount */ \
        NULL,                                   /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        RTI_XCDR_TRUE,                          /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

#define RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(kind__, copyable__) \
    { \
        kind__,                                 /* _kind */ \
        RTI_XCDR_FALSE,                         /* _isPointer */ \
        -1,                                     /* _default_index */ \
        NULL,                                   /* _name */ \
        NULL,                                   /* _typeCode */ \
        0,                                      /* _maximumLength */ \
        0,                                      /* _dimensionsCount */ \
        NULL,                                   /* _dimensions */ \
        0,                                      /* _memberCount */ \
        NULL,                                   /* _members */ \
        RTI_XCDR_VM_NONE,                       /* _typeModifier */ \
        RTIXCdrTypeCodeAnnotations_INITIALIZER, /* _annotations */ \
        copyable__,                             /* _isCopyable */ \
        NULL,                                   /* _sampleAccessInfo */ \
        NULL                                    /* _typePlugin */ \
    }

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_null;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_boolean;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_octet;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_int8;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_uint8;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_short;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ushort;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_long;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ulong;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_longlong;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_ulonglong;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_float;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_double;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_longdouble;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_char;

extern RTIXCdrDllVariable const RTIXCdrTypeCode RTIXCdr_g_tc_wchar;

/**
 * Property used by RTIXCdrTypeCode_equal to configure how the comparison works.
 */
typedef struct RTIXCdrTypeCodeCompareProperty {
    /* If RTI_XCDR_TRUE, annotations are ignored during comparison */
    RTIXCdrBoolean ignoreTypeCodeAnnotations;
    /* If RTI_XCDR_TRUE, type names are ignored during comparison */
    RTIXCdrBoolean ignoreTypeCodeNames;
} RTIXCdrTypeCodeCompareProperty;

#define RTIXCdrTypeCodeCompareProperty_INITIALIZER { \
    RTI_XCDR_FALSE, /* ignoreTypeCodeAnnotations */ \
    RTI_XCDR_FALSE /* ignoreTypeCodeNames */ \
}

/**
 * @brief Compares two TypeCodes for equality.
 *
 * This function checks if two given TypeCodes are equal. The comparison
 * can optionally ignore the annotations of the TypeCodes.
 *
 * @param[in] typecode1 The first TypeCode to compare.
 * @param[in] typecode2 The second TypeCode to compare.
 * @param[in] property Property that configures what we compare in the TypeCodes
 * @return RTI_XCDR_TRUE if the TypeCodes are equal, RTI_XCDR_FALSE otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property);

/**
 * @brief Destroys a TypeCode object.
 *
 * This function releases all resources associated with the given TypeCode
 * object. After calling this function, the TypeCode object should not be
 * used.
 *
 * @param[in] self Pointer to the TypeCode object to be destroyed.
 */
extern RTIXCdrDllExport
void RTIXCdrTypeCode_destroyTypeCode(
        RTIXCdrTypeCode *self);

/*
 * @brief Initializes the default and range annotations of a TypeCode as
 * appropriate
 */
extern RTIXCdrDllExport
void RTIXCdrTypeCode_initializeAnnotations(
        RTIXCdrTypeCode *self,
        RTIXCdrTCKind kind);

/*
 * @brief Initializes the default and range annotations of the given member
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrTypeCodeMember_initializeAnnotations(
        RTIXCdrTypeCodeMember *self);

/**
 * @brief Retrieves the ID of a member in the TypeCode.
 *
 * @param[in] self Pointer to the RTIXCdrTypeCodeMember structure.
 * @param[out] idOut Pointer to store the retrieved ID.
 *
 * @pre \p self must not be NULL.
 * @pre \p idOut must not be NULL.
 */
extern RTIXCdrDllExport
void RTIXCdrTypeCodeMember_getId(
        const RTIXCdrTypeCodeMember *self,
        RTIXCdrUnsignedLong *idOut);

/**
 * TypeCode node to break recursive type codes.
 */
typedef struct RTIXCdrResourceNode {
    const struct RTIXCdrResourceNode *prev;
    const void *resource;
    void *result;
} RTIXCdrResourceNode;

#define RTIXCdrResourceNode_INITIALIZER \
    { \
        NULL, /* prev */ \
        NULL, /* resource */ \
        NULL  /* result */ \
    }

/**
 * @brief Check if a node has been visited
 * @param resource \b In. The current resource.
 * @param parentVisitedNode \b In. The parent node.
 * @param result \b Out. The result of the function. Default is NULL.
 * @return RTI_TRUE if the node has been visited.
 *         RTI_FALSE if the node has not been visited.
 */
extern RTIXCdrDllExport
RTIBool RTIXCdrTypeCode_isResourceVisited(
        const void *resource,
        const RTIXCdrResourceNode *parentVisitedNode,
        void **result);

/**
 * @brief Indicates if a TypeCode kind corresponds to a primitive type.
 *
 * @param kind In. The TypeCode kind.
 * @param enumAsPrimitive In. Indicates if an enumeration is considered a
 * primitive type.
 *
 * @return RTI_TRUE if the TypeCode kind corresponds to a primitive type.
 */
extern RTIXCdrDllExport RTIXCdrBoolean RTIXCdrTypeCode_isPrimitiveKind(
        RTIXCdrTCKind kind,
        RTIXCdrBoolean enumAsPrimitive);

#ifdef __cplusplus
    }	/* extern "C" */
#endif


/*
 * When using rtiddsgen generated code below dds_c.1.0 a wrapper around
 * DDS_TypeCode is required in order to serialize samples. See the implementation
 * of RTIXCdrTypeCodeWrapper_createSerializationPrograms.
 */
typedef struct RTIXCdrTypeCodeWrapper {
    RTIXCdrTypeCode _data;
} RTIXCdrTypeCodeWrapper;

#include "xcdr/xcdr_typeCode_impl.h"

#endif /* xcdr_typeCode_h */
