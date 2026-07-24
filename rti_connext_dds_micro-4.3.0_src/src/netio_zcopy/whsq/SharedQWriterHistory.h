/*
 * FILE: SharedQWriterHistory.h - Shared Queue Writer History implementation
 *
 * Copyright 2022-2023 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */


#include "dds_c/dds_c_wh_plugin.h"
#include "netio_zcopy/netio_zcopy_whsq.h"
#include "netio_zcopy/netio_zcopy_whsq_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "netio_zcopy/netio_zcopy_sharedq.h"
#include "netio_zcopy/netio_zcopy_plugin.h"

#ifndef SharedQWH_h
#define SharedQWH_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct WHSQ_History 
{

    /*ci
     * \brief Inherited from the base-class
     */
    struct DDSHST_Writer _parent;

    /*ci
     * \brief The properties the cache was created with
     */
    struct WHSQ_HistoryProperty property;

    /*ci
     * \brief The listener the cache was created with
     */
    struct DDSHST_WriterListener listener_orig;

    /*ci
     * \brief The DDS WH that invocations have to be forwarded to
     */
    struct DDSHST_Writer *dds_wh;

    /*ci
     * \brief 
     * Structure to save sample info provided by the writer
     * \details 
     * We intercept the get entry call sample and save the
     * sample info. This is used when commiting the sample.
     * We assume only one outstanding entry is available for commiting. 
     * It is the callers responsibility to always have only one outstanding entry. 
     */
    struct SQ_SampleInfo intercepted_sample_info;

   /*ci
    * \brief Reference to managed pool for easy access. 
    */
    void* sq_writer;
};

struct WHSQ_HistoryFactory
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief Factory for the WH this one is inserting itself around 
     */
    struct RT_ComponentFactory* dds_wh_factory;

};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*SharedQWH_h*/
