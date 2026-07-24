/*
 * FILE: Type.h - DDS Type implementation
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 11jun2013,tk MICRO-633: max type_length is 255 excluding NUL
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 *
 */
/*ce
 * \file
 * \brief DDS Type implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef Type_h
#define Type_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

#if DDS_XTYPES_IS_ENABLED
#include "xcdr/xcdr_interpreter.h"
#endif

/*ci
 * \brief Implementation of the DDS Type
 */
struct DDS_TypeImpl
{
    /*ci
     * \brief The type-name, only allocate what is needed
     */
    char *name;

    /*ci
     * \brief Pointer to the type-plugin interface
     */
    struct DDS_TypePluginI *plugin;

    /*ci
     * \brief Counter to keep track of many how many topics reference this type.
     *        A type can only be unregistered if there are 0 references to it
     */
    DDS_Long topic_count;

    /*ci
     * \brief Counter to keep track of many times a type has been registered.
     *        A type must be unregistered as many times as it has been
     *        registered.
     */
    DDS_Long ref_count;

#if DDS_XTYPES_IS_ENABLED

    /*ci
     * \brief Programs created a type registration used by the interpreter
     */
    struct RTIXCdrInterpreterPrograms *programs;

    /*ci
     * \brief needed types annotated with @transfer_mode(FLAT_DATA).
     * Needed as an intermediate step when generating key hashes
     */
    void *flat_data_plain_sample_helper;
    /*ci
     * \brief Typecode needed by the interpreter
     */
    struct DDS_TypeCode *typecode;
#endif
};

MUST_CHECK_RETURN extern RTI_INT32
DDS_TypeImpl_compare(RTI_INT32 flags,
                    const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_initalize(DDS_DomainParticipant *participant,
                       struct DDS_TypeImpl *type,
                       const char *type_name,
                       struct DDS_TypePluginI *plugin);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_finalize(struct DDS_TypeImpl *type);

SHOULD_CHECK_RETURN extern RTI_BOOL
DDS_Type_dtor(DB_Record_T type_record);

#endif /* !RTI_CERT */

extern void
DDS_TypeImpl_attach_topic(struct DDS_TypeImpl *type);

extern void
DDS_TypeImpl_detach_topic(struct DDS_TypeImpl *type);

extern DDS_Boolean
DDS_TypeImpl_is_attached(struct DDS_TypeImpl *type);

extern void
DDS_TypeImpl_reference(struct DDS_TypeImpl *type);

extern void
DDS_TypeImpl_dereference(struct DDS_TypeImpl *type);

extern DDS_Boolean
DDS_TypeImpl_is_referenced(struct DDS_TypeImpl *type);

extern void
DDS_TypeImpl_reset_reference(struct DDS_TypeImpl *type);

MUST_CHECK_RETURN struct DDS_TypePluginI*
DDS_TypeImpl_get_plugin(DDS_Type *self);

MUST_CHECK_RETURN extern const char*
DDS_TypeImpl_get_type_name_reference(DDS_Type *self);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_add_encapsulation_plugin(DDS_DomainParticipant *participant,
                                      struct DDS_TypeImpl *type,
                                      struct DDS_TypeEncapsulationI *intf);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_add_memory_plugin(DDS_DomainParticipant *participant,
                               struct DDS_TypeImpl *type,
                               struct DDS_TypeMemoryI *intf);

/* The qos parameter has been deprecated and is no longer used */

MUST_CHECK_RETURN extern struct DDS_TypePlugin*
DDS_TypeImpl_create_plugin(struct DDS_TypeImpl *type,
                           DDS_DomainParticipant *participant,
                           struct DDS_DomainParticipantQos *dp_qos,
                           DDS_TypePluginMode_T endpoint_mode,
                           DDS_TypePluginEndpoint *endpoint,
                           DDS_TypePluginEndpointQos *qos,
                           struct DDS_TypePluginProperty *property);

DDS_Boolean
DDS_TypePlugin_create_wire_plugin(struct DDS_TypePlugin *plugin,
                                 DDS_DomainParticipant *participant,
                                 struct DDS_DomainParticipantQos *dp_qos,
                                 DDS_TypePluginMode_T endpoint_mode,
                                 DDS_TypePluginEndpoint *endpoint,
                                 DDS_TypePluginEndpointQos *qos);

MUST_CHECK_RETURN extern DDS_EncapsulationId_t
DDS_Type_get_default_encapsulation(DDS_Type *type);

#endif

/*ci @} */
