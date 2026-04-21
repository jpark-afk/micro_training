/*
 * FILE: test_arinc653.h - Unit-test support for ARINC 653
 *
 * (c) Copyright, Real-Time Innovations, 2022-2022.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Unit-test support for ARINC 653
 */
#ifndef test_arinc653_h
#define test_arinc653_h

#define UT_ARINC_CONTROLLER_SEND_PORT "Controller_Send"
#define UT_ARINC_CONTROLLER_RECV_PORT "Controller_Recv"
#define UT_ARINC_WORKER_SEND_PORT "Worker_Send"
#define UT_ARINC_WORKER_RECV_PORT "Worker_Recv"

#if defined(RTI_CERT) || defined(RTI_DEOS)
#define UTEST_ARINC_CERT_ARG_STRING "-id 0"
#endif

struct UT_ArincTestResult
{
    int result;
    char test_last_error[1000];
};

#define UT_ARINC_PORT_INITIALIZE(entry_,name_) \
    CREATE_QUEUING_PORT(UT_ARINC_SEND_PORT_NAME, \
                1024, 1, SOURCE, FIFO, \
                &entry_##_send_id, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("Failed to create send port rc=%d\n", rc);\
        return;\
    }\
    CREATE_QUEUING_PORT(UT_ARINC_RECV_PORT_NAME, \
                1024, 1, DESTINATION, FIFO, \
                &entry_##_recv_id, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("Failed to create recv port rc=%d\n", rc);\
        return;\
    }

#define UT_ARINC_RUN_PROCESS \
    CREATE_PROCESS(&UT_ARINC_PROCESS_ATTR, &UT_ARINC_PROCESS_ID, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("CREATE_PROCESS: FAILED, rc == %d !\n", rc);\
        return;\
    }\
    START(UT_ARINC_PROCESS_ID, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("START: FAILED, rc == %d !\n", rc);\
        return;\
    }\
    SET_PARTITION_MODE(NORMAL, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("SET_PARTITION_MODE NORMAL: FAILED, rc == %d !\n", rc);\
        return;\
    }

#if defined(UT_ARINC_CONTROLLER)
#define UT_ARINC_SEND_PORT_NAME UT_ARINC_CONTROLLER_SEND_PORT
#define UT_ARINC_RECV_PORT_NAME UT_ARINC_CONTROLLER_RECV_PORT

#define UT_ARINC_PROCESS(entry_,name_) \
    RETURN_CODE_TYPE rc;\
    UTEST_Stdio_printf("Running test controller process [%s]\n", name_);\
    if (!UTEST_Runner_run_tests(&entry_##_setting,name_,\
                entry_##_tests,UTEST_TestEntryCount(entry_##_tests)))\
    {\
        UTEST_Stdio_printf("Running tests failed [%s]\n", name_);\
    }\
    UTEST_Stdio_printf("Finished running tests. Restarting...\n");\
    /* Pause to let standard output be flushed before restarting */\
    for (int i = 0; i < 10; ++i)\
    {\
        TIMED_WAIT((SYSTEM_TIME_TYPE) (1000 * 1000 * 1000), &rc);\
    }\
    SET_PARTITION_MODE(COLD_START, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("SET_PARTITION_MODE COLD_START: FAILED, rc == %d !\n", rc);\
    }

/* VxWorks 653 certified partitions and Deos do not support gets() */
#if defined(RTI_CERT) || defined(RTI_DEOS)
#define UT_ARINC_GET_ARG_STRING(buffer_) \
    strcpy((buffer_), (UTEST_ARINC_CERT_ARG_STRING));\
    printf("%s\n", (buffer_));
#else
#define UT_ARINC_GET_ARG_STRING(buffer_) \
    gets(buffer_);
#endif

#define UT_ARINC_MAIN(entry_,name_) \
    char buffer[1000];\
    char **argvi;\
    int argci = 0;\
    RETURN_CODE_TYPE rc;\
    unsigned char result;\
\
    UT_ARINC_PORT_INITIALIZE(entry_,name_)\
\
    UTEST_Context_init(&entry_##_setting);\
    entry_##_setting.dont_run = 1;\
\
    do \
    {\
        UTEST_Stdio_printf("Enter test arguments: ");\
        UT_ARINC_GET_ARG_STRING(buffer)\
\
        UTEST_String_argv_from_string(name_,buffer,&argci,&argvi);\
        result = UTEST_Main_parse_arguments(&entry_##_setting, argci, argvi);\
        if (!result)\
        {\
            UTEST_Stdio_printf("Failed to parse arguments\n");\
        }\
    } while (!result);\
    UT_ARINC_RUN_PROCESS

#else
#define UT_ARINC_SEND_PORT_NAME UT_ARINC_WORKER_SEND_PORT
#define UT_ARINC_RECV_PORT_NAME UT_ARINC_WORKER_RECV_PORT

#define UT_ARINC_PROCESS(entry_,name_) \
    RETURN_CODE_TYPE rc;\
    struct UT_ArincTestResult test_result;\
    UTEST_Stdio_printf("Running test worker process [%s]\n", name_);\
    if (entry_##_setting.error_count == 0)\
    {\
        test_result.result = UTEST_Runner_run_tests(&entry_##_setting,name_,\
                    entry_##_tests,UTEST_TestEntryCount(entry_##_tests));\
    }\
    else\
    {\
        test_result.result = 0;\
    }\
    strncpy(test_result.test_last_error, entry_##_setting.test_last_error, 1000);\
    if (!test_result.result)\
    {\
        UTEST_Stdio_printf("Running tests failed [%s]\n", name_);\
        TIMED_WAIT((SYSTEM_TIME_TYPE) (1000 * 1000 * 1000), &rc);\
    }\
    SEND_QUEUING_MESSAGE(entry_##_send_id,\
                                (MESSAGE_ADDR_TYPE)&test_result,\
                                (MESSAGE_SIZE_TYPE)sizeof(struct UT_ArincTestResult),\
                                0, /* timeout */\
                                &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("SEND_QUEUING_MESSAGE: FAILED, rc == %d !\n", rc);\
        return;\
    }\
    UTEST_Stdio_printf("Restarting [%s]\n", name_);\
    /* Pause to let standard output be flushed before restarting */\
    TIMED_WAIT((SYSTEM_TIME_TYPE) (100 * 1000 * 1000), &rc);\
    SET_PARTITION_MODE(COLD_START, &rc);\
    if (rc != NO_ERROR)\
    {\
        UTEST_Stdio_printf("SET_PARTITION_MODE COLD_START: FAILED, rc == %d !\n", rc);\
    }

#define UT_ARINC_MAIN(entry_,name_) \
    char **argvi;\
    int argci = 0;\
    RETURN_CODE_TYPE rc;\
\
    char buffer[1024];\
    char args[1024];\
    RTI_INT32 msg_size;\
    struct OSAPI_LogProperty log_prop = OSAPI_LogProperty_INIITALIZER;\
\
    UT_ARINC_PORT_INITIALIZE(entry_,name_)\
\
    UTEST_Stdio_printf("Waiting for test arguments...\n");\
    while (RTI_TRUE)\
    {\
        RECEIVE_QUEUING_MESSAGE(entry_##_recv_id,\
                            0 /*ZERO_TIME_VALUE */,\
                            (MESSAGE_ADDR_TYPE)buffer,\
                            (MESSAGE_SIZE_TYPE*)&msg_size,\
                            &rc);\
        if (rc == NO_ERROR || rc == INVALID_CONFIG)\
        {\
            UTEST_Stdio_printf("Received: %s\n", buffer);\
            break;\
        }\
        else if (rc == NOT_AVAILABLE)\
        {\
            /* No message available */ \
            continue;\
        }\
        else\
        {\
            UTEST_Stdio_printf("RECEIVE_QUEUING_MESSAGE: FAILED, rc == %d !\n", rc);\
            return;\
        }\
    }\
\
    UTEST_Context_init(&entry_##_setting);\
    UTEST_Stdio_snprintf(args, 1024, "%s %s", UTEST_ARG_STRING(""), buffer); \
    UTEST_String_argv_from_string(name_,args,&argci,&argvi);\
\
    if (!UTEST_Main_parse_arguments(&entry_##_setting, argci, argvi))\
    {\
        UTEST_Stdio_printf("failed to parse argument string [%s]\n",buffer);\
        return;\
    }\
\
    if (!OSAPI_System_initialize())\
    {\
        UTEST_Stdio_printf("failed to initialize system\n");\
        return;\
    }\
\
    if (!OSAPI_Log_set_property(&log_prop))\
    {\
        UTEST_Stdio_printf("failed to set log property\n");\
        return;\
    }\
    if (!OSAPI_Log_initialize())\
    {\
        UTEST_Stdio_printf("failed to initialize log\n");\
        return;\
    }\
\
    UTEST_Stdio_printf("Setting up tests [%s]\n", name_);\
    if (!UTEST_Runner_run_tests(&entry_##_setting,name_,\
                entry_##_tests,UTEST_TestEntryCount(entry_##_tests)))\
    {\
        UTEST_Stdio_printf("Test setup failed [%s]\n", name_);\
    }\
    UT_ARINC_RUN_PROCESS

#endif

#define UT_DEFINE_ARINC_PROCESS(entry_,name_) \
    static PROCESS_ID_TYPE UT_ARINC_PROCESS_ID;\
    static QUEUING_PORT_ID_TYPE entry_##_send_id;\
    static QUEUING_PORT_ID_TYPE entry_##_recv_id;\
    void \
    entry_##_process_run (void) \
    {\
        UT_ARINC_PROCESS(entry_,name_)\
    }\
    static PROCESS_ATTRIBUTE_TYPE UT_ARINC_PROCESS_ATTR = \
        {.NAME=name_, .ENTRY_POINT=(void*)entry_##_process_run, \
         .STACK_SIZE=32768, .BASE_PRIORITY=98, .PERIOD=INFINITE_TIME_VALUE, \
         .TIME_CAPACITY=INFINITE_TIME_VALUE, .DEADLINE=SOFT};

#define UTEST_DEFINE_ARINC_RUN_TEST_ENTRY \
static unsigned char \
UTEST_Runner_run_test_entry(struct UTEST_Context *setting,\
                            struct UTEST_TestEntry test_entry)\
{\
    RETURN_CODE_TYPE rc;\
    int msg_size;\
    char buffer[1024];\
    QUEUING_PORT_ID_TYPE send_id;\
    QUEUING_PORT_ID_TYPE recv_id;\
    struct UT_ArincTestResult *test_result;\
\
    /* If this is the controller partition and the test is not a top-level\
     * test, then tell the worker partition to execute the test.\
     */\
    if (setting->dont_run && strlen(setting->test_path) != 0)\
    {\
        CHECK_DO_RUN_TEST(setting);\
\
        GET_QUEUING_PORT_ID(UT_ARINC_CONTROLLER_SEND_PORT, &send_id, &rc);\
        if (rc != NO_ERROR)\
        {\
            UTEST_Stdio_snprintf(setting->test_last_error, 1000, "Failed to get send port id rc=%d\n", rc);\
            return 0;\
        }\
        GET_QUEUING_PORT_ID(UT_ARINC_CONTROLLER_RECV_PORT, &recv_id, &rc);\
        if (rc != NO_ERROR)\
        {\
            UTEST_Stdio_snprintf(setting->test_last_error, 1000, "Failed to get recv port id rc=%d\n", rc);\
            return 0;\
        }\
\
        UTEST_Stdio_snprintf(buffer, 1024, "-id %d -include \"%s\"", \
                setting->domain_id, setting->test_path);\
        SEND_QUEUING_MESSAGE(send_id,\
                                (MESSAGE_ADDR_TYPE)buffer,\
                                (MESSAGE_SIZE_TYPE)strlen(buffer)+1,\
                                INFINITE_TIME_VALUE,\
                                &rc);\
        if (rc != NO_ERROR)\
        {\
            UTEST_Stdio_snprintf(setting->test_last_error, 1000, "Failed to send message rc=%d\n", rc);\
            return 0;\
        }\
\
        RECEIVE_QUEUING_MESSAGE(recv_id,\
                        INFINITE_TIME_VALUE,\
                        (MESSAGE_ADDR_TYPE)&buffer,\
                        (MESSAGE_SIZE_TYPE*)&msg_size,\
                        &rc);\
        if (rc != NO_ERROR && rc != INVALID_CONFIG)\
        {\
            UTEST_Stdio_snprintf(setting->test_last_error, 1000, "Failed to receive message rc=%d\n", rc);\
            return 0;\
        }\
        test_result = (struct UT_ArincTestResult*)&buffer;\
        strncpy(setting->test_last_error, test_result->test_last_error, 1000);\
        return (unsigned char)test_result->result;\
    }\
    return test_entry.test_func(setting);\
}

#endif /* test_arinc653 */