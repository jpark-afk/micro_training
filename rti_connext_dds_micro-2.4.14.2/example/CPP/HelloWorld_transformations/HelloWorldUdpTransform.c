/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#include "HelloWorldUdpTransform.h"
#include "HelloWorldEncryption.h"

/*ci
 * \brief Default properties for the UDP transform factory
 */
struct HelloWorldUdpTransformFactoryProperty 
HELLOWORLD_UDP_TRANSFORM_FACTORY_PROPERTY_DEFAULT =
                             HelloWorldUdpTransformFactoryProperty_INITIALIZER;

struct HelloWorldUdpTransformFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;

    struct HelloWorldUdpTransformFactoryProperty *property;
};

struct HelloWorldUdpTransform
{
    /*ci
     * \brief Base-class
     */
    struct UDP_Transform _parent;

    struct HelloWorldUdpTransformFactory *factory;

    NETIO_Packet_T packet;

    unsigned char *buffer;

    RTI_INT32 buffer_length;

    EVP_CIPHER_CTX *cypher_context;
};

static struct UDP_TransformI HelloWorldUdpTransform_fv_Intf;
static struct RT_ComponentFactoryI HelloWorldUdpTransformFactory_fv_Intf;

RTI_BOOL
HelloWorldUdpTransformFactoryProperty_initialize(
                               struct HelloWorldUdpTransformFactoryProperty *p)
{
    struct HelloWorldUdpTransformFactoryProperty init =
                            HelloWorldUdpTransformFactoryProperty_INITIALIZER;

    *p = init;

    return RTI_TRUE;
}

RTI_BOOL
HelloWorldUdpTransformFactoryProperty_finalize
                              (struct HelloWorldUdpTransformFactoryProperty *p)
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
RTI_PRIVATE struct HelloWorldUdpTransform*
HelloWorldUdpTransform_create(struct HelloWorldUdpTransformFactory *factory,
                              const struct UDP_TransformProperty *const property)
{
    struct HelloWorldUdpTransform *t;

    OSAPI_Heap_allocate_struct(&t, struct HelloWorldUdpTransform);
    if (t == NULL)
    {
        return NULL;
    }

    if (!HelloWorldEncryption_initalize(&t->cypher_context))
    {
        OSAPI_Heap_free_struct(t);
        return NULL;
    }

    RT_Component_initialize(&t->_parent._parent,
                            &HelloWorldUdpTransform_fv_Intf._parent,
                            0,
                            (property ? &property->_parent : NULL),
                            NULL);

    t->factory = factory;

    if (property->max_receive_message_size > property->max_send_message_size)
    {
        t->buffer_length = property->max_receive_message_size;
    }
    else
    {
        t->buffer_length = property->max_send_message_size;
    }
    OSAPI_Heap_allocate_buffer((char**)&t->buffer, 
                               t->buffer_length, 
                               OSAPI_ALIGNMENT_DEFAULT);

    if (t->buffer == NULL)
    {
        (void)HelloWorldEncryption_finalize(t->cypher_context);
        OSAPI_Heap_free_struct(t);
        t = NULL;
    }

    return t;
}

/*ci
 * \brief Deletes a transformation instance 
 *
 * \param[in] t Pointer to transformation to delete
 */
RTI_PRIVATE void
HelloWorldUdpTransform_delete(struct HelloWorldUdpTransform *t)
{
    if (t != NULL)
    {
        OSAPI_Heap_free_buffer((char**)t->buffer);

        (void)HelloWorldEncryption_finalize(t->cypher_context);

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
HelloWorldUdpTransform_create_destination_transform(
                                     UDP_Transform_T *udptf,
                                     void **const context,
                                     const struct NETIO_Address *destination,
                                     const struct NETIO_Netmask *netmask,
                                     void *user_data,
                                     const struct UDP_TransformProperty *property,
                                     RTI_INT32 *ec)
{
    struct HelloWorldUdpTransform *self = (struct HelloWorldUdpTransform*)udptf;

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
HelloWorldUdpTransform_delete_destination_transform(
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
HelloWorldUdpTransform_create_source_transform(
                                       UDP_Transform_T *const udptf,
                                       void **const context,
                                       const struct NETIO_Address *const source,
                                       const struct NETIO_Netmask *const netmask,
                                       void *user_data,
                                       const struct UDP_TransformProperty *const property,
                                       RTI_INT32 *ec)
{
    struct HelloWorldUdpTransform *self = (struct HelloWorldUdpTransform*)udptf;

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
HelloWorldUdpTransform_delete_source_transform(
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
HelloWorldUdpTransform_transform_source(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const source,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *ec)
{
    struct HelloWorldUdpTransform *self = (struct HelloWorldUdpTransform*)udptf;
    unsigned char *buf_ptr;
    unsigned char *from_buf_ptr;
    int input_length, output_length;

    UNUSED_ARG(context);
    UNUSED_ARG(source);

    *ec = 0;

    if (!NETIO_Packet_initialize_from(&self->packet,
                                      in_packet,
                                      self->buffer,
                                      self->buffer_length,
                                      0,
                                      self->buffer_length))
    {
        return RTI_FALSE;
    }

    *out_packet = &self->packet;

    buf_ptr = NETIO_Packet_get_head(&self->packet);
    from_buf_ptr = NETIO_Packet_get_head(in_packet);

    input_length = NETIO_Packet_get_payload_length(in_packet);
    output_length = self->buffer_length;
    if (!HelloWorldEncryption_decrypt(self->cypher_context,
                                      from_buf_ptr, input_length, 
                                      buf_ptr, &output_length))
    {
        return RTI_FALSE;
    }
    if (!NETIO_Packet_set_tail(&self->packet, output_length - self->buffer_length))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
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
HelloWorldUdpTransform_transform_destination(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const destination,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *ec)
{
    struct HelloWorldUdpTransform *self = (struct HelloWorldUdpTransform*)udptf;
    unsigned char *buf_ptr;
    unsigned char *from_buf_ptr;
    int input_length, output_length;

    UNUSED_ARG(context);
    UNUSED_ARG(destination);

    *ec = 0;

    if (!NETIO_Packet_initialize_from(&self->packet,
                                      in_packet,
                                      self->buffer,
                                      self->buffer_length,
                                      0,
                                      self->buffer_length))
    {
        return RTI_FALSE;
    }

    *out_packet = &self->packet;

    buf_ptr = NETIO_Packet_get_head(&self->packet);
    from_buf_ptr = NETIO_Packet_get_head(in_packet);

    input_length = NETIO_Packet_get_payload_length(in_packet);
    output_length = self->buffer_length;
    if (!HelloWorldEncryption_encrypt(self->cypher_context,
                                      from_buf_ptr, input_length, 
                                      buf_ptr, &output_length))
    {
        return RTI_FALSE;
    }
    if (!NETIO_Packet_set_tail(&self->packet, output_length - self->buffer_length))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE struct UDP_TransformI HelloWorldUdpTransform_fv_Intf =
{
    RT_COMPONENTI_BASE,
    HelloWorldUdpTransform_create_destination_transform,
    HelloWorldUdpTransform_create_source_transform,
    HelloWorldUdpTransform_transform_source,
    HelloWorldUdpTransform_transform_destination,
    HelloWorldUdpTransform_delete_destination_transform,
    HelloWorldUdpTransform_delete_source_transform
};

MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
HelloWorldUdpTransformFactory_create_component(
                                         struct RT_ComponentFactory *factory,
                                         struct RT_ComponentProperty *property,
                                         struct RT_ComponentListener *listener)
{
    struct HelloWorldUdpTransform *t;
    UNUSED_ARG(listener);

    t = HelloWorldUdpTransform_create(
                                (struct HelloWorldUdpTransformFactory*)factory,
                                (struct UDP_TransformProperty*)property);

    return &t->_parent._parent;
}

RTI_PRIVATE void
HelloWorldUdpTransformFactory_delete_component(
                                       struct RT_ComponentFactory *factory,
                                       RT_Component_T *component)
{
    UNUSED_ARG(factory);

    HelloWorldUdpTransform_delete((struct HelloWorldUdpTransform*)component);
}


MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
HelloWorldUdpTransformFactory_initialize(
                                 struct RT_ComponentFactoryProperty* property,
                                 struct RT_ComponentFactoryListener *listener)
{
    struct HelloWorldUdpTransformFactory *fac;

    UNUSED_ARG(listener);

    OSAPI_Heap_allocate_struct(&fac,struct HelloWorldUdpTransformFactory);

    fac->_parent._factory = &fac->_parent;
    fac->_parent.intf = &HelloWorldUdpTransformFactory_fv_Intf;
    fac->property = (struct HelloWorldUdpTransformFactoryProperty*)property;

    return &fac->_parent;
}

RTI_PRIVATE void
HelloWorldUdpTransformFactory_finalize(struct RT_ComponentFactory *factory,
                                   struct RT_ComponentFactoryProperty **property,
                                   struct RT_ComponentFactoryListener **listener)
{
    struct HelloWorldUdpTransformFactory *fac =
                                 (struct HelloWorldUdpTransformFactory*)factory;

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

RTI_PRIVATE struct RT_ComponentFactoryI HelloWorldUdpTransformFactory_fv_Intf =
{
    UDP_INTERFACE_INTERFACE_ID,
    HelloWorldUdpTransformFactory_initialize,
    HelloWorldUdpTransformFactory_finalize,
    HelloWorldUdpTransformFactory_create_component,
    HelloWorldUdpTransformFactory_delete_component,
    NULL
};

struct RT_ComponentFactoryI*
HelloWorldUdpTransformFactory_get_interface(void)
{
    return &HelloWorldUdpTransformFactory_fv_Intf;
}

RTI_BOOL
HelloWorldUdpTransformFactory_register(RT_Registry_T *registry,
                         const char *const name,
                         struct HelloWorldUdpTransformFactoryProperty *property)
{
    return RT_Registry_register(registry, name,
                                HelloWorldUdpTransformFactory_get_interface(),
                                &property->_parent, NULL);
}

RTI_BOOL
HelloWorldUdpTransformFactory_unregister(RT_Registry_T *registry,
            const char *const name,
            struct HelloWorldUdpTransformFactoryProperty **property)
{
    return RT_Registry_unregister(registry, name,
                              (struct RT_ComponentFactoryProperty**)property,
                              NULL);
}
