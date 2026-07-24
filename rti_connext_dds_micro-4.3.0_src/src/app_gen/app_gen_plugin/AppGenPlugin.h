/*
 * FILE: AppGenPlugin.h - DDS Application Generator plugin interface
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
/*ce
 * \file
 * \brief Micro Application Generator implementation
 */
/*ci \addtogroup AppGenModule
 * @{
 */
#ifndef AppGenPlugin_h
#define AppGenPlugin_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif

#ifndef appgen_h
#include "app_gen/app_gen.h"
#endif

#ifndef appgen_plugin_h
#include "app_gen/app_gen_plugin.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*ci
 * \brief Base-class for application generation factories
 */
struct APPGEN_Factory
{
    /*ci
     * \brief Inherit from RT base-class
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief Pointer to properties for the factory. This pointer must be
     *        valid for as long as the factory is registered
     */
    const struct APPGEN_LibraryModelSeq *_model;

    /*ci
     * \brief RTI_TRUE if the reader history plugin have been
     * registered when the first domain participant is created. If that is
     * the case it will be unregistered when the plug-in is unregistered.
     */
    RTI_BOOL registered_reader_history;

    /*ci
     * \brief RTI_TRUE if the writer history plugin have been
     * registered when the first domain participant is created. If that is
     * the case it will be unregistered when the plug-in is unregistered.
     */
    RTI_BOOL registered_writer_history;

    /*ci
     * \brief Number of participants created
     */
    RTI_UINT32 number_participants;
};

/*ci
 * \brief Base-class for application generation plugin
 */
struct APPGEN_Plugin
{
    /*ci
     * \brief Inherit from Application Generation plugin base-class
     */
    struct DDS_AppGenPlugin _parent;

    /*ci
     * \brief Properties for the plugin. 
     */
    struct DDS_AppGen_ComponentProperty property;

    /*ci
     * \brief The factory that created the App Generation
     */
    struct APPGEN_Factory *factory;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AppGenPlugin_h */

/*ci @} */
