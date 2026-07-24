
/*
 * FILE: XcdrTypeInterface.c - XCDR TypeSystem interface
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "Interpreter.h"
#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"
#include "dds_c/dds_c_typecode.h"
#include "xcdr/xcdr_dds_interpreter.h"

struct RTIXCdrInterpreterPrograms*
XCdrTypeInterfaceI_create_program(
    const RTIXCdrTypeCode *tc,
    const struct RTIXCdrInterpreterProgramsGenProperty *property,
    RTIXCdrProgramMask mask)
{
    return RTIXCdrInterpreterPrograms_new(tc,property,mask);
}

void
XCdrTypeInterfaceI_delete_program(
            struct RTIXCdrInterpreterPrograms *programs)
{
    RTIXCdrInterpreterPrograms_delete(programs);
}

DDS_Boolean
XCdrTypeInterfaceI_set_padding_options(const DDS_TypeCode *const tc,
                                       DDS_EncapsulationId_t eid)
{
    RTIXCdrBoolean is_v2_cncapsulation =
                XCDR_Interpreter_isEncapsulationCdrV2(eid);

    if (RTIXCdrTypeCode_sampleMayRequirePadding(
                        (const struct RTIXCdrTypeCode*)tc,
                        is_v2_cncapsulation))
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_BOOLEAN_FALSE;
}

struct RTIXCdrTypePluginProgramContext*
XCdrTypeInterfaceI_create_execution_context(const DDS_TypeCode *const tc)
{
    struct RTIXCdrTypePluginProgramContext *retval = NULL;
    UNUSED_ARG(tc);

    OSAPI_Heap_allocate_struct(&retval,struct RTIXCdrTypePluginProgramContext);

    return retval;
}

void
XCdrTypeInterfaceI_delete_execution_context(
                            struct RTIXCdrTypePluginProgramContext *ctxt)
{
    if (ctxt != NULL)
    {
        OSAPI_Heap_free_struct(ctxt);
    }
}
