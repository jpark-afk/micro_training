/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_interpreter.h"

struct RTIXCdrTypeCodeNode;

typedef struct RTIXCdrTypeCodeNode {
    const struct RTIXCdrTypeCodeNode *prev;
    const RTIXCdrTypeCode *tc;
} RTIXCdrTypeCodeNode;

extern RTIXCdrBoolean RTIXCdrTypeCode_isCdrRepresentation(
        const RTIXCdrTypeCode *tc);

extern RTIXCdrBoolean RTIXCdrTypeCode_isCollectionKind(RTIXCdrTCKind kind);

extern RTIXCdrBoolean RTIXCdrTypeCode_isAggregationKind(RTIXCdrTCKind kind);

extern void RTIXCdrTypeCode_getMemberOrdinal(
        const struct RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong memberIndex,
        RTIXCdrLong *ordinal);

extern const struct RTIXCdrTypeCode *RTIXCdrTypeCode_getMemberTypeCode(
        const struct RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong memberIndex);

/**
 * @brief Resolves any alias type codes to their underlying type code.
 *
 * This function takes a type code and resolves any alias type codes
 * to their underlying type code. If the input type code is not an alias,
 * it is returned as is. If the input type code is an alias, the function
 * follows the alias chain until it reaches a non-alias type code.
 *
 * @pre \p tc must not be NULL.
 *
 * @param[in] tc The input type code to resolve.
 * @return The resolved type code.
 */
 extern const RTIXCdrTypeCode *RTIXCdrTypeCode_resolveAlias(
        const RTIXCdrTypeCode *tc);

/**
 * @brief Resolves any alias type codes to their underlying type code, stopping
 * at pointers.
 *
 * This function takes a type code and resolves any alias type codes
 * to their underlying type code. If the input type code is not an alias,
 * it is returned as is. If the input type code is an alias, the function
 * follows the alias chain until it reaches a non-alias type code or a pointer.
 *
 * @pre \p tc must not be NULL.
 *
 * @param[in] tc The input type code to resolve.
 * @return The resolved type code.
 */
extern const RTIXCdrTypeCode *RTIXCdrTypeCode_resolveAliasUntilPointer(
        const RTIXCdrTypeCode *tc);

extern RTIXCdrTypeCode *RTIXCdrTypeCode_getContentType(
        const RTIXCdrTypeCode *tc);

extern RTIXCdrExtensibilityKind RTIXCdrTypeCode_getExtensibilityKind(
        const RTIXCdrTypeCode *tc);

#define RTI_XCDR_TYPECODE_INVALID_INDEX 0xFFFFFFFF

extern void RTIXCdrTypeCode_getUnionMemberIndex(
        RTIXCdrUnsignedLong *memberIndex,
        const RTIXCdrTypeCode *tc, 
        RTIXCdrLong caseValue);

/* 
 * Get the total member count. This method is useful when there is a type that
 * uses inheritence to count all of the members including the base types. In
 * the case of structs with no base type or unions, tc->_memberCount is
 * returned. This method should only be used for:
 * - RTI_XCDR_TK_STRUCT
 * - RTI_XCDR_TK_VALUE
 * - RTI_XCDR_TK_UNION
 * - RTI_XCDR_TK_ENUM
 */
extern void RTIXCdrTypeCode_getMemberCount(
        RTIXCdrUnsignedLong *memberCount,
        const RTIXCdrTypeCode *tc);

/* 
 * Get the type that contains the given index. This is used for cases where
 * there is inheritence to find the enclosing typecode of a given member
 */
extern void RTIXCdrTypeCode_getTypeContainingIndex(
        const RTIXCdrTypeCode **tcOut,
        const RTIXCdrTypeCode *tcIn,
        RTIXCdrUnsignedLong totalMemberCount, 
        RTIXCdrUnsignedLong index);

/*
 * @brief Select the default discriminator for a union.
 * If useDiscriminatorDefault is set to true, the default value of the
 * discriminator type is used.
 * If it is set to false, if there is a 'default' label then this method
 * will pick the first unused value given the type of the discriminator.
 * For example, if the discriminator type is an enumeration, the first
 * ordinal (regardless of the associated value) that is not explicitly
 * used as a case label will be chosen. For other types, beginning at 0,
 * the first value that is found to not be used explitly as a case label
 * will be selected.
 * If there is no default label then the case label with the smallest value is
 * chosen as the default.
 */
extern RTIXCdrDllExport RTIXCdrBoolean
RTIXCdrTypeCode_selectDefaultDiscriminator(
        const struct RTIXCdrTypeCode *typeCode,
        RTIXCdrLong *discValue,
        RTIXCdrBoolean useDiscriminatorDefault);

/*
 * @brief Indicates if a TypeCode has a keyed member. For structs inheriting
 * from other structs this function considers the base
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_hasKey(const struct RTIXCdrTypeCode *tc);

/*
 * @brief Gets alignment and size information about a primitive member
 */
extern void RTIXCdrTypeCode_getPrimitiveInfo(
        RTIXCdrTCKind kind,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrAlignment *alignment,
        RTIXCdrOctet *primitiveSize);

/*
 * @brief Gets the number of optional members in a STRUCT
 */
extern RTIXCdrUnsignedLong RTIXCdrTypeCode_getOptionalMemberCount(
        const RTIXCdrTypeCode *tc);

/*
 * @brief Indicates if the member of a STRUCT is optional
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_isOptionalMember(
        const RTIXCdrTypeCode *tc,
        RTIXCdrUnsignedLong index);

/* 
 * @brief Indicates if an annotation parameter value is non zero.
 */ 
extern RTIXCdrBoolean RTIXCdrAnnotationParameterValue_isNonZero(
        const RTIXCdrAnnotationParameterValue *value);

/* 
 * @brief Indicates if a member has a non-zero default value 
 */ 
extern RTIXCdrBoolean RTIXCdrTypeCodeMember_hasNonZeroDefault(
        const RTIXCdrTypeCodeMember *tcMember);

extern void RTIXCdrTypeCode_getFirstMemberAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrAlignment *alignment,
        RTIXCdrBoolean v2Encapsulation);

extern void RTIXCdrTypeCode_getMaxMemberAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrAlignment *alignment,
        RTIXCdrBoolean v2Encapsulation);

extern RTIXCdrUnsignedLong RTIXCdrTypeCode_getLabelCount(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean includeDefault);

extern RTIXCdrBoolean RTIXCdrTypeCode_hasBase(const RTIXCdrTypeCode *tc);

/*
 * @brief Indicates if tc is part of the list whose last element is 
 * parentVisitedNode.
 * 
 * This function is used to detect type circular dependencies.
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_isTypeCodeVisited(
        const RTIXCdrTypeCode *tc,
        const RTIXCdrTypeCodeNode *parentVisitedNode);

/*
 * @brief Indicates if a TypeCode is unbounded. A Typecode is unbounded if
 * any of the following conditions is TRUE:
 * 
 * - It has an unbounded sequence
 * - It has an unbounded string
 * - There is a cycle
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_isUnbounded(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean keyOnly,
        RTIXCdrUnsignedLong unboundedSize);

/*
 * @brief Indicates if a TypeCode contains any optional members at any level
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_hasOptionals(const RTIXCdrTypeCode *tc);

/**
 * @brief Indicates if a TypeCode contains any members that are serialized
 * with a member header. This is happening because the member are optional
 * or because they are mutable.
 *
 * @param tc In. The TypeCode.
 *
 * @return RTI_TRUE if the TypeCode contains any member header.
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_hasMemberHeaders(
        const RTIXCdrTypeCode *tc);

/*
 * @brief Indicates if a TypeCode represents a type whose memory representation
 * always takes the same size. This is relevant for a binding such that
 * RTIXCdrSampleAccessInfo_isContiguousMemoryBinding is false (i.e. C) because
 * when this function is true, the memory is guaranteed to be contiguous.
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_isFixedSize(const RTIXCdrTypeCode *tc);

/*
 * @brief Indicates if a TypeCode represents a type whose memory representation
 * always takes the same size and it does not have primitive members
 * using the @default annotation with a non-zero value.
 */
extern RTIXCdrBoolean RTIXCdrTypeCode_isFixedSizeWithZeroDefault(
        const RTIXCdrTypeCode *tc);

extern RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_hasNonDefaultRangeMinMax(
        const struct RTIXCdrTypeCodeAnnotations *annotations,
        RTIXCdrBoolean *hasNonDefaultMin,
        RTIXCdrBoolean *hasNonDefaultMax);

extern RTIXCdrBoolean RTIXCdrTypeCode_hasNonDefaultRangeAnnotation(
        const struct RTIXCdrTypeCode *tc);

extern RTIXCdrBoolean RTIXTypeCode_useSampleAccessor(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean isOptional);

/*
 * @brief Returns the number of aggregation TypeCodes (union, valuetype, struct)
 * referenced by tc. 
 * 
 * This method does not consider TypeCode equality when reporting the result.
 * Therefore, if the same TypeCode is referenced n times this method will
 * increase the result n times.
 * 
 * @param tc In. The TypeCode.
 * @return Number of aggregation TypeCodes.
 */
extern 
RTIXCdrUnsignedLong RTIXCdrTypeCode_getAggregationTypeCodeCount(
        const RTIXCdrTypeCode *tc);

/*
 * @brief Returns the number of non-primitive collection members
 * (arrays or sequences) in an aggregated, collection, or alias type.
 *
 * @param enumAsPrimitive. If RTI_FALSE, enums are considered non-primitive
 * members.
 * @param resolveAlias. Indicates if aliases have to be resolved for the member
 * types. For example:
 *
 * typedef sequence<Foo, 3> MyFooSeq;
 *
 * struct MyType {
 *     MyFooSeq m1;
 * };
 *
 * The call to this function with resolveAlias set to TRUE returns 1.
 * The call to this function with resolveAlias set to FALSE returns 0.
 */
extern 
RTIXCdrUnsignedLong RTIXCdrTypeCode_getNonPrimitiveCollectionMemberCount(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean enumAsPrimitive,
        RTIXCdrBoolean resolveAlias);

/* ------------------------------------------------------------------------- */
/* ---- Implementation ----------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrTypeCode_isCollectionKind(kind) \
    ((kind) == RTI_XCDR_TK_SEQUENCE \
            || (kind) == RTI_XCDR_TK_ARRAY)

#define RTIXCdrTypeCode_isAggregationKind(kind__) \
    ((kind__) == RTI_XCDR_TK_STRUCT \
    || (kind__) == RTI_XCDR_TK_VALUE \
    || (kind__) == RTI_XCDR_TK_UNION)

#define RTIXCdrTypeCode_getUnionMemberIndexMacro(memberIndex, tc, caseValue) \
{ \
    RTIXCdrUnsignedLong mc; \
    RTIXCdrUnsignedLong lc; \
    RTIXCdrBoolean __found = RTI_XCDR_FALSE; \
 \
    *(memberIndex) = RTI_XCDR_TYPECODE_INVALID_INDEX; \
    for (mc=0; mc<(tc)->_memberCount; mc++) { \
        if (((RTIXCdrLong)mc) != (tc)->_default_index) { \
            if ((tc)->_members[mc]._labelsCount == 1) { \
                if ((caseValue) == (tc)->_members[mc]._label) { \
                    *(memberIndex) = mc; \
                    break; \
                } \
            } else { \
                for (lc=0; lc<(tc)->_members[mc]._labelsCount; lc++) { \
                    if ((caseValue) == (tc)->_members[mc]._labels[lc]) { \
                        *(memberIndex) = mc; \
                        __found = RTI_XCDR_TRUE; \
                        break; \
                    } \
                } \
                if (__found) { \
                    break; \
                } \
            }\
        } \
    } \
 \
    if (*(memberIndex) == RTI_XCDR_TYPECODE_INVALID_INDEX && \
            (tc)->_default_index >= 0) { \
        *(memberIndex) = (RTIXCdrUnsignedLong)((tc)->_default_index); \
    } \
}

#define RTIXCdrTypeCode_getContentType(tc) ((tc)->_typeCode)

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrTypeCode_getUnionMemberIndex \
    RTIXCdrTypeCode_getUnionMemberIndexMacro
#endif

#define RTIXCdrTypeCode_getMemberCountMacro(memberCount, tc) \
{ \
    RTIXCdrTCKind kind = RTIXCdrTypeCode_getKind((tc)); \
 \
    if (kind == RTI_XCDR_TK_STRUCT || kind == RTI_XCDR_TK_UNION || kind == RTI_XCDR_TK_ENUM) { \
        *(memberCount) = (tc)->_memberCount; \
    } else if (kind == RTI_XCDR_TK_VALUE) { \
        const RTIXCdrTypeCode *typeCode = tc; \
        while (typeCode != NULL && kind != RTI_XCDR_TK_NULL) { \
            *(memberCount) += typeCode->_memberCount; \
            typeCode = typeCode->_typeCode == NULL ? \
                    NULL : RTIXCdrTypeCode_resolveAlias(typeCode->_typeCode); \
            if (typeCode != NULL) { \
                kind = RTIXCdrTypeCode_getKind(typeCode); \
            } \
        } \
    } else { \
        *(memberCount) = 0; \
    } \
}

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrTypeCode_getMemberCount \
    RTIXCdrTypeCode_getMemberCountMacro
#endif

#define RTIXCdrTypeCode_getMemberOrdinalMacro(self, memberIndex, ordinal) \
{ \
    *(ordinal) = (self)->_members[(memberIndex)]._ordinal; \
}

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrTypeCode_getMemberOrdinal \
    RTIXCdrTypeCode_getMemberOrdinalMacro
#endif

#define RTIXCdrTypeCode_getMemberTypeCodeMacro(self, memberIndex) \
    (self)->_members[(memberIndex)]._representation._typeCode;

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrTypeCode_getMemberTypeCode \
    RTIXCdrTypeCode_getMemberTypeCodeMacro
#endif

#define RTIXCdrTypeCode_getTypeContainingIndexMacro(                        \
        tcOut__,                                                            \
        tcIn__,                                                             \
        totalMemberCount__,                                                 \
        index__)                                                            \
    {                                                                       \
        const RTIXCdrTypeCode *typeCodeTmp = (tcIn__);                      \
        RTIXCdrUnsignedLong memberCountTmp = 0;                             \
                                                                            \
        while (typeCodeTmp != NULL) {                                       \
            memberCountTmp += typeCodeTmp->_memberCount;                    \
            if ((index__) >= ((totalMemberCount__) -memberCountTmp)) {      \
                *(tcOut__) = typeCodeTmp;                                   \
                break;                                                      \
            }                                                               \
                                                                            \
            typeCodeTmp = typeCodeTmp->_typeCode == NULL                    \
                    ? NULL                                                  \
                    : RTIXCdrTypeCode_resolveAlias(typeCodeTmp->_typeCode); \
        }                                                                   \
    }

#ifndef RTI_DISABLE_FUNCTION_MACROS
#define RTIXCdrTypeCode_getTypeContainingIndex \
    RTIXCdrTypeCode_getTypeContainingIndexMacro
#endif

#define RTIXCdrTypeCode_hasBase(tc) \
    ( \
        RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE && \
        (tc)->_typeCode != NULL && \
        RTIXCdrTypeCode_getKind((tc)->_typeCode) != RTI_XCDR_TK_NULL \
    )
    
#define RTIXCdrTypeCode_getPrimitiveInfo( \
        kind, \
        v2Encapsulation, \
        alignment, \
        primitiveSize) \
    *(alignment) = RTIXCdr_TCKind_g_primitiveCdrAlignments[(v2Encapsulation)][(kind)]; \
    *(primitiveSize) = (RTIXCdrOctet) RTIXCdr_TCKind_g_primitiveCdrSizes[(v2Encapsulation)][(kind)]

#define RTIXCdrTypeCode_isCdrRepresentation(tc) \
        ((tc)->_kind & (RTIXCdrLong)(0x80000000|0x00000080))
