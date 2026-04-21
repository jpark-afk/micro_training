/*
 * FILE: UTEST_Main.c - Unit-test main entry
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
 * 31dec2013,tk Refactored from Setting.c
 * 01aug2012,tk Many enhancements
 * 01dec2004,cc Created, based on Waveworks tree.
 */
/*ce
 * \file UTEST_Main.c
 * \brief Unit-test main entry
 */
#include "test/test_setting.h"

#include "UTEST_Property.h"
#include "UTEST_Runner.h"
#include "UTEST_File.h"
#include "UTEST_System.h"
#include "UTEST_String.h"
#include "UTEST_Stdio.h"
#include "UTEST_Output.h"

UTEST_LogVerbosity_T UTEST_gv_VerbosityLevel = UTEST_LOGVERBOSITY_SILENT;

#if HAVE_CONFIG_FILE && DEVTREE_BUILD
static const char* const UTEST_gv_DefaultConfigFile = "resource.1.0/buildAutomation/configFiles/unittest.cfg";
#endif

void
UTEST_Log_set_verbosity(UTEST_LogVerbosity_T verbosity)
{
    UTEST_gv_VerbosityLevel = verbosity;
}

/*
 * Valid options
 * -config <file>         - Read configuration from <file>
 * -list                  - List all test-cases
 * -user <user>           - Run tests as user <user>
 * -os <OS name>          - The OS is <os>
 * -hostname <host>       - The hostname of machine is <hostname>
 * -arch <arch>           - The run-time archicture is <arch>
 * -include <regex>       - Run all tests matching <name>. Multiple can be given
 * -exclude <regex>       - Run all tests not matching <name>. Multiple can be given
 * -verbosity <level>     - Log verbosity (error,warning,debug,silent)
 * -property "name=value" - Add property "name=value"
 * -ignore                - Ignore failures (if possible)
 * -sandbox               - Run each test in its own process (if possible)
 * -valgrind              - The test is being run under valgrind
 *
 */
static void
UTEST_Main_print_help(void)
{
    UTEST_Stdio_printf("<unit-tester> -id <domain_id> [options]\n\n");
    UTEST_Stdio_printf("Typical options:\n");
    UTEST_Stdio_printf("-config <file>         - Read configuration from <file>\n");
    UTEST_Stdio_printf("-list                  - List all test-cases\n");
    UTEST_Stdio_printf("-user <user>           - Run tests as user <user>\n");
    UTEST_Stdio_printf("-include <regex>       - Run all tests matching <name>. Multiple can be given\n");
    UTEST_Stdio_printf("-exclude <regex>       - Run all tests not matching <name>. Multiple can be given\n");
    UTEST_Stdio_printf("-property \"name=value\" - Add property \"name=value\"\n");
    UTEST_Stdio_printf("-ignore                - Ignore failures if possible\n");
    UTEST_Stdio_printf("-verbosity             - Log verbosity (error,warning,debug,silent)\n");
    UTEST_Stdio_printf("\n");
    UTEST_Stdio_printf("\n");
    UTEST_Stdio_printf("Advanced options:\n");
    UTEST_Stdio_printf("-os <OS name>          - The OS is <os>\n");
    UTEST_Stdio_printf("-arch <arch>           - The run-time archicture is <arch>\n");
    UTEST_Stdio_printf("-hostname <host>       - The hostname of machine is <hostname>\n");
    UTEST_Stdio_printf("-sandbox               - Run each test in its own process (if possible)\n");
    UTEST_Stdio_printf("-valgrind              - The test is being run under valgrind\n");
}

unsigned char
UTEST_Main_parse_arguments(struct UTEST_Context *setting,
                           int argc, char **argv)
{
    int i;
    const char *debug_level = NULL;
#if HAVE_CONFIG_FILE
    const char *config_file = NULL;
#endif
    char *ptr;

    UTEST_System_get_info(&setting->sysinfo);

    /* only use basename of nodename */
    ptr = setting->sysinfo.nodename;
    while (*ptr && (*ptr != '.'))
    {
        if ((*ptr >= 'A') && (*ptr <= 'Z'))
        {
            *ptr = (char)(*ptr - 'A' + 'a');
        }
        ptr++;
    }

    if (*ptr)
    {
        *ptr = 0;
    }

    UTEST_Log_set_verbosity(UTEST_LOGVERBOSITY_ERROR);
    sprintf(setting->dts_path,".");

    setting->platform_name = NULL;

    for (i = 0; i < UTEST_SETTING_MAX_PROPERTIES; ++i)
    {
        setting->property[i].name = NULL;
        setting->property[i].value = NULL;
    }

    setting->property_length = 0;
    setting->rtiarch = UTEST_STRINGIFY_DEFINE(RTIME_TARGET_NAME);

    for (i = 1; i < argc; ++i)
    {
        if (strncmp(argv[i], "-help", 4) == 0)
        {
            UTEST_Main_print_help();
#if defined(RTI_ARINC653)
            goto fail;
#else
            exit(1);
#endif
        }
        else if (strncmp(argv[i], "-dts", 4) == 0)
        {
#if HAVE_CONFIG_FILE
            if (++i >= argc)
            {
                goto fail;
            }
            sprintf(setting->dts_path,"%s",argv[i]);
#else
            goto fail;
#endif
        }
        else if (strncmp(argv[i], "-list", 4) == 0)
        {
            setting->path_expr = "<><><><>";
            setting->print_all = 1;
            setting->dont_run = 1;
        }
        else if (strncmp(argv[i], "-all", 4) == 0)
        {
            setting->print_all = 1;
        }
        else if (strncmp(argv[i], "-cm", 3) == 0)
        {
            setting->module_from_path = 1;
        }
        else if (strncmp(argv[i], "-lua", 4) == 0)
        {
            setting->no_lua = 0;
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                ++i;
                setting->lua_path = argv[i];
            }
        }
        else if (strncmp(argv[i], "-type", 5) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->file_type = argv[i];
        }
        else if (strncmp(argv[i], "-module", 7) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->module_name = argv[i];
        }
        else if (strncmp(argv[i], "-log", 4) == 0)
        {
            setting->no_log = 0;
        }
        else if (strncmp(argv[i], "-sql", 4) == 0)
        {
            setting->no_sql = 0;
        }
        else if (strncmp(argv[i], "-config", 4) == 0)
        {
#if HAVE_CONFIG_FILE
            if (++i >= argc)
            {
                goto fail;
            }
            config_file = argv[i];
#else
            goto fail;
#endif
        }
        else if (strncmp(argv[i], "-os", 3) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->sysname = argv[i];
        }
        else if (strncmp(argv[i], "-user", 5) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->user = argv[i];
        }
        else if (strncmp(argv[i], "-id", 3) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->domain_id = (int)strtol(argv[i],NULL,0);
        }
        else if (strncmp(argv[i], "-hostname", 9) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->nodename = argv[i];
        }
        else if (strncmp(argv[i], "-arch", 5) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->rtiarch = argv[i];
        }
        else if (strcmp(argv[i], "-verbosity") == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            if (!strncmp(argv[i],"debug",3))
            {
                UTEST_Log_set_verbosity(UTEST_LOGVERBOSITY_DEBUG);
            }
            else if (!strncmp(argv[i],"warning",4))
            {
                UTEST_Log_set_verbosity(UTEST_LOGVERBOSITY_WARNING);
            }
            else if (!strncmp(argv[i],"error",3))
            {
                UTEST_Log_set_verbosity(UTEST_LOGVERBOSITY_ERROR);
            }
            else if (!strncmp(argv[i],"silent",3))
            {
                UTEST_Log_set_verbosity(UTEST_LOGVERBOSITY_SILENT);
            }
            else
            {
                    UTEST_Stdio_printf("invalid test debug level: %s",argv[i]);
                    goto fail;
            }
        }
        else if (strncmp(argv[i], "-property", 4) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            UTEST_Property_add_property(setting,argv[i]);
        }
        else if (strncmp(argv[i], "-ulog", 4) == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            UTEST_Property_add_property_nv(setting,
                                           UTEST_String_strdup("osapi.log.verbosity"),
                                           UTEST_String_strdup(argv[i]));

            if (!strcmp(argv[i],"silent") || !strncmp(argv[i],"error",3) ||
                !strncmp(argv[i],"warning",4) || !strcmp(argv[i],"info") ||
                !strncmp(argv[i],"debug",3))
            {
                UTEST_Property_add_property_nv(setting,
                                               UTEST_String_strdup("osapi.log.verbosity"),
                                               UTEST_String_strdup(argv[i]));
            }
            else
            {
                UTEST_Stdio_printf("-log silent | warning | error | info must be specified\n");
                goto fail;
            }
        }
        else if (strcmp(argv[i], "-include") == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->path_expr = argv[i];
        }
        else if (strcmp(argv[i], "-build_root") == 0)
        {
            if (++i >= argc)
            {
                goto fail;
            }
            setting->build_root = argv[i];
        }
        else if ((strncmp(argv[i], "-ignoreFailure", 14) == 0) ||
                 (strncmp(argv[i], "-ignore", 7) == 0))
        {
            setting->ignoreFailure = 1;
        }
        else if (strcmp(argv[i], "-verbose") == 0)
        {
            setting->verbose = 1;
        }
        else if(strcmp(argv[i], "-valgrind") == 0)
        {
            setting->valgrind = 1;
        }
        else
        {
            goto fail;
        }
    }

    if (setting->domain_id == -1)
    {
        UTEST_Stdio_printf("-id <domain_id> must be specified\n");
        goto fail;
    }

    UTEST_Stdio_snprintf(setting->arch_as_filename,UTEST_MAX_ARCH_NAME,"%s",setting->rtiarch);
    ptr = setting->arch_as_filename;

    while (*ptr)
    {
        if (*ptr == '.')
        {
            *ptr = '_';
        }
        ++ptr;
    }

    setting->argv = argv;
    setting->argc = argc;

    if (setting->nodename == NULL)
    {
        setting->nodename = setting->sysinfo.nodename;
    }

    if (setting->sysname == NULL)
    {
        setting->sysname = setting->sysinfo.sysname;
    }

    if (setting->rtiarch == NULL)
    {
        setting->rtiarch = setting->sysinfo.rtiarch;
    }

#if HAVE_CONFIG_FILE
    if (config_file != NULL)
    {
        if (UTEST_Property_read_property_file(config_file,&setting->sysinfo,setting) < 0)
        {
            goto fail;
        }
    }
#if DEVTREE_BUILD
    else
    {
        if (UTEST_Property_read_property_file(UTEST_gv_DefaultConfigFile,&setting->sysinfo,setting) < 0)
        {
            goto fail;
        }
    }
#endif
#endif

#if defined(RTI_CERT)
    /* Setting for VxWorks Cert target, XCalibur1700 */
    UTEST_Property_add_property(setting,UTEST_String_strdup("netio.udp.allow_interface_multicast = 1"));
    UTEST_Property_add_property(setting,UTEST_String_strdup("netio.udp.allow_interface = mottsec0"));
    UTEST_Property_add_property(setting,UTEST_String_strdup("netio.udp.allow_interface_address = 0x0a0a1ecd"));
    UTEST_Property_add_property(setting,UTEST_String_strdup("netio.udp.allow_interface_netmask = 0xffff0000"));
    UTEST_Property_add_property(setting,UTEST_String_strdup("netio.udp.multicast_if = mottsec0"));
    UTEST_Property_add_property(setting,UTEST_String_strdup("osapi.system.my_hostname = unknown"));
#endif

    debug_level = UTEST_Property_lookup_property(setting,"osapi.log.verbosity");
    if (debug_level == NULL)
    {
        UTEST_Property_add_property_nv(setting,
                                       UTEST_String_strdup("osapi.log.verbosity"),
                                       UTEST_String_strdup("silent"));
    }

    return 1;

fail:

    return 0;
}

