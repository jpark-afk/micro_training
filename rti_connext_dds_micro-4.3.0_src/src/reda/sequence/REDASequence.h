/*
 * FILE: REDASequence.h - Sequence implementation
 *
 * Copyright 2012-2016 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 09aug2012,tk Written
 */
/*ce
 * \file
 */
/*ci \addtogroup REDASequenceClass
 * @{
 */
#ifndef REDASequence_h
#define REDASequence_h

/*ci
 * \brief Concrete implementation of a generic sequence structure
 */
struct REDA_Sequence
{
    /*ci
     * \brief Pointer to memory holding the elements
     */
    void *_contiguous_buffer;

    /*ci
     * \brief The maximum number of elements the sequence can hold
     */
    RTI_INT32 _maximum;

    /*ci
     * \brief The current number of valid elements in the sequence
     */
    RTI_INT32 _length;

    /*ci
     * \brief The size in bytes of each element
     */
    RTI_INT32 _element_size;

    /*ci
     * \brief Token1 associated with the sequence
     */
    void *_token1;

    /*ci
     * \brief Token1 associated with the sequence
     */
    void *_token2;

    /*ci
     * \brief Internal flags to manage the sequence
     */
    RTI_UINT8 _flags;
};

/*ci \brief Return the address of a sequence element in a discontigous sequence
 *
 * \param [in] self - The sequence. Must not be NULL.
 * \param [in] i - The element index. Must be a valid index.
 *
 * \return address of the i-th element.
 */
extern void**
REDA_Sequence_get_address(const struct REDA_Sequence *self,RTI_INT32 i);

#endif

/*ci @} */
