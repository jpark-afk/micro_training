/*
 * FILE: DataWriterDiscovery.c - DataWriter discovery implementation
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
 * 08jun2015,tk MICRO-1217/PR#14763 Cleared up use of bind/bind_external use
 * 08jun2015,tk MICRO-1285/PR#14932 Reset local retval to error in case
 *                                  subsequent calls fail in delete_route
 * 16may2015,tk MICRO-1218/PR#14764 Added comment on why multicast addresses
 *                                  are ignored on the datawriter
 * 16may2015,tk MICRO-1216/PR#14762 Removed unused parameter from
 *                                  add_route_from_locator
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 26jan2015,tk MICRO-1028/PR#13473 Removed magic number 0xc0
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 25feb2014,tk  MICRO-714: Set DDS writer peer state
 * 27jan2014,eh  MICRO-714: Set first_sn based on durability kind
 * 23may2013,tk  Major update for discovery
 * 18may2012,tk  Major update for discovery
 * 26aug2011,yy  Fixed returning errors
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief DataWriter discovery implementation
 *
 * \details
 * This file implements functionality to manage DDS discovery functionality
 * related to a DDS datawriter. This includes matching/unmatching with
 * local and remote datareaders and establishing communication with datareaders.
 */
/*ci
 * \addtogroup DDSPublicationModule
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
#include "DataWriterImpl.h"
#include "DataWriterEvent.h"
#include "DataWriterQos.h"
#include "DataWriterInterface.h"
#include "DataWriterDiscovery.h"
#include "PublisherEvent.h"
#include "PublisherImpl.h"
#include "DataReaderImpl.h"
#include "PartitionQosPolicy.h"

#if DDS_FILTERING_ENABLED
#include "DataWriterFilter.h"
#include "DataReaderFilter.h"
#endif

/*** SOURCE_BEGIN ***/
RTI_PRIVATE DDS_DataRepresentationId_t
DDS_DataWriter_resolve_data_representation(DDS_DataWriter *dw,
                   const struct DDS_DataRepresentationQosPolicy *policy);

RTI_PRIVATE DDS_EncapsulationId_t
DDS_DataWriter_resolve_builtin_encapsulation(const struct DDS_BuiltinTopicKey_t *const key)
{
#if !DDS_LIVELINESS_CHANNEL_ENABLED
    UNUSED_ARG(key);
#else
#if DDS_LIVELINESS_CHANNEL_ENABLED
    if  ((key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID]   == RTPS_OBJECT_ID_WRITER_IPC_MESSAGE_DATA)
        || (key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] == RTPS_OBJECT_ID_READER_IPC_MESSAGE_DATA))
    {
        return DDS_ENCAPSULATION_ID_CDR_NATIVE;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
#endif /* !DDS_LIVELINESS_CHANNEL_ENABLED */

    return DDS_ENCAPSULATION_ID_PL_CDR_NATIVE;
}

/*ci
 * \brief Check if a datawriter is compatible with a datareader's qos
 *
 * \param[in] datawriter Datawriter to match with the datareader's Qos
 * \param[in] dr_qos     The datareader Qos to match against
 *
 * \return DDS_BOOLEAN_TRUE if it is compatible, DDS_BOOLEAN false otherwise
 */
DDS_Boolean
DDS_DataWriter_reader_is_compatible(DDS_DataWriter *datawriter,
                                    const struct DDS_DataReaderQos *dr_qos)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
    DDS_DataRepresentationId_t data_id;

    if (!DDS_DeadlineQosPolicy_is_compatible(&dr_qos->deadline,
                                             &dw->deadline))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DEADLINE_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_compatible(&dr_qos->ownership,
                                              &dw->ownership))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_OWNERSHIP_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_compatible(&dr_qos->reliability,
                                                &dw->reliability))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_RELIABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_compatible(&dr_qos->liveliness,
                                               &dw->liveliness))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_LIVELINESS_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_compatible(&dr_qos->durability,
                                               &dw->durability))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DURABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_compatible(&dr_qos->destination_order,
                                                     &dw->destination_order))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DESTINATIONORDER_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DataRepresentationQosPolicy_is_compatible(&dr_qos->representation,
                                                        dw->representation,
                                                        &data_id))
    {
        /* The user specified representation may not match the actual
         * representations used by the writer.
         */
        if (DDS_DataWriter_resolve_data_representation(datawriter,
                   &dr_qos->representation) == DDS_INVALID_DATA_REPRESENTATION)
        {
            datawriter->off_incompatible_qos_status.last_policy_id = DDS_DATA_REPRESENTATION_QOS_POLICY_ID;
            retval = DDS_BOOLEAN_FALSE;
        }
    }

    if (!DDS_LatencyBudgetQosPolicy_is_compatible(&dr_qos->latency_budget,
                                                  &dw->latency_budget))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_LATENCYBUDGET_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    return retval;
}

DDS_Boolean
DDS_DataWriter_lookup_route(DDS_DataWriter *src_writer,
                            struct NETIO_Address *dst_reader,
                            const struct DDS_LocatorSeq *loc_seq)
{
    RTI_INT32 length, i;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct NETIO_Address netio_address;
    const struct NETIO_AddressSeq *na_seq =
                                        (const struct NETIO_AddressSeq*)loc_seq;
    RTI_BOOL route_exists = RTI_FALSE;

    if (!NETIO_Interface_lookup_route((NETIO_Interface_T*)src_writer->dw_intf,
                                      dst_reader,src_writer->rtps_intf,
                                      dst_reader,&route_exists))
    {
        DDSC_LOG_NETIO_ROUTE_LOOKUP_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!route_exists)
    {
        goto done;
    }

    length = NETIO_AddressSeq_get_length(na_seq);
    for (i = 0; i < length; ++i)
    {
        netio_address = *NETIO_AddressSeq_get_reference(na_seq,i);

        if (!NETIO_RouteResolver_lookup_route(
                        src_writer->config->route_resolver,
                        src_writer->rtps_intf,dst_reader,
                        &netio_address,&route_exists))
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (route_exists)
        {
            retval = DDS_BOOLEAN_TRUE;
            break;
        }
    }

done:
    return retval;
}

DDS_Boolean
DDS_DataWriter_add_anonymous_peer(DDS_DataWriter *datawriter,
                                  struct NETIO_Address *dst_reader,
                                  const char *address)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
    struct DDS_DataWriterBindProperty dwb_property =
                                        DDS_DataWriterBindProperty_INITIALIZER;
    struct DDS_DataWriterRouteProperty dwr_property =
                                        DDS_DataWriterRouteProperty_INITIALIZER;
    struct RTPS_RouteProperty route_property = RTPSRouteProperty_INITIALIZER;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);


    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    dwb_property.encapsulation_id = DDS_DataWriter_resolve_builtin_encapsulation(&dw_key);
    dwr_property.encapsulation = dwb_property.encapsulation_id;
    dwb_property.next_new_sn = dw->last_sn;
    REDA_SequenceNumber_plusplus(&dwb_property.next_new_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.low_history_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.high_history_sn);
    dwb_property.is_reliable = DDS_BOOLEAN_FALSE;

    if (!NETIO_Interface_add_route((NETIO_Interface_T*)dw->dw_intf,
                                   dst_reader,dw->rtps_intf,
                                   dst_reader,&dwr_property._parent,NULL))
    {
        DDSC_LOG_NETIO_ADD_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAWRITER_NETIO_KIND,
                                 DDSC_LOG_DATAREADER_NETIO_KIND)
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    route_property.first_sn = dwb_property.low_history_sn;
    route_property.last_sn = dwb_property.high_history_sn;
    route_property.reliable = DDS_BOOLEAN_FALSE;
    route_property.encapsulation = dwb_property.encapsulation_id;
    route_property.last_acked_sn = dwb_property.high_history_sn;

    if (!NETIO_Interface_bind((NETIO_Interface_T*)dw->dw_intf,
                              dst_reader,&dwb_property._parent,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_DATAWRITER_NETIO_KIND,
                            DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_RouteResolver_add_peer(datawriter->config->route_resolver,
                                (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                                NETIO_ROUTEKIND_META,address,
                                &route_property._parent,NULL,NULL))
    {
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_TRACE || OSAPI_ENABLE_LOG
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to add anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else
    {
        OSAPI_TRACE_DDS("datawriter added anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&dw_key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_DataWriter_delete_anonymous_peer(DDS_DataWriter *datawriter,
                                      struct NETIO_Address *dst_reader,
                                      const char *address)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
#if OSAPI_ENABLE_TRACE || OSAPI_ENABLE_LOG
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif

    OSAPI_TRACE_DDS("delete anon route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

#if 0
    if (!NETIO_Interface_unbind((NETIO_Interface_T*)dw->dw_intf,
                                dst_reader,(NETIO_Interface_T*)dw->dw_intf,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_DATAWRITER_NETIO_KIND,
                              DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }
#endif

    if (!NETIO_RouteResolver_delete_peer(datawriter->config->route_resolver,
            (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
            NETIO_ROUTEKIND_META,address,NULL,NULL))
    {
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_TRACE || OSAPI_ENABLE_LOG
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to delete anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else
    {
        OSAPI_TRACE_DDS("datawriter deleted anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}
#endif /* !RTI_CERT */

DDS_Boolean
DDS_DataWriter_add_anonymous_route(DDS_DataWriter *datawriter,
                                   struct NETIO_Address *dst_reader,
                                   struct NETIO_Address *via_address)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
    struct DDS_DataWriterBindProperty dwb_property =
                                        DDS_DataWriterBindProperty_INITIALIZER;
    struct RTPS_RouteProperty route_property = RTPSRouteProperty_INITIALIZER;
    struct DDS_DataWriterRouteProperty dwr_property =
                                        DDS_DataWriterRouteProperty_INITIALIZER;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

    OSAPI_TRACE_DDS("datawriter adding route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    dwb_property.encapsulation_id = DDS_DataWriter_resolve_builtin_encapsulation(&dw_key);
    dwr_property.encapsulation = dwb_property.encapsulation_id;

    if (!NETIO_Interface_add_route((NETIO_Interface_T*)dw->dw_intf,
                                   dst_reader,dw->rtps_intf,
                                   dst_reader,&dwr_property._parent,NULL))
    {
        DDSC_LOG_NETIO_ADD_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAWRITER_NETIO_KIND,
                                 DDSC_LOG_DATAREADER_NETIO_KIND)
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    dwb_property.next_new_sn = dw->last_sn;
    REDA_SequenceNumber_plusplus(&dwb_property.next_new_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.low_history_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.high_history_sn);
    dwb_property.is_reliable = DDS_BOOLEAN_FALSE;

    if (!NETIO_Interface_bind((NETIO_Interface_T*)dw->dw_intf,
                                        dst_reader,&dwb_property._parent,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_DATAWRITER_NETIO_KIND,
                            DDSC_LOG_DATAREADER_NETIO_KIND)
                retval = DDS_BOOLEAN_FALSE;
                goto done;
    }

    route_property.first_sn = dwb_property.low_history_sn;
    route_property.last_sn = dwb_property.high_history_sn;
    route_property.reliable = DDS_BOOLEAN_FALSE;
    route_property.encapsulation = dwb_property.encapsulation_id;
    route_property.last_acked_sn = dwb_property.high_history_sn;

    if (!NETIO_RouteResolver_add_route(datawriter->config->route_resolver,
            (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
            via_address,&route_property._parent,NULL,NULL))
    {
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to add route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else
    {
        OSAPI_TRACE_DDS("datawriter added anon route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

DDS_Boolean
DDS_DataWriter_delete_anonymous_route(DDS_DataWriter *datawriter,
                                      struct NETIO_Address *dst_reader,
                                      struct NETIO_Address *via_address)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

#if 0
    if (!NETIO_Interface_unbind((NETIO_Interface_T*)dw->dw_intf,
                              dst_reader,(NETIO_Interface_T*)dw->dw_intf,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_DATAWRITER_NETIO_KIND,
                              DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }
#endif

    if (!NETIO_RouteResolver_delete_route(datawriter->config->route_resolver,
            (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
            via_address,NULL,NULL))
    {
        retval = DDS_BOOLEAN_FALSE;
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to delete route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else
    {
        OSAPI_TRACE_DDS("datawriter deleted route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

DDS_Boolean
DDS_DataWriter_delete_anonymous_route_from_seq(DDS_DataWriter *datawriter,
                                               struct NETIO_Address *dst_reader,
                                               const struct DDS_LocatorSeq *locator_seq)
{
    RTI_INT32 loc_length, loc_index;
    struct DDS_Locator *a_locator = NULL;
    struct NETIO_Address dst_address = NETIO_Address_INITIALIZER;

    NETIO_Address_init(&dst_address,0);

    loc_length = DDS_LocatorSeq_get_length(locator_seq);
    for (loc_index = 0; loc_index < loc_length; loc_index++)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator_seq,loc_index);
        if (a_locator == NULL)
        {
            /* This is not expected. Use unicast as we don't know what kind of
             * sequence is being passed in.
             */
            DDSC_LOG_SEQ_GETREF(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_METAUNICAST_SEQUENCE,
                                loc_index)
            continue;
        }
        OSAPI_Memory_copy(&dst_address,a_locator,sizeof(struct DDS_Locator));
        if (!DDS_DataWriter_delete_anonymous_route(
                        datawriter,dst_reader,&dst_address))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Add a route from a datawriter to a datareader from a locator sequence
 *
 * \details
 *
 * This is a helper function to add routes to a datareader for a datawriter
 * using a route resolver to find an interface which can route to the datareader
 * via the locators.
 *
 * \param[in]  dw             Datawriter
 * \param[in]  locator_seq    Sequence of routes to add
 * \param[in]  route_property The properties of the route
 * \param[in]  dst_reader     The address of the destination reader
 *
 * \return DDS_BOOLEAN_TRUE if it is enabled, DDS_BOOLEAN false otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataWriter_add_route_from_locator(struct DDS_DataWriterImpl *dw,
                                      const struct DDS_LocatorSeq *locator_seq,
                                      struct RTPS_RouteProperty *route_property,
                                      struct NETIO_Address *dst_reader)
{
    DDS_Long i,j,k;
    struct DDS_Locator *locator;
    struct NETIO_Address via_address;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        locator = DDS_LocatorSeq_get_reference(locator_seq,i);

        via_address = *((struct NETIO_Address*)locator);

        if (!NETIO_RouteResolver_add_route(
                dw->config->route_resolver,
                (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                &via_address,&route_property->_parent,NULL,NULL))
        {
            break;
        }
    }

    if (i < j)
    {
        for (k = 0; k < i; k++)
        {
            via_address = *(struct NETIO_Address*)
                                            DDS_LocatorSeq_get_reference(locator_seq,k);
            if (!NETIO_RouteResolver_delete_route(dw->config->route_resolver,
                                                  (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                                                  &via_address,NULL,NULL))
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_DATAWRITER_NETIO_KIND,
                                            DDSC_LOG_DATAREADER_NETIO_KIND);
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Add a datawriter as a listener to a datareader from a locator sequence
 *
 * \details
 *
 * This is a helper function to add a datawriter as a listener to data from
 * a datareader using a bind resolver to find an interface which can
 * listen to the datareader via the locators. A datawriter listens to
 * reliability related traffic from a datareader, such as ACKNACKs.
 *
 * \param[in] dw             Datawriter
 * \param[in] locator_seq    Sequence of routes to add
 * \param[in] recv_kind      The type of route
 * \param[in] ext_intf       The interface to bind to
 * \param[in] dst_reader     The address of the reader to listen to
 *
 * \return DDS_BOOLEAN_TRUE if it is enabled, DDS_BOOLEAN false otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataWriter_bind_from_locator(struct DDS_DataWriterImpl *dw,
                                 struct DDS_LocatorSeq *locator_seq,
                                 NETIO_RouteKind_T recv_kind,
                                 NETIO_Interface_T *ext_intf,
                                 struct NETIO_Address *dst_reader)
{
    DDS_Long i,j,k;
    struct NETIO_Address from_address;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = *(struct NETIO_Address*)
                          DDS_LocatorSeq_get_reference(locator_seq,i);

        /* notif is handled else where */
        if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
        {
            if (!NETIO_BindResolver_bind(dw->config->bind_resolver,
                    (struct REDA_StringSeq*)dw->config->enabled_transports,
                    recv_kind,&from_address,
                    ext_intf,dst_reader,NULL,NULL))
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

            /* notif is handled else where */
            if (NETIO_Address_get_kind(&from_address) != NETIO_ADDRESS_KIND_NOTIF)
            {
                if (!NETIO_BindResolver_unbind(dw->config->bind_resolver,
                        (struct REDA_StringSeq*)dw->config->enabled_transports,
                        recv_kind,&from_address,
                        ext_intf,dst_reader,NULL,NULL))
                {
                    DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                        DDSC_LOG_DATAWRITER_NETIO_KIND,
                                        DDSC_LOG_DATAREADER_NETIO_KIND);
                }
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Delete route from a datawriter to a datareader for the notification interface
 *
 * \param[in]  datawriter     Datawriter to delete routes from
 * \param[in]  key            The datareader key
 * \param[in]  uc_locator     The unicast locators to the datareader
 * \param[out] use_notif      RTI_TRUE if notification interface is used
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
RTI_PRIVATE RTI_BOOL
DDS_DataWriter_delete_notif_interface_route(DDS_DataWriter *datawriter,
                            const DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            RTI_BOOL *use_notif)

{   struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    struct DDS_Locator *curr_loc = NULL;
    struct NETIO_Address dst_reader_notif = NETIO_Address_INITIALIZER;
    struct NETIO_Address from_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address via_address = NETIO_Address_INITIALIZER;
    RTI_BOOL found = RTI_FALSE;
    NETIO_Interface_T *found_notif_intf = NULL;
    NETIO_Interface_T notif_key_intf = NETIO_Interface_INITIALIZER;
    NETIO_Interface_T *ext_intf = NULL;
    RTI_INT32 i = 0, j = 0;
    RTI_BOOL retval = RTI_FALSE;

    *use_notif = RTI_FALSE;

    j = DDS_LocatorSeq_get_length(uc_locator);
    if (j == 0)
    {
        retval = RTI_TRUE;
        goto done;
    }

    for (i = 0; i < j; i++)
    {
        curr_loc = DDS_LocatorSeq_get_reference(uc_locator, i);
        if (curr_loc != NULL)
        {
            DDS_Locator_to_netio_address(curr_loc,&via_address);
            if (NETIO_Address_get_kind(&via_address) == NETIO_ADDRESS_KIND_NOTIF)
            {
                if (NETIO_RouteResolver_find_interface(
                                dw->config->route_resolver, &found_notif_intf, &via_address))
                {
                    found = RTI_TRUE;
                    break;
                }
            }
        }
    }

    if (!found)
    {
        retval = RTI_TRUE;
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(
            dw->dw_intf, NULL, &ext_intf, &from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    /* notif delete route needs the BuiltinTopic key to search its tables.
     * There is no easy way to pass in this information so it's packaged
     * into an interface called <notif_key_intf> that has the local
     * address as the key. This interace isn't used for anything else.
     */
    NETIO_Address_set_guid_from_key(&notif_key_intf.local_address,0,(struct NETIO_AddressInt32*)key);
    if (!NETIO_Interface_delete_route(
            found_notif_intf, &from_address,
            &notif_key_intf, &via_address, use_notif))
    {
        DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    NETIO_Address_set_guid_from_key(&dst_reader_notif,0,(struct NETIO_AddressInt32*)key);
    NETIO_Address_set_kind(&dst_reader_notif, NETIO_ADDRESS_KIND_NOTIF, 0);

    if (!NETIO_Interface_delete_route(
            (NETIO_Interface_T *)dw->dw_intf,
            &dst_reader_notif,
            found_notif_intf,
            &dst_reader_notif,
            use_notif))
    {
        DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    retval = RTI_TRUE;
done:
    return retval;
}

/*ci
 * \brief Add a route from a datawriter to a datareader for the notification interface
 *
 * \details
 *
 * Helper function to add unicast routes to the Notification Interface.
 *
 * \param[in]  datawriter     Datawriter to add routes to
 * \param[in]  key            The datareader key
 * \param[in]  uc_locator     The unicast locators to the datareader
 * \param[in]  is_reliable    RTI_TRUE if the route is reliable
 * \param[out] use_notif      RTI_TRUE if notification interface is used
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
RTI_PRIVATE RTI_BOOL
DDS_DataWriter_add_notif_interface_route(DDS_DataWriter *datawriter,
                         const DDS_BuiltinTopicKey_t *key,
                         const struct DDS_LocatorSeq *uc_locator,
                         RTI_BOOL is_reliable,
                         RTI_BOOL *use_notif)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    struct DDS_DataWriterRouteProperty dwr_property =
                                        DDS_DataWriterRouteProperty_INITIALIZER;
    struct DDS_Locator *curr_loc = NULL;
    struct NETIO_Address dst_reader_notif = NETIO_Address_INITIALIZER;
    struct NETIO_Address from_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address via_address = NETIO_Address_INITIALIZER;
    RTI_BOOL found = RTI_FALSE;
    NETIO_Interface_T *found_notif_intf = NULL;
    NETIO_Interface_T *ext_intf = NULL;
    NETIO_Interface_T notif_key_intf;
    RTI_INT32 i = 0, j = 0;
    RTI_BOOL dwi_add_route = RTI_FALSE, retval = RTI_FALSE;
    struct NETIORouteProperty notif_prop = NETIORouteProperty_INITIALIZER;
    *use_notif = RTI_FALSE;
    notif_prop.is_reliable = is_reliable;

    /* notification specific encapsulation kind */
    dwr_property.encapsulation = DDS_ENCAPSULATION_ID_SHMEM_V2;
    j = DDS_LocatorSeq_get_length(uc_locator);

    if (j == 0)
    {
        retval = RTI_TRUE;
        goto done;
    }

    for (i = 0; i < j; i++)
    {
        curr_loc = DDS_LocatorSeq_get_reference(uc_locator, i);
        if (curr_loc != NULL)
        {
            DDS_Locator_to_netio_address(curr_loc,&via_address);
            if (NETIO_Address_get_kind(&via_address) == NETIO_ADDRESS_KIND_NOTIF)
            {
                if (NETIO_RouteResolver_find_interface(
                                dw->config->route_resolver, &found_notif_intf, &via_address))
                {
                    found = RTI_TRUE;
                    break;
                }
            }
        }
    }

    /* Either no locator for the notif exists or no interface can route the locators
     * In any case move forward with the communication
     */
    if (!found)
    {
        retval = RTI_TRUE;
        goto done;
    }

    *use_notif = RTI_TRUE;
    NETIO_Address_set_guid_from_key(&dst_reader_notif,0,(struct NETIO_AddressInt32*)key);
    NETIO_Address_set_kind(&dst_reader_notif, NETIO_ADDRESS_KIND_NOTIF, 0);

    if (!NETIO_Interface_add_route(
            (NETIO_Interface_T *)dw->dw_intf,
            &dst_reader_notif,
            found_notif_intf,
            &dst_reader_notif,
            &dwr_property._parent, NULL))
    {
        DDSC_LOG_NETIO_ADD_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAWRITER_NETIO_KIND,
                                 DDSC_LOG_NETIO_NETIO_KIND)

        goto done;
    }

    dwi_add_route = RTI_TRUE;

    if (!NETIO_Interface_get_external_interface(
            dw->dw_intf, NULL, &ext_intf, &from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    /* Add route from the notif to the final dst reader */
    NETIO_Address_set_guid_from_key(&notif_key_intf.local_address,0,(struct NETIO_AddressInt32*)key);
    if (!NETIO_Interface_add_route( found_notif_intf, &from_address,
            &notif_key_intf, &via_address, &notif_prop, NULL))
    {
        goto done;
    }

    retval = RTI_TRUE;
done:
    if (dwi_add_route && !retval)
    {
        RTI_BOOL brc = NETIO_Interface_delete_route(
                        (NETIO_Interface_T *)dw->dw_intf,
                        &dst_reader_notif,
                        found_notif_intf,
                        &dst_reader_notif,
                        NULL);
        IGNORE_RETVAL(brc);
    }
    return retval;
}

/*ci
 * \brief Add a route from a datawriter to a datareader
 *
 * \details
 *
 * Add unicast and multicast routes to a datareader for a datawriter using a
 * route resolver to find an interface which can route to the datareader
 * via the unicast and multicast locators.
 *
 * \param[in]  datawriter     Datawriter to add routes to
 * \param[in]  key            The datareader key
 * \param[in]  qos            The datareader qos
 * \param[in]  uc_locator     The unicast locators to the datareader
 * \param[in]  mc_locator     The multicast locators to the datareader
 * \param[out] route_existed  RTI_TRUE if a route to the reader already exists
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref DDS_DataWriter_delete_route
 */
DDS_Boolean
DDS_DataWriter_add_route(DDS_DataWriter *datawriter,
                         const DDS_BuiltinTopicKey_t *key,
                         const struct DDS_DataReaderQos *const qos,
                         const struct DDS_LocatorSeq *uc_locator,
                         const struct DDS_LocatorSeq *mc_locator,
                         DDS_EncapsulationId_t encapsulation,
                         DDS_DataRepresentationId_t representation,
                         RTI_BOOL *route_existed)
{
#define DW_OPERATION_NONE                 0
#define DW_OPERATION_BIND_DWINTF          0x1
#define DW_OPERATION_RESOLVER_ADD_ROUTE   0x2
#define DW_OPERATION_ADD_ROUTE_DWINTF     0x4
#define DW_OPERATION_BIND_EXTERNAL_RTPS   0x8
#define DW_OPERATION_ADD_ROUTE_UC         0x10
#define DW_OPERATION_ADD_ROUTE_MC         0x20
#define DW_OPERATION_BIND_RTPS            0x40
#define DW_OPERATION_BIND_UC              0x80
#define DW_OPERATION_BIND_MC              0x100

    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTPS_RouteProperty route_property;
    struct DDS_DataWriterBindProperty dwb_property =
                                        DDS_DataWriterBindProperty_INITIALIZER;
    struct DDSHST_WriterState *wh_state;
    struct NETIO_Address from_address = NETIO_Address_INITIALIZER;
    NETIO_Interface_T *ext_intf = NULL;
    struct DDS_LocatorSeq *dw_uc_locator_seq;
    struct DDS_LocatorSeq *dw_mc_locator_seq;
    NETIO_RouteKind_T recv_kind;
    RTI_BOOL found_route;
    RTI_UINT32 ops = DW_OPERATION_NONE;
    struct NETIOBindProperty *b_prop = NULL;
    struct DDS_DataWriterRouteProperty dwr_property =
                                        DDS_DataWriterRouteProperty_INITIALIZER;
    RTI_BOOL is_reliable = RTI_FALSE;
    UNUSED_ARG(representation);

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

    wh_state = DDSHST_Writer_get_state(dw->wh);

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dw_key) &&
            DDS_ObjectId_is_builtin(dw->as_entity.entity_id))
    {
        *route_existed = RTI_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    NETIO_Address_set_guid_from_key(&dst_reader,0,(struct NETIO_AddressInt32*)key);

    OSAPI_TRACE_DDS("datawriter adding route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)

    if (DB_Database_lock(dw->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    dwb_property.next_new_sn = dw->last_sn;
    REDA_SequenceNumber_plusplus(&dwb_property.next_new_sn);

    if (qos->durability.kind > DDS_VOLATILE_DURABILITY_QOS)
    {
        /* If the reader requests historical data then set the
         * lowest and highest claimable SN to current history. The
         * wh_state->history_high_sn is always the most recently written SN
         * at the time of the match.
         */
        dwb_property.low_history_sn = wh_state->history_low_sn;
        dwb_property.high_history_sn = wh_state->history_high_sn;
    }
    else
    {
        /* If the reader does not request historical data set the historical
         * claimable SN to 0. A request can never fall within this window.
         */
        REDA_SequenceNumber_set_zero(&dwb_property.low_history_sn);
        REDA_SequenceNumber_set_zero(&dwb_property.high_history_sn);
    }

    dwb_property.is_reliable = (qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

    if ((encapsulation == DDS_ENCAPSULATION_ID_RESOLVE) &&
        DDS_BuiltinTopicKey_is_builtin(key))
    {
        dwb_property.encapsulation_id = DDS_DataWriter_resolve_builtin_encapsulation(key);
    }
    else
    {
        dwb_property.encapsulation_id = encapsulation;
    }

    if (!NETIO_Interface_bind(dw->dw_intf,&dst_reader,
                              &dwb_property._parent,route_existed))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_NETIO_KIND,
                            DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    ops |= DW_OPERATION_BIND_DWINTF;

    /* If there was an existing match, no need to continue as the
     * QoS are all immutable
     */
    if (*route_existed)
    {
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    if (qos->durability.kind > DDS_VOLATILE_DURABILITY_QOS)
    {
        /* If the reader is a new match increase the historical ackcount.
         * This ensures that historical samples are appropriately acked
         * before they are removed if they move out of the history window.
         */
        DDS_DataWriter_update_historical_ackcount(dw);
    }

    /* A route is only added upon a match. Thus, add route is only concerned
     * with managing resources to reach its peer based on destination
     * information for the peer.
     */
    /* if the entity is within the same participant, use the loopback
     * interface.
     */
    if (DDS_BuiltinTopicKey_prefix_equals(key,&dw_key))
    {
        /* Routes not going through rtps dont need to serialize */
        dwr_property.encapsulation = DDS_ENCAPSULATION_ID_MEMORY;
        if (!NETIO_RouteResolver_add_route(datawriter->config->route_resolver,
                                (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                                &dst_reader,&dwr_property._parent,
                                route_existed,&found_route))
        {
            goto done;
        }

        if (found_route)
        {
            retval = DDS_BOOLEAN_TRUE;
            goto done;
        }

        /* Continue with regular communication. Readers are enabled first so
         * if there was a local reader it is already asserted and enabled. Thus,
         * if the execution path gets here there is no INTRA transport.
         */
    }

    ops |= DW_OPERATION_RESOLVER_ADD_ROUTE;

    /* Now we check for NOTIF interface. The path for
     * Notification interface  does not go through RTPS.
     */
    is_reliable = (qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    if (!DDS_DataWriter_add_notif_interface_route(
        datawriter,key,uc_locator,is_reliable,&found_route))
    {
        goto done;
    }

    if (found_route)
    {
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    /* All other communication paths relies on RTPS. The datawriter
     * creates and established the RTPS interface upon creation, thus
     * it already exists. All we need to do is add a route DW->RTPS and
     * a route from RTPS-> requested DR interface.
     */
    route_property.reliable = is_reliable;

    /*
     * Set peer's First and Last SN, its send window, according to the
     * Durability and History Qos policies.
     */
    route_property.first_sn = wh_state->low_sn;
    route_property.last_sn = wh_state->high_sn;
    route_property.encapsulation = dwb_property.encapsulation_id;

    if (qos->durability.kind == DDS_VOLATILE_DURABILITY_QOS)
    {
        route_property.last_acked_sn = wh_state->high_sn;
    }
    else
    {
        route_property.last_acked_sn = wh_state->low_sn;
        if (!REDA_SequenceNumber_is_zero(&route_property.last_acked_sn))
        {
            REDA_SequenceNumber_minusminus(&route_property.last_acked_sn);
        }
    }

    dwr_property.encapsulation = dwb_property.encapsulation_id;

    if (!NETIO_Interface_add_route(
            (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
            dw->rtps_intf,&dst_reader,&dwr_property._parent,NULL))
    {
        DDSC_LOG_NETIO_ADD_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAWRITER_NETIO_KIND,
                                 DDSC_LOG_DATAREADER_NETIO_KIND)
                        goto done;
    }

    ops |= DW_OPERATION_ADD_ROUTE_DWINTF;

    if (!NETIO_Interface_get_external_interface(
            dw->dw_intf,&dst_reader,&ext_intf,&from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_bind_external(dw->rtps_intf,&dst_reader,
                                       ext_intf,&from_address,NULL,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_RTPS_NETIO_KIND,
                            DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    ops |= DW_OPERATION_BIND_EXTERNAL_RTPS;

    if (!DDS_DataWriter_add_route_from_locator(dw,uc_locator,
                                               &route_property,&dst_reader))
    {
        goto done;
    }

    ops |= DW_OPERATION_ADD_ROUTE_UC;

    if (!DDS_DataWriter_add_route_from_locator(dw,mc_locator,
                                               &route_property,&dst_reader))
    {
        goto done;
    }

    ops |= DW_OPERATION_ADD_ROUTE_MC;

    retval = DDS_BOOLEAN_FALSE;

    b_prop = NULL;

    if (!NETIO_Interface_bind(dw->rtps_intf,&dst_reader,b_prop,NULL))
    {
        DDSC_LOG_NETIO_BIND(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_RTPS_NETIO_KIND,
                            DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    ops |= DW_OPERATION_BIND_RTPS;

    if (!NETIO_Interface_get_external_interface(dw->rtps_intf,&dst_reader,
                                                &ext_intf,&from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (DDS_ObjectId_is_builtin(dw->as_entity.entity_id))
    {
        dw_uc_locator_seq = dw->config->default_meta_unicast;
        dw_mc_locator_seq = dw->config->default_meta_multicast;
        recv_kind = NETIO_ROUTEKIND_META;
    }
    else
    {
        /* The datawriter can specify its own unicast locator to receive
         * packets from a datareader on. However, the multicast address is
         * always based on the default multicast address specified in
         * the participant. This is to simplify the datareaders decision on
         * where to send replies to.
         */
        dw_uc_locator_seq = &dw->uc_locator_seq;
        if (DDS_LocatorSeq_get_length(dw_uc_locator_seq) == 0)
        {
            dw_uc_locator_seq = dw->config->default_unicast;
        }
        dw_mc_locator_seq = dw->config->default_multicast;
        recv_kind = NETIO_ROUTEKIND_USER;
    }

    if (!DDS_DataWriter_bind_from_locator(dw,dw_uc_locator_seq,
                                          recv_kind,ext_intf,&from_address))
    {
        goto done;
    }

    ops |= DW_OPERATION_BIND_UC;

    if (!DDS_DataWriter_bind_from_locator(dw,dw_mc_locator_seq,
                                          recv_kind,ext_intf,&from_address))
    {
        goto done;
    }

    ops |= DW_OPERATION_BIND_MC;

    retval = DDS_BOOLEAN_TRUE;

done:
    if (!retval)
    {
        /* Unwind previous operations in case of failure.
         *
         */
        if (ops & DW_OPERATION_BIND_DWINTF)
        {
            if (!NETIO_Interface_unbind(dw->dw_intf,
                                         &dst_reader,NULL,route_existed))
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND);

            }
        }

        if (ops & DW_OPERATION_RESOLVER_ADD_ROUTE)
        {
            if (!NETIO_RouteResolver_delete_route(
                                datawriter->config->route_resolver,
                                (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                                &dst_reader,NULL,&found_route))
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND);
            }
        }

        if (ops & DW_OPERATION_ADD_ROUTE_DWINTF)
        {
            if (!NETIO_Interface_delete_route((NETIO_Interface_T*)dw->dw_intf,
                                  &dst_reader,dw->rtps_intf,&dst_reader,NULL))
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_RTPS_NETIO_KIND);
            }
        }

        /* The following operations do not require any action as these are
         * already cleaned up in case of failure.
         * DW_OPERATION_ADD_ROUTE_UC
         * DW_OPERATION_ADD_ROUTE_MC
         * DW_OPERATION_BIND_UC
         * DW_OPERATION_BIND_MC
         */

        if (ops & DW_OPERATION_BIND_RTPS)
        {
            if (!NETIO_Interface_unbind(dw->rtps_intf,&dst_reader,NULL,NULL))
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_RTPS_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND);
            }
        }

        if (NETIO_Interface_get_external_interface(dw->dw_intf,&dst_reader,
                                                   &ext_intf,&from_address))
        {
            if (ops & DW_OPERATION_BIND_EXTERNAL_RTPS)
            {
                if (!NETIO_Interface_unbind_external(dw->rtps_intf,&dst_reader,
                                                     ext_intf,&dst_reader,NULL))
                {
                    DDSC_LOG_NETIO_UNBIND_EXTERNAL(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_RTPS_NETIO_KIND,
                                          DDSC_LOG_DATAREADER_NETIO_KIND);
                }
            }
        }

        *route_existed = RTI_FALSE;
    }

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to add route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)

        DDSC_LOG_NETIO_ADD_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else if (!*route_existed)
    {
        OSAPI_TRACE_DDS("datawriter added route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(dw->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;

#undef DW_OPERATION_NONE
#undef DW_OPERATION_BIND_DWINTF
#undef DW_OPERATION_RESOLVER_ADD_ROUTE
#undef DW_OPERATION_ADD_ROUTE_DWINTF
#undef DW_OPERATION_BIND_EXTERNAL_RTPS
#undef DW_OPERATION_BIND_EXTERNAL_RTPS
#undef DW_OPERATION_ADD_ROUTE_UC
#undef DW_OPERATION_ADD_ROUTE_MC
#undef DW_OPERATION_BIND_RTPS
#undef DW_OPERATION_BIND_UC
#undef DW_OPERATION_BIND_MC
}

/*ci
 * \brief Delete a route from a datawriter to a datareader based on a locator
 *        sequence
 *
 * \param[in]  dw             Datawriter
 * \param[in]  locator_seq    The locator sequence
 * \param[in]  dst_reader     The address of the datareader
 *
 * \return DDS_BOOLEAN_TRUE if successful, DDS_BOOLEAN_FALSE otherwise
 *
 * \sa \ref DDS_DataWriter_delete_route
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataWriter_delete_route_from_locator(struct DDS_DataWriterImpl *dw,
                                         const struct DDS_LocatorSeq *locator_seq,
                                         struct NETIO_Address *dst_reader)
{
    DDS_Long i,j;
    struct NETIO_Address via_address;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        via_address = *(struct NETIO_Address*)
                                DDS_LocatorSeq_get_reference(locator_seq,i);
        if (NETIO_Address_get_kind(&via_address) != NETIO_ADDRESS_KIND_NOTIF)
        {
            if (!NETIO_RouteResolver_delete_route(dw->config->route_resolver,
                    (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                    &via_address,NULL,NULL))
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Remove a datawriter as a listener to a datareader from a locator sequence
 *
 * \details
 *
 * This is a helper function to remove a datawriter as a listener to data from
 * a datareader using a bind resolver to find an interface which can
 * listen to the datareader via the locators
 *
 * \param[in] dw             Datawriter
 * \param[in] locator_seq    Sequence of routes to remove
 * \param[in] recv_kind      The type of route
 * \param[in] ext_intf       The interface to unbind from
 * \param[in] dst_reader     The address of the reader to stop listenting to
 *
 * \return DDS_BOOLEAN_TRUE if successful, DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DDS_DataWriter_unbind_from_locator(struct DDS_DataWriterImpl *dw,
                                   const struct DDS_LocatorSeq *locator_seq,
                                   NETIO_RouteKind_T recv_kind,
                                   NETIO_Interface_T *ext_intf,
                                   struct NETIO_Address *dst_reader)

{
    DDS_Long i,j;
    struct NETIO_Address from_address;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = *(struct NETIO_Address*)
                          DDS_LocatorSeq_get_reference(locator_seq,i);

        if (!NETIO_BindResolver_unbind(dw->config->bind_resolver,
                (struct REDA_StringSeq*)dw->config->enabled_transports,
                recv_kind,&from_address,
                ext_intf,dst_reader,NULL,NULL))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Delete a route from a datawriter to a datareader
 *
 * \details
 *
 * Delete unicast and multicast routes to a datareader from a datawriter.
 *
 * \param[in]  datawriter     Datawriter
 * \param[in]  key            The datareader key
 * \param[in]  uc_locator     The unicast locators to the datareader
 * \param[in]  mc_locator     The multicast locators to the datareader
 * \param[out] route_existed  RTI_TRUE if a route to the reader already existed
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref DDS_DataWriter_delete_route
 */
DDS_Boolean
DDS_DataWriter_delete_route(DDS_DataWriter *datawriter,
                            const DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator,
                            RTI_BOOL *route_existed)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    struct DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_InstanceHandle_t ih;
    struct NETIO_Address dst_reader = NETIO_Address_INITIALIZER;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    NETIO_Interface_T *ext_intf = NULL;
    struct NETIO_Address from_address;
    NETIO_RouteKind_T recv_kind;
    struct DDS_LocatorSeq *dw_uc_locator_seq;
    struct DDS_LocatorSeq *dw_mc_locator_seq;
    RTI_BOOL found_route = RTI_FALSE;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dw_key) &&
            DDS_ObjectId_is_builtin(dw->as_entity.entity_id))
    {
        *route_existed = RTI_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    if (DDS_ObjectId_is_builtin(dw->as_entity.entity_id))
    {
        recv_kind = NETIO_ROUTEKIND_META;
        dw_uc_locator_seq = dw->config->default_meta_unicast;
        dw_mc_locator_seq = dw->config->default_meta_multicast;
    }
    else
    {
        recv_kind = NETIO_ROUTEKIND_USER;
        dw_uc_locator_seq = &dw->uc_locator_seq;
        if (DDS_LocatorSeq_get_length(dw_uc_locator_seq) == 0)
        {
            dw_uc_locator_seq = dw->config->default_unicast;
        }
        dw_mc_locator_seq = dw->config->default_multicast;
    }

    NETIO_Address_set_guid_from_key(&dst_reader,0,
                                    (struct NETIO_AddressInt32*)key);

    OSAPI_TRACE_DDS("datawriter deleting route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)

    if (DB_Database_lock(dw->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!NETIO_Interface_unbind(dw->dw_intf,&dst_reader,NULL,route_existed))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_DATAWRITER_NETIO_KIND,
                              DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!(*route_existed))
    {
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    if (DDS_BuiltinTopicKey_prefix_equals(key,&dw_key))
    {
        if (!NETIO_RouteResolver_delete_route(datawriter->config->route_resolver,
                                (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                                &dst_reader,NULL,&found_route))
        {
            goto done;
        }

        if (found_route)
        {
            retval = DDS_BOOLEAN_TRUE;
            goto done;
        }
        /* Fall through to delete regular communication */
    }

    if (!DDS_DataWriter_delete_notif_interface_route(dw,key,
                                                     uc_locator,&found_route))
    {
        goto done;
    }

    /* Even if a notif interface route is found, the writer may have added
     * other routes which still need to be removed. All other communication
     * paths rely on RTPS. The datawriter creates and established the RTPS
     * interface upon creation, thus it already exists. Delete all routes from
     * the DW and RTPS interface.
     */
    if (!NETIO_Interface_delete_route((NETIO_Interface_T*)dw->dw_intf,
                                      &dst_reader,dw->rtps_intf,&dst_reader,
                                      NULL))
    {
        DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_DATAWRITER_NETIO_KIND,
                                    DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    if (!DDS_DataWriter_delete_route_from_locator(dw,uc_locator,&dst_reader))
    {
        goto done;
    }

    if (!DDS_DataWriter_delete_route_from_locator(dw,mc_locator,&dst_reader))
    {
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dw->rtps_intf,&dst_reader,
                                                &ext_intf,&from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }


    if (!DDS_DataWriter_unbind_from_locator(dw,dw_uc_locator_seq,recv_kind,
                                            ext_intf,&from_address))
    {
        goto done;
    }

    if (!DDS_DataWriter_unbind_from_locator(dw,dw_mc_locator_seq,recv_kind,
                                            ext_intf,&from_address))
    {
        goto done;
    }

    if (!NETIO_Interface_unbind(dw->rtps_intf,&dst_reader,NULL,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_RTPS_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_get_external_interface(dw->dw_intf,&dst_reader,
                                                &ext_intf,&from_address))
    {
        DDSC_LOG_NETIO_GET_EXTERNAL_INTF(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    if (!NETIO_Interface_unbind_external(dw->rtps_intf,&dst_reader,
                                         ext_intf,&dst_reader,NULL))
    {
        DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,DDSC_LOG_RTPS_NETIO_KIND,
                              DDSC_LOG_DATAWRITER_NETIO_KIND)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    if (!retval)
    {
        OSAPI_TRACE_DDS("datawriter failed to delete route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)

        DDSC_LOG_NETIO_DELETE_TOPIC_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDS_TopicDescription_get_name(
                                 DDS_Topic_as_topicdescription(dw->topic)),
                                 dw_key.value[3])
    }
#if OSAPI_ENABLE_TRACE
    else if (*route_existed)
    {
        OSAPI_TRACE_DDS("datawriter deleted route:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",key,RTI_TRUE)
    }
#endif
#endif

    if (DB_Database_unlock(dw->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

/*ci
 * \brief Match a datawriter's publisher with a Subscriber
 *
 * \details
 *
 * The PresentationQos policy has not been fully implemented. This function
 * does the bare minimum to match a subscriber with the writer.
 *
 * \param[in] datawriter The datawriter to match with a reader
 * \param[in] data       SubscriptionBuiltinTopicData
 *
 * \return DDS_BOOLEAN_TRUE if compatible, DDS_BOOLEAN_FALSE if not
 */
RTI_PRIVATE DDS_Boolean
DDS_Publisher_subscriber_is_compatible(DDS_DataWriter *writer,
                   const struct DDS_SubscriptionBuiltinTopicData *const data)
{
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;

    if (!DDS_PresentationQosPolicy_is_compatible(
                            &data->presentation,
                            &DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT))
    {
        writer->off_incompatible_qos_status.last_policy_id = DDS_PRESENTATION_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    return retval;
}

/*ci
 * \brief Match a datawriter with a datareader
 *
 * \param[in] datawriter The datawriter to match with a reader
 * \param[in] key        The key of the datareader to match with the datawriter
 * \param[in] qos        The qos of the datareader to match with the datawriter
 * \param[in] uc_locator Sequence of unicast addresses to the datareader
 * \param[in] mc_locator Sequence of multicast addresses to the datareader
 * \param[in] encapsulation Type of encapsulation used by datareader
 * \param[in] representation Type of data representation used by datareader
 *
 * \sa \ref DDS_DataWriter_unmatch_reader
 */
void
DDS_DataWriter_match_reader(DDS_DataWriter *datawriter,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_DataReaderQos *const qos,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator,
                            DDS_EncapsulationId_t encapsulation,
                            DDS_DataRepresentationId_t representation)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    RTI_BOOL route_existed = RTI_FALSE;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    if (!DDS_DataWriter_reader_is_compatible(datawriter,qos))
    {
        retval = DDS_DataWriter_delete_route(
                    dw,key,uc_locator,mc_locator,&route_existed);
        if (retval)
        {
#if DDS_FILTERING_ENABLED
            if (DDS_DataWriter_is_filtering_enabled(datawriter))
            {
                DDS_DataWriter_delete_reader_filter(datawriter, key);
            }
#endif /* DDS_FILTERING_ENABLED */

            if (route_existed)
            {
                DDS_DataWriterEvent_on_publication_matched(
                        dw,key,DDS_BOOLEAN_TRUE,DDS_BOOLEAN_FALSE);
            }
            DDS_DataWriterEvent_on_incompatible_qos(dw,key);
        }

        return;
    }

    OSAPI_TRACE_TRUST_DR_READY(
            &dw->rtps_intf->local_address.value.guid,
            key)

    retval = DDS_DataWriter_add_route(dw,key,qos,
                                      uc_locator,
                                      mc_locator,
                                      encapsulation,
                                      representation,
                                      &route_existed);
    if (!route_existed && retval)
    {
        DDS_DataWriterEvent_on_publication_matched(
                dw, key, DDS_BOOLEAN_FALSE, DDS_BOOLEAN_TRUE);
    }
}

/*ci
 * \brief Unmatch a datawriter from a datareader
 *
 * \param[in] datawriter The datawriter to unmatch from a reader
 * \param[in] key        The key of the reader to unmatch from the writer
 * \param[in] uc_locator Sequence of unicast addresses to the datareader
 * \param[in] mc_locator Sequence of multicast addresses to the datareader
 *
 * \sa \ref DDS_DataWriter_match_reader
 */
void
DDS_DataWriter_unmatch_reader(DDS_DataWriter *datawriter,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator)
{
    DDS_Boolean retval;
    RTI_BOOL route_existed = RTI_FALSE;

    if (datawriter->management.is_anonymous)
    {
        return;
    }

    retval = DDS_DataWriter_delete_route(
                datawriter,key,uc_locator,mc_locator,&route_existed);
    if (retval)
    {

#if DDS_FILTERING_ENABLED
        if (DDS_DataWriter_is_filtering_enabled(datawriter))
        {
            DDS_DataWriter_delete_reader_filter(datawriter, key);
        }
#endif /* DDS_FILTERING_ENABLED */

         if (route_existed)
         {
             DDS_DataWriterEvent_on_publication_matched(
                     datawriter,key,DDS_BOOLEAN_TRUE,DDS_BOOLEAN_FALSE);
         }
    }
}

RTI_PRIVATE DDS_DataRepresentationId_t
DDS_DataWriter_resolve_data_representation(DDS_DataWriter *dw,
                   const struct DDS_DataRepresentationQosPolicy *policy)
{
    RTI_INT32 seq_length;
    RTI_INT32 seq_index;
    DDS_DataRepresentationId_t dr_id;

    if (DDS_DataRepresentationIdSeq_get_length(&policy->value) == 0)
    {
        if (DDS_TypePlugin_is_representation_enabled(dw->type_plugin,
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

            if (DDS_TypePlugin_is_representation_enabled(dw->type_plugin,dr_id))
            {
                return dr_id;
            }
        }
    }

    return DDS_INVALID_DATA_REPRESENTATION;
}

/*ci
 * \brief Resolve which unicast and multicast locators should be used to reach
 *        a local datareader
 *
 * \details
 *
 * If a datareader specifies its own locators those are used instead of the
 * default locators specified on the participant.
 *
 * \param[in]  datawriter     Datawriter to resolve for
 * \param[in]  key            Datareader key
 * \param[in]  qos            Datareader qos
 * \param[out] uc_locator_seq The unicast locators to use for the datareader
 * \param[out] mc_locator_seq The multicast locators to use for the datareader
 */
RTI_PRIVATE RTI_BOOL
DDS_DataWriter_resolve_local_locators(DDS_DataWriter *datawriter,
                                      struct DDS_BuiltinTopicKey_t *key,
                                      const DDS_DataReader *const reader,
                                      const struct DDS_LocatorSeq **uc_locator_seq,
                                      const struct DDS_LocatorSeq **mc_locator_seq)
{
    if (DDS_BuiltinTopicKey_is_builtin(key))
    {
        *uc_locator_seq = datawriter->config->default_meta_unicast;
        *mc_locator_seq = datawriter->config->default_meta_multicast;
    }
    else
    {
        *uc_locator_seq = DDS_DataReader_get_unicast_locator_ref(reader);
        if (DDS_LocatorSeq_get_length(*uc_locator_seq) == 0)
        {
            *uc_locator_seq = datawriter->config->default_unicast;
        }

        *mc_locator_seq = DDS_DataReader_get_multicast_locator_ref(reader);
        if (DDS_LocatorSeq_get_length(*mc_locator_seq) == 0)
        {
            *mc_locator_seq = datawriter->config->default_multicast;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_DataWriter_is_valid_transport_encapsulation(struct DDS_TypePlugin *const plugin,
                                                const struct DDS_TransportEncapsulationSettings_t *const ts,
                                                DDS_DataRepresentationId_t dr_id,
                                                DDS_EncapsulationId_t enc)
{
    RTI_INT32 e_length;
    RTI_INT32 e_index;

    if (ts == NULL)
    {
        /* Check against all the supported encapsulations for this writer */
        if (DDS_TypePlugin_find_encapsulation_plugin(plugin,enc,dr_id) != NULL)
        {
            return RTI_TRUE;
        }
        return RTI_FALSE;
    }

    /* Check only the listed subset of encapsulations */
    e_length = DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);
    for (e_index = 0; e_index < e_length; e_index++)
    {
        DDS_EncapsulationId_t *e_value_ref =
            DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,e_index);
        if (e_value_ref == NULL)
        {
            return RTI_FALSE;
        }

        if ((enc == *e_value_ref) &&
            (DDS_TypePlugin_find_encapsulation_plugin(plugin,enc,dr_id) != NULL))
        {
            return RTI_TRUE;
        }
    }

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
DDS_DataWriter_resolve_peer_locators(
            const struct DDS_ParticipantBuiltinTopicData *const parent_data,
            const struct DDS_SubscriptionBuiltinTopicData *const data,
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
        /* static cast to the basic struct DDS_LocatorSeq. This is just to be
         * able to pass the pointer to common functions. The content is
         * not accessed.
         */
        *uc_locator_seq = OSAPI_Compiler_reinterpret_cast(
                                    struct DDS_LocatorSeq*,
                                    &data->unicast_locator);

        if (DDS_LocatorSeq_get_length(*uc_locator_seq) == 0)
        {
            *uc_locator_seq = &parent_data->default_unicast_locators;
        }

        *mc_locator_seq = &data->multicast_locator;
        if (DDS_LocatorSeq_get_length(*mc_locator_seq) == 0)
        {
            *mc_locator_seq = &parent_data->default_multicast_locators;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_DataWriter_resolve_transports(DDS_DataWriter *dw,
                                  const struct DDS_LocatorSeq *in_seq,
                                  struct DDS_LocatorSeq *reslvd_seq,
                                  DDS_EncapsulationId_t *encapsulation,
                                  DDS_DataRepresentationId_t representation)
{
    RTI_INT32 loc_length;
    RTI_INT32 loc_index;
    RTI_INT32 enc_index;
    RTI_INT32 cur_loc_kind = NETIO_ADDRESS_KIND_INVALID;
    RTI_INT32 current_prio = NETIO_DEFAULT_PRIORITY;
    RTI_INT32 loc_prio;
    RT_ComponentFactoryId_T intf_name;
    struct DDS_TransportEncapsulationSettings_t *ts = NULL;
    RTI_BOOL is_reachable = RTI_FALSE;
    UNUSED_ARG(dw);

    /* The input sequence can be a basic locator or an extended locator. The
     * length field is in the same place for both types, so we can read it
     * without knowing the type of the sequence.
     */
    loc_length = DDS_LocatorSeq_get_length(in_seq);

    /* Nothing to resolve */
    if (loc_length == 0)
    {
        return RTI_TRUE;
    }

    for (loc_index = 0; loc_index < loc_length; ++loc_index)
    {
        struct NETIO_AddressEx addr_ex = NETIO_AddressEx_INITIALIZER;
        const struct DDS_LocatorEx *a_loc_data_ex = NULL;
        struct DDS_Locator basic_loc = DDS_LOCATOR_INVALID;
        struct DDS_LocatorExSeq *extended_loc_seq = NULL;

        /* This function uses the size of the element and is thus type agnostic
         * copy out the basic locator to be able to determine the
         * transport priority.
         */
        if (DDS_LocatorSeq_is_extended(in_seq))
        {
            extended_loc_seq = OSAPI_Compiler_reinterpret_cast(
                                                struct DDS_LocatorExSeq*,
                                                in_seq);
            a_loc_data_ex = DDS_LocatorExSeq_get_reference(
                                                extended_loc_seq,loc_index);
            DDS_Locator_from(&basic_loc,a_loc_data_ex);
        }
        else
        {
            basic_loc = *DDS_LocatorSeq_get_reference(in_seq,loc_index);
        }

        loc_prio = DDS_Transport_get_locator_priority(&basic_loc);

        if (loc_prio > current_prio)
        {
            /* Already have a locator with higher priority, continue; */
            continue;
        }

        /* Found a potentially new locator, check if it is supported */
        if (!DDS_Locator_get_interface(&basic_loc,&intf_name,
                                       dw->config->route_resolver,
                                       dw->config->addr_resolver))
        {
            /* This should not happen since unsupported locators are dropped
             * during discovery.
             */
            continue;
        }

        /* The transport may be supported, check if this particular address
         * can be reached. is_address_reachable must be passed a pointer to
         * struct NETIO_Address*, but the structure must be
         * struct NETIO_AddressEx*.
         */
        NETIO_AddressEx_from(&addr_ex,&basic_loc);

        if (!NETIO_RouteResolver_is_address_reachable(
                                        dw->config->route_resolver,&addr_ex,
                                        &is_reachable))
        {
            OSAPI_TRACE_PRINTF1("Failed to determine if %A is reachable, skipping\n",
                                 &addr_ex);
            continue;
        }

        if (!is_reachable)
        {
            OSAPI_TRACE_PRINTF1("Locator %A is not reachable, skipping\n",
                                 &addr_ex);
            continue;
        }

        /* This check is necessary for cases where the writer has enabled both
         * V1 and Zcopy V2, and a reader is advertising V2 locators. In this
         * scenario, the writer only supports V1, but the V2 interface still
         * exists. Additionally, the reachability check below will succeed
         * because the reader has created resources for V2. Therefore, we
         * explicitly verify if the writer is using V2.
         */
        if ((NETIO_Address_kind(basic_loc.kind) == NETIO_ADDRESS_KIND_NOTIF)
            && !DDS_TypePlugin_allocator_plugin_is_v2(dw->type_plugin))
        {
            continue;
        }

        if ((a_loc_data_ex == NULL) || (a_loc_data_ex->length == 0))
        {
            /* The reader did not request any particular encapsulations.
             * Use the default for the data representation determined before
             * this function was called.
             */
            cur_loc_kind = basic_loc.kind;
            current_prio = loc_prio;
            continue;
        }

        /* The reader requested particular encapsulations, in order of
         * priority.
         */
        ts = DDS_TransportEncapsulationQosPolicy_find_transport_setting(
                                                   dw->encapsulation,
                                                   &intf_name);

        for (enc_index = 0; enc_index < a_loc_data_ex->length; ++enc_index)
        {
            if (DDS_DataWriter_is_valid_transport_encapsulation(
                            dw->type_plugin,
                            ts,
                            representation,
                            a_loc_data_ex->encapsulations[enc_index]))
            {
                /* A match was found, set the new locator and encapsulation */
                cur_loc_kind = basic_loc.kind;
                current_prio = loc_prio;
                *encapsulation = a_loc_data_ex->encapsulations[enc_index];
                break;
            }
        }
    }

    /* At this point a locator has been selected based on a selection of
     * matching criterias based on the following order:
     * representation
     * transport
     * encapsulation
     *
     * If cur_loc_kind == NETIO_ADDRESS_KIND_INVALID no supported locators could be found.
     */
    if (cur_loc_kind == NETIO_ADDRESS_KIND_INVALID)
    {
        return RTI_FALSE;
    }

    if (!DDS_Locator_append_locator_kind(in_seq,reslvd_seq,
                                         NETIO_Address_kind(cur_loc_kind)))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Resolve which unicast and multicast locators should be used to reach
 *       a datareader
 */
RTI_PRIVATE RTI_BOOL
DDS_DataWriter_resolve_locators(DDS_DataWriter *dw,
                const struct DDS_BuiltinTopicKey_t *const key,
                const struct DDS_LocatorSeq **uc_locator_seq,
                const struct DDS_LocatorSeq **mc_locator_seq,
                DDS_EncapsulationId_t *encapsulation,
                DDS_DataRepresentationId_t representation)
{
    /* Get the default encapsulation for this data representation unless
     * another one is found based on the TransportQosEncapsulation later.
     */
    *encapsulation = DDS_TypePlugin_match_representation(dw->type_plugin,
                                                         representation);
    if (*encapsulation == DDS_ENCAPSULATION_ID_INVALID)
    {
        return RTI_FALSE;
    }


    /* determine which locators shall be used. This is needed to be able
     * to choose between a prioritized transport, such as shared memory
     * vs UDP and between different encapsulations.
     */
    if (DDS_BuiltinTopicKey_is_builtin(key))
    {
        *encapsulation = DDS_DataWriter_resolve_builtin_encapsulation(key);
    }

    /* uc_locator_seq and mc_locator_seq points to the effective locators.
     * Next determine which locators to actually use based on transport
     * and encapsulation properties.
     */
    if (!DDS_LocatorSeq_set_length(dw->config->resolved_unicast_seq,0))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(dw->config->resolved_multicast_seq,0))
    {
        return RTI_FALSE;
    }

    if (!DDS_DataWriter_resolve_transports(dw,*uc_locator_seq,
                                           dw->config->resolved_unicast_seq,
                                           encapsulation,
                                           representation))
    {
        return RTI_FALSE;
    }

    if (DDS_LocatorSeq_get_length(dw->config->resolved_unicast_seq) == 0)
    {
        /* If no unicat locators were found resolve the multicast
         * cast locators independently.
         */
        if (!DDS_DataWriter_resolve_transports(dw,*mc_locator_seq,
                                               dw->config->resolved_multicast_seq,
                                               encapsulation,
                                               representation))
        {
            return RTI_FALSE;
        }
    }
    else
    {
        /* Append all multicast locators of the same kind */
        if (!DDS_Locator_append_locator_kind(
                    *mc_locator_seq,
                    dw->config->resolved_multicast_seq,
                    NETIO_Address_kind(
                       DDS_LocatorSeq_get_reference(
                            dw->config->resolved_unicast_seq,0)->kind)))
        {
            return RTI_FALSE;
        }
    }

    *uc_locator_seq = dw->config->resolved_unicast_seq;
    *mc_locator_seq = dw->config->resolved_multicast_seq;

    return RTI_TRUE;
}

/*ci
 * \brief
 *
 * \param[in] self        DataWriter to check compatibility with a subscription
 * \param[in] data        The subscription qos
 *
 * \return DDS_BOOLEAN_TRUE if it is compatible, DDS_BOOLEAN_FALSE otherwise
 */
RTI_PRIVATE DDS_Boolean
DDS_Publisher_is_partition_match(DDS_Publisher *publisher,
                const struct DDS_PartitionQosPolicy *data)
{
    if ((data != NULL) &&
        (DDS_PartitionQosPolicy_is_compatible(
                    &publisher->partition->name,
                    &data->name)))
    {
        return DDS_BOOLEAN_TRUE;
    }
    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Match a datawriter with a local datareader
 *
 * \param[in] datawriter  Datawriter to match with a datareader
 * \param[in] key         Key of the datareader
 * \param[in] topic_name  The topic name published by the datareader
 * \param[in] type_name   The type name published by the datareader
 * \param[in] dr          The datareader
 */
void
DDS_DataWriter_match_local_reader(DDS_DataWriter *datawriter,
                                  struct DDS_BuiltinTopicKey_t *key,
                                  const char *topic_name,
                                  const char *type_name,
                                  const DDS_DataReader *dr)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;
    DDS_EncapsulationId_t encapsulation;
    DDS_DataRepresentationId_t representation;
    DDS_Subscriber *subscriber = NULL;
    DDS_Publisher *publisher = NULL;
    struct DDS_DataReaderQos dr_qos = DDS_DataReaderQos_INITIALIZER;
#if DDS_FILTERING_ENABLED
    DDS_Boolean retval;
#endif /* DDS_FILTERING_ENABLED */

    dr_qos.deadline = dr->deadline;
    dr_qos.liveliness = dr->liveliness;
    dr_qos.ownership = dr->ownership;
    dr_qos.reliability = dr->reliability;
    dr_qos.durability = dr->durability;
    dr_qos.destination_order = dr->destination_order;
    dr_qos.representation = *dr->representation;
    dr_qos.latency_budget = dr->latency_budget;

    subscriber = DDS_DataReader_get_subscriber((DDS_DataReader *)dr);
    publisher = DDS_DataWriter_get_publisher(dw);

    if (publisher == NULL || subscriber == NULL)
    {
        return;
    }

    if (!DDS_Publisher_is_partition_match(publisher,
                        DDS_Subscriber_get_partition_ref(subscriber)))
    {
        DDSC_LOG_INCOMPATIBLE_PARTITION_QOS(OSAPI_LOGKIND_INFO, topic_name)
        return;
    }

    if (!DDS_Topic_is_compatible(dw->topic,topic_name,
                                 type_name,
                                 key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID],
                                 datawriter->as_entity.entity_id))
    {
        return;
    }

    representation = DDS_DataWriter_resolve_data_representation(
                            datawriter,
                            DDS_DataReader_get_data_representation_ref(dr));
    if (representation == DDS_INVALID_DATA_REPRESENTATION)
    {
        dw->off_incompatible_qos_status.last_policy_id = DDS_DATA_REPRESENTATION_QOS_POLICY_ID;
        DDS_DataWriterEvent_on_incompatible_qos(dw, key);
        return;
    }

    DDS_DataWriter_resolve_local_locators(datawriter,key,dr,
                                          &uc_locator_seq,
                                          &mc_locator_seq);

    if (!DDS_DataWriter_resolve_locators(dw,
                                         key,
                                         &uc_locator_seq,
                                         &mc_locator_seq,
                                         &encapsulation,
                                         representation))
    {
        return;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_DataWriter_is_filtering_enabled(datawriter))
    {
        retval = DDS_DataWriter_add_local_reader_filter(datawriter,
                                                        key,
                                                        dr->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS,
                                                        dr->compiled_filter);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDSC_LOG_ENABLE_WRITER_FILTERING(OSAPI_LOGKIND_WARNING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
    }
#endif /* DDS_FILTERING_ENABLED */

    DDS_DataWriter_match_reader(datawriter,
                                key,
                                &dr_qos,
                                uc_locator_seq,
                                mc_locator_seq,
                                encapsulation,
                                representation);
}

/*ci
 * \brief Determine if a Micro reader is compatible with the padding bits
 *
 * \param[in] self        Datawriter to match with the remote subscription
 * \param[in] parent_data The remote participant data
 *
 * \return DDS_BOOLEAN_TRUE on if the reader is compatible, DDS_BOOLEAN_FALSE
 *         otherwise.
 */
RTI_PRIVATE DDS_Boolean
DDS_DataWriter_is_reader_padding_compatible(
        DDS_DataWriter *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl*)self;
    RTI_INT32 major, minor, release, revision;


    if (!dw->set_cdr_options_padding)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* Assume that all vendors except Micro are compatible
     */
    if (parent_data->rtps_vendor_id.vendorId[0] != RTPS_VENDOR_ID_MAJOR)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (parent_data->rtps_vendor_id.vendorId[1] != RTPS_VENDOR_ID_MINOR)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* The VendorID is Micro  */

    /* Static cast to unsigned char to avoid cert_str34_c_violation */
    major = (unsigned char) parent_data->product_version.major;
    minor = (unsigned char) parent_data->product_version.minor;
    release = (unsigned char) parent_data->product_version.release;
    revision = (unsigned char) parent_data->product_version.revision;

    if (major > 4)
    {
        /* Assume Micro 5+ can handle padding */
        return DDS_BOOLEAN_TRUE;
    }

    if ((major == 4) && !((minor == 0) && (release == 0) && (revision == 0)))
    {
        /* Micro 4 except 4.0.0.0 can handle padding */
        return DDS_BOOLEAN_TRUE;
    }

    /* Padding may be possible, fail for Micro 3 and Micro 4.0.0.0 */
    if (major >= 3)
    {
        /* Micro 3 cannot parse padding bits so we are incompatible */
        goto not_compatible;
    }

    /*
     * Major version <= 2. If there any future versions of Micro 2.x
     * where x > 4, they must be compatible and support interpreting padding
     * bits.
     */

    /* Due to MICRO-1682 we have to swap revision and release */
    release = (unsigned char) parent_data->product_version.revision;
    revision = (unsigned char) parent_data->product_version.release;

    if (minor <= 3)
    {
        goto not_compatible;
    }

    if (minor == 4)
    {
        if ((release == 15) && (revision == 1))
        {
            goto not_compatible;
        }

        if ((release == 14) && (revision <= 1))
        {
            goto not_compatible;
        }

        if ((release == 13) && (revision <= 5))
        {
            goto not_compatible;
        }

        if (release <= 12)
        {
            goto not_compatible;
        }
    }

    return DDS_BOOLEAN_TRUE;

not_compatible:

    DDSC_LOG_DW_INCOMPATIBLE_PADDING(OSAPI_LOGKIND_WARNING,
                                      major,minor,release,revision);

    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Match a datawriter with a remote subscription
 *
 * \param[in] self        Datawriter to match with the remote subscription
 * \param[in] parent_data The remote participant qos
 * \param[in] data        The remote subscription qos
 */
void
DDS_DataWriter_match_remote_reader(DDS_DataWriter *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)self;
    struct DDS_DataReaderQos dr_qos = DDS_DataReaderQos_INITIALIZER;
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;
    DDS_EncapsulationId_t encapsulation;
    DDS_DataRepresentationId_t representation;
    DDS_Publisher *pub = NULL;
#if DDS_FILTERING_ENABLED
    DDS_Boolean retval;
#endif /* DDS_FILTERING_ENABLED */

    if (!DDS_DataWriter_is_reader_padding_compatible(dw,parent_data))
    {
        return;
    }

    /* SubscriptionBuiltinTopicData is a subset of the DataReaderQos. Note that
     * the dr_qos is a shallow copy for pointer types (if any) since these
     * are only used in this function and it avoids memory allocations. Thus,
     * the dr_qos is not finalized.
     */
    dr_qos.deadline = data->deadline;
    dr_qos.liveliness = data->liveliness;
    dr_qos.ownership = data->ownership;
    dr_qos.reliability = data->reliability;
    dr_qos.durability = data->durability;
    dr_qos.destination_order = data->destination_order;
    dr_qos.representation = data->representation;
    dr_qos.latency_budget = data->latency_budget;

    pub = DDS_DataWriter_get_publisher(dw);
    if (pub == NULL)
    {
        goto done;
    }

    if (!DDS_Publisher_is_partition_match(pub, &data->partition))
    {
        DDSC_LOG_INCOMPATIBLE_PARTITION_QOS(OSAPI_LOGKIND_INFO, data->topic_name)
        goto done;
    }

    /* If the topic is not compatible it is not an incompatible qos */
    if (!DDS_Topic_is_compatible(dw->topic,data->topic_name,
                                 data->type_name,
                                 data->key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID],
                                 dw->as_entity.entity_id))
    {
        goto done;
    }

    if (!DDS_Publisher_subscriber_is_compatible(dw,data))
    {
        DDS_DataWriterEvent_on_incompatible_qos(dw,&data->key);
        goto done;
    }

    representation = DDS_DataWriter_resolve_data_representation(dw, &data->representation);
    if (representation == DDS_INVALID_DATA_REPRESENTATION)
    {
        dw->off_incompatible_qos_status.last_policy_id = DDS_DATA_REPRESENTATION_QOS_POLICY_ID;
        DDS_DataWriterEvent_on_incompatible_qos(dw, &data->key);
        goto done;
    }

    if (!DDS_DataWriter_resolve_peer_locators(parent_data,data,
                                              &uc_locator_seq,
                                              &mc_locator_seq))
    {
        goto done;
    }

    if (!DDS_DataWriter_resolve_locators(self,
                                         &data->key,
                                         &uc_locator_seq,
                                         &mc_locator_seq,
                                         &encapsulation,
                                         representation))
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_DataWriter_is_filtering_enabled(self))
    {
        retval = DDS_DataWriter_add_remote_reader_filter(self, data);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDSC_LOG_ENABLE_WRITER_FILTERING(OSAPI_LOGKIND_WARNING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
    }
#endif /* DDS_FILTERING_ENABLED */

    DDS_DataWriter_match_reader(self,
                                &data->key,
                                &dr_qos,
                                uc_locator_seq,
                                mc_locator_seq,
                                encapsulation,
                                representation);

done:
    return;
}

#ifndef RTI_CERT
/*ci
 * \brief Unmatch a datawriter from a local datareader
 *
 * \param[in] self Datawriter to unmatch with a datareader
 * \param[in] key  Key of the datareader to unmatch from
 * \param[in] reader  The datareader being unmatched
 *
 * \sa \ref DDS_DataWriter_match_local_reader
 */
void
DDS_DataWriter_unmatch_local_reader(DDS_DataWriter *self,
                                    struct DDS_BuiltinTopicKey_t *key,
                                    DDS_DataReader *const reader)
{
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;

    DDS_DataWriter_resolve_local_locators(self,key,reader,
                                          &uc_locator_seq,
                                          &mc_locator_seq);

    DDS_DataWriter_unmatch_reader(self,key,uc_locator_seq,mc_locator_seq);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Unmatch a datawriter from a remote subscroption
 *
 * \param[in] self        Datawriter to unmatch with a datareader
 * \param[in] parent_data The remote participant qos
 * \param[in] data        The remote subscription qos
 *
 * \sa \ref DDS_DataWriter_match_remote_reader
 */
void
DDS_DataWriter_unmatch_remote_reader(DDS_DataWriter *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data)
{
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)self;

    if (!DDS_DataWriter_resolve_peer_locators(parent_data,
                                              data,
                                              &uc_locator_seq,
                                              &mc_locator_seq))
    {
        return;
    }

    /* Convert to a regular sequence, the encapsulation is not needed. */
    if (DDS_LocatorSeq_is_extended(uc_locator_seq))
    {
        struct DDS_LocatorExSeq *extended_loc_seq =
                    OSAPI_Compiler_reinterpret_cast(
                                            struct DDS_LocatorExSeq*,
                                            uc_locator_seq);
        if (!DDS_LocatorSeq_copy_from(
                dw->config->resolved_unicast_seq,
                extended_loc_seq))
        {
            return;
        }
        uc_locator_seq = dw->config->resolved_unicast_seq;
    }

    /* For unmatching we do not need the encapsulation */
    DDS_DataWriter_unmatch_reader(self,&data->key,
                                  uc_locator_seq,
                                  mc_locator_seq);
}

/*ci @} */
