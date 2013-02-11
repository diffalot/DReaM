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
 * $(@)Queue.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)Queue.c 1.3	99/10/27 SMI"

#include <errno.h>
#include <stdlib.h>
#include <synch.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/Queue.h"

/*
 * OBJECT Queue Instance Variables
 */
struct _Queue {
    void              **items;
    int                 len;
    int                 in;
    int                 out;
    mutex_t             mutex;
    cond_t              inAvailCond;
    cond_t              outAvailCond;
    Boolean             isShutdown;
};

static Boolean      _queueIsFull(Queue queue);
static Boolean      _queueIsEmpty(Queue queue);

/*
 * OBJECT Queue Class Interface
 */
Queue
queueNew(int maxLen)
{
    Queue               queue = NEW(struct _Queue, 1);

    queue->len = maxLen + 1;
    queue->items = NEW(void *, queue->len);
    queue->in = 0;
    queue->out = 0;
    queue->isShutdown = FALSE;

    ABORT_IF_ERRNO(mutex_init(&queue->mutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&queue->inAvailCond, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&queue->outAvailCond, USYNC_THREAD, 0));

    return queue;
}

/*
 * OBJECT Queue Instance Interface
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
queueEnqueue(Queue queue, void *item, timestruc_t *timeoutDatep)
{
    Boolean             didEnqueue = FALSE;
    int                 error = 0;

    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    while (_queueIsFull(queue) && !queue->isShutdown && error != ETIME) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&queue->inAvailCond, &queue->mutex, timeoutDatep)
	  : cond_wait(&queue->inAvailCond, &queue->mutex);
	if (error != 0 && error != ETIME && error != EINTR) {
	    ABORT_WITH_ERRNO(error, "cond_timedwait");
	}
    }
    if (! _queueIsFull(queue) && ! queue->isShutdown) {
	CHECK_IF_ERRNO(cond_broadcast(&queue->outAvailCond));
	queue->items[queue->in++] = item;
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
 * queue shutdown is done.
 * 
 * Returns NULL if timeout or queue shutdown and empty.
 */
void               *
queueDequeue(Queue queue, timestruc_t *timeoutDatep)
{
    /*
     * Dequeues item from queue, blocks if not items are present.
     */
    void               *item = NULL;
    int                 error = 0;

    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    while (_queueIsEmpty(queue) && !queue->isShutdown && error != ETIME) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&queue->outAvailCond, &queue->mutex, timeoutDatep)
	  : cond_wait(&queue->outAvailCond, &queue->mutex);
	if (error != 0 && error != ETIME && error != EINTR) {
	    ABORT_WITH_ERRNO(error, "cond_timedwait");
	}
    }
    if (! _queueIsEmpty(queue)) {
	item = queue->items[queue->out++];
	if (queue->out >= queue->len) {
	    queue->out = 0;
	}
	CHECK_IF_ERRNO(cond_broadcast(&queue->inAvailCond));
    }
    CHECK_IF_ERRNO(mutex_unlock(&queue->mutex));
    return item;
}

void
queueShutdown(Queue queue)
{
    CHECK_IF_ERRNO(mutex_lock(&queue->mutex));
    queue->isShutdown = TRUE;
    CHECK_IF_ERRNO(cond_broadcast(&queue->outAvailCond));
    CHECK_IF_ERRNO(cond_broadcast(&queue->inAvailCond));
    CHECK_IF_ERRNO(mutex_unlock(&queue->mutex));
}

Boolean
queueIsActive(Queue queue)
{
    return Boolean(!queue->isShutdown);
}

void
queueFree(Queue queue)
{
    ASSERT(queue->in == queue->out);
    ABORT_IF_ERRNO(mutex_destroy(&queue->mutex));
    ABORT_IF_ERRNO(cond_destroy(&queue->inAvailCond));
    ABORT_IF_ERRNO(cond_destroy(&queue->outAvailCond));
    free(queue->items);
    free(queue);
}

static Boolean
_queueIsFull(Queue queue)
{
    /*
     * Returns TRUE if queue is full, FALSE otherwise.
     */
    return Boolean(((queue->in + 1) % queue->len) == queue->out);
}

static Boolean
_queueIsEmpty(Queue queue)
{
    /*
     * Returns TRUE if the queue is empty, FALSE otherwise.
     */
    return Boolean(queue->in == queue->out);
}
