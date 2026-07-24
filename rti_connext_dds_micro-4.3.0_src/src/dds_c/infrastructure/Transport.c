/*
 * FILE: Transport.c - DDS Transport functions
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief DDS Transport functions
 */
/*ci
 * \addtogroup DDSInfrastructureModule
 * @{
 */
#include "Transport.h"

struct DDS_EncapsulationEntry
{
    DDS_EncapsulationId_t encapsulation_le;
    DDS_EncapsulationId_t encapsulation_be;
};

#define DDS_ENCAPSULATION_SENTINEL \
{\
    DDS_ENCAPSULATION_ID_INVALID,\
    DDS_ENCAPSULATION_ID_INVALID\
}

#define DDS_TRANSPORT_HIGHEST_PRIORITY (0x7fffffff)

#define DDS_STD_INBAND_ENCAPSULATIONS \
{\
    DDS_ENCAPSULATION_ID_XCDR2_F_LE,\
    DDS_ENCAPSULATION_ID_XCDR2_F_BE\
},\
{\
    DDS_ENCAPSULATION_ID_XCDR2_A_LE,\
    DDS_ENCAPSULATION_ID_XCDR2_A_BE\
},\
{\
    DDS_ENCAPSULATION_ID_XCDR2_M_LE,\
    DDS_ENCAPSULATION_ID_XCDR2_M_BE\
},\
{\
    DDS_ENCAPSULATION_ID_XCDR2_XML,\
    DDS_ENCAPSULATION_ID_XCDR2_XML\
},\
{\
    DDS_ENCAPSULATION_ID_CDR_LE,\
    DDS_ENCAPSULATION_ID_CDR_BE\
},\
{\
    DDS_ENCAPSULATION_ID_PL_CDR_LE,\
    DDS_ENCAPSULATION_ID_PL_CDR_BE\
}

RTI_PRIVATE const struct DDS_EncapsulationEntry DDS_ShmemEncapsulations[]=
{
    {
        DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_LE,
        DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_BE
    },
    {
        DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_LE,
        DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_BE
    },
    DDS_STD_INBAND_ENCAPSULATIONS,
    DDS_ENCAPSULATION_SENTINEL
};

RTI_PRIVATE const struct DDS_EncapsulationEntry DDS_Udpv4Encapsulations[]=
{
    DDS_ENCAPSULATION_SENTINEL
};


RTI_PRIVATE const struct DDS_EncapsulationEntry DDS_NotifEncapsulations[]=
{   
    DDS_ENCAPSULATION_SENTINEL
};

struct DDS_TransportEncapsulationEntry
{
    RTI_INT32 locator_kind;
    RTI_INT32  priority;
    const struct DDS_EncapsulationEntry *encapsulations;
};

RTI_PRIVATE const struct DDS_TransportEncapsulationEntry DDS_TransportEncapsulations[]=
{
    {
        NETIO_ADDRESS_KIND_SHMEM,
        1,
        &DDS_ShmemEncapsulations[0]
    },
    {
        NETIO_ADDRESS_KIND_NOTIF,
        2,
        &DDS_NotifEncapsulations[0]
    },
    {
        NETIO_ADDRESS_KIND_UDPv4,
        NETIO_DEFAULT_PRIORITY,
        &DDS_Udpv4Encapsulations[0]
    },
    {
    	/* Unknown locators defaults to the standard 
    	 * inband encapsulations supported by UDPv4.
    	 */
        NETIO_ADDRESS_KIND_RESERVED,
        NETIO_DEFAULT_PRIORITY,
        &DDS_Udpv4Encapsulations[0]
    },
    {
        NETIO_ADDRESS_KIND_INVALID,
        0,
        NULL
    }
};

RTI_INT32
DDS_Transport_get_locator_priority(const struct DDS_Locator *loc)
{
    const struct DDS_TransportEncapsulationEntry *entry;

    entry = &DDS_TransportEncapsulations[0];
    while (entry->locator_kind != NETIO_ADDRESS_KIND_RESERVED)
    {
        if (entry->locator_kind ==
            NETIO_Address_get_kind((const struct NETIO_Address *const)loc))
        {
        break;
        }
        entry++;
    }

    return entry->priority;
}

RTI_PRIVATE RTI_BOOL
DDS_Transport_add_encapsulation(
                        struct DDS_TypePlugin *plugin,
                        struct DDS_TransportEncapsulationQosPolicy *policy,
                        DDS_EncapsulationId_t eid,
                        RT_ComponentFactoryId_T *intf_name)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    struct DDS_TypeEncapsulationPlugin *wpdef = NULL;
    RTI_INT32 ts_index;
    RTI_INT32 ss_length;
    struct DDS_TransportEncapsulationSettings_t *ts;
    DDS_EncapsulationId_t *eid_entry;

    ts_index = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);

    if (!DDS_TransportEncapsulationSettingsSeq_set_maximum(
                                                &policy->value,ts_index+1))
    {
        return RTI_FALSE;
    }

    if (!DDS_TransportEncapsulationSettingsSeq_set_length(
                                                &policy->value,ts_index+1))
    {
        return RTI_FALSE;
    }

    ts = DDS_TransportEncapsulationSettingsSeq_get_reference(
                                                &policy->value,ts_index);
    if (ts == NULL)
    {
        return RTI_FALSE;
    }

    /* By convention, the "default" wire plugin is the first
     * plugin which is supported across all transports. Add
     * the default plugin first if different than the one
     * supporting the encapsulation.
     */
    wpdef = (struct DDS_TypeEncapsulationPlugin*)
                         REDA_CircularList_get_first(&plugin->wire_plugins);
    if (wpdef != wp)
    {
        if (!DDS_EncapsulationIdSeq_set_maximum(&ts->encapsulations,2))
        {
            return RTI_FALSE;
        }
        if (!DDS_EncapsulationIdSeq_set_length(&ts->encapsulations,2))
        {
            return RTI_FALSE;
        }
        eid_entry = DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,1);
        if (eid_entry == NULL)
        {
            return RTI_FALSE;
        }
#ifdef RTI_ENDIAN_LITTLE
        *eid_entry = wpdef->_intf->encapsulation_kind->le_identifier;
#else
        *eid_entry = wpdef->_intf->encapsulation_kind->be_identifier;
#endif
    }
    else
    {
        if (!DDS_EncapsulationIdSeq_set_maximum(&ts->encapsulations,1))
        {
            return RTI_FALSE;
        }
        if (!DDS_EncapsulationIdSeq_set_length(&ts->encapsulations,1))
        {
            return RTI_FALSE;
        }
    }

    eid_entry = DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,0);
    if (eid_entry == NULL)
    {
        return RTI_FALSE;
    }

    *eid_entry = eid;

    ss_length = DDS_StringSeq_get_length(&ts->transports);
    if (!DDS_StringSeq_set_maximum(&ts->transports,ss_length+1))
    {
        return RTI_FALSE;
    }

    if (!DDS_StringSeq_set_length(&ts->transports,ss_length+1))
    {
        return RTI_FALSE;
    }

    *DDS_StringSeq_get_reference(&ts->transports,ss_length) =
            DDS_String_dup(RT_ComponentFactoryId_get_name(intf_name));

    return RTI_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_Transport_assert_encapsulation(struct DDS_TransportEncapsulationSettings_t *ts,
                                   DDS_EncapsulationId_t eid)
{
    RTI_INT32 e_index, e_length;
    DDS_EncapsulationId_t e_value;
    DDS_EncapsulationId_t *enc_id1;
    DDS_EncapsulationId_t *enc_id2;

    e_length = DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);

    /* At this point the RCC always has a default eid. Coverity throws a
     * warning without this check. See MICRO-7336.
     */
    if (e_length < 1)
    {
        return DDS_BOOLEAN_FALSE;
    }

    for (e_index = 0; e_index < e_length; e_index++)
    {
        DDS_EncapsulationId_t *enc_ref =
            DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations, e_index);
        if (enc_ref == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        e_value = *enc_ref;
        if (e_value == eid)
        {
            /* Found the encapsulation, don't do anything */
            break;
        }
    }

    if (e_index < e_length)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* Didn't find the eid, add before the default */
    if (!DDS_EncapsulationIdSeq_set_maximum(&ts->encapsulations,e_length+1))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_EncapsulationIdSeq_set_length(&ts->encapsulations,e_length+1))
    {
        return DDS_BOOLEAN_FALSE;
    }

    enc_id1 = DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,e_length);
    enc_id2 = DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,
                                                  e_length-1);
    if ((enc_id1 == NULL) || (enc_id2 == NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    *enc_id1 = *enc_id2;
    *enc_id2 = eid;

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_Transport_add_transport_encapsulations(
                        struct DDS_TypePlugin *plugin,
                        struct DDS_TransportEncapsulationQosPolicy *policy,
                        const struct DDS_TransportEncapsulationEntry *entry,
                        RT_ComponentFactoryId_T *intf_name)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    DDS_EncapsulationId_t eid;
    const struct DDS_EncapsulationEntry *enc_entry;
    struct DDS_TransportEncapsulationSettings_t *ts;

    eid = DDS_ENCAPSULATION_ID_INVALID;

    enc_entry = entry->encapsulations;
    while (enc_entry->encapsulation_be != DDS_ENCAPSULATION_ID_INVALID)
    {
        wp = DDS_TypePlugin_find_encapsulation_plugin(plugin,
                                          enc_entry->encapsulation_be,
                                          DDS_INVALID_DATA_REPRESENTATION);
        if (wp == NULL)
        {
            /* The encapsulation is not supported by the endpoint,
             * ignore.
             */
            ++enc_entry;
            continue;
        }

        /* The endpoint supports the transport encapsulation add the
         * preferences.
         */
#ifdef RTI_ENDIAN_LITTLE
        eid = enc_entry->encapsulation_le;
#else
        eid = enc_entry->encapsulation_be;
#endif
        /* Found an encapsulation that is preferred on a transport
         * Check if any of the supplied transports supports the
         * locator kind
         */
        ts = DDS_TransportEncapsulationQosPolicy_find_transport_setting(
                                                    policy,intf_name);

        if (ts == NULL)
        {
            /* New transport setting and encapsulation */
            if (!DDS_Transport_add_encapsulation(plugin,policy,eid,
                                                 intf_name))
            {

            }
        }
        else if (!DDS_Transport_assert_encapsulation(ts,eid))
        {

        }

        ++enc_entry;
    }

    return DDS_BOOLEAN_TRUE;
}

void
DDS_Transport_set_encapsulation_policy(
                            struct DDS_TransportEncapsulationQosPolicy *policy,
                            struct DDS_TypePlugin *plugin,
                            struct DDS_Locator *a_loc,
                            NETIO_AddressResolver_T *ar,
                            NETIO_RouteResolver_T *rr)
{
    RT_ComponentFactoryId_T intf_name;
    const struct DDS_TransportEncapsulationEntry *entry;
    RTI_INT32 a_loc_kind;

    if (!DDS_Locator_get_interface(a_loc,&intf_name,rr,ar))
    {
        return;
    }

    a_loc_kind = NETIO_Address_get_kind(
                        DDS_Locator_const_cast(struct NETIO_Address,a_loc));

    entry = DDS_TransportEncapsulations;
    while (entry->locator_kind != NETIO_ADDRESS_KIND_INVALID)
    {
    	/* The reserved locator is at the end, if it is reached then
    	 * the locator kind is unknown, add the default encapsulations
    	 */
        if ((a_loc_kind == entry->locator_kind) ||
            (NETIO_ADDRESS_KIND_RESERVED == entry->locator_kind))
        {
            if (!DDS_Transport_add_transport_encapsulations(plugin,
                                                            policy,
                                                            entry,
                                                            &intf_name))
            {

            }
        }
        ++entry;
    }
}

RTI_PRIVATE DDS_Boolean
DDS_Transport_is_name_duplicate(struct DDS_TransportEncapsulationQosPolicy *policy,
                                DDS_String ts_name)
{
    struct DDS_TransportEncapsulationSettings_t *ts;
    RTI_INT32 ts_length;
    RTI_INT32 ts_index;
    RTI_INT32 ss_length;
    RTI_INT32 ss_index;
    DDS_String ss_name;

    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);
    for (ts_index = 0; ts_index < ts_length; ts_index++)
    {
        ts = DDS_TransportEncapsulationSettingsSeq_get_reference(
                                                    &policy->value,ts_index);
        if (ts == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        ss_length = DDS_StringSeq_get_length(&ts->transports);
        for (ss_index = 0; ss_index < ss_length; ss_index++)
        {
            ss_name = *DDS_StringSeq_get_reference(&ts->transports,ss_index);
            if ((ss_name == NULL) || (ts_name == ss_name))
            {
                continue;
            }
            if (!DDS_String_cmp(ss_name,ts_name))
            {
                return DDS_BOOLEAN_TRUE;
            }
        }
    }

    return DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE DDS_Boolean
DDS_Transport_has_duplicate_names(struct DDS_TransportEncapsulationQosPolicy *policy)
{
    RTI_INT32 ts_length;
    RTI_INT32 ts_index;
    RTI_INT32 ss_length;
    RTI_INT32 ss_index;
    DDS_String ss_name;
    struct DDS_TransportEncapsulationSettings_t *ts;

    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);
    for (ts_index = 0; ts_index < ts_length; ts_index++)
    {
        ts = DDS_TransportEncapsulationSettingsSeq_get_reference(
                                                    &policy->value,ts_index);
        if (ts == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        ss_length = DDS_StringSeq_get_length(&ts->transports);
        for (ss_index = 0; ss_index < ss_length; ss_index++)
        {
            ss_name = *DDS_StringSeq_get_reference(&ts->transports,ss_index);
            if ((ss_name == NULL) ||
                DDS_Transport_is_name_duplicate(policy,ss_name))
            {
                return DDS_BOOLEAN_TRUE;
            }
        }
    }

    return DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE DDS_Boolean
DDS_Transport_is_encapsulation_supported(struct DDS_TransportEncapsulationSettings_t *ts,
                                         struct DDS_TypePlugin *plugin,
                                         const struct DDS_EncapsulationEntry *entry)
{
    struct DDS_TypeEncapsulationPlugin *wp = NULL;
    RTI_INT32 e_index, e_length;
    DDS_EncapsulationId_t e_value;

    /* The interface supports the locator kind, check if the
      * encapsulations are supported.
      */
     e_length = DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);
     for (e_index = 0; e_index < e_length; e_index++)
     {
        DDS_EncapsulationId_t *e_value_ref =
            DDS_EncapsulationIdSeq_get_reference(&ts->encapsulations,e_index);
        if (e_value_ref == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        e_value = *e_value_ref;

        while (entry->encapsulation_be != DDS_ENCAPSULATION_ID_INVALID)
        {
            if ((e_value == entry->encapsulation_be) ||
                (e_value == entry->encapsulation_le))
            {
                wp = DDS_TypePlugin_find_encapsulation_plugin(plugin,
                                            entry->encapsulation_be,
                                            DDS_INVALID_DATA_REPRESENTATION);
                if (wp == NULL)
                {
                    /* The transport kind supports it, but the
                    * type-plugin doesn't so it is invalid.
                    */
                    return DDS_BOOLEAN_FALSE;
                }
            }
            ++entry;
        }

        if (entry->encapsulation_be == DDS_ENCAPSULATION_ID_INVALID)
        {
            /* The encapsulation is not supported for the transport kind */
            return DDS_BOOLEAN_FALSE;
        }
     }

     return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_Transport_is_encapsulation_policy_valid(
                            struct DDS_TransportEncapsulationQosPolicy *policy,
                            struct DDS_TypePlugin *plugin,
                            NETIO_AddressResolver_T *ar,
                            NETIO_RouteResolver_T *rr)
{
    struct DDS_TransportEncapsulationSettings_t *ts;
    RTI_INT32 ts_length,ts_index;
    RTI_INT32 ss_length,ss_index;
    DDS_String ss_name;
    RTI_INT32 loc_index;
    NETIO_Interface_T *netio_intf;
    const struct DDS_EncapsulationEntry *entry;

    if (DDS_Transport_has_duplicate_names(policy))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* No duplicate transport names found, check that all encapsulations are
     * valid and supported
     */
    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);
    for (ts_index = 0; ts_index < ts_length; ts_index++)
    {
        ts = DDS_TransportEncapsulationSettingsSeq_get_reference(
                                                    &policy->value,ts_index);
        if (ts == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        ss_length = DDS_StringSeq_get_length(&ts->transports);
        for (ss_index = 0; ss_index < ss_length; ss_index++)
        {
            ss_name = *DDS_StringSeq_get_reference(&ts->transports,ss_index);
            if (!NETIO_AddressResolver_lookup_interface(ar,ss_name,&netio_intf))
            {
                /* The transport name is not supported */
                return DDS_BOOLEAN_FALSE;
            }

            /* Check if the locator type is supported by this interface and
             * if so check if the encapsulation is supported.
             */
            for (loc_index = 0; DDS_TransportEncapsulations[loc_index].locator_kind != NETIO_ADDRESS_KIND_INVALID; ++loc_index)
            {
                /* Find the locator type supported by this interface, unless 
                 * the reserved entry is found in which case the default 
                 * encapsulations are assumed.
                 */
                if ((DDS_TransportEncapsulations[loc_index].locator_kind != NETIO_ADDRESS_KIND_RESERVED) &&
                    !NETIO_RouteResolver_interface_supports_kind(rr,netio_intf,
                        DDS_TransportEncapsulations[loc_index].locator_kind))
                {
                    continue;
                }

                entry = DDS_TransportEncapsulations[loc_index].encapsulations;
                if (!DDS_Transport_is_encapsulation_supported(ts,plugin,entry))
                {
                    return DDS_BOOLEAN_FALSE;
                }
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci @} */

