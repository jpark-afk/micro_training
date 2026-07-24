/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
-------------------- 
23jan2016,tk MICRO-1523 Use bool instead of RTI_BOOL
22dec2015,as Created
===================================================================== */


#ifndef dds_cpp_osapi_hxx
#include "dds_cpp/dds_cpp_osapi.hxx"
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_ENABLE_LOG

bool
OSAPILog::initialize()
{
    return (OSAPI_Log_initialize() == RTI_TRUE);
}

#ifndef RTI_CERT
bool
OSAPILog::finalize()
{
    return (OSAPI_Log_finalize() == RTI_TRUE);
}
#endif

bool
OSAPILog::clear()
{
    return (OSAPI_Log_clear() == RTI_TRUE);
}

bool
OSAPILog::set_log_handler(OSAPI_LogHandler_T handler,void *param)
{
    return (OSAPI_Log_set_log_handler(handler,param) == RTI_TRUE);
}

#ifndef RTI_CERT
bool
OSAPILog::get_log_handler(OSAPI_LogHandler_T *handler,void **param)
{
    return (OSAPI_Log_get_log_handler(handler,param) == RTI_TRUE);
}
#endif

#if OSAPI_ENABLE_TRACE
bool
OSAPILog::set_trace_handler(OSAPI_TraceHandler_T handler,void *param)
{
    return (OSAPI_Log_set_trace_handler(handler,param) == RTI_TRUE);
}

bool
OSAPILog::get_trace_handler(OSAPI_TraceHandler_T *handler,void **param)
{
    return (OSAPI_Log_get_trace_handler(handler,param) == RTI_TRUE);
}
#endif /* OSAPI_ENABLE_TRACE */

bool
OSAPILog::set_display_handler(OSAPI_LogDisplay_T handler,void *param)
{
    return (OSAPI_Log_set_display_handler(handler,param) == RTI_TRUE);
}

#ifndef RTI_CERT
bool
OSAPILog::get_display_handler(OSAPI_LogDisplay_T *handler,void **param)
{
    return (OSAPI_Log_get_display_handler(handler,param) == RTI_TRUE);
}
#endif

#if OSAPI_ENABLE_TRACE
void
OSAPITrace::set_trace_mask(RTI_UINT32 mask)
{
    OSAPI_Trace_set_trace_mask(mask);
}
#endif /* OSAPI_ENABLE_TRACE */

#endif /* OSAPI_ENABLE_LOG */
