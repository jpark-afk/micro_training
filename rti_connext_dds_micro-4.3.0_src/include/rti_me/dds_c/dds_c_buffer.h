/*
 * FILE: dds_c_buffer.h - Buffer API
 *
 * (c) Copyright, Real-Time Innovations, 2019
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
 * 28jan2019,am Created
 *
 */
/*ci
 * \file
 * \brief Buffer API *
 *  \details
 *  A DDS Buffer encapsulates a pointer to a character array and a length.
 *  No assumptions are made regarding the content of the octet array
 */
/*ci \addtogroup REDABufferClass
 *   @{
 */
#ifndef dds_c_buffer_h
#define dds_c_buffer_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Generic buffer to hold data, described by a pointer to the data, as
 *        well as the length of the data. The length is signed int 
 *        (making it different from REDA_Buffer)
 */
struct DDSCDllExport DDS_Buffer 
{
    /*ci
     * \brief Length of the buffer pointed to by pointer.
     */
    DDS_Long length;

    /*ci
     * \brief Pre-allocated (by the caller) buffer.
     */
    char *pointer;
};

/*ci
 * \brief Set the elements of a REDA_Buffer.
 *
 * \details
 *
 * Initialize a DDS buffer with a pre-allocated buffer. It is safer and
 * more portable than setting the individual fields.
 *
 * @param[in] me      REDA_Buffer structure to initialize
 * @param[in] pointer Underlying memory for the buffer, pre-allocated by the
 *                    caller
 * @param[in] length  The length of the underlying memory for the buffer
 *
*/
DDSCDllExport void
DDS_Buffer_set(struct DDS_Buffer *me,char *pointer,DDS_Long length);

#ifndef RTI_CERT
/*ci
 * \brief Finalize the DDS__Buffer.
 *
 * \details
 * Finalizes the DDS buffer by calling free and setting the length to zero
 *
 * @param[in] me      DDS_Buffer structure to finalize
 *
*/
DDSCDllExport void
DDS_Buffer_finalize(struct DDS_Buffer *me);
#endif /*RTI_CERT*/

/*ci
 * \brief allocate memory for the DDS_Buffer.
 *
 * \details
 * Allocates memory for the buffer. If new length is larger than 
 * the current length, more memory is allocated if possible. If new 
 * length is less than the current length function returns RTI_TRUE
 * without changes to the buffer. 
 *
 * @param[in] me        DDS_Buffer structure to allocate buffer for
 * @param[in] new_length      size of the new buffer
 *
 *\return RTI_TRUE if successful otherwise false.
 * 
*/
DDSCDllExport RTI_BOOL
DDS_Buffer_assert_buffer(struct DDS_Buffer* buf, DDS_Long new_length);

/*ci
 * \brief DDS_Buffer initializer
 *
 * \details
 *
 * DDS_Buffer initializer useful for initialization as indicated by
 * the usage:
 *
 * \code
 * struct DDS_Buffer buf = DDS_BUFFER_INVALID;
 * \endcode
*/

#define DDS_BUFFER_INVALID {0, NULL}

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_buffer_h */

/*ci
 *   @}
 */
