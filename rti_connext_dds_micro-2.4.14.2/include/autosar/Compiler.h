/*
 * FILE: Compiler.h - Minimum needed AutoSAR 4.2.2 Compiler definitions
 *
 * (c) Copyright, Real-Time Innovations, 2020-2022
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * AUTOSAR_SWS_CompilerAbstraction.pdf
 *
 * Modification History
 * --------------------
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Added _ to macro parameter names.
 */
#ifdef RTIME_AUTOSAR_OSEKCORE
#include "Os_Compiler.h"
#endif

#ifdef RTIME_AUTOSAR_WINCORE
/*ci
 * \brief    The compiler abstraction shall provide the NULL_PTR define with
 *           a void pointer to zero definition.
 */
#define NULL_PTR    ((void *)0)

/*ci
 * \brief    The compiler abstraction shall define the FUNC macro for the
 *           declaration and definition of functions that ensures correct
 *           syntax of function declarations as required by a specific compiler.
 */
#define FUNC(rettype_, memclass_) rettype_

/*ci
 * \brief    The compiler abstraction shall define the CONSTP2VAR macro for the
 *           declaration and definition of constant pointers accessing variables.
 */
#define CONSTP2VAR(ptrtype_, memclass_, ptrclass_) ptrtype_ * const

/*ci
 * \brief    The compiler abstraction shall define the CONST macro for the
 *           declaration and definition of constants.
 */
#define CONST(consttype_, memclass_) consttype_ const

/*ci
 * \brief    The compiler abstraction shall define the VAR macro for the
 *           declaration and definition of variables.
 */
#define VAR(vartype_, memclass_) vartype_

/*ci
 * \brief    The compiler abstraction shall define the P2VAR macro for the
 *           declaration and definition of pointers in RAM, pointing to variables.
 */
#define P2VAR(ptrtype_, memclass_, ptrclass_) ptrtype_ *

/*ci
 * \brief    The compiler abstraction shall define the P2CONST macro for the
 *           declaration and definition of pointers in RAM pointing to constants
 */
#define P2CONST(ptrtype_, memclass_, ptrclass_) ptrtype_ const *

/*ci
 * \brief    The compiler abstraction shall define the CONSTP2CONST macro for the
 *           declaration and definition of constant pointers accessing constants.
 */
#define CONSTP2CONST(ptrtype_, memclass_, ptrclass_) ptrtype_ const * const

/*ci
 * \brief    The compiler abstraction shall define the P2FUNC macro for the
 *           type definition of pointers to functions.
 */
#define P2FUNC(rettype_, ptrclass_, fctname_) rettype_ (* fctname_)
#endif
