/*
 * FILE: UDPTransformTester.c - UDP Transform Tester
 *
 * Copyright 2017-2024 Real-Time Innovations, Inc.
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
 * 15aug2017,tk  Written
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_time.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_system.h"
#include "rt/rt_rt.h"
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#include "test/test_setting.h"

#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif

#include "UDPInterface.h"
#include "UDPTransformTester.h"

#if UDP_TRANSFORMS_ENABLED

struct MyUdpTransformCreateCall
{
    RTI_BOOL is_source;
    RTI_BOOL retval;
    UDP_Transform_T *transform;
    void *context_data;
    struct NETIO_Address address;
    struct NETIO_Netmask netmask;
    void *user_data;
    struct UDP_TransformProperty property;
    RTI_INT32 ec;
};

struct MyUdpTransformDeleteCall
{
    RTI_BOOL is_source;
    RTI_BOOL retval;
    UDP_Transform_T *transform;
    void *context_data;
    struct NETIO_Address address;
    struct NETIO_Netmask netmask;
    RTI_INT32 ec;
};

struct MyUdpTransformTransformCall
{
    RTI_BOOL is_source;
    RTI_BOOL retval;
    void *context_data;
    struct NETIO_Address address;
    const NETIO_Packet_T *in_packet;
    const NETIO_Packet_T *out_packet;
    RTI_INT32 ec;
};

struct MyUdpTransformContextCall
{
    RTI_INT32 call_name;
    union
    {
        struct MyUdpTransformCreateCall create_call;
        struct MyUdpTransformDeleteCall delete_call;
        struct MyUdpTransformTransformCall transform_call;
    } call_param;
};

struct MyUdpTransformContextCallTest
{
    struct MyUdpTransformContextCall input;
    struct MyUdpTransformContextCall output;
};

struct MyUdpTransformContextCalls
{
    RTI_INT32 current_call_index;
    struct MyUdpTransformContextCallTest calls[32];
};

struct MyUdpTransformFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;

    struct MyUdpTransformFactoryProperty *property;
};

struct MyUdpTransform
{
    /*ci
     * \brief Base-class
     */
    struct UDP_Transform _parent;

    struct MyUdpTransformFactory *factory;

    struct UDP_TransformProperty property;
};

struct MyUdpTransformFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;

    RTI_BOOL ignore_callparam;
};

RTI_PRIVATE struct UDP_TransformI MyUdpTransform_fv_Intf;
static struct RT_ComponentFactoryI MyUdpTransformFactory_fv_Intf;

/* Transformation test */
RTI_PRIVATE struct MyUdpTransform*
MyUdpTransform_create(struct MyUdpTransformFactory *factory,
                      const struct UDP_TransformProperty *const property)
{
    struct MyUdpTransform *t;
    UNUSED_ARG(factory);

    OSAPI_Heap_allocate_struct(&t, struct MyUdpTransform);
    if (t == NULL)
    {
        return NULL;
    }

    OSAPI_Memory_zero(t,sizeof(struct MyUdpTransform));

    RT_Component_initialize(&t->_parent._parent,
                           &MyUdpTransform_fv_Intf._parent,
                           0,
                           (property ? &property->_parent : NULL),
                           NULL);

    t->factory = factory;
    
    if (property != NULL)
    {
        t->property = *property;
    }
    
    return t;
}

RTI_PRIVATE void
MyUdpTransform_delete(struct MyUdpTransform *t)
{
#ifdef RTI_CERT
    UNUSED_ARG(t);
#else
    OSAPI_Heap_free_struct(t);
#endif
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_create_transform(struct MyUdpTransform *const self,
                                void **const context,
                                const struct NETIO_Address *const destination,
                                const struct NETIO_Netmask *const netmask,
                                void *user_data,
                                const struct UDP_TransformProperty *const property,
                                RTI_INT32 *const ec,
                                RTI_BOOL is_source)
{
    struct MyUdpTransformContextCalls *call =
                            (struct MyUdpTransformContextCalls *)user_data;
    struct MyUdpTransformContextCallTest *api_call;


    *context = user_data;

    if (self->factory->property->ignore_callparam)
    {
        return RTI_TRUE;
    }

    ++call->current_call_index;
    api_call = &call->calls[call->current_call_index];

    /* Input */
    api_call->input.call_param.create_call.transform = (struct UDP_Transform*)self;
    api_call->input.call_param.create_call.address = *destination;
    api_call->input.call_param.create_call.netmask = *netmask;
    api_call->input.call_param.create_call.user_data = user_data;

    if (property != NULL)
    {
        api_call->input.call_param.create_call.property = *property;
    }

    /* Output */
    api_call->input.call_param.create_call.is_source = is_source;
    *ec = api_call->input.call_param.create_call.ec;
    *context = user_data;
    api_call->output.call_param.create_call.ec = *ec;
    api_call->output.call_param.create_call.context_data = *context;

    return api_call->output.call_param.create_call.retval;
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_create_destination_transform(UDP_Transform_T *const udptf,
                            void **const context,
                            const struct NETIO_Address *const destination,
                            const struct NETIO_Netmask *const netmask,
                            void *user_data,
                            const struct UDP_TransformProperty *const property,
                            RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_create_transform(
            self,context,destination,netmask,user_data,property,ec,RTI_FALSE);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_create_source_transform(UDP_Transform_T *const udptf,
                           void **const context,
                           const struct NETIO_Address *const source,
                           const struct NETIO_Netmask *const netmask,
                           void *user_data,
                           const struct UDP_TransformProperty *const property,
                           RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_create_transform(
            self,context,source,netmask,user_data,property,ec,RTI_TRUE);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_delete_transform(struct MyUdpTransform *const self,
                                void *context,
                                const struct NETIO_Address *const address,
                                const struct NETIO_Netmask *const netmask,
                                RTI_INT32 *const ec,
                                RTI_BOOL is_source)
{
    struct MyUdpTransformContextCalls *call =
                            (struct MyUdpTransformContextCalls *)context;
    struct MyUdpTransformContextCallTest *api_call;
    UNUSED_ARG(address);
    UNUSED_ARG(netmask);
    UNUSED_ARG(ec);
    UNUSED_ARG(is_source);

    if (self->factory->property->ignore_callparam)
    {
        return RTI_TRUE;
    }

    ++call->current_call_index;
    api_call = &call->calls[call->current_call_index];

    api_call->input.call_param.delete_call.address = *address;
    api_call->input.call_param.delete_call.netmask = *netmask;
    *ec = api_call->input.call_param.delete_call.ec;
    api_call->input.call_param.delete_call.is_source = is_source;
    api_call->output.call_param.delete_call.ec = *ec;

    return api_call->output.call_param.delete_call.retval;
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_delete_destination_transform(UDP_Transform_T *const udptf,
                            void *context,
                            const struct NETIO_Address *const destination,
                            const struct NETIO_Netmask *const netmask,
                            RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_delete_transform(
            self,context,destination,netmask,ec,RTI_FALSE);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_delete_source_transform(UDP_Transform_T *const udptf,
                                   void *context,
                                   const struct NETIO_Address *const source,
                                   const struct NETIO_Netmask *const netmask,
                                   RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_delete_transform(
            self,context,source,netmask,ec,RTI_FALSE);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_transform(struct MyUdpTransform *const self,
                         void *context,
                         const struct NETIO_Address *const address,
                         const NETIO_Packet_T *const in_packet,
                         NETIO_Packet_T **out_packet,
                         RTI_INT32 *const ec,
                         RTI_BOOL is_source)
{
    struct MyUdpTransformContextCalls *call =
                            (struct MyUdpTransformContextCalls *)context;
    struct MyUdpTransformContextCallTest *api_call;

    if (self->factory->property->ignore_callparam)
    {
        return RTI_TRUE;
    }

    ++call->current_call_index;
    api_call = &call->calls[call->current_call_index];

    /* Input */
    api_call->input.call_param.transform_call.address = *address;
    api_call->input.call_param.transform_call.in_packet = in_packet;
    api_call->input.call_param.transform_call.out_packet = in_packet;


    /* Output */
    api_call->input.call_param.transform_call.is_source = is_source;
    *ec = api_call->input.call_param.transform_call.ec;
    api_call->output.call_param.transform_call.ec = *ec;

    /* NOTE: This is done for testing purposes only */
    *out_packet = (NETIO_Packet_T *)in_packet;

    return api_call->output.call_param.transform_call.retval;
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_transform_destination(UDP_Transform_T *const udptf,
                                 void *context,
                                 const struct NETIO_Address *const destination,
                                 const NETIO_Packet_T *const in_packet,
                                 NETIO_Packet_T **out_packet,
                                 RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_transform(
                self,context,destination,in_packet,out_packet,ec,RTI_FALSE);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransform_transform_source(UDP_Transform_T *const udptf,
                                void *context,
                                const struct NETIO_Address *const source,
                                const NETIO_Packet_T *const in_packet,
                                NETIO_Packet_T **out_packet,
                                RTI_INT32 *const ec)
{
    struct MyUdpTransform *self = (struct MyUdpTransform*)udptf;

    return MyUdpTransform_transform(
                self,context,source,in_packet,out_packet,ec,RTI_TRUE);
}

RTI_PRIVATE struct UDP_TransformI MyUdpTransform_fv_Intf =
{
    RT_COMPONENTI_BASE,
    MyUdpTransform_create_destination_transform,
    MyUdpTransform_create_source_transform,
    MyUdpTransform_transform_source,
    MyUdpTransform_transform_destination,
    MyUdpTransform_delete_destination_transform,
    MyUdpTransform_delete_source_transform
};

MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
MyUdpTransformFactory_create_component(struct RT_ComponentFactory *factory,
                                struct RT_ComponentProperty *property,
                                struct RT_ComponentListener *listener)
{
    struct MyUdpTransform *t;
    UNUSED_ARG(listener);

    t = MyUdpTransform_create(
                (struct MyUdpTransformFactory*)factory,
                (struct UDP_TransformProperty*)property);

    return &t->_parent._parent;
}

RTI_PRIVATE void
MyUdpTransformFactory_delete_component(struct RT_ComponentFactory *factory,
                                RT_Component_T *component)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(component);

    MyUdpTransform_delete((struct MyUdpTransform*)component);
}

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
MyUdpTransformFactory_initialize(struct RT_ComponentFactoryProperty *property,
                          struct RT_ComponentFactoryListener *listener)
{
    struct MyUdpTransformFactory *fac;
    UNUSED_ARG(listener);

    OSAPI_Heap_allocate_struct(&fac,struct MyUdpTransformFactory);

    if (fac == NULL)
    {
        return NULL;
    }

    fac->_parent._factory = &fac->_parent;
    fac->_parent.intf = &MyUdpTransformFactory_fv_Intf;
    fac->property = (struct MyUdpTransformFactoryProperty*)property;

    return &fac->_parent;
}

RTI_PRIVATE void
MyUdpTransformFactory_finalize(struct RT_ComponentFactory *factory,
                        struct RT_ComponentFactoryProperty **property,
                        struct RT_ComponentFactoryListener **listener)
{
    struct MyUdpTransformFactory *fac = (struct MyUdpTransformFactory*)factory;

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

RTI_PRIVATE struct RT_ComponentFactoryI MyUdpTransformFactory_fv_Intf =
{
    UDP_INTERFACE_INTERFACE_ID,
    MyUdpTransformFactory_initialize,
    MyUdpTransformFactory_finalize,
    MyUdpTransformFactory_create_component,
    MyUdpTransformFactory_delete_component,
    NULL,
    NULL
};

RTI_PRIVATE struct RT_ComponentFactoryI*
MyUdpTransformFactory_get_interface(void)
{
    return &MyUdpTransformFactory_fv_Intf;
}

RTI_PRIVATE RTI_BOOL
MyUdpTransformFactory_register(RT_Registry_T *registry,
                            const char *const name,
                            struct MyUdpTransformFactoryProperty *property)
{
    return RT_Registry_register(registry, name,
                        MyUdpTransformFactory_get_interface(),
                        &property->_parent, NULL);
}

RTI_PRIVATE RTI_BOOL
MyUdpTransformFactory_unregister(RT_Registry_T *registry,
            const char *const name,
            struct MyUdpTransformFactoryProperty **property)
{
    return RT_Registry_unregister(registry, name,
                              (struct RT_ComponentFactoryProperty**)property,
                              NULL);
}

struct UDPTransformTester
{
    RT_Registry_T *rt;
    struct UDP_TransformTable rules;
    struct UDP_TransformRuleSeq dst_rules;
    struct UDP_TransformRuleSeq src_rules;
    DB_Database_T db;
};

RTI_PRIVATE unsigned char
UDPTransformTester_initialize(struct UDPTransformTester *tester,
                              struct UTEST_Context *setting)
{
    struct  RT_RegistryProperty prop = RT_RegistryProperty_INITIALIZER;
    unsigned char retval = RTI_FALSE;
    struct DB_DatabaseProperty dbp = DB_DatabaseProperty_INITIALIZER;
    DB_ReturnCode_T retcode;

    dbp.lock_mode = DB_LOCK_LEVEL_GLOBAL;
    dbp.max_tables = 1;
    tester->db = NULL;

    retcode = DB_Database_create(&tester->db,"trans",&dbp,NULL);
    TEST_ASSERT(retcode == DB_RETCODE_OK,
                "failed to create database",printf("retcode = %d\n",retcode);
                goto done);

    tester->rt = RT_Registry_get_instance();

    TEST_ASSERT(RT_Registry_get_property(tester->rt,&prop),
                "failed to get properties",
                goto done);

    prop.db = tester->db;
    prop.max_factories = 11;

    TEST_ASSERT(RT_Registry_set_property(tester->rt,&prop),
                "failed to set properties",
                goto done);

    TEST_ASSERT(UDP_TransformRuleSeq_initialize(&tester->dst_rules),
                "failed to initialize dst_rules",
                goto done);

    TEST_ASSERT(UDP_TransformRuleSeq_initialize(&tester->src_rules),
                "failed to initialize src_rules",
                goto done);


    retval = RTI_TRUE;

done:
    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_finalize(struct UDPTransformTester *tester,
                            struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;

#ifndef RTI_CERT
    TEST_ASSERT(UDP_TransformRuleSeq_finalize(&tester->dst_rules),
                "failed to finalize dst_rules",
                goto done);

    TEST_ASSERT(UDP_TransformRuleSeq_finalize(&tester->src_rules),
                "failed to finalize src_rules",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(tester->rt),
                "failed to finalize registry",
                goto done);

    TEST_ASSERT(DB_Database_delete(tester->db) == DB_RETCODE_OK,
                "failed to delete database",
                goto done);
#else
    UNUSED_ARG(tester);
    UNUSED_ARG(setting);
#endif

    retval = RTI_TRUE;

#ifndef RTI_CERT
done:
#endif
    return retval;
}

#define MyUdpTransformFactoryProperty_INITIALIZER {\
    RT_ComponentFactoryProperty_INITIALIZER,\
    RTI_TRUE \
}

RTI_PRIVATE unsigned char
UDPTransformTester_basic(struct UTEST_Context *setting)
{
    struct UDPTransformTester tester;
    unsigned char retval = RTI_FALSE;
    struct MyUdpTransformFactoryProperty property =
                MyUdpTransformFactoryProperty_INITIALIZER;
    struct MyUdpTransformFactoryProperty *ptr_property = NULL;
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    transform_property.max_receive_message_size = 1234;
    transform_property.max_send_message_size = 5678;

    /* Test that a setup and shutdown works with no rules added */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    /* Test rules are properly cleaned up when the the rules are finalized */

    /* Assert rules only destination rules  */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T2",&property),
                "failed to register T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T3",&property),
                "failed to register T3",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000101,0xffffff00,
                                                      "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000202,0xffffff00,
                                                      "T2",(void*)0xabcd),
                "failed to add source rule",
                goto done);
    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000303,0xffffff00,
                                                      "T3",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",&ptr_property),
                "failed to unregister T1",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T2",&ptr_property),
                "failed to unregister T2",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T3",&ptr_property),
                "failed to unregister T3",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    /* Assert rules only source rules */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T2",&property),
                "failed to register T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T3",&property),
                "failed to register T3",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f00101,0xffffff00,
                                                      "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000202,0xffffff00,
                                                      "T2",(void*)0xabcd),
                "failed to add source rule",
                goto done);
    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000303,0xffffff00,
                                                      "T3",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",&ptr_property),
                "failed to unregister T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T2",&ptr_property),
                "failed to unregister T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T3",&ptr_property),
                "failed to unregister T3",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    /* Test both rules */

    /* Assert rules only destination rules  */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T2",&property),
                "failed to register T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T3",&property),
                "failed to register T3",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000101,0xffffff00,
                                                      "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000202,0xffffff00,
                                                      "T2",(void*)0xabcd),
                "failed to add source rule",
                goto done);
    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000303,0xffffff00,
                                                      "T3",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000101,0xffffff00,
                                                      "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000202,0xffffff00,
                                                      "T2",(void*)0xabcd),
                "failed to add source rule",
                goto done);
    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000303,0xffffff00,
                                                      "T3",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",&ptr_property),
                "failed to unregister T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T2",&ptr_property),
                "failed to unregister T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T3",&ptr_property),
                "failed to unregister T3",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_bad_rules(struct UTEST_Context *setting)
{
    struct UDPTransformTester tester;
    unsigned char retval = RTI_FALSE;
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    /* Test: UDP_TransformTable_initialize shall fail when a factory
     *       has not been registered.
     */
    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                          0x7f000001,0xffffff00,
                                                          "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(!UDP_TransformTable_initialize(&tester.rules,
                                               tester.rt,
                                               &tester.dst_rules,
                                               &tester.src_rules,
                                               &transform_property),
                "Should have failed without T1",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    /* Test: UDP_TransformTable_initialize shall fail when a factory
     *       has not been registered for a source rule
     */
    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000001,0xffffff00,
                                                      "T1",(void*)0xabcd),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(!UDP_TransformTable_initialize(&tester.rules,
                                               tester.rt,
                                               &tester.dst_rules,
                                               &tester.src_rules,
                                               &transform_property),
                "Should have failed without T1",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    retval = RTI_TRUE;

done:

    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_bad_parameters(struct UTEST_Context *setting)
{
    struct UDPTransformTester tester;
    unsigned char retval = RTI_FALSE;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    /* Test: UDP_TransformTable_initialize shall fail with NULL arguments
     */
    TEST_ASSERT(!UDP_TransformRules_assert_destination_rule(NULL,
                                                          0x7f000001,0xffffff00,
                                                          "T1",(void*)0xabcd),
                "should have failed with NULL sequence",
                goto done);

    /* Test: UDP_TransformTable_initialize shall fail with NULL arguments
     */
    TEST_ASSERT(!UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                          0x7f000001,0xffffff00,
                                                          NULL,(void*)0xabcd),
                "should have failed with NULL sequence",
                goto done);

    /* Test: UDP_TransformTable_initialize shall fail with NULL arguments
     */
    TEST_ASSERT(!UDP_TransformRules_assert_destination_rule(NULL,
                                                          0x7f000001,0xffffff00,
                                                          NULL,(void*)0xabcd),
                "should have failed with NULL sequence",
                goto done);


    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_lookup(struct UTEST_Context *setting)
{
    struct UDPTransformTester tester;
    unsigned char retval = RTI_FALSE;
    struct MyUdpTransformFactoryProperty property =
                MyUdpTransformFactoryProperty_INITIALIZER;
    struct MyUdpTransformFactoryProperty *ptr_property = NULL;
    struct NETIO_Address address;
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    /* Test that a setup and shutdown works with no rules added */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    /* Test rules are properly cleaned up when the the rules are finalized */

    /* Assert rules only destination rules  */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T2",&property),
                "failed to register T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T3",&property),
                "failed to register T3",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000101,0xffffff00,
                                                      "T1",(void*)0x1),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x7f000203,0xffffff00,
                                                      "T3",(void*)0x3),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000202,0xffffff00,
                                                      "T2",(void*)0x2),
                "failed to add destination rule",
                goto done);
    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                      0x7f000303,0xffffff00,
                                                      "T3",(void*)0x3),
                "failed to add destination rule",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    NETIO_Address_set_ipv4(&address,7410,0x7f0001a1);

    TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&address),
                "failed to lookup transform",
                goto done);

    NETIO_Address_set_ipv4(&address,7410,0x7f0002b2);

    TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&address),
                "failed to lookup transform",
                goto done);

    NETIO_Address_set_ipv4(&address,7410,0x7f0003c3);

    TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&address),
                "failed to lookup transform",
                goto done);

    NETIO_Address_set_ipv4(&address,7410,0x7e000004);

    TEST_ASSERT(!UDP_TransformTable_has_transform(&tester.rules,&address),
                "failed to lookup transform",
                goto done);

    NETIO_Address_set_ipv4(&address,7410,0x7f001001);

    TEST_ASSERT(!UDP_TransformTable_has_transform(&tester.rules,&address),
                "failed to lookup transform",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",&ptr_property),
                "failed to unregister T1",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T2",&ptr_property),
                "failed to unregister T2",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T3",&ptr_property),
                "failed to unregister T3",
                goto done);

    TEST_ASSERT(&property == ptr_property,
                "Invalid property returned",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);


    retval = RTI_TRUE;

done:
    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_payload_transform(struct UTEST_Context *setting,
                                     RTI_BOOL assert_source)
{
    struct UDPTransformTester tester;
    unsigned char retval = RTI_FALSE;
    struct MyUdpTransformFactoryProperty property =
                MyUdpTransformFactoryProperty_INITIALIZER;
    struct MyUdpTransformFactoryProperty *ptr_property = NULL;
    struct MyUdpTransformContextCalls T1_calls;
    struct MyUdpTransformContextCalls T2_calls;
    struct MyUdpTransformContextCalls T3_calls;
    struct NETIO_Address srcadr;
    struct NETIO_Packet in_packet = NETIO_Packet_INITIALIZER;
    struct NETIO_Packet *out_packet = NULL;
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;

    OSAPI_Memory_zero(&T1_calls,sizeof(struct MyUdpTransformContextCalls));
    OSAPI_Memory_zero(&T2_calls,sizeof(struct MyUdpTransformContextCalls));
    OSAPI_Memory_zero(&T3_calls,sizeof(struct MyUdpTransformContextCalls));

    T1_calls.current_call_index = -1;
    T2_calls.current_call_index = -1;
    T3_calls.current_call_index = -1;

    property.ignore_callparam = RTI_FALSE;

    /* Test that a setup and shutdown works with no rules added */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    /* Test rules are properly cleaned up when the the rules are finalized */

    /* Assert rules only destination rules  */
    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T2",&property),
                "failed to register T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T3",&property),
                "failed to register T3",
                goto done);

    T1_calls.calls[0].output.call_param.create_call.retval = RTI_TRUE;
    T1_calls.calls[0].input.call_param.create_call.ec = 0xdead;
    T1_calls.calls[0].input.call_param.create_call.context_data = (void*)1;

    if (assert_source)
    {
        TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.src_rules,
                                                               0x7f000001,0xfffffff0,
                                                               "T1",&T1_calls),
                    "failed to add source rule",
                    goto done);
    }
    else
    {
        TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                               0x7f000001,0xfffffff0,
                                                               "T1",&T1_calls),
                    "failed to add source rule",
                    goto done);
    }

    T3_calls.calls[0].output.call_param.create_call.retval = RTI_TRUE;
    T3_calls.calls[0].input.call_param.create_call.ec = 0xacdc;
    T3_calls.calls[0].input.call_param.create_call.context_data = (void*)3;

    if (assert_source)
    {
        TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.src_rules,
                                                               0x7f000101,0xffffff00,
                                                               "T3",&T3_calls),
                    "failed to add source rule",
                    goto done);
    }
    else
    {
        TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                               0x7f000101,0xffffff00,
                                                               "T3",&T3_calls),
                    "failed to add source rule",
                    goto done);
    }

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    /* Check that transformations are called properly */

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.address.value.ipv4.address == 0x7f000001,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.netmask.bits == 32,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.netmask.mask[0] == 0xfffffff0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.netmask.mask[1] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.netmask.mask[2] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.user_data == &T1_calls,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.address.value.ipv4.address == 0x7f000101,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.netmask.bits == 32,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.netmask.mask[0] == 0xffffff00,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.netmask.mask[1] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.netmask.mask[2] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.user_data == &T3_calls,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.create_call.is_source == assert_source,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].input.call_param.create_call.is_source == assert_source,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].output.call_param.create_call.ec == 0xdead,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].output.call_param.create_call.ec == 0xacdc,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].output.call_param.create_call.context_data == &T1_calls,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[0].output.call_param.create_call.context_data == &T3_calls,
                "asserted destination rule not equal",
                goto done);


    /* Transform outgoing packet */

    NETIO_Address_set_ipv4(&srcadr,0,0x7f000007);

    T1_calls.calls[1].output.call_param.transform_call.retval = RTI_TRUE;
    T1_calls.calls[1].input.call_param.transform_call.ec = 5;

    if (assert_source)
    {
        TEST_ASSERT(UDP_TransformTable_transform_incoming(&tester.rules,
                                                          &srcadr,
                                                          &in_packet,
                                                          &out_packet),
                    "failed to transform incoming packet",
                    goto done);
    }
    else
    {
        TEST_ASSERT(UDP_TransformTable_transform_outgoing(&tester.rules,
                                                          &srcadr,
                                                          &in_packet,
                                                          &out_packet),
                    "failed to transform incoming packet",
                    goto done);
    }

    NETIO_Address_set_ipv4(&srcadr,0,0x7f000107);
    T3_calls.calls[1].output.call_param.transform_call.retval = RTI_TRUE;
    T3_calls.calls[1].input.call_param.transform_call.ec = 6;

    if (assert_source)
    {
        TEST_ASSERT(UDP_TransformTable_transform_incoming(&tester.rules,
                                                          &srcadr,
                                                          &in_packet,
                                                          &out_packet),
                    "failed to transform incoming packet",
                    goto done);
    }
    else
    {
        TEST_ASSERT(UDP_TransformTable_transform_outgoing(&tester.rules,
                                                          &srcadr,
                                                          &in_packet,
                                                          &out_packet),
                    "failed to transform incoming packet",
                    goto done);
    }

    TEST_ASSERT(T1_calls.calls[1].input.call_param.transform_call.address.value.ipv4.address == 0x7f000007,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[1].input.call_param.transform_call.in_packet == &in_packet,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[1].input.call_param.transform_call.out_packet == &in_packet,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[1].input.call_param.create_call.is_source == assert_source,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[1].input.call_param.transform_call.address.value.ipv4.address == 0x7f000107,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[1].input.call_param.transform_call.in_packet == &in_packet,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[1].input.call_param.transform_call.out_packet == &in_packet,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[1].input.call_param.create_call.is_source == assert_source,
                "asserted destination rule not equal",
                goto done);

    T1_calls.calls[2].output.call_param.delete_call.retval = RTI_TRUE;
    T3_calls.calls[2].output.call_param.delete_call.retval = RTI_TRUE;
    T1_calls.calls[2].input.call_param.delete_call.ec = 10;
    T3_calls.calls[2].input.call_param.delete_call.ec = 20;


    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.address.value.ipv4.address == 0x7f000001,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.bits == 32,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.mask[0] == 0xfffffff0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.mask[1] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.mask[2] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[0].input.call_param.delete_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T1_calls.calls[2].output.call_param.delete_call.ec == 10,
                "asserted destination rule not equal",
                goto done);

    /* T3 */
    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.address.value.ipv4.address == 0x7f000101,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.bits == 32,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.mask[0] == 0xffffff00,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.mask[1] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.mask[2] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].input.call_param.delete_call.netmask.mask[3] == 0,
                "asserted destination rule not equal",
                goto done);

    TEST_ASSERT(T3_calls.calls[2].output.call_param.delete_call.ec == 20,
                "asserted destination rule not equal",
                goto done);


    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",&ptr_property),
                "failed to unregister T1",
                goto done);
    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T2",&ptr_property),
                "failed to unregister T2",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T3",&ptr_property),
                "failed to unregister T3",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_PRIVATE unsigned char
UDPTransformTester_outgoing(struct UTEST_Context *setting)
{
    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    return UDPTransformTester_payload_transform(setting,RTI_FALSE);
}

RTI_PRIVATE unsigned char
UDPTransformTester_incoming(struct UTEST_Context *setting)
{
    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    return UDPTransformTester_payload_transform(setting,RTI_TRUE);
}

/* Test template */

struct UDPTransformTesterAddress
{
    RTI_UINT32 address;
    RTI_UINT32 port;
    RTI_BOOL match_expected;
    void *match_context;
};

/* Test rules for 0x5555,0x5550,0x5500,0x5000,0x4000,0x3000,0x2000,0x1000 */

RTI_PRIVATE struct UDPTransformTesterAddress UDPTransformTesterAddress_fv_DstAddressList[]=
{
    {0x10000,100,RTI_TRUE,(void*)8},
    {0x1abcd,10,RTI_TRUE,(void*)8},
    {0x1ffff,5,RTI_TRUE,(void*)8},

    {0x20000,100,RTI_TRUE,(void*)7},
    {0x2abcd,10,RTI_TRUE,(void*)7},
    {0x2ffff,5,RTI_TRUE,(void*)7},

    {0x30000,100,RTI_TRUE,(void*)6},
    {0x3abcd,10,RTI_TRUE,(void*)6},
    {0x3ffff,5,RTI_TRUE,(void*)6},

    {0x40000,100,RTI_TRUE,(void*)5},
    {0x4abcd,10,RTI_TRUE,(void*)5},
    {0x4ffff,5,RTI_TRUE,(void*)5},

    {0x50000,100,RTI_TRUE,(void*)1},
    {0x5ffff,5,RTI_TRUE,(void*)1},

    {0x5abcd,10,RTI_TRUE,(void*)1},
    {0x55555,100,RTI_TRUE,(void*)4},
    {0x55550,100,RTI_TRUE,(void*)4},
    {0x55500,100,RTI_TRUE,(void*)3},
    {0x55000,5,RTI_TRUE,(void*)2},

    {0x0ffff,10,RTI_FALSE,NULL},
    {0x60000,100,RTI_FALSE,NULL}
};

RTI_PRIVATE struct UDPTransformTesterAddress UDPTransformTesterAddress_fv_SrcAddressList[]=
{
    {0x110000,100,RTI_TRUE,(void*)18},
    {0x11abcd,10,RTI_TRUE,(void*)18},
    {0x11ffff,5,RTI_TRUE,(void*)18},

    {0x120000,100,RTI_TRUE,(void*)17},
    {0x12abcd,10,RTI_TRUE,(void*)17},
    {0x12ffff,5,RTI_TRUE,(void*)17},

    {0x130000,100,RTI_TRUE,(void*)16},
    {0x13abcd,10,RTI_TRUE,(void*)16},
    {0x13ffff,5,RTI_TRUE,(void*)16},

    {0x140000,100,RTI_TRUE,(void*)15},
    {0x14abcd,10,RTI_TRUE,(void*)15},
    {0x14ffff,5,RTI_TRUE,(void*)15},

    {0x150000,100,RTI_TRUE,(void*)11},
    {0x15ffff,5,RTI_TRUE,(void*)11},

    {0x15abcd,10,RTI_TRUE,(void*)11},
    {0x155555,100,RTI_TRUE,(void*)14},
    {0x155550,100,RTI_TRUE,(void*)14},
    {0x155500,100,RTI_TRUE,(void*)13},
    {0x155000,5,RTI_TRUE,(void*)12},

    {0x0ffff,10,RTI_FALSE,NULL},
    {0x60000,100,RTI_FALSE,NULL}
};

RTI_PRIVATE unsigned char
UDPTransformTester_matching(struct UTEST_Context *setting)
{
    struct UDPTransformTester tester;
    struct MyUdpTransformFactoryProperty property =
                                    MyUdpTransformFactoryProperty_INITIALIZER;
    unsigned char retval = RTI_FALSE;
    struct NETIO_Address an_address;
    RTI_INT32 c;
    RTI_INT32 address_count = sizeof(UDPTransformTesterAddress_fv_DstAddressList)/
                              sizeof(struct UDPTransformTesterAddress);
    RTI_BOOL result;
    void *context;
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(UDPTransformTester_initialize(&tester,setting),
                "failed to initialize tester",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_register(tester.rt,"T1",&property),
                "failed to register T1",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x50000,0xffff0000,
                                                           "T1",(void*)1),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x55001,0xfffff000,
                                                           "T1",(void*)2),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x55500,0xffffff00,
                                                           "T1",(void*)3),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x55550,0xfffffff0,
                                                           "T1",(void*)4),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x40000,0xffff0000,
                                                           "T1",(void*)5),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x30000,0xffff0000,
                                                           "T1",(void*)6),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x20000,0xffff0000,
                                                           "T1",(void*)7),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_destination_rule(&tester.dst_rules,
                                                           0x10000,0xffff0000,
                                                           "T1",(void*)8),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                      0x150000,0xffff0000,
                                                      "T1",(void*)11),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x155001,0xfffff000,
                                                           "T1",(void*)12),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x155500,0xffffff00,
                                                           "T1",(void*)13),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x155550,0xfffffff0,
                                                           "T1",(void*)14),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x140000,0xffff0000,
                                                           "T1",(void*)15),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x130000,0xffff0000,
                                                           "T1",(void*)16),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x120000,0xffff0000,
                                                           "T1",(void*)17),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformRules_assert_source_rule(&tester.src_rules,
                                                           0x110000,0xffff0000,
                                                           "T1",(void*)18),
                "failed to add source rule",
                goto done);

    TEST_ASSERT(UDP_TransformTable_initialize(&tester.rules,
                                              tester.rt,
                                              &tester.dst_rules,
                                              &tester.src_rules,
                                              &transform_property),
                "failed to initialize rules",
                goto done);

    for (c = 0; c < address_count; ++c)
    {
        NETIO_Address_set_ipv4(&an_address,
                           UDPTransformTesterAddress_fv_DstAddressList[c].port,
                           UDPTransformTesterAddress_fv_DstAddressList[c].address);

        TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&an_address) ==
                    UDPTransformTesterAddress_fv_DstAddressList[c].match_expected,
                    "matching failed",
                    printf("%x\n",UDPTransformTesterAddress_fv_DstAddressList[c].address);
                    goto done);

        TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&an_address) ==
                    UDPTransformTesterAddress_fv_DstAddressList[c].match_expected,
                    "matching failed",
                    printf("%x\n",UDPTransformTesterAddress_fv_DstAddressList[c].address);
                    goto done);

        result = UDP_TransformTable_find_destination_context(&tester.rules,&an_address,
                                                        &context);

        TEST_ASSERT(result == UDPTransformTesterAddress_fv_DstAddressList[c].match_expected,
                    "find_transform_context failed",
                    goto done);

        if (result)
        {
            TEST_ASSERT(context == UDPTransformTesterAddress_fv_DstAddressList[c].match_context,
                        "invalid match context",
                        goto done);
        }

        result = UDP_TransformTable_find_source_context(&tester.rules,&an_address,
                                                        &context);
        TEST_ASSERT(!result,
                    "found destination rule in source context",
                    goto done);
    }

    for (c = 0; c < address_count; ++c)
    {
        NETIO_Address_set_ipv4(&an_address,
                           UDPTransformTesterAddress_fv_SrcAddressList[c].port,
                           UDPTransformTesterAddress_fv_SrcAddressList[c].address);

        TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&an_address) ==
                UDPTransformTesterAddress_fv_SrcAddressList[c].match_expected,
                    "matching failed",
                    printf("%x\n",UDPTransformTesterAddress_fv_SrcAddressList[c].address);
                    goto done);

        TEST_ASSERT(UDP_TransformTable_has_transform(&tester.rules,&an_address) ==
                UDPTransformTesterAddress_fv_SrcAddressList[c].match_expected,
                    "matching failed",
                    printf("%x\n",UDPTransformTesterAddress_fv_SrcAddressList[c].address);
                    goto done);

        result = UDP_TransformTable_find_source_context(&tester.rules,&an_address,
                                                        &context);

        TEST_ASSERT(result == UDPTransformTesterAddress_fv_SrcAddressList[c].match_expected,
                    "find_transform_context failed",
                    printf("c = %d\n",c);
                    goto done);

        if (result)
        {
            TEST_ASSERT(context == UDPTransformTesterAddress_fv_SrcAddressList[c].match_context,
                        "invalid match context",
                        goto done);
        }

        result = UDP_TransformTable_find_destination_context(&tester.rules,&an_address,
                                                             &context);
        TEST_ASSERT(!result,
                    "found destination rule in source context",
                    goto done);
    }

    TEST_ASSERT(UDP_TransformTable_finalize(&tester.rules),
                "failed to finalize rules",
                goto done);

    TEST_ASSERT(MyUdpTransformFactory_unregister(tester.rt,"T1",NULL),
                "failed to unregister T1",
                goto done);

    TEST_ASSERT(UDPTransformTester_finalize(&tester,setting),
                "failed to finalize tester",
                goto done);

    retval = RTI_TRUE;

done:

    return retval;
}

#if 0
RTI_PRIVATE unsigned char
UDPTransformTester_foo(struct UTEST_Context *setting)
{
    RTI_BOOL retval = RTI_FALSE;

    CHECK_DO_RUN_TEST(setting);

    retval = RTI_TRUE;

done:
    return retval;
}
#endif

RTI_PRIVATE struct UTEST_TestEntry UDPTransformTester_tests[]=
{
        RTITestCase("basic",
                    UDPTransformTester_basic,
                    TEST_ENABLED),

        RTITestCase("bad_rules",
                    UDPTransformTester_bad_rules,
                    TEST_ENABLED),

        RTITestCase("bad_param",
                    UDPTransformTester_bad_parameters,
                    TEST_ENABLED),

        RTITestCase("lookup",
                    UDPTransformTester_lookup,
                    TEST_ENABLED),

        RTITestCase("outgoing",
                    UDPTransformTester_outgoing,
                    TEST_ENABLED),

        RTITestCase("incoming",
                    UDPTransformTester_incoming,
                    TEST_ENABLED),

        RTITestCase("matching",
                    UDPTransformTester_matching,
                    TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(UDPTransformTester,"udp/transform")

#endif
