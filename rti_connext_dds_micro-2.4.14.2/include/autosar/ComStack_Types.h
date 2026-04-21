/*
 * FILE: ComStack_Types.h - Basic communication stack types
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*
 * Specification of Platform Types AUTOSAR CP Release 4.0.3
 * AUTOSAR_SWS_CommunicationStackTypes.pdf
 */

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"
#include "ComStack_Cfg.h"


/*ci \brief PduInfoType
 *
 * \details
 *
 * Basic information about a PDU of any type
 */
typedef struct
{
   /*ci \brief SduDataPtr
    *
    * \details
    *
    * Pointer variable pointing to SDU (payload).
    */
   uint8 *SduDataPtr;

   /*ci \brief SduLength
    *
    * \details
    *
    * Length of the SDU in bytes.
    */
   PduLengthType   SduLength;
} PduInfoType;

/*ci \brief PduInfoType
 *
 * \details
 *
 * Variables of this type shall be used to store the result of a buffer request.
 */
typedef enum
{
    BUFREQ_OK,
    BUFREQ_E_NOT_OK,
    BUFREQ_E_BUSY,
    BUFREQ_E_OVFL
}BufReq_ReturnType;

#endif
