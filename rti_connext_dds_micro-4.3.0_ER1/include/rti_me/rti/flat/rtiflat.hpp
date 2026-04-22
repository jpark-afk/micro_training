/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

// Note: this file shouldn't be included directly. Include the header for the
// corresponding C++ API:
//
//  - Modern C++ API: "rti/topic/flat/FlatData.hpp"
//  - Traditional C++ API: "ndds/ndds_flat_data_cpp.h"

#ifndef RTI_DDS_FLAT_RTIFLAT_HPP_
#define RTI_DDS_FLAT_RTIFLAT_HPP_

/** 
 * @defgroup RTIFlatDataModule FlatData Topic-Types
 * @ingroup DDSTopicModule
 * @brief @st_ext FlatData Language Binding for IDL topic-types
 *
 * @note For a complete description of the FlatData language binding and its
 * benefits, and a tutorial, see the "Sending Large Data" chapter in the 
 * <b>\ndds User's Manual</b>.
 * 
 * @note For buildable <b>code examples</b>, 
 * see https://community.rti.com/kb/flatdata-and-zerocopy-examples.
 * 
 * @note The FlatData language binding is available in the Traditional C++ API 
 * and in the Modern C++ API.
 * 
 * RTI FlatData&trade; is a language binding for IDL types in which the in-memory  
 * representation of a sample matches the wire representation. Therefore, the 
 * cost of serialization/deserialization is zero.
 * 
 * To select FlatData as the language binding of a type, annotate it with 
 * \p \@language_binding(FLAT_DATA) in IDL or apply the attribute 
 * \p languageBinding="flat_data" in XML.
 * 
 * There are some restrictions regarding the kinds of types to which the 
 * FlatData language binding can be applied. 
 * 
 * - For final types, the FlatData language binding can be applied only to 
 * fixed-size types. A fixed-size type is a type whose wire representation 
 * always has the same size. This includes primitive types, arrays of 
 * fixed-size types, and structs containing only members of fixed-size types. 
 * Unions are not fixed-size types.
 * - For mutable types, any member is permitted.
 * - Extensible types cannot be marked as FlatData.
 * 
 * Final types are more efficient, but more restrictive. In general, a good
 * compromise is to define mutable top-level types, but making sure their
 * largest data members are final.
 * 
 * When a type is marked with the FlatData language binding, its mapping into
 * C++ is different than that of a regular type (plain language binding). Rather
 * than a single C++ class with direct access to its members, for a FlatData
 * type \nddsgen generates the following:
 * 
 * \li A \ref RTIFlatSampleModule "Sample", the data in serialized format
 * \li An \ref RTIFlatOffsetsModule "Offset", which allows reading the data 
 *     members of that type inside a Sample, and modifying them without changing
 *     the size
 * \li A \ref RTIFlatBuildersModule "Builder", if the type is mutable, which 
 *     allows creating a variable-size Sample
 * 
 * For example, for the IDL types \ref MyFlatFinal and \ref MyFlatMutable 
 * \nddsgen generates the following C++ types:
 * \li The Sample types \ref MyFlatFinal and \ref MyFlatMutable
 * \li The Offset types MyFlatFinalOffset and MyFlatMutableOffset.
 * \li The Builder type MyFlatMutableBuilder.
 * 
 * @section PublishingFlatData Publishing FlatData
 * The typical way to publish FlatData samples includes the following steps:
 * - Create a \idref_FooDataWriter as you would for a non-FlatData (plain) 
 *   topic-type (see \ref DDSWriterExampleModule_writer_setup).
 * 
 * (For final topic-types such as \ref MyFlatFinal)
 * - Obtain a FlatData sample with \idref_FooDataWriter_get_loan.
 * \code
 * MyFlatFinal *foo_sample = writer.get_loan();
 * \endcode
 * 
 * - Initialize the sample, starting from the \ref rti::flat::Sample::root "root()" offset.
 * \code
 * MyFlatFinalOffset foo_root = foo_sample->root();
 * foo_root.my_primitive(3);
 * auto my_complex_offset = foo_root.my_complex();
 * // ... initialize my_complex_offset 
 * \endcode
 * 
 * (For mutable topic-types such as \ref MyFlatMutable)
 * - Obtain a \ref RTIFlatBuildersModule "Builder" to create a variable-size 
 *   sample with rti::flat::build_data().
 * \code
 * MyFlatMutableBuilder foo_builder = rti::flat::build_data(writer);
 * \endcode
 * 
 * - Use this builder (and possibly nested member builders) to initialize the
 *   members.
 * \code
 * foo_builder.add_my_primitive(3);
 * auto my_mutable_builder = foo_builder.build_my_mutable();
 * // ... build 'my_mutable' (a member with a mutable type)
 * my_mutable_builder.finish(); // completes a member
 * auto my_final_offset = foo_builder.add_my_final();
 * // ... initialize 'my_final' (a member with a final type)
 * \endcode
 * 
 * - Obtain the completed sample with MyFlatMutableBuilder::finish_sample(). After
 * that, the builder is no longer usable, and the sample size cannot change.
 * \code
 * MyFlatMutable *foo_sample = foo_builder.finish_sample();
 * \endcode
 * - Optionally, it is possible to change the values of the sample accessing
 * its root(), as long as the size doesn't change. For example, if the sample
 * was built with a sequence member with two elements, it is possible to modify
 * any of those elements, but it's not possible to add a third element.
 * 
 * (For both final and mutable topic-types)
 * - Write the sample with \idref_FooDataWriter_write.
 * \code
 * writer.write(*foo_sample);
 * \endcode
 * 
 * After write, the DataWriter owns the FlatData sample. This allows avoiding
 * additional copies, the main goal of the FlatData language binding. This
 * means that the sample cannot be reused. The DataWriter will return it to the
 * sample pool when appropriate, as described in \idref_FooDataWriter_get_loan.
 * 
 * @section SubscribingFlatData Subscribing to FlatData
 * 
 * To subscribe to a topic using a FlatData topic-type:
 * - Create a \idref_FooDataReader normally (see 
 *   \ref DDSReaderExampleModule_reader_setup).
 * - Read or take the samples using a loaning operation, such as 
 *   \idref_FooDataReader_take (copying read/take operations cannot be used 
 *   with FlatData types).
 * - Access the \ref rti::flat::Sample::root "root()" offset, and any of the 
 *   sample's members from there.
 * 
 * Note that the language binding is a local concept. It is possible
 * to publish with a FlatData topic-type and subscribe to it with a <b>plain</b>
 * topic-type with the same (or assignable) definition. It is also possible to
 * use a plain topic-type on the publisher side and subscribe to it using FlatData.
 * The DataWriter or DataReader of the plain topic-type has to use
 * \idref_XCDR2_DATA_REPRESENTATION in \idref_DataRepresentationQosPolicy.
 * 
 */

#ifdef DOXYGEN_DOCUMENTATION_ONLY
namespace rti {

    /**
     * @brief \st_ext Support for \ref RTIFlatDataModule
     */
    namespace flat {
    }
}
#endif

#include "rti/flat/ExceptionHelper.hpp"
#include "rti/flat/FlatSample.hpp"
#include "rti/flat/FlatTypeTraits.hpp"
#include "rti/flat/SequenceBuilders.hpp"

#endif // RTI_DDS_FLAT_RTIFLAT_HPP_

