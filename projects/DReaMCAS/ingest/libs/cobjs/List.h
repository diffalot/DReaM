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
 * $(@)List.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/************************************************************************
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 ************************************************************************/

/************************************************************************
 * List.h
 *
 * A list is a doubly-linked list of item pointers.
 ************************************************************************/

#ifndef	_COBJS_LIST_H
#define	_COBJS_LIST_H

#pragma ident "@(#)List.h 1.1	98/10/22 SMI"

/************************************************************************
 * Includes
 ************************************************************************/
#include <stddef.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/************************************************************************
 * OBJECT List Instance Type
 ************************************************************************/
typedef struct _List *List;

/************************************************************************
 * OBJECT ListIter Instance Type
 ************************************************************************/
typedef struct _ListIter *ListIter;

/************************************************************************
 * OBJECT List Public Types
 ************************************************************************/
/*
 * ListItemSort routine should return TRUE if the itemp pointed to by
 * newItemp should be inserted closer to the list head than the item pointed
 * to by listItemp.
 */
typedef Boolean     (*ListItemSort) (const void *listItemp,
				                      const void *newItemp);
typedef Boolean     (*ListItemIdentical) (const void *listItemp,
				                     const void *testItemp);

typedef int ListRef;

/************************************************************************
 * OBJECT List Inline Support
 ************************************************************************/

#if	defined(DO_INLINING) && ! defined(LIST_BODY)
#define	LIST_INLINE	static
#else	/* defined(DO_INLINING) && ! defined(LIST_BODY) */
#define	LIST_INLINE	extern
#endif	/* defined(DO_INLINING) && ! defined(LIST_BODY) */

/************************************************************************
 * OBJECT List Class Interface
 ************************************************************************/

/*
 * Create a new list with sort function lis and identity function lii.
 */
extern List         listNew(ListItemSort lis, ListItemIdentical lii);

extern List
listNewWithSize(ListItemSort lis, ListItemIdentical lii, size_t size);

/************************************************************************
 * OBJECT List Instance Interface
 ************************************************************************/

/*
 * Return the head of the list, NULL if list empty.
 */
LIST_INLINE void   *listHead(const List list);

/*
 * Return the tail of the list, NULL if list empty.
 */
LIST_INLINE void   *listTail(const List list);

/*
 * Insert itemp at the head of the list.
 */
LIST_INLINE ListRef listHeadInsert(List list, const void *itemp);

/*
 * Insert itemp at the tail of the list.
 */
LIST_INLINE ListRef listTailInsert(List list, const void *itemp);

/*
 * Insert itemp sorted in the list.
 */
extern ListRef	    listSortedInsert(List list, const void *itemp);

/*
 * Remove the head of the list and return it. Returns NULL if list empty.
 */
LIST_INLINE void        *listHeadRemove(List list);

/*
 * Remove the tail of the list and return it. Returns NULL if list empty.
 */
LIST_INLINE void        *listTailRemove(List list);

/*
 * Remove the referenced item and return it.
 */
LIST_INLINE void        *listRefRemove(List list, ListRef ref);

/*
 * Return value of list item referenced
 */
LIST_INLINE void        *listRefItem(List list, ListRef ref);

/*
 * Update list item referenced by ref
 */
LIST_INLINE void      listRefUpdate(List list, ListRef ref, const void *itemp);

/*
 * Return TRUE if item is member of list.
 */
extern Boolean      listIsMember(const List list, const void *itemp);

/*
 * Remove itemp from the list and return it. Returns NULL if itemp not found
 * in list.
 */
extern void        *listRemove(List list, const void *itemp);

/*
 * Return TRUE if list is empty.
 */
LIST_INLINE Boolean listIsEmpty(const List list);

/*
 * Return number of list entries
 */
LIST_INLINE size_t  listSize(const List list);

/*
 * Free the list.
 */
extern void         listFree(List list);

/************************************************************************
 * OBJECT ListIter Class Interface
 ************************************************************************/

/*
 * Create a list Iterator
 * 
 * BEWARE: A list iterator is invalidated by any non-local-iterator list removal
 * of an item currently referenced by an iterator.  Doing so will cause
 * "undefined" results.
 */
extern ListIter     listIterNew(const List list);

/************************************************************************
 * OBJECT ListIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at list head Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
LIST_INLINE Boolean listIterHead(ListIter li);

/*
 * Position iterator at list tail Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
LIST_INLINE Boolean listIterTail(ListIter li);

/*
 * Position iterator at item.  Search for item proceeds from head to tail.
 * Return TRUE if item found.  Return FALSE if item not found. Position of
 * iterator is unchanged if item not found.
 */
extern Boolean      listIterFind(ListIter li, const void *itemp);

/*
 * Position iterator at item referred to by ref
 */
LIST_INLINE void listIterRef(ListIter li, ListRef ref);

/*
 * Position iterator after item. If item is not found, iterator is positioned
 * at first item that would be after the item if it were present.
 * Returns TRUE if such an item exists; FALSE otherwise.
 */
extern Boolean         listIterAfter(ListIter li, const void *itemp);

/*
 * Position iterator before item. If item is not found, iterator is
 * positioned at first item that would be before the item if it were present.
 * Returns TRUE if such an item exists; FALSE otherwise.
 */
extern Boolean         listIterBefore(ListIter li, const void *itemp);

/*
 * Position iterator at next item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
LIST_INLINE Boolean listIterNext(ListIter li);

/*
 * Position iterator at previous item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
LIST_INLINE Boolean listIterPrev(ListIter li);

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
LIST_INLINE void   *listIterItem(const ListIter li);

/*
 * Return TRUE if iterator points to valid item
 */
LIST_INLINE Boolean listIterValid(const ListIter li);

/*
 * Converts current iter position to a ListRef
 */
LIST_INLINE ListRef listIterToRef(const ListIter li);

/*
 * Removes and returns the item referenced by the iterator. Returns NULL if
 * iterator not positioned at item.
 */
LIST_INLINE void        *listIterRemove(ListIter li);

/*
 * Insert an item before the item reference by the iterator. Iterator
 * position is unchanged.  If iterator is not positioned at item, insert at
 * list head.
 */
LIST_INLINE ListRef     listIterInsert(ListIter li, const void *itemp);

/*
 * Append an item after the item reference by the iterator. Iterator position
 * is unchanged.  If iterator is not positioned at item, insert at list tail.
 */
LIST_INLINE ListRef    listIterAppend(ListIter li, const void *itemp);

/*
 * Free an iterator
 */
extern void         listIterFree(ListIter li);

/************************************************************************
 * OBJECT List Inline Function Bodies
 ************************************************************************/

#if	defined(DO_INLINING) && ! defined(LIST_BODY)
#define	LIST_HEADER
#include "cobjs/List.c"
#undef	LIST_HEADER
#endif	/* defined(DO_INLINING) && ! defined(LIST_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_LIST_H */
