/*
 * FILE: NETIO_ZeroCopy.c - Zero Copy transport API
 *
 * (c) Copyright 2022-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */


#include "rt/rt_rt.h"
#include "wh_sm/wh_sm_history.h"
#include "netio_zcopy/netio_zcopy.h"
#include "netio_zcopy/netio_zcopy_log.h"
#include "SharedQWriterHistory.h"


/*** SOURCE_BEGIN ***/

/*e \dref_NDDS_Transport_ZeroCopy_initialize
 */
RTI_BOOL
NDDS_Transport_ZeroCopy_initialize(
    RT_Registry_T *registry,
    const char *transport_name,
    struct ZCOPY_NotifInterfaceFactoryProperty *property)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL retval;
    struct WHSQ_HistoryFactoryProperty whsq_prop =
            WHSQ_HistoryFactoryProperty_INITIALIZER;

    /* It is ok for the transport_name and property to be NULL.
     * If transport name is null We use the default transport name.
     * If property is NULL the transport is not registered.
     */

    OSAPI_PRECONDITION_ALWAYS(
            (NULL == registry),
            goto done,
            OSAPI_Log_entry_add_pointer("registry", registry, RTI_TRUE);)

    if (transport_name == NULL)
    {
        transport_name = NETIO_DEFAULT_NOTIF_NAME;
    }

    /* Unregister the and re-initialize existing WH. This might fail as the
     * user might have NOT registered the Writer History. It is not considered
     * an error.
     */
    retval = RT_Registry_unregister(
            registry,
            DDSHST_WRITER_DEFAULT_HISTORY_NAME,
            NULL,
            NULL);
    IGNORE_RETVAL(retval);

    if (!RT_Registry_register(
                registry,
                DDSHST_WRITER_WRAPPED_HISTORY_NAME,
                WHSM_HistoryFactory_get_interface(),
                NULL,
                NULL))
    {
        ZCOPY_LOG_WH_REGISTER_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    whsq_prop.dds_wh_factory_name = DDSHST_WRITER_WRAPPED_HISTORY_NAME;

    /* Register the new WH, wrapping the old one */
    if (!RT_Registry_register(
                registry,
                DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                WHSQ_HistoryFactory_get_interface(),
                &whsq_prop._parent._parent,
                NULL))
    {
        ZCOPY_LOG_WH_REGISTER_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* The assumption is that if the user has provided a property we will
     * register the transport for them. However if the property is NULL,
     * they can register the Notification Interface manually.
     */
    if (property != NULL)
    {
        if (!ZCOPY_NotifInterfaceFactory_register(registry, transport_name, property))
        {
            return RTI_FALSE;
        }
    }

    result = RTI_TRUE;

done:
    return result;
}

/*e \dref_NDDS_Transport_ZeroCopy_finalize
 */
RTI_BOOL
NDDS_Transport_ZeroCopy_finalize(
    RT_Registry_T *registry,
    const char *transport_name)
{
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL rtn = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS((NULL == registry),
            goto done,
            OSAPI_Log_entry_add_pointer("registry", registry, RTI_TRUE);)

    if (!RT_Registry_unregister(
                registry,
                DDSHST_WRITER_WRAPPED_HISTORY_NAME,
                NULL,
                NULL))
    {
        ZCOPY_LOG_WH_REGISTER_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* Unregister the WH. This might fail as the user might
     * have unregistered the Writer History. It is not considered
     * an error.
     */
    rtn = RT_Registry_unregister(
                registry,
                DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                NULL,
                NULL);
    UNUSED_ARG(rtn);

    if (transport_name != NULL)
    {
        if (!ZCOPY_NotifInterfaceFactory_unregister(registry, transport_name))
        {
            goto done;
        }
    }

    result = RTI_TRUE;

done:
    return result;
}
