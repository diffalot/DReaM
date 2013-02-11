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
 * $(@)DequeOf.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * DequeOf.h -- A double-ended queue data structure.
 */

#ifndef	_COBJS_DEQUEOF_H
#define	_COBJS_DEQUEOF_H

#pragma ident "@(#)Deque.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _DequeOf *DequeOf;
typedef struct _DequeOfIter *DequeOfIter;

typedef Boolean (*DequeOfIsEqualFunc)(const void *item1p, const void *item2p,
	size_t itemLen);

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(DEQUEOF_BODY)
#define	DEQUEOF_INLINE	static
#else	/* defined(DO_INLINING) && ! defined(DEQUEOF_BODY) */
#define	DEQUEOF_INLINE	extern
#endif	/* defined(DO_INLINING) && ! defined(DEQUEOF_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

/*
 * Create a deque.  Initialize size is initSize (if 0, a default
 * will be used),  if deque must be grown, it will be grown by
 * sizeIncr (if 0, a value proportional to the current size
 * will be used). isEqualFunc is used to compare to deque items for
 * equality.  If null, a bitwise compare is used.
 */
extern DequeOf
dequeOfNew(size_t itemSize, int initSize, int sizeIncr,
	DequeOfIsEqualFunc isEqualFunc);

#define	NEW_DEQUEOF(type, initSize, sizeIncr, isEqualFunc)	    \
	dequeOfNew(sizeof(type), initSize, sizeIncr, isEqualFunc)

/***********************************************************************
 * Instance Interface
 ***********************************************************************/

/*
 * Enqueues item on head of deque.
 */
DEQUEOF_INLINE void         _dequeOfEnqueueHead(DequeOf dequeOf, void *item);

#define	dequeOfEnqueueHead(dequeOf, item) _dequeOfEnqueueHead(dequeOf, &(item))

/*
 * Enqueues item on tail of deque.
 */
DEQUEOF_INLINE void         _dequeOfEnqueueTail(DequeOf dequeOf, void *item);

#define	dequeOfEnqueueTail(dequeOf, item) _dequeOfEnqueueTail(dequeOf, &(item))

/*
 * Dequeues item from head of deque, returns NULL if deque is empty.
 */
DEQUEOF_INLINE void        *_dequeOfDequeueHead(DequeOf dequeOf);

#define	dequeOfDequeueHead(dequeOf, type)			\
	(*(type *) _dequeOfDequeueHead(dequeOf))

/*
 * Dequeues item from tail of deque, returns NULL if deque is empty.
 */
DEQUEOF_INLINE void        *_dequeOfDequeueTail(DequeOf dequeOf);

#define	dequeOfDequeueTail(dequeOf, type)			\
	(*(type *) _dequeOfDequeueTail(dequeOf))

/*
 * Returns item at index position in deque.  Index 0 refers to head,
 * index dequeLength() - 1 refers to tail.
 *
 * Returns NULL if index outside of deque.
 */
DEQUEOF_INLINE void        *_dequeOfItemAt(DequeOf dequeOf, int index);

#define	dequeOfItemAt(dequeOf, type, index)			\
	(*(type *) _dequeOfItemAt(dequeOf, index))

/*
 * Stores item into deque at index position.  Overwrites item previously at
 * index position.  Index 0 refers to head, index dequeOfLength() - 1 refers
 * to tail.
 *
 * Aborts if index outside of deque.
 */
DEQUEOF_INLINE void
_dequeOfSetItemAt(DequeOf dequeOf, int index, void *item);

#define	dequeOfSetItemAt(dequeOf, index, item)		\
	_dequeOfSetItemAt(dequeOf, index, &(item))

/*
 * Returns value at head (tail) of deque without dequeue of item.
 */
DEQUEOF_INLINE void	*_dequeOfHead(DequeOf dequeOf);
DEQUEOF_INLINE void	*_dequeOfTail(DequeOf dequeOf);

#define	dequeOfHead(dequeOf, type)			\
	(*(type *) _dequeOfHead(dequeOf))

#define	dequeOfTail(dequeOf, type)			\
	(*(type *) _dequeOfTail(dequeOf))

/*
 * Returns number of items in deque
 */
DEQUEOF_INLINE int          dequeOfLength(DequeOf dequeOf);

/*
 * Returns TRUE if item is on deque, FALSE otherwise.
 */
extern Boolean _dequeOfIsMember(DequeOf dequeOf, void *item);

#define	dequeOfIsMember(dequeOf, item)	_dequeOfIsMember(dequeOf, &(item))

/*
 * Finds item from deque and returns (but does not remove).
 *
 * NOTE: This is expensive.
 */
extern void	    *_dequeOfFind(DequeOf dequeOf, void *item);

#define	dequeOfFind(dequeOf, type, item)			\
	(*(type *) _dequeOfFind(dequeOf, &(item)))

/*
 * Finds and removes item from deque.
 *
 * NOTE: This is expensive.
 */
extern Boolean       _dequeOfDelete(DequeOf dequeOf, void *item);

#define	dequeOfDelete(dequeOf, item)	_dequeOfDelete(dequeOf, &(item))

/*
 * Frees DequeOf object.  Does NOT free current contents of deque.
 */
extern void         dequeOfFree(DequeOf dequeOf);

/************************************************************************
 * OBJECT DequeOfIter Class Interface
 ************************************************************************/

/*
 * Create a deque Iterator
 */
extern DequeOfIter     dequeOfIterNew(const DequeOf dequeOf);

/************************************************************************
 * OBJECT DequeOfIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at deque head Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUEOF_INLINE Boolean dequeOfIterHead(DequeOfIter di);

/*
 * Position iterator at deque tail Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUEOF_INLINE Boolean dequeOfIterTail(DequeOfIter di);

/*
 * Position iterator at next item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterNext moves from head towards tail.
 */
DEQUEOF_INLINE Boolean dequeOfIterNext(DequeOfIter di);

/*
 * Position iterator at previous item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterPrev moves from tail towards head.
 */
DEQUEOF_INLINE Boolean dequeOfIterPrev(DequeOfIter di);

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
DEQUEOF_INLINE void   *_dequeOfIterItem(const DequeOfIter di);

#define	dequeOfIterItem(di, type)   (*(type *) _dequeOfIterItem(di))

/*
 * Return TRUE if iterator points to valid item
 */
DEQUEOF_INLINE Boolean dequeOfIterIsValid(const DequeOfIter di);

/*
 * Free an iterator
 */
extern void         dequeOfIterFree(DequeOfIter di);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && !defined(DEQUEOF_BODY)
#define	DEQUEOF_HEADER
#include "cobjs/DequeOf.c"
#undef	DEQUEOF_HEADER
#endif	/* defined(DO_INLINING) && !defined(DEQUEOF_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_DEQUEOF_H */
