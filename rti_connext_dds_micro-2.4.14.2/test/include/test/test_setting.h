/*
 * FILE: test_setting.h - Unit-test settings definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020.
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
 * 30apr2014,tk  Minor refactoring to better handle Android and iOS specifics
 * 01dec2004,cc  Created, based on Waveworks tree
 *
 */
/*ce
 * \file
 * \brief Unit-test settings definitions
 */

#ifndef test_setting_h
#define test_setting_h

/*ci \brief Function declaration of UTEST_thread_port_properties_callout
 * The implementation will belong to the test environment
 */
unsigned int UTEST_thread_port_properties_callout(void* threadProperties);

/*ci \brief Function declaration of UTEST_system_port_properties_callout
 * The implementation will belong to the test environment
 */
unsigned int UTEST_system_port_properties_callout(void);

/*ci \brief Macro to stringify a macro using the C preprocessor
 */
#define UTEST_STRINGIFY_DEFINE(v_) UTEST_STRINGIFY_VALUE(v_)

/*ci \brief Macro to stringify a value using the C preprocessor
 */
#define UTEST_STRINGIFY_VALUE(v_) #v_

#define HAVE_TEST_RESULTS_FILE 1

/* If the platform has not been specified, attempt to determine it.
 */
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__linux__) || defined(__QNXNTO__)
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200112L
#ifndef RTI_UNIX
#define RTI_UNIX
#endif
#if defined(__APPLE__) && !defined(RTI_DARWIN)
#define RTI_DARWIN
#endif
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#ifndef RTI_DARWIN
#define RTI_DARWIN
#endif
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#ifndef RTI_IOS
#define RTI_IOS
#endif
#include "test_ios.h"
#define HAVE_ARG_STRING  1
#define HAVE_CONFIG_FILE 0
#else
#define HAVE_CONFIG_FILE 1
#endif
#elif defined(__linux__)
#ifndef RTI_LINUX
#define RTI_LINUX
#endif
#ifdef __ANDROID__
#include "test_android.h"
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  1
#else
#define HAVE_CONFIG_FILE 1
#endif
#elif __autosar__
#ifndef RTI_AUTOSAR
#define RTI_AUTOSAR
#endif /* RTI_AUTOSAR */
#include "test_autosar.h"
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  0
#ifdef HAVE_TEST_RESULTS_FILE
#undef HAVE_TEST_RESULTS_FILE
#endif
#define HAVE_TEST_RESULTS_FILE 0
#define ENTRYPOINT emain
int ENTRYPOINT(int argc, char **argv);
#elif defined(_MSC_VER) || defined(WIN32)
#define HAVE_CONFIG_FILE 1
#ifndef RTI_WIN32
#define RTI_WIN32
#endif
#elif defined(__vxworks)
#define HAVE_CONFIG_FILE 1
#ifndef RTI_VXWORKS
#define RTI_VXWORKS
#endif
#elif defined(__QNXNTO__)
#define HAVE_CONFIG_FILE 1
#ifndef RTI_QNX6
#define RTI_QNX6
#endif
#endif

#if defined(__THREADX__)
#ifndef RTI_THREADX
#define RTI_THREADX
#endif
#include "test_threadx.h"
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  0
#ifdef HAVE_TEST_RESULTS_FILE
#undef HAVE_TEST_RESULTS_FILE
#endif
#define HAVE_TEST_RESULTS_FILE 0
#define ENTRYPOINT emain
#endif 

#if __freertos__
#ifndef RTI_FREERTOS
#define RTI_FREERTOS
#endif /* RTI_FREERTOS */
#include "test_freertos.h"
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  0
#ifdef HAVE_TEST_RESULTS_FILE
#undef HAVE_TEST_RESULTS_FILE
#endif
#define HAVE_TEST_RESULTS_FILE 0
#define ENTRYPOINT emain
#endif /* __freertos__ */

#ifdef RTI_ARINC653
#include "test_arinc653.h"
#endif /* RTI_ARINC653 */

#ifdef RTI_VX653
#include "test_vxworks653.h"
#ifdef HAVE_CONFIG_FILE
#undef HAVE_CONFIG_FILE
#endif
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  1
#ifdef HAVE_TEST_RESULTS_FILE
#undef HAVE_TEST_RESULTS_FILE
#endif
#define HAVE_TEST_RESULTS_FILE 0
#endif /* RTI_VX653 */

#ifdef RTI_DEOS
#include "test_deos.h"
#ifdef HAVE_CONFIG_FILE
#undef HAVE_CONFIG_FILE
#endif
#define HAVE_CONFIG_FILE 0
#define HAVE_ARG_STRING  1
#ifdef HAVE_TEST_RESULTS_FILE
#undef HAVE_TEST_RESULTS_FILE
#endif
#define HAVE_TEST_RESULTS_FILE 0
#endif /* RTI_DEOS */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#if !defined(RTI_WIN32) && !defined(RTI_THREADX) && !defined(RTI_DEOS) && !defined(RTI_AUTOSAR)
#include <unistd.h>
#endif
#if defined(RTI_UNIX)
#if !defined(RTI_VXWORKS)
#include <sys/utsname.h>
#endif
#if defined(RTI_LINUX)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_VXWORKS)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_DARWIN)
#define UTEST_SYSTEM_NAME_MAX_LENGTH _SYS_NAMELEN
#elif defined(RTI_QNX6)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#endif
#elif defined(RTI_WIN32)
#include <Windows.h>
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_VX653) || defined(RTI_DEOS)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_VXWORKS)
#ifndef RTI_CERT
#include <hostLib.h>
#endif
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_THREADX)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_AUTOSAR)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#elif defined(RTI_FREERTOS)
#define UTEST_SYSTEM_NAME_MAX_LENGTH 255
#else
#error "Unknown platform. Please port UT_System.c to this platform."
#endif

#ifndef test_dll_h
#include "test/test_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct UTEST_SystemInfo
{
    char    sysname[UTEST_SYSTEM_NAME_MAX_LENGTH];
    char    nodename[UTEST_SYSTEM_NAME_MAX_LENGTH];
    char    release[UTEST_SYSTEM_NAME_MAX_LENGTH];
    char    version[UTEST_SYSTEM_NAME_MAX_LENGTH];
    char    machine[UTEST_SYSTEM_NAME_MAX_LENGTH];
    char    rtiarch[UTEST_SYSTEM_NAME_MAX_LENGTH];
};

typedef enum
{
    UTEST_LOGVERBOSITY_SILENT  = 0,
    UTEST_LOGVERBOSITY_ERROR   = 1,
    UTEST_LOGVERBOSITY_WARNING = 2,
    UTEST_LOGVERBOSITY_DEBUG   = 3
} UTEST_LogVerbosity_T;

RTITestDllVariable extern UTEST_LogVerbosity_T UTEST_gv_VerbosityLevel;

struct UTEST_Property
{
    char *name;
    char *value;
};

#define UTEST_SETTING_MAX_PROPERTIES 16
#define UTEST_PATHNAME_MAX_LENGTH    1000
#define UTEST_MAX_DEPTH              5
#define UTEST_LAST_ERROR             1000
#define UTEST_MAX_COLUMN_WIDTH       70
#define UTEST_MAX_ARCH_NAME          255

struct UTEST_Context
{
    /*e If set to RTI_TRUE, all tests in specified range will be run, even if one
     * or more tests fail. If set to RTI_FALSE, the testing stops as soon as one
     * of the tests fails. */
    unsigned char ignoreFailure;

    /*e If set to RTI_TRUE, run interactive tests as well. Otherwise, skip all
     * the interactive tests. Default is RTI_FALSE. */
    unsigned char runInteractiveTest;

    /* Original environment argument */
    char **argv;

    /* Original environment count */
    int argc;

    char *platform_name;

    char test_path[UTEST_PATHNAME_MAX_LENGTH];

    char test_id_path[UTEST_PATHNAME_MAX_LENGTH];

    int  test_counter[UTEST_MAX_DEPTH];

    int test_level;

    char test_last_error[UTEST_LAST_ERROR];

    int dont_run;

    int print_id;

    int print_path;

    const char *path_expr;

    char test_description[UTEST_MAX_COLUMN_WIDTH];

    char *c_ptr;

    unsigned char extended_mode;

    struct UTEST_Property property[UTEST_SETTING_MAX_PROPERTIES];

    int property_length;

    int verbose;

    int error_count;

    char *default_properties;

    const char *nodename;

    const char *rtiarch;

    const char *user;

    const char *sysname;

    struct UTEST_SystemInfo sysinfo;

    char sys_init;

    int no_sql;

    int no_lua;

    int no_log;

    int no_stdout;

    char arch_as_filename[UTEST_MAX_ARCH_NAME];

    struct UTEST_TestEntry *current_entry;

    int domain_id;

    char dts_path[UTEST_PATHNAME_MAX_LENGTH];

    int module_from_path;

    const char *file_type;

    const char *module_name;

    int print_all;

    char *build_root;

    char *lua_path;

    int repeat;

    int valgrind;
};

typedef unsigned char
(*UTEST_TestEntryFunction)(struct UTEST_Context *setting);

struct UTEST_TestEntry
{
    const char *title;
    UTEST_TestEntryFunction test_func;
    unsigned char enabled;
    const char *tags;
};

#define UTEST_TestEntry_INITIALIZER \
{\
    NULL,\
    NULL,\
    0,\
    NULL\
}

/* UTEST_String functions */

/*e \brief
 * Appends string s2 to s1. Maximun size of s1, including null-character
 * is n.
 *
 * \param[in/out] s1  Destination string
 * \param[in]     s2  Source string
 * \param[in]     n   Maximun size of s1, including null-character.
 *
 * \return If successful returns the number length of the output string.
 *         If error returns a negative number.
 */
RTITestDllExport char*
UTEST_String_strncat(char *s1,const char *s2,size_t n);

/*e \brief
 * Copy the first n characters of s2 to s1.
 *
 * \param[in/out] s1  Destination string
 * \param[in]     s2  Source string
 * \param[in]     n   Maximun number of characters to copy.
 *
 * \return If successful returns the number length of the output string.
 *         If error returns a negative number.
 */
RTITestDllExport char*
UTEST_String_strncpy(char *s1,const char *s2,size_t n);

/*e \brief
 * Duplicates the input string by allocating a new buffer.
 *
 * \param[in] string  Input string to duplicate.
 *
 * \return Pointer to new string or NULL if memory can not be allocated.
 */
RTITestDllExport char*
UTEST_String_strdup(const char *string);

/*e \brief
 * Parse a string with arguments to argc and argv main-function-like
 * arguments.
 *
 * \details I.e. input parameters appname="OSAPITester" and
 * arg_string="-id 58 -property netio.udp.allow_interface_multicast=1"
 * would have as output *argc=5 and *argv=
 *    ["OSAPITester", "-id", "58", "-property",
 *      "netio.udp.allow_interface_multicast=1"]
 *
 * \param[in]  appname     Application name
 * \param[in]  arg_string  String with arguments
 * \param[out] argc        Number of parameters in argv
 * \param[out] argv        Pointer to parameters
 *
 * \return If successful returns 1.
 *         If error returns 0.
 */
RTITestDllExport int
UTEST_String_argv_from_string(const char *appname,
                              const char *arg_string,
                              int *argc,
                              char ***argv);

#define UTEST_STRING_FNM_PATHNAME    0x02
#define UTEST_STRING_FNM_PERIOD      0x04
#define UTEST_STRING_FNM_CASEFOLD    0x10
#define UTEST_STRING_FNM_DOTTED_PATH 0x20
#define UTEST_STRING_FNM_NOMATCH     1
#define UTEST_STRING_FNM_NOESCAPE    0x01
#define UTEST_STRING_FNM_LEADING_DIR 0x08

/*e \brief
 * Check if a string matches a given regular expression.
 *
 * \param[in]  pattern     Regular expression patern
 * \param[in]  string      String to check whether it matches the
 *                         regular expression
 * \param[out] flags       Mask with UTEST_STRING_FNM_xxxx flags
 *
 * \return Returns zero if string matches pattern, UTEST_STRING_FNM_NOMATCH
 * if there is no match or another non-zero value if there is an error.
 */
RTITestDllExport int
UTEST_String_fnmatch(const char *pattern, const char *string, int flags);

/* UTEST_Stdio functions */

/*e \brief
 * Printf the string with the specified format in the standard output.
 *
 * \details
 * Provides same functionality as printf.
 * 
 * \param[in]  format Format to print
 */
RTITestDllExport void
UTEST_Stdio_printf(const char *format, ...);

#define OSAPI_Stdio_printf UTEST_Stdio_printf
#if !defined(NO_RTI_UNIT_MAIN) && HAVE_ARG_STRING && !defined(RTI_AUTOSAR)
/*e \brief
 * Main UTEST function implementation for systems without dynamic linking.
 * 
 * \brief
 * We recommend that you create a separate thread and call that test_main()
 * function from that thread. This thread should have at least 32Kbytes
 * of stack.
 *
 * \return[in]  0 if success. Otherwise error.
 */
RTITestDllExport int
UTEST_main(void);
#endif

/*e \brief
 * Writes at most n characters in the output string according to a
 * format.
 *
 * \details
 * Provides same functionality as snprintf.
 * 
 * \param[out] s      Pointer to store the resulting string
 * \param[in]  n      Number of bytes available in the input string
 * \param[in]  format Format to print
 *
 * \return If successful returns the number length of the output string.
 *         If error returns a negative number.
 */
RTITestDllExport int
UTEST_Stdio_snprintf(char *s, size_t n, const char *format, ...);

/* UTEST_Log functions */

/*e \brief
 * Sets UTEST log verbosity
 * 
 * \param[in] verbosity  New verbosity
 */
RTITestDllExport void
UTEST_Log_set_verbosity(UTEST_LogVerbosity_T verbosity);

/* UTEST_Property functions */

/*e \brief
 * Adds a property to the UTEST context. The property string must have the
 * property name followed by symbol '=' and the property value.
 *
 * \details
 * Memory is allocated internally for the property name and value.
 * 
 * \param[in] setting   UTEST context settings
 * \param[in] property  Property name and value separated by symbol '='
 */
RTITestDllExport void
UTEST_Property_add_property(struct UTEST_Context *setting, char *property);

/*e \brief
 * Lookup a property in the UTEST settings.
 *
 * 
 * \param[in] setting   UTEST context settings
 * \param[in] name      Property name
 *
 * \return Pointer to property value if property is found.
 *         If property name is not found return a NULL pointer.
 */
RTITestDllExport const char*
UTEST_Property_lookup_property(struct UTEST_Context *setting,
                               const char *const name);

/*e \brief
 * Lookup an integer property in the UTEST settings. The output value
 * is meaningful only if this return value is 1.
 *
 * 
 * \param[in]  setting  UTEST context settings
 * \param[in]  name     Property name
 * \param[out] value    Pointer to store the property value.
 *
 * \return 0 if property name is not found.
 *         1 if property name is found.
 */
RTITestDllExport int
UTEST_Property_lookup_int_property(struct UTEST_Context *setting,
                                   const char *const name,
                                   int *value);
/*e \brief
 * Lookup an integer property in the UTEST settings. The output value
 * is meaningful only if this return value is 1.
 *
 * 
 * \param[in]  setting  UTEST context settings
 * \param[in]  name     Property name
 * \param[out] value    Pointer to store the property value.
 *
 * \return 0 if property name is not found.
 *         1 if property name is found.
 */
RTITestDllExport int
UTEST_Property_lookup_uint_property(struct UTEST_Context *setting,
                                    const char *const name,
                                    unsigned int *value);

/* UTEST_Main functions */

/*e \brief
 * PArse main input arguments and populates UTEST context settings
 * 
 * \param[out] setting UTEST context settings
 * \param[in]  argc    Number of input arguments in argv
 * \param[in]  argv    Array of input arguments
 * 
 * \return 1 if success or 0 if error.
 */
RTITestDllExport unsigned char
UTEST_Main_parse_arguments(struct UTEST_Context *setting,
                           int argc,
                           char **argv);

/* UTEST_Runner functions */

/*e \brief
 * Runs unit test.
 * 
 * \param[in] setting      UTEST context
 * \param[in] test_name    Test name
 * \param[in] test_vector  Pointer to all tests to run
 * \param[in] count        Number of tests in 'test_vector'
 *
 * \return 1 if sucess. 0 if error.
 */
RTITestDllExport unsigned char
UTEST_Runner_run_tests(struct UTEST_Context *setting,
                       const char* test_name,
                       struct UTEST_TestEntry *test_vector,
                       int count);

/* UTEST_File functions */

/*e \brief
 * Gets a pointer to the string after the last file separator character
 * in the input string
 * 
 * \param[in] filename Input path
 * 
 * \return Pointer to the string after the last file separator character
 * in the input string. If the input string does not have a file separator
 * character, the return value is the same as the input value.
 */
RTITestDllExport const char*
UTEST_File_get_basename(const char *filename);

/* UTEST_Output functions */

/*e \brief
 * Open all output destinations
 * 
 * \param[in] setting UTEST context settings
 * 
 * \return 1 if success or 0 if error.
 */
RTITestDllExport int
UTEST_Output_open(struct UTEST_Context *setting);

/*e \brief
 * Close all output destinations
 */
RTITestDllExport void
UTEST_Output_close(void);

/*e \brief
 * Append to the lua output a string with specified format.
 * 
 * \param[in] setting UTEST context settings
 * \param[in] format  Format string to append
 * 
 * \return 0 if success or -1 if error.
 */
RTITestDllExport int
UTEST_Output_lua_append(struct UTEST_Context *setting, const char *format , ...);

/*e \brief
 * Append a string to lua output.
 * 
 * \param[in] setting  UTEST context settings
 * \param[in] chunk    String to append
 * 
 * \return 0 if success or -1 if error.
 */
RTITestDllExport int
UTEST_Output_lua_write(struct UTEST_Context *setting, const char *chunk);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#ifndef UTEST_ENABLE_LOG
#if NDEBUG && defined(RTI_CERT)
#define UTEST_ENABLE_LOG 0
#else
#define UTEST_ENABLE_LOG 1
#endif
#endif

#ifndef UTEST_ENABLE_TRACE
#if NDEBUG || defined(RTI_CERT)
#define UTEST_ENABLE_TRACE 0
#else
#define UTEST_ENABLE_TRACE 1
#endif
#endif

#if UTEST_ENABLE_TRACE && !UTEST_ENABLE_LOG
#warning "UTEST_ENABLE_TRACE=1 requires UTEST_ENABLE_LOG=1, but UTEST_ENABLE_LOG=0. Forcing UTEST_ENABLE_LOG=1"
#ifdef UTEST_ENABLE_LOG
#undef UTEST_ENABLE_LOG
#endif
#define UTEST_ENABLE_LOG 1
#endif

#ifndef UTEST_ENABLE_PRECONDITION
#if NDEBUG || defined(RTI_CERT)
#define UTEST_ENABLE_PRECONDITION 0
#else
#define UTEST_ENABLE_PRECONDITION 1
#endif
#endif

#ifndef RTI_CERT
#define UTEST_TEST_RUNNER_FINALIZE(sysinit_) \
{\
   if (sysinit_ && !OSAPI_System_finalize()) {  /* return RTI_FALSE; */ }\
}
#else
#define UTEST_TEST_RUNNER_FINALIZE(sysinit_) 
#endif

#if UTEST_ENABLE_LOG
#define UTEST_TEST_RUNNER_INITIALIZE(sysinit_) \
    struct OSAPI_LogProperty log_prop = OSAPI_LogProperty_INIITALIZER;\
    if (sysinit_ && !OSAPI_Log_set_property(&log_prop))\
    {\
        /* return RTI_FALSE; */\
    }\
    if (sysinit_ && !OSAPI_Log_initialize())\
    {\
        /* return RTI_FALSE; */\
    }

#define UTEST_TEST_RUNNER_INITIALIZE_LOG \
    dbglvl = UTEST_Property_lookup_property(setting,"osapi.log.verbosity");\
    if (dbglvl != NULL)\
    {\
        if (!strncmp(dbglvl,"debug",3))\
        {\
            OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_DEBUG);\
        }\
        else if (!strncmp(dbglvl,"warning",4))\
        {\
            OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_WARNING);\
        }\
        else if (!strncmp(dbglvl,"error",3))\
        {\
            OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_ERROR);\
        }\
        else if (!strncmp(dbglvl,"silent",3))\
        {\
            OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_SILENT);\
        }\
        else\
        {\
            return RTI_FALSE;\
        }\
    }

#ifndef RTI_CERT
#define UTEST_TEST_RUNNER_FINALIZE_LOG(sysinit_) \
        if (sysinit_ && !OSAPI_Log_finalize()) { /* return RTI_FALSE; */ }
#else
#define UTEST_TEST_RUNNER_FINALIZE_LOG(sysinit_) 
#endif

#else
#define UTEST_TEST_RUNNER_INITIALIZE(sysinit_)
#define UTEST_TEST_RUNNER_INITIALIZE_LOG
#define UTEST_TEST_RUNNER_FINALIZE_LOG(sysinit_)
#endif

#if UTEST_ENABLE_TRACE || UTEST_ENABLE_LOG
#define UTEST_TEST_RUNNER_DBGLVL  const char *dbglvl = NULL;
#else
#define UTEST_TEST_RUNNER_DBGLVL
#endif

#if UTEST_ENABLE_TRACE
#define UTEST_TEST_RUNNER_INITIALIZE_TRACE \
    dbglvl = UTEST_Property_lookup_property(setting,"osapi.trace.mask");\
    if (dbglvl != NULL)\
    {\
        if (!strncmp(dbglvl,"all",3))\
        {\
            OSAPI_Trace_set_trace_mask(0xffffffff);\
        }\
    }
#else
#define UTEST_TEST_RUNNER_INITIALIZE_TRACE
#endif

#if defined(RTI_LINUX)
/* MICRO-5454 Sleep before exiting to ensure any detached
 * pthreads are cleaned up before the process exits.
 */
#define UTEST_TEST_RUNNER_BEFORE_EXIT \
    if (setting->valgrind)\
    {\
        sleep(10);\
    }
#else
#define UTEST_TEST_RUNNER_BEFORE_EXIT
#endif

#define UT_DEFINE_TEST_RUNNER(test_,name_,luaentry_,sysinit_) \
unsigned char \
test_##_run(struct UTEST_Context *setting) \
{\
    unsigned char retval;\
    UTEST_TEST_RUNNER_DBGLVL \
    UTEST_TEST_RUNNER_INITIALIZE(sysinit_) \
    if (setting->module_name == NULL) \
    {\
        setting->module_name = UTEST_STRINGIFY_DEFINE(RTI_MODULE_NAME);\
    }\
    UTEST_TEST_RUNNER_INITIALIZE_LOG \
    if (sysinit_ && !OSAPI_System_initialize()) \
    {\
        /* return RTI_FALSE; */\
    }\
    UTEST_TEST_RUNNER_INITIALIZE_TRACE \
    if (!UTEST_Output_open(setting))\
    {\
        return RTI_FALSE;\
    };\
    UTEST_Output_lua_write(setting,luaentry_);\
    retval =  (unsigned char)UTEST_Runner_run_tests(setting,name_,\
                    test_##_tests,UTEST_TestEntryCount(test_##_tests));\
    UTEST_TEST_RUNNER_FINALIZE(sysinit_) \
    UTEST_TEST_RUNNER_FINALIZE_LOG(sysinit_) \
    UTEST_Output_close();\
    UTEST_TEST_RUNNER_BEFORE_EXIT\
    return retval;\
}

#ifdef RTI_ARINC653
#define UTEST_VAR static

typedef enum {
    UTEST_NOT_SETUP_STATE,
    UTEST_SETTING_UP_STATE,
    UTEST_SETUP_STATE,
    UTEST_FINISHED_STATE
} UTEST_TestState;

#define UTEST_SETUP_BEGIN \
    static UTEST_TestState UTEST_STATE = UTEST_NOT_SETUP_STATE;\
    if (UTEST_STATE == UTEST_NOT_SETUP_STATE)\
    {\
        UTEST_STATE = UTEST_SETTING_UP_STATE;\

#define UTEST_SETUP_END \
        UTEST_STATE = UTEST_SETUP_STATE;\
        return RTI_TRUE;\
    } else if (UTEST_STATE != UTEST_SETUP_STATE)\
    {\
        return RTI_FALSE;\
    }\
    UTEST_STATE = UTEST_FINISHED_STATE;

#define UTEST_SETUP_ONLY \
    static UTEST_TestState UTEST_STATE = UTEST_NOT_SETUP_STATE;\
    if (UTEST_STATE == UTEST_SETUP_STATE)\
    {\
        return RTI_TRUE;\
    }\
    UTEST_STATE = UTEST_SETUP_STATE;

#else
#define UTEST_VAR
#define UTEST_SETUP_BEGIN
#define UTEST_SETUP_END
#define UTEST_SETUP_ONLY
#endif


#ifdef RTI_VX653
#include <vxWorks.h>
#include <hostLib.h>
#include <653POSAPI.h>
#if RTI_VX653 >= 2500
#include <apex/ARINC653.h>
#else
#include <apex/apexLib.h>
#endif

#ifdef RTI_CERT
#define UT_DEFINE_MAIN(entry_,name_) \
struct UTEST_Context entry_##_setting;\
UT_DEFINE_ARINC_PROCESS(entry_,name_)\
void \
_653AppEntry(validatePOSAPIFuncPtr_t validatePOSAPIFuncPtr)\
{\
    arinc653POSAPITable = (*validatePOSAPIFuncPtr)(arinc653POSAPIVersionString);\
    UT_ARINC_MAIN(entry_,name_)\
    return;\
}
#else
#define UT_DEFINE_MAIN(entry_,name_) \
struct UTEST_Context entry_##_setting;\
UT_DEFINE_ARINC_PROCESS(entry_,name_)\
void \
usrAppInit(void) \
{\
    UT_ARINC_MAIN(entry_,name_)\
    return;\
}
#endif

#elif defined(RTI_DEOS)
#include <apex.h>
#include <deos.h>
#include <debug.h>
#include <videobuf.h>
#if defined(UT_ARINC_CONTROLLER) || defined(UT_ARINC_WORKER)
#include "print.h"
#endif

#if defined(UT_ARINC_CONTROLLER)
#define UT_DEOS_SETUP_PRINTF initVideoPrintf(0, 0, 10, 80); \
    initFtpPrintf("TestResults", 0, 8192);
#else
#define UT_DEOS_SETUP_PRINTF initVideoPrintf(11, 0, 10, 80); \
    initFtpPrintf("TestResults", 8192, 8192);
#endif

#define UT_DEFINE_MAIN(entry_,name_) \
struct UTEST_Context entry_##_setting;\
UT_DEFINE_ARINC_PROCESS(entry_,name_)\
void \
main(void) \
{\
    UT_DEOS_SETUP_PRINTF\
    UT_ARINC_MAIN(entry_,name_)\
    return;\
}

#elif defined(RTI_VXWORKS) && !defined(RTI_RTP) && !defined(RTI_DARWIN)
#include <vxWorks.h>
#ifndef RTI_CERT
#include <taskLib.h>
#define UT_DEFINE_MAIN(entry_,name_) \
unsigned char \
entry_##_start(int beginSubmoduleTestIndex,\
               int beginUnitTestIndex, \
               int endSubmoduleTestIndex, \
               int endUnitTestIndex, RTI_BOOL ignoreFailure, \
               const char *args);\
unsigned char \
entry_##_startALL(RTI_BOOL ignoreFailure,const char* args);\
unsigned char \
entry_##_start(int beginSubmoduleTestIndex,\
               int beginUnitTestIndex, \
               int endSubmoduleTestIndex, \
               int endUnitTestIndex, RTI_BOOL ignoreFailure, \
               const char *args) \
{ \
    RTI_INT32 argc = 0;\
    struct UTEST_Context setting;\
    char **argv;\
    (void)beginSubmoduleTestIndex;\
    (void)beginUnitTestIndex;\
    (void)endSubmoduleTestIndex;\
    (void)endUnitTestIndex;\
    (void)ignoreFailure;\
    UTEST_Context_init(&setting);\
    taskSafe();\
    UTEST_String_argv_from_string(name_,args,&argc,&argv);\
    if (!UTEST_Main_parse_arguments(&setting, argc, argv))\
    {\
        return 1;\
    }\
    return entry_##_run(&setting);\
}\
unsigned char \
entry_##_startALL(RTI_BOOL ignoreFailure,const char* args)\
{\
    return entry_##_start(-1,-1,-1,-1, ignoreFailure, args);\
}
#else
#define UT_DEFINE_MAIN(entry_,name_) \
unsigned char  \
entry_##_start(int beginSubmoduleTestIndex,\
               int beginUnitTestIndex, \
               int endSubmoduleTestIndex, \
               int endUnitTestIndex, RTI_BOOL ignoreFailure, \
               const char *args);\
unsigned char \
entry_##_startALL(RTI_BOOL ignoreFailure,const char* args);\
unsigned char \
entry_##_start(int beginSubmoduleTestIndex,\
               int beginUnitTestIndex, \
               int endSubmoduleTestIndex, \
               int endUnitTestIndex, RTI_BOOL ignoreFailure, \
               const char *args) \
{ \
    RTI_INT32 argc = 0;\
    char **argv;\
    struct UTEST_Context setting;\
    (void)beginSubmoduleTestIndex;\
    (void)beginUnitTestIndex;\
    (void)endSubmoduleTestIndex;\
    (void)endUnitTestIndex;\
    (void)ignoreFailure;\
    UTEST_Context_init(&setting);\
    UTEST_String_argv_from_string(name_,args,&argc,&argv);\
    if (!UTEST_Main_parse_arguments(&setting, argc, argv))\
    {\
        return 1;\
    }\
    return entry_##_run(&setting);\
}\
unsigned char  \
entry_##_startALL(RTI_BOOL ignoreFailure,const char* args)\
{\
    return entry_##_start(-1,-1,-1,-1, ignoreFailure, args);\
}
#endif
#elif defined(__ANDROID__)

#include <android/log.h>
#include <android_native_app_glue.h>
#include <GLES/gl.h>
#include <EGL/egl.h>

struct eglengine
{
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    struct UTEST_Context setting;
};

extern void
custom_handle_cmd(struct android_app* app, int32_t cmd);

extern int32_t
custom_handle_input(struct android_app* app, AInputEvent* event);
#define UT_DEFINE_MAIN(entry_,name_) \
int \
RTITester_unit_main(struct UTEST_Context *setting) \
{\
    return !entry_##_run(setting);\
}\
void \
android_main(struct android_app* state)\
{\
    int events;\
    char **argv;\
    int argc = 0;\
    struct eglengine engine;\
    app_dummy();\
    memset(&engine, 0, sizeof(engine));\
    state->userData = &engine;\
    state->onAppCmd = custom_handle_cmd;\
    state->onInputEvent = custom_handle_input;\
    UTEST_Context_init(&engine.setting);\
    UTEST_String_argv_from_string(name_,UTEST_ARG_STRING(name_),&argc,&argv);\
    if (!UTEST_Main_parse_arguments(&engine.setting, argc, argv))\
    {\
        return;\
    }\
    while (1)\
    {\
        struct android_poll_source* source;\
        while (ALooper_pollAll(-1, NULL, &events, (void**)&source) >= 0)\
        {\
            if (source != NULL)\
            {\
                source->process(state, source);\
            }\
            if (state->destroyRequested != 0)\
            {\
                return;\
            }\
        }\
    }\
}
#elif defined(RTI_IOS)
#ifdef __cplusplus
extern "C" {
    int ios_main(void);
}
#else
extern int ios_main(void);
#endif
#define UT_DEFINE_MAIN(entry_,name_) \
int \
ios_main(void)\
{\
    char **argv;\
    int argc = 0;\
    struct UTEST_Context setting;\
    UTEST_Context_init(&setting);\
    UTEST_String_argv_from_string(name_,UTEST_ARG_STRING(name_),&argc,&argv);\
    if (!UTEST_Main_parse_arguments(&setting, argc, argv))\
    {\
        return 1;\
    }\
    return !entry_##_run(&setting);\
}
#elif !defined(NO_RTI_UNIT_MAIN)
#if HAVE_ARG_STRING
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE || __ANDROID__
#define UT_DEFINE_MAIN(entry_,name_) \
int \
main(int argc, char **argv) \
{ \
    char **argvi;\
    int argci = 0;\
    struct UTEST_Context setting;\
    UTEST_Context_init(&setting);\
    UTEST_String_argv_from_string(name_,UTEST_ARG_STRING(name_),&argci,&argvi);\
    if (!UTEST_Main_parse_arguments(&setting, argci, argvi))\
    {\
        UTEST_Stdio_printf("failed to parse argument string [%s]\n",UTEST_ARG_STRING(name_));\
        return -1;\
    }\
    return !entry_##_run(&setting);\
}
#else
#define UT_DEFINE_MAIN(entry_,name_) \
int \
UTEST_main(void) \
{ \
    char **argvi;\
    int argci = 0;\
    struct UTEST_Context setting;\
    UTEST_Context_init(&setting);\
    UTEST_String_argv_from_string(name_,UTEST_ARG_STRING(name_),&argci,&argvi);\
    if (!UTEST_Main_parse_arguments(&setting, argci, argvi))\
    {\
        UTEST_Stdio_printf("failed to parse argument string [%s]\n",UTEST_ARG_STRING(name_));\
        return -1;\
    }\
    return !entry_##_run(&setting);\
}
#endif
#else
#if !defined(ENTRYPOINT)
#define ENTRYPOINT main
#endif
#define UT_DEFINE_MAIN(entry_,name_) \
int \
ENTRYPOINT(int argc, char **argv); \
int \
ENTRYPOINT(int argc, char **argv) \
{ \
    struct UTEST_Context setting;\
    int result = 0;\
    UTEST_Context_init(&setting);\
    if (!UTEST_Main_parse_arguments(&setting, argc, argv))\
    {\
        return -1;\
    }\
    while (setting.repeat)\
    {\
        setting.repeat--;\
        result = result || !entry_##_run(&setting);\
    }\
    return result;\
}
#endif
#else
#define UT_DEFINE_MAIN(entry,name_)
#endif

#define UT_DEFINE_TEST(entry_,name_,luaentry_) \
    UT_DEFINE_TEST_RUNNER(entry_,name_,luaentry_,1)\
    UT_DEFINE_MAIN(entry_,name_)

#define UT_DEFINE_TEST_NO_SYS_INIT(entry_,name_,luaentry_) \
    UT_DEFINE_TEST_RUNNER(entry_,name_,luaentry_,0)\
    UT_DEFINE_MAIN(entry_,name_)

#define UT_DEFINE_SUBMODULE_RUNNER(entry_,name_) \
unsigned char \
entry_##_run(struct UTEST_Context *setting)\
{\
    unsigned char rval;\
    rval = UTEST_Runner_run_tests(setting,name_,entry_##_tests,\
                                UTEST_TestEntryCount(entry_##_tests));\
    return rval;\
}

#include "test/test_setting_impl.h"


#endif /* test_setting_h */
