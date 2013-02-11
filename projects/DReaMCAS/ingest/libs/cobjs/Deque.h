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
 * $(@)Deque.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Deque.h -- A double-ended queue data structure.
 */

#ifndef	_COBJS_DEQUE_H
#define	_COBJS_DEQUE_H

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

typedef struct _Deque *Deque;
typedef struct _DequeIter *DequeIter;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(DEQUE_BODY)
#define	DEQUE_INLINE	static
#else	/* defined(DO_INLINING) && ! defined(DEQUE_BODY) */
#define	DEQUE_INLINE	extern
#endif	/* defined(DO_INLINING) && ! defined(DEQUE_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

/*
 * Create a deque.  Initialize size is initSize (if 0, a default
 * will be used),  if deque must be grown, it will be grown by
 * sizeIncr (if 0, a value proportional to the current size
 * will be used).
 */
extern Deque       dequeNew(int initSize, int sizeIncr);

/***********************************************************************
 * Instance Interface
 ***********************************************************************/

/*
 * Enqueues item on head of deque.
 */
DEQUE_INLINE void         dequeEnqueueHead(Deque deque, void *item);

/*
 * Enqueues item on tail of deque.
 */
DEQUE_INLINE void         dequeEnqueueTail(Deque deque, void *item);

/*
 * Dequeues item from head of deque, returns NULL if deque is empty.
 */
DEQUE_INLINE void        *dequeDequeueHead(Deque deque);

/*
 * Dequeues item from tail of deque, returns NULL if deque is empty.
 */
DEQUE_INLINE void        *dequeDequeueTail(Deque deque);

/*
 * Returns item at index position in deque.  Index 0 refers to head,
 * index dequeLength() - 1 refers to tail.
 *
 * Returns NULL if index outside of deque.
 */
DEQUE_INLINE void        *dequeItemAt(Deque deque, int index);

/*
 * Stores item into deque at index position.  Overwrites item previously at
 * index position.  Index 0 refers to head, index dequeLength() - 1 refers
 * to tail.
 *
 * Aborts if index outside of deque.
 */
DEQUE_INLINE void        dequeSetItemAt(Deque deque, int index, void *item);

/*
 * Returns value at head (tail) of deque without dequeue of item.
 */
DEQUE_INLINE void	*dequeHead(Deque deque);
DEQUE_INLINE void	*dequeTail(Deque deque);

/*
 * Returns number of items in deque
 */
DEQUE_INLINE int          dequeLength(Deque deque);

/*
 * Returns TRUE if item is on deque, FALSE otherwise.
 */
extern Boolean dequeIsMember(Deque deque, void *item);

/*
 * Finds and removes item from deque.
 *
 * NOTE: This is expensive.
 */
extern Boolean       dequeDelete(Deque deque, void *item);

/*
 * Frees Deque object.  Does NOT free current contents of deque.
 */
extern void         dequeFree(Deque deque);

/************************************************************************
 * OBJECT DequeIter Class Interface
 ************************************************************************/

/*
 * Create a deque Iterator
 */
extern DequeIter     dequeIterNew(const Deque deque);

/************************************************************************
 * OBJECT DequeIter Instance Interface
 ************************************************************************/

/*
 * Position iterator at deque head Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUE_INLINE Boolean dequeIterHead(DequeIter di);

/*
 * Position iterator at deque tail Returns TRUE if deque is non-empty; FALSE if
 * deque is empty.
 */
DEQUE_INLINE Boolean dequeIterTail(DequeIter di);

/*
 * Position iterator at next item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterNext moves from head towards tail.
 */
DEQUE_INLINE Boolean dequeIterNext(DequeIter di);

/*
 * Position iterator at previous item in deque. Return TRUE if there is a next
 * item; FALSE if wrapping around.
 *
 * NOTE: dequeIterPrev moves from tail towards head.
 */
DEQUE_INLINE Boolean dequeIterPrev(DequeIter di);

/*
 * Return the item referenced by the iterator Returns NULL if iterator not
 * positioned at item.
 */
DEQUE_INLINE void   *dequeIterItem(const DequeIter di);

/*
 * Return TRUE if iterator points to valid item
 */
DEQUE_INLINE Boolean dequeIterIsValid(const DequeIter di);

/*
 * Free an iterator
 */
extern void         dequeIterFree(DequeIter di);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && !defined(DEQUE_BODY)
#define	DEQUE_HEADER
#include "cobjs/Deque.c"
#undef	DEQUE_HEADER
#endif	/* defined(DO_INLINING) && !defined(DEQUE_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_DEQUE_H */
