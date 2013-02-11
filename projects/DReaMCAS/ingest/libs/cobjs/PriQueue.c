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
 * $(@)PriQueue.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * PriQueue.c -- Description.
 */

#if	!defined(PRIQUEUE_HEADER)

#pragma ident "@(#)PriQueue.c 1.3	99/03/22 SMI"

#define	PRIQUEUE_BODY
#define	PRIQUEUE_INLINE		extern
#include "cobjs/PriQueue.h"

#endif	/* !defined(PRIQUEUE_HEADER) */

#include "cobjs/ArrayOf.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

#define	PRIQUEUE_DEFAULT_LENGTH		100

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _PriQueue {
    ArrayOf		items;		/* heap of items */
    int			nItems;		/* items in heap */
    PriQueueInOrderFunc	inOrderFunc;	/* user-supplied priority cmp func */
};

#if	!defined(PRIQUEUE_HEADER)

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

static void priQueueUpHeap(PriQueue pq, int inx);
static void priQueueDownHeap(PriQueue pq, int inx);
static int priQueueLocate(PriQueue pq, void *item);

#ifdef DEBUG
static Boolean priQueueValidate(PriQueue pq);
#endif /* DEBUG */

/*************************************************************************
 * Class Methods
 *************************************************************************/

PriQueue
priQueueNew(int initSize, PriQueueInOrderFunc inOrderFunc)
{
    PriQueue pq;

    if (initSize <= 0) initSize = PRIQUEUE_DEFAULT_LENGTH;
    pq = NEW_ZEROED(struct _PriQueue, 1);
    pq->inOrderFunc = inOrderFunc;
    pq->items = NEW_ARRAY_WITH_SIZE_INCR(void *, initSize, 50);
    pq->nItems = 0;
    arrayOfItemAt(pq->items, void *, 0) = NULL;
    return pq;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

void
priQueueInsert(PriQueue pq, void *item)
{
    arrayOfItemAt(pq->items, void *, ++pq->nItems) = item;
    priQueueUpHeap(pq, pq->nItems);
#ifdef	DEBUG
    ASSERT(priQueueValidate(pq));
#endif	/* DEBUG */
}

void *
priQueueRemove(PriQueue pq)
{
    void *item = NULL;

    if (pq->nItems > 0) {
	item = arrayOfItemAt(pq->items, void *, 1);
	arrayOfItemAt(pq->items, void *, 1) = arrayOfItemAt(pq->items,
						         void *, pq->nItems--);
	priQueueDownHeap(pq, 1);
    }
#ifdef	DEBUG
    ASSERT(priQueueValidate(pq));
#endif	/* DEBUG */
    return item;
}

/*
 * Return current highest priority item, but do NOT remove.
 */
void *
priQueueHead(PriQueue pq)
{
    void *item = NULL;

    if (pq->nItems > 0) {
	item = arrayOfItemAt(pq->items, void *, 1);
    }
    return item;
}

void *
priQueueReplace(PriQueue pq, void *item)
{
    arrayOfItemAt(pq->items, void *, 0) = item;
    priQueueDownHeap(pq, 0);
    item = arrayOfItemAt(pq->items, void *, 0);
    arrayOfItemAt(pq->items, void *, 0) = NULL;
#ifdef	DEBUG
    ASSERT(priQueueValidate(pq));
#endif	/* DEBUG */
    return item;
}

int
priQueueLength(PriQueue pq)
{
    return pq->nItems;
}

Boolean
priQueueIsMember(PriQueue pq, void *item)
{
    return Boolean(priQueueLocate(pq, item) > 0);
}

Boolean
priQueueDelete(PriQueue pq, void *item)
{
    int inx = priQueueLocate(pq, item);
    void *t;

    if (inx <= 0) {
	return FALSE;
    }
    t = arrayOfItemAt(pq->items, void *, pq->nItems--);
    if (inx <= pq->nItems) {
	arrayOfItemAt(pq->items, void *, inx) = t;
	if (! (*pq->inOrderFunc)(t, item)) {
	    priQueueDownHeap(pq, inx);
	} else {
	    priQueueUpHeap(pq, inx);
	}
    }
#ifdef	DEBUG
    ASSERT(priQueueValidate(pq));
#endif	/* DEBUG */
    return TRUE;
}

void
priQueueFree(PriQueue pq)
{
    arrayOfFree(pq->items);
    free(pq);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

static void
priQueueUpHeap(PriQueue pq, int inx)
{
    void *item = arrayOfItemAt(pq->items, void *, inx);
    void *t;

    while ((t = arrayOfItemAt(pq->items, void *, inx / 2)) != NULL
      && ! (*pq->inOrderFunc)(t, item)) {
	arrayOfItemAt(pq->items, void *, inx) = t;
	inx /= 2;
    }
    arrayOfItemAt(pq->items, void *, inx) = item;;
}

static void
priQueueDownHeap(PriQueue pq, int inx)
{
    void *item = arrayOfItemAt(pq->items, void *, inx);

    while (inx <= pq->nItems / 2) {
	int pbx = inx + inx;
	void *t = arrayOfItemAt(pq->items, void *, pbx);
	void *t2;

	if (pbx < pq->nItems
	  && ! (*pq->inOrderFunc)(t,
		  t2 = arrayOfItemAt(pq->items, void *, pbx + 1))) {
	    pbx += 1;
	    t = t2;
	}
	if ((*pq->inOrderFunc)(item, t)) {
	    break;
	}
	arrayOfItemAt(pq->items, void *, inx) = t;
	inx = pbx;
    }
    arrayOfItemAt(pq->items, void *, inx) = item;
}

static int
priQueueLocate(PriQueue pq, void *item)
{
    int i;

    for (i = pq->nItems;
	 i > 0 && arrayOfItemAt(pq->items, void *, i) != item;
	 i--) {
	continue;
    }
    return i;
}

#ifdef DEBUG
static Boolean
priQueueValidate(PriQueue pq)
{
    void *item1;
    void *itemx;
    int i;

    if (arrayOfItemAt(pq->items, void *, 0) != NULL) {
	return FALSE;
    }
    if (pq->nItems > 0) {
	if ((item1 = arrayOfItemAt(pq->items, void *, 1)) == NULL) {
	    return FALSE;
	}
	for (i = 2; i <= pq->nItems; i++) {
	    if ((itemx = arrayOfItemAt(pq->items, void *, i)) == NULL) {
		return FALSE;
	    }
	    if (! (*pq->inOrderFunc)(item1, itemx)) {
		return FALSE;
	    }
	}
    }
    return TRUE;
}
#endif /* DEBUG */

#endif					   /* !defined(PRIQUEUE_HEADER) */
