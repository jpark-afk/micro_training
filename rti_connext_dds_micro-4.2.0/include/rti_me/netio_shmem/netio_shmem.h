/*
 * FILE: netio_shmem.h - netio_shmem
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \file
 * \defgroup NETIO_SHMEMInterfaceClass Shared Memory Transport Interface
 * \ingroup NETIOModule
 * \brief Shared Memory Transport Interface
 *
 * \details
 *
 * The shared memory interface is implemented as a NETIO interface and
 * NETIO interface factory.
 */
/*ci \addtogroup NETIO_SHMEMInterfaceClass
 * @{
 */
#ifndef netio_shmem_h
#define netio_shmem_h

#include "netio/netio_interface.h"

#ifndef netio_shmem_dll_h
#include "netio_shmem/netio_shmem_dll.h"
#endif
#ifndef netio_shmem_config_h
#include "netio_shmem/netio_shmem_config.h"
#endif
#ifndef netio_shmem_log_h
#include "netio_shmem/netio_shmem_log.h"
#endif

#include "osapi/osapi_mutex.h"
#include "osapi/osapi_thread.h"
#include "dds_c/dds_c_infrastructure.h"

/*e
 *  \defgroup NETIO_SharedMemoryClass NETIO Shared Memory API 
 *  \ingroup SHMEMPluginModule
 *
 *  \brief OS independent way to setup and access shared-memory
 *  segments, shared-memory mutexes and
 *  shared memory binary semaphores.
 *
 *  This module consist of three constructs:
 *      SharedMemorySegment,
 *      SharedMemoryBinarySemaphore,
 *      SharedMemoryMutex.
 *
 *  These objects,
 *  represent operating-system resources that are accessible from
 *  different processes running on the same computer. These three
 *  objects are manipulated in very similar ways and have also very
 *  similar lifecycles.
 *
 *  The model is as follows. The underlying shared memory resource (
 *  segment, semaphore, and mutex) is identified using an integer
 *  "key". Processes wishing to create or interact with the resource
 *  objects must do it via the appropriate handles. To be portable
 *  across operating systems the application should consider keys to be
 *  unique even across different kinds of shared memory resources, that
 *  is if a key is specified for a shared-memory segment it cannot be
 *  re-used to specify a shared-memory semaphore or mutex. Different keys
 *  must be used for different resources even if they are of different
 *  kinds.
 *
 *  The thing to keep in mind when looking at the lifecycles is that
 *  there are potentially many shared-memory handles referencing the
 *  same underlying shared-memory resource. That is in fact the whole
 *  point of using shared memory resource.
 *
 *  A shared-memory object can be in one of the following states:
 *  CREATED, OPEN, CLOSED.
 *
 *  A shared-memory handle can be in one of the following states:
 *  DETACHED, ATTACHED.
 *
 *  The state transitions of the shared-memory object are surrogate to
 *  those of corresponding handles because processes can only manipulate
 *  shared memory using the handles. The following charts describe this:
 *
 *  \code
 *          +-> DETACHED -----+
 *          |     |           |
 *          |     | create()  |
 * detach() |     |           | attach()
 *          |     v           |
 *          +-- ATTACHED <----+
 *                ^           | additional operations
 *                |           | depending on the resource
 *                +-----------+
 *  \endcode
 *
 *  State of the shared-memory resource:
 *  \code
 *                 INITIAL
 *                    |
 *                    | create()
 *                    |
 *                    |  +---------+
 *                    v  |         |
 *                 CREATED         | attach()
 *                    |  ^         | detach()
 *                    |  |         |
 *                    |  +---------+
 *                    |
 *                    | delete()
 *                    v
 *                  FINAL
 *  \endcode
 *
 *  No control is performed on the permission of the delete() function.
 *  Once a resource is deleted, it is permanently destroyed and any
 *  subsequent call to attach() should fail.
 *
 *  The shared-memory objects used in this module represent named
 *  operating-system resources. Many OSs provide utilities to view and
 *  administer these operating system resources from an independent
 *  API. For example, in Unix/Linux the command-line utility "ipcs" can
 *  be used to see a listing of the current shared memory segments,
 *  semaphores and mutexes. Similarly "ipcrm" can be used to remove
 *  shared-memory resources.
 */

#define NETIO_SHMEM_HANDLE_MAX_LENGTH  (2)

typedef RTI_UINT32 NETIO_SharedMemoryHandle[NETIO_SHMEM_HANDLE_MAX_LENGTH];

#define NETIO_SharedMemoryNativeHandleSegment NETIO_SharedMemoryHandle
#define NETIO_SharedMemoryNativeHandleSemMutex NETIO_SharedMemoryHandle

#include "netio_shmem/netio_shmem_semaphore.h"
#include "netio_shmem/netio_shmem_mutex.h"
#include "netio_shmem/netio_shmem_segment.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSAPI_SHARED_MEMORY_ATTACHED                        6
#define OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN             7
#define OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED     8
#define OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY            9
#define OSAPI_SHARED_MEMORY_CREATED                         10
#define OSAPI_SHARED_MEMORY_MAXCOUNT_REACHED                11
#define OSAPI_SHARED_MEMORY_FAIL_REASON_NOT_OWNER           12
#define OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION        13
#define OSAPI_SHARED_MEMORY_SUCCESS                         14
#define OSAPI_SHARED_MEMORY_REQUESTED_SEGMENT_TOO_BIG       15

#define NETIO_SHMEM_THREAD_PROPERTY_DEFAULT \
{                                           \
    OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE,   \
    OSAPI_THREAD_PRIORITY_INHERIT,          \
    OSAPI_THREAD_SUSPEND_ENABLE             \
}

#define NETIO_SHMEM_MESSAGE_SIZE_MAX_DEFAULT (65536)

#define NETIO_SHMEM_RECEIVED_MESSAGE_COUNT_MAX_DEFAULT  (64)

#define NETIO_SHMEM_RECEIVE_BUFFER_SIZE_DEFAULT       \
    (NETIO_SHMEM_RECEIVED_MESSAGE_COUNT_MAX_DEFAULT * \
    NETIO_SHMEM_MESSAGE_SIZE_MAX_DEFAULT / 4)

#define NETIO_SHMEMInterfaceFactoryProperty_INITIALIZER \
    {                                                   \
        NETIO_InterfaceFactoryProperty_INITIALIZER,     \
        NETIO_SHMEM_RECEIVED_MESSAGE_COUNT_MAX_DEFAULT, \
        NETIO_SHMEM_RECEIVE_BUFFER_SIZE_DEFAULT,        \
        NETIO_SHMEM_MESSAGE_SIZE_MAX_DEFAULT,           \
        NETIO_SHMEM_THREAD_PROPERTY_DEFAULT,            \
        DDS_PRODUCTVERSION_UNKNOWN                      \
    }

#define NETIO_SHMEM_SEGMENT_KEY_OFFSET 0x400000
#define NETIO_SHMEM_SEGMENT_KEY_FACTOR 1

#define NETIO_SHMEM_SEMAPHORE_KEY_OFFSET 0x800000
#define NETIO_SHMEM_SEMAPHORE_KEY_FACTOR 1

#define NETIO_SHMEM_MUTEX_KEY_OFFSET 0xB00000
#define NETIO_SHMEM_MUTEX_KEY_FACTOR 1

/*i
 * \brief Major version for the transport plugin after fixing bug 14240 (RTI-28)
 */
#define NETIO_SHMEM_MAJOR_AFTER_BUG_14240_FIX (2)

/*i \brief
 * This is the version of the shared memory transport that uses a concurrent
 * queue with a writer cookie, used in Network Capture (CORE-10280) to get the
 * PID of the source for inbound traffic.
 */
#define NETIO_SHMEM_MAJOR_AFTER_WRITER_COOKIE_SUPPORT (4)

/*i
 * \brief Version of the shared memory transport that uses a robust
 *        pthread mutex to protect the shared memory segment.
 */
#define NETIO_SHMEM_MAJOR_AFTER_ROBUST_PTHREAD_MUTEX (5)

/* \brief Default major version */
#define NETIO_SHMEM_VERSION_MAJOR_DEFAULT NETIO_SHMEM_MAJOR_AFTER_BUG_14240_FIX

/* \brief Default minor version */
#define NETIO_SHMEM_VERSION_MINOR_DEFAULT (0)

struct NETIO_SHMEMInterfaceFactoryProperty;

/*ce \dref_SHMEM_InterfaceFactoryProperty_initialize
 *   \ingroup SHMEMPluginModule
 */
NETIO_SHMEMDllExport RTI_BOOL
NETIO_SHMEMInterfaceFactoryProperty_initialize(
        struct NETIO_SHMEMInterfaceFactoryProperty *self);

/*ce \dref_SHMEM_InterfaceFactoryProperty_finalize
 *   \ingroup SHMEMPluginModule
 */
NETIO_SHMEMDllExport RTI_BOOL
NETIO_SHMEMInterfaceFactoryProperty_finalize(
        struct NETIO_SHMEMInterfaceFactoryProperty *p);

#ifdef __cplusplus
}
#endif

/*e \dref_SHMEM_InterfaceFactoryProperty
 *  \ingroup SHMEMPluginModule
 */
struct NETIO_SHMEMInterfaceFactoryProperty
{
    struct NETIO_InterfaceFactoryProperty _parent;

    /*e \dref_SHMEM_InterfaceFactoryProperty_received_message_count_max
     * \brief Max number of received message sizes that can be residing
     *  inside the shared memory transport concurrent queue.
     */
    RTI_INT32 received_message_count_max;

    /*e \dref_SHMEM_InterfaceFactoryProperty_receive_buffer_size
     * \brief The size of the receive socket buffer
     */
    RTI_INT32 receive_buffer_size;

    /*e \dref_SHMEM_InterfaceFactoryProperty_message_size_max
     * \brief The maximum size of the message which can be received
     */
    RTI_INT32 message_size_max;

    /*e \dref_SHMEM_InterfaceFactoryProperty_recv_thread_property
     * \brief Thread properties for each receive thread created by this
     * NETIO interface.
     */
    struct OSAPI_ThreadProperty recv_thread_property;

    /*e
     * \brief The minimum Connext Pro version that the shared memory
     *        transport should be compatible with.
     */
    struct DDS_ProductVersion pro_minimum_compatibility_version;

#ifdef RTI_CPP
public:
    NETIO_SHMEMInterfaceFactoryProperty()
    {
        NETIO_SHMEMInterfaceFactoryProperty_initialize(this);
    }
    ~NETIO_SHMEMInterfaceFactoryProperty() {
    }
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

#define NETIO_SHMEM_INTERFACE_ID RT_MKINTERFACEID( \
        RT_COMPONENT_CLASS_NETIO,                  \
        RT_COMPONENT_INSTANCE_SHMEM)

NETIO_SHMEMDllVariable extern struct NETIO_SHMEMInterfaceFactoryProperty
        NETIO_SHMEMINTERFACE_FACTORY_PROPERTY_DEFAULT;

/*ce \dref_SHMEM_InterfaceFactory_get_interface
 */
MUST_CHECK_RETURN NETIO_SHMEMDllExport struct RT_ComponentFactoryI*
NETIO_SHMEMInterfaceFactory_get_interface(void);

MUST_CHECK_RETURN NETIO_SHMEMDllExport RTI_BOOL
NETIO_SHMEMInterfaceFactory_register(
        RT_Registry_T *registry,
        const char *name,
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener);

/*ci \dref_SHMEMInterfaceFactory_get_version
 */
NETIO_SHMEMDllExport const char*
NETIO_SHMEMInterfaceFactory_get_version(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* netio_shmem_h */

/*ci @} */

