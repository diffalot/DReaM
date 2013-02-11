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
 * $(@)ArrayOf.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:33 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * ArrayOf.c -- Self-growing array of an arbitrary type.
 */

#if	!defined(ARRAYOF_HEADER)

#pragma ident "@(#)ArrayOf.c 1.1	98/10/22 SMI"

#define	ARRAYOF_BODY
#define	ARRAYOF_INLINE		extern
#include "cobjs/ArrayOf.h"

#endif	/* !defined(ARRAYOF_HEADER) */

#include <stdlib.h>
#include <string.h>

#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"

/*************************************************************************
 * OBJECT ArrayOf Defines
 *************************************************************************/

#define	INIT_LIST_SIZE		20	   /* initial list size (MUST BE > 0) */


/*************************************************************************
 * OBJECT ArrayOf Instance Variables
 *************************************************************************/

struct _ArrayOf {
    int                 sizeofType;	   /* sizeof entries */
    int                 length;		   /* last used array index + 1 */
    int                 arrayLength;	   /* storage allocated for items */
    int                 increment;	   /* size increase when necessary */
    void               *items;		   /* ptr to array of some type */
};

/*************************************************************************
 * OBJECT Private Routines Referenced from Inlines
 *************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

INLINE_PRIVATE void *arrayGrow(ArrayOf arrayOf, int index);

#ifdef __cplusplus
}
#endif

#if	!defined(ARRAYOF_HEADER)

/*************************************************************************
 * OBJECT ArrayOf Class Methods
 *************************************************************************/

ArrayOf
arrayOfNew(size_t sizeofType)
{
    return arrayOfNewWithSizeAndIncrement(sizeofType, INIT_LIST_SIZE, 0);
}

ArrayOf
arrayOfNewWithSizeAndIncrement(size_t sizeofType, int size, int incr)
{
    /*
     * Creates a new Array with the given initial size and growth increment.
     * 
     * If incr is zero, array will double in size when growth is needed.
     */
    ArrayOf             arrayOf = NEW(struct _ArrayOf, 1);

    ASSERT(size > 0 && incr >= 0);
    arrayOf->length = 0;
    arrayOf->sizeofType = sizeofType;
    arrayOf->arrayLength = size;
    arrayOf->increment = incr;
    arrayOf->items = calloc(arrayOf->arrayLength, arrayOf->sizeofType);
    if (arrayOf->items == NULL) {
	free(arrayOf);
	arrayOf = NULL;
    }
    return arrayOf;
}

/*************************************************************************
 * OBJECT ArrayOf Instance Methods
 *************************************************************************/

void
arrayOfSetLength(ArrayOf arrayOf, int length)
{
    /*
     * Sets the length of the array.  If the array is made larger, the new
     * elements are initialized to zero.
     */
    ASSERT(length >= 0);
    if (length > arrayOf->length) {
	/* getting larger, expand by referencing new last element */
	(void) arrayOfItemAtByPtr(arrayOf, length - 1);
    } else if (length < arrayOf->length) {
	/* smaller, zero truncated entries */
	int                 oldLength = arrayOf->length;

	arrayOf->length = length;
	(void) memset((char *) arrayOf->items
		      + (arrayOf->length * arrayOf->sizeofType), 0,
		      (oldLength - length) * arrayOf->sizeofType);
    }
}

void
arrayOfFree(ArrayOf arrayOf)
{
    /*
     * Deallocates and frees memory associated with the array. It is illegal
     * to reference the array after a call to arrayFree().
     * 
     * Does NOT free the items contained within the array.
     */
    free(arrayOf->items);
    free(arrayOf);
}

INLINE_PRIVATE void *
arrayGrow(ArrayOf arrayOf, int index)
{
    /*
     * Returns the item at the given index.  Indices are 0 based.
     */
    void               *item;

    ASSERT(index >= 0);
    ASSERT(arrayOf->length <= arrayOf->arrayLength);
    if (index >= arrayOf->arrayLength) {
	int                 newBytes;
	int                 oldBytes;

	oldBytes = arrayOf->arrayLength * arrayOf->sizeofType;
	arrayOf->arrayLength = arrayOf->arrayLength == 0 ? INIT_LIST_SIZE
	  : arrayOf->increment <= 0 ? arrayOf->arrayLength * 2
	  : arrayOf->arrayLength + arrayOf->increment;
	if (index >= arrayOf->arrayLength) {
	    arrayOf->arrayLength = index + 1;
	}
	arrayOf->items = realloc(arrayOf->items,
				 arrayOf->arrayLength * arrayOf->sizeofType);
	if (arrayOf->items == NULL) {
	    ABORT("no memory");
	}
	newBytes = arrayOf->sizeofType * arrayOf->arrayLength;
	(void) memset((char *) arrayOf->items + oldBytes, 0,
		      newBytes - oldBytes);
    }
    if (index >= arrayOf->length) {
	arrayOf->length = index + 1;
    }
    item = ((char *) (arrayOf->items)) + (index * arrayOf->sizeofType);
    return item;
}

#endif					   /* !defined(ARRAYOF_HEADER) */

/*************************************************************************
 * OBJECT ArrayOf Inline Methods
 *************************************************************************/

ARRAYOF_INLINE void *
arrayOfItemAtByPtr(ArrayOf arrayOf, int index)
{
    /*
     * Returns the item at the given index.  Indices are 0 based.
     */
    if ((unsigned) index >= arrayOf->length) {
	if ((unsigned) index >= arrayOf->arrayLength) {
	    return arrayGrow(arrayOf, index);
	}
	arrayOf->length = index + 1;
    }
    return ((char *) (arrayOf->items)) + (index * arrayOf->sizeofType);
}

/*
 * Returns pointer to storage for new item appended to end of array.
 */
ARRAYOF_INLINE void *
arrayOfItemAppendByPtr(ArrayOf arrayOf)
{
    return arrayOfItemAtByPtr(arrayOf, arrayOf->length);
}


/*
 * Return pointer to the allocated array. USE WITH CARE: Array may move after
 * call
 */
ARRAYOF_INLINE void *
arrayOfArrayPtr(const ArrayOf arrayOf)
{
    return (arrayOf->length > 0) ? arrayOf->items : NULL;
}

ARRAYOF_INLINE int
arrayOfLength(const ArrayOf arrayOf)
{
    /*
     * Returns the number of entries in the Array. The array indices run from
     * 0 .. (length-1).
     */
    return arrayOf->length;
}
