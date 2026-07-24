/*
 * FILE: netio_zcopy_whsq.h - Shared Queue Writer History
 *
 * (c) Copyright, Real-Time Innovations, 2023.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Shared Queue Writer History includes. 
 */
/*ci \addtogroup WHSQModule
 * @{
 */

#include "netio_zcopy/netio_zcopy_dll.h"
#include "dds_c/dds_c_wh_plugin.h"

#ifndef netio_zcopy_whsq_h
#define netio_zcopy_whsq_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*ci
 * \brief WHSQ History properties
 */
struct WHSQ_HistoryProperty
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct DDSHST_WriterProperty _parent;
};

/*ci
 * \def WHSQ_HistoryProperty_INITIALIZER
 * \brief Constant to initialize \ref WHSQ_HistoryProperty
 */
#define WHSQ_HistoryProperty_INITIALIZER  {\
    DDSHST_WriterProperty_INITIALIZER\
}

/*ci
 * \def WHSQ_HISTORY_INTERFACE_ID
 * \brief The WHSQ interface id
 */
#define RT_COMPONENT_INSTANCE_WHISTORY_SQ (RT_COMPONENT_INSTANCE_WHISTORY_SM + 1)

#define WHSQ_HISTORY_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_WHISTORY,RT_COMPONENT_INSTANCE_WHISTORY_SQ)


/*ci
 * \brief Properties for the WHSQ factory. 
 */
struct WHSQ_HistoryFactoryProperty
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct DDSHST_WriterFactoryProperty _parent;

    /*ci
     * \brief Name of the factory for getting the original writer history component 
     */ 
    const char *dds_wh_factory_name;
};

/*ci
 * \def WHSQ_HistoryFactoryProperty_INITIALIZER
 * \brief Constant to initialize \ref WHSQ_HistoryFactoryProperty
 */
#define WHSQ_HistoryFactoryProperty_INITIALIZER \
{\
    DDSHST_WriterFactoryProperty_INITIALIZER, /*_parent*/\
    NULL /*dds_wh_factory_name*/\
}

/*ce
 * \brief Function to retrieve the concrete implementation 
 *        of the WHSQ interface factory
 *
 * \return Pointer to RT Component factory implementation
 */
NETIO_ZCOPYDllExport struct RT_ComponentFactoryI*
WHSQ_HistoryFactory_get_interface(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*netio_zcopy_whsq_h*/

/*ci @} */
