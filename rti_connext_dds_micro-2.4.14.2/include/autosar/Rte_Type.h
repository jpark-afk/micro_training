/*
 * FILE: Rte_Type.h - AutoSAR 4.2.2 Rte types
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * AUTOSAR_SWS_OS.pdf, AUTOSAR_SWS_DEM.pdf, AUTOSAR_SWS_DiagnosticEventManager.pdf.
 */

 
#ifndef RTE_TYPES
#define RTE_TYPES

#include "Os_Types.h"

/*ci
 * \brief    AppMode of the core shall be inherited from another core. 
 */
typedef uint32   AppModeType;

/*ci
 * \brief    This type holds a timer value.
 */
typedef uint32   TickType;

/*ci
 * \brief    This type holds the state of an OS-Application.
 */
typedef uint8    ApplicationStateType;

/*ci
 * \brief    This type is the reference to a OS-Application.
 */
typedef uint8    ApplicationType;

/*ci
 * \brief    This type holds the restart parameter.
 */
typedef uint8    RestartType;

/*ci
 * \brief    This type is the reference to the requested Counter .
 */
typedef uint32   CounterType;

/*ci
 * \brief    Identification of an Event by assigned EventId.
 *           The EventId is configured in the DEM. 
 */
typedef uint16   Dem_EventIdType;

/*ci
 * \brief    In this data-type each bit has an individual meaning.
  *          The bit is set to 1 when the condition holds.
 */
typedef uint8    Dem_UdsStatusByteType;

/*ci
 * \brief    This type contains all definitions to control an internal debounce
 *           counter/timer via the function Dem_ResetEventDebounceStatus().
 */
typedef uint8    Dem_DebounceResetStatusType;

/*ci
 * \brief    This type contains all monitor test result values, which can be
 *           reported via Dem_SetEventStatus().
 */
typedef uint8    Dem_EventStatusType;

#endif /* RTE_TYPES */
