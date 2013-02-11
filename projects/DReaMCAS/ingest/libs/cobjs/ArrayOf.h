/*
 * The contents of this file are subject to the terms
 * of the Common Development and Distribution License
 * (the "License").  You may not use this file except
 * in compliance with the License.
 *
 * You can obtain a copy of the license at
 * http://www.opensource.org/licenses/cddl1.php
 * See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL
 * HEADER in each file and include the License file at
 * http://www.opensource.org/licenses/cddl1.php.  If 
 * applicable, add the following below this CDDL HEADER, 
 * with the fields enclosed by brackets "[]" replaced 
 * with your own identifying information: 
 * Portions Copyright [yyyy]
 * [name of copyright owner]
 */ 

/*
 * $(@)ArrayOf.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * OBJECT ArrayOf -- An array is an indexed collection.  Individual items
 * within an array may be stored and retrieved by index.
 * 
 * The array will grow as necessary to hold all stored items.
 */
#ifndef	_COBJS_ARRAYOF_H
#define	_COBJS_ARRAYOF_H

#pragma ident "@(#)ArrayOf.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * OBJECT ArrayOf Instance Type
 ***********************************************************************/
typedef struct _ArrayOf *ArrayOf;

/***********************************************************************
 * OBJECT ArrayOf Inline Support
 ***********************************************************************/
#if	defined(DO_INLINING) && ! defined(ARRAYOF_BODY)
#define	ARRAYOF_INLINE	static
#else	/* defined(DO_INLINING) && ! defined(ARRAYOF_BODY) */
#define	ARRAYOF_INLINE	extern
#endif	/* defined(DO_INLINING) && ! defined(ARRAYOF_BODY) */

/***********************************************************************
 * OBJECT ArrayOf Class Interface
 ***********************************************************************/

/*
 * Creates a new ArrayOf some type with default initial length and default
 * growth increment.
 */
extern ArrayOf      arrayOfNew(size_t sizeofType);

#define	NEW_ARRAY(type)							\
	(arrayOfNew(sizeof(type)))

/*
 * Creates a new ArrayOf with the given initial length and growth increment.
 * 
 * If incr is zero, array will double in size when growth is needed.
 */
extern ArrayOf
arrayOfNewWithSizeAndIncrement(size_t sizeofType, int length,
			       int incr);

#define	NEW_ARRAY_WITH_SIZE_INCR(type, size, incr)			\
	(arrayOfNewWithSizeAndIncrement(sizeof(type), size, incr))

/***********************************************************************
 * OBJECT ArrayOf Instance Interface
 ***********************************************************************/

/*
 * Returns pointer to storage for item at given index.
 */
ARRAYOF_INLINE void *arrayOfItemAtByPtr(ArrayOf arrayOf, int index);

#define	arrayOfItemAt(arrayOf, type, index)				\
	(*(type *)arrayOfItemAtByPtr(arrayOf, index))

/*
 * Returns pointer to storage for new item appended to end of array.
 */
ARRAYOF_INLINE void *arrayOfItemAppendByPtr(ArrayOf arrayOf);

#define	arrayOfItemAppend(arrayOf, type)				\
	(*(type *)arrayOfItemAppendByPtr(arrayOf))

/*
 * Return pointer to the allocated array. USE WITH CARE: Array may move after
 * call
 */
ARRAYOF_INLINE void *arrayOfArrayPtr(const ArrayOf arrayOf);

#define	arrayOfGetArray(arrayOf, type)					\
	((type *)arrayOfArrayPtr(arrayOf))


/*
 * Returns the number of entries in the group list. The array indices run
 * from 0 .. (length-1).
 */
ARRAYOF_INLINE int  arrayOfLength(const ArrayOf array);


/*
 * Sets the length of the array.  If the array is made larger, the new
 * elements are initialized to zero.
 */
extern void         arrayOfSetLength(ArrayOf arrayOf, int length);


/*
 * Deallocates and frees memory associated with the array. It is illegal to
 * reference the array after a call to arrayFree().
 * 
 * Does NOT free the items contained within the array.
 */
extern void         arrayOfFree(ArrayOf arrayOf);

/***********************************************************************
 * OBJECT ArrayOf Inlined Function Bodies
 ***********************************************************************/
#if	defined(DO_INLINING) && ! defined(ARRAYOF_BODY)
#define	ARRAYOF_HEADER
#include "cobjs/ArrayOf.c"
#undef	ARRAYOF_HEADER
#endif	/* defined(DO_INLINING) && ! defined(ARRAYOF_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_ARRAYOF_H */
