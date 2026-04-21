/*
 * FILE: DomainFactory.c - DomainFactory implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2022.
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
 * 09aug2022,tk MICRO-4102/PR.30495
 * - Only create 1 table for RT. RT only requires one table to store factories
 *   in get_instance
 * 08aug2022,tk MICRO-4062/PR.30753
 * - Added robustness check for unique participant GUID prefix in
 *   create_participant
 * 05may2022,jh MICRO-3538
 * - Excluded c_factory in DDS_DomainParticipantFactory_finalize_instance
 *   when building without UDP support
 * 16mar2022,am MICRO-3519/PR.30311
 * - Excluded netio_udp.h when builtin UDP is excluded from the build.
 * 13dec2021,tk MICRO-3368/PR.30028
 * - Check for DDS_DomainParticipant_initialize == NULL instead of
 *   !DDS_DomainParticipant_initialize since DDS_DomainParticipant_initialize
 *   returns a pointer.
 * 6oct2020,fmt MICRO-2585/PR.28155
 *     - Initialize system before any object is created.
 * 18dec2015,eh  MICRO-1490: Set first 2 bytes of GUID to Vendor ID
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 15jun2015,tk  MICRO-1299/PR#14981 Check return value from generate_uuid()
 *                                   after change in API
 * 12mar2015,tk  MICRO-1102/PR#14163 Removed magic constants
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 20feb2015,eh  MICRO-813/PR#9172 Fix Lint warnings
 * 02feb2015,tk  MICRO-976/PR#12939 Added logging to create_participant()
 * 01dec2014,tk  MICRO-979/PR#12948 Removed unused variable
 * 01dec2014,tk  MICRO-970/PR#12919 Enable MC/DC coverage testing
 * 01jul2014,eh  MICRO-819 Initialize DPF participants list
 * 05may2014,as  MICRO-270 Always enable precondition
 *                         checks for public API operations
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *                         and support for StatusConditions
 * 02aug2013,tk  MICRO-298/PR#1436: Removed duplicate assignment
 * 16jun2012,tk  Update
 * 30apr2008,tk  Written
 */
/*ci
 * \file
 * \brief DomainFactory implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_loopback_h
#include "netio/netio_loopback.h"
#endif
#if !UDP_EXCLUDE_BUILTIN
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "Entity.h"
#include "DomainParticipantQos.h"
#include "DomainParticipant.h"
#include "DomainFactoryQos.h"
#include "DataWriterInterface.h"
#include "DataReaderInterface.h"
#include "DomainFactory.h"

/*ci
 * \brief Structure used to maintain a list of participants created by the
 *        DomainParticipantFactory
 */
struct DDS_DomainParticipantEntry
{
    /*ci
     * \brief base-class
     */
    struct REDA_CircularListNode node;

    /*ci
     * \brief Pointer to participant
     */
    struct DDS_DomainParticipantImpl participant;
};

#if defined(__GNUC__)
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct DDS_DomainParticipantFactoryImpl DDS_fv_DomainFactory =
{
        .is_initialized = DDS_BOOLEAN_FALSE
};
#else
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct DDS_DomainParticipantFactoryImpl DDS_fv_DomainFactory =
{
    DDS_BOOLEAN_FALSE
};
#endif

#if !UDP_EXCLUDE_BUILTIN
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE 
struct UDP_InterfaceFactoryProperty DomainParticipantFactory_gv_UdpProperty =
                                       UDP_InterfaceFactoryProperty_INITIALIZER;
#endif /* !UDP_EXCLUDE_BUILTIN */

/*** SOURCE_BEGIN ***/

DDS_DomainParticipantFactory*
DDS_DomainParticipantFactory_get_instance(void)
{
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;

    if (!DDS_fv_DomainFactory.is_initialized)
    {
        /* Before intializing factory, print version */
        OSAPI_TRACE_DDS("RTI Connext Micro, Version",RTI_FALSE)
        OSAPI_TRACE_INT32("major",RTIME_DDS_VERSION_MAJOR,RTI_FALSE)
        OSAPI_TRACE_INT32("minor",RTIME_DDS_VERSION_MINOR,RTI_FALSE)
        OSAPI_TRACE_INT32("revision",RTIME_DDS_VERSION_REVISION,RTI_FALSE)
#ifndef RTI_CERT
        OSAPI_TRACE_INT32("release",RTIME_DDS_VERSION_RELEASE,RTI_FALSE)
        OSAPI_TRACE_STRING("buildid",DDSC_Library_get_version(),RTI_TRUE)
#else
        OSAPI_TRACE_INT32("release",RTIME_DDS_VERSION_RELEASE,RTI_TRUE)
#endif

        if (!OSAPI_System_initialize())
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SYSTEM_OBJECT)
            return NULL;
        }

        DDS_fv_DomainFactory.qos = DDS_PARTICIPANT_FACTORY_QOS_DEFAULT;
        DDS_fv_DomainFactory.default_participant_qos = DDS_PARTICIPANT_QOS_DEFAULT;
        DDS_fv_DomainFactory.participant_pool = NULL;
        REDA_CircularList_init(&DDS_fv_DomainFactory.participants);
        DDS_fv_DomainFactory.immutable_qos_enabled = DDS_BOOLEAN_FALSE;
        DDS_fv_DomainFactory.registry = RT_Registry_get_instance();
        DDS_fv_DomainFactory.instance_counter = 0;
        DDS_fv_DomainFactory.factory_lock = OSAPI_Mutex_new();

        if (DDS_fv_DomainFactory.factory_lock == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
            return NULL;
        }

#if OSAPI_ENABLE_LOG
        if (!OSAPI_Log_initialize())
        {
            return NULL;
        }
#endif

        DDS_fv_DomainFactory.db = NULL;

        /* Maximum number of tables for RT. RT only needs one table for
         * factories.
         */
        db_property.max_tables = 1;
        db_property.lock_mode = DB_LOCK_LEVEL_SHARED;
        dbrc = DB_Database_create(&DDS_fv_DomainFactory.db,"shared",&db_property,
                                  DDS_fv_DomainFactory.factory_lock);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_DATABASE_CREATE(OSAPI_LOGKIND_ERROR,dbrc)
            return NULL;
        }

        if (!RT_Registry_get_property(DDS_fv_DomainFactory.registry,&rt_property))
        {
            DDSC_LOG_OBJECT_GET_PROPERTY(OSAPI_LOGKIND_ERROR,DDSC_LOG_RT_OBJECT)
            return NULL;
        }

        rt_property.db = DDS_fv_DomainFactory.db;
        rt_property.max_factories = (RTI_SIZE_T)DDS_fv_DomainFactory.qos.resource_limits.max_components;

        if (!RT_Registry_set_property(DDS_fv_DomainFactory.registry,&rt_property))
        {
            DDSC_LOG_OBJECT_SET_PROPERTY(OSAPI_LOGKIND_ERROR,DDSC_LOG_RT_OBJECT)
            return NULL;
        }

        if (!RT_Registry_register(DDS_fv_DomainFactory.registry,
                                  NETIO_DEFAULT_INTRA_NAME,
                                  LOOP_InterfaceFactory_get_interface(),
                                  NULL, NULL))
        {
            return NULL;
        }

        /* register default UDP transport factory only in case
         * UDP transport is not excluded from the build
         */
#if !UDP_EXCLUDE_BUILTIN
        if (!RT_Registry_register(DDS_fv_DomainFactory.registry,
                      NETIO_DEFAULT_UDP_NAME,
                      UDP_InterfaceFactory_get_interface(),
                      &DomainParticipantFactory_gv_UdpProperty._parent._parent,
                      NULL))
        {
            return NULL;
        }
#endif /* !UDP_EXCLUDE_BUILTIN */

        if (!RT_Registry_register(DDS_fv_DomainFactory.registry,
                                  NETIO_DEFAULT_RTPS_NAME,
                                  RTPS_InterfaceFactory_get_interface(),
                                  NULL,NULL))
        {
            return NULL;
        }

        if (!RT_Registry_register(DDS_fv_DomainFactory.registry,
                    DDS_DEFAULT_DATAWRITER_NETIO_NAME,
                    DDS_DataWriterInterfaceFactory_get_interface(),NULL, NULL))
        {
            return NULL;
        }

        if (!RT_Registry_register(DDS_fv_DomainFactory.registry,
                    DDS_DEFAULT_DATAREADER_NETIO_NAME,
                    DDS_DataReaderInterfaceFactory_get_interface(),
                    NULL, NULL))
        {
            return NULL;
        }

        DDS_fv_DomainFactory.is_initialized = DDS_BOOLEAN_TRUE;
    }

    return &DDS_fv_DomainFactory;
}

RT_Registry_T*
DDS_DomainParticipantFactory_get_registry(DDS_DomainParticipantFactory *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->registry;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipantFactory_finalize_instance(void)
{
#if !UDP_EXCLUDE_BUILTIN
    struct RT_ComponentFactory *c_factory;
#endif /* !UDP_EXCLUDE_BUILTIN */

    if (!DDS_fv_DomainFactory.is_initialized)
    {
        return DDS_RETCODE_OK;
    }

    DDS_fv_DomainFactory.is_initialized = DDS_BOOLEAN_FALSE;

    if (DDS_fv_DomainFactory.factory_lock)
    {
        if (!OSAPI_Mutex_delete(DDS_fv_DomainFactory.factory_lock))
        {
            return DDS_RETCODE_ERROR;
        }
        DDS_fv_DomainFactory.factory_lock = NULL;
    }

    if (DDS_fv_DomainFactory.participant_pool)
    {
        if (!REDA_BufferPool_delete(DDS_fv_DomainFactory.participant_pool))
        {
            return DDS_RETCODE_ERROR;
        }
        DDS_fv_DomainFactory.participant_pool = NULL;
    }

    if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                NETIO_DEFAULT_INTRA_NAME,NULL, NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                NETIO_DEFAULT_RTPS_NAME,NULL, NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                DDS_DEFAULT_DATAREADER_NETIO_NAME,NULL, NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                DDS_DEFAULT_DATAWRITER_NETIO_NAME,NULL, NULL))
    {
        return DDS_RETCODE_ERROR;
    }

#if !UDP_EXCLUDE_BUILTIN
    c_factory = RT_Registry_lookup(DDS_fv_DomainFactory.registry,
                                   NETIO_DEFAULT_UDP_NAME);
    if (c_factory != NULL)
    {
        if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                    NETIO_DEFAULT_UDP_NAME,NULL, NULL))
        {
            return DDS_RETCODE_ERROR;
        }
    }
#endif /* !UDP_EXCLUDE_BUILTIN */

    if (!RT_Registry_finalize(DDS_fv_DomainFactory.registry))
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_fv_DomainFactory.db != NULL)
    {
        if (DB_Database_delete(DDS_fv_DomainFactory.db) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        DDS_fv_DomainFactory.db = NULL;
    }

    if (!OSAPI_System_finalize())
    {
        DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SYSTEM_OBJECT)
        return DDS_RETCODE_ERROR;
    }

#if OSAPI_ENABLE_LOG
    if (!OSAPI_Log_finalize())
    {
        return DDS_RETCODE_ERROR;
    }
#endif

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos(DDS_DomainParticipantFactory *self,
                                    struct DDS_DomainParticipantFactoryQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DomainParticipantFactoryQos_copy(qos, &self->qos);

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_qos(DDS_DomainParticipantFactory *self,
                             const struct DDS_DomainParticipantFactoryQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_DomainParticipantFactoryQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_PARTICIPANTFACTORYQOS_OBJECT)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if ((self->immutable_qos_enabled) &&
        !DDS_DomainParticipantFactoryQos_immutable_is_equal(&self->qos, qos))
    {
        DDSC_LOG_QOS_IMMUTABLE(OSAPI_LOGKIND_ERROR,
                               DDSC_LOG_PARTICIPANTFACTORYQOS_OBJECT)
        retcode = DDS_RETCODE_IMMUTABLE_POLICY;
        goto done;
    }

    retcode = DDS_DomainParticipantFactoryQos_copy(&self->qos, qos);

done:
    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant(
                        DDS_DomainParticipantFactory *self,
                        DDS_DomainId_t domain_id,
                        const struct DDS_DomainParticipantQos *qos,
                        const struct DDS_DomainParticipantListener *listener,
                        DDS_StatusMask mask)
{
    DDS_DomainParticipant *retval = NULL;
    struct REDA_BufferPoolProperty p_prop = REDA_BufferPoolProperty_INITIALIZER;
    struct DDS_DomainParticipantEntry *participant;
    struct NDDS_ParticipantConfig dp_cfg = NDDS_ParticipantConfig_INITIALIZER;
    DDS_ReturnCode_t ddsrc;
    struct RTPS_Guid guid;
    struct OSAPI_SystemUUID uuid;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (domain_id < 0) || (qos == NULL),
                       return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_int("domain_id",domain_id,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (((qos->protocol.rtps_host_id == DDS_RTPS_AUTO_ID) ||
        (qos->protocol.rtps_app_id == DDS_RTPS_AUTO_ID) ||
        (qos->protocol.rtps_instance_id == DDS_RTPS_AUTO_ID)) &&
        ((qos->protocol.rtps_host_id + qos->protocol.rtps_app_id +
         qos->protocol.rtps_instance_id) != DDS_RTPS_AUTO_ID))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
        return NULL;
    }

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return NULL;
    }

    if (!self->immutable_qos_enabled)
    {
        p_prop.buffer_size = sizeof(struct DDS_DomainParticipantEntry);
        p_prop.max_buffers = (RTI_SIZE_T)self->qos.resource_limits.max_participants;
        self->participant_pool = REDA_BufferPool_new("participants",
                                            &p_prop,NULL,NULL,NULL,NULL);
        if (self->participant_pool == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                     DDSC_LOG_PARTICIPANT_POOL_OBJECT)
            goto done;
        }
        self->immutable_qos_enabled = DDS_BOOLEAN_TRUE;
    }

    dp_cfg.registry = self->registry;
    dp_cfg.instance = DDS_fv_DomainFactory.instance_counter;

    /* Generate GUID for the participant. It is sufficient to test only
     * on id for DDS_RTPS_AUTO_ID, since at this point either all ID are
     * DDS_RTPS_AUTO_ID or none are DDS_RTPS_AUTO_ID.
     */
    if (qos->protocol.rtps_host_id == DDS_RTPS_AUTO_ID)
    {
        if (!OSAPI_System_generate_uuid(&uuid))
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                     DDSC_LOG_UUID_OBJECT)
            goto done;
        }
        OSAPI_Memory_copy(&dp_cfg.guid, &uuid.value, 12);
    }
    else
    {
        guid.prefix.host_id = qos->protocol.rtps_host_id;
        guid.prefix.app_id  = qos->protocol.rtps_app_id;
        guid.prefix.instance_id  = qos->protocol.rtps_instance_id;
        /* Set by the participant itself */
        guid.object_id = 0;
        DDS_GUID_from_rtps(&dp_cfg.guid,&guid);
    }

    /* Most significant 2 bytes of GUID prefix are RTPS Vendor ID */
    dp_cfg.guid.value[0] = RTPS_VENDOR_ID_MAJOR;
    dp_cfg.guid.value[1] = RTPS_VENDOR_ID_MINOR;


    /* Check if the generated dp_cfg.guid already exists, fail if it does
     */
    {
        struct DDS_DomainParticipantEntry *a_dp;

        a_dp = (struct DDS_DomainParticipantEntry*)
                            REDA_CircularList_get_first(&self->participants);
        while (!REDA_CircularList_node_at_head(&self->participants,a_dp))
        {
            if ((domain_id == a_dp->participant.domain_id) &&
                !OSAPI_Memory_compare(&a_dp->participant.config.guid.value,
                                      &dp_cfg.guid.value,
                                      RTI_SIZEOF(struct RTPS_GuidPrefix)))
            {
                /* Duplicate GUID prefix, exit. The Participant object ID is
                 * always the same (well-known)
                 */
                DDSC_LOG_DUPLICATE_GUID_PREFIX(OSAPI_LOGKIND_ERROR)
                goto done;
            }
            a_dp = (struct DDS_DomainParticipantEntry*)
                            REDA_CircularListNode_get_next(&a_dp->node);
        }
    }

    participant = (struct DDS_DomainParticipantEntry*)
                             REDA_BufferPool_get_buffer(self->participant_pool);
    if (participant == NULL)
    {
        DDSC_LOG_OBJECT_EMPTY(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_PARTICIPANT_POOL_OBJECT)
        goto done;
    }

    if (DDS_DomainParticipant_initialize(&participant->participant,
         self,domain_id,qos,listener,mask,&dp_cfg,self->factory_lock) == NULL)
    {
        REDA_BufferPool_return_buffer(self->participant_pool,participant);
        goto done;
    }

    if (self->qos.entity_factory.autoenable_created_entities)
    {
        ddsrc = DDS_Entity_enable(&participant->participant.as_entity);
        if (ddsrc != DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_PARTICIPANT_ENTITY)
            goto done;
        }
    }

    REDA_CircularList_link_node_after(&self->participants,&participant->node);

    DDS_fv_DomainFactory.instance_counter++;
    retval = &participant->participant;

done:

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return NULL;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_participant(
                                            DDS_DomainParticipantFactory *self,
                                            DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct DDS_DomainParticipantEntry *p_entry;
    struct REDA_CircularListNode *a_node;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    retcode = DDS_DomainParticipant_finalize(participant);

    if (retcode == DDS_RETCODE_OK)
    {
        if (!OSAPI_Mutex_take(self->factory_lock))
        {
            return DDS_RETCODE_ERROR;
        }

        a_node = (struct REDA_CircularListNode *)participant - 1;
        p_entry = (struct DDS_DomainParticipantEntry*)a_node;
        REDA_CircularList_unlink_node(&p_entry->node);
        REDA_BufferPool_return_buffer(self->participant_pool,p_entry);

        if (!OSAPI_Mutex_give(self->factory_lock))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    return retcode;
}
#endif

/*******************************************************************************
 *                              OPTIONAL APIs
 ******************************************************************************/
DDS_DomainParticipant*
DDS_DomainParticipantFactory_lookup_participant(
                            DDS_DomainParticipantFactory *self,
                            DDS_DomainId_t domain_id)
{
    DDS_DomainParticipant *retval = NULL;
    struct DDS_DomainParticipantEntry *p_entry;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return NULL;
    }

    p_entry = (struct DDS_DomainParticipantEntry *)
                              REDA_CircularList_get_first(&self->participants);
    while (p_entry != (struct DDS_DomainParticipantEntry *)&self->participants)
    {
        if (DDS_DomainParticipant_get_domain_id(&p_entry->participant)
                                                == domain_id)
        {
            retval = &p_entry->participant;
            break;
        }
        p_entry = (struct DDS_DomainParticipantEntry *)
                                REDA_CircularListNode_get_next(&p_entry->node);
    }

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return NULL;
    }

    return retval;
}

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos(
                                DDS_DomainParticipantFactory *self,
                                const struct DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t retcode;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DomainParticipantQos_copy(&self->default_participant_qos,qos);

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_default_participant_qos(
                            DDS_DomainParticipantFactory *self,
                            struct DDS_DomainParticipantQos * qos)
{
    DDS_ReturnCode_t retcode;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DomainParticipantQos_copy(qos,&self->default_participant_qos);

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

/*ci @} */
