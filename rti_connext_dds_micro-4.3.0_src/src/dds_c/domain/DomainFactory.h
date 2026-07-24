/*
 * FILE: DomainFactory.h - DomainFactory implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 16jun2012,tk  Update
 * 30apr2008,tk  Written
 */
/*ci
 * \file
 * \brief DomainFactory implementation
 */
#ifndef DomainFactory_h
#define DomainFactory_h

#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif

#ifndef osapi_atomic_h
#include "osapi/osapi_atomic.h"
#endif

/*ci \addtogroup DDSDomainModule
 * @{
 */

/*ci
 * \brief DDS_DomainParticipantFactory implementation
 */
struct DDS_DomainParticipantFactoryImpl
{
    /*ci
     * \brief Flag used to ensure to factory is only initialized once.
     *        Accessed with atomic operations for memory safety.
     */
    RTI_ATOMIC(DDS_Boolean) is_initialized;

    /*ci
     * \brief Mutex to make the Factory API thread-safe
     */
    struct OSAPI_Mutex *factory_lock;

    /*ci
     * \brief The current QoS policies for the factory, mutable until the
     *        first entity is created
     */
    struct DDS_DomainParticipantFactoryQos qos;

    /*ci
     * \brief The current default participant Qos policies
     */
    struct DDS_DomainParticipantQos default_participant_qos;

    /*ci
     * \brief The run-time registry used by this factory
     */
    RT_Registry_T *registry;

    /*ci
     * \brief Memory pool to create participant from
     */
    REDA_BufferPool_T participant_pool;

    /*ci
     * \brief List of currently created participants
     */
    REDA_CircularList_T participants;

    /*ci
     * \brief The factory qos policies are mutable until the first participant
     *        is created, this flag controls when
     */
    DDS_Boolean immutable_qos_enabled;

    /*ci
     * \brief The factory maintains its own database for tables shared between
     *        participants, such as the registry
     */
    DB_Database_T db;

    /*ci
     * \brief Internal constant used to distinguish between different
     *        participants created from the same factory
     */
    DDS_Long instance_counter;

    /*ci
     * \brief List of programs created by the public CDR serialize/deserialize
     *        functions.
     */
    REDA_CircularList_T program_list;
};

extern DDS_Boolean
DDS_DomainParticipantFactory_is_participant_name_unique(
    DDS_DomainParticipantFactory *self,
    const char *name,
    DDS_DomainId_t domain_id);

#endif

/*ci @} */
