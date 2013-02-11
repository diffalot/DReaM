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
 * $(@)SortTable.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:35 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * SortTable.c -- Iterable red-black binary tree.
 *
 * FIXME: This doesn't free key's properly -- how should duplicated
 * internal keys be handled??
 */

#pragma ident "@(#)SortTable.c 1.2	99/03/22 SMI"

#if	!defined(SORTTABLE_HEADER)
#define	SORTTABLE_BODY
#define	SORTTABLE_INLINE		extern
#include "cobjs/SortTable.h"
#endif					   /* !defined(SORTTABLE_HEADER) */

#include <stdlib.h>

#include "cobjs/Deque.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

#define	SORT_TABLE_DEFAULT_SIZE		128

/*************************************************************************
 * Instance Variables
 *************************************************************************/

typedef struct SortNode SortNode;

struct SortNode {
    SortItem	item;
    SortNode	*link[2];
    SortNode	*p;
    Boolean	isRed:1;
    Boolean	isFree:1;
    Boolean	wasSeen:1;
    Boolean	hasData:1;
    int		printLow:10;
    int		printHigh:10;
    int		level:8;
};

struct _SortTable {
    SortTableMode	mode;
    int			length;		/* used entries in nodeArray */
    int			size;		/* total entries in nodeArray */
    int			blackDepth;
    SortNode		head;
    SortNode		*freeNodep;
    SortNode		*nodeArray;
    SortKeyCmp		keyCmp;
    SortKeyDup		keyDup;
    SortKeyFree		keyFree;
    SortTablePrintFunc	pFunc;
    SortTableItemToStringFunc iFunc;
    int			maxWidth;
#ifdef	ASSERTS
    Boolean		wasModified;
#endif	/* ASSERTS */
};

struct _SortIter {
    SortTable		sortTable;
    SortNode		*curNode;
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void sortTableInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * Returns the current length of the sort table.
 */
SORTTABLE_INLINE int
sortTableLength(const SortTable sortTable)
{
    return sortTable->length;
}

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE SortItem   *
sortIterItem(const SortIter si)
{
    ASSERT(! si->sortTable->wasModified);
    return sortIterValid(si) ? &si->curNode->item : NULL;
}

/*
 * Return the key referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE const void   *
sortIterKey(const SortIter si)
{
    ASSERT(! si->sortTable->wasModified);
    return si->curNode->item.key;
}

/*
 * Return the value referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
SORTTABLE_INLINE const void   *
sortIterValue(const SortIter si)
{
    ASSERT(! si->sortTable->wasModified);
    return si->curNode->item.value;
}

/*
 * Return TRUE if iterator points to valid item
 */
SORTTABLE_INLINE Boolean
sortIterValid(const SortIter si)
{
    ASSERT(! si->sortTable->wasModified);
    return Boolean(si->curNode != &si->sortTable->head);
}

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(SORTTABLE_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

typedef enum SortNodeDir {
    SORT_NODE_DIR_RIGHT = 0,
    SORT_NODE_DIR_LEFT = 1
}  SortNodeDir;

/*************************************************************************
 * Private method prototypes
 *************************************************************************/
static SortNode *sortTableFind(const SortTable sortTable, const void *key);
static SortNode *sortTableSplit(SortTable sortTable, const void *key,
				SortNode *s);
static SortNode *sortTableRotate(SortNode *r, SortNodeDir cDir,
				 SortNodeDir gcDir);
static SortNode *sortTableNodeAlloc(SortTable sortTable, SortNode **reallocpp);
static void sortTableNodeFree(SortTable sortTable, SortNode *nodep);
static void sortTableTraverse(SortTable sortTable, int blackDepth, SortNode *n,
	SortNode *min, SortNode *max);
static void sortTableTraversePrint(SortTable sortTable, Deque deque,
	SortTablePrintFunc pFunc, SortTableItemToStringFunc iFunc, int level,
	int maxWidth);

/*************************************************************************
 * Private function prototypes
 *************************************************************************/
static int strKeyCmp(const void *key1, const void *key2);
static const void *strKeyDup(const void *key, const void *value);
static int intKeyCmp(const void *key1, const void *key2);
static void strInsert(char *dst, char *src, int maxlen);

/*************************************************************************
 * Private class data
 *************************************************************************/

/*************************************************************************
 * Class Methods
 *************************************************************************/

/*
 * Create a string keyed sorttable.
 */
SortTable
sortTableStrNew(SortTableMode mode)
{
    return sortTableNewWithSize(mode, SORT_TABLE_DEFAULT_SIZE, strKeyCmp,
				strKeyDup, free);
}

/*
 * Create an integer keyed sorttable.
 */
SortTable
sortTableIntNew(SortTableMode mode)
{
    return sortTableNewWithSize(mode, SORT_TABLE_DEFAULT_SIZE, intKeyCmp,
				NULL, NULL);
}

/*
 * Creates and returns a new sort table with default size and fill factor.
 * 
 * keyCmp, keyDup, and keyFree are routines which implement the
 * named key functions.
 */
SortTable
sortTableNew(SortTableMode mode, SortKeyCmp keyCmp, SortKeyDup keyDup,
	SortKeyFree keyFree)
{
    return sortTableNewWithSize(mode, SORT_TABLE_DEFAULT_SIZE, keyCmp, keyDup,
				keyFree);
}

SortTable
sortTableIntNewWithSize(SortTableMode mode, int size)
{
    return sortTableNewWithSize(mode, size, intKeyCmp, NULL, NULL);
}

SortTable
sortTableStrNewWithSize(SortTableMode mode, int size)
{
    return sortTableNewWithSize(mode, size, strKeyCmp, strKeyDup, free);
}

/*
 * Create a sort table with a given initial size.
 */
SortTable
sortTableNewWithSize(SortTableMode mode, int size, SortKeyCmp keyCmp,
	SortKeyDup keyDup, SortKeyFree keyFree)
{
    SortTable sortTable = NEW_ZEROED(struct _SortTable, 1);
    int i;

    switch (mode) {
    case SORT_TABLE_MODE_UNIQUE:
    case SORT_TABLE_MODE_OVERWRITE:
    case SORT_TABLE_MODE_MULTIPLE:
	break;
    default:
	ABORT("illegal sortTable mode value");
    }
    sortTable->mode = mode;
    sortTable->keyCmp = keyCmp;
    sortTable->keyDup = keyDup;
    sortTable->keyFree = keyFree;
    sortTable->pFunc = NULL;
    sortTable->iFunc = NULL;
    sortTable->maxWidth = 80;
    sortTable->length = 0;
    sortTable->nodeArray = NEW_ZEROED(SortNode, size);
    sortTable->size = size;
    sortTable->freeNodep = NULL;
    for (i = 0; i < size; i++) {
	sortTableNodeFree(sortTable, &sortTable->nodeArray[i]);
    }
    sortTable->head.item.key = NULL;
    sortTable->head.item.value = NULL;
    sortTable->head.link[SORT_NODE_DIR_LEFT] = &sortTable->head;
    sortTable->head.link[SORT_NODE_DIR_RIGHT] = &sortTable->head;
    sortTable->head.p = &sortTable->head;
    sortTable->head.isRed = FALSE;
    sortTable->head.hasData = FALSE;
    sortTable->head.isFree = FALSE;
#ifdef	ASSERTS
    sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
    return sortTable;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

/*
 * Enters key-value pair into sort table. If key already exists, it's value
 * is overwritten.
 */
Boolean
_sortTablePut(SortTable sortTable, const void *key, const void *value)
{
    SortNode *n = sortTableNodeAlloc(sortTable, NULL);
    SortNode *t = sortTableNodeAlloc(sortTable, &n);
    SortNode *s = sortTable->head.link[SORT_NODE_DIR_RIGHT];
    SortNode *p;
    SortNodeDir pDir = SORT_NODE_DIR_RIGHT;
    Boolean retVal = TRUE;

    n->link[SORT_NODE_DIR_LEFT] = &sortTable->head;
    n->link[SORT_NODE_DIR_RIGHT] = &sortTable->head;
    n->item.key = sortTable->keyDup != NULL
		? (*sortTable->keyDup)(key, NULL) : key;
    n->item.value = value;
    n->hasData = TRUE;

    while (s->link[SORT_NODE_DIR_RIGHT] != &sortTable->head) {
	if (s->link[SORT_NODE_DIR_LEFT]->isRed
	    && s->link[SORT_NODE_DIR_RIGHT]->isRed) {
	    s = sortTableSplit(sortTable, key, s);
	}
	pDir = SortNodeDir((*sortTable->keyCmp)(key, s->item.key) < 0);
	s = s->link[pDir];
    }
    p = s->p;
    if (p != s) {
	int cmp = (*sortTable->keyCmp)(key, s->item.key);
	const void *iKey;
	SortNodeDir dir = SortNodeDir(cmp < 0);
	if (cmp == 0) {
	    retVal = FALSE;
	    switch (sortTable->mode) {
	    case SORT_TABLE_MODE_UNIQUE:
		goto done;
	    case SORT_TABLE_MODE_OVERWRITE:
		s->item.value = value;
		goto done;
	    }
	}
	iKey = (dir == SORT_NODE_DIR_LEFT) ? s->item.key : n->item.key;
	t->item.key = sortTable->keyDup != NULL
		    ? (*sortTable->keyDup)(iKey, NULL) : iKey;
	t->item.value = NULL;
	t->link[dir] = n;
	t->link[! dir] = s;
	t->hasData = FALSE;
	n->p = t;
	s->p = t;
	n = t;
	t = NULL;
    }
    n->p = p;
    p->link[pDir] = n;
    (void) sortTableSplit(sortTable, n->item.key, n);
    n = NULL;
    sortTable->length += 1;
done:
    if (t != NULL) {
	sortTableNodeFree(sortTable, t);
    }
    if (n != NULL) {
	if (sortTable->keyFree != NULL) {
	    (*sortTable->keyFree)((void *)n->item.key);
	}
	sortTableNodeFree(sortTable, t);
    }
#ifdef	ASSERTS
    sortTable->wasModified = TRUE;
#endif	/* ASSERTS */
    return retVal;
}

void        *
_sortTableGet(const SortTable sortTable, const void *key)
{
    return (void *)sortTableFind(sortTable, key)->item.value;
}

/*
 * Returns true if the key is a member of the sorttable.
 */
Boolean
_sortTableIsMember(const SortTable sortTable, const void *key)
{
    return Boolean(sortTableFind(sortTable, key) != &sortTable->head);
}

/*
 * Remove the key-value pair from the sort table. It is not an error if key
 * does not exist. Returns the value associated with the removed key-value.
 * Returns NULL if the key is not found.
 */
void        *
_sortTableRemove(SortTable sortTable, const void *key)
{
    SortNode *s = sortTable->head.link[SORT_NODE_DIR_RIGHT];
    SortNodeDir dir = SORT_NODE_DIR_RIGHT;
    void *valuep = NULL;

    if (sortTable->mode == SORT_TABLE_MODE_MULTIPLE) {
	ABORT("attempt to remove from SORT_TABLE_MODE_MULTIPLE SortTable");
    }
    if (! s->link[SORT_NODE_DIR_LEFT]->isRed
	&& ! s->link[SORT_NODE_DIR_RIGHT]->isRed) {
	s->isRed = TRUE;
    }
    for (;;) {
	SortNodeDir nextDir;

	if (s->link[SORT_NODE_DIR_LEFT] == &sortTable->head) {
	    break;
	}
	nextDir = SortNodeDir((*sortTable->keyCmp)(key, s->item.key) < 0);
	if (! s->isRed && !s->link[nextDir]->isRed) {
	    if (s->link[!nextDir]->isRed) {
		(void) sortTableRotate(s->p, dir, SortNodeDir(!nextDir));
		s->isRed = TRUE;
		s->p->isRed = FALSE;
	    } else {
		SortNode *sib = s->p->link[!dir];

		ASSERT(s->p->isRed);
		ASSERT(! sib->isRed);
		ASSERT(sib->link[SORT_NODE_DIR_LEFT] != &sortTable->head);
		s->isRed = TRUE;
		s->p->isRed = FALSE;
		if (!sib->link[SORT_NODE_DIR_LEFT]->isRed
		    && !sib->link[SORT_NODE_DIR_RIGHT]->isRed) {
		    sib->isRed = TRUE;
		} else {
		    SortNodeDir redKidDir
			= (sib->link[SORT_NODE_DIR_LEFT]->isRed)
			      ? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;
		    SortNode *p = s->p;
		    SortNode *g = p->p;
		    SortNodeDir cDir = (p == g->link[SORT_NODE_DIR_LEFT])
			? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;

		    if (dir != redKidDir) {
			sib->isRed = TRUE;
			sib->link[redKidDir]->isRed = FALSE;
			(void) sortTableRotate(g, cDir, SortNodeDir(!dir));
		    } else {
			(void) sortTableRotate(s->p, SortNodeDir(!dir), redKidDir);
			(void) sortTableRotate(g, cDir, SortNodeDir(!dir));
		    }
		}
	    }
	}
	dir = nextDir;
	s = s->link[dir];
    }
    if (s != &sortTable->head && (*sortTable->keyCmp)(key, s->item.key) == 0) {
	SortNode *p = s->p;
	SortNode *g = p->p;
	SortNode *c = p->link[!dir];
	SortNodeDir gDir = (g == &sortTable->head) ? SORT_NODE_DIR_RIGHT
	     : SortNodeDir((*sortTable->keyCmp)(s->item.key, g->item.key) < 0);

	ASSERT (p->isRed || p == &sortTable->head);
	g->link[gDir] = c;
	c->p = g;
	if (p != &sortTable->head) {
	    if (sortTable->keyFree != NULL) {
		(*sortTable->keyFree)((void *)p->item.key);
	    }
	    sortTableNodeFree(sortTable, p);
	}

	valuep = (void *) s->item.value;
	if (sortTable->keyFree != NULL) {
	    (*sortTable->keyFree)((void *)s->item.key);
	}
	sortTableNodeFree(sortTable, s);
	sortTable->length -= 1;
    }
    sortTable->head.link[SORT_NODE_DIR_RIGHT]->isRed = FALSE;
#ifdef	ASSERTS
    sortTable->wasModified = TRUE;
#endif	/* ASSERTS */
    return valuep;
}

void
sortTableVerify(SortTable sortTable)
{
    /*
     * Verify:
     *	All nodes are either in tree or in free list.
     *  No node appears more than once.
     *  Children have correct parent pointers.
     *  Tree is correctly partitioned.
     *  No consecutive red nodes.
     *  All leaves are black.
     *  All data is in leaves.
     *  Black height is uniform.
     */
    int i;
    SortNode *n;

    sortTable->blackDepth = -1;
    sortTable->head.wasSeen = FALSE;
    for (i = 0; i < sortTable->size; i++) {
	sortTable->nodeArray[i].wasSeen = FALSE;
    }
    for (n = sortTable->freeNodep; n != NULL;
	    n = n->link[SORT_NODE_DIR_RIGHT]) {
	if (! n->isFree || n->wasSeen || n->hasData || n->isRed) {
	    ABORT("free corrupted");
	}
	n->wasSeen = TRUE;
    }
    if (sortTable->head.link[SORT_NODE_DIR_LEFT] != &sortTable->head
	|| sortTable->head.isRed || sortTable->head.isFree
	|| sortTable->head.wasSeen || sortTable->head.hasData
	|| sortTable->head.p != &sortTable->head) {
	ABORT("head corrupted");
    }
    sortTable->head.wasSeen = TRUE;
    if (sortTable->head.link[SORT_NODE_DIR_RIGHT] != &sortTable->head) {
	n = sortTable->head.link[SORT_NODE_DIR_RIGHT];
	if (n->p != &sortTable->head) {
	    ABORT("root parent");
	}
	if (n->link[SORT_NODE_DIR_LEFT] == &sortTable->head
		&& n->link[SORT_NODE_DIR_RIGHT] == &sortTable->head) {
	    /* Single node tree */
	    if (! n->hasData || n->isRed || n->isFree || n->wasSeen) {
		ABORT("single node corrupt");
	    }
	    n->wasSeen = TRUE;
	} else {
	    sortTableTraverse(sortTable, 0, n, NULL, NULL);
	}
    }
    for (i = 0; i < sortTable->size; i++) {
	if (! sortTable->nodeArray[i].wasSeen) {
	    ABORT("lost node");
	}
    }

}

void
sortTablePrint(SortTable sortTable, SortTablePrintFunc pFunc,
	       SortTableItemToStringFunc iFunc, int maxWidth)
{
    SortNode *n = sortTable->head.link[SORT_NODE_DIR_RIGHT];

    if (pFunc != NULL) {
	sortTable->pFunc = pFunc;
    } else {
	pFunc = sortTable->pFunc;
    }
    if (pFunc == NULL) {
	ABORT("no pFunc");
    }
    if (iFunc != NULL) {
	sortTable->iFunc = iFunc;
    } else {
	iFunc = sortTable->iFunc;
    }
    if (iFunc == NULL) {
	ABORT("no iFunc");
    }
    if (maxWidth != 0) {
	sortTable->maxWidth = maxWidth;
    } else {
	maxWidth = sortTable->maxWidth;
    }
    (*pFunc)("SortTable Contents:\n\n");
    if (n == &sortTable->head) {
	(*pFunc)("empty tree\n");
    } else {
	Deque deque = dequeNew(10, 10);
	n->printLow = 0;
	n->printHigh = maxWidth;
	n->level = 0;
	dequeEnqueueTail(deque, n);
	sortTableTraversePrint(sortTable, deque, pFunc, iFunc, 0, maxWidth);
	dequeFree(deque);
    }
    (*pFunc)("\n\n\n\n");
}


/*
 * Frees the sorttable. Does not free the values.
 */
void
sortTableFree(SortTable sortTable)
{
    if (sortTable->keyFree != NULL) {
	SortIter si = sortIterNew(sortTable);
	while (sortIterNext(si)) {
	    (*sortTable->keyFree)((void *)sortIterItem(si)->key);
	}
	sortIterFree(si);
    }
    free(sortTable->nodeArray);
    free(sortTable);
}

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
SortIter
sortIterNew(const SortTable sortTable)
{
    SortIter si = NEW_ZEROED(struct _SortIter, 1);
    si->sortTable = sortTable;
    si->curNode = &sortTable->head;
    return si;
}

/************************************************************************
 * SortIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at list head Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
Boolean
sortIterHead(SortIter si)
{
    SortTable sortTable = si->sortTable;
    SortNode *n = sortTable->head.link[SORT_NODE_DIR_RIGHT];

    if (n == &sortTable->head) {
	return FALSE;
    }
    while (n->link[SORT_NODE_DIR_LEFT] != &sortTable->head) {
	n = n->link[SORT_NODE_DIR_LEFT];
    }
    si->curNode = n;
#ifdef	ASSERTS
    sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
    return TRUE;
}

/*
 * Position iterator at list tail Returns TRUE if list is non-empty; FALSE if
 * list is empty.
 */
Boolean
sortIterTail(SortIter si)
{
    SortTable sortTable = si->sortTable;
    SortNode *n = sortTable->head.link[SORT_NODE_DIR_RIGHT];

    if (n == &sortTable->head) {
	return FALSE;
    }
    while (n->link[SORT_NODE_DIR_RIGHT] != &sortTable->head) {
	n = n->link[SORT_NODE_DIR_RIGHT];
    }
    si->curNode = n;
#ifdef	ASSERTS
    sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
    return TRUE;
}

/*
 * Position iterator at item.  If duplicates are present, it is unpredictable
 * which of them will be found.
 * Return TRUE if item found.  Return FALSE if item not found. Position of
 * iterator is unchanged if item not found.
 */
extern Boolean
_sortIterFind(SortIter si, const void *key)
{
    SortTable sortTable = si->sortTable;
    SortNode *n = sortTableFind(sortTable, key);
    if (n != &sortTable->head) {
	si->curNode = n;
#ifdef	ASSERTS
	sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
	return TRUE;
    }
    return FALSE;
}

/*
 * Position iterator after item. If item is not found, iterator is positioned
 * at first item that would be after the item if it were present.
 * Returns TRUE if there is an item after key, FALSE otherwise.
 */
extern Boolean
_sortIterAfter(SortIter si, const void *key)
{
    SortTable sortTable = si->sortTable;

    si->curNode = sortTable->head.link[SORT_NODE_DIR_RIGHT];
    for (;;) {
	if (si->curNode->link[SORT_NODE_DIR_RIGHT] == &sortTable->head) {
	    break;
	}
	si->curNode = si->curNode->link[
		(*sortTable->keyCmp)(key, si->curNode->item.key) < 0];
    }
    while (si->curNode != &sortTable->head
	   && (*sortTable->keyCmp)(key, si->curNode->item.key) >= 0) {
	(void) sortIterNext(si);
    }
#ifdef	ASSERTS
    sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
    return Boolean(si->curNode != &sortTable->head);
}

/*
 * Position iterator before item. If item is not found, iterator is
 * positioned at first item that would be before the item if it were present.
 */
extern Boolean
_sortIterBefore(SortIter si, const void *key)
{
    SortTable sortTable = si->sortTable;

    si->curNode = sortTable->head.link[SORT_NODE_DIR_RIGHT];
    for (;;) {
	if (si->curNode->link[SORT_NODE_DIR_RIGHT] == &sortTable->head) {
	    break;
	}
	si->curNode = si->curNode->link[
		(*sortTable->keyCmp)(key, si->curNode->item.key) < 0];
    }
    while (si->curNode != &sortTable->head
	   && (*sortTable->keyCmp)(key, si->curNode->item.key) <= 0) {
	(void) sortIterPrev(si);
    }
#ifdef	ASSERTS
    sortTable->wasModified = FALSE;
#endif	/* ASSERTS */
    return Boolean(si->curNode != &sortTable->head);
}

/*
 * Position iterator at next item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
Boolean
sortIterNext(SortIter si)
{
    SortNode *s = si->curNode;
    SortNode *c;

    if (s == &si->sortTable->head) {
	return sortIterHead(si);
    }

    ASSERT(! si->sortTable->wasModified);
    do {
	c = s;
	s = s->p;
    } while (s != &si->sortTable->head && s->link[SORT_NODE_DIR_RIGHT] == c);
    if (s != &si->sortTable->head) {
	s = s->link[SORT_NODE_DIR_RIGHT];
	while (s->link[SORT_NODE_DIR_LEFT] != &si->sortTable->head) {
	    s = s->link[SORT_NODE_DIR_LEFT];
	}
    }
    si->curNode = s;
    return Boolean(s != &si->sortTable->head);
}

/*
 * Position iterator at previous item in list. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 */
Boolean
sortIterPrev(SortIter si)
{
    SortNode *s = si->curNode;
    SortNode *c;

    if (s == &si->sortTable->head) {
	return sortIterTail(si);
    }
    ASSERT(! si->sortTable->wasModified);
    do {
	c = s;
	s = s->p;
    } while (s != &si->sortTable->head && s->link[SORT_NODE_DIR_LEFT] == c);
    if (s != &si->sortTable->head) {
	s = s->link[SORT_NODE_DIR_LEFT];
	while (s->link[SORT_NODE_DIR_RIGHT] != &si->sortTable->head) {
	    s = s->link[SORT_NODE_DIR_RIGHT];
	}
    }
    si->curNode = s;
    return Boolean(s != &si->sortTable->head);
}

/*
 * Free an iterator
 */
extern void
sortIterFree(SortIter si)
{
    free(si);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 * INLINE_PRIVATE void
 * sortTableInlinePrivate(void)
 * {
 * }
 * 
 */

/*
 * Returns the value associated with key. Returns NULL if the key is not
 * found.
 */
static SortNode *
sortTableFind(const SortTable sortTable, const void *key)
{
    SortNode *s = sortTable->head.link[SORT_NODE_DIR_RIGHT];

    sortTable->head.item.key = key;	/* guarantee termination */
    for (;;) {
	int cmp = (*sortTable->keyCmp)(key, s->item.key);
	if (cmp == 0 && s->link[SORT_NODE_DIR_LEFT] == &sortTable->head) {
	    break;
	}
	s = s->link[cmp < 0];
    }
    return s;
}

static SortNode *
sortTableSplit(SortTable sortTable, const void *key, SortNode *s)
{
    SortNode *p = s->p;
    SortNodeDir cDir;
    SortNodeDir gcDir;

    s->isRed = TRUE;
    s->link[SORT_NODE_DIR_LEFT]->isRed = FALSE;
    s->link[SORT_NODE_DIR_RIGHT]->isRed = FALSE;
    if (p->isRed) {
	SortNode *g = p->p;
	SortNode *gg = g->p;
	g->isRed = TRUE;
	if (((*sortTable->keyCmp)(key, g->item.key) < 0)
		!= ((*sortTable->keyCmp)(key, p->item.key) < 0)) {
	    cDir = (p == g->link[SORT_NODE_DIR_LEFT])
		? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;
	    gcDir = (s == p->link[SORT_NODE_DIR_LEFT])
		? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;
	    p = sortTableRotate(g, cDir, gcDir);
	}
	cDir = (g == gg->link[SORT_NODE_DIR_LEFT])
		? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;
	gcDir = (p == g->link[SORT_NODE_DIR_LEFT])
		? SORT_NODE_DIR_LEFT : SORT_NODE_DIR_RIGHT;
	s = sortTableRotate(g->p, cDir, gcDir);
	s->isRed = FALSE;
    }
    sortTable->head.link[SORT_NODE_DIR_RIGHT]->isRed = FALSE;
    return s;
}

static SortNode *
sortTableRotate(SortNode *r, SortNodeDir cDir, SortNodeDir gcDir)
{
    SortNode *c;
    SortNode *gc; 

    c = r->link[cDir];
    gc = c->link[gcDir];

    c->link[gcDir] = gc->link[!gcDir];
    gc->link[!gcDir]->p = c;

    gc->link[!gcDir] = c;
    c->p = gc;

    r->link[cDir] = gc;
    gc->p = r;

    return gc;
}

static void
sortTableNodeFree(SortTable sortTable, SortNode *nodep)
{
    nodep->link[SORT_NODE_DIR_RIGHT] = sortTable->freeNodep;
    sortTable->freeNodep = nodep;
    nodep->hasData = FALSE;
    nodep->isFree = TRUE;
    nodep->isRed = FALSE;
}

#define	RELOCATE(sortTable, type, p, delta)			\
    BEGIN_STMT							\
	if ((p) != &(sortTable)->head) {			\
	    (p) = (type)(((char *)(p)) + (delta));		\
	}							\
    END_STMT

static SortNode *
sortTableNodeAlloc(SortTable sortTable, SortNode **reallocpp)
{
    SortNode *n;

    if (sortTable->freeNodep == NULL) {
	int newSize = (sortTable->size + 1) * 1.5;
	SortNode *newArray = RENEW(SortNode, sortTable->nodeArray, newSize);
	int ptrDelta = (char *)newArray - (char *)sortTable->nodeArray;
	int i;
	for (i = 0; i < sortTable->size; i++) {
	    SortNode *p = &newArray[i];
	    /* LINTED */
	    RELOCATE(sortTable, SortNode *, p->link[SORT_NODE_DIR_LEFT],
			ptrDelta);
	    /* LINTED */
	    RELOCATE(sortTable, SortNode *, p->link[SORT_NODE_DIR_RIGHT],
			ptrDelta);
	    /* LINTED */
	    RELOCATE(sortTable, SortNode *, p->p, ptrDelta);
	}
	if (reallocpp != NULL) {
	    /* LINTED */
	    RELOCATE(sortTable, SortNode *, *reallocpp, ptrDelta);
	}
	/* LINTED */
	RELOCATE(sortTable, SortNode *,
		sortTable->head.link[SORT_NODE_DIR_LEFT], ptrDelta);
	/* LINTED */
	RELOCATE(sortTable, SortNode *,
		sortTable->head.link[SORT_NODE_DIR_RIGHT], ptrDelta);
	/* LINTED */
	RELOCATE(sortTable, SortNode *, sortTable->head.p, ptrDelta);
	for (i = sortTable->size; i < newSize; i++) {
	    sortTableNodeFree(sortTable, &newArray[i]);
	}
	sortTable->nodeArray = newArray;
	sortTable->size = newSize;
    }
    n = sortTable->freeNodep;
    sortTable->freeNodep = n->link[SORT_NODE_DIR_RIGHT];
    n->isFree = FALSE;
    return n;
}

static void
sortTableTraverse(SortTable sortTable, int blackDepth, SortNode *n,
	SortNode *min, SortNode *max)
{
    SortNode *l = n->link[SORT_NODE_DIR_LEFT];
    SortNode *r = n->link[SORT_NODE_DIR_RIGHT];

    if (n->wasSeen) {
	ABORT("node multiple");
    }
    if (n->isFree) {
	ABORT("used free");
    }
    if (! n->isRed) {
	blackDepth += 1;
    }
    n->wasSeen = TRUE;
    if (min != NULL && (*sortTable->keyCmp)(n->item.key, min->item.key) < 0) {
	ABORT("low partition");
    }
    if (max != NULL && (*sortTable->keyCmp)(n->item.key, max->item.key) > 0) {
	ABORT("high partition");
    }
    if (l == &sortTable->head && r == &sortTable->head) {
	/* a leaf */
	if (! n->hasData || n->isRed) {
	    ABORT("corrupt leaf");
	}
	if (sortTable->blackDepth != -1
	    && sortTable->blackDepth != blackDepth) {
	    ABORT("black level");
	}
	sortTable->blackDepth = blackDepth;
    } else {
	/* an internal node */
	if (l == &sortTable->head || r == &sortTable->head) {
	    ABORT("missing child");
	}
	if (n->hasData) {
	    ABORT("internal data");
	}
	if (l->p != n || r->p != n) {
	    ABORT("bad parent");
	}
	if (n->isRed && (l->isRed || r->isRed)) {
	    ABORT("red length");
	}
	sortTableTraverse(sortTable, blackDepth, l, min, n);
	sortTableTraverse(sortTable, blackDepth, r, n, max);
    }
}

static void
sortTableTraversePrint(SortTable sortTable, Deque deque,
	SortTablePrintFunc pFunc, SortTableItemToStringFunc iFunc, int level,
	int maxWidth)
{
    char *buf = NEW(char, maxWidth + 2);
    char buf2[100];
    SortNode *n;
    int i;

    for (i = 0; i < maxWidth; i++) {
	buf[i] = ' ';
    }
    buf[maxWidth] = '\n';
    buf[maxWidth + 1] = '\0';
    while ((n = (SortNode*)dequeItemAt(deque, 0)) != NULL && n->level == level) {
	int startPoint;
	int midPoint;
	SortNode *l;
	SortNode *r;

	n = (SortNode*)dequeDequeueHead(deque);
	midPoint = (n->printHigh - n->printLow) / 2 + n->printLow;
	l = n->link[SORT_NODE_DIR_LEFT];
	r = n->link[SORT_NODE_DIR_RIGHT];

	(*iFunc)(&n->item, n->isRed, buf2, sizeof(buf2));
	startPoint = midPoint - strlen(buf2) / 2;
	if (startPoint > maxWidth) {
	    startPoint = maxWidth;
	}
	strInsert(&buf[startPoint], buf2, maxWidth - startPoint);
	if (l != &sortTable->head) {
	    l->printLow = n->printLow; 
	    l->printHigh = midPoint;
	    l->level = level + 1;
	    dequeEnqueueTail(deque, l);
	}
	if (r != &sortTable->head) {
	    r->printLow = midPoint; 
	    r->printHigh = n->printHigh;
	    r->level = level + 1;
	    dequeEnqueueTail(deque, r);
	}
    }
    (*pFunc)(buf);
    if (dequeLength(deque) != 0) {
	sortTableTraversePrint(sortTable, deque, pFunc, iFunc, level + 1,
	    maxWidth);
    }
    free(buf);
}

	
/*************************************************************************
 * Private Functions
 *************************************************************************/


static void
strInsert(char *dst, char *src, int maxlen)
{
    while (*src != '\0' && maxlen-- > 0) {
	*dst++ = *src++;
    }
}

static int
strKeyCmp(const void *key1, const void *key2)
{
    const char *s1 = (const char *)key1;
    const char *s2 = (const char *)key2;

    if(s1 == s2) {
	return(0);
    }
    while(*s1 == *s2++) {
	if(*s1++ == '\0') {
	    return(0);
	}
    }
    return(*s1 - s2[-1]);
}

/* ARGSUSED1 */
static const void *
strKeyDup(const void *key, const void *value)
{
    int keyLen = strlen((const char*)key) + 1;
    return memcpy(NEW(char, keyLen), key, keyLen);
}

static int
intKeyCmp(const void *key1, const void *key2)
{
    return (int)key1 - (int)key2;
}

#endif					   /* !defined(SORTTABLE_HEADER) */
