/*
 * FILE: LogWrapper.c - Wrapper around calls to log function
 *
 * Copyright (c) 2025-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"

void
OSPSL_Log_entry_create(OSAPI_LOG_FORMAL_ARG,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->create(OSAPI_LOG_PARAMETER_ARG,is_final);
    }
}

void
OSPSL_Log_entry_add_int(const char *name,RTI_INT32 value,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_int(name,value,is_final);
    }
}

void
OSPSL_Log_entry_add_uint(const char *name,RTI_UINT32 value,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_uint(name,value,is_final);
    }
}

void
OSPSL_Log_entry_add_int_hex(const char *name,RTI_INT32 value,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_int_hex(name,value,is_final);
    }
}

void
OSPSL_Log_entry_add_string(const char *name,const char *value,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_string(name,value,is_final);
    }
}

void
OSPSL_Log_entry_add_pointer(const char *name,const void *value,RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_pointer(name,value,is_final);
    }
}

void
OSPSL_Log_entry_add_1int_hex(OSAPI_LOG_FORMAL_ARG,const char *name,
                              RTI_INT32 value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1int_hex(OSAPI_LOG_PARAMETER_ARG,name,value);
    }
}

void
OSPSL_Log_entry_add_1int(OSAPI_LOG_FORMAL_ARG,const char *name,RTI_INT32 value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1int(OSAPI_LOG_PARAMETER_ARG,name,value);
    }
}

void
OSPSL_Log_entry_add_1uint(OSAPI_LOG_FORMAL_ARG,const char *name,RTI_UINT32 value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1uint(OSAPI_LOG_PARAMETER_ARG,name,value);
    }
}

void
OSPSL_Log_entry_add_2int(OSAPI_LOG_FORMAL_ARG,
                         const char *name1,RTI_INT32 value1,
                         const char *name2,RTI_INT32 value2)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_2int(OSAPI_LOG_PARAMETER_ARG,
                                       name1,value1,name2,value2);
    }
}

void
OSPSL_Log_entry_add_3int(OSAPI_LOG_FORMAL_ARG,
                              const char *name1,RTI_INT32 value1,
                              const char *name2,RTI_INT32 value2,
                              const char *name3,RTI_INT32 value3)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_3int(OSAPI_LOG_PARAMETER_ARG,
                                       name1,value1,name2,value2,name3,value3);
    }
}

void
OSPSL_Log_entry_add_1string(OSAPI_LOG_FORMAL_ARG,
                            const char *name,const char* value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1string(OSAPI_LOG_PARAMETER_ARG,name,value);
    }
}

void
OSPSL_Log_entry_add_2string(OSAPI_LOG_FORMAL_ARG,
                            const char *name1,const char* value1,
                            const char *name2,const char* value2)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_2string(OSAPI_LOG_PARAMETER_ARG,
                                          name1,value1,name2,value2);
    }
}

void
OSPSL_Log_entry_add_1string_1int(OSAPI_LOG_FORMAL_ARG,
                                      const char *s_name,const char* s_value,
                                      const char *i_name,RTI_INT32 i_value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1string_1int(OSAPI_LOG_PARAMETER_ARG,
                                               s_name,s_value,i_name,i_value);
    }
}

void
OSPSL_Log_entry_add_1pointer(OSAPI_LOG_FORMAL_ARG,
                                  const char *name,
                                  const void* value)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1pointer(OSAPI_LOG_PARAMETER_ARG,name,value);
    }
}

void
OSPSL_Log_entry_add(OSAPI_LOG_FORMAL_ARG)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add(OSAPI_LOG_PARAMETER_ARG);
    }
}

void
OSPSL_Log_entry_add_exception(OSAPI_LOG_FORMAL_ARG,
                              const RTI_INT32 i_value1,
                              const RTI_INT32 i_value2,
                              const char* s_value)

{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_exception(OSAPI_LOG_PARAMETER_ARG,
                                            i_value1,i_value2,s_value);
    }
}

void
OSPSL_Log_entry_add_1string_2int(OSAPI_LOG_FORMAL_ARG,
                                 const char *s_name, const char* s_value,
                                 const char *i_name1, RTI_INT32 i_value1,
                                 const char *i_name2, RTI_INT32 i_value2)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_1string_2int(OSAPI_LOG_PARAMETER_ARG,
                                               s_name,s_value,
                                               i_name1,i_value1,
                                               i_name2,i_value2);
    }
}

void
OSPSL_Log_entry_add_3string(OSAPI_LOG_FORMAL_ARG,
                                  const char *s_name1, const char* s_value1,
                                  const char *s_name2, const char* s_value2,
                                  const char *s_name3, const char* s_value3)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->add_3string(OSAPI_LOG_PARAMETER_ARG,
                                         s_name1,s_value1,
                                         s_name2,s_value2,
                                         s_name3,s_value3);
    }
}

#if OSAPI_ENABLE_TRACE
void
OSPSL_Log_entry_trace_handler(RTI_UINT32 trace_mask,
                            void *param,
                            RTI_UINT32 context,
                            const char *const module,
                            const char *const file,
                            const char *const function,
                            RTI_INT32 line_no,
                            OSAPI_TraceType_T type,
                            const void *title,
                            RTI_INT32 int_value,
                            const void *ptr_value,
                            const char *str_value,
                            RTI_BOOL is_final)
{
    if (OSAPI_Log_gv_LogIntf != NULL)
    {
        OSAPI_Log_gv_LogIntf->trace_handler(trace_mask,
                                            param,
                                            context,
                                            module,
                                            file,
                                            function,
                                            line_no,
                                            type,
                                            title,
                                            int_value,
                                            ptr_value,
                                            str_value,
                                            is_final);
    }
}
#endif
