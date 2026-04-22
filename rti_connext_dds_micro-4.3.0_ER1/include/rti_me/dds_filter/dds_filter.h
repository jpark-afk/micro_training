/*
 * FILE: dds_filter.h - DDS Filter definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief DDS Filter definitions
 */

#ifndef dds_filter_h
#define dds_filter_h

#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif
#ifndef dds_c_content_filter_h
#include "dds_c/dds_c_content_filter.h"
#endif
#ifndef dds_filter_dll_h
#include "dds_filter/dds_filter_dll.h"
#endif
#ifndef dds_filter_log_h
#include "dds_filter/dds_filter_log.h"
#endif

/*e \dref_DDSFILTERGroupDocs
 */

/*ci \addtogroup DDSFILTERModule
 * @{
 */

#ifndef DDS_ENABLE_SQL_FILTER
#if DDS_FILTERING_ENABLED && DDS_XTYPES_IS_ENABLED
#define DDS_ENABLE_SQL_FILTER 1
#else
#define DDS_ENABLE_SQL_FILTER 0
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_SQL_CONTENT_FILTER_CLASS
 */
#define DDS_SQL_CONTENT_FILTER_CLASS "DDSSQL"

/*e \dref_FilterPluginFactory_register
 */
DDS_FILTER_DllExport DDS_ReturnCode_t
DDS_FilterPluginFactory_register(void);

#ifndef RTI_CERT
/*e \dref_FilterPluginFactory_unregister
 */
DDS_FILTER_DllExport DDS_ReturnCode_t
DDS_FilterPluginFactory_unregister(void);
#endif

/*ci \brief Get the interface of the filter plugin factory
 *
 * \details This is an internal functions which is exposed only to allow
 *          MAG to register the filter plugin factory. Users are intended
 *          to use the DDS_FilterPluginFactory_register() function instead.
 */
DDS_FILTER_DllExport struct RT_ComponentFactoryI *
DDS_FilterPluginFactory_get_interface(void);

#if DDS_ENABLE_SQL_FILTER
/*ci \brief Get the interface of the SQL content filter
 *
 * \details This is an internal functions which is exposed only to allow
 *          testing to directly access the SQL content filter interface.
 */
DDS_FILTER_DllExport const struct DDS_ContentFilterI *
DDS_SqlContentFilter_get_interface(void);
#endif

#ifndef RTI_CERT
DDS_FILTER_DllExport const char*
DDS_FILTER_get_version(void);
#endif /* !RTI_CERT */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*ci @} */

#endif /* dds_filter_h */
