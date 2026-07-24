/*
 * FILE: InterpreterSupport.c
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "dds_c/dds_c_log.h"
#include "dds_c/dds_c_infrastructure.h"
#include "xcdr/xcdr_dds_interpreter.h"
#include "dds_c/dds_c_typecode.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"

RTI_PRIVATE struct DDS_StringSeq*
DDS_Sequence_create_or_trim_from_tc(
        struct DDS_StringSeq *self,
        RTIXCdrUnsignedLong maxElementCount,
        struct RTIXCdrTypeCode *elementTc,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrBoolean trimToSize)
{
    struct DDS_StringSeq *shortSeq = NULL;
    RTIXCdrUnsignedLong i = 0;
    RTIXCdrBoolean ok = RTI_XCDR_FALSE;
    RTIXCdrUnsignedLong typeSize;
    RTIXCdrUnsignedLong actualElementCount = 0;
    RTIXCdrTCKind kind;
    char * elPtr = NULL;
#ifndef RTI_CERT
    RTIXCdrBoolean isAllocated = RTI_XCDR_FALSE;
#endif /* RTI_CERT */
    kind = RTIXCdrTypeCode_getKind(elementTc);

    if (elementTc->_sampleAccessInfo != NULL)
    {
        typeSize = elementTc->_sampleAccessInfo->typeSize[0];
    }
    else
    {
        typeSize = DDS_TCKind_g_primitiveSizes[kind];
    }

    if (elementCount > INT_MAX)
    {
        goto done;
    }

    if (self == NULL)
    {
        OSAPI_Heap_allocate_struct(&shortSeq, struct DDS_StringSeq);

        if (shortSeq == NULL)
        {
            DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
            goto done;
        }
#ifndef RTI_CERT
        isAllocated = RTI_XCDR_TRUE;
#endif /* RTI_CERT */

        if (!DDS_StringSeq_initialize(shortSeq))
        {
            DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
            goto done;
        }
    }
    else
    {
        /* Just trimToSize */
        shortSeq = self;

        if (self->_maximum != 0)
        {
            if (elementTc->_typePlugin != NULL &&
                    elementTc->_typePlugin->finalizeSampleFnc != NULL)
            {
                elPtr = (char *)shortSeq->_contiguous_buffer;

                for (i=0; i < (RTI_UINT32)self->_maximum; i++)
                {
                    /* The TC _typePlugin does not currently have a method to finalize
                     * samples. We continue initializing the rest of the elements
                     */
                    if (!elementTc->_typePlugin->finalizeSampleFnc(elPtr))
                    {
                        DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                    }
                    elPtr += typeSize;
                }
            }
            else if (kind == RTI_XCDR_TK_STRING)
            {
                char **strPtr = (char **)shortSeq->_contiguous_buffer;

                for (i=0; i< (RTI_UINT32)self->_maximum; i++)
                {
                    if (*strPtr != NULL)
                    {
                        DDS_String_free(*strPtr);
                    }
                    strPtr += typeSize;
                }
            }
#ifndef RTI_CERT
            else if (kind == RTI_XCDR_TK_WSTRING)
            {
                DDS_Wchar **strPtr = (DDS_Wchar **)shortSeq->_contiguous_buffer;

                for (i=0; i< (RTI_UINT32)self->_maximum; i++)
                {
                    if (*strPtr != NULL)
                    {
                        OSAPI_Heap_free(*strPtr);
                    }
                    strPtr += typeSize;
                }
            }

            OSAPI_Heap_free(shortSeq->_contiguous_buffer);
#endif /* RTI_CERT */
            shortSeq->_maximum = 0;
            shortSeq->_length = 0;
            shortSeq->_contiguous_buffer = NULL;
        }
    }

    if (trimToSize)
    {
        actualElementCount = elementCount;
    }
    else
    {
        actualElementCount = maxElementCount;
    }

    if (actualElementCount != 0)
    {
        RTIXCdrUnsignedLongLong newBufferSize =
                                (RTIXCdrUnsignedLongLong) actualElementCount *
                                (RTIXCdrUnsignedLongLong) typeSize;

        if (newBufferSize > RTIXCdrLong_MAX)
        {
            goto done;
        }

        /* The buffer will be deallocated with RTIOsapiHeap_freeArray.
         * Therefore we need to provide RTI_OSAPI_ARRAY_ALLOC as the allocation
         * kind. Otherwise, heap monitoring will complain.
         *
         * RTI_SIZE_T is 32bit
         */
        shortSeq->_contiguous_buffer = OSAPI_Heap_allocate(1, (RTI_SIZE_T)newBufferSize);

        if (shortSeq->_contiguous_buffer == NULL) {
            DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
            goto done;
        }

        if (elementTc->_typePlugin != NULL &&
                    elementTc->_typePlugin->initializeSampleFnc != NULL)
        {
            elPtr = (char *)shortSeq->_contiguous_buffer;
            for (i=0; i<actualElementCount; i++)
            {
                if (!elementTc->_typePlugin->initializeSampleFnc(elPtr,
                                                                 RTI_TRUE,
                                                                 RTI_TRUE))
                {
                    DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                }
                elPtr += typeSize;
            }
        }
        else
        {
            OSAPI_Memory_zero(
                    shortSeq->_contiguous_buffer,
                    (RTI_SIZE_T)newBufferSize);
        }

        if (actualElementCount > INT_MAX)
        {
            goto done;
        }

        shortSeq->_maximum = (RTI_INT32)actualElementCount;
    }

    shortSeq->_length = 0;
    ok = RTI_XCDR_TRUE;

done:

    if (!ok && (shortSeq != NULL))
    {
#ifndef RTI_CERT
        if (isAllocated)
        {
            OSAPI_Heap_free_struct(shortSeq);
        }
#endif /* RTI_CERT */
        shortSeq = NULL;
    }

    return shortSeq;
}

RTI_PRIVATE RTIBool
DDS_Sequence_initialize_elements_from_tc(
        struct DDS_StringSeq *self,
        struct RTIXCdrTypeCode *elementTc,
        RTIXCdrUnsignedLong elementCount)
{

    RTIXCdrBoolean ok = RTI_XCDR_FALSE;

    if (elementCount != 0)
    {
        if (elementTc->_typePlugin != NULL &&
                    elementTc->_typePlugin->initializeSampleFnc != NULL)
        {
            RTIXCdrUnsignedLong i = 0;
            RTIXCdrUnsignedLong typeSize;
            char * elPtr = NULL;

            typeSize = elementTc->_sampleAccessInfo->typeSize[0];
            elPtr = (char *)self->_contiguous_buffer;

            for (i=0; i<elementCount; i++)
            {
                /* The memory for the elements should be allocated. This is
                 * why we pass RTI_FALSE to initialize func
                 */
                if (!elementTc->_typePlugin->initializeSampleFnc(
                        elPtr,
                        RTI_FALSE,
                        RTI_FALSE))
                {
                    DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                    goto done;
                }
                elPtr += typeSize;
            }
        }
        /* else { */
        /* We do not have to do anything because the element
         * (primitive, string, wstring) value will be overwritten
         */
        /* } */
    }

    ok = RTI_XCDR_TRUE;
  done:
    return ok;
}

RTIXCdrMemberValue
DDS_Sequence_get_member_value_pointer(
    void *sample,
    RTIXCdrUnsignedLong *elementCount,
    RTIXCdrUnsignedLongLong bindingMemberValueOffset,
    RTIXCdrUnsignedLong elementIndex,
    const struct RTIXCdrTypeCode *memberTc,
    const struct RTIXCdrTypeCodeMember *memberInfo,
    RTIXCdrBoolean allocateMemberIfNull,
    void *programData)
{
    RTIXCdrMemberValue val = RTI_XCDR_MEMBER_VALUE_NIL;
    struct DDS_StringSeq *shortSeq = NULL;
    UNUSED_ARG(allocateMemberIfNull);
    UNUSED_ARG(memberTc);
    UNUSED_ARG(elementIndex);
    UNUSED_ARG(programData);

    /* Sequences should not be allocated in this method call but
     * DDS_Sequence_set_member_element_count
     */
    if (memberInfo != NULL && RTIXCdrTypeCodeMember_isOptional(memberInfo))
    {
        struct DDS_StringSeq **shortSeqRef = NULL;

        shortSeqRef = OSAPI_Compiler_reinterpret_cast(
                                  struct DDS_StringSeq**,
                                  (char *)sample + bindingMemberValueOffset);

        shortSeq = *shortSeqRef;

        if (shortSeq == NULL)
        {
            val.isNull = RTI_XCDR_TRUE;
            return val;
        }
    }
    else
    {
        shortSeq = OSAPI_Compiler_reinterpret_cast(
                                  struct DDS_StringSeq*,
                                  (char *)sample + bindingMemberValueOffset);
    }

    if (elementCount != NULL)
    {
        /* Sequences cannot have a length < 0 */
        *elementCount = (RTIXCdrUnsignedLong)shortSeq->_length;
    }

    val.value.ptr = (char *)shortSeq->_contiguous_buffer;
    return val;
}

RTIXCdrMemberValue
DDS_Sequence_set_member_element_count(
        RTIXCdrBoolean *failure,
        void *sample,
        RTIXCdrUnsignedLong elementCount,
        RTIXCdrUnsignedLongLong bindingMemberValueOffset,
        const struct RTIXCdrTypeCode *memberTc,
        const struct RTIXCdrTypeCodeMember *memberInfo,
        RTIXCdrBoolean allocateMemberIfNull,
        RTIXCdrBoolean trimToSize,
        RTIXCdrBoolean initializeElement,
        void *programData)
{
    RTIXCdrMemberValue val = RTI_XCDR_MEMBER_VALUE_NIL;
    struct DDS_StringSeq *shortSeq = NULL;
    RTIBool newSeq = RTI_FALSE;
    UNUSED_ARG(programData);

    *failure = RTI_XCDR_TRUE;

    if (memberInfo != NULL && RTIXCdrTypeCodeMember_isOptional(memberInfo))
    {
        struct DDS_StringSeq **shortSeqRef = NULL;

        shortSeqRef = OSAPI_Compiler_reinterpret_cast(
                                  struct DDS_StringSeq**,
                                  (char *)sample + bindingMemberValueOffset);
        shortSeq = *shortSeqRef;

        if (shortSeq == NULL)
        {
            if (!allocateMemberIfNull)
            {
                val.isNull = RTI_XCDR_TRUE;
                *failure = RTI_XCDR_FALSE;
                return val;
            }

            *shortSeqRef = DDS_Sequence_create_or_trim_from_tc(
                    NULL,
                    memberTc->_maximumLength,
                    memberTc->_typeCode,
                    elementCount,
                    trimToSize);

            if (*shortSeqRef == NULL)
            {
                DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                return RTI_XCDR_MEMBER_VALUE_INVALID;
            }

            shortSeq = *shortSeqRef;
            newSeq = RTI_TRUE;
        }
    }
    else
    {
        shortSeq = OSAPI_Compiler_reinterpret_cast(
                                   struct DDS_StringSeq*,
                                   (char *)sample + bindingMemberValueOffset);
    }

      if (!newSeq)
      {
        if (elementCount != (RTI_UINT32)shortSeq->_maximum &&
                trimToSize)
        {
            shortSeq = DDS_Sequence_create_or_trim_from_tc(
                    shortSeq,
                    memberTc->_maximumLength,
                    memberTc->_typeCode,
                    elementCount,
                    trimToSize);

            if (shortSeq == NULL)
            {
                DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                return RTI_XCDR_MEMBER_VALUE_INVALID;
            }
        }
        else
        {
            if (initializeElement &&
                !DDS_Sequence_initialize_elements_from_tc(
                        shortSeq,memberTc->_typeCode,elementCount))
            {
                DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(OSAPI_LOGKIND_ERROR);
                return RTI_XCDR_MEMBER_VALUE_INVALID;
            }
        }
    }

    shortSeq->_length = (RTI_INT32)elementCount;
    *failure = RTI_XCDR_FALSE;
    val.isNull = RTI_XCDR_FALSE;
    val.value.ptr = (char *)shortSeq->_contiguous_buffer;

    return val;
}

RTIXCdrSampleAccessInfo DDS_g_sai_seq =
{
        RTI_XCDR_TYPE_BINDING_C,
        {sizeof(struct DDS_StringSeq),0,0,0},
        RTI_XCDR_FALSE,
        DDS_Sequence_get_member_value_pointer,
        DDS_Sequence_set_member_element_count,
        NULL,
        NULL,
        NULL
};

const struct DDS_TypeAllocationParams_t DDS_TYPE_ALLOCATION_PARAMS_DEFAULT =
{
    DDS_BOOLEAN_TRUE,
    DDS_BOOLEAN_FALSE,
    DDS_BOOLEAN_TRUE
};

const struct DDS_TypeDeallocationParams_t DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT =
{
    DDS_BOOLEAN_TRUE,
    DDS_BOOLEAN_TRUE
};
