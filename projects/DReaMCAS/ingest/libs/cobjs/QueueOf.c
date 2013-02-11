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
 * $(@)QueueOf.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)QueueOf.c 1.3	99/10/27 SMI"

#include <errno.h>
#include <stdlib.h>
#include <synch.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/QueueOf.h"

/*
 * OBJECT QueueOf Instance Variables
 */
struct _QueueOf {
    void              **items;
    int                 len;
    size_t              itemSize;
    int                 in;
    int                 out;
    mutex_t             mutex;
    cond_t              inAvailCond;
    cond_t              outAvailCond;
    Boolean             isShutdown;
};

static Boolean      _queueOfIsFull(QueueOf queue);
static Boolean      _queueOfIsEmpty(QueueOf queue);

/*
 * OBJECT QueueOf Class Interface
 */
QueueOf
queueOfNew(size_t itemSize, int maxLen)
{
    QueueOf             queue = NEW(struct _QueueOf, 1);

    queue->len = maxLen + 1;
    queue->itemSize = itemSize;
    queue->items = (void **)CALLOC(queue->len, queue->itemSize);
    queue->in = 0;
    queue->out = 0;
    queue->isShutdown = FALSE;

    ABORT_IF_ERRNO(mutex_init(&queue->mutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&queue->inAvailCond, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&queue->outAvailCond, USYNC_THREAD, 0));

    return queue;
}

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
Boolean
queueOfEnqueue(QueueOf queue, void *item, timestruc_t *timeoutDatep)
{
    Boolean             didEnqueue = FALSE;
    int                 error = 0;

    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    while (_queueOfIsFull(queue) && !queue->isShutdown && error != ETIME) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&queue->inAvailCond, &queue->mutex, timeoutDatep)
	  : cond_wait(&queue->inAvailCond, &queue->mutex);
	if (error != 0 && error != ETIME && error != EINTR) {
	    ABORT_WITH_ERRNO(error, "cond_timedwait");
	}
    }
    if (! _queueOfIsFull(queue) && ! queue->isShutdown) {
	void               *queueItem;

	CHECK_IF_ERRNO(cond_broadcast(&queue->outAvailCond));
	queueItem = ((char *) (queue->items)) + (queue->in++ * queue->itemSize);
	(void) memcpy(queueItem, item, queue->itemSize);
	if (queue->in >= queue->len) {
	    queue->in = 0;
	}
	didEnqueue = TRUE;
    }
    CHECK_IF_ERRNO(mutex_unlock(&queue->mutex));
    return didEnqueue;
}

/*
 * Dequeues item from queue, blocks if no items are present until
 * timeoutDate.  timeoutDatep == NULL implies block until item available or
 * queue shutdown is done. Returns TRUE if item dequeued. Returns FALSE if
 * timeout or queue shutdown.
 *
 * Returns FALSE if timeout or queue shutdown and empty.
 */
Boolean
queueOfDequeue(QueueOf queue, void *item, timestruc_t *timeoutDatep)
{
    /*
     * Dequeues item from queue, blocks if not items are present.
     */
    int                 error = 0;
    Boolean             retVal = FALSE;

    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    while (_queueOfIsEmpty(queue) && !queue->isShutdown && error != ETIME) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&queue->outAvailCond, &queue->mutex, timeoutDatep)
	  : cond_wait(&queue->outAvailCond, &queue->mutex);
	if (error != 0 && error != ETIME && error != EINTR) {
	    ABORT_WITH_ERRNO(error, "cond_timedwait");
	}
    }
    if (! _queueOfIsEmpty(queue)) {
	void               *queueItem;

	queueItem = ((char *) (queue->items))
	    + (queue->out++ * queue->itemSize);
	(void) memcpy(item, queueItem, queue->itemSize);
	retVal = TRUE;
	if (queue->out >= queue->len) {
	    queue->out = 0;
	}
	CHECK_IF_ERRNO(cond_broadcast(&queue->inAvailCond));
    }
    CHECK_IF_ERRNO(mutex_unlock(&queue->mutex));
    return retVal;
}

void
queueOfShutdown(QueueOf queue)
{
    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    queue->isShutdown = TRUE;
    CHECK_IF_ERRNO(cond_broadcast(&queue->outAvailCond));
    CHECK_IF_ERRNO(cond_broadcast(&queue->inAvailCond));
    CHECK_IF_ERRNO(mutex_unlock(&queue->mutex));
}

Boolean
queueOfIsActive(QueueOf queue)
{
    return Boolean(!queue->isShutdown);
}

void
queueOfFree(QueueOf queue)
{
    ASSERT(queue->in == queue->out);
    ABORT_IF_ERRNO(mutex_destroy(&queue->mutex));
    ABORT_IF_ERRNO(cond_destroy(&queue->inAvailCond));
    ABORT_IF_ERRNO(cond_destroy(&queue->outAvailCond));
    free(queue->items);
    free(queue);
}

static Boolean
_queueOfIsFull(QueueOf queue)
{
    /*
     * Returns TRUE if queue is full, FALSE otherwise.
     */
    return Boolean(((queue->in + 1) % queue->len) == queue->out);
}

static Boolean
_queueOfIsEmpty(QueueOf queue)
{
    /*
     * Returns TRUE if the queue is empty, FALSE otherwise.
     */
    return Boolean(queue->in == queue->out);
}
