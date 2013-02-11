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
 * $(@)PriQueue.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * PriQueue.h -- Description
 */

#ifndef	_COBJS_PRIQUEUE_H
#define	_COBJS_PRIQUEUE_H

#pragma ident "@(#)PriQueue.h 1.1	98/10/22 SMI"

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

typedef struct _PriQueue *PriQueue;

/***********************************************************************
 * Public Types
 ***********************************************************************/

/*
 * Returns TRUE if *item1p is higher priority than *itemp2.
 */
typedef Boolean (*PriQueueInOrderFunc)(const void *item1p, const void *item2p);

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PRIQUEUE_BODY)
#define	PRIQUEUE_INLINE	static
#else	/* defined(DO_INLINING) && ! defined(PRIQUEUE_BODY) */
#define	PRIQUEUE_INLINE	extern
#endif	/* defined(DO_INLINING) && ! defined(PRIQUEUE_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern PriQueue
priQueueNew(int initLength,
	    PriQueueInOrderFunc inOrderFunc);

/***********************************************************************
 * Instance Interface
 ***********************************************************************/

/*
 * Insert item into priority queue
 */
extern void priQueueInsert(PriQueue priQueue, void *item);

/*
 * Remove and return highest priority item.  If queue is empty, return NULL.
 */
extern void *priQueueRemove(PriQueue priQueue);

/*
 * Return current highest priority item, but do NOT remove.
 */
extern void *priQueueHead(PriQueue priQueue);

/*
 * Insert item into priority queue, then remove and return highest priority
 * item (possibly item just inserted).
 */
extern void *priQueueReplace(PriQueue priQueue, void *item);

/*
 * Return the number of items in the priority queue.
 */
extern int priQueueLength(PriQueue priQueue);

/*
 * Returns TRUE if item is on PriQueue, FALSE otherwise
 */
extern Boolean priQueueIsMember(PriQueue pq, void *item);

/*
 * Delete given item from the priority queue.
 */
extern Boolean priQueueDelete(PriQueue priQueue, void *item);

/*
 * Free the priority queue.
 */
extern void         priQueueFree(PriQueue priQueue);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PRIQUEUE_BODY)
#define	PRIQUEUE_HEADER
#include "cobjs/PriQueue.c"
#undef	PRIQUEUE_HEADER
#endif	defined(DO_INLINING) && ! defined(PRIQUEUE_BODY)

_FOREIGN_END

#endif					   /* _COBJS_PRIQUEUE_H */
