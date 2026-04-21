/*
 * FILE: dds_c_sequence.h - DDS sequence definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2016.
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
 * 10feb2016,tk MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 06oct2015,tk MICRO-1493 Added missing DDS built-in sequences
 * 08jun2012,tk Written
 */
/*ce
 * \file
 * \brief DDS sequence definitions
 */
/*e \dref_SequenceGroupDocs
 */
#ifndef dds_c_sequence_h
#define dds_c_sequence_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Make sure that NDDSUSERDllExport expands to nothing by default */
#ifndef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

/*ci @ingroup DDSSequenceModule
  @brief    Empty Sequence dll export variable

  Since for dds c sequence structure, there are no methods. Therefore, there is
  no need to export anything.
*/
#define DDS_SEQUENCE_DLL_EXPORT

/*i @ingroup DDSSequenceModule

  @brief Declares an abstract \st_interface \p TSeq, for IDL \p
  sequence\_T of type \p T.

   @uses     T      base type
   @defines  TSeq   name of the sequence type to be defined

   Using this macro introduces a class declaration TSeq in the
   enclosing name space.

   <STRONG>Incarnation:</STRONG>

   \em Incarnation is the process of realizing a concrete type
   from the \em generic skeleton defined by TSeq.  Given a user type
   \p Foo, a type \p FooSeq representing the IDL type \p
   sequence\_Foo is incarnated as follows.

   <EM>\em Value-type sequences</EM>

   When the base type \p T is a value-type, \p sequence\_T is declared
   follows.
   \code
      DDS_SEQUENCE(FooSeq, Foo);
   \endcode

   <EM>\em Reference-type sequences</EM>

   When the base type \p Foo is a reference-type, \p sequence\_Foo is
   declared as follows.

   \code
      typedef  Foo*  Foo_ptr;

      DDS_SEQUENCE(FooSeq, Foo_ptr);

   \endcode

   We use a two step process, because in C++
   \code
       const Foo_ptr  =   Foo *const
                 !=   const Foo*
   \endcode
   and we want the \p Foo_ptr semantics for the method:
   \code
       const T& TSeq::operator[]() const
   \endcode

  \hideinitializer
*/

#define DDS_SEQUENCE REDA_DEFINE_SEQUENCE

/*ce \dref_FooSeq_SEQUENCE_INITIALIZER
 */
#define DDS_SEQUENCE_INITIALIZER REDA_DEFINE_EMPTY_SEQUENCE_INITIALIZER

/* ================================================================= */
#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*ce \dref_FooSeq
 */
struct FooSeq
{
    /*e @brief Pointer to array of contiguous or discontiguous data
     *
     */
    Foo *_contiguous_buffer;


    /*e @brief Maximum size of the sequence
     *
     * The allocated length of this sequence. It applies to whichever of
     * the above buffers is non-NULL, if any. If both a NULL, its value
     * must be 0.
     *
     * If _maximum == 0, _owned == true.
     */
    DDS_UnsignedLong _maximum;

    /*e @brief Actual length of the sequence that contains data
     *
     * The current logical length of this seqeunce, i.e. the number of valid
     * elements it contains. It applies to whichever of the above
     * buffers is non-null, if any. If both are NULL, its value must be 0.
     */
    DDS_UnsignedLong _length;

    /*e @brief Size of data element in the sequence
     *
     * Each element in the sequence has this size.
     */
    DDS_Long _element_size;

    /*e @brief Reserved for internal use
     */
    void *_token1;

    /*e @brief Reserved for internal use
     */
    void *_token2;

    /*e @brief The maximum allocation for pointer elements (string,wstring)
     *  Only in debug librarires
     */
    DDS_UnsignedLong max_alloc_size;

    /*e @brief Reserved for internal use
     */
    DDS_Char _flags;
};

/*ce \dref_FooSeq_initialize
 */
DDS_Boolean FooSeq_initialize(struct FooSeq *self);

/*ce \dref_FooSeq_finalize
 */
DDS_Boolean FooSeq_finalize(struct FooSeq *self);

/*ce \dref_FooSeq_get_maximum
 */
DDS_Long FooSeq_get_maximum(const struct FooSeq *self);

/*ce \dref_FooSeq_set_maximum
 */
DDS_Boolean FooSeq_set_maximum(struct FooSeq *self, DDS_Long new_max);

/*ce \dref_FooSeq_set_maximum_w_max
 */
DDS_Boolean FooSeq_set_maximum_w_max(struct FooSeq *self, DDS_Long new_max,DDS_UnsignedLong max_size);

/*ce \dref_FooSeq_get_length
 */
DDS_Long FooSeq_get_length(const struct FooSeq *self);

/*ce \dref_FooSeq_set_length
 */
DDS_Boolean FooSeq_set_length(struct FooSeq *self, DDS_Long new_length);

/*ce \dref_FooSeq_get_reference
 */
Foo *FooSeq_get_reference(const struct FooSeq *self, DDS_Long i);

/*ce \dref_FooSeq_copy
 */
struct FooSeq *FooSeq_copy(struct FooSeq *self, const struct FooSeq *src_seq);

/*ce \dref_FooSeq_copy_w_max
 */
struct FooSeq *FooSeq_copy_w_max(struct FooSeq *self, const struct FooSeq *src_seq,DDS_UnsignedLong max_size);

/*ce \dref_FooSeq_is_equal
 */
DDS_Boolean FooSeq_is_equal(struct FooSeq* self, struct FooSeq* other);

/*ce \dref_FooSeq_loan_contiguous
 */
DDS_Boolean FooSeq_loan_contiguous(struct FooSeq *self, void *buffer,
                                   DDS_Long new_length, DDS_Long new_max);

/*ce \dref_FooSeq_loan_discontiguous
 */
DDS_Boolean FooSeq_loan_discontiguous(struct FooSeq *self, void *buffer,
                                   DDS_Long new_length, DDS_Long new_max);

/*ce \dref_FooSeq_unloan
 */
DDS_Boolean FooSeq_unloan(struct FooSeq *self);

/*ce \dref_FooSeq_has_ownership
 */
DDS_Boolean FooSeq_has_ownership(const struct FooSeq *self);

/*ce \dref_FooSeq_get_contiguous_buffer
 */
Foo* FooSeq_get_contiguous_buffer(struct FooSeq *self);

/*ce \dref_FooSeq_set_contiguous_buffer
 */
DDS_Boolean FooSeq_set_contiguous_buffer(struct FooSeq *self, Foo* buffer);

/*ce \dref_FooSeq_has_discontiguous_buffer
 */
DDS_Boolean FooSeq_has_discontiguous_buffer(const struct FooSeq *self);

/*ce \dref_FooSeq_set_token
 */
DDS_Boolean FooSeq_set_token(const struct FooSeq *self,
                             void *token1, void *token2);

/*ce \dref_FooSeq_get_token
 */
DDS_Boolean FooSeq_get_token(const struct FooSeq *self,
                             void **token1, void **token2);

/*ce \dref_FooSeq_ensure_length
 */
DDS_Boolean FooSeq_ensure_length(struct FooSeq *self,DDS_Long length, DDS_Long max);

/*ce \dref_FooSeq_ensure_length_w_max
 */
DDS_Boolean FooSeq_ensure_length_w_max(struct FooSeq *self,DDS_Long length, DDS_Long max,DDS_UnsignedLong max_size);

/*ce \dref_FooSeq_from_array
 */
DDS_Boolean FooSeq_from_array(struct FooSeq *self, const Foo array[], DDS_Long length);

/*ce \dref_FooSeq_from_array_w_max
 */
DDS_Boolean FooSeq_from_array_w_max(struct FooSeq *self, const Foo array[], DDS_Long length,DDS_UnsignedLong max_size);

/*ce \dref_FooSeq_to_array
 */
DDS_Boolean FooSeq_to_array(struct FooSeq *self, const Foo array[], DDS_Long length);

/*ce \dref_FooSeq_to_array_w_max
 */
DDS_Boolean FooSeq_to_array_w_max(struct FooSeq *self, const Foo array[], RTI_INT32 length,DDS_UnsignedLong max_size);

#endif /* DOXYGEN_DOCUMENTATION_ONLY */

#define T    DDS_Wchar
#define TSeq DDS_WcharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

#define T    DDS_Wstring
#define TSeq DDS_WstringSeq
#define TSeq_isCDRStringType
#define TSeq_isCDRStringType_no_max
#define REDA_SEQUENCE_USER_API
#ifndef RTI_CERT
#define TSeq_is_equal
#endif
#include <reda/reda_sequence_decl.h>

#ifndef REDA_SEQUENCE_API
#define REDA_SEQUENCE_API REDA_SEQUENCE_API_USER_DEFAULT
#endif /* REDA_SEQUENCE_API */

#define DDS_UnsignedLongSeq                             CDR_UnsignedLongSeq
#define DDS_ShortSeq                                    CDR_ShortSeq
#define DDS_BooleanSeq                                  CDR_BooleanSeq
#define DDS_FloatSeq                                    CDR_FloatSeq
#define DDS_LongSeq                                     CDR_LongSeq
#define DDS_DoubleSeq                                   CDR_DoubleSeq
#define DDS_EnumSeq                                     CDR_EnumSeq
#define DDS_LongLongSeq                                 CDR_LongLongSeq
#define DDS_OctetSeq                                    CDR_OctetSeq
#define DDS_UnsignedLongLongSeq                         CDR_UnsignedLongLongSeq
#define DDS_CharSeq                                     CDR_CharSeq
#define DDS_UnsignedShortSeq                            CDR_UnsignedShortSeq
#define DDS_LongDoubleSeq                               CDR_LongDoubleSeq
#define DDS_StringSeq                                   CDR_StringSeq

#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_UNTYPED
#define DDS_StringSeq_initialize                            CDR_StringSeq_initialize
#define DDS_UnsignedLongSeq_initialize                      CDR_UnsignedLongSeq_initialize
#define DDS_ShortSeq_initialize                             CDR_ShortSeq_initialize
#define DDS_BooleanSeq_initialize                           CDR_BooleanSeq_initialize
#define DDS_FloatSeq_initialize                             CDR_FloatSeq_initialize
#define DDS_LongSeq_initialize                              CDR_LongSeq_initialize
#define DDS_DoubleSeq_initialize                            CDR_DoubleSeq_initialize
#define DDS_EnumSeq_initialize                              CDR_EnumSeq_initialize
#define DDS_LongLongSeq_initialize                          CDR_LongLongSeq_initialize
#define DDS_OctetSeq_initialize                             CDR_OctetSeq_initialize
#define DDS_UnsignedLongLongSeq_initialize                  CDR_UnsignedLongLongSeq_initialize
#define DDS_CharSeq_initialize                              CDR_CharSeq_initialize
#define DDS_UnsignedShortSeq_initialize                     CDR_UnsignedShortSeq_initialize
#define DDS_LongDoubleSeq_initialize                        CDR_LongDoubleSeq_initialize

#define DDS_StringSeq_set_length                            CDR_StringSeq_set_length
#define DDS_UnsignedLongSeq_set_length                      CDR_UnsignedLongSeq_set_length
#define DDS_ShortSeq_set_length                             CDR_ShortSeq_set_length
#define DDS_BooleanSeq_set_length                           CDR_BooleanSeq_set_length
#define DDS_FloatSeq_set_length                             CDR_FloatSeq_set_length
#define DDS_LongSeq_set_length                              CDR_LongSeq_set_length
#define DDS_DoubleSeq_set_length                            CDR_DoubleSeq_set_length
#define DDS_EnumSeq_set_length                              CDR_EnumSeq_set_length
#define DDS_LongLongSeq_set_length                          CDR_LongLongSeq_set_length
#define DDS_OctetSeq_set_length                             CDR_OctetSeq_set_length
#define DDS_UnsignedLongLongSeq_set_length                  CDR_UnsignedLongLongSeq_set_length
#define DDS_CharSeq_set_length                              CDR_CharSeq_set_length
#define DDS_UnsignedShortSeq_set_length                     CDR_UnsignedShortSeq_set_length
#define DDS_LongDoubleSeq_set_length                        CDR_LongDoubleSeq_set_length

#define DDS_StringSeq_get_length                            CDR_StringSeq_get_length
#define DDS_UnsignedLongSeq_get_length                      CDR_UnsignedLongSeq_get_length
#define DDS_ShortSeq_get_length                             CDR_ShortSeq_get_length
#define DDS_BooleanSeq_get_length                           CDR_BooleanSeq_get_length
#define DDS_FloatSeq_get_length                             CDR_FloatSeq_get_length
#define DDS_LongSeq_get_length                              CDR_LongSeq_get_length
#define DDS_DoubleSeq_get_length                            CDR_DoubleSeq_get_length
#define DDS_EnumSeq_get_length                              CDR_EnumSeq_get_length
#define DDS_LongLongSeq_get_length                          CDR_LongLongSeq_get_length
#define DDS_OctetSeq_get_length                             CDR_OctetSeq_get_length
#define DDS_UnsignedLongLongSeq_get_length                  CDR_UnsignedLongLongSeq_get_length
#define DDS_CharSeq_get_length                              CDR_CharSeq_get_length
#define DDS_UnsignedShortSeq_get_length                     CDR_UnsignedShortSeq_get_length
#define DDS_LongDoubleSeq_get_length                        CDR_LongDoubleSeq_get_length

#define DDS_StringSeq_get_maximum                           CDR_StringSeq_get_maximum
#define DDS_UnsignedLongSeq_get_maximum                     CDR_UnsignedLongSeq_get_maximum
#define DDS_ShortSeq_get_maximum                            CDR_ShortSeq_get_maximum
#define DDS_BooleanSeq_get_maximum                          CDR_BooleanSeq_get_maximum
#define DDS_FloatSeq_get_maximum                            CDR_FloatSeq_get_maximum
#define DDS_LongSeq_get_maximum                             CDR_LongSeq_get_maximum
#define DDS_DoubleSeq_get_maximum                           CDR_DoubleSeq_get_maximum
#define DDS_EnumSeq_get_maximum                             CDR_EnumSeq_get_maximum
#define DDS_LongLongSeq_get_maximum                         CDR_LongLongSeq_get_maximum
#define DDS_OctetSeq_get_maximum                            CDR_OctetSeq_get_maximum
#define DDS_UnsignedLongLongSeq_get_maximum                 CDR_UnsignedLongLongSeq_get_maximum
#define DDS_CharSeq_get_maximum                             CDR_CharSeq_get_maximum
#define DDS_UnsignedShortSeq_get_maximum                    CDR_UnsignedShortSeq_get_maximum
#define DDS_LongDoubleSeq_get_maximum                       CDR_LongDoubleSeq_get_maximum

#define DDS_StringSeq_set_maximum                           REDA_StringSeq_set_maximum
#define DDS_StringSeq_set_maximum_w_max                      REDA_StringSeq_set_maximum_w_max
#define DDS_UnsignedLongSeq_set_maximum                     CDR_UnsignedLongSeq_set_maximum
#define DDS_ShortSeq_set_maximum                            CDR_ShortSeq_set_maximum
#define DDS_BooleanSeq_set_maximum                          CDR_BooleanSeq_set_maximum
#define DDS_FloatSeq_set_maximum                            CDR_FloatSeq_set_maximum
#define DDS_LongSeq_set_maximum                             CDR_LongSeq_set_maximum
#define DDS_DoubleSeq_set_maximum                           CDR_DoubleSeq_set_maximum
#define DDS_EnumSeq_set_maximum                             CDR_EnumSeq_set_maximum
#define DDS_LongLongSeq_set_maximum                         CDR_LongLongSeq_set_maximum
#define DDS_OctetSeq_set_maximum                            CDR_OctetSeq_set_maximum
#define DDS_UnsignedLongLongSeq_set_maximum                 CDR_UnsignedLongLongSeq_set_maximum
#define DDS_CharSeq_set_maximum                             CDR_CharSeq_set_maximum
#define DDS_UnsignedShortSeq_set_maximum                    CDR_UnsignedShortSeq_set_maximum
#define DDS_LongDoubleSeq_set_maximum                       CDR_LongDoubleSeq_set_maximum

#define DDS_StringSeq_get_reference                         CDR_StringSeq_get_reference
#define DDS_UnsignedLongSeq_get_reference                   CDR_UnsignedLongSeq_get_reference
#define DDS_ShortSeq_get_reference                          CDR_ShortSeq_get_reference
#define DDS_BooleanSeq_get_reference                        CDR_BooleanSeq_get_reference
#define DDS_FloatSeq_get_reference                          CDR_FloatSeq_get_reference
#define DDS_LongSeq_get_reference                           CDR_LongSeq_get_reference
#define DDS_DoubleSeq_get_reference                         CDR_DoubleSeq_get_reference
#define DDS_EnumSeq_get_reference                           CDR_EnumSeq_get_reference
#define DDS_LongLongSeq_get_reference                       CDR_LongLongSeq_get_reference
#define DDS_OctetSeq_get_reference                          CDR_OctetSeq_get_reference
#define DDS_UnsignedLongLongSeq_get_reference               CDR_UnsignedLongLongSeq_get_reference
#define DDS_CharSeq_get_reference                           CDR_CharSeq_get_reference
#define DDS_UnsignedShortSeq_get_reference                  CDR_UnsignedShortSeq_get_reference
#define DDS_LongDoubleSeq_get_reference                     CDR_LongDoubleSeq_get_reference

#ifndef RTI_CERT
#define DDS_StringSeq_finalize                              CDR_StringSeq_finalize
#define DDS_UnsignedLongSeq_finalize                        CDR_UnsignedLongSeq_finalize
#define DDS_ShortSeq_finalize                               CDR_ShortSeq_finalize
#define DDS_BooleanSeq_finalize                             CDR_BooleanSeq_finalize
#define DDS_FloatSeq_finalize                               CDR_FloatSeq_finalize
#define DDS_LongSeq_finalize                                CDR_LongSeq_finalize
#define DDS_DoubleSeq_finalize                              CDR_DoubleSeq_finalize
#define DDS_EnumSeq_finalize                                CDR_EnumSeq_finalize
#define DDS_LongLongSeq_finalize                            CDR_LongLongSeq_finalize
#define DDS_OctetSeq_finalize                               CDR_OctetSeq_finalize
#define DDS_UnsignedLongLongSeq_finalize                    CDR_UnsignedLongLongSeq_finalize
#define DDS_CharSeq_finalize                                CDR_CharSeq_finalize
#define DDS_UnsignedShortSeq_finalize                       CDR_UnsignedShortSeq_finalize
#define DDS_LongDoubleSeq_finalize                          CDR_LongDoubleSeq_finalize
#endif /* RTI_CERT */
#endif /* REDA_SEQUENCE_API_UNTYPED */

/* This one is used internally and must be present */
#define DDS_StringSeq_is_equal                              REDA_StringSeq_is_equal
#define DDS_OctetSeq_is_equal                               CDR_OctetSeq_is_equal
#define DDS_OctetSeq_get_contiguous_buffer                  CDR_OctetSeq_get_contiguous_buffer
#define DDS_OctetSeq_loan_contiguous                        CDR_OctetSeq_loan_contiguous

#define DDS_StringSeq_copy                                  REDA_StringSeq_copy
#define DDS_StringSeq_copy_w_max                            REDA_StringSeq_copy_w_max
#define DDS_UnsignedLongSeq_copy                            CDR_UnsignedLongSeq_copy
#define DDS_ShortSeq_copy                                   CDR_ShortSeq_copy
#define DDS_BooleanSeq_copy                                 CDR_BooleanSeq_copy
#define DDS_FloatSeq_copy                                   CDR_FloatSeq_copy
#define DDS_LongSeq_copy                                    CDR_LongSeq_copy
#define DDS_DoubleSeq_copy                                  CDR_DoubleSeq_copy
#define DDS_EnumSeq_copy                                    CDR_EnumSeq_copy
#define DDS_LongLongSeq_copy                                CDR_LongLongSeq_copy
#define DDS_OctetSeq_copy                                   CDR_OctetSeq_copy
#define DDS_UnsignedLongLongSeq_copy                        CDR_UnsignedLongLongSeq_copy
#define DDS_CharSeq_copy                                    CDR_CharSeq_copy
#define DDS_UnsignedShortSeq_copy                           CDR_UnsignedShortSeq_copy
#define DDS_LongDoubleSeq_copy                              CDR_LongDoubleSeq_copy

/* Define the following aliases if the DEFAULT API is > BASIC */
#if REDA_SEQUENCE_API_USER_DEFAULT > REDA_SEQUENCE_API_BASIC

#define DDS_UnsignedLongSeq_is_equal                        CDR_UnsignedLongSeq_is_equal
#define DDS_ShortSeq_is_equal                               CDR_ShortSeq_is_equal
#define DDS_BooleanSeq_is_equal                             CDR_BooleanSeq_is_equal
#define DDS_FloatSeq_is_equal                               CDR_FloatSeq_is_equal
#define DDS_LongSeq_is_equal                                CDR_LongSeq_is_equal
#define DDS_DoubleSeq_is_equal                              CDR_DoubleSeq_is_equal
#define DDS_EnumSeq_is_equal                                CDR_EnumSeq_is_equal
#define DDS_LongLongSeq_is_equal                            CDR_LongLongSeq_is_equal
#define DDS_UnsignedLongLongSeq_is_equal                    CDR_UnsignedLongLongSeq_is_equal
#define DDS_CharSeq_is_equal                                CDR_CharSeq_is_equal
#define DDS_UnsignedShortSeq_is_equal                       CDR_UnsignedShortSeq_is_equal
#define DDS_LongDoubleSeq_is_equal                          CDR_LongDoubleSeq_is_equal
#define DDS_StringSeq_loan_contiguous                       CDR_StringSeq_loan_contiguous
#define DDS_UnsignedLongSeq_loan_contiguous                 CDR_UnsignedLongSeq_loan_contiguous
#define DDS_ShortSeq_loan_contiguous                        CDR_ShortSeq_loan_contiguous
#define DDS_BooleanSeq_loan_contiguous                      CDR_BooleanSeq_loan_contiguous
#define DDS_FloatSeq_loan_contiguous                        CDR_FloatSeq_loan_contiguous
#define DDS_LongSeq_loan_contiguous                         CDR_LongSeq_loan_contiguous
#define DDS_DoubleSeq_loan_contiguous                       CDR_DoubleSeq_loan_contiguous
#define DDS_EnumSeq_loan_contiguous                         CDR_EnumSeq_loan_contiguous
#define DDS_LongLongSeq_loan_contiguous                     CDR_LongLongSeq_loan_contiguous
#define DDS_UnsignedLongLongSeq_loan_contiguous             CDR_UnsignedLongLongSeq_loan_contiguous
#define DDS_CharSeq_loan_contiguous                         CDR_CharSeq_loan_contiguous
#define DDS_UnsignedShortSeq_loan_contiguous                CDR_UnsignedShortSeq_loan_contiguous
#define DDS_LongDoubleSeq_loan_contiguous                   CDR_LongDoubleSeq_loan_contiguous

#define DDS_StringSeq_loan_discontiguous                    CDR_StringSeq_loan_discontiguous
#define DDS_UnsignedLongSeq_loan_discontiguous              CDR_UnsignedLongSeq_loan_discontiguous
#define DDS_ShortSeq_loan_discontiguous                     CDR_ShortSeq_loan_discontiguous
#define DDS_BooleanSeq_loan_discontiguous                   CDR_BooleanSeq_loan_discontiguous
#define DDS_FloatSeq_loan_discontiguous                     CDR_FloatSeq_loan_discontiguous
#define DDS_LongSeq_loan_discontiguous                      CDR_LongSeq_loan_discontiguous
#define DDS_DoubleSeq_loan_discontiguous                    CDR_DoubleSeq_loan_discontiguous
#define DDS_EnumSeq_loan_discontiguous                      CDR_EnumSeq_loan_discontiguous
#define DDS_LongLongSeq_loan_discontiguous                  CDR_LongLongSeq_loan_discontiguous
#define DDS_OctetSeq_loan_discontiguous                     CDR_OctetSeq_loan_discontiguous
#define DDS_UnsignedLongLongSeq_loan_discontiguous          CDR_UnsignedLongLongSeq_loan_discontiguous
#define DDS_CharSeq_loan_discontiguous                      CDR_CharSeq_loan_discontiguous
#define DDS_UnsignedShortSeq_loan_discontiguous             CDR_UnsignedShortSeq_loan_discontiguous
#define DDS_LongDoubleSeq_loan_discontiguous                CDR_LongDoubleSeq_loan_discontiguous

#define DDS_StringSeq_has_ownership                         CDR_StringSeq_has_ownership
#define DDS_UnsignedLongSeq_has_ownership                   CDR_UnsignedLongSeq_has_ownership
#define DDS_ShortSeq_has_ownership                          CDR_ShortSeq_has_ownership
#define DDS_BooleanSeq_has_ownership                        CDR_BooleanSeq_has_ownership
#define DDS_FloatSeq_has_ownership                          CDR_FloatSeq_has_ownership
#define DDS_LongSeq_has_ownership                           CDR_LongSeq_has_ownership
#define DDS_DoubleSeq_has_ownership                         CDR_DoubleSeq_has_ownership
#define DDS_EnumSeq_has_ownership                           CDR_EnumSeq_has_ownership
#define DDS_LongLongSeq_has_ownership                       CDR_LongLongSeq_has_ownership
#define DDS_OctetSeq_has_ownership                          CDR_OctetSeq_has_ownership
#define DDS_UnsignedLongLongSeq_has_ownership               CDR_UnsignedLongLongSeq_has_ownership
#define DDS_CharSeq_has_ownership                           CDR_CharSeq_has_ownership
#define DDS_UnsignedShortSeq_has_ownership                  CDR_UnsignedShortSeq_has_ownership
#define DDS_LongDoubleSeq_has_ownership                     CDR_LongDoubleSeq_has_ownership

#define DDS_UnsignedLongSeq_has_discontiguous_buffer        CDR_UnsignedLongSeq_has_discontiguous_buffer
#define DDS_StringSeq_has_discontiguous_buffer              CDR_StringSeq_has_discontiougous_buffer
#define DDS_ShortSeq_has_discontiguous_buffer               CDR_ShortSeq_has_discontiguous_buffer
#define DDS_BooleanSeq_has_discontiguous_buffer             CDR_BooleanSeq_has_discontiguous_buffer
#define DDS_FloatSeq_has_discontiguous_buffer               CDR_FloatSeq_has_discontiguous_buffer
#define DDS_LongSeq_has_discontiguous_buffer                CDR_LongSeq_has_discontiguous_buffer
#define DDS_DoubleSeq_has_discontiguous_buffer              CDR_DoubleSeq_has_discontiguous_buffer
#define DDS_EnumSeq_has_discontiguous_buffer                CDR_EnumSeq_has_discontiguous_buffer
#define DDS_LongLongSeq_has_discontiguous_buffer            CDR_LongLongSeq_has_discontiguous_buffer
#define DDS_OctetSeq_has_discontiguous_buffer               CDR_OctetSeq_has_discontiguous_buffer
#define DDS_UnsignedLongLongSeq_has_discontiguous_buffer    CDR_UnsignedLongLongSeq_has_discontiguous_buffer
#define DDS_CharSeq_has_discontiguous_buffer                CDR_CharSeq_has_discontiguous_buffer
#define DDS_UnsignedShortSeq_has_discontiguous_buffer       CDR_UnsignedShortSeq_has_discontiguous_buffer
#define DDS_LongDoubleSeq_has_discontiguous_buffer          CDR_LongDoubleSeq_has_discontiguous_buffer

#define DDS_StringSeq_get_contiguous_buffer                 CDR_StringSeq_get_contiguous_buffer
#define DDS_UnsignedLongSeq_get_contiguous_buffer           CDR_UnsignedLongSeq_get_contiguous_buffer
#define DDS_ShortSeq_get_contiguous_buffer                  CDR_ShortSeq_get_contiguous_buffer
#define DDS_BooleanSeq_get_contiguous_buffer                CDR_BooleanSeq_get_contiguous_buffer
#define DDS_FloatSeq_get_contiguous_buffer                  CDR_FloatSeq_get_contiguous_buffer
#define DDS_LongSeq_get_contiguous_buffer                   CDR_LongSeq_get_contiguous_buffer
#define DDS_DoubleSeq_get_contiguous_buffer                 CDR_DoubleSeq_get_contiguous_buffer
#define DDS_EnumSeq_get_contiguous_buffer                   CDR_EnumSeq_get_contiguous_buffer
#define DDS_LongLongSeq_get_contiguous_buffer               CDR_LongLongSeq_get_contiguous_buffer
#define DDS_UnsignedLongLongSeq_get_contiguous_buffer       CDR_UnsignedLongLongSeq_get_contiguous_buffer
#define DDS_CharSeq_get_contiguous_buffer                   CDR_CharSeq_get_contiguous_buffer
#define DDS_UnsignedShortSeq_get_contiguous_buffer          CDR_UnsignedShortSeq_get_contiguous_buffer
#define DDS_LongDoubleSeq_get_contiguous_buffer             CDR_LongDoubleSeq_get_contiguous_buffer

#define DDS_StringSeq_set_contiguous_buffer                 CDR_StringSeq_set_contiguous_buffer
#define DDS_UnsignedLongSeq_set_contiguous_buffer           CDR_UnsignedLongSeq_set_contiguous_buffer
#define DDS_ShortSeq_set_contiguous_buffer                  CDR_ShortSeq_set_contiguous_buffer
#define DDS_BooleanSeq_set_contiguous_buffer                CDR_BooleanSeq_set_contiguous_buffer
#define DDS_FloatSeq_set_contiguous_buffer                  CDR_FloatSeq_set_contiguous_buffer
#define DDS_LongSeq_set_contiguous_buffer                   CDR_LongSeq_set_contiguous_buffer
#define DDS_DoubleSeq_set_contiguous_buffer                 CDR_DoubleSeq_set_contiguous_buffer
#define DDS_EnumSeq_set_contiguous_buffer                   CDR_EnumSeq_set_contiguous_buffer
#define DDS_LongLongSeq_set_contiguous_buffer               CDR_LongLongSeq_set_contiguous_buffer
#define DDS_OctetSeq_set_contiguous_buffer                  CDR_OctetSeq_set_contiguous_buffer
#define DDS_UnsignedLongLongSeq_set_contiguous_buffer       CDR_UnsignedLongLongSeq_set_contiguous_buffer
#define DDS_CharSeq_set_contiguous_buffer                   CDR_CharSeq_set_contiguous_buffer
#define DDS_UnsignedShortSeq_set_contiguous_buffer          CDR_UnsignedShortSeq_set_contiguous_buffer
#define DDS_LongDoubleSeq_set_contiguous_buffer             CDR_LongDoubleSeq_set_contiguous_buffer

#define DDS_StringSeq_unloan                                CDR_StringSeq_unloan
#define DDS_UnsignedLongSeq_unloan                          CDR_UnsignedLongSeq_unloan
#define DDS_ShortSeq_unloan                                 CDR_ShortSeq_unloan
#define DDS_BooleanSeq_unloan                               CDR_BooleanSeq_unloan
#define DDS_FloatSeq_unloan                                 CDR_FloatSeq_unloan
#define DDS_LongSeq_unloan                                  CDR_LongSeq_unloan
#define DDS_DoubleSeq_unloan                                CDR_DoubleSeq_unloan
#define DDS_EnumSeq_unloan                                  CDR_EnumSeq_unloan
#define DDS_LongLongSeq_unloan                              CDR_LongLongSeq_unloan
#define DDS_OctetSeq_unloan                                 CDR_OctetSeq_unloan
#define DDS_UnsignedLongLongSeq_unloan                      CDR_UnsignedLongLongSeq_unloan
#define DDS_CharSeq_unloan                                  CDR_CharSeq_unloan
#define DDS_UnsignedShortSeq_unloan                         CDR_UnsignedShortSeq_unloan
#define DDS_LongDoubleSeq_unloan                            CDR_LongDoubleSeq_unloan

#define DDS_StringSeq_ensure_length                         REDA_StringSeq_ensure_length
#define DDS_StringSeq_ensure_length_w_max                   REDA_StringSeq_ensure_length_w_max
#define DDS_UnsignedLongSeq_ensure_length                   CDR_UnsignedLongSeq_ensure_length
#define DDS_ShortSeq_ensure_length                          CDR_ShortSeq_ensure_length
#define DDS_BooleanSeq_ensure_length                        CDR_BooleanSeq_ensure_length
#define DDS_FloatSeq_ensure_length                          CDR_FloatSeq_ensure_length
#define DDS_LongSeq_ensure_length                           CDR_LongSeq_ensure_length
#define DDS_DoubleSeq_ensure_length                         CDR_DoubleSeq_ensure_length
#define DDS_EnumSeq_ensure_length                           CDR_EnumSeq_ensure_length
#define DDS_LongLongSeq_ensure_length                       CDR_LongLongSeq_ensure_length
#define DDS_OctetSeq_ensure_length                          CDR_OctetSeq_ensure_length
#define DDS_UnsignedLongLongSeq_ensure_length               CDR_UnsignedLongLongSeq_ensure_length
#define DDS_CharSeq_ensure_length                           CDR_CharSeq_ensure_length
#define DDS_UnsignedShortSeq_ensure_length                  CDR_UnsignedShortSeq_ensure_length
#define DDS_LongDoubleSeq_ensure_length                     CDR_LongDoubleSeq_ensure_length

#define DDS_StringSeq_to_array                              REDA_StringSeq_to_array
#define DDS_StringSeq_to_array_w_max                        REDA_StringSeq_to_array_w_max
#define DDS_UnsignedLongSeq_to_array                        CDR_UnsignedLongSeq_to_array
#define DDS_ShortSeq_to_array                               CDR_ShortSeq_to_array
#define DDS_BooleanSeq_to_array                             CDR_BooleanSeq_to_array
#define DDS_FloatSeq_to_array                               CDR_FloatSeq_to_array
#define DDS_LongSeq_to_array                                CDR_LongSeq_to_array
#define DDS_DoubleSeq_to_array                              CDR_DoubleSeq_to_array
#define DDS_EnumSeq_to_array                                CDR_EnumSeq_to_array
#define DDS_LongLongSeq_to_array                            CDR_LongLongSeq_to_array
#define DDS_OctetSeq_to_array                               CDR_OctetSeq_to_array
#define DDS_UnsignedLongLongSeq_to_array                    CDR_UnsignedLongLongSeq_to_array
#define DDS_CharSeq_to_array                                CDR_CharSeq_to_array
#define DDS_UnsignedShortSeq_to_array                       CDR_UnsignedShortSeq_to_array
#define DDS_LongDoubleSeq_to_array                          CDR_LongDoubleSeq_to_array

#define DDS_StringSeq_from_array                            REDA_StringSeq_from_array
#define DDS_StringSeq_from_array_w_max                      REDA_StringSeq_from_array_w_max
#define DDS_UnsignedLongSeq_from_array                      CDR_UnsignedLongSeq_from_array
#define DDS_ShortSeq_from_array                             CDR_ShortSeq_from_array
#define DDS_BooleanSeq_from_array                           CDR_BooleanSeq_from_array
#define DDS_FloatSeq_from_array                             CDR_FloatSeq_from_array
#define DDS_LongSeq_from_array                              CDR_LongSeq_from_array
#define DDS_DoubleSeq_from_array                            CDR_DoubleSeq_from_array
#define DDS_EnumSeq_from_array                              CDR_EnumSeq_from_array
#define DDS_LongLongSeq_from_array                          CDR_LongLongSeq_from_array
#define DDS_OctetSeq_from_array                             CDR_OctetSeq_from_array
#define DDS_UnsignedLongLongSeq_from_array                  CDR_UnsignedLongLongSeq_from_array
#define DDS_CharSeq_from_array                              CDR_CharSeq_from_array
#define DDS_UnsignedShortSeq_from_array                     CDR_UnsignedShortSeq_from_array
#define DDS_LongDoubleSeq_from_array                        CDR_LongDoubleSeq_from_array

#endif /* REDA_SEQUENCE_API > REDA_SEQUENCE_API_BASIC */

#if REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL
#error "REDA_SEQUENCE_API_UNTYPED <= REDA_SEQUENCE_API <= REDA_SEQUENCE_API_FULL"
#endif /* REDA_SEQUENCE_API > REDA_SEQUENCE_API_FULL */

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_sequence_h */
