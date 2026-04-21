/*
 * FILE: UTEST_Runner.c - Unit-test execution support
 *
 * (c) Copyright, Real-Time Innovations, 2004-2020
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
 * 31dec2013,tk  Refactored from Setting.c
 * 01aug2012,tk  Many enhancements
 * 01dec2004,cc  Created, based on Waveworks tree.
 */
/*ce
 * \file UTEST_Runner.c
 * \brief Unit-test execution support
 */

#include "UTEST_Runner.h"
#include "UTEST_Stdio.h"
#include "UTEST_String.h"
#include "test/test_setting.h"

/*** SOURCE_BEGIN ***/

/* ARINC 653 requires it's own implementation of run_test_entry to support
 * having a test controller and test worker which execute tests differently.
 */
#ifdef RTI_ARINC653
UTEST_DEFINE_ARINC_RUN_TEST_ENTRY
#else
static unsigned char
UTEST_Runner_run_test_entry(struct UTEST_Context *setting,
                            struct UTEST_TestEntry test_entry)
{
    return test_entry.test_func(setting);
}
#endif

unsigned char
UTEST_Runner_run_tests(struct UTEST_Context *setting,
                       const char *test_name,
                       struct UTEST_TestEntry *test_vector,
                       int count)
{
    int i;
    unsigned char ok = 1;
    const char *test_title;
    int length;
    size_t end_of_path;
    size_t end_of_id_path;
    char text_buf[1000];
    size_t patt_length;

    ++setting->test_level;
    setting->test_counter[setting->test_level]=0;
    end_of_path = strlen(setting->test_path);
    end_of_id_path = strlen(setting->test_id_path);

    if (setting->test_level > 0)
    {
        if (setting->test_path[0] == 0)
        {
            UTEST_Stdio_snprintf(setting->test_path,1000,"%s",test_name);
        }
    }

    for (i = 0; (i < count); ++i)
    {
        unsigned char testOk;

        if (test_vector[i].test_func == NULL)
        {
            continue;
        }

#if !DEVTREE_BUILD
        /* For a devtree build we still want to see what tests are disabled */
        if (!CHECK_TEST_ENABLED(&test_vector[i]))
        {
            continue;
        }
#endif

        if (test_vector[i].title)
        {
            test_title = test_vector[i].title;
        }
        else
        {
            test_title = "unknown";
        }

        setting->c_ptr = setting->test_description;
        if (setting->test_level > 0)
        {
            if (setting->test_path[0] == 0)
            {
                length = UTEST_Stdio_snprintf(setting->test_description,
                                              UTEST_MAX_COLUMN_WIDTH-1,
                                              "%s ",
                                              test_title);
            }
            else
            {
                length = UTEST_Stdio_snprintf(setting->test_description,
                                              UTEST_MAX_COLUMN_WIDTH-1,
                                              "%s/%s ",
                                              setting->test_path,
                                              test_title);
            }

            setting->c_ptr += length;

            if (setting->c_ptr < (setting->test_description + (UTEST_MAX_COLUMN_WIDTH-1)))
            {
                memset(setting->c_ptr,'.',(size_t)(UTEST_MAX_COLUMN_WIDTH - length - 1));
            }

            setting->c_ptr[UTEST_MAX_COLUMN_WIDTH-length-1]=0;
        }

        setting->test_counter[setting->test_level+1]=-1;
        patt_length = strlen(setting->test_path);

        if (setting->test_level > 0)
        {
            UTEST_Stdio_snprintf(text_buf,1000,"/%s",test_title);
            UTEST_String_strncat(setting->test_path,text_buf,1000);
        }

        setting->test_last_error[0] = 0;

        setting->current_entry = &test_vector[i];

        testOk = UTEST_Runner_run_test_entry(setting,test_vector[i]);

        if (testOk == 0)
        {
            setting->error_count++;
        }
        ok = (unsigned char)(ok && testOk);

        setting->test_path[patt_length] = 0;

        /* Only print result for leaf-nodes */
        if ((setting->test_level > 0) &&
             setting->test_counter[setting->test_level+1] == -1)
        {
            if (testOk == 0)
            {
                UTEST_Stdio_printf(": Failed (%s)\n",setting->test_last_error);
            }
            else if (testOk == 1)
            {
                UTEST_Stdio_printf(": Passed\n");
            }
            else if (testOk == 2)
            {
                if (setting->print_all)
                {
                    UTEST_Stdio_printf(": Excluded\n");
                }
            }
            else if (testOk == 3)
            {
                UTEST_Stdio_printf("%s: Disabled [%s]\n",setting->test_description,
                                   setting->current_entry->tags != NULL ?
                                           setting->current_entry->tags : "no info");
            }
            else
            {
                setting->error_count++;
                UTEST_Stdio_printf(": Unknown return-code (%s)\n",setting->test_last_error);
                if (!setting->ignoreFailure)
                {
                    break;
                }
            }
        }

        if (!setting->ignoreFailure && (!testOk))
        {
            break;
        }

        setting->test_counter[setting->test_level]++;
    }

    setting->test_path[end_of_path]=0;
    setting->test_level--;
    setting->test_id_path[end_of_id_path]=0;


    if (setting->test_level == -1)
    {
#ifndef RTI_CERT
        UTEST_Property_delete_property(setting);
#endif
        UTEST_Stdio_printf("%s:TESTS COMPLETED\n", test_name);
    }

    return ok;
}
