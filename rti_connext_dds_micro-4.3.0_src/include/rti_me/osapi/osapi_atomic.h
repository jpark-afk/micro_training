/*
 * FILE: osapi_atomic.h - Atomic abstraction
 *
 * Copyright 2026-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/* Atomic operations abstraction
 *
 * This header is the generic front-door for OSAPI atomics.
 *
 * - API documentation and generic wrapper macros live here.
 * - Platform/compiler-specific details live in the selected implementation
 *   header (included below), including:
 *   - Atomic type declaration macro: RTI_ATOMIC(type__)
 *   - Memory order values: OSAPI_ATOMIC_MEMORY_ORDER_* constants
 *   - Primitive implementations: OSAPI_Atomic_*I(...) macros
 */

#ifndef osapi_atomic_h
#define osapi_atomic_h

#include "osapi/osapi_features.h"

#ifndef OSAPI_ATOMIC_DEF_H
#if defined(RTIME_OSAPI_ATOMIC)
#define OSAPI_ATOMIC_DEF_H RTIME_OSAPI_ATOMIC
#elif defined(_MSC_VER)
#define OSAPI_ATOMIC_DEF_H osapi_cc_msvc_atomic_impl.h
#elif defined(__clang__)
#define OSAPI_ATOMIC_DEF_H osapi_cc_clang_atomic_impl.h
#elif defined(__GNUC__)
#define OSAPI_ATOMIC_DEF_H osapi_cc_gcc_atomic_impl.h
#elif defined(__TASKING__)
#define OSAPI_ATOMIC_DEF_H osapi_cc_tasking_atomic_impl.h
#else
#error "Unable to determine atomic implementation"
#endif
#endif

#include OSAPI_CC_STRINGIFY_DEFINE(OSAPI_ATOMIC_DEF_H)

#if !defined(RTI_ATOMIC)
    #error "Atomic implementation must define RTI_ATOMIC(type__)"
#endif

#if !defined(OSAPI_ATOMIC_MEMORY_ORDER_RELAXED) || \
    !defined(OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE) || \
    !defined(OSAPI_ATOMIC_MEMORY_ORDER_RELEASE) || \
    !defined(OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE_RELEASE) || \
    !defined(OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT)
    #error "Atomic implementation must define OSAPI_ATOMIC_MEMORY_ORDER_*"
#endif

#if !defined(OSAPI_Atomic_memory_barrier)
    #error "Atomic implementation must define OSAPI_Atomic_memory_barrier"
#endif

#if !defined(OSAPI_Atomic_initializeI) || \
    !defined(OSAPI_Atomic_addI) || \
    !defined(OSAPI_Atomic_subI) || \
    !defined(OSAPI_Atomic_storeI) || \
    !defined(OSAPI_Atomic_loadI)
    #error "Atomic implementation must define OSAPI_Atomic_*I primitives"
#endif

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

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i
 * \brief Establishes memory synchronization ordering of non-atomic and
 *        relaxed atomic accesses, ensuring at least the memory order
 *        constraints specified by the memory_order__ argument.
 *
 * \param[in] memory_order__  The memory order constraints to be established.
 */
void
OSAPI_Atomic_memory_barrier(RTI_UINT32 memory_order__);
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


/*i
 * \brief Atomically load the value of the variable pointed by var__. Cannot be NULL.
 *
 * \param[in] var__           A pointer to the variable to load from.
 * \param[in] memory_order__  The memory order constraints to be established.
 *                            Only OSAPI_ATOMIC_MEMORY_ORDER_RELAXED,
 *                            OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE, and
 *                            OSAPI_ATOMIC_MEMORY_ORDER_SEQ_CONSISTENT are allowed.
 */
#define OSAPI_Atomic_load(var__, memory_order__) \
    OSAPI_Atomic_loadI(var__, memory_order__)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* osapi_atomic_h */
