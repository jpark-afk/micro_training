/*
 * FILE: osapi_types.h - Definition of primitive data-types
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc.
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
 * 05Jun2012,kaj Add RTI_INT64, RTI_UINT64, RTI_DOUBLE128
 * 29feb2012,tk  Written
 */

#ifndef osapi_types_h
#define osapi_types_h

/*e \dref_OSAPITypeGroupDocs
 */

/*ci
 * \file 
 * \brief Primitive data type definitions
 */

/*i \defgroup OSAPITypeClass OSAPI Types
 * \ingroup OSAPIModule
 */

/*i \ingroup OSAPITypeClass

    \brief Defines platform independent types
*/

#if DOXYGEN_DOCUMENTATION_ONLY

/*e \dref_RTI_INT8
 */
#define RTI_INT8

/*e \dref_RTI_UINT8
 */
#define RTI_UINT8

/*e \dref_RTI_INT16
 */
#define RTI_INT16

/*e \dref_RTI_UINT16
 */
#define RTI_UINT16

/*e \dref_RTI_INT32
 */
#define RTI_INT32

/*e \dref_RTI_UINT32
 */
#define RTI_UINT32

/*e \dref_RTI_INT64
 */
#define RTI_INT64

/*e \dref_RTI_UINT64
 */
#define RTI_UINT64

/*e \dref_RTI_BOOL
 */
typedef RTI_INT32 RTI_BOOL;

/*e \dref_RTI_FALSE
 */
#define RTI_FALSE   ((RTI_BOOL) 0)

/*e \dref_RTI_TRUE
 */
#define RTI_TRUE    ((RTI_BOOL) 1)

/*e \dref_RTI_SIZE_T
 */
typedef RTI_UINT32 RTI_SIZE_T;

#endif

#include "osapi_config.h"

#endif
