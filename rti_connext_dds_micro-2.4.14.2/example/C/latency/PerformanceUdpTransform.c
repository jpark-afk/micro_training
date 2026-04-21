/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#include "PerformanceUdpTransform.h"

#if PERF_TRANSFORMS_ENABLED

/*ci
 * \brief Default properties for the UDP transform factory
 */
struct PerformanceUdpTransformFactoryProperty 
PERFORMANCE_UDP_TRANSFORM_FACTORY_PROPERTY_DEFAULT =
                             PerformanceUdpTransformFactoryProperty_INITIALIZER;

struct PerformanceUdpTransformFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;

    struct PerformanceUdpTransformFactoryProperty *property;
};

struct PerformanceUdpTransform
{
    /*ci
     * \brief Base-class
     */
    struct UDP_Transform _parent;

    struct PerformanceUdpTransformFactory *factory;

    NETIO_Packet_T packet;
};

static struct UDP_TransformI PerformanceUdpTransform_fv_Intf;
static struct RT_ComponentFactoryI PerformanceUdpTransformFactory_fv_Intf;

RTI_BOOL
PerformanceUdpTransformFactoryProperty_initialize(
                               struct PerformanceUdpTransformFactoryProperty *p)
{
    struct PerformanceUdpTransformFactoryProperty init =
                            PerformanceUdpTransformFactoryProperty_INITIALIZER;

    *p = init;

    return RTI_TRUE;
}

RTI_BOOL
PerformanceUdpTransformFactoryProperty_finalize
                              (struct PerformanceUdpTransformFactoryProperty *p)
{
    return RTI_TRUE;
}

/*ci
 * \brief Creates a transformation instance 
 *
 * \param[in] factory  Factory which is creating the transformation instance.
 * \param[in] property Transformation properties registered with the 
 *                     transformation.
 *
 * \return NULL in case an error occurs or a pointer to the UDP transformation
 *         created if no error.
 */
RTI_PRIVATE struct PerformanceUdpTransform*
PerformanceUdpTransform_create(struct PerformanceUdpTransformFactory *factory,
                              const struct UDP_TransformProperty *const property)
{
    struct PerformanceUdpTransform *t;

    OSAPI_Heap_allocate_struct(&t, struct PerformanceUdpTransform);
    if (t == NULL)
    {
        return NULL;
    }

    RT_Component_initialize(&t->_parent._parent,
                            &PerformanceUdpTransform_fv_Intf._parent,
                            0,
                            (property ? &property->_parent : NULL),
                            NULL);

    t->factory = factory;

    return t;
}

/*ci
 * \brief Deletes a transformation instance 
 *
 * \param[in] t Pointer to transformation to delete
 */
RTI_PRIVATE void
PerformanceUdpTransform_delete(struct PerformanceUdpTransform *t)
{
    if (t != NULL)
    {
        OSAPI_Heap_free_struct(t);
    }
}

/*ci
 * \brief Creates a destination transformation
 *
 * \param[in]  udptf       UDP Transform instance that creates the transformation
 * \param[out] context     Destination transformation can store a context data
 * \param[in]  destination Destination address
 * \param[in]  netmask     Destination mask
 * \param[in]  user_data   The user_data the rule was asserted with
 * \param[in]  property    UDP transform specific properties
 * \param[out] ec          User-defined error code
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_create_destination_transform(
                                     UDP_Transform_T *udptf,
                                     void **const context,
                                     const struct NETIO_Address *destination,
                                     const struct NETIO_Netmask *netmask,
                                     void *user_data,
                                     const struct UDP_TransformProperty *property,
                                     RTI_INT32 *ec)
{
    struct PerformanceUdpTransform *self = (struct PerformanceUdpTransform*)udptf;

    UNUSED_ARG(self);
    UNUSED_ARG(destination);
    UNUSED_ARG(user_data);
    UNUSED_ARG(property);
    UNUSED_ARG(ec);
    UNUSED_ARG(netmask);

    *context = (void*)NULL;

    return RTI_TRUE;
}

/*ci
 * \brief Deletes a destination transformation
 *
 * \param[in]  udptf       UDP Transform instance that creates the transformation
 * \param[out] context     Destination transformation context
 * \param[in]  destination Destination address
 * \param[in]  netmask     Destination mask
 * \param[out] ec          User-defined error code
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_delete_destination_transform(
                                            UDP_Transform_T *const udptf,
                                            void *context,
                                            const struct NETIO_Address *const destination,
                                            const struct NETIO_Netmask *const netmask,
                                            RTI_INT32 *ec)
{
    UNUSED_ARG(udptf);
    UNUSED_ARG(context);
    UNUSED_ARG(destination);
    UNUSED_ARG(ec);
    UNUSED_ARG(netmask);

    return RTI_TRUE;
}

/*ci
 * \brief Creates a source transformation
 *
 * \param[in]  udptf       UDP Transform instance that creates the transformation
 * \param[out] context     Source transformation can store a context data
 * \param[in]  destination Source address
 * \param[in]  netmask     Source mask
 * \param[in]  user_data   The user_data the rule was asserted with
 * \param[in]  property    UDP transform specific properties
 * \param[out] ec          User-defined error code
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_create_source_transform(
                                       UDP_Transform_T *const udptf,
                                       void **const context,
                                       const struct NETIO_Address *const source,
                                       const struct NETIO_Netmask *const netmask,
                                       void *user_data,
                                       const struct UDP_TransformProperty *const property,
                                       RTI_INT32 *ec)
{
    struct PerformanceUdpTransform *self = (struct PerformanceUdpTransform*)udptf;

    UNUSED_ARG(self);
    UNUSED_ARG(source);
    UNUSED_ARG(user_data);
    UNUSED_ARG(property);
    UNUSED_ARG(ec);
    UNUSED_ARG(netmask);

    *context = (void*)NULL;

    return RTI_TRUE;
}

/*ci
 * \brief Deletes a source transformation
 *
 * \param[in]  udptf       UDP Transform instance that creates the transformation
 * \param[out] context     Source transformation context
 * \param[in]  destination Source address
 * \param[in]  netmask     Source mask
 * \param[out] ec          User-defined error code
 *
 * \return This function always returns RTI_TRUE.
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_delete_source_transform(
                                       UDP_Transform_T *const udptf,
                                       void *context,
                                       const struct NETIO_Address *const source,
                                       const struct NETIO_Netmask *const netmask,
                                       RTI_INT32 *ec)
{
    UNUSED_ARG(udptf);
    UNUSED_ARG(context);
    UNUSED_ARG(source);
    UNUSED_ARG(ec);
    UNUSED_ARG(netmask);

    return RTI_TRUE;
}

/*ci
 * \brief Called when a received packet matches a transformation rule
 *        or when a sent packet matches a transformation rule
 *
 * \param[in]  self        UDP_Transform_T that performs the transformation
 * \param[in]  source      Source address for the transformation
 * \param[in]  context     Reference to context created by create_source_transform
 * \param[in]  packet_in   The NETIO packet to transform
 * \param[out] packet_out  The transformed NETIO packet
 * \param[out] ec          User defined error code
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_transform(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const source,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *ec)
{
    struct PerformanceUdpTransform *self = (struct PerformanceUdpTransform*)udptf;
    unsigned char *from_buf_ptr;
    int input_length;

    UNUSED_ARG(context);
    UNUSED_ARG(source);

    *ec = 0;

    /* Tthis is only valid because the output packet is exactly the same as the
     * input packet. In case there was any change it would be needed to use
     * a different buffer for the output packet.
     */
    input_length = NETIO_Packet_get_payload_length(in_packet);
    from_buf_ptr = NETIO_Packet_get_head(in_packet);

    if (!NETIO_Packet_initialize_from(&self->packet,
                                      in_packet,
                                      from_buf_ptr,
                                      input_length,
                                      0,
                                      input_length))
    {
        return RTI_FALSE;
    }

    *out_packet = &self->packet;

    return RTI_TRUE;
}
/*ci
 * \brief Called when a received packet matches a transformation rule
 *
 * \param[in]  self        UDP_Transform_T that performs the transformation
 * \param[in]  source      Source address for the transformation
 * \param[in]  context     Reference to context created by create_source_transform
 * \param[in]  packet_in   The NETIO packet to transform
 * \param[out] packet_out  The transformed NETIO packet
 * \param[out] ec          User defined error code
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_transform_source(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const source,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *ec)
{
    return PerformanceUdpTransform_transform(udptf,
                                             context,
                                             source,
                                             in_packet,
                                             out_packet,
                                             ec);
}

/*ci
 * \brief Called when a sent packet matches a transformation rule
 *
 * \param[in]  self        UDP_Transform_T that performs the transformation
 * \param[in]  destination Destination address for the transformation
 * \param[in]  context     Reference to context created by create_destination_transform
 * \param[in]  packet_in   The NETIO packet to transform
 * \param[out] packet_out  The transformed NETIO packet
 * \param[out] ec          User defined error code
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
PerformanceUdpTransform_transform_destination(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const destination,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *ec)
{
    return PerformanceUdpTransform_transform(udptf,
                                             context,
                                             destination,
                                             in_packet,
                                             out_packet,
                                             ec);
}

RTI_PRIVATE struct UDP_TransformI PerformanceUdpTransform_fv_Intf =
{
    RT_COMPONENTI_BASE,
    PerformanceUdpTransform_create_destination_transform,
    PerformanceUdpTransform_create_source_transform,
    PerformanceUdpTransform_transform_source,
    PerformanceUdpTransform_transform_destination,
    PerformanceUdpTransform_delete_destination_transform,
    PerformanceUdpTransform_delete_source_transform
};

MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
PerformanceUdpTransformFactory_create_component(
                                         struct RT_ComponentFactory *factory,
                                         struct RT_ComponentProperty *property,
                                         struct RT_ComponentListener *listener)
{
    struct PerformanceUdpTransform *t;
    UNUSED_ARG(listener);

    t = PerformanceUdpTransform_create(
                                (struct PerformanceUdpTransformFactory*)factory,
                                (struct UDP_TransformProperty*)property);

    return &t->_parent._parent;
}

RTI_PRIVATE void
PerformanceUdpTransformFactory_delete_component(
                                       struct RT_ComponentFactory *factory,
                                       RT_Component_T *component)
{
    UNUSED_ARG(factory);

    PerformanceUdpTransform_delete((struct PerformanceUdpTransform*)component);
}


MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
PerformanceUdpTransformFactory_initialize(
                                 struct RT_ComponentFactoryProperty* property,
                                 struct RT_ComponentFactoryListener *listener)
{
    struct PerformanceUdpTransformFactory *fac;

    UNUSED_ARG(listener);

    OSAPI_Heap_allocate_struct(&fac,struct PerformanceUdpTransformFactory);

    fac->_parent._factory = &fac->_parent;
    fac->_parent.intf = &PerformanceUdpTransformFactory_fv_Intf;
    fac->property = (struct PerformanceUdpTransformFactoryProperty*)property;

    return &fac->_parent;
}

RTI_PRIVATE void
PerformanceUdpTransformFactory_finalize(struct RT_ComponentFactory *factory,
                                   struct RT_ComponentFactoryProperty **property,
                                   struct RT_ComponentFactoryListener **listener)
{
    struct PerformanceUdpTransformFactory *fac =
                                 (struct PerformanceUdpTransformFactory*)factory;

    if (listener != NULL)
    {
        *listener = NULL;
    }

    if (property != NULL)
    {
        *property = (struct RT_ComponentFactoryProperty*)fac->property;
    }

    OSAPI_Heap_free_struct(factory);

    return;
}

RTI_PRIVATE struct RT_ComponentFactoryI PerformanceUdpTransformFactory_fv_Intf =
{
    UDP_INTERFACE_INTERFACE_ID,
    PerformanceUdpTransformFactory_initialize,
    PerformanceUdpTransformFactory_finalize,
    PerformanceUdpTransformFactory_create_component,
    PerformanceUdpTransformFactory_delete_component,
    NULL
};

struct RT_ComponentFactoryI*
PerformanceUdpTransformFactory_get_interface(void)
{
    return &PerformanceUdpTransformFactory_fv_Intf;
}

RTI_BOOL
PerformanceUdpTransformFactory_register(RT_Registry_T *registry,
                         const char *const name,
                         struct PerformanceUdpTransformFactoryProperty *property)
{
    return RT_Registry_register(registry, name,
                                PerformanceUdpTransformFactory_get_interface(),
                                &property->_parent, NULL);
}

RTI_BOOL
PerformanceUdpTransformFactory_unregister(RT_Registry_T *registry,
            const char *const name,
            struct PerformanceUdpTransformFactoryProperty **property)
{
    return RT_Registry_unregister(registry, name,
                              (struct RT_ComponentFactoryProperty**)property,
                              NULL);
}

#endif /* PERF_TRANSFORMS_ENABLED */
