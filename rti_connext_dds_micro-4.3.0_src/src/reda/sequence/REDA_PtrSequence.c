/*
 * FILE: REDA_PtrSequence.c - Pointer Sequence implementation
 *
 * Copyright 2017 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------s
 * 12oct2017,am Written
 *
 */

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_ptr_sequence_h
#include "reda/reda_ptr_sequence.h"
#endif


/*** SOURCE_BEGIN ***/

#define T REDA_Ptr
#define TSeq REDA_PtrSeq
#define REDA_SEQUENCE_API REDA_SEQUENCE_API_DEFAULT
#include <reda/reda_sequence_defn.h>



