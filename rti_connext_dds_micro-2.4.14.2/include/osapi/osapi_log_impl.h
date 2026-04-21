/*
 * FILE: osapi_log_impl.h - Implementation of Log functions
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
  * 15feb2022,tk MICRO-3455
 * - Added OSAPI_CC_STRINGIFY_DEFINE macro to workaround incorrect
 *   macro expansion on Deos
 * 07mar2013,tk Updated
 * 22sep2011,tk Updated
 * 23sep2008,yy Created
 */
/*ce
 * \file 
 * \brief Implementation of log functionality
 */
#include "osapi/osapi_config.h"

#ifndef osapi_log_impl_h
#define osapi_log_impl_h

#define OSAPI_TRACEKIND_NONE            0x00
#define OSAPI_TRACEKIND_NET             0x01
#define OSAPI_TRACEKIND_DDS             0x02
#define OSAPI_TRACEKIND_THREAD          0x04

#define NETIO_FORMAT(s_)  "NETIO ...: " s_
#define DDSC_FORMAT(s_)   "DDS .....: " s_
#define THREAD_FORMAT(s_) "THREAD ..: " s_

#if !OSAPI_ENABLE_TRACE

#define OSAPI_TRACE(title_,final_)
#define OSAPI_TRACE_DDS(title_,final_)
#define OSAPI_TRACE_NET(title_,final_)
#define OSAPI_TRACE_THREAD(title_,final_)
#define OSAPI_TRACE_INT32(name_,value_,final_)
#define OSAPI_TRACE_STRING(name_,value_,final_)
#define OSAPI_TRACE_GUID(name_,value_,final_)
#define OSAPI_TRACE_PTR(name_,value_,final_)
#define OSAPI_TRACE_A4_AS_INT32(name_,value_,final_)
#define OSAPI_TRACE_A12_AS_INT32(name_,value_,final_)

#define OSAPI_TRACE_ONLY_VARIABLE(x_) (void)(x_)

#else

#define OSAPI_TRACE_ONLY_VARIABLE(x_)

#define OSAPI_TRACE(title_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_HEADER,title_,0,NULL,NULL,final_);

#define OSAPI_TRACE_KIND(c_) OSAPI_gv_TraceMask,OSAPI_gv_TraceFunctionParam,\
            (c_),OSAPI_CC_STRINGIFY_DEFINE(RTI_MODULE_NAME),__FILE__,RTIME_FUNCTION_NAME,__LINE__

#define OSAPI_TRACE_DDS(title_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_DDS),\
     OSAPI_TRACETYPE_HEADER,DDSC_FORMAT(title_),0,NULL,NULL,final_);

#define OSAPI_TRACE_NET(title_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NET),\
     OSAPI_TRACETYPE_HEADER,NETIO_FORMAT(title_),0,NULL,NULL,final_);

#define OSAPI_TRACE_THREAD(title_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_THREAD),\
     OSAPI_TRACETYPE_HEADER,THREAD_FORMAT(title_),0,NULL,NULL,final_);

#define OSAPI_TRACE_INT32(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_INT32,name_,(RTI_INT32)value_,NULL,NULL,final_);

#define OSAPI_TRACE_STRING(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_STRING,name_,0,NULL,value_,final_);

#define OSAPI_TRACE_GUID(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_GUID,name_,0,value_,NULL,final_);

#define OSAPI_TRACE_PTR(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_PTR,name_,0,value_,NULL,final_);

#define OSAPI_TRACE_A4_AS_INT32(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_V4_AS_INT32 ,name_,0,value_,NULL,final_);

#define OSAPI_TRACE_A12_AS_INT32(name_,value_,final_) \
     OSAPI_gv_TraceFunction(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_V12_AS_INT32,name_,0,value_,NULL,final_);

#endif

#define OSAPI_PRECONDITION_PARAM OSAPI_LOGKIND_PRECONDITION,0,\
                                 OSAPI_LOG_MSG_PN_X2_STD_PARAM

/*\ci
 * \brief Add a pre-condition check independent of whether debug or release
 *        libraries are built.
 *
 * \sa OSAPI_LOG_PRECONDITION
 */
#if OSAPI_ENABLE_LOG
#define OSAPI_PRECONDITION_ALWAYS(cond_,action_,args_) \
if ((cond_)) \
{\
    OSAPI_Log_entry_create(OSAPI_PRECONDITION_PARAM,RTI_FALSE);\
    args_;\
    action_;\
}
#else
#define OSAPI_PRECONDITION_ALWAYS(cond_,action_,args_) \
if ((cond_)) \
{\
    action_;\
}
#endif

#if OSAPI_ENABLE_PRECONDITION

/*ci
 * \brief Precondition test macro
 *
 * \details
 *
 * This macro is used to perform pre-condition checks, typically on
 * function arguments. The macro automatically adds a log-entry and
 * the user _must_ add at least one pre-condition argument, the one that
 * filed, or more. The last argument added must have the is_final flag
 * set to RTI_TRUE.
 *
 * Example:
 *
 * Consider the function:
 *
 * RTI_BOOL
 * A_Function(void *pointer)
 * {
 *      OSAPI_LOG_PRECONDITION(pointer != NULL, return RTI_FALSE,
 *                    OSAPI_Log_entry_add_pointer("pointer",pointer,RTI_TRUE);)
 *
 *    .....
 * }
 *
 * This function tests whether the input pointer is NULL or not, and it its
 * NULL adds the name and value of the pointer to the log-entry.
 *
 * RTI_BOOL
 * Another_Function(void *pointer,RTI_INT32 length)
 * {
 *      OSAPI_LOG_PRECONDITION(pointer != NULL || length > 100, return RTI_FALSE,
 *                    OSAPI_Log_entry_add_pointer("pointer",pointer,RTI_FALSE);
 *                    OSAPI_Log_entry_add_int("length",length,RTI_TRUE);)
 *
 *    .....
 * }
 *
 * This function tests whether the input pointer is NULL or not and that the
 * length <= 100, otherwise both the arguments are added to the log-buffer.
 * Note that the last entry as is_final set to RTI_TRUE to indicate to no
 * more entries should be added to that current log-entries.
 *
 *
 * \sa OSAPI_Log_entry_add_int
 * \sa OSAPI_Log_entry_add_string
 * \sa OSAPI_Log_entry_add_pointer
 *
 */
#if OSAPI_ENABLE_LOG
#define OSAPI_PRECONDITION(cond_,action_,args_) \
if ((cond_)) \
{\
    OSAPI_Log_entry_create(OSAPI_PRECONDITION_PARAM,RTI_FALSE);\
    args_;\
    action_;\
}
#else
#define OSAPI_PRECONDITION(cond_,action_,args_) \
if ((cond_)) \
{\
    action_;\
}
#endif

#else

#define OSAPI_PRECONDITION(cond_,action_,args_)

#endif

#endif /* osapi_log_impl_h */
