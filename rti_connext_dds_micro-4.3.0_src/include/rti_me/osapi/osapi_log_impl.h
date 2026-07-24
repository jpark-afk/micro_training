/*
 * FILE: osapi_log_impl.h - Implementation of Log functions
 *
 * Copyright 2008-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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


/******************************************************************************
 *               Some temporary debugging helpers
 *****************************************************************************/
#if OSAPI_ENABLE_PRECONDITION

#define TDBG_GUID_BIT(g_,i_) \
    ((unsigned char)((char*)(g_))[i_])

#define TDBG_GUID_FMT   "%02X%02X%02X%02X.%02X%02X%02X%02X."\
                        "%02X%02X%02X%02X.%02X%02X%02X%02X"
#define TDBG_GUID_ARGS(g_) \
    TDBG_GUID_BIT(g_,0), TDBG_GUID_BIT(g_,1), TDBG_GUID_BIT(g_,2), TDBG_GUID_BIT(g_,3), \
    TDBG_GUID_BIT(g_,4), TDBG_GUID_BIT(g_,5), TDBG_GUID_BIT(g_,6), TDBG_GUID_BIT(g_,7), \
    TDBG_GUID_BIT(g_,8), TDBG_GUID_BIT(g_,9), TDBG_GUID_BIT(g_,10), TDBG_GUID_BIT(g_,11), \
    TDBG_GUID_BIT(g_,12), TDBG_GUID_BIT(g_,13), TDBG_GUID_BIT(g_,14), TDBG_GUID_BIT(g_,15)

#define TDBG_GUID_ARGS_BE(g_) \
    TDBG_GUID_BIT(g_,3), TDBG_GUID_BIT(g_,2), TDBG_GUID_BIT(g_,1), TDBG_GUID_BIT(g_,0), \
    TDBG_GUID_BIT(g_,7), TDBG_GUID_BIT(g_,6), TDBG_GUID_BIT(g_,5), TDBG_GUID_BIT(g_,4), \
    TDBG_GUID_BIT(g_,11), TDBG_GUID_BIT(g_,10), TDBG_GUID_BIT(g_,9), TDBG_GUID_BIT(g_,8), \
    TDBG_GUID_BIT(g_,15), TDBG_GUID_BIT(g_,14), TDBG_GUID_BIT(g_,13), TDBG_GUID_BIT(g_,12)


#else

#define TDBG_GUID_BIT(g_,i_)
#define TDBG_GUID_FMT   ""
#define TDBG_GUID_ARGS(g_)
#define TDBG_GUID_ARGS_BE(g_)

#endif /* OSAPI_ENABLE_PRECONDITION */
/******************************************************************************
 *               //Some temporary debugging helpers
 *****************************************************************************/

/*e \dref_OSAPI_TRACEKIND_NONE
 */
#define OSAPI_TRACEKIND_NONE            0x00

/*e \dref_OSAPI_TRACEKIND_NET
 */
#define OSAPI_TRACEKIND_NET             0x01

/*e \dref_OSAPI_TRACEKIND_DDS
 */
#define OSAPI_TRACEKIND_DDS             0x02

/*e \dref_OSAPI_TRACEKIND_THREAD
 */
#define OSAPI_TRACEKIND_THREAD          0x04

/*e \dref_OSAPI_TRACEKIND_SHMEM
 */
#define OSAPI_TRACEKIND_SHMEM           0x10

/*e \dref_OSAPI_TRACEKIND_RTPS
 */
#define OSAPI_TRACEKIND_RTPS            0x20

#define OSAPI_TRACEKIND_TRUST           0x40

#define NETIO_FORMAT(s_)      "NETIO ...: " s_
#define DDSC_FORMAT(s_)       "DDS .....: " s_
#define THREAD_FORMAT(s_)     "THREAD ..: " s_
#define SHMEM_COPY_FORMAT(s_) "SHMEM ...: " s_
#define TRUST_FORMAT(s_)      "TRUST ...: " s_

#define OSAPI_TRACE_INT_AS_PTR(x_) (((RTI_INT32)(x_)) + (char*)NULL)

#define OSAPI_TRACE_STDPARAM OSAPI_gv_TraceMask,NULL,0,OSAPI_CC_STRINGIFY_DEFINE(RTI_MODULE_NAME),\
                                        __FILE__,RTIME_FUNCTION_NAME,__LINE__

#if !OSAPI_ENABLE_TRACE

#define OSAPI_TRACE(title_,final_)
#define OSAPI_TRACE_DDS(title_,final_)
#define OSAPI_TRACE_NET(title_,final_)
#define OSAPI_TRACE_SHMEM(title_,final_)
#define OSAPI_TRACE_THREAD(title_,final_)
#define OSAPI_TRACE_INT32(name_,value_,final_)
#define OSAPI_TRACE_STRING(name_,value_,final_)
#define OSAPI_TRACE_GUID(name_,value_,final_)
#define OSAPI_TRACE_PTR(name_,value_,final_)
#define OSAPI_TRACE_A4_AS_INT32(name_,value_,final_)
#define OSAPI_TRACE_A12_AS_INT32(name_,value_,final_)

#define OSAPI_TRACE_ONLY_VARIABLE(x_) (void)(x_)

#define OSAPI_TRACE_PRINTF0(fmt)

#define OSAPI_TRACE_PRINTF1(fmt,arg0_)

#define OSAPI_TRACE_PRINTF2(fmt,arg0_,arg1_)

#define OSAPI_TRACE_PRINTF3(fmt,arg0_,arg1_,arg2_)

#define OSAPI_TRACE_PRINTF4(fmt,arg0_,arg1_,arg2_,arg3_)

#define OSAPI_TRACE_PRINTF5(fmt,arg0_,arg1_,arg2_,arg3_,arg4_)

#define OSAPI_TRACE_PRINTF6(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_)

#define OSAPI_TRACE_PRINTF7(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_)

#define OSAPI_TRACE_PRINTF8(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,arg7_)

#define OSAPI_TRACE_PRINTF9(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,\
                                arg6_,arg7_,arg8_,arg9_)

#define OSAPI_TRACE_PRINTF10(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,\
                                 arg6_,arg7_,arg8_,arg9_)

#define OSAPI_Trace_write(fmt_,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                          arg7_,arg8_,arg9_)

#define OSAPI_TRACE_TRUST_PGM(type_,sample_)
#define OSAPI_TRACE_TRUST_PGM_WRITE(sample_)
#define OSAPI_TRACE_TRUST_PGM_RECVD(sample_)
#define OSAPI_TRACE_TRUST_LOG_GUIDS(type_,g1_,g2_)
#define OSAPI_TRACE_TRUST_DP_AUTHD(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DP_TIMEOUT(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DP_READY(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DW_READY(local_dr_,remote_dw_)
#define OSAPI_TRACE_TRUST_DR_READY(local_dw_,remote_dr_)
#define OSAPI_TRACE_RTPS_SEND_HB(intf_addr_,hb_first_,hb_last_,hb_count_)
#define OSAPI_TRACE_RTPS_SEND_ACKNACK(intf_addr_,ack_bm_,ack_count_)
#define OSAPI_TRACE_RTPS_RCVD_ACKNACK(intf_addr_,ack_bm_,ack_count_)
#define OSAPI_TRACE_RTPS_SEND_GAP(intf_addr_,gap_start_,gap_bm_)
#define OSAPI_TRACE_RTPS_RCVD_GAP(intf_addr_,gap_start_,gap_bm_)
#define OSAPI_TRACE_RTPS_RCVD_DATA(intf_addr_,sn_)
#else

#define OSAPI_TRACE_ONLY_VARIABLE(x_)

#define OSAPI_TRACE_KIND(c_) \
            OSAPI_Log_gv_LogIntf->trace_mask,\
            OSAPI_Log_gv_LogIntf->trace_param,\
            (c_),OSAPI_CC_STRINGIFY_DEFINE(RTI_MODULE_NAME),\
            __FILE__,RTIME_FUNCTION_NAME,__LINE__

#define OSAPI_TRACE(title_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_HEADER,title_,0,NULL,NULL,final_))

#define OSAPI_TRACE_DDS(title_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_DDS),\
     OSAPI_TRACETYPE_HEADER,DDSC_FORMAT(title_),0,NULL,NULL,final_))

#define OSAPI_TRACE_NET(title_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NET),\
     OSAPI_TRACETYPE_HEADER,NETIO_FORMAT(title_),0,NULL,NULL,final_))

#define OSAPI_TRACE_SHMEM(title_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_SHMEM),\
     OSAPI_TRACETYPE_HEADER,NETIO_FORMAT(title_),0,NULL,NULL,final_))

#define OSAPI_TRACE_THREAD(title_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_THREAD),\
     OSAPI_TRACETYPE_HEADER,THREAD_FORMAT(title_),0,NULL,NULL,final_))

#define OSAPI_TRACE_INT32(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_INT32,name_,(RTI_INT32)value_,NULL,NULL,final_))

#define OSAPI_TRACE_STRING(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_STRING,name_,0,NULL,value_,final_))

#define OSAPI_TRACE_GUID(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_GUID,name_,0,value_,NULL,final_))

#define OSAPI_TRACE_PTR(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_PTR,name_,0,value_,NULL,final_))

#define OSAPI_TRACE_A4_AS_INT32(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_V4_AS_INT32 ,name_,0,value_,NULL,final_))

#define OSAPI_TRACE_A12_AS_INT32(name_,value_,final_) \
     OSAPI_Log_call_log_intf(trace_handler(OSAPI_TRACE_KIND(OSAPI_TRACEKIND_NONE),\
     OSAPI_TRACETYPE_V12_AS_INT32,name_,0,value_,NULL,final_))

#if !OSAPI_ENABLE_DEBUG_TRACE
#define OSAPI_TRACE_PRINTF0(fmt)

#define OSAPI_TRACE_PRINTF1(fmt,arg0_)

#define OSAPI_TRACE_PRINTF2(fmt,arg0_,arg1_)

#define OSAPI_TRACE_PRINTF3(fmt,arg0_,arg1_,arg2_)

#define OSAPI_TRACE_PRINTF4(fmt,arg0_,arg1_,arg2_,arg3_)

#define OSAPI_TRACE_PRINTF5(fmt,arg0_,arg1_,arg2_,arg3_,arg4_)

#define OSAPI_TRACE_PRINTF6(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_)

#define OSAPI_TRACE_PRINTF7(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_)

#define OSAPI_TRACE_PRINTF8(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,arg7_)

#define OSAPI_TRACE_PRINTF9(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,\
                                arg6_,arg7_,arg8_,arg9_)

#define OSAPI_TRACE_PRINTF10(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,\
                                 arg6_,arg7_,arg8_,arg9_)

#define OSAPI_Trace_write(fmt_,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                          arg7_,arg8_,arg9_)
#else
#define OSAPI_TRACE_PRINTF0(fmt) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,NULL,NULL,NULL,NULL,NULL,\
                           NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF1(fmt,arg0_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                           NULL,NULL,NULL,NULL,\
                           NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF2(fmt,arg0_,arg1_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,NULL,NULL,NULL,\
                       NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF3(fmt,arg0_,arg1_,arg2_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,NULL,NULL,\
                       NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF4(fmt,arg0_,arg1_,arg2_,arg3_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,(char*)arg3_,NULL,\
                       NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF5(fmt,arg0_,arg1_,arg2_,arg3_,arg4_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,\
                       NULL,NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF6(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                       NULL,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF7(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                       (char*)arg6_,NULL,NULL,NULL);

#define OSAPI_TRACE_PRINTF8(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                            arg7_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                       (char*)arg6_,(char*)arg7_,NULL,NULL);

#define OSAPI_TRACE_PRINTF9(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                            arg7_,arg8_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                       (char*)arg6_,(char*)arg7_,(char*)arg8_,NULL);

#define OSAPI_TRACE_PRINTF10(fmt,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                            arg7_,arg8_,arg9_) \
    OSAPI_Trace_printf(OSAPI_TRACE_STDPARAM,fmt,(char*)arg0_,\
                       (char*)arg1_,(char*)arg2_,\
                       (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                       (char*)arg6_,(char*)arg7_,(char*)arg8_,\
                       (char*)arg9_);

#define OSAPI_Trace_write(fmt_,arg0_,arg1_,arg2_,arg3_,arg4_,arg5_,arg6_,\
                              arg7_,arg8_,arg9_) \
                          OSAPI_Trace_write_impl(OSAPI_TRACE_STDPARAM,(fmt_),\
                          (char*)arg0_,(char*)arg1_,(char*)arg2_,\
                          (char*)arg3_,(char*)arg4_,(char*)arg5_,\
                          (char*)arg6_,(char*)arg7_,(char*)arg8_,\
                          (char*)arg9_)
#endif

#if 1
#define OSAPI_TRACE_TRUST_PGM(type_,sample_)
#define OSAPI_TRACE_TRUST_PGM_WRITE(sample_)
#define OSAPI_TRACE_TRUST_PGM_RECVD(sample_)
#define OSAPI_TRACE_TRUST_LOG_GUIDS(type_,g1_,g2_)
#define OSAPI_TRACE_TRUST_DP_AUTHD(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DP_TIMEOUT(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DP_READY(local_dp_,remote_dp_)
#define OSAPI_TRACE_TRUST_DW_READY(local_dr_,remote_dw_)
#define OSAPI_TRACE_TRUST_DR_READY(local_dw_,remote_dr_)
#define OSAPI_TRACE_RTPS_SEND_HB(intf_addr_,hb_first_,hb_last_,hb_count_)
#define OSAPI_TRACE_RTPS_SEND_ACKNACK(intf_addr_,ack_bm_,ack_count_)
#define OSAPI_TRACE_RTPS_RCVD_ACKNACK(intf_addr_,ack_bm_,ack_count_)
#define OSAPI_TRACE_RTPS_SEND_GAP(intf_addr_,gap_start_,gap_bm_)
#define OSAPI_TRACE_RTPS_RCVD_GAP(intf_addr_,gap_start_,gap_bm_)
#define OSAPI_TRACE_RTPS_RCVD_DATA(intf_addr_,sn_)
#else
#define OSAPI_TRACE_TRUST_PGM(type_,sample_) \
    printf("[PGM][" type_ "][%p] CLASS=%s; ID=[" TDBG_GUID_FMT ";%lld]; " \
             "REL_ID=[" TDBG_GUID_FMT ";%lld]; SRC_ENDP=" TDBG_GUID_FMT "; " \
             "DST_DP=" TDBG_GUID_FMT "; DST_ENDP=" TDBG_GUID_FMT ";\n", \
            (sample_), \
            (sample_)->message_class_id, \
            TDBG_GUID_ARGS_BE(&(sample_)->message_identity.source_guid), \
            (sample_)->message_identity.sequence_number, \
            TDBG_GUID_ARGS_BE(&(sample_)->related_message_identity.source_guid), \
            (sample_)->related_message_identity.sequence_number,\
            TDBG_GUID_ARGS_BE(&(sample_)->source_endpoint_guid),\
            TDBG_GUID_ARGS_BE(&(sample_)->destination_participant_guid),\
            TDBG_GUID_ARGS_BE(&(sample_)->destination_endpoint_guid));

#define OSAPI_TRACE_TRUST_PGM_WRITE(sample_) \
        OSAPI_TRACE_TRUST_PGM("write",(sample_))

#define OSAPI_TRACE_TRUST_PGM_RECVD(sample_) \
        OSAPI_TRACE_TRUST_PGM("recvd",(sample_))

#define OSAPI_TRACE_TRUST_LOG_GUIDS(type_,g1_,g2_) \
        printf(type_ " " TDBG_GUID_FMT " " TDBG_GUID_FMT "\n", \
                TDBG_GUID_ARGS((g1_)),\
                TDBG_GUID_ARGS_BE((g2_)));

#define OSAPI_TRACE_TRUST_DP_AUTHD(local_dp_,remote_dp_) \
        OSAPI_TRACE_TRUST_LOG_GUIDS("DP_AUTHD",(local_dp_),(remote_dp_))

#define OSAPI_TRACE_TRUST_DP_TIMEOUT(local_dp_,remote_dp_) \
        OSAPI_TRACE_TRUST_LOG_GUIDS("DP_TIMEOUT",(local_dp_),(remote_dp_))

#define OSAPI_TRACE_TRUST_DP_READY(local_dp_,remote_dp_) \
        OSAPI_TRACE_TRUST_LOG_GUIDS("DP_READY",(local_dp_),(remote_dp_))

#define OSAPI_TRACE_TRUST_DW_READY(local_dr_,remote_dw_) \
        OSAPI_TRACE_TRUST_LOG_GUIDS("DW_READY",(local_dr_),(remote_dw_))

#define OSAPI_TRACE_TRUST_DR_READY(local_dw_,remote_dr_) \
        OSAPI_TRACE_TRUST_LOG_GUIDS("DR_READY",(local_dw_),(remote_dr_))

#define OSAPI_TRACE_RTPS_SEND_HB(intf_addr_,hb_first_,hb_last_,hb_count_) \
        printf("RTPS_SEND_HB " TDBG_GUID_FMT \
               " 1st=(%d,%d), last=(%d,%d), count=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (hb_first_)->low, (hb_first_)->high, \
               (hb_last_)->low, (hb_last_)->high, \
               (hb_count_));

#define OSAPI_TRACE_RTPS_RCVD_HB(intf_addr_,hb_first_,hb_last_,hb_count_) \
        printf("RTPS_RCVD_HB " TDBG_GUID_FMT \
               " 1st=(%d,%d), last=(%d,%d), count=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (hb_first_)->low, (hb_first_)->high, \
               (hb_last_)->low, (hb_last_)->high, \
               (hb_count_));

#define OSAPI_TRACE_RTPS_SEND_ACKNACK(intf_addr_,ack_bm_,ack_count_) \
        printf("RTPS_SEND_ACKNACK " TDBG_GUID_FMT \
               " MAP.1st=(%d,%d), MAP.bits=%d, count=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (ack_bm_)->lead.low, (ack_bm_)->lead.high, \
               RTPS_Interface_get_bitmap_int_count((ack_bm_)->bit_count), \
               (ack_count_));

#define OSAPI_TRACE_RTPS_RCVD_ACKNACK(intf_addr_,ack_bm_,ack_count_) \
        printf("RTPS_RCVD_ACKNACK " TDBG_GUID_FMT \
               " MAP.1st=(%d,%d), MAP.bits=%d, count=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (ack_bm_)->lead.low, (ack_bm_)->lead.high, \
               RTPS_Interface_get_bitmap_int_count((ack_bm_)->bit_count), \
               (ack_count_));

#define OSAPI_TRACE_RTPS_RCVD_DATA(intf_addr_,sn_) \
        printf("RTPS_RCVD_DATA " TDBG_GUID_FMT " sn=(%d,%d)\n",\
               TDBG_GUID_ARGS((intf_addr_)), (sn_)->low, (sn_)->high);

#define OSAPI_TRACE_RTPS_SEND_GAP(intf_addr_,gap_start_,gap_bm_) \
        printf("RTPS_SEND_GAP " TDBG_GUID_FMT \
               " START=(%d,%d) MAP.1st=(%d,%d), MAP.bits=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (gap_start_)->low, (gap_start_)->high, \
               (gap_bm_)->lead.low, (gap_bm_)->lead.high, \
               RTPS_Interface_get_bitmap_int_count((gap_bm_)->bit_count));

#define OSAPI_TRACE_RTPS_RCVD_GAP(intf_addr_,gap_start_,gap_bm_) \
        printf("RTPS_RCVD_GAP " TDBG_GUID_FMT \
               " START=(%d,%d) MAP.1st=(%d,%d), MAP.bits=%d\n",\
               TDBG_GUID_ARGS((intf_addr_)), \
               (gap_start_)->low, (gap_start_)->high, \
               (gap_bm_)->lead.low, (gap_bm_)->lead.high, \
               RTPS_Interface_get_bitmap_int_count((gap_bm_)->bit_count));

#endif /* 1 */

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
    OSAPI_Log_call_log_intf(create(OSAPI_PRECONDITION_PARAM,RTI_FALSE))\
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
    OSAPI_Log_call_log_intf(create(OSAPI_PRECONDITION_PARAM,RTI_FALSE))\
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
