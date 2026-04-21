/*
 * FILE: reda_regex.h - fnmatch API
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
 * \file
 *
 * \brief The REDA regex API provides commonly used for regular expression
 * pattern matching
 *
 * \ingroup REDAModule
 *
 * \details
 *
 */


#ifndef reda_fnmatch_h
#define reda_fnmatch_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#include "reda/reda_sequence.h"


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
#define REDA_REGEX_FNM_NOMATCH     1
#define REDA_REGEX_FNM_MATCH       0
#define REDA_REGEX_FNM_DOTTED_PATH 0x20
#define REDA_REGEX_FNM_PATHNAME    0x02    /* Slash must be matched by slash. */
#define REDA_REGEX_FNM_PERIOD      0x04    /* Period must be matched by period. */
#define REDA_REGEX_FNM_LEADING_DIR 0x08    /* Ignore /<tail> after Imatch. */
#define REDA_REGEX_FNM_NOESCAPE    0x01    /* Disable backslash escaping. */
#define REDA_REGEX_FNM_CASEFOLD    0x10    /* Case insensitive search. */
#define REDA_REGEX_EOS             '\0'
#define REDA_REGEX_RANGE_MATCH     1
#define REDA_REGEX_RANGE_NOMATCH   0
#define REDA_REGEX_RANGE_ERROR     (-1)
#define REDA_REGEX_SUPPORTED_REGEX_CHARACTERS "[]?*!^"

/*ci
 * \brief The fnmatch() function checks whether the string argument matches
 * the pattern argument
 *
 * \return Returns RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_String_fnmatch(const char *pattern, const char *string, int flags);


#endif /* reda_string_h */

/*ci @} */

