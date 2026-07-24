/*
 * FILE: reda_buffer.h - Buffer API
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015
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
 * 22sep2008,tk Created
 *
 */
/*ci
 * \file
 * \defgroup REDABufferClass REDA Buffer
 * \ingroup REDAModule
 * \brief Buffer API
 *
 *  \details
 *  A REDA Buffer encapsulates a pointer to a character array and a length.
 *  No assumptions are made regarding the content of the octet array
 */
/*ci \addtogroup REDABufferClass
 *   @{
 */
#ifndef reda_buffer_h
#define reda_buffer_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif
#ifndef osapi_hash_h
#include "osapi/osapi_hash.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Generic buffer to hold data, described by a pointer to the data, as
 *        well as the length of the data.
 */
#define REDA_Buffer OSAPI_Buffer

/*ci
 * \brief Set the elements of a REDA_Buffer.
 *
 * \details
 *
 * Initialize a REDA buffer with a pre-allocated buffer. It is safer and
 * more portable than setting the individual fields.
 *
 * @param[in] me      REDA_Buffer structure to initialize
 * @param[in] pointer Underlying memory for the buffer, pre-allocated by the
 *                    caller
 * @param[in] length  The length of the underlying memory for the buffer
 *
 * \sa \ref REDA_Buffer_get
*/
REDADllExport void
REDA_Buffer_set(struct REDA_Buffer *me,char *pointer,RTI_UINT32 length);

#ifndef RTI_CERT
/*ci
 * \brief Finalize the REDA_Buffer.
 *
 * \details
 * Finalizes the REDA buffer by calling free and setting the length to zero
 *
 * @param[in] me      REDA_Buffer structure to finalize
 *
 * \sa \ref REDA_Buffer_get
*/
REDADllExport void
REDA_Buffer_finalize(struct REDA_Buffer *me);
#endif /*RTI_CERT*/

/*ci
 * \brief allocate memory for the REDA_Buffer.
 *
 * \details
 * Allocates memory for the buffer. If new length is larger than 
 * the current length, more memory is allocated if possible. If new 
 * length is less than the current length function returns RTI_TRUE
 * without changes to the buffer. 
 *
 * @param[in] me        REDA_Buffer structure to allocate buffer for
 * @param[in] new_length      size of the new buffer
 *
 *\return RTI_TRUE if successful otherwise false.
 * 
 * \sa \ref REDA_Buffer_get
*/
REDADllExport RTI_BOOL
REDA_Buffer_assert_buffer(struct REDA_Buffer* buf, RTI_UINT32 new_length);

/*ci
 * \brief REDA_Buffer initializer
 *
 * \details
 *
 * REDA_Buffer initializer useful for initialization as indicated by
 * the usage:
 *
 * \code
 * struct REDA_Buffer buf = REDA_BUFFER_INVALID;
 * \endcode
*/

#define REDA_BUFFER_INVALID {0, NULL}

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_buffer_h */

/*ci
 *   @}
 */
