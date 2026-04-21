/*
 * FILE: UTEST_Property.c - Unit-test support for properties
 *
 * (c) Copyright 2013-2020 Real-Time Innovations, Inc.
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
 * 31dec2013,tk Refactored from UTEST_Property.c
 * 08nov2013,tk Written
 */
/*ce
 * \file UTEST_Property.c
 * \brief Unit-test support for properties
 */

#include "test/test_setting.h"
#include "UTEST_Property.h"
#include "UTEST_File.h"
#include "UTEST_Stdio.h"
#include "UTEST_String.h"

typedef enum {
    UTEST_TOKEN_STRING,
    UTEST_TOKEN_DIGIT,
    UTEST_TOKEN_NULL,
    UTEST_TOKEN_UNKNOWN,
    UTEST_TOKEN_EOF,
    UTEST_TOKEN_EQUAL,
    UTEST_TOKEN_AT,
    UTEST_TOKEN_DURATION,
    UTEST_TOKEN_ADDRESS,
    UTEST_TOKEN_HASH,
    UTEST_TOKEN_COLON,
    UTEST_TOKEN_DOT,
    UTEST_TOKEN_SLASH,
    UTEST_TOKEN_DOLLAR,
    UTEST_TOKEN_DELIMITER,
    UTEST_TOKEN_SLASHSLASH,
    UTEST_TOKEN_RIGHT_BRACE,
    UTEST_TOKEN_LEFT_BRACE,
    UTEST_TOKEN_RIGHT_P,
    UTEST_TOKEN_LEFT_P,
    UTEST_TOKEN_SEMICOLON,
    UTEST_TOKEN_COMMA,
    UTEST_TOKEN_RIGHT_BRACKET,
    UTEST_TOKEN_LEFT_BRACKET,
    UTEST_TOKEN_KEYWORD,
    UTEST_TOKEN_LT,
    UTEST_TOKEN_LE,
    UTEST_TOKEN_GT,
    UTEST_TOKEN_GE,
    UTEST_TOKEN_NOT_EQUAL,
    UTEST_TOKEN_NOT,
    UTEST_TOKEN_TRUE,
    UTEST_TOKEN_FALSE,
    UTEST_TOKEN_AND,
    UTEST_TOKEN_OR,
    UTEST_TOKEN_TIMEOUTEST_AND,
    UTEST_TOKEN_TIMEOUTEST_OR,
    UTEST_TOKEN_TEST,
    UTEST_TOKEN_INCLUDE,
    UTEST_TOKEN_COMMENT,
    UTEST_TOKEN_C_COMMENT,
    UTEST_TOKEN_C_COMMENT_END,
    UTEST_TOKEN_PROPERTY,
    UTEST_TOKEN_HOST
} UTEST_PropertyTokenKind_T;

#define UTEST_PROPERTYPARSER_MAX_TOKEN_LENGTH 255

union UTEST_PropertyValue
{
    char sz_value[UTEST_PROPERTYPARSER_MAX_TOKEN_LENGTH];
    int int_value;
};

struct UTEST_PropertyToken
{
    UTEST_PropertyTokenKind_T kind;
    union UTEST_PropertyValue value;
};

#define UTEST_FD_BUF_MAX       1024
#define UTEST_MAX_NESTED_FILE    32

struct UTEST_PropertyFile
{
    char *cur_ptr;
    int column;
    int linenum;
    int fd_input;
    char fd_buf[UTEST_FD_BUF_MAX];
#if !defined(RTI_WIN32) && !defined(RTI_AUTOSAR)
    ssize_t nbytes;
#else
    size_t nbytes;
#endif
};

struct UTEST_PropertyParser
{
    struct UTEST_PropertyFile files[UTEST_MAX_NESTED_FILE];
    int file_index;
    char *cur_ptr;
    int column;
    int linenum;
    int fd_input;
    char *fd_buf;
    char is_file;

#if !defined(RTI_WIN32) && !defined(RTI_AUTOSAR)
    ssize_t nbytes;
#else
    size_t nbytes;
#endif

    struct UTEST_SystemInfo sysinfo;
};

#if HAVE_CONFIG_FILE
static int
UTEST_Property_get_next_token(struct UTEST_PropertyParser *parser,
                              struct UTEST_PropertyToken *token)
{
    int string_index=0;
    char done = 0;
    char rval = 0;
    char is_property = 0;
    /* int base = 10; */

    token->kind = UTEST_TOKEN_NULL;

    while (!done)
    {
        if (parser->is_file && ((parser->cur_ptr == NULL) ||
             (parser->cur_ptr == (parser->fd_buf + parser->nbytes))))
        {
            parser->nbytes = read(parser->fd_input,parser->fd_buf,UTEST_FD_BUF_MAX);
            if (parser->nbytes == 0)
            {
                token->kind = UTEST_TOKEN_EOF;
                done = 1;
                rval = 1;
                continue;
            }
            if (parser->nbytes == -1)
            {
                done = 1;
                rval = 0;
                continue;
            }
            parser->cur_ptr = parser->fd_buf;
        }
        else if (!parser->is_file &&
                 (parser->cur_ptr == (parser->fd_buf + parser->nbytes)))
        {
            token->kind = UTEST_TOKEN_EOF;
            done = 1;
            rval = 1;
            continue;
        }

        if (*parser->cur_ptr == '\n')
        {
            if (token->kind == UTEST_TOKEN_COMMENT)
            {
                token->kind = UTEST_TOKEN_NULL;
            }
            parser->column = 0;
            ++parser->linenum;
            ++parser->cur_ptr;
            continue;
        }
        switch(token->kind)
        {
        case UTEST_TOKEN_NULL:
            if (((*parser->cur_ptr >= 'a') && (*parser->cur_ptr <= 'z')) ||
                    ((*parser->cur_ptr >= 'A') && (*parser->cur_ptr <= 'Z')) ||
                    (*parser->cur_ptr == '_') || (*parser->cur_ptr == '.') ||
                    ((*parser->cur_ptr >= '0') && (*parser->cur_ptr <= '9')))
            {
                token->kind = UTEST_TOKEN_KEYWORD;
                string_index = 0;
                token->value.sz_value[string_index] = *parser->cur_ptr;
                ++string_index;
                token->value.sz_value[string_index] = 0;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (*parser->cur_ptr == '"')
            {
                token->kind = UTEST_TOKEN_STRING;
                (parser->cur_ptr)++;
                ++parser->column;
                string_index = 0;
                token->value.sz_value[string_index] = 0;
            }
            else if (*parser->cur_ptr == ':')
            {
                token->kind = UTEST_TOKEN_COLON;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (*parser->cur_ptr == ';')
            {
                token->kind = UTEST_TOKEN_SEMICOLON;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '=')
            {
                token->kind = UTEST_TOKEN_EQUAL;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '<')
            {
                parser->cur_ptr++;
                ++parser->column;
                if (*parser->cur_ptr == '>')
                {
                    token->kind = UTEST_TOKEN_NOT_EQUAL;
                    parser->cur_ptr++;
                    ++parser->column;
                }
                else if (*parser->cur_ptr == '=')
                {
                    token->kind = UTEST_TOKEN_LE;
                    parser->cur_ptr++;
                    ++parser->column;
                }
                else
                {
                    token->kind = UTEST_TOKEN_LT;
                }
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '>')
            {
                parser->cur_ptr++;
                ++parser->column;
                if (*parser->cur_ptr == '=')
                {
                    token->kind = UTEST_TOKEN_GE;
                    parser->cur_ptr++;
                    ++parser->column;
                }
                else
                {
                    token->kind = UTEST_TOKEN_GT;
                }
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '{')
            {
                token->kind = UTEST_TOKEN_LEFT_BRACE;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '(')
            {
                token->kind = UTEST_TOKEN_LEFT_P;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '$')
            {
                token->kind = UTEST_TOKEN_DOLLAR;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (*parser->cur_ptr == ')')
            {
                token->kind = UTEST_TOKEN_RIGHT_P;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == ']')
            {
                token->kind = UTEST_TOKEN_RIGHT_BRACKET;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '[')
            {
                token->kind = UTEST_TOKEN_LEFT_BRACKET;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == ',')
            {
                token->kind = UTEST_TOKEN_COMMA;
                (parser->cur_ptr)++;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '}')
            {
                token->kind = UTEST_TOKEN_RIGHT_BRACE;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
                rval = 1;
            }
            else if (*parser->cur_ptr == '#')
            {
                token->kind = UTEST_TOKEN_COMMENT;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (*parser->cur_ptr == '/')
            {
                token->kind = UTEST_TOKEN_SLASH;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (*parser->cur_ptr <= ' ')
            {
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else
            {
                token->kind = UTEST_TOKEN_UNKNOWN;
                (parser->cur_ptr)++;
                ++parser->column;
                done = 1;
            }
            break;
        case UTEST_TOKEN_SLASH:
            if (*parser->cur_ptr == '*')
            {
                token->kind = UTEST_TOKEN_C_COMMENT;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else
            {
                rval = 0;
                done = 1;
            }
            break;
        case UTEST_TOKEN_C_COMMENT:
            if (*parser->cur_ptr == '*')
            {
                token->kind = UTEST_TOKEN_C_COMMENT_END;
            }
            (parser->cur_ptr)++;
            ++parser->column;
            break;
        case UTEST_TOKEN_C_COMMENT_END:
            if (*parser->cur_ptr == '/')
            {
                token->kind = UTEST_TOKEN_NULL;
            }
            else
            {
                token->kind = UTEST_TOKEN_C_COMMENT;
            }
            (parser->cur_ptr)++;
            ++parser->column;
            break;
        case UTEST_TOKEN_COMMENT:
            if (*parser->cur_ptr == '\n')
            {
                token->kind = UTEST_TOKEN_NULL;
            }
            (parser->cur_ptr)++;
            ++parser->column;
            break;
        case UTEST_TOKEN_DIGIT:
            if ((*parser->cur_ptr >= '0') && (*parser->cur_ptr <= '9'))
            {
                ++string_index;
                token->value.int_value *= 10;
                token->value.int_value += *parser->cur_ptr - '0';
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else if (((*parser->cur_ptr == 'x') ||
                     (*parser->cur_ptr == 'X')))
            {
                /* base = 16; */
            }
            else
            {
                done = 1;
                rval = 1;
            }
            break;

        case UTEST_TOKEN_COLON:
            if (*parser->cur_ptr == ':')
            {
                token->kind = UTEST_TOKEN_DELIMITER;
                done = 1;
                rval = 1;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            break;

        case UTEST_TOKEN_KEYWORD:
            if (((*parser->cur_ptr >= 'a') && (*parser->cur_ptr <= 'z')) ||
                    ((*parser->cur_ptr >= 'A') && (*parser->cur_ptr <= 'Z')) ||
                    ((*parser->cur_ptr >= '0') && (*parser->cur_ptr <= '9')) ||
                    (*parser->cur_ptr == '_') || (*parser->cur_ptr == '.') ||
                    (*parser->cur_ptr == ':'))
            {
                token->value.sz_value[string_index] = *parser->cur_ptr;
                ++string_index;
                token->value.sz_value[string_index] = 0;
                (parser->cur_ptr)++;
                ++parser->column;
            }
            else
            {
                if (is_property)
                {
                    token->kind = UTEST_TOKEN_PROPERTY;
                }
                else
                {
                    if (!strcmp("true",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_TRUE;
                    }
                    else if (!strcmp("false",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_FALSE;
                    }
                    else if (!strcmp("not",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_NOT;
                    }
                    else if (!strcmp("and",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_AND;
                    }
                    else if (!strcmp("or",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_OR;
                    }
                    else if (!strcmp("timeout_or",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_TIMEOUTEST_OR;
                    }
                    else if (!strcmp("timeout_and",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_TIMEOUTEST_AND;
                    }
                    else if (!strcmp("test",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_TEST;
                    }
                    else if (!strcmp("include",token->value.sz_value))
                    {
                        token->kind = UTEST_TOKEN_INCLUDE;
                    }
                    done = 1;
                    rval = 1;
                }
            }
            break;
        case UTEST_TOKEN_STRING:
            if (*parser->cur_ptr != '"')
            {
                token->value.sz_value[string_index] = *parser->cur_ptr;
                ++string_index;
            }
            else
            {
                token->value.sz_value[string_index] = 0;
                done = 1;
                rval = 1;
            }
            (parser->cur_ptr)++;
            ++parser->column;
            break;
        default:
            token->kind = UTEST_TOKEN_UNKNOWN;
            done = 1;
            rval = 0;
            break;
        }
    }

    if (done)
    {
        rval = 1;
    }

    return rval;
}
#endif /* #if HAVE_CONFIG_FILE */

#if HAVE_CONFIG_FILE
static void
UTEST_Property_error(struct UTEST_PropertyParser *parser, char *format,...)
{
#define UTEST_MAXLOG_MSG     2048
#define UTEST_MAXLOG_BUFFER  (UTEST_MAXLOG_MSG - 500)
    char error_msg[UTEST_MAXLOG_BUFFER];
    va_list ap;
    char *c_ptr;

    va_start(ap,format);

    c_ptr = error_msg;
    c_ptr += UTEST_Stdio_snprintf(error_msg,UTEST_MAXLOG_BUFFER,
             "Error on line %d, column %d: ",parser->linenum,parser->column);

    UTEST_Stdio_snprintf(c_ptr,UTEST_MAXLOG_BUFFER,format,ap);

    va_end(ap);

    UTEST_Stdio_printf(error_msg);
}
#endif /* #if HAVE_CONFIG_FILE */

#if HAVE_CONFIG_FILE
static char
UTEST_Property_get_assignment(struct UTEST_PropertyParser *parser,
                              struct UTEST_PropertyToken *lvalue,
                              struct UTEST_PropertyToken *rvalue)

{
    struct UTEST_PropertyToken token;

    if (!UTEST_Property_get_next_token(parser,lvalue))
    {
        UTEST_Property_error(parser,"failed to read token");
        return 0;
    }

    if (lvalue->kind != UTEST_TOKEN_KEYWORD)
    {
        return 0;
    }

    if (!UTEST_Property_get_next_token(parser,&token))
    {
        UTEST_Property_error(parser,"failed to read token");
        return 0;
    }

    if (token.kind != UTEST_TOKEN_EQUAL)
    {
        UTEST_Property_error(parser,"unexpected token: %s, expected =",
                         token.value.sz_value);
        return 0;
    }

    if (!UTEST_Property_get_next_token(parser,rvalue))
    {
        UTEST_Property_error(parser,"failed to read token");
        return 0;
    }

    if (!UTEST_Property_get_next_token(parser,&token))
    {
        UTEST_Property_error(parser,"failed to read token");
        return 0;
    }

    if (token.kind != UTEST_TOKEN_SEMICOLON)
    {
        UTEST_Property_error(parser,"expected ;");
        return 0;
    }

    return 1;
}
#endif /* #if HAVE_CONFIG_FILE */

void
UTEST_Property_add_property_nv(struct UTEST_Context *setting,
                               char *name,
                               char *value)
{
    setting->property[setting->property_length].name = name;
    setting->property[setting->property_length].value = value;

    UTEST_Log_debug("Adding property [%s] = [%s]\n",
            setting->property[setting->property_length].name,
            setting->property[setting->property_length].value);

    ++setting->property_length;
}

#if HAVE_CONFIG_FILE
static char
UTEST_Property_parse_property(struct UTEST_PropertyParser *parser,
                              struct UTEST_SystemInfo *sysinfo,
                              struct UTEST_Context *setting)
{
    struct UTEST_PropertyToken token;
    struct UTEST_PropertyToken qos_name;
    struct UTEST_PropertyToken qos_value;
    char skip_property = 0;
    const char *property;

    if (sysinfo == NULL)
    {
        UTEST_Property_error(parser,"no sysinfo");
        return 0;
    }

    if (!UTEST_Property_get_next_token(parser,&token))
    {
        UTEST_Property_error(parser,"failed to read token");
        return 0;
    }

    if ((token.kind == UTEST_TOKEN_KEYWORD) &&
         !strcmp(token.value.sz_value,"host"))
    {
        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_EQUAL)
        {
            UTEST_Property_error(parser,"expected = after host");
            return 0;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_STRING)
        {
            UTEST_Property_error(parser,"expected hostname after =");
            return 0;
        }

        if ((setting->nodename == NULL) ||
            strcmp(setting->nodename,token.value.sz_value))
        {
            skip_property = 1;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }
    }
    else if ((token.kind == UTEST_TOKEN_KEYWORD) &&
                !strcmp(token.value.sz_value,"os"))
    {
        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_EQUAL)
        {
            UTEST_Property_error(parser,"expected = after host");
            return 0;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_STRING)
        {
            UTEST_Property_error(parser,"expected hostname after =");
            return 0;
        }

        if ((setting->sysname == NULL) ||
             strcmp(setting->sysname,token.value.sz_value))
        {
            skip_property = 1;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }
    }
    else if ((token.kind == UTEST_TOKEN_KEYWORD) &&
                !strcmp(token.value.sz_value,"target"))
    {
        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_EQUAL)
        {
            UTEST_Property_error(parser,"expected = after host");
            return 0;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_STRING)
        {
            UTEST_Property_error(parser,"expected <target> after =");
            return 0;
        }

        if ((setting->rtiarch == NULL) ||
             strcmp(setting->rtiarch,token.value.sz_value))
        {
            skip_property = 1;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }
    }
    else if ((token.kind == UTEST_TOKEN_KEYWORD) &&
                !strcmp(token.value.sz_value,"user"))
    {
        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_EQUAL)
        {
            UTEST_Property_error(parser,"expected = after user");
            return 0;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }

        if (token.kind != UTEST_TOKEN_STRING)
        {
            UTEST_Property_error(parser,"expected username after =");
            return 0;
        }

        if ((setting->user == NULL) ||
            strcmp(setting->user,token.value.sz_value))
        {
            skip_property = 1;
        }

        if (!UTEST_Property_get_next_token(parser,&token))
        {
            UTEST_Property_error(parser,"failed to read token");
            return 0;
        }
    }

    if (token.kind != UTEST_TOKEN_LEFT_BRACE)
    {
        UTEST_Property_error(parser,"expected {, got %s",token.value.sz_value);
        return 0;
    }

    /* From here we expect qos = value  until } */
    while (UTEST_Property_get_assignment(parser,&qos_name,&qos_value))
    {
        if (skip_property)
        {
            UTEST_Log_debug("skip property %s\n",qos_name.value.sz_value);
            continue;
        }

        property = UTEST_Property_lookup_property(setting,qos_name.value.sz_value);

        if (property != NULL)
        {
            UTEST_Stdio_printf("property %s already exists\n",
                           qos_name.value.sz_value);
            continue;
        }

        UTEST_Property_add_property_nv(setting,
                UTEST_String_strdup(qos_name.value.sz_value),
                UTEST_String_strdup(qos_value.value.sz_value));

        UTEST_Log_debug("added property %s = %s\n",
                       qos_name.value.sz_value,qos_value.value.sz_value);
    }

    if (qos_name.kind != UTEST_TOKEN_RIGHT_BRACE)
    {
        UTEST_Property_error(parser,"expected }");
        return 0;
    }

    return 1;
}
#endif /* #if HAVE_CONFIG_FILE */

#if HAVE_CONFIG_FILE
int
UTEST_Property_read_property_file(const char *file,
                                  struct UTEST_SystemInfo *sysinfo,
                                  struct UTEST_Context *setting)
{
    struct UTEST_PropertyParser parser;
    struct UTEST_PropertyToken ftoken;
    struct UTEST_PropertyToken token;

    parser.file_index = 0;
    parser.is_file = 1;

    parser.files[parser.file_index].fd_input = UTEST_File_open_file(file,O_RDONLY,0);
    if (parser.files[parser.file_index].fd_input < 0)
    {
#ifndef RTI_CERT
        perror("open file failed");
#endif
        return -1;
    }

    parser.files[parser.file_index].fd_buf[0] = 0;
    parser.files[parser.file_index].linenum = 1;
    parser.files[parser.file_index].column = 1;
    parser.files[parser.file_index].cur_ptr = NULL;

    parser.linenum = parser.files[parser.file_index].linenum;
    parser.column = parser.files[parser.file_index].column;
    parser.cur_ptr = parser.files[parser.file_index].cur_ptr;
    parser.fd_input = parser.files[parser.file_index].fd_input;
    parser.fd_buf = parser.files[parser.file_index].fd_buf;

    while (UTEST_Property_get_next_token(&parser,&token))
    {
        if (token.kind == UTEST_TOKEN_EOF)
        {
            /* close current file */
            close(parser.fd_input);
            --parser.file_index;
            if (parser.file_index >= 0)
            {
                parser.linenum = parser.files[parser.file_index].linenum;
                parser.column = parser.files[parser.file_index].column;
                parser.cur_ptr = parser.files[parser.file_index].cur_ptr;
                parser.fd_input = parser.files[parser.file_index].fd_input;
                parser.fd_buf = parser.files[parser.file_index].fd_buf;
                parser.nbytes = parser.files[parser.file_index].nbytes;
            }
            else
            {
                break;
            }
        }
        else if (token.kind == UTEST_TOKEN_INCLUDE)
        {
            if (parser.file_index == (UTEST_MAX_NESTED_FILE - 1))
            {
                UTEST_Property_error(&parser,"too many open files");
                goto done;
            }
            if (!UTEST_Property_get_next_token(&parser,&ftoken))
            {
                UTEST_Property_error(&parser,"expected file-name");
                goto done;
            }
            if (ftoken.kind != UTEST_TOKEN_STRING)
            {
                UTEST_Property_error(&parser,"expected file-name");
                goto done;
            }
            parser.files[parser.file_index].nbytes = parser.nbytes;
            parser.files[parser.file_index].linenum = parser.linenum;
            parser.files[parser.file_index].column = parser.column;
            parser.files[parser.file_index].cur_ptr = parser.cur_ptr;

            ++parser.file_index;
            parser.files[parser.file_index].fd_input =
                UTEST_File_open_file(ftoken.value.sz_value,O_RDONLY,0);
            if (parser.files[parser.file_index].fd_input < 0)
            {
                UTEST_Property_error(&parser,
                                     "unable to open file-name [%s]\n",
                                     ftoken.value.sz_value);
#ifndef RTI_CERT
                perror("open file failed");
#endif
                goto done;
            }
            parser.files[parser.file_index].fd_buf[0] = 0;
            parser.files[parser.file_index].linenum = 1;
            parser.files[parser.file_index].column = 1;
            parser.files[parser.file_index].cur_ptr = NULL;

            parser.linenum = parser.files[parser.file_index].linenum;
            parser.column = parser.files[parser.file_index].column;
            parser.cur_ptr = parser.files[parser.file_index].cur_ptr;
            parser.fd_input = parser.files[parser.file_index].fd_input;
            parser.fd_buf = parser.files[parser.file_index].fd_buf;
        }
        else if (token.kind == UTEST_TOKEN_KEYWORD)
        {
            if (!strcmp(token.value.sz_value,"property"))
            {
                if (!UTEST_Property_parse_property(&parser,sysinfo,setting))
                {
                    UTEST_Property_error(&parser,"failed to add properties");
                    goto done;
                }
            }
            else
            {
                UTEST_Property_error(&parser,"unknown keyword: [%s]",
                                     token.value.sz_value);
                goto done;
            }
        }
        else if (token.kind == UTEST_TOKEN_SEMICOLON)
        {
            /* ok */
        }
        else
        {
            UTEST_Property_error(&parser,"unknown token: %s\n",
                                 token.value.sz_value);
            goto done;
        }
    }

    return 0;

done:
    return -1;
}
#endif /* HAVE_CONFIG_FILE */

const char*
UTEST_Property_lookup_property(struct UTEST_Context *setting,
                               const char *const name)
{
    int i;

    for (i = 0; i < setting->property_length; ++i)
    {
        if (!strcmp(setting->property[i].name,name))
        {
            return setting->property[i].value;
        }
    }

    return NULL;
}

int
UTEST_Property_lookup_int_property(struct UTEST_Context *setting,
                                   const char *const name,
                                   int *value)
{
    int i;
    char *sz_value = NULL;

    for (i = 0; i < setting->property_length; ++i)
    {
        if (!strcmp(setting->property[i].name,name))
        {
            sz_value = setting->property[i].value;
        }
    }

    if (sz_value == NULL)
    {
        return 0;
    }

    *value = (int)strtol(sz_value,NULL,0);

    return 1;
}

int
UTEST_Property_lookup_uint_property(struct UTEST_Context *setting,
                                    const char *const name,
                                    unsigned int *value)
{
    int i;
    char *sz_value = NULL;

    for (i = 0; i < setting->property_length; ++i)
    {
        if (!strcmp(setting->property[i].name,name))
        {
            sz_value = setting->property[i].value;
        }
    }

    if (sz_value == NULL)
    {
        return 0;
    }

    *value = (unsigned int)strtoul(sz_value,NULL,0);

    return 1;
}

#ifndef RTI_CERT
void
UTEST_Property_delete_property(struct UTEST_Context *setting)
{
    int i;

    for (i = 0; i < setting->property_length; ++i)
    {
        free(setting->property[i].name);
        free(setting->property[i].value);
    }

    setting->property_length = 0;
}
#endif

void
UTEST_Property_add_property(struct UTEST_Context *setting, char *property)
{
    char *p = property;
    char *n = NULL;
    char *v = NULL;

    while (*p && (*p <= 0x20))
    {
        p++;
    }
    n = p;
    while (*p && ((*p > 0x21) && (*p < 0x7e)) && (*p != '='))
    {
        p++;
    }
    if (*p == '=')
    {
        *p = 0;
        ++p;
        while (*p && (*p <= 0x20)) p++;
        if (*p) v = p;
    }
    else
    {
        *p = 0;
        ++p;
        while (*p && (*p != '='))
        {
            p++;
        }
        if (*p)
        {
            *p =0;
        }
        ++p;
        while (*p && (*p <= 0x20))
        {
            p++;
        }
        if (*p)
        {
            v = p;
        }
    }

    while  (*p && ((*p >= 0x20) && (*p < 0x7e)))
    {
        p++;
    }


    if (*p)
    {
        *p = 0;
    }

    if ((n != NULL) && (*n) && (v != NULL) && (*v))
    {
        setting->property[setting->property_length].name = UTEST_String_strdup(n);
        setting->property[setting->property_length].value = UTEST_String_strdup(v);

        UTEST_Stdio_printf("Adding property [%s] = [%s]\n",
                setting->property[setting->property_length].name,
                setting->property[setting->property_length].value);

        ++setting->property_length;
    }
}
