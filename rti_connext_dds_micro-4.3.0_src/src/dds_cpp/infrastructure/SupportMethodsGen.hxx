/*
 SupportMethodsGen.hxx
 
 Copyright (c) 2014-2025 Real-Time Innovations, Inc. All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
07apr2016,tk MICRO-1541 Fixed assignment operator issues in C++
20sep2014,as  Created
===================================================================== */

#define concatenate(A, B)   A ## B

#ifndef T_RetVal
#define T_RetVal                DDS_ReturnCode_t
#define T_RetVal_UNSUPPORTED    DDS_RETCODE_UNSUPPORTED
#else
#if !defined(T_RetVal_UNSUPPORTED)
#error "T_RetVal_UNSUPPORTED undefined"
#endif
#endif

#ifndef T_initialize
#define _initialize(T) concatenate(T, _initialize)
#define T_initialize _initialize(T)
#endif

#ifndef T_copy
#define _copy(T) concatenate(T, _copy)
#define T_copy _copy(T)
#endif

#ifndef T_finalize
#define _finalize(T) concatenate(T, _finalize)
#define T_finalize _finalize(T)
#endif

#ifndef T_is_equal
#define _is_equal(T) concatenate(T, _is_equal)
#define T_is_equal _is_equal(T)
#endif

#ifdef RTI_CERT
#define T_EMPTY_IMPL
#endif

#ifndef T_constructor
T::T()
{
    T_initialize(this);
}

T::T(const T& from)
{
    RTI_BOOL rc;
    T_initialize(this);
     rc = T_copy(this, &from);
    IGNORE_RETVAL(rc);
}
#endif

T::~T()
{
    /* coverity[check_return : FALSE] */
    T_finalize(this);
}

#ifdef T_generate_extended
T_RetVal
T::copy(const T& from)
{
#ifndef T_EMPTY_IMPL
#if T_Retval_is_bool
    return T_copy(this, &from) ? true : false;
#else
    return T_copy(this, &from);
#endif
#else
    UNUSED_ARG(from);
    return T_RetVal_UNSUPPORTED;
#endif
}

#ifdef T_generate_assignment
T&
T::operator=(const T& from)
{
    IGNORE_RETVAL(this->copy(from));
    return *this;
}
#endif /* T_generate_assignment*/
#endif /* T_generate_extended*/


bool
T::operator==(const T& other)
{
#ifndef T_EMPTY_IMPL
    return T_is_equal(this, &other) ? true : false;
#else
    UNUSED_ARG(other);
    return false;
#endif
}

bool
T::operator!=(const T& other)
{
#ifndef T_EMPTY_IMPL
    return T_is_equal(this, &other) ? false : true;
#else
    UNUSED_ARG(other);
    return false;
#endif
}

#undef T_initialize
#undef _initialize
#undef T_finalize
#undef _finalize
#undef T_copy
#undef _copy
#undef T_is_equal
#undef _is_equal
#undef concatenate
#undef T_generate_extended
#undef T_generate_assignment
#undef T_generate_extended_copy
#undef T_RetVal
#undef T_RetVal_UNSUPPORTED
#undef T_EMPTY_IMPL
#undef T_Retval_is_bool
#undef T_constructor
#undef T
