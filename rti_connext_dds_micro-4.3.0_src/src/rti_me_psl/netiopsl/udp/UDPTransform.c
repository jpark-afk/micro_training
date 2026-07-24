/*
 * FILE: UDPTransform.c - UDP Transform Interface
 *
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.
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
 * 11aug2017,tk  Written
 */
/*ci
 * \file
 * \brief UDP Tranform Implementation
 *
 * \details
 * This file implements the UDP Transform interface.
 *
 * \addtogroup NETIO_UDPTransformClass
 * @{
 */
#include "UDPTransform.h"

/*** SOURCE_BEGIN ***/

#if UDP_TRANSFORMS_ENABLED

/*ci
 * \brief Compare two factory entries
 *
 * @param[in] record    The record to compare against key
 * @param[in] is_record The key is a complete record
 * @param[in] key       The key/record to compare with
 *
 * \return return 0 if the record and key is equal
 *         return positive value if the record is larger then the key
 *         return negative value if the record is smaller then the key
 */
RTI_PRIVATE RTI_INT32
UDP_TransformTable_compare_factory(const void *const record,
                                   RTI_BOOL key_is_record,
                                   const void *const key)
{
    const RT_ComponentFactoryId_T *const lval = (RT_ComponentFactoryId_T*)record;
    const RT_ComponentFactoryId_T *const rval = (RT_ComponentFactoryId_T*)key;
    UNUSED_ARG(key_is_record);

    return RT_ComponentFactoryId_compare(lval,rval);
}

/*ci
 * \brief Compare two address entries for rules
 *
 * @param[in] record    The record to compare against key
 * @param[in] is_record The key is a complete record
 * @param[in] key       The key/record to compare with
 *
 * \return return 0 if the record and key is equal
 *         return positive value if the record is larger then the key
 *         return negative value if the record is smaller then the key
 */
RTI_PRIVATE RTI_INT32
UDP_TransformTable_compare_address(const void *const record,
                                   RTI_BOOL key_is_record,
                                   const void *const key)
{
    const struct NETIO_Address *const lval = (struct NETIO_Address*)record;
    const struct NETIO_Address *const rval = (struct NETIO_Address*)key;
    RTI_INT32 diff = 0;
    UNUSED_ARG(key_is_record);

    /* NOTE:  Sorted by address in reverse order */
    if (rval->value.ipv4.address > lval->value.ipv4.address)
    {
        diff = 1;
    }
    else if (rval->value.ipv4.address < lval->value.ipv4.address)
    {
        diff = -1;
    }

    return diff;
}

/*ci
 * \brief Compare two IPv4 netmasks entries for rules
 *
 * @param[in] record    The record to compare against key
 * @param[in] is_record The key is a complete record
 * @param[in] key       The key/record to compare with
 *
 * \return return 0 if the record and key is equal
 *         return positive value if the record is larger then the key
 *         return negative value if the record is smaller then the key
 */
RTI_PRIVATE RTI_INT32
UDP_TransformTable_compare_ipv4mask(const void *const record,
                                    RTI_BOOL key_is_record,
                                    const void *const key)
{
    const struct NETIO_Netmask *lval = &((struct UDP_TransformEntry*)record)->netmask;
    const struct NETIO_Netmask *rval;
    RTI_INT32 diff = 0;

    if (key_is_record)
    {
        rval = &((struct UDP_TransformEntry*)key)->netmask;
    }
    else
    {
        rval = (struct NETIO_Netmask*)key;
    }


    /* NOTE:  Sorted by address in reverse order */
    if (rval->mask[0] > lval->mask[0])
    {
        diff = 1;
    }
    if (rval->mask[0] < lval->mask[0])
    {
        diff = -1;
    }

    return diff;
}

/*ci
 * \brief Add unique factory names to a sequence.
 *
 * @param[in]  rules     A rule sequence to extract names from
 * @param[out] factories A sequence of unique factory names
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
UDP_TransformTable_add_factory_name(struct UDP_TransformRuleSeq *rules,
                                    struct REDA_StringSeq *factories)
{
    RTI_INT32 fac_cnt = 0;
    RTI_INT32 length = 0;
    RTI_INT32 length2 = 0;
    RTI_INT32 index = 0;
    RTI_INT32 index2 = 0;
    struct UDP_TransformRule *entry = NULL;
    char *fac_name;

    fac_cnt = REDA_StringSeq_get_length(factories);
    length = UDP_TransformRuleSeq_get_length(rules);

    for (index = 0; index < length; ++index)
    {
        entry = UDP_TransformRuleSeq_get_reference(rules,index);

        length2 = REDA_StringSeq_get_length(factories);
        for (index2 = 0; index2 < length2; ++index2)
        {
            fac_name = *REDA_StringSeq_get_reference(factories,index2);
            if (RT_ComponentFactoryId_equals(&entry->transformation,fac_name))
            {
                break;
            }
        }

        /* Must have found a new index since the search is exhausted */
        if (index2 == length2)
        {
            fac_cnt++;

            if (!REDA_StringSeq_set_maximum(factories,fac_cnt))
            {
                /* set_maximum logs error */
                return RTI_FALSE;
            }

            if (!REDA_StringSeq_set_length(factories,fac_cnt))
            {
                /* set_length logs error */
                return RTI_FALSE;
            }

            *REDA_StringSeq_get_reference(factories,fac_cnt-1) =
                    REDA_String_dup(RT_ComponentFactoryId_get_name(
                                                    &entry->transformation));

            if (*REDA_StringSeq_get_reference(factories,fac_cnt-1) == NULL)
            {
                UDP_TRANSFORM_LOG_INVALID_FACTORY_REFERENCE(OSAPI_LOGKIND_ERROR);
                return RTI_FALSE;
            }

            OSAPI_TRACE_NET("Added transformation factory",RTI_FALSE);
            OSAPI_TRACE_STRING("name",
                               *REDA_StringSeq_get_reference(factories,fac_cnt-1),
                               RTI_TRUE);
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Add rules
 *
 * \details
 * Rules are used to determine if a specific address A lies between
 * an address N after netmask M has been applied. That is if A is between
 * [N & M,N | M ].
 *
 * The rules are sorted by the base address. To check if an address
 * is with in a group first find the location the address would have been
 * added to if it was to be inserted. Then look at the previous entry
 * and apply the mask. If the address matches the rule address it is within
 * the group, otherwise not.
 *
 * @param[out] rule_index  The index to add rules to
 * @param[in]  rules       The rules to add
 * @param[in]  factories   The factories to use to create transformations from
 * @param[in]  is_source   Whether this is a source or destination transform
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
UDP_TransformTable_add_rules(REDA_Indexer_T *rule_index,
                             REDA_Indexer_T *masks,
                             struct UDP_TransformRuleSeq *rules,
                             REDA_Indexer_T *factories,
                             RTI_BOOL is_source,
                             struct UDP_TransformProperty *property)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 length = 0;
    RTI_INT32 index = 0;
    struct UDP_TransformFactoryEntry *factory = NULL;
    struct UDP_TransformEntry *entry = NULL;
    struct UDP_TransformEntry *dup_entry = NULL;
    struct UDP_TransformRule *rule = NULL;
    RTI_INT32 ec;
    struct NETIO_Address key;

    length = UDP_TransformRuleSeq_get_length(rules);
    for (index = 0; index < length; ++index)
    {
        entry = NULL;
        rule = UDP_TransformRuleSeq_get_reference(rules,index);

        factory = REDA_Indexer_find_entry(factories,
                      RT_ComponentFactoryId_get_name(&rule->transformation));
        if (factory == NULL)
        {
            UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                    RT_ComponentFactoryId_get_name(&rule->transformation));
            goto done;
        }

        /* Check if a rule already exists */
        key = rule->address;
        key.value.ipv4.address = key.value.ipv4.address & rule->netmask.mask[0];

        dup_entry = REDA_Indexer_find_entry(rule_index,&key);
        if (dup_entry != NULL)
        {
            if (is_source)
            {
                UDP_TRANSFORM_LOG_DUPLICATE(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE);
            }
            else
            {
                UDP_TRANSFORM_LOG_DUPLICATE(OSAPI_LOGKIND_ERROR,
                                UDP_TRANSFORM_LOG_KIND_TRANSFORM_DESTINATION);
            }
            goto done;
        }

        OSAPI_Heap_allocate_struct(&entry,struct UDP_TransformEntry);
        if (entry == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_ENTRY);
            goto done;
        }

        entry->key = key;
        entry->address = rule->address;
        entry->netmask = rule->netmask;
        entry->transform = factory->transform;
        entry->context = NULL;
        entry->user_data = rule->user_data;

        if (is_source)
        {
            if (!UDP_Transform_create_source_transform(factory->transform,
                               &entry->context,&rule->address,&rule->netmask,
                               rule->user_data,property,&ec))
            {
                UDP_TRANSFORM_LOG_CREATE(OSAPI_LOGKIND_ERROR,
                                         UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE);
                goto done;
            }
        }
        else
        {
            if (!UDP_Transform_create_destination_transform(factory->transform,
                                &entry->context,&rule->address,&rule->netmask,
                                rule->user_data,property,&ec))
            {
                UDP_TRANSFORM_LOG_CREATE(OSAPI_LOGKIND_ERROR,
                             UDP_TRANSFORM_LOG_KIND_TRANSFORM_DESTINATION);
                goto done;
            }
        }

        if (!REDA_Indexer_add_entry(rule_index,entry))
        {
            UDP_TRANSFORM_LOG_ADD(OSAPI_LOGKIND_ERROR,
                                  UDP_TRANSFORM_LOG_KIND_ENTRY);
            goto done;
        }

        if (REDA_Indexer_find_entry(masks,&entry->netmask) == NULL)
        {
            if (!REDA_Indexer_add_entry(masks,entry))
            {
                UDP_TRANSFORM_LOG_CREATE(OSAPI_LOGKIND_ERROR,
                             UDP_TRANSFORM_LOG_KIND_TRANSFORM_NETMASK_INDEX);
                goto done;
            }
        }

        /* set variable entry to NULL as it has been successfully added 
         * to the indexer. This would avoid undesired free in case there is
         * an error after this point 
         */
        entry = NULL;
    }

    retval = RTI_TRUE;

done:

    if (retval != RTI_TRUE)
    {
        if (entry != NULL)
        {
            OSAPI_Heap_free_struct(entry);
            entry = NULL;
        }
    }

    return retval;
}

RTI_BOOL
UDP_TransformTable_initialize(struct UDP_TransformTable *rules,
                              RT_Registry_T *registry,
                              struct UDP_TransformRuleSeq *dst_rules,
                              struct UDP_TransformRuleSeq *src_rules,
                              struct UDP_TransformProperty *property)
{
    RTI_BOOL rval = RTI_FALSE;
    RTI_INT32 fac_cnt = 0;
    RTI_INT32 count = 0;
    struct REDA_IndexerProperty iprop = REDA_IndexerProperty_INITIALIZER;
    struct UDP_TransformFactoryEntry *rule = NULL;
    struct REDA_StringSeq fac_names = REDA_StringSeq_INITIALIZER;

    UNUSED_ARG(rules);
    UNUSED_ARG(dst_rules);
    UNUSED_ARG(src_rules);

    /* Calculate the number of source transformations needed, one instance of
     * a transformation for each unique name. Then create one instance of
     * each transformation. These are later used to create transformation
     * contexts, one per rule.
     */
    if (!UDP_TransformTable_add_factory_name(dst_rules,&fac_names))
    {
        /* Error logged by function */
        goto done;
    }

    if (!UDP_TransformTable_add_factory_name(src_rules,&fac_names))
    {
        /* Error logged by function */
        goto done;
    }

    rules->factories = NULL;
    rules->source_rules = NULL;
    rules->destination_rules = NULL;
    rules->masks = NULL;
    rules->property = *property;

    fac_cnt = REDA_StringSeq_get_length(&fac_names);

    if (fac_cnt == 0)
    {
        return RTI_TRUE;
    }

    /* Create the search index for factories */
    iprop.max_entries = fac_cnt;
    rules->factories = REDA_Indexer_new(UDP_TransformTable_compare_factory,&iprop);
    if (rules->factories == NULL)
    {
        UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                UDP_TRANSFORM_LOG_KIND_FACTORY_INDEX);
        goto done;
    }

    /* Create the search index for destination rules */
    iprop.max_entries = UDP_TransformRuleSeq_get_length(dst_rules);
    if (iprop.max_entries > 0)
    {
        rules->destination_rules = REDA_Indexer_new(
                UDP_TransformTable_compare_address,&iprop);
        if (rules->destination_rules == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_DST_INDEX);
            goto done;
        }
    }

    /* Create the search index for source rules */
    iprop.max_entries = UDP_TransformRuleSeq_get_length(src_rules);
    if (iprop.max_entries > 0)
    {
        rules->source_rules = REDA_Indexer_new(
                UDP_TransformTable_compare_address,&iprop);
        if (rules->source_rules == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_SRC_INDEX);
            goto done;
        }
    }

    iprop.max_entries = UDP_TransformRuleSeq_get_length(src_rules) +
                        UDP_TransformRuleSeq_get_length(dst_rules);

    if (iprop.max_entries > 0)
    {
        rules->masks = REDA_Indexer_new(
                        UDP_TransformTable_compare_ipv4mask,&iprop);
        if (rules->masks == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                            UDP_TRANSFORM_LOG_KIND_TRANSFORM_NETMASK_INDEX);
            goto done;
        }
    }

    /* Now instantiate each factory
     *
     * 1. Lookup factory
     * 2. Create instance
     * 3. Add instance to search index, ordered by name
     */
    for (count = 0; count < fac_cnt; ++count)
    {
        OSAPI_Heap_allocate_struct(&rule,struct UDP_TransformFactoryEntry);

        if (rule == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_FACTORY);
            goto done;
        }

        rule->factory = RT_Registry_lookup(
                            registry,
                            *REDA_StringSeq_get_reference(&fac_names,count));

        if (rule->factory == NULL)
        {
            UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                            *REDA_StringSeq_get_reference(&fac_names,count));
            goto done;
        }
        else
        {
            OSAPI_TRACE_NET("Found factory ",RTI_FALSE);
            OSAPI_TRACE_STRING("name",
                               *REDA_StringSeq_get_reference(&fac_names,count),
                               RTI_TRUE);
        }

        rule->transform =
            (struct UDP_Transform *)rule->factory->intf->create_component(
                                        rule->factory,&property->_parent,NULL);

        if (rule->transform == NULL)
        {
            UDP_TRANSFORM_LOG_ALLOC(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_FACTORY);
            goto done;
        }
        else
        {
            OSAPI_TRACE_NET("Created factory ",RTI_FALSE);
            OSAPI_TRACE_STRING("name",
                               *REDA_StringSeq_get_reference(&fac_names,count),
                               RTI_TRUE);
        }

        if (!RT_ComponentFactoryId_set_name(&rule->name,
                       *REDA_StringSeq_get_reference(&fac_names,count)))
        {
            /* Error logged by function */
            rule->factory->intf->delete_component(rule->factory,
                                                  &rule->transform->_parent);
            goto done;
        }

        if (!REDA_Indexer_add_entry(rules->factories,rule))
        {
            /* Error logged by function */
            rule->factory->intf->delete_component(rule->factory,
                                                  &rule->transform->_parent);
            UDP_TRANSFORM_LOG_ADD(OSAPI_LOGKIND_ERROR,
                                  UDP_TRANSFORM_LOG_KIND_FACTORY);
            goto done;
        }
        else
        {
            OSAPI_TRACE_NET("Added factory ",RTI_FALSE);
            OSAPI_TRACE_STRING("name",
                               RT_ComponentFactoryId_get_name(&rule->name),
                               RTI_TRUE);
        }

        /* set variable entry to NULL as it has been successfully added 
         * to the indexer. This would avoid undesired free in case there is
         * an error after this point 
         */
        rule = NULL;
    }

    /* Create transformation contexts, one for each rule. If there are
     * duplicates an error is logged and this function fails.
     */
    if (!UDP_TransformTable_add_rules(rules->destination_rules,
                                      rules->masks,
                                      dst_rules,rules->factories,RTI_FALSE,
                                      &rules->property))
    {
        /* Error logged by add_rules() */
        goto done;
    }

    if (!UDP_TransformTable_add_rules(rules->source_rules,
                                      rules->masks,
                                      src_rules,rules->factories,RTI_TRUE,
                                      &rules->property))
    {
        /* Error logged by add_rules() */
        goto done;
    }

    rval = RTI_TRUE;

done:

#ifndef RTI_CERT
    REDA_StringSeq_finalize(&fac_names);
#endif

    if (rval != RTI_TRUE)
    {
        if (rule != NULL)
        {
            OSAPI_Heap_free_struct(rule);
        }

        UDP_TransformTable_finalize(rules);
    }

    return rval;
}

/*ci
 * \brief Remove rules previously added by function 
 *        \ref UDP_TransformTable_add_rules
 *
 * @param[in] rule_index  The index to add rules to
 *
 */
RTI_PRIVATE RTI_BOOL
UDP_TransformTable_remove_rules(REDA_Indexer_T *rule_index,
                                RTI_BOOL is_source)
{
    REDA_IndexIterator_T *iterator = NULL;
    struct UDP_TransformEntry *entry = NULL;
    RTI_INT32 ec;

    iterator = REDA_Indexer_iterator_begin(rule_index);
    while ((entry = REDA_Indexer_iterator_next(iterator)) != NULL)
    {
        if (is_source)
        {

            if (!UDP_Transform_delete_source_transform(
                    entry->transform,entry->context,
                    &entry->address,&entry->netmask,&ec))
            {
                return RTI_FALSE;
            }
        }
        else
        {
            if (!UDP_Transform_delete_destination_transform(
                    entry->transform,entry->context,
                    &entry->address,&entry->netmask,&ec))
            {
                return RTI_FALSE;
            }
        }

        OSAPI_Heap_free_struct(entry);
    }

    return RTI_TRUE;
}

RTI_BOOL
UDP_TransformTable_finalize(struct UDP_TransformTable *rules)
{
    REDA_IndexIterator_T *iterator;
    struct UDP_TransformFactoryEntry *rule;
    RTI_BOOL retval = RTI_FALSE;

    if (rules == NULL)
    {
        goto done;
    }

    if (rules->factories == NULL)
    {
        /* Cannot be any rules */
        retval = RTI_TRUE;
        goto done;
    }

    /* Delete all the rules */
    if (rules->destination_rules != NULL)
    {
        if (!UDP_TransformTable_remove_rules(rules->destination_rules,RTI_FALSE))
        {
            goto done;
        }

#ifndef RTI_CERT
        if (!REDA_Indexer_delete(rules->destination_rules))
        {
            goto done;
        }
#endif
    }

    if (rules->source_rules != NULL)
    {
        if (!UDP_TransformTable_remove_rules(rules->source_rules,RTI_TRUE))
        {
            goto done;
        }

#ifndef RTI_CERT
        if (!REDA_Indexer_delete(rules->source_rules))
        {
            goto done;
        }
#endif
    }

    if (rules->masks != NULL)
    {
#ifndef RTI_CERT
        if (!REDA_Indexer_delete(rules->masks))
        {
            goto done;
        }
#endif
    }

    iterator = REDA_Indexer_iterator_begin(rules->factories);
    while((rule = (struct UDP_TransformFactoryEntry*)
                      REDA_Indexer_iterator_next(iterator)) != NULL)
    {
        if (rule->transform != NULL)
        {
            rule->factory->intf->delete_component(rule->factory,
                                          (RT_Component_T *)rule->transform);
        }
        OSAPI_Heap_free_struct(rule);
    }

#ifndef RTI_CERT

    if (!REDA_Indexer_delete(rules->factories))
    {
        goto done;
    }
#endif

    retval = RTI_TRUE;

done:
    return retval;
}

/*ci
 * \brief Search for a transformation rules for the given address
 *
 * @param[in] rules   Index to search in
 * @param[in] masks   index of netmasks
 * @param[in] address Address to find a matching rule for
 *
 * \return pointer to TransformationEntry if a rule is found, NULL otherwise
 */
RTI_PRIVATE struct UDP_TransformEntry*
UDP_TransformTable_find_transform(REDA_Indexer_T *rules,
                                  REDA_Indexer_T *masks,
                                  struct NETIO_Address *address)
{
    REDA_IndexIterator_T *mask_iter;
    struct NETIO_Address masked_addr;
    struct UDP_TransformEntry *entry = NULL;
    struct UDP_TransformEntry *mask_entry = NULL;

    mask_iter = REDA_Indexer_iterator_begin(masks);

    mask_entry = REDA_Indexer_iterator_next(mask_iter);
    while (mask_entry != NULL)
    {
        masked_addr.value.ipv4.address =
                    address->value.ipv4.address & mask_entry->netmask.mask[0];

        entry = REDA_Indexer_find_entry(rules,&masked_addr);
        if (entry != NULL)
        {
            break;
        }

        mask_entry = REDA_Indexer_iterator_next(mask_iter);
    }

    return entry;
}

/*ci
 * \brief Search for a transformation context for the given address
 *
 * @param[in]  rules   Index to search in
 * @param[in]  masks   index of netmasks
 * @param[in]  address Address to find a matching rule for
 * @param[out] context Context associated with rule if it exists
 *
 * \return RTI_TRUE if a rule was found, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
UDP_TransformTable_find_context(REDA_Indexer_T *rules,
                                REDA_Indexer_T *masks,
                                struct NETIO_Address *address,
                                void **context)
{
    struct UDP_TransformEntry *entry = NULL;

    if (rules == NULL)
    {
        return RTI_FALSE;
    }

    entry = UDP_TransformTable_find_transform(rules,masks,address);

    if (entry != NULL)
    {
        *context = entry->context;
    }

    return (entry != NULL ? RTI_TRUE : RTI_FALSE);
}

/*ci
 * \brief Search for a source transformation context
 *
 * @param[in]  rules   Rules table to search in
 * @param[in]  address Address to find a matching rule for
 * @param[out] context Context associated with rule if it exists
 *
 * NOTE: This function is currently only used for testing
 *
 * \return RTI_TRUE if a rule was found, RTI_FALSE otherwise
 */
RTI_BOOL
UDP_TransformTable_find_source_context(struct UDP_TransformTable *rules,
                                          struct NETIO_Address *address,
                                          void **context)
{
    return UDP_TransformTable_find_context(rules->source_rules,
                                           rules->masks,address,context);
}

/*ci
 * \brief Search for a destination transformation context
 *
 * @param[in]  rules   Rules table to search in
 * @param[in]  address Address to find a matching rule for
 * @param[out] context Context associated with rule if it exists
 *
 * NOTE: This function is currently only used for testing
 *
 * \return RTI_TRUE if a rule was found, RTI_FALSE otherwise
 */
RTI_BOOL
UDP_TransformTable_find_destination_context(struct UDP_TransformTable *rules,
                                            struct NETIO_Address *address,
                                            void **context)
{
    return UDP_TransformTable_find_context(rules->destination_rules,
                                           rules->masks,address,context);
}

/*ci
 * \brief Check if any transformation exists for the specified address
 *
 * @param[in] rules   Rules to search in
 * @param[in] address Address to find a matching rule for
 *
 * \return RTI_TRUE if a rule exists, otherwise RTI_FALSE
 */
RTI_BOOL
UDP_TransformTable_has_transform(struct UDP_TransformTable *rules,
                                 struct NETIO_Address *address)
{
    struct UDP_TransformEntry *entry = NULL;

    if (rules->destination_rules)
    {
        entry = UDP_TransformTable_find_transform(rules->destination_rules,
                                                  rules->masks,
                                                  address);
    }

    if ((entry == NULL) && rules->source_rules)
    {
        entry = UDP_TransformTable_find_transform(rules->source_rules,
                                                  rules->masks,
                                                  address);
    }

    return (entry != NULL ? RTI_TRUE : RTI_FALSE);
}

/*ci
 * \brief Transform an outgoing packet based on the destination address
 *
 * Search for a matching transformation entry. If none is found, return
 * an error since it is considered an error if no rule if found. if a rule
 * is found, but the transformation fails return an error.
 *
 * @param[in]  rules       The rules database to finalize
 * @param[in]  destination The destination address
 * @param[in]  in_packet   The packet to transform
 * @param[out] out_packet  The transformed packet
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
UDP_TransformTable_transform_outgoing(struct UDP_TransformTable *rules,
                                      struct NETIO_Address *destination,
                                      NETIO_Packet_T *in_packet,
                                      NETIO_Packet_T **out_packet)
{
    struct UDP_TransformEntry *entry = NULL;
    RTI_INT32 ec;

    if (rules->destination_rules == NULL)
    {
        return RTI_FALSE;
    }

    entry = UDP_TransformTable_find_transform(rules->destination_rules,
                                              rules->masks,
                                              destination);

    if (entry == NULL)
    {
        UDP_TRANSFORM_LOG_FIND(OSAPI_LOGKIND_ERROR,
                               UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE);
        return RTI_FALSE;
    }

    if (!UDP_Transform_transform_destination(entry->transform,entry->context,
                                          destination,in_packet,out_packet,&ec))
    {
        UDP_TRANSFORM_LOG_TRANSFORM(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_TRANSFORM_DESTINATION,
                                    ec);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Transform an incoming packet based on the destination address
 *
 * Search for a matching transformation entry. If none is found, return
 * an error since it is considered an error if no rule if found. if a rule
 * is found, but the transformation fails return an error.
 *
 * @param[in]  rules       The rules database to finalize
 * @param[in]  destination The source address
 * @param[in]  in_packet   The packet to transform
 * @param[out] out_packet  The transformed packet
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
UDP_TransformTable_transform_incoming(struct UDP_TransformTable *rules,
                                      struct NETIO_Address *source,
                                      NETIO_Packet_T *in_packet,
                                      NETIO_Packet_T **out_packet)
{
    struct UDP_TransformEntry *entry = NULL;
    RTI_INT32 ec;

    if (rules->source_rules == NULL)
    {
        return RTI_FALSE;
    }

    entry = UDP_TransformTable_find_transform(rules->source_rules,rules->masks,
                                              source);

    if (entry == NULL)
    {
        UDP_TRANSFORM_LOG_FIND(OSAPI_LOGKIND_ERROR,
                               UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE);
        return RTI_FALSE;
    }

    if (!UDP_Transform_transform_source(entry->transform,
                                        entry->context,source,
                                        in_packet,out_packet,&ec))
    {
        UDP_TRANSFORM_LOG_TRANSFORM(OSAPI_LOGKIND_ERROR,
                                    UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE,ec);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * @}
 */

#endif
