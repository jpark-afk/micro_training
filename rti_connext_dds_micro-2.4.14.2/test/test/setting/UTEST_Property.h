/*
 * FILE: UTEST_Property.c - Unit-test support for properties
 *
 * (c) Copyright, Real-Time Innovations, 2013-2020.
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
 * 31dec2013,tk Refactored from UT_Parser.c
 * 08nov2013,tk Written
 */
/*ce
 * \file UTEST_Property.h
 * \brief Unit-test support for properties
 */

#ifndef UTEST_Property_h
#define UTEST_Property_h

#ifndef RTI_CERT
/*e \brief
 * Delete all properties in the UTEST context
 * 
 * \param[in] setting UTEST context to delete all properties from
 */
RTITestDllExport void
UTEST_Property_delete_property(struct UTEST_Context *setting);
#endif

/*e \brief
 * Read a file with properties and add the properties to the UTEST context
 * 
 * \param[in] file    File to read the properties from
 * \param[in] sysinfo System information
 * \param[in] setting UTEST context
 *
 * \return 0 if sucess. -1 if error.
 */
RTITestDllExport int
UTEST_Property_read_property_file(const char *file,
                                  struct UTEST_SystemInfo *sysinfo,
                                  struct UTEST_Context *setting);

/*e \brief
 * Adds a property to the UTEST context. The propery name and value
 * are passed as arguments to this function.
 *
 * \details
 * Memory is NOT allocated internally for the property name and value.
 * 
 * \param[in] setting   UTEST context settings
 * \param[in] name      Property name
 * \param[in] value     Property value
 */
RTITestDllExport void
UTEST_Property_add_property_nv(struct UTEST_Context *setting,
                               char *name,
                               char *value);

#endif

