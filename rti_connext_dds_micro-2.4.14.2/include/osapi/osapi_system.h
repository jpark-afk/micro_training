/*
 * FILE: osapi_system.h - Definition of System API
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 06sep2022,tk MICRO-4151/PR.30884
 * - Corrected descriptions for OSAPI_SYSTEM_OBJECTID_START and
 *   OSAPI_SYSTEM_OBJECTID_MAX to not to refer to
 *   OSAPI_System_get_next_object_id and instead mention
 *   DDS_DomainParticipant_get_next_objectid as a function generating
 *   object IDs.
 * 08aug2022,ad MICRO-3830/PR.30774
 * - Only include declaration of OSAPI_System_gv_PortProperty for autosar
 * 08mar2022,tk MICRO-3487
 * - Moved documentation of public OSAPI System APIs to ifdoc.
 *   This change was unrelated specifically to MICRO-3487, but was
 *   done at the same time as the osapi documentation was updated.
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include thread_semaphore when OSAPI_THREAD_SEMAPHORE_ENABLED
 *   is TRUE.
 * 14sep2021,tk MICRO-3152/PR.29564
 * - Added OSAPI_System_add_thread_semaphore and
 *   OSAPI_System_delete_thread_semaphore declarations. This allows other
 *   modules to add semaphores without creating threads.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_System_get_interface
 *   OSAPI_System_set_listener
 *   OSAPI_System_get_listener
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Made all public documentation external
 * 13jan2021,tk MICRO-2807/PR27549 Removed unused functions for CERT.
 * 08sep2020,tk  MICRO-2506/PR#28039 Corrected internal/external doxygen tags
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 22mar2014,tk Updated API and documentation
 * 01deb2014,tk MICRO-716: Abstract system API
 * 12mar2012,tk Written
 */
/*ce
 * \file 
 * \brief System API definition
 */
#ifndef osapi_system_h
#define osapi_system_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPI_SYSTEM_MAX_HOSTNAME
 */
#define OSAPI_SYSTEM_MAX_HOSTNAME (64)

/*e \dref_OSAPISystemGroupDocs
 */

struct OSAPI_System;

/*ci
 * \brief The first valid object ID returned by functions generating object IDs.
 *
 * \details
 * DDS_DomainParticipant_get_next_objectid generates object IDs for DDS
 * entities.
 */
#define OSAPI_SYSTEM_OBJECTID_START 0x01000000U

/*ci
 * \brief The maximum returned valid object ID returned by functions
 *        generating object IDs is OSAPI_SYSTEM_OBJECTID_MAX - 1.
 *
 * \details
 * DDS_DomainParticipant_get_next_objectid generates object IDs for DDS
 * entities.
 */
#define OSAPI_SYSTEM_OBJECTID_MAX   0x7fffffffU

/*i \ingroup OSAPI_SystemClass
 *
 *  start timer definition
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_start_timer_T)(OSAPI_Timer_T self,
                              OSAPI_TimerTickHandlerFunction tick_handler)
)

/*i \ingroup OSAPI_SystemClass
 *
 *  stop timer definition
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_stop_timer_T)(OSAPI_Timer_T self)
)

/*i \ingroup OSAPI_SystemClass
 *
 *  get timer resolution
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_INT32
(*OSAPI_System_get_timer_resolution_T)(void)
)

/*i \ingroup OSAPI_SystemClass
 *
 * get time
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_time_T)(OSAPI_NtpTime *now)
)

/*i \ingroup OSAPI_SystemClass
 *
 * initialize
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_initialize_T)(void)
)

/*i \ingroup OSAPI_SystemClass
 *
 * finalize
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_finalize_T)(void)
)

/*i \ingroup OSAPI_SystemClass
 *
 * get_hostname
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_hostname_T)(char *const hostname)
)

struct OSAPI_SystemUUID;

/*i \ingroup OSAPI_SystemClass
 *
 * generate_uuid
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_generate_uuid_T)(struct OSAPI_SystemUUID *uuid_out)
)

/*i \ingroup OSAPI_SystemClass
 *
 * get_ticktime
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_ticktime_T)(RTI_INT32 *sec,RTI_UINT32 *nanosec)
)

/*e \dref_OSAPI_SystemI
 */
struct OSAPI_SystemI
{
    /*e \dref_OSAPI_SystemI_start_timer
     */
    OSAPI_System_start_timer_T start_timer;

    /*e \dref_OSAPI_SystemI_stop_timer
     */
    OSAPI_System_stop_timer_T stop_timer;

    /*e \dref_OSAPI_SystemI_get_timer_resolution
     */
    OSAPI_System_get_timer_resolution_T get_timer_resolution;

    /*e \dref_OSAPI_SystemI_get_time
     */
    OSAPI_System_get_time_T get_time;

    /*e \dref_OSAPI_SystemI_initialize
     */
    OSAPI_System_initialize_T initialize;

    /*e \dref_OSAPI_SystemI_finalize
     */
    OSAPI_System_finalize_T finalize;

    /*e \dref_OSAPI_SystemI_generate_uuid
     */
    OSAPI_System_generate_uuid_T generate_uuid;

    /*e \dref_OSAPI_SystemI_get_hostname
     */
    OSAPI_System_get_hostname_T get_hostname;

    /*e \dref_OSAPI_SystemI_get_ticktime
     */
    OSAPI_System_get_ticktime_T get_ticktime;
};

/*ce \dref_OSAPI_SystemI_INITIALIZER
 */
#define OSAPI_SystemI_INITIALIZER \
{\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL\
}

extern OSAPIDllVariable struct OSAPI_System *OSAPI_System_gv_System;

/*ce \dref_OSAPI_System_set_interface
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_interface(struct OSAPI_SystemI *intf);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 *  \brief Get the current system interface
 *
 *  \details
 *  This function returns the current system interface
 *
 *  \param [out] intf - The current system interface
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_interface(struct OSAPI_SystemI *intf);
#endif

/*i \ingroup OSAPI_SystemClass
 *
 *  System listener
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_on_system_initialize_T)(void *listener_data,struct OSAPI_System *system)
)

typedef void
(*OSAPI_System_on_system_finalize_T)(void *listener_data,struct OSAPI_System *system);

/*i \ingroup OSAPI_SystemClass
 *
 *  System listener
 */
struct OSAPI_SystemListener
{
    void *listener_data;
    OSAPI_System_on_system_initialize_T on_system_initialize;
    OSAPI_System_on_system_finalize_T on_system_finalize;
};

#define OSAPI_SystemListener_INITIALIZER \
{\
    NULL,\
    NULL,\
    NULL,\
}

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 *  \brief Install a system listeners
 *
 *  \details
 *
 *  The system listeners are called during the life-time of the system.
 *  Only listeners which are non-NULL are called, thus it is ok to install
 *  a partially set listener structure. System listeners can be installed
 *  until the system has been initialized by calling OSAPI_System_initialize.
 *  This function is not considered thread-safe. The listener value should
 *  always be initialized with OSAPI_SystemListener_DEFAULT before use.
 *
 *  \param [in] listener System listeners to call
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_listener(struct OSAPI_SystemListener *listener);

/*i \ingroup OSAPI_SystemClass
 *  \brief Get current system listeners
 *
 *  \details
 *  Return the current set of system listeners. This function is not thread-safe.
 *
 *  \param[in] listener System listener structure to fill in.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_listener(struct OSAPI_SystemListener *listener);
#endif

/*e \dref_OSAPI_SystemProperty
 *  \ingroup OSAPI_SystemClass
 *
 *  System properties
 */
struct OSAPI_SystemProperty
{
    /*e \dref_OSAPI_SystemProperty_timer_property
     */
    struct OSAPI_TimerProperty timer_property;

    /*e \dref_OSAPI_SystemProperty_hostname
     */
    char hostname[OSAPI_SYSTEM_MAX_HOSTNAME];
    
#if OSAPI_HAVE_PORT_PROPERTY
    /*e \dref_OSAPI_SystemProperty_port_property
     */
    struct OSAPI_PortProperty port_property;
#endif /* OSAPI_HAVE_PORT_PROPERTY */
};

#if OSAPI_HAVE_PORT_PROPERTY
#ifndef OSAPI_PortProperty_INITIALIZER
#error "OSAPI_HAVE_PORT_PROPERTY is TRUE, but OSAPI_PortProperty_INITIALIZER is undefined"
#endif

/*\ci Initializer for Port properties when present
 */
#define OSAPI_PORT_PROPERTY_INITIALIZER ,OSAPI_PortProperty_INITIALIZER

#ifdef RTI_AUTOSAR
/*ci \brief Global variables for port properties.
 *
 * \details
 *
 * This global variable is an exception to the coding stds about global
 * variables. It is allowed because the properties for a port may be
 * accessed across different modules.
 */
extern OSAPIDllVariable
struct OSAPI_PortProperty *OSAPI_System_gv_PortProperty;
#endif

#else
#define OSAPI_PORT_PROPERTY_INITIALIZER
#endif

#define OSAPI_SystemProperty_INITIALIZER \
{\
    OSAPI_TimerProperty_INITIALIZER,\
    {0}\
    OSAPI_PORT_PROPERTY_INITIALIZER \
}

/*i \ingroup OSAPI_SystemClass
 * \brief UUID definition
 *
 *  Abstract UUID object,a 128-bit value.
 */
struct OSAPI_SystemUUID
{
    RTI_UINT32 value[4];
};

/*i \ingroup OSAPI_SystemClass
 * \brief Generate a unique universal identifier (UUID)
 *
 * \param [out] uuid_out The generated UUID value
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_generate_uuid(struct OSAPI_SystemUUID *uuid_out);

/*i \ingroup OSAPI_SystemClass
 * \brief Get the current system time.
 *
 *  \details
 * In general, the system time is used by components to correlate both
 * internal and external events, such as data reception and ordering. Thus,
 * it is recommended that this function returns the real time. However, it is not
 * strictly required.
 *
 * Notes:
 * - It is assumed that the time returned by get_time is monotonically
 *   increasing. It is up to the implementation of this function to ensure
 *   this holds true.
 * - It is ok to return the same time as the last call.
 * - The clock used to report real-time can be different than the clock used to
 *   support start_timer and stop_timer.
 *
 * @param [out] now Time in NtpTime format.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 *
 * @exception None.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_time(OSAPI_NtpTime *now);

/*i \ingroup OSAPI_SystemClass
 *  \brief Get the resolution of the clock driving the timer in nano seconds.
 *
 *  \details
 *
 *  This function must return the frequency of the system timer used to implement
 *  OSAPI_SystemI::start_timer and OSAPI_SystemI::stop_timer API.
 *
 *  @return timer resolution in nanoseconds.
 */
OSAPIDllExport RTI_INT32
OSAPI_System_get_timer_resolution(void);

/*i \ingroup OSAPI_SystemClass
 *
 * \brief Start the timer.
 *
 * @param [in] self         Timer object.
 * @param [in] tick_handler Timer handle.
 *
 * @pre Initialized system.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_start_timer(OSAPI_Timer_T self,
                        OSAPI_TimerTickHandlerFunction tick_handler);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 * \brief Stop the timer.
 *
 * @param [in] self Timer
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_stop_timer(OSAPI_Timer_T self);
#endif

/*i \ingroup OSAPI_SystemClass
 *  \brief Initialize the system.
 *
 *  \details
 *
 *  This function initializes the system and calls the port specific initialize
 *  method first. The port specific initialization method must return RTI_TRUE
 *  on success and RTI_FALSE on failure. A system can only be initialized once.
 *
 *  @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_initialize(void);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
    \brief Finalize the system.

    \details

    This function finalizes the system and calls the port specific finalize
    method last. The port specific initialization method must return RTI_TRUE
    on success and RTI_FALSE on failure.

    @return RTI_TRUE on success, RTI_FALSE on failure.
*/
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_finalize(void);
#endif /* !RTI_CERT */

/*e \dref_OSAPI_System_get_property
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_property(struct OSAPI_SystemProperty *property);

/*e \dref_OSAPI_System_set_property
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_property(struct OSAPI_SystemProperty *property);

/*e \dref_OSAPI_System_get_native_interface
 */
OSAPIDllExport void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf);


/*i \ingroup OSAPI_SystemClass
 * \brief Get the hostname
 *
 * \details
 * Get the hostname
 *
 * @param [out] hostname  The buffer to store the hostname. Must be at
 *                        least OSAPI_SYSTEM_MAX_HOSTNAME bytes. If the
 *                        actual hostname is longer than
 *                        OSAPI_SYSTEM_MAX_HOSTNAME bytes (including \0) the
 *                        hostname is truncated.
 *
 * @pre Initialized system.
 *
 * @exception None.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @mtsafety SAFE
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_hostname(char *const hostname);


/*i \ingroup OSAPI_SystemClass
 *
 * \brief Get current tick time
 *
 * \details
 *
 * The ticktime is a time measurement used by Micro to determine how much
 * time has elapsed in a period. It does not have to be an absolute time,
 * but it _must_ be monotonically increasing. For this reason it is important
 * to choose the time source with care. For example, the system time may
 * mbe adjusted backward or forward and this could affect the measure time
 * lapse. The resolution of the tick time is expected to be no less than the
 * system timer, although it is not a requirement.
 *
 * @param[out] sec     The current ticktime in seconds
 *
 * @param[out] nanosec Additional nanoseconds in the current ticktime
 *
 * @pre Initialized system.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec);

#if OSAPI_THREAD_SEMAPHORE_ENABLED
/*i \ingroup OSAPI_SystemClass
 *
 * \brief Get a semaphore to suspend a thread
 *
 * \return A semaphore on success, NULL on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport OSAPI_Semaphore_T*
OSAPI_System_get_thread_semaphore(void);

/*i \ingroup OSAPI_SystemClass
 *
 * \brief Return a semaphore acquired with \ref OSAPI_System_get_thread_semaphore
 *
 * \param[in] sem Semaphore to return
 */
OSAPIDllExport void
OSAPI_System_return_thread_semaphore(OSAPI_Semaphore_T *sem);

/*e \ingroup OSAPI_SystemClass
 *
 * \brief Add a semaphore to possible suspend a thread created with
 *        OSAPI_THREAD_SUSPEND_ENABLE
 *
 * \details
 * Threads that are created with the OSAPI_THREAD_SUSPEND_ENABLE option
 * may be suspended in the receive path. When a thread is created with this
 * option enabled a semaphore is automatically created and added to a pool
 * by calling this method.
 *
 * Code paths that need to suspend the thread should use \ref
 * OSAPI_System_get_thread_semaphore and \ref
 * OSAPI_System_return_thread_semaphore.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_add_thread_semaphore(void);

#ifndef RTI_CERT
/*e \ingroup OSAPI_SystemClass
 *
 * \brief Delete a semaphore previously added with \ref
 * OSAPI_System_add_thread_semaphore.
 *
 * \details
 * When a thread is deleted that was created with the
 * OSAPI_THREAD_SUSPEND_ENABLE this function is automatically to delete
 * a semaphore from the pool.
 */
OSAPIDllExport void
OSAPI_System_delete_thread_semaphore(void);
#endif

#endif

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* osapi_system_h */
