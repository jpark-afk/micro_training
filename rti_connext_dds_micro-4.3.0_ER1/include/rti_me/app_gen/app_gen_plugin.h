/*
 * FILE: appgen_plugin.h - Application Generator plugin interface
 *
 * (c) Copyright, Real-Time Innovations, 2017-2018.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ci
 * \file
 * \brief Application Generator plugin interface
 */
/*ci \defgroup AppGenModule Application Generation API
 *   \ingroup DDSAPPGENModule
 */
/*e \addtogroup DDSAPPGENModule
 * @{
 */
#ifndef appgen_plugin_h
#define appgen_plugin_h

#ifndef appgen_h
#include "app_gen/app_gen.h"
#endif

#ifndef appgen_dll_h
#include "app_gen/app_gen_dll.h"
#endif

#ifndef dds_c_profile_plugin_h
#include "dds_c/dds_c_profile_plugin.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*i \dref_AppGen_FactoryProperty
 */
struct APPGEN_FactoryProperty
{
    /*ci
     * \brief Inherit from a component factory property
     */
    struct RT_ComponentFactoryProperty _parent;

    /*ci
     * \brief Pointer to a sequence with all library models
     */
    const struct APPGEN_LibraryModelSeq *_model;
};

/*i \dref_AppGenFactoryProperty_INITIALIZER
 */
#define APPGEN_FactoryProperty_INITIALIZER \
{\
  RT_ComponentFactoryProperty_INITIALIZER,\
  NULL\
}

#define APPGEN_INTERFACE_ID RT_MKINTERFACEID( \
                   RT_COMPONENT_CLASS_APPGEN,\
                   RT_COMPONENT_INSTANCE_APPGEN)

/*e \dref_AppGen_ComponentFactory_get_interface
 */
MUST_CHECK_RETURN APPGENDllExport struct RT_ComponentFactoryI*
APPGEN_Factory_get_interface(void);

/*e \dref_AppGen_ComponentFactory_register
 */
APPGENDllExport RTI_BOOL
APPGEN_Factory_register(RT_Registry_T *registry,
                        struct APPGEN_FactoryProperty *property);

/*e \dref_AppGen_ComponentFactory_unregister
 */
APPGENDllExport RTI_BOOL
APPGEN_Factory_unregister(RT_Registry_T *registry,
                          struct APPGEN_FactoryProperty **property);

/*i \dref_AppGen_ComponentFactory_get_version
 */
#ifndef RTI_CERT
APPGENDllExport const char*
APPGEN_Factory_get_version(void);
#endif

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* appgen_plugin_h */

/*e @} */


