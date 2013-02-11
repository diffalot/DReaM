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
 * $(@)SortTable.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * SortTable.h -- Description
 */

#ifndef	_COBJS_SORTTABLE_H
#define	_COBJS_SORTTABLE_H

#pragma ident "@(#)SortTable.h 1.2	99/03/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _SortTable *SortTable;
typedef struct _SortIter  *SortIter;

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef enum SortTableMode {
    SORT_TABLE_MODE_OVERWRITE,
    SORT_TABLE_MODE_UNIQUE,
    SORT_TABLE_MODE_MULTIPLE
} SortTableMode;

typedef struct SortItem {
    const void         *key;
    const void         *value;
} SortItem;

/*
 * These three routines must be implemented to construct a sorttable with an
 * arbitrary key.  (Currently, sorttable's must have a void * value.)
 * 
 * SortKeyCmp returns a value < 0 if key1 should be closer to the head
 * than key2, 0 if key1 and key2 are equal, and a value > 0 if key1
 * should be farther from the head than key2.
 * 
 * SortKeyDup duplicates a key (a malloc if key is a reference, a simple
 * return of the value if key is a value).
 * 
 * SortKeyFree frees a duplicated key (a free if key is a reference, a no-op
 * if key is a value).
 * 
 * Implementations are provided for string and integer keys (although you might
 * still want to do your own implementations using better knowledge of your
 * keys.
 */
typedef int         (*SortKeyCmp) (const void *key1, const void *key2);
typedef const void *(*SortKeyDup) (const void *key, const void *value);
typedef void        (*SortKeyFree) (void *key);

typedef void (*SortTablePrintFunc)(const char *msg);
typedef void (*SortTableItemToStringFunc)(SortItem *itemp, Boolean isRed,
			char *buf, size_t bufSize);

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(SORTTABLE_BODY)
#define	SORTTABLE_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(SORTTABLE_BODY) */
#define	SORTTABLE_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(SORTTABLE_BODY) */

/**********************************************************************
 * OBJECT SortTable Class Interface
 **********************************************************************/

/*
 * Create a string keyed sorttable.
 */
extern SortTable    sortTableStrNew(SortTableMode mode);

/*
 * Create an integer keyed sorttable.
 */
extern SortTable    sortTableIntNew(SortTableMode mode);

/*
 * Creates and returns a new sort table with default size and fill factor.
 * 
 * keyCmp, keyDup, and keyFree are routines which implement the
 * named key functions.
 */
extern SortTable
sortTableNew(SortTableMode mode, SortKeyCmp keyCmp, SortKeyDup keyDup,
	     SortKeyFree keyFree);

extern SortTable    sortTableIntNewWithSize(SortTableMode mode, int size);

extern SortTable    sortTableStrNewWithSize(SortTableMode mode, int size);

/*
 * Create a sort table with a given initial size.
 */
extern SortTable
sortTableNewWithSize(SortTableMode mode, int size, SortKeyCmp keyCmp,
		     SortKeyDup keyDup, SortKeyFree keyFree);

/**********************************************************************
 * OBJECT SortTable Instance Interface
 **********************************************************************/

/*
 * Enters key-value pair into sort table. If key already exists, it's value
 * is overwritten.
 *
 * Returns TRUE if key was not previously in table, FALSE otherwise.
 * For SORT_TABLE_MODE_OVERWRITE, value replaces previous value associated
 *	with key.
 * For SORT_TABLE_MODE_UNIQUE, table is not updated if key is equal to
 *	currently existing key
 * For SORT_TABLE_MODE_MULTIPLE, table will have multiple values associated
 *	with key.  It is unpredictable which value will be returned via
 *	sortTableGet or the order that iteraters will return duplicate
 *	keys.  MULTIPLE mode tables do NOT support sortTableRemove.
 */
extern Boolean
_sortTablePut(SortTable sortTable, const void *key,
	      const void *value);

#define	sortTablePut(sortTable, key, value)		\
	_sortTablePut(sortTable, (void *)(key), (void *)(value))

/*
 * Returns the value associated with key. Returns NULL if the key is not
 * found.
 */
extern void        *_sortTableGet(const SortTable sortTable, const void *key);

#define	sortTableGet(sortTable, key)			\
	_sortTableGet(sortTable, (void *)(key))

/*
 * Returns true if the key is a member of the sorttable.
 */
extern Boolean
_sortTableIsMember(const SortTable sortTable,
		   const void *key);

#define	sortTableIsMember(sortTable, key)		\
	_sortTableIsMember(sortTable, (void *)(key))

/*
 * Remove the key-value pair from the sort table. It is not an error if key
 * does not exist. Returns the value associated with the removed key-value.
 * Returns NULL if the key is not found.
 *
 * Not legal for SORT_TABLE_MODE_MULTIPLE tables.
 */
extern void        *_sortTableRemove(SortTable sortTable, const void *key);

#define	sortTableRemove(sortTable, key)			\
	_sortTableRemove(sortTable, (void *)(key))

/*
 * Returns the current length of the sort table.
 */
SORTTABLE_INLINE int          sortTableLength(const SortTable sortTable);

extern void sortTableVerify(SortTable sortTable);

extern void
sortTablePrint(SortTable sortTable, SortTablePrintFunc pFunc,
	       SortTableItemToStringFunc iFunc, int maxWidth);

/*
 * Frees the sorttable. Does not free the values.
 */
extern void         sortTableFree(SortTable sortTable);

/************************************************************************
 * SortIter Class Interface
 ************************************************************************/

/*
 * Create a list Iterator
 * 
 * BEWARE: A list iterator is invalidated by any non-local-iterator list removal
 * of an item currently referenced by an iterator.  Doing so will cause
 * "undefined" results.
 */
extern SortIter     sortIterNew(const SortTable sortTable);

/************************************************************************
 * SortIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at list head Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
extern Boolean sortIterHead(SortIter si);

/*
 * Position iterator at list tail Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
extern Boolean sortIterTail(SortIter si);

/*
 * Position iterator at item with given key.  If duplicates are present,
 * it is unpredictable which of them will be found.
 * Return TRUE if item found.  Return FALSE if item not found. Position of
 * iterator is unchanged if item not found.
 */
extern Boolean      _sortIterFind(SortIter si, const void *key);

#define	sortIterFind(si, key)	_sortIterFind(si, (void *)(key))

/*
 * Position iterator after item with key.
 * If item is not found, iterator is positioned
 * at first item that would be after the item if it were present.
 * Returns TRUE if there is an item after; FALSE if not.
 */
extern Boolean         _sortIterAfter(SortIter si, const void *key);

#define	sortIterAfter(si, key)	_sortIterAfter(si, (void *)(key))

/*
 * Position iterator before item with key. If item is not found, iterator is
 * positioned at first item that would be before the item if it were present.
 * Returns TRUE if there is an item before; FALSE if not.
 */
extern Boolean         _sortIterBefore(SortIter si, const void *key);

#define	sortIterBefore(si, key)	_sortIterBefore(si, (void *)(key))

/*
 * Position iterator at next item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
extern Boolean sortIterNext(SortIter si);

/*
 * Position iterator at previous item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
extern Boolean sortIterPrev(SortIter si);

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE SortItem   *sortIterItem(const SortIter si);

/*
 * Return the key referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE const void   *sortIterKey(const SortIter si);

/*
 * Return the value referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE const void   *sortIterValue(const SortIter si);

/*
 * Return TRUE if iterator points to valid item
 */
SORTTABLE_INLINE Boolean sortIterValid(const SortIter si);

/*
 * Free an iterator
 */
extern void         sortIterFree(SortIter si);

#if	defined(DO_INLINING) && ! defined(SORTTABLE_BODY)
#define	SORTTABLE_HEADER
#include "cobjs/SortTable.c"
#undef	SORTTABLE_HEADER
#endif		   /* defined(DO_INLINING) && !defined(SORTTABLE_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_SORTTABLE_H */
