/*
 * FILE: DataReaderImpl.c - DDS DataReader implementation
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
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to
 *                                   get_serialized_key_max_size
 * 16mar2015,tk  MICRO-1123/PR#14259 Removed redundant if test
 * 19sep2014,tk  MICRO-877 Added checks for writer enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 08aug2014,eh  MICRO-867: max_window_size limit now set by RTPS
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 07feb2013,eh  MICRO-238: removed debug msg causing segfault
 * 06feb2013,eh  MICRO-262: assign RTPS resource limits
 * 08jun2012,tk  Refactored from DataReader.c
 */
/*ci
 * \file
 * \brief DDS DataReader implementation
 *
 * \details
 * This file implements internal functions needed to support the public
 * DDS datareader API, mainly related to the life-cycle of a DDS datareader
 * object.
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "InstanceHandle.h"
#include "Conditions.h"
#include "QosPolicy.h"
#include "Entity.h"
#include "Locator.h"
#include "Transport.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"
#include "DataReaderInterface.h"
#include "Conditions.h"
#include "DataReaderImpl.h"
#include "SubscriberImpl.h"
#include "BuiltinCdr.h"
#include "UserDataQosPolicy.h"

#if DDS_LIVELINESS_CHANNEL_ENABLED
#include "RemotePublication.h"
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

#if DDS_FILTERING_ENABLED
#include "DataReaderFilter.h"
#include "DomainParticipantFilter.h"
#endif /* DDS_FILTERING_ENABLED */

const char* const DDS_DEFAULT_DATAREADER_NETIO_NAME = "ri";

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare two Keyhash structures based on the remote keyhash
 */
RTI_PRIVATE RTI_INT32
DDS_DataReaderImpl_cmp_remote_keyhash(const void *const record,
                                      RTI_BOOL key_is_record,
                                      const void *const key)
{
    const struct DDS_DataReaderKeyHashEntry *left =
            (const struct DDS_DataReaderKeyHashEntry*)record;
    const DDS_InstanceHandle_t *remote_hash;
    UNUSED_ARG(key_is_record);

    if (key_is_record)
    {
        remote_hash = &((struct DDS_DataReaderKeyHashEntry*)key)->remote_hash;
    }
    else
    {
        remote_hash = (const DDS_InstanceHandle_t*)key;
    }

    return OSAPI_Memory_compare(&left->remote_hash,remote_hash,
                                (RTI_SIZE_T)sizeof(left->remote_hash.octet));
}

/*ci
 * \brief Compare two Keyhash structures based on the local keyhash
 */
RTI_PRIVATE RTI_INT32
DDS_DataReaderImpl_cmp_local_keyhash(const void *const record,
                                     RTI_BOOL key_is_record,
                                     const void *const key)
{
    const struct DDS_DataReaderKeyHashEntry *left =
            (struct DDS_DataReaderKeyHashEntry*)record;
    const DDS_InstanceHandle_t *local_hash;

    if (key_is_record)
    {
        local_hash = &((struct DDS_DataReaderKeyHashEntry*)key)->local_hash;
    }
    else
    {
        local_hash = (const DDS_InstanceHandle_t*)key;
    }

    return OSAPI_Memory_compare(&left->local_hash,local_hash,
                                (RTI_SIZE_T)sizeof(left->local_hash.octet));
}

/*ci
 * \brief Create the data structures needed to map between local and
 *        remote keyhashes
 *
 * \details
 * If a reader supports multiple data representations using different
 * keyhash algorithms then a mapping table is required. The implementation
 * uses a bufferpool and two indices. The primary index is the remote
 * keyhash (since the mapping is from remote to local) while the secondary
 * is the local keyhash (needed to remove entries from the local keyhash).
 *
 * These resources are not allocated when a reader only supports one keyhash.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
DDS_DataReaderImpl_create_keyhash_index(struct DDS_DataReaderImpl *datareader)
{
    struct REDA_IndexerProperty idx_p = REDA_IndexerProperty_INITIALIZER;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;

    if (!DDS_TypePlugin_is_v1_and_v2_enabled(datareader->type_plugin))
    {
        datareader->keyhash_remote_index = NULL;
        datareader->keyhash_local_index = NULL;
        datareader->keyhash_pool = NULL;

        return RTI_TRUE;
    }

    idx_p.max_entries = datareader->resource_limits.max_instances;
    datareader->keyhash_remote_index = REDA_Indexer_new(
                                    	DDS_DataReaderImpl_cmp_remote_keyhash,&idx_p);
    if (datareader->keyhash_remote_index == NULL)
    {
        return RTI_FALSE;
    }

    datareader->keyhash_local_index = REDA_Indexer_new(
                                        DDS_DataReaderImpl_cmp_local_keyhash,
                                        &idx_p);
    if (datareader->keyhash_local_index == NULL)
    {
#ifndef RTI_CERT
        REDA_Indexer_delete(datareader->keyhash_remote_index);
#endif
        datareader->keyhash_remote_index = NULL;
        return RTI_FALSE;
    }

    datareader->keyhash_pool = NULL;
    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_DataReaderKeyHashEntry);
    pool_property.max_buffers = (RTI_SIZE_T)datareader->resource_limits.max_instances;

    datareader->keyhash_pool = REDA_BufferPool_new("keyhash_pool",
                                                   &pool_property,
                                                   NULL,NULL,
                                                   NULL,NULL);
    if (datareader->keyhash_pool == NULL)
    {
#ifndef RTI_CERT
        REDA_Indexer_delete(datareader->keyhash_remote_index);
        REDA_Indexer_delete(datareader->keyhash_local_index);
#endif
        datareader->keyhash_remote_index = NULL;
        datareader->keyhash_local_index = NULL;
        return RTI_FALSE;
    }


    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete the data structures needed to map between local and remote
 *        keyhashes.
 */
RTI_PRIVATE RTI_BOOL
DDS_DataReaderImpl_delete_keyhash_index(struct DDS_DataReaderImpl *datareader)
{
    struct DDS_DataReaderKeyHashEntry *hash = NULL;
    struct REDA_IndexIterator *it = NULL;

    if (datareader->keyhash_remote_index != NULL)
    {
        it = REDA_Indexer_iterator_begin(datareader->keyhash_remote_index);

        if ((datareader->keyhash_pool != NULL) && (it != NULL))
        {
            hash = (struct DDS_DataReaderKeyHashEntry*)REDA_Indexer_iterator_next(it);
            while (hash != NULL)
            {
                REDA_BufferPool_return_buffer(datareader->keyhash_pool,hash);
                hash = (struct DDS_DataReaderKeyHashEntry*)REDA_Indexer_iterator_next(it);
            }

            if (!REDA_BufferPool_delete(datareader->keyhash_pool))
            {
                return RTI_FALSE;
            }

            if (!REDA_Indexer_delete(datareader->keyhash_remote_index))
            {
                return RTI_FALSE;
            }
        }
        else
        {
            return RTI_FALSE;
        }
    }

    if (datareader->keyhash_local_index != NULL)
    {
        if (!REDA_Indexer_delete(datareader->keyhash_local_index))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}
#endif

/*ci
 * \brief Add a mapping from a remote -> local keyhash
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
DDS_DataReaderImpl_add_remote_keyhash(struct DDS_DataReaderImpl *datareader,
                                      const DDS_InstanceHandle_t *remote_key,
                                      const DDS_InstanceHandle_t *local_key)
{
    struct DDS_DataReaderKeyHashEntry *keyhash_entry = NULL;

    if (REDA_Indexer_find_entry(datareader->keyhash_remote_index,
                                remote_key) != NULL)
    {
        return RTI_TRUE;
    }

    keyhash_entry = REDA_BufferPool_get_buffer(datareader->keyhash_pool);
    if (keyhash_entry == NULL)
    {
        /*
         * The entries in the keyhash_pool are in lockstep with the number of
         * instances managed by the reader history. Since it is not possible
         * to query the actual number of instances in the reader
         * history, the assumption here is that if there are no more entries
         * then the reader history must be also be full and it is not
         * considered an error if allocation fails.
         */
        return RTI_TRUE;
    }

    keyhash_entry->remote_hash = *remote_key;
    keyhash_entry->local_hash = *local_key;

    if (!REDA_Indexer_add_entry(datareader->keyhash_remote_index,
                                keyhash_entry))
    {
        return RTI_FALSE;
    }

    if (!REDA_Indexer_add_entry(datareader->keyhash_local_index,
                                keyhash_entry))
    {
        REDA_Indexer_remove_entry(datareader->keyhash_remote_index,
                                  remote_key);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Lookup local keyhash from remote keyhash
 *
 * \return Keyhash entry on sucess, NULL if it does not exist.
 */
const struct DDS_DataReaderKeyHashEntry*
DDS_DataReaderImpl_lookup_local_keyhash(struct DDS_DataReaderImpl *datareader,
                                        const DDS_InstanceHandle_t *remote_key)
{
    const struct DDS_DataReaderKeyHashEntry *retval;

    if (datareader->keyhash_remote_index == NULL)
    {
        return NULL;
    }

    retval = REDA_Indexer_find_entry(datareader->keyhash_remote_index,
                                   remote_key);

    return retval;
}

/*ci
 * \brief Delete local keyhash
 */
void
DDS_DataReaderImpl_delete_local_keyhash(struct DDS_DataReaderImpl *datareader,
                                        const DDS_InstanceHandle_t *local_key)
{
    struct DDS_DataReaderKeyHashEntry *local_hash = NULL;
    struct DDS_DataReaderKeyHashEntry *remote_hash = NULL;

    if (datareader->keyhash_local_index == NULL)
    {
        return;
    }

    /* Find the keyhash entry based on the local key */
    local_hash = REDA_Indexer_remove_entry(datareader->keyhash_local_index,
                                           local_key);

    if (local_hash != NULL)
    {
        /* If it exists remove the remote index and return the remote
         * entry to the pool.
         */
        remote_hash = REDA_Indexer_remove_entry(datareader->keyhash_remote_index,
                                                &local_hash->remote_hash);

        if (remote_hash != NULL)
        {
            REDA_BufferPool_return_buffer(datareader->keyhash_pool,remote_hash);
        }
    }
}

DDS_ReturnCode_t
DDS_DataReader_get_qos_from(DDS_DataReader *self,
                            struct DDS_DataReaderQos *out)
{
    OSAPI_PRECONDITION((self == NULL) || (out == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);)

    out->deadline = self->deadline;
    out->liveliness = self->liveliness;
    out->history = self->history;
    out->resource_limits = self->resource_limits;
    out->ownership = self->ownership;
    out->reliability = self->reliability;
    out->protocol = self->protocol;
    out->type_support = self->type_support;
    out->management = self->management;
    out->durability = self->durability;
    out->reader_resource_limits = self->reader_resource_limits;
    out->destination_order = self->destination_order;
    out->latency_budget = self->latency_budget;

    if (!REDA_String_copy(out->subscription_name.name,DDS_ENTITYNAME_QOS_NAME_MAX,
                     self->subscription_name))
    {
        return DDS_RETCODE_ERROR;
    }

    /* NOTE: data is an an internal variable, don't copy */

    if (DDS_TransportQosPolicy_copy(&out->transport,&self->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_UserDataQosPolicy_copy(&out->user_data,self->user_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PropertyQosPolicy_copy(&out->property,&self->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportEncapsulationQosPolicy_copy(&out->encapsulation,
                                         self->encapsulation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataRepresentationQosPolicy_copy(&out->representation,
                                     self->representation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_ContentFilterQosPolicy_copy(&out->content_filter,
                                        self->content_filter_qos) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }
#endif

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataReader_set_qos_from(DDS_DataReader *self,
                            const struct DDS_DataReaderQos *in,
                            DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (in == NULL) || (participant == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    self->deadline = in->deadline;
    self->history = in->history;
    self->liveliness = in->liveliness;
    self->ownership = in->ownership;
    self->protocol = in->protocol;
    self->reliability = in->reliability;
    self->resource_limits = in->resource_limits;
    self->type_support = in->type_support;
    self->reader_resource_limits = in->reader_resource_limits;
    self->management = in->management;
    self->durability = in->durability;
    self->destination_order = in->destination_order;
    self->latency_budget = in->latency_budget;

    if (DDS_PropertyQosPolicy_copy(&self->property,&in->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportQosPolicy_copy(&self->transport,&in->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_UserDataQosPolicy_is_equal(self->user_data,&in->user_data))
    {
        if (self->user_data == &DDS_USER_DATA_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->user_data,
                                       struct DDS_UserDataQosPolicy);
            if (self->user_data == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            DDS_UserDataQosPolicy_initialize(self->user_data);
        }

        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_DATAREADER_TYPE,
                &in->user_data.value,
                &self->user_data->value))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (REDA_String_compare(self->subscription_name,in->subscription_name.name))
    {
#ifndef RTI_CERT
        if (self->subscription_name != DDS_ENTITY_NAME_DEFAULT)
        {
            REDA_String_free(self->subscription_name);
        }
#endif
        self->subscription_name = REDA_String_dup(in->subscription_name.name);
    }

    if (!DDS_TransportEncapsulationQosPolicy_is_equal(self->encapsulation,
                                                      &in->encapsulation))
    {
        if (self->encapsulation == &DDS_TRANSPORT_ENCAPSULATION_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->encapsulation,
                                   struct DDS_TransportEncapsulationQosPolicy);
            if (self->encapsulation == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            *self->encapsulation = DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
        }
        if (DDS_TransportEncapsulationQosPolicy_copy(self->encapsulation,
                                                     &in->encapsulation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (!DDS_DataRepresentationQosPolicy_is_equal(self->representation,
                                                  &in->representation))
    {
        if (self->representation == &DDS_DATAREPRESENTATION_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->representation,
                                   struct DDS_DataRepresentationQosPolicy);
            if (self->representation == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            *self->representation = DDS_DATAREPRESENTATION_DEFAULT;
        }
        if (DDS_DataRepresentationQosPolicy_copy(self->representation,
                                                 &in->representation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_ContentFilterQosPolicy_is_equal(self->content_filter_qos,
                                             &in->content_filter))
    {
        if (self->content_filter_qos == &DDS_CONTENT_FILTER_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->content_filter_qos,
                                       struct DDS_ContentFilterQosPolicy);
            if (self->content_filter_qos == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            if (DDS_ContentFilterQosPolicy_initialize(self->content_filter_qos) != DDS_RETCODE_OK)
            {
                return DDS_RETCODE_ERROR;
            }
        }
        if (!DDS_DataReader_update_filter(self, &in->content_filter))
        {
            DDSC_LOG_COMPILE_CONTENT_FILTER(OSAPI_LOGKIND_ERROR)
            return DDS_RETCODE_ERROR;
        }
    }
#endif /* DDS_FILTERING_ENABLED */

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataReader_finalize_managed(
    DDS_DataReader *self,
    DDS_DomainParticipant *participant)
{
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);
                       OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);)

    if (self->user_data != &DDS_USER_DATA_DEFAULT)
    {
        if (DDS_UserDataQosPolicy_finalize_no_dealloc(
                self->user_data,
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_DATAREADER_TYPE) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(self->user_data);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->user_data = (struct DDS_UserDataQosPolicy*)&DDS_USER_DATA_DEFAULT;
    }

#ifndef RTI_CERT
    if (DDS_TransportQosPolicy_finalize(&self->transport) != DDS_RETCODE_OK)
    {
       return DDS_RETCODE_ERROR;
    }
#endif

    if (self->encapsulation != &DDS_TRANSPORT_ENCAPSULATION_DEFAULT)
    {
#ifndef RTI_CERT
        if (self->encapsulation->value._contiguous_buffer != NULL)
        {
            if (DDS_TransportEncapsulationQosPolicy_finalize(self->encapsulation) != DDS_RETCODE_OK)
            {
                return DDS_RETCODE_ERROR;
            }
        }
        OSAPI_Heap_free_struct(self->encapsulation);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->encapsulation = (struct DDS_TransportEncapsulationQosPolicy*)&DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
    }

    if (self->representation != &DDS_DATAREPRESENTATION_DEFAULT)
    {
#ifndef RTI_CERT
        if (DDS_DataRepresentationQosPolicy_finalize(self->representation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        OSAPI_Heap_free_struct(self->representation);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->representation = (struct DDS_DataRepresentationQosPolicy*)&DDS_DATAREPRESENTATION_DEFAULT;
    }

    if (self->subscription_name != DDS_ENTITY_NAME_DEFAULT)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_string(self->subscription_name);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->subscription_name = (char*)DDS_ENTITY_NAME_DEFAULT;
    }

#ifndef RTI_CERT
    if (DDS_PropertyQosPolicy_finalize(&self->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }
#endif

#if DDS_FILTERING_ENABLED
    if (self->content_filter_qos != &DDS_CONTENT_FILTER_DEFAULT)
    {
        DDS_DomainParticipant_content_filter_qos_finalize_shallow_copy(participant,
                                                                       self->content_filter_qos);

#ifndef RTI_CERT
        if (DDS_ContentFilterQosPolicy_finalize(self->content_filter_qos) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }

        OSAPI_Heap_free_struct(self->content_filter_qos);
#endif /* !RTI_CERT */

        /* the cast is ok, we never write to this variable unless allocated */
        self->content_filter_qos = (struct DDS_ContentFilterQosPolicy*)&DDS_CONTENT_FILTER_DEFAULT;
    }
#endif /* DDS_FILTERING_ENABLED */

    return DDS_RETCODE_OK;
}

DDS_Boolean
DDS_DataReader_is_announced(const DDS_DataReader *self)
{
    return self->management.is_announced;
}

DDS_Boolean
DDS_DataReader_is_anonymous(const DDS_DataReader *self)
{
    return self->management.is_anonymous;
}

/*ci
 * \brief Check that the immutable part of a datareader qos policy has changed
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataReader_immutable_is_equal(const DDS_DataReader *left,
                                  const struct DDS_DataReaderQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    /* NOTE: data is an an internal variable, don't compare */
    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline,
                                        &right->deadline) ||
        !DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness) ||
        !DDS_HistoryQosPolicy_is_equal(&left->history,
                                       &right->history) ||
        !DDS_ResourceLimitsQosPolicy_is_equal(&left->resource_limits,
                                              &right->resource_limits) ||
        !DDS_DataReaderResourceLimitsQosPolicy_is_equal(
                                              &left->reader_resource_limits,
                                              &right->reader_resource_limits) ||
        !DDS_OwnershipQosPolicy_is_equal(&left->ownership,
                                         &right->ownership) ||
        !DDS_TypeSupportQosPolicy_is_equal(&left->type_support,
                                           &right->type_support) ||
        !DDS_DataReaderProtocolQosPolicy_is_equal(&left->protocol,
                                                  &right->protocol) ||
        !DDS_LatencyBudgetQosPolicy_is_equal(&left->latency_budget,
                                             &right->latency_budget) ||
        !DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability) ||
        !DDS_DurabilityQosPolicy_is_equal(&left->durability,
                                          &right->durability) ||
        !DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order) ||
        !DDS_TransportQosPolicy_is_equal(&left->transport,
                                         &right->transport) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management) ||
        !DDS_UserDataQosPolicy_is_equal(left->user_data,&right->user_data) ||
        !DDS_PropertyQosPolicy_is_equal(&left->property,&right->property) ||
        !DDS_DataRepresentationQosPolicy_is_equal(left->representation,
                                                  &right->representation) ||
        !DDS_TransportEncapsulationQosPolicy_is_equal(left->encapsulation,
                                                      &right->encapsulation) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management) ||
        REDA_String_compare(left->subscription_name,
                            right->subscription_name.name))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Compare two datareader records
 *
 * \details
 * This function is used as a compare function for a database table and
 * confirms to the semantics for \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataReaderImpl already in the database
 * \param[in] op2   Either a DDS_DataReaderImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DataReaderImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_DataReaderImpl *record_left = (struct DDS_DataReaderImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_DataReaderImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

/*ci
 * \brief Return the object-id for a datareader
 *
 * \param[in] self The datareader to return the object id for
 *
 * \return The object id for the datareader
 */
DDS_UnsignedLong
DDS_DataReader_get_objectid(DDS_DataReader *self)
{
    return self->as_entity.entity_id;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a datareader
 *
 * \details
 * Free up all resources used by a datareader. Note that this function does
 * not free the memory used to hold the datareader itself, this memory is
 * freed by the subscriber because the subscriber is the factory.
 *
 * \param[in] self Datareader to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataReaderImpl_finalize(DDS_DataReader *self)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RT_ComponentFactory *factory;
    RTI_BOOL netiorc_uc = RTI_TRUE;
    RTI_BOOL netiorc_mc = RTI_TRUE;
    RTI_INT32 mc_len = 0;
    RTI_INT32 uc_len = 0;

#ifdef ENABLE_QOS_DEADLINE
    if ((datareader->config != NULL) && (datareader->config->timer != NULL) &&
         DDS_Entity_is_enabled(DDS_DataReader_as_entity(self)) &&
        !DDS_Duration_is_infinite(&datareader->deadline.period))
    {
        if (!OSAPI_Timer_delete_timeout(datareader->config->timer,
                        &datareader->deadline_event))
        {
            goto done;
        }
    }
#endif

    if (datareader->mc_locator_seq != NULL)
    {
        mc_len = DDS_LocatorSeq_get_length(datareader->mc_locator_seq);
    }

    if (datareader->uc_locator_seq != NULL)
    {
        uc_len = DDS_LocatorSeq_get_length(datareader->uc_locator_seq);
    }

    if ((mc_len > 0) || (uc_len > 0))
    {
        /* release the lock so transport threads can finish */
        if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (mc_len > 0)
    {
        netiorc_mc = NETIO_BindResolver_release_addresses(
                        datareader->config->bind_resolver,
                        datareader->config->enabled_transports,
                        NETIO_ROUTEKIND_USER,
                        (struct NETIO_AddressSeq*)datareader->mc_locator_seq);
    }

    if (uc_len > 0)
    {
        netiorc_uc = NETIO_BindResolver_release_addresses(
                        datareader->config->bind_resolver,
                        datareader->config->enabled_transports,
                        NETIO_ROUTEKIND_USER,
                        (struct NETIO_AddressSeq*)datareader->uc_locator_seq);
    }

    if (datareader->mc_locator_seq != NULL)
    {
        if (!DDS_LocatorSeq_finalize(datareader->mc_locator_seq))
        {
            goto done;
        }
        OSAPI_Heap_free_struct(datareader->mc_locator_seq);
        datareader->mc_locator_seq = NULL;
    }

    if (datareader->uc_locator_seq != NULL)
    {
        if (!DDS_LocatorSeq_finalize(datareader->uc_locator_seq))
        {
            goto done;
        }
        OSAPI_Heap_free_struct(datareader->uc_locator_seq);
        datareader->uc_locator_seq = NULL;
    }

    if ((mc_len > 0) || (uc_len > 0))
    {
        /* restore locking state to the input state */
        if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
        {
            return DDS_BOOLEAN_FALSE;
        }

        /* if releasing any addresses failed, fail the call and return
         */
        if (!netiorc_mc || !netiorc_uc)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (datareader->md5_stream != NULL)
    {
        CDR_Stream_free(datareader->md5_stream);
    }

    if (self->_rh != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,
                                     DDSHST_READER_DEFAULT_HISTORY_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDSHST_READER_DEFAULT_HISTORY_NAME)
            goto done;
        }
        DDSHST_ReaderFactory_delete_component(factory, self->_rh);
        self->_rh = NULL;
    }

    if (datareader->dr_intf != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,
                                     DDS_DEFAULT_DATAREADER_NETIO_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,DDS_DEFAULT_DATAREADER_NETIO_NAME)
            goto done;
        }
        NETIO_InterfaceFactory_delete_component(factory,datareader->dr_intf);
        datareader->dr_intf = NULL;
    }

    if (datareader->rtps_intf != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,NETIO_DEFAULT_RTPS_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
            goto done;
        }

        NETIO_InterfaceFactory_delete_component(factory,datareader->rtps_intf);
        datareader->rtps_intf = NULL;
    }

#if DDS_FILTERING_ENABLED
    if (datareader->compiled_filter != NULL)
    {
        DDS_DataReader_finalize_filter(datareader);
    }
#endif /* DDS_FILTERING_ENABLED */

    if ((datareader->cdr_samples != NULL)
         && !REDA_BufferPool_delete(datareader->cdr_samples))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
        goto done;
    }

    if (!DDS_DataReaderImpl_delete_keyhash_index(datareader))
    {
        goto done;
    }

    if (datareader->type_plugin != NULL)
    {
        DDS_TypePlugin_delete(datareader->type_plugin);
    }

    if (DDS_DataReader_finalize_managed(datareader,
            DDS_Subscriber_get_participant(datareader->subscriber)) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        goto done;
    }

    if ((datareader->opaque_samples != NULL)
         && !REDA_BufferPool_delete(datareader->opaque_samples))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
        goto done;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Return the instance handle of a datareader
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function and
 * is called via DDS_Entity_get_instance_handle.
 *
 * \param[in] entity The base-class for the datareader
 *
 * \return The instance handle
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_InstanceHandle_t
DDS_DataReaderImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = dr->config->get_parent_handle((DDS_Entity*)dr->subscriber);

    DDS_InstanceHandle_set_suffix(&retval,dr->as_entity.entity_id);

    return retval;
}

RTI_PRIVATE void
DDS_DataReader_get_encapsulation_policy(struct DDS_TypePlugin *plugin,
                        NETIO_AddressResolver_T *ar,
                        NETIO_RouteResolver_T *rr,
                        struct DDS_LocatorSeq *uc_loc_seq,
                        struct DDS_LocatorSeq *mc_loc_seq,
                        struct DDS_TransportEncapsulationQosPolicy *policy)
{
    RTI_INT32 loc_index,loc_length;
    struct DDS_Locator *a_loc;

    loc_length = DDS_LocatorSeq_get_length(uc_loc_seq);
    for (loc_index = 0; loc_index < loc_length; ++loc_index)
    {
        a_loc = DDS_LocatorSeq_get_reference(uc_loc_seq,loc_index);

        DDS_Transport_set_encapsulation_policy(policy,plugin,a_loc,ar,rr);
    }

    loc_length = DDS_LocatorSeq_get_length(mc_loc_seq);
    for (loc_index = 0; loc_index < loc_length; ++loc_index)
    {
        a_loc = DDS_LocatorSeq_get_reference(mc_loc_seq,loc_index);

        DDS_Transport_set_encapsulation_policy(policy,plugin,a_loc,ar,rr);
    }
}

/*ci
 * \brief Initialize a datareader
 *
 * \details
 *
 * The subscriber allocates memory to store the datareader data and passes
 * it to the datareader for initialization. The datareader allocates all its
 * internal resources. The datareader is passed shared resources in the
 * config structure, such as database, timers resolvers etc. These resources
 * are typically managed by the domain participant.
 *
 * \param[in] datareader A datareader structure to initialize
 * \param[in] subscriber The subscriber creating the datareader
 * \param[in] topic_description The topic the datareader is subscribing to
 * \param[in] qos        The datareader qos policy
 * \param[in] listener   The datareader listener
 * \param[in] mask       Mask with enabled statuses on the datareader
 * \param[in] object_id  The datareader object id generated by the factory
 * \param[in] config     General datareader configuration
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataReaderImpl_initialize(struct DDS_DataReaderImpl *datareader,
                       DDS_Subscriber *subscriber,
                       DDS_TopicDescription *topic_description,
                       const struct DDS_DataReaderQos *qos,
                       const struct DDS_DataReaderListener *listener,
                       DDS_StatusMask mask,
                       DDS_UnsignedLong object_id,
                       struct NDDS_DataReaderConfig *config)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    DDS_Topic *topic;
    struct DDS_DataReaderInterfaceProperty drintf_property =
                                DDS_DataReaderInterfaceProperty_INITIALIZER;
    struct DDSHST_ReaderProperty rh_property =
                                        DDSHST_ReaderProperty_INITIALIZER;
    struct DDSHST_ReaderListener rh_listener =
                                        DDSHST_ReaderListener_INITIALIZE;
    DDS_InstanceHandle_t instance_handle;
    struct RT_ComponentFactory *factory;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct RTPS_InterfaceProperty rtps_property =
                                            RTPS_InterfaceProperty_INITIALIZER;
    DDS_UnsignedLong serialized_key_size;
    struct DDS_DataReaderListener nil_listener = DDS_DataReaderListener_INITIALIZER;
    struct DDS_TypePluginProperty type_property =
                                            DDS_TypePluginProperty_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos = NULL;
#if INCLUDE_API_QOS
    DDS_ReturnCode_t ddsrc;
#endif
    RTI_UINT32 max_serialized_size = 0;

    OSAPI_Memory_zero(datareader,sizeof(struct DDS_DataReaderImpl));

    topic = DDS_Topic_narrow(topic_description);
    if (topic == NULL)
    {
        DDSC_LOG_TOPIC_NARROW(OSAPI_LOGKIND_ERROR,
                            DDS_TopicDescription_get_name(topic_description))
        goto done;
    }

    datareader->config = config;
    datareader->subscriber = subscriber;
    datareader->topic = topic;

    /* Initialize optional members, all others are
     * set in DDS_DataWriter_set_qos_from.
     * the cast is ok, we never write to these variables unless allocated
     */
    datareader->user_data = (struct DDS_UserDataQosPolicy*)&DDS_USER_DATA_DEFAULT;
    datareader->encapsulation = (struct DDS_TransportEncapsulationQosPolicy*)&DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
    datareader->representation = (struct DDS_DataRepresentationQosPolicy*)&DDS_DATAREPRESENTATION_DEFAULT;
    datareader->subscription_name = (char*)DDS_ENTITY_NAME_DEFAULT;
#if DDS_FILTERING_ENABLED
    datareader->content_filter_qos = (struct DDS_ContentFilterQosPolicy*)&DDS_CONTENT_FILTER_DEFAULT;
#endif /* DDS_FILTERING_ENABLED */

    if ((listener != NULL) &&
        !DDS_DataReaderListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_LISTENER,mask)
        goto done;
    }

    if (!DDS_PropertyQosPolicy_is_valid(&qos->property,
                                        DDS_DATAREADER_ENTITY_KIND))
    {
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    dp_qos = DDS_DomainParticipant_get_qos_ref(subscriber->participant);
    if (!DDS_UserDataQosPolicy_is_consistent(&qos->user_data,
                        dp_qos->resource_limits.reader_user_data_max_length))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_ContentFilterQosPolicy_is_consistent(&qos->content_filter,
                                                  &dp_qos->filter.resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                 DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_CONTENT_FILTER_QOS_POLICY);
        goto done;
    }
#endif /* DDS_FILTERING_ENABLED */

    if (!DDS_DataReaderQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
        return DDS_BOOLEAN_FALSE;
    }

#if INCLUDE_API_QOS
    if (qos == &DDS_DATAREADER_QOS_DEFAULT)
    {
            ddsrc = DDS_DataReader_set_qos_from(datareader,
                                                subscriber->default_qos,
                                                subscriber->participant);
        if (ddsrc != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_GET(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAREADER_QOS)
            goto done;
        }
    }
    else
#endif
    {

        if (DDS_DataReader_set_qos_from(datareader,
                                        qos,
                                        subscriber->participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
            goto done;
        }
    }


    /* pool for loaned sample and info sequence buffers */
    if (datareader->history.kind == DDS_KEEP_ALL_HISTORY_QOS)
    {
        if (datareader->resource_limits.max_samples_per_instance == DDS_LENGTH_UNLIMITED)
        {
            /* NOTE: Assumes finite max_samples. A consistency check has already
             * been performed.
             */
            datareader->read_seq_max = datareader->resource_limits.max_instances *
                                        (datareader->resource_limits.max_samples + 1);
        }
        else
        {
            datareader->read_seq_max =
                datareader->resource_limits.max_instances *
                (datareader->resource_limits.max_samples_per_instance + 1);
        }
    }
    else /* KEEP_LAST */
    {
        /* NOTE: Assumes finite max_instances */
        /* add max_instances to accomodate meta samples */
        datareader->read_seq_max =
                            (datareader->resource_limits.max_instances *
                             datareader->history.depth + 1);
    }

    datareader->reader_data.resolved_participant_locators = config->default_unicast;
    datareader->reader_data.unicast_locator = NULL;
    datareader->reader_data.multicast_locator = NULL;

    if (DDS_StringSeq_get_length(&datareader->transport.enabled_transports) > 0)
    {
        OSAPI_Heap_allocate_struct(&datareader->uc_locator_seq,
                                   struct DDS_LocatorSeq);

        if (datareader->uc_locator_seq == NULL)
        {
            goto done;
        }

        OSAPI_Heap_allocate_struct(&datareader->mc_locator_seq,
                                   struct DDS_LocatorSeq);

        if (datareader->mc_locator_seq == NULL)
        {
            goto done;
        }

        if (!DDS_LocatorSeq_initialize(datareader->uc_locator_seq))
        {
        	goto done;
        }

        if (!DDS_LocatorSeq_initialize(datareader->mc_locator_seq))
        {
        	goto done;
        }

        if (!DDS_LocatorSeq_set_maximum(datareader->uc_locator_seq,
                                   RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_USERUNICAST_SEQUENCE,
                                RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(datareader->uc_locator_seq,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_USERUNICAST_SEQUENCE,0)
            goto done;
        }

        if (!DDS_LocatorSeq_set_maximum(datareader->mc_locator_seq,
                                        RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_USERMULTICAST_SEQUENCE,
                                RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(datareader->mc_locator_seq,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_USERMULTICAST_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             config->bind_resolver,*config->participant_id,
             config->enabled_transports,
             (struct REDA_StringSeq*)&datareader->transport.enabled_transports,
             NETIO_ROUTEKIND_USER,
             (struct NETIO_AddressSeq*)datareader->mc_locator_seq,
             (struct NETIO_AddressSeq*)datareader->uc_locator_seq))
        {
            goto done;
        }
        datareader->reader_data.unicast_locator = datareader->uc_locator_seq;
        datareader->reader_data.multicast_locator = datareader->mc_locator_seq;
    }
    else
    {
        datareader->reader_data.unicast_locator = datareader->config->default_unicast;
        datareader->reader_data.multicast_locator = datareader->config->default_multicast;
    }

    type_property.plugin_param = datareader->type_support.plugin_data;
    type_property.head_padding = 0;
    type_property.tail_padding = 0;

    type_property.max_buffers = datareader->resource_limits.max_samples;
    type_property.max_buffers++;

    datareader->type_plugin = DDS_TypeImpl_create_plugin(
                                DDS_Topic_get_type(datareader->topic),
                                DDS_Subscriber_get_participant(subscriber),
                                DDS_DomainParticipant_get_qos_ref(
                                    DDS_Subscriber_get_participant(subscriber)),
                                DDS_TYPEPLUGIN_MODE_READER,
                                datareader,NULL,
                                &type_property);

    if (datareader->type_plugin == NULL)
    {
        DDSC_LOG_LOOKUP_TYPE_PLUGIN(OSAPI_LOGKIND_ERROR,
                                        DDS_Topic_get_type(datareader->topic))
        goto done;
    }

    datareader->cdr_id = DDS_TypePlugin_get_cdr_encapsulation(datareader->type_plugin);

    if (!DDS_DataReaderImpl_create_keyhash_index(datareader))
    {
        goto done;
    }

    if (DDS_TransportEncapsulationSettingsSeq_get_length(
                                &datareader->encapsulation->value) == 0)
    {
        if (datareader->encapsulation == &DDS_TRANSPORT_ENCAPSULATION_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&datareader->encapsulation,
                                       struct DDS_TransportEncapsulationQosPolicy);
            if (datareader->encapsulation == NULL)
            {
                goto done;
            }
            *datareader->encapsulation = DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
        }
        /* Set the defaults */
        DDS_DataReader_get_encapsulation_policy(datareader->type_plugin,
                                datareader->config->address_resolver,
                                datareader->config->route_resolver,
                                datareader->reader_data.unicast_locator,
                                datareader->reader_data.multicast_locator,
                                datareader->encapsulation);
    }
    else if (!DDS_Transport_is_encapsulation_policy_valid(
                                            datareader->encapsulation,
                                            datareader->type_plugin,
                                            datareader->config->address_resolver,
                                            datareader->config->route_resolver))
    {
        goto done;
    }

    /* If there are no restrictions and defaults are used, then revert back to
     * the custom so the endpoint locators are not set
     */
    if ((DDS_TransportEncapsulationSettingsSeq_get_length(&datareader->encapsulation->value) == 0)
            && (datareader->reader_data.unicast_locator == datareader->config->default_unicast))
    {
        datareader->reader_data.unicast_locator = NULL;
        datareader->reader_data.multicast_locator = NULL;
    }

    datareader->key_kind = DDS_TypePlugin_get_key_kind(datareader->type_plugin);

    if (!DDS_EntityImpl_initialize(&datareader->as_entity,
                                   DDS_DATAREADER_ENTITY_KIND,
                                   object_id,DDS_DataReader_enable,
                                   DDS_DataReaderImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }


    if (listener == NULL)
    {
        datareader->listener = nil_listener;
    }
    else
    {
        datareader->listener = *listener;
    }

    datareader->mask = mask;
    datareader->md5_stream = NULL;

    pool_property.buffer_size = (RTI_SIZE_T)sizeof(struct RTI_DataReaderSample);
    pool_property.max_buffers = (RTI_SIZE_T)datareader->resource_limits.max_samples;

    /* Add one for keep last purposes */
    pool_property.max_buffers++;

    pool_property.flags |= REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;

#ifndef RTI_CERT
    datareader->cdr_samples = REDA_BufferPool_new("cdr_samples",&pool_property,
                                    NULL,
                                    datareader,
                                    NULL,
                                    datareader);
#else
       /* No finalize fn for RTI_CERT */
    datareader->cdr_samples = REDA_BufferPool_new("cdr_samples",&pool_property,
                                    NULL,
                                    datareader,
                                    NULL,
                                    NULL);
#endif
    if (datareader->cdr_samples == NULL)
    {
        DDSC_LOG_CDR_POOL_ALLOC(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR,
                                 pool_property.buffer_size,
                                 pool_property.max_buffers)
        goto done;
    }

    rh_listener.listener_data = (void *)datareader;
    rh_listener.on_sample_removed = DDS_DataReaderEvent_on_sample_removed;
    rh_listener.on_data_available = DDS_DataReaderEvent_on_data_available;
    rh_listener.on_deadline_missed = DDS_DataReaderEvent_on_deadline_missed;
    rh_listener.on_sample_rejected = DDS_DataReaderEvent_on_hst_sample_rejected;
    rh_listener.on_sample_lost = DDS_DataReaderEvent_on_hst_sample_lost;
    rh_listener.on_instance_replaced = DDS_DataReaderEvent_on_instance_replaced;
    rh_listener.on_sample_committed = DDS_DataReaderEvent_on_sample_committed;
    rh_listener.on_key_removed = DDS_DataReaderEvent_on_key_removed;

    factory = RT_Registry_lookup(datareader->config->registry,
                                 DDSHST_READER_DEFAULT_HISTORY_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDSHST_READER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    rh_property._parent.db = datareader->config->db;
    rh_property.datareader = datareader;
    rh_property._parent.timer = datareader->config->timer;
    rh_property.deadline = datareader->deadline;
    rh_property.destination_order = datareader->destination_order;
    rh_property.history = datareader->history;
    rh_property.durability = datareader->durability;
    rh_property.reliability = datareader->reliability;
    rh_property.resource_limits = datareader->resource_limits;
    rh_property.ownership = datareader->ownership;
    rh_property.reader_resource_limits = datareader->reader_resource_limits;

    datareader->_rh = DDSHST_ReaderFactory_create_component(factory,
                                                        &rh_property._parent,
                                                        &rh_listener._parent);

    if (datareader->_rh == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAREADERIO_COMPONENT,
                                  DDSHST_READER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    /* DataReader Interface */
    factory = RT_Registry_lookup(datareader->config->registry,
                                 DDS_DEFAULT_DATAREADER_NETIO_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDS_DEFAULT_DATAREADER_NETIO_NAME)
        goto done;
    }

    drintf_property._parent._parent.db = datareader->config->db;

    drintf_property._parent.max_binds =
         (RTI_SIZE_T)datareader->reader_resource_limits.max_remote_writers;

    drintf_property.datareader = datareader;
    instance_handle = DDS_DataReaderImpl_get_instance_handle(
                                                    (DDS_Entity*)datareader);

    NETIO_Address_set_guid(&drintf_property.intf_address,
                          0,(struct NETIO_Guid*)&instance_handle.octet);

    datareader->dr_intf = NETIO_InterfaceFactory_create_component(factory,
                                        &drintf_property._parent._parent,NULL);
    if (datareader->dr_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAREADERIO_COMPONENT,
                                  DDS_DEFAULT_DATAREADER_NETIO_NAME)
        goto done;
    }

    factory = RT_Registry_lookup(datareader->config->registry,NETIO_DEFAULT_RTPS_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    NETIO_Address_set_guid(&rtps_property.intf_address,
                           (RTI_UINT32)*datareader->config->domain_id,
                           (struct NETIO_Guid*)&instance_handle.octet);

    rtps_property._parent._parent.db = datareader->config->db;
    rtps_property.anonymous = datareader->management.is_anonymous;

    rtps_property._parent.max_routes =
        (RTI_SIZE_T)(datareader->reader_resource_limits.max_remote_writers *
                        datareader->reader_resource_limits.max_routes_per_writer);

    rtps_property.mode = RTPS_INTERFACEMODE_READER;
    rtps_property.reliable =
        (datareader->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    rtps_property.max_peer_count =
        datareader->reader_resource_limits.max_remote_writers;
    rtps_property._parent._parent.timer = datareader->config->timer;
    rtps_property.max_window_size =
        datareader->reader_resource_limits.max_samples_per_remote_writer;
    rtps_property.max_samples = datareader->resource_limits.max_samples;
    rtps_property.ext_rtps_intf = datareader->config->ext_rtps_intf;

    /* Determine the number of fragmented samples needed.
     *
     * - If qos->reader_resource_limits.max_fragmented_samples == AUTO
     *   determine if the serialized  samples fits in the message buffer
     *   based on the MTU. Note that it is not possible to know if a sender
     *   is going to send fragmented samples. Even if the MTU in Micro is
     *   greater than the serialized size, it is  unknown what the sender is
     *   doing. Thus, this covers the common case, where the serialized
     *   sample is larger than the MTU.
     *
     * - If qos->reader_resource_limits.max_fragmented_samples > 0
     *   then that is number of fragmented samples that will be allocated,
     *   regardless of whether it is required or not.
     *
     * - If qos->reader_resource_limits.max_fragmented_samples == 0
     *   fragmented samples are ignored.
     */
    max_serialized_size = DDS_TypePlugin_get_serialized_sample_size_max(
                                                datareader->type_plugin);

    if (qos->reader_resource_limits.max_fragmented_samples == DDS_MAX_AUTO)
    {
        RTI_INT32 min_mtu = 0;

        min_mtu = NETIO_RouteResolver_get_minimum_mtu(config->route_resolver);

        if (min_mtu < 0)
        {
            DDSC_LOG_GET_MTU(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        if (min_mtu == 0)
        {
            DDSC_LOG_GET_MTU_NO_ROUTES(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if (((RTI_UINT32)min_mtu <= RTPS_MINIMUM_PROTOCOL_OVERHEAD_DATA_FRAG)
            || (max_serialized_size > ((RTI_UINT32)min_mtu - RTPS_MINIMUM_PROTOCOL_OVERHEAD_DATA_FRAG)))
        {
            rtps_property.max_fragmented_samples = rtps_property.max_samples;
        }
        else
        {
            rtps_property.max_fragmented_samples = 0;
        }
    }
    else
    {
        rtps_property.max_fragmented_samples =
                    datareader->reader_resource_limits.max_fragmented_samples;
    }

    if (rtps_property.max_fragmented_samples > 0)
    {
        rtps_property.rcv_fragment_size_bytes =
                                RTPS_MAX_INLINE_QOS + max_serialized_size;

        /* Add one state sample per remote reader */
        rtps_property.max_fragmented_samples +=
                        datareader->reader_resource_limits.max_remote_writers;

        rtps_property.max_fragment_buffers =
                                        rtps_property.max_fragmented_samples;

        rtps_property.max_fragmented_samples_per_remote_writer =
                                            rtps_property.max_fragment_buffers;
    }

    /* Note: for this "local" bind, max_binds could in theory just be set to the
     * max number of upstream interfaces.  However, in RTPS implementation,
     * the bind table is used to track src-peer mappings, and thus has a record
     * per src-peer pair and must be set to max peers (max_remote_writers).
     */
    rtps_property._parent.max_binds =
          (RTI_SIZE_T)datareader->reader_resource_limits.max_remote_writers;

#if OSAPI_ENABLE_TRACE
    rtps_property.session_name =
            DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(datareader->topic));
#endif

    /* QoS consistency validates that nack_period.sec < INT_MAX */
    rtps_property.nack_period.sec =
            datareader->protocol.rtps_reliable_reader.nack_period.sec;
    rtps_property.nack_period.nanosec =
            datareader->protocol.rtps_reliable_reader.nack_period.nanosec;
    rtps_property.transport_priority = qos->transport_priority.value;

    datareader->rtps_intf = NETIO_InterfaceFactory_create_component(factory,
                                        &rtps_property._parent._parent,NULL);
    if (datareader->rtps_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_RTPS_COMPONENT,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    if (datareader->key_kind == NDDS_TYPEPLUGIN_USER_KEY)
    {
        serialized_key_size = DDS_TypePlugin_get_serialized_key_size(
                                    datareader->type_plugin, 0);

        datareader->md5_stream = CDR_Stream_alloc(serialized_key_size);
        if (datareader->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
    }
    else if (datareader->key_kind == NDDS_TYPEPLUGIN_GUID_KEY)
    {
        datareader->md5_stream = CDR_Stream_alloc(16);
        if (datareader->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
    }

    if (datareader->md5_stream != NULL)
    {
#ifdef RTI_ENDIAN_LITTLE
        CDR_Stream_byteswap_set(datareader->md5_stream, RTI_TRUE);
#else
        CDR_Stream_byteswap_set(datareader->md5_stream, RTI_FALSE);
#endif
    }

    datareader->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        (void)DDS_DataReaderImpl_finalize(datareader);
    }
#endif
    return retval;
}

/*ci
 * \brief Check if a datareader is enabled
 *
 * \param[in] self Datareader to check
 *
 * \return DDS_BOOLEAN_TRUE if the datareader is enabled, DDS_BOOLEAN_FALSE
 *         otherwise
 */
DDS_Boolean
DDS_DataReader_is_enabled(DDS_DataReader *self)
{
    return (self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) ?
                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check if a DDS_DataReaderListener is consistent
 *
 * \param[in] l DDS_DataReaderListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
DDS_Boolean
DDS_DataReaderListener_is_consistent(const struct DDS_DataReaderListener *l,
                                     DDS_StatusMask m)
{
    return (((!((m) & DDS_REQUESTED_DEADLINE_MISSED_STATUS)) ||
            (l->on_requested_deadline_missed != NULL)) &&
        ((!((m) & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)) ||
            (l->on_requested_incompatible_qos != NULL)) &&
        ((!((m) & DDS_SAMPLE_REJECTED_STATUS)) ||
            (l->on_sample_rejected != NULL)) &&
        ((!((m) & DDS_SAMPLE_LOST_STATUS)) ||
            (l->on_sample_lost != NULL)) &&
        ((!((m) & DDS_LIVELINESS_CHANGED_STATUS)) ||
            (l->on_liveliness_changed != NULL)) &&
        ((!((m) & DDS_SUBSCRIPTION_MATCHED_STATUS)) ||
            (l->on_subscription_matched != NULL)) &&
        ((!((m) & DDS_INSTANCE_REPLACED_STATUS)) ||
            (l->on_instance_replaced != NULL)) &&
        ((!((m) & DDS_DATA_AVAILABLE_STATUS)) ||
            (l->on_data_available != NULL))) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

NETIO_BindResolver_T*
DDS_DataReader_get_bind_resolver(const DDS_DataReader *const self)
{
    return self->config->bind_resolver;
}

DDS_Long
DDS_DataReader_get_max_samples(const DDS_DataReader *self)
{
    return self->resource_limits.max_samples;
}

const struct DDS_DataRepresentationQosPolicy*
DDS_DataReader_get_data_representation_ref(const DDS_DataReader *self)
{
    return self->representation;
}

const struct DDS_TransportEncapsulationQosPolicy*
DDS_DataReader_get_encapsulation_ref(const DDS_DataReader *self)
{
    return self->encapsulation;
}

DDS_Long
DDS_DataReader_get_max_remote_writers(const DDS_DataReader *self)
{
    return self->reader_resource_limits.max_remote_writers;
}

DDS_Long
DDS_DataReader_get_shmem_ref_transfer_mode_attached_segment_allocation(
                                const DDS_DataReader *self)
{
    return self->reader_resource_limits.shmem_ref_transfer_mode_attached_segment_allocation;
}

const struct DDS_LocatorSeq*
DDS_DataReader_get_unicast_locator_ref(const DDS_DataReader *self)
{
    if (self->reader_data.unicast_locator == NULL)
    {
        return &DDS_NullLocatorSeq;
    }
    return self->reader_data.unicast_locator;
}

const struct DDS_LocatorSeq*
DDS_DataReader_get_multicast_locator_ref(const DDS_DataReader *self)
{
    if (self->reader_data.multicast_locator == NULL)
    {
        return &DDS_NullLocatorSeq;
    }
    return self->reader_data.multicast_locator;
}

const struct DDS_LocatorSeq *
DDS_DataReader_get_resolved_locator_ref(const DDS_DataReader *self)
{
    return self->reader_data.resolved_participant_locators;
}

void
DDS_DataReader_get_request_offered_qos(const DDS_DataReader *self,
                                       struct DDS_DataReaderQos *qos)

{
    qos->deadline = self->deadline;
    qos->destination_order = self->destination_order;
    qos->durability = self->durability;
    qos->latency_budget = self->latency_budget;
    qos->liveliness = self->liveliness;
    qos->ownership = self->ownership;
    qos->reliability = self->reliability;
    qos->representation = *self->representation;
}

#if DDS_FILTERING_ENABLED
const struct DDS_ContentFilterQosPolicy*
DDS_DataReader_get_content_filter_qos_ref(const DDS_DataReader *self)
{
    return self->content_filter_qos;
}
#endif /* DDS_FILTERING_ENABLED */

DDS_Boolean
DDS_DataReader_serialize(const DDS_DataReader *self,struct CDR_Stream_t *stream)
{
    const struct DDS_LatencyBudgetQosPolicy default_latency_budget =
        DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT;
    const struct DDS_DeadlineQosPolicy default_deadline =
        DDS_DEADLINE_QOS_POLICY_DEFAULT;
    const struct DDS_OwnershipQosPolicy default_ownership =
        DDS_OWNERSHIP_QOS_POLICY_DEFAULT;
    const struct DDS_ReliabilityQosPolicy default_reliability =
        DDS_DATAREADER_RELIABILITY_QOS_POLICY_DEFAULT;
    const struct DDS_LivelinessQosPolicy default_liveliness =
        DDS_LIVELINESS_QOS_POLICY_DEFAULT;
    const struct DDS_DurabilityQosPolicy default_durability =
        DDS_DURABILITY_QOS_POLICY_DEFAULT;
    const struct DDS_DestinationOrderQosPolicy default_destination_order =
        DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT;

    DDS_Boolean has_non_default_latency_budget =
        !DDS_Duration_equal(&self->latency_budget.duration,
                            &default_latency_budget.duration);
    DDS_Boolean has_non_default_deadline =
        !DDS_Duration_equal(&self->deadline.period, &default_deadline.period);
    DDS_Boolean has_non_default_ownership =
        (self->ownership.kind != default_ownership.kind);
    DDS_Boolean has_non_default_reliability =
        (self->reliability.kind != default_reliability.kind) ||
        !DDS_Duration_equal(&self->reliability.max_blocking_time,
                            &default_reliability.max_blocking_time);
    DDS_Boolean has_non_default_liveliness =
        !DDS_LivelinessQosPolicy_is_equal(&self->liveliness, &default_liveliness);
    DDS_Boolean has_non_default_durability =
        !DDS_DurabilityQosPolicy_is_equal(&self->durability, &default_durability);
    DDS_Boolean has_non_default_destination_order =
        (self->destination_order.kind != default_destination_order.kind);

    if (has_non_default_latency_budget &&
        !DDS_CdrQosPolicy_serialize_latency_budget(stream,
                                                   &self->latency_budget, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_deadline &&
        !DDS_CdrQosPolicy_serialize_deadline(stream,
                                             &self->deadline, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_ownership &&
        !DDS_CdrQosPolicy_serialize_ownership(stream,&self->ownership, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_reliability &&
        !DDS_CdrQosPolicy_serialize_reliability(stream,
                                                &self->reliability, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_liveliness &&
        !DDS_CdrQosPolicy_serialize_liveliness(stream,
                                               &self->liveliness, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_durability &&
        !DDS_CdrQosPolicy_serialize_durability(stream,
                                                &self->durability, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_destination_order &&
        !DDS_CdrQosPolicy_serialize_destination_order(stream,
                                                &self->destination_order, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_CdrQosPolicy_serialize_user_data(stream,self->user_data))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->reader_data.unicast_locator != NULL)
    {
        if (!DDS_CdrQosPolicy_serialize_locator_sequence(
                stream,
                self->reader_data.unicast_locator,
                RTPS_PID_UNICAST_LOCATOR6,
                RTPS_PID_UNICAST_LOCATOR6_EX,
                self->encapsulation,
                DDS_DataReader_get_bind_resolver(self)))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (self->reader_data.multicast_locator != NULL)
    {
        if (!DDS_CdrQosPolicy_serialize_locator_sequence(
                stream,
                self->reader_data.multicast_locator,
                RTPS_PID_MULTICAST_LOCATOR6,
                RTPS_PID_MULTICAST_LOCATOR6_EX,
                self->encapsulation,
                DDS_DataReader_get_bind_resolver(self)))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

#if DDS_FILTERING_ENABLED
   if (!DDS_DataReader_serialize_filter_property(self, stream))
    {
        return RTI_FALSE;
    }
#endif

    return DDS_BOOLEAN_TRUE;
}

DDS_ReturnCode_t
DDS_DataReader_return_loan_sample(
        DDS_DataReader *self,
        void *sample,
        struct DDS_SampleInfo *info)
{
    void *sample_array[1];
    struct DDS_SampleInfo *info_array[1];
    void **sample_array_ptr = &sample_array[0];
    struct DDS_SampleInfo **info_array_ptr = &info_array[0];

    sample_array[0] = sample;
    info_array[0] = info;

    return DDSHST_Reader_finish_read_or_take(self->_rh,
            &sample_array_ptr,&info_array_ptr,1,DDS_BOOLEAN_FALSE);
}

/*ci @} */
