/*
(c) Copyright, Real-Time Innovations, 2014-2017.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_rtiddsgen_infrastructure.h"
#include "../infrastructure/Infrastructure.h"
#include "TypeCode.h"

const RTIXCdrMemberValue RTI_XCDR_MEMBER_VALUE_INVALID =
        RTIXCdrMemberValue_INITIALIZER;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

const RTIXCdrMemberValue RTI_XCDR_MEMBER_VALUE_NIL = {
        RTI_XCDR_TRUE, /* isNull */
        RTI_XCDR_FALSE, /* isDiscontiguous */
        { /* Initializing the largest member in the */
          /* union ensures that all members get */
          /* initialized to 0 (PLATFORMS-2155) */
                .llVal = 0ll
        }
};

#else

const RTIXCdrMemberValue RTI_XCDR_MEMBER_VALUE_NIL = { 
        RTI_XCDR_TRUE, /* isNull */
        RTI_XCDR_FALSE, /* isDiscontiguous */
        { 0 } 
};

#endif

const RTIXCdrTypeCode RTIXCdr_g_tc_null =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(
                RTI_XCDR_TK_NULL,
                RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_boolean =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_BOOLEAN, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_octet =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(
                RTI_XCDR_TK_OCTET,
                RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_int8 =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_INT8, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_uint8 =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_UINT8, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_short =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_SHORT, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_ushort =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_USHORT, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_long =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_LONG, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_ulong =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_ULONG, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_longlong =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_LONGLONG, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_ulonglong =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_ULONGLONG, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_float =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_FLOAT, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_double =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_DOUBLE, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_longdouble =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_LONGDOUBLE, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_char =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_CHAR, RTI_XCDR_FALSE);

const RTIXCdrTypeCode RTIXCdr_g_tc_wchar =
        RTI_XCDR_INITIALIZE_PRIMITIVE_TYPECODE(RTI_XCDR_TK_WCHAR, RTI_XCDR_FALSE);

RTIXCdrUnsignedLong RTIXCdrTypeCode_getArrayElementCount(
        const struct RTIXCdrTypeCode *tc)
{
    RTIXCdrUnsignedLongLong elementCount = 1;

    RTIXCdrLog_testPrecondition(tc == NULL, return 0);
    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(tc) != RTI_XCDR_TK_ARRAY,
            return 0);
    RTIXCdrLog_testPrecondition(tc->_dimensionsCount == 0, return 0);

    if (tc->_dimensionsCount == 1) {
        elementCount = tc->_maximumLength;
    } else {
        RTIXCdrUnsignedLong i = 0;
        for (i = 0; i < tc->_dimensionsCount; i++) {
            elementCount *= tc->_dimensions[i];
        }
    }

    if (elementCount > RTIXCdrLong_MAX) {
        return 0;
    }

    return (RTIXCdrUnsignedLong)elementCount;
}

const RTIXCdrTypeCode *RTIXCdrTypeCode_resolveAlias(const RTIXCdrTypeCode *tc)
{
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    kind = RTIXCdrTypeCode_getKind(tc);

    while (kind == RTI_XCDR_TK_ALIAS) {
        tc = tc->_typeCode;
        kind = RTIXCdrTypeCode_getKind(tc);
    }

    return tc;
}

const RTIXCdrTypeCode *RTIXCdrTypeCode_resolveAliasUntilPointer(
        const RTIXCdrTypeCode *tc)
{
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);

    kind = RTIXCdrTypeCode_getKind(tc);

    while (kind == RTI_XCDR_TK_ALIAS) {
        if (tc->_isPointer) {
            break;
        }
        tc = tc->_typeCode;
        kind = RTIXCdrTypeCode_getKind(tc);
    }

    return tc;
}

RTIXCdrExtensibilityKind RTIXCdrTypeCode_getExtensibilityKind(
        const RTIXCdrTypeCode *tc)
{
    RTIXCdrExtensibilityKind extKind =
            RTI_XCDR_EXTENSIBLE_EXTENSIBILITY;
    RTIXCdrTCKind kind =
            RTI_XCDR_TK_NULL;

    RTIXCdrLog_testPrecondition(
            tc == NULL,
            return RTI_XCDR_FINAL_EXTENSIBILITY);

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ARRAY:
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_STRING:
        case RTI_XCDR_TK_WSTRING:
        {
            extKind = RTI_XCDR_MUTABLE_EXTENSIBILITY;
        } break;
        case RTI_XCDR_TK_ALIAS:
        {
            extKind = RTIXCdrTypeCode_getExtensibilityKind(tc->_typeCode);
        } break;
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_UNION:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_ENUM:
        {
            if (tc->_kind & RTI_XCDR_TK_FLAGS_IS_FINAL) {
                extKind = RTI_XCDR_FINAL_EXTENSIBILITY;
            } else if (tc->_kind & RTI_XCDR_TK_FLAGS_IS_MUTABLE) {
                extKind = RTI_XCDR_MUTABLE_EXTENSIBILITY;
            }
        } break;
        default:
        {
            extKind = RTI_XCDR_FINAL_EXTENSIBILITY;
        } break;
    }

    return extKind;
}

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrTypeCode_isValidEnumValue(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean *isValid,
        RTIXCdrEnum value)
{
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            return);
    RTIXCdrLog_testPrecondition(
            isValid == NULL,
            return);

    RTIXCdrTypeCode_isValidEnumValueMacro(tc, isValid, value);
}
#endif

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrTypeCode_getUnionMemberIndex(
        RTIXCdrUnsignedLong *memberIndex,
        const RTIXCdrTypeCode *tc, 
        RTIXCdrLong caseValue) 
{
    RTIXCdrLog_testPrecondition(
            memberIndex == NULL,
            return);
    RTIXCdrLog_testPrecondition(
            tc == NULL,
            return);
 
    RTIXCdrTypeCode_getUnionMemberIndexMacro(memberIndex, tc, caseValue);
}
#endif

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrTypeCode_getMemberCount(
        RTIXCdrUnsignedLong *memberCount,
        const RTIXCdrTypeCode *tc) 
{
    RTIXCdrLog_testPrecondition(memberCount == NULL, return);
    RTIXCdrLog_testPrecondition(tc == NULL, return);
 
    RTIXCdrTypeCode_getMemberCountMacro(memberCount, tc);
}
#endif

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrTypeCode_getTypeContainingIndex(
        const RTIXCdrTypeCode **tcOut,
        const RTIXCdrTypeCode *tcIn,
        RTIXCdrUnsignedLong totalMemberCount, 
        RTIXCdrUnsignedLong index)
{
    RTIXCdrTypeCode_getTypeContainingIndexMacro(
            tcOut, 
            tcIn, 
            totalMemberCount, 
            index);
}
#endif

#ifdef RTI_DISABLE_FUNCTION_MACROS
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_checkMemberIndexBounds(
        const struct RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong memberIndex)
{
    RTIXCdrUnsignedLong memberCount = 0;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);

    RTIXCdrTypeCode_getMemberCount(&memberCount, self);

    /* Check that memberIndex isn't out of bounds. */
    return (memberIndex < memberCount);
}
#endif /* RTI_DISABLE_FUNCTION_MACROS */

#ifdef RTI_DISABLE_FUNCTION_MACROS
void RTIXCdrTypeCode_getMemberOrdinal(
        const struct RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong memberIndex,
        RTIXCdrLong *ordinal)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(ordinal == NULL, return);
    RTIXCdrLog_testPrecondition(
            !RTIXCdrTypeCode_checkMemberIndexBounds(self, memberIndex),
            return);

    *ordinal = self->_members[memberIndex]._ordinal;
}
#endif /* RTI_DISABLE_FUNCTION_MACROS */

#ifdef RTI_DISABLE_FUNCTION_MACROS
RTI_PRIVATE
const struct RTIXCdrTypeCode *RTIXCdrTypeCode_getMemberTypeCode(
        const struct RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong memberIndex)
{
    RTIXCdrLog_testPrecondition(self == NULL, return NULL);
    RTIXCdrLog_testPrecondition(
            !RTIXCdrTypeCode_checkMemberIndexBounds(self, memberIndex),
            return NULL);

    return self->_members[memberIndex]._representation._typeCode;
}
#endif /* RTI_DISABLE_FUNCTION_MACROS */

RTIXCdrBoolean RTIXCdrTypeCode_selectDefaultDiscriminator(
        const struct RTIXCdrTypeCode *typeCode,
        RTIXCdrLong *discValue,
        RTIXCdrBoolean useDiscriminatorDefault)
{
    const RTIXCdrTypeCode *discTc = NULL;
    RTIXCdrTypeCode *elementTc = NULL;
    RTIXCdrUnsignedLong numMembers = 0;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrUnsignedLong j = 0;
    RTIXCdrUnsignedLong labelCount = 0;
    RTIXCdrLong defaultIndex = (RTIXCdrLong)RTI_XCDR_TYPECODE_INVALID_INDEX;
    RTIXCdrLong label = 0;
    RTIXCdrLong enumOrdinal = 0;
    RTIXCdrTCKind discKind = RTI_XCDR_TK_NULL;
    RTIXCdrBoolean useSmallerLabel = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(typeCode == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(discValue == NULL, return RTI_XCDR_FALSE);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(typeCode) != RTI_XCDR_TK_UNION, 
            return RTI_XCDR_FALSE);
 
    numMembers = typeCode->_memberCount;
    defaultIndex = typeCode->_default_index;

    discTc = RTIXCdrTypeCode_resolveAlias(typeCode->_typeCode);
    if (discTc == NULL) {
        return RTI_XCDR_FALSE;
    }

    discKind = RTIXCdrTypeCode_getKind(discTc);

    if (useDiscriminatorDefault) {
        /**
         * Obtain the default value of the discriminator type.
         * This might be an explicit default value of a typedef
         * or a @default_literal enum annotation.
         */
        *discValue = discTc->_annotations._defaultValue._u.long_value;
        return RTI_XCDR_TRUE;
    }

    /* Start: there is default label */
    if (defaultIndex != -1) { 
        RTIXCdrUnsignedLong k = 0;
        RTIXCdrBoolean skipK;

        if (discKind == RTI_XCDR_TK_ENUM) {
            RTIXCdrUnsignedLong enumElementCount = discTc->_memberCount;

            for (k = 0; k < enumElementCount; k++) {
                RTIXCdrTypeCode_getMemberOrdinal(discTc, k, &enumOrdinal);
                skipK = RTI_XCDR_FALSE;
                for (i = 0; (i < numMembers && skipK == RTI_XCDR_FALSE); i++) {
                    /* For each member of the union i get the number of label */
                    labelCount = typeCode->_members[i]._labelsCount;

                    /* For each label of this element i check that
                       it is the smaller*/
                    for (j = 0; j < labelCount; j++) {  
                        if (labelCount == 1) {
                            label = typeCode->_members[i]._label;
                        } else {
                            label = typeCode->_members[i]._labels[j];
                        }

                        if (enumOrdinal == label) {
                            skipK = RTI_XCDR_TRUE;
                            break;
                        }
                    }
                }
                if (skipK == RTI_XCDR_FALSE) {
                    break;
                }
            }

            /* All the enum valid value are used in the union cases */
            if (k == enumElementCount) {
                /* I do not expect to reach this point since this means
                 * that we have a default label when all the possible
                 * case values are already covered
                 */
                /* have to set the discriminator to the lowest enum value*/
                useSmallerLabel = RTI_XCDR_TRUE;
            } else {
                /* 
                 * enumOrdinal is the (unused) enum value I want to use for the
                 * default
                 */
                *discValue = enumOrdinal;
            }
        } else {
            /* Find a value that is different from any label */
            for (k = 0; k < 0xFFFFFFFF; k++) {
                skipK = RTI_XCDR_FALSE;
                for (i = 0; (i < numMembers && skipK == RTI_XCDR_FALSE); i++) {
                    /* For each member of the union i get the number of label*/
                    labelCount = typeCode->_members[i]._labelsCount;

                    /* For each label of this element i check that
                       it is the smaller*/
                    for (j = 0; j < labelCount; j++) {  
                        if (labelCount == 1) {
                            label = typeCode->_members[i]._label;
                        } else {
                            label = typeCode->_members[i]._labels[j];
                        }

                        if ((RTIXCdrLong)k == label) {
                            skipK = RTI_XCDR_TRUE;
                            break;
                        }
                    }
                }

                if (skipK == RTI_XCDR_FALSE) {
                    break;
                }
            }

            *discValue = (RTIXCdrLong)k;
        }

        if (useSmallerLabel != RTI_XCDR_TRUE) {
            elementTc = typeCode->_members[defaultIndex]._representation._typeCode;
        }
    }
    /* End: there is default label */

    /* Start: there is no default label */
    if (defaultIndex == -1 || useSmallerLabel) {
        for (i = 0; i < numMembers; i++) {
            /* For each member of the union i get the number of label*/
            labelCount = typeCode->_members[i]._labelsCount;

            /* For each label of this element i check that
            it is the smallest */
            for (j = 0; j < labelCount; j++) {
                if (labelCount == 1) {
                    label = typeCode->_members[i]._label;
                } else {
                    label = typeCode->_members[i]._labels[j];
                }

                if ((j == 0 && i == 0) || (label < *discValue)) {
                    elementTc = typeCode->_members[i]._representation._typeCode;
                    *discValue = label;
                }
            }
        }

        if (elementTc == NULL) {
            return RTI_XCDR_FALSE;
        }
    }
    /* End: there is no default label */

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCode_hasKey(const struct RTIXCdrTypeCode *tc) {
    RTIXCdrUnsignedLong i;
    
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    
    if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_UNION) {
        return RTI_XCDR_FALSE;
    }
    
    if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE) {
        if (tc->_typeCode != NULL) {
            if (RTIXCdrTypeCode_getKind(tc->_typeCode) != RTI_XCDR_TK_NULL) {
                if (RTIXCdrTypeCode_hasKey(tc->_typeCode)) {
                    return RTI_XCDR_TRUE;
                }
            }
        }
    }
    
    for (i=0; i<tc->_memberCount; i++) {
        if (tc->_members[i]._memberFlags & RTI_XCDR_KEY_MEMBER) {
            return RTI_XCDR_TRUE;
        }
    }
    
    return RTI_XCDR_FALSE;
}

typedef enum RTIXCdrTypeCodeMemberAlignmentKind {
    RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT,
    RTI_XCDR_TYPECODE_MAX_MEMBER_ALIGNMENT
} RTIXCdrTypeCodeMemberAlignmentKind;

/*
 * This functions gets a member alignment in a CDR stream
 */
RTI_PRIVATE
void RTIXCdrTypeCode_getMemberAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrAlignment *alignment,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrTypeCodeMemberAlignmentKind alignmentKind,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrOctet primitiveSize = 0;
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrTypeCodeNode thisNode;

    RTIXCdrLog_testPrecondition(tc == NULL, return);
    RTIXCdrLog_testPrecondition(alignment == NULL, return);
    
    *alignment = -1;

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        return;
    }

    thisNode.tc = tc;
    thisNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);
    
    if (kind == RTI_XCDR_TK_ALIAS) {
        tc = RTIXCdrTypeCode_resolveAlias(tc);
        kind = RTIXCdrTypeCode_getKind(tc);
    }
    
    switch (kind) {
        case RTI_XCDR_TK_ARRAY:
        {
            RTIXCdrTypeCode_getFirstMemberAlignment(
                    tc->_typeCode,
                    alignment,
                    v2Encapsulation);
        } break;
        case RTI_XCDR_TK_UNION:
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_VALUE:
        {
            RTIXCdrUnsignedLong i;
            RTIXCdrAlignment tmpAlignment = -1;

            if (kind == RTI_XCDR_TK_UNION) {
                RTIXCdrTypeCode_getFirstMemberAlignment(
                        tc->_typeCode,
                        alignment,
                        v2Encapsulation);

                if (alignmentKind == RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT) {
                    /* For a union the first member is the discriminator */
                    return;
                }
            } if (kind == RTI_XCDR_TK_VALUE) {
                if (tc->_typeCode != NULL) {
                    if (RTIXCdrTypeCode_getKind(tc->_typeCode) != RTI_XCDR_TK_NULL) {
                        RTIXCdrTypeCode_getMemberAlignment(
                                tc->_typeCode,
                                alignment,
                                v2Encapsulation,
                                alignmentKind,
                                &thisNode);

                        /* Alignment will be -1 if the base class is empty */
                        if (*alignment != -1
                                && alignmentKind == RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT) {
                            return;
                        }
                    }
                }
            }
            
            /* We iterator through all the members because a member can have
             * a type that is an empty structure
             */
            for (i=0; i<tc->_memberCount; i++) {
                RTIXCdrTypeCode_getMemberAlignment(
                        tc->_members[i]._representation._typeCode,
                        &tmpAlignment,
                        v2Encapsulation,
                        alignmentKind,
                        &thisNode);

                if (tmpAlignment != -1
                        && alignmentKind == RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT) {
                    *alignment = tmpAlignment;
                    return;
                } else if (tmpAlignment > *alignment) {
                    *alignment = tmpAlignment;
                }
            }
        } break;
        case RTI_XCDR_TK_STRING:
        case RTI_XCDR_TK_WSTRING:
        {
            /* 
             * Regarding this function, strings are considered as a type
             * with two members:
             * + the length member
             * + a primitive member (TK_CHAR for strings and TK_WCHAR 
             * for wstrings)
             * 
             * The alignment of the second member is always smaller or equal to
             * the alignment of the first member.
             */

            /* String are serialized with a length first */
            RTIXCdrTypeCode_getPrimitiveInfo(
                    RTI_XCDR_TK_LONG,
                    v2Encapsulation,
                    alignment,
                    &primitiveSize);
        } break;
        case RTI_XCDR_TK_SEQUENCE:
        {
            RTIXCdrAlignment tmpAlignment = -1;

            /* 
             * Regarding this function, sequences are considered as a type
             * with two members:
             * + the length member
             * + the sequence element member
             */
            RTIXCdrTypeCode_getPrimitiveInfo(
                    RTI_XCDR_TK_LONG,
                    v2Encapsulation,
                    alignment,
                    &primitiveSize);

            if (alignmentKind == RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT) {
                /* Sequences are initialized with a length first */
                return;
            }

            RTIXCdrTypeCode_getMemberAlignment(
                    tc->_typeCode,
                    &tmpAlignment,
                    v2Encapsulation,
                    alignmentKind,
                    &thisNode);

            if (tmpAlignment > *alignment) {
                *alignment = tmpAlignment;
            }
        } break;
        default:
        {
            RTIXCdrTypeCode_getPrimitiveInfo(
                    kind,
                    v2Encapsulation,
                    alignment,
                    &primitiveSize);    
        } break;
    }
}

void RTIXCdrTypeCode_getMaxMemberAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrAlignment *alignment,
        RTIXCdrBoolean v2Encapsulation)
{
    RTIXCdrTypeCodeNode rootNode = { NULL, NULL };

    RTIXCdrTypeCode_getMemberAlignment(
            tc,
            alignment,
            v2Encapsulation,
            /* Maximum alignment across all members */
            RTI_XCDR_TYPECODE_MAX_MEMBER_ALIGNMENT,
            &rootNode);
}

void RTIXCdrTypeCode_getFirstMemberAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrAlignment *alignment,
        RTIXCdrBoolean v2Encapsulation)
{
    RTIXCdrTypeCodeNode rootNode = { NULL, NULL };

    RTIXCdrTypeCode_getMemberAlignment(
            tc,
            alignment,
            v2Encapsulation,
            RTI_XCDR_TYPECODE_FIRST_MEMBER_ALIGNMENT,
            &rootNode);
}

RTIXCdrBoolean RTIXTypeCode_useSampleAccessor(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean isOptional) 
{
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    if (tc->_sampleAccessInfo == NULL) {
        return RTI_XCDR_FALSE;
    }

    if (tc->_sampleAccessInfo->getMemberValuePointerFcn == NULL) {
        return RTI_XCDR_FALSE;
    }

    if (!isOptional
            && tc->_sampleAccessInfo->useGetMemberValueOnlyWithRef) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}


RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_hasCFriendlyCdrLayoutWithInitialAlignment(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrUnsignedLongLong *serSize,
        RTIXCdrAlignment intialAlignment,
        RTIXCdrAlignment maxMemberAlignment,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean dHeaderInNonPrimitiveCollection,
        RTIXCdrBoolean enumAsPrimitiveInCollection)
{
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrUnsignedLongLong tmpSerSize = 0;
    RTIXCdrAlignment memberAlignment = 0;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(serSize == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(maxMemberAlignment <= 0, return RTI_XCDR_FALSE);
    
    kind = RTIXCdrTypeCode_getKind(tc);
    
    if (kind == RTI_XCDR_TK_ALIAS) {
        tc = RTIXCdrTypeCode_resolveAlias(tc);
        kind = RTIXCdrTypeCode_getKind(tc);
    }

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT && kind != RTI_XCDR_TK_VALUE,
            return RTI_XCDR_FALSE);
    
    *serSize = (RTIXCdrUnsignedLongLong)intialAlignment;
    
    if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE ||
            RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_STRUCT) {
        if (RTIXCdrTypeCode_getExtensibilityKind(tc)
                == RTI_XCDR_MUTABLE_EXTENSIBILITY ||
                (RTIXCdrTypeCode_getExtensibilityKind(tc) !=
                        RTI_XCDR_FINAL_EXTENSIBILITY &&
                v2Encapsulation)) {
            /* V2 value and struct have dheader and this make them not
             * C friendly.
             *
             * Mutable types have also headers and they are not inlinable
             */
            return RTI_XCDR_FALSE;
        }
    }

    if (RTIXCdrTypeCode_getKind(tc) == RTI_XCDR_TK_VALUE) {
        if (tc->_typeCode != NULL) {
            if (RTIXCdrTypeCode_getKind(tc->_typeCode) != RTI_XCDR_TK_NULL) {
                /* Structures inheriting from other structures cannot be inline
                 */
                return RTI_XCDR_FALSE;
            }
        }
    }

    for (i=0; i<tc->_memberCount; i++) {
        const RTIXCdrTypeCode *memberTc = NULL;
        RTIXCdrTCKind memberTcKind = RTI_XCDR_TK_NULL;
        RTIXCdrOctet memberSize = 0;
        RTIXCdrUnsignedLong arrElementCount = 1;

        if (!tc->_members[i]._annotations._isSerializable) {
            /*
             * If we have a member that is non serialized then we are not C
             * friendly
             */
            return RTI_XCDR_FALSE;
        }

        memberTc = tc->_members[i]._representation._typeCode;
        memberTc = RTIXCdrTypeCode_resolveAlias(memberTc);

        memberTcKind = RTIXCdrTypeCode_getKind(memberTc);
        
        if (RTIXCdrTypeCodeMember_isOptional(&tc->_members[i]) ||
                tc->_members[i]._representation._isPointer) {
            return RTI_XCDR_FALSE;
        }

        /* 
         * If the memberTc has a sample accessor we have assume that the memory
         * layout is not C layout. If it was the same, there would not be a need
         * to install the accessor.
         */
        if (RTIXTypeCode_useSampleAccessor(
                memberTc,
                RTI_XCDR_FALSE)) {
            return RTI_XCDR_FALSE;
        }

        if (memberTcKind == RTI_XCDR_TK_ARRAY) {
            arrElementCount = RTIXCdrTypeCode_getArrayElementCount(memberTc);
            memberTc = RTIXCdrTypeCode_resolveAlias(memberTc->_typeCode);
            memberTcKind = RTIXCdrTypeCode_getKind(memberTc);

            if (v2Encapsulation
                    && dHeaderInNonPrimitiveCollection
                    && !RTIXCdrTypeCode_isPrimitiveKind(
                            memberTcKind, 
                            enumAsPrimitiveInCollection)) {
                return RTI_XCDR_FALSE;
            }
        } else {
            arrElementCount = 1;
        }

        if (RTIXCdrTypeCode_isPrimitiveKind(memberTcKind, RTI_XCDR_TRUE)) {
            if (memberTcKind == RTI_XCDR_TK_WCHAR ||
                    memberTcKind == RTI_XCDR_TK_BOOLEAN ||
                    memberTcKind == RTI_XCDR_TK_LONGDOUBLE) {
                /* The size of this primitive types may be different in
                 * different languages. We are conservative here and we
                 * return FALSE making the type not inline
                 */
                return RTI_XCDR_FALSE;
            }

            if (memberTcKind == RTI_XCDR_TK_ENUM) {
                return RTI_XCDR_FALSE;
            }
                    
            RTIXCdrTypeCode_getPrimitiveInfo(
                    memberTcKind,
                    /* We do not provide v2Encapsulation here because
                     * this alignment must be the most restrictive alignment
                     * that corresponds to v1
                     * 
                     * V1 alignment is compatible with C alignment
                     */
                    RTI_XCDR_FALSE,
                    &memberAlignment,
                    &memberSize);

            /* Guard here against zero or negative modulo operation */
            RTIXCdrLog_testPrecondition(
                    memberAlignment <= 0,
                    return RTI_XCDR_FALSE);
                    
            if (*serSize % ((RTIXCdrUnsignedLongLong) memberAlignment) != 0) {
                return RTI_XCDR_FALSE;
            }

            tmpSerSize = memberSize*(RTIXCdrUnsignedLongLong)arrElementCount;
        } else {   
            if (memberTcKind != RTI_XCDR_TK_STRUCT &&
                    memberTcKind != RTI_XCDR_TK_VALUE) {
                return RTI_XCDR_FALSE;
            }
            
            /* 
             * CORE-10820: In C, a structure is aligned to the maximum 
             * alignment across all its members.
             */
            RTIXCdrTypeCode_getMaxMemberAlignment(
                    memberTc,
                    &memberAlignment,
                    /* 
                     * We do not provide v2Encapsulation here because
                     * this alignment must be the most restrictive alignment
                     * that corresponds to v1
                     * 
                     * V1 alignment is compatible with C alignment
                     * 
                     */
                    RTI_XCDR_FALSE);

            if (memberAlignment == -1) {
                /* Empty structure: we don't support inlining empty structures
                 * as they can be tricky and have special layouts in some
                 * compilers
                 */
                return RTI_XCDR_FALSE;
            }

            if (memberAlignment < RTI_XCDR_FOUR_BYTE_ALIGNMENT 
                && tc->_sampleAccessInfo != NULL 
                && tc->_sampleAccessInfo->languageBinding 
                        == RTI_XCDR_TYPE_BINDING_DYN_DATA) {
                /* 
                 * DynamicData aligns all structures to 4 bytes, so any struct
                 * with an alignment != 4 or 8 may not be able to be
                 * inlined. For example, a structure with a single short in it
                 * will have 2 bytes of padding at the end to align to 4, which
                 * prevents us from inlining
                 */
                return RTI_XCDR_FALSE;
            }

            /* Guard here against zero or negative modulo operation */
            RTIXCdrLog_testPrecondition(
                    memberAlignment <= 0,
                    return RTI_XCDR_FALSE);

            if (*serSize % ((RTIXCdrUnsignedLongLong) memberAlignment) != 0) {
                return RTI_XCDR_FALSE;
            }

            if (!RTIXCdrTypeCode_hasCFriendlyCdrLayoutWithInitialAlignment(
                    memberTc,
                    &tmpSerSize,
                    (RTIXCdrAlignment) (*serSize % RTI_XCDR_MAX_ALIGNMENT),
                    memberAlignment,
                    arrElementCount,
                    v2Encapsulation,
                    dHeaderInNonPrimitiveCollection,
                    enumAsPrimitiveInCollection)) {
                return RTI_XCDR_FALSE;
            }

            /* 
             * CORE-10311: We have to check that there is no padding at the end
             * of the memberTc
             */
            if (tmpSerSize % ((RTIXCdrUnsignedLongLong)memberAlignment) != 0) {
                return RTI_XCDR_FALSE;
            }
        }
        
        *serSize += tmpSerSize;
    }

    if (elementCount != 1) {
        if (*serSize % ((RTIXCdrUnsignedLongLong)maxMemberAlignment) != 0) {
            return RTI_XCDR_FALSE;
        }
    }
    
    *serSize -= (RTIXCdrUnsignedLongLong)intialAlignment;
    *serSize *= elementCount;
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCode_hasCFriendlyCdrLayout(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrUnsignedLongLong *serSize,
        RTIXCdrAlignment *alignment,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrBoolean v2Encapsulation,
        RTIXCdrBoolean dHeaderInNonPrimitiveCollection,
        RTIXCdrBoolean enumAsPrimitiveInCollection)
{
    RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;
    RTIXCdrAlignment initialAlignment;
    /* V1 alignment is equivalent or more conservative than to C
     * alignment */
    RTIXCdrAlignment v1Alignment;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(serSize == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(alignment == NULL, return RTI_XCDR_FALSE);

    kind = RTIXCdrTypeCode_getKind(tc);
    
    if (kind == RTI_XCDR_TK_ALIAS) {
        tc = RTIXCdrTypeCode_resolveAlias(tc);
        kind = RTIXCdrTypeCode_getKind(tc);
    }

    if (kind != RTI_XCDR_TK_STRUCT &&
            kind != RTI_XCDR_TK_VALUE) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrTypeCode_getFirstMemberAlignment(
            tc,
            alignment,
            v2Encapsulation);
    
    if (*alignment == -1) {
        /* Empty structure: we don't support inlining empty structures
         * as they can be tricky and have special layouts in some
         * compilers
         */
        *serSize = 0;
        return RTI_XCDR_FALSE;
    }

    initialAlignment = 8;
    RTIXCdrTypeCode_getFirstMemberAlignment(
            tc,
            &v1Alignment,
            RTI_XCDR_FALSE);

    if (v1Alignment < RTI_XCDR_FOUR_BYTE_ALIGNMENT 
        && tc->_sampleAccessInfo != NULL
        && tc->_sampleAccessInfo->languageBinding 
                == RTI_XCDR_TYPE_BINDING_DYN_DATA) {
        /* 
         * DynamicData aligns all structures to 4 bytes, so any struct
         * with an alignment != 4 or 8 may not be able to be
         * inlined. For example, a structure with a single short in it
         * will have 2 bytes of padding at the end to align to 4, which
         * prevents us from inlining
         */
        return RTI_XCDR_FALSE;
    }
    
    while (initialAlignment >= v1Alignment) {
        /* We try to see if the CDR layout is friendly to C starting with 
         * each possible alignment compatible with the first member
         * alignment of the input tc
         */
        if (!RTIXCdrTypeCode_hasCFriendlyCdrLayoutWithInitialAlignment(
                tc,
                serSize,
                initialAlignment,
                v1Alignment,
                elementCount,
                v2Encapsulation,
                dHeaderInNonPrimitiveCollection,
                enumAsPrimitiveInCollection)) {
            return RTI_XCDR_FALSE;
        }

        initialAlignment/=2;
    }
    
    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCode_isOptionalMember(
        const RTIXCdrTypeCode *tc,
        RTIXCdrUnsignedLong index)
{
    RTIXCdrLog_preconditionOnly(RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;)

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    RTIXCdrLog_preconditionOnly(kind = RTIXCdrTypeCode_getKind(tc);)

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT &&
            kind != RTI_XCDR_TK_VALUE, 
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            index >= tc->_memberCount,
            return RTI_XCDR_FALSE);
    
    return (tc->_members[index]._memberFlags & 
            (RTI_XCDR_REQUIRED_MEMBER | RTI_XCDR_KEY_MEMBER))
        ? RTI_XCDR_FALSE : RTI_XCDR_TRUE;
}

RTI_PRIVATE
void RTIXCdrTypeCodeAnnotations_initializePrimitiveKinds(
        RTIXCdrTypeCodeAnnotations *self,
        RTIXCdrTCKind kind)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);

    switch (kind) {
    case RTI_XCDR_TK_BOOLEAN:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.boolean_value = RTI_XCDR_FALSE;
        break;
    case RTI_XCDR_TK_CHAR:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.char_value = 0;
        break;
    case RTI_XCDR_TK_WCHAR:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.wchar_value = 0;
        break;
    case RTI_XCDR_TK_SHORT:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.short_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.short_value = RTIXCdrShort_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.short_value = RTIXCdrShort_MAX;
        break;
    case RTI_XCDR_TK_LONG:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.long_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.long_value = RTIXCdrLong_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.long_value = RTIXCdrLong_MAX;
        break;
    case RTI_XCDR_TK_USHORT:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.ushort_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.ushort_value = RTIXCdrUnsignedShort_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.ushort_value = RTIXCdrUnsignedShort_MAX;
        break;
    case RTI_XCDR_TK_ULONG:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.ulong_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.ulong_value = RTIXCdrUnsignedLong_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.ulong_value = RTIXCdrUnsignedLong_MAX;
        break;
    case RTI_XCDR_TK_FLOAT:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.float_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.float_value = RTIXCdrFloat_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.float_value = RTIXCdrFloat_MAX;
        break;
    case RTI_XCDR_TK_DOUBLE:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.double_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.double_value = RTIXCdrDouble_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.double_value = RTIXCdrDouble_MAX;
        break;
    case RTI_XCDR_TK_OCTET:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.octet_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.octet_value = RTIXCdrOctet_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.octet_value = RTIXCdrOctet_MAX;
        break;
    case RTI_XCDR_TK_INT8:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.int8_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.int8_value = RTIXCdrInt8_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.int8_value = RTIXCdrInt8_MAX;
        break;
    case RTI_XCDR_TK_UINT8:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.uint8_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.uint8_value = RTIXCdrOctet_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.uint8_value = RTIXCdrOctet_MAX;
        break;
    case RTI_XCDR_TK_LONGLONG:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.long_long_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.long_long_value = RTIXCdrLongLong_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.long_long_value = RTIXCdrLongLong_MAX;
        break;
    case RTI_XCDR_TK_ULONGLONG:
        self->_defaultValue._d = kind;
        self->_defaultValue._u.ulong_long_value = 0;
        self->_minValue._d = kind;
        self->_minValue._u.ulong_long_value = RTIXCdrUnsignedLongLong_MIN;
        self->_maxValue._d = kind;
        self->_maxValue._u.ulong_long_value = RTIXCdrUnsignedLongLong_MAX;
        break;
    case RTI_XCDR_TK_STRING:
        /*
         * We initialize strings to NULL rather than the empty string to avoid
         * allocating an empty string for each string member or the complexity
         * of declaring a global empty string variable and then checking for it
         * whenever we need to finalize/copy annotations.
         */
        self->_defaultValue._d = kind;
        self->_defaultValue._u.string_value = NULL;
        break;
    case RTI_XCDR_TK_WSTRING:
        /*
         * We initialize wstrings to NULL rather than the empty string to avoid
         * allocating an empty string for each string member or the complexity
         * of declaring a global empty string variable and then checking for it
         * whenever we need to finalize/copy annotations.
         */
        self->_defaultValue._d = kind;
        self->_defaultValue._u.wstring_value = NULL;
        break;
    case RTI_XCDR_TK_LONGDOUBLE:
        /*
         * We do not support any annotations with LONGDOUBLE yet, just leave
         * default values
         */
    case RTI_XCDR_TK_NULL:
    default:
        break;
    }
}

void RTIXCdrTypeCode_initializeAnnotations(
        RTIXCdrTypeCode *self,
        RTIXCdrTCKind kind)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);

    RTIXCdrTypeCodeAnnotations_initialize(&self->_annotations);

    switch (kind) {
    case RTI_XCDR_TK_NULL:
    case RTI_XCDR_TK_STRUCT:
    case RTI_XCDR_TK_UNION:
    case RTI_XCDR_TK_SEQUENCE:
    case RTI_XCDR_TK_ARRAY:
    case RTI_XCDR_TK_VALUE:
        /* These type kinds do not support default, min, and max annotations. */
        return;
    case RTI_XCDR_TK_SHORT:
    case RTI_XCDR_TK_LONG:
    case RTI_XCDR_TK_USHORT:
    case RTI_XCDR_TK_ULONG:
    case RTI_XCDR_TK_FLOAT:
    case RTI_XCDR_TK_DOUBLE:
    case RTI_XCDR_TK_BOOLEAN:
    case RTI_XCDR_TK_CHAR:
    case RTI_XCDR_TK_OCTET:
    case RTI_XCDR_TK_STRING:
    case RTI_XCDR_TK_LONGLONG:
    case RTI_XCDR_TK_ULONGLONG:
    case RTI_XCDR_TK_LONGDOUBLE:
    case RTI_XCDR_TK_WCHAR:
    case RTI_XCDR_TK_WSTRING:
    case RTI_XCDR_TK_INT8:
    case RTI_XCDR_TK_UINT8:
        RTIXCdrTypeCodeAnnotations_initializePrimitiveKinds(
                &self->_annotations,
                kind);
        break;
    case RTI_XCDR_TK_ENUM: {
        if (self->_memberCount > 0) {
            RTIXCdrLong memberOrdinal = 0;
            /*
             * For an enum, the initial default value is the first literal,
             * so we need to obtain the first member using the typecode and
             * retrieve its ordinal.
             */
            RTIXCdrLog_testPrecondition(self->_members == NULL, return);
            RTIXCdrTypeCode_getMemberOrdinal(
                    self,
                    0,
                    &memberOrdinal);
            self->_annotations._defaultValue._d = RTI_XCDR_TK_ENUM;
            self->_annotations._defaultValue._u.enumerated_value =
                    memberOrdinal;
        }
        break;
    }
    case RTI_XCDR_TK_ALIAS:
    {
        const RTIXCdrTypeCode *resolvedTypeCode =
                RTIXCdrTypeCode_resolveAlias(self);
        RTIXCdrTCKind resolvedTypeKind;

        RTIXCdrLog_testPrecondition(resolvedTypeCode == NULL, return);
        resolvedTypeKind = RTIXCdrTypeCode_getKind(resolvedTypeCode);
        if (resolvedTypeKind == RTI_XCDR_TK_ENUM) {
            /*
             * The default annotation for the alias is the default
             * annotation for the related enum. Min and max do not apply. We
             * have to handle it as a special case here because we can't pass
             * both the alias TypeCode and the resolved TypeCode to this
             * initializeAnnotations function.
             */
            self->_annotations._defaultValue._d = RTI_XCDR_TK_ENUM;
            self->_annotations._defaultValue._u.enumerated_value =
                    resolvedTypeCode->_annotations._defaultValue._u
                            .enumerated_value;

        } else {
            RTIXCdrTypeCode_initializeAnnotations(self, resolvedTypeKind);
        }
        break;
    }
    default:
        break;
    }

    if (self->_isPointer) {
        RTIXCdrAnnotationParameterValue defaultValue =
                RTIXCdrAnnotationParameterValue_INITIALIZER;
        /*
         * Optional members and pointers (externals) do not have default
         * annotations, so clear any value that was set above.
         */
        RTIXCdrAnnotationParameterValue_finalize(
                &self->_annotations._defaultValue);
        self->_annotations._defaultValue = defaultValue;
    }
}

RTIXCdrBoolean RTIXCdrTypeCodeMember_initializeAnnotations(
        RTIXCdrTypeCodeMember *self)
{
    RTIXCdrTCKind memberKind = RTI_XCDR_TK_NULL;
    RTIXCdrTypeCode *memberType = NULL;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_FALSE);
    RTIXCdrLog_testPrecondition(
            self->_representation._typeCode == NULL,
            return RTI_FALSE);

    RTIXCdrTypeCodeAnnotations_initialize(&self->_annotations);

    memberType = self->_representation._typeCode;

    memberKind = RTIXCdrTypeCode_getKind(memberType);

    switch (memberKind) {
    case RTI_XCDR_TK_BOOLEAN:
    case RTI_XCDR_TK_CHAR:
    case RTI_XCDR_TK_WCHAR:
    case RTI_XCDR_TK_SHORT:
    case RTI_XCDR_TK_LONG:
    case RTI_XCDR_TK_USHORT:
    case RTI_XCDR_TK_ULONG:
    case RTI_XCDR_TK_FLOAT:
    case RTI_XCDR_TK_DOUBLE:
    case RTI_XCDR_TK_OCTET:
    case RTI_XCDR_TK_INT8:
    case RTI_XCDR_TK_UINT8:
    case RTI_XCDR_TK_LONGLONG:
    case RTI_XCDR_TK_ULONGLONG:
    case RTI_XCDR_TK_STRING:
    case RTI_XCDR_TK_WSTRING:
    case RTI_XCDR_TK_LONGDOUBLE:
    case RTI_XCDR_TK_NULL:
        RTIXCdrTypeCodeAnnotations_initializePrimitiveKinds(
                &self->_annotations,
                memberKind);
        break;
    case RTI_XCDR_TK_ENUM:
    case RTI_XCDR_TK_STRUCT:
    case RTI_XCDR_TK_UNION:
    case RTI_XCDR_TK_VALUE:
    case RTI_XCDR_TK_SEQUENCE:
    case RTI_XCDR_TK_ARRAY:
    case RTI_XCDR_TK_ALIAS: {
        /* The type already contains resolved annotations, so
         * use the values to initialize the member */
        if (!RTIXCdrTypeCodeAnnotations_copy(
                    &self->_annotations,
                    &memberType->_annotations)) {
            return RTI_XCDR_FALSE;
        }
        break;
    }
    default:
        break;
    }

    if (RTIXCdrTypeCodeMember_isOptional(self)
            || self->_representation._isPointer) {
        RTIXCdrAnnotationParameterValue defaultValue =
                RTIXCdrAnnotationParameterValue_INITIALIZER;
        /*
         * Optional members do not have default annotations, so clear any
         * value that was set above. We finalize the default value
         * because the string and wstring values may be allocated.
         */
        RTIXCdrAnnotationParameterValue_finalize(
                &self->_annotations._defaultValue);
        self->_annotations._defaultValue = defaultValue;
    }

    /* Make sure that the must_understand flag is always false
     * for key members. */
    if (self->_memberFlags & RTI_XCDR_FLAG_KEY_MEMBER) {
        self->_annotations._isMustUnderstand = RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrAnnotationParameterValue_isNonZero(
        const RTIXCdrAnnotationParameterValue *value)
{
    switch (value->_d) {
        case RTI_XCDR_TK_SHORT:
        {
            return value->_u.short_value != 0;
        } break;
        case RTI_XCDR_TK_USHORT:
        {
            return value->_u.ushort_value != 0;
        } break;
        case RTI_XCDR_TK_ENUM: 
        case RTI_XCDR_TK_LONG: 
        {
            return value->_u.long_value != 0;
        } break;
        case RTI_XCDR_TK_ULONG:
        {
            return value->_u.ulong_value != 0;
        } break;
        case RTI_XCDR_TK_FLOAT:
        {
            return !RTIXCdrUtility_floatNearlyEqual(value->_u.float_value, 0);
        } break;
        case RTI_XCDR_TK_DOUBLE:
        {
            return !RTIXCdrUtility_doubleNearlyEqual(value->_u.double_value, 0);
        } break;
        case RTI_XCDR_TK_BOOLEAN:
        {
            return value->_u.boolean_value != 0;
        } break;
        case RTI_XCDR_TK_CHAR: 
        {
            return value->_u.char_value != 0;
        } break;
        case RTI_XCDR_TK_OCTET:
        {
            return value->_u.octet_value != 0;
        } break;
        case RTI_XCDR_TK_INT8:
        {
            return value->_u.int8_value != 0;
        } break;
        case RTI_XCDR_TK_UINT8:
        {
            return value->_u.uint8_value != 0;
        } break;
        case RTI_XCDR_TK_LONGLONG:
        {
            return value->_u.long_long_value != 0;
        } break;
        case RTI_XCDR_TK_ULONGLONG:
        {
            return value->_u.ulong_long_value != 0;
        } break;
        case RTI_XCDR_TK_WCHAR:
        {
            return value->_u.wchar_value != 0;
        } break;
        case RTI_XCDR_TK_STRING:
        {
            return value->_u.string_value != NULL;
        } break;
        case RTI_XCDR_TK_WSTRING:
        {
            return value->_u.wstring_value != NULL;
        } break;
        /* Long doubles do not support the default annotation */
        case RTI_XCDR_TK_LONGDOUBLE:
        default:
            return RTI_XCDR_FALSE;
    }
}

RTIXCdrBoolean RTIXCdrTypeCodeMember_hasNonZeroDefault(
        const RTIXCdrTypeCodeMember *tcMember)
{
    RTIXCdrLog_testPrecondition(tcMember == NULL, return RTI_XCDR_FALSE);

    return RTIXCdrAnnotationParameterValue_isNonZero(
            &tcMember->_annotations._defaultValue);
}

RTIXCdrUnsignedLong RTIXCdrTypeCode_getOptionalMemberCount(
        const RTIXCdrTypeCode *tc)
{
    RTIXCdrUnsignedLong optionalCount = 0;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrLog_preconditionOnly(RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;)

    RTIXCdrLog_testPrecondition(tc == NULL, return optionalCount);

    RTIXCdrLog_preconditionOnly(kind = RTIXCdrTypeCode_getKind(tc);)

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT &&
            kind != RTI_XCDR_TK_VALUE, 
            return optionalCount);

    for (i=0; i<tc->_memberCount; i++) {
        if (RTIXCdrTypeCode_isOptionalMember(tc, i)) {
            optionalCount++;
        }
    }            
    
    return optionalCount;
}

RTIXCdrBoolean RTIXCdrTypeCode_discValuesSelectSameMember(
        const RTIXCdrTypeCode *tc,
        RTIXCdrLong labelLeft,
        RTIXCdrLong labelRight)
{
    RTIXCdrBoolean foundLeft = RTI_XCDR_FALSE, foundRight = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong i = 0, j = 0;
    RTIXCdrLong leftIndex = -1, rightIndex = -1;

    RTIXCdrLog_preconditionOnly(RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;)
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_preconditionOnly(kind = RTIXCdrTypeCode_getKind(tc);)
    RTIXCdrLog_testPrecondition(kind != RTI_XCDR_TK_UNION, return RTI_XCDR_FALSE);

    for (i = 0; 
         i < tc->_memberCount; 
         i++, foundLeft = RTI_XCDR_FALSE, foundRight = RTI_XCDR_FALSE) {
        if (tc->_members[i]._labelsCount == 1) {
            if (tc->_members[i]._label == labelLeft 
                    || tc->_members[i]._label == labelRight) {
                if (labelLeft == labelRight) {
                    return RTI_XCDR_TRUE;
                }
                return RTI_XCDR_FALSE;
            }
        } else {
            for (j = 0; j < tc->_members[i]._labelsCount; j++) {
                if (tc->_members[i]._labels[j] == labelLeft) {
                    foundLeft = RTI_XCDR_TRUE;
                    leftIndex = (RTIXCdrLong) i;
                } 
                if (tc->_members[i]._labels[j] == labelRight) {
                    foundRight = RTI_XCDR_TRUE;
                    rightIndex = (RTIXCdrLong) i;
                }
                if (foundLeft && foundRight) {
                    return RTI_XCDR_TRUE;
                }
            }
        }
    }

    /* If they both identify the default label, we also return true */
    if (tc->_default_index != -1) {
        if (leftIndex == -1) {
            leftIndex = tc->_default_index;
        }
        if (rightIndex == -1) {
            rightIndex = tc->_default_index;
        }
        if (leftIndex == rightIndex) {
            return RTI_XCDR_TRUE;
        }
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrUnsignedLong RTIXCdrTypeCode_getLabelCount(
    const RTIXCdrTypeCode *tc,
    RTIXCdrBoolean includeDefault)
{
    RTIXCdrUnsignedLong count = 0;
    RTIXCdrUnsignedLong i = 0;

    RTIXCdrLog_preconditionOnly(RTIXCdrTCKind kind = RTI_XCDR_TK_NULL;)
    RTIXCdrLog_testPrecondition(tc == NULL, return 0);
    RTIXCdrLog_preconditionOnly(kind = RTIXCdrTypeCode_getKind(tc);)
    RTIXCdrLog_testPrecondition(kind != RTI_XCDR_TK_UNION, return 0);

    for (i=0; i<tc->_memberCount; i++) {
        if (((RTIXCdrLong)i) ==
                (tc)->_default_index && !includeDefault) {
            continue;
        }
        count += tc->_members[i]._labelsCount;
    }

    return count;
}

void RTIXCdrAnnotationParameterValue_finalize(
        struct RTIXCdrAnnotationParameterValue *in)
{
    static const struct RTIXCdrAnnotationParameterValue defaultValue =
            RTIXCdrAnnotationParameterValue_INITIALIZER;

    RTIXCdrLog_testPrecondition(in == NULL, return);

    switch(in->_d) {
    case RTI_XCDR_TK_STRING:
        if (in->_u.string_value != NULL) {
            RTIXCdrHeap_freeString(in->_u.string_value);
        }
        break;
    case RTI_XCDR_TK_WSTRING:
        if (in->_u.wstring_value != NULL) {
            RTIXCdrHeap_freeWString(in->_u.wstring_value);
        }
        break;
    default:
        break;
    }

    *in = defaultValue;
}

void RTIXCdrTypeCodeAnnotations_initialize(
        struct RTIXCdrTypeCodeAnnotations *annotations)
{
    static const struct RTIXCdrTypeCodeAnnotations initialValue =
            RTIXCdrTypeCodeAnnotations_INITIALIZER;
    *annotations = initialValue;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrAnnotationParameterValue_copy(
        struct RTIXCdrAnnotationParameterValue *out,
        const struct RTIXCdrAnnotationParameterValue *in) {


    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);

    RTIXCdrAnnotationParameterValue_finalize(out);

    out->_d = in->_d;

    switch(out->_d) {
    case RTI_XCDR_TK_NULL:
        break;
    case RTI_XCDR_TK_STRING:
        if (in->_u.string_value != NULL) {
            if (RTIXCdrString_replace(
                        &out->_u.string_value,
                        in->_u.string_value)
                == NULL) {
                return RTI_XCDR_FALSE;
            }
        } else if (out->_u.string_value != NULL) {
            RTIXCdrHeap_freeString(out->_u.string_value);
            out->_u.string_value = NULL;
        }
        break;
    case RTI_XCDR_TK_WSTRING:
        if (in->_u.wstring_value != NULL) {
            if (RTIXCdrWString_replace(
                        &out->_u.wstring_value,
                        in->_u.wstring_value)
                == NULL) {
                return RTI_XCDR_FALSE;
            }
        } else if (out->_u.wstring_value != NULL) {
            RTIXCdrHeap_freeWString(out->_u.wstring_value);
            out->_u.wstring_value = NULL;
        }
        break;
    default:
        out->_u = in->_u;
        break;
    }

    return RTI_XCDR_TRUE;
}

/* Returns true if first is less than second */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrAnnotationParameterValue_lessThan(
        const struct RTIXCdrAnnotationParameterValue *first,
        const struct RTIXCdrAnnotationParameterValue *second)
{
    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (first->_d != second->_d) {
        return RTI_XCDR_FALSE;
    }

    switch (first->_d) {
    case RTI_XCDR_TK_NULL:
        break;
    case RTI_XCDR_TK_STRING:
        if (RTIXCdrString_cmp(first->_u.string_value, second->_u.string_value)
                < 0) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_WSTRING:
        if (RTIXCdrWString_cmp(
                    first->_u.wstring_value,
                    second->_u.wstring_value)
                < 0) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_BOOLEAN:
        if (first->_u.boolean_value < second->_u.boolean_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_OCTET:
        if (first->_u.octet_value < second->_u.octet_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_INT8:
        if (first->_u.int8_value < second->_u.int8_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_UINT8:
        if (first->_u.uint8_value < second->_u.uint8_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_SHORT:
        if (first->_u.short_value < second->_u.short_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_USHORT:
        if (first->_u.ushort_value < second->_u.ushort_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_LONG:
        if (first->_u.long_value < second->_u.long_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_ULONG:
        if (first->_u.ulong_value < second->_u.ulong_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_LONGLONG:
        if (first->_u.long_long_value < second->_u.long_long_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_ULONGLONG:
        if (first->_u.ulong_long_value < second->_u.ulong_long_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_FLOAT:
        if (first->_u.float_value < second->_u.float_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_DOUBLE:
        if (first->_u.double_value < second->_u.double_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    /*
     * Not supported yet
     * case RTI_XCDR_TK_LONGDOUBLE:
     *   if (first->_u.long_double_value < second->_u.long_double_value) {
     *        return RTI_XCDR_TRUE;
     *   }
     *   break;
     */
    case RTI_XCDR_TK_CHAR:
        if (first->_u.char_value < second->_u.char_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_WCHAR:
        if (first->_u.wchar_value < second->_u.wchar_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    case RTI_XCDR_TK_ENUM:
        if (first->_u.enumerated_value < second->_u.enumerated_value) {
            return RTI_XCDR_TRUE;
        }
        break;
    default:
        return RTI_XCDR_FALSE;
        break;
    }
    return RTI_XCDR_FALSE;
}

RTI_PRIVATE
void RTIXCdrObservableAnnotation_finalize(
        struct RTIXCdrObservableAnnotation *in) 
{
    RTIXCdrLog_testPrecondition(in == NULL, return);

    in->isSet = RTI_XCDR_FALSE;
}

RTI_PRIVATE
void RTIXCdrObservableAnnotation_copy(
        struct RTIXCdrObservableAnnotation *out,
        const struct RTIXCdrObservableAnnotation *in) 
{
    RTIXCdrLog_testPrecondition(in == NULL, return);
    RTIXCdrLog_testPrecondition(out == NULL, return);

    RTIXCdrObservableAnnotation_finalize(out);

    out->isSet = in->isSet;
    out->distributionKind = in->distributionKind;
}

RTI_PRIVATE
void RTIXCdrResourceAnnotation_finalize(
        struct RTIXCdrResourceAnnotation *in) 
{
    RTIXCdrLog_testPrecondition(in == NULL, return);

    if (in->className != NULL) {
        RTIXCdrHeap_freeString(in->className);
        in->className = NULL;
    }

    if (in->owner != NULL) {
        RTIXCdrHeap_freeString(in->owner);
        in->owner = NULL;
    }

    in->isSet = RTI_XCDR_FALSE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrResourceAnnotation_copy(
        struct RTIXCdrResourceAnnotation *out,
        const struct RTIXCdrResourceAnnotation *in) 
{
    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);

    RTIXCdrResourceAnnotation_finalize(out);

    out->isSet = in->isSet;
    out->isRoot = in->isRoot;

    if (in->className != NULL) {
        out->className = RTIXCdrString_dup(in->className);
        if (out->className == NULL) {
            return RTI_XCDR_FALSE;
        }
    }

    if (in->owner != NULL) {
        out->owner = RTIXCdrString_dup(in->owner);
        if (out->owner == NULL) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_copy(
        struct RTIXCdrTypeCodeAnnotations *out,
        const struct RTIXCdrTypeCodeAnnotations *in) {

    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrAnnotationParameterValue_copy(
            &out->_defaultValue,
            &in->_defaultValue)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotationParameterValue_copy(
            &out->_maxValue,
            &in->_maxValue)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotationParameterValue_copy(
            &out->_minValue,
            &in->_minValue)) {
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrString_replace(&out->_unitValue, in->_unitValue) == NULL
        && in->_unitValue != NULL) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrObservableAnnotation_copy(&out->_observable, &in->_observable);

    if (!RTIXCdrResourceAnnotation_copy(
            &out->_resource,
            &in->_resource)) {
        return RTI_XCDR_FALSE;
    }

    out->_isSerializable = in->_isSerializable;

    out->_allowedDataRepresentationMask =
            in->_allowedDataRepresentationMask;

    out->_isNested = in->_isNested;

    out->_isAutoIdHash = in->_isAutoIdHash;

    out->_isMustUnderstand = in->_isMustUnderstand;

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrAnnotation_equalsString(
        const char *first,
        const char *second)
{
    if (first == NULL) {
        if (second != NULL) {
            return RTI_XCDR_FALSE;
        }

        return RTI_XCDR_TRUE;
    }

    if (second == NULL) {
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrString_cmp(first, second) != 0) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrObservableAnnotation_equals(
        const struct RTIXCdrObservableAnnotation *first,
        const struct RTIXCdrObservableAnnotation *second)
{
    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (first->isSet != second->isSet) {
        return RTI_XCDR_FALSE;
    }

    if (first->distributionKind != second->distributionKind) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrResourceAnnotation_equals(
        const struct RTIXCdrResourceAnnotation *first,
        const struct RTIXCdrResourceAnnotation *second)
{
    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (first->isSet != second->isSet) {
        return RTI_XCDR_FALSE;
    }

    if (first->isRoot != second->isRoot) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotation_equalsString(first->className, second->className)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotation_equalsString(first->owner, second->owner)) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

/**
 * @brief Check if the min, max, default, and unit annotations are equal.
 *
 * This function is used to improve robustness towards unexpected changes
 * to other fields in the Annotations of a member, when compared within
 * the enclosing TypeCode.
 *
 * @param[in] first Annotations for the first member.
 * @param[in] second Annotations for the second member.
 * @return RTI_XCDR_TRUE if the annotations are equal, RTI_XCDR_FALSE otherwise.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_memberSubsetEquals(
        const struct RTIXCdrTypeCodeAnnotations *first,
        const struct RTIXCdrTypeCodeAnnotations *second)
{

    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrAnnotationParameterValue_equals(
            &first->_defaultValue,
            &second->_defaultValue)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotationParameterValue_equals(
            &first->_maxValue,
            &second->_maxValue)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrAnnotationParameterValue_equals(
            &first->_minValue,
            &second->_minValue)) {
        return RTI_XCDR_FALSE;
    }

    if (RTIXCdrString_cmp(first->_unitValue, second->_unitValue) != 0) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_equals(
        const struct RTIXCdrTypeCodeAnnotations *first,
        const struct RTIXCdrTypeCodeAnnotations *second)
{
    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (!RTIXCdrTypeCodeAnnotations_memberSubsetEquals(first, second)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrObservableAnnotation_equals(
            &first->_observable,
            &second->_observable)) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrResourceAnnotation_equals(
            &first->_resource,
            &second->_resource)) {
        return RTI_XCDR_FALSE;
    }

    if (first->_isSerializable != second->_isSerializable) {
        return RTI_XCDR_FALSE;
    }

    if (first->_allowedDataRepresentationMask !=
            second->_allowedDataRepresentationMask) {
        return RTI_XCDR_FALSE;
    }

    if (first->_isNested != second->_isNested) {
        return RTI_XCDR_FALSE;
    }

    if (first->_isAutoIdHash != second->_isAutoIdHash) {
        return RTI_XCDR_FALSE;
    }

    /*  _isMustUnderstand is not checked here, because it is
     * checked as part of RTIXCdrStructTypeCode_equal().
     * See comment in the declaration of _isMustUnderstand.
     */

    return RTI_XCDR_TRUE;
}

/* Returns true if min <= default <= max */
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_isDefaultAndRangeConsistent(
        const struct RTIXCdrTypeCodeAnnotations *self,
        RTIXCdrBoolean isRequiredMember,
        RTIXCdrBoolean isPointerMember)
{
    RTIXCdrTCKind discriminator = RTI_XCDR_TK_NULL;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);

    /* Some values may be TK_NULL because they're not set. */
    if (self->_defaultValue._d != RTI_XCDR_TK_NULL) {
        discriminator = self->_defaultValue._d;
    } else if (self->_minValue._d != RTI_XCDR_TK_NULL) {
        discriminator = self->_minValue._d;
    } else if (self->_maxValue._d != RTI_XCDR_TK_NULL) {
        discriminator = self->_maxValue._d;
    }

    switch (discriminator) {
    case RTI_XCDR_TK_NULL:
    case RTI_XCDR_TK_STRING:
    case RTI_XCDR_TK_WSTRING:
    case RTI_XCDR_TK_BOOLEAN:
    case RTI_XCDR_TK_ENUM:
        return RTI_XCDR_TRUE;
    default:
        break;
    }
    if (RTIXCdrAnnotationParameterValue_lessThan(
            &self->_defaultValue,
            &self->_minValue)
        || RTIXCdrAnnotationParameterValue_lessThan(
                &self->_maxValue,
                &self->_defaultValue)
        || RTIXCdrAnnotationParameterValue_lessThan(
                &self->_maxValue,
                &self->_minValue)) {
        return RTI_XCDR_FALSE;
    }

    if (((self->_minValue._d != RTI_XCDR_TK_NULL)
         || (self->_maxValue._d != RTI_XCDR_TK_NULL))
        && (self->_defaultValue._d == RTI_XCDR_TK_NULL) && isRequiredMember
        && !isPointerMember) {
        /*
         * The user specified min or max but not default, and it's not an
         * optional member. Check if the min-max range includes 0, which is the
         * default default.
         */
        RTIXCdrAnnotationParameterValue zeroValue =
                RTIXCdrAnnotationParameterValue_INITIALIZER;

        zeroValue._d = discriminator;
        if (RTIXCdrAnnotationParameterValue_lessThan(
                &zeroValue,
                &self->_minValue)
            || RTIXCdrAnnotationParameterValue_lessThan(
                &self->_maxValue,
                &zeroValue)) {
            return RTI_XCDR_FALSE;
        }
    }
    return RTI_XCDR_TRUE;
}

void RTIXCdrTypeCodeAnnotations_finalize(
        struct RTIXCdrTypeCodeAnnotations *annotations) {

    RTIXCdrLog_testPrecondition(annotations == NULL, return);

    RTIXCdrAnnotationParameterValue_finalize(&annotations->_defaultValue);
    RTIXCdrAnnotationParameterValue_finalize(&annotations->_maxValue);
    RTIXCdrAnnotationParameterValue_finalize(&annotations->_minValue);
    if (annotations->_unitValue != NULL) {
        RTIXCdrHeap_freeString(annotations->_unitValue);
        annotations->_unitValue = NULL;
    }
}

RTIXCdrBoolean RTIXCdrTypeCode_isTypeCodeVisited(
        const RTIXCdrTypeCode *tc,
        const RTIXCdrTypeCodeNode *parentVisitedNode) 
{
    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    while (parentVisitedNode != NULL) {
        if (parentVisitedNode->tc == tc) {
            return RTI_XCDR_TRUE;
        }

        parentVisitedNode = parentVisitedNode->prev;
    }

    return RTI_XCDR_FALSE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_isUnboundedWithNode(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean keyOnly,
        RTIXCdrUnsignedLong unboundedSize,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrTCKind kind;
    RTIXCdrTypeCodeNode visitedNode;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            parentVisitedNode == NULL, 
            return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        /* Type is unbounded if there is recursion */
        return RTI_XCDR_TRUE;
    }

    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ALIAS:
        {
            return RTIXCdrTypeCode_isUnboundedWithNode(
                    tc->_typeCode, 
                    keyOnly, 
                    unboundedSize,
                    &visitedNode);
        } break;
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_UNION:
        {
            RTIXCdrUnsignedLong i = 0;
            RTIXCdrBoolean hasKey = RTI_XCDR_FALSE;
            RTIXCdrBoolean hasKeyBase = RTI_XCDR_FALSE;

            if (kind != RTI_XCDR_TK_UNION) {
                hasKey = RTIXCdrTypeCode_hasKey(tc);
            } else {
                hasKey = RTI_XCDR_FALSE;
            }

            if (kind == RTI_XCDR_TK_VALUE && RTIXCdrTypeCode_hasBase(tc)) {
                RTIXCdrBoolean processType = RTI_XCDR_FALSE;
                RTIXCdrTypeCode *baseType = tc->_typeCode;

                if (!keyOnly) {
                    processType = RTI_XCDR_TRUE;
                } else {
                    hasKeyBase = RTIXCdrTypeCode_hasKey(baseType);
                    if (hasKeyBase || !hasKey) {
                        processType = RTI_XCDR_TRUE;
                    }
                }

                if (processType) {
                    if (RTIXCdrTypeCode_isUnboundedWithNode(
                            baseType, 
                            keyOnly, 
                            unboundedSize,
                            &visitedNode)) {
                        return RTI_XCDR_TRUE;
                    }
                }
            }

            for (i=0; i<tc->_memberCount; i++) {
                if (keyOnly && (hasKey || hasKeyBase)) {
                    if (!(tc->_members[i]._memberFlags & RTI_XCDR_KEY_MEMBER)) {
                        continue;
                    }
                }

                if (RTIXCdrTypeCode_isUnboundedWithNode(
                        tc->_members[i]._representation._typeCode,
                        keyOnly,
                        unboundedSize,
                        &visitedNode)) {
                    return RTI_XCDR_TRUE;
                }
            }

            return RTI_XCDR_FALSE;
        } break;
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ARRAY:
        {
            if (kind == RTI_XCDR_TK_SEQUENCE) {
                if (tc->_maximumLength >= unboundedSize) {
                    return RTI_XCDR_TRUE;
                }

            }

            if (RTIXCdrTypeCode_isUnboundedWithNode(
                    tc->_typeCode, 
                    keyOnly,
                    unboundedSize,
                    &visitedNode)) {
                return RTI_XCDR_TRUE;
            }
        } break;
        case RTI_XCDR_TK_STRING:
        case RTI_XCDR_TK_WSTRING:
        {
            if (tc->_maximumLength >= unboundedSize) {
                return RTI_XCDR_TRUE;
            }
        } break;
        default:
            return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCode_isUnbounded(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean keyOnly,
        RTIXCdrUnsignedLong unboundedSize)
{
    struct RTIXCdrTypeCodeNode visitedNode;

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_isUnboundedWithNode(
            tc,
            keyOnly,
            unboundedSize,
            &visitedNode);
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_hasMemberHeadersWithNode(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean keyOnly,
        RTIXCdrBoolean mutableHeaders,
        RTIXCdrBoolean optionalHeaders,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrTCKind kind;
    RTIXCdrTypeCodeNode visitedNode;
    RTIXCdrExtensibilityKind extKind;


    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            !mutableHeaders && !optionalHeaders,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            parentVisitedNode == NULL,
            return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        /* If we've already visited this type, we won't again. We return false,
         * but if there were optionals we would find out by examining the type
         * the first time.
         */
        return RTI_XCDR_FALSE;
    }

    extKind = RTIXCdrTypeCode_getExtensibilityKind(tc);
    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
    case RTI_XCDR_TK_ALIAS:
        return RTIXCdrTypeCode_hasMemberHeadersWithNode(
                tc->_typeCode,
                keyOnly,
                mutableHeaders,
                optionalHeaders,
                &visitedNode);
    case RTI_XCDR_TK_STRUCT:
    case RTI_XCDR_TK_VALUE:
    case RTI_XCDR_TK_UNION: {
        RTIXCdrUnsignedLong i = 0;
        RTIXCdrBoolean hasKey = RTI_XCDR_FALSE;
        RTIXCdrBoolean hasKeyBase = RTI_XCDR_FALSE;

        if (kind != RTI_XCDR_TK_UNION) {
            hasKey = RTIXCdrTypeCode_hasKey(tc);
        } else {
            hasKey = RTI_XCDR_FALSE;
        }

        if (kind == RTI_XCDR_TK_VALUE && RTIXCdrTypeCode_hasBase(tc)) {
            RTIXCdrBoolean processType = RTI_XCDR_FALSE;
            RTIXCdrTypeCode *baseType = tc->_typeCode;

            if (!keyOnly) {
                processType = RTI_XCDR_TRUE;
            } else {
                hasKeyBase = RTIXCdrTypeCode_hasKey(baseType);
                if (hasKeyBase || !hasKey) {
                    processType = RTI_XCDR_TRUE;
                }
            }

            if (processType) {
                if (RTIXCdrTypeCode_hasMemberHeadersWithNode(
                            baseType,
                            keyOnly,
                            mutableHeaders,
                            optionalHeaders,
                            &visitedNode)) {
                    return RTI_XCDR_TRUE;
                }
            }
        }

        for (i = 0; i < tc->_memberCount; i++) {
            if ((optionalHeaders
                    && RTIXCdrTypeCodeMember_isOptional(&tc->_members[i]))
                    || (mutableHeaders
                            && extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY)) {
                return RTI_XCDR_TRUE;
            }

            if (RTIXCdrTypeCode_hasMemberHeadersWithNode(
                        tc->_members[i]._representation._typeCode,
                        keyOnly,
                        mutableHeaders,
                        optionalHeaders,
                        &visitedNode)) {
                return RTI_XCDR_TRUE;
            }
        }

        return RTI_XCDR_FALSE;
    }
    case RTI_XCDR_TK_SEQUENCE:
    case RTI_XCDR_TK_ARRAY:
        return RTIXCdrTypeCode_hasMemberHeadersWithNode(
                tc->_typeCode,
                keyOnly,
                mutableHeaders,
                optionalHeaders,
                &visitedNode);
    default:
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCode_hasOptionals(const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode;

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_hasMemberHeadersWithNode(
            tc,
            RTI_XCDR_FALSE, /* keyOnly */
            RTI_XCDR_FALSE, /* mutableHeaders */
            RTI_XCDR_TRUE, /* optionalHeaders */
            &visitedNode);
}

RTIXCdrBoolean RTIXCdrTypeCode_hasMemberHeaders(const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode;

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_hasMemberHeadersWithNode(
            tc,
            RTI_XCDR_FALSE, /* keyOnly */
            RTI_XCDR_TRUE, /* mutableHeaders */
            RTI_XCDR_TRUE, /* optionalHeaders */
            &visitedNode);
}

/**
 * @brief Returns RTI_XCDR_TRUE if a type is fixed size.
 * 
 * A type is fixed size if all its members at any level are primitive members,
 * or array of primitive members.
 * 
 * @param tc TypeCode.
 * @param zeroDefaultIsRequired When this parameter is set to RTI_XCDR_TRUE
 * being fixed size also requires having a zero default value.
 * @param parentVisitedNode 
 * @return Returns RTI_XCDR_TRUE if a type is fixed size. Otherwise,
 * RTI_XCDR_FALSE.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_isFixedSizeWithNode(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean zeroDefaultIsRequired,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrTCKind kind;
    RTIXCdrTypeCodeNode visitedNode;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            parentVisitedNode == NULL,
            return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        /* Type is unbounded if there is recursion */
        return RTI_XCDR_TRUE;
    }

    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
    case RTI_XCDR_TK_UNION:
    case RTI_XCDR_TK_WSTRING:
    case RTI_XCDR_TK_STRING:
    case RTI_XCDR_TK_SEQUENCE:
        return RTI_XCDR_FALSE;

    case RTI_XCDR_TK_ARRAY:
    {
        if (tc->_isPointer) {
            /* Arrays of pointers are not considered fixed size */
            return RTI_XCDR_FALSE;
        }
        return RTIXCdrTypeCode_isFixedSizeWithNode(
                tc->_typeCode,
                zeroDefaultIsRequired,
                &visitedNode);
    } break;
    case RTI_XCDR_TK_ALIAS:
    {
        tc = RTIXCdrTypeCode_resolveAliasUntilPointer(tc->_typeCode);

        kind = RTIXCdrTypeCode_getKind(tc);

        if (tc->_isPointer) {
            /*
             * External alias are not considered fixed size. For example:
             * typedef long *MyLong;
             */
            return RTI_XCDR_FALSE;
        }

        if (RTIXCdrTypeCode_isPrimitiveKind(kind, RTI_XCDR_TRUE)) {
            /*
             * If the alias is an alias to a primitive type (e.g, long)
             * we return true or false depending on default value.
             */
            if (zeroDefaultIsRequired
                    && RTIXCdrAnnotationParameterValue_isNonZero(
                            &tc->_annotations._defaultValue)) {
                return RTI_XCDR_FALSE;
            } 

            return RTI_XCDR_TRUE;
        }

        return RTIXCdrTypeCode_isFixedSizeWithNode(
                tc,
                zeroDefaultIsRequired,
                &visitedNode);
    } break;
    case RTI_XCDR_TK_STRUCT:
    case RTI_XCDR_TK_VALUE: {
        RTIXCdrUnsignedLong i = 0;

        if (kind == RTI_XCDR_TK_VALUE && RTIXCdrTypeCode_hasBase(tc)) {
            RTIXCdrTypeCode *baseType = tc->_typeCode;

            if (!RTIXCdrTypeCode_isFixedSizeWithNode(
                        baseType,
                        zeroDefaultIsRequired,
                        &visitedNode)) {
                return RTI_XCDR_FALSE;
            }
        }

        for (i = 0; i < tc->_memberCount; i++) {
            if (RTIXCdrTypeCodeMember_isOptional(&tc->_members[i])) {
                return RTI_XCDR_FALSE;
            }

            if (tc->_members[i]._representation._isPointer) {
                return RTI_XCDR_FALSE;
            }
    
            if (RTIXCdrTypeCode_isPrimitiveKind(RTIXCdrTypeCode_getKind(
                    tc->_members[i]._representation._typeCode),
                    RTI_XCDR_TRUE)) {
                if (zeroDefaultIsRequired
                        && RTIXCdrTypeCodeMember_hasNonZeroDefault(
                                &tc->_members[i])) {
                    return RTI_XCDR_FALSE;
                }
            } else {
                if (!RTIXCdrTypeCode_isFixedSizeWithNode(
                        tc->_members[i]._representation._typeCode,
                        zeroDefaultIsRequired,
                        &visitedNode)) {
                    return RTI_XCDR_FALSE;
                }
            }
        }

        return RTI_XCDR_TRUE;
    } break;

    default:
        return RTI_XCDR_TRUE;
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCode_isFixedSize(const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode = { NULL, NULL };
    return RTIXCdrTypeCode_isFixedSizeWithNode(
            tc,
            RTI_XCDR_FALSE,
            &visitedNode);
}

RTIXCdrBoolean RTIXCdrTypeCode_isFixedSizeWithZeroDefault(
        const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode = { NULL, NULL };
    return RTIXCdrTypeCode_isFixedSizeWithNode(tc, RTI_XCDR_TRUE, &visitedNode);
}

RTIXCdrBoolean RTIXCdrTypeCode_hasNonDefaultDefault(
        const struct RTIXCdrTypeCode *self,
        const RTIXCdrTypeCodeAnnotations *annotations)
{
    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(annotations == NULL, return RTI_XCDR_FALSE);

    switch (annotations->_defaultValue._d) {
    case RTI_XCDR_TK_NULL:
        /*
         * If the discriminator of the default value is not set, the default is
         * not being changed.
         */
        return RTI_XCDR_FALSE;
    case RTI_XCDR_TK_SHORT:
        return (annotations->_defaultValue._u.short_value != RTIXCdrShort_DEFAULT);
    case RTI_XCDR_TK_LONG:
        return (annotations->_defaultValue._u.long_value != RTIXCdrLong_DEFAULT);
    case RTI_XCDR_TK_USHORT:
        return (annotations->_defaultValue._u.ushort_value != RTIXCdrUnsignedShort_DEFAULT);
    case RTI_XCDR_TK_ULONG:
        return (annotations->_defaultValue._u.ulong_value != RTIXCdrUnsignedLong_DEFAULT);
    case RTI_XCDR_TK_FLOAT:
        return !RTIXCdrUtility_floatNearlyEqual(
                annotations->_defaultValue._u.float_value,
                RTIXCdrFloat_DEFAULT);
    case RTI_XCDR_TK_DOUBLE:
        return !RTIXCdrUtility_doubleNearlyEqual(
                annotations->_defaultValue._u.double_value,
                RTIXCdrDouble_DEFAULT);
    case RTI_XCDR_TK_OCTET:
        return (annotations->_defaultValue._u.octet_value != RTIXCdrOctet_DEFAULT);
    case RTI_XCDR_TK_INT8:
        return annotations->_defaultValue._u.int8_value != RTIXCdrInt8_DEFAULT;
    case RTI_XCDR_TK_UINT8:
        return (annotations->_defaultValue._u.uint8_value != RTIXCdrUInt8_DEFAULT);
    case RTI_XCDR_TK_LONGLONG:
        return (annotations->_defaultValue._u.long_long_value != RTIXCdrLongLong_DEFAULT);
    case RTI_XCDR_TK_ULONGLONG:
        return (annotations->_defaultValue._u.ulong_long_value != RTIXCdrUnsignedLongLong_DEFAULT);
    case RTI_XCDR_TK_CHAR:
        return (annotations->_defaultValue._u.char_value != RTIXCdrChar_DEFAULT);
    case RTI_XCDR_TK_WCHAR:
        return (annotations->_defaultValue._u.wchar_value != RTIXCdrWChar_DEFAULT);
    case RTI_XCDR_TK_BOOLEAN:
        return (annotations->_defaultValue._u.boolean_value != RTIXCdrBoolean_DEFAULT);
    case RTI_XCDR_TK_STRING:
        /*
         * string_value and wstring_value can be NULL if the TypeCode is created
         * through the TypeCode API. In these cases NULL is also considered
         * default.
         */
        return ((annotations->_defaultValue._u.string_value != NULL) &&
            (*(annotations->_defaultValue._u.string_value) != 0));
    case RTI_XCDR_TK_WSTRING:
        return ((annotations->_defaultValue._u.wstring_value != NULL) &&
            (*(annotations->_defaultValue._u.wstring_value) != 0));
    case RTI_XCDR_TK_ENUM:
    {
        RTIXCdrLong memberOrdinal = 0;
        RTIXCdrTCKind topLevelKind = RTIXCdrTypeCode_getKind(self);

        if (topLevelKind == RTI_XCDR_TK_ALIAS) {
            const RTIXCdrTypeCode *resolvedTypeCode =
                    RTIXCdrTypeCode_resolveAlias(self);
            RTIXCdrLog_testPrecondition(
                    resolvedTypeCode == NULL,
                    return RTI_XCDR_FALSE);
            /*
             * For aliases to enums, the default value is the default value of
             * the resolved type.
             */
            return (annotations->_defaultValue._u.enumerated_value
                    != resolvedTypeCode->_annotations._defaultValue._u
                               .enumerated_value);
        }

        RTIXCdrLog_testPrecondition(
                self->_memberCount == 0,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                self->_members == NULL,
                return RTI_XCDR_FALSE);

        /*
         * For plain enums (not aliases), the default default value is the first
         * literal, so we need to obtain the first member using the typecode
         * and retrieve its ordinal. We only want to return true if a member
         * other than the first member is the default.
         */
        RTIXCdrTypeCode_getMemberOrdinal(self, 0, &memberOrdinal);
        return (annotations->_defaultValue._u.enumerated_value
                != memberOrdinal);
    } break;
    default:
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCode_hasMemberNonDefaultDefault(
        const struct RTIXCdrTypeCode *self,
        const struct RTIXCdrTypeCodeMember *member,
        const RTIXCdrUnsignedLong memberIndex)
{
    const RTIXCdrTypeCode *memberTypeCode = NULL;

    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(member == NULL, return RTI_XCDR_FALSE);

    memberTypeCode = RTIXCdrTypeCode_getMemberTypeCode(self, memberIndex);
    if (memberTypeCode == NULL) {
        return RTI_XCDR_FALSE;
    }

    return RTIXCdrTypeCode_hasNonDefaultDefault(
            memberTypeCode,
            &member->_annotations);
}

RTIXCdrBoolean RTIXCdrAnnotationParameterValue_equals(
        const RTIXCdrAnnotationParameterValue *first,
        const RTIXCdrAnnotationParameterValue *second)
{
    RTIXCdrLog_testPrecondition(first == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(second == NULL, return RTI_XCDR_FALSE);

    if (first->_d != second->_d) {
        return RTI_XCDR_FALSE;
    }

    switch (first->_d) {
    case RTI_XCDR_TK_NULL:
        return RTI_XCDR_TRUE;
    case RTI_XCDR_TK_SHORT:
        return first->_u.short_value == second->_u.short_value;
    case RTI_XCDR_TK_LONG:
        return first->_u.long_value == second->_u.long_value;
    case RTI_XCDR_TK_USHORT:
        return first->_u.ushort_value == second->_u.ushort_value;
    case RTI_XCDR_TK_ULONG:
        return first->_u.ulong_value == second->_u.ulong_value;
    case RTI_XCDR_TK_FLOAT:
        return RTIXCdrUtility_floatNearlyEqual(
                first->_u.float_value,
                second->_u.float_value);
    case RTI_XCDR_TK_DOUBLE:
        return RTIXCdrUtility_doubleNearlyEqual(
                first->_u.double_value,
                second->_u.double_value);
    case RTI_XCDR_TK_BOOLEAN:
        return first->_u.boolean_value == second->_u.boolean_value;
    case RTI_XCDR_TK_CHAR:
        return first->_u.char_value == second->_u.char_value;
    case RTI_XCDR_TK_OCTET:
        return first->_u.octet_value == second->_u.octet_value;
    case RTI_XCDR_TK_INT8:
        return first->_u.int8_value == second->_u.int8_value;
    case RTI_XCDR_TK_UINT8:
        return first->_u.uint8_value == second->_u.uint8_value;
    case RTI_XCDR_TK_ENUM:
        return first->_u.enumerated_value == second->_u.enumerated_value;
    case RTI_XCDR_TK_STRING:
        if (first->_u.string_value == NULL && second->_u.string_value == NULL) {
            return RTI_XCDR_TRUE;
        }
        if (first->_u.string_value == NULL) {
            if (second->_u.string_value[0] == 0) {
                return RTI_XCDR_TRUE;
            }
            return RTI_XCDR_FALSE;
        }
        if (second->_u.string_value == NULL) {
            if (first->_u.string_value[0] == 0) {
                return RTI_XCDR_TRUE;
            }
            return RTI_XCDR_FALSE;
        }
        return RTIXCdrString_cmp(
                       first->_u.string_value,
                       second->_u.string_value)
                == 0;
    case RTI_XCDR_TK_LONGLONG:
        return first->_u.long_long_value == second->_u.long_long_value;
    case RTI_XCDR_TK_ULONGLONG:
        return first->_u.ulong_long_value == second->_u.ulong_long_value;
        /*
         * Not supported yet
         * case RTI_XCDR_TK_LONGDOUBLE:
         *   if (first->_u.long_double_value != second->_u.long_double_value) {
         *        return RTI_XCDR_FALSE;
         *   }
         *   break;
         */
    case RTI_XCDR_TK_WCHAR:
        return first->_u.wchar_value == second->_u.wchar_value;
    case RTI_XCDR_TK_WSTRING:
        if (first->_u.wstring_value == NULL
                && second->_u.wstring_value == NULL) {
            return RTI_XCDR_TRUE;
        }
        if (first->_u.wstring_value == NULL) {
            if (second->_u.wstring_value[0] == 0) {
                return RTI_XCDR_TRUE;
            }
            return RTI_XCDR_FALSE;
        }
        if (second->_u.wstring_value == NULL) {
            if (first->_u.wstring_value[0] == 0) {
                return RTI_XCDR_TRUE;
            }
            return RTI_XCDR_FALSE;
        }
        return RTIXCdrWString_cmp(
                       first->_u.wstring_value,
                       second->_u.wstring_value) == 0;
    default:
        return RTI_XCDR_FALSE;
    }
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_hasNonDefaultRangeMinMaxI(
        const struct RTIXCdrTypeCodeAnnotations *annotations,
        RTIXCdrBoolean *hasNonDefaultMinParam,
        RTIXCdrBoolean *hasNonDefaultMaxParam)
{
    RTIXCdrBoolean hasNonDefaultMin = RTI_XCDR_FALSE;
    RTIXCdrBoolean hasNonDefaultMax = RTI_XCDR_FALSE;

    RTIXCdrLog_testPrecondition(annotations == NULL, return RTI_XCDR_FALSE);

    if (hasNonDefaultMinParam != NULL) {
        *hasNonDefaultMinParam = hasNonDefaultMin;
    }
    if (hasNonDefaultMaxParam != NULL) {
        *hasNonDefaultMaxParam = hasNonDefaultMax;
    }

    switch (annotations->_minValue._d) {
    case RTI_XCDR_TK_SHORT:
        hasNonDefaultMin =
                (annotations->_minValue._u.short_value != RTIXCdrShort_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.short_value != RTIXCdrShort_MAX);
        break;
    case RTI_XCDR_TK_LONG:
        hasNonDefaultMin =
                (annotations->_minValue._u.long_value != RTIXCdrLong_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.long_value != RTIXCdrLong_MAX);
        break;
    case RTI_XCDR_TK_USHORT:
        hasNonDefaultMin =
                (annotations->_minValue._u.ushort_value
                 != RTIXCdrUnsignedShort_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.ushort_value
                 != RTIXCdrUnsignedShort_MAX);
        break;
    case RTI_XCDR_TK_ULONG:
        hasNonDefaultMin =
                (annotations->_minValue._u.ulong_value
                 != RTIXCdrUnsignedLong_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.ulong_value
                 != RTIXCdrUnsignedLong_MAX);
        break;
    /*
     * For FLOAT and DOUBLE, we check if the max|min is higher|lower than the
     * default. This is because we are not using the limits provided by
     * e.g., limits.h
     */
    case RTI_XCDR_TK_FLOAT:
        hasNonDefaultMin =
                (annotations->_minValue._u.float_value > RTIXCdrFloat_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.float_value < RTIXCdrFloat_MAX);
        break;
    case RTI_XCDR_TK_DOUBLE:
        hasNonDefaultMin =
                (annotations->_minValue._u.double_value > RTIXCdrDouble_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.double_value < RTIXCdrDouble_MAX);
        break;
    case RTI_XCDR_TK_OCTET:
        hasNonDefaultMin =
                (annotations->_minValue._u.octet_value != RTIXCdrOctet_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.octet_value != RTIXCdrOctet_MAX);
        break;
    case RTI_XCDR_TK_INT8:
        hasNonDefaultMin =
                (annotations->_minValue._u.int8_value != RTIXCdrInt8_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.int8_value != RTIXCdrInt8_MAX);
        break;
    case RTI_XCDR_TK_UINT8:
        hasNonDefaultMin =
                (annotations->_minValue._u.uint8_value != RTIXCdrUInt8_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.uint8_value != RTIXCdrUInt8_MAX);
        break;
    case RTI_XCDR_TK_LONGLONG:
        hasNonDefaultMin =
                (annotations->_minValue._u.long_long_value
                 != RTIXCdrLongLong_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.long_long_value
                 != RTIXCdrLongLong_MAX);
        break;
    case RTI_XCDR_TK_ULONGLONG:
        hasNonDefaultMin =
                (annotations->_minValue._u.ulong_long_value
                 != RTIXCdrUnsignedLongLong_MIN);
        hasNonDefaultMax =
                (annotations->_maxValue._u.ulong_long_value
                 != RTIXCdrUnsignedLongLong_MAX);
        break;
    default:
        break;
    }

    if (hasNonDefaultMinParam != NULL) {
        *hasNonDefaultMinParam = hasNonDefaultMin;
    }
    if (hasNonDefaultMaxParam != NULL) {
        *hasNonDefaultMaxParam = hasNonDefaultMax;
    }

    return hasNonDefaultMin || hasNonDefaultMax;
}

RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_hasNonDefaultRange(
        const struct RTIXCdrTypeCodeAnnotations *annotations)
{
    RTIXCdrLog_testPrecondition(annotations == NULL, return RTI_XCDR_FALSE);
    return RTIXCdrTypeCodeAnnotations_hasNonDefaultRangeMinMaxI(
            annotations,
            NULL /* hasNonDefaultMin */,
            NULL /* hasNonDefaultMax */);
}

RTIXCdrBoolean RTIXCdrTypeCodeAnnotations_hasNonDefaultRangeMinMax(
        const struct RTIXCdrTypeCodeAnnotations *annotations,
        RTIXCdrBoolean *hasNonDefaultMin,
        RTIXCdrBoolean *hasNonDefaultMax)
{
    RTIXCdrLog_testPrecondition(annotations == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            hasNonDefaultMin == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            hasNonDefaultMax == NULL,
            return RTI_XCDR_FALSE);
    return RTIXCdrTypeCodeAnnotations_hasNonDefaultRangeMinMaxI(
            annotations,
            hasNonDefaultMin,
            hasNonDefaultMax);
}

RTIXCdrChar *RTIXCdrTypeCodeAnnotations_getUnit(
        const struct RTIXCdrTypeCodeAnnotations *annotations)
{
    RTIXCdrChar *unit = NULL;

    RTIXCdrLog_testPrecondition(annotations == NULL, return NULL);

    if (annotations->_unitValue != NULL && annotations->_unitValue[0] != '\0') {
        unit = annotations->_unitValue;
    }

    return unit;
}

/* Returns true if any of the members in the input TypeCode have a non default
 * value for the min, max, and range annotations
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_hasNonDefaultRangeAnnotationWithNode(
        const RTIXCdrTypeCode *tc,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrTCKind kind;
    struct RTIXCdrTypeCodeNode visitedNode;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        /* Type is unbounded if there is recursion */
        return RTI_XCDR_TRUE;
    }

    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ALIAS:
        {
            if (RTIXCdrTypeCodeAnnotations_hasNonDefaultRange(
                    &tc->_annotations)) {
                return RTI_XCDR_TRUE;
            }

            return RTIXCdrTypeCode_hasNonDefaultRangeAnnotation(
                    tc->_typeCode);
        } break;
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_UNION:
        {
            RTIXCdrUnsignedLong i = 0;

            if (kind == RTI_XCDR_TK_VALUE && RTIXCdrTypeCode_hasBase(tc)) {
                if (RTIXCdrTypeCode_hasNonDefaultRangeAnnotationWithNode(
                        tc->_typeCode,
                        &visitedNode)) {
                    return RTI_XCDR_TRUE;
                }
            }

            for (i=0; i<tc->_memberCount; i++) {
                if (RTIXCdrTypeCodeAnnotations_hasNonDefaultRange(
                        &tc->_members[i]._annotations)) {
                    return RTI_XCDR_TRUE;
                }

                if (RTIXCdrTypeCode_hasNonDefaultRangeAnnotationWithNode(
                        tc->_members[i]._representation._typeCode,
                        &visitedNode)) {
                    return RTI_XCDR_TRUE;
                }
            }

            return RTI_XCDR_FALSE;
        } break;
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ARRAY:
        {
            if (RTIXCdrTypeCode_hasNonDefaultRangeAnnotationWithNode(
                    tc->_typeCode,
                    &visitedNode)) {
                return RTI_XCDR_TRUE;
            }
        } break;
        default:
            return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCode_hasNonDefaultRangeAnnotation(
        const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode;

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_hasNonDefaultRangeAnnotationWithNode(
            tc,
            &visitedNode);
}

/*
 * @brief Private version of RTIXCdrTypeCode_getAggregationTypeCodeCount that
 * receives the parentVisitedNode to avoid infinite recursion in TypeCodes with
 * cycles.
 * 
 * @param tc In. The TypeCode.
 * @param parentVisitedNode In. Node corresponding to the last visited TypeCode.
 * @return Number of aggregation TypeCodes.
 */
RTI_PRIVATE
RTIXCdrUnsignedLong RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
        const RTIXCdrTypeCode *tc,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrUnsignedLong count = 0;
    RTIXCdrTCKind kind;
    struct RTIXCdrTypeCodeNode visitedNode;

    RTIXCdrLog_testPrecondition(tc == NULL, return 0);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        return 0;
    }

    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ALIAS:
        {
            count += RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
                    tc->_typeCode,
                    &visitedNode);
        } break;
        case RTI_XCDR_TK_STRUCT:
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_UNION:
        {
            RTIXCdrUnsignedLong i = 0;

            count++;

            if (kind == RTI_XCDR_TK_VALUE && RTIXCdrTypeCode_hasBase(tc)) {
                count += RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
                        tc->_typeCode,
                        &visitedNode);
            }

            for (i=0; i<tc->_memberCount; i++) {
                count += RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
                        tc->_members[i]._representation._typeCode,
                        &visitedNode);
            }
        } break;
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ARRAY:
        {
            count += RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
                    tc->_typeCode,
                    &visitedNode);
        } break;
        default:
        {
        } break;
    }

    return count;
}

RTIXCdrUnsignedLong RTIXCdrTypeCode_getAggregationTypeCodeCount(
        const RTIXCdrTypeCode *tc)
{
    struct RTIXCdrTypeCodeNode visitedNode;

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_getAggregationTypeCodeCountWithNode(
            tc,
            &visitedNode);
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_isPrimitive(const RTIXCdrTypeCode *typeCode)
{
    RTIXCdrTCKind typeKind;

    RTIXCdrLog_testPrecondition(typeCode == NULL, return RTI_XCDR_FALSE);

    typeKind = RTIXCdrTypeCode_getKind(typeCode);

    return RTIXCdrTypeCode_isPrimitiveKind(typeKind, RTI_XCDR_FALSE);
}

void RTIXCdrTypeCode_destroyTypeCode(RTIXCdrTypeCode *self)
{
    RTIXCdrUnsignedLong i = 0;

    if (self == NULL) {
        return;
    }

    /* members */
    for (i = 0; i < self->_memberCount; ++i) {
        if (self->_members[i]._representation._typeCode != NULL) {
            if (!RTIXCdrTypeCode_isPrimitive(
                        self->_members[i]._representation._typeCode)) {
                RTIXCdrTypeCode_destroyTypeCode(
                        self->_members[i]._representation._typeCode);
            }
        }
        if (self->_members[i]._name != NULL) {
            RTIXCdrHeap_freeString(self->_members[i]._name);
        }
        if (self->_members[i]._labelsCount > 1) {
            RTIXCdrHeap_freeArray(self->_members[i]._labels);
        }
        RTIXCdrTypeCodeAnnotations_finalize(&self->_members[i]._annotations);
    }
    if (self->_members != NULL) {
        RTIXCdrHeap_freeArray(self->_members);
        self->_memberCount = 0;
    }
    if (self->_name != NULL) {
        RTIXCdrHeap_freeString(self->_name);
    }
    /* dimension */
    if (self->_dimensionsCount > 1) {
        RTIXCdrHeap_freeArray(self->_dimensions);
    }
    self->_dimensionsCount = 0;

    /* base type code */
    if (self->_typeCode != NULL) {
        /* array or sequences may have a primitive base typecode */
        if (!RTIXCdrTypeCode_isPrimitive(self->_typeCode)) {
            RTIXCdrTypeCode_destroyTypeCode(self->_typeCode);
        }
    }

    RTIXCdrTypeCodeAnnotations_finalize(&self->_annotations);

    RTIXCdrHeap_freeStruct(self);
}

RTI_PRIVATE
RTIXCdrTypeCode *RTIXCdrTypeCode_getDiscriminatorType(
        const RTIXCdrTypeCode *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return NULL);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(self) != RTI_XCDR_TK_UNION,
            return NULL);

    return self->_typeCode;
}

RTI_PRIVATE
void RTIXCdrTypeCode_getMemberFlags(
        const RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong index,
        RTIXCdrMemberFlags *memberFlags)
{
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(memberFlags == NULL, return);

    kind = RTIXCdrTypeCode_getKind(self);

    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT && kind != RTI_XCDR_TK_VALUE
                    && kind != RTI_XCDR_TK_UNION,
            return);

    RTIXCdrLog_testPrecondition(self->_memberCount == 0, return);
    RTIXCdrLog_testPrecondition(index >= self->_memberCount, return);

    if (kind == RTI_XCDR_TK_UNION) {
        /*
         * Union members are always non-key, optional.
         *
         * TypeCodes don't serialize these flags for unions, so we need to
         * return this value explicitly.
         */
        *memberFlags = RTI_XCDR_NONKEY_MEMBER;
    } else {
        const RTIXCdrTypeCodeMember *member = &self->_members[index];

        *memberFlags = member->_memberFlags;
    }
}

/**
 * @brief Checks if a member of a TypeCode is required.
 *
 * @param[in] self Pointer to the TypeCode object.
 * @param[in] index The index of the member to check.
 * @param[out] isRequired Pointer to a boolean that will be set to RTI_XCDR_TRUE
 * if the member is required, RTI_XCDR_FALSE otherwise.
 */
RTI_PRIVATE
void RTIXCdrTypeCode_isMemberRequired(
        const RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong index,
        RTIXCdrBoolean *isRequired)
{
    RTIXCdrMemberFlags memberFlags = 0;

    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(isRequired == NULL, return);

    RTIXCdrTypeCode_getMemberFlags(self, index, &memberFlags);

    if (memberFlags
            & (RTI_XCDR_FLAG_REQUIRED_MEMBER | RTI_XCDR_FLAG_KEY_MEMBER)) {
        *isRequired = RTI_XCDR_TRUE;
    } else {
        *isRequired = RTI_XCDR_FALSE;
    }
}

/**
 * @brief Checks if a member of a TypeCode is a key.
 *
 * @param[in] self Pointer to the TypeCode object.
 * @param[in] index The index of the member to check.
 * @param[out] isKey Pointer to a boolean that will be set to RTI_XCDR_TRUE if
 * the member is a key, RTI_XCDR_FALSE otherwise.
 */
RTI_PRIVATE
void RTIXCdrTypeCode_isMemberKey(
        const RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong index,
        RTIXCdrBoolean *isKey)
{
    RTIXCdrMemberFlags memberFlags = 0;

    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(isKey == NULL, return);

    RTIXCdrTypeCode_getMemberFlags(self, index, &memberFlags);

    *isKey = memberFlags & RTI_XCDR_FLAG_KEY_MEMBER;
}

/**
 * @brief Checks if a member of a TypeCode is marked as must understand.
 *
 * @param[in] self Pointer to the TypeCode object.
 * @param[in] index The index of the member to check.
 * @param[out] isMustUnderstand Pointer to a boolean that will be set to
 * RTI_XCDR_TRUE if the member is marked as must understand, RTI_XCDR_FALSE
 * otherwise.
 */
RTI_PRIVATE
void RTIXCdrTypeCode_isMemberMustUnderstand(
        const RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong index,
        RTIXCdrBoolean *isMustUnderstand)
{
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(isMustUnderstand == NULL, return);

    /* Explicitly initialize the output value to avoid coverity warnings
     * if any of the following preconditions is false. */
    RTIXCdrLog_preconditionOnly(*isMustUnderstand = RTI_XCDR_FALSE);

    /* must_understand flag is only available within aggregate types */
    kind = RTIXCdrTypeCode_getKind(self);
    RTIXCdrLog_testPrecondition(
            kind != RTI_XCDR_TK_STRUCT && kind != RTI_XCDR_TK_VALUE
                    && kind != RTI_XCDR_TK_UNION,
            return);

    /* Check _members boundaries */
    RTIXCdrLog_testPrecondition(index >= self->_memberCount, return);

    *isMustUnderstand =
            (kind != RTI_XCDR_TK_UNION
             && RTIXCdrTypeCodeMember_isMustUnderstand(&self->_members[index]))
            ? RTI_XCDR_TRUE
            : RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCodeMember_isMustUnderstand(
        const RTIXCdrTypeCodeMember *self)
{
    RTIXCdrLog_testPrecondition(self == NULL, return RTI_XCDR_FALSE);

    return self->_memberFlags & RTI_XCDR_FLAG_KEY_MEMBER
            || self->_annotations._isMustUnderstand;
}

void RTIXCdrTypeCodeMember_getId(
        const RTIXCdrTypeCodeMember *self,
        RTIXCdrUnsignedLong *idOut)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(idOut == NULL, return);

    *idOut = self->_representation._pid;
}

RTI_PRIVATE
void RTIXCdrTypeCode_getArrayDimension(
        const RTIXCdrTypeCode *self,
        RTIXCdrUnsignedLong index,
        RTIXCdrUnsignedLong *dimension)
{
    RTIXCdrLog_testPrecondition(self == NULL, return);
    RTIXCdrLog_testPrecondition(dimension == NULL, return);

    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_getKind(self) != RTI_XCDR_TK_ARRAY,
            return);

    RTIXCdrLog_testPrecondition(self->_dimensionsCount == 0, return);
    RTIXCdrLog_testPrecondition(index >= self->_dimensionsCount, return);

    if (self->_dimensionsCount == 1) {
        *dimension = self->_maximumLength;
    } else {
        *dimension = self->_dimensions[index];
    }
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrStructTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrUnsignedLong i;
    RTIXCdrTCKind kind1, kind2;
    RTIXCdrBoolean isKey1, isKey2;
    RTIXCdrBoolean isMustUnderstand1, isMustUnderstand2;
    RTIXCdrUnsignedLong pid1, pid2;
    RTIXCdrBoolean isRequired1, isRequired2;

    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    kind1 = RTIXCdrTypeCode_getKind(typecode1);
    kind2 = RTIXCdrTypeCode_getKind(typecode2);

    if (!property->ignoreTypeCodeNames) {
        RTIXCdrLog_testPrecondition(typecode1->_name == NULL, return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(typecode2->_name == NULL, return RTI_XCDR_FALSE);
        if (RTIXCdrString_cmp(typecode1->_name, typecode2->_name) != 0) {
            return RTI_XCDR_FALSE;
        }
    }

    /*
     * RTI_XCDR_TK_STRUCT can never derive from another type, typecode
     * structures with a base type are converted to value types.
     */
    if (kind1 == RTI_XCDR_TK_VALUE || kind2 == RTI_XCDR_TK_VALUE) {
        /* Check whether they both have base types or not */
        if (RTIXCdrTypeCode_hasBase(typecode1)
                != RTIXCdrTypeCode_hasBase(typecode2)) {
            return RTI_XCDR_FALSE;
        }

        /* If they have base types, check if they are equal */
        if (RTIXCdrTypeCode_hasBase(typecode1)
                && !RTIXCdrTypeCode_equal(
                    typecode1->_typeCode,
                    typecode2->_typeCode,
                    property)) {
            return RTI_XCDR_FALSE;
        }

        if (typecode1->_typeModifier != typecode2->_typeModifier) {
            return RTI_XCDR_FALSE;
        }
    }

    if (typecode1->_memberCount != typecode2->_memberCount) {
        return RTI_XCDR_FALSE;
    }

    for (i = 0; i < typecode1->_memberCount; ++i) {
        RTIXCdrLog_testPrecondition(
                typecode1->_members[i]._name == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_members[i]._name == NULL,
                return RTI_XCDR_FALSE);

        if (!property->ignoreTypeCodeNames) {
            if (RTIXCdrString_cmp(
                        typecode1->_members[i]._name,
                        typecode2->_members[i]._name)
                    != 0) {
                return RTI_XCDR_FALSE;
            }
        }

        RTIXCdrTypeCode_isMemberRequired(typecode1, i, &isRequired1);
        RTIXCdrTypeCode_isMemberRequired(typecode2, i, &isRequired2);
        if (isRequired1 != isRequired2) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrTypeCode_isMemberKey(typecode1, i, &isKey1);
        RTIXCdrTypeCode_isMemberKey(typecode2, i, &isKey2);
        if (isKey1 != isKey2) {
            return RTI_XCDR_FALSE;
        }

        isMustUnderstand1 = RTI_XCDR_FALSE;
        RTIXCdrTypeCode_isMemberMustUnderstand(
                typecode1,
                i,
                &isMustUnderstand1);
        isMustUnderstand2 = RTI_XCDR_FALSE;
        RTIXCdrTypeCode_isMemberMustUnderstand(
                typecode2,
                i,
                &isMustUnderstand2);
        if (isMustUnderstand1 != isMustUnderstand2) {
            return RTI_XCDR_FALSE;
        }

        if (!property->ignoreTypeCodeAnnotations) {
            /* It may be more correct to call RTIXCdrTypeCodeAnnotations_equals
             * here, but for time and risk reasons we only updated this code
             * path to check unit instead of adding in additional checks for
             * annotations that we were not checking before */
            if (!RTIXCdrTypeCodeAnnotations_memberSubsetEquals(
                        &typecode1->_members[i]._annotations,
                        &typecode2->_members[i]._annotations)) {
                return RTI_XCDR_FALSE;
            }
        }

        RTIXCdrTypeCodeMember_getId(&typecode1->_members[i], &pid1);
        RTIXCdrTypeCodeMember_getId(&typecode2->_members[i], &pid2);
        if (pid1 != pid2) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrLog_testPrecondition(
                typecode1->_members[i]._representation._typeCode == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_members[i]._representation._typeCode == NULL,
                return RTI_XCDR_FALSE);

        if (!RTIXCdrTypeCode_equal(
                    typecode1->_members[i]._representation._typeCode,
                    typecode2->_members[i]._representation._typeCode,
                    property)) {
            return RTI_XCDR_FALSE;
        }

        if (typecode1->_members[i]._representation._isPointer
            != typecode2->_members[i]._representation._isPointer) {
            return RTI_XCDR_FALSE;
        }

        if (typecode1->_members[i]._visibility
                != typecode2->_members[i]._visibility) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrUnionTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrUnsignedLong i, j;
    RTIXCdrUnsignedLong pid1, pid2;

    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (!property->ignoreTypeCodeNames) {
        RTIXCdrLog_testPrecondition(
                typecode1->_name == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_name == NULL,
                return RTI_XCDR_FALSE);
        if (RTIXCdrString_cmp(typecode1->_name, typecode2->_name) != 0) {
            return RTI_XCDR_FALSE;
        }
    }

    if (typecode1->_default_index != typecode2->_default_index) {
        return RTI_XCDR_FALSE;
    }

    if (!RTIXCdrTypeCode_equal(
                RTIXCdrTypeCode_getDiscriminatorType(typecode1),
                RTIXCdrTypeCode_getDiscriminatorType(typecode2),
                property)) {
        return RTI_XCDR_FALSE;
    }

    if (typecode1->_memberCount != typecode2->_memberCount) {
        return RTI_XCDR_FALSE;
    }

    for (i = 0; i < typecode1->_memberCount; ++i) {
        RTIXCdrBoolean isRequired1, isRequired2;

        if (!property->ignoreTypeCodeNames) {
            RTIXCdrLog_testPrecondition(
                    typecode1->_members[i]._name == NULL,
                    return RTI_XCDR_FALSE);
            RTIXCdrLog_testPrecondition(
                    typecode2->_members[i]._name == NULL,
                    return RTI_XCDR_FALSE);

            if (RTIXCdrString_cmp(
                        typecode1->_members[i]._name,
                        typecode2->_members[i]._name)
                    != 0) {
                return RTI_XCDR_FALSE;
            }
        }

        RTIXCdrTypeCode_isMemberRequired(typecode1, i, &isRequired1);
        RTIXCdrTypeCode_isMemberRequired(typecode2, i, &isRequired2);
        if (isRequired1 != isRequired2) {
            return RTI_XCDR_FALSE;
        }

        if (!property->ignoreTypeCodeAnnotations) {
            if (!RTIXCdrTypeCodeAnnotations_memberSubsetEquals(
                        &typecode1->_members[i]._annotations,
                        &typecode2->_members[i]._annotations)) {
                return RTI_XCDR_FALSE;
            }
        }

        RTIXCdrTypeCodeMember_getId(&typecode1->_members[i], &pid1);
        RTIXCdrTypeCodeMember_getId(&typecode2->_members[i], &pid2);
        if (pid1 != pid2) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrLog_testPrecondition(
                typecode1->_members[i]._representation._typeCode == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_members[i]._representation._typeCode == NULL,
                return RTI_XCDR_FALSE);

        if (!RTIXCdrTypeCode_equal(
                    typecode1->_members[i]._representation._typeCode,
                    typecode2->_members[i]._representation._typeCode,
                    property)) {
            return RTI_XCDR_FALSE;
        }

        if (typecode1->_members[i]._representation._isPointer
            != typecode2->_members[i]._representation._isPointer) {
            return RTI_XCDR_FALSE;
        }

        RTIXCdrLog_testPrecondition(
                typecode1->_members[i]._labelsCount <= 0,
                return RTI_XCDR_FALSE);

        if (typecode1->_members[i]._labelsCount
                != typecode2->_members[i]._labelsCount) {
            return RTI_XCDR_FALSE;
        }

        if (typecode1->_members[i]._labelsCount == 1) {
            if (typecode1->_members[i]._label
                    != typecode2->_members[i]._label) {
                return RTI_XCDR_FALSE;
            }
        } else {
            for (j = 0; j < typecode1->_members[i]._labelsCount; j++) {
                if (typecode1->_members[i]._labels[j]
                        != typecode2->_members[i]._labels[j]) {
                    return RTI_XCDR_FALSE;
                }
            }
        }
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrEnumTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrUnsignedLong i;

    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);

    if (!property->ignoreTypeCodeNames) {
        RTIXCdrLog_testPrecondition(typecode1->_name == NULL, return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(typecode2->_name == NULL, return RTI_XCDR_FALSE);

        if (RTIXCdrString_cmp(typecode1->_name, typecode2->_name) != 0) {
            return RTI_XCDR_FALSE;
        }
    }

    if (typecode1->_memberCount != typecode2->_memberCount) {
        return RTI_XCDR_FALSE;
    }

    for (i = 0; i < typecode1->_memberCount; ++i) {
        RTIXCdrLog_testPrecondition(
                typecode1->_members[i]._name == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_members[i]._name == NULL,
                return RTI_XCDR_FALSE);

        if (!property->ignoreTypeCodeNames) {
            if (RTIXCdrString_cmp(
                        typecode1->_members[i]._name,
                        typecode2->_members[i]._name)
                    != 0) {
                return RTI_XCDR_FALSE;
            }
        }

        if (typecode1->_members[i]._ordinal
                != typecode2->_members[i]._ordinal) {
            return RTI_XCDR_FALSE;
        }

    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrSequenceTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (typecode1->_maximumLength != typecode2->_maximumLength) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrLog_testPrecondition(
            typecode1->_typeCode == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            typecode2->_typeCode == NULL,
            return RTI_XCDR_FALSE);

    if (!RTIXCdrTypeCode_equal(
                typecode1->_typeCode,
                typecode2->_typeCode,
                property)) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrArrayTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong dimension1 = 0, dimension2 = 0;

    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (typecode1->_maximumLength != typecode2->_maximumLength) {
        return RTI_XCDR_FALSE;
    }

    RTIXCdrLog_testPrecondition(
            typecode1->_dimensionsCount <= 0,
            return RTI_XCDR_FALSE);

    if (typecode1->_dimensionsCount != typecode2->_dimensionsCount) {
        return RTI_XCDR_FALSE;
    }

    /* dimension array is only used for more than 1 dimension */
    for (i = 1; i < typecode1->_dimensionsCount; i++) {
        RTIXCdrTypeCode_getArrayDimension(
                typecode1,
                i,
                &dimension1);

        RTIXCdrTypeCode_getArrayDimension(
                typecode2,
                i,
                &dimension2);

        if (dimension1 != dimension2) {
            return RTI_XCDR_FALSE;
        }
    }

    RTIXCdrLog_testPrecondition(
            typecode1->_typeCode == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            typecode2->_typeCode == NULL,
            return RTI_XCDR_FALSE);

    if (!RTIXCdrTypeCode_equal(
                typecode1->_typeCode,
                typecode2->_typeCode,
                property)) {
        return RTI_XCDR_FALSE;
    }

    return RTI_XCDR_TRUE;
}

RTI_PRIVATE
RTIXCdrBoolean RTIXCdrAliasTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            typecode1->_typeCode == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            typecode2->_typeCode == NULL,
            return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);

    if (!property->ignoreTypeCodeNames) {
        RTIXCdrLog_testPrecondition(
                typecode1->_name == NULL,
                return RTI_XCDR_FALSE);
        RTIXCdrLog_testPrecondition(
                typecode2->_name == NULL,
                return RTI_XCDR_FALSE);
        if (RTIXCdrString_cmp(typecode1->_name, typecode2->_name) != 0) {
            return RTI_XCDR_FALSE;
        }
    }

    if (!RTIXCdrTypeCode_equal(
                typecode1->_typeCode,
                typecode2->_typeCode,
                property)) {
        return RTI_XCDR_FALSE;
    }

    if (typecode1->_isPointer != typecode2->_isPointer) {
        return RTI_XCDR_FALSE;
    }

    if (!property->ignoreTypeCodeAnnotations) {
        if (!RTIXCdrTypeCodeAnnotations_equals(
                    &typecode1->_annotations,
                    &typecode2->_annotations)) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrTypeCode_equal(
        const RTIXCdrTypeCode *typecode1,
        const RTIXCdrTypeCode *typecode2,
        const RTIXCdrTypeCodeCompareProperty *property)
{
    RTIXCdrTCKind kind1, kind2;

    RTIXCdrLog_testPrecondition(typecode1 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(typecode2 == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(property == NULL, return RTI_XCDR_FALSE);


    kind1 = RTIXCdrTypeCode_getKind(typecode1);
    kind2 = RTIXCdrTypeCode_getKind(typecode2);

    if (kind1 != kind2) {
        /* Starting with 5.0.0 value types and structs are equivalent */
        if (!((kind1 == RTI_XCDR_TK_VALUE && kind2 == RTI_XCDR_TK_STRUCT)
              || (kind1 == RTI_XCDR_TK_STRUCT &&  kind2 == RTI_XCDR_TK_VALUE))) {
            return RTI_XCDR_FALSE;
        }
    }

    if (RTIXCdrTypeCode_getExtensibilityKind(typecode1)
            != RTIXCdrTypeCode_getExtensibilityKind(typecode2)) {
        return RTI_XCDR_FALSE;
    }

    switch (kind1) {
    case RTI_XCDR_TK_VALUE:
    case RTI_XCDR_TK_STRUCT:
        return RTIXCdrStructTypeCode_equal(
                typecode1,
                typecode2,
                property);
    case RTI_XCDR_TK_UNION:
        return RTIXCdrUnionTypeCode_equal(
                typecode1,
                typecode2,
                property);
    case RTI_XCDR_TK_ENUM:
        return RTIXCdrEnumTypeCode_equal(
                typecode1,
                typecode2,
                property);
    case RTI_XCDR_TK_STRING:
    case RTI_XCDR_TK_WSTRING:
        if (typecode1->_maximumLength != typecode2->_maximumLength) {
            return RTI_XCDR_FALSE;
        }
        break;
    case RTI_XCDR_TK_SEQUENCE:
        return RTIXCdrSequenceTypeCode_equal(
                typecode1,
                typecode2,
                property);
    case RTI_XCDR_TK_ARRAY:
        return RTIXCdrArrayTypeCode_equal(
                typecode1,
                typecode2,
                property);
    case RTI_XCDR_TK_ALIAS:
        return RTIXCdrAliasTypeCode_equal(
                typecode1,
                typecode2,
                property);
    default:
        break;
    }

    return RTI_XCDR_TRUE;
}
struct RTIXCdrInterpreterPrograms * RTIXCdrTypeCodeWrapper_createSerializationPrograms(
        const RTIXCdrTypeCodeWrapper *tc,
        const RTIXCdrInterpreterProgramsGenProperty *programProperties)
{
    RTIXCdrLog_testPrecondition(tc == NULL, return NULL);
    RTIXCdrLog_testPrecondition(programProperties == NULL, return NULL);

    return RTIXCdrInterpreterPrograms_new(
            &tc->_data,
            programProperties,
            RTI_XCDR_SER_PROGRAM
                    | RTI_XCDR_DESER_PROGRAM
                    | RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM
                    | RTI_XCDR_GET_SER_SIZE_PROGRAM);
}

RTIXCdrUnsignedLong RTIXCdrTypeCode_getNonPrimitiveCollectionMemberCount(
        const RTIXCdrTypeCode *tc,
        RTIXCdrBoolean enumAsPrimitive,
        RTIXCdrBoolean resolveAlias)
{
    RTIXCdrTCKind kind;
    RTIXCdrUnsignedLong i;
    RTIXCdrUnsignedLong result = 0;
    const RTIXCdrTypeCode *memberTc;
    RTIXCdrTCKind memberTcKind;
    const RTIXCdrTypeCode *collectionElementTc;
    RTIXCdrTCKind collectionElementTcKind;

    RTIXCdrLog_testPrecondition(tc == NULL, return 0);

    kind = RTIXCdrTypeCode_getKind(tc);
    RTIXCdrLog_testPrecondition(
            RTIXCdrTypeCode_isPrimitiveKind(kind, enumAsPrimitive) 
                    || kind == RTI_XCDR_TK_STRING
                    || kind == RTI_XCDR_TK_WSTRING,
            return 0);

    if (kind == RTI_XCDR_TK_ALIAS
            || kind == RTI_XCDR_TK_ARRAY
            || kind == RTI_XCDR_TK_SEQUENCE) {
        if (resolveAlias) {
            collectionElementTc = RTIXCdrTypeCode_resolveAlias(tc->_typeCode);
        } else {
            collectionElementTc = tc->_typeCode;
        }

        /*
         * We always resolve alias here to see if the sequence or array
         * element type is primitive
         */
        collectionElementTc = RTIXCdrTypeCode_resolveAlias(collectionElementTc);
        collectionElementTcKind = RTIXCdrTypeCode_getKind(collectionElementTc);

        if (!RTIXCdrTypeCode_isPrimitiveKind(
                collectionElementTcKind, 
                enumAsPrimitive)) {
            result++;
        }

        return result;
    }

    for (i = 0; i < tc->_memberCount; i++) {
        memberTc = tc->_members[i]._representation._typeCode;

        if (resolveAlias) {
            memberTc = RTIXCdrTypeCode_resolveAlias(memberTc);
        }

        memberTcKind = RTIXCdrTypeCode_getKind(memberTc);

        if (memberTcKind == RTI_XCDR_TK_SEQUENCE
                || memberTcKind == RTI_XCDR_TK_ARRAY) {
            collectionElementTc = memberTc->_typeCode;

            /*
             * We always resolve alias here to see if the sequence or array
             * element type is primitive
             */
            collectionElementTc =
                    RTIXCdrTypeCode_resolveAlias(collectionElementTc);
            collectionElementTcKind =
                    RTIXCdrTypeCode_getKind(collectionElementTc);

            if (!RTIXCdrTypeCode_isPrimitiveKind(
                    collectionElementTcKind, 
                    enumAsPrimitive)) {
                result++;
            }
        }
    }

    return result;
}

/**
 * @brief Returns RTI_XCDR_TRUE if the serialization of a sample for tc may 
 * require padding to 4-byte boundary at the end of the sample.
 *
 * See RTIXCdrTypeCode_sampleMayRequirePadding for more information.
 *
 * This function is friendly to recursion and cycles in the TypeCode.
 *
 * @param[in] tc TypeCode.
 * @param[in] v2Encapsulation RTI_XCDR_TRUE if the sample is being serialized 
 * with v2 encapsulation. RTI_XCDR_FALSE if the sample is being serialized with 
 * v1.
 * @param[inout] parentVisitedNode Node corresponding to the last visited 
 * TypeCode.
 *
 * @return RTI_XCDR_TRUE if the serialization of a sample may require padding to
 * 4-byte boundary at the end of the sample. RTI_XCDR_FALSE otherwise.
 */
RTI_PRIVATE
RTIXCdrBoolean RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrBoolean v2Encapsulation,
        const RTIXCdrTypeCodeNode *parentVisitedNode)
{
    RTIXCdrTypeCodeNode visitedNode;
    RTIXCdrTCKind kind;

    RTIXCdrLog_testPrecondition(tc == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            parentVisitedNode == NULL, 
            return RTI_XCDR_FALSE);

    if (RTIXCdrTypeCode_isTypeCodeVisited(tc, parentVisitedNode)) {
        /* Finish recursion */
        return RTI_XCDR_FALSE;
    }

    visitedNode.tc = tc;
    visitedNode.prev = parentVisitedNode;

    kind = RTIXCdrTypeCode_getKind(tc);

    switch (kind) {
        case RTI_XCDR_TK_ARRAY:
        case RTI_XCDR_TK_SEQUENCE:
        case RTI_XCDR_TK_ALIAS: 
            /* 
             * Padding is required if the type of the alias, array or sequence 
             * requires padding 
             */
            return RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
                    tc->_typeCode,
                    v2Encapsulation,
                    &visitedNode);
        case RTI_XCDR_TK_STRING:
            /* Strings may require padding */
            return RTI_XCDR_TRUE;
        case RTI_XCDR_TK_WSTRING:
            if (v2Encapsulation) {
                
                /* 
                 * wstrings may require padding in v2 encapsulation because each
                 * character is 2-bytes 
                 */
                return RTI_XCDR_TRUE;
            } else {
                
                /* 
                 * In v1 encapsulation, wstrings do not require padding because
                 * each character is 4-bytes
                 */
                return RTI_XCDR_FALSE;
            }
        case RTI_XCDR_TK_VALUE:
        case RTI_XCDR_TK_STRUCT:
        {
            /* 
             * We only look at the last member to see if padding maybe required 
             */
            if (tc->_memberCount == 0) {
                if (RTIXCdrTypeCode_hasBase(tc)) {
                    return RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
                            tc->_typeCode,
                            v2Encapsulation,
                            &visitedNode);
                }

                /* 
                 * To not complicate the algorithm empty structures are marked
                 * as requiring padding 
                 */
                return RTI_XCDR_TRUE;
            }

            if (v2Encapsulation && 
                    RTIXCdrTypeCodeMember_isOptional(
                            &tc->_members[tc->_memberCount - 1])) {
                /* v2 optional members are serialized as a single byte */
                return RTI_XCDR_TRUE;
            }
            return RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
                    tc->_members[tc->_memberCount - 1]
                            ._representation._typeCode,
                    v2Encapsulation,
                    &visitedNode);
        } break;
        case RTI_XCDR_TK_UNION:
        {
            RTIXCdrUnsignedLong i;
            /* 
             * We will return RTI_XCDR_TRUE if any of the members may require 
             * padding including the discriminator.
             */
            for (i = 0; i < tc->_memberCount; i++) {
                /* 
                 * No need to look at the optional flag because union members
                 * cannot be marked as optional 
                 */
                if (RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
                            tc->_members[i]._representation._typeCode,
                            v2Encapsulation,
                            &visitedNode)) {
                return RTI_XCDR_TRUE;
                }
            }
            /* Discriminator */
            return RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
                    tc->_typeCode,
                    v2Encapsulation,
                    &visitedNode);
        } break;
        default:
        {
            RTIXCdrAlignment alignment;
            RTIXCdrOctet primitiveSize;

            RTIXCdrTypeCode_getPrimitiveInfo(
                    kind,
                    v2Encapsulation,
                    &alignment,
                    &primitiveSize);
            
            if (alignment >= 4 && (primitiveSize % 4) == 0) {
                return RTI_XCDR_FALSE;
            } else {
                return RTI_XCDR_TRUE;
            }
        } break;
    }

    return RTI_XCDR_FALSE;
}

RTIXCdrBoolean RTIXCdrTypeCode_sampleMayRequirePadding(
        const struct RTIXCdrTypeCode *tc,
        RTIXCdrBoolean v2Encapsulation)
{
    RTIXCdrExtensibilityKind extKind;
    struct RTIXCdrTypeCodeNode visitedNode;

    extKind = RTIXCdrTypeCode_getExtensibilityKind(tc);

    if (!v2Encapsulation && extKind == RTI_XCDR_MUTABLE_EXTENSIBILITY) {
        /* 
         * V1 mutable types samples are never padded because they always end 
         * with the PID_SENTINEL, guaranteeing that the end of the serialized 
         * data is 4-byte aligned.
         */
        return RTI_XCDR_FALSE;
    }

    visitedNode.prev = NULL;
    visitedNode.tc = NULL;

    return RTIXCdrTypeCode_sampleMayRequirePaddingWithNode(
            tc,
            v2Encapsulation,
            &visitedNode);
}

RTIBool RTIXCdrTypeCode_isResourceVisited(
        const void *resource,
        const RTIXCdrResourceNode *parentVisitedNode,
        void **result)
{
    RTIXCdrLog_testPrecondition(resource == NULL, return RTI_XCDR_FALSE);

    while (parentVisitedNode != NULL) {
        if (parentVisitedNode->resource == resource) {
            if (result != NULL) {
                *result = parentVisitedNode->result;
            }
            return RTI_TRUE;
        }

        parentVisitedNode = parentVisitedNode->prev;
    }

    return RTI_FALSE;
}
