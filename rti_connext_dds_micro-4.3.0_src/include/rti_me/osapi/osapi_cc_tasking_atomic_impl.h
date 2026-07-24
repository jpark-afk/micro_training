/*
 * FILE: osapi_cc_tasking_atomic_impl.h - TASKING atomic operation helpers
 *
 * Copyright 2026-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef osapi_cc_tasking_atomic_impl_h
#define osapi_cc_tasking_atomic_impl_h

#if !defined(RTI_TASKING_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
    #if defined(__TASKING__)
        #define RTI_TASKING_ATOMIC_PRIMITIVES_ARE_SUPPORTED
    #endif
#endif

/* Memory order constants and atomic type */

#if defined(RTI_TASKING_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
    #include <stdatomic.h>
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELAXED memory_order_relaxed
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE memory_order_acquire
    #define OSAPI_ATOMIC_MEMORY_ORDER_RELEASE memory_order_release
    #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE memory_order_acq_rel
    #define OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT memory_order_seq_cst
    #define RTI_ATOMIC(type__) _Atomic(type__)
#else
    #error "No atomic primitives are supported"
#endif

/******************************* Memory barrier *******************************/

#if !defined(RTI_HAS_C11_ATOMICS) && \
    defined(RTI_TASKING_ATOMIC_PRIMITIVES_ARE_SUPPORTED)

    #define OSAPI_Atomic_memory_barrier(memory_order__) \
        atomic_thread_fence(memory_order__)

    #define OSAPI_Atomic_initializeI(var__, value__) \
        atomic_init(var__, value__)

    #define OSAPI_Atomic_addI(var__, value__, memory_order__) \
        ((RTI_INT64) atomic_fetch_add_explicit(var__, value__, memory_order__))

    #define OSAPI_Atomic_subI(var__, value__, memory_order__) \
        ((RTI_INT64) atomic_fetch_sub_explicit(var__, value__, memory_order__))

    #define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
        atomic_store_explicit(var__, value__, memory_order__)

    #define OSAPI_Atomic_loadI(var__, memory_order__) \
        atomic_load_explicit(var__, memory_order__)

#elif !defined(RTI_HAS_C11_ATOMICS)
    #error "No atomic primitives are supported"
#endif

#endif /* osapi_cc_tasking_atomic_impl_h */
