/*
 * FILE: autosarLog.c - Autosar Log implementation
 *
 * (c) Copyright, Real-Time Innovations, 2019-2021.
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
#include "osapi/osapi_config.h"

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_AUTOSAR

#include "osapi/osapi_log.h"

#if OSAPI_ENABLE_LOG

#include <Det.h>

FUNC(void, SOAD_CODE) 
OSAPI_AutosarLog_socket_write(const char *buffer, RTI_SIZE_T length)
{
    static TcpIp_SocketIdType socket;
    static boolean initialized = FALSE;
    Std_ReturnType ret_val = E_NOT_OK;

    /* if socket is not created -> create it */
    if ((!initialized) && (OSAPI_System_gv_PortProperty->get_socket != NULL))
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
        P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC) remote_addr_ptr;
        TcpIp_SockAddrInetType remote_addr;

        remote_addr_ptr =
                (P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC))
                (P2CONST(void, AUTOMATIC, AUTOMATIC))
                &remote_addr;
           
        remote_addr.addr[0] = OSAPI_System_gv_PortProperty->log_dst_address;
        remote_addr.port = OSAPI_System_gv_PortProperty->log_dst_port;
#if MENTOR_OLD_COMPAT
        /* Defect DR/ER #: VOL-VSTARMOD-16814. See SR #: 3386249991 */
        remote_addr.TcpIp_Domain = TCPIP_AF_INET;
#else
        remote_addr.domain = TCPIP_AF_INET;
#endif

        ret_val = TcpIp_UdpTransmit((TcpIp_SocketIdType)socket,
                                    (P2CONST(uint8, AUTOMATIC, SOAD_APPL_DATA))buffer,
                                    remote_addr_ptr,
                                    (uint16)length);

        /* Return values TCPIP_E_PHYS_ADDR_MISS and TCPIP_E_ARP_CACHE_MISS mean
         * UDP message could not be sent because of an ARP cache miss. We ignore
         * that return codes and return success.
         */
#if defined(TCPIP_E_PHYS_ADDR_MISS) || (OS_VENDOR_ID == 31)
        /* Mentor (VENDOR_ID 31) defines TCPIP_E_PHYS_ADDR_MISS
         * as an enum not as a macro
         */
        if (ret_val == TCPIP_E_PHYS_ADDR_MISS)
#else
        if (ret_val == TCPIP_E_ARP_CACHE_MISS)
#endif
        {
            ret_val = E_OK;
        }
    }
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

    /* Module id fits in 8 bits, actually in 4 bits. But error_id doesn't fits
     * in 8 bits, we have many errors id with higher value than 256.
     * Solution? Use 4 LSB of ApiId for module ID. That is sufficient.
     * Use ErrorId and 4 MSB of ApiId for error_id; that should be enough as
     * 256 * 16 is higher than any error code that we have at the moment.
     *
     * Most of error codes are smaller than 256, so we would have ApiId == module
     * and ErrorId == error code. Otherwise we would need to calculate them.
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

#endif /* OSAPI_INCLUDE_AUTOSAR */

