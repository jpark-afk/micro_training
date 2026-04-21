/*
 * FILE: UTEST_Property.c - Unit-test output support
 *
 * (c) Copyright, Real-Time Innovations, 2013-2020
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
 * 31dec2013,tk Refactored from UT_Property.c
 * 08nov2013,tk Written
 */
/*ce
 * \file
 * \brief Unit-test output support
 */
#include "test/test_setting.h"
#include "UTEST_Output.h"
#include "UTEST_File.h"

static struct UTEST_Output UTEST_gv_Output;

#define UTEST_LUA_BUFFER (8*1024)
#define UTEST_SQL_BUFFER (8*1024)
#define UTEST_BASENAME "unittest"

#if HAVE_TEST_RESULTS_FILE
static char UTEST_Context_g_lua_buffer[UTEST_LUA_BUFFER];
static char UTEST_Context_g_lua_buffer_scratch[32];
static char *UTEST_Context_g_lua_buffer_ptr = UTEST_Context_g_lua_buffer;
static char *UTEST_Context_g_lua_buffer_end = &UTEST_Context_g_lua_buffer[UTEST_LUA_BUFFER];
#endif /* HAVE_TEST_RESULTS_FILE */
static const char *const UTEST_Context_gv_LuaPath = "rti_me.2.0/resource/datasheet";
static const char *const UTEST_Context_gv_LogPath = ".";
static const char *const UTEST_Context_gv_SqlPath = ".";

static int
UTEST_Output_create_file(int *outfd,
                         struct UTEST_Context *setting,
                         const char *const in_path,
                         const char *const basename,
                         const char *const type,
                         const char *const suffix)
{
#if HAVE_TEST_RESULTS_FILE
    char filename[1024];
    char path[1024] = {'\0'};
    int len;
    int fd;
#if NDEBUG
    const char *kind = "release";
#else
    const char *kind = "debug";
#endif

    *outfd = -1;

    if (setting->build_root != NULL)
    {
        len = UTEST_Stdio_snprintf(path,1024,"%s/%s",setting->build_root,in_path);
    }
    else
    {
        len = UTEST_Stdio_snprintf(path,1024,"%s",in_path);
    }

    if (len >= 255)
    {
        UTEST_Log_error("Could not create filename %s/%s",
                         setting->build_root,in_path);
        return 0;
    }

    if (type == NULL)
    {
        len = UTEST_Stdio_snprintf(filename,255,"%s/%s_%s_%s_%s.%s",
                path,basename,setting->module_name,
                setting->rtiarch,kind,
                suffix);
        if (len >= 255)
        {
            UTEST_Log_error("Could not create log filename %s/%s_%s_%s_%s.%s",
                    path,basename,setting->module_name,
                    setting->rtiarch,kind,
                    suffix);
            return 0;
        }

    }
    else
    {
        len = UTEST_Stdio_snprintf(filename,255,"%s/%s_%s_%s_%s_%s.%s",
                path,basename,setting->module_name,
                setting->rtiarch,kind,type,
                suffix);
        if (len >= 255)
        {
            UTEST_Log_error("Could not create log filename %s/%s_%s_%s_%s_%s.%s",
                    path,basename,setting->module_name,
                    setting->rtiarch,kind,type,
                    suffix);
            return 0;
        }
    }

    fd = UTEST_File_open_file(filename,O_CREAT | O_RDWR | O_TRUNC,0666);
    if (fd < 0)
    {
        UTEST_Log_error("Could not create file %s",filename);
        return 0;
    }

    UTEST_Stdio_printf("Created file %s",filename);

    *outfd = fd;

    return 1;
#else

    (void)(outfd);
    (void)(setting);
    (void)(in_path);
    (void)(basename);
    (void)(type);
    (void)(suffix);

    return 0;
#endif /* HAVE_TEST_RESULTS_FILE */
}

int
UTEST_Output_open(struct UTEST_Context *setting)
{
    UTEST_gv_Output.lua_fd = -1;
    UTEST_gv_Output.sql_fd = -1;
    UTEST_gv_Output.log_fd = -1;

    if (!setting->no_log &&
        !UTEST_Output_create_file(&UTEST_gv_Output.log_fd,setting,
                                  UTEST_Context_gv_LogPath,
                                  UTEST_BASENAME,setting->file_type,"log"))
    {
        return 0;
    }

    /* SQL and Lua _always goes to a file if requested */
    if (!setting->no_lua)
    {
        if (!UTEST_Output_create_file(&UTEST_gv_Output.lua_fd,setting,
                          ((setting->lua_path != NULL) ?
                                  setting->lua_path : UTEST_Context_gv_LuaPath),
                          UTEST_BASENAME,setting->file_type,"lua"))
        {
            printf("failed to create lua file %s\n",setting->lua_path);
            return 0;
        }

#if NDEBUG
        UTEST_Output_lua_append(setting,"if (rti_me.heap.%s == nil) then \n"
                                        "    rti_me.heap.%s={};\n"
                                        "end\n",setting->arch_as_filename,
                                                setting->arch_as_filename);
        UTEST_Output_lua_append(setting,"if (rti_me.heap.%s.release == nil) then \n"
                                    "    rti_me.heap.%s.release={};\n"
                                    "end\n",setting->arch_as_filename,
                                            setting->arch_as_filename);
    UTEST_Output_lua_append(setting,"rti_me_heap = rti_me.heap.%s.release;\n",
                            setting->arch_as_filename);
#else
    UTEST_Output_lua_append(setting,"if (rti_me.heap.%s == nil) then \n"
                                    "    rti_me.heap.%s={};\n"
                                    "end\n",setting->arch_as_filename,
                                            setting->arch_as_filename);
    UTEST_Output_lua_append(setting,"if (rti_me.heap.%s.debug == nil) then \n"
                                    "    rti_me.heap.%s.debug={};\n"
                                    "end\n",setting->arch_as_filename,
                                            setting->arch_as_filename);
    UTEST_Output_lua_append(setting,"rti_me_heap = rti_me.heap.%s.debug;\n",
                            setting->arch_as_filename);
#endif
    }

    if (!setting->no_sql &&
        !UTEST_Output_create_file(&UTEST_gv_Output.sql_fd,setting,
                                  UTEST_Context_gv_SqlPath,
                                  UTEST_BASENAME,setting->file_type,"sql"))
    {
        return 0;
    }

    return 1;
 }

void
UTEST_Output_close(void)
{
#if HAVE_TEST_RESULTS_FILE
    if (UTEST_gv_Output.log_fd != -1)
    {
        close(UTEST_gv_Output.log_fd);
    }

    if (UTEST_gv_Output.lua_fd != -1)
    {
        if (UTEST_Context_g_lua_buffer_ptr > UTEST_Context_g_lua_buffer)
        {
            *(UTEST_Context_g_lua_buffer_ptr) = 0;

            if (write(UTEST_gv_Output.lua_fd,UTEST_Context_g_lua_buffer,
                    (unsigned int)(UTEST_Context_g_lua_buffer_ptr - UTEST_Context_g_lua_buffer)) < 0)
            {
#ifndef RTI_CERT
                perror("UTEST_Output_close:write");
#endif
            }
        }
        close(UTEST_gv_Output.lua_fd);
        UTEST_gv_Output.lua_fd = -1;
    }

    if (UTEST_gv_Output.sql_fd != -1)
    {
        close(UTEST_gv_Output.sql_fd);
        UTEST_gv_Output.sql_fd = -1;
    }
#endif /* HAVE_TEST_RESULTS_FILE */
}

int
UTEST_Output_lua_append(struct UTEST_Context *setting,
                        const char *format, ...)
{
#if HAVE_TEST_RESULTS_FILE
    va_list arglist;
    int length;
    (void)setting;

    if (UTEST_gv_Output.lua_fd == -1)
    {
        return 0;
    }

    va_start(arglist,format);

    /* Do a test print to see if the statements fit */
#if defined(_MSC_VER) || defined(WIN32)
    length = vsnprintf_s(UTEST_Context_g_lua_buffer_scratch,
                         32,_TRUNCATE,format,arglist);
#elif (defined(__vxworks) && (VXWORKS_MAJOR_VERSION == 5)) || \
        ((__GNUC__ == 3) && (__GNUC_MINOR__ == 3))
    length = vsprintf(UTEST_Context_g_lua_buffer_scratch,format,arglist);
#else
    length = vsnprintf(UTEST_Context_g_lua_buffer_scratch,32,format,arglist);
#endif

    va_end(arglist);

    /* Need room for NULL termination */
    if (length > (UTEST_LUA_BUFFER - 1))
    {
        return -1;
    }

    /* Flush current string if there is no room in current buffer */
    if ((UTEST_Context_g_lua_buffer_end - UTEST_Context_g_lua_buffer_ptr)
            < length)
    {
        *UTEST_Context_g_lua_buffer_ptr = 0;
#if defined(_MSC_VER) || defined(WIN32)
        if (write(UTEST_gv_Output.lua_fd,UTEST_Context_g_lua_buffer,
                  (unsigned int)(UTEST_Context_g_lua_buffer_ptr - UTEST_Context_g_lua_buffer)) < 0)
#else
        if (write(UTEST_gv_Output.lua_fd,UTEST_Context_g_lua_buffer,
                  (size_t)(UTEST_Context_g_lua_buffer_ptr - UTEST_Context_g_lua_buffer)) < 0)
#endif
        {
#ifndef RTI_CERT
            perror("UTEST_Output_lua_append:write");
#endif
            return -1;
        }

        UTEST_Context_g_lua_buffer_ptr = UTEST_Context_g_lua_buffer;
        *UTEST_Context_g_lua_buffer_ptr = 0;
    }

    va_start(arglist,format);

#if defined(_MSC_VER) || defined(WIN32)
    length = vsnprintf_s(UTEST_Context_g_lua_buffer_ptr,
            length+1,_TRUNCATE,format,arglist);
#elif (defined(__vxworks) && (VXWORKS_MAJOR_VERSION == 5))  || \
        ((__GNUC__ == 3) && (__GNUC_MINOR__ == 3))
    length = vsprintf(UTEST_Context_g_lua_buffer_ptr,format,arglist);
#else
    length = vsnprintf(UTEST_Context_g_lua_buffer_ptr,(size_t)(length+1),format,arglist);
#endif

    va_end(arglist);

    /* Move ptr at the termination character */
    UTEST_Context_g_lua_buffer_ptr += length;
    *UTEST_Context_g_lua_buffer_ptr = 0;
#else
    (void)setting;
    (void)format;
#endif /* HAVE_TEST_RESULTS_FILE */

    return 0;
}

int
UTEST_Output_lua_write(struct UTEST_Context *setting,
                       const char *chunk)
{
    return UTEST_Output_lua_append(setting,"%s\n",chunk);
}

