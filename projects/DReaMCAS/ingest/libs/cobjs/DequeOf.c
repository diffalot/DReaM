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
 * $(@)DequeOf.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:33 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * DequeOf.c -- Description.
 */

#if	!defined(DEQUEOF_HEADER)

#pragma ident "@(#)Deque.c 1.1	98/10/22 SMI"

#define	DEQUEOF_INLINE		extern
#define	DEQUEOF_BODY
#include "cobjs/DequeOf.h"

#endif					   /* !defined(DEQUEOF_HEADER) */

#include <string.h>

#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"


/*************************************************************************
 * Defines
 *************************************************************************/

#define	DEQUEOF_INIT_SIZE		50

/*************************************************************************
 * Instance Variables
 *************************************************************************/

/*
 * OBJECT DequeOf Instance Variables
 *
 * +--------------------------------------------------------------------+
 * |              |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|        |
 * +--------------------------------------------------------------------+
 *                 ^                                            ^
 *                 |                                            |
 *                 tail                                         head
 *
 * +--------------------------------------------------------------------+
 * |xxxxxxxxxxxxxx|                                            |xxxxxxxx|
 * +--------------------------------------------------------------------+
 *                 ^                                            ^
 *                 |                                            |
 *                 head                                         tail
 *
 * head == tail => empty
 * (head + 1) % size == tail => full
 */
struct _DequeOf {
    size_t		itemSize;
    void               *items;
    int                 size;
    int                 sizeIncr;
    DequeOfIsEqualFunc	isEqualFunc;
    int                 head;
    int                 tail;
#ifdef	ASSERTS
    Boolean		wasModified;
#endif	/* ASSERTS */
};

/*
 * OBJECT DequeOfIter Instance Variables
 */
struct _DequeOfIter {
    DequeOf		dequeOf;
    int			pos;
};

/*************************************************************************
 * Inline Private method prototypes
 *************************************************************************/

INLINE_PRIVATE void dequeOfGrow(DequeOf dequeOf);
static Boolean      dequeOfIsFull(DequeOf dequeOf);
static Boolean      dequeOfIsEmpty(DequeOf dequeOf);
static void	    *dequeOfItemPtr(DequeOf dequeOf, int index);

#if	!defined(DEQUEOF_HEADER)
#ifdef __cplusplus
extern "C" {
#endif    
static Boolean
dequeOfDefaultIsEqualFunc(const void *item1p, const void *item2p,
	size_t itemLen);
#ifdef __cplusplus
}
#endif
/***********************************************************************
 * OBJECT DequeOf Class Methods
 ***********************************************************************/

/*
 * Create a deque.  Initialize size is initSize (if 0, a default
 * will be used),  if deque must be grown, it will be grown by
 * sizeIncr (if 0, a value proportional to the current size
 * will be used).
 */
DequeOf
dequeOfNew(size_t itemSize, int initSize, int sizeIncr,
	DequeOfIsEqualFunc isEqualFunc)
{
    DequeOf              dequeOf;

    if (initSize <= 0) {
	initSize = DEQUEOF_INIT_SIZE;
    }
    dequeOf = NEW(struct _DequeOf, 1);
    dequeOf->itemSize = itemSize;
    dequeOf->size = initSize + 1;
    dequeOf->sizeIncr = sizeIncr;
    dequeOf->isEqualFunc =  isEqualFunc != NULL
	? isEqualFunc : dequeOfDefaultIsEqualFunc;
    dequeOf->items = CALLOC(dequeOf->size, dequeOf->itemSize);
    dequeOf->head = 0;
    dequeOf->tail = 0;
#ifdef	ASSERTS
    dequeOf->wasModified = FALSE;
#endif	/* ASSERTS */
    return dequeOf;
}

/************************************************************************
 * OBJECT DequeOfIter Class Interface
 ************************************************************************/

/*
 * Create a deque Iterator
 */
DequeOfIter
dequeOfIterNew(const DequeOf dequeOf)
{
    DequeOfIter di = NEW(struct _DequeOfIter, 1);
    di->dequeOf = dequeOf;
    di->pos = -1;
    return di;
}

/*************************************************************************
 * Non-inlinable Instance Methods
 *************************************************************************/

/*
 * Finds item from deque and returns (but does not remove).
 *
 * NOTE: This is expensive.
 */
void	    *
_dequeOfFind(DequeOf dequeOf, void *item)
{
    int i;

    for (i = dequeOf->tail; i != dequeOf->head;
	    i++, i = i >= dequeOf->size ? 0 : i) {
	void *cmpItemp = dequeOfItemPtr(dequeOf, i);
	if ((*dequeOf->isEqualFunc)(cmpItemp, item, dequeOf->itemSize)) {
	    return cmpItemp;
	}
    }
    return NULL;
}


/*
 * Returns TRUE if item is on deque, FALSE otherwise.
 *
 * NOTE: This is expensive.
 */
Boolean
_dequeOfIsMember(DequeOf dequeOf, void *item)
{
    return Boolean(_dequeOfFind(dequeOf, item) != NULL);
}

/*
 * Finds and removes item from deque.
 *
 * NOTE: This is expensive.
 */
Boolean
_dequeOfDelete(DequeOf dequeOf, void *item)
{
    Boolean retval = FALSE;
    int i;

    for (i = dequeOf->tail; i != dequeOf->head;
	    i++, i = i >= dequeOf->size ? 0 : i) {
	if ((*dequeOf->isEqualFunc)(dequeOfItemPtr(dequeOf, i), item,
		    dequeOf->itemSize)) {
	    if (i < dequeOf->tail
		    || (i < dequeOf->head
			&& (dequeOf->head - i) <= (i - dequeOf->tail))) {
		dequeOf->head -= 1;
		(void) memmove(dequeOfItemPtr(dequeOf, i),
			       dequeOfItemPtr(dequeOf, i + 1),
		    (dequeOf->head - i) * dequeOf->itemSize);
	    } else {
		(void) memmove(dequeOfItemPtr(dequeOf, dequeOf->tail + 1),
		    dequeOfItemPtr(dequeOf, dequeOf->tail),
		    (i - dequeOf->tail) * dequeOf->itemSize);
		dequeOf->tail += 1;
		if (dequeOf->tail >= dequeOf->size) {
		    dequeOf->tail = 0;
		}
	    }
	    retval = TRUE;
	    break;
	}
    }
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
    return retval;
}

/*
 * Frees DequeOf object.  Does NOT free current contents of deque.
 */
void
dequeOfFree(DequeOf dequeOf)
{
    free(dequeOf->items);
    free(dequeOf);
}

/*
 * Free an iterator
 */
extern void
dequeOfIterFree(DequeOfIter di)
{
    free(di);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Referenced by inlines, but not inlinable
 */
INLINE_PRIVATE		     void
dequeOfGrow(DequeOf dequeOf)
{
    int oldSize = dequeOf->size;
    int sizeIncr = dequeOf->sizeIncr;

    if (sizeIncr <= 0) {
	sizeIncr = dequeOf->size / 10;
	if (sizeIncr < DEQUEOF_INIT_SIZE) {
	    sizeIncr = DEQUEOF_INIT_SIZE;
	}
    }
    dequeOf->size += sizeIncr;
    dequeOf->items = REALLOC(dequeOf->items, dequeOf->size, dequeOf->itemSize);
    if (dequeOf->head < dequeOf->tail) {
	(void) memmove(dequeOfItemPtr(dequeOf, dequeOf->tail + sizeIncr),
		       dequeOfItemPtr(dequeOf, dequeOf->tail),
		       (oldSize - dequeOf->tail) * dequeOf->itemSize);
	dequeOf->tail += sizeIncr;
    }
}

static Boolean
dequeOfDefaultIsEqualFunc(const void *item1p, const void *item2p,
	size_t itemLen)
{
    return Boolean(memcmp(item1p, item2p, itemLen) == 0);
}

#endif					   /* !defined(DEQUEOF_HEADER) */

/***********************************************************************
 * Inlinable Instance Interface
 ***********************************************************************/

/*
 * Enqueues item on head of deque.
 */
DEQUEOF_INLINE void
_dequeOfEnqueueHead(DequeOf dequeOf, void *item)
{
    if (dequeOfIsFull(dequeOf)) {
	dequeOfGrow(dequeOf);
    }
    (void) memcpy(dequeOfItemPtr(dequeOf, dequeOf->head++), item,
		  dequeOf->itemSize);
    if (dequeOf->head >= dequeOf->size) {
	dequeOf->head = 0;
    }
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Enqueues item on tail of deque.
 */
DEQUEOF_INLINE void
_dequeOfEnqueueTail(DequeOf dequeOf, void *item)
{
    if (dequeOfIsFull(dequeOf)) {
	dequeOfGrow(dequeOf);
    }
    if (dequeOf->tail <= 0) {
	dequeOf->tail = dequeOf->size;
    }
    (void) memcpy(dequeOfItemPtr(dequeOf, --dequeOf->tail), item,
		  dequeOf->itemSize);
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Dequeues item from head of deque, returns NULL if deque is empty.
 */
DEQUEOF_INLINE void  *
_dequeOfDequeueHead(DequeOf dequeOf)
{
    if (dequeOfIsEmpty(dequeOf)) {
	return NULL;
    }
    if (dequeOf->head <= 0) {
	dequeOf->head = dequeOf->size;
    }
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
    return dequeOfItemPtr(dequeOf, --dequeOf->head);
}

/*
 * Dequeues item from tail of deque, returns NULL if deque is empty.
 */
DEQUEOF_INLINE void *
_dequeOfDequeueTail(DequeOf dequeOf)
{
    void *item;

    if (dequeOfIsEmpty(dequeOf)) {
	return NULL;
    }
    item = dequeOfItemPtr(dequeOf, dequeOf->tail++);
    if (dequeOf->tail >= dequeOf->size) {
	dequeOf->tail = 0;
    }
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
    return item;
}

/*
 * Returns item at index position in deque.  Index 0 refers to head,
 * index dequeOfLength() - 1 refers to tail.
 *
 * Returns NULL if index outside of deque.
 */
DEQUEOF_INLINE void        *
_dequeOfItemAt(DequeOf dequeOf, int index)
{
    void *item = NULL;

    if (index < dequeOfLength(dequeOf)) {
	int itemx = dequeOf->head - index - 1;
	if (itemx < 0) {
	    itemx += dequeOf->size;
	}
	item = dequeOfItemPtr(dequeOf, itemx);
    }
    return item;
}

DEQUEOF_INLINE void	*
_dequeOfHead(DequeOf dequeOf)
{
    return dequeOfIsEmpty(dequeOf) ? NULL
	: dequeOfItemPtr(dequeOf,
		dequeOf->head == 0 ? dequeOf->size - 1 : dequeOf->head - 1);
}

DEQUEOF_INLINE void	*
_dequeOfTail(DequeOf dequeOf)
{
    return dequeOfIsEmpty(dequeOf) ? NULL
	: dequeOfItemPtr(dequeOf, dequeOf->tail);
}

/*
 * Stores item into deque at index position.  Overwrites item previously at
 * index position.  Index 0 refers to head, index dequeLength() - 1 refers
 * to tail.
 *
 * Aborts if index outside of deque.
 */
DEQUEOF_INLINE void
_dequeOfSetItemAt(DequeOf dequeOf, int index, void *item)
{
    int itemx;

    if (index >= dequeOfLength(dequeOf)) ABORT("dequeOfSetItemAt: bad index");

    itemx = dequeOf->head + index;
    if (itemx >= dequeOf->size) {
	itemx -= dequeOf->size;
    }
    (void) memcpy(dequeOfItemPtr(dequeOf, itemx), item, dequeOf->itemSize);
#ifdef	ASSERTS
    dequeOf->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Returns number of items in deque
 */
DEQUEOF_INLINE int
dequeOfLength(DequeOf dequeOf)
{
    int                 length;

    length = dequeOf->head - dequeOf->tail;
    return length < 0 ? length + dequeOf->size : length;
}

/************************************************************************
 * OBJECT DequeOfIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at deque head Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUEOF_INLINE Boolean
dequeOfIterHead(DequeOfIter di)
{
    DequeOf dequeOf = di->dequeOf;

    if (dequeOfIsEmpty(dequeOf)) {
	return FALSE;
    }
#ifdef	ASSERTS
    dequeOf->wasModified = FALSE;
#endif	/* ASSERTS */
    di->pos = ((dequeOf->head == 0) ? dequeOf->size : dequeOf->head) - 1;
    return TRUE;
}

/*
 * Position iterator at deque tail Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUEOF_INLINE Boolean
dequeOfIterTail(DequeOfIter di)
{
    DequeOf dequeOf = di->dequeOf;

    if (dequeOfIsEmpty(dequeOf)) {
	return FALSE;
    }
#ifdef	ASSERTS
    dequeOf->wasModified = FALSE;
#endif	/* ASSERTS */
    di->pos = dequeOf->tail;
    return TRUE;
}

/*
 * Position iterator at next item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterNext moves from head towards tail.
 */
DEQUEOF_INLINE Boolean
dequeOfIterNext(DequeOfIter di)
{
    DequeOf dequeOf = di->dequeOf;

    if (di->pos == -1) {
	di->pos = dequeOf->head;
#ifdef	ASSERTS
	dequeOf->wasModified = FALSE;
#endif	/* ASSERTS */
    }
    ASSERT(! dequeOf->wasModified);
    ASSERT(dequeOfIterIsValid(di) || di->pos == dequeOf->head);
    if (di->pos == dequeOf->tail) {
	return FALSE;
    }
    di->pos = ((di->pos == 0) ? dequeOf->size : di->pos) - 1;
    return TRUE;
}

/*
 * Position iterator at previous item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeOfIterPrev moves from tail towards head.
 */
DEQUEOF_INLINE Boolean
dequeOfIterPrev(DequeOfIter di)
{
    DequeOf dequeOf = di->dequeOf;
    int newPos = di->pos + 1;

    if (di->pos == -1) {
	di->pos = dequeOf->tail - 1;
#ifdef	ASSERTS
	dequeOf->wasModified = FALSE;
#endif	/* ASSERTS */
    }

    ASSERT(! dequeOf->wasModified);
    ASSERT(dequeOfIterIsValid(di));
    if (newPos >= dequeOf->size) newPos = 0;
    if (newPos == dequeOf->head) {
	return FALSE;
    }
    di->pos = newPos;
    return TRUE;
}

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
DEQUEOF_INLINE void   *
_dequeOfIterItem(const DequeOfIter di)
{
    ASSERT(! di->dequeOf->wasModified);
    return dequeOfIterIsValid(di) ? dequeOfItemPtr(di->dequeOf, di->pos) : NULL;
}

/*
 * Return TRUE if iterator points to valid item
 */
DEQUEOF_INLINE Boolean
dequeOfIterIsValid(const DequeOfIter di)
{
    DequeOf dequeOf = di->dequeOf;

    ASSERT(! dequeOf->wasModified);
    ASSERT(di->pos >= 0 && di->pos < dequeOf->size);
    return Boolean((dequeOf->head >= dequeOf->tail)
        ? (dequeOf->tail <= di->pos && di->pos < dequeOf->head)
	: (di->pos < dequeOf->head || di->pos >= dequeOf->tail));
}

/************************************************************************
 * OBJECT Inlinable Private Routines
 ************************************************************************/

static              Boolean
dequeOfIsEmpty(DequeOf dequeOf)
{
    /*
     * Returns TRUE if the deque is empty, FALSE otherwise.
     */
    return Boolean(dequeOf->head == dequeOf->tail);
}

static              Boolean
dequeOfIsFull(DequeOf dequeOf)
{
    /*
     * Returns TRUE if deque is full, FALSE otherwise.
     */
    return Boolean( ((dequeOf->head + 1) % dequeOf->size) == dequeOf->tail );
}

static		    void *
dequeOfItemPtr(DequeOf dequeOf, int index)
{
    return (char *)(dequeOf->items) + (index * dequeOf->itemSize);
}
