/*
 * (c) Copyright, Real-Time Innovations, 2024.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef posixAtomicImpl_h
#define posixAtomicImpl_h

/******************************* Memory barrier *******************************/

#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_memory_barrier(memory_order__) \
    atomic_thread_fence(memory_order__)
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_memory_barrier(memory_order__) \
    __atomic_thread_fence(memory_order__)
#else
  #error "No atomic primitives are supported"
#endif

/***************************** Atomic initializer *****************************/

#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_initializeI(var__, value__) atomic_init(var__, value__)
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_initializeI(var__, value__) *var__ = value__
#else
  #error "No atomic primitives are supported"
#endif

/***************************** Atomic operations *****************************/

/*
 * Even if regular APIs returns a value of the underlying type, C11 and
 * GCC implementation are inconsistent in this value. So, since it is not
 * needed, we are ignoring return value making this API void.
 */
#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_addI(var__, value__, memory_order__) \
      ((RTI_INT64) atomic_fetch_add_explicit(var__, value__, memory_order__))
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_addI(var__, value__, memory_order__) \
      ((RTI_INT64) __atomic_fetch_add(var__, value__, memory_order__))
#else
  #error "No atomic primitives are supported"
#endif

/*
 * Even if regular APIs returns a value of the underlying type, C11 and GCC
 * implementation are inconsistent in this value. So, since it is not needed,
 * we are ignoring return value making this API void.
 */
#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_subI(var__, value__, memory_order__) \
      ((RTI_INT64) atomic_fetch_sub_explicit(var__, value__, memory_order__))
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_subI(var__, value__, memory_order__) \
      ((RTI_INT64) __atomic_fetch_sub(var__, value__, memory_order__))
#else
  #error "No atomic primitives are supported"
#endif

#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
    atomic_store_explicit(var__, value__, memory_order__)
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_storeI(var__, value__, memory_order__) \
    __atomic_store_n(var__, value__, memory_order__)
#else
  #error "No atomic primitives are supported"
#endif

#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_loadI(var__, memory_order__) \
    atomic_load_explicit(var__, memory_order__)
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_Atomic_loadI(var__, memory_order__) \
    __atomic_load_n(var__, memory_order__)
#else
  #error "No atomic primitives are supported"
#endif

#endif /* posixAtomicImpl_h */
