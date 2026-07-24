/*
 * FILE: Type.c - DDS Type implementation
 *
 * (c) Copyright 2008-2025 Real-Time Innovations,
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
 * 03oct2016,tk  MICRO-1569 Improved type registration/unregistration API
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 09feb2015,tk MICRO-1062/PR#13614 Use DDS_BOOLEAN_FALSE instead of RTI_FALSE
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous parameters in DB
 *                                  compare function
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 11jun2013,tk MICRO-633: max type length is 255 excluding NUL
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS Type implementation
 *
 * \details
 * This file implements the API to manage DDS data-types.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#include "Type.h"

/*** SOURCE_BEGIN ***/
RTI_PRIVATE const struct DDS_TypePlugin DDS_TypePluginInitializer = DDS_TypePlugin_INITIALIZER;

DDSCDllExport void
DDS_TypePlugin_initialize(struct DDS_TypePlugin *plugin)
{
    *plugin = DDS_TypePluginInitializer;
}

void*
DDS_TypePlugin_init_inline_buffer(struct DDS_TypeEncapsulationPlugin *wp,
                                  REDA_BufferPool_T pool,RTI_UINT32 size)
{
    struct DDS_TypePluginBuffer *tbuf = NULL;

    tbuf = (struct DDS_TypePluginBuffer*)REDA_BufferPool_get_buffer(pool);
    if (tbuf == NULL)
    {
        return NULL;
    }

    tbuf->wp = wp;
    tbuf->data = NULL;

    /* Only one pbuf is used for data */
    tbuf->data_pbuf._next = NULL;
    tbuf->data_pbuf.buffer = (char*)&tbuf[1];
    tbuf->data_pbuf.head_pos = 0;
    tbuf->data_pbuf.tail_pos = 0;
    tbuf->data_pbuf.max_length = size - (RTI_UINT32)sizeof(struct DDS_TypePluginBuffer);

    return tbuf;
}

struct DDS_TypeEncapsulationPlugin*
DDS_TypePlugin_find_encapsulation_plugin(struct DDS_TypePlugin *plugin,
                                         DDS_EncapsulationId_t enc_id,
                                         DDS_DataRepresentationId_t dr_id)
{
    struct DDS_TypeEncapsulationPlugin *wp;
    struct DDS_TypeEncapsulationPlugin *retval = NULL;

    /* The assumption is that dr_id is already valid */
    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);

    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        if ((wp->_intf->encapsulation_kind[0].le_identifier == enc_id) ||
            (wp->_intf->encapsulation_kind[0].be_identifier == enc_id))
        {
            if (dr_id == DDS_INVALID_DATA_REPRESENTATION)
            {
                retval = wp;
            }
            else if (wp->_intf->representation_id == dr_id)
            {
                retval = wp;
            }
            else if (wp->_intf->representation_id_aliases != NULL)
            {
                DDS_DataRepresentationId_t *id_alias =
                                        wp->_intf->representation_id_aliases;

                while ((*id_alias != DDS_INVALID_DATA_REPRESENTATION) &&
                       (*id_alias != dr_id))
                {
                    ++id_alias;
                }

                if (*id_alias == dr_id)
                {
                    retval = wp;
                }
            }

            break;
        }

        wp = (struct DDS_TypeEncapsulationPlugin*)
                                    REDA_CircularListNode_get_next(&wp->_node);
    }

    return retval;
}

DDS_Boolean
DDS_TypePlugin_is_representation_supported(struct DDS_TypePlugin *plugin,
                                           DDS_DataRepresentationId_t dr_id)
{
    RTI_UINT32 count;
    struct DDS_TypeEncapsulationI *wire_intf;

    count = 0;
    while (plugin->_intf->wire_intf[count] != NULL)
    {
        wire_intf = plugin->_intf->wire_intf[count];

        if (dr_id == wire_intf->representation_id)
        {
            return DDS_BOOLEAN_TRUE;
        }

        if (wire_intf->representation_id_aliases != NULL)
        {
            DDS_DataRepresentationId_t *id = wire_intf->representation_id_aliases;

            while ((*id != DDS_INVALID_DATA_REPRESENTATION) &&
                   (*id != dr_id))
            {
                ++id;
            }

            if (*id == dr_id)
            {
                return DDS_BOOLEAN_TRUE;
            }
        }
        ++count;
    }

    return DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE struct DDS_TypeMemoryPlugin *
DDS_TypePlugin_assert_memory_plugin(struct DDS_TypePlugin *plugin,
                                    struct DDS_TypeEncapsulationI *wire_intf,
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos_deprecated)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeMemoryI *memory_intf = NULL;
    RTI_UINT32 count;

    if (plugin->_intf->memory_intf == NULL)
    {
        /* no mp to be found */
        goto done;
    }

    /* Find memory plugin */
    mp = (struct DDS_TypeMemoryPlugin*)REDA_CircularList_get_first(&plugin->memory_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        if (mp->_intf->id == wire_intf->memory_id)
        {
            break;
        }

        if ((mp->_intf->type == RTI_MEMORY_TYPE_SHMEM) &&
            (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER))
        {
            /* only one memory plugin with shmem type can exist for the writer.
             * If one has already been created refuse creation of another
             */
            return NULL;

        }

        mp = (struct DDS_TypeMemoryPlugin*)REDA_CircularListNode_get_next(&mp->_node);
    }

    if (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        /* mp is ok */
        goto done;
    }

    mp = NULL;
    count = 0;

    while (plugin->_intf->memory_intf[count] != NULL)
    {
        memory_intf = plugin->_intf->memory_intf[count];

        if (memory_intf->id == wire_intf->memory_id)
        {
            break;
        }
        memory_intf = NULL;
        count++;
    }

    if (memory_intf == NULL)
    {
        /* no mp found */
        goto done;
    }

    mp = memory_intf->create_plugin(plugin,participant,dp_qos,
                                    endpoint_mode,endpoint,qos_deprecated);
    if (mp == NULL)
    {
        goto done;
    }

    mp->_intf = memory_intf;
    REDA_CircularListNode_init(&mp->_node);
    REDA_CircularList_append(&plugin->memory_plugins,&mp->_node);

done:
    return mp;
}

RTI_PRIVATE DDS_Boolean
DDS_TypePlugin_set_defaults(struct DDS_TypePlugin *plugin)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;

    if (!REDA_CircularList_is_empty(&plugin->wire_plugins))
    {
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                        REDA_CircularList_get_first(&plugin->wire_plugins);
#ifdef RTI_ENDIAN_LITTLE
        if (!DDS_TypePlugin_set_encapsulation(plugin,
                                              wp->_intf->encapsulation_kind[0].le_identifier))
        {
            return DDS_BOOLEAN_FALSE;
        }
#else
        if (!DDS_TypePlugin_set_encapsulation(plugin,
                                              wp->_intf->encapsulation_kind[0].be_identifier))
        {
            return DDS_BOOLEAN_FALSE;
        }
#endif
    }

    if (!REDA_CircularList_is_empty(&plugin->memory_plugins))
    {
        mp = (struct DDS_TypeMemoryPlugin*)
                                    REDA_CircularList_get_first(&plugin->memory_plugins);
        plugin->allocator_plugin = mp;
        if (mp->_intf->type != RTI_MEMORY_TYPE_SHMEM)
        {
            while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
            {
                if (mp->_intf->type == RTI_MEMORY_TYPE_SHMEM)
                {
                    plugin->allocator_plugin = mp;
                    break;
                }
                mp = (struct DDS_TypeMemoryPlugin*)REDA_CircularListNode_get_next(&mp->_node);
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE void
DDS_TypePlugin_add_data_representation(struct DDS_TypePlugin *plugin,
                                       DDS_DataRepresentationId_t id)
{
    RTI_INT32 index = 0;

    for (index = 0; index < plugin->representation_count; ++index)
    {
        if (plugin->representation[index] == id)
        {
            return;
        }
    }

    plugin->representation[plugin->representation_count] = id;
    ++plugin->representation_count;
}

RTI_PRIVATE DDS_Boolean
DDS_TypePlugin_create_writer_plugin(struct DDS_TypePlugin *plugin,
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_DataWriter *writer,
                                    struct DDS_DataWriterQos *dw_qos_deprecated)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    struct DDS_TypeEncapsulationI *wire_intf;
    RTI_UINT32 count;
    DDS_EncapsulationId_t enc_id = DDS_ENCAPSULATION_ID_INVALID;
    RTI_INT32 ts_length = 0;
    struct DDS_TransportEncapsulationSettings_t *ts;
    const struct DDS_TransportEncapsulationQosPolicy *t_qos;
    RTI_BOOL ec_found;
    DDS_Long data_rep_length;
    DDS_Long ec_length;
    DDS_Long ts_index,ec_index;
    const struct DDS_DataRepresentationQosPolicy *data_rep_qos;
    DDS_DataRepresentationId_t drid;
    UNUSED_ARG(dw_qos_deprecated);

    if (plugin->_intf->wire_intf == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    data_rep_qos = DDS_DataWriter_get_data_representation_ref(writer);
    t_qos = DDS_DataWriter_get_encapsulation_ref(writer);

    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&t_qos->value);
    data_rep_length = DDS_DataRepresentationIdSeq_get_length(&data_rep_qos->value);

    if (data_rep_length > 1)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* The specification allows mis-configuration by the application, find
     * the best option.
     */
    if (data_rep_length == 0)
    {
        if (DDS_TypePlugin_is_representation_supported(plugin,DDS_XCDR_DATA_REPRESENTATION))
        {
            drid = DDS_XCDR_DATA_REPRESENTATION;
        }
        else if (DDS_TypePlugin_is_representation_supported(plugin,DDS_XCDR2_DATA_REPRESENTATION))
        {
            drid = DDS_XCDR2_DATA_REPRESENTATION;
        }
        else
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    else
    {
        drid = *DDS_DataRepresentationIdSeq_get_reference(&data_rep_qos->value,0);

        if (drid == DDS_AUTO_DATA_REPRESENTATION)
        {
            if (DDS_TypePlugin_is_representation_supported(plugin,
                                               DDS_XCDR_DATA_REPRESENTATION))
            {
                drid = DDS_XCDR_DATA_REPRESENTATION;
            }
            else if (DDS_TypePlugin_is_representation_supported(plugin,
                                                DDS_XCDR2_DATA_REPRESENTATION))
            {
                drid = DDS_XCDR2_DATA_REPRESENTATION;
            }
            else
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (!DDS_TypePlugin_is_representation_supported(plugin,drid))
        {
            if ((drid == DDS_XCDR_DATA_REPRESENTATION) &&
                DDS_TypePlugin_is_representation_supported(plugin,
                                               DDS_XCDR2_DATA_REPRESENTATION))
            {
                DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR(OSAPI_LOGKIND_WARNING,
                                           DDS_XCDR2_DATA_REPRESENTATION)
                drid =  DDS_XCDR2_DATA_REPRESENTATION;
            }
            else if ((drid == DDS_XCDR2_DATA_REPRESENTATION) &&
                     DDS_TypePlugin_is_representation_supported(plugin,
                                                DDS_XCDR_DATA_REPRESENTATION))
            {
                DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR(OSAPI_LOGKIND_WARNING,
                                           DDS_XCDR_DATA_REPRESENTATION)
                drid =  DDS_XCDR_DATA_REPRESENTATION;
            }
            else
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    /* There may be multiple plugins supporting the same data representation,
     * create all of them.
     */
    count = 0;
    while (plugin->_intf->wire_intf[count] != NULL)
    {
        DDS_DataRepresentationId_t match_id = DDS_INVALID_DATA_REPRESENTATION;

        wire_intf = plugin->_intf->wire_intf[count];
        if (wire_intf->representation_id == drid)
        {
            match_id = drid;
        }
        else if (wire_intf->representation_id_aliases != NULL)
        {
            DDS_DataRepresentationId_t *id = wire_intf->representation_id_aliases;

            while ((*id != DDS_INVALID_DATA_REPRESENTATION) && (*id != drid))
            {
                ++id;
            }

            if (*id == drid)
            {
                match_id = drid;
            }
        }

        if (match_id == DDS_INVALID_DATA_REPRESENTATION)
        {
            ++count;
            continue;
        }

        /* Found a match, check if there any encapsulations */
        if (ts_length > 0)
        {
            ec_found = RTI_FALSE;

            for (ts_index = 0; ts_index < ts_length; ++ts_index)
            {
                ts = DDS_TransportEncapsulationSettingsSeq_get_reference(&t_qos->value,ts_index);
                if (ts == NULL)
                {
                    return DDS_BOOLEAN_FALSE;
                }

                ec_length = DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);
                for (ec_index = 0; ec_index < ec_length; ++ec_index)
                {
                    DDS_EncapsulationId_t * enc_id_ref =
                        DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,
                                                            ec_index);
                    if (enc_id_ref == NULL)
                    {
                        return DDS_BOOLEAN_FALSE;
                    }

                    enc_id = *enc_id_ref;
                    if ((wire_intf->encapsulation_kind[0].le_identifier == enc_id) ||
                        (wire_intf->encapsulation_kind[0].be_identifier == enc_id))
                    {
                        ec_found = RTI_TRUE;
                        break;
                    }
                }
            }

            if (!ec_found)
            {
	            ++count;
    	        continue;
            }
        }
        else
        {
            enc_id = wire_intf->encapsulation_kind[0].le_identifier;
        }

        if (enc_id == DDS_ENCAPSULATION_ID_INVALID)
        {
            return DDS_BOOLEAN_FALSE;
        }

        wp = DDS_TypePlugin_find_encapsulation_plugin(plugin,enc_id,
                                                      DDS_INVALID_DATA_REPRESENTATION);
        if (wp != NULL)
        {
       		/* No duplicate encapsulation ids */
            return DDS_BOOLEAN_FALSE;
        }

        mp = DDS_TypePlugin_assert_memory_plugin(plugin,wire_intf,
                                                 participant,dp_qos,
                                                 DDS_TYPEPLUGIN_MODE_WRITER,
                                                 writer,NULL);

        if (mp == NULL)
        {
            /* If memory plugin is not created try other wire interfaces */
            ++count;
            continue;
        }

        wp = wire_intf->create_plugin(plugin,participant,dp_qos,
                                      DDS_TYPEPLUGIN_MODE_WRITER,
                                      writer,NULL,mp);
        if (wp == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        /* A writer can only support 1 data representation */
        DDS_TypePlugin_add_data_representation(plugin,drid);
        wp->_intf = wire_intf;
        REDA_CircularListNode_init(&wp->_node);
        REDA_CircularList_append(&plugin->wire_plugins,&wp->_node);
        count++;
    }

    if (!DDS_TypePlugin_set_defaults(plugin))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_TypePlugin_create_reader_plugin(struct DDS_TypePlugin *plugin,
                                  DDS_DomainParticipant *participant,
                                  struct DDS_DomainParticipantQos *dp_qos,
                                  DDS_DataReader *reader,
                                  struct DDS_DataReaderQos *qos_deprecated)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    struct DDS_TypeEncapsulationI *wire_intf;
    RTI_UINT32 count;
    DDS_EncapsulationId_t enc_id = DDS_ENCAPSULATION_ID_INVALID;
    RTI_INT32 ts_length = 0;
    struct DDS_TransportEncapsulationSettings_t *ts;
    const struct DDS_TransportEncapsulationQosPolicy *t_qos;
    RTI_BOOL ec_found;
    DDS_Long ec_length;
    DDS_Long ts_index,ec_index;
    const struct DDS_DataRepresentationQosPolicy *data_rep_qos;
    DDS_Long data_rep_index,data_rep_length;
    DDS_DataRepresentationId_t sanitized_drid[DDS_DATA_REPRESENTATION_COUNT];
    DDS_Long sanitized_length = 0;
    UNUSED_ARG(qos_deprecated);

    if (plugin->_intf->wire_intf == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    data_rep_qos = DDS_DataReader_get_data_representation_ref(reader);
    t_qos = DDS_DataReader_get_encapsulation_ref(reader);

    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&t_qos->value);
    data_rep_length = DDS_DataRepresentationIdSeq_get_length(&data_rep_qos->value);

    /* Sanitize the data representations provided by the application. The
     * specification allows the user to specify the wrong data representation
     * and automatically fix it.
     */
    if (data_rep_length > 0)
    {
        DDS_DataRepresentationId_t drid = DDS_INVALID_DATA_REPRESENTATION;
        DDS_Long index = 0;

        /* Figure out what the application was trying to do. If the specified
         * representation is not supported, pick something else that is.
         */
        for (data_rep_index = 0; data_rep_index < data_rep_length; ++data_rep_index)
        {
            drid = *DDS_DataRepresentationIdSeq_get_reference(
                                        &data_rep_qos->value,data_rep_index);
            if (drid == DDS_AUTO_DATA_REPRESENTATION)
            {
                /* Try everything */
                sanitized_drid[0] = DDS_XCDR2_DATA_REPRESENTATION;
                sanitized_drid[1] = DDS_XCDR_DATA_REPRESENTATION;
                sanitized_length = 2;
                break;
            }

            if (!DDS_TypePlugin_is_representation_supported(plugin,drid))
            {
                if ((drid == DDS_XCDR_DATA_REPRESENTATION) &&
                    DDS_TypePlugin_is_representation_supported(plugin,
                                               DDS_XCDR2_DATA_REPRESENTATION))
                {
                    DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR(OSAPI_LOGKIND_WARNING,
                                               DDS_XCDR2_DATA_REPRESENTATION)
                    drid =  DDS_XCDR2_DATA_REPRESENTATION;
                }
                else if ((drid == DDS_XCDR2_DATA_REPRESENTATION) &&
                    DDS_TypePlugin_is_representation_supported(plugin,
                                               DDS_XCDR_DATA_REPRESENTATION))
                {
                    DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR(OSAPI_LOGKIND_WARNING,
                                               DDS_XCDR2_DATA_REPRESENTATION)
                    drid =  DDS_XCDR_DATA_REPRESENTATION;
                }
            }

            /* Prevent duplicates */
            for (index = 0; index < sanitized_length; ++index)
            {
                if (sanitized_drid[index] == drid)
                {
                    break;
                }
            }

            /* The assumption is that the Qos has already been validated */
            if (index == sanitized_length)
            {
                sanitized_drid[sanitized_length] = drid;
                ++sanitized_length;
            }
        }
    }
    else
    {
        /* Try everything */
        sanitized_drid[0] = DDS_XCDR2_DATA_REPRESENTATION;
        sanitized_drid[1] = DDS_XCDR_DATA_REPRESENTATION;
        sanitized_length = 2;
    }

    count = 0;
    while (plugin->_intf->wire_intf[count] != NULL)
    {
        DDS_DataRepresentationId_t dr_id = DDS_INVALID_DATA_REPRESENTATION;
        DDS_DataRepresentationId_t match_id = DDS_INVALID_DATA_REPRESENTATION;

        wire_intf = plugin->_intf->wire_intf[count];

        for (data_rep_index = 0; data_rep_index < sanitized_length; ++data_rep_index)
        {
            dr_id = sanitized_drid[data_rep_index];

            if (dr_id == wire_intf->representation_id)
            {
                match_id = dr_id;
                DDS_TypePlugin_add_data_representation(plugin,dr_id);
            }

            /* Check if the plugin supports more requested data representations.
             */
            if (wire_intf->representation_id_aliases != NULL)
            {
                DDS_DataRepresentationId_t *id = wire_intf->representation_id_aliases;

                while (*id != DDS_INVALID_DATA_REPRESENTATION)
                {
                    RTI_INT32 sl = 0;

                    for (sl = 0; sl < sanitized_length; ++sl)
                    {
                        if (sanitized_drid[sl] == *id)
                        {
                            DDS_TypePlugin_add_data_representation(plugin,*id);
                        }
                    }

                    if ((*id == dr_id) && (match_id == DDS_INVALID_DATA_REPRESENTATION))
                    {
                        match_id = dr_id;
                    }
                    ++id;
                }
            }
        }

        if (match_id == DDS_INVALID_DATA_REPRESENTATION)
        {
            ++count;
            continue;
        }

        if (ts_length > 0)
        {
            ec_found = RTI_FALSE;
            for (ts_index = 0; ts_index < ts_length; ++ts_index)
            {
                ts = DDS_TransportEncapsulationSettingsSeq_get_reference(&t_qos->value,ts_index);
                if (ts == NULL)
                {
                    return DDS_BOOLEAN_FALSE;
                }

                ec_length = DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);
                for (ec_index = 0; ec_index < ec_length; ++ec_index)
                {
                    DDS_EncapsulationId_t * enc_id_ref =
                        DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,
                                                            ec_index);
                    if (enc_id_ref == NULL)
                    {
                        return DDS_BOOLEAN_FALSE;
                    }

                    enc_id = *enc_id_ref;
                    if ((wire_intf->encapsulation_kind[0].le_identifier == enc_id) ||
                        (wire_intf->encapsulation_kind[0].be_identifier == enc_id))
                    {
                        ec_found = RTI_TRUE;
                        break;
                    }
                }
            }

            if (!ec_found)
            {
                ++count;
                continue;
            }
        }
        else
        {
            enc_id = wire_intf->encapsulation_kind[0].le_identifier;
        }

        if (enc_id == DDS_ENCAPSULATION_ID_INVALID)
        {
            return DDS_BOOLEAN_FALSE;
        }

        wp = DDS_TypePlugin_find_encapsulation_plugin(plugin,enc_id,
                                              DDS_INVALID_DATA_REPRESENTATION);
        if (wp != NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }
        mp = DDS_TypePlugin_assert_memory_plugin(plugin,wire_intf,
                                               participant,dp_qos,
                                               DDS_TYPEPLUGIN_MODE_READER,
                                               reader,
                                               NULL);

        if (mp == NULL)
        {
            ++count;
            continue;

        }

        wp = wire_intf->create_plugin(plugin,participant,dp_qos,
                                      DDS_TYPEPLUGIN_MODE_READER,reader,
                                      NULL,mp);
        if (wp == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        wp->_intf = wire_intf;
        REDA_CircularListNode_init(&wp->_node);
        REDA_CircularList_append(&plugin->wire_plugins,&wp->_node);
        count++;
    }

    if (!DDS_TypePlugin_set_defaults(plugin))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}



/*ci
 * \brief Determine the CDR encapsulation for the type-plugin
 *
 * \details
 *
 * The CDR encapsulation is the encapsulation that would be used if data was sent inline in CDR
 * format. For example, a shared memory reference can never be an CDR encapsulation. This
 * function determines the CDR encapsulation based on the instantiated type-plugin.
 *
 * The encapsulation is returned as follows:
 * - If only V1 is supported a V1 CDR encapsulation
 * - If only V2 is supported a V2 CDR encapsulation
 * - If V1 and V2 is supported a V1 CDR encapsulation
 *
 * \return CDR encapsulation
 */
RTI_PRIVATE DDS_EncapsulationId_t
DDS_TypePlugin_determine_cdr_encapsulation(struct DDS_TypePlugin *plugin)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    DDS_EncapsulationId_t cdr_id;
    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
#ifdef RTI_ENDIAN_LITTLE
    cdr_id = wp->_intf->encapsulation_kind[0].le_identifier;
#else
    cdr_id = wp->_intf->encapsulation_kind[0].be_identifier;
#endif

    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
#ifdef RTI_ENDIAN_LITTLE
        if (wp->_intf->encapsulation_kind[0].le_identifier < cdr_id)
        {
            cdr_id = wp->_intf->encapsulation_kind[0].le_identifier;
        }
#else
        if (wp->_intf->encapsulation_kind[0].be_identifier < cdr_id)
        {
            cdr_id = wp->_intf->encapsulation_kind[0].be_identifier;
        }
#endif
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    return cdr_id;

}

DDS_Boolean
DDS_TypePlugin_create_wire_plugin(struct DDS_TypePlugin *plugin,
                                  DDS_DomainParticipant *participant,
                                  struct DDS_DomainParticipantQos *dp_qos,
                                  DDS_TypePluginMode_T endpoint_mode,
                                  DDS_TypePluginEndpoint *endpoint,
                                  DDS_TypePluginEndpointQos *qos_deprecated)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    if (plugin->_intf->wire_intf == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

#if DDS_XTYPES_IS_ENABLED
    if ((plugin->property.program_context == NULL) &&
        (plugin->_intf->type_factory != NULL))
    {
        plugin->property.program_context =
                DDS_TypeInterfaceI_create_execution_context(
                        plugin->_intf->type_factory, NULL);

        if (plugin->property.program_context == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
#endif

    if (endpoint_mode == DDS_TYPEPLUGIN_MODE_WRITER)
    {
        retval = DDS_TypePlugin_create_writer_plugin(plugin,participant,
                                                   dp_qos,
                                                   (DDS_DataWriter*)endpoint,
                                                   NULL);
    }
    else if (endpoint_mode == DDS_TYPEPLUGIN_MODE_READER)
    {
        retval = DDS_TypePlugin_create_reader_plugin(plugin,participant,
                                                   dp_qos,
                                                   (DDS_DataReader*)endpoint,
                                                   qos_deprecated);
    }

    if (retval)
    {
        plugin->cdr_id = DDS_TypePlugin_determine_cdr_encapsulation(plugin);
    }


    return retval;
}

struct DDS_TypePlugin*
DDS_TypePlugin_create_w_intf(struct DDS_TypePluginI *intf,
                             DDS_DomainParticipant *participant,
                             struct DDS_DomainParticipantQos *dp_qos,
                             DDS_TypePluginMode_T endpoint_mode,
                             DDS_TypePluginEndpoint *endpoint,
                             DDS_TypePluginEndpointQos *qos_deprecated,
                             struct DDS_TypePluginProperty *property)
{
    struct DDS_TypePlugin *plugin = NULL;

    plugin = DDS_TypePlugin_create(intf,participant,dp_qos,
                                   endpoint_mode,endpoint,qos_deprecated,
                                   property);
    if (plugin == NULL)
    {
        return NULL;
    }

    plugin->representation_count = 0;

    if (!DDS_TypePlugin_create_wire_plugin(plugin,participant,dp_qos,
                                       endpoint_mode,endpoint,qos_deprecated))
    {
        return NULL;
    }

    return plugin;
}

void
DDS_TypePlugin_delete(struct DDS_TypePlugin *plugin)
{

    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp_next = NULL;
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeMemoryPlugin *mp_next = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)REDA_CircularList_get_first(&plugin->memory_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        mp_next = (struct DDS_TypeMemoryPlugin*)REDA_CircularListNode_get_next(&mp->_node);

        mp->_intf->delete_plugin(plugin,mp);

        mp = mp_next;
    }

    wp = (struct DDS_TypeEncapsulationPlugin*)REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        wp_next = (struct DDS_TypeEncapsulationPlugin*)REDA_CircularListNode_get_next(&wp->_node);

        wp->_intf->delete_plugin(plugin,wp);

        wp = wp_next;
    }

#if DDS_XTYPES_IS_ENABLED
    if ((plugin->property.program_context != NULL) &&
        (plugin->_intf->type_factory != NULL))
    {
        DDS_TypeInterfaceI_delete_execution_context(
                plugin->_intf->type_factory,
                plugin->property.program_context);

        plugin->property.program_context = NULL;
    }
#endif

    plugin->_intf->delete_plugin(plugin);
}

DDS_Boolean
DDS_TypePlugin_set_stream(struct DDS_TypePlugin *tp,
                          struct CDR_Stream_t *stream,
                          struct DDS_TypePluginBuffer *tbuf)
{
#ifdef RTI_ENDIAN_LITTLE
    stream->endian = RTI_CDR_ENDIAN_LITTLE;
#else
    stream->endian = RTI_CDR_ENDIAN_BIG;
#endif
    stream->need_byte_swap = RTI_FALSE;

    if (tp->big_endian)
    {
        CDR_Stream_set_endianess(stream,RTI_TRUE);
    }
    else
    {
        CDR_Stream_set_endianess(stream,RTI_FALSE);
    }

    stream->start_ptr = (void*)tbuf;

    if (!CDR_Stream_set_buffer(stream,
                               tbuf->data_pbuf.buffer,
                               tbuf->data_pbuf.max_length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_TypePlugin_set_stream_encapsulation(struct DDS_TypePlugin *tp,
                                        struct CDR_Stream_t *stream,
                                        DDS_EncapsulationId_t id)
{
    if (!DDS_TypePlugin_set_encapsulation(tp,id))
    {
        return DDS_BOOLEAN_FALSE;
    }

#ifdef RTI_ENDIAN_LITTLE
    stream->endian = RTI_CDR_ENDIAN_LITTLE;
#else
    stream->endian = RTI_CDR_ENDIAN_BIG;
#endif
    stream->need_byte_swap = RTI_FALSE;

    if (tp->big_endian)
    {
        CDR_Stream_set_endianess(stream,RTI_TRUE);
    }
    else
    {
        CDR_Stream_set_endianess(stream,RTI_FALSE);
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_EncapsulationId_t
DDS_TypePlugin_get_encapsulation(struct DDS_TypePlugin *plugin)
{
    return plugin->current_encapsulation;
}

DDS_Boolean
DDS_TypePlugin_set_encapsulation(struct DDS_TypePlugin *plugin,
                                 DDS_EncapsulationId_t id)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    DDS_Boolean found = RTI_FALSE;

    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        if (wp->_intf->encapsulation_kind->le_identifier == id)
        {
            plugin->big_endian = RTI_FALSE;
            found = RTI_TRUE;
            break;
        }
        else  if (wp->_intf->encapsulation_kind->be_identifier == id)
        {
            plugin->big_endian = RTI_TRUE;
            found = RTI_TRUE;
            break;
        }
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    if (found)
    {
        plugin->wire_plugin = wp;
        plugin->wp_intf = wp->_intf;
        plugin->memory_plugin = wp->memory_plugin;
        plugin->mp_intf = wp->memory_plugin->_intf;
        plugin->current_encapsulation = id;

        /* Zero Copy (Plain/Flat) encapsulation values are the same for BE and LE architectures
         * thus, when that is the case, the plugin needs to be set to the machines
         * native endianness
         */
        if (wp->_intf->encapsulation_kind->le_identifier == wp->_intf->encapsulation_kind->be_identifier)
        {
#ifdef RTI_ENDIAN_LITTLE
            plugin->big_endian = RTI_FALSE;
#else
            plugin->big_endian = RTI_TRUE;
#endif
        }
    }

    return found;
}

DDS_EncapsulationId_t
DDS_TypePlugin_match_representation(struct DDS_TypePlugin *plugin,
                                    DDS_DataRepresentationId_t id)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;

    /* Only checks the primary encapsulation. The assumption is that
     * id is enabled.
     */
    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        if (wp->_intf->representation_id == id)
        {
#if RTI_ENDIAN_LITTLE
            return wp->_intf->encapsulation_kind[0].le_identifier;
#else
            return wp->_intf->encapsulation_kind[0].be_identifier;
#endif
        }
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    return DDS_ENCAPSULATION_ID_INVALID;
}

DDS_Boolean
DDS_TypePlugin_set_allocator(struct DDS_TypePlugin *plugin,
                             NDDSMemoryManager id)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    DDS_Boolean found = RTI_FALSE;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&plugin->memory_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        if (mp->_intf->id == id)
        {
            found = RTI_TRUE;
            break;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    if (found)
    {
        plugin->allocator_plugin = mp;
    }

    return found;
}

DDS_Boolean
DDS_TypePlugin_delete_sample(struct DDS_TypePlugin *tp,void *sample)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeMemoryPlugin *heap = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&tp->memory_plugins);

    while (!REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_owner(mp) &&
            DDS_TypeMemoryPlugin_is_owner(mp,sample))
        {
            break;
        }
        else if (mp == tp->allocator_plugin)
        {
            heap = mp;
            break;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    if (REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (heap == NULL)
        {
            /* Cannot free samples with no owner */
            return DDS_BOOLEAN_TRUE;
        }
        else
        {
            mp = heap;
        }
    }

    tp->memory_plugin = mp;
    tp->mp_intf = mp->_intf;

    return (mp->_intf->delete_sample(tp,sample) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);
}

DDS_Boolean
DDS_TypePlugin_serialize_sample(struct DDS_TypePlugin *tp,
                                struct CDR_Stream_t *stream,
                                const void *sample,
                                DDS_InstanceHandle_t *destination)
{
    struct DDS_TypePluginBuffer *tbuf = NULL;

    tbuf = OSAPI_Compiler_reinterpret_cast(struct DDS_TypePluginBuffer*,
                                           stream->start_ptr);

    tbuf->data = sample;

    return ((struct DDS_TypeEncapsulationI*)((((struct DDS_TypePlugin*)tp)))->
            wp_intf)->serialize((struct DDS_TypePlugin*)tp,stream,
                                sample,destination) ?
                                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

void
DDS_TypePlugin_return_buffer(struct DDS_TypePlugin *tp,void *buffer)
{
    struct DDS_TypePluginBuffer *tbuf;

    tbuf = (struct DDS_TypePluginBuffer*)buffer;

    tp->wire_plugin = tbuf->wp;
    tp->wp_intf = tbuf->wp->_intf;
    tbuf->wp->_intf->return_buffer(tp,buffer);
}

DDS_Boolean
DDS_TypePlugin_add_peer(struct DDS_TypePlugin *plugin,
                        DDS_InstanceHandle_t *peer)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&plugin->memory_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_add_peer(mp))
        {
            mp->_intf->add_peer(mp,peer);
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        if (DDS_TypeEncapsulationPlugin_has_add_peer(wp))
        {
            wp->_intf->add_peer(wp,peer);
        }
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_TypePlugin_remove_peer(struct DDS_TypePlugin *plugin,
                           DDS_InstanceHandle_t *peer)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&plugin->memory_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_remove_peer(mp))
        {
            mp->_intf->remove_peer(mp,peer);
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        if (DDS_TypeEncapsulationPlugin_has_remove_peer(wp))
        {
            wp->_intf->remove_peer(wp,peer);
        }
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_TypePlugin_is_sample_consistent(struct DDS_TypePlugin *tp,
                                    DDS_Boolean *is_data_consistent,
                                    const void *sample,
                                    const struct DDS_SampleInfo *sample_info)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&tp->memory_plugins);

    while (!REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_owner(mp) &&
            DDS_TypeMemoryPlugin_is_owner(mp,sample) &&
            DDS_TypeMemoryPlugin_has_is_sample_consistent(mp))
        {
            return mp->_intf->is_sample_consistent(tp,mp,is_data_consistent,sample,sample_info) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    /* Currently, there will never be a case where there are 2 memory plugins
     * that have the is_sample_consistent API. This is only used by the zero copy
     * shared memory manager. For Heap Manager, the default behavior this function
     * will enforce the default behavior --> to always return
     * RTI_TRUE and set is_data_consistent to DDS_BOOLEAN_TRUE
     * In summary, the default behavior is to assume it always consistent
     * if it is unmanaged/unknown
     */
    *is_data_consistent = DDS_BOOLEAN_TRUE;

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_TypePlugin_serialize_inline_qos(struct DDS_TypePlugin *tp,
                                    struct CDR_Stream_t *stream,
                                    const void *const sample,
                                    DDS_InstanceHandle_t *destination)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&tp->memory_plugins);


    while (!REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_owner(mp) &&
            DDS_TypeMemoryPlugin_is_owner(mp,sample) &&
            DDS_TypeMemoryPlugin_has_serialize_inline_qos(mp))
        {
            return mp->_intf->serialize_inline_qos(tp,mp,stream,sample,destination) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_TypePlugin_return_address(struct DDS_TypePlugin *tp,
                              void *address)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&tp->memory_plugins);


    while (!REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_owner(mp) &&
            DDS_TypeMemoryPlugin_is_owner(mp,address) &&
            DDS_TypeMemoryPlugin_has_return_address(mp))
        {
            return mp->_intf->return_address(mp,address) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_TypePlugin_deserialize_inline_qos(struct DDS_TypePlugin *tp,
                                      struct CDR_Stream_t *stream,
                                      const void *const sample,
                                      DDS_InstanceHandle_t *source)
{
    struct DDS_TypeMemoryPlugin *mp = NULL;

    mp = (struct DDS_TypeMemoryPlugin*)
                            REDA_CircularList_get_first(&tp->memory_plugins);

    while (!REDA_CircularList_node_at_head(&tp->memory_plugins,&mp->_node))
    {
        if (DDS_TypeMemoryPlugin_has_owner(mp) &&
            DDS_TypeMemoryPlugin_is_owner(mp,sample) &&
            DDS_TypeMemoryPlugin_has_deserialize_inline_qos(mp))
        {
            return mp->_intf->deserialize_inline_qos(tp,mp,stream,sample,source) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
        }

        mp = (struct DDS_TypeMemoryPlugin*)
                                REDA_CircularListNode_get_next(&mp->_node);
    }

    return RTI_FALSE;

}

RTI_UINT32
DDS_TypePlugin_get_serialized_sample_size_max(struct DDS_TypePlugin *plugin)
{
    RTI_UINT32 retval = 0;
    RTI_UINT32 size = 0;
    struct DDS_TypeEncapsulationPlugin *wp = NULL;

    wp = (struct DDS_TypeEncapsulationPlugin*)
                            REDA_CircularList_get_first(&plugin->wire_plugins);
    while (!REDA_CircularList_node_at_head(&plugin->wire_plugins,&wp->_node))
    {
        size = wp->_intf->get_serialized_sample_size(plugin,wp,0);

        /* The get_serialized_sample_size does not include the encapsulation
         * header. Note that the interpreter _can_ include it if requested to,
         * but it is not by default.
         */
        size += RTI_CDR_ENCAPSULATION_HEADER_SIZE;
        if ((RTI_UINT32)size > retval)
        {
            retval = size;
        }
        wp = (struct DDS_TypeEncapsulationPlugin*)
                                REDA_CircularListNode_get_next(&wp->_node);
    }

    return retval;
}

/*ci
 * \brief Return the CDR encapsulation for the type-plugin
 */
DDS_EncapsulationId_t
DDS_TypePlugin_get_cdr_encapsulation(struct DDS_TypePlugin *plugin)
{
    return plugin->cdr_id;
}

/*ci
 * \brief Returns the cdr encapsulation based on the data representation
 */
DDS_EncapsulationId_t
DDS_TypePlugin_resolve_encapsulation(DDS_EncapsulationId_t cdr_id,
                                     const struct DDS_DataRepresentationQosPolicy *dr)
{
    DDS_Long index,length;
    DDS_EncapsulationId_t retval = DDS_ENCAPSULATION_ID_INVALID;
    DDS_DataRepresentationId_t a_id;

    length = DDS_DataRepresentationIdSeq_get_length(&dr->value);

    /* If the data representation is empty the default is v1 */
    if (length == 0)
    {
        if (cdr_id <= DDS_ENCAPSULATION_ID_PL_CDR)
        {
            return cdr_id;
        }
        else
        {
            /* Pick a V1 encapsulation, it does not matter which one */
            return DDS_ENCAPSULATION_ID_CDR_BE;
        }
    }

    /* This loop exits if an exact match can be found between the
     * input cdr_id and the data representation. If an exact match is
     * not found use the highest supported.
     */
    for (index = 0; index < length; index++)
    {
        a_id = *DDS_DataRepresentationIdSeq_get_reference(&dr->value,index);

        if (a_id == DDS_XCDR_DATA_REPRESENTATION)
        {
            if (cdr_id <= DDS_ENCAPSULATION_ID_PL_CDR)
            {
                retval = cdr_id;
                break;
            }
            else
            {
                /* Pick a V1 encapsulation, it does not matter which one */
                retval = DDS_ENCAPSULATION_ID_CDR_BE;
            }
        }
        else if (a_id == DDS_XCDR2_DATA_REPRESENTATION)
        {
            if ((cdr_id >= DDS_ENCAPSULATION_ID_XCDR2_F_BE) &&
                (cdr_id <= DDS_ENCAPSULATION_ID_XCDR2_M_LE))
            {
                retval = cdr_id;
                break;
            }
            else
            {
                /* Pick a V2 encapsulation, it does not matter which one */
                retval = DDS_ENCAPSULATION_ID_XCDR2_F_BE;
            }
        }
    }

    return retval;
}

/*ci
 * \brief Return true if the specified data representation has been enabled.
 */
DDS_Boolean
DDS_TypePlugin_is_representation_enabled(struct DDS_TypePlugin *plugin,
                                         DDS_DataRepresentationId_t dr_id)
{
    DDS_Long len;

    for (len = 0; len < plugin->representation_count; ++len)
    {
        if (plugin->representation[len] == dr_id)
        {
            return DDS_BOOLEAN_TRUE;
        }
    }

    return DDS_BOOLEAN_FALSE;
}

DDS_Boolean
DDS_TypePlugin_is_v1_and_v2_enabled(struct DDS_TypePlugin *plugin)
{
    return DDS_TypePlugin_is_representation_enabled(plugin,DDS_XCDR_DATA_REPRESENTATION) &&
           DDS_TypePlugin_is_representation_enabled(plugin,DDS_XCDR2_DATA_REPRESENTATION);
}

#if DDS_XTYPES_IS_ENABLED
void
DDS_TypePlugin_initialize_static(struct DDS_TypePlugin *plugin,
                                 const struct DDS_TypePluginI *intf,
                                 const struct RTIXCdrInterpreterPrograms *programs)
{
    plugin->property.programs = (struct RTIXCdrInterpreterPrograms*)programs;
    plugin->_intf = intf;
#if RTI_ENDIAN_LITTLE
    plugin->big_endian = RTI_FALSE;
#else
    plugin->big_endian = RTI_TRUE;
#endif
}
#endif

DDSCDllExport void
DDS_TypePluginBuffer_initialize_static(struct DDS_TypePluginBuffer *tbuf,
                                       char *buffer,
                                       unsigned int length)
{
    tbuf->data_pbuf.buffer = (char*)buffer;
    tbuf->data_pbuf.max_length = length;
}

DDS_ReturnCode_t
DDS_TypeSupport_resolve_representation(
    DDS_DataRepresentationId_t *representation,
    DDS_DataRepresentationId_t auto_representation,
    DDS_DataRepresentationId_t xcdr1_representation,
    DDS_DataRepresentationId_t xcdr2_representation,
    DDS_EncapsulationId_t *encapsulation)
{
    /* The encapsulation is only used to determine endianess and V1 vs V2,
    * thus any V1 or V2 is fine.
    */
    if (*representation == DDS_AUTO_DATA_REPRESENTATION)
    {
        *representation = auto_representation;
    }

    if ((*representation == DDS_XCDR_DATA_REPRESENTATION) &&
        (xcdr1_representation != DDS_INVALID_DATA_REPRESENTATION))
    {
        *representation = DDS_XCDR_DATA_REPRESENTATION;
        *encapsulation = RTI_ENCAPSULATION_ID_CDR_NATIVE;
    }
    else if ((*representation == DDS_XCDR2_DATA_REPRESENTATION) &&
             (xcdr2_representation != DDS_INVALID_DATA_REPRESENTATION))
    {
        *representation = DDS_XCDR2_DATA_REPRESENTATION;
        *encapsulation = DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE;
    }
    else
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

/*ci @} */

