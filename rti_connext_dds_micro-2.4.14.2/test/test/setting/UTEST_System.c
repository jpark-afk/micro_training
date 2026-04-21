/*
 * FILE: UTEST_System.c - Unit-test System information
 *
 * (c) Copyright, Real-Time Innovations, 2013-2015.
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
 * \file UTEST_System.c
 * \brief Unit-test System information
 */
#include "test/test_setting.h"
#include "UTEST_System.h"
#include "UTEST_Stdio.h"

/*** SOURCE_BEGIN ***/

unsigned int UTEST_thread_port_properties_callout(void *threadProperties)
{
#if 0
    /* Fill in with the platform-specific thread properties populator,
     * if needed. Ex.
     * return UTEST_safertos_set_thread_properties(threadProperties);
     */
#else
    (void)(threadProperties);

    return 1;
#endif
}

unsigned int UTEST_system_port_properties_callout() {
#if defined(RTI_AUTOSAR)
    return UTEST_autosar_set_system_properties();
#else
    return 1;
#endif
}

char
UTEST_System_get_info(struct UTEST_SystemInfo *sysinfo)
{
#if defined(RTI_UNIX)
    struct utsname utsinfo;

    if (uname(&utsinfo) < 0)
    {
        return 0;
    }

    /*
     * machine  Machine hardware platform.
     * nodename Network name of this machine.
     * release  Release level of the operating system.
     * sysname  Name of the operating system implementation.
     * version  Version level of the operating system.
     * rtiarch  RTI Architecture
     */
    UTEST_Stdio_snprintf(sysinfo->machine,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",utsinfo.machine);
    UTEST_Stdio_snprintf(sysinfo->nodename,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",utsinfo.nodename);
    UTEST_Stdio_snprintf(sysinfo->release,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",utsinfo.release);
    UTEST_Stdio_snprintf(sysinfo->sysname,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",utsinfo.sysname);
    UTEST_Stdio_snprintf(sysinfo->version,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",utsinfo.version);

#elif defined(RTI_AUTOSAR)
    UTEST_Stdio_snprintf(sysinfo->machine, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "TC297TFT");
    UTEST_Stdio_snprintf(sysinfo->nodename, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Autosar-host");
    UTEST_Stdio_snprintf(sysinfo->release, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "v6.2r2");
    UTEST_Stdio_snprintf(sysinfo->sysname, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Autosar");
    UTEST_Stdio_snprintf(sysinfo->version, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Autosar");

#elif defined(_MSC_VER) || defined(WIN32)
    char nodename[MAX_COMPUTERNAME_LENGTH + 1];
    SYSTEM_INFO winsysinfo;
    OSVERSIONINFO osversion;
    DWORD maxnodename = UTEST_SYSTEM_NAME_MAX_LENGTH;

    GetSystemInfo(&winsysinfo);

    switch (winsysinfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        UTEST_Stdio_snprintf(sysinfo->machine,
                             UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","x86_64");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        UTEST_Stdio_snprintf(sysinfo->machine,
                             UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","ARM");
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        UTEST_Stdio_snprintf(sysinfo->machine,
                             UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","IA64");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        UTEST_Stdio_snprintf(sysinfo->machine,
                             UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","i386");
        break;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
        /* let it fall through */
    default:
        UTEST_Stdio_snprintf(sysinfo->machine,
                             UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","unknown");
        break;
    }

    /* nodename */
    nodename[0] = 0;

    if (!GetComputerName(nodename,&maxnodename))
    {
        return 0;
    }

    UTEST_Stdio_snprintf(sysinfo->nodename,
                         UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",nodename);

    osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osversion))
    {
        return 0;
    }

    UTEST_Stdio_snprintf(sysinfo->release,UTEST_SYSTEM_NAME_MAX_LENGTH,"%d.%d",
                         osversion.dwMajorVersion,
                         osversion.dwMinorVersion);

    /* system, Always this */
    UTEST_Stdio_snprintf(sysinfo->sysname,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows");

    /* version
  UTEST_Stdio_snprintf an approximation
     */

#define DT_OS_VERSION_IS(mj_,mi_) (osversion.dwMajorVersion == (mj_) && \
                                   osversion.dwMinorVersion == (mi_))

    if (DT_OS_VERSION_IS(6,2))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows 8");
    }
    else if (DT_OS_VERSION_IS(6,1))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows 7");
    }
    else if (DT_OS_VERSION_IS(6,0))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows Vista");
    }
    else if (DT_OS_VERSION_IS(5,2))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows Server 2003");
    }
    else if (DT_OS_VERSION_IS(5,1))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows XP");
    }
    else if (DT_OS_VERSION_IS(5,0))
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows 2000");
    }
    else
    {
        UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","Windows version unknown");
    }

#undef DT_OS_VERSION_IS
#elif defined(RTI_VXWORKS)

#if CPU==PPC32
    UTEST_Stdio_snprintf(sysinfo->machine,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","PPC32");
#elif CPU==PPC64
    UTEST_Stdio_snprintf(sysinfo->machine,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","PPC64");
#elif CPU==PENTIUM
    UTEST_Stdio_snprintf(sysinfo->machine,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","PENTIUM");
#else
    UTEST_Stdio_snprintf(sysinfo->machine,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","unknown");
#endif

#ifndef RTI_CERT
    if (gethostname(sysinfo->nodename,UTEST_SYSTEM_NAME_MAX_LENGTH) != OK)
    {
        return 0;
    }
#else
    sysinfo->nodename[0] = '\0';
#endif

    UTEST_Stdio_printf("hostname is %s\n",sysinfo->nodename);

#if defined(_WRS_VXWORKS_MAJOR)
    UTEST_Stdio_snprintf(sysinfo->release,UTEST_SYSTEM_NAME_MAX_LENGTH,"%d.%d",
                     _WRS_VXWORKS_MAJOR,_WRS_VXWORKS_MINOR);
#elif defined(VXWORKS_MAJOR_VERSION)
    UTEST_Stdio_snprintf(sysinfo->release,UTEST_SYSTEM_NAME_MAX_LENGTH,"%d.%d",
            VXWORKS_MAJOR_VERSION,VXWORKS_MINOR_VERSION);
#else
#error "No macro defining VxWorks version. Please update " __FILE__ "line:" __LINE__
#endif

    /* system, Always this */
    UTEST_Stdio_snprintf(sysinfo->sysname,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","VxWorks");

#ifdef RTI_RTP
    UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","VxWorks RTP");
#else
    UTEST_Stdio_snprintf(sysinfo->version,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","VxWorks");
#endif
#elif defined(RTI_THREADX)
    UTEST_Stdio_snprintf(sysinfo->machine, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "S7G2");
    UTEST_Stdio_snprintf(sysinfo->nodename, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "ThreadX-host");
    UTEST_Stdio_snprintf(sysinfo->release, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "v5.7");
    UTEST_Stdio_snprintf(sysinfo->sysname, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "ThreadX");
    UTEST_Stdio_snprintf(sysinfo->version, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "ThreadX");
#elif defined(RTI_FREERTOS)
    UTEST_Stdio_snprintf(sysinfo->machine, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "STM32F769");
    UTEST_Stdio_snprintf(sysinfo->nodename, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "FreeRTOS-host");
    UTEST_Stdio_snprintf(sysinfo->release, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", tskKERNEL_VERSION_NUMBER);
    UTEST_Stdio_snprintf(sysinfo->sysname, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "FreeRTOS");
    UTEST_Stdio_snprintf(sysinfo->version, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%d.%d.%d", tskKERNEL_VERSION_MAJOR,
                         tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD);
#elif defined(RTI_DEOS)
    UTEST_Stdio_snprintf(sysinfo->machine, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "PPC32");
    UTEST_Stdio_snprintf(sysinfo->nodename, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Deos653-host");
    UTEST_Stdio_snprintf(sysinfo->release, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "10.3.1");
    UTEST_Stdio_snprintf(sysinfo->sysname, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Deos");
    UTEST_Stdio_snprintf(sysinfo->version, UTEST_SYSTEM_NAME_MAX_LENGTH,
                         "%s", "Deos");
#else
#error "Unknown system: Please port UTEST_System_get_info  in " __FILE__
#endif

#ifdef RTIME_TARGET_NAME
    UTEST_Stdio_snprintf(sysinfo->rtiarch,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s",UTEST_STRINGIFY_DEFINE(RTIME_TARGET_NAME));
#else
    UTEST_Stdio_snprintf(sysinfo->rtiarch,UTEST_SYSTEM_NAME_MAX_LENGTH,"%s","unknown");
#endif

    UTEST_Stdio_printf("hostname is %s\n",sysinfo->nodename);

    return 1;
}
