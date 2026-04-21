/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#ifndef PerformanceUdpTransform_h
#define PerformanceUdpTransform_h

#include "rti_me_c.h"
#include "netio/netio_udp.h"

#ifdef UDP_TRANSFORMS_ENABLED
/* set next definition to 0 to run performance tests with transforms enabled 
 * but not used in the performance application
 */
#define PERF_TRANSFORMS_ENABLED (1)
#else
#define PERF_TRANSFORMS_ENABLED (0)
#endif /* UDP_TRANSFORMS_ENABLED */

#if PERF_TRANSFORMS_ENABLED

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct PerformanceUdpTransformFactoryProperty;

/*ci
 * \brief Initialize a Performance Udp Transform factory property
 *
 * \param[in] p Property to initialize
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref PerformanceUdpTransformFactoryProperty_finalize
 *
 */
RTI_BOOL
PerformanceUdpTransformFactoryProperty_initialize
                             (struct PerformanceUdpTransformFactoryProperty *p);

/*ci
 * \brief Finalize a Performance Udp Transform factory property
 *
 * \param[in] p Property to finalize
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref PerformanceUdpTransformFactoryProperty_initialize
 *
 */
RTI_BOOL
PerformanceUdpTransformFactoryProperty_finalize
                             (struct PerformanceUdpTransformFactoryProperty *p);

#ifdef __cplusplus
}
#endif /* __cplusplus */


struct PerformanceUdpTransformFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;

#ifdef __cplusplus
    public:
        PerformanceUdpTransformFactoryProperty()
        {
            PerformanceUdpTransformFactoryProperty_initialize(this);
        }
#endif
};

/*e \dref_PerformanceUdpTransformFactoryProperty_INITIALIZER
 */
#define PerformanceUdpTransformFactoryProperty_INITIALIZER \
{\
    RT_ComponentFactoryProperty_INITIALIZER, \
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern struct PerformanceUdpTransformFactoryProperty 
                             Performance_UDP_TRANSFORM_FACTORY_PROPERTY_DEFAULT;

extern struct RT_ComponentFactoryI*
PerformanceUdpTransformFactory_get_interface(void);

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
PerformanceUdpTransformFactory_register(RT_Registry_T *registry,
                        const char *const name,
                        struct PerformanceUdpTransformFactoryProperty *property);

/*ci
 * \brief Unregisters a transformation
 *
 * \param[in] registry Pointer to registry
 * \param[in] name Transformation name
 * \param[in] property Pointer to transformation properties. The
 *            properties pointer pass as parameter to function 
 *            \ref PerformanceUdpTransformFactory_register will be copied here.
 *            In case a NULL pointer is used, nothing will be copied.
 *
 * \return RTI_TRUE if transformation was unregistered or RTI_FALSE 
 *         in case of error
 */
extern RTI_BOOL
PerformanceUdpTransformFactory_unregister(RT_Registry_T *registry,
                               const char *const name,
                               struct PerformanceUdpTransformFactoryProperty **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PERF_TRANSFORMS_ENABLED */

#endif /* PerformanceUdpTransform_h */

