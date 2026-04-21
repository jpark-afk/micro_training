/*
 * FILE: DataReader.c - DDS DataReader implementation
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 20oct2021,tk MICRO-3311/PR.29816
 * - Removed race-conditions in _enable()
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 * - Added suppression of dereference and cert_exp34_c_violation
 *   in DDS_DataReader_read_or_take.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_DataReader_get_qos_ref
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 11sep2015,tk  MICRO-1499/PR#16514 Fixed issues when resetting a remote
 *                                   participant that prevented it from being
 *                                   rediscovered.
 * 20jul2015,tk  MICRO-1426/PR#15358 Use Deadline_get_sample_freq() for deadline
 *                                   check
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to get_key_kind
 * 29jun2015,tk  MICRO-1325/PR#15039 Clear DATA_ON_READERS status in read/take
 * 12mar2015,tk  MICRO-1033/PR#13478 Removed redundant code when log is disabled
 * 09feb2015,tk  MICRO-1030/PR#13475 Removed potential infinite loop on failure
 *               MICRO-1031/PR#13476 Removed +1 on max samples read
 *               MICRO-1032/PR#13477 Check return value from set_length
 *               MICRO-1034/PR#13479 Return correct return code
 *               MICRO-1036/PR#13483 Return correct return code
 * 26jan2015,tk  MICRO-1028/PR#13473 Removed magic number 0xc0
 * 21jan2015,eh  MICRO-355/PR#1493 No DDS_DataReader_get_listener in Cert
 * 05dec2014,as  Additional fixes for MICRO-969
 * 07nov2014,eh  MICRO-923: check NULL key_holder for lookup_instance()
 * 14oct2014,tk  MICRO-923 Always enable pre-condition check on
 *                         DDS_DataReader_lookup_instance
 * 20sep2014,as  Moved implementation of support functions for Status types to
 *               DataReaderStatus.c
 * 19sep2014,tk  MICRO-878 Verify returned sequences comes from the reader
 *                         that loanded them, and that data/info pairs are
 *                         correct
 * 16sep2014,tk  MICRO-877 Added checks for reader enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 09may2014,as  MICRO-786 Update StatusCondition when DDS_DATA_AVAILABLE_STATUS
 *               is reset on DataReader
 * 07may2014,as  MICRO-784 Expose get_X_status API in C++ (implementation of
 *               support functions for status types)
 * 05may2014,as  MICRO-270 Always enable precondition
 *               checks for public API operations
 * 04mar2014,tk  MICRO-672: Check if DataReader is enabled in set_qos()
 * 12dec2013,eh  MICRO-732: DB lock return_loan, lookup_instance
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 07feb2013,eh  MICRO-224: lookup_instance() returns nil_handle
 * 18may2012,tk  Written
 */
/*ce
 * \file
 * \brief DDS DataReader implementation
 *
 * \details
 * This file implements the public DDS datareader API. Functionality to support
 * the public APIs is implemented in the supporting DataReaderNNN files.
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
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "QosPolicy.h"
#include "Entity.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "Conditions.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"
#include "DataReaderInterface.h"
#include "DataReaderImpl.h"

const DDS_InstanceStateMask DDS_ANY_INSTANCE_STATE =
    DDS_ALIVE_INSTANCE_STATE |
    DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE |
    DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

const DDS_InstanceStateMask DDS_NOT_ALIVE_INSTANCE_STATE =
    DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE |
    DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

const DDS_ViewStateMask DDS_ANY_VIEW_STATE =
    DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE;

const DDS_SampleStateMask DDS_ANY_SAMPLE_STATE =
    DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataReader_enable(DDS_Entity *self)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl*)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    struct DDS_Duration_t deadline_sample_hz = DDS_DURATION_ZERO;

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (datareader->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        goto done;
    }

    if (!DDS_Entity_is_enabled(DDS_Topic_as_entity(datareader->topic)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_Entity_is_enabled(DDS_Subscriber_as_entity(datareader->subscriber)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_ENTITY)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    OSAPI_TRACE_DDS("enable datareader",RTI_TRUE)

    storage.field[0] = (void *)datareader;

#ifdef ENABLE_QOS_DEADLINE
    if (!DDS_Duration_is_infinite(&datareader->qos.deadline.period))
    {
        DDS_DeadlineQosPolicy_get_sample_freq(&datareader->qos.deadline,
                                              &deadline_sample_hz);

        if (!OSAPI_Timer_create_timeout(datareader->config->timer,
                        &datareader->deadline_event,
                        deadline_sample_hz.sec,
                        (RTI_INT32)deadline_sample_hz.nanosec,
                        OSAPI_TIMER_PERIODIC,
                        DDS_DataReaderEvent_on_deadline_timeout,
                        &storage))
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }
#endif

    if (!NETIO_Interface_set_state(datareader->rtps_intf,
                                   NETIO_INTERFACESTATE_ENABLED))
    {
        DDSC_LOG_NETIO_SET_STATE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_RTPS_NETIO_KIND,
                                 NETIO_INTERFACESTATE_ENABLED)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!NETIO_Interface_set_state(datareader->dr_intf,
                                   NETIO_INTERFACESTATE_ENABLED))
    {
        DDSC_LOG_NETIO_SET_STATE(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_DATAREADER_NETIO_KIND,
                                 NETIO_INTERFACESTATE_ENABLED)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    datareader->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

#if !ENABLE_DISCOVERY_MATCH_BUILTIN
    if (!DDS_ObjectId_is_builtin(datareader->as_entity.entity_id) &&
                                  datareader->config->on_after_enabled)
    {
        if (!datareader->config->on_after_enabled((DDS_DataReader*)self,&datareader->qos))
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
#else
    if (datareader->config->on_after_enabled)
    {
        if (!datareader->config->on_after_enabled((DDS_DataRader*)self,&datareader->qos))
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
#endif

done:

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_TopicDescription*
DDS_DataReader_get_topicdescription(DDS_DataReader *self)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(datareader == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_Topic_as_topicdescription(self->topic);
}

DDS_Subscriber*
DDS_DataReader_get_subscriber(DDS_DataReader *self)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(datareader == NULL,
                              return NULL,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->subscriber;
}

#ifndef RTI_CERT 
DDS_ReturnCode_t
DDS_DataReader_set_listener(DDS_DataReader *self,
                            const struct DDS_DataReaderListener *l,
                            DDS_StatusMask mask)
{
    struct DDS_DataReaderListener nil_listener =
                    DDS_DataReaderListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((l != NULL) && !DDS_DataReaderListener_is_consistent(l,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_LISTENER,mask)
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (l == NULL)
    {
        self->listener = nil_listener;
    }
    else
    {
        self->listener = *l;
    }

    self->mask = mask;

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

#ifndef RTI_CERT 
struct DDS_DataReaderListener
DDS_DataReader_get_listener(DDS_DataReader *self)
{
    struct DDS_DataReaderListener retval = DDS_DataReaderListener_INITIALIZER;
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    struct DDS_DataReaderListener nil_retval = DDS_DataReaderListener_INITIALIZER;

    OSAPI_PRECONDITION(datareader == NULL,
                           return retval,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    retval = datareader->listener;

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    return retval;
}
#endif 

/*ci
 * \brief Common functionality for DDS datareaer read/take APIs
 *
 * \details
 *
 * This function combines the functionality of all the datareader read/take
 * APIs. This function supports both loan and copy of the data and info
 * sequences.
 *
 * \param[in]  self            Datareader to read/take data from
 * \param[out] received_data   Sequence with 0 or more samples from the cache
 * \param[out] info_seq        Sequence with 0 or more info
 * \param[in]  max_samples     The maximum number of samples
 * \param[in]  a_handle        The intance to read/take
 * \param[in]  sample_states   Valid sample states to include
 * \param[in]  view_states     Valid view states to include
 * \param[in]  instance_states Valid instance states to include
 * \param[in]  take            DDS_BOOLEAN_TRUE if this is a take
 * \param[in]  valid_handle    DDS_BOOLEAN_TRUE is a_handle is valid
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t
 *         on error
 *
 * \sa \ref DDS_DataReader_read,
 *     \ref DDS_DataReader_take,
 *     \ref DDS_DataReader_read_instance,
 *     \ref DDS_DataReader_take_instance
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_DataReader_read_or_take(DDS_DataReader *self,
                            struct DDS_UntypedSampleSeq *received_data, 
                            struct DDS_SampleInfoSeq *info_seq,
                            DDS_Long max_samples,
                            const DDS_InstanceHandle_t *a_handle,
                            DDS_SampleStateMask sample_states,
                            DDS_ViewStateMask view_states,
                            DDS_InstanceStateMask instance_states,
                            DDS_Boolean take,
                            DDS_Boolean valid_handle)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_NO_DATA;
    DDS_ReturnCode_t retcode2 = DDS_RETCODE_ERROR;
    DDS_Boolean loan = RTI_FALSE;
    void **sample_ptrs_array = NULL;
    struct DDS_SampleInfo **info_array = NULL;
    struct DDS_SampleInfo *sample_info = NULL;
    RTI_INT32 data_seq_max;
    DDS_Long sample_count = 0;
    RTI_INT32 i;

    OSAPI_PRECONDITION_ALWAYS((datareader == NULL) || (received_data == NULL) ||
                                  (info_seq == NULL) || (valid_handle && a_handle == NULL),
         return DDS_RETCODE_BAD_PARAMETER,
        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("received_data",received_data,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("info_seq",info_seq,RTI_FALSE);
        OSAPI_Log_entry_add_int("valid_handle",valid_handle,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("a_handle",a_handle,RTI_TRUE);)


    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    /* values of len, max_len, and owns must be identical between seqs */
    if ((DDS_UntypedSampleSeq_get_maximum(received_data) != 
        DDS_SampleInfoSeq_get_maximum(info_seq)) || 
        (DDS_UntypedSampleSeq_get_length(received_data) != 
         DDS_SampleInfoSeq_get_length(info_seq)) ||
        (DDS_UntypedSampleSeq_has_ownership(received_data) !=     
         DDS_SampleInfoSeq_has_ownership(info_seq)))
    {
        DDSC_LOG_SEQ_INVALID(OSAPI_LOGKIND_ERROR,DDSC_LOG_READTAKE_SEQUENCE)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    /* cannot read/take using sequence with loaned buffer */
    if (max_samples > 0 && !DDS_UntypedSampleSeq_has_ownership(received_data))
    {
        DDSC_LOG_SEQ_INVALID(OSAPI_LOGKIND_ERROR,DDSC_LOG_READTAKE_SEQUENCE)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    data_seq_max = DDS_UntypedSampleSeq_get_maximum(received_data);

    /* cannot expect max_samples if sequence max not that large */
    if ((max_samples != DDS_LENGTH_UNLIMITED) && (data_seq_max > 0) && 
        (max_samples > data_seq_max))
    {
        DDSC_LOG_SEQ_INVALID(OSAPI_LOGKIND_ERROR,DDSC_LOG_READTAKE_SEQUENCE)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    /* set bound on max_samples returned by reader-history */
    if ((max_samples == DDS_LENGTH_UNLIMITED) ||
        (max_samples > datareader->read_seq_max))
    {
        /* set finite max_samples */
        max_samples = datareader->read_seq_max;
    }

    /* whether to loan buffers to seqs */
    loan = (data_seq_max == 0 ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);

    /* read/take from reader history */
    retcode = DDSHST_Reader_read_or_take(
       datareader->_rh, &sample_ptrs_array, &info_array, &sample_count, 
       (valid_handle ? a_handle : NULL),
       max_samples, sample_states, view_states, instance_states, take);

    if (retcode != DDS_RETCODE_OK)
    {
#if OSAPI_ENABLE_LOG
        if (retcode != DDS_RETCODE_NO_DATA)
        {
            DDSC_LOG_DR_READ_TAKE_FAILURE(OSAPI_LOGKIND_ERROR,retcode)
        }
#endif
        goto done;
    }

    retcode = DDS_RETCODE_ERROR;

    if (loan)
    {

        /* loan buffer of sample ptrs */
        if (!DDS_UntypedSampleSeq_loan_discontiguous(received_data,
                        (void *)sample_ptrs_array,sample_count,sample_count))
        {
            goto done;
        }

        /* loan buffer of sample infos */
        if (!DDS_SampleInfoSeq_loan_discontiguous(info_seq, (void*)info_array,
                                               sample_count,sample_count))
        {
            goto done;
        }

        DDS_UntypedSampleSeq_set_token(received_data,self,info_seq);
        DDS_SampleInfoSeq_set_token(info_seq,self,received_data);

        retcode = DDS_RETCODE_OK;
    }
    else /* !loan */
    {
        if (!DDS_UntypedSampleSeq_set_length(received_data, sample_count))
        {
            goto done;
        }

        if (!DDS_SampleInfoSeq_set_length(info_seq, sample_count))
        {
            goto done;
        }

        for (i = 0; i < sample_count; ++i)
        {
            /* copy info */

            /* Setting the length succeeded above
             */
            /* coverity[cert_exp34_c_violation] */
            sample_info = DDS_SampleInfoSeq_get_reference(info_seq, i);

            /* coverity[dereference] */
            *sample_info = *info_array[i];
                
            /* copy data
             * Setting the length succeeded above
             */
            /* coverity[cert_exp34_c_violation] */
            if (sample_info->valid_data)
            {
                if (!datareader->type_plugin->copy_sample(
                   datareader->type_plugin,
                   DDS_UntypedSampleSeq_get_reference(received_data, i),
                   sample_ptrs_array[i],
                   datareader->qos.type_support.plugin_data))
                {
                    DDSC_LOG_DR_COPY_DATA_SAMPLE(OSAPI_LOGKIND_ERROR)
                    break;
                }
            }
        }

        retcode2 = DDSHST_Reader_finish_read_or_take(datareader->_rh,
                     &sample_ptrs_array,&info_array,sample_count,take);

        if (i == sample_count)
        {
            /* done with successful copying, get new return code */
            retcode = retcode2;
        }
        else
        {
            /* copying failed, keep return-code. Note that if the copy fails
             * no attempt is made to roll back the history queue to the
             * previous state. The code is written like this because the
             * retcode from DDSHST_Reader_finish_read_or_take must be checked.
             */
            IGNORE_RETVAL(retcode2);
        }
    }

done:

    if (!DDS_EntityImpl_disable_status(&self->as_entity,
                                       DDS_DATA_AVAILABLE_STATUS))
    {
        retcode = DDS_RETCODE_ERROR;
    }
    
    if (!DDS_EntityImpl_disable_status(DDS_Subscriber_as_entity(self->subscriber),
                                       DDS_DATA_ON_READERS_STATUS))
    {
        retcode =  DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Common functionality for read_next_sample/take_next_sample functions
 *
 * \details
 *
 * This function combines the functionality of all the datareader
 * read_next_sample and take_next_sample APIs.
 *
 * \param[in]  self            Datareader to read/take data from
 * \param[out] received_data   sample
 * \param[out] sample_info     sample_info
 * \param[in]  take            DDS_BOOLEAN_TRUE if this is a take
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t
 *         on error
 *
 * \sa \ref DDS_DataReader_take_next_sample,
 *     \ref DDS_DataReader_read_next_sample
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_DataReader_read_or_take_next_sample(DDS_DataReader *self,
                                        void *received_data,
                                        struct DDS_SampleInfo *sample_info,
                                        DDS_Boolean take)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_NO_DATA;
    DDS_ReturnCode_t retcode2 = DDS_RETCODE_ERROR;
    void **sample_ptr_array = NULL;
    struct DDS_SampleInfo **info_array = NULL;
    DDS_Long sample_count;

    OSAPI_PRECONDITION_ALWAYS((datareader == NULL) || (received_data == NULL) ||
                           (sample_info == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("received_data",received_data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("sample_info",sample_info,RTI_TRUE);)
    
    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    /* read/take from reader history */
    retcode = DDSHST_Reader_read_or_take(
       datareader->_rh, &sample_ptr_array, &info_array, &sample_count, NULL, 1,
       DDS_NOT_READ_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE, 
       take);

    if (retcode != DDS_RETCODE_OK)
    {
#if OSAPI_ENABLE_LOG
        if (retcode != DDS_RETCODE_NO_DATA)
        {
            DDSC_LOG_DR_READ_TAKE_FAILURE(OSAPI_LOGKIND_ERROR,retcode)
        }
#endif
        goto done;
    }

    /* copy sample info */
    *sample_info = *info_array[0];

    if (sample_info->valid_data)
    {
        if (!self->type_plugin->copy_sample(datareader->type_plugin, 
                                    received_data,sample_ptr_array[0],
                                    datareader->qos.type_support.plugin_data))
        {   
            DDSC_LOG_DR_COPY_DATA_SAMPLE(OSAPI_LOGKIND_ERROR)
            retcode = DDS_RETCODE_ERROR;
        }
    }

    retcode2 = DDSHST_Reader_finish_read_or_take(datareader->_rh,
            &sample_ptr_array,&info_array, 1, take);

    if (retcode == DDS_RETCODE_ERROR)
    {
        /* If the copy was unsuccessful we ignore the return value from
         * DDSHST_Reader_finish_read_or_take and return the current value
         * of retcode. It is not sufficient to directly ignore the return
         * value from DDSHST_Reader_finish_read_or_take as this will cause
         * the compiler to give a warning that the return value is ignored.
         */
        IGNORE_RETVAL(retcode2);
    }
    else
    {
        /* otherwise the copy was successful and the return value
         * DDSHST_Reader_finish_read_or_take returneds.
         */
        retcode = retcode2;
    }

done:

    if (!DDS_EntityImpl_disable_status(&self->as_entity,
                                       DDS_DATA_AVAILABLE_STATUS))
    {
        retcode =  DDS_RETCODE_ERROR;
    }

    if (!DDS_EntityImpl_disable_status(DDS_Subscriber_as_entity(self->subscriber),
                                       DDS_DATA_ON_READERS_STATUS))
    {
        retcode =  DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_read(
        DDS_DataReader *self,
        struct DDS_UntypedSampleSeq *received_data,
        struct DDS_SampleInfoSeq *info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read_or_take(self, received_data, info_seq, 
        max_samples, NULL, sample_states, view_states, instance_states, 
        DDS_BOOLEAN_FALSE, DDS_BOOLEAN_FALSE);
}

DDS_ReturnCode_t
DDS_DataReader_take(
        DDS_DataReader *self,
        struct DDS_UntypedSampleSeq *received_data,
        struct DDS_SampleInfoSeq *info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read_or_take(self, received_data, info_seq, 
        max_samples, NULL, sample_states, view_states, instance_states, 
        DDS_BOOLEAN_TRUE, DDS_BOOLEAN_FALSE);
}

DDS_ReturnCode_t
DDS_DataReader_read_instance(
        DDS_DataReader *self,
        struct DDS_UntypedSampleSeq *received_data,
        struct DDS_SampleInfoSeq *info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t *a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read_or_take(self, received_data, info_seq, 
        max_samples, a_handle, sample_states, view_states, instance_states, 
        DDS_BOOLEAN_FALSE, DDS_BOOLEAN_TRUE);
}

DDS_ReturnCode_t
DDS_DataReader_take_instance(
        DDS_DataReader *self,
        struct DDS_UntypedSampleSeq *received_data,
        struct DDS_SampleInfoSeq *info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t *a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read_or_take(self, received_data, info_seq, 
        max_samples, a_handle, sample_states, view_states, instance_states, 
        DDS_BOOLEAN_TRUE, DDS_BOOLEAN_TRUE);
}

DDS_ReturnCode_t
DDS_DataReader_read_next_sample(DDS_DataReader *self,
                                void *received_data,
                                struct DDS_SampleInfo *sample_info)
{
    return DDS_DataReader_read_or_take_next_sample(
       self, received_data, sample_info, DDS_BOOLEAN_FALSE /* take */);
}

DDS_ReturnCode_t
DDS_DataReader_take_next_sample(DDS_DataReader *self,
                                void *received_data,
                                struct DDS_SampleInfo *sample_info)
{
    return DDS_DataReader_read_or_take_next_sample(
       self, received_data, sample_info, DDS_BOOLEAN_TRUE /* take */);
}


DDS_ReturnCode_t
DDS_DataReader_return_loan(DDS_DataReader *self,
                           struct DDS_UntypedSampleSeq *received_data,
                           struct DDS_SampleInfoSeq *info_seq)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_OK;
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    void **sample_ptr_array = NULL;
    struct DDS_SampleInfo **info_array = NULL;
    DDS_Long sample_count;
    void *token1;
    void *token2;

    OSAPI_PRECONDITION_ALWAYS(self == NULL || received_data == NULL ||
                                  info_seq == NULL,
                   return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("received_data",received_data,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("info_seq",info_seq,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DDS_UntypedSampleSeq_has_ownership(received_data) || 
        DDS_SampleInfoSeq_has_ownership(info_seq))
    {
        /* sequence(s) not loaned --> no side effects */
        return DDS_RETCODE_OK;
    }

    DDS_UntypedSampleSeq_get_token(received_data,&token1,&token2);
    if (((DDS_DataReader*)token1 != self) ||
        ((struct DDS_SampleInfoSeq*)token2 != info_seq))
    {
        DDSC_LOG_SEQ_INVALID(OSAPI_LOGKIND_ERROR,DDSC_LOG_READTAKE_SEQUENCE)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    DDS_SampleInfoSeq_get_token(info_seq,&token1,&token2);
    if (((DDS_DataReader*)token1 != self) ||
         ((struct DDS_UntypedSampleSeq*)token2 != received_data))
    {
        DDSC_LOG_SEQ_INVALID(OSAPI_LOGKIND_ERROR,DDSC_LOG_READTAKE_SEQUENCE)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    /* get (loaned) buffers from sequences. return_loan() will return them to
       the reader queue */
    sample_ptr_array = (void**)DDS_UntypedSampleSeq_get_contiguous_buffer(received_data);
    info_array = (struct DDS_SampleInfo**)DDS_SampleInfoSeq_get_contiguous_buffer(info_seq);
    sample_count = DDS_SampleInfoSeq_get_length(info_seq);

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retval = DDSHST_Reader_finish_read_or_take(datareader->_rh,
                                               &sample_ptr_array,&info_array,
                                               sample_count,DDS_BOOLEAN_FALSE);

    if (retval != DDS_RETCODE_OK)
    {
        DDSC_LOG_DR_READ_TAKE_FAILURE(OSAPI_LOGKIND_ERROR,retval)
        goto done;
    }

    if (!DDS_UntypedSampleSeq_unloan(received_data))
    {
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!DDS_SampleInfoSeq_unloan(info_seq))
    {
        retval = DDS_RETCODE_ERROR;
        goto done;
    }

done:

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retval;
}


DDS_InstanceHandle_t
DDS_DataReader_lookup_instance(DDS_DataReader *self,
                               const void *key_holder)
{
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;
    DDS_InstanceHandle_t nil_handle = DDS_HANDLE_NIL;
    DDS_KeyHash_t key_hash;
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (key_holder == NULL),
                           return nil_handle,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("key_holder",key_holder,RTI_TRUE);)

    /* check key kind, does not require a lock */
    if (datareader->type_plugin->get_key_kind(datareader->type_plugin,
            datareader->qos.type_support.plugin_data) == NDDS_TYPEPLUGIN_NO_KEY)
    {
        /* no key, not failure */
        return nil_handle;
    }

    key_hash.length = RTPS_KEY_HASH_MAX_LENGTH;
    OSAPI_Memory_zero(key_hash.value, key_hash.length);
    OSAPI_Memory_zero(handle.octet, key_hash.length);

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return nil_handle;
    }

    CDR_Stream_reset(datareader->md5_stream);

    if (!datareader->type_plugin->instance_to_keyhash(
       datareader->type_plugin, datareader->md5_stream, &key_hash, key_holder,  
       datareader->qos.type_support.plugin_data))
    {
        DDSC_LOG_DR_INSTANCE_TO_KEYHASH(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    OSAPI_Memory_copy(handle.octet, key_hash.value, key_hash.length);

    if (DDSHST_Reader_lookup_key(datareader->_rh, &handle) != NULL)
    {
        handle.is_valid = DDS_BOOLEAN_TRUE;
    }

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        return nil_handle;
    }

    return (handle.is_valid ? handle : nil_handle);
}

void
DDS_DataReader_liveliness_lost(DDS_DataReader *self,
                               DDS_InstanceHandle_t *publication_handle)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    struct DDSHST_ReaderEvent event;

    event.kind = DDSHST_READEREVENT_KIND_REMOTE_WRITER_DELETED;
    event.data.rw_deleted.rw_guid = *publication_handle;
    event.data.rw_deleted.rw_guid.is_valid = DDS_BOOLEAN_TRUE;

    if (self->_rh)
    {
        DDSHST_Reader_post_event(datareader->_rh, &event, NULL);
    }
}

/*******************************************************************************
 *                             OPTIONAL APIs
 ******************************************************************************/
#if INCLUDE_API_QOS
#ifndef RTI_CERT
/* Not in CERT */
DDS_ReturnCode_t
DDS_DataReader_set_qos(DDS_DataReader *self,
                       const struct DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;

    OSAPI_PRECONDITION((datareader == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_DataReaderQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_Entity_is_enabled(DDS_DataReader_as_entity(datareader)))
    {
        DDSC_LOG_QOS_SET_ON_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_DataReaderQos_immutable_is_equal(&datareader->qos, qos))
    {
        DDSC_LOG_QOS_IMMUTABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        retcode = DDS_RETCODE_IMMUTABLE_POLICY;
        goto done;
    }

    retcode = DDS_DataReaderQos_finalize(&datareader->qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        goto done;
    }

    retcode = DDS_DataReaderQos_initialize(&datareader->qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS,retcode)
        goto done;
    }

    retcode = DDS_DataReaderQos_copy(&datareader->qos, qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        goto done;
    }

done:

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */
#endif

#ifndef RTI_CERT
struct DDS_DataReaderQos*
DDS_DataReader_get_qos_ref(DDS_DataReader *self)
{
    OSAPI_PRECONDITION(self == NULL,
                       return NULL,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

   return &self->qos;
}
#endif

#if INCLUDE_API_QOS
#ifndef RTI_CERT
/* Not in CERT */
DDS_ReturnCode_t
DDS_DataReader_get_qos(DDS_DataReader *self, struct DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;

    OSAPI_PRECONDITION((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)


    if (DB_Database_lock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DataReaderQos_copy(qos, &self->qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
    }
#endif

    if (DB_Database_unlock(datareader->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */
#endif

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDS_DataReader_get_matched_publications(DDS_DataReader *self,
                            struct DDS_InstanceHandleSeq *publication_handles)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    PRECOND_ARG(publication_handles)

    OSAPI_PRECONDITION((datareader == NULL) ||
                           (publication_handles == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("publication_handles",publication_handles,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&datareader->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    return DDS_RETCODE_UNSUPPORTED;
}
#endif /* INCLUDE_API_LOOKUP */

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDS_DataReader_get_matched_publication_data(DDS_DataReader *self,
                struct DDS_PublicationBuiltinTopicData *publication_data,
                const DDS_InstanceHandle_t *publication_handle)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    PRECOND_ARG(publication_data)
    PRECOND_ARG(publication_handle)

    OSAPI_PRECONDITION((datareader == NULL) ||
                            (publication_data == NULL) ||
                            (publication_handle == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("publication_data",publication_data,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("publication_handle",publication_handle,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&datareader->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    return DDS_RETCODE_UNSUPPORTED;
}
#endif /* INCLUDE_API_LOOKUP */

DDS_ReturnCode_t
DDS_DataReader_get_sample_rejected_status(
        DDS_DataReader *self,
        struct DDS_SampleRejectedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->sample_rejected_status;

    DDS_SampleRejectedStatus_reset(&self->sample_rejected_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_SAMPLE_REJECTED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_get_liveliness_changed_status(
        DDS_DataReader *self,
        struct DDS_LivelinessChangedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->liveliness_changed_status;

    DDS_LivelinessChangedStatus_reset(&self->liveliness_changed_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_LIVELINESS_CHANGED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_get_requested_deadline_missed_status(
        DDS_DataReader *self,
        struct DDS_RequestedDeadlineMissedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->req_deadline_missed_status;

    DDS_RequestedDeadlineMissedStatus_reset(&self->req_deadline_missed_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_REQUESTED_DEADLINE_MISSED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode ;
}

DDS_ReturnCode_t
DDS_DataReader_get_requested_incompatible_qos_status(
        DDS_DataReader *self,
        struct DDS_RequestedIncompatibleQosStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->req_incompatible_qos_status;

    DDS_RequestedIncompatibleQosStatus_reset(&self->req_incompatible_qos_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_get_subscription_matched_status(
        DDS_DataReader *self,
        struct DDS_SubscriptionMatchedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->subscription_matched_status;

    DDS_SubscriptionMatchedStatus_reset(&self->subscription_matched_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_SUBSCRIPTION_MATCHED_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_get_sample_lost_status(
        DDS_DataReader *self,
        struct DDS_SampleLostStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->sample_lost_status;

    DDS_SampleLostStatus_reset(&self->sample_lost_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_SAMPLE_LOST_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_get_instance_replaced_status(
        DDS_DataReader *self,
        struct DDS_DataReaderInstanceReplacedStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->instance_replaced_status;

    DDS_DataReaderInstanceReplacedStatus_reset(&self->instance_replaced_status);
    if (!DDS_EntityImpl_disable_status(DDS_DataReader_as_entity(self),
                                       DDS_INSTANCE_REPLACED_STATUS))
    {
        goto done;
    }
    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci @} */
