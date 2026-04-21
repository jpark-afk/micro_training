/*
 * FILE: RTRegistry.c - RT Implementation
 *
 * Copyright 2011-2021 Real-Time Innovations, Inc.
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
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check for RT_ComponentFactoryId_set_name
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 05may2014,tk MICRO-256 / PR#1419: Removed redundant code
 * 05may2014,tk MICRO-72: Updated based on CR-232
 * 02aug2013,tk MICRO-82 / PR#256: Updated after code review.
 * 02aug2013,tk MICRO-246 / PR#1416: Use NULL, not RTI_FALSE in precond check
 * 12dec2011,tk Written
 */
/*ci
 * \file
 *
 * \brief Implementation of the RT API.
 */
/*ci \addtogroup RTInternalModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef rt_log_h
#include "rt/rt_log.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#include "RTRegistry.h"

const struct RT_RegistryProperty RTCOMPONENTFACTORY_REGISTRY_PROPERTY_DEFAULT =
                                                RT_RegistryProperty_INITIALIZER;

#define RT_COMPONENT_FACTORY_NAME "rt"

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_Registry RT_ComponentFactory_fv_Instance =
{
        RTI_FALSE,
        NULL,
        RT_RegistryProperty_INITIALIZER
};

/*** SOURCE_BEGIN ***/

/*ci
 *
 * \brief Implementation of required database function to compare
 *        component factories
 *
 * \details
 *
 * This function is installed as the compare function for the
 * table of registered components.
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A RT_ComponentFactory already in the database
 * \param[in] op2   Either a RT_ComponentFactory being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
RT_ComponentFactory_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct RT_ComponentFactory *left_entry = (struct RT_ComponentFactory*)op1;
    union RT_ComponentFactoryId *id;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id = (union RT_ComponentFactoryId*)op2;
    }
    else
    {
        id = &((struct RT_ComponentFactory*)op2)->_id;
    }

    if (left_entry->_id._value._high > id->_value._high)
    {
        return 1;
    }
    if (left_entry->_id._value._high < id->_value._high)
    {
        return -1;
    }

    if (left_entry->_id._value._low > id->_value._low)
    {
        return 1;
    }
    if (left_entry->_id._value._low < id->_value._low)
    {
        return -1;
    }

    return 0;
}

RTI_BOOL
RT_Registry_get_property(RT_Registry_T *registry,
                            struct RT_RegistryProperty *property)
{
    OSAPI_PRECONDITION((registry == NULL) || (property == NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    *property = registry->property;

    return RTI_TRUE;
}

RTI_BOOL
RT_Registry_set_property(RT_Registry_T *registry,
                            struct RT_RegistryProperty *property)
{
    OSAPI_PRECONDITION((registry == NULL) || (property == NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    if (registry->_is_initialized)
    {
        RT_LOG_SET_IMMUTABLE_PROPERTY(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    registry->property = *property;

    return RTI_TRUE;
}

/*ci
 * \brief Initialize the registry
 *
 * \details
 * This function is called internally by \ref RT_Registry_register
 * when the first component is registered with the run-time.
 * Until the registry is initialized its properties can be changed.
 *
 * \param[in] registry Registry to initialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RT_Registry_initialize(RT_Registry_T *registry)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((registry == NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("registry",registry,RTI_TRUE);)

    registry->factory_table = NULL;

    tbl_property.max_cursors = 1;
    tbl_property.max_indices = 0;
    tbl_property.max_records = registry->property.max_factories;

    dbrc = DB_Database_create_table(&registry->factory_table,
                                   registry->property.db,
                                   RT_COMPONENT_FACTORY_NAME,
                                   sizeof(struct RT_ComponentFactory),
                                   RT_ComponentFactory_compare,
                                   &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        RT_LOG_REGISTRY_INIT_FAILURE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    registry->_is_initialized = RTI_TRUE;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
RT_Registry_finalize(RT_Registry_T *registry)
{
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((registry == NULL),
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("registry",registry,RTI_TRUE);)

    if (!registry->_is_initialized)
    {
        return RTI_TRUE;
    }

    dbrc = DB_Database_delete_table(
                            registry->property.db,registry->factory_table);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        RT_LOG_REGISTRY_FINALIZE(OSAPI_LOGKIND_ERROR,dbrc)
    }
#endif

    registry->_is_initialized = RTI_FALSE;

    return (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
}
#endif /* !RTI_CERT */

RT_Registry_T*
RT_Registry_get_instance(void)
{
    return &RT_ComponentFactory_fv_Instance;
}

RTI_BOOL
RT_Registry_register(RT_Registry_T *registry,
                     const char *name,
                     struct RT_ComponentFactoryI *intf,
                     struct RT_ComponentFactoryProperty *property,
                     struct RT_ComponentFactoryListener *listener)
{
    struct RT_ComponentFactory *reg_entry = NULL;
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T  dbrc;

    OSAPI_PRECONDITION_ALWAYS((registry==NULL) || (name==NULL) || (intf==NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    if (!registry->_is_initialized)
    {
        if (!RT_Registry_initialize(registry))
        {
            return RTI_FALSE;
        }
    }

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(registry->factory_table,
                DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&reg_entry,(DB_Key_T)&id);
    if (dbrc == DB_RETCODE_OK)
    {
        RT_LOG_REGISTRY_EXISTS(OSAPI_LOGKIND_WARNING,name)
        return RTI_FALSE;
    }

    dbrc = DB_Table_create_record(registry->factory_table,(DB_Record_T*)&reg_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        RT_LOG_REGISTRY_REGISTER(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    if (property != NULL)
    {
        property->registry = registry;
    }

    reg_entry->_factory = intf->initialize(property, listener);
    if (reg_entry->_factory == NULL)
    {
        RT_LOG_REGISTRY_INIT_FACTORY(OSAPI_LOGKIND_ERROR)
        /* Don't care if it fails */
        (void)DB_Table_delete_record(registry->factory_table,reg_entry);
        return RTI_FALSE;
    }

    reg_entry->_id._value = id._value;
    reg_entry->intf = intf;
    reg_entry->_factory->_id._value = id._value;

    dbrc = DB_Table_insert_record(registry->factory_table,(DB_Record_T)reg_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        RT_LOG_REGISTRY_REGISTER(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(registry->factory_table,
                                     (DB_Record_T)reg_entry);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
RT_Registry_unregister(RT_Registry_T *registry,
                       const char *name,
                       struct RT_ComponentFactoryProperty **property,
                       struct RT_ComponentFactoryListener **listener)
{
    union RT_ComponentFactoryId id;
    struct RT_ComponentFactory *reg_entry;
    DB_ReturnCode_T  dbrc;

    OSAPI_PRECONDITION_ALWAYS((registry==NULL) || (name==NULL),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    if (!registry->_is_initialized)
    {
        RT_LOG_REGISTRY_NOT_INITIALIZED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(registry->factory_table,
             DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&reg_entry,(DB_Key_T)&id);
    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (reg_entry->_factory->intf->finalize)
    {
        reg_entry->_factory->intf->finalize(reg_entry->_factory,property,listener);
    }

    dbrc = DB_Table_delete_record(registry->factory_table,(DB_Key_T)reg_entry);

    return ((dbrc == DB_RETCODE_OK) ? RTI_TRUE : RTI_FALSE);
}

struct RT_ComponentFactory*
RT_Registry_lookup(RT_Registry_T *registry,const char *name)
{
    return RT_Registry_lookup_cid(registry,name,RT_COMPONENT_CLASS_UNKNOWN);
}

struct RT_ComponentFactory*
RT_Registry_lookup_cid(RT_Registry_T *registry,const char *name,RTI_INT32 cid)
{
    union RT_ComponentFactoryId id;
    struct RT_ComponentFactory *reg_entry;
    DB_ReturnCode_T  dbrc;
    RTI_INT32 fcid;

    OSAPI_PRECONDITION((registry==NULL) || (name==NULL),
                            return NULL,
                            OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    if (!registry->_is_initialized)
    {
        RT_LOG_REGISTRY_NOT_INITIALIZED(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return NULL;
    }

    dbrc = DB_Table_select_match(registry->factory_table,
                DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&reg_entry,(DB_Key_T)&id);
    if (dbrc != DB_RETCODE_OK)
    {
        return NULL;
    }

    fcid = RT_ComponentFactory_get_id(reg_entry->_factory);

    if ((cid != RT_COMPONENT_CLASS_UNKNOWN)
        && (RT_INTERFACE_CLASS(fcid) != cid))
    {
        RT_LOG_REGISTRY_INCONSISTENT_CID(OSAPI_LOGKIND_WARNING,name,cid,fcid)
        return NULL;
    }

    return reg_entry->_factory;
}

RTI_INT32
RT_ComponentFactory_get_id(struct RT_ComponentFactory *factory)
{
    OSAPI_PRECONDITION(factory == NULL,
                            return -1,
                            OSAPI_Log_entry_add_pointer("factory",factory,RTI_TRUE);)

    return factory->intf->id;
}

const char*
RT_ComponentFactoryId_get_name(const RT_ComponentFactoryId_T *const id)
{
    OSAPI_PRECONDITION(id == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("id",id,RTI_TRUE);)

    return id->_name._name;
}

RTI_BOOL
RT_ComponentFactoryId_set_name(RT_ComponentFactoryId_T *id,
                               const char *const name)
{
    RTI_SIZE_T len;

    OSAPI_PRECONDITION_ALWAYS((id == NULL) || (name == NULL),
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("id",id,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    len = OSAPI_String_length(name);
    if (len > RT_MAX_FACTORY_NAME)
    {
        RT_LOG_REGISTRY_NAME_TOO_LONG(OSAPI_LOGKIND_ERROR,name,len)
        return RTI_FALSE;
    }

    RT_ComponentFactoryId_clear(id);

    OSAPI_Memory_copy(id->_name._name,name,len+1);

    return RTI_TRUE;
}

RTI_BOOL
RT_ComponentFactoryId_equals(const RT_ComponentFactoryId_T *const id,
                             const char *const name)
{
    return REDA_String_ncompare(
                id->_name._name,name,
                RT_MAX_FACTORY_NAME) ? RTI_FALSE : RTI_TRUE;
}

RTI_INT32
RT_ComponentFactoryId_compare(const RT_ComponentFactoryId_T *const id1,
                              const RT_ComponentFactoryId_T *const id2)
{
    return REDA_String_ncompare(
                id1->_name._name,id2->_name._name,RT_MAX_FACTORY_NAME);
}

void
RT_ComponentFactoryId_clear(union RT_ComponentFactoryId *id)
{
    id->_value._high = 0;
    id->_value._low = 0;
}
/*ci @} */

