/*
 * FILE: reda_ptr_sequence.h - VOID pointer sequence API
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
 * 12oct2017,am Written
 */

/*ci
 * \file
 *
 * \brief The REDA void sequence stores a sequence of void* pointers
 * 
 * \ingroup REDAModule
 *
 */

#ifndef reda_ptr_sequence_h
#define reda_ptr_sequence_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#include "reda/reda_sequence.h"

/*ci
 * \brief Alias for void*
 */
typedef void* REDA_Ptr;

#define T REDA_Ptr
#define TSeq REDA_PtrSeq
#define REDA_SEQUENCE_API REDA_SEQUENCE_API_DEFAULT
#include <reda/reda_sequence_decl.h>

#endif /*reda_voidSequence_h*/
