/* 

 (c) Copyright, Real-Time Innovations, 2006-2015.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/
/*
modification history
--------------------
23jan2016,tk MICRO-1523 Use bool instead of RTI_BOOL
22as2015,as  Created
=========================================================================*/

#ifndef dds_cpp_osapi_hxx
#define dds_cpp_osapi_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#if OSAPI_ENABLE_LOG

/*i \addtogroup OSAPILogClass
 */

/*i \brief OSAPI Log Class
 */
class DDSCPPDllExport OSAPILog
{
public:
    static bool initialize();
#ifndef RTI_CERT
    static bool finalize();
#endif /* RTI_CERT */
    static bool clear();

    /*i
     * \brief Install a log handler
     *
     * \details
     * The log functionality allows the user to specify a log handler. The log
     * handler is a function which is called for every logged event. It is up
     * to the user to decide what to do with the log message. The handler is
     * a global function pointer.
     *
     * \param [in] handler Pointer to log handler function
     * \param [in] param   Parameter passed to the log handler function. This
     *                     parameter is transparent to the log functionality.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool set_log_handler(OSAPI_LogHandler_T handler,void *param);

    /*i
     * \ingroup OSAPILogClass
     * \brief Return the current log handler
     *
     * \details
     * Return the current log handler.
     *
     * \param [in] handler Pointer to store log handler function
     * \param [in] param   Pointer to store the current log handler parameter.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool get_log_handler(OSAPI_LogHandler_T *handler,void **param);

#if OSAPI_ENABLE_TRACE
    /*i
     * \ingroup OSAPILogClass
     * \brief Install a trace handler
     *
     * \details
     * Install a custom trace handler. Traces are not stored in the log-buffer
     * and is generally used to analyze behavior interactively. the default
     * trace handler outputs the trace data to a console.
     *
     * \param [in] handler Pointer to trace handler function
     * \param [in] param   Parameter passed to the trace handler function. This
     *                     parameter is transparent to the trace functionality.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool set_trace_handler(OSAPI_TraceHandler_T handler,void *param);

    /*i
     * \brief Return the trace handler
     * \ingroup OSAPILogClass
     *
     * \details
     * Return the current trace handler and trace parameter. This information
     * can be used to daisy-chain calls to multiple trace-handlers.
     *
     * \param [in] handler Pointer to store trace handler function
     * \param [in] param   Pointer to store the trace handler parameter.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool get_trace_handler(OSAPI_TraceHandler_T *handler,void **param);
#endif /* OSAPI_ENABLE_TRACE */

    /*i
     * \ingroup OSAPILogClass
     * \brief Install a display handler
     *
     * \details
     * The display handler is responsible for outputting log messages to a console.
     *
     * \param [in] handler Pointer to display function
     * \param [in] param   Parameter passed to the display handler function. This
     *                     parameter is transparent to the log functionality.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool set_display_handler(OSAPI_LogDisplay_T handler,void *param);

    /*i
     * \brief Return the current display function
     * \ingroup OSAPILogClass
     *
     * \details
     * Return the current log handler.
     *
     * \param [in] handler Pointer to store display handler function
     * \param [in] param   Pointer to store the current display handler parameters.
     *
     * \return RTI_TRUE on success, RTI_FALSE on failure
     *
     */
    static bool get_display_handler(OSAPI_LogDisplay_T *handler,void **param);
};

#if OSAPI_ENABLE_TRACE
class DDSCPPDllExport OSAPITrace
{
public:
    static void set_trace_mask(RTI_UINT32 mask);
};
#endif /* OSAPI_ENABLE_TRACE */

#endif /* OSAPI_ENABLE_LOG */

#endif /* dds_cpp_osapi_hxx */
