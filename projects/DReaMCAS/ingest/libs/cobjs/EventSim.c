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
 * $(@)EventSim.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)EventSim.c 1.2	99/03/22 SMI"

/*
 * EventSim.c -- Event simulation framework object
 */

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <thread.h>
#include <synch.h>
#include <stdlib.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/PriQueue.h"
#include "cobjs/Types.h"
#include "cobjs/Time.h"

#include "cobjs/EventSim.h"

/*************************************************************************
 * OBJECT EventSim Instance Variables
 *************************************************************************/

struct _EventSim {
    mutex_t             lock;		   /* locks sim and all actors */
    PriQueue            delayQueue;	   /* queue of all blocked actors */

    cond_t              runCond;	   /* start/stop condition */
    Boolean             shouldRun;	   /* sim should run cmd */
    Boolean             isRunning;	   /* sim running status */
    Time                nowTime;	   /* current sim time */
    Time                stopTime;	   /* time when sim stops */

    cond_t              realTimeCond;	   /* realtime sim condition */
    double              realTimeFrac;	   /* sim rate control */
    Time                realTimeBase;	   /* real time base */
    Time                simTimeBase;	   /* sim time base */
};

/*************************************************************************
 * OBJECT EventSim Private types
 *************************************************************************/

typedef enum EventQueueType {
    EVENT_QUEUE_RUNNING,		   /* Actor running */
    EVENT_QUEUE_DELAY,			   /* Actor delaying */
    EVENT_QUEUE_WAIT			   /* Actor waiting for signal */
} EventQueueType;

struct _EventActor {
    EventSim            es;		   /* parent sim environment */

    EventBody           body;		   /* Actor body */
    void               *bodyArg;	   /* arg to actor body */

    cond_t              delayCond;	   /* wakeup condition */
    Time                wakeupTime;	   /* end of delay time */
    EventQueueType      queueType;	   /* actor state */

    cond_t              liveCond;	   /* live condition */
    Boolean             shouldLive;	   /* eventActor should live */
    Boolean             isLive;		   /* eventActor is live */
    thread_t            threadId;	   /* actor thread id */
};

/*************************************************************************
 * OBJECT EventSim Private method prototypes
 *************************************************************************/

static void         eventSimStart(EventSim es, Boolean shouldRun);

/*************************************************************************
 * OBJECT EventActor Private method prototypes
 *************************************************************************/

static Time
eventActorYield(EventActor ea, EventQueueType queueType, Time delayTime);

static void         eventActorSwitch(EventActor ea);

/*************************************************************************
 * OBJECT EventSim Private function prototypes
 *************************************************************************/

static void        *eventActor(void *arg);
static Boolean      eventSimInOrder(const void *item1p, const void *item2p);

/*************************************************************************
 * OBJECT EventSim Class Methods
 *************************************************************************/

EventSim
eventSimNew(void)
{
    EventSim            es = NEW_ZEROED(struct _EventSim, 1);

    ABORT_IF_ERRNO(mutex_init(&es->lock, USYNC_THREAD, 0));
    es->delayQueue = priQueueNew(400, eventSimInOrder);

    ABORT_IF_ERRNO(cond_init(&es->runCond, USYNC_THREAD, 0));
    es->shouldRun = FALSE;
    es->isRunning = FALSE;
    es->nowTime = 0LL;
    es->stopTime = EVENT_TIME_FOREVER;

    ABORT_IF_ERRNO(cond_init(&es->realTimeCond, USYNC_THREAD, 0));
    es->realTimeFrac = 0.0;

    return es;
}

/*************************************************************************
 * OBJECT EventSim Instance Methods
 *************************************************************************/

void
eventSimFree(EventSim es)
{
    ASSERT(!es->isRunning && !es->shouldRun);

    ABORT_IF_ERRNO(cond_destroy(&es->realTimeCond));
    ABORT_IF_ERRNO(cond_destroy(&es->runCond));

    ASSERT(priQueueLength(es->delayQueue) == 0);
    priQueueFree(es->delayQueue);

    ABORT_IF_ERRNO(mutex_destroy(&es->lock));

    free(es);
}

double
eventSimSetRealTimeFrac(EventSim es, double realTimeFrac)
{
    double              oldRealTimeFrac;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    oldRealTimeFrac = es->realTimeFrac;
    es->realTimeFrac = realTimeFrac;
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return oldRealTimeFrac;
}

Time
eventSimSetStopTime(EventSim es, Time stopTime)
{
    Time           oldStopTime;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    oldStopTime = es->stopTime;
    es->stopTime = stopTime;
    if (es->stopTime < es->nowTime) {
	CHECK_IF_ERRNO(cond_signal(&es->realTimeCond));
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return oldStopTime;
}

Time
eventSimNow(EventSim es)
{
    Time           nowTime;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    nowTime = es->nowTime;
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return nowTime;
}

void
eventSimRun(EventSim es)
{
    eventSimStart(es, TRUE);
}

void
eventSimStep(EventSim es)
{
    eventSimStart(es, FALSE);
}

void
eventSimStop(EventSim es)
{
    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    es->shouldRun = FALSE;
    while (es->isRunning) {
	CHECK_IF_ERRNO(cond_wait(&es->runCond, &es->lock));
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
}

void
eventSimWaitStopped(EventSim es)
{
    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    while (es->isRunning) {
	CHECK_IF_ERRNO(cond_wait(&es->runCond, &es->lock));
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
}

/*************************************************************************
 * OBJECT EventActor Class Methods
 *************************************************************************/

EventActor
eventActorNew(EventSim es, size_t stackSize, EventBody body, void *bodyArg)
{
    EventActor          ea = NEW_ZEROED(struct _EventActor, 1);

    ea->es = es;

    ea->body = body;
    ea->bodyArg = bodyArg;

    ABORT_IF_ERRNO(cond_init(&ea->delayCond, USYNC_THREAD, 0));
    ea->wakeupTime = 0LL;
    ea->queueType = EVENT_QUEUE_RUNNING;

    ABORT_IF_ERRNO(cond_init(&ea->liveCond, USYNC_THREAD, 0));
    ea->shouldLive = TRUE;
    ea->isLive = FALSE;
    ABORT_IF_ERRNO(thr_create(NULL, stackSize, eventActor, ea, 0,
	&ea->threadId));

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    while (!ea->isLive) {
	CHECK_IF_ERRNO(cond_wait(&ea->liveCond, &es->lock));
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return ea;
}

/*************************************************************************
 * OBJECT EventActor Instance Methods
 *************************************************************************/

void
eventActorFree(EventActor ea)
{
    EventSim            es = ea->es;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    ea->shouldLive = FALSE;
    ASSERT(!es->isRunning);
    ASSERT(ea->queueType != EVENT_QUEUE_RUNNING);
    (void) priQueueDelete(es->delayQueue, ea);
    CHECK_IF_ERRNO(cond_signal(&ea->delayCond));
    while (ea->isLive) {
	CHECK_IF_ERRNO(cond_wait(&ea->liveCond, &es->lock));
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    ABORT_IF_ERRNO(thr_join(ea->threadId, NULL, NULL));
    ABORT_IF_ERRNO(cond_destroy(&ea->liveCond));
    ABORT_IF_ERRNO(cond_destroy(&ea->delayCond));
    free(ea);
}

Boolean
eventActorShouldRun(EventActor ea)
{
    return ea->shouldLive;
}

Time
eventActorDelay(EventActor ea, Time delayTime)
{
    EventSim            es = ea->es;
    Time           nowTime;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    if (delayTime == 0LL) {
	delayTime = 1LL;
    }
    nowTime = eventActorYield(ea, EVENT_QUEUE_DELAY, delayTime);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return nowTime;
}

Time
eventActorWait(EventActor ea)
{
    EventSim            es = ea->es;
    Time           nowTime;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    nowTime = eventActorYield(ea, EVENT_QUEUE_WAIT, EVENT_TIME_FOREVER);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
    return nowTime;
}

Time
eventActorTimedWait(EventActor ea, Time maxWaitTime)
{
    EventSim            es = ea->es;
    Time           nowTime;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    nowTime = eventActorYield(ea, EVENT_QUEUE_WAIT, maxWaitTime);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
    return nowTime;
}

void
eventActorSignal(EventActor ea)
{
    EventSim            es = ea->es;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    if (ea->queueType == EVENT_QUEUE_WAIT) {
	(void) priQueueDelete(es->delayQueue, ea);
	ea->wakeupTime = es->nowTime;
	priQueueInsert(es->delayQueue, ea);
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
}

Time
eventActorNow(EventActor ea)
{
    return eventSimNow(ea->es);
}

void
eventActorStopSim(EventActor ea)
{
    EventSim            es = ea->es;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    es->shouldRun = FALSE;
    (void) eventActorYield(ea, EVENT_QUEUE_WAIT, 0LL);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
}

/*************************************************************************
 * OBJECT EventSim Private Methods
 *************************************************************************/

static void
eventSimStart(EventSim es, Boolean shouldRun)
{
    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    es->shouldRun = shouldRun;
    if (!es->isRunning) {
	EventActor          ea = (EventActor)priQueueRemove(es->delayQueue);

	if (ea != NULL) {
	    CHECK_IF_ERRNO(cond_signal(&ea->delayCond));
	    es->isRunning = TRUE;
	    es->realTimeBase = timeOfDay();
	    es->simTimeBase = es->nowTime;
	}
    }
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));
}

/*************************************************************************
 * OBJECT EventSim Private Methods
 *************************************************************************/

static Time
eventActorYield(EventActor ea, EventQueueType queueType, Time delayTime)
{
    EventSim            es = ea->es;

    ASSERT(ea->queueType == EVENT_QUEUE_RUNNING);

    if (!ea->shouldLive) {
	return -1LL;
    }
    ea->wakeupTime = es->nowTime + delayTime;
    ea->queueType = queueType;

    eventActorSwitch(ea);

    ea->queueType = EVENT_QUEUE_RUNNING;

    if (!ea->shouldLive) {
	return -1LL;
    }
    if (es->realTimeFrac > 0.0) {
	double              realDelay;
	Time           futureTime;
	Time           curTime;

	realDelay = timeToDouble(ea->wakeupTime - es->simTimeBase)
	  * es->realTimeFrac;
	futureTime = timeFromDouble(realDelay) + es->realTimeBase;

	curTime = timeOfDay();

	if (futureTime > curTime) {
	    timestruc_t         to;

	    to = timeToTimestruc(futureTime);

	    while (es->stopTime > ea->wakeupTime) {
		int error; 
		error = cond_timedwait(&es->realTimeCond, &es->lock, &to);
		switch (error) {
		case ETIME:
		    goto done;
		case 0:
		case EINTR:
		    break;
		default:
		    ABORT_WITH_ERRNO(error, "cond_timedwait");
		}
	done:	;
	    }
	}
    }
    ASSERT(ea->wakeupTime >= es->nowTime);
    es->nowTime = ea->wakeupTime;
    return es->nowTime;
}

static void
eventActorSwitch(EventActor ea)
{
    EventSim            es = ea->es;
    EventActor          newEa;

    if (ea->isLive) {
	priQueueInsert(es->delayQueue, ea);
    }
    newEa = (EventActor)priQueueRemove(es->delayQueue);
    if (newEa == NULL || newEa->wakeupTime > es->stopTime || !es->shouldRun) {
	if (newEa != NULL) {
	    priQueueInsert(es->delayQueue, newEa);
	    newEa = NULL;
	}
	es->isRunning = FALSE;
	es->shouldRun = FALSE;
	CHECK_IF_ERRNO(cond_broadcast(&es->runCond));
    }
    if (newEa != ea) {
	if (newEa != NULL) {
	    CHECK_IF_ERRNO(cond_signal(&newEa->delayCond));
	}
	if (ea->isLive) {
	    CHECK_IF_ERRNO(cond_wait(&ea->delayCond, &es->lock));
	}
    }
}

/*************************************************************************
 * OBJECT EventSim Private Functions
 *************************************************************************/

static void        *
eventActor(void *arg)
{
    EventActor          ea = (EventActor) arg;
    EventSim            es = ea->es;

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    ea->isLive = TRUE;
    CHECK_IF_ERRNO(cond_signal(&ea->liveCond));
    (void) eventActorYield(ea, EVENT_QUEUE_DELAY, 0LL);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    (ea->body) (ea, ea->bodyArg);

    CHECK_IF_ERRNO(mutex_lock(&es->lock));
    ea->isLive = FALSE;
    CHECK_IF_ERRNO(cond_signal(&ea->liveCond));
    eventActorSwitch(ea);
    CHECK_IF_ERRNO(mutex_unlock(&es->lock));

    return NULL;
}
static Boolean
eventSimInOrder(const void *item1p, const void *item2p)
{
    EventActor          eventActor1 = (EventActor) item1p;
    EventActor          eventActor2 = (EventActor) item2p;

    return Boolean(eventActor1->wakeupTime <= eventActor2->wakeupTime);
}
