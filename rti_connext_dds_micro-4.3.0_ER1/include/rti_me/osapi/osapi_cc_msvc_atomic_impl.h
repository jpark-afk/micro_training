/*
 * FILE: osapi_cc_msvc_atomic_impl.h - MSVC atomic operation helpers
 *
 * Copyright 2026-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*
 * NOTE on MSVC vs. POSIX (GCC/Clang) Implementation:
 *
 * In the POSIX/Clang implementations, we use __atomic built-ins which allow 
 * the compiler to optimize hardware instructions based on the memory_order 
 * (e.g., using specialized Acquire/Release instructions on ARM).
 *
 * MSVC's legacy <intrin.h> Interlocked API does not support the new memory 
 * models. Interlocked intrinsics always emit a 'LOCK' prefix, 
 * which acts as a full Sequentially Consistent barrier. 
 *
 * Consequently:
 * 1. Relaxed operations: We use volatile access + compiler barriers where 
 * possible, but RMW operations (Add/Sub) still incur a full hardware 
 * fence because MSVC lacks a "relaxed" InterlockedAdd.
 * 2. Sequentially Consistent operations: We use _InterlockedExchange or
 * CompareExchange (0,0) to force the necessary hardware bus lock/store-buffer
 * drain that standard volatile loads/stores do not provide.
 */

#ifndef osapi_cc_msvc_atomic_impl_h
#define osapi_cc_msvc_atomic_impl_h

#if !defined(RTI_MSVC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
/* __MSC_VER is defined by Microsoft Visual Studio compilers. */
    #if defined(_MSC_VER)
        #define RTI_MSVC_ATOMIC_PRIMITIVES_ARE_SUPPORTED
    #endif
#endif

#if defined(RTI_MSVC_ATOMIC_PRIMITIVES_ARE_SUPPORTED) && defined(_MSC_VER)
    #include <intrin.h>
#endif

/* Memory order constants and atomic type */

#if defined(RTI_MSVC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELAXED         0
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE         1
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELEASE         2
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE 3
    #define OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT  4
    #define RTI_ATOMIC(type__) type__ volatile
#else
    #error "No atomic primitives are supported"
#endif

/******************************* Memory barrier *******************************/

#if !defined(RTI_HAS_C11_ATOMICS) && \
    defined(RTI_MSVC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)

    /* _ReadWriteBarrier is a compiler-only fence. To enforce hardware ordering
     * without windows.h,perform a dummy interlocked operation.
     */
    #define OSAPI_Atomic_memory_barrier(memory_order__) \
        do { \
            if ((memory_order__) == OSAPI_ATOMIC_MEMORY_ORDER_RELAXED) \
            { \
                _ReadWriteBarrier(); \
            } \
            else \
            { \
                long _barrier_dummy = 0; \
                _InterlockedOr(&_barrier_dummy, 0); \
            } \
        } while (0)

    #define OSAPI_Atomic_initializeI(var__, value__) *var__ = value__

    /* Standard MSVC Interlocked Fallback */
    #define OSAPI_Atomic_addI(var__, value__, memory_order__) \
        ((sizeof(*(var__)) == 8) ? \
            (RTI_INT64)_InterlockedExchangeAdd64((__int64 volatile *)(var__), (__int64)(value__)) : \
            (RTI_INT64)_InterlockedExchangeAdd((long volatile *)(var__), (long)(value__)))

    #define OSAPI_Atomic_subI(var__, value__, memory_order__) \
        ((sizeof(*(var__)) == 8) ? \
            (RTI_INT64)_InterlockedExchangeAdd64((__int64 volatile *)(var__), -(__int64)(value__)) : \
            (RTI_INT64)_InterlockedExchangeAdd((long volatile *)(var__), -(long)(value__)))

    /*
     * If order is Relaxed, we use volatile assignment for performance.
     * Otherwise, we use _InterlockedExchange to ensure a full barrier.
     */
    #define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
        (((memory_order__) == OSAPI_ATOMIC_MEMORY_ORDER_RELAXED) ? \
            (void)(*(var__) = (value__)) : \
            ((sizeof(*(var__)) == 8) ? \
                (void)_InterlockedExchange64((__int64 volatile *)(var__), (__int64)(value__)) : \
                (void)_InterlockedExchange((long volatile *)(var__), (long)(value__))))

    /* Compare exchange using 0,0 to act as a Load with Fence */
    #define OSAPI_Atomic_loadI(var__, memory_order__) \
        (((memory_order__) == OSAPI_ATOMIC_MEMORY_ORDER_RELAXED) ? \
            (RTI_INT64)(*(var__)) : \
            ((sizeof(*(var__)) == 8) ? \
                (RTI_INT64)_InterlockedCompareExchange64((__int64 volatile *)(var__), 0, 0) : \
                (RTI_INT64)_InterlockedCompareExchange((long volatile *)(var__), 0, 0)))

#elif !defined(RTI_HAS_C11_ATOMICS)
    #error "No atomic primitives are supported"
#endif

#endif /* osapi_cc_msvc_atomic_impl_h */
