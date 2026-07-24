/*
 * FILE: osapi_log.h - Logging API
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 02dec2014,tk MICRO-9763/PR#12922 Added documentation for log-entry use
 * 16sep2014,tk MICRO-887/PR#10752 Removed reference to removed function
 * 22sep2011,tk Updated
 * 23sep2008,yy Created
 *
 */
/*e
 * \file
 * \brief OSAPI Log API
 * \addtogroup OSAPILogCodesClass
 */
#ifndef osapi_log_h
#define osapi_log_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif /*  */

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif /*  */
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif /*  */

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                            OSAPI Error codes
 ******************************************************************************/

#ifndef RTI_MODULE_NAME
#define RTI_MODULE_NAME undefined
#endif


#define OSAPI_LOG_MSG_PN_X2_STD_PARAM OSAPI_CC_STRINGIFY_DEFINE(RTI_MODULE_NAME),\
                                        __FILE__,RTIME_FUNCTION_NAME, __LINE__

#if OSAPI_ENABLE_LOG

#define  OSAPI_Log_call_log_intf(call_) \
        OSPSL_Log_entry_##call_;

#define OSAPI_LOG_ENTRY_CREATE(kind_,error_code_,p_,is_finale_)\
    OSAPI_Log_call_log_intf(create(kind_,error_code_,p_,is_finale_))

#define OSAPI_LOG_ENTRY_ADD(kind_,error_code_,p_)\
    OSAPI_Log_call_log_intf(add(kind_,error_code_,p_))

#define OSAPI_LOG_ENTRY_ADD_INT(name_,value_,is_final_)\
    OSAPI_Log_call_log_intf(add_int(name_,(RTI_INT32)value_,is_final_))

#define OSAPI_LOG_ENTRY_ADD_UINT(name_,value_,is_final_)\
    OSAPI_Log_call_log_intf(add_uint(name_,(RTI_UINT32)value_,is_final_))

#define OSAPI_LOG_ENTRY_ADD_INT_HEX(name_,value_,is_final_)\
    OSAPI_Log_call_log_intf(add_int_hex(name_,(RTI_INT32)value_,is_final_))

#define OSAPI_LOG_ENTRY_ADD_STRING(name_,value_,is_final_)\
    OSAPI_Log_call_log_intf(add_string(name_,value_,is_final_))

#define OSAPI_LOG_ENTRY_ADD_POINTER(name_,value_,is_final_)\
    OSAPI_Log_call_log_intf(add_pointer(name_,value_,is_final_))

#define OSAPI_LOG_ENTRY_ADD_1STRING_1INT(kind_,error_code_,p_,s_name_,s_value_,i_name_,i_value_)\
    OSAPI_Log_call_log_intf(add_1string_1int(kind_,error_code_,p_,s_name_,s_value_,i_name_,(RTI_INT32)i_value_))

#define OSAPI_LOG_ENTRY_ADD_1POINTER(kind_,error_code_,p_,name_,value_)\
    OSAPI_Log_call_log_intf(add_1pointer(kind_,error_code_,p_,name_,value_))

#define OSAPI_LOG_ENTRY_ADD_1STRING(kind_,error_code_,p_,name_,value_)\
    OSAPI_Log_call_log_intf(add_1string(kind_,error_code_,p_,name_,value_))

#define OSAPI_LOG_ENTRY_ADD_2STRING(kind_,error_code_,p_,name1_,value1_,name2_,value2_)\
    OSAPI_Log_call_log_intf(add_2string(kind_,error_code_,p_,name1_,value1_,name2_,value2_))

#define OSAPI_LOG_ENTRY_ADD_1INT_HEX(kind_,error_code_,p_,name1_,value1_)\
        OSAPI_Log_call_log_intf(add_1int_hex(kind_,error_code_,p_,name1_,(RTI_INT32)value1_))

#define OSAPI_LOG_ENTRY_ADD_1INT(kind_,error_code_,p_,name1_,value1_)\
        OSAPI_Log_call_log_intf(add_1int(kind_,error_code_,p_,name1_,(RTI_INT32)value1_))

#define OSAPI_LOG_ENTRY_ADD_1UINT(kind_,error_code_,p_,name1_,value1_)\
        OSAPI_Log_call_log_intf(add_1uint(kind_,error_code_,p_,name1_,(RTI_UINT32)value1_))

#define OSAPI_LOG_ENTRY_ADD_2INT(kind_,error_code_,p_,name1_,value1_,name2_,value2_)\
    OSAPI_Log_call_log_intf(add_2int(kind_,error_code_,p_,name1_,(RTI_INT32)value1_,name2_,(RTI_INT32)value2_))

#define OSAPI_LOG_ENTRY_ADD_3INT(kind_,error_code_,p_,name1_,value1_,name2_,value2_,name3_,value3_)\
    OSAPI_Log_call_log_intf(add_3int(kind_,error_code_,p_,name1_,(RTI_INT32)value1_,name2_,(RTI_INT32)value2_,name3_,(RTI_INT32)value3_))

#define OSAPI_LOG_ENTRY_ADD_3STRING(kind_,error_code_,p_,\
                                name1_,value1_,\
                                name2_,value2_,\
                                name3_,value3_)\
OSAPI_Log_call_log_intf(add_3string(kind_,error_code_,p_,\
                            name1_,value1_,\
                            name2_,value2_,\
                            name3_,value3_))

#define OSAPI_LOG_ENTRY_ADD_1STRING_2INT(kind_,error_code_,p_,\
                                s_name_,s_value_,\
                                i_name1_,i_value1_,\
                                i_name2_,i_value2_)\
OSAPI_Log_call_log_intf(add_1string_2int(kind_,error_code_,p_,\
                                 s_name_,s_value_,\
                                 i_name1_,(RTI_INT32)i_value1_,\
                                 i_name2_,(RTI_INT32)i_value2_))
#else
#define OSAPI_LOG_ENTRY_CREATE(kind_,error_code_,p_,is_finale_)
#define OSAPI_LOG_ENTRY_ADD(kind_,error_code_,p_)
#define OSAPI_LOG_ENTRY_ADD_INT(name_,value_,is_final_)
#define OSAPI_LOG_ENTRY_ADD_UINT(name_,value_,is_final_)
#define OSAPI_LOG_ENTRY_ADD_INT_HEX(name_,value_,is_final_)
#define OSAPI_LOG_ENTRY_ADD_STRING(name_,value_,is_final_)
#define OSAPI_LOG_ENTRY_ADD_POINTER(name_,value_,is_final_)
#define OSAPI_LOG_ENTRY_ADD_1STRING_1INT(\
                            kind_,error_code_,p_,s_name_,s_value_,i_name_,i_value_)
#define OSAPI_LOG_ENTRY_ADD_1POINTER(kind_,error_code_,p_,name_,value_)
#define OSAPI_LOG_ENTRY_ADD_1STRING(kind_,error_code_,p_,name_,value_)
#define OSAPI_LOG_ENTRY_ADD_2STRING(kind_,error_code_,p_,name1_,value1_,name2_,value2_)
#define OSAPI_LOG_ENTRY_ADD_1INT_HEX(kind_,error_code_,p_,name1_,value1_)
#define OSAPI_LOG_ENTRY_ADD_1INT(kind_,error_code_,p_,name1_,value1_)
#define OSAPI_LOG_ENTRY_ADD_1UINT(kind_,error_code_,p_,name1_,value1_)
#define OSAPI_LOG_ENTRY_ADD_2INT(kind_,error_code_,p_,name1_,value1_,name2_,value2_)
#define OSAPI_LOG_ENTRY_ADD_3INT(kind_,error_code_,p_,name1_,value1_,name2_,value2_,name3_,value3_)
#define OSAPI_LOG_ENTRY_ADD_4INT(kind_,error_code_,p_,name1_,value1_,name2_,value2_,name3_,value3_,name4_,value4_)
#define OSAPI_LOG_ENTRY_ADD_3STRING(kind_,error_code_,p_,name1_,value1_,name2_,value2_,name3_,value3)
#define OSAPI_LOG_ENTRY_ADD_1STRING_2INT(kind_,error_code_,p_,s_name_,s_value_,i_name1_,i_value1_,i_name2_,i_value2_)

#define OSAPI_Log_finalize(void)

#endif

/******************************************************************************
 *                            OSAPI Log Modules
 ******************************************************************************/
/*e \def OSAPI_LOG_BASE
 *   \brief Log Id base-number
 */
#define OSAPI_LOG_BASE                                      (0)

/*e \def REDA_LOG_BASE
 *   \brief REDA Module
 */
#define REDA_LOG_BASE                                       (1 << 16)

/*e \def DB_LOG_BASE
 *   \brief DB Module
 */
#define DB_LOG_BASE                                         (2 << 16)

/*e \def RT_LOG_BASE
 *   \brief RT Module
 */
#define RT_LOG_BASE                                         (3 << 16)

/*e \def NETIO_LOG_BASE
 *   \brief NETIO Module
 */
#define NETIO_LOG_BASE                                      (4 << 16)

/*e \def UDP_LOG_BASE
 *   \brief UDP Module, same as NETIO
 */
#define UDP_LOG_BASE                                        NETIO_LOG_BASE

/*e \def SHMEM_LOG_BASE
 *  \brief NETIO_SHMEM Module, same as NETIO
 */
#define SHMEM_LOG_BASE                                    NETIO_LOG_BASE + 10000

/*e \def SDM_LOG_BASE
 *   \brief SDM Module, same as NETIO
 */
#define SDM_LOG_BASE                                      NETIO_LOG_BASE + 20000

/*e \def CDR_LOG_BASE
 *   \brief CDR Module
 */
#define CDR_LOG_BASE                                        (5 << 16)

/*e \def XCDR_LOG_BASE
 *   \brief CDR Module
 */
#define XCDR_LOG_BASE                                      CDR_LOG_BASE + 20000

/*e \def RTPS_LOG_BASE
 *   \brief RTPS Module
 */
#define RTPS_LOG_BASE                                       (6 << 16)

/*e \def DDSC_LOG_BASE
 *   \brief DDS_C Module
 */
#define DDSC_LOG_BASE                                       (7 << 16)

/*e \def RHSM_LOG_BASE
 *   \brief RHSM Module
 */
#define RHSM_LOG_BASE                                       (8 << 16)

/*e \def WHSM_LOG_BASE
 *   \brief WHSM Module
 */
#define WHSM_LOG_BASE                                       (9 << 16)

/*e \def DPSE_LOG_BASE
 *   \brief DPSE Module
 */
#define DPSE_LOG_BASE                                       (10 << 16)

/*e \def DPDE_LOG_BASE
 *   \brief DPDE Module
 */
#define DPDE_LOG_BASE                                       (11 << 16)

/*e \def SEC_CORE_LOG_BASE
 *   \brief DDDS Security plugin
 */
#define SEC_CORE_LOG_BASE                                   (12 << 16)

/*e \def APPGEN_LOG_BASE
 *   \brief APPGEN Module
 */
#define APPGEN_LOG_BASE                                     (13 << 16)

/*e \def NETIO_ZCOPY_LOG_BASE
 *   \brief NETIO Zero Copy Module
 */
#define NETIO_ZCOPY_LOG_BASE                                (14 << 16)

/*e \def DDS_FILTER_LOG_BASE
 *   \brief DDS Filter Module
 */
#define DDS_FILTER_LOG_BASE                                 (15 << 16)

/*e
 * \defgroup OSAPILogCodesClass OSAPI
 * \brief OSAPI. ModuleID = 0
 * \ingroup LoggingModule
 */
/*e
 * \brief Retrieve the next error code failed
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_GET_NEXT_OBJECT_ID_EC                     (OSAPI_LOG_BASE + 1)
#define OSAPI_LOG_GET_NEXT_OBJECT_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_GET_NEXT_OBJECT_ID_EC ,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/* System messages */

/*e
 * \brief An error occurred while setting the system properties
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SYSTEM_SET_PROPERTY_EC                    (OSAPI_LOG_BASE + 2)
#define OSAPI_LOG_SYSTEM_SET_PROPERTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SYSTEM_SET_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred when starting the system timer
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SYSTEM_TIMER_START_EC                     (OSAPI_LOG_BASE + 4)
#define OSAPI_LOG_SYSTEM_TIMER_START(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SYSTEM_TIMER_START_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred when stopping the system timer
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SYSTEM_TIMER_STOP_EC                      (OSAPI_LOG_BASE + 5)
#define OSAPI_LOG_SYSTEM_TIMER_STOP(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SYSTEM_TIMER_STOP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/* THREAD message */
/*e
 * \brief An error when allocating the a thread object
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_NEW_EC                             (OSAPI_LOG_BASE + 6)
#define OSAPI_LOG_THREAD_NEW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_THREAD_NEW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error when creating the thread object
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_CREATE_EC                          (OSAPI_LOG_BASE + 7)
#define OSAPI_LOG_THREAD_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_THREAD_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error when creating thread sync semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_SEM_EC                             (OSAPI_LOG_BASE + 8)
#define OSAPI_LOG_THREAD_SEM(level_,ss_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_THREAD_SEM_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("ss",(ss_),RTI_TRUE)

/*e
 * \brief Failed to signal that a thread has been created
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_EXEC_CREATE_EC                     (OSAPI_LOG_BASE + 9)
#define OSAPI_LOG_THREAD_EXEC_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_THREAD_EXEC_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to signal the start a created thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_EXEC_START_EC                     (OSAPI_LOG_BASE + 10)
#define OSAPI_LOG_THREAD_EXEC_START(level_,ss_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_THREAD_EXEC_START_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ss",(ss_))

/*e
 * \brief Failed to start a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_START_EC                          (OSAPI_LOG_BASE + 11)
#define OSAPI_LOG_THREAD_START(level_,ss_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_THREAD_START_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ss",(ss_))

/*e
 * \brief Failed to destroy a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_DESTROY_EC                        (OSAPI_LOG_BASE + 12)
#define OSAPI_LOG_THREAD_DESTROY(level_,ss_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_THREAD_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ss",(ss_))

/*e
 * \brief Failed to start an unstarted thread being destroyed
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_DESTROY_NO_START_EC               (OSAPI_LOG_BASE + 13)
#define OSAPI_LOG_THREAD_DESTROY_NO_START(level_,ss_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_THREAD_DESTROY_NO_START_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ss",(ss_))

/*e
 * \brief Failed to wake up a thread that's being destroyed
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP_EC              (OSAPI_LOG_BASE + 14)
#define OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(level_,ss_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ss",(ss_))

/*e
 * \brief Failed initializing a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_INIT_EC                           (OSAPI_LOG_BASE + 15)
#define OSAPI_LOG_THREAD_INIT(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_THREAD_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))


/*e
 * \brief Failed to set scheduling policy of a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_SCHEDPARAM_EC                     (OSAPI_LOG_BASE + 16)
#define OSAPI_LOG_THREAD_SCHEDPARAM(level_,sysrc_,prio_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),OSAPI_LOG_THREAD_SCHEDPARAM_EC ,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "sysrc",(sysrc_),"prio",(prio_))

/*e
 * \brief Failed to get the scheduling policy of a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_GET_POLICY_EC                     (OSAPI_LOG_BASE + 17)
#define OSAPI_LOG_THREAD_GET_POLICY(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_THREAD_GET_POLICY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Scheduling policy mismatch between a created thread and the application
 * thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_POLICY_DIFFER_EC                  (OSAPI_LOG_BASE + 18)
#define OSAPI_LOG_THREAD_POLICY_DIFFER(level_,get_policy_,set_policy_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),OSAPI_LOG_THREAD_POLICY_DIFFER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "get_policy",(get_policy_),"set_policy",(set_policy_))

/*e
 * \brief Failed to map to native thread priority values
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_PRIORITY_MAP_EC                   (OSAPI_LOG_BASE + 19)
#define OSAPI_LOG_THREAD_PRIORITY_MAP(level_,min_,max_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),OSAPI_LOG_THREAD_PRIORITY_MAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "min",(min_),"max",(max_))

/* TIMER messages */

/*e
 * \brief Failed to delete the Timer object
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_DELETE_EC                          (OSAPI_LOG_BASE + 20)
#define OSAPI_LOG_TIMER_DELETE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_TIMER_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#define OSAPI_LOG_TIMER_TICK_EC                            (OSAPI_LOG_BASE + 21)

/*e
 * \brief Failed taking or giving the Timer mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_TICK_MUTEX_EC                      (OSAPI_LOG_BASE + 22)
#define OSAPI_LOG_TIMER_TICK_MUTEX(level_,mutex_,take_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_TIMER_TICK_MUTEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_POINTER("mutex",(mutex_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("take",(take_),RTI_TRUE)

#define OSAPI_LOG_TIMER_CREATE_TIMEOUT_EC                  (OSAPI_LOG_BASE + 23)

#define OSAPI_LOG_TIMER_UPDATE_TIMEOUT_EC                  (OSAPI_LOG_BASE + 24)

#define OSAPI_LOG_TIMER_DELETE_TIMEOUT_EC                  (OSAPI_LOG_BASE + 25)


/*e
 * \brief Failed to return user data for a timeout due to mismatched epochs
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_GET_USER_DATA_EPOCH_EC             (OSAPI_LOG_BASE + 27)
#define OSAPI_LOG_TIMER_GET_USER_DATA_EPOCH(level_,h_,e_,e1_,e2_) \
OSAPI_LOG_ENTRY_CREATE( (level_),OSAPI_LOG_TIMER_GET_USER_DATA_EPOCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("h",(h_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("e",(e_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("e1",(e1_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("e2",(e2_),RTI_TRUE)
/*e
 * \brief Failed to allocate memory for a new Timer
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_NEW_EC                             (OSAPI_LOG_BASE + 28)
#define OSAPI_LOG_TIMER_NEW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_TIMER_NEW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for a new Timer entry
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_NEW_ENTRY_EC                       (OSAPI_LOG_BASE + 29)
#define OSAPI_LOG_TIMER_NEW_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_TIMER_NEW_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for a new Timer wheel
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_NEW_WHEEL_EC                       (OSAPI_LOG_BASE + 30)
#define OSAPI_LOG_TIMER_NEW_WHEEL(level_,slots_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_TIMER_NEW_WHEEL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"slots",(slots_))

/*e
 * \brief Failed to create a new Timer mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_NEW_MUTEX_EC                       (OSAPI_LOG_BASE + 31)
#define OSAPI_LOG_TIMER_NEW_MUTEX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_TIMER_NEW_MUTEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to start a new Timer being created
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_NEW_START_TIMER_EC                 (OSAPI_LOG_BASE + 32)
#define OSAPI_LOG_TIMER_NEW_START_TIMER(level_,timer_,ticr_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_TIMER_NEW_START_TIMER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("timer",(timer_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("ticr",(ticr_),RTI_TRUE)

/*e
 * \brief Failed to stop a Timer being deleted
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_DELETE_STOP_TIMER_EC               (OSAPI_LOG_BASE + 33)
#define OSAPI_LOG_TIMER_DELETE_STOP_TIMER(level_,timer_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_TIMER_DELETE_STOP_TIMER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"timer",(timer_))

/*e
 * \brief Failed to delete the Timer mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_DELETE_MUTEX_EC                    (OSAPI_LOG_BASE + 34)
#define OSAPI_LOG_TIMER_DELETE_MUTEX(level_,timer_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),OSAPI_LOG_TIMER_DELETE_MUTEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"timer",(timer_))

/*e
 * \brief Failed to take or give a Timer mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_TIMER_MUTEX_EC                           (OSAPI_LOG_BASE + 35)
#define OSAPI_LOG_TIMER_MUTEX(level_,mutex_,take_) \
        OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_TIMER_MUTEX_EC,OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("mutex",(mutex_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("take",(take_),RTI_TRUE)

/* Semaphore messages */

/*e
 * \brief Failed to delete a semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEMAPHORE_DELETE_EC                      (OSAPI_LOG_BASE + 36)
#define OSAPI_LOG_SEMAPHORE_DELETE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT( (level_),OSAPI_LOG_SEMAPHORE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to create a semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEMAPHORE_NEW_EC                         (OSAPI_LOG_BASE + 37)
#define OSAPI_LOG_SEMAPHORE_NEW(level_) \
OSAPI_LOG_ENTRY_ADD( (level_),OSAPI_LOG_SEMAPHORE_NEW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize a new semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEMAPHORE_NEW_INIT_EC                    (OSAPI_LOG_BASE + 38)
#define OSAPI_LOG_SEMAPHORE_NEW_INIT(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEMAPHORE_NEW_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to give a semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEMAPHORE_GIVE_EC                        (OSAPI_LOG_BASE + 39)
#define OSAPI_LOG_SEMAPHORE_GIVE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEMAPHORE_GIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to take a semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEMAPHORE_TAKE_EC                        (OSAPI_LOG_BASE + 40)
#define OSAPI_LOG_SEMAPHORE_TAKE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEMAPHORE_TAKE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/* Mutex messages */

/*e
 * \brief Failed to delete a mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_MUTEX_DELETE_EC                          (OSAPI_LOG_BASE + 41)
#define OSAPI_LOG_MUTEX_DELETE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_MUTEX_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to create a mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_MUTEX_NEW_EC                             (OSAPI_LOG_BASE + 42)
#define OSAPI_LOG_MUTEX_NEW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_MUTEX_NEW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to take a mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_MUTEX_TAKE_EC                            (OSAPI_LOG_BASE + 43)
#define OSAPI_LOG_MUTEX_TAKE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_MUTEX_TAKE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to give a mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_MUTEX_GIVE_EC                            (OSAPI_LOG_BASE + 44)
#define OSAPI_LOG_MUTEX_GIVE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_MUTEX_GIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to initialize a mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_MUTEX_INIT_EC                            (OSAPI_LOG_BASE + 45)
#define OSAPI_LOG_MUTEX_INIT(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_MUTEX_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/* HEAP */

/*e
 * \brief Failed to allocate a buffer from the heap
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_HEAP_INTERNAL_ALLOCATE_EC                (OSAPI_LOG_BASE + 46)
#define OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(level_,s_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),OSAPI_LOG_HEAP_INTERNAL_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "s",(s_))

#define OSAPI_LOG_HEAP_FREE_EC                             (OSAPI_LOG_BASE + 47)

/*e
 * \brief Failed to get current system time
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SYSTEM_GET_TIME_EC                       (OSAPI_LOG_BASE + 48)
#define OSAPI_LOG_SYSTEM_GET_TIME(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SYSTEM_GET_TIME_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Return the last recorded error-code for the calling thread
 *
 * \details
 * This log-messages retrieves the last recorded error-code for the
 * calling thread. It is used a function calls another function that
 * fails.
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_LAST_RECORDED_ERROR_EC                   (OSAPI_LOG_BASE + 49)
#define OSAPI_LOG_LAST_RECORDED_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_LAST_RECORDED_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",OSAPI_Log_get_last_error_code())

/*e
 * \brief Failed to set the thread name in the OS
 *
 * \details
 * When OSAPI starts a thread it also calls the OS to set the thread name.
 * If this fails this warning message indicates why.
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SET_THREAD_NAME_EC                       (OSAPI_LOG_BASE + 50)
#define OSAPI_LOG_SET_THREAD_NAME(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SET_THREAD_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*
 * \brief Failed to create shared memory segment
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEGMENT_CREATED_EC                       (OSAPI_LOG_BASE + 51)
#define OSAPI_LOG_SEGMENT_CREATED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEGMENT_CREATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during call VxWorks semOpen(...) or Posix sem_open(...)
 *         when managing shared memory segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEM_OPEN_EC                              (OSAPI_LOG_BASE + 52)
#define OSAPI_LOG_SEM_OPEN(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEM_OPEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during VxWorks call semInfoGet when managing shared memory
 *        segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEM_INFO_GET_EC                          (OSAPI_LOG_BASE + 53)
#define OSAPI_LOG_SEM_INFO_GET(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEM_INFO_GET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during VxWorks call sdUnmap when managing shared memory
 *        segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SD_UNMAP_EC                              (OSAPI_LOG_BASE + 54)
#define OSAPI_LOG_SD_UNMAP(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SD_UNMAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during VxWorks call sdOpen when managing shared memory
 *        segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SD_OPEN_EC                               (OSAPI_LOG_BASE + 55)
#define OSAPI_LOG_SD_OPEN(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SD_OPEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during VxWorks call semGive when managing shared memory
 *        segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEM_GIVE_EC                              (OSAPI_LOG_BASE + 56)
#define OSAPI_LOG_SEM_GIVE(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEM_GIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Failure during VxWorks call semTake when managing shared memory
 *        segments, semaphores, or mutexes
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEM_TAKE_EC                              (OSAPI_LOG_BASE + 57)
#define OSAPI_LOG_SEM_TAKE(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEM_TAKE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_))

/*e
 * \brief Trying to perform an operation an unknown semmutex type
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_UNKNOWN_SEMAPHORE_TYPE_EC                (OSAPI_LOG_BASE + 58)
#define OSAPI_LOG_UNKNOWN_SEMAPHORE_TYPE(level_,type_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_UNKNOWN_SEMAPHORE_TYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"type",(type_))

/*e
 * \brief Failed to create segment due to a segment already existing
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEGMENT_KEY_ALREADY_EXISTS_EC            (OSAPI_LOG_BASE + 59)
#define OSAPI_LOG_SEGMENT_KEY_ALREADY_EXISTS(level_,key_) \
OSAPI_LOG_ENTRY_ADD_1INT_HEX((level_),OSAPI_LOG_SEGMENT_KEY_ALREADY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"key",(key_))

/*e
 * \brief Segment trying to be attached to does not exist
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEGMENT_KEY_DOESNT_EXIST_EC              (OSAPI_LOG_BASE + 60)
#define OSAPI_LOG_SEGMENT_KEY_DOESNT_EXIST(level_,key_) \
OSAPI_LOG_ENTRY_ADD_1INT_HEX((level_),OSAPI_LOG_SEGMENT_KEY_DOESNT_EXIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"key",(key_))

/*e
 * \brief Failed to detach from shared memory segment
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SEGMENT_DETACH_FAILED_EC                 (OSAPI_LOG_BASE + 61)
#define OSAPI_LOG_SEGMENT_DETACH_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEGMENT_DETACH_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"detach failed",(ec_))
/*e
 * \brief Attaching to shared memory segment/mutex/signaling semaphore failed
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_ATTACH_FAILED_EC                   (OSAPI_LOG_BASE + 62)
#define OSAPI_LOG_SHMEM_ATTACH_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_ATTACH_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"attach failed",(ec_))

/*e
 * \brief Failed to give shared memory mutex/signaling semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_GIVE_FAILED_EC                     (OSAPI_LOG_BASE + 63)
#define OSAPI_LOG_SHMEM_GIVE_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_GIVE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"give/take failed",(ec_))

/*e
 * \brief Failed to take shared memory mutex/signaling semaphore
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_TAKE_FAILED_EC                     (OSAPI_LOG_BASE + 64)
#define OSAPI_LOG_SHMEM_TAKE_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_TAKE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"give/take failed",(ec_))

/*e
 * \brief Failed to delete shared memory semaphore or mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SEM_DELETE_FAILED_EC               (OSAPI_LOG_BASE + 65)
#define OSAPI_LOG_SHMEM_SEM_DELETE_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SEM_DELETE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sem_mutex del failed",(ec_))

/*e
 * \brief Error during shared memory semaphore creation/deletion
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED_EC               (OSAPI_LOG_BASE + 66)
#define OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED(level_, ec_, semtype_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"error code",(ec_),"semtype", (semtype_))

/*e
 * \brief Windows specific API MapViewOfFile failed
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED_EC         (OSAPI_LOG_BASE + 67)
#define OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"mapofviewfile failed",(ec_))

/*e
 * \brief Windows specific error during setting security descriptor
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SET_DESCRIPTOR_EC                  (OSAPI_LOG_BASE + 68)
#define OSAPI_LOG_SHMEM_SET_DESCRIPTOR(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SET_DESCRIPTOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Windows specific error during initialization of security descriptor
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_INIT_DESCRIPTOR_EC                 (OSAPI_LOG_BASE + 69)
#define OSAPI_LOG_SHMEM_INIT_DESCRIPTOR(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_INIT_DESCRIPTOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during ftruncate(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_FTRUNCATE_EC                       (OSAPI_LOG_BASE + 70)
#define OSAPI_LOG_SHMEM_FTRUNCATE(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_FTRUNCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during mmap(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_MMAP_EC                            (OSAPI_LOG_BASE + 71)
#define OSAPI_LOG_SHMEM_MMAP(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_MMAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during shm_open(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SHM_OPEN_EC                        (OSAPI_LOG_BASE + 72)
#define OSAPI_LOG_SHMEM_SHM_OPEN(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SHM_OPEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during munmap(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_MUNMAP_EC                          (OSAPI_LOG_BASE + 73)
#define OSAPI_LOG_SHMEM_MUNMAP(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_MUNMAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during close(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_CLOSE_EC                                 (OSAPI_LOG_BASE + 74)
#define OSAPI_LOG_CLOSE(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_CLOSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during shared memory unlink(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_UNLINK_EC                          (OSAPI_LOG_BASE + 76)
#define OSAPI_LOG_SHMEM_UNLINK(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_UNLINK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief System ran out of space to create semaphores. Consider increasing
 *        Posix sem limit
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_EXHAUSTED_POSIX_SEM_LIMIT_EC             (OSAPI_LOG_BASE + 77)
#define OSAPI_LOG_EXHAUSTED_POSIX_SEM_LIMIT(level_) \
        OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_EXHAUSTED_POSIX_SEM_LIMIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Posix specific error during shared memory wait call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SEM_WAIT_EC                        (OSAPI_LOG_BASE + 78)
#define OSAPI_LOG_SHMEM_SEM_WAIT(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SEM_WAIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during shared memory sem unlink call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SEM_UNLINK_EC                      (OSAPI_LOG_BASE + 79)
#define OSAPI_LOG_SHMEM_SEM_UNLINK(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_SEM_UNLINK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Posix specific error during os specific implementation of a semaphore
 *        mutex give.
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_POSIX_GIVE_EC                      (OSAPI_LOG_BASE + 80)
#define OSAPI_LOG_SHMEM_POSIX_GIVE(level_,fnname_, ec_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),\
        OSAPI_LOG_SHMEM_POSIX_GIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"fname", (fnname_), "ec",(ec_))

/*e
 * \brief Failed to add a semaphore to the thread semaphore pool.
 *
 * \details
 * This maximum number of semaphores is configured with
 * \idref_OSAPI_SystemProperty_max_user_blocking_threads. One semaphore
 * is needed per receive thread created.
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_SEMPOOL_EXCEEDED_EC               (OSAPI_LOG_BASE + 90)
#define OSAPI_LOG_THREAD_SEMPOOL_EXCEEDED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_THREAD_SEMPOOL_EXCEEDED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a semaphore from the thread semaphore pool
 *
 * \details
 * This is error indicates an imbalanced number of add/delete calls to the
 * thread semaphore pool.
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_SEMPOOL_INVALID_SEM_EC            (OSAPI_LOG_BASE + 91)
#define OSAPI_LOG_THREAD_SEMPOOL_INVALID_SEM(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_THREAD_SEMPOOL_INVALID_SEM_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Trying to create a shared memory segment that is greater than
 * (2 ^ 31) - 1. The middleware cannot support shared memory segments created
 * than ~ 2 GB even though the operating system may.
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_SIZE_EXCEEDED_EC                   (OSAPI_LOG_BASE + 92)
#define OSAPI_LOG_SHMEM_SIZE_EXCEEDED(level_,size_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),OSAPI_LOG_SHMEM_SIZE_EXCEEDED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"size",(size_))

/*e
 * \brief A math overflow occurred
 */
#define OSAPI_LOG_MATH_OFV_EC                              (OSAPI_LOG_BASE + 93)
#define OSAPI_LOG_MATH_OFV(level_,a_,b_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),OSAPI_LOG_MATH_OFV_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"a",(a_),"b",(b_))

/*e
 * \brief An invalid timer resolution was specified
 */
#define OSAPI_LOG_INVALID_TIMER_RES_EC                     (OSAPI_LOG_BASE + 94)
#define OSAPI_LOG_INVALID_TIMER_RES(level_,res_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_INVALID_TIMER_RES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"res",(res_))

/*e
 * \brief Failed to join a thread
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_JOIN_EC                           (OSAPI_LOG_BASE + 95)
#define OSAPI_LOG_THREAD_JOIN(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_THREAD_JOIN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief Failed to get the maximum priority value for a scheduling policy
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_THREAD_GET_MAX_PRIORITY_EC               (OSAPI_LOG_BASE + 96)
#define OSAPI_LOG_THREAD_GET_MAX_PRIORITY(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_THREAD_GET_MAX_PRIORITY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"sysrc",(sysrc_))

/*e
 * \brief POSIX specific error during fstat(...) call
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_FSTAT_EC                           (OSAPI_LOG_BASE + 97)
#define OSAPI_LOG_SHMEM_FSTAT(level_,ec_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SHMEM_FSTAT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"ec",(ec_))

/*e
 * \brief Timed out waiting for a shared memory mutex to be initialized
 *
 * \details This condition could occur if a process dies while initializing
 *          a shared memory mutex segment. The segment containing the mutex
 *          must be manually removed before it can be reinitialized.
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_SHMEM_MUTEX_INIT_TIMEOUT_EC              (OSAPI_LOG_BASE + 97)
#define OSAPI_LOG_SHMEM_MUTEX_INIT_TIMEOUT(level_,key_) \
OSAPI_LOG_ENTRY_ADD_1INT_HEX((level_),OSAPI_LOG_SHMEM_MUTEX_INIT_TIMEOUT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"key",(key_))

/*e
 * \brief Autosar SetEvent system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_SETEVENT_CALL_EC                 (OSAPI_LOG_BASE + 100)
#define OSAPI_LOG_AUTOSAR_SETEVENT_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_SETEVENT_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar ClearEvent system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_CLEAREVENT_CALL_EC               (OSAPI_LOG_BASE + 101)
#define OSAPI_LOG_AUTOSAR_CLEAREVENT_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_CLEAREVENT_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar WaitEvent system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_WAITEVENT_CALL_EC               (OSAPI_LOG_BASE + 102)
#define OSAPI_LOG_AUTOSAR_WAITEVENT_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_WAITEVENT_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar GetEvent system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_GETEVENT_CALL_EC                (OSAPI_LOG_BASE + 103)
#define OSAPI_LOG_AUTOSAR_GETEVENT_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_GETEVENT_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar TerminateTask system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_TERMINATETASK_CALL_EC           (OSAPI_LOG_BASE + 104)
#define OSAPI_LOG_AUTOSAR_TERMINATETASK_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_TERMINATETASK_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar ActivateTask system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_ACTIVATETASK_CALL_EC           (OSAPI_LOG_BASE + 105)
#define OSAPI_LOG_AUTOSAR_ACTIVATETASK_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_ACTIVATETASK_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar Schedule system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_SCHEDULE_CALL_EC                (OSAPI_LOG_BASE + 106)
#define OSAPI_LOG_AUTOSAR_SCHEDULE_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_SCHEDULE_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar SetRelAlarm system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_SETRELALARM_CALL_EC             (OSAPI_LOG_BASE + 107)
#define OSAPI_LOG_AUTOSAR_SETRELALARM_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_SETRELALARM_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/*e
 * \brief Autosar CancelAlarm system call failed
 *
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_AUTOSAR_CANCELALARM_CALL_EC             (OSAPI_LOG_BASE + 108)
#define OSAPI_LOG_AUTOSAR_CANCELALARM_CALL(level_,sysrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_AUTOSAR_CANCELALARM_CALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("sysrc",(sysrc_),RTI_TRUE)

/******************************************************************************
 *                                     SHMEM
 ******************************************************************************/

 /*e
 * \brief Shared memory segment already exists.
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Failed to create a shared memory segment because a segment with the
 * specified name already exists in the system.
 */
#define OSAPI_LOG_SEGMENT_ALREADY_EXISTS_EC               (OSAPI_LOG_BASE + 150)
#define OSAPI_LOG_SEGMENT_ALREADY_EXISTS(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEGMENT_ALREADY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Shared memory segment does not exist.
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Failed to attach to a shared memory segment because a segment with the
 * specified name does not exist in the system.
 */
#define OSAPI_LOG_SEGMENT_DOESNT_EXIST_EC                 (OSAPI_LOG_BASE + 151)
#define OSAPI_LOG_SEGMENT_DOESNT_EXIST(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_SEGMENT_DOESNT_EXIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Invalid shared memory segment name.
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * The specified shared memory segment name is invalid. The name may be
 * too long, empty, or contain invalid characters for the platform.
 */
#define OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME_EC           (OSAPI_LOG_BASE + 152)
#define OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Invalid shared memory segment mode
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * The specified shared memory segment mode is invalid or incompatible
 * with the requested operation.
 */
#define OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE_EC           (OSAPI_LOG_BASE + 153)
#define OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE(level_,name_,mode_) \
OSAPI_LOG_ENTRY_CREATE((level_),OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("mode",(mode_),RTI_TRUE)

/*e
 * \brief Shared memory segment does not support locking
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Attempted to perform a lock operation on a shared memory segment that
 * was not created with lockable mode.
 */
#define OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE_EC           (OSAPI_LOG_BASE + 154)
#define OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Shared memory segment does not support robust mutexes
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Attempted to perform a robust mutex operation on a shared memory segment
 * that was not created with robust mode, or the platform does not support
 * robust mutexes.
 */
#define OSAPI_LOG_SHMEM_SEGMENT_NOT_ROBUST_EC             (OSAPI_LOG_BASE + 155)
#define OSAPI_LOG_SHMEM_SEGMENT_NOT_ROBUST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SHMEM_SEGMENT_NOT_ROBUST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Shared memory segment is already attached
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Cannot attach to a shared memory segment because the handle is already
 * attached to a segment. Detach from the current segment before attaching
 * to another.
 */
#define OSAPI_LOG_SHMEM_ALREADY_ATTACHED_EC               (OSAPI_LOG_BASE + 156)
#define OSAPI_LOG_SHMEM_ALREADY_ATTACHED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SHMEM_ALREADY_ATTACHED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Shared memory segment is not attached
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Cannot perform the requested operation because the handle is not
 * currently attached to a shared memory segment.
 */
#define OSAPI_LOG_SHMEM_NOT_ATTACHED_EC                   (OSAPI_LOG_BASE + 157)
#define OSAPI_LOG_SHMEM_NOT_ATTACHED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),OSAPI_LOG_SHMEM_NOT_ATTACHED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Checking if shared memory segment has been unlinked
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Information message indicating that the system is checking whether
 * the specified shared memory segment has been unlinked.
 */
#define OSAPI_LOG_SHMEM_CHECK_UNLINKED_EC                 (OSAPI_LOG_BASE + 158)
#define OSAPI_LOG_SHMEM_CHECK_UNLINKED(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),OSAPI_LOG_SHMEM_CHECK_UNLINKED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Shared memory segment is not initialized
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Failed to attach to a shared memory segment because it has not been
 * properly initialized by the creator. This may occur if the creator
 * process terminated prematurely or if the maximum wait time for
 * initialization was exceeded.
 */
#define OSAPI_SHMEM_SEGMENT_NOT_INITIALIZED_EC            (OSAPI_LOG_BASE + 159)
#define OSAPI_SHMEM_SEGMENT_NOT_INITIALIZED(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),OSAPI_SHMEM_SEGMENT_NOT_INITIALIZED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Shared memory segment with specified name already exists
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Failed to create a shared memory segment because a segment with the
 * specified name already exists in the system. This is an informational
 * message variant that includes the segment name.
 */
#define OSAPI_LOG_SHMEM_SEGMENT_NAME_ALREADY_EXISTS_EC    (OSAPI_LOG_BASE + 160)
#define OSAPI_LOG_SHMEM_SEGMENT_NAME_ALREADY_EXISTS(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),OSAPI_LOG_SEGMENT_ALREADY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Shared memory segment with specified name does not exist
 * \ingroup OSAPILogCodesClass
 *
 * \details
 * Not currently used. Reserved for future use to maintain numbering consistency.
 */
#define OSAPI_LOG_SHMEM_SEGMENT_NAME_DOESNT_EXIST_EC      (OSAPI_LOG_BASE + 161)
#define OSAPI_LOG_SHMEM_SEGMENT_NAME_DOESNT_EXIST(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),OSAPI_LOG_SEGMENT_DOESNT_EXIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/******************************************************************************
 *                                     PTHREAD
 ******************************************************************************/

/*e
 * \brief Failed to create a pthread mutex attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_INIT_EC               (OSAPI_LOG_BASE + 200)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_INIT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the pthread mutex attribute to be process shared
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED_EC         (OSAPI_LOG_BASE + 201)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the pthread mutex attribute to be robust
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST_EC          (OSAPI_LOG_BASE + 202)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to destroy the pthread mutex attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY_EC            (OSAPI_LOG_BASE + 203)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to initialize a pthread mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEX_INIT_EC                   (OSAPI_LOG_BASE + 204)
#define OSAPI_LOG_PTHREAD_MUTEX_INIT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEX_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to destroy a pthread mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEX_DESTROY_EC                (OSAPI_LOG_BASE + 205)
#define OSAPI_LOG_PTHREAD_MUTEX_DESTROY(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEX_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to lock a pthread mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEX_LOCK_EC                   (OSAPI_LOG_BASE + 206)
#define OSAPI_LOG_PTHREAD_MUTEX_LOCK(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEX_LOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to unlock a pthread mutex
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEX_UNLOCK_EC                 (OSAPI_LOG_BASE + 207)
#define OSAPI_LOG_PTHREAD_MUTEX_UNLOCK(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEX_UNLOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the pthread mutex to be consistent
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT_EC             (OSAPI_LOG_BASE + 208)
#define OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to create a pthread condition variable attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_CONDATTR_INIT_EC                (OSAPI_LOG_BASE + 209)
#define OSAPI_LOG_PTHREAD_CONDATTR_INIT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_CONDATTR_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the pthread condition variable attribute to be process shared
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_CONDATTR_SETPSHARED_EC          (OSAPI_LOG_BASE + 210)
#define OSAPI_LOG_PTHREAD_CONDATTR_SETPSHARED(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_CONDATTR_SETPSHARED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to destroy the pthread condition variable attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_CONDATTR_DESTROY_EC             (OSAPI_LOG_BASE + 211)
#define OSAPI_LOG_PTHREAD_CONDATTR_DESTROY(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_CONDATTR_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to initialize a pthread condition variable
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_COND_INIT_EC                    (OSAPI_LOG_BASE + 212)
#define OSAPI_LOG_PTHREAD_COND_INIT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_COND_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to destroy a pthread condition variable
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_COND_DESTROY_EC                 (OSAPI_LOG_BASE + 213)
#define OSAPI_LOG_PTHREAD_COND_DESTROY(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_COND_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to wait on a pthread condition variable
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_COND_WAIT_EC                    (OSAPI_LOG_BASE + 214)
#define OSAPI_LOG_PTHREAD_COND_WAIT(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_COND_WAIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to signal a pthread condition variable
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_COND_SIGNAL_EC                  (OSAPI_LOG_BASE + 215)
#define OSAPI_LOG_PTHREAD_COND_SIGNAL(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_COND_SIGNAL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the type of a pthread mutex attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETTYPE_EC            (OSAPI_LOG_BASE + 216)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETTYPE(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_SETTYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the protocol of a pthread mutex attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPROTOCOL_EC        (OSAPI_LOG_BASE + 217)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPROTOCOL(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_SETPROTOCOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/*e
 * \brief Failed to set the priority ceiling of a pthread mutex attribute
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPRIOCEILING_EC     (OSAPI_LOG_BASE + 218)
#define OSAPI_LOG_PTHREAD_MUTEXATTR_SETPRIOCEILING(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_PTHREAD_MUTEXATTR_SETPRIOCEILING_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/******************************************************************************
 *                                     Windows
 ******************************************************************************/
/*e
 * \brief WaitForSingleObject returned an error while waiting for the timer
 * semaphore to time out. Micro continues to run, but it may indicate a
 * platform problem.
 * \ingroup OSAPILogCodesClass
 */
#define OSAPI_LOG_WIN_WAIT_FAILED_EC                      (OSAPI_LOG_BASE + 300)
#define OSAPI_LOG_WIN_WAIT_FAILED(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),OSAPI_LOG_WIN_WAIT_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"rc",(rc_))

/******************************************************************************/

#if OSAPI_ENABLE_LOG

/******************************************************************************
 *                                Log types
 ******************************************************************************/
/*ci
 * \ingroup OSAPILogClass
 * \brief Log message as an error
 */
typedef enum
{
    OSAPI_LOGKIND_ERROR   = 0,
    OSAPI_LOGKIND_WARNING,
    OSAPI_LOGKIND_INFO,
    OSAPI_LOGKIND_PRECONDITION
} OSAPI_LogKind_T;

typedef RTI_UINT32 OSAPI_LogEntryHeader_T;
typedef RTI_UINT32 OSAPI_LogEntryPayloadElement_T;

/*  31  30  29   28-27   26-16   15 14-0
 * +---+---+---+------+--------+--------+
 * | X | E | T | TYPE | MODULE | F | EC |
 * +---+---+---+------+--------+--------+
 */
#define OSAPI_LOG_HEADER_GET_X(hdr_)      ((hdr_)>>31U)
#define OSAPI_LOG_HEADER_GET_E(hdr_)      ((hdr_)>>30U & 0x1U)
#define OSAPI_LOG_HEADER_GET_T(hdr_)      ((hdr_)>>29U & 0x1U)
#define OSAPI_LOG_HEADER_GET_TYPE(hdr_)   (((hdr_) >>27U) & 0x3U)
#define OSAPI_LOG_HEADER_GET_MODULE(hdr_) (((hdr_) >>16U) & 0x7ffU)
#define OSAPI_LOG_HEADER_GET_EC(hdr_)     ((hdr_) & 0x7fffU)
#define OSAPI_LOG_HEADER_GET_F(hdr_)      ((hdr_)>>15U & 0x1U)

#define OSAPI_LOG_HEADER_SET_X(hdr_)          ((hdr_) |= 0x80000000U)
#define OSAPI_LOG_HEADER_CLR_X(hdr_)          ((hdr_) &= ~0x80000000U)
#define OSAPI_LOG_HEADER_SET_E(hdr_)          ((hdr_) |= 0x40000000U)
#define OSAPI_LOG_HEADER_SET_T(hdr_)          ((hdr_) |= 0x20000000U)
#define OSAPI_LOG_HEADER_CLR_T(hdr_)          ((hdr_) &= ~0x20000000U)
#define OSAPI_LOG_HEADER_SET_TYPE(hdr_,type_) ((hdr_) |= ((type_) << 27U))
#define OSAPI_LOG_HEADER_SET_MODULE(hdr_,m_)  ((hdr_) |= ((m_) << 16U))
#define OSAPI_LOG_HEADER_SET_EC(hdr_,ec)      ((hdr_) |= (ec_))
#define OSAPI_LOG_HEADER_SET_F(hdr_)          ((hdr_) |= 0x8000U)
#define OSAPI_LOG_HEADER_CLR_F(hdr_)          ((hdr_) &= ~0x8000U)

#define OSAPI_LOG_STATUS_MN (0x80000000U)
#define OSAPI_LOG_STATUS_SF (0x40000000U)
#define OSAPI_LOG_STATUS_FN (0x20000000U)
#define OSAPI_LOG_STATUS_LN (0x10000000U)
#define OSAPI_LOG_STATUS_F  (0x08000000U)

#define OSAPI_LOG_STATUS_ALL \
        (OSAPI_LOG_STATUS_MN | \
         OSAPI_LOG_STATUS_SF | \
         OSAPI_LOG_STATUS_FN | \
         OSAPI_LOG_STATUS_LN)

#define OSAPI_LOG_STATUS_NO_DATA \
        (OSAPI_LOG_STATUS_MN | \
         OSAPI_LOG_STATUS_SF | \
         OSAPI_LOG_STATUS_FN | \
         OSAPI_LOG_STATUS_LN)

typedef enum
{
    OSAPI_LOGTYPE_INTEGER,
    OSAPI_LOGTYPE_UINTEGER,
    OSAPI_LOGTYPE_HEX,
    OSAPI_LOGTYPE_STRING,
    OSAPI_LOGTYPE_POINTER
} OSAPI_LogType_T;

typedef struct OSAPI_LogDataEntry
{
    OSAPI_LogType_T type;
    char *name;
    union
    {
        RTI_INT32 int_value;
        char *string_value;
        void *ptr_value;
    } value;
} OSAPI_LogDataEntry_T;

#define OSAPI_LOGDATA_SET_F(hdr_)          ((hdr_) |= 0x80000000U)
#define OSAPI_LOGDATA_GET_F(hdr_)          ((hdr_)>>31U & 0x1U)
#define OSAPI_LOGDATA_SET_TYPE(hdr_,type_) ((hdr_) |= ((type_)<<27U))
#define OSAPI_LOGDATA_GET_TYPE(hdr_)       (((hdr_)>>27U) & 0xfU)

#if OSAPI_ENABLE_TRACE
typedef enum
{
    OSAPI_TRACETYPE_HEADER,
    OSAPI_TRACETYPE_INT32,
    OSAPI_TRACETYPE_PTR,
    OSAPI_TRACETYPE_STRING,
    OSAPI_TRACETYPE_GUID,
    OSAPI_TRACETYPE_V4_AS_INT32,
    OSAPI_TRACETYPE_V12_AS_INT32
} OSAPI_TraceType_T;
#endif

/*
 * \brief A log-entry in the log-buffer
 *
 * \details
 * Each log-entry in the log-buffer has a fixed sized buffer followed
 * by a variable length part that depends on the log-information stored.
 * The following notation is commonly used in the code:
 *
 * data_ptr = (char*)&log_entry[1]
 *
 * The data-pointer now points to the beginning of the variable length
 * data part that follows the fixed size header.
 *
 * \verbatim
 * +-----------------+ Log-buffer
 * |  OSAPI_LogEntry |
 * |                 |
 * +- - - - - - - - -+ <- data_ptr
 * |                 |
 * | log-data        |
 * |                 |
 * +-----------------+ Log-buffer
 * |  OSAPI_LogEntry |
 * |                 |
 * +- - - - - - - - -+
 * |                 |
 * | log-data        |
 * |                 |
 * +-----------------+
 *
 * \endverbatim
 */

/*ce \dref_OSAPI_LogEntry
 */
typedef struct OSAPI_LogEntry
{
    /*ci
     * \brief time this message was logged
     */
    OSAPI_SystemTime timestamp;

    /*ci
     * \brief The error code
     */
    RTI_UINT32 error_code;

} OSAPI_LogEntry_T;


/*e \dref_OSAPI_LogVerbosity_T
 */
typedef enum
{
    /*e \brief Logs are not written
     */
    OSAPI_LOG_VERBOSITY_SILENT  = 0,

    /*e \brief Only error logs are written
     */
    OSAPI_LOG_VERBOSITY_ERROR   = 1,

    /*e \brief Error and warning logs are written
     */
    OSAPI_LOG_VERBOSITY_WARNING = 2,

    /*e \brief All logs are written
     */
    OSAPI_LOG_VERBOSITY_DEBUG   = 3
} OSAPI_LogVerbosity_T;

/*e \dref_OSAPI_Log_write_buffer_T
 */
typedef void
(*OSAPI_Log_write_buffer_T)(const char *buffer,RTI_SIZE_T length);

/*e \dref_OSAPI_LogProperty
 */
struct OSAPI_LogProperty
{
    /*e \dref_OSAPI_LogProperty_max_buffer_size
     */
    RTI_SIZE_T max_buffer_size;

    /*e \dref_OSAPI_LogProperty_log_detail
     */
    RTI_UINT32 log_detail;

    /*e \dref_OSAPI_LogProperty_write_buffer
     */
    OSAPI_Log_write_buffer_T write_buffer;
};


/*e \dref_OSAPI_LOG_DETAIL_MODULENAME
 */
#define OSAPI_LOG_DETAIL_MODULENAME                             (0x80000000UL)

/*e \dref_OSAPI_LOG_DETAIL_SOURCEFILE
 */
#define OSAPI_LOG_DETAIL_SOURCEFILE                             (0x40000000UL)

/*e \dref_OSAPI_LOG_DETAIL_LINENUMBER
 */
#define OSAPI_LOG_DETAIL_LINENUMBER                             (0x20000000UL)

/*e \dref_OSAPI_LOG_DETAIL_FUNCTIONAME
 */
#define OSAPI_LOG_DETAIL_FUNCTIONAME                            (0x10000000UL)

#define OSAPI_LOG_DETAIL_FORMAT                                 (0x08000000UL)

/*e \dref_OSAPI_LOG_DETAIL_DATA_ONLY
 */
#define OSAPI_LOG_DETAIL_DATA_ONLY                              (0x04000000UL)

/*e \dref_OSAPI_LOG_DETAIL_ALL
 */
#define OSAPI_LOG_DETAIL_ALL \
        (OSAPI_LOG_DETAIL_MODULENAME | \
         OSAPI_LOG_DETAIL_SOURCEFILE | \
         OSAPI_LOG_DETAIL_LINENUMBER | \
         OSAPI_LOG_DETAIL_FUNCTIONAME | \
         OSAPI_LOG_DETAIL_FORMAT | \
         OSAPI_LOG_DETAIL_DATA_ONLY)

/*e \dref_OSAPI_LOG_DETAIL_NO_DATA
 */
#define OSAPI_LOG_DETAIL_NO_DATA \
        (OSAPI_LOG_DETAIL_MODULENAME | \
         OSAPI_LOG_DETAIL_SOURCEFILE | \
         OSAPI_LOG_DETAIL_LINENUMBER | \
         OSAPI_LOG_DETAIL_FUNCTIONAME )

#if OSAPI_ENABLE_PRECONDITION
#define OSAPI_LOG_BUFFER_SIZE (16384)
#else
#define OSAPI_LOG_BUFFER_SIZE (128)
#endif

/*e \dref_OSAPI_LogProperty_INITIALIZER
 */
#define OSAPI_LogProperty_INITIALIZER \
{\
    OSAPI_LOG_BUFFER_SIZE,\
    OSAPI_LOG_DETAIL_ALL,\
    NULL\
}

/* backwards compatability for applications using the misspelled macro */
#define OSAPI_LogProperty_INIITALIZER OSAPI_LogProperty_INITIALIZER

#if OSAPI_ENABLE_TRACE
/*i \dref_OSAPI_TraceHandler_T
 */
typedef void
(*OSAPI_TraceHandler_T)(RTI_UINT32 trace_mask,
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
                        RTI_BOOL is_final);

OSAPIDllVariable extern OSAPI_TraceHandler_T OSAPI_gv_TraceFunction;
OSAPIDllVariable extern void* OSAPI_gv_TraceFunctionParam;
OSAPIDllVariable extern RTI_UINT32 OSAPI_gv_TraceMask;
#endif

/*e
 * \ingroup OSAPILogClass
 * \brief Optional user-defined function for processing new log messages
 *
 * \details
 *
 * Definition of a function that can be installed with the logger and
 * that will be called for each new log event.
 *
 * The handler is set by OSAPI_Log_set_log_handler().
 *
 * The function is called while holding a global lock.
 *
 * When called, provides parameters containing the raw log entry and additional
 * optional information.
 *
 * \param[in] param User-defined parameter
 * \param[in] entry Log entry
 */
typedef void
(*OSAPI_LogHandler_T)(void *param,OSAPI_LogEntry_T *entry);

/*e \brief Optional user-defined function for processing log messages for display
 *
 * \details
 *
 * Definition of a function that can be installed with the logger and
 * that will be called for each new log event.
 *
 * The function is called while holding a global lock.
 *
 * The handler is set by OSAPI_Log_set_display_handler().
 *
 * When called, provides parameters containing the raw log entry and additional
 * optional information.
 *
 * \param[in] param User-defined parameter
 * \param[in] entry Log entry
 */
typedef void
(*OSAPI_LogDisplay_T)(void *param,OSAPI_LogEntry_T *entry);

OSAPIDllVariable extern OSAPI_LogHandler_T OSAPI_gv_LogFunction;
OSAPIDllVariable extern void* OSAPI_gv_LogFunctionParam;

OSAPIDllVariable extern OSAPI_LogDisplay_T OSAPI_gv_LogDisplayFunction;
OSAPIDllVariable extern void* OSAPI_gv_LogDisplayFunctionParam;

/*e \dref_OSAPI_Log_set_log_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_set_log_handler(OSAPI_LogHandler_T handler,void *param);

/*e \dref_OSAPI_Log_get_log_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_get_log_handler(OSAPI_LogHandler_T *handler,void **param);

#if OSAPI_ENABLE_TRACE
/*i \dref_OSAPI_Log_set_trace_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_set_trace_handler(OSAPI_TraceHandler_T handler,void *param);

/*i \dref_OSAPI_Log_get_trace_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_get_trace_handler(OSAPI_TraceHandler_T *handler,void **param);
#endif

/*e \dref_OSAPI_Log_set_display_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_set_display_handler(OSAPI_LogDisplay_T handler,void *param);

/*e \dref_OSAPI_Log_get_display_handler
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_get_display_handler(OSAPI_LogDisplay_T *handler,void **param);

/*i \dref_OSAPI_Log_initialize
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_initialize(void);

/*i \dref_OSAPI_Log_finalize
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_finalize(void);

/*e \dref_OSAPI_Log_set_verbosity
 */
OSAPIDllExport void
OSAPI_Log_set_verbosity(OSAPI_LogVerbosity_T verbosity);

#if OSAPI_ENABLE_TRACE
/*e \dref_OSAPITraceGroupDocs
 */

/*e \dref_OSAPI_Trace_set_trace_mask
 */
OSAPIDllExport void
OSAPI_Trace_set_trace_mask(RTI_UINT32 mask);
#endif

/*e \dref_OSAPI_Log_clear
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Log_clear(void);

/*e \dref_OSAPI_Log_get_property
 */
OSAPIDllExport void
OSAPI_Log_get_property(struct OSAPI_LogProperty *property);

/*e \dref_OSAPI_Log_set_property
 */
OSAPIDllExport RTI_BOOL
OSAPI_Log_set_property(struct OSAPI_LogProperty *property);

/*e \dref_OSAPI_Log_get_verbosity
 */
OSAPIDllExport OSAPI_LogVerbosity_T
OSAPI_Log_get_verbosity(void);

/*ci
 * \brief Create a new log-entry
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] is_final   If RTI_TRUE more data follows, if RTI_FALSE then
 *                       the log meesage is considered complete.
 */
#define  OSAPI_Log_entry_create(kind_, ec_,module_,file_,func_,line_,is_final_)\
OSAPI_Log_call_log_intf(create(kind_, ec_, module_, file_, func_, line_,is_final_))

typedef void
(*OSAPI_Log_entry_create_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,RTI_BOOL is_final);

/*ci
 * \brief Add an integer to a log-entry
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] name     The name of the integer being logged
 * \param[in] value    The value of the integer being logged
 * \param[in] is_final RTI_TRUE if this is the last value added to the log entry
 *                     RTI_FALSE if more values are being added to the log entry
 */
#define OSAPI_Log_entry_add_int(name_,value_,is_final_) \
OSAPI_Log_call_log_intf(add_int(name_,value_,is_final_))

typedef void
(*OSAPI_Log_entry_add_int_T)(const char *name,RTI_INT32 value,RTI_BOOL is_final);

/*ci
 * \brief Add an integer to a log-entry in hex format
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] name     The name of the integer being logged
 * \param[in] value    The value of the integer being logged
 * \param[in] is_final RTI_TRUE if this is the last value added to the log entry
 *                     RTI_FALSE if more values are being added to the log entry
 */
#define OSAPI_Log_entry_add_int_hex(name_,value_,is_final_) \
OSAPI_Log_call_log_intf(add_int_hex(name_,value_,is_final_))

typedef void
(*OSAPI_Log_entry_add_int_hex_T)(const char *name,RTI_INT32 value,RTI_BOOL is_final);

/*ci
 * \brief Add a string to a log-entry
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] name     The name of the string being logged
 * \param[in] value    The value of the string being logged
 * \param[in] is_final RTI_TRUE if this is the last value added to the log entry
 *                     RTI_FALSE if more values are being added to the log entry
 */
#define OSAPI_Log_entry_add_string(name,value,is_final) \
OSAPI_Log_call_log_intf(add_string(name,value,is_final))

typedef void
(*OSAPI_Log_entry_add_string_T)(const char *name,const char *value,RTI_BOOL is_final);

/*ci
 * \brief Add a pointer to a log-entry
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] name     The name of the pointer being logged
 * \param[in] value    The value of the pointer being logged
 * \param[in] is_final RTI_TRUE if this is the last value added to the log entry
 *                     RTI_FALSE if more values are being added to the log entry
 */
#define OSAPI_Log_entry_add_pointer(name,value,is_final) \
OSAPI_Log_call_log_intf(add_pointer(name,value,is_final))
typedef void
(*OSAPI_Log_entry_add_pointer_T)(const char *name,const void *value,RTI_BOOL is_final);

/*ci
 * \brief Add an unsigned integer to a log-entry
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] name     The name of the unsigned integer being logged
 * \param[in] value    The value of the unsigned integer being logged
 * \param[in] is_final RTI_TRUE if this is the last value added to the log entry
 *                     RTI_FALSE if more values are being added to the log entry
 */
#define OSAPI_Log_entry_add_uint(name,value,is_final)\
OSAPI_Log_call_log_intf(add_uint(name,value,is_final))
typedef void
(*OSAPI_Log_entry_add_uint_T)(const char *name,RTI_UINT32 value,RTI_BOOL is_final);

OSAPIDllExport RTI_BOOL
OSAPI_Log_entry_get_data(OSAPI_LogEntry_T *log_entry,char **data_ptr,
                         OSAPI_LogType_T *type,const char **name,
                         const void **value,RTI_BOOL *is_final);

/*ci
 * \brief Create a new log-entry for an error-code with no additional data
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 *
 * \sa OSAPI_Log_entry_create
 */
#define OSAPI_Log_entry_add(kind, error_code,module, file,func,line) \
OSAPI_Log_call_log_intf(add(kind, error_code,module, file,func,line))
typedef void
(*OSAPI_Log_entry_add_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                         const char *const module, const char *const file,
                         const char *const func, RTI_INT32 line);

/*ci
 * \brief Create a new log-entry with a single integer
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name       The name of the integer being logged
 * \param[in] value      The value of the integer being logged
 *
 */
#define OSAPI_Log_entry_add_1int(kind, error_code,module, file,func, line,name,value) \
OSAPI_Log_call_log_intf(add_1int(kind, error_code,module, file,func, line,name,value))

typedef void
(*OSAPI_Log_entry_add_1int_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,const char *name,
                        RTI_INT32 value);

#define OSAPI_Log_entry_add_1uint(kind, error_code,module, file,func, line,name,value) \
OSAPI_Log_call_log_intf(add_1uint(kind, error_code,module, file,func, line,name,value))

typedef void
(*OSAPI_Log_entry_add_1uint_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,const char *name,
                        RTI_UINT32 value);

#define OSAPI_Log_entry_add_1int_hex(kind, error_code,module, file,func, line,name,value) \
OSAPI_Log_call_log_intf(add_1int_hex(kind, error_code,module, file,func, line,name,value))

typedef void
(*OSAPI_Log_entry_add_1int_hex_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                             const char *const module, const char *const file,
                             const char *const func, RTI_INT32 line,
                             const char *name, RTI_INT32 value);

/*ci
 * \brief Create a new log-entry with a 2 integers
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name1      The name of the 1st integer being logged
 * \param[in] value1     The value of the 1st integer being logged
 * \param[in] name2      The name of the 2nd integer being logged
 * \param[in] value2     The value of the 2nd integer being logged
 *
 */
#define OSAPI_Log_entry_add_2int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2) \
OSAPI_Log_call_log_intf(add_2int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2))
typedef void
(*OSAPI_Log_entry_add_2int_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,
                        const char *name1,RTI_INT32 value1,
                        const char *name2,RTI_INT32 value2);

/*ci
 * \brief Create a new log-entry with a 3 integers
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name1      The name of the 1st integer being logged
 * \param[in] value1     The value of the 1st integer being logged
 * \param[in] name2      The name of the 2nd integer being logged
 * \param[in] value2     The value of the 2nd integer being logged
 * \param[in] name3      The name of the 3rd integer being logged
 * \param[in] value3     The value of the 3rd integer being logged
 *
 */
#define OSAPI_Log_entry_add_3int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2,\
                        name3,value3) \
OSAPI_Log_call_log_intf(add_3int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2,\
                        name3,value3))

typedef void
(*OSAPI_Log_entry_add_3int_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,
                        const char *name1,RTI_INT32 value1,
                        const char *name2,RTI_INT32 value2,
                        const char *name3,RTI_INT32 value3);

/*ci
 * \brief Create a new log-entry with a single string
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name       The name of the string being logged
 * \param[in] value      The value of the string being logged
 *
 */
#define OSAPI_Log_entry_add_1string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1) \
OSAPI_Log_call_log_intf(add_1string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1))

typedef void
(*OSAPI_Log_entry_add_1string_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *name,const char* value);

/*ci
 * \brief Create a new log-entry with a 2 strings
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name1       The name of the 1st string being logged
 * \param[in] value1      The value of the 1st string being logged
 * \param[in] name2       The name of the 2nd string being logged
 * \param[in] value2      The value of the 2nd string being logged
 *
 */
#define OSAPI_Log_entry_add_2string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1, \
                        name2,value2) \
OSAPI_Log_call_log_intf(add_2string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2))

typedef void
(*OSAPI_Log_entry_add_2string_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,
                        const char *name1,const char* value1,
                        const char *name2,const char* value2);

/*ci
 * \brief Create a new log-entry with a single pointer
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] name       The name of the pointer being logged
 * \param[in] value      The value of the pointer being logged
 *
 */
#define OSAPI_Log_entry_add_1pointer(kind, error_code,module, file,\
                        func, line,\
                        name1,value1) \
OSAPI_Log_call_log_intf(add_1pointer(kind, error_code,module, file,\
                        func, line,\
                        name1,value1))

typedef void
(*OSAPI_Log_entry_add_1pointer_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *name, const void* value);

/*ci
 * \brief Create a new log-entry with a single string and a integer
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] s_name     The name of the string being logged
 * \param[in] s_value    The value of the string being logged
 * \param[in] i_name     The name of the integer being logged
 * \param[in] i_value    The value of the string being logged
 */
#define OSAPI_Log_entry_add_1string_1int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1, \
                        name2,value2) \
OSAPI_Log_call_log_intf(add_1string_1int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2))
typedef void
(*OSAPI_Log_entry_add_1string_1int_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *s_name,const char* s_value,
                            const char *i_name,RTI_INT32 i_value);

/*ci
 * \brief Create a new log-entry with a trust exception,
 * 2 int and 1 string
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] i_value1    The value of the string being logged
 * \param[in] i_value2    The value of the string being logged
 * \param[in] s_value    The value of the string being logged
 */
#define OSAPI_Log_entry_add_exception(kind, error_code,module, file,\
                        func, line,\
                        value1, \
                        value2, \
                        value3) \
OSAPI_Log_call_log_intf(add_exception(kind, error_code,module, file,\
                        func, line,\
                        value1,\
                        value2,\
                        value3))

typedef void
(*OSAPI_Log_entry_add_exception_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                                   const char *const module, const char *const file,
                                   const char *const func, RTI_INT32 line,
                                   const RTI_INT32 i_value1,
                                   const RTI_INT32 i_value2,
                                   const char* s_value);

/*ci
 * \brief Create a new log-entry with a single string and 2 integer
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] s_name     The name of the string being logged
 * \param[in] s_value    The value of the string being logged
 * \param[in] i_name1     The name of the integer being logged
 * \param[in] i_value1    The value of the integer being logged
 * \param[in] i_name2     The name of the integer being logged
 * \param[in] i_value2    The value of the integer being logged
 */
#define OSAPI_Log_entry_add_1string_2int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1, \
                        name2,value2, \
                        name3,value3) \
OSAPI_Log_call_log_intf(add_1string_2int(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2,\
                        name3,value3))

typedef void
(*OSAPI_Log_entry_add_1string_2int_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *s_name, const char* s_value,
                            const char *i_name1, RTI_INT32 i_value1,
                            const char *i_name2, RTI_INT32 i_value2);

/*ci
 * \brief Create a new log-entry with 3 strings
 *
 * \ingroup OSAPILogClass
 *
 * \param[in] kind       The type of log entry
 * \param[in] error_code The error-code
 * \param[in] module     The name of the module logging the error
 * \param[in] file       The file logging the error
 * \param[in] line       The line number in file the error is logged
 * \param[in] s_name1     The name of the string being logged
 * \param[in] s_value1    The value of the string being logged
 * \param[in] s_name2     The name of the string being logged
 * \param[in] s_value2    The value of the string being logged
 * \param[in] s_name3     The name of the string being logged
 * \param[in] s_value3    The value of the string being logged
 */
#define OSAPI_Log_entry_add_3string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1, \
                        name2,value2, \
                        name3,value3) \
OSAPI_Log_call_log_intf(add_3string(kind, error_code,module, file,\
                        func, line,\
                        name1,value1,\
                        name2,value2,\
                        name3,value3))
typedef void
(*OSAPI_Log_entry_add_3string_T)(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *s_name1, const char* s_value1,
                            const char *s_name2, const char* s_value2,
                            const char *s_name3, const char* s_value3);

/*ci
 * \brief Return RTI_TRUE if logging has been initialized, RTI_FALSE otherwise.
 *
 * \details
 *
 * There are special cases where it is necessary to check if logging has been
 * initialized before creating log messages. Any logging performed as part of system
 * initialization must be single-threaded. For example, any threads
 * created during system initialization has the potential to violate this
 * rule and in this case spawned threads must check if logging
 * is available before creating log messages.
 *
 * This test cannot be done in the logging functions because of race conditions
 * with initialization of logging itself.
 *
 * \return RTI_TRUE if logging has been initialized, RTI_FALSE otherwise.
 */
OSAPIDllExport RTI_BOOL
OSAPI_Log_is_initialized(void);

struct OSAPI_LogEntryI
{
        OSAPI_Log_entry_create_T create;
        OSAPI_Log_entry_add_int_T add_int;
        OSAPI_Log_entry_add_int_hex_T add_int_hex;
        OSAPI_Log_entry_add_string_T add_string;
        OSAPI_Log_entry_add_pointer_T add_pointer;
        OSAPI_Log_entry_add_uint_T add_uint;
        OSAPI_Log_entry_add_T add;
        OSAPI_Log_entry_add_1int_T add_1int;
        OSAPI_Log_entry_add_1uint_T add_1uint;
        OSAPI_Log_entry_add_1int_hex_T add_1int_hex;
        OSAPI_Log_entry_add_2int_T add_2int;
        OSAPI_Log_entry_add_3int_T add_3int;
        OSAPI_Log_entry_add_1string_T add_1string;
        OSAPI_Log_entry_add_2string_T add_2string;
        OSAPI_Log_entry_add_1pointer_T add_1pointer;
        OSAPI_Log_entry_add_1string_1int_T add_1string_1int;
        OSAPI_Log_entry_add_exception_T add_exception;
        OSAPI_Log_entry_add_1string_2int_T add_1string_2int;
        OSAPI_Log_entry_add_3string_T add_3string;
#if OSAPI_ENABLE_TRACE
        OSAPI_TraceHandler_T trace_handler;
        void *trace_param;
        RTI_UINT32 trace_mask;
#endif
};

#define OSAPI_LogEntryI_INITIALIZER \
{\
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, \
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, \
    NULL,NULL,NULL,0 \
}

extern OSPSLDllVariable struct OSAPI_LogEntryI *OSAPI_Log_gv_LogIntf;
extern OSAPIDllVariable struct OSAPI_LogEntryI OSAPI_Log_fv_LogIntf;

/* These functions are wrappers around the indirect calls to log function
 * pointers
 */
#define OSAPI_LOG_FORMAL_ARG OSAPI_LogKind_T kind, RTI_UINT32 error_code, \
        const char *const module, const char *const file, \
        const char *const func, RTI_INT32 line

#define OSAPI_LOG_PARAMETER_ARG kind, error_code, module, file, func, line

OSPSLDllExport void
OSPSL_Log_entry_create(OSAPI_LOG_FORMAL_ARG,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_int(const char *name,RTI_INT32 value,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_uint(const char *name,RTI_UINT32 value,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_int_hex(const char *name,RTI_INT32 value,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_string(const char *name,const char *value,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_pointer(const char *name,const void *value,RTI_BOOL is_final);

OSPSLDllExport void
OSPSL_Log_entry_add_1int_hex(OSAPI_LOG_FORMAL_ARG,const char *name,
                              RTI_INT32 value);

OSPSLDllExport void
OSPSL_Log_entry_add_1int(OSAPI_LOG_FORMAL_ARG,const char *name,RTI_INT32 value);

OSPSLDllExport void
OSPSL_Log_entry_add_1uint(OSAPI_LOG_FORMAL_ARG,const char *name,RTI_UINT32 value);

OSPSLDllExport void
OSPSL_Log_entry_add_2int(OSAPI_LOG_FORMAL_ARG,
                         const char *name1,RTI_INT32 value1,
                         const char *name2,RTI_INT32 value2);

OSPSLDllExport void
OSPSL_Log_entry_add_3int(OSAPI_LOG_FORMAL_ARG,
                              const char *name1,RTI_INT32 value1,
                              const char *name2,RTI_INT32 value2,
                              const char *name3,RTI_INT32 value3);

OSPSLDllExport void
OSPSL_Log_entry_add_1string(OSAPI_LOG_FORMAL_ARG,const char *name,const char* value);

OSPSLDllExport void
OSPSL_Log_entry_add_2string(OSAPI_LOG_FORMAL_ARG,
                            const char *name1,const char* value1,
                            const char *name2,const char* value2);

OSPSLDllExport void
OSPSL_Log_entry_add_1string_1int(OSAPI_LOG_FORMAL_ARG,
                                      const char *s_name,const char* s_value,
                                      const char *i_name,RTI_INT32 i_value);

OSPSLDllExport void
OSPSL_Log_entry_add_1pointer(OSAPI_LOG_FORMAL_ARG,
                                  const char *name,
                                  const void* value);

OSPSLDllExport void
OSPSL_Log_entry_add(OSAPI_LOG_FORMAL_ARG);

OSPSLDllExport void
OSPSL_Log_entry_add_exception(OSAPI_LOG_FORMAL_ARG,
                              const RTI_INT32 i_value1,
                              const RTI_INT32 i_value2,
                              const char* s_value);

OSPSLDllExport void
OSPSL_Log_entry_add_1string_2int(OSAPI_LOG_FORMAL_ARG,
                                 const char *s_name, const char* s_value,
                                 const char *i_name1, RTI_INT32 i_value1,
                                 const char *i_name2, RTI_INT32 i_value2);

OSPSLDllExport void
OSPSL_Log_entry_add_3string(OSAPI_LOG_FORMAL_ARG,
                                  const char *s_name1, const char* s_value1,
                                  const char *s_name2, const char* s_value2,
                                  const char *s_name3, const char* s_value3);

/*e
 * \brief Returns the error code for a function that failed
 * \ingroup OSAPILogClass
 *
 * \details
 * Many functions returns RTI_FALSE or NULL on failure. In order to provide
 * additional information about reason for the failure fucntions may set
 * an additional error code. This function returns the last error-code
 * recorded for the calling thread.
 *
 * \return Last error-code recorded for this thread
 *
 */
OSPSLDllExport RTI_INT32
OSAPI_Log_get_last_error_code(void);

OSPSLDllExport void
OSAPI_Log_set_last_error_code(RTI_INT32 err);

#endif /* OSAPI_ENABLE_LOG */

/*ci
 * \brief Convert an integer to a string in decimal format
 * *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    d           The digit to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
OSAPIDllExport RTI_SIZE_T
OSAPI_Log_itoa(char *buffer,RTI_SIZE_T max_length,RTI_INT32 d);


#if OSAPI_ENABLE_TRACE

/*ci
 * \brief Basic trace function
 *
 */
OSAPIDllExport void
OSAPI_Trace_printf(RTI_UINT32 trace_mask,void *param,RTI_UINT32 context,
                   const char *const module,const char *const file,
                   const char *const function,RTI_INT32 line_no,
                   const char *fmt,
                   char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,
                   char *arg5,char *arg6,char *arg7,char *arg8,char *arg9);

OSAPIDllExport void
OSAPI_Trace_write_impl(RTI_UINT32 trace_mask,void *param,RTI_UINT32 context,
                       const char *const module,const char *const file,
                       const char *const function,RTI_INT32 line_no,
                       const char *fmt,
                       char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,
                       char *arg5,char *arg6,char *arg7,char *arg8,char *arg9);

OSPSLDllExport void
OSPSL_Log_entry_trace_handler(RTI_UINT32 trace_mask,void *param,
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
                              RTI_BOOL is_final);
#endif


OSAPIDllExport RTI_SIZE_T
OSAPI_Trace_itoh(char *buffer,RTI_SIZE_T max_length, RTI_UINT32 pid,
                 RTI_INT32 size,RTI_BOOL nlz);

OSPSLDllExport void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length);

/*ci
 * \brief Convert a long long integer to a string in decimal format
 *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    d           The digit to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
OSAPIDllExport RTI_SIZE_T
OSAPI_Log_itoa64(char *buffer, RTI_SIZE_T max_length, RTI_INT64 d);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#include "osapi/osapi_log_impl.h"

#endif /* osapi_log_h */
