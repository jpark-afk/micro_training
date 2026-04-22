/*
 * FILE: autosarLog.c - Autosar Log implementation
 *
 * (c) Copyright, Real-Time Innovations, 2019-2026.
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
 * 21jul2021,tk MICRO-3045 Fixed filenames and dates in file header comments
 * 05may2020,tk  MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 01oct2019,fmt Created
 */
/*ce
 * \file
 * \brief Implementation of logging facilities
 */
#include "rti_me_psl.h"

/*** SOURCE_BEGIN ***/

#if OSAPI_ENABLE_LOG

#include <Det.h>

FUNC(void, SOAD_CODE) 
OSAPI_AutosarLog_socket_write(const char *buffer, RTI_SIZE_T length)
{
    static TcpIp_SocketIdType socket;
    static boolean initialized = FALSE;
    Std_ReturnType ret_val = E_NOT_OK;

    /* if socket is not created -> create it */
    if (!initialized && (OSAPI_System_gv_PortProperty->get_socket != NULL))
    {
        ret_val = OSAPI_System_gv_PortProperty->get_socket(TCPIP_AF_INET,
                                                           TCPIP_IPPROTO_UDP,
                                                           &socket);
        if (ret_val == E_OK)
        {
            initialized = TRUE;
        }
        /*
        else
        {
        */
            /* As this is the log function, we can't log an error or we risk
             * going to an infinite loop
             */
        /*
        }
        */
    }
   
    /* if socket is created -> send data */
    if (initialized && (buffer != NULL) && (length > 0))
    {
        TcpIp_SockAddrInetType remote_addr;
           
        remote_addr.addr[0] = OSAPI_System_gv_PortProperty->log_dst_address;
        remote_addr.port = OSAPI_System_gv_PortProperty->log_dst_port;
        remote_addr.DOMAIN_FIELD = TCPIP_AF_INET;

        ret_val = TcpIp_UdpTransmit((TcpIp_SocketIdType)socket,
                                    (P2VAR(uint8, AUTOMATIC, AUTOMATIC))buffer,
                                    (P2VAR(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC))&remote_addr,
                                    (uint16)length);

        /* Return values TCPIP_E_PHYS_ADDR_MISS and TCPIP_E_ARP_CACHE_MISS mean
         * UDP message could not be sent because of an ARP cache miss. We ignore
         * that return codes and return success.
         */
#if defined(TCPIP_E_PHYS_ADDR_MISS) || (OS_VENDOR_ID == 31)
        /* Some implementations use TCPIP_E_PHYS_ADDR_MISS while others use
         * TCPIP_E_ARP_CACHE_MISS for an ARP cache miss condition.
         * Mentor (VENDOR_ID 31) uses TCPIP_E_PHYS_ADDR_MISS but defines it
         * as an enum rather than a macro, so it is not detectable with #if defined().
         */
        if (ret_val == TCPIP_E_PHYS_ADDR_MISS)
#else
        if (ret_val == TCPIP_E_ARP_CACHE_MISS)
#endif
        {
            ret_val = E_OK;
        }
    }
    IGNORE_RETVAL(ret_val); 
}

FUNC(void, SOAD_CODE) 
OSAPI_AutosarLog_default_display(void *param, OSAPI_LogEntry_T *log_entry)
{
    Std_ReturnType stat;
    uint32 kind;
    OSAPI_LogVerbosity_T verbosity;
    uint32 module_id;
    uint32 error_id;
    uint8 api_id;
    uint8 api_error_id;

    UNUSED_ARG(param);

    verbosity = OSAPI_Log_get_verbosity();

    kind = OSAPI_LOG_HEADER_GET_TYPE(log_entry->error_code);

    if ((kind == OSAPI_LOGKIND_INFO) 
            && (verbosity >= OSAPI_LOG_VERBOSITY_DEBUG))
    {
    }
    else if ((kind == OSAPI_LOGKIND_WARNING)
            && (verbosity >= OSAPI_LOG_VERBOSITY_WARNING))
    {
    }
    else if ((kind == OSAPI_LOGKIND_ERROR)
            && (verbosity >= OSAPI_LOG_VERBOSITY_ERROR))
    {
    }
    else if ((kind == OSAPI_LOGKIND_PRECONDITION)
            && (verbosity >= OSAPI_LOG_VERBOSITY_ERROR))
    {
    }
    else
    {
        return;
    }

    module_id = OSAPI_LOG_HEADER_GET_MODULE(log_entry->error_code);
    error_id = OSAPI_LOG_HEADER_GET_EC(log_entry->error_code);

    /* Det_ReportError only provides 8-bit ApiId and 8-bit ErrorId fields, but
     * our module_id needs 4 bits and error_id can exceed 8 bits. We pack both
     * into the two available bytes as follows:
     *
     *   api_id      [7:4] = error_id[11:8]   (high 4 bits of error_id)
     *   api_id      [3:0] = module_id[3:0]   (4-bit module_id)
     *   api_error_id[7:0] = error_id[7:0]    (low 8 bits of error_id)
     *
     * This supports error codes up to 12 bits (4096 values).
     */
    api_id = (((uint8)(error_id & 0x00000F00)) >> 4) | ((uint8)(module_id & 0x0000000F));
    api_error_id = (uint8)(error_id & 0x000000FF);

    stat = Det_ReportError(RTIME_DDS_MODULE_ID,
                           RTIME_DDS_INSTANCE_ID,
                           api_id,
                           api_error_id);
    IGNORE_RETVAL(stat); 
}
#endif /* OSAPI_ENABLE_LOG */

RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return errno;
}

void
OSAPI_Log_set_last_error_code(RTI_INT32 err)
{
    errno = err;
}

void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length)
{
    UNUSED_ARG(length);
    
    if (buffer == NULL)
    {
        return;
    }
    
    // serialprintf("%s", buffer);
}
