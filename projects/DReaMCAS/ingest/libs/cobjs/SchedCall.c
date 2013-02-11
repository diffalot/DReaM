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
 * $(@)SchedCall.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:35 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)SchedCall.c 1.2	98/12/18 SMI"

/*
 * SchedCall.c -- SchedCall is an object which manages future callbacks.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <synch.h>
#include <thread.h>
#include <sys/time.h>

#include "cobjs/PriQueue.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Thread.h"
#include "cobjs/Time.h"
#include "cobjs/Types.h"

#include "SchedCall.h"

#define SCHED_CALL_INITIAL_QUEUE_LEN	20

/*
 * Private types
 */
typedef struct SchedCallItem {
    SchedCallHandler    handler;
    Time	        date;
    Object              delegate;
} SchedCallItem;

/*
 * OBJECT SchedCall Instance Variables
 */
struct _SchedCall {
    mutex_t             mutex;
    cond_t              queueCond;
    cond_t              scipCond;
    PriQueue            priQueue;
    Boolean             isActive;
    Time                accuracy;
    Boolean		isRealtime;
    int			priority;
    SchedCallItem	*curScip;
    thread_t		threadId;
};

/*
 * Private function prototypes
 */
static Boolean      schedCallInOrder(const void *item1p, const void *item2p);
static void        *schedCallThread(void *arg);

/*
 * Private data
 */
static SchedCall    globalSc;

/*
 * OBJECT Template Class Methods
 */

/*
 * Initialize global SchedCall object.
 */
void
schedCallInit(Time accuracy, Boolean isRealtime, int priority)
{
    ABORT_IF_FALSE(globalSc == NULL);
    globalSc = schedCallNew(accuracy, isRealtime, priority);
}

/*
 * Shutdown global SchedCall object.
 */
void
schedCallFini(void)
{
    if (globalSc != NULL) {
	schedCallFree(globalSc);
	globalSc = NULL;
    }
}

SchedCall
schedCallNew(Time accuracy, Boolean isRealtime, int priority)
{
    SchedCall           sc;

    ASSERT(accuracy >= 0);
    sc = NEW_ZEROED(struct _SchedCall, 1);
    sc->priQueue = priQueueNew(SCHED_CALL_INITIAL_QUEUE_LEN, schedCallInOrder);
    sc->isRealtime = isRealtime;
    sc->priority = priority;
    ABORT_IF_ERRNO(mutex_init(&sc->mutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&sc->queueCond, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&sc->scipCond, USYNC_THREAD, 0));
    sc->isActive = TRUE;
    sc->accuracy = accuracy;
    sc->curScip = NULL;
    ABORT_IF_ERRNO(thr_create(NULL, 0, schedCallThread, sc,
	THR_BOUND, &sc->threadId));
    return sc;
}

/*
 * OBJECT Template Instance Methods
 */
SchedCallTag
schedCallAdd(SchedCall sc, Time date, SchedCallHandler handler, Object delegate)
{
    SchedCallItem      *scip;

    if (sc == NULL) {
	ABORT_IF_FALSE(globalSc != NULL);
	sc = globalSc;
    }
    if ((scip = NEW_ZEROED(SchedCallItem, 1)) == NULL) {
	return NULL;
    }
    if (sc->accuracy != 0) {
	date = ROUND_UP(date, sc->accuracy);
    }
    scip->date = date;
    scip->handler = handler;
    scip->delegate = delegate;
    CHECK_IF_ERRNO(mutex_lock(&sc->mutex));
    priQueueInsert(sc->priQueue, scip);
    CHECK_IF_ERRNO(cond_signal(&sc->queueCond));
    CHECK_IF_ERRNO(mutex_unlock(&sc->mutex));
    return scip;
}

Boolean
schedCallRemove(SchedCall sc, SchedCallTag tag)
{
    SchedCallItem      *scip = NULL;

    if (sc == NULL) {
	ABORT_IF_FALSE(globalSc != NULL);
	sc = globalSc;
    }
    ASSERT(tag != NULL);
    CHECK_IF_ERRNO(mutex_lock(&sc->mutex));
    if (priQueueDelete(sc->priQueue, tag)) {
	scip = (SchedCallItem *)tag;
    } else while (sc->curScip == (SchedCallItem *)tag) {
	CHECK_IF_ERRNO(cond_wait(&sc->scipCond, &sc->mutex));
    }
    CHECK_IF_ERRNO(cond_signal(&sc->queueCond));
    CHECK_IF_ERRNO(mutex_unlock(&sc->mutex));
    if (scip != NULL) {
	free(scip);
    }
    return Boolean(scip != NULL);
}

void
schedCallFree(SchedCall sc)
{
    SchedCallItem      *scip;

    CHECK_IF_ERRNO(mutex_lock(&sc->mutex));
    sc->isActive = FALSE;
    CHECK_IF_ERRNO(cond_signal(&sc->queueCond));
    CHECK_IF_ERRNO(mutex_unlock(&sc->mutex));

    CHECK_IF_ERRNO(thr_join(sc->threadId, NULL, NULL));
    while ((scip = (SchedCallItem*)priQueueRemove(sc->priQueue)) != NULL) {
	free(scip);
    }
    priQueueFree(sc->priQueue);
    ABORT_IF_ERRNO(cond_destroy(&sc->queueCond));
    ABORT_IF_ERRNO(mutex_destroy(&sc->mutex));
    free(sc);
}

/*
 * OBJECT Template Private Functions
 */
static void        *
schedCallThread(void *arg)
{
    SchedCall           sc = (SchedCall) arg;
    SchedCallItem      *scip;

    if (sc->isRealtime) {
	threadSetRtPriority(sc->priority);
    }
    CHECK_IF_ERRNO(mutex_lock(&sc->mutex));
    while (sc->isActive) {

	scip = (SchedCallItem*)priQueueHead(sc->priQueue);

	if (scip == NULL) {
	    CHECK_IF_ERRNO(cond_wait(&sc->queueCond, &sc->mutex));
	} else {
	    int                 err;
	    timestruc_t         waitDate = timeToTimestruc(scip->date);
	    Time		tod;

	    err = cond_timedwait(&sc->queueCond, &sc->mutex, &waitDate);
	    tod = timeOfDay();
	    switch (err) {
	    case 0:
	    case EINTR:
		break;
	    case ETIME:
		while ((scip = (SchedCallItem*)priQueueHead(sc->priQueue)) != NULL
			&& scip->date <= tod) {
		    scip = (SchedCallItem*)priQueueRemove(sc->priQueue);
		    sc->curScip = scip;
		    CHECK_IF_ERRNO(mutex_unlock(&sc->mutex));
		    (*scip->handler) (scip->delegate, scip->date);
		    free(scip);
		    CHECK_IF_ERRNO(mutex_lock(&sc->mutex));
		    sc->curScip = NULL;
		    CHECK_IF_ERRNO(cond_signal(&sc->scipCond));
		}
		break;
	    default:
		ABORT("schedCall");
		break;
	    }
	}
    }
    CHECK_IF_ERRNO(mutex_unlock(&sc->mutex));
    return NULL;
}

static Boolean
schedCallInOrder(const void *item1p, const void *item2p)
{
    const SchedCallItem *sciItem1p = (SchedCallItem *) item1p;
    const SchedCallItem *sciItem2p = (SchedCallItem *) item2p;

    return Boolean(sciItem1p->date <= sciItem2p->date);
}
