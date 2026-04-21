/*
 * FILE: Type.h - DDS Type implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   NDDS_TypePlugin_is_valid
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 11jun2013,tk MICRO-633: max type_length is 255 excluding NUL
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 *
 */
/*ce
 * \file
 * \brief DDS Type implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef Type_h
#define Type_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

/*ci
 * \brief Implementation of the DDS Type
 */
struct DDS_TypeImpl
{
    /*ci
     * \brief The type-name, only allocate what is needed
     */
    char *name;

    /*ci
     * \brief Pointer to the type-plugin interface
     */
    struct NDDS_Type_Plugin *plugin;

    /*ci
     * \brief Counter to keep track of many how many topics reference this type.
     *        A type can only be unregistered if there are 0 references to it
     */
    DDS_Long topic_count;

    /*ci
     * \brief Counter to keep track of many times a type has been registered.
     *        A type must be unregistered as many times as it has been
     *        registered.
     */
    DDS_Long ref_count;
};

MUST_CHECK_RETURN extern RTI_INT32
DDS_TypeImpl_compare(RTI_INT32 flags,
                    const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_initalize(struct DDS_TypeImpl *type,
                        const char *type_name,
                        struct NDDS_Type_Plugin *plugin);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_TypeImpl_finalize(struct DDS_TypeImpl *type);

SHOULD_CHECK_RETURN extern RTI_BOOL
DDS_Type_dtor(DB_Record_T type_record);

#endif /* !RTI_CERT */

extern void
DDS_TypeImpl_attach_topic(struct DDS_TypeImpl *type);

#ifndef RTI_CERT
extern void
DDS_TypeImpl_detach_topic(struct DDS_TypeImpl *type);
#endif

#ifndef RTI_CERT
extern DDS_Boolean
DDS_TypeImpl_is_attached(struct DDS_TypeImpl *type);
#endif

extern void
DDS_TypeImpl_reference(struct DDS_TypeImpl *type);

#ifndef RTI_CERT
extern void
DDS_TypeImpl_dereference(struct DDS_TypeImpl *type);
#endif

#ifndef RTI_CERT
extern DDS_Boolean
DDS_TypeImpl_is_referenced(struct DDS_TypeImpl *type);
#endif

#ifndef RTI_CERT
extern void
DDS_TypeImpl_reset_reference(struct DDS_TypeImpl *type);
#endif

MUST_CHECK_RETURN struct NDDS_Type_Plugin*
DDS_TypeImpl_get_plugin(DDS_Type *self);

MUST_CHECK_RETURN extern const char*
DDS_TypeImpl_get_type_name_reference(DDS_Type *self);

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_Boolean
NDDS_TypePlugin_is_valid(struct NDDS_Type_Plugin *type_plugin);
#endif

#endif

/*ci @} */
