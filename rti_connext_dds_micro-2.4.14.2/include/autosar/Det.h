/*
 * FILE: Det.h - Development Error Tracer
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
 * AUTOSAR_SWS_DevelopmentErrorTracer.pdf
 */

#ifndef DET_H
#define DET_H

#include "Std_Types.h"          /* Standard type header */

/*ci
 * \brief    Service to report development errors. 
 *
 * \param    ModuleId    Module ID of calling module. 
 * \param    InstanceId  The identifier of the index based instance of a module,
 *                       starting from 0, If the module is a single instance
 *                       module it shall pass 0 as the InstanceId.
 * \param    ApiId       ID of API service in which error is detected
 *                       (defined in SWS of calling module).
 * \param    ErrorId     ID of detected development error (defined in SWS of
 *                       calling module).
 *
 * \return   Std_ReturnType returns always E_OK (is required for services).
 *
 */
extern FUNC(Std_ReturnType, DET_CODE) Det_ReportError
(
    VAR(uint16, AUTOMATIC) ModuleId, 
    VAR(uint8, AUTOMATIC) InstanceId, 
    VAR(uint8, AUTOMATIC) ApiId, 
    VAR(uint8, AUTOMATIC) ErrorId
);

#endif /* #ifdef DET_H */

