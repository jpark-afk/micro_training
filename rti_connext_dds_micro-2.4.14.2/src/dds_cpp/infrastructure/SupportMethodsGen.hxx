/*
 SupportMethods.hxx
 
 (c) Copyright, Real-Time Innovations, Sep 20, 2014-2016.
 All rights reserved.
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

T::T()
{
    T_initialize(this);
}

T::~T()
{
#ifndef T_EMPTY_IMPL
    T_finalize(this);
#endif
}

#ifdef T_generate_extended
DDS_ReturnCode_t
T::copy(const T& from)
{
#ifndef T_EMPTY_IMPL
    return T_copy(this, &from);
#else
    UNUSED_ARG(from);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

T::T(const T& from)
{
#ifndef T_EMPTY_IMPL
    DDS_ReturnCode_t retcode;
    T_initialize(this);
    retcode = T_copy(this, &from);
    IGNORE_RETVAL(retcode);

#else
    UNUSED_ARG(from);
#endif
}

#endif

#ifdef T_generate_extended_copy
DDS_ReturnCode_t
T::copy(const T& from)
{
#ifndef T_EMPTY_IMPL
    return T_copy(this, &from) == DDS_BOOLEAN_TRUE ? DDS_RETCODE_OK : DDS_RETCODE_ERROR;
#else
    UNUSED_ARG(from);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}
#endif

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

T&
T::operator=(const T& from)
{
#ifndef T_EMPTY_IMPL
    if (T_is_equal(this, &from))
    {
        return *this;
    }
    else
    {
        DDS_ReturnCode_t retcode;

        retcode = this->copy(from);
        IGNORE_RETVAL(retcode);

        return *this;
    }
#else
    UNUSED_ARG(from);
    return *this;
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
#undef T_EMPTY_IMPL
