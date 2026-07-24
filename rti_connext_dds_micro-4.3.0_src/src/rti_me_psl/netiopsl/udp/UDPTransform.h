/*
 * FILE: UDPTransform.h - UDP Transform Interface
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
#include "rti_me_psl.h"

#include "netio/netio_config.h"
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif

#if UDP_TRANSFORMS_ENABLED

#ifndef UDPTransform_h
#define UDPTransform_h

#include "osapi/osapi_heap.h"
#include "osapi/osapi_log.h"
#include "reda/reda_indexer.h"
#include "rt/rt_rt.h"
#include "netio/netio_log.h"
#include "netio/netio_udp.h"

/*ci \brief A transformation factory entry
 */
struct UDP_TransformFactoryEntry
{
    /*ci \brief The name of the transformation
     */
    RT_ComponentFactoryId_T name;
    
    /*ci \brief The factory that created the entry
     */
    struct RT_ComponentFactory *factory;
    
    /*ci \brief The transformations
     */
    struct UDP_Transform *transform;
};

/*ci \brief A tranformation entry
 */
struct UDP_TransformEntry
{
    /*ci \brief The source/destination of a transformation 
     */
    struct NETIO_Address key;

    /*ci \brief The address the transformation applies to 
     */
    struct NETIO_Address address;

    /*ci \brief The netmask to apply to a transformation to 
     *   determine if a transformation applies to not. 
     */
    struct NETIO_Netmask netmask;
    
    /*ci \brief User defined contexts
     */
    void *context;
    
    /*ci \brief The transformation
     */
    struct UDP_Transform *transform;
    
    /*ci \brief User defined data
     */
    void *user_data;
};

/*ci \brief The transformation table
 */
struct UDP_TransformTable
{
    /*ci \brief An index of all the transformation factories
     */
    REDA_Indexer_T *factories;
    
    /*ci \brief Transformations
     */
    struct UDP_Transform *transform;
    
    /*ci \brief Index of transformation masks
     */
    REDA_Indexer_T *masks;
    
    /*ci \brief Index of source rules
     */
    REDA_Indexer_T *source_rules;
    
    /*ci \brief Index of destination rules
     */
    REDA_Indexer_T *destination_rules;
    
    /*ci \brief Properties for the transformation table
     */
    struct UDP_TransformProperty property;
};

/*ci
 * \brief Initialize a transformation database based on source and destination
 *        rules.
 *
 * \details
 * This function initializes a rules database based on the input source and
 * destination rule. For each rule a transformation context is created which
 * is later passed to the transformation rule. A single instance of each
 * transformation is created.
 *
 * @param[in] rules     The rules database to initialize
 * @param[in] dst_rules The destination rules
 * @param[in] src_rules The source rules
 * @param[in] property  Transformation Properties
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
UDP_TransformTable_initialize(struct UDP_TransformTable *rules,
                              RT_Registry_T *registry,
                              struct UDP_TransformRuleSeq *dst_rules,
                              struct UDP_TransformRuleSeq *src_rules,
                              struct UDP_TransformProperty *property);

/*ci
 * \brief Finalize transformation database based on source and destination
 *        rules.
 *
 * \details
 * This function finalizes a rules database and releases all resources. A rules
 * database must never been accessed after it has been finalized.
 *
 * @param[in] rules The rules database to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
UDP_TransformTable_finalize(struct UDP_TransformTable *rules);

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
extern RTI_BOOL
UDP_TransformTable_transform_outgoing(struct UDP_TransformTable *rules,
                                      struct NETIO_Address *destination,
                                      NETIO_Packet_T *in_packet,
                                      NETIO_Packet_T **out_packet);

/*ci
 * \brief Transform an incoming packet based on the source address
 *
 * @param[in]  rules       The rules database to finalize
 * @param[in]  source      The source address
 * @param[in]  in_packet   The packet to transform
 * @param[out] out_packet  The transformed packet
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
UDP_TransformTable_transform_incoming(struct UDP_TransformTable *rules,
                                      struct NETIO_Address *destination,
                                      NETIO_Packet_T *in_packet,
                                      NETIO_Packet_T **out_packet);

/*ci
 * \brief Initialize a transformation database based on source and destination
 *        rules.
 *
 * @param[in] rules The rules database where to search for the transform
 * @param[in] address The destination rules
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
UDP_TransformTable_has_transform(struct UDP_TransformTable *rules,
                                 struct NETIO_Address *address);

/*ci
 * \brief Search for a destination transformation context for the given address
 *
 * @param[in]  rules   Rules table
 * @param[in]  address Address to find a matching rule for
 * @param[out] context Context associated with rule if it exists
 *
 * \return RTI_TRUE if a rule was found, RTI_FALSE otherwise
 */
extern RTI_BOOL
UDP_TransformTable_find_destination_context(struct UDP_TransformTable *rules,
                                            struct NETIO_Address *address,
                                            void **context);

/*ci
 * \brief Search for a source transformation context for the given address
 *
 * @param[in]  rules   Rules table
 * @param[in]  address Address to find a matching rule for
 * @param[out] context Context associated with rule if it exists
 *
 * \return RTI_TRUE if a rule was found, RTI_FALSE otherwise
 */
extern RTI_BOOL
UDP_TransformTable_find_source_context(struct UDP_TransformTable *rules,
                                       struct NETIO_Address *address,
                                       void **context);

#endif

#endif

/*ci
 * @}
 */
