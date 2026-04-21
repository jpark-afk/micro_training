/*
 * FILE: Log.c - Log buffer implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 07may2020,tk MICRO-2364/PR#27267 Removed inclusion of <stdio.h> to avoid
 *                                  warning on QNX.
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 02apr2015,eh MICRO-813/PR#9172 fix Lint warning, vars declared static
 * 12mar2015,tk MICRO-1106/PR#14182 Correctly handle negative numbers, - first
 * 27jan2015,tk MICRO-972/PR#12921 Conditionally include functions that are not
 *                                 supported by Cert
 * 27jan2015,tk MICRO-1026/PR#13453 Allow NULL strings to be logged as
 *                                  pointers
 * 21jan2015,tk Updated log functionality
 * 01dec2014,tk MICRO-942/PR#11892 Removed redundant check on
 *                                 OSAPI_LOG_MIN_LOG_ENTRY_SIZE
 * 01dec2014,tk MICRO-941/PR#11890 Removed check for return value from
 *                                 OSAPI_Mutex_give when no action is taken
 * 01dec2014,tk MICRO-936/PR#11854 Added code to handle failures in get_time()
 * 01dec2014,tk MICRO-939/PR#11860 Added code to handle failures in get_time()
 * 01dec2014,tk MICRO-935/PR#11853 Removed redundant variable
 * 01dec2014,tk MICRO-938/PR#11889 Removed redundant code
 * 16sep2014,tk MICRO-887/PR#10752 Removed reference to removed function
 * 02jul2014,eh MICRO-820: allow OSAPI_Log_set_verbosity before
 *                         OSAPI_Log_initialize
 * 08nov2013,as MICRO-723 OSAPI_Log_msg_pN_X2_i causes buffer overflow when
 *              invoked with log buffer full
 * 07mar2013,tk Updated
 * 22sep2011,tk Updated
 * 23sep2008,yy Created
 */
/*ce
 * \file
 * \brief Implementation of logging facilities
 */
#include "Log.h"

#if !OSAPI_ENABLE_LOG
/*** SOURCE_BEGIN ***/
#endif

#if OSAPI_ENABLE_LOG

#define OSAPI_TRACE_BUF_MAX 511

/*  31  30  29   28-27   26-16   15-0
 * +---+---+---+------+--------+---+----+
 * | X | E | T | TYPE | MODULE | F | EC |
 * +---+---+---+------+--------+--------+
 * |MN|FN|LN|S|  resvd         | length |
 * |------------------------------------+
 * |  Line number   (LN=1)              |
 * |------------------------------------+
 * |  Module name   (MN=1)              |
 * |------------------------------------+
 * |  Function name (FN=1)              |
 * +------------------------------------+
 *
 * F = 1 => no sub header
 * F = 0 => sub headers
 *
 * SUBHEADER:
 * +------------------------------------+
 * |F | TYPE |                          |
 * +------------------------------------+
 * |  NAME                              |
 * |------------------------------------+
 * |  VALUE(S)                          |
 * +------------------------------------+
 *
 * F     - Final subheader
 * TYPE  - The type of the provided value
 * NAME  - The name of the value
 * VALUE - The value (depends on the type)
 *
 */

/*ci
 * \dref_OSAPI_LogImpl
 */
typedef struct OSAPI_LogImpl
{
    /*ci \dref_write_pointer
     * Current write pointer into the log ring-buffer
     */
    char *write_pointer;

    /*ci \dref_buffer_start
     * The beginning of the log buffer
     */
    char *buffer_start;

    /*ci \dref_buffer_end
     * The end of the log buffer
     */
    char *buffer_end;

    /*ci \dref_lock
     * Mutex to protect the ring-buffer
     */
    struct OSAPI_Mutex *lock;

    /*ci \dref_verbosity
     * The verbosity level to log to the ring buffer
     */
    OSAPI_LogVerbosity_T verbosity;

    /*ci \dref_property
     * Log properties, immutable
     */
    struct OSAPI_LogProperty property;

    /*ci \dref_buffer_full
     * buffer full or not
     */
    RTI_BOOL buffer_full;

    /*ci
     * \brief Save the last successfully retrieved timestamp. This
     *        is used if a subsequent call to get_time fails.
     *
     */
    struct OSAPI_NtpTime last_timestamp;

    /*ci
     * \brief The current log entry
     *
     * \details
     * When a log entry is created with is_final to FALSE the log entry
     * is open to accept more variables. This pointer points to the
     * currently open log-entry. Only a single log-entry can be open
     * at any given time.
     */
    struct OSAPI_LogEntry *current_entry;

    /*ci \dref_trace_buffer
     * Temporary buffer to store string for traces and logs
     */
    char trace_buffer[OSAPI_TRACE_BUF_MAX + 1];

    /*ci \dref_is_initialized
     * Whether logging has been initialized or not
     */
    RTI_BOOL is_initialized;
} OSAPI_LogImpl;

/*ci
 * \brief Default function to output log/trace entries
 *
 * \param[in] buffer Buffer to writer, NUL terminated
 * \param[in] length Number of bytes in buffer to write
 */
RTI_PRIVATE void
OSAPI_Log_write(const char *buffer, RTI_SIZE_T length);

/*ci @ingroup OSAPI
 * 
 * @brief The singleton log buffer instance.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct OSAPI_LogImpl OSAPI_LogImpl_gv_Singleton =
{
    NULL,
    NULL,
    NULL, NULL, OSAPI_LOG_VERBOSITY_ERROR,
    {
        OSAPI_LOG_BUFFER_SIZE,
        OSAPI_LOG_DETAIL_ALL,
        OSAPI_Log_write
    },
    RTI_FALSE,
    OSAPI_NTP_TIME_ZERO,
    NULL,
    {0 },
    RTI_FALSE
};

#if OSAPI_ENABLE_TRACE
RTI_PRIVATE void
OSAPI_Trace_default_handler(RTI_UINT32 trace_mask,void *param,RTI_UINT32 context,
        const char *const module,const char *const file,
        const char *const function,RTI_INT32 line_no,
        OSAPI_TraceType_T type,const void *title,
        RTI_INT32 intv,const void *ptrv,const char *strv,
        RTI_BOOL is_final);
#endif /*OSAPI_ENABLE_TRACE*/


RTI_PRIVATE void
OSAPI_Log_default_display(void *param, OSAPI_LogEntry_T *log_message);

/*ci @ingroup OSAPI
 *
 * Optional parameter passed to the user installed log handler
 */
LINK_SECTION_BSS_SDRAM
void *OSAPI_gv_LogFunctionParam = NULL;

LINK_SECTION_BSS_SDRAM
OSAPI_LogHandler_T OSAPI_gv_LogFunction = NULL;

/*ci @ingroup OSAPI
 *
 * Optional parameter passed to the user installed display function
 */
LINK_SECTION_BSS_SDRAM
void* OSAPI_gv_LogDisplayFunctionParam = NULL;

LINK_SECTION_DATA_SDRAM
OSAPI_LogDisplay_T OSAPI_gv_LogDisplayFunction = OSAPI_Log_default_display;

/*ci @ingroup OSAPI
 *
 * Single log_entry used in case log-buffer is full to ensure there is space
 * for a minimum amount of temporary logging if the log-buffer is full
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct OSAPI_LogEntry OSAPI_gv_LogEntry;

#define OSAPI_LOG_MIN_LOG_ENTRY_SIZE \
                                  ((RTI_INT32)(sizeof(struct OSAPI_LogEntry)))

#if OSAPI_ENABLE_TRACE
LINK_SECTION_DATA_SDRAM
OSAPI_TraceHandler_T OSAPI_gv_TraceFunction = OSAPI_Trace_default_handler;

LINK_SECTION_BSS_SDRAM
void *OSAPI_gv_TraceFunctionParam = NULL;

LINK_SECTION_BSS_SDRAM
RTI_UINT32 OSAPI_gv_TraceMask;
#endif

RTI_PRIVATE const char *const OSAPI_Log_fv_Error   = "ERROR";
RTI_PRIVATE const char *const OSAPI_Log_fv_Warning = "WARNING";
RTI_PRIVATE const char *const OSAPI_Log_fv_Info    = "INFO";
RTI_PRIVATE const char *const OSAPI_Log_fv_Precond = "PRECOND";

RTI_PRIVATE const char OSAPI_Log_fv_LB[]="[";
RTI_PRIVATE const char OSAPI_Log_fv_RB[]="]";
#if OSAPI_ENABLE_TRACE
RTI_PRIVATE const char OSAPI_Log_fv_AT[]="@";
#endif
RTI_PRIVATE const char OSAPI_Log_fv_NEWLINE[]="\n";
RTI_PRIVATE const char OSAPI_Log_fv_DOT[]=".";
RTI_PRIVATE const char OSAPI_Log_fv_COLON[]=":";
RTI_PRIVATE const char OSAPI_Log_fv_SLASH[]="/";
RTI_PRIVATE const char OSAPI_Log_fv_EQUAL[]="=";

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Take the logging lock mutex.
 * Log lock mutex might be NULL if the logging module has not been yet
 * initialized. In that case we proceed without locking and assume that
 * at the moment there is only one thread.
 *
 * \return If logging lock mutex is not NULL return value of call to
 *         OSAPI_Mutex_take(). Otherwise TRUE.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Log_lock(void)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;

    if (log->lock != NULL)
    {
        return OSAPI_Mutex_take(log->lock);
    }
    
    return RTI_TRUE;
}

/*ci
 * \brief Release the logging lock mutex. Log lock mutex might be NULL if the
 * logging module is not initialized. In that case this function does not
 * perform any operation.
 *
 * \return If logging lock mutex is not NULL return value of call to
 *         OSAPI_Mutex_give(). Otherwise TRUE.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Log_unlock(void)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;

    if (log->lock != NULL)
    {
        return OSAPI_Mutex_give(log->lock);
    }
    
    return RTI_TRUE;
}

/*ci
 * \brief Checks whether the logging module is initialized.
 *
 * \return RTI_TRUE if the logging module has been initialied.
 *         Otherwise returns RTI_FALSE.
 */
RTI_BOOL
OSAPI_Log_is_initialized(void)
{
    return OSAPI_LogImpl_gv_Singleton.is_initialized;
}

/*ci
 * \brief Gets sysmte time.
 *
 * If the logging module is not initialized, it might happen that
 * the system module is not initialized. This is the reason why
 * this function returns RTI_FALSE and OSAPI_NTP_TIME_ZERO time
 * if the logging module is not initialized.
 *
 * \return RTI_TRUE system returned a correct time.
 *         RTI_FALSE in case the logging module is not initialized
 *                   or the system returned error. In that case
 *                   the output value of the input parameter will
 *                   be set to OSAPI_NTP_TIME_ZERO.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Log_get_time(struct OSAPI_NtpTime *timestamp)
{
    struct OSAPI_NtpTime zero_time = OSAPI_NTP_TIME_ZERO;

    if (OSAPI_Log_is_initialized())
    {
        if (OSAPI_System_get_time(timestamp))
        {
            return RTI_TRUE;
        }
    }

    *timestamp = zero_time;
    
    return RTI_FALSE;
}

RTI_PRIVATE void
OSAPI_Log_write(const char *buffer,RTI_SIZE_T length)
{
#ifdef OSAPI_LOG_WRITE_BUFFER
    OSAPI_LOG_WRITE_BUFFER(buffer,length);
#else
    UNUSED_ARG(buffer);
    UNUSED_ARG(length);
#warning "This platform does not have a known method to print to standard output. To see trace and log output, please install a write_buffer in the logger"
#endif
}

void
OSAPI_Log_get_property(struct OSAPI_LogProperty *property)
{
    *property = OSAPI_LogImpl_gv_Singleton.property;
}

RTI_BOOL
OSAPI_Log_set_property(struct OSAPI_LogProperty *property)
{
    if (OSAPI_Log_is_initialized())
    {
        return RTI_FALSE;
    }

    OSAPI_LogImpl_gv_Singleton.property = *property;

    return RTI_TRUE;
}

OSAPI_LogVerbosity_T
OSAPI_Log_get_verbosity(void)
{
    return OSAPI_LogImpl_gv_Singleton.verbosity;
}

RTI_BOOL
OSAPI_Log_set_log_handler(OSAPI_LogHandler_T handler, void *param)
{
    if (handler == NULL)
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    OSAPI_gv_LogFunction = handler;
    OSAPI_gv_LogFunctionParam = param;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Log_get_log_handler(OSAPI_LogHandler_T *handler, void **param)
{
    if ((handler == NULL) || (param == NULL))
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    *handler = OSAPI_gv_LogFunction;
    *param = OSAPI_gv_LogFunctionParam;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

#if OSAPI_ENABLE_TRACE
RTI_BOOL
OSAPI_Log_set_trace_handler(OSAPI_TraceHandler_T handler, void *param)
{
    if (handler == NULL)
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    OSAPI_gv_TraceFunction = handler;
    OSAPI_gv_TraceFunctionParam = param;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_Log_get_trace_handler(OSAPI_TraceHandler_T *handler, void **param)
{
    if ((handler == NULL) || (param == NULL))
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    *handler = OSAPI_gv_TraceFunction;
    *param = OSAPI_gv_TraceFunctionParam;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

RTI_BOOL
OSAPI_Log_set_display_handler(OSAPI_LogDisplay_T handler, void *param)
{
    if (handler == NULL)
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    OSAPI_gv_LogDisplayFunction = handler;
    OSAPI_gv_LogDisplayFunctionParam = param;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Log_get_display_handler(OSAPI_LogDisplay_T *handler, void **param)
{
    if ((handler == NULL) || (param == NULL))
    {
        return RTI_FALSE;
    }

    if ((OSAPI_LogImpl_gv_Singleton.lock == NULL) ||
         ((OSAPI_LogImpl_gv_Singleton.lock != NULL) &&
         !OSAPI_Mutex_take(OSAPI_LogImpl_gv_Singleton.lock)))
    {
        return RTI_FALSE;
    }

    *handler = OSAPI_gv_LogDisplayFunction;
    *param = OSAPI_gv_LogDisplayFunctionParam;

    if (!OSAPI_Mutex_give(OSAPI_LogImpl_gv_Singleton.lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

RTI_BOOL
OSAPI_Log_initialize(void)
{
    struct OSAPI_NtpTime zero_time = OSAPI_NTP_TIME_ZERO;

    if (OSAPI_Log_is_initialized())
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_allocate_buffer(
            (char **)&OSAPI_LogImpl_gv_Singleton.buffer_start,
            OSAPI_LogImpl_gv_Singleton.property.max_buffer_size,
            OSAPI_ALIGNMENT_DEFAULT);

    if (OSAPI_LogImpl_gv_Singleton.buffer_start == NULL)
    {
        return RTI_FALSE;
    }

    OSAPI_gv_LogFunctionParam = NULL;
    OSAPI_gv_LogFunction = NULL;

    OSAPI_gv_LogDisplayFunctionParam = NULL;
    OSAPI_gv_LogDisplayFunction = OSAPI_Log_default_display;

#if OSAPI_ENABLE_TRACE
    OSAPI_gv_TraceFunction = OSAPI_Trace_default_handler;
    OSAPI_gv_TraceFunctionParam = NULL;
#endif

    OSAPI_LogImpl_gv_Singleton.lock = OSAPI_Mutex_new();
    if (OSAPI_LogImpl_gv_Singleton.lock == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(OSAPI_LogImpl_gv_Singleton.buffer_start);
#endif /* !RTI_CERT */
        OSAPI_LogImpl_gv_Singleton.buffer_start = NULL;
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(OSAPI_LogImpl_gv_Singleton.buffer_start,
                      OSAPI_LogImpl_gv_Singleton.property.max_buffer_size);

    OSAPI_LogImpl_gv_Singleton.write_pointer =
                                       OSAPI_LogImpl_gv_Singleton.buffer_start;

    /* buffer_end points at the last byte + 1 in the buffer.
     * write_pointer points to the first free address
     *
     * If write_pointer >= buffer_end the buffer is full
     * buffer_end - writer = number of bytes free
     */
    OSAPI_LogImpl_gv_Singleton.buffer_end =
                OSAPI_LogImpl_gv_Singleton.buffer_start +
                OSAPI_LogImpl_gv_Singleton.property.max_buffer_size;

    OSAPI_LogImpl_gv_Singleton.last_timestamp = zero_time;
    OSAPI_LogImpl_gv_Singleton.current_entry = NULL;
    OSAPI_LogImpl_gv_Singleton.buffer_full = RTI_FALSE;

    if (OSAPI_LogImpl_gv_Singleton.property.write_buffer == NULL)
    {
        OSAPI_LogImpl_gv_Singleton.property.write_buffer = OSAPI_Log_write;
    }

    OSAPI_LogImpl_gv_Singleton.trace_buffer[0] = 0;


    OSAPI_LogImpl_gv_Singleton.is_initialized = RTI_TRUE;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Log_finalize(void)
{
    if (!OSAPI_Log_is_initialized())
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free_buffer((char *)OSAPI_LogImpl_gv_Singleton.buffer_start);
    OSAPI_LogImpl_gv_Singleton.buffer_start = NULL;

    if (OSAPI_LogImpl_gv_Singleton.lock != NULL)
    {
        if (!OSAPI_Mutex_delete(OSAPI_LogImpl_gv_Singleton.lock))
        {
            return RTI_FALSE;
        }
        OSAPI_LogImpl_gv_Singleton.lock = NULL;
    }

    /* reset singleton to its state before OSAPI_Log_initialize().
     * This is needed in case OSAPI_Log_finalize() is called again.
     * An initializer is not used because the structure has an array
     * which is 511 bytes long, and that would use too much stack
     * memory.
     */
    OSAPI_LogImpl_gv_Singleton.write_pointer = NULL;
    OSAPI_LogImpl_gv_Singleton.buffer_end = NULL;
    OSAPI_LogImpl_gv_Singleton.lock = NULL;
    OSAPI_LogImpl_gv_Singleton.buffer_full = RTI_FALSE;
    OSAPI_LogImpl_gv_Singleton.current_entry = NULL;
    OSAPI_LogImpl_gv_Singleton.trace_buffer[0] = 0;

    OSAPI_gv_LogFunctionParam = NULL;
    OSAPI_gv_LogFunction = NULL;

    OSAPI_gv_LogDisplayFunctionParam = NULL;
    OSAPI_gv_LogDisplayFunction = OSAPI_Log_default_display;

#if OSAPI_ENABLE_TRACE
    OSAPI_gv_TraceFunction = OSAPI_Trace_default_handler;
    OSAPI_gv_TraceFunctionParam = NULL;
#endif

    OSAPI_LogImpl_gv_Singleton.is_initialized = RTI_FALSE;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

void
OSAPI_Log_set_verbosity(OSAPI_LogVerbosity_T verbosity)
{
    OSAPI_LogImpl_gv_Singleton.verbosity = verbosity;
}

#if OSAPI_ENABLE_TRACE
void
OSAPI_Trace_set_trace_mask(RTI_UINT32 mask)
{
    OSAPI_gv_TraceMask = mask;
}
#endif

/*ci
 * \brief Convert a binary to a string representation
 *
 * \param[inout] buffer     Buffer to store result in
 * \param[in]    max_length Maximum length of the buffer
 * \param[in]    name       Optional name for the converted data
 * \param[in]    format     Format specifier
 * \param[in]    args       Pointer to data to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
RTI_PRIVATE void
OSAPI_Log_write_line(char *buffer,RTI_SIZE_T max_length,
                     const char *name,RTI_INT32 format,const void *args)
{
    RTI_SIZE_T n_length = 0;
    RTI_SIZE_T v_length = 0;
    RTI_INT32 *intv = NULL;
    char *ptr = NULL;
    char *optr = buffer;
    static const char overrun[]="overrun";
    RTI_SIZE_T i;
    RTI_INT8 *cv;
    static const char hexdigit[]="0123456789abcdef";

    if (name != NULL)
    {
        buffer[0] = ' ';
        ++buffer;
        --max_length;
        n_length = OSAPI_String_length((char*)name);
        v_length = OSAPI_String_length(OSAPI_Log_fv_EQUAL);
        if ((n_length + v_length) >= max_length)
        {
            goto failure;
        }
        OSAPI_Memory_copy(buffer,name,n_length);
        buffer += n_length;
        max_length -= n_length;
        OSAPI_Memory_copy(buffer,OSAPI_Log_fv_EQUAL,v_length+1);
        buffer += v_length;
        max_length -= v_length;
        *buffer = 0;
    }
    else
    {
        *buffer = 0;
    }

    switch(format)
    {
    case 's':
        v_length = OSAPI_String_length((char*)args);
        if (v_length >= max_length)
        {
            goto failure;
        }
        OSAPI_Memory_copy(buffer,args,v_length+1);
        break;
    case 'g':
        intv = (RTI_INT32*)args;
        ptr = buffer;
        for (i = 0; i < 4U; ++i)
        {
            v_length = OSAPI_Log_itoa(ptr,max_length,intv[i]);
            if (v_length >= max_length)
            {
                goto failure;
            }
            ptr += v_length;
            max_length -= v_length;
            if (i < 3U)
            {
                if (max_length <= 1)
                {
                    goto failure;
                }
                *ptr = '.';
                ++ptr;
            }
        }
        break;
    case 'x':
        ptr = buffer;
        /* Add 2 for leading 0x */
        if (max_length <= (sizeof(void*) * 2 + 2))
        {
            goto failure;
        }
        *ptr = '0';
        ptr++;
        *ptr = 'x';
        ptr++;
        cv = (RTI_INT8*) args;
#ifdef RTI_ENDIAN_LITTLE
        cv += sizeof(RTI_INT32) - 1;
#endif
        for (i = 0U; i < sizeof(RTI_INT32); i++)
        {
            *ptr = hexdigit[(*cv >> 4) & 0xf];
            ++ptr;
            *ptr = hexdigit[*cv & 0xf];
            ++ptr;
#ifdef RTI_ENDIAN_LITTLE
            --cv;
#else
            ++cv;
#endif
        }
        *ptr = 0;
        break;

    case 'p':
        ptr = buffer;
        if (max_length <= (sizeof(void*)*2))
        {
            goto failure;
        }
        cv = (RTI_INT8*)&args;
#ifdef RTI_ENDIAN_LITTLE
        cv += (RTI_INT32)sizeof(void*)-1;
#endif
        for (i = 0; i < sizeof(void*); i++)
        {
            *ptr = hexdigit[(*cv >> 4) & 0xf];
            ++ptr;
            *ptr = hexdigit[*cv & 0xf];
            ++ptr;
#ifdef RTI_ENDIAN_LITTLE
            --cv;
#else
            ++cv;
#endif
        }
        *ptr = 0;
        break;
    case 'd':
        if (OSAPI_Log_itoa(buffer,max_length,*(RTI_INT32*)args) >= max_length)
        {
            goto failure;
        }
        break;
    case 'v':
        intv = (RTI_INT32*)args;
        if (OSAPI_Log_itoa(buffer,max_length,intv[0]) >= max_length)
        {
            goto failure;
        }
        break;
    case 'V':
        intv = (RTI_INT32*)args;
        for (i = 0; i < 3; ++i)
        {
            if (OSAPI_Log_itoa(buffer,max_length,intv[i]) >= max_length)
            {
                goto failure;
            }
        }
        break;
    }

    OSAPI_LogImpl_gv_Singleton.property.write_buffer(optr,OSAPI_String_length(optr)+1);

    return;

failure:
    OSAPI_LogImpl_gv_Singleton.property.write_buffer(overrun,OSAPI_String_length(overrun)+1);
}

/*ci
 * \brief Find the basename of a file (exclude any directory path)
 *
 * \param[in]    file       File to find the basename of
 *
 * \return Pointer to basename of file
 */
RTI_PRIVATE const char*
OSAPI_Log_basename(const char *file)
{
    const char *basename = file + OSAPI_String_length(file);

    while ((basename != file) && (*basename != '/')
            && (*basename != '\\'))
    {
        basename--;
    }

    if (basename != file)
    {
        basename++;
    }

    return basename;
}

#if OSAPI_ENABLE_TRACE
/*ci
 * \brief Format a trace message
 *
 * \details
 * This function prints a trace meessage to standard output
 *
 * \param[in] trace_mask - Which mask this trace uses
 * \param[in] param      - Optional trace parameter (not used)
 * \param[in] context    - The trace context
 * \param[in] module     - Module of log message
 * \param[in] file       - Name of source file
 * \param[in] function   - Name of function
 * \param[in] line_no    - Line number
 * \param[in] type       - The type of the trace
 * \param[in] title      - The title of the trace
 * \param[in] intv       - Pointer to integer type trace
 * \param[in] ptrv       - Pointer to pointer type trace
 * \param[in] strv       - Pointer to string type trace
 * \param[in] is_final   - RTI_TRUE if this is the last datum in the trace
 */
RTI_PRIVATE void
OSAPI_Trace_default_handler(RTI_UINT32 trace_mask,void *param,RTI_UINT32 context,
                            const char *const module,const char *const file,
                            const char *const function,RTI_INT32 line_no,
                            OSAPI_TraceType_T type,const void *title,
                            RTI_INT32 intv,const void *ptrv,const char *strv,
                            RTI_BOOL is_final)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    char *ptr;
    RTI_INT32 sec;
    RTI_UINT32 nsec;
    struct OSAPI_NtpTime timestamp = OSAPI_NTP_TIME_ZERO;
    RTI_BOOL bval;
    RTI_SIZE_T max_length = OSAPI_TRACE_BUF_MAX;
    OSAPI_ProcessId pid;
    UNUSED_ARG(param);
    UNUSED_ARG(is_final);
    UNUSED_ARG(module);
    UNUSED_ARG(file);
    UNUSED_ARG(function);
    UNUSED_ARG(line_no);

    if ((trace_mask == 0)
        || ((trace_mask != 0) && (context != 0) && !(context & trace_mask)))
    {
        return;
    }

    if (!OSAPI_Log_lock())
    {
        return;
    }

    ptr = log->trace_buffer;
    *ptr = 0;

    if (type == OSAPI_TRACETYPE_HEADER)
    {
        OSAPI_Log_write_line(ptr,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);

        bval = OSAPI_Log_get_time(&timestamp);
        IGNORE_RETVAL(bval);

        OSAPI_NtpTime_to_nanosec(&sec, &nsec, &timestamp);

        /* Create a log-string:
         * [PID@sec.nanosec]: title\n
         */
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_LB);
        pid = OSAPI_Process_getpid();
        if (max_length == OSAPI_Process_pid_as_string(log->trace_buffer,
                                                      max_length,pid))
        {
            return;
        }
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_AT);
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'d',&sec);
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_DOT);
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'d',&nsec);
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_RB);
        OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_COLON);
        OSAPI_Log_write_line(ptr,max_length,NULL,'s',title);
        OSAPI_Log_write_line(ptr,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);
    }
    else
    {
        switch (type)
        {
        case OSAPI_TRACETYPE_GUID:
            OSAPI_Log_write_line(log->trace_buffer,max_length,title,'g',ptrv);
            break;
        case OSAPI_TRACETYPE_INT32:
            OSAPI_Log_write_line(log->trace_buffer,max_length,title,'d',&intv);
            break;
        case OSAPI_TRACETYPE_STRING:
            OSAPI_Log_write_line(log->trace_buffer,max_length,title,'s',strv);
            break;
        case OSAPI_TRACETYPE_V4_AS_INT32:
            OSAPI_Log_write_line(log->trace_buffer,max_length,title,'v',ptrv);
            break;
        case OSAPI_TRACETYPE_V12_AS_INT32:
            OSAPI_Log_write_line(log->trace_buffer,max_length,title,'V',ptrv);
            break;
        default:
            break;
        }
    }

    OSAPI_Log_write_line(ptr,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);

    /* Since the function is returning anyway, ignore the result */
    bval = OSAPI_Log_unlock();
    IGNORE_RETVAL(bval);
}
#endif

/*ci
 * \brief Display a log message
 *
 * \details
 * This function prints a trace message to standard output
 *
 * \param[in] param     - Optional trace parameter (not used)
 * \param[in] log_entry - The log_entry to display
 *
 */
RTI_PRIVATE void
OSAPI_Log_default_display(void *param, OSAPI_LogEntry_T *log_entry)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    const char *log_type = NULL;
    RTI_INT32 kind;
    RTI_UINT32 eh;
    RTI_UINT32 status = 0;
    const char *module = "<not found>";
    const char *file = "<not found>";
    const char *func = "<not found>";
    RTI_INT32 lineno = -1;
    RTI_INT32 sec;
    RTI_UINT32 nsec;
    char *data_ptr;
    OSAPI_LogType_T type;
    RTI_BOOL is_final = RTI_FALSE;
    const char *name;
    const void *value;
    RTI_SIZE_T max_length = OSAPI_TRACE_BUF_MAX;
    RTI_UINT32 val;

    UNUSED_ARG(param);

    kind = OSAPI_LOG_HEADER_GET_TYPE(log_entry->error_code);

    if ((kind == OSAPI_LOGKIND_INFO)
            && (OSAPI_LogImpl_gv_Singleton.verbosity
                    >= OSAPI_LOG_VERBOSITY_DEBUG))
    {
        log_type = OSAPI_Log_fv_Info;
    }
    else if ((kind == OSAPI_LOGKIND_WARNING)
            && (OSAPI_LogImpl_gv_Singleton.verbosity
                    >= OSAPI_LOG_VERBOSITY_WARNING))
    {
        log_type = OSAPI_Log_fv_Warning;
    }
    else if ((kind == OSAPI_LOGKIND_ERROR)
            && (OSAPI_LogImpl_gv_Singleton.verbosity
                    >= OSAPI_LOG_VERBOSITY_ERROR))
    {
        log_type = OSAPI_Log_fv_Error;
    }
    else if ((kind == OSAPI_LOGKIND_PRECONDITION)
            && (OSAPI_LogImpl_gv_Singleton.verbosity
                    >= OSAPI_LOG_VERBOSITY_ERROR))
    {
        log_type = OSAPI_Log_fv_Precond;
    }
    else
    {
        return;
    }

    eh = log_entry->error_code;

    if (OSAPI_LOG_HEADER_GET_X(log_entry->error_code))
    {
        data_ptr = (char*)&log_entry[1];
        OSAPI_Memory_copy(&status,data_ptr,sizeof(RTI_UINT32));
        data_ptr += sizeof(RTI_INT32);
        if (status & OSAPI_LOG_STATUS_LN)
        {
            OSAPI_Memory_copy(&lineno,data_ptr,sizeof(RTI_INT32));
            data_ptr += (RTI_INT32)sizeof(RTI_INT32);
        }
        if (status & OSAPI_LOG_STATUS_MN)
        {
            module = data_ptr;
            data_ptr += OSAPI_String_length(data_ptr) + 1;
        }
        if (status & OSAPI_LOG_STATUS_SF)
        {
            file = data_ptr;
            data_ptr += OSAPI_String_length(data_ptr) + 1;
        }
        if (status & OSAPI_LOG_STATUS_FN)
        {
            func = data_ptr;
            data_ptr += OSAPI_String_length(data_ptr) + 1;
        }
        if (status & OSAPI_LOG_STATUS_F)
        {
            data_ptr += OSAPI_String_length(data_ptr) + 1;
        }
    }
    OSAPI_NtpTime_to_nanosec(&sec, &nsec, &log_entry->timestamp);

    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);

    /* [%d.%d]%s:*/
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_LB);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'d',&sec);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_DOT);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'d',&nsec);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_RB);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',log_type);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_COLON);

    /* ModuleID=%d,Errcode=%d,X=%d,E=%d,T=%d\n*/
    val = OSAPI_LOG_HEADER_GET_MODULE(log_entry->error_code);
    OSAPI_Log_write_line(log->trace_buffer,max_length,"ModuleID",'d',&val);
    val = OSAPI_LOG_HEADER_GET_EC(log_entry->error_code);
    OSAPI_Log_write_line(log->trace_buffer,max_length,"Errcode",'d',&val);
    val = OSAPI_LOG_HEADER_GET_X(eh);
    OSAPI_Log_write_line(log->trace_buffer,max_length,"X",'d',&val);
    val = OSAPI_LOG_HEADER_GET_E(eh);
    OSAPI_Log_write_line(log->trace_buffer,max_length,"E",'d',&val);
    val = OSAPI_LOG_HEADER_GET_T(eh);
    OSAPI_Log_write_line(log->trace_buffer,max_length,"T",'d',&val);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);

    /* module/file:lineno/func: args */
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',module);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_SLASH);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',file);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_COLON);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'d',&lineno);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_SLASH);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',func);
    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_COLON);

    data_ptr = NULL;
    is_final = RTI_FALSE;

    while (OSAPI_Log_entry_get_data(log_entry,&data_ptr,&type,
                                    &name,&value,&is_final))
    {
        switch (type)
        {
        case OSAPI_LOGTYPE_HEX:
            OSAPI_Log_write_line(log->trace_buffer,max_length,name,'x',value);
            break;
        case OSAPI_LOGTYPE_INTEGER:
            OSAPI_Log_write_line(log->trace_buffer,max_length,name,'d',value);
            break;
        case OSAPI_LOGTYPE_UINTEGER:
            OSAPI_Log_write_line(log->trace_buffer,max_length,name,'u',value);
            break;
        case OSAPI_LOGTYPE_POINTER:
            OSAPI_Log_write_line(log->trace_buffer,max_length,name,'p',value);
            break;
        case OSAPI_LOGTYPE_STRING:
            OSAPI_Log_write_line(log->trace_buffer,max_length,name,'s',value);
            break;
        default:
            break;
        }

        if (is_final)
        {
            break;
        }
    }

    OSAPI_Log_write_line(log->trace_buffer,max_length,NULL,'s',&OSAPI_Log_fv_NEWLINE);

}

RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return errno;
}

RTI_BOOL
OSAPI_Log_clear(void)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    
    /* OSAPI_Log_clear can only be called inside a log handler */
    if (log->lock == NULL)
    {
        return RTI_FALSE;
    }
    OSAPI_LogImpl_gv_Singleton.write_pointer =
            OSAPI_LogImpl_gv_Singleton.buffer_start;

    OSAPI_LogImpl_gv_Singleton.buffer_full = RTI_FALSE;

    OSAPI_Memory_zero(
            OSAPI_LogImpl_gv_Singleton.buffer_start,
            (RTI_SIZE_T)(OSAPI_LogImpl_gv_Singleton.buffer_end -
                         OSAPI_LogImpl_gv_Singleton.buffer_start));
    
    return RTI_TRUE;
}

/*ci
 * \brief Call user installed handles for log-entries
 *
 * \details
 * This function prints a trace meessage to standard output
 *
 * \param[in] log_entry Log entry to pass to user installed handlers
 */
RTI_PRIVATE void
OSAPI_Log_call_user_handlers(OSAPI_LogEntry_T *log_entry)
{
    if (OSAPI_gv_LogDisplayFunction != NULL)
    {
        OSAPI_gv_LogDisplayFunction(OSAPI_gv_LogDisplayFunctionParam,
                log_entry);
    }

    if (OSAPI_gv_LogFunction != NULL)
    {
        OSAPI_gv_LogFunction(OSAPI_gv_LogFunctionParam,log_entry);
    }
}

/*ci
 * \brief Create a header for log entry
 *
 * \details
 * Add a header to a new log entry
 *
 * \param[in] kind        The kind of log entry
 * \param[in] error_code  The error code
 * \param[in] module      The module the error occurred in
 * \param[in] file        The file the error occurred in
 * \param[in] func        The function the error occurred in
 * \param[in] line        The line the error occurred in
 */
RTI_PRIVATE void
OSAPI_Log_add_entry_header(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                           const char *const module, const char *const file,
                           const char *const func, RTI_INT32 line)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    OSAPI_LogEntry_T *log_entry = NULL;
    RTI_SIZE_T w_length;
    RTI_UINT32 *status;
    RTI_UINT32 log_detail = 0;
    const char *basename = NULL;

    /* Always log additional data for info, because for info logs
     * that is what important. For example, library version needs addition
     * information.
     */
    log_detail = log->property.log_detail;
    if (kind == OSAPI_LOGKIND_INFO)
    {
        log_detail |= OSAPI_LOG_DETAIL_DATA_ONLY;
    }

#define space_left ((RTI_UINT32)(log->buffer_end - log->write_pointer))

    if ((!OSAPI_Log_is_initialized()) ||
        (space_left < OSAPI_LOG_MIN_LOG_ENTRY_SIZE) ||
        (log->buffer_full))
    {
        log_entry = &OSAPI_gv_LogEntry;
        log_entry->error_code = 0;
        OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
        log_detail = 0;
    }
    else
    {
        OSAPI_Memory_copy(&log_entry,&log->write_pointer,sizeof(void*));
        log_entry->error_code = 0;
    }

    log->current_entry = log_entry;

    if (OSAPI_Log_get_time(&log_entry->timestamp))
    {
        OSAPI_LOG_HEADER_SET_T(log_entry->error_code);
        log->last_timestamp = log_entry->timestamp;
    }
    else
    {
        log_entry->timestamp = log->last_timestamp;
        OSAPI_LOG_HEADER_CLR_T(log_entry->error_code);
    }

    log_entry->error_code |= error_code;
    OSAPI_LOG_HEADER_SET_TYPE(log_entry->error_code, kind);

    if (log_detail)
    {
        log->write_pointer = (char*)&log_entry[1];

        OSAPI_LOG_HEADER_SET_X(log_entry->error_code);

        if (space_left < sizeof(RTI_INT32))
        {
            OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
            goto failure;
        }

        OSAPI_Memory_copy(&status,&log->write_pointer,sizeof(RTI_UINT32*));
        *status = 0;
        log->write_pointer += sizeof(RTI_INT32);

        if (log_detail & OSAPI_LOG_DETAIL_LINENUMBER)
        {
            if (space_left < sizeof(RTI_INT32))
            {
                OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
                goto failure;
            }
            *status |= OSAPI_LOG_STATUS_LN;
            OSAPI_Memory_copy(log->write_pointer,&line,sizeof(RTI_INT32));
            log->write_pointer += sizeof(RTI_INT32);
        }

        if (log_detail & OSAPI_LOG_DETAIL_MODULENAME)
        {
            w_length = OSAPI_String_length((char*)module)+1;
            if (w_length > space_left)
            {
                OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
                goto failure;
            }
            OSAPI_Memory_copy(log->write_pointer,module,w_length);
            /* move beyond '\0' */
            *status |= OSAPI_LOG_STATUS_MN;
            log->write_pointer += w_length;
        }

        if (log_detail & OSAPI_LOG_DETAIL_SOURCEFILE)
        {
            basename = OSAPI_Log_basename(file);
            w_length = OSAPI_String_length((char*)basename) + 1;
            if (w_length > space_left)
            {
                OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
                goto failure;
            }
            OSAPI_Memory_copy(log->write_pointer,basename,w_length);

            /* move beyond '\0' */
            *status |= OSAPI_LOG_STATUS_SF;
            log->write_pointer += w_length;
        }

        if (log_detail & OSAPI_LOG_DETAIL_FUNCTIONAME)
        {
            w_length = OSAPI_String_length((char*)func)+1;
            if (w_length > space_left)
            {
                OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
                goto failure;
            }

            OSAPI_Memory_copy(log->write_pointer,func,w_length);
            *status |= OSAPI_LOG_STATUS_FN;
            log->write_pointer += w_length;
        }

        /* At this point log->write_pointer may be unaligned. Always align
         * up the pointer to a void* for the next entry (either data or
         * a new log_entry)
         */
        w_length = (RTI_SIZE_T)(log->write_pointer - (char*)&log_entry[1]);
        w_length = (RTI_SIZE_T)((w_length + (RTI_SIZE_T)(sizeof(void*) - 1)) & ~(RTI_SIZE_T)(sizeof(void*) - 1));
        *status |= w_length;
        log->write_pointer = (char*)&log_entry[1] + w_length;

        if (log->write_pointer >= log->buffer_end)
        {
            OSAPI_LOG_HEADER_SET_E(log_entry->error_code);
            goto failure;
        }
    }
    else if (log_entry != &OSAPI_gv_LogEntry)
    {
        /* if log_detail is 0 and log_entry is from buffer, update
         * writer pointer
         */
        log->write_pointer += OSAPI_LOG_MIN_LOG_ENTRY_SIZE;
    }
    else
    {
        /* log_detail is 0 and log_entry is local => nothing to do.
         */
    }

failure:

    if (OSAPI_LOG_HEADER_GET_E(log_entry->error_code))
    {
        OSAPI_LOG_HEADER_CLR_X(log_entry->error_code);
        log->buffer_full = RTI_TRUE;
    }

    if (kind == OSAPI_LOGKIND_ERROR)
    {
        errno = (RTI_INT32)(error_code & 0x7fffffff);
    }
#undef space_left
}

void
OSAPI_Log_entry_create(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                       const char *const module, const char *const file,
                       const char *const func, RTI_INT32 line,
                       RTI_BOOL is_final)
{
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    RTI_BOOL retval;
    OSAPI_LogEntry_T *user_log_entry = NULL;
    struct OSAPI_LogEntry min_entry;

    /* lock might be null if the logging module has not been initialized yet.
     * We proceed with the log and assume that at the moment there is only one
     * thread.
     */
    if (!OSAPI_Log_lock())
    {
        return;
    }

    /* There is always space for a header, but the buffer may be full */
    OSAPI_Log_add_entry_header(kind,error_code,module,file,func,line);
    if (!is_final)
    {
        /* Additional data will be added */
        OSAPI_LOG_HEADER_CLR_F(log->current_entry->error_code);
        return;
    }

    /* The final data-element was added, clean-up */
    OSAPI_LOG_HEADER_SET_F(log->current_entry->error_code);

    if (OSAPI_LOG_HEADER_GET_E(log->current_entry->error_code) || log->buffer_full)
    {
        min_entry = *log->current_entry;
        user_log_entry = &min_entry;
    }
    else
    {
        user_log_entry = log->current_entry;
    }

    log->current_entry = NULL;

    OSAPI_Log_call_user_handlers(user_log_entry);
    
    retval = OSAPI_Log_unlock();
    IGNORE_RETVAL(retval);
}

/*ci
 * \brief Calculate the difference need to add to a pointer to align after a
 *        an offset is added
 *
 * \param[in] inptr  The base pointer
 * \param[in] offset The offset to add to the pointer before realigning
 * \param[in] align  The alignment for the pointer after the offset is added
 *
 * \return The offset to add to inptr to align it to align
 */
RTI_SIZE_T
OSAPI_Log_ptr_diff(const char *inptr,RTI_SIZE_T offset,RTI_UINT32 align)
{
    RTI_UINT32 inptr_val = (RTI_UINT32)(inptr - OSAPI_ADDRESS_ZERO);
    if (align == 0)
    {
        return offset;
    }

    return (((inptr_val + offset) + (align-1U)) & ~(align - 1U)) - inptr_val;
}

/*ci
 * \brief Copy a string into the log-entry
 *
 * \param[in] log   The log entry
 * \param[in] value The string value
 * \param[in] value The alignment required after the string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure (not enough space)
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Log_entry_copy_string(struct OSAPI_LogImpl *log,
                            const char *value,
                            RTI_UINT32 alignment)
{
#define space_left ((RTI_SIZE_T)(log->buffer_end - log->write_pointer))
    RTI_SIZE_T str_len;

    str_len = OSAPI_String_length((char*)value) + 1;
    if (str_len > space_left)
    {
        OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(log->write_pointer,value,str_len);

    /* This works for string < 4GB */

    if (alignment > 0)
    {
        str_len = OSAPI_Log_ptr_diff(log->write_pointer,str_len,alignment);
    }

    if (str_len > space_left)
    {
        OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
        return RTI_FALSE;
    }

    log->write_pointer += str_len;

    return RTI_TRUE;

#undef space_left
}

/*ci
 * \brief Copy a data into the log-entry
 *
 * \param[in] type     The type of the data
 * \param[in] name     The name of the data
 * \param[in] value    The value
 * \param[in] is_final RTI_TRUE if this is the last value in the log-entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure (not enough space)
 */
RTI_PRIVATE void
OSAPI_Log_entry_add_data(OSAPI_LogType_T type,const char *name,
                         const void *value,RTI_BOOL is_final)
{
#define space_left ((RTI_SIZE_T)(log->buffer_end - log->write_pointer))
    struct OSAPI_LogImpl *log = &OSAPI_LogImpl_gv_Singleton;
    RTI_UINT32 *header = NULL;
    RTI_BOOL retval;
    OSAPI_LogEntry_T *user_log_entry = NULL;
    struct OSAPI_LogEntry min_entry;

    /* Extra precaution in case the log-buffer has not been initialized yet.
     */
    if (log->current_entry == NULL)
    {
        return;
    }

    if (OSAPI_LOG_HEADER_GET_E(log->current_entry->error_code) || log->buffer_full)
    {
        OSAPI_LOG_HEADER_SET_F(log->current_entry->error_code);
        if (is_final)
        {
            goto done;
        }
        return;
    }

    if (sizeof(RTI_INT32) > space_left)
    {
        OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
        goto done;
    }

    OSAPI_Memory_copy(&header,&log->write_pointer,sizeof(RTI_UINT32*));
    if (is_final)
    {
        OSAPI_LOGDATA_SET_F(*header);
    }

    OSAPI_LOGDATA_SET_TYPE(*header,type);

    log->write_pointer += sizeof(RTI_INT32);

    switch(type)
    {
    case OSAPI_LOGTYPE_HEX:
    case OSAPI_LOGTYPE_INTEGER:
        if (!OSAPI_Log_entry_copy_string(log,name,sizeof(RTI_INT32)))
        {
            goto done;
        }
        if ( sizeof(RTI_INT32) > space_left)
        {
            OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
            goto done;
        }
        OSAPI_Memory_copy(log->write_pointer,value,sizeof(RTI_INT32));
        log->write_pointer += sizeof(RTI_INT32);
        break;
    case OSAPI_LOGTYPE_UINTEGER:
        if (!OSAPI_Log_entry_copy_string(log,name,sizeof(RTI_UINT32)))
        {
            goto done;
        }
        if (sizeof(RTI_UINT32) > space_left)
        {
            OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
            goto done;
        }
        OSAPI_Memory_copy(log->write_pointer,value,sizeof(RTI_UINT32));
        log->write_pointer += sizeof(RTI_UINT32);
        break;
    case OSAPI_LOGTYPE_STRING:
        if (!OSAPI_Log_entry_copy_string(log,name,0))
        {
            goto done;
        }
        if (!OSAPI_Log_entry_copy_string(log,value,sizeof(RTI_INT32)))
        {
            goto done;
        }
        break;
    case OSAPI_LOGTYPE_POINTER:
        if (!OSAPI_Log_entry_copy_string(log,name,sizeof(void*)))
        {
            goto done;
        }
        if (sizeof(void*)*2 > space_left)
        {
            OSAPI_LOG_HEADER_SET_E(log->current_entry->error_code);
            goto done;
        }
        log->write_pointer += OSAPI_Log_ptr_diff(log->write_pointer,0,sizeof(void*));
        OSAPI_Memory_copy(log->write_pointer,&value,sizeof(void*));
        log->write_pointer += sizeof(void*);
        break;
    default:
        break;
    }

done:

    /* If the message is full, ignore it */
    if (OSAPI_LOG_HEADER_GET_E(log->current_entry->error_code) || log->buffer_full)
    {
        OSAPI_LOG_HEADER_SET_F(log->current_entry->error_code);
    }

    if (!is_final)
    {
        return;
    }

    if (OSAPI_LOG_HEADER_GET_E(log->current_entry->error_code) || log->buffer_full)
    {
        min_entry = *log->current_entry;
        user_log_entry = &min_entry;
        /* Make sure that extra data is not parsed by a log-handler */
        OSAPI_LOG_HEADER_CLR_X(user_log_entry->error_code);
    }
    else
    {
        user_log_entry = log->current_entry;
    }

    log->current_entry = NULL;


    OSAPI_Log_call_user_handlers(user_log_entry);
    
    retval = OSAPI_Log_unlock();
    IGNORE_RETVAL(retval);

#undef space_left
}

void
OSAPI_Log_entry_add_int(const char *name,RTI_INT32 value,RTI_BOOL is_final)
{
    OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_INTEGER,name,(const void*)&value,is_final);
}

void
OSAPI_Log_entry_add_uint(const char *name,RTI_UINT32 value,RTI_BOOL is_final)
{
    OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_UINTEGER,name,(const void*)&value,is_final);
}

void
OSAPI_Log_entry_add_int_hex(const char *name,RTI_INT32 value,RTI_BOOL is_final)
{
    OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_HEX,name,(const void*)&value,is_final);
}

void
OSAPI_Log_entry_add_string(const char *name,const char *value,RTI_BOOL is_final)
{
    /* Allow a string NULL pointer to be logged, but logged those as pointers
     * with a NULL value instead.
     */
    if (value == NULL)
    {
        OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_POINTER,name,(const void*)value,is_final);
    }
    else
    {
        OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_STRING,name,(const void*)value,is_final);
    }
}

void
OSAPI_Log_entry_add_pointer(const char *name,const void *value,RTI_BOOL is_final)
{
    OSAPI_Log_entry_add_data(OSAPI_LOGTYPE_POINTER,name,value,is_final);
}

void
OSAPI_Log_entry_add_1int_hex(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,const char *name,
                        RTI_INT32 value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_int_hex(name,value,RTI_TRUE);
}

void
OSAPI_Log_entry_add_1int(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,const char *name,
                        RTI_INT32 value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_int(name,value,RTI_TRUE);
}

void
OSAPI_Log_entry_add_1uint(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,const char *name,
                        RTI_UINT32 value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_uint(name,value,RTI_TRUE);
}

void
OSAPI_Log_entry_add_2int(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,
                        const char *name1,RTI_INT32 value1,
                        const char *name2,RTI_INT32 value2)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_int(name1,value1,RTI_FALSE);
    OSAPI_Log_entry_add_int(name2,value2,RTI_TRUE);
}

void
OSAPI_Log_entry_add_3int(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                        const char *const module, const char *const file,
                        const char *const func, RTI_INT32 line,
                        const char *name1,RTI_INT32 value1,
                        const char *name2,RTI_INT32 value2,
                        const char *name3,RTI_INT32 value3)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_int(name1,value1,RTI_FALSE);
    OSAPI_Log_entry_add_int(name2,value2,RTI_FALSE);
    OSAPI_Log_entry_add_int(name3,value3,RTI_TRUE);
}

void
OSAPI_Log_entry_add_4int(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                         const char *const module, const char *const file,
                         const char *const func, RTI_INT32 line,
                         const char *name1,RTI_INT32 value1,
                         const char *name2,RTI_INT32 value2,
                         const char *name3,RTI_INT32 value3,
                         const char *name4,RTI_INT32 value4)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_int(name1,value1,RTI_FALSE);
    OSAPI_Log_entry_add_int(name2,value2,RTI_FALSE);
    OSAPI_Log_entry_add_int(name3,value3,RTI_FALSE);
    OSAPI_Log_entry_add_int(name4,value4,RTI_TRUE);
}

void
OSAPI_Log_entry_add_1string(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,const char *name,
                            const char* value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    if (value != NULL)
    {
        OSAPI_Log_entry_add_string(name,value,RTI_TRUE);
    }
    else
    {
        OSAPI_Log_entry_add_string(name,"NULL",RTI_TRUE);
    }
}

void
OSAPI_Log_entry_add_2string(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *name1,const char* value1,
                            const char *name2,const char* value2)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    if (value1 != NULL)
    {
        if (value2 == NULL)
        {
            OSAPI_Log_entry_add_string(name1,value1,RTI_TRUE);
        }
        else
        {
            OSAPI_Log_entry_add_string(name1,value1,RTI_FALSE);
            OSAPI_Log_entry_add_string(name2,value2,RTI_TRUE);
        }
    }
    else
    {
        OSAPI_Log_entry_add_string(name2,value2,RTI_TRUE);
    }

    
}

void
OSAPI_Log_entry_add_1string_1int(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,
                            const char *s_name,const char* s_value,
                            const char *i_name,RTI_INT32 i_value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_string(s_name,s_value,RTI_FALSE);
    OSAPI_Log_entry_add_int(i_name,i_value,RTI_TRUE);
}


void
OSAPI_Log_entry_add_1pointer(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                            const char *const module, const char *const file,
                            const char *const func, RTI_INT32 line,const char *name,
                            const void* value)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_FALSE);
    OSAPI_Log_entry_add_pointer(name,value,RTI_TRUE);
}

void
OSAPI_Log_entry_add(OSAPI_LogKind_T kind, RTI_UINT32 error_code,
                       const char *const module, const char *const file,
                       const char *const func, RTI_INT32 line)
{
    OSAPI_Log_entry_create(kind,error_code,module,file,func,line,RTI_TRUE);
}

RTI_BOOL
OSAPI_Log_entry_get_data(OSAPI_LogEntry_T *log_entry,char **data_ptr,
                         OSAPI_LogType_T *type, const char **name,
                         const void **value,RTI_BOOL *is_final)
{
    RTI_UINT32 status;
    RTI_UINT32 dstatus;
    char *ldata_ptr = *data_ptr;

    if (OSAPI_LOG_HEADER_GET_F(log_entry->error_code) == 1)
    {
        return RTI_FALSE;
    }

    if (ldata_ptr == NULL)
    {
        ldata_ptr = (char*)&log_entry[1];
        OSAPI_Memory_copy(&status,ldata_ptr,sizeof(RTI_UINT32));
        ldata_ptr += (status & 0xffff);
    }

    OSAPI_Memory_copy(&dstatus,ldata_ptr,sizeof(RTI_UINT32));

    if (OSAPI_LOGDATA_GET_F(dstatus) == 1)
    {
        *is_final = RTI_TRUE;
    }

    ldata_ptr += sizeof(RTI_INT32);
    *name = ldata_ptr;

    *type = (OSAPI_LogType_T)OSAPI_LOGDATA_GET_TYPE(dstatus);
    switch (*type)
    {
    case OSAPI_LOGTYPE_HEX:
    case OSAPI_LOGTYPE_INTEGER:
        ldata_ptr += OSAPI_Log_ptr_diff(ldata_ptr,(OSAPI_String_length(ldata_ptr) + 1),sizeof(RTI_INT32));
        OSAPI_Memory_copy(((RTI_INT32**)value),&ldata_ptr,sizeof(RTI_INT32*));
        ldata_ptr += sizeof(RTI_INT32);
        break;
    case OSAPI_LOGTYPE_UINTEGER:
        ldata_ptr += OSAPI_Log_ptr_diff(ldata_ptr,(OSAPI_String_length(ldata_ptr) + 1),sizeof(RTI_UINT32));
        OSAPI_Memory_copy(((RTI_INT32**)value),&ldata_ptr,sizeof(RTI_UINT32*));
        ldata_ptr += sizeof(RTI_UINT32);
        break;
    case OSAPI_LOGTYPE_STRING:
        ldata_ptr += OSAPI_Log_ptr_diff(ldata_ptr,(OSAPI_String_length(ldata_ptr) + 1),0);
        *((char **)value) = (char*)ldata_ptr;
        ldata_ptr += OSAPI_Log_ptr_diff(ldata_ptr,(OSAPI_String_length(ldata_ptr) + 1),sizeof(RTI_INT32));
        break;
    case OSAPI_LOGTYPE_POINTER:
        ldata_ptr += OSAPI_Log_ptr_diff(ldata_ptr,(OSAPI_String_length(ldata_ptr) + 1),sizeof(void*));
        OSAPI_Memory_copy(((char**)value),&ldata_ptr,sizeof(void*));
        ldata_ptr += sizeof(void*);
        break;
    default:
        break;
    }

    *data_ptr = ldata_ptr;

    return RTI_TRUE;
}

#endif /* OSAPI_ENABLE_LOG */

/*ci
 * \brief Convert an integer to a string in decimal format
 * *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    d           The digit to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         is returned.
 */
RTI_SIZE_T
OSAPI_Log_itoa(char *buffer,RTI_SIZE_T max_length,RTI_INT32 d)
{
    /* This is sufficient to hold a 32 bit signed value */
#define OSAPI_LOG_MAX_CONVERTED_DIGITS 12
    char *bptr;
    RTI_INT32 r;
    const char digit[] = "0123456789";
    char converted[OSAPI_LOG_MAX_CONVERTED_DIGITS];
    char *result;
    RTI_SIZE_T rlen;

    bptr = converted;
    *bptr = 0;

    if (d < 0)
    {
        *bptr = '-';
        bptr++;
        *bptr = 0;

        if (d == INT_MIN)
        {
            *bptr = '8';
            bptr++;
            *bptr = 0;
            d = d / 10;
        }
        d = -d;
    }

    do
    {
        r = d % 10;
        d = d / 10;
        *bptr = digit[r];
        bptr++;
        *bptr = 0;
    } while (d > 0);

    rlen = (RTI_SIZE_T)(bptr - converted);
    if (rlen >= max_length)
    {
        return max_length;
    }

    bptr = converted + rlen - 1;
    result = buffer;
    if (converted[0] == '-')
    {
        *(result++) = '-';
    }

    while ((bptr >= converted) && (*bptr != '-'))
    {
        *(result++) = *(bptr--);
    }

    *result = 0;

    return rlen;
#undef OSAPI_LOG_MAX_CONVERTED_DIGITS
}

#ifndef RTI_CERT
/*ci
 * \brief Return pointer to log buffer
 *
 * \details
 *
 * This function is used for testing purposes and should not be called
 * in normal operation. Thus, it is not part of the public API.
 *
 * \return pointer to log-buffer
 */
#if OSAPI_ENABLE_LOG
void*
OSAPI_Log_get_log_buffer(void)
{
    return OSAPI_LogImpl_gv_Singleton.buffer_start;
}

/*ci
 * \brief Pointer to log buffer
 *
 * \details
 *
 * This function is used for testing purposes and should not be called
 * in normal operation. Thus, it is not part of the public API.
 *
 */
void
OSAPI_Log_set_log_buffer(void *buffer)
{
    OSAPI_LogImpl_gv_Singleton.buffer_start = buffer;
}

#endif
#endif
