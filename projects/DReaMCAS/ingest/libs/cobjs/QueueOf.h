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
 * $(@)QueueOf.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * OBJECT QueueOf -- A queue is a MT-safe value fifo.
 */
#ifndef	_COBJS_QUEUEOF_H
#define	_COBJS_QUEUEOF_H

#pragma ident "@(#)QueueOf.h 1.2	99/10/27 SMI"

#include <time.h>

#include "cobjs/Types.h"
#include "cobjs/Foreign.h"

_FOREIGN_START

/*
 * OBJECT QueueOf Instance Type
 */
typedef struct _QueueOf *QueueOf;

/*
 * OBJECT QueueOf Class Interface
 */

/*
 * Create a queue that will hold up to maxLen items.  Attempts to enqueue
 * more than maxLen items will block until items are dequeued.
 */
extern QueueOf      queueOfNew(size_t itemSize, int maxLen);

#define	NEW_QUEUE(type, maxLen)	queueOfNew(sizeof(type), maxLen)

/*
 * OBJECT QueueOf Instance Interface
 */

/*
 * Enqueues item on queue.  If queue is full, will block until room available
 * in queue or until the timeoutDate occurs.  timeoutDatep == NULL implies
 * block until successful.
 * 
 * Attempting to enqueue NULL is an error.
 * 
 * Returns TRUE if enqueue successful, FALSE if timeout or queue is shutdown.
 */
extern Boolean
queueOfEnqueue(QueueOf queue, void *item,
	       timestruc_t *timeoutDatep);

/*
 * Dequeues item from queue, blocks if no items are present until
 * timeoutDate.  timeoutDatep == NULL implies block until item available or
 * queue shutdown is done.
 * 
 * Returns TRUE if item dequeued.
 * Returns FALSE if timeout or queue shutdown and empty.
 */
extern Boolean
queueOfDequeue(QueueOf queue, void *item,
	       timestruc_t *timeoutDatep);

/*
 * Shutdown queue.  After shutdown all enqueues are rejected and
 * dequeues return NULL and do not block when queue is empty.
 */
extern void         queueOfShutdown(QueueOf queue);

/*
 * Returns TRUE if queue is not shutdown.
 */
extern Boolean      queueOfIsActive(QueueOf queue);

/*
 * Frees QueueOf object.  Error if queue is not empty.
 */
extern void         queueOfFree(QueueOf queue);

_FOREIGN_END

#endif					   /* _COBJS_QUEUEOF_H */
