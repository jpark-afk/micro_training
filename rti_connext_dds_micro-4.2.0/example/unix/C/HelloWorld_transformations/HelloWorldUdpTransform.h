/*********************************************************************************************
Copyright (c) 2017-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#ifndef HelloWorldUdpTransform_h
#define HelloWorldUdpTransform_h

#include "rti_me_c.h"
#include "netio/netio_udp.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct HelloWorldUdpTransformFactoryProperty;

/*ci
 * \brief Initialize a HelloWorld Udp Transform factory property
 *
 * \param[in] p Property to initialize
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref HelloWorldUdpTransformFactoryProperty_finalize
 *
 */
RTI_BOOL
HelloWorldUdpTransformFactoryProperty_initialize
                             (struct HelloWorldUdpTransformFactoryProperty *p);

/*ci
 * \brief Finalize a HelloWorld Udp Transform factory property
 *
 * \param[in] p Property to finalize
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref HelloWorldUdpTransformFactoryProperty_initialize
 *
 */
RTI_BOOL
HelloWorldUdpTransformFactoryProperty_finalize
                             (struct HelloWorldUdpTransformFactoryProperty *p);

#ifdef __cplusplus
}
#endif /* __cplusplus */


struct HelloWorldUdpTransformFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;

#ifdef __cplusplus
    public:
        HelloWorldUdpTransformFactoryProperty()
        {
            HelloWorldUdpTransformFactoryProperty_initialize(this);
        }
#endif
};

/*e \dref_HelloWorldUdpTransformFactoryProperty_INITIALIZER
 */
#define HelloWorldUdpTransformFactoryProperty_INITIALIZER \
{\
    RT_ComponentFactoryProperty_INITIALIZER, \
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern struct HelloWorldUdpTransformFactoryProperty
                             HELLOWORLD_UDP_TRANSFORM_FACTORY_PROPERTY_DEFAULT;

extern struct RT_ComponentFactoryI*
HelloWorldUdpTransformFactory_get_interface(void);

/*ci
 * \brief Registers a transformation
 *
 * \param[in] registry Pointer to registry
 * \param[in] name Transformation name
 * \param[in] property Transformation properties
 *
 * \return RTI_TRUE if transformation was registered or RTI_FALSE
 *         in case of error
 */
extern RTI_BOOL
HelloWorldUdpTransformFactory_register(RT_Registry_T *registry,
                        const char *const name,
                        struct HelloWorldUdpTransformFactoryProperty *property);

/*ci
 * \brief Unregisters a transformation
 *
 * \param[in] registry Pointer to registry
 * \param[in] name Transformation name
 * \param[in] property Pointer to transformation properties. The
 *            properties pointer pass as parameter to function
 *            \ref HelloWorldUdpTransformFactory_register will be copied here.
 *            In case a NULL pointer is used, nothing will be copied.
 *
 * \return RTI_TRUE if transformation was unregistered or RTI_FALSE
 *         in case of error
 */
extern RTI_BOOL
HelloWorldUdpTransformFactory_unregister(RT_Registry_T *registry,
                               const char *const name,
                               struct HelloWorldUdpTransformFactoryProperty **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HelloWorldUdpTransform_h */

