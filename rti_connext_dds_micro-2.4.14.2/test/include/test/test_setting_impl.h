/*
 * FILE: test_setting_impl.h - Unit-test setting macro definitions
 *
 * (c) Copyright, Real-Time Innovations, 2004-2020.
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
 * 01dec2004,cc  Created, based on Waveworks tree
 *
 */
/*ce
 * \file
 */
#ifndef test_setting_impl_h
#define test_setting_impl_h

#define UTEST_Log_debug \
    if (UTEST_gv_VerbosityLevel >= UTEST_LOGVERBOSITY_DEBUG) \
        UTEST_Stdio_printf

#define UTEST_Log_warning \
    if (UTEST_gv_VerbosityLevel >= UTEST_LOGVERBOSITY_WARNING) \
        UTEST_Stdio_printf

#define UTEST_Log_error \
    if (UTEST_gv_VerbosityLevel >= UTEST_LOGVERBOSITY_ERROR) \
        UTEST_Stdio_printf

#define TEST_EXECUTE(name_)\
if (setting->verbose)\
{\
    UTEST_Stdio_printf("%s\n",name_);\
}

#define TEST_ASSERT(cond_,msg_,action_) \
    if (!(cond_)) \
    {\
        if (setting->extended_mode) \
        {\
            char tbuf_[512];\
            if (setting->test_last_error[0] != 0) \
            {\
                UTEST_Stdio_snprintf(tbuf_,512,"\nFAILURE: %s:%d %s",UTEST_File_get_basename(__FILE__),__LINE__,msg_);\
            }\
            else \
            {\
                UTEST_Stdio_snprintf(tbuf_,512,"FAILURE: %s:%d %s",UTEST_File_get_basename(__FILE__),__LINE__,msg_);\
            }\
            UTEST_String_strncat(setting->test_last_error,tbuf_,UTEST_LAST_ERROR);\
        }\
        else\
        {\
            UTEST_Stdio_printf("FAILURE: %s:%d %s",UTEST_File_get_basename(__FILE__),__LINE__,msg_);\
        }\
        action_;\
    }

#define TEST_ASSERT_NO_SETTING(cond_,msg_,action_) \
    if (!(cond_)) \
    {\
        UTEST_Stdio_printf("FAILURE: %s:%d %s",UTEST_File_get_basename(__FILE__),__LINE__,msg_);\
        action_;\
    }

#define RTIUNIT_ASSERT(cond_,action_) \
        if (!(cond_)) {\
            UTEST_Stdio_printf("ASSERT: Failed @ line %d\n",__LINE__);\
            UTEST_Stdio_snprintf(setting->test_last_error,512,"%s:%d ASSERT Failed",UTEST_File_get_basename(__FILE__),__LINE__);\
            action_;\
        }

#define TEST_ENABLED  1
#define TEST_DISABLED 0

#define RTI_TEST_SETTING_DEFAULT {{-1, -1}, {-1, -1}, \
                                  RTI_FALSE, RTI_FALSE}

#define CHECK_DO_RUN_TEST(s_) \
if ((s_)->print_all) { \
    UTEST_Stdio_printf("%s",setting->test_description);\
}\
if ((s_)->path_expr && \
    UTEST_String_fnmatch((s_)->path_expr,\
                         (s_)->test_path,\
                         UTEST_STRING_FNM_PATHNAME))\
{\
    return 2;\
}\
else if (!(s_)->current_entry->enabled) \
{\
    return 3;\
}\
else if (!(s_)->print_all) \
    UTEST_Stdio_printf("%s",setting->test_description)

#define CHECK_TEST_ENABLED(t_) \
    ((t_)->enabled)

#define UTEST_Context_set(setting,\
                          beginModule, beginUnit,\
                          endModule, endUnit,\
                          ignore) \
{ \
  (setting)->ignoreFailure = (ignore); \
  (setting)->runInteractiveTest = 0; \
  (setting)->test_path[0]=0;\
  (setting)->test_id_path[0]=0;\
  (setting)->test_counter[0]=0;\
  (setting)->test_level=-1;\
  (setting)->test_last_error[0]=0;\
  (setting)->dont_run=0;\
  (setting)->print_id=0;\
  (setting)->print_path=0;\
  (setting)->path_expr=NULL;\
  (setting)->test_description[0]=0;\
  (setting)->c_ptr=NULL;\
  (setting)->extended_mode=1;\
  (setting)->verbose=0;\
  (setting)->error_count = 0;\
  (setting)->default_properties = NULL;\
  (setting)->nodename = NULL;\
  (setting)->rtiarch = NULL;\
  (setting)->user = NULL;\
  (setting)->sysname = NULL;\
  (setting)->sys_init = 1;\
  (setting)->domain_id = -1;\
  (setting)->no_lua = 1;\
  (setting)->no_log = 1;\
  (setting)->no_sql = 1;\
  (setting)->arch_as_filename[0]=0;\
  (setting)->dts_path[0]=0;\
  (setting)->module_from_path=0;\
  (setting)->file_type=NULL;\
  (setting)->module_name=NULL;\
  (setting)->print_all=0;\
  (setting)->build_root=NULL;\
  (setting)->lua_path=NULL;\
  (setting)->repeat=1;\
  (setting)->valgrind=0;\
}

#define UTEST_Context_init(setting) \
  UTEST_Context_set(setting, -1, -1, -1, -1, 0)

#define RTI_TEST_ASSERT(expr_,string_,action_) \
if (!(expr_)) {\
    UTEST_Stdio_printf("%s@%d: %s\n",__FILE__,__LINE__,string_);\
    action_;\
}

#define UTEST_TestEntryCount(tc_) (int)(sizeof(tc_)/sizeof(struct UTEST_TestEntry))

#define RTITestCase(t_,f_,e_) \
{ t_,f_,e_,NULL}

#define RTITestCaseTags(t_,f_,e_,tags_) \
{ t_,f_,e_,tags_}

#if defined(RTI_WIN32) || defined(RTI_WINCE30)
#define RTI_TEST_RECEIVE_THREAD_PRIORITY (1)
#elif defined(RTI_VXWORKS)
#define RTI_TEST_RECEIVE_THREAD_PRIORITY \
  (RTI_THREAD_DEFAULT_PRIORITY-1)
#elif defined(RTI_LYNX310) || defined(RTI_PSOS) \
            || defined(RTI_NETOS) || defined(RTI_INTY)
#define RTI_TEST_RECEIVE_THREAD_PRIORITY \
  (RTI_THREAD_DEFAULT_PRIORITY+1)
#else
#define RTI_TEST_RECEIVE_THREAD_PRIORITY RTI_THREAD_DEFAULT_PRIORITY
#endif /* defined(RTI_WIN32) || defined(RTI_WINCE30) */


/* ------------------------------------------------------------------------- */
/* ----              Macro Helpers to implement test cases               ----*/
/* ------------------------------------------------------------------------- */

#ifndef TEST_PREFIX
#define TEST_PREFIX                     "UndefinedTestNs"
#endif /* TEST_PREFIX */

#define TEST_PFX                        concat(TEST_PREFIX,Tester)

/* Stringify argument */
#define xstr(A)             # A
#define str(A)              xstr(A)
/* Concatenate token arguments */
#define xconcat(A, B)       A ## B
#define concat(A, B)        xconcat(A,B)

#define TEST_NS(name_)                  concat(concat(TEST_PFX,_),name_)

#define TEST_CASE_FN(name_)             TEST_NS(concat(test_,name_))

#define TEST_CASE(name_) \
unsigned char \
TEST_CASE_FN(name_)(struct UTEST_Context *setting)

#define TEST_CASE_HEADER(name_) \
    unsigned char result = RTI_FALSE;\
    CHECK_DO_RUN_TEST(setting);\
    {\

#define TEST_CASE_FOOTER(name_) \
    }\
    result = RTI_TRUE;\
done:\
    return (unsigned char)result;

#define TEST_CASE_ENTRY(name_) \
    RTITestCase(str(name_), TEST_CASE_FN(name_), TEST_ENABLED)

#define TEST_CASES \
    static struct UTEST_TestEntry  TEST_NS(tests)[]

#define TEST_RUNNER_FN(prefix_,name_) \
    UT_DEFINE_SUBMODULE_RUNNER(prefix_,name_)

#define TEST_RUNNER \
    TEST_RUNNER_FN(TEST_PFX,str(TEST_PFX))

#define TEST_VAR(name_)                 TEST_NS(concat(fv_,name_))

#define TEST_VAR_FN_RESULT(method_)     TEST_VAR(concat(FnResult_,method_))
#define TEST_VAR_FN_CALLED(method_)     TEST_VAR(concat(FnCalled_,method_))

#define TEST_FN_STATE(method_,res_type_, res_init_) \
    res_type_           TEST_VAR_FN_RESULT(method_) = (res_init_); \
    RTI_UINT32          TEST_VAR_FN_CALLED(method_) = 0

#define TEST_FN_SET_RESULT(method_,res_) \
{\
    TEST_VAR_FN_RESULT(method_) = (res_);\
}

#define TEST_FN_CALLED(method_) \
{\
    TEST_VAR_FN_CALLED(method_) += 1;\
}

#define TEST_FN_GET_RESULT(method_)     (TEST_VAR_FN_RESULT(method_))

#define TEST_FN_GET_CALLED(method_)     (TEST_VAR_FN_CALLED(method_))


#define TEST_ASSERT_FN_CALLED(method_,count_) \
{\
    TEST_ASSERT(TEST_FN_GET_CALLED(method_) == (count_),\
                "invalid call count for function " str(method_) ", "\
                "expected: " str(count_),\
                goto done);\
}


/* ------------------------------------------------------------------------- */

#endif /* test_setting_impl_h */
