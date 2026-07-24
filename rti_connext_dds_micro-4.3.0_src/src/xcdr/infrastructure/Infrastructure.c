/*
(c) Copyright, Real-Time Innovations, 2014-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_infrastructure.h"
#include "Infrastructure.h"

char *RTIXCdrString_dup(const char *string)
{
    char *clone = NULL;

    if (string != NULL) {
        RTIXCdrUnsignedLong length = RTIXCdrString_getLength(string);

        RTIXCdrHeap_allocateString(&clone, length);

        if (clone == NULL) {
            goto done;
        }

        RTIXCdrMemory_copy(clone, string, length + 1);
    }

done:
    return clone;
}

RTIXCdrUnsignedLong RTIXCdrWString_getLength(const RTIXCdrWchar * str)
{
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrLog_testPrecondition(str == NULL, return 0);
    while ((*str++) != '\0') i++;
    return i;
}

RTIXCdrUnsignedLong RTIXCdrWString_getLengthWithMax(
        const RTIXCdrWchar *str,
        RTIXCdrUnsignedLong maxLength)
{
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrLog_testPrecondition(str == NULL, return 0);
    while ((*str++) != 0 && i < maxLength) i++;
    return i;
}

RTIXCdrLong RTIXCdrWString_cmp(
        const RTIXCdrWchar *str1,
        const RTIXCdrWchar *str2)
{
    RTIXCdrUnsignedLong i;
    if (RTIXCdrWString_getLength(str1) !=
            RTIXCdrWString_getLength(str2)) {
        return 1;
    }
    for (i = 0; i < RTIXCdrWString_getLength(str1); ++i) {
        if (str1[i] != str2[i]) {
            return 1;
        }
    }
    return 0;
}

RTIXCdrWchar *RTIXCdr_allocateWString(RTIXCdrUnsignedLong length) {
    RTIXCdrWchar *string = NULL;

    if (length > (RTIXCdrLong_MAX-1)) {
        return NULL;
    }

    RTIXCdrHeap_allocateArray(&string, length + 1, RTIXCdrWchar);
    return string;
}

RTIXCdrBoolean RTIXCdrWString_copy(RTIXCdrWchar* dst, const RTIXCdrWchar* src) {
    RTIXCdrUnsignedLong length = 0;

    if (dst == NULL) {
        return RTI_XCDR_FALSE;
    }
    if (src == NULL) {
        return RTI_XCDR_FALSE;
    }

    length = RTIXCdrWString_getLength(src);
    RTIXCdrMemory_copy(
            dst,
            src,
            ((length + 1u) * (RTIXCdrUnsignedLong) sizeof(RTIXCdrWchar)));
    return RTI_XCDR_TRUE;
}

RTIXCdrWchar *RTIXCdrWString_dup(const RTIXCdrWchar* string) {
    RTIXCdrWchar *clone = NULL;
    RTIXCdrUnsignedLong length = 0;

    if (string == NULL) {
        goto done;
    }

    length = (RTIXCdrUnsignedLong) RTIXCdrWString_getLength(string);
    clone = RTIXCdr_allocateWString(length);
    if (clone == NULL) {
        goto done;
    }
    RTIXCdrWString_copy(clone, string);

  done:
    return clone;
}

RTIXCdrUnsignedLong RTIXCdrType_getWstringLength(const RTIXCdrWchar *str)
{
    unsigned int i = 0;
    RTIXCdrLog_testPrecondition(str == NULL, return 0);
    while ((*str++) != '\0')
        i++;

    return i;
}

RTIXCdrBoolean RTIXCdrType_copyWstringEx(
        RTIXCdrWchar **out,
        const RTIXCdrWchar *in,
        RTIXCdrUnsignedLong maximumLength,
        RTIXCdrBoolean reallocate)
{
    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);

    if (reallocate) {
        if (*out != NULL) {
            RTIXCdrHeap_freeArray(*out);
            *out = NULL;
        }

        RTIXCdrHeap_allocateArray(
                out,
                (RTIXCdrType_getWstringLength(in) + 1),
                RTIXCdrWchar);

        if (*out == NULL) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTIXCdrType_copyWstring(*out, in, maximumLength);
}

RTIXCdrBoolean RTIXCdrType_copyStringEx(
        RTIXCdrChar **out,
        const RTIXCdrChar *in,
        RTIXCdrUnsignedLong maximumLength,
        RTIXCdrBoolean reallocate)
{
    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);

    if (reallocate) {
        RTIXCdrHeap_reallocateString(out, RTIXCdrString_getLength(in));

        if (*out == NULL) {
            return RTI_XCDR_FALSE;
        }
    }

    return RTIXCdrType_copyString(*out, in, maximumLength);
}

char *RTIXCdrString_replace(char **stringPtr, const char *newValue)
{
    /* --- check parameters --- */
    if (stringPtr == NULL) {
        return NULL;
    }

    if (newValue == NULL) {
        RTIXCdrHeap_freeString(*stringPtr);
        *stringPtr = NULL;
        return NULL;
    }

    if (!RTIXCdrHeap_reallocateString(
                stringPtr,
                RTIXCdrString_getLength(newValue))) {
        return NULL;
    }

    RTIXCdrMemory_copy(
            *stringPtr,
            newValue,
            RTIXCdrString_getLength(newValue) + 1);

    return *stringPtr;
}

char *RTIXCdrString_alloc(RTIXCdrUnsignedLong length)
{
    char *string = NULL;

    RTIXCdrHeap_allocateString(&string, length);

    return string;
}

void RTIXCdrString_free(char *string)
{
    RTIXCdrHeap_freeString(string);
}

RTIXCdrWchar *RTIXCdrWstring_alloc(RTIXCdrUnsignedLong length)
{
    RTIXCdrWchar *string = NULL;

    if (length > (RTIXCdrLong_MAX - 1)) {
        return NULL;
    }

    RTIXCdrHeap_allocateArray(&string, length + 1, RTIXCdrWchar);

    return string;
}

void RTIXCdrWString_free(RTIXCdrWchar *string)
{
    RTIXCdrHeap_freeWString(string);
}

RTIXCdrWchar *RTIXCdrWString_replace(
        RTIXCdrWchar **stringPtr,
        const RTIXCdrWchar *newValue)
{
    RTIXCdrWchar *result = NULL;
    RTIXCdrUnsignedLong length = 0;

    if (stringPtr == NULL) {
        return NULL;
    }

    /* NULL string */
    if (newValue == NULL) {
        RTIXCdrWString_free(*stringPtr);
        *stringPtr = result = NULL;
        /* Replace old value */
    } else {
        length = RTIXCdrWString_getLength(newValue) + 1;
        if (RTIXCdrHeap_reallocateArray(stringPtr, length, RTIXCdrWchar)) {
            result = *stringPtr;
            RTIXCdrWString_copy(*stringPtr, newValue);
        }
    }

    return result;
}

RTIXCdrBoolean RTIXCdrType_initStringArray(
        void *value,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong maximumStringSize,
        RTIXCdrPrimitiveType type)
{
    unsigned int i;
    unsigned int size;
    RTIXCdrChar **valueChar = NULL;
    RTIXCdrWchar **valueWchar = NULL;

    RTIXCdrLog_testPrecondition(value == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(maximumStringSize < 1, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            type != RTI_XCDR_CHAR_TYPE && type != RTI_XCDR_WCHAR_TYPE,
            return RTI_XCDR_FALSE);

    if (type == RTI_XCDR_WCHAR_TYPE) {
        valueWchar = (RTIXCdrWchar **) value;
        size = sizeof(RTIXCdrWchar);
    } else {
        valueChar = (RTIXCdrChar **) value;
        size = sizeof(RTIXCdrChar);
    }

    RTIXCdrMemory_zero(value, size * length);

    for (i = 0; i < length; i++) {
        if (type == RTI_XCDR_WCHAR_TYPE) {
            valueWchar[i] = RTIXCdrWstring_alloc(maximumStringSize - 1);
            if (valueWchar[i] == NULL) {
                RTIXCdrType_finalizeStringArray(value, length, type);
                return RTI_XCDR_FALSE;
            }
            if (!RTIXCdrType_initWstring(
                        valueWchar[i],
                        maximumStringSize)) {
                RTIXCdrType_finalizeStringArray(value, length, type);
                return RTI_XCDR_FALSE;
            }
        } else {
            valueChar[i] = NULL;
            RTIXCdrHeap_allocateString(&valueChar[i], maximumStringSize - 1);
            if (valueChar[i] == NULL) {
                RTIXCdrType_finalizeStringArray(value, length, type);
                return RTI_XCDR_FALSE;
            }
            if (!RTIXCdrType_initString(
                        valueChar[i],
                        maximumStringSize)) {
                RTIXCdrType_finalizeStringArray(value, length, type);
                return RTI_XCDR_FALSE;
            }
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrType_finalizeStringArray(
        void *value,
        RTIXCdrUnsignedLong length,
        RTIXCdrPrimitiveType type)
{
    unsigned int i;

    RTIXCdrLog_testPrecondition(
            type != RTI_XCDR_CHAR_TYPE && type != RTI_XCDR_WCHAR_TYPE,
            return RTI_XCDR_FALSE);

    if (value == NULL) {
        return RTI_XCDR_TRUE;
    }

    if (type == RTI_XCDR_WCHAR_TYPE) {
        RTIXCdrWchar **valueWchar = (RTIXCdrWchar **) value;
        for (i = 0; i < length; ++i) {
            if (valueWchar[i] != NULL) {
                RTIXCdrHeap_freeArray(valueWchar[i]);
                valueWchar[i] = NULL;
            }
        }
    } else {
        RTIXCdrChar **valueChar = (RTIXCdrChar **) value;
        for (i = 0; i < length; ++i) {
            if (valueChar[i] != NULL) {
                RTIXCdrHeap_freeString(valueChar[i]);
                valueChar[i] = NULL;
            }
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrBoolean RTIXCdrType_copyStringArrayEx(
        void *out,
        const void *in,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong maximumStringLength,
        RTIXCdrPrimitiveType type,
        RTIXCdrBoolean reallocate)
{
    RTIXCdrUnsignedLong i;

    RTIXCdrLog_testPrecondition(out == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(in == NULL, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(maximumStringLength < 1, return RTI_XCDR_FALSE);
    RTIXCdrLog_testPrecondition(
            type != RTI_XCDR_CHAR_TYPE && type != RTI_XCDR_WCHAR_TYPE,
            return RTI_XCDR_FALSE);

    if (type == RTI_XCDR_WCHAR_TYPE) {
        for (i = 0; i < length; i++) {
            if (!RTIXCdrType_copyWstringEx(
                        &((RTIXCdrWchar **) out)[i],
                        ((RTIXCdrWchar **) in)[i],
                        maximumStringLength,
                        reallocate)) {
                return RTI_XCDR_FALSE;
            }
        }
    } else {
        for (i = 0; i < length; i++) {
            if (!RTIXCdrType_copyStringEx(
                        &((RTIXCdrChar **) out)[i],
                        ((RTIXCdrChar **) in)[i],
                        maximumStringLength,
                        reallocate)) {
                return RTI_XCDR_FALSE;
            }
        }
    }

    return RTI_XCDR_TRUE;
}

RTIXCdrUnsignedLong RTIXCdrUtility_pointerToUnsignedLong(const void *pointer)
{
    RTIXCdrUnsignedLongLong uint64 =
            RTIXCdrUtility_pointerToUnsignedLongLong(pointer);

    if (uint64 > RTIXCdrUnsignedLong_MAX) {
        RTIXCdrLog_logStr(
                RTI_XCDR_LOG_EXCEPTION,
                RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
                "pointer doesn't fit in unsigned long");
        return RTIXCdrUnsignedLong_MAX;
    }
    return (RTIXCdrUnsignedLong) uint64;
}

#ifdef RTI_PRECONDITION_TEST
void RTIXCdrLog_preconditionBreakPoint(void) {
    return;
}
#endif
