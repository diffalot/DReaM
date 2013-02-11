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
 * $(@)List.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * List.c -- A doubly-linked list of item pointers.
 */

#pragma ident "@(#)List.c 1.4	99/05/28 SMI"

#if	!defined(LIST_HEADER)
#define	LIST_BODY
#define	LIST_INLINE	extern
#include "cobjs/List.h"
#endif	/* !defined(LIST_HEADER) */

#include <stdlib.h>
#include <synch.h>

#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

#define	LIST_INIT_FREECNT		100

typedef struct ListItem ListItem;

struct ListItem {
    ListItem           *nextp;
    ListItem           *prevp;
    const void         *itemp;
};

struct _ListIter {
    List                list;
    ListItem           *curLip;
    ListItem            curLi;
    ListIter	        nextIterp;
};

/*
 * OBJECT List Instance Variables
 */
struct _List {
    ListItemSort        lis;
    ListItemIdentical   lii;
    size_t              size;		    /* INCLUDING ANCHOR! */
    ListItem		*freeItemp;
    ListItem		*itemArray;
#ifdef	ASSERTS
    Boolean		wasModified;
#endif /* ASSERTS */
    mutex_t		iterLock;
    struct _ListIter	iterHead;
};

static ListItem *listItemNew(List list);
static ListItem *listItem(List list, ListRef ref);
static ListRef listRef(List list, ListItem *lip);
static void listItemFree(List list, ListItem *lip);

INLINE_PRIVATE void listExpandFree(List list);

LIST_INLINE void   *
listHead(const List list)
{
    ListItem           *lip = list->itemArray[0].nextp;

    return lip != &list->itemArray[0] ? (void *) lip->itemp : NULL;
}

LIST_INLINE void   *
listTail(const List list)
{
    ListItem           *lip = list->itemArray[0].prevp;

    return lip != &list->itemArray[0] ? (void *) lip->itemp : NULL;
}

LIST_INLINE ListRef
listHeadInsert(List list, const void *itemp)
{
    ListItem           *nip = listItemNew(list);
    ListItem           *lip = list->itemArray[0].nextp;

    nip->itemp = itemp;

    lip->prevp->nextp = nip;
    nip->nextp = lip;
    nip->prevp = lip->prevp;
    lip->prevp = nip;
    list->size += 1;
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return listRef(list, nip);
}

LIST_INLINE ListRef
listTailInsert(List list, const void *itemp)
{
    ListItem           *nip = listItemNew(list);
    ListItem           *lip = &list->itemArray[0];

    nip->itemp = itemp;

    lip->prevp->nextp = nip;
    nip->nextp = lip;
    nip->prevp = lip->prevp;
    lip->prevp = nip;
    list->size += 1;
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return listRef(list, nip);
}

LIST_INLINE void               *
listHeadRemove(List list)
{
    ListItem           *lip = list->itemArray[0].nextp;
    void               *itemp = NULL;

    if (lip != &list->itemArray[0]) {
	lip->prevp->nextp = lip->nextp;
	lip->nextp->prevp = lip->prevp;
	itemp = (void *) lip->itemp;
	listItemFree(list, lip);
	list->size -= 1;
    }
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return itemp;
}

LIST_INLINE void               *
listTailRemove(List list)
{
    ListItem           *lip = list->itemArray[0].prevp;
    void               *itemp = NULL;

    if (lip != &list->itemArray[0]) {
	lip->prevp->nextp = lip->nextp;
	lip->nextp->prevp = lip->prevp;
	itemp = (void *) lip->itemp;
	listItemFree(list, lip);
	list->size -= 1;
    }
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return itemp;
}

LIST_INLINE void               *
listRefRemove(List list, ListRef ref)
{
    ListItem           *lip = listItem(list, ref);
    void               *itemp;

    lip->prevp->nextp = lip->nextp;
    lip->nextp->prevp = lip->prevp;
    itemp = (void *) lip->itemp;
    listItemFree(list, lip);
    list->size -= 1;
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return itemp;
}

LIST_INLINE void        *
listRefItem(List list, ListRef ref)
{
    return (void *) listItem(list, ref)->itemp;
}

LIST_INLINE void
listRefUpdate(List list, ListRef ref, const void *itemp)
{
    listItem(list, ref)->itemp = itemp;
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
}

LIST_INLINE Boolean
listIsEmpty(const List list)
{
    return Boolean(list->itemArray[0].nextp == &list->itemArray[0]);
}

LIST_INLINE size_t
listSize(const List list)
{
    /* decr by 1 because anchor is counted */
    return list->size - 1;
}

/************************************************************************
 * OBJECT ListIter Inlinable Instance Interface
 ************************************************************************/

/*
 * Position iterator at list head Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
LIST_INLINE Boolean
listIterHead(ListIter li)
{
    li->curLip = li->list->itemArray[0].nextp;
    li->curLi = *li->curLip;
#ifdef	ASSERTS
    li->list->wasModified = FALSE;
#endif /* ASSERTS */
    return Boolean(li->curLip != &li->list->itemArray[0]);
}

/*
 * Position iterator at list tail Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
LIST_INLINE Boolean
listIterTail(ListIter li)
{
#ifdef	ASSERTS
    li->list->wasModified = FALSE;
#endif /* ASSERTS */
    li->curLip = li->list->itemArray[0].prevp;
    li->curLi = *li->curLip;
    return Boolean(li->curLip != &li->list->itemArray[0]);
}

/*
 * Position iterator at next item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
LIST_INLINE Boolean
listIterNext(ListIter li)
{
    if (li->curLip == NULL) {
	li->curLip = &li->list->itemArray[0];
	li->curLi = *li->curLip;
#ifdef	ASSERTS
	li->list->wasModified = FALSE;
#endif /* ASSERTS */
    }
    ASSERT(! li->list->wasModified);
    li->curLip = li->curLi.nextp;
    li->curLi = *li->curLip;
    return Boolean(li->curLip != &li->list->itemArray[0]);
}

/*
 * Position iterator at previous item in list. Return TRUE if there is a
 * previous item; FALSE if wrapping around.
 */
LIST_INLINE Boolean
listIterPrev(ListIter li)
{
    if (li->curLip == NULL) {
	li->curLip = &li->list->itemArray[0];
	li->curLi = *li->curLip;
#ifdef	ASSERTS
	li->list->wasModified = FALSE;
#endif /* ASSERTS */
    }
    ASSERT(! li->list->wasModified);
    li->curLip = li->curLi.prevp;
    li->curLi = *li->curLip;
    return Boolean(li->curLip != &li->list->itemArray[0]);
}

LIST_INLINE void
listIterRef(ListIter li, ListRef ref)
{
    li->curLip = listItem(li->list, ref);
    li->curLi = *li->curLip;
#ifdef	ASSERTS
    li->list->wasModified = FALSE;
#endif /* ASSERTS */
}

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
LIST_INLINE void   *
listIterItem(const ListIter li)
{
    ASSERT(! li->list->wasModified);
    return (void *) li->curLi.itemp;
}

/*
 * Return TRUE if iterator points to valid item
 */
LIST_INLINE Boolean
listIterValid(const ListIter li)
{
    ASSERT(! li->list->wasModified);
    return Boolean(li->curLip != NULL && li->curLip != &li->list->itemArray[0]);
}

LIST_INLINE ListRef
listIterToRef(const ListIter li)
{
    ASSERT(! li->list->wasModified);
    return li->curLip == NULL ? 0 : li->curLip - &li->list->itemArray[0];
}

/*
 * Removes and returns the item referenced by the iterator. Returns NULL if
 * iterator not positioned at item.
 */
LIST_INLINE void               *
listIterRemove(ListIter li)
{
    ListItem           *lip = li->curLip;

    ASSERT(! li->list->wasModified);
    if (lip != NULL && lip != &li->list->itemArray[0]) {
	lip->prevp->nextp = lip->nextp;
	lip->nextp->prevp = lip->prevp;
	listItemFree(li->list, lip);
	li->list->size -= 1;
    }
    li->curLip = &li->list->itemArray[0];
    return (void *) li->curLi.itemp;
}

/*
 * Insert an item before the item reference by the iterator. Iterator
 * position is unchanged.  If iterator is not positioned at item, insert at
 * list head.
 */
LIST_INLINE ListRef
listIterInsert(ListIter li, const void *itemp)
{
    ListItem           *nip = listItemNew(li->list);
    ListItem           *lip = li->curLip;

    ASSERT(! li->list->wasModified);
    if (lip == NULL || lip == &li->list->itemArray[0]) {
	lip = li->list->itemArray[0].nextp;
    }
    nip->itemp = itemp;

    lip->prevp->nextp = nip;
    nip->nextp = lip;
    nip->prevp = lip->prevp;
    lip->prevp = nip;

    li->curLi = *li->curLip;
    li->list->size += 1;
    return listRef(li->list, nip);
}

/*
 * Append an item after the item reference by the iterator. Iterator position
 * is unchanged.  If iterator is not positioned at item, insert at list tail.
 */
LIST_INLINE ListRef
listIterAppend(ListIter li, const void *itemp)
{
    ListItem           *nip = listItemNew(li->list);
    ListItem           *lip = li->curLip;

    ASSERT(! li->list->wasModified);
    if (lip == NULL || lip == &li->list->itemArray[0]) {
	lip = li->list->itemArray[0].prevp;
    }
    nip->itemp = itemp;

    lip->nextp->prevp = nip;
    nip->prevp = lip;
    nip->nextp = lip->nextp;
    lip->nextp = nip;

    li->curLi = *li->curLip;
    li->list->size += 1;
    return listRef(li->list, nip);
}

static ListItem *
listItemNew(List list)
{
    ListItem *nip;

    if (list->freeItemp == NULL) {
	listExpandFree(list);
    }
    ASSERT(list->freeItemp != NULL);
    nip = list->freeItemp;
    list->freeItemp = nip->nextp;
    return nip;
}

static ListRef
listRef(List list, ListItem *lip)
{
    return lip - list->itemArray;
}

static ListItem *
listItem(List list, ListRef ref)
{
    ASSERT(ref != 0);
    return &list->itemArray[ref];
}

static void
listItemFree(List list, ListItem *lip)
{
    lip->prevp = NULL;
    lip->itemp = NULL;
    lip->nextp = list->freeItemp;
    list->freeItemp = lip;
}

#if	!defined(LIST_HEADER)
/*
 * Private function prototypes
 */

#ifdef __cplusplus
extern "C" {
#endif

static Boolean
listDefaultSort(const void *listItemp,
		const void *newItemp);
static Boolean
listDefaultIdentical(const void *listItemp,
		     const void *testItemp);

#ifdef __cplusplus
}
#endif

/*
 * OBJECT List Class Methods
 */
List
listNew(ListItemSort lis, ListItemIdentical lii)
{
    return listNewWithSize(lis, lii, LIST_INIT_FREECNT);
}

List
listNewWithSize(ListItemSort lis, ListItemIdentical lii, size_t size)
{
    List                list;

    if ((list = NEW_ZEROED(struct _List, 1)) != NULL) {
	int i;
	list->lis = lis != NULL ? lis : listDefaultSort;
	list->lii = lii != NULL ? lii : listDefaultIdentical;
	list->itemArray = NEW_ZEROED(ListItem, size);
	list->itemArray[0].nextp = &list->itemArray[0];
	list->itemArray[0].prevp = &list->itemArray[0];
	list->size = 1;
	for (i = 1; i < size - 1; i++) {
	    list->itemArray[i].nextp = &list->itemArray[i + 1];
	}
	list->freeItemp = &list->itemArray[1];
#ifdef	ASSERTS
	list->wasModified = FALSE;
#endif	/* ASSERTS */
	(void) mutex_init(&list->iterLock, USYNC_THREAD, 0);
	/*
	 * Add sentinel at end of list.  A NULL won't work because
	 * of list relocation.
	 */
	list->iterHead.nextIterp = NEW_ZEROED(struct _ListIter, 1);
	list->iterHead.nextIterp->nextIterp = list->iterHead.nextIterp;
    }
    return list;
}

void
listFree(List list)
{
    ASSERT(list->iterHead.nextIterp->nextIterp == list->iterHead.nextIterp);
    (void) mutex_destroy(&list->iterLock);
    free(list->iterHead.nextIterp);
    free(list->itemArray);
    free(list);
}

ListRef
listSortedInsert(List list, const void *itemp)
{
    ListItem           *nip = listItemNew(list);
    ListItem           *lip;

    /*
     * ListItemSort routine should return TRUE if the itemp pointed to by
     * newItemp should be inserted closer to the list head than the item
     * pointed to by listItemp.
     */
    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lis) (lip->itemp, itemp)) {
	    break;
	}
    }
    nip->itemp = itemp;

    lip->prevp->nextp = nip;
    nip->nextp = lip;
    nip->prevp = lip->prevp;
    lip->prevp = nip;
    list->size += 1;
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return listRef(list, nip);
}

void               *
listRemove(List list, const void *itemp)
{
    ListItem           *lip;

    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lii) (lip->itemp, itemp)) {
	    void               *retItemp;

	    lip->prevp->nextp = lip->nextp;
	    lip->nextp->prevp = lip->prevp;
	    retItemp = (void *) lip->itemp;
	    listItemFree(list, lip);
	    list->size -= 1;
	    return retItemp;
	}
    }
#ifdef	ASSERTS
    list->wasModified = TRUE;
#endif /* ASSERTS */
    return NULL;
}

Boolean
listIsMember(const List list, const void *itemp)
{
    ListItem           *lip;

    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lii) (lip->itemp, itemp)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/************************************************************************
 * OBJECT ListIter Class Interface
 ************************************************************************/

/*
 * Create a list Iterator
 * 
 * List iterator is initialized to reference head item of list.
 * 
 * A list iterator is invalidated by any non-iterator-based list operation.
 */
ListIter
listIterNew(const List list)
{
    ListIter            li;

    if ((li = NEW_ZEROED(struct _ListIter, 1)) != NULL) {
	li->list = list;
	li->curLip = NULL;
	li->curLi.itemp = NULL;
	(void) mutex_lock(&list->iterLock);
	li->nextIterp = list->iterHead.nextIterp;
	list->iterHead.nextIterp = li;
	(void) mutex_unlock(&list->iterLock);
    }
    return li;
}

/*
 * Position iterator at item.  Search for item proceeds from head to tail.
 * Return TRUE if item found.  Return FALSE if item not found. Position of
 * iterator is unchanged if item not found.
 */
Boolean
listIterFind(ListIter li, const void *itemp)
{
    List		list = li->list;
    ListItem           *lip;

    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lii) (lip->itemp, itemp)) {
	    li->curLip = lip;
	    li->curLi = *li->curLip;
#ifdef	ASSERTS
	    list->wasModified = FALSE;
#endif /* ASSERTS */
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Position iterator after item. If item is not found, iterator is positioned
 * at first item that would be after the item if it were present.
 */
Boolean
listIterAfter(ListIter li, const void *itemp)
{
    List		list = li->list;
    ListItem           *lip;

    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lis) (lip->itemp, itemp)) {
	    /*
	     * Sort routine returns TRUE when looking at item we should
	     * insert before, so if item doesn't exist, lip is now pointing
	     * at item after where item would be.
	     * 
	     * If item does exist, sort routine can place us at first of run of
	     * identical items or after item (depending on whether sort
	     * intends identical items to  insert before or after existing
	     * identical items).  If pointing at the first of a run of
	     * identical items, skip them here.
	     */
	    while (lip != &list->itemArray[0]
		    && (*list->lii) (lip->itemp, itemp)) {
		lip = lip->nextp;
	    }
	    break;
	}
    }
    li->curLip = lip;
    li->curLi = *li->curLip;
#ifdef	ASSERTS
    list->wasModified = FALSE;
#endif /* ASSERTS */
    return Boolean(lip != &list->itemArray[0]);
}

/*
 * Position iterator before item. If item is not found, iterator is
 * positioned at first item that would be before the item if it were present.
 */
Boolean
listIterBefore(ListIter li, const void *itemp)
{
    List		list = li->list;
    ListItem           *lip;

    for (lip = list->itemArray[0].nextp;
	    lip != &list->itemArray[0];
	    lip = lip->nextp) {
	if ((*list->lis) (lip->itemp, itemp)) {
	    /*
	     * Sort routine returns TRUE when looking at item we should
	     * insert before, so if item doesn't exist, lip is now pointing
	     * at item after where item would be.  Since this routine should
	     * return item BEFORE, back lip up one item.
	     * 
	     * If item does exist, sort routine can place us at first of run of
	     * identical items or after item (depending on whether sort
	     * intends identical items to  insert before or after existing
	     * identical items).  Since we've backed-up one item, we are now
	     * either before a run of identical items or pointing at the last
	     * of a run of identical items. If pointing at the last of a run
	     * of identical items, skip them here.
	     */
	    lip = lip->prevp;
	    while (lip != &list->itemArray[0]
		    && (*list->lii) (lip->itemp, itemp)) {
		lip = lip->prevp;
	    }
	    break;
	}
    }
    li->curLip = lip;
    li->curLi = *li->curLip;
#ifdef	ASSERTS
    list->wasModified = FALSE;
#endif /* ASSERTS */
    return Boolean(lip != &list->itemArray[0]);
}

/*
 * Free an iterator
 */
void
listIterFree(ListIter li)
{
    List list = li->list;
    ListIter prevLi;

    (void) mutex_lock(&list->iterLock);
    prevLi = &list->iterHead;
    while (prevLi->nextIterp != li) {
	prevLi = prevLi->nextIterp;
	ASSERT(prevLi->nextIterp != prevLi);
    }
    prevLi->nextIterp = li->nextIterp;
    (void) mutex_unlock(&list->iterLock);
    free(li);
}


#define	LIST_RELOCATE(list, type, p, delta)	    \
	(p) = (type)(((char *)(p)) + (delta));

INLINE_PRIVATE void
listExpandFree(List list)
{
    ListItem *oldArray = list->itemArray;
    int incr = list->size / 10;
    ListIter li;
    long reloc;
    int i;

    if (incr < LIST_INIT_FREECNT) incr = LIST_INIT_FREECNT;

    list->itemArray = RENEW(ListItem, list->itemArray, list->size + incr);
    reloc = (char *)list->itemArray - (char *)oldArray;
    if (reloc != 0) {
	/*
	 * Relocate all listItem pointers
	 */
	for (i = 0; i < list->size; i++) {
	    /* LINTED */
	    LIST_RELOCATE(list, ListItem *, list->itemArray[i].nextp, reloc);
	    /* LINTED */
	    LIST_RELOCATE(list, ListItem *, list->itemArray[i].prevp, reloc);
	}
	/*
	 * Relocate all listIter pointers
	 */
	(void) mutex_lock(&list->iterLock);
	for (li = list->iterHead.nextIterp;
	     li->nextIterp != li;
	     li = li->nextIterp) {
	    if (li->curLip != NULL) {
		/* LINTED */
		LIST_RELOCATE(list, ListItem *, li->curLip, reloc);
		/* LINTED */
		LIST_RELOCATE(list, ListItem *, li->curLi.nextp, reloc);
		/* LINTED */
		LIST_RELOCATE(list, ListItem *, li->curLi.prevp, reloc);
	    }
	}
	(void) mutex_unlock(&list->iterLock);
    }
    /*
     * Link up new free entries
     */
    (void) memset(&list->itemArray[list->size], 0, incr * sizeof(ListItem));
    for (i = list->size; i < list->size + incr - 1; i++) {
	list->itemArray[i].nextp = &list->itemArray[i + 1];
    }
    list->freeItemp = &list->itemArray[list->size];
}

static Boolean
listDefaultIdentical(const void *listItemp, const void *testItemp)
{
    return Boolean(listItemp == testItemp);
}

static Boolean
listDefaultSort(const void *listItemp, const void *newItemp)
{
    return Boolean(newItemp < listItemp);
}

#endif					   /* !defined(LIST_HEADER) */
