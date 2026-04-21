/*
 * (c) Copyright, Real-Time Innovations, 2024.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef posixAtomic_h
#define posixAtomic_h

/*i
 * \brief This header implements macros to perform atomic operations for all RTI
 *        supported platforms by using C11 Atomics or GCC built-in atomic support.
 *
 * \see https://en.cppreference.com/w/c/atomic for reference on C11 Atomics.
 * \see https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins
 *      for reference on GCC built-in atomic support.
 */

/*
 * Check if the compiler supports C11 Atomics.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L \
        && !defined(__STDC_NO_ATOMICS__)
  #define RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED
#endif

/*
 * Check if GCC's built-in atomic support is available (GCC >= 4.7).
 */
#if defined(__GNUC__) && (__GNUC__ >= 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
  #define RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED
#endif

/*
 * If C11 Atomics are available, we should trust in the C11 standard to increase
 * performance (no acquire-release order is needed if not specified).
 */
#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #include <stdatomic.h>
#endif

#include "rti_me_psl/ospsl/ospsl_os_posix_atomic_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*i
 * \brief The memory order for memory barriers and atomic operations.
 *
 * \details There are five options of memory barrier to choose:
 * - Relaxed: no ordering constraints.
 * - Acquire: a load operation with this memory order performs the acquire
 *   operation on the affected memory location: no reads or writes in the
 *   current thread can be reordered before this load. All writes in other
 *   threads that release the same atomic variable are visible in the current
 *   thread.
 * - Release: a store operation with this memory order performs the release
 *   operation: no reads or writes in the current thread can be reordered after
 *   this store. All writes in the current thread are visible in other threads
 *   that acquire the same atomic variable (see Release-Acquire ordering below)
 *   and writes that carry a dependency into the atomic variable become visible
 *   in other threads that consume the same atomic.
 * - Acquire-release: A read-modify-write operation with this memory order is
 *   both an acquire operation and a release operation. No memory reads or
 *   writes in the current thread can be reordered before the load, nor after
 *   the store. All writes in other threads that release the same atomic
 *   variable are visible before the modification and the modification is
 *   visible in other threads that acquire the same atomic variable.
 * - Sequentially consistent: acquire-release memory barrier + all memory
 *   modifications before the barrier are ensured to be visible in all threads.
 *
 * Using these options in atomic operations allows three kind of approaches for
 * memory ordering synchronization between threads:
 * - Relaxed: no ordering constraints, only atomicity is guaranteed.
 * - Acquire-release: all memory accesses (included non-atomic and relaxed
 *   atomic) before an atomic store tagged release are guaranteed to happen
 *   before an atomic load (in the same memory address) tagged acquire. This
 *   affects only to the involved threads, and no ordering is guaranteed between
 *   other threads.
 * - Sequentially consistent: all memory accesses (included non-atomic and all
 *   atomic) are guaranteed to happen in a single total order, and this order
 *   is the same for all threads.
 *
 * Note we are ignoring consume memory order since most of the compilers have
 * reported that they cannot implement it safely so, since it is mostly the
 * acquire, we avoid using it.
 * \see https://en.cppreference.com/w/c/atomic/memory_order for reference.
 */
#define OSAPI_AtomicMemoryOrder RTI_UINT32

#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_ATOMIC_MEMORY_ORDER_RELAXED memory_order_relaxed
  #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE memory_order_acquire
  #define OSAPI_ATOMIC_MEMORY_ORDER_RELEASE memory_order_release
  #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE memory_order_acq_rel
  #define OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT memory_order_seq_cst
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define OSAPI_ATOMIC_MEMORY_ORDER_RELAXED __ATOMIC_RELAXED
  #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE __ATOMIC_ACQUIRE
  #define OSAPI_ATOMIC_MEMORY_ORDER_RELEASE __ATOMIC_RELEASE
  #define OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE __ATOMIC_ACQ_REL
  #define OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT __ATOMIC_SEQ_CST
#endif

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i
 * \brief Establishes memory synchronization ordering of non-atomic and
          relaxed atomic accesses, ensuring at least the memory order
          constraints specified by the memory_order__ argument.
 *
 * \param[in] memory_order__  The memory order constraints to be established.
 */
void
OSAPI_Atomic_memory_barrier(OSAPI_AtomicMemoryOrder memory_order);
#endif

/*i
 * \brief Declare a variable of type type__ as atomic.
 * \details This variable cannot be initialized directly but using
 * OSAPI_Atomic_initialize.
 */
#if defined(RTI_C11_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define RTI_ATOMIC(type__) _Atomic(type__)
#elif defined(RTI_GCC_ATOMIC_PRIMITIVES_ARE_SUPPORTED)
  #define RTI_ATOMIC(type__) type__ volatile
#else
  #error "No atomic primitives are supported"
#endif

/*i
 * \brief Initializes an atomic variable, perform a non-atomic store.
 *
 * \param[in] var    A pointer to the variable to be initialized.
 * \param[in] value  The value to initialize the variable with.
 *
 * \details Note that this operation is not thread-safe, it is not an atomic
 * operation. This operation is needed because an atomic object can contain more
 * fields than the underlying type to make atomicity and synchronization work,
 * and this API ensures the correct initialization of these other fields. This
 * is mandatory only for variables with automatic storage duration, since static
 * or thread-local storage duration variables are guaranteed to be initialized
 * in a valid state with the default (zero) initialization.
 * Note this is also a way to perform a non-atomic store operation.
 */
#define OSAPI_Atomic_initialize(var__, value__) \
  OSAPI_Atomic_initializeI(var__, value__)

/**************************** Atomic add operation ****************************/

/*i
 * \brief Atomically adds value__ to the variable pointed by var__.
 *
 * \param[inout] var__  The pointer to the variable to be modified. Cannot be NULL.
 * \param[in] value__   The value to be added.
 * \param[in] memory_order__ The memory order constraints to be established.
 *                           Only OSAPI_ATOMIC_MEMORY_ORDER_RELAXED,
 *                           OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE, and
 *                           OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT are allowed.
 *
 * \return The value held previously by the atomic object.
 *
 * \details The returned value has type RTI_INT64 regardless of the type of the
 * atomic variable due to some platforms limitations. Cast the return value back
 * to the right type if needed.
 */
#define OSAPI_Atomic_add(var__, value__, memory_order__) \
  OSAPI_Atomic_addI(var__, value__, memory_order__)

/**************************** Atomic sub operation ****************************/

/*i
 * \brief Atomically subtract value__ from the variable pointed by var__.
 *
 * \param[inout] var__        A pointer to the variable to be modified. Cannot be NULL.
 * \param[in] value__         The value to be subtract.
 * \param[in] memory_order__  The memory order constraints to be established.
 *                            Only OSAPI_ATOMIC_MEMORY_ORDER_RELAXED,
 *                            OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE, and
 *                            OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT are allowed.
 *
 * \return The value held previously by the atomic object.
 *
 * \details The returned value has type RTI_INT64 regardless of the type of the
 * atomic variable due to some platforms limitations. Cast the return value back
 * to the right type if needed.
 */
#define OSAPI_Atomic_sub(var__, value__, memory_order__) \
  OSAPI_Atomic_subI(var__, value__, memory_order__)

/*************************** Atomic store operation ***************************/

/*i
 * \brief Atomically store value__ to the variable pointed by var__.
 *
 * \param[inout] var__        A pointer to the variable variable to be modified. Cannot be NULL.
 * \param[in] value__         The value to be stored.
 * \param[in] memory_order__  The memory order constraints to be established.
 *                            Only OSAPI_ATOMIC_MEMORY_ORDER_RELAXED,
 *                            OSAPI_ATOMIC_MEMORY_ORDER_RELEASE, and
 *                            OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT are allowed.
 */
#define OSAPI_Atomic_store(var__, value__, memory_order__) \
  OSAPI_Atomic_storeI(var__, value__, memory_order__)

/*************************** Atomic load operation ****************************/

/*i
 * \brief Atomically load the value of the variable pointed by var__. Cannot be NULL.
 *
 * \param[in] var__           A pointer to the variable to load from.
 * \param[in] memory_order__  The memory order constraints to be established.
 *                            Only OSAPI_ATOMIC_MEMORY_ORDER_RELAXED,
 *                            OSAPI_ATOMIC_MEMORY_ORDER_RELEASE, and
 *                            OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT are allowed.
 */
#define OSAPI_Atomic_load(var__, memory_order__) \
  OSAPI_Atomic_loadI(var__, memory_order__)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* posixAtomic_h */
