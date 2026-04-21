/*
 * FILE: UTEST_String.c - Unit-test string support
 *
 * (c) Copyright, Real-Time Innovations, 2012-2020.
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
 * 01aug2012,tk  Written
 */
/*ce
 * \file UTEST_Sring.h
 * \brief Unit-test string support
 */
#include "test/test_setting.h"
#include "UTEST_String.h"

/* VxWorks 653 certified partitions must undefine the standard tolower macro
 * located in ctypes.h which replaces tolower with __cytpe_tolower. Otherwise,
 * the linker will fail to find __ctype_tolower.
 */
#if defined(RTI_VX653) && defined(RTI_CERT)
#undef tolower
#endif

/*** SOURCE_BEGIN ***/

char*
UTEST_String_strncat(char *s1, const char *s2, size_t n)
{
#if defined(_MSC_VER) || defined(WIN32)
    return (!strcat_s(s1,n,s2) ? s1 : NULL);
#else
    size_t len = strlen(s1);

    if (n < 1)
    {
        return NULL;
    }
    if (len >= n)
    {
        return NULL;
    }
    return strncat(s1,s2,n-len-1);
#endif
}

char*
UTEST_String_strncpy(char *s1, const char *s2, size_t n)
{
    if (n == 0)
    {
        return NULL;
    }

#if defined(_MSC_VER) || defined(WIN32)
    return (!strncpy_s(s1,n,s2,_TRUNCATE) ? s1 : NULL);
#else
    return strncpy(s1,s2,n);
#endif
}

char*
UTEST_String_strdup(const char *string)
{
    char *clone = NULL;
    size_t len;

    len = strlen(string);

    clone = malloc((len + 1) * sizeof(char));
    if (clone == NULL)
    {
        return NULL;
    }

    UTEST_String_strncpy(clone, string, len + 1);
    clone[len] = '\0';
    
    return clone;
}

int
UTEST_String_argv_from_string(const char *appname,
                              const char *arg_string,
                              int *argc,
                              char ***argv)
{
    const char *ptr;
    int found_quote = 0;
    char **targv;
    const char *begin;

#define add_arg()\
    targv[*argc] = malloc((size_t)(ptr - begin + 1));\
    memcpy(targv[*argc],begin,(size_t)(ptr - begin + 1));\
    targv[*argc][ptr - begin] = 0;\
    begin = NULL;\
    found_quote = 0;\
    (*argc)++

    *argv = (char **)malloc(1024*sizeof(char*));
    *argc = 0;
    targv = *argv;

    targv[*argc] = malloc(strlen(appname)+1);
    memcpy(targv[*argc],appname,strlen(appname)+1);
    (*argc)++;

    ptr = arg_string;
    begin = NULL;

    while (*ptr)
    {
        if ((*ptr >= 0x21) && (*ptr <= 0x7e) && (*ptr != '"'))
        {
            if (begin == NULL)
            {
                begin = ptr;
            }
        }
        else if (*ptr == '"')
        {
            if (!found_quote)
            {
                ptr++;
                begin = ptr;
                found_quote = 1;
            }
            else
            {
                /* found argv */
                add_arg();
            }
        }
        else if (!found_quote && (*ptr == ' ') && (begin != NULL))
        {
            add_arg();
        }
        ptr++;
    }

    if (begin != NULL)
    {
        add_arg();
    }

    return 1;

#undef add_arg
}

/*
 * Copyright 2013-2015 Real-Time Innovations, Inc.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 31dec2013,tk Refactored from UT_Property.c
 * 08nov2013,tk Included
 */
/*ce
 * \file UTEST_Regex.c Unit-test regex support
 */

/*
 * Copyright (c) 1989, 1993, 1994-2015
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define EOS     '\0'

#define RANGE_MATCH     1
#define RANGE_NOMATCH   0
#define RANGE_ERROR     (-1)

/* have to use this form to conform to strcmp equality equals zero */
#define UTEST_STRING_FNM_NOMATCH     1       /* Match failed. */
#define UTEST_STRING_FNM_MATCH       0

static int UTEST_String_rangematch(const char *, char, int, char **);

/*
  Returns zero if string matches pattern, UTEST_STRING_FNM_NOMATCH if there is
  no match or another non-zero value if there is an error.
 */
int UTEST_String_fnmatch(const char *pattern, const char *string, int flags)
{
    const char *stringstart;
    char *newp;
    char c, test;
    char path_char = '/';

    /* specify delimination character, but exclusive of leading period */
    if ((flags & UTEST_STRING_FNM_DOTTED_PATH) &&
        (flags & UTEST_STRING_FNM_PATHNAME) &&
       !(flags & UTEST_STRING_FNM_PERIOD))
    {
        path_char = '.';
    }

    for (stringstart = string;;)
        switch (c = *pattern++) {
          case EOS:
            if ((flags & UTEST_STRING_FNM_LEADING_DIR) && *string == path_char)
                return (0);
            return (*string == EOS ? 0 : UTEST_STRING_FNM_NOMATCH);
          case '?':
            if (*string == EOS)
                return (UTEST_STRING_FNM_NOMATCH);
            if (*string == path_char && (flags & UTEST_STRING_FNM_PATHNAME))
                return (UTEST_STRING_FNM_NOMATCH);
            if (*string == '.' && (flags & UTEST_STRING_FNM_PERIOD) &&
                (string == stringstart ||
                 ((flags & UTEST_STRING_FNM_PATHNAME) && *(string - 1) == '/')))
                return (UTEST_STRING_FNM_NOMATCH);
            ++string;
            break;
          case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            while (c == '*')
                c = *++pattern;

            if (*string == '.' && (flags & UTEST_STRING_FNM_PERIOD) &&
                (string == stringstart ||
                 ((flags & UTEST_STRING_FNM_PATHNAME) && *(string - 1) == '/')))
                return (UTEST_STRING_FNM_NOMATCH);

            /* Optimize for pattern with * at end or before /. */
            if (c == EOS) {
                if (flags & UTEST_STRING_FNM_PATHNAME)
                    return ((flags & UTEST_STRING_FNM_LEADING_DIR) ||
                            strrchr(string, path_char) == NULL ?
                            0 : UTEST_STRING_FNM_NOMATCH);
                else
                    return (0);
            } else if (c == path_char && (flags & UTEST_STRING_FNM_PATHNAME)) {
                if ((string = strrchr(string, path_char)) == NULL)
                    return (UTEST_STRING_FNM_NOMATCH);
                break;
            }

            /* General case, use recursion. */
            while ((test = *string) != EOS) {
                if (!UTEST_String_fnmatch(pattern, string, flags & ~UTEST_STRING_FNM_PERIOD))
                    return (0);
                if (test == path_char && (flags & UTEST_STRING_FNM_PATHNAME))
                    break;
                ++string;
            }
            return (UTEST_STRING_FNM_NOMATCH);
          case '[':
            if (*string == EOS)
                return (UTEST_STRING_FNM_NOMATCH);
            if (*string == path_char && (flags & UTEST_STRING_FNM_PATHNAME))
                return (UTEST_STRING_FNM_NOMATCH);
            if (*string == '.' && (flags & UTEST_STRING_FNM_PERIOD) &&
                (string == stringstart ||
                 ((flags & UTEST_STRING_FNM_PATHNAME) && *(string - 1) == '/')))
                return (UTEST_STRING_FNM_NOMATCH);

            switch (UTEST_String_rangematch(pattern, *string, flags, &newp)) {
              case RANGE_ERROR:
                                /* not a good range, treat as normal text */
                goto normal;
              case RANGE_MATCH:
                pattern = newp;
                break;
              case RANGE_NOMATCH:
                return (UTEST_STRING_FNM_NOMATCH);
              default:
                  break;
            }
            ++string;
            break;
          /* case '\\' handled in the if test below */
          default:
              if (c == '\\')
              {
                  if (!(flags & UTEST_STRING_FNM_NOESCAPE)) {
                      if ((c = *pattern++) == EOS) {
                          c = '\\';
                          --pattern;
                      }
                  }
              }

normal:       if (c != *string && !((flags & UTEST_STRING_FNM_CASEFOLD) &&
                                  (tolower((unsigned char)c) ==
                                   tolower((unsigned char)*string))))
              {
                return (UTEST_STRING_FNM_NOMATCH);
              }
            ++string;
            break;
        }
    /* NOTREACHED */
    return UTEST_STRING_FNM_NOMATCH;
}

static int
UTEST_String_rangematch(const char *pattern, char test, int flags, char **newp)
{
    int negate, ok;
    char c, c2;
    char path_char = '/';

    /* specify delimination character, but exclusive of leading period */
    if ((flags & UTEST_STRING_FNM_DOTTED_PATH) &&
        (flags & UTEST_STRING_FNM_PATHNAME) &&
       !(flags & UTEST_STRING_FNM_PERIOD)) {
        path_char = '.';
    }

    /*
     * A bracket expression starting with an unquoted circumflex
     * character produces unspecified results (IEEE 1003.2-1992,
     * 3.13.2).  This implementation treats it like '!', for
     * consistency with the regular expression syntax.
     * J.T. Conklin (conklin@ngai.kaleida.com)
     */
    if ((negate = (*pattern == '!' || *pattern == '^')))
        ++pattern;

    if (flags & UTEST_STRING_FNM_CASEFOLD)
        test = (char)tolower((unsigned char)test);

    /*
     * A right bracket shall lose its special meaning and represent
     * itself in a bracket expression if it occurs first in the list.
     * -- POSIX.2 2.8.3.2
     */
    ok = 0;
    c = *pattern++;
    do {
        if (c == '\\' && !(flags & UTEST_STRING_FNM_NOESCAPE))
            c = *pattern++;
        if (c == EOS)
            return (RANGE_ERROR);
        if (c == path_char && (flags & UTEST_STRING_FNM_PATHNAME))
            return (RANGE_NOMATCH);
        if ((flags & UTEST_STRING_FNM_CASEFOLD))
            c = (char)tolower((unsigned char)c);
        if (*pattern == '-'
            && (c2 = *(pattern+1)) != EOS && c2 != ']') {
            pattern += 2;
            if (c2 == '\\' && !(flags & UTEST_STRING_FNM_NOESCAPE))
                c2 = *pattern++;
            if (c2 == EOS)
                return (RANGE_ERROR);
            if (flags & UTEST_STRING_FNM_CASEFOLD)
                c2 = (char)tolower((unsigned char)c2);
            if (c <= test && test <= c2)
                ok = 1;
        } else if (c == test)
            ok = 1;
    } while ((c = *pattern++) != ']');

    *newp = (char *)pattern;
    return (ok == negate ? RANGE_NOMATCH : RANGE_MATCH);
}

