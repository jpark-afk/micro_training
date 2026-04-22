/*
(c) Copyright, Real-Time Innovations, 2018-2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_typeCode_impl_h
#define xcdr_typeCode_impl_h


#ifdef __cplusplus
    extern "C" {
#endif


#define RTIXCdrTypeCodeMember_isOptional(member) \
    ( \
        (((member)->_memberFlags & (RTI_XCDR_REQUIRED_MEMBER | RTI_XCDR_KEY_MEMBER)) || \
         ((member)->_labelsCount > 0) \
        ) \
    ? RTI_XCDR_FALSE : RTI_XCDR_TRUE)


#define RTIXCdrTypeCode_getKind(tc__) \
    (RTIXCdrTCKind)((tc__)->_kind & ~RTI_XCDR_TK_FLAGS_ALL)


#define RTIXCdrTypeCode_isValidEnumValue(tc, isValid, value) \
{ \
    RTIXCdrUnsignedLong __mIndex = 0; \
    \
    *(isValid) = RTI_XCDR_FALSE; \
    for (; __mIndex<(tc)->_memberCount; __mIndex++) { \
        if ((tc)->_members[__mIndex]._ordinal == (value)) { \
            *(isValid) = RTI_XCDR_TRUE; \
            break; \
        } \
    } \
}

#define RTIXCdrTypeCode_hasDefaultLabel(tc) \
    (((tc)->_default_index != -1)?RTI_XCDR_TRUE:RTI_XCDR_FALSE)
    
#define RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding(languageBinding) \
    (((languageBinding) & RTI_XCDR_TYPE_BINDING_FLAT_DATA_MASK)? \
            RTI_XCDR_TRUE: \
            RTI_XCDR_FALSE)

#define RTIXCdrSampleAccessInfo_isContiguousMemoryBinding(languageBinding) \
    ((languageBinding) == RTI_XCDR_TYPE_BINDING_DYN_DATA                   \
     || (languageBinding) == RTI_XCDR_TYPE_BINDING_SQL_FILTER              \
     || RTIXCdrSampleAccessInfo_isFlatDataLanguageBinding((languageBinding)))

/*
 * Check if the given type code kind is a struct or value type
 */
#define RTIXCdrTypeCode_isStructOrValue(kind__) \
    ((kind__) == RTI_XCDR_TK_STRUCT || (kind__) == RTI_XCDR_TK_VALUE)

/*
 * Check if the given type code kind is an 8-bit integer type
 */
#define RTIXCdrTypeCode_is8BitInteger(kind__) \
    ((kind__) == RTI_XCDR_TK_OCTET || (kind__) == RTI_XCDR_TK_INT8 \
     || (kind__) == RTI_XCDR_TK_UINT8)

/*
 * Check if two type code kinds are considered equivalent for type comparison
 * purposes. This is used to determine if two different kinds should be treated
 * as compatible during type equality checks.
 */
#define RTIXCdrTypeCode_areKindsEquivalent(kind1__, kind2__) \
    (((RTIXCdrTypeCode_isStructOrValue(kind1__) \
       && RTIXCdrTypeCode_isStructOrValue(kind2__))) \
     || ((RTIXCdrTypeCode_is8BitInteger(kind1__) \
          && RTIXCdrTypeCode_is8BitInteger(kind2__))))

#define RTIXCdrTypeCode_isPrimitiveKind(kind__, enumAsPrimitive__) \
    ((kind__) == RTI_XCDR_TK_SHORT || (kind__) == RTI_XCDR_TK_LONG \
     || (kind__) == RTI_XCDR_TK_USHORT || (kind__) == RTI_XCDR_TK_ULONG \
     || (kind__) == RTI_XCDR_TK_FLOAT || (kind__) == RTI_XCDR_TK_DOUBLE \
     || (kind__) == RTI_XCDR_TK_BOOLEAN || (kind__) == RTI_XCDR_TK_CHAR \
     || (kind__) == RTI_XCDR_TK_OCTET || (kind__) == RTI_XCDR_TK_INT8 \
     || (kind__) == RTI_XCDR_TK_UINT8 \
     || ((enumAsPrimitive__) && (kind__) == RTI_XCDR_TK_ENUM) \
     || (kind__) == RTI_XCDR_TK_LONGLONG || (kind__) == RTI_XCDR_TK_ULONGLONG \
     || (kind__) == RTI_XCDR_TK_LONGDOUBLE || (kind__) == RTI_XCDR_TK_WCHAR)

#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_typeCode_impl_h */
