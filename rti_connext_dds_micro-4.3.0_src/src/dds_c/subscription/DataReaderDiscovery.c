/*
 * FILE: DataReaderDiscovery.c - DDS DataReader Discovery implementation
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
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 20jul2015,tk MICRO-1444/PR#15488 Unbind in case bind_external fails (RTPS)
 * 29jun2015,tk MICRO-1356/PR#15146 Return FALSE, not ERROR in add_route()
 * 10jun2015,tk MICRO-1298/PR#14969 Removed redundant code
 * 15may2015,tk MICRO-1220/PR#14766 Initialize retval to FALSE in
 *                                  add_anonymous_route
 * 15may2015,tk MICRO-1219/PR#14765 Removed invalid comment in
 *                                  DDS_DataReader_add_anonymous_route
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 26jan2015,tk MICRO-1028/PR#13473 Removed magic number 0xc0
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 30sep2013,tk MICRO-704: Unbind RTPS from DDS when a route is deleted
 * 18may2012,tk Major rewrite
 * 26aug2011,yy Fixed returning errors
 * 30apr2008,tk Written
 */
/*ce
 * \file
 * \brief DDS DataReader Discovery implementation
 *
 * \details
 * This file implements functionality to manage DDS discovery functionality
 * related to a DDS datareader. This includes matching/unmatching with
 * local and remote datawriters and establishing communication with datawriters.
 */
/*ci
 * \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "QosPolicy.h"
#include "Entity.h"
#include "Locator.h"
#include "Transport.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Conditions.h"
#include "DataReaderImpl.h"
#include "DataReaderEvent.h"
#include "DataReaderQos.h"
#include "DataReaderInterface.h"
#include "DataReaderDiscovery.h"
#include "PartitionQosPolicy.h"

#ifdef DDS_LIVELINESS_CHANNEL_ENABLED
#include "DataWriterImpl.h"
#include "DomainParticipant.h"
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

/*** SOURCE_BEGIN ***/

RTI_PRIVATE DDS_DataRepresentationId_t
DDS_DataReader_resolve_data_representation(struct DDS_DataReaderImpl *datareader,
                   const struct DDS_DataRepresentationQosPolicy *policy)
{
    RTI_INT32 seq_length;
    RTI_INT32 seq_index;
    DDS_DataRepresentationId_t dr_id;

    if (DDS_DataRepresentationIdSeq_get_length(&policy->value) == 0)
    {
        if (DDS_TypePlugin_is_representation_enabled(datareader->type_plugin,
                                                 DDS_XCDR_DATA_REPRESENTATION))
        {
            return DDS_XCDR_DATA_REPRESENTATION;
        }
        else
        {
            return DDS_INVALID_DATA_REPRESENTATION;
        }
    }
    else
    {
        seq_length = DDS_DataRepresentationIdSeq_get_length(&policy->value);

        for (seq_index = 0; seq_index < seq_length; ++seq_index)
        {
            dr_id = *DDS_DataRepresentationIdSeq_get_reference(
                                                    &policy->value,seq_index);

            if (DDS_TypePlugin_is_representation_enabled(datareader->type_plugin,dr_id))
            {
                return dr_id;
            }
        }
    }

    return DDS_INVALID_DATA_REPRESENTATION;
}

/*ci
 * \brief Check if a datareader is compatible with a datawriter qos
 *
 * \param[in] datareader Datareader to test
 * \param[in] dw_qos     The datawriter Qos to match against
 *
 * \return DDS_BOOLEAN_TRUE if it is compatible, DDS_BOOLEAN false otherwise
 */
DDS_Boolean
DDS_DataReader_writer_is_compatible(DDS_DataReader *datareader,
                                    const struct DDS_DataWriterQos *dw_qos)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)datareader;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
    DDS_DataRepresentationId_t data_id;

    if (!DDS_DeadlineQosPolicy_is_compatible(&dr->deadline,
                                             &dw_qos->deadline))
    {
        dr->last_policy_id = DDS_DEADLINE_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_compatible(&dr->ownership,
                                              &dw_qos->ownership))
    {
        dr->last_policy_id = DDS_OWNERSHIP_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_compatible(&dr->reliability,
                                                &dw_qos->reliability))
    {
        dr->last_policy_id = DDS_RELIABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_compatible(&dr->liveliness,
                                               &dw_qos->liveliness))
    {
        dr->last_policy_id = DDS_LIVELINESS_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_compatible(&dr->durability,
                                               &dw_qos->durability))
    {
        dr->last_policy_id = DDS_DURABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_compatible(&dr->destination_order,
                                                     &dw_qos->destination_order))
    {
        dr->req_incompatible_qos_status.last_policy_id = DDS_DESTINATIONORDER_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DataRepresentationQosPolicy_is_compatible(dr->representation,
                                                       &dw_qos->representation,
                                                       &data_id))
    {
        /* The user specified representation may not match the actual
         * representations used by the reader.
         */
        if (DDS_DataReader_resolve_data_representation(dr,
                   &dw_qos->representation) == DDS_INVALID_DATA_REPRESENTATION)
        {
            dr->req_incompatible_qos_status.last_policy_id = DDS_DATA_REPRESENTATION_QOS_POLICY_ID;
            retval = DDS_BOOLEAN_FALSE;
        }
    }

    if (!DDS_LatencyBudgetQosPolicy_is_compatible(&dr->latency_budget,
                                                  &dw_qos->latency_budget))
    {
        dr->req_incompatible_qos_status.last_policy_id = DDS_LATENCYBUDGET_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    return retval;
}


DDS_Boolean
DDS_DataReader_add_anonymous_route(DDS_DataReader *self,
                                   struct NETIO_Address *src_writer,
                                   struct NETIO_Address *from_address)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    NETIO_Interface_T *ext_intf = NULL;
    NETIO_RouteKind_T bind_kind;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct NETIO_Address dst_writer = NETIO_Address_INITIALIZER;
#if OSAPI_ENABLE_TRACE
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(self));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif

    if (DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        bind_kind = NETIO_ROUTEKIND_META;
    }
    else
    {
        bind_kind = NETIO_ROUTEKIND_USER;
    }


    OSAPI_TRACE_DDS("datareader adding anon route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)

    if (DB_Database_lock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!NETIO_RouteResolver_add_route(dr->config->route_resolver,
                                       (NETIO_Interface_T*)dr->rtps_intf,
                                       src_writer,from_address,NULL,NULL,NULL))
    {
        goto done;
    }

    if (!NETIO_Interface_bind(dr->dr_intf,src_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_NETIO_KIND,
                                          DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_bind(dr->rtps_intf,src_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,DDSC_LOG_RTPS_NETIO_KIND,
                                          DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dr->dr_intf,src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_bind_external(dr->rtps_intf,src_writer,ext_intf,
                                       &dst_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_BIND_EXTERNAL(OSAPI_LOGKIND_ERROR,
                                     DDSC_LOG_RTPS_NETIO_KIND,
                                     DDSC_LOG_DATAREADER_NETIO_KIND)

        if (!NETIO_Interface_unbind(dr->rtps_intf,src_writer,NULL,NULL))
        {
            DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_RTPS_NETIO_KIND,
                                  DDSC_LOG_RTPS_NETIO_KIND)
        }
        goto done;
    }

    /* Retrieve the address and interface the downstream interface should
     * forward packets to.
     */
    if (!NETIO_Interface_get_external_interface(dr->rtps_intf,src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_BindResolver_bind(dr->config->bind_resolver,
                         (struct REDA_StringSeq*)dr->config->enabled_transports,
                         bind_kind,from_address,ext_intf,&dst_writer,NULL,NULL))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datareader failed to add anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)
    }
    else
    {
        OSAPI_TRACE_DDS("datareader added anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)
    }
#endif

    if (DB_Database_unlock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_DataReader_delete_anonymous_route(DDS_DataReader *self,
                                      struct NETIO_Address *src_writer,
                                      struct NETIO_Address *from_address)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    NETIO_Interface_T *ext_intf = NULL;
    NETIO_RouteKind_T bind_kind;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct NETIO_Address dst_writer = NETIO_Address_INITIALIZER;
#if OSAPI_ENABLE_TRACE
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(self));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif

    if (DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        bind_kind = NETIO_ROUTEKIND_META;
    }
    else
    {
        bind_kind = NETIO_ROUTEKIND_USER;
    }

    OSAPI_TRACE_DDS("datareader deleting to add anon route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)

    if (DB_Database_lock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!NETIO_RouteResolver_delete_route(dr->config->route_resolver,
                                          (NETIO_Interface_T*)dr->rtps_intf,
                                          src_writer,from_address,NULL,NULL))
    {
        goto done;
    }

    if (!NETIO_Interface_unbind(dr->dr_intf,src_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_DATAREADER_NETIO_KIND,
                              DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_unbind(dr->rtps_intf,src_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dr->rtps_intf,src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
            (struct REDA_StringSeq*)dr->config->enabled_transports,
            bind_kind,from_address,ext_intf,&dst_writer,NULL,NULL))
    {
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dr->dr_intf,src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_unbind_external(dr->rtps_intf,src_writer,ext_intf,
                                         &dst_writer,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_TRACE
    if (retval)
    {
        OSAPI_TRACE_DDS("datareader deleted anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)
    }
    else
    {
        OSAPI_TRACE_DDS("datareader failed to delete anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)
    }
#endif /* !RTI_CERT */

    if (DB_Database_unlock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Add a route from a datareader to a datawriter from a locator sequence
 *
 * \details
 *
 * This is a helper function to add routes to a datawriter for a datareader
 * using a route resolver to find an interface which can route to the datawriter
 * via the locators
 *
 * \param[in]  dr          Datareader
 * \param[in]  locator_seq Sequence of routes to add
 * \param[in]  src_writer  The address of the destination writer
 *
 * \return DDS_BOOLEAN_TRUE if it is enabled, DDS_BOOLEAN false otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataReader_add_route_from_locator(struct DDS_DataReaderImpl *dr,
                                      const struct DDS_LocatorSeq *locator_seq,
                                      struct RTPS_RouteProperty *route_property,
                                      struct NETIO_Address *src_writer)
{
    struct DDS_Locator *a_locator;
    DDS_Long i,j,k;
    struct NETIO_Address from_address;

    j = DDS_LocatorSeq_get_length(locator_seq);

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator_seq,i);
        from_address = *((struct NETIO_Address*)a_locator);

        if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
        {
            if (!NETIO_RouteResolver_add_route(dr->config->route_resolver,
                    dr->rtps_intf,src_writer,&from_address,&route_property->_parent,NULL,NULL))
            {
                break;
            }
        }
    }

    /* If the addition failed, unwind and the let the caller know. The
     * caller should cancel the entire current operation.
     */
    if (i < j)
    {
        for (k = 0; k < i; k++)
        {
            a_locator = DDS_LocatorSeq_get_reference(locator_seq,k);
            from_address = *((struct NETIO_Address*)a_locator);

            if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
            {
                if (!NETIO_RouteResolver_delete_route(dr->config->route_resolver,
                                                    dr->rtps_intf,src_writer,
                                                    &from_address,NULL,NULL))
                {
                    DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                                DDSC_LOG_DATAREADER_NETIO_KIND,
                                                DDSC_LOG_DATAWRITER_NETIO_KIND);
                }
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Add a datareader as a listener to a datawriter from a locator sequence
 *
 * \details
 *
 * This is a helper function to add a datareader as a listener to data from
 * a datawriter using a bind resolver to find an interface which can
 * listen to the datawriter via the locators
 *
 * \param[in] dr             Datareader
 * \param[in] locator_seq    Sequence of addresses to listen to
 * \param[in] bind_kind      The type of bind
 * \param[in] ext_intf       The interface to bind to
 * \param[in] src_writer     The address of the datawriter to listen to
 *
 * \return DDS_BOOLEAN_TRUE if it is enabled, DDS_BOOLEAN false otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataReader_bind_from_locator(struct DDS_DataReaderImpl *dr,
                                 const struct DDS_LocatorSeq *locator_seq,
                                 NETIO_RouteKind_T bind_kind,
                                 NETIO_Interface_T *ext_intf,
                                 struct NETIO_Address *src_writer)
{
    DDS_Long i,j,k;
    struct NETIO_Address from_address  = NETIO_Address_INITIALIZER;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = *(struct NETIO_Address*)
                          DDS_LocatorSeq_get_reference(locator_seq,i);

        if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
        {
            if (!NETIO_BindResolver_bind(dr->config->bind_resolver,
                    (struct REDA_StringSeq*)dr->config->enabled_transports,
                    bind_kind,&from_address,ext_intf,src_writer,NULL,NULL))
            {
                break;
            }
        }
    }

    if (i < j)
    {
        for (k = 0; k < i; k++)
        {
            from_address = *(struct NETIO_Address*)
                              DDS_LocatorSeq_get_reference(locator_seq,k);

            if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
            {
                if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
                        (struct REDA_StringSeq*)dr->config->enabled_transports,
                        bind_kind,&from_address,ext_intf,src_writer,NULL,NULL))
                {
                    DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                        DDSC_LOG_DATAREADER_NETIO_KIND,
                                        DDSC_LOG_DATAWRITER_NETIO_KIND);
                }
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Helper function to bind to the notification interface
 *
 * \details
 *
 * Binds the Notification Interface and the Data Reader to the notif interface
 * if required. In case of a failure or notif interface is not selected it winds backs
 *
 * \param[in]  self                 Datareader
 * \param[in]  key                  The datawriter key
 * \param[in]  self_locator_seq     The unicast locators for the reader
 * \paraqm[in] reslvd_locator_seq   The locator sequence resolved.
 * \param[in]  bind_kind            THe kind of bind being added.
 * \param[out] use_notif            RTI_TRUE if a notif is to be used
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 */

RTI_PRIVATE RTI_BOOL
DDS_DataReader_bind_to_notif(DDS_DataReader *self,
                             const DDS_BuiltinTopicKey_t *key,
                             const struct DDS_LocatorSeq *self_locator_seq,
                             const struct DDS_LocatorSeq *reslvd_locator_seq,
                             NETIO_RouteKind_T bind_kind,
                             RTI_BOOL *use_notif)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_writer = NETIO_Address_INITIALIZER;
    NETIO_Interface_T *ext_intf = NULL;
    RTI_INT32 i = 0, j = 0, k = 0;
    struct NETIO_Address *from_address = NULL;
    RTI_BOOL retval = RTI_FALSE, brc = RTI_FALSE;

    *use_notif = RTI_FALSE;

    j = DDS_LocatorSeq_get_length(reslvd_locator_seq);
    if (j == 0)
    {
        retval = RTI_TRUE;
        goto done;
    }

    for (i = 0; i < j; i++)
    {
        from_address = (struct NETIO_Address *)
            DDS_LocatorSeq_get_reference(reslvd_locator_seq, i);

        if ((from_address != NULL) && (from_address->kind == NETIO_ADDRESS_KIND_NOTIF))
        {
            *use_notif = RTI_TRUE;
            break;
        }
    }
    if (!*use_notif)
    {
        retval = RTI_TRUE;
        goto done;
    }

    NETIO_Address_set_guid_from_key(&src_writer, 0, (struct NETIO_AddressInt32 *)key);
    NETIO_Address_set_kind(&src_writer, NETIO_ADDRESS_KIND_NOTIF, 0);

    if (!NETIO_Interface_get_external_interface(dr->dr_intf, NULL,
                                                &ext_intf, &dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    j = DDS_LocatorSeq_get_length(self_locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = (struct NETIO_Address *)
            DDS_LocatorSeq_get_reference(self_locator_seq, i);

        if ((from_address != NULL) && (from_address->kind == NETIO_ADDRESS_KIND_NOTIF))
        {
            if (!NETIO_BindResolver_bind(dr->config->bind_resolver,
                                         (struct REDA_StringSeq *)dr->config->enabled_transports,
                                         bind_kind,
                                         from_address,
                                         ext_intf,
                                         &src_writer,
                                         NULL,NULL))
            {
                break;
            }
        }
    }

    /*Unwind if there was a an issue with binds*/
    if (i < j)
    {
        for (k = 0; k < i; k++)
        {
            from_address = (struct NETIO_Address *)
                DDS_LocatorSeq_get_reference(self_locator_seq, k);
            if (from_address->kind == NETIO_ADDRESS_KIND_NOTIF)
            {
                brc = NETIO_BindResolver_unbind(dr->config->bind_resolver,
                        (struct REDA_StringSeq *)dr->config->enabled_transports,
                     bind_kind, from_address, ext_intf, &src_writer, NULL, NULL);
#if OSAPI_ENABLE_LOG
                if (!brc)
                {
                    DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                        DDSC_LOG_DATAREADER_NETIO_KIND,
                                        DDSC_LOG_DATAWRITER_NETIO_KIND)
                }
#else
                IGNORE_RETVAL(brc);

#endif
            }
        }

    }
    else
    {
        retval = RTI_TRUE;
    }
done:
    return retval;
}

/*ci
 * \brief Helper function to unbind from the notification interface
 *
 * \details
 *
 * Unbinds the Notification Interface and the Data Reader from the notif
 * interface if required.
 *
 * \param[in]  self            Datareader
 * \param[in]  key             The datawriter key
 * \param[in]  locator_seq     The unicast locators for the reader
 * \param[in]  bind_kind       Kind of the bind being removed.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */

RTI_PRIVATE RTI_BOOL
DDS_DataReader_unbind_from_notif(DDS_DataReader *self,
                                 const DDS_BuiltinTopicKey_t *key,
                                 const struct DDS_LocatorSeq *locator_seq,
                                 NETIO_RouteKind_T bind_kind,
                                 RTI_BOOL *notif_route_existed)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address from_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_writer = NETIO_Address_INITIALIZER;
    struct DDS_Locator *a_locator = NULL;
    NETIO_Interface_T *ext_intf = NULL;
    RTI_INT32 i = 0;
    RTI_INT32 j = 0;
    RTI_BOOL route_existed = RTI_FALSE;

    *notif_route_existed = RTI_FALSE;

    j = DDS_LocatorSeq_get_length(locator_seq);
    if (j == 0)
    {
        return RTI_TRUE;
    }

    NETIO_Address_set_guid_from_key(&src_writer, 0, (struct NETIO_AddressInt32 *)key);
    NETIO_Address_set_kind(&src_writer, NETIO_ADDRESS_KIND_NOTIF, 0);

    if (!NETIO_Interface_get_external_interface(dr->dr_intf, NULL,
            &ext_intf, &dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADER_NETIO_KIND)
        return RTI_FALSE;
    }

    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator_seq, i);
        if (a_locator != NULL)
        {
            DDS_Locator_to_netio_address(DDS_LocatorSeq_get_reference(locator_seq, i),
                                    &from_address);

            if (NETIO_Address_get_kind(&from_address) == NETIO_ADDRESS_KIND_NOTIF)
            {
                if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
                        (struct REDA_StringSeq *)dr->config->enabled_transports,
                        bind_kind, &from_address, ext_intf, &src_writer, &route_existed, NULL))
                {
                    return RTI_FALSE;
                }
                if (route_existed == RTI_TRUE)
                {
                    *notif_route_existed = RTI_TRUE;
                }
            }
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Add a route from a datareader to a datawriter
 *
 * \details
 *
 * Add unicast and multicast routes to a datawriter for a datareader using a
 * route resolver to find an interface which can route to the datawriter
 * via the unicast and multicast locators.
 *
 * \param[in]  self           Datareader
 * \param[in]  key            The datawriter key
 * \param[in]  qos            The datawriter qos
 * \param[in]  uc_locator     The unicast locators for the datawriter
 * \param[in]  mc_locator     The multicast locators for the datawriter
 * \param[out] route_existed  RTI_TRUE if a route to the writer already existed
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref DDS_DataReader_delete_route
 */
DDS_Boolean
DDS_DataReader_add_route(DDS_DataReader *self,
                         const DDS_BuiltinTopicKey_t *key,
                         const struct DDS_DataWriterQos *const qos,
                         const struct DDS_LocatorSeq *uc_locator,
                         const struct DDS_LocatorSeq *mc_locator,
                         RTI_BOOL *route_existed)
{
#define DR_OPERATION_NONE                 0
#define DR_OPERATION_BIND_DRINTF          0x1
#define DR_OPERATION_RESOLVER_BIND_ROUTE  0x2
#define DR_OPERATION_ADD_ROUTE_DRINTF     0x4
#define DR_OPERATION_BIND_EXTERNAL_RTPS   0x8
#define DR_OPERATION_ADD_ROUTE_UC         0x10
#define DR_OPERATION_ADD_ROUTE_MC         0x20
#define DR_OPERATION_BIND_RTPS            0x40
#define DR_OPERATION_BIND_UC              0x80
#define DR_OPERATION_BIND_MC              0x100

    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    struct NETIO_Address dst_addr = NETIO_Address_INITIALIZER;
    struct NETIO_Address src_writer  = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_writer  = NETIO_Address_INITIALIZER;
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dr_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTPS_RouteProperty route_property = RTPSRouteProperty_INITIALIZER;
    NETIO_Interface_T *ext_intf = NULL;
    RTI_BOOL local_r_existed = RTI_FALSE;
    const struct DDS_LocatorSeq *dr_uc_locator_seq;
    const struct DDS_LocatorSeq *dr_mc_locator_seq;
    NETIO_RouteKind_T bind_kind;
    struct NETIOBindProperty dr_b_prop = NETIOBindProperty_INITIALIZER;
    struct NETIOBindProperty *b_prop = NULL;
    RTI_BOOL found_route = RTI_FALSE;
    RTI_UINT32 ops = 0;

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(dr));
    DDS_BuiltinTopicKey_from_guid(&dr_key,&ih);

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dr_key) &&
            DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        *route_existed = RTI_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    if (DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        bind_kind = NETIO_ROUTEKIND_META;
        dr_uc_locator_seq = self->config->default_meta_unicast;
        dr_mc_locator_seq = self->config->default_meta_multicast;
    }
    else
    {
        bind_kind = NETIO_ROUTEKIND_USER;
        dr_uc_locator_seq = DDS_DataReader_get_unicast_locator_ref(self);
        if (DDS_LocatorSeq_get_length(dr_uc_locator_seq) == 0)
        {
            dr_uc_locator_seq = self->config->default_unicast;
        }
        dr_mc_locator_seq = DDS_DataReader_get_multicast_locator_ref(self);
        if (DDS_LocatorSeq_get_length(dr_mc_locator_seq) == 0)
        {
            dr_mc_locator_seq = self->config->default_multicast;
        }
    }

    NETIO_Address_set_guid_from_key(&src_writer,0,(struct NETIO_AddressInt32*)key);
    NETIO_Address_set_guid_from_key(&dst_addr,0,(struct NETIO_AddressInt32*)&dr_key);

    OSAPI_TRACE_DDS("datareader adding route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)

    if (DB_Database_lock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (qos->ownership.kind == DDS_EXCLUSIVE_OWNERSHIP_QOS)
    {
        dr_b_prop.strength = qos->ownership_strength.value;
    }

    dr_b_prop.lease_duration.sec = qos->liveliness.lease_duration.sec;
    dr_b_prop.lease_duration.nanosec = qos->liveliness.lease_duration.nanosec;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    dr_b_prop.liveliness_kind = qos->liveliness.kind;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    dr_b_prop.data_format = DDS_TypePlugin_resolve_encapsulation(dr->cdr_id,
                                                                 &qos->representation);

    /* Match with data-writer. If already matched return */
    if (!NETIO_Interface_bind(dr->dr_intf,&src_writer,&dr_b_prop,route_existed))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_DATAREADER_NETIO_KIND,
                            DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    ops |= DR_OPERATION_BIND_DRINTF;

    if (*route_existed)
    {
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    /* If the loop-back interface exists, bind the reader interface to it
     * so the reader can receive data directly from the writer. Note that
     * for loop-back there is no back-channel, thus we only bind to it.
     */
    if (DDS_BuiltinTopicKey_prefix_equals(key,&dr_key))
    {
        if (!NETIO_Interface_get_external_interface(
                dr->dr_intf,&src_writer,&ext_intf,&dst_writer))
        {
            DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                             DDSC_LOG_DATAREADER_NETIO_KIND)
            goto done;
        }

        /* NOTE: For the intra-transport we bind the reader directly to the
         * writer. Thus, the destination address is that of the reader
         * (dst_addr), not the modified writer source (dst_writer).
         */
        if (!NETIO_BindResolver_bind(dr->config->bind_resolver,
                (struct REDA_StringSeq*)dr->config->enabled_transports,
                bind_kind,
                &src_writer,ext_intf,&dst_addr,route_existed,&found_route))
        {
            goto done;
        }

        ops |= DR_OPERATION_RESOLVER_BIND_ROUTE;

        if (found_route)
        {
            /* If the loopback interface was registered and a route was found
             * to the writer, then nothing else to do.
             */
            retval = DDS_BOOLEAN_TRUE;
            goto done;
        }
        /* INTRA routes for writers are created first, if none exists then
         * continue with regular communication.
         */
    }

    /*now check for notif interface*/
    if (!DDS_DataReader_bind_to_notif(self, key,
                                      dr_uc_locator_seq,
                                      uc_locator,
                                      bind_kind,
                                      &found_route))
    {
        goto done;
    }

    if (found_route)
    {
        /* If the notif interface was registered and a route was found
         * to the writer, then nothing else to do.
         */
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    b_prop = NULL;
    /* coverity[var_deref_model : FALSE] */
    /* coverity[cert_exp34_c_violation : FALSE] */
    if (!NETIO_Interface_bind(dr->rtps_intf,&src_writer,b_prop,&local_r_existed))
    {
        if (!local_r_existed)
        {
            DDSC_LOG_NETIO_NO_ROUTE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    ops |= DR_OPERATION_BIND_RTPS;

    /* All other communication paths relies on RTPS. The data-writer
     * creates and established the RTPS interface upon creation, thus
     * it already exists. All we need to do is add a route DW->RTPS and
     * a route from RTPS-> requested DR interface.
     */
    if (!NETIO_Interface_get_external_interface(dr->dr_intf,&src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    /* Bind to RTPS interface */
    if (!NETIO_Interface_bind_external(dr->rtps_intf,&src_writer,ext_intf,
                                       &dst_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_BIND_EXTERNAL(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_RTPS_NETIO_KIND,
                            DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    ops |= DR_OPERATION_BIND_EXTERNAL_RTPS;

    route_property.reliable = (qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS ? RTI_TRUE : RTI_FALSE);

    if (!DDS_DataReader_add_route_from_locator(dr,uc_locator,&route_property,&src_writer))
    {
        goto done;
    }

    ops |= DR_OPERATION_ADD_ROUTE_UC;

    if (!DDS_DataReader_add_route_from_locator(dr,mc_locator,&route_property,&src_writer))
    {
        goto done;
    }

    ops |= DR_OPERATION_ADD_ROUTE_MC;

    if (!NETIO_Interface_get_external_interface(
                            dr->rtps_intf,&src_writer,&ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)

        goto done;
    }

    if (!DDS_DataReader_bind_from_locator(dr,
                            dr_uc_locator_seq,bind_kind,ext_intf,&dst_writer))
    {
        goto done;
    }

    ops |= DR_OPERATION_BIND_UC;

    if (!DDS_DataReader_bind_from_locator(dr,
                            dr_mc_locator_seq,bind_kind,ext_intf,&dst_writer))
    {
        goto done;
    }

    ops |= DR_OPERATION_BIND_MC;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    {
        struct DDS_DomainParticipantImpl *participant =
                    (struct DDS_DomainParticipantImpl*)
                              DDS_Subscriber_get_participant(self->subscriber);
        if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
            !DDS_IpcLiveliness_add_remote_writer(participant->ipc_liveliness,
                                                 self->as_entity.entity_id,key,
                                                 &qos->liveliness))
        {
            goto done;
        }
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval)
    {
        /* Unwind previous operations in case of failure.
         *
         */
        if (ops & DR_OPERATION_RESOLVER_BIND_ROUTE)
        {
            if (NETIO_Interface_get_external_interface(dr->dr_intf,&src_writer,
                                                       &ext_intf,&dst_addr))
            {
                if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
                                               (struct REDA_StringSeq*)dr->config->enabled_transports,
                                               bind_kind,&src_writer,ext_intf,&dst_addr,route_existed,
                                               &found_route))
                {
                    DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_DATAREADER_NETIO_KIND,
                                          DDSC_LOG_DATAWRITER_NETIO_KIND);
                }
            }
        }

        if (ops & DR_OPERATION_BIND_DRINTF)
        {
            if (!NETIO_Interface_unbind(dr->dr_intf,
                                        &src_writer,NULL,route_existed))
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAREADER_NETIO_KIND,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND);
            }
        }

        /* The following operations do not require any action as these are
         * already cleaned up in case of failure.
         * DR_OPERATION_ADD_ROUTE_UC
         * DR_OPERATION_ADD_ROUTE_MC
         * DR_OPERATION_BIND_UC
         * DR_OPERATION_BIND_MC
         */
        if (ops & DR_OPERATION_BIND_RTPS)
        {
            if (!NETIO_Interface_unbind(dr->rtps_intf,&src_writer,NULL,NULL))
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_RTPS_NETIO_KIND,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND);
            }
        }

        if (NETIO_Interface_get_external_interface(dr->dr_intf,&src_writer,
                                                   &ext_intf,&dst_writer))
        {
            if (ops & DR_OPERATION_BIND_EXTERNAL_RTPS)
            {
                if (!NETIO_Interface_unbind_external(dr->rtps_intf,
                                        &src_writer,ext_intf,&dst_writer,NULL))
                {
                    DDSC_LOG_NETIO_UNBIND_EXTERNAL(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_RTPS_NETIO_KIND,
                                          DDSC_LOG_DATAWRITER_NETIO_KIND);
                }
            }
        }

        *route_existed = RTI_FALSE;
    }

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datareader failed to add route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dr->topic)),
                                 dr_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else if (!*route_existed)
    {
       OSAPI_TRACE_DDS("datareader added route:",RTI_FALSE)
       OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
          DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
       OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
       OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;

#undef DR_OPERATION_NONE
#undef DR_OPERATION_BIND_DRINTF
#undef DR_OPERATION_RESOLVER_BIND_ROUTE
#undef DR_OPERATION_ADD_ROUTE_DRINTF
#undef DR_OPERATION_BIND_EXTERNAL_RTPS
#undef DR_OPERATION_ADD_ROUTE_UC
#undef DR_OPERATION_ADD_ROUTE_MC
#undef DR_OPERATION_BIND_RTPS
#undef DR_OPERATION_BIND_UC
#undef DR_OPERATION_BIND_MC
}

/*ci
 * \brief Remove a datareader as a listener to a datawriter
 *
 * \details
 *
 * This is a helper function to remove a datareader as a listener to data from
 * a datawriter using a bind resolver to find an interface which can
 * listen to the datawriter via the locators
 *
 * \param[in] dr             Datareader
 * \param[in] locator_seq    Sequence of routes to remove
 * \param[in] recv_kind      The type of route
 * \param[in] ext_intf       The interface to unbind from
 * \param[in] src_writer     The address of the datawriter
 *
 * \return DDS_BOOLEAN_TRUE if successful, DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataReader_unbind_from_locator(struct DDS_DataReaderImpl *dr,
                                   const struct DDS_LocatorSeq *locator_seq,
                                   NETIO_RouteKind_T recv_kind,
                                   NETIO_Interface_T *ext_intf,
                                   struct NETIO_Address *src_writer)
{
    DDS_Long i,j;
    struct NETIO_Address from_address;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = *(struct NETIO_Address*)
                          DDS_LocatorSeq_get_reference(locator_seq,i);

        if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
                (struct REDA_StringSeq*)dr->config->enabled_transports,
                recv_kind,&from_address,ext_intf,src_writer,NULL,NULL))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Delete a route from a datareader to a datawriter based on a locator
 *
 * \param[in] dr           Datareader
 * \param[in] locator_seq  The locators  key
 * \param[in] recv_kind    The type of route
 * \param[in] src_writer   The address of the datawriter
 *
 * \return DDS_BOOLEAN_TRUE if successful, DDS_BOOLEAN_FALSE otherwise
 *
 * \sa \ref DDS_DataReader_delete_route
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataReader_delete_route_from_locator(struct DDS_DataReaderImpl *dr,
                                   const struct DDS_LocatorSeq *locator_seq,
                                   NETIO_RouteKind_T recv_kind,
                                   struct NETIO_Address *src_writer)
{
    DDS_Long i,j;
    struct NETIO_Address from_address;
    struct DDS_Locator *a_locator;
    UNUSED_ARG(recv_kind);

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator_seq,i);
        if (a_locator != NULL)
        {
            DDS_Locator_to_netio_address(a_locator,&from_address);
            if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
            {
                if (!NETIO_RouteResolver_delete_route(dr->config->route_resolver,
                                                    dr->rtps_intf,src_writer,
                                                    &from_address,NULL,NULL))
                {
                    return DDS_BOOLEAN_FALSE;
                }
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Delete a route from a datareader to a datawriter
 *
 * \details
 *
 * Delete unicast and multicast routes to a datawriter from a datareader.
 *
 * \param[in]  self           Datareader
 * \param[in]  key            The datawriter key
 * \param[in]  uc_locator     The unicast locators to the datawriter
 * \param[in]  mc_locator     The multicast locators to the datawriter
 * \param[out] route_existed  RTI_TRUE if a route to the reader existed
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref DDS_DataReader_delete_route
 */
DDS_Boolean
DDS_DataReader_delete_route(DDS_DataReader *self,
                            const DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator,
                            RTI_BOOL *route_existed)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    struct NETIO_Address dst_addr = NETIO_Address_INITIALIZER;
    struct NETIO_Address src_writer = NETIO_Address_INITIALIZER;
    struct NETIO_Address dst_writer = NETIO_Address_INITIALIZER;
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dr_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    NETIO_Interface_T *ext_intf = NULL;
    NETIO_RouteKind_T bind_kind;
    const struct DDS_LocatorSeq *dr_uc_locator_seq;
    const struct DDS_LocatorSeq *dr_mc_locator_seq;
    RTI_BOOL found_route;
    RTI_BOOL notif_route_existed  = RTI_FALSE;

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(dr));
    DDS_BuiltinTopicKey_from_guid(&dr_key,&ih);

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dr_key) &&
            DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        *route_existed = RTI_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    if (DDS_ObjectId_is_builtin(self->as_entity.entity_id))
    {
        bind_kind = NETIO_ROUTEKIND_META;
        dr_uc_locator_seq = self->config->default_meta_unicast;
        dr_mc_locator_seq = self->config->default_meta_multicast;
    }
    else
    {
        bind_kind = NETIO_ROUTEKIND_USER;
        dr_uc_locator_seq = DDS_DataReader_get_unicast_locator_ref(self);
        if (DDS_LocatorSeq_get_length(dr_uc_locator_seq) == 0)
        {
            dr_uc_locator_seq = self->config->default_unicast;
        }
        dr_mc_locator_seq = DDS_DataReader_get_multicast_locator_ref(self);
        if (DDS_LocatorSeq_get_length(dr_mc_locator_seq) == 0)
        {
            dr_mc_locator_seq = self->config->default_multicast;
        }
    }

    NETIO_Address_set_guid_from_key(&src_writer,0,(struct NETIO_AddressInt32*)key);
    NETIO_Address_set_guid_from_key(&dst_addr,0,(struct NETIO_AddressInt32*)&dr_key);

    OSAPI_TRACE_DDS("datareader deleting route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)

    /* If the loop-back interface exists, bind the reader interface to it
     * so the reader can receive data directly from the writer. Note that
     * for loop-back there is no back-channel, thus we only bind to it.
     */
    if (DB_Database_lock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dr_key))
    {
        if (!NETIO_Interface_get_external_interface(dr->dr_intf,&src_writer,
                                                    &ext_intf,&dst_addr))
        {
            DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                             DDSC_LOG_DATAREADER_NETIO_KIND)
            goto done;
        }

        if (!NETIO_BindResolver_unbind(dr->config->bind_resolver,
                (struct REDA_StringSeq*)dr->config->enabled_transports,
                bind_kind,&src_writer,ext_intf,&dst_addr,route_existed,
                &found_route))
        {
            goto done;
        }

        if (found_route)
        {
            if (!NETIO_Interface_unbind(dr->dr_intf,
                    &src_writer,dr->dr_intf,route_existed))
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAREADER_NETIO_KIND,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND)
                goto done;
            }

            retval = DDS_BOOLEAN_TRUE;
            goto done;
        }
    }

    if (!DDS_DataReader_unbind_from_notif(self, key, dr_uc_locator_seq, bind_kind, &notif_route_existed))
    {
        goto done;
    }

    if (!NETIO_Interface_unbind(dr->dr_intf,&src_writer,NULL,route_existed))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_DATAREADER_NETIO_KIND,
                              DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    /* if a notif route was found all the routes have been deleted*/
    if (!*route_existed || notif_route_existed)
    {
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dr->rtps_intf,&src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!DDS_DataReader_unbind_from_locator(dr,
                        dr_uc_locator_seq,bind_kind,ext_intf,&dst_writer))
    {
        goto done;
    }

    if (!DDS_DataReader_unbind_from_locator(dr,
                        dr_mc_locator_seq,bind_kind,ext_intf,&dst_writer))
    {
        goto done;
    }

    if (!NETIO_Interface_unbind(dr->rtps_intf,&src_writer,NULL,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dr->dr_intf,&src_writer,
                                                &ext_intf,&dst_writer))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAREADER_NETIO_KIND)

        goto done;
    }

    if (!NETIO_Interface_unbind_external(dr->rtps_intf,&src_writer,ext_intf,
                                         &dst_writer,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!DDS_DataReader_delete_route_from_locator(dr,uc_locator,bind_kind,
                                                  &src_writer))
    {
        goto done;
    }

    if (!DDS_DataReader_delete_route_from_locator(dr,mc_locator,bind_kind,
                                                  &src_writer))
    {
        goto done;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    {
        struct DDS_DomainParticipantImpl *participant =
                    (struct DDS_DomainParticipantImpl*)
                          DDS_Subscriber_get_participant(self->subscriber);

        if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
            !DDS_IpcLiveliness_remove_remote_writer(participant->ipc_liveliness,
                                                    self->as_entity.entity_id,
                                                    key))
        {
            goto done;
        }
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datareader failed to delete route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)

        DDSC_LOG_NETIO_DELETE_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dr->topic)),
                                 dr_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else if (*route_existed)
    {
        OSAPI_TRACE_DDS("datareader deleted route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dr->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dr_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(dr->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

/*ci
 * \brief Match a datareader with a local datawriter
 *
 * \param[in] self       The datareader to match with a datawriter
 * \param[in] key        The key of the datawriter to match with the datareader
 * \param[in] qos        The datareader qos
 * \param[in] uc_locator Sequence of unicast addresses to the datawriter
 * \param[in] mc_locator Sequence of multicast addresses to the datawriter
 * \param[in] parent_data
 * \param[in] data
 *
 * \sa \ref DDS_DataReader_unmatch_writer
 */
void
DDS_DataReader_match_writer(DDS_DataReader *self,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_DataWriterQos *const qos,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator)
{
    RTI_BOOL route_existed = RTI_FALSE;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    if (!DDS_DataReader_writer_is_compatible(self,qos))
    {
        retval = DDS_DataReader_delete_route(
                    self,key,uc_locator,mc_locator,&route_existed);
        if (retval)
        {
            if (route_existed)
            {
                DDS_DataReaderEvent_on_subscription_matched(
                        self,key,DDS_BOOLEAN_TRUE,DDS_BOOLEAN_FALSE);
            }
            DDS_DataReaderEvent_on_incompatible_qos(self,key);
        }

        return;
    }



    retval = DDS_DataReader_add_route(
                self,key,qos,uc_locator,mc_locator,&route_existed);
    if (!route_existed && retval)
    {
        DDS_DataReaderEvent_on_subscription_matched(
                self, key, DDS_BOOLEAN_FALSE, DDS_BOOLEAN_TRUE);
    }
}

/*ci
 * \brief Unmatch a datareader from a datawriter
 *
 * \param[in] datareader The datareader to unmatch from a datawriter
 * \param[in] key        The key of the writer to unmatch from the datareader
 * \param[in] uc_locator Sequence of unicast addresses to the datawriter
 * \param[in] mc_locator Sequence of multicast addresses to the datawriter
 *
 * \sa \ref DDS_DataReader_match_writer
 */
void
DDS_DataReader_unmatch_writer(DDS_DataReader *datareader,
                              const struct DDS_BuiltinTopicKey_t *key,
                              const struct DDS_LocatorSeq *uc_locator,
                              const struct DDS_LocatorSeq *mc_locator)
{
    DDS_Boolean retval;
    RTI_BOOL route_existed = RTI_FALSE;

    if (DDS_DataReader_is_anonymous(datareader))
    {
        return;
    }


    retval = DDS_DataReader_delete_route(datareader,key,uc_locator,mc_locator,
                                         &route_existed);

    if (retval)
    {
        if (route_existed)
        {
            DDS_DataReaderEvent_on_subscription_matched(
                    datareader,key,DDS_BOOLEAN_TRUE,DDS_BOOLEAN_FALSE);
        }
    }
}

/*ci
 * \brief Resolve which unicast and multicast locators should be used to reach
 *        a local datawriter
 *
 * \details
 *
 * If a datawriter specifies its own locators those are used instead of the
 * default locators specified on the participant. The datawriter can specify
 * its own unicast locator to receive packets from a datareader on. However,
 * the multicast address is always based on the default multicast
 * address specified in the participant. This is to simplify the datareaders
 * decision on where to send replies to a datawriter.
 *
 * \param[in]  datareader     Datareaeder to resolve for
 * \param[in]  key            Datawriter key
 * \param[in]  qos            Datawriter qos
 * \param[out] uc_locator_seq The unicast locators to use for the datawriter
 * \param[out] mc_locator_seq The multicast locators to use for the datawriter
 */
RTI_PRIVATE void
DDS_DataReader_resolve_local_locators(DDS_DataReader *datareader,
                                      struct DDS_BuiltinTopicKey_t *key,
                                      const DDS_DataWriter *const writer,
                                      const struct DDS_LocatorSeq **uc_locator_seq,
                                      const struct DDS_LocatorSeq **mc_locator_seq)
{
    if (DDS_BuiltinTopicKey_is_builtin(key))
    {
        *uc_locator_seq = datareader->config->default_meta_unicast;
        *mc_locator_seq = datareader->config->default_meta_multicast;
    }
    else
    {
        *uc_locator_seq = DDS_DataWriter_get_unicast_locator_ref(writer);
        if (DDS_LocatorSeq_get_length(*uc_locator_seq) == 0)
        {
            *uc_locator_seq = datareader->config->default_unicast;
        }
        *mc_locator_seq = datareader->config->default_multicast;
    }
}

RTI_PRIVATE RTI_BOOL
DDS_DataReader_resolve_peer_locators(
            const struct DDS_ParticipantBuiltinTopicData *const parent_data,
            const struct DDS_PublicationBuiltinTopicData *const data,
            const struct DDS_LocatorSeq **uc_locator_seq,
            const struct DDS_LocatorSeq **mc_locator_seq)
{
    /* determine which locators shall be used. This is needed to be able
     * to choose between a prioritized transport, such as shared memory
     * vs UDP and between different encapsulations.
     */
    if (DDS_BuiltinTopicKey_is_builtin(&data->key))
    {
        *uc_locator_seq = &parent_data->metatraffic_unicast_locators;
        *mc_locator_seq = &parent_data->metatraffic_multicast_locators;
    }
    else
    {
        *uc_locator_seq = &data->unicast_locator;
        if (DDS_LocatorSeq_get_length(*uc_locator_seq) == 0)
        {
            *uc_locator_seq = &parent_data->default_unicast_locators;
        }
        *mc_locator_seq = &parent_data->default_multicast_locators;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_DataReader_resolve_transports(DDS_DataReader *dr,
                                  const struct DDS_LocatorSeq *in_seq,
                                  struct DDS_LocatorSeq *reslvd_seq,
                                  const struct DDS_BuiltinTopicKey_t *const key)
{
    const struct DDS_Locator *a_loc;
    RTI_INT32 loc_length;
    RTI_INT32 loc_index;
    const struct DDS_Locator *cur_loc = NULL;
    RTI_INT32 current_prio = NETIO_DEFAULT_PRIORITY;
    RT_ComponentFactoryId_T intf_name;
    RTI_BOOL is_reachable = RTI_FALSE;
    struct NETIO_Address dst_addr = NETIO_Address_INITIALIZER;


    loc_length = DDS_LocatorSeq_get_length(in_seq);

    /* Nothing to resolve */
    if (loc_length == 0)
    {
        return RTI_TRUE;
    }

    for (loc_index = 0; loc_index < loc_length; ++loc_index)
    {
        struct NETIO_AddressEx addr_ex = NETIO_AddressEx_INITIALIZER;

        a_loc = DDS_LocatorSeq_get_reference(in_seq,loc_index);

        if (DDS_Transport_get_locator_priority(a_loc) > current_prio)
        {
            /* Already have a locator with higher priority, continue; */
            continue;
        }

        /* Found a potentially new locator, check if it is supported */
        if (!DDS_Locator_get_interface(a_loc,&intf_name,
                                       dr->config->route_resolver,
                                       dr->config->address_resolver))
        {
            /* This should not happen since unsupported locators are dropped
             * during discovery.
             */
            continue;
        }

        /* The transport may be supported, check if this particular address
         * can be reached. If there is an error, ignore it.
         */
        DDS_Locator_to_netio_address(a_loc,&dst_addr);

        OSAPI_Memory_copy(&addr_ex,&dst_addr,sizeof(struct NETIO_Address));

        if (NETIO_Address_get_kind(&dst_addr) == NETIO_ADDRESS_KIND_NOTIF)
        {
            /* copy the key into the into the data field of NETIO_Address
             * It is used by the notification interface to determine if it
             * can reach the datawriter
             */
            OSAPI_Memory_copy(addr_ex.data, key, NETIO_ADDRESS_MAX_8BIT);
        }
        else
        {
            OSAPI_Memory_zero(addr_ex.data,sizeof(addr_ex.data));
        }

        if (!NETIO_RouteResolver_is_address_reachable(
                dr->config->route_resolver,&addr_ex,
                &is_reachable))
        {
            continue;
        }

        if (!is_reachable)
        {
            continue;
        }

        cur_loc = a_loc;
        current_prio = DDS_Transport_get_locator_priority(cur_loc);
    }

    /*
     * If cur_loc == NULL no supported locators could be found.
     */
    if (cur_loc == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_Locator_append_locator_kind(in_seq,reslvd_seq,
                             NETIO_Address_get_kind(
                                     DDS_Locator_as_netioaddress(cur_loc))))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;

}

/*ci
 * \brief Resolve which unicast and multicast locators should be used to reach
 *        a remote publication
 *
 * \details
 *
 * If a publication specifies its own locators those are used instead of the
 * default locators specified on the participant.
 *
 * \param[in]  parent_data    The remote participant qos
 * \param[in]  data           The remote publication qos
 * \param[out] uc_locator_seq The unicast locators to use for the publication
 * \param[out] mc_locator_seq The multicast locators to use for the publication
 */
RTI_PRIVATE RTI_BOOL
DDS_DataReader_resolve_locators(DDS_DataReader *dr,
        const struct DDS_BuiltinTopicKey_t *const key,
        const struct DDS_DataRepresentationQosPolicy *data_qos,
        const struct DDS_LocatorSeq **uc_locator_seq,
        const struct DDS_LocatorSeq **mc_locator_seq)
{
    UNUSED_ARG(key);
    UNUSED_ARG(data_qos);
    UNUSED_ARG(mc_locator_seq);

    /* uc_locator_seq and mc_locator_seq points to the effective locators.
     * Next determine which locators to actually use based on transport
     * and encapsulation properties.
     */
    if (!DDS_LocatorSeq_set_length(dr->config->resolved_unicast_seq,0))
    {
        return RTI_FALSE;
    }

    if (!DDS_DataReader_resolve_transports(dr,
                                           *uc_locator_seq,
                                           dr->config->resolved_unicast_seq,key))
    {
        return RTI_FALSE;
    }

    if (DDS_LocatorSeq_get_length(dr->config->resolved_unicast_seq) == 0)
    {
        /* If no unicat locators were found resolve the multicast
         * cast locators independently.
         */
        if (!DDS_DataReader_resolve_transports(dr,*mc_locator_seq,
                                       dr->config->resolved_multicast_seq,key))
        {
            return RTI_FALSE;
        }
    }
    else
    {
        /* Append all multicast locators of the same kind */
        if (!DDS_Locator_append_locator_kind(
                    *mc_locator_seq,
                    dr->config->resolved_multicast_seq,
                    NETIO_Address_get_kind(
                       (struct NETIO_Address*)
                       DDS_LocatorSeq_get_reference(dr->config->resolved_unicast_seq,0))))
        {
            return RTI_FALSE;
        }
    }

    *uc_locator_seq = dr->config->resolved_unicast_seq;
    *mc_locator_seq = dr->config->resolved_multicast_seq;

    return RTI_TRUE;
}

/*ci
 * \brief
 *
 * \param[in] self        Datareader to check compatibility with a publication
 * \param[in] data        The partition qos
 *
 * \return DDS_BOOLEAN_TRUE if it is compatible, DDS_BOOLEAN false otherwise
 */
RTI_PRIVATE DDS_Boolean
DDS_Subscriber_is_partition_match(DDS_Subscriber *subscriber,
                                  const struct DDS_PartitionQosPolicy *data)
{
    if ((data != NULL) &&
        (DDS_PartitionQosPolicy_is_compatible(
                        &subscriber->partition->name,
                        &data->name)))
    {
        return DDS_BOOLEAN_TRUE;
    }
    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Match a datareader with a local datawriter
 *
 * \param[in] self        Datareader to match with a datawriter
 * \param[in] key         Key of the datawriter
 * \param[in] topic_name  The topic name published by the datawriter
 * \param[in] type_name   The type name published by the datawriter
 * \param[in] dw          The datawriter
 */
void
DDS_DataReader_match_local_writer(DDS_DataReader *self,
                                  struct DDS_BuiltinTopicKey_t *key,
                                  const char *topic_name,
                                  const char *type_name,
                                  const DDS_DataWriter *dw)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;
    DDS_Publisher *publisher = NULL;
    DDS_Subscriber *subscriber = NULL;
    const struct DDS_DataRepresentationQosPolicy *data_qos;
    struct DDS_DataWriterQos dw_qos = DDS_DataWriterQos_INITIALIZER;

    /* PublicationBuiltinTopicData is a subset of the DataWriterQos. Note that
     * the dw_qos is a shallow copy for pointer types (if any) since these
     * are only used in this function and it avoids memory allocations. Thus,
     * the dw_qos is not finalized.
     */
    dw_qos.deadline = dw->deadline;
    dw_qos.liveliness = dw->liveliness;
    dw_qos.ownership = dw->ownership;
    dw_qos.ownership_strength = dw->ownership_strength;
    dw_qos.reliability = dw->reliability;
    dw_qos.durability = dw->durability;
    dw_qos.destination_order = dw->destination_order;
    dw_qos.latency_budget = dw->latency_budget;


    publisher = DDS_DataWriter_get_publisher((DDS_DataWriter *)dw);
    subscriber = DDS_DataReader_get_subscriber(dr);

    if ((publisher == NULL) || (subscriber == NULL))
    {
        return;
    }

    if (!DDS_Subscriber_is_partition_match(subscriber,
                    DDS_Publisher_get_partition_ref(publisher)))
    {
        DDSC_LOG_INCOMPATIBLE_PARTITION_QOS(OSAPI_LOGKIND_INFO, topic_name)
        return;
    }

    if (!DDS_Topic_is_compatible(dr->topic,topic_name,
                                 type_name,
                                 dr->as_entity.entity_id,
                                 key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID]))
    {
        return;
    }

    DDS_DataReader_resolve_local_locators(self,key,dw,
                                          &uc_locator_seq,
                                          &mc_locator_seq);


    data_qos = DDS_DataWriter_get_data_representation_ref(dw);

    if (!DDS_DataReader_resolve_locators(self,
                                         key,
                                         data_qos,
                                         &uc_locator_seq,
                                         &mc_locator_seq))
    {
        return;
    }

    DDS_DataReader_match_writer(self,
                                key,
                                &dw_qos,
                                uc_locator_seq,
                                mc_locator_seq);
}

/*ci
 * \brief Match a datareader with a remote publication
 *
 * \param[in] self        Datareader to match with the remote publication
 * \param[in] parent_data The remote participant qos
 * \param[in] data        The remote publication qos
 */
void
DDS_DataReader_match_remote_writer(DDS_DataReader *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_PublicationBuiltinTopicData *const data)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)self;
    struct DDS_DataWriterQos dw_qos = DDS_DataWriterQos_INITIALIZER;
    const struct DDS_LocatorSeq *uc_locator_seq = NULL;
    const struct DDS_LocatorSeq *mc_locator_seq = NULL;
    DDS_Subscriber* sub = NULL;

    /* PublicationBuiltinTopicData is a subset of the DataWriterQos. Note that
     * the dw_qos is a shallow copy for pointer types (if any) since these
     * are only used in this function and it avoids memory allocations. Thus,
     * the dw_qos is not finalized.
     */
    dw_qos.deadline = data->deadline;
    dw_qos.liveliness = data->liveliness;
    dw_qos.ownership = data->ownership;
    dw_qos.ownership_strength = data->ownership_strength;
    dw_qos.reliability = data->reliability;
    dw_qos.durability = data->durability;
    dw_qos.destination_order = data->destination_order;
    dw_qos.representation = data->representation;
    dw_qos.latency_budget = data->latency_budget;

    sub = DDS_DataReader_get_subscriber(dr);
    if (sub == NULL)
    {
        goto done;
    }

    if (!DDS_Subscriber_is_partition_match(sub, &data->partition))
    {
        DDSC_LOG_INCOMPATIBLE_PARTITION_QOS(OSAPI_LOGKIND_INFO, data->topic_name)
        goto done;
    }

    if (!DDS_Topic_is_compatible(dr->topic,data->topic_name,
                                 data->type_name,
                                 dr->as_entity.entity_id,
                                 data->key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID]))
    {
        goto done;
    }


    if (!DDS_DataReader_resolve_peer_locators(parent_data,
                                              data,
                                              &uc_locator_seq,
                                              &mc_locator_seq))
    {
        goto done;
    }

    if (!DDS_DataReader_resolve_locators(self,
                                         &data->key,
                                         &dw_qos.representation,
                                         &uc_locator_seq,
                                         &mc_locator_seq))
    {
        return;
    }

    DDS_DataReader_match_writer(self,
                                &data->key,
                                &dw_qos,
                                uc_locator_seq,
                                mc_locator_seq);

done:
    return;
}

#ifndef RTI_CERT
/*ci
 * \brief Unmatch a datareader from a local datawriter
 *
 * \param[in] self   Datareader to unmatch with a datawriter
 * \param[in] key    Key of the datawriter
 * \param[in] writer The datawriter
 *
 * \sa \ref DDS_DataReader_match_local_writer
 */
void
DDS_DataReader_unmatch_local_writer(DDS_DataReader *self,
                                    struct DDS_BuiltinTopicKey_t *key,
                                    const DDS_DataWriter *writer)
{
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;

    DDS_DataReader_resolve_local_locators(self,key,writer,
                                          &uc_locator_seq,
                                          &mc_locator_seq);

    DDS_DataReader_unmatch_writer(self,key,uc_locator_seq,mc_locator_seq);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Unmatch a datareader from a remote publication
 *
 * \param[in] self        Datareader to unmatch with a remote publication
 * \param[in] parent_data The remote participant qos
 * \param[in] data        The remote publication qos
 *
 * \sa \ref DDS_DataReader_match_remote_writer
 */
void
DDS_DataReader_unmatch_remote_writer(DDS_DataReader *self,
                const struct DDS_ParticipantBuiltinTopicData *const parent_data,
                const struct DDS_PublicationBuiltinTopicData *const data)
{
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;

    if (!DDS_DataReader_resolve_peer_locators(parent_data,data,
                                              &uc_locator_seq,
                                              &mc_locator_seq))
    {
        return;
    }

    DDS_DataReader_unmatch_writer(self,
                                  &data->key,
                                  uc_locator_seq,
                                  mc_locator_seq);
}

/*ci @} */

