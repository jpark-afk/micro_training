/*
 * FILE: REDAFnmatch.c - fnmatch API implementation
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ci
 * \addtogroup REDAFnmatchClass
 * @{
 */

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_regex_h
#include "reda/reda_regex.h"
#endif

/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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

RTI_PRIVATE RTI_INT16
REDA_String_rangematch(const char *pattern, char test, int flags, char **newp)
{
    int negate, ok;
    char c, c2;
    char path_char = '/';

    /* specify delimination character, but exclusive of leading period */
    if ((flags & REDA_REGEX_FNM_DOTTED_PATH) &&
            (flags & REDA_REGEX_FNM_PATHNAME) &&
            !(flags & REDA_REGEX_FNM_PERIOD))
    {
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
    {
        ++pattern;
    }

    if (flags & REDA_REGEX_FNM_CASEFOLD)
    {
        test = (char) OSAPI_String_char_to_lowercase((unsigned char)test);
    }

    /*
     * A right bracket shall lose its special meaning and represent
     * itself in a bracket expression if it occurs first in the list.
     * -- POSIX.2 2.8.3.2
     */
    ok = 0;
    c = *pattern++;
    do
    {
        if (c == '\\' && !(flags & REDA_REGEX_FNM_NOESCAPE))
        {
            c = *pattern++;
        }
        if (c == REDA_REGEX_EOS)
        {
            return (REDA_REGEX_RANGE_ERROR);
        }
        if (c == path_char && (flags & REDA_REGEX_FNM_PATHNAME))
        {
            return (REDA_REGEX_RANGE_NOMATCH);
        }
        if ((flags & REDA_REGEX_FNM_CASEFOLD))
        {
            c = (char) OSAPI_String_char_to_lowercase((unsigned char)c);
        }
        if (*pattern == '-'
                && (c2 = *(pattern + 1)) != REDA_REGEX_EOS && c2 != ']')
        {
            pattern += 2;
            if (c2 == '\\' && !(flags & REDA_REGEX_FNM_NOESCAPE))
            {
                c2 = *pattern++;
            }
            if (c2 == REDA_REGEX_EOS)
            {
                return (REDA_REGEX_RANGE_ERROR);
            }
            if (flags & REDA_REGEX_FNM_CASEFOLD)
            {
                c2 = (char) OSAPI_String_char_to_lowercase((unsigned char)c2);
            }
            if (c <= test && test <= c2)
            {
                ok = 1;
            }
        }
        else if (c == test)
        {
            ok = 1;
        }
    }
    while ((c = *pattern++) != ']');

    *newp = (char *)pattern;
    return (ok == negate ? REDA_REGEX_RANGE_NOMATCH : REDA_REGEX_RANGE_MATCH);
}

RTI_INT32
REDA_String_fnmatch(const char *pattern, const char *string, int flags)
{
    const char *stringstart;
    char *newp;
    char c, test;
    char path_char = '/';

    /* specify delimination character, but exclusive of leading period */
    if ((flags & REDA_REGEX_FNM_DOTTED_PATH) &&
            (flags & REDA_REGEX_FNM_PATHNAME) &&
            !(flags & REDA_REGEX_FNM_PERIOD))
    {
        path_char = '.';
    }

    for (stringstart = string;;)
        switch (c = *pattern++)
        {
            case REDA_REGEX_EOS:
                if ((flags & REDA_REGEX_FNM_LEADING_DIR) && *string == path_char)
                {
                    return (REDA_REGEX_FNM_MATCH);
                }
                return (*string == REDA_REGEX_EOS ? REDA_REGEX_FNM_MATCH :
                                                    REDA_REGEX_FNM_NOMATCH);
            case '?':
                if (*string == REDA_REGEX_EOS)
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                if (*string == path_char && (flags & REDA_REGEX_FNM_PATHNAME))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                if (*string == '.' && (flags & REDA_REGEX_FNM_PERIOD) &&
                        (string == stringstart ||
                         ((flags & REDA_REGEX_FNM_PATHNAME) && *(string - 1) == '/')))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                ++string;
                break;
            case '*':
                c = *pattern;
                /* Collapse multiple stars. */
                while (c == '*')
                {
                    c = *++pattern;
                }

                if (*string == '.' && (flags & REDA_REGEX_FNM_PERIOD) &&
                        (string == stringstart ||
                         ((flags & REDA_REGEX_FNM_PATHNAME) && *(string - 1) == '/')))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }

                /* Optimize for pattern with * at end or before /. */
                if (c == REDA_REGEX_EOS)
                {
                    if (flags & REDA_REGEX_FNM_PATHNAME)
                    {
                        return ((flags & REDA_REGEX_FNM_LEADING_DIR) ||
                            OSAPI_String_strchr(string, path_char) == NULL ?
                            REDA_REGEX_FNM_MATCH : REDA_REGEX_RANGE_NOMATCH);
                    }
                    else
                    {
                        return (REDA_REGEX_FNM_MATCH);
                    }
                }
                else if (c == path_char && (flags & REDA_REGEX_FNM_PATHNAME))
                {
                    if ((string = OSAPI_String_strchr(string, path_char)) == NULL)
                    {
                        return (REDA_REGEX_FNM_NOMATCH);
                    }
                    break;
                }

                /* General case, use recursion. */
                while ((test = *string) != REDA_REGEX_EOS)
                {
                    if (!REDA_String_fnmatch(pattern, string, flags &
                                                        ~REDA_REGEX_FNM_PERIOD))
                    {
                        return (REDA_REGEX_FNM_MATCH);
                    }
                    if (test == path_char && (flags & REDA_REGEX_FNM_PATHNAME))
                    {
                        break;
                    }
                    ++string;
                }
                return (REDA_REGEX_FNM_NOMATCH);
            case '[':
                if (*string == REDA_REGEX_EOS)
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                if (*string == path_char && (flags & REDA_REGEX_FNM_PATHNAME))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                if (*string == '.' && (flags & REDA_REGEX_FNM_PERIOD) &&
                        (string == stringstart ||
                         ((flags & REDA_REGEX_FNM_PATHNAME) && *(string - 1) == '/')))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }

                switch (REDA_String_rangematch(pattern, *string, flags, &newp))
                {
                    case REDA_REGEX_RANGE_ERROR:
                        /* not a good range, treat as normal text */
                        goto normal;
                    case REDA_REGEX_RANGE_MATCH:
                        pattern = newp;
                        break;
                    case REDA_REGEX_RANGE_NOMATCH:
                        return (REDA_REGEX_FNM_NOMATCH);
                }
                ++string;
                break;
            case '\\':
                if (!(flags & REDA_REGEX_FNM_NOESCAPE))
                {
                    if ((c = *pattern++) == REDA_REGEX_EOS)
                    {
                        c = '\\';
                        --pattern;
                    }
                }
            /* FALLTHROUGH */
            default:
normal:
                if (c != *string && !((flags & REDA_REGEX_FNM_CASEFOLD) &&
                                      (OSAPI_String_char_to_lowercase((unsigned char)c) ==
                                       OSAPI_String_char_to_lowercase((unsigned char)*string))))
                {
                    return (REDA_REGEX_FNM_NOMATCH);
                }
                ++string;
                break;
        }
    /* NOTREACHED */
    return REDA_REGEX_FNM_NOMATCH;
}

/*ci @} */
