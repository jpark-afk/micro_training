/*
 * FILE: DataWriterDiscovery.c - DataWriter discovery implementation
 *
 * (c) Copyright 2008-2021 Real-Time Innovations,
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
 * 13dec2021,tk MICRO-3362/PR.30006
 * - Moved setting dwb_property in DDS_DataWriter_add_anonymous_peer inside
 *   the critical section. Not all properties need protection, but for for
 *   clarity all are set within the critical section.
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Corrected parameter name from datawriter to writer in
 *   DDS_Publisher_subscriber_is_compatible
 * - Removed duplicate DW_OPERATION_BIND_EXTERNAL_RTPS in
 *   DDS_DataWriter_add_route.
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in DDS_DataWriter_add_route_from_locator()
 *  - Removed empty blocks in DDS_DataWriter_bind_from_locator
 *  - Removed empty blocks in DDS_DataWriter_add_route
 * 06apr2021,tk MICRO-2929/PR.28869
 *  - Set default value in DDS_DataWriter_add_anonymous_peer() to FALSE
 *    and TRUE on success.
 * 24feb2021,tk MICRO-2918/PR#28777
 *   - Moved ops |= DW_OPERATION_RESOLVER_ADD_ROUTE in DDS_DataWriter_add_route
 *     to inside the if-test.
 *   - Changed order for reverting DW_OPERATION_BIND_EXTERNAL_RTPS
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
#include "TopicDescription.h"
#include "Topic.h"
#include "DataWriterImpl.h"
#include "DataWriterEvent.h"
#include "DataWriterQos.h"
#include "DataWriterInterface.h"
#include "DataWriterDiscovery.h"
#include "PublisherEvent.h"

/*** SOURCE_BEGIN ***/

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

    if (!DDS_DeadlineQosPolicy_is_compatible(&dr_qos->deadline,
                                             &dw->qos.deadline))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DEADLINE_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_compatible(&dr_qos->ownership,
                                              &dw->qos.ownership))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_OWNERSHIP_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_compatible(&dr_qos->reliability,
                                                &dw->qos.reliability))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_RELIABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_compatible(&dr_qos->liveliness,
                                               &dw->qos.liveliness))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_LIVELINESS_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_compatible(&dr_qos->durability,
                                               &dw->qos.durability))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DURABILITY_QOS_POLICY_ID;
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_compatible(&dr_qos->destination_order,
                                                     &dw->qos.destination_order))
    {
        datawriter->off_incompatible_qos_status.last_policy_id = DDS_DESTINATIONORDER_QOS_POLICY_ID;
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
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DDS_DataWriterBindProperty dwb_property =
                                        DDS_DataWriterBindProperty_INITIALIZER;

#if OSAPI_ENABLE_TRACE || OSAPI_ENABLE_LOG
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dw_key;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif


    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    dwb_property.next_new_sn = dw->last_sn;
    REDA_SequenceNumber_plusplus(&dwb_property.next_new_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.low_history_sn);
    REDA_SequenceNumber_set_zero(&dwb_property.high_history_sn);
    dwb_property.is_reliable = DDS_BOOLEAN_FALSE;

    if (!NETIO_Interface_add_route((NETIO_Interface_T*)dw->dw_intf,
                                   dst_reader,dw->rtps_intf,
                                   dst_reader,NULL,NULL))
    {
        DDSC_LOG_NETIO_ADD_ROUTE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAWRITER_NETIO_KIND,
                                 DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

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
                                NETIO_ROUTEKIND_META,address,NULL,NULL,NULL))
    {
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
    DDS_BuiltinTopicKey_t dw_key;

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

#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dw_key;

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);
#endif

    OSAPI_TRACE_DDS("datawriter adding route:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                       DDS_Topic_as_topicdescription(dw->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dw_key,RTI_TRUE)

    if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!NETIO_Interface_add_route((NETIO_Interface_T*)dw->dw_intf,
                                   dst_reader,dw->rtps_intf,
                                   dst_reader,NULL,NULL))
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

    if (!NETIO_RouteResolver_add_route(datawriter->config->route_resolver,
            (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
            via_address,NULL,NULL,NULL))
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

#ifndef RTI_CERT
DDS_Boolean
DDS_DataWriter_delete_anonymous_route(DDS_DataWriter *datawriter,
                                      struct NETIO_Address *dst_reader,
                                      struct NETIO_Address *via_address)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;
#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dw_key;

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
#endif /* !RTI_CERT */

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
    RTI_BOOL brc;

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

            brc = NETIO_RouteResolver_delete_route(dw->config->route_resolver,
                               (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                               &via_address,NULL,NULL);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                            DDSC_LOG_DATAWRITER_NETIO_KIND,
                                            DDSC_LOG_DATAREADER_NETIO_KIND)
            }
#else
            IGNORE_RETVAL(brc);
#endif
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
    RTI_BOOL brc;

    j = DDS_LocatorSeq_get_length(locator_seq);
    for (i = 0; i < j; i++)
    {
        from_address = *(struct NETIO_Address*)
                          DDS_LocatorSeq_get_reference(locator_seq,i);

        if (!NETIO_BindResolver_bind(dw->config->bind_resolver,
                (struct REDA_StringSeq*)dw->config->enabled_transports,
                recv_kind,&from_address,
                ext_intf,dst_reader,NULL,NULL))
        {
            break;
        }
    }

    if (i < j)
    {
        for (k = 0; k < i; k++)
        {
            from_address = *(struct NETIO_Address*)
                                  DDS_LocatorSeq_get_reference(locator_seq,k);

            brc = NETIO_BindResolver_unbind(dw->config->bind_resolver,
                        (struct REDA_StringSeq*)dw->config->enabled_transports,
                        recv_kind,&from_address,
                        ext_intf,dst_reader,NULL,NULL);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)
            }
#else
            IGNORE_RETVAL(brc);
#endif
        }
        
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
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
    struct NETIO_Address dst_reader;
    DDS_InstanceHandle_t ih;
    DDS_BuiltinTopicKey_t dw_key;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTPS_RouteProperty route_property;
    struct DDS_DataWriterBindProperty dwb_property =
                                        DDS_DataWriterBindProperty_INITIALIZER;
    struct DDSHST_WriterState *wh_state;
    struct NETIO_Address from_address;
    NETIO_Interface_T *ext_intf;
    struct DDS_LocatorSeq *dw_uc_locator_seq;
    struct DDS_LocatorSeq *dw_mc_locator_seq;
    NETIO_RouteKind_T recv_kind;
    RTI_BOOL found_route;
    RTI_UINT32 ops = DW_OPERATION_NONE;
    RTI_BOOL brc;

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
        if (!NETIO_RouteResolver_add_route(datawriter->config->route_resolver,
                                (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                                &dst_reader,NULL,
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
        ops |= DW_OPERATION_RESOLVER_ADD_ROUTE;
    }

    /* All other communication paths relies on RTPS. The datawriter
     * creates and established the RTPS interface upon creation, thus
     * it already exists. All we need to do is add a route DW->RTPS and
     * a route from RTPS-> requested DR interface.
     */
    route_property.reliable =
                      (qos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

    /*
     * Set peer's First and Last SN, its send window, according to the
     * Durability and History Qos policies.
     */
    route_property.first_sn = wh_state->low_sn;
    route_property.last_sn = wh_state->high_sn;

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

    if (!NETIO_Interface_add_route(
                        (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                        dw->rtps_intf,&dst_reader,NULL,NULL))
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

    if (!NETIO_Interface_bind(dw->rtps_intf,&dst_reader,NULL,NULL))
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
            brc = NETIO_Interface_unbind(dw->dw_intf,
                                         &dst_reader,NULL,route_existed);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)

            }
#else
            IGNORE_RETVAL(brc);
#endif
        }

        if (ops & DW_OPERATION_RESOLVER_ADD_ROUTE)
        {
            brc = NETIO_RouteResolver_delete_route(
                    datawriter->config->route_resolver,
                    (NETIO_Interface_T*)dw->dw_intf,&dst_reader,
                    &dst_reader,NULL,&found_route);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)
            }
#else
            IGNORE_RETVAL(brc);
#endif
        }

        if (ops & DW_OPERATION_ADD_ROUTE_DWINTF)
        {
            brc = NETIO_Interface_delete_route((NETIO_Interface_T*)dw->dw_intf,
                                   &dst_reader,dw->rtps_intf,&dst_reader,NULL);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_DATAWRITER_NETIO_KIND,
                                      DDSC_LOG_RTPS_NETIO_KIND)
            }
#else
            IGNORE_RETVAL(brc);
#endif
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
            brc = NETIO_Interface_unbind(dw->rtps_intf,&dst_reader,NULL,NULL);
#if OSAPI_ENABLE_LOG
            if (!brc)
            {
                DDSC_LOG_NETIO_UNBIND(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_RTPS_NETIO_KIND,
                                      DDSC_LOG_DATAREADER_NETIO_KIND)
            }
#else
            IGNORE_RETVAL(brc);
#endif
        }

        if (ops & DW_OPERATION_BIND_EXTERNAL_RTPS)
        {
            if (NETIO_Interface_get_external_interface(dw->dw_intf,&dst_reader,
                                                       &ext_intf,&from_address))
            {
                brc = NETIO_Interface_unbind_external(dw->rtps_intf,&dst_reader,
                                                  ext_intf,&dst_reader,NULL);
#if OSAPI_ENABLE_LOG
                if (!brc)
                {
                    DDSC_LOG_NETIO_UNBIND_EXTERNAL(OSAPI_LOGKIND_ERROR,
                                                   DDSC_LOG_RTPS_NETIO_KIND,
                                                   DDSC_LOG_DATAREADER_NETIO_KIND)
                }
#else
            IGNORE_RETVAL(brc);
#endif
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
        if (!NETIO_RouteResolver_delete_route(dw->config->route_resolver,
                (NETIO_Interface_T*)dw->rtps_intf,dst_reader,
                &via_address,NULL,NULL))
        {
            return DDS_BOOLEAN_FALSE;
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
    DDS_BuiltinTopicKey_t dw_key;
    DDS_InstanceHandle_t ih;
    struct NETIO_Address dst_reader;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    NETIO_Interface_T *ext_intf;
    struct NETIO_Address from_address;
    NETIO_RouteKind_T recv_kind;
    struct DDS_LocatorSeq *dw_uc_locator_seq;
    struct DDS_LocatorSeq *dw_mc_locator_seq;
    RTI_BOOL found_route;

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

    if (!NETIO_Interface_delete_route((NETIO_Interface_T*)dw->dw_intf,
                                      &dst_reader,dw->rtps_intf,&dst_reader,
                                      NULL))
    {
        DDSC_LOG_NETIO_DELETE_ROUTE(OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_DATAWRITER_NETIO_KIND,
                                    DDSC_LOG_DATAREADER_NETIO_KIND)
        goto done;
    }

    /* All other communication paths relies on RTPS. The datawriter
     * creates and established the RTPS interface upon creation, thus
     * it already exists. All we need to do is add a route DW->RTPS and
     * a route from RTPS-> requested DR interface.
     */
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
 * \param[in] writer     The datawriter to match with a reader
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
 *
 * \sa \ref DDS_DataWriter_unmatch_reader
 */
void
DDS_DataWriter_match_reader(DDS_DataWriter *datawriter,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_DataReaderQos *const qos,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    RTI_BOOL route_existed = RTI_FALSE;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    if (!DDS_DataWriter_reader_is_compatible(datawriter,qos))
    {
        retval = DDS_DataWriter_delete_route(dw,key,uc_locator,mc_locator,
                                             &route_existed);
        if (retval)
        {
            if (route_existed)
            {
                DDS_DataWriterEvent_on_publication_matched(
                                    datawriter,key,(DDS_Boolean)route_existed,
                                    DDS_BOOLEAN_FALSE);
            }
            DDS_DataWriterEvent_on_incompatible_qos(datawriter,key);
        }
    }
    else
    {
        retval = DDS_DataWriter_add_route(datawriter,key,qos,
                                          uc_locator,mc_locator,&route_existed);
        if (!route_existed && retval)
        {
            DDS_DataWriterEvent_on_publication_matched(datawriter,key,
                                                   (DDS_Boolean)route_existed,
                                                   DDS_BOOLEAN_TRUE);
        }
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

    if (datawriter->qos.management.is_anonymous)
    {
        return;
    }

    retval = DDS_DataWriter_delete_route(datawriter,key,uc_locator,mc_locator,
                                         &route_existed);
    if (route_existed && retval)
    {
        DDS_DataWriterEvent_on_publication_matched(datawriter,key,
                                (DDS_Boolean)route_existed,DDS_BOOLEAN_FALSE);
    }
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
RTI_PRIVATE void
DDS_DataWriter_resolve_local_locators(DDS_DataWriter *datawriter,
                                      struct DDS_BuiltinTopicKey_t *key,
                                      const struct DDS_DataReaderQos *const qos,
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
        *uc_locator_seq = qos->data->unicast_locator;
        if (DDS_LocatorSeq_get_length(*uc_locator_seq) == 0)
        {
            *uc_locator_seq = datawriter->config->default_unicast;
        }

        *mc_locator_seq = qos->data->multicast_locator;
        if (DDS_LocatorSeq_get_length(*mc_locator_seq) == 0)
        {
            *mc_locator_seq = datawriter->config->default_multicast;
        }
    }
}

/*ci
 * \brief Match a datawriter with a local datareader
 *
 * \param[in] datawriter  Datawriter to match with a datareader
 * \param[in] key         Key of the datareader
 * \param[in] topic_name  The topic name published by the datareader
 * \param[in] type_name   The type name published by the datareader
 * \param[in] qos         The datareader qos
 */
void
DDS_DataWriter_match_local_reader(DDS_DataWriter *datawriter,
                                  struct DDS_BuiltinTopicKey_t *key,
                                  const char *topic_name,
                                  const char *type_name,
                                  const struct DDS_DataReaderQos *const qos)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)datawriter;
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;

    DDS_DataWriter_resolve_local_locators(datawriter,key,qos,
                                          &uc_locator_seq,
                                          &mc_locator_seq);

    if (!DDS_Topic_is_compatible(dw->topic,topic_name,
                                 type_name,
                                 key->value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID],
                                 datawriter->as_entity.entity_id))
    {
        return;
    }

    DDS_DataWriter_match_reader(datawriter,key,qos,uc_locator_seq,mc_locator_seq);
}

/*ci
 * \brief Resolve which unicast and multicast locators should be used to reach
 *        a remote subscription
 *
 * \details
 *
 * If a subscription specifies its own locators those are used instead of the
 * default locators specified on the participant.
 *
 * \param[in]  parent_data    The remote participant qos
 * \param[in]  data           The remote subscription qos
 * \param[out] uc_locator_seq The unicast locators to use for the subscription
 * \param[out] mc_locator_seq The multicast locators to use for the subscription
 */
RTI_PRIVATE void
DDS_DataWriter_resolve_locators(
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data,
        const struct DDS_LocatorSeq **uc_locator_seq,
        const struct DDS_LocatorSeq **mc_locator_seq)
{

    /* Locators for builtin endpoints are always in the participant
     */
    if (DDS_BuiltinTopicKey_is_builtin(&data->key))
    {
        *uc_locator_seq = &parent_data->metatraffic_unicast_locators;
        *mc_locator_seq = &parent_data->metatraffic_multicast_locators;
        return;
    }

    *uc_locator_seq = &data->unicast_locator;
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

    /* SubscriptionBuiltinTopicData is a subset of the DataReaderQos
     */
    dr_qos.deadline = data->deadline;
    dr_qos.liveliness = data->liveliness;
    dr_qos.ownership = data->ownership;
    dr_qos.reliability = data->reliability;
    dr_qos.durability = data->durability;
    dr_qos.destination_order = data->destination_order;

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

    DDS_DataWriter_resolve_locators(parent_data,data,
                                    &uc_locator_seq,&mc_locator_seq);

    DDS_DataWriter_match_reader(self,&data->key,
                                &dr_qos,uc_locator_seq,mc_locator_seq);

done:
    return;
}

#ifndef RTI_CERT
/*ci
 * \brief Unmatch a datawriter from a local datareader
 *
 * \param[in] self Datawriter to unmatch with a datareader
 * \param[in] key  Key of the datareader to unmatch from
 * \param[in] qos  The datareader qos
 *
 * \sa \ref DDS_DataWriter_match_local_reader
 */
void
DDS_DataWriter_unmatch_local_reader(DDS_DataWriter *self,
                                    struct DDS_BuiltinTopicKey_t *key,
                                    struct DDS_DataReaderQos *const qos)
{
    const struct DDS_LocatorSeq *uc_locator_seq;
    const struct DDS_LocatorSeq *mc_locator_seq;

    DDS_DataWriter_resolve_local_locators(self,key,qos,
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

    DDS_DataWriter_resolve_locators(parent_data,data,
                                    &uc_locator_seq,&mc_locator_seq);

    DDS_DataWriter_unmatch_reader(self,&data->key,uc_locator_seq,mc_locator_seq);
}

/*ci @} */
