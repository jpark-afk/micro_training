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


#define RTIXCdrTypeCode_getKind(tc) \
    (RTIXCdrTCKind)((tc)->_kind & ~RTI_XCDR_TK_FLAGS_ALL)


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


#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* xcdr_typeCode_impl_h */
