/*
 * FILE: osapi_cc_clang_atomic_impl.h - Clang atomic operation helpers
 *
 * Copyright 2026-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*
 * NOTE on Clang Implementation:
 *
 * This implementation uses Clang's __atomic built-ins, which are 
 * binary-compatible with GCC's atomic model. 
 *
 * 1. Alignment Requirement: The pointers passed to these macros MUST be 
 * naturally aligned (4-byte for 32-bit, 8-byte for 64-bit). Clang 
 * built-ins on unaligned addresses can cause undefined behavior.
 *
 * 2. Type: We cast the result to RTI_INT64 to ensure a consistent return
 * type across the OSAPI layer.
 */
    
#ifndef osapi_cc_clang_atomic_impl_h
#define osapi_cc_clang_atomic_impl_h

#if !defined(RTI_CLANG_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
    #if defined(__clang__)
        #if defined(__has_builtin)
            #if __has_builtin(__atomic_load_n)
                #define RTI_CLANG_ATOMIC_PRIMITIVES_ARE_SUPPORTED
            #endif
        #endif
    #endif
#endif

/* Memory order constants and atomic type */

#if defined(RTI_CLANG_ATOMIC_PRIMITIVES_ARE_SUPPORTED) || \
            defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
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
    (defined(RTI_CLANG_ATOMIC_PRIMITIVES_ARE_SUPPORTED) || \
     defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED))

#define OSAPI_Atomic_memory_barrier(memory_order__) \
    __atomic_thread_fence(memory_order__)

#define OSAPI_Atomic_initializeI(var__, value__) \
    *var__ = value__

/*
    * Implementation using Clang's atomic built-ins.
    * Returns RTI_INT64 to maintain platform consistency.
    */
#define OSAPI_Atomic_addI(var__, value__, memory_order__) \
    ((RTI_INT64) __atomic_fetch_add(var__, value__, memory_order__))

#define OSAPI_Atomic_subI(var__, value__, memory_order__) \
    ((RTI_INT64) __atomic_fetch_sub(var__, value__, memory_order__))

#define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
    __atomic_store_n(var__, value__, memory_order__)

#define OSAPI_Atomic_loadI(var__, memory_order__) \
    __atomic_load_n(var__, memory_order__)

#elif !defined(RTI_HAS_C11_ATOMICS)
    #error "No atomic primitives are supported"
#endif

#endif /* osapi_cc_clang_atomic_impl_h */
