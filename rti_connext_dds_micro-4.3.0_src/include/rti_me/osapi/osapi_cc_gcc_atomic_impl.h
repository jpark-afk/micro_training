/*
 * FILE: osapi_cc_gcc_atomic_impl.h - GCC atomic operation helpers
 *
 * Copyright 2026-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef osapi_cc_gcc_atomic_impl_h
#define osapi_cc_gcc_atomic_impl_h

#if !defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
/* __GNUC__ is defined by both GCC and QCC compilers. */
    #if defined(__GNUC__) && \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
        #define RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED
    #endif
#endif

/* Memory order constants and atomic type */

#if defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELAXED __ATOMIC_RELAXED
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE __ATOMIC_ACQUIRE
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELEASE __ATOMIC_RELEASE
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE __ATOMIC_ACQ_REL
    #define OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT __ATOMIC_SEQ_CST
    #define RTI_ATOMIC(type__) type__ volatile
#else
    #error "No atomic primitives are supported"
#endif

/******************************* Memory barrier *******************************/

#if !defined(RTI_HAS_C11_ATOMICS) && \
    defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)

    #define OSAPI_Atomic_memory_barrier(memory_order__) \
        __atomic_thread_fence(memory_order__)

    #define OSAPI_Atomic_initializeI(var__, value__) \
        *var__ = value__

    /*
     * Even if regular APIs returns a value of the underlying type, GCC
     * implementation is inconsistent in this value. So, since it is not
     * needed, we are ignoring return value making this API void.
     */
    #define OSAPI_Atomic_addI(var__, value__, memory_order__) \
        ((RTI_INT64) __atomic_fetch_add(var__, value__, memory_order__))

    /*
     * Even if regular APIs returns a value of the underlying type, GCC
     * implementation is inconsistent in this value. So, since it is not
     * needed, we are ignoring return value making this API void.
     */
    #define OSAPI_Atomic_subI(var__, value__, memory_order__) \
        ((RTI_INT64) __atomic_fetch_sub(var__, value__, memory_order__))

    #define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
        __atomic_store_n(var__, value__, memory_order__)

    #define OSAPI_Atomic_loadI(var__, memory_order__) \
        __atomic_load_n(var__, memory_order__)

#elif !defined(RTI_HAS_C11_ATOMICS)
    #error "No atomic primitives are supported"
#endif

#endif /* osapi_cc_gcc_atomic_impl_h */
