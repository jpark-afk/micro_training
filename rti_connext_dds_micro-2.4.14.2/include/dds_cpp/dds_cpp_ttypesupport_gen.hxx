/* 

 (c) Copyright, Real-Time Innovations, 2013-2015
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
--------------------
23jan2016,as  MICRO-1523 Replace RTI_BOOL with bool
19may2015,as  MICRO-1193 Refactoring of Sequence API levels
06jun2014,as  Added compiler error on incorrect use of template header;
              source files do not need to undefine dds_c_ttypesupport_gen_hxx
              before including this header;
              Added TTypeSupport::register_type/unregister_type.
===================================================================== */

#ifndef dds_cpp_ttypesupport_gen_hxx
#define dds_cpp_ttypesupport_gen_hxx

#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif


/* ================================================================== */
/*i
   <<IMPLEMENTATION>> 

   Requires:  TTYPENAME

   Defines:   TTypeSupport
*/
#if defined(TTypeSupport) && defined(TData) 

/* --- Constructor/Destructor ---------------------------------------------- */

TTypeSupport::TTypeSupport() 
{
    /* empty */
}

TTypeSupport::~TTypeSupport() 
{
    /* empty */
}


/* --- Methods ------------------------------------------------------------- */

#define concatenate(A, B)  A ## B

/*i
   TTypeSupport::initialize()

*/
bool TTypeSupport::initialize_data(TData* a_data)
{
#define TType_initialize_c(TData) concatenate(TData, _initialize)
#define TType_initialize TType_initialize_c(TData)

    return (TType_initialize(a_data) == RTI_TRUE);

#undef TType_initialize_c
#undef TType_initialize
}


#ifndef RTI_CERT
/*i
   TTypeSupport::finalize()

*/
bool TTypeSupport::finalize_data(TData* a_data)
{
#define TType_finalize_c(TData) concatenate(TData, _finalize)
#define TType_finalize TType_finalize_c(TData)

    return (TType_finalize(a_data) == RTI_TRUE);

#undef TType_finalize_c
#undef TType_finalize
}
#endif

/*i
   TTypeSupport::create()

*/
TData* TTypeSupport::create_data() 
{
#define TType_create_c(TData) concatenate(TData, _create)
#define TType_create TType_create_c(TData)

    return TType_create();

#undef TType_create_c
#undef TType_create
}


#ifndef RTI_CERT
/*i
   TTypeSupport::delete()

*/
void TTypeSupport::delete_data(TData* a_data) 
{
#define TType_delete_c(TData) concatenate(TData, _delete)
#define TType_delete TType_delete_c(TData)

    TType_delete(a_data);

#undef TType_delete_c
#undef TType_delete
}
#endif


/*i
   TTypeSupport::copy()

*/
bool TTypeSupport::copy_data(TData* a_dst, const TData* a_src)
{
#define TType_copy_c(TData) concatenate(TData, _copy)
#define TType_copy TType_copy_c(TData)

    return (TType_copy(a_dst, a_src) == RTI_TRUE);

#undef TType_copy_c
#undef TType_copy
}


#define TTypePlugin_get_default_type_name_c(TData) concatenate(TData, TypePlugin_get_default_type_name)
#define TTypePlugin_get_default_type_name TTypePlugin_get_default_type_name_c(TData)
#define TTypePlugin_get_c(TData) concatenate(TData, TypePlugin_get)
#define TTypePlugin_get TTypePlugin_get_c(TData)

DDS_ReturnCode_t
TTypeSupport::register_type(
                    DDSDomainParticipant* participant,
                    const char* type_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (participant == NULL)
    {
        goto done;
    }

    if (type_name == NULL)
    {
        type_name = TTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    retcode = participant->register_type(
                type_name,
                TTypePlugin_get());

    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

DDS_ReturnCode_t
TTypeSupport::unregister_type(
                    DDSDomainParticipant* participant,
                    const char* type_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (participant == NULL)
    {
        goto done;
    }

    if (type_name == NULL)
    {
        type_name = TTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    if (TTypePlugin_get() !=
            participant->unregister_type(type_name))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

#undef TTypePlugin_get_default_type_name_c
#undef TTypePlugin_get_default_type_name
#undef TTypePlugin_get_c
#undef TTypePlugin_get

/*i
   TTypeSupport::get_type_name()

*/
const char*
TTypeSupport::get_type_name()
{
    return TTYPENAME;
}

#undef concatenate

#else
#error "Incorrect use of dds_cpp_ttypesupport_gen.hxx: TTypeSupport, and TData must be defined"
#endif

#undef dds_cpp_ttypesupport_gen_hxx
#endif /* dds_cpp_ttypesupport_gen_hxx */


