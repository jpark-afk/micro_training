/*
(c) Copyright, Real-Time Innovations, 2018-2025.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_infrastructure_h
#define xcdr_infrastructure_h

#include "xcdr/xcdr_dll.h"

/******************************************************************************
 * Including infrastructure_psm must be the done before any other declaration.
 ******************************************************************************/
#include "xcdr/xcdr_infrastructure_psm.h"


#ifdef __cplusplus
    extern "C" {
#endif


/* ------------------------------------------------------------------------- */
/* ---- Constants and Types ------------------------------------------------ */
/* ------------------------------------------------------------------------- */
#define RTI_XCDR_ONE_BYTE_SIZE 1
#define RTI_XCDR_TWO_BYTE_SIZE 2
#define RTI_XCDR_FOUR_BYTE_SIZE 4
#define RTI_XCDR_EIGHT_BYTE_SIZE 8
#define RTI_XCDR_SIXTEEN_BYTE_SIZE 16

#define RTI_XCDR_CHAR_SIZE RTI_XCDR_ONE_BYTE_SIZE
#define RTI_XCDR_OCTET_SIZE RTI_XCDR_ONE_BYTE_SIZE
#define RTI_XCDR_LEGACY_WCHAR_SIZE RTI_XCDR_FOUR_BYTE_SIZE
#define RTI_XCDR_WCHAR_SIZE RTI_XCDR_TWO_BYTE_SIZE
#define RTI_XCDR_SHORT_SIZE RTI_XCDR_TWO_BYTE_SIZE
#define RTI_XCDR_UNSIGNED_SHORT_SIZE RTI_XCDR_TWO_BYTE_SIZE
#define RTI_XCDR_LONG_SIZE RTI_XCDR_FOUR_BYTE_SIZE
#define RTI_XCDR_UNSIGNED_LONG_SIZE RTI_XCDR_FOUR_BYTE_SIZE
#define RTI_XCDR_LONG_LONG_SIZE RTI_XCDR_EIGHT_BYTE_SIZE
#define RTI_XCDR_UNSIGNED_LONG_LONG_SIZE RTI_XCDR_EIGHT_BYTE_SIZE
#define RTI_XCDR_FLOAT_SIZE RTI_XCDR_FOUR_BYTE_SIZE
#define RTI_XCDR_DOUBLE_SIZE RTI_XCDR_EIGHT_BYTE_SIZE
#define RTI_XCDR_LONG_DOUBLE_SIZE RTI_XCDR_SIXTEEN_BYTE_SIZE
#define RTI_XCDR_BOOLEAN_SIZE RTI_XCDR_ONE_BYTE_SIZE
#define RTI_XCDR_ENUM_SIZE RTI_XCDR_FOUR_BYTE_SIZE

#define RTI_XCDR_TRUE ((RTIXCdrBoolean)1)
#define RTI_XCDR_FALSE ((RTIXCdrBoolean)0)
#define RTI_XCDR_MAYBE ((RTIXCdrBoolean)2)
#define RTIXCdrBoolean_DEFAULT (RTI_XCDR_FALSE)

#define RTIXCdrChar_MAX 127
#define RTIXCdrChar_MIN (-RTIXCdrChar_MAX-1)
#define RTIXCdrChar_DEFAULT 0

#define RTIXCdrWChar_DEFAULT 0

#define RTIXCdrOctet_MAX 255
#define RTIXCdrOctet_MIN 0
#define RTIXCdrOctet_DEFAULT (RTIXCdrOctet_MIN)

#define RTIXCdrInt8_MAX 127
#define RTIXCdrInt8_MIN (-RTIXCdrInt8_MAX-1)
#define RTIXCdrInt8_DEFAULT 0

#define RTIXCdrUInt8_MAX 255
#define RTIXCdrUInt8_MIN 0
#define RTIXCdrUInt8_DEFAULT (RTIXCdrUInt8_MIN)

#define RTIXCdrShort_MAX 32767
#define RTIXCdrShort_MIN (-RTIXCdrShort_MAX-1)
#define RTIXCdrShort_DEFAULT 0

#define RTIXCdrUnsignedShort_MAX 65535
#define RTIXCdrUnsignedShort_MIN 0
#define RTIXCdrUnsignedShort_DEFAULT (RTIXCdrUnsignedShort_MIN)

#define RTIXCdrWchar_MAX RTIXCdrUnsignedShort_MAX
#define RTIXCdrWchar_MIN RTIXCdrUnsignedShort_MIN
#define RTIXCdrWchar_DEFAULT (RTIXCdrUnsignedShort_DEFAULT)

/* This is not defined as -2147483648 because that value is processed in
   two stages. From https://msdn.microsoft.com/en-us/library/4kh09110.aspx
   1. The number 2147483648 is evaluated. Because it is greater than the
      maximum integer value of 2147483647, the type of 2147483648 is not int,
      but unsigned int
   2. Unary minus is applied to the value, with an unsigned result, which also
      happens to be 2147483648.
*/
#define RTIXCdrLong_MAX 2147483647
#define RTIXCdrLong_MIN (-RTIXCdrLong_MAX - 1)
#define RTIXCdrLong_DEFAULT 0

#define RTIXCdrUnsignedLong_MAX 4294967295U
#define RTIXCdrUnsignedLong_MIN 0
#define RTIXCdrUnsignedLong_DEFAULT (RTIXCdrUnsignedLong_MIN)

#define RTIXCdrLongLong_MAX 9223372036854775807LL
#define RTIXCdrLongLong_MIN (-RTIXCdrLongLong_MAX - 1LL)
#define RTIXCdrLongLong_DEFAULT 0

#define RTIXCdrUnsignedLongLong_MAX 18446744073709551615ULL
#define RTIXCdrUnsignedLongLong_MIN 0ULL
#define RTIXCdrUnsignedLongLong_DEFAULT (RTIXCdrUnsignedLongLong_MIN)

#define RTIXCdrFloat_MAX 3.40282346638528859811704183484516925e+38F
#define RTIXCdrFloat_MIN (-RTIXCdrFloat_MAX)
#define RTIXCdrFloat_DEFAULT 0.0f

#define RTIXCdrDouble_MAX ((double)1.79769313486231570814527423731704357e+308L)
#define RTIXCdrDouble_MIN (-RTIXCdrDouble_MAX)
#define RTIXCdrDouble_DEFAULT 0.0

typedef RTIXCdrLong RTIXCdrAlignment;
typedef RTIXCdrLong RTIXCdrCompressionId;

struct RTIXCdrInlineListNode {
    struct RTIXCdrInlineListNode *next;
    struct RTIXCdrInlineListNode *prev;
};

typedef enum {
    RTI_XCDR_CHAR_TYPE,
    RTI_XCDR_WCHAR_TYPE,
    RTI_XCDR_OCTET_TYPE,
    RTI_XCDR_SHORT_TYPE,
    RTI_XCDR_UNSIGNED_SHORT_TYPE,
    RTI_XCDR_LONG_TYPE,
    RTI_XCDR_UNSIGNED_LONG_TYPE,
    RTI_XCDR_LONG_LONG_TYPE,
    RTI_XCDR_UNSIGNED_LONG_LONG_TYPE,
    RTI_XCDR_FLOAT_TYPE,
    RTI_XCDR_DOUBLE_TYPE,
    RTI_XCDR_LONG_DOUBLE_TYPE,
    RTI_XCDR_BOOLEAN_TYPE,
    RTI_XCDR_ENUM_TYPE
} RTIXCdrPrimitiveType;

/* ------------------------------------------------------------------------- */
/* ---- Function declarations ---------------------------------------------- */
/* ------------------------------------------------------------------------- */

extern RTIXCdrDllExport
void * RTIXCdrHeap_allocate(RTIXCdrUnsignedLong size);

extern RTIXCdrDllExport
void RTIXCdrHeap_free(void *ptr);

#define TYPE RTIXCdrUnsignedLong

extern RTIXCdrDllExport
void RTIXCdrHeap_allocateStruct(
        TYPE **structStoragePointer,
        TYPE);

extern RTIXCdrDllExport
void RTIXCdrHeap_freeStruct(
        TYPE *structStorage);

#undef TYPE

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrWString_getLength(const RTIXCdrWchar * str);

/*
 * @brief Returns the length of a wstring.
 *
 * @param In. The string.
 * @param In. The maximum length allowed.
 *
 * @return This function returns the length of the string in characters, if 
 * that is less than maxLength, or maxLength if there is no null terminating 
 * among the first maxLength characters pointed to by str.
 */
extern RTIXCdrDllExport 
RTIXCdrUnsignedLong RTIXCdrWString_getLengthWithMax(
        const RTIXCdrWchar *str,
        RTIXCdrUnsignedLong maxLength);

/*
 * @brief Returns the length of a string.
 *
 * @param In. The string.
 * @param In. The maximum length allowed.
 *
 * @return This function returns the length of the string in characters, if 
 * that is less than maxLength, or maxLength if there is no null terminating 
 * among the first maxLength characters pointed to by str.
 */
extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrString_getLengthWithMax(
        const RTIXCdrChar *str,
        RTIXCdrUnsignedLong maxLength);

extern RTIXCdrDllExport void RTIXCdrMemory_copy(
        void *dest,
        const void *src,
        RTIXCdrUnsignedLong size);

extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrAlignment_alignSizeUp(
        RTIXCdrUnsignedLong size,
        RTIXCdrAlignment alignment);

/**
 * @brief Converts a pointer to an unsigned long integer.
 *
 * As a general rule, we shouldn't convert a pointer (which could be an
 * unsigned long long) to an unsigned long.
 * However, this function is used for the bindingMemberValueOffset member in
 * RTIXCdrMemberAccessInfo. This member describes an offset from the beginning
 * of the sample, and our implementation restricts sample size to 2^31 bytes.
 *
 * @param[in] ptr__. The input pointer to convert.
 *
 * @return The converted unsigned long integer.
 */
extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrUtility_pointerToUnsignedLong(const void *pointer);

/**
 * @brief Converts a pointer to an unsigned long long integer.
 *
 * @param[in] ptr__. The input pointer to convert.
 *
 * @return The converted unsigned long long integer.
 */
extern RTIXCdrDllExport RTIXCdrUnsignedLongLong
RTIXCdrUtility_pointerToUnsignedLongLong(const void *pointer);

/*
 * @brief Copy an XCDR char.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyChar(RTIXCdrChar *out, const RTIXCdrChar *in);

/*
 * @brief Copy an XCDR wchar.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyWchar(RTIXCdrWchar *out, const RTIXCdrWchar *in);

/*
 * @brief Copy an XCDR short.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyEnum(RTIXCdrEnum *out, const RTIXCdrEnum *in);

/*
 * @brief Copy an XCDR octet.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyOctet(RTIXCdrOctet *out, const RTIXCdrOctet *in);

/*
 * @brief Copy an XCDR int8.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyInt8(RTIXCdrInt8 *out, const RTIXCdrInt8 *in);

/*
 * @brief Copy an XCDR uint8.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyUInt8(RTIXCdrUInt8 *out, const RTIXCdrUInt8 *in);

/*
 * @brief Copy an XCDR short.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyShort(RTIXCdrShort *out, const RTIXCdrShort *in);

/*
 * @brief Copy an XCDR unsigned short.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyUnsignedShort(
        RTIXCdrUnsignedShort *out,
        const RTIXCdrUnsignedShort *in);

/*
 * @brief Copy an XCDR long.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyLong(RTIXCdrLong *out, const RTIXCdrLong *in);

/*
 * @brief Copy an XCDR unsigned long.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyUnsignedLong(
        RTIXCdrUnsignedLong *out,
        const RTIXCdrUnsignedLong *in);

/*
 * @brief Copy an XCDR long long.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyLongLong(
        RTIXCdrLongLong *out,
        const RTIXCdrLongLong *in);

/*
 * @brief Copy an XCDR long long.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyUnsignedLongLong(
        RTIXCdrUnsignedLongLong *out,
        const RTIXCdrUnsignedLongLong *in);

/*
 * @brief Copy an XCDR float.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyFloat(RTIXCdrFloat *out, const RTIXCdrFloat *in);

/*
 * @brief Copy an XCDR double.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyDouble(
        RTIXCdrDouble *out,
        const RTIXCdrDouble *in);

/*
 * @brief Copy an XCDR long double.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyLongDouble(
        RTIXCdrLongDouble *out,
        const RTIXCdrLongDouble *in);

/*
 * @brief Copy an XCDR boolean.
 *
 * @param Out. Destination.
 * @param In. Source.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyBoolean(
        RTIXCdrBoolean *out,
        const RTIXCdrBoolean *in);

/*
 * @brief Copy an XCDR boolean.
 *
 * @param Out. Destination.
 * @param In. Source.
 * @param length In. Number of elements in array.
 * @param elementSize In. Size of a single element.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyArray(
        void *out,
        const void *in,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong elementSize);

/*
 * @brief Copy an XCDR string.
 *
 * @param Out. Destination.
 * @param In. Source.
 * @param maximumLength In. Max number of characters to copy.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyString(
        RTIXCdrChar *out,
        const RTIXCdrChar *in,
        RTIXCdrUnsignedLong maximumLength);

/*
 * @brief Copy an XCDR wstring.
 *
 * @param out Out. Destination.
 * @param in In. Source.
 * @param maximumLength In. Max number of characters to copy.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyWstring(
        RTIXCdrWchar *out,
        const RTIXCdrWchar *in,
        RTIXCdrUnsignedLong maximumLength);

/*
 * @brief Copy an XCDR string (extended).
 *
 * @param str In. String whose length is calculated.
 *
 * @return Length of string.
 */
extern RTIXCdrDllExport
RTIXCdrUnsignedLong RTIXCdrType_getWstringLength(const RTIXCdrWchar *str);

/*
 * @brief Copy an XCDR string (extended).
 *
 * @param out Out. Destination.
 * @param in In. Source.
 * @param maximumLength In. Max number of characters to copy.
 * @param reallocate In. Reallocate the destination?
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyStringEx(
        RTIXCdrChar **out,
        const RTIXCdrChar *in,
        RTIXCdrUnsignedLong maximumLength,
        RTIXCdrBoolean reallocate);

/*
 * @brief Copy an XCDR wstring (extended).
 *
 * @param out Out. Destination.
 * @param in In. Source.
 * @param maximumLength In. Max number of characters to copy.
 * @param reallocate In. Reallocate the destination?
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_copyWstringEx(
        RTIXCdrWchar **out,
        const RTIXCdrWchar *in,
        RTIXCdrUnsignedLong maximumLength,
        RTIXCdrBoolean reallocate);

/*
 * @brief Initialize an array.
 *
 * @param value Out. Array to initialize.
 * @param length In. Number of elements in array.
 * @param elementSize In. Size of a single element.
 *
 * @return Success of operation.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_initArray(
        void *value,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong elementSize);

/**
 * @brief Initializes a long value.
 *
 * This function initializes a given long value to its default state.
 *
 * @param[out] value Pointer to the long value to be initialized.
 *
 * @return RTI_XCDR_TRUE if the initialization was successful, RTI_XCDR_FALSE
 * otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_initLong(RTIXCdrLong *value);

/**
 * @brief Initializes an unsigned short value.
 *
 * This function initializes a given unsigned short value to its default state.
 *
 * @param[out] value Pointer to the unsigned short value to be initialized.
 *
 * @return RTI_XCDR_TRUE if the initialization was successful, RTI_XCDR_FALSE
 * otherwise.
 */
extern RTIXCdrDllExport
RTIXCdrBoolean RTIXCdrType_initUnsignedShort(RTIXCdrUnsignedShort *value);

/**
 * @brief Initializes an array of strings.
 *
 * This function initializes an array of strings with the specified length and
 * maximum string size.
 *
 * @param[out] value Pointer to the array of strings to be initialized.
 * @param[in] length The number of elements in the array.
 * @param[in] maximumStringSize The size of each string in the array, including
 * space for the terminating NUL (i.e., maximum string length + 1).
 * @param[in] type The type of the strings in the array, must be either
 * RTI_XCDR_CHAR_TYPE or RTI_XCDR_WCHAR_TYPE
 *
 * @return RTI_XCDR_TRUE if the initialization was successful, RTI_XCDR_FALSE
 * otherwise.
 *
 * @pre \p value must not be NULL.
 * @pre \p arraySize must be greater than 0.
 * @pre \p type must be either RTI_CDR_CHAR_TYPE or RTI_CDR_WCHAR_TYPE.
 */
extern RTIXCdrDllExport RTIXCdrBoolean RTIXCdrType_initStringArray(
        void *value,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong maximumStringSize,
        RTIXCdrPrimitiveType type);

/*
 * @brief Finalizes an array of strings.
 *
 * This function finalizes an array of strings, releasing any resources
 * associated with the strings in the array.
 *
 * @param[out] value Pointer to the array of strings to be finalized.
 * @param[in] length The number of elements in the array.
 * @param[in] type The type of the strings in the array, must be either
 * RTI_XCDR_CHAR_TYPE orRTI_XCDR_WCHAR_TYPE
 *
 * @return RTI_XCDR_TRUE if the finalization was successful, RTI_XCDR_FALSE
 * otherwise.
 */
extern RTIXCdrDllExport RTIXCdrBoolean RTIXCdrType_finalizeStringArray(
        void *value,
        RTIXCdrUnsignedLong length,
        RTIXCdrPrimitiveType type);

/*
 * @brief Copies an array of strings with optional reallocation.
 *
 * This function copies an array of strings from the source to the destination.
 * It can optionally reallocate the destination strings if necessary.
 *
 * @param[out] out Pointer to the destination array of strings.
 * @param[in] in Pointer to the source array of strings.
 * @param[in] length The number of elements in the array.
 * @param[in] maximumStringLength The maximum length of each string in the
 * array.
 * @param[in] type The type of the strings in the array, must be either
 * RTI_XCDR_CHAR_TYPE or RTI_XCDR_WCHAR_TYPE.
 * @param[in] reallocate Boolean flag indicating whether to reallocate the
 * destination strings if necessary.
 *
 * @return RTI_XCDR_TRUE if the copy operation was successful, RTI_XCDR_FALSE
 * otherwise.
 */
extern RTIXCdrDllExport RTIXCdrBoolean RTIXCdrType_copyStringArrayEx(
        void *out,
        const void *in,
        RTIXCdrUnsignedLong length,
        RTIXCdrUnsignedLong maximumStringLength,
        RTIXCdrPrimitiveType type,
        RTIXCdrBoolean reallocate);

/*
 * @brief Allocate a string.
 *
 * @param length In. Length of string.
 *
 * @returns The allocated string, or NULL on error.
 */
extern RTIXCdrDllExport char *RTIXCdrString_alloc(RTIXCdrUnsignedLong length);

/*
 * @brief Free a string.
 *
 * @param string In. String to be freed.
 */
extern RTIXCdrDllExport
void RTIXCdrString_free(char *string);

/*
 * @brief return length of string.
 * @param s1 In. String to return length for.
 *
 * @return The length of the string.
 */
extern RTIXCdrDllExport RTIXCdrUnsignedLong
RTIXCdrString_getLength(const char *s1);

/*
 * @brief Replace a string. The string pointed to by *string_ptr is freed
 *        and replaced with new_value.
 *
 * @param string_ptr InOut. Pointer to string to be replaced.
 * @param newValue In. String value to copy into the *string_ptr.
 *
 * @returns The replaced string, or NULL on error.
 */
extern RTIXCdrDllExport
char * RTIXCdrString_replace(char **stringPtr, const char *newValue);

/*
 * @brief Allocate a WString.
 *
 * @param length In. Length of Wstring.
 *
 * @returns The allocated WString, or NULL on error.
 */
extern RTIXCdrDllExport RTIXCdrWchar *RTIXCdrWstring_alloc(
        RTIXCdrUnsignedLong length);

/*
 * @brief Free a wstring.
 *
 * @param string In. WString to be freed.
 */
extern RTIXCdrDllExport
void RTIXCdrWString_free(RTIXCdrWchar *string);

/*
 * @brief Replaces the content of a wide string with a new value.
 *
 * @detail This function replaces the content of the wide string pointed to by
 * \p stringPtr with the new value \p newValue. If \p newValue is NULL,
 * the function frees the memory allocated for the original string.
 * If \p newValue is not NULL, the function reallocates memory for
 * \p stringPtr if necessary and copies \p newValue into it.
 *
 * @pre \p stringPtr is non-NULL.
 *
 * @param[in,out] stringPtr Pointer to the wide string to be replaced.
 * @param[in] newValue The new value to copy into the *stringPtr.
 *
 * @return Pointer to the replaced wide string, or NULL on error.
 */
extern RTIXCdrDllExport
RTIXCdrWchar * RTIXCdrWString_replace(
        RTIXCdrWchar **stringPtr,
        const RTIXCdrWchar *newValue);

struct RTIXCdrPluggableMemoryAllocator;

/**
 * Creates a RTIXCdrPluggableMemoryAllocator that uses an unbounded buffer pool
 * as the underlying allocator.
 *
 * @param[in] numberOfBuffers The maximum number of the buffers that can be
 * created by this pool
 * @param[in] initialRequestedBufferSize If greater than zero, all
 * numberOfBuffers are allocated to this size.
 *
 * @returns the RTIXCdrPluggableMemoryAllocator, or NULL upon failure.
 */
extern RTIXCdrDllExport
struct RTIXCdrPluggableMemoryAllocator *
RTIXCdrUnboundedBufferPool_newPluggableMemoryAllocator(
        RTI_INT32 numberOfBuffers,
        RTI_INT32 initialRequestedBufferSize);

/**
 * Delete this RTIXCdrPluggableMemoryAllocator.
 *
 * @param[in] allocator The Allocator to delete.
 */
extern RTIXCdrDllExport
void RTIXCdrPluggableMemoryAllocator_delete(
        struct RTIXCdrPluggableMemoryAllocator *allocator);

/* ------------------------------------------------------------------------- */
/* ---- Memory ------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

extern RTIXCdrDllExport void RTIXCdrMemory_zero(
        void *ptr,
        RTIXCdrUnsignedLong size);


extern RTIXCdrDllExport
int RTIXCdrMemory_compare(
        const void *left,
        const void *right,
        RTIXCdrUnsignedLong size);

#ifdef __cplusplus
}
#endif

/**
 * This structure is also defined in
 * dds_c.1.0/interface/common.ifc. If changes are made,
 * please ensure they are made in both locations.
 */
struct RTIXCdrTypeAllocationParams {
    RTIXCdrBoolean allocate_pointers;
    RTIXCdrBoolean allocate_optional_members;
    RTIXCdrBoolean allocate_memory;
};
extern const struct RTIXCdrTypeAllocationParams
        RTI_XCDR_TYPE_ALLOCATION_PARAMS_DEFAULT;
struct RTIXCdrTypeDeallocationParams {
    RTIXCdrBoolean delete_pointers;
    RTIXCdrBoolean delete_optional_members;
};
extern const struct RTIXCdrTypeDeallocationParams
        RTI_XCDR_TYPE_DEALLOCATION_PARAMS_DEFAULT;

/* ------------------------------------------------------------------------- */
/* ---- Utilities ---------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/**
 * @brief Converts a network byte order unsigned short integer to host byte
 * order.
 *
 * @param value. In. Network byte order unsigned short integer.
 *
 * @return Host byte order unsigned short integer.
 */
RTIXCdrUnsignedShort RTIXCdrUtility_ntohs(RTIXCdrUnsignedShort s);


/******************************************************************************
 * Including infrastructure_impl must be the last thing we do in this file.
 ******************************************************************************/
#include "xcdr/xcdr_infrastructure_impl.h"

#endif /* xcdr_infrastructure_h */
