
/*
 * FILE: DomainFactory.c - DomainFactory implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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
#ifndef rtps_trust_plugin_h
#include "rtps/rtps_trust_plugin.h"
#endif

#include "Entity.h"
#include "DomainParticipantQos.h"
#include "DomainParticipant.h"
#include "DomainFactoryQos.h"
#include "DataWriterInterface.h"
#include "DataReaderInterface.h"
#include "DomainFactory.h"
#include "DomainParticipantTrust.h"

#if DDS_FLOW_CONTROLLER_ENABLED
#include <FlowControl.h>
#include <LeakyBucketFlowController.h>
#endif

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

#if DDS_XTYPES_IS_ENABLED
RTI_PRIVATE void
DDS_DomainParticipantFactory_finalize_programs(DDS_DomainParticipantFactory *self)
{
    DDS_TypeProgramNode *programs = NULL;
    DDS_TypeProgramNode *programs_next = NULL;

    programs = (DDS_TypeProgramNode*)
                            REDA_CircularList_get_first(&self->program_list);

    while (!REDA_CircularList_node_at_head(&self->program_list,
                                           &programs->_node))
    {
        programs_next = (DDS_TypeProgramNode*)
                          REDA_CircularListNode_get_next(&programs->_node);

        if (programs->type_intf->type_factory != NULL)
        {
            if (programs->context != NULL)
            {
                DDS_TypeInterfaceI_delete_execution_context(
                                programs->type_intf->type_factory,
                                programs->context);
                programs->context = NULL;
            }
            DDS_TypeInterfaceI_delete_program(programs->type_intf->type_factory,
                                              programs->programs);

        }
        REDA_CircularList_unlink_node(&programs->_node);
        programs = programs_next;
    }
}
#endif

DDS_DomainParticipantFactory*
DDS_DomainParticipantFactory_get_instance(void)
{
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;

    if (!OSAPI_Atomic_load(&DDS_fv_DomainFactory.is_initialized, 
                           OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE))
    {
        if (!OSAPI_System_initialize())
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SYSTEM_OBJECT)
            return NULL;
        }

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

        DDS_fv_DomainFactory.qos = (struct DDS_DomainParticipantFactoryQos)DDS_DomainParticipantFactoryQos_INITIALIZER;
        DDS_fv_DomainFactory.default_participant_qos = (struct DDS_DomainParticipantQos)DDS_DomainParticipantQos_INITIALIZER;
        DDS_fv_DomainFactory.participant_pool = NULL;
        REDA_CircularList_init(&DDS_fv_DomainFactory.participants);
        DDS_fv_DomainFactory.immutable_qos_enabled = DDS_BOOLEAN_FALSE;
        DDS_fv_DomainFactory.registry = RT_Registry_get_instance();
        DDS_fv_DomainFactory.instance_counter = 0;
        REDA_CircularList_init(&DDS_fv_DomainFactory.program_list);
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

        /* Maximum number of factories to register. Create two tables in the
         * registry. One table is for default components, the 2nd for user
         * registered components.
         */
        db_property.max_tables = 2;
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

        /* max_system_factories must equal the number of calls to
         * RT_Registry_system_register below in this function. The value
         * is local to this function as system components are not
         * registered anywhere else. The flow-controller registers 1 component.
         */
#if !UDP_EXCLUDE_BUILTIN
        rt_property.max_system_factories = 6;
#else
        rt_property.max_system_factories = 5;
#endif
        if (!RT_Registry_set_property(DDS_fv_DomainFactory.registry,&rt_property))
        {
            DDSC_LOG_OBJECT_SET_PROPERTY(OSAPI_LOGKIND_ERROR,DDSC_LOG_RT_OBJECT)
            return NULL;
        }

        if (!RT_Registry_system_register(DDS_fv_DomainFactory.registry,
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
        if (!RT_Registry_system_register(DDS_fv_DomainFactory.registry,
                      NETIO_DEFAULT_UDP_NAME,
                      UDP_InterfaceFactory_get_interface(),
                      &DomainParticipantFactory_gv_UdpProperty._parent._parent,
                      NULL))
        {
            return NULL;
        }
#endif /* !UDP_EXCLUDE_BUILTIN */

        if (!RT_Registry_system_register(DDS_fv_DomainFactory.registry,
                                  NETIO_DEFAULT_RTPS_NAME,
                                  RTPS_InterfaceFactory_get_interface(),
                                  NULL,NULL))
        {
            return NULL;
        }

        if (!RT_Registry_system_register(DDS_fv_DomainFactory.registry,
                    DDS_DEFAULT_DATAWRITER_NETIO_NAME,
                    DDS_DataWriterInterfaceFactory_get_interface(),NULL, NULL))
        {
            return NULL;
        }

        if (!RT_Registry_system_register(DDS_fv_DomainFactory.registry,
                    DDS_DEFAULT_DATAREADER_NETIO_NAME,
                    DDS_DataReaderInterfaceFactory_get_interface(),
                    NULL, NULL))
        {
            return NULL;
        }

#if DDS_FLOW_CONTROLLER_ENABLED
        if (DDS_LeakyBucketFlowControllerFactory_register(
                                    DDS_fv_DomainFactory.registry,NULL)
                                    != DDS_RETCODE_OK)
         {
             return NULL;
         }
#endif

        OSAPI_Atomic_store(&DDS_fv_DomainFactory.is_initialized, 
                               DDS_BOOLEAN_TRUE, 
                               OSAPI_ATOMIC_MEMORY_ORDER_RELEASE);
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
    struct RT_ComponentFactory *c_factory;

    if (!OSAPI_Atomic_load(&DDS_fv_DomainFactory.is_initialized, 
                           OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE))
    {
        return DDS_RETCODE_OK;
    }

    OSAPI_Atomic_store(&DDS_fv_DomainFactory.is_initialized, 
                       DDS_BOOLEAN_FALSE, 
                       OSAPI_ATOMIC_MEMORY_ORDER_RELEASE);

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

    c_factory = RT_Registry_lookup(DDS_fv_DomainFactory.registry,
                                   NETIO_DEFAULT_INTRA_NAME);
    if (c_factory != NULL)
    {
        if (!RT_Registry_unregister(DDS_fv_DomainFactory.registry,
                                    NETIO_DEFAULT_INTRA_NAME,NULL, NULL))
        {
            return DDS_RETCODE_ERROR;
        }
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

#if DDS_FLOW_CONTROLLER_ENABLED
        if (DDS_LeakyBucketFlowControllerFactory_unregister(DDS_fv_DomainFactory.registry,NULL)
                                                            != DDS_RETCODE_OK)
         {
             return DDS_RETCODE_ERROR;
         }
#endif

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

#if DDS_XTYPES_IS_ENABLED
    DDS_DomainParticipantFactory_finalize_programs(&DDS_fv_DomainFactory);
#endif

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

    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    {
        struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;

        if (!RT_Registry_get_property(DDS_fv_DomainFactory.registry,&rt_property))
        {
            DDSC_LOG_OBJECT_GET_PROPERTY(OSAPI_LOGKIND_ERROR,DDSC_LOG_RT_OBJECT)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }

        /* This handles the case where the immutable properties can no longer
         * be changed by only setting them if they are different and have passed
         * the previous test above for trying to change immutable.
         */
        if (rt_property.max_factories != (RTI_SIZE_T)DDS_fv_DomainFactory.qos.resource_limits.max_components)
        {
            rt_property.max_factories = (RTI_SIZE_T)DDS_fv_DomainFactory.qos.resource_limits.max_components;

            if (!RT_Registry_set_property(DDS_fv_DomainFactory.registry,&rt_property))
            {
                DDSC_LOG_OBJECT_SET_PROPERTY(OSAPI_LOGKIND_ERROR,DDSC_LOG_RT_OBJECT)
                retcode = DDS_RETCODE_ERROR;
            }
        }
    }

done:
    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Check if the participant name is unique in the domain.

 * \param self The DomainParticipantFactory instance.
 * \param name The participant name to check.
 * \param domain_id The domain ID to check against.
 *
 * \return DDS_BOOLEAN_TRUE if the name is unique, DDS_BOOLEAN_FALSE otherwise.
 */
DDS_Boolean
DDS_DomainParticipantFactory_is_participant_name_unique(
    DDS_DomainParticipantFactory *self,
    const char *name,
    DDS_DomainId_t domain_id)
{
    struct DDS_DomainParticipantEntry *a_dp;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (name == NULL),
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    a_dp = (struct DDS_DomainParticipantEntry*)
                            REDA_CircularList_get_first(&self->participants);
    while (!REDA_CircularList_node_at_head(&self->participants,a_dp))
    {
        if ((domain_id == a_dp->participant.domain_id) &&
            OSAPI_String_cmp(a_dp->participant.qos.participant_name.name,
                             name) == 0)
        {
            return DDS_BOOLEAN_FALSE;
        }
        a_dp = (struct DDS_DomainParticipantEntry*)
                            REDA_CircularListNode_get_next(&a_dp->node);
    }

    return DDS_BOOLEAN_TRUE;
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
        p_prop.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_DomainParticipantEntry);
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

    /* Check if the participant name is unique and not empty when
     * discovery by participant name is enabled.
     */
    if (qos->discovery.enable_participant_discovery_by_name)
    {
        if (OSAPI_String_length(qos->participant_name.name) == 0)
        {
            DDSC_LOG_PARTICIPANT_NAME_EMPTY(OSAPI_LOGKIND_ERROR);
            goto done;
        }

        if (!DDS_DomainParticipantFactory_is_participant_name_unique(self,
                qos->participant_name.name, domain_id))
        {
            DDSC_LOG_DUPLICATE_PARTICIPANT_NAME(OSAPI_LOGKIND_ERROR,
                                                qos->participant_name.name);
            goto done;
        }
    }

    if (!DDS_DomainParticipantFactory_assert_trust_config(
        self->registry,qos,&dp_cfg.trust))
    {
        DDSC_LOG_TRUST_CREATE_TRUST_PLUGINS_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!DDS_DomainParticipant_validate_local_participant_trust(
        dp_cfg.trust, domain_id, qos, &dp_cfg.guid))
    {
        DDSC_LOG_TRUST_PREPARE_LOCAL_PARTICIPANT_STATE_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    participant = (struct DDS_DomainParticipantEntry*)
                             REDA_BufferPool_get_buffer(self->participant_pool);
    if (participant == NULL)
    {
        DDSC_LOG_OBJECT_EMPTY(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_PARTICIPANT_POOL_OBJECT)
        goto done;
    }

    if (!DDS_DomainParticipant_initialize(&participant->participant,
                self,domain_id,qos,listener,mask,&dp_cfg,self->factory_lock))
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

#ifndef RTI_CERT
    if (retval == NULL)
    {
        if (!DDS_DomainParticipantFactory_delete_trust_config(
            self->registry,qos,dp_cfg.trust))
        {
            DDSC_LOG_TRUST_DELETE_TRUST_PLUGINS(OSAPI_LOGKIND_ERROR)
        }
    }

#endif

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
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_DomainParticipantEntry *p_entry;
    struct REDA_CircularListNode *a_node;
    const struct DDS_DomainParticipantQos *qos;
    struct NDDS_ParticipantConfig *dp_cfg;


    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DomainParticipant_finalize(participant);

    if (retcode == DDS_RETCODE_OK)
    {

        qos = DDS_DomainParticipant_get_qos_ref(participant);
        dp_cfg = DDS_DomainParticipant_get_cfg_ref(participant);
        DDS_Trust_DomainParticipant_invalidate_local_participant_trust(dp_cfg->trust);
        retval = DDS_DomainParticipantFactory_delete_trust_config(
                                        self->registry,qos,dp_cfg->trust);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDSC_LOG_TRUST_DELETE_TRUST_PLUGINS(OSAPI_LOGKIND_ERROR);
        }
#else
        UNUSED_ARG(retval);
#endif
        a_node = (struct REDA_CircularListNode *)participant - 1;
        p_entry = (struct DDS_DomainParticipantEntry*)a_node;
        REDA_CircularList_unlink_node(&p_entry->node);
        REDA_BufferPool_return_buffer(self->participant_pool,p_entry);
    }

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return DDS_RETCODE_ERROR;
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

#if DDS_ENABLE_APPGEN
DDS_DomainParticipant*
DDS_DomainParticipantFactory_lookup_participant_by_name(
                            DDS_DomainParticipantFactory *self,
                            const char *participant_name)
{
    DDS_DomainParticipant *retval = NULL;
    struct DDS_DomainParticipantEntry *p_entry;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant_name == NULL),
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("participant_name",
                                                       participant_name,
                                                       RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return NULL;
    }

    p_entry = (struct DDS_DomainParticipantEntry *)
                              REDA_CircularList_get_first(&self->participants);
    while (p_entry != (struct DDS_DomainParticipantEntry *)&self->participants)
    {
        if (!OSAPI_String_cmp(
                       p_entry->participant.builtin_data.participant_name.name,
                       participant_name))
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
#endif /* DDS_ENABLE_APPGEN */

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

#if DDS_XTYPES_IS_ENABLED
/*ci \dref_DomainParticipantFactory_assert_program
 */
const struct RTIXCdrInterpreterPrograms*
DDS_DomainParticipantFactory_assert_program(
                                    DDS_DomainParticipantFactory *self,
                                    const struct DDS_TypePluginI *intf,
                                    DDS_TypeProgramNode *program,
                                    const DDS_TypeCode *const tc)
{
    if (!OSAPI_Mutex_take(self->factory_lock))
    {
        return NULL;
    }

    if (!REDA_CircularListNode_is_linked(&program->_node))
    {
        struct RTIXCdrInterpreterProgramsGenProperty programProperty =
                            RTIXCdrInterpreterProgramsGenProperty_INITIALIZER;

        programProperty.resolveAlias = RTI_XCDR_TRUE;
        programProperty.inlineStruct = RTI_XCDR_TRUE;
        programProperty.optimizeEnum = RTI_XCDR_TRUE;
        program->programs = NULL;

        if (intf->type_factory != NULL)
        {
            program->context =  DDS_TypeInterfaceI_create_execution_context(
                                                        intf->type_factory,tc);

            if (program->context == NULL)
            {
                goto done;
            }

            program->programs = DDS_TypeInterfaceI_create_program(
                                            intf->type_factory,
                                            (RTIXCdrTypeCode*)tc,
                                            &programProperty,
                                            RTI_XCDR_SER_PROGRAM |
                                            RTI_XCDR_DESER_PROGRAM |
                                            RTI_XCDR_GET_MAX_SER_SIZE_PROGRAM);
        }

        if (program->programs != NULL)
        {
            REDA_CircularList_append(&self->program_list,&program->_node);
        }
        else if (program->context != NULL)
        {
            /* program->context is always initialized to NULL */
            DDS_TypeInterfaceI_delete_execution_context(intf->type_factory,
                                                        program->context);
            program->context = NULL;
        }
    }

 done:

    if (!OSAPI_Mutex_give(self->factory_lock))
    {
        return NULL;
    }

    return program->programs;
}
#endif


/*ci @} */
