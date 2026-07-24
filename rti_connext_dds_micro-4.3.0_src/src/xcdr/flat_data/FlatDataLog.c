/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#include "xcdr/xcdr_flat_data.h"
#include "xcdr/xcdr_stream.h"
#include "../infrastructure/Infrastructure.h"
#include "../stream/Stream.h"

void RTIXCdrFlatData_logPreconditionError(
        const char *fileName,
        RTIXCdrUnsignedLong line,
        const char *errorMessage)
{
    RTIXCdrLogParam param;

    param.kind = RTI_XCDR_LOG_STR_PARAM;
    param.value.strVal = errorMessage;
    RTIXCdrLog_logWithParams(
            fileName,
            "rti::flat",
            line,
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s,
            1,
            &param);
}

void RTIXCdrFlatData_logCreationError(
        const char *fileName,
        RTIXCdrUnsignedLong line,
        const char *entityName)
{
    RTIXCdrLogParam param;

    param.kind = RTI_XCDR_LOG_STR_PARAM;
    param.value.strVal = entityName;
    RTIXCdrLog_logWithParams(
            fileName,
            "rti::flat",
            line,
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
            1,
            &param);
}

void RTIXCdrFlatData_logBuilderOutOfResources(
        const char *fileName,
        RTIXCdrUnsignedLong line)
{
    RTIXCdrLog_logWithParams(
            fileName,
            "rti::flat",
            line,
            RTI_XCDR_LOG_EXCEPTION,
            RTI_XCDR_LOG_BUILDER_OUT_OF_RESOURCES_FAILURE_ID,
            0,
            NULL);
}

