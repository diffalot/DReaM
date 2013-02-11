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
 * $(@)Deque.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:33 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Deque.c -- Description.
 */

#if	!defined(DEQUE_HEADER)

#pragma ident "@(#)Deque.c 1.2	99/03/22 SMI"

#define	DEQUE_INLINE		extern
#define	DEQUE_BODY
#include "cobjs/Deque.h"

#endif					   /* !defined(DEQUE_HEADER) */

#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"


/*************************************************************************
 * Defines
 *************************************************************************/

#define	DEQUE_INIT_SIZE		50

/*************************************************************************
 * Instance Variables
 *************************************************************************/

/*
 * OBJECT Deque Instance Variables
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
struct _Deque {
    void              **items;
    int                 size;
    int                 sizeIncr;
    int                 head;
    int                 tail;
#ifdef	ASSERTS
    Boolean		wasModified;
#endif	/* ASSERTS */
};

/*
 * OBJECT DequeIter Instance Variables
 */
struct _DequeIter {
    Deque		deque;
    int			pos;
};

/*************************************************************************
 * Inline Private method prototypes
 *************************************************************************/

INLINE_PRIVATE void dequeGrow(Deque deque);
static Boolean      dequeIsFull(Deque deque);
static Boolean      dequeIsEmpty(Deque deque);

#if	!defined(DEQUE_HEADER)

/***********************************************************************
 * OBJECT Deque Class Methods
 ***********************************************************************/

/*
 * Create a deque.  Initialize size is initSize (if 0, a default
 * will be used),  if deque must be grown, it will be grown by
 * sizeIncr (if 0, a value proportional to the current size
 * will be used).
 */
Deque
dequeNew(int initSize, int sizeIncr)
{
    Deque              deque;

    if (initSize <= 0) {
	initSize = DEQUE_INIT_SIZE;
    }
    deque = NEW(struct _Deque, 1);
    deque->size = initSize + 1;
    deque->sizeIncr = sizeIncr;
    deque->items = NEW(void *, deque->size);
    deque->head = 0;
    deque->tail = 0;
#ifdef	ASSERTS
    deque->wasModified = FALSE;
#endif	/* ASSERTS */
    return deque;
}

/************************************************************************
 * OBJECT DequeIter Class Interface
 ************************************************************************/

/*
 * Create a deque Iterator
 */
DequeIter
dequeIterNew(const Deque deque)
{
    DequeIter di = NEW(struct _DequeIter, 1);
    di->deque = deque;
    di->pos = -1;
    return di;
}

/*************************************************************************
 * Non-inlinable Instance Methods
 *************************************************************************/

/*
 * Returns TRUE if item is on deque, FALSE otherwise.
 *
 * NOTE: This is expensive.
 */
Boolean
dequeIsMember(Deque deque, void *item)
{
    Boolean retval = FALSE;
    int i;

    for (i = deque->tail; i != deque->head; i++, i = i >= deque->size ? 0 : i) {
	if (deque->items[i] == item) {
	    retval = TRUE;
	    break;
	}
    }
    return retval;
}

/*
 * Finds and removes item from deque.
 *
 * NOTE: This is expensive.
 */
Boolean
dequeDelete(Deque deque, void *item)
{
    Boolean retval = FALSE;
    int i;

    for (i = deque->tail; i != deque->head; i++, i = i >= deque->size ? 0 : i) {
	if (deque->items[i] == item) {
	    if (i < deque->tail
		    || (i < deque->head
			&& (deque->head - i) <= (i - deque->tail))) {
		deque->head -= 1;
		(void) memmove(&deque->items[i], &deque->items[i + 1],
		    (deque->head - i) * sizeof(void *));
	    } else {
		(void) memmove(&deque->items[deque->tail + 1],
		    &deque->items[deque->tail],
		    (i - deque->tail) * sizeof(void *));
		deque->tail += 1;
		if (deque->tail >= deque->size) {
		    deque->tail = 0;
		}
	    }
	    retval = TRUE;
	    break;
	}
    }
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
    return retval;
}

/*
 * Frees Deque object.  Does NOT free current contents of deque.
 */
void
dequeFree(Deque deque)
{
    free(deque->items);
    free(deque);
}

/*
 * Free an iterator
 */
extern void
dequeIterFree(DequeIter di)
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
dequeGrow(Deque deque)
{
    int oldSize = deque->size;
    int sizeIncr = deque->sizeIncr;

    if (sizeIncr <= 0) {
	sizeIncr = deque->size / 10;
	if (sizeIncr < DEQUE_INIT_SIZE) {
	    sizeIncr = DEQUE_INIT_SIZE;
	}
    }
    deque->size += sizeIncr;
    deque->items = RENEW(void *, deque->items, deque->size);
    if (deque->head < deque->tail) {
	(void) memmove(&deque->items[deque->tail + sizeIncr],
		       &deque->items[deque->tail],
		       (oldSize - deque->tail) * sizeof(void *));
	deque->tail += sizeIncr;
    }
}

#endif					   /* !defined(DEQUE_HEADER) */

/***********************************************************************
 * Inlinable Instance Interface
 ***********************************************************************/

/*
 * Enqueues item on head of deque.
 */
DEQUE_INLINE void
dequeEnqueueHead(Deque deque, void *item)
{
    if (dequeIsFull(deque)) {
	dequeGrow(deque);
    }
    deque->items[deque->head++] = item;
    if (deque->head >= deque->size) {
	deque->head = 0;
    }
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Enqueues item on tail of deque.
 */
DEQUE_INLINE void
dequeEnqueueTail(Deque deque, void *item)
{
    if (dequeIsFull(deque)) {
	dequeGrow(deque);
    }
    if (deque->tail <= 0) {
	deque->tail = deque->size;
    }
    deque->items[--deque->tail] = item;
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Dequeues item from head of deque, returns NULL if deque is empty.
 */
DEQUE_INLINE void  *
dequeDequeueHead(Deque deque)
{
    if (dequeIsEmpty(deque)) {
	return NULL;
    }
    if (deque->head <= 0) {
	deque->head = deque->size;
    }
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
    return deque->items[--deque->head];
}

/*
 * Dequeues item from tail of deque, returns NULL if deque is empty.
 */
DEQUE_INLINE void *
dequeDequeueTail(Deque deque)
{
    void *item;

    if (dequeIsEmpty(deque)) {
	return NULL;
    }
    item = deque->items[deque->tail++];
    if (deque->tail >= deque->size) {
	deque->tail = 0;
    }
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
    return item;
}

/*
 * Returns item at index position in deque.  Index 0 refers to head,
 * index dequeLength() - 1 refers to tail.
 *
 * Returns NULL if index outside of deque.
 */
DEQUE_INLINE void        *
dequeItemAt(Deque deque, int index)
{
    void *item = NULL;

    if (index < dequeLength(deque)) {
	int itemx = deque->head - index - 1;
	if (itemx < 0) {
	    itemx += deque->size;
	}
	item = deque->items[itemx];
    }
    return item;
}

DEQUE_INLINE void	*
dequeHead(Deque deque)
{
    return dequeIsEmpty(deque) ? NULL
	: deque->items[deque->head == 0 ? deque->size - 1 : deque->head - 1];
}

DEQUE_INLINE void	*
dequeTail(Deque deque)
{
    return dequeIsEmpty(deque) ? NULL
	: deque->items[deque->tail];
}

/*
 * Stores item into deque at index position.  Overwrites item previously at
 * index position.  Index 0 refers to head, index dequeLength() - 1 refers
 * to tail.
 *
 * Aborts if index outside of deque.
 */
DEQUE_INLINE void
dequeSetItemAt(Deque deque, int index, void *item)
{
    int itemx;

    if (index >= dequeLength(deque)) ABORT("dequeSetItemAt: bad index");

    itemx = deque->head + index;
    if (itemx >= deque->size) {
	itemx -= deque->size;
    }
    deque->items[itemx] = item;
#ifdef	ASSERTS
    deque->wasModified = TRUE;
#endif	/* ASSERTS */
}

/*
 * Returns number of items in deque
 */
DEQUE_INLINE int
dequeLength(Deque deque)
{
    int                 length;

    length = deque->head - deque->tail;
    return length < 0 ? length + deque->size : length;
}

/************************************************************************
 * OBJECT DequeIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at deque head Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUE_INLINE Boolean
dequeIterHead(DequeIter di)
{
    Deque deque = di->deque;

    if (dequeIsEmpty(deque)) {
	return FALSE;
    }
#ifdef	ASSERTS
    deque->wasModified = FALSE;
#endif	/* ASSERTS */
    di->pos = ((deque->head == 0) ? deque->size : deque->head) - 1;
    return TRUE;
}

/*
 * Position iterator at deque tail Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUE_INLINE Boolean
dequeIterTail(DequeIter di)
{
    Deque deque = di->deque;

    if (dequeIsEmpty(deque)) {
	return FALSE;
    }
#ifdef	ASSERTS
    deque->wasModified = FALSE;
#endif	/* ASSERTS */
    di->pos = deque->tail;
    return TRUE;
}

/*
 * Position iterator at next item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterNext moves from head towards tail.
 */
DEQUE_INLINE Boolean
dequeIterNext(DequeIter di)
{
    Deque deque = di->deque;

    if (di->pos == -1) {
	di->pos = deque->head;
#ifdef	ASSERTS
	deque->wasModified = FALSE;
#endif	/* ASSERTS */
    }
    ASSERT(! deque->wasModified);
    ASSERT(dequeIterIsValid(di) || di->pos == deque->head);
    if (di->pos == deque->tail) {
	di->pos = -1;
	return FALSE;
    }
    di->pos = ((di->pos == 0) ? deque->size : di->pos) - 1;
    return TRUE;
}

/*
 * Position iterator at previous item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterPrev moves from tail towards head.
 */
DEQUE_INLINE Boolean
dequeIterPrev(DequeIter di)
{
    Deque deque = di->deque;
    int newPos;

    if (di->pos == -1) {
	di->pos = deque->tail - 1;
#ifdef	ASSERTS
	deque->wasModified = FALSE;
#endif	/* ASSERTS */
    }
    ASSERT(! deque->wasModified);
    ASSERT(dequeIterIsValid(di));
    newPos = di->pos + 1;
    if (newPos >= deque->size) newPos = 0;
    if (newPos == deque->head) {
	return FALSE;
    }
    di->pos = newPos;
    return TRUE;
}

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
DEQUE_INLINE void   *
dequeIterItem(const DequeIter di)
{
    ASSERT(! di->deque->wasModified);
    return dequeIterIsValid(di) ? di->deque->items[di->pos] : NULL;
}

/*
 * Return TRUE if iterator points to valid item
 */
DEQUE_INLINE Boolean
dequeIterIsValid(const DequeIter di)
{
    Deque deque = di->deque;

    ASSERT(! deque->wasModified);
    ASSERT(di->pos >= 0 && di->pos < deque->size);
    return Boolean( (deque->head >= deque->tail)
        ? (deque->tail <= di->pos && di->pos < deque->head)
	: (di->pos < deque->head || di->pos >= deque->tail) );
}

/************************************************************************
 * OBJECT Inlinable Private Routines
 ************************************************************************/

static              Boolean
dequeIsEmpty(Deque deque)
{
    /*
     * Returns TRUE if the deque is empty, FALSE otherwise.
     */
    return Boolean(deque->head == deque->tail);
}

static              Boolean
dequeIsFull(Deque deque)
{
    /*
     * Returns TRUE if deque is full, FALSE otherwise.
     */
    return Boolean( ((deque->head + 1) % deque->size) == deque->tail );
}
