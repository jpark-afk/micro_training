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
 * AUTOSAR_SWS_StandardTypes.pdf
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"
#include "Compiler.h"
#include "Os_Types.h"

/*ci
 * \brief Standard return OK value
 */
#define E_OK     0x00

/*ci
 * \brief Standard return NOT OK value
 */
#define E_NOT_OK 0x01

/*ci
 * \brief Standard return type
 */
typedef uint8  Std_ReturnType;

/*ci
 * \brief Status type per AutoSAR standard
 */
typedef unsigned char StatusType;

/*ci
 * \brief Pointer type per AutoSAR standard
 */
typedef uint32 uintptr;

/*ci
 * \brief Structure for the Version of the module.
 *        This is requested by calling <Module name>_GetVersionInfo()
 */
typedef struct
{
  uint16  vendorID;
  uint16  moduleID;
  uint8   sw_major_version;
  uint8   sw_minor_version;
  uint8   sw_patch_version;
}Std_VersionInfoType;

#endif /* STD_TYPES_H */
