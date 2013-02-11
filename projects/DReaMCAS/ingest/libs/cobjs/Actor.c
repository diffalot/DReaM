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
 * $(@)Actor.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:32 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Actor.c
 * 
 * Actor is a thread which carries out actions.
 * 
 * FIXME: Add func arg to actorNew that allows client sorting of actions on
 * enqueue.  Think about how to handle actor thread priority. Think about
 * priority inheritance. FIXME: Think about support for aborting requests.
 */

#pragma	ident "@(#)Actor.c 1.2	99/03/22 SMI"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <synch.h>
#include <thread.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

#include "cobjs/Actor.h"

struct _Actor {
    ActorImpl           impl;
    Object              delegate;
    ActorWakeup         implWakeup;
    Boolean             isRunning;
    thread_t            threadId;

    mutex_t             mutex;
    cond_t              workCond;
    Action              anchor;
};

struct _Action {
    size_t              parameterSize;
    void               *parameterp;

    Action              nextp;
    Action              prevp;

    mutex_t             mutex;
    cond_t              stateCond;
    ActionState         state;
    ActionType		type;
    ActionFreeFunc      freeFunc;
    Boolean             isFreed;
};

static void        *actorThread(void *arg);
static void         actorPutAction(Actor actor, Action action);
static void	    actionDoInitiate(Action action, Actor actor,
			    	const void *parameterp, ActionType type);

/*
 * Public methods
 */
Actor
actorNew(ActorImpl impl, Object delegate, ActorWakeup implWakeup,
	 void *stackBase, size_t stackSize, ActorType type)
{
    Actor               actor = NEW_ZEROED(struct _Actor, 1);

    actor->impl = impl;
    actor->delegate = delegate;
    actor->implWakeup = implWakeup;
    actor->isRunning = TRUE;

    ABORT_IF_ERRNO(mutex_init(&actor->mutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&actor->workCond, USYNC_THREAD, 0));
    actor->anchor = actionNew(NULL, NULL);

    ABORT_IF_ERRNO(thr_create(stackBase, stackSize, actorThread, actor,
	type == ACTOR_TYPE_LWP ? THR_BOUND : 0, &actor->threadId));

    return actor;
}

void
actorFree(Actor actor)
{
    ASSERT(actor->isRunning);

    actorPutAction(actor, NULL);
    ABORT_IF_ERRNO(thr_join(actor->threadId, NULL, NULL));

    ASSERT(actor->anchor->nextp == actor->anchor
	   && actor->anchor->prevp == actor->anchor);

    ABORT_IF_ERRNO(cond_destroy(&actor->workCond));
    ABORT_IF_ERRNO(mutex_destroy(&actor->mutex));
    actionFree(actor->anchor);

    free(actor);
}

/*
 * Body methods
 */

Boolean
actorIsRunning(Actor actor)
{
    Boolean             retVal;

    /*
     * This is technically necessary for portability, but probably overly
     * paranoid.
     */
    CHECK_IF_ERRNO(mutex_lock(&actor->mutex));
    retVal = actor->isRunning;
    CHECK_IF_ERRNO(mutex_unlock(&actor->mutex));
    return retVal;
}

Action
actorGetAction(Actor actor, const timestruc_t *timeoutDatep)
{
    Action              anchor = actor->anchor;
    Action              action = NULL;
    int                 error = 0;

    CHECK_IF_ERRNO(mutex_lock(&actor->mutex));
    while (anchor->nextp == anchor && error != ETIME && actor->isRunning) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&actor->workCond, &actor->mutex,
			   (timestruc_t *) timeoutDatep)
	  : cond_wait(&actor->workCond, &actor->mutex);
	ASSERT(error == 0 || error == ETIME || error == EINTR);
    }
    if (anchor->nextp != anchor) {
	action = anchor->nextp;
	action->prevp->nextp = action->nextp;
	action->nextp->prevp = action->nextp;
	action->nextp = action;
	action->prevp = action;

	CHECK_IF_ERRNO(mutex_lock(&action->mutex));
	action->state = ACTION_STATE_ACTIVE;
	CHECK_IF_ERRNO(cond_broadcast(&action->stateCond));
	CHECK_IF_ERRNO(mutex_unlock(&action->mutex));
    }
    CHECK_IF_ERRNO(mutex_unlock(&actor->mutex));

    return action;
}

Boolean
actorIsActionPending(Actor actor)
{
    Action              anchor = actor->anchor;
    Boolean             retval;

    CHECK_IF_ERRNO(mutex_lock(&actor->mutex));
    retval = Boolean(anchor->nextp != anchor);
    CHECK_IF_ERRNO(mutex_unlock(&actor->mutex));

    return retval;
}

/*
 * Private routines
 */
static void        *
actorThread(void *arg)
{
    Actor               actor = (Actor) arg;

    (*actor->impl) (actor, actor->delegate);

    return NULL;
}

static void
actorPutAction(Actor actor, Action action)
{
    Action              anchor = actor->anchor;

    CHECK_IF_ERRNO(mutex_lock(&actor->mutex));
    ASSERT(actor->isRunning);

    if (action != NULL) {
	/*
	 * Insure action not already on some list
	 */
	ASSERT(action->nextp == action && action->prevp == action);
	anchor->prevp->nextp = action;
	action->nextp = anchor;
	action->prevp = anchor->prevp;
	anchor->prevp = action;

	ASSERT(action->state == ACTION_STATE_QUEUED);
    } else {
	actor->isRunning = FALSE;
    }

    CHECK_IF_ERRNO(cond_signal(&actor->workCond));
    CHECK_IF_ERRNO(mutex_unlock(&actor->mutex));

    if (actor->implWakeup != NULL) {
	(*actor->implWakeup) (actor->delegate);
    }
}


/*
 * Public methods
 */
Action
actionNew(size_t parameterSize, ActionFreeFunc freeFunc)
{
    Action              action = NEW_ZEROED(struct _Action, 1);

    action->parameterSize = parameterSize;
    action->freeFunc = freeFunc;

    if (action->parameterSize != 0) {
	action->parameterp = NEW(char, action->parameterSize);
    }
    action->nextp = action;
    action->prevp = action;

    ABORT_IF_ERRNO(mutex_init(&action->mutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&action->stateCond, USYNC_THREAD, 0));
    action->state = ACTION_STATE_FREE;
    action->isFreed = FALSE;

    return action;
}

void
actionFree(Action action)
{
    Boolean             doFreeNow = FALSE;

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));

    switch (action->state) {
    case ACTION_STATE_QUEUED:
    case ACTION_STATE_ACTIVE:
	action->isFreed = TRUE;
	break;
    case ACTION_STATE_FREE:
    case ACTION_STATE_DONE:
	doFreeNow = TRUE;
	break;
    default:
	ABORT("invalid action state");
    }

    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));

    if (doFreeNow) {
	if (action->freeFunc != NULL) {
	    (*action->freeFunc) (action->parameterp);
	}
	free(action->parameterp);
	ABORT_IF_ERRNO(mutex_destroy(&action->mutex));
	ABORT_IF_ERRNO(cond_destroy(&action->stateCond));
	free(action);
    }
}

void
actionPerform(Action action, Actor actor, void *parameterp)
{
    actionInitiate(action, actor, parameterp, ACTION_TYPE_RESULTS);
    actionWaitTilDone(action, parameterp);
}

void
actionInitiate(Action action, Actor actor, const void *parameterp,
	ActionType type)
{

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));
    while (action->state != ACTION_STATE_FREE) {
	CHECK_IF_ERRNO(cond_wait(&action->stateCond, &action->mutex));
    }
    action->state = ACTION_STATE_QUEUED;
    CHECK_IF_ERRNO(cond_broadcast(&action->stateCond));
    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));

    actionDoInitiate(action, actor, parameterp, type);
}

void
actionWaitTilDone(Action action, void *parameterp)
{
    (void) actionWaitForState(action, ACTION_STATE_DONE, NULL);

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));
    if (parameterp != NULL) {
	ASSERT(action->type == ACTION_TYPE_RESULTS);
	ASSERT(action->state == ACTION_STATE_DONE);
	(void) memcpy(parameterp, action->parameterp, action->parameterSize);
    }
    if (action->state == ACTION_STATE_DONE) {
	action->state = ACTION_STATE_FREE;
	CHECK_IF_ERRNO(cond_broadcast(&action->stateCond));
    }
    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));
}

ActionState
actionWaitForState(Action action, ActionState state,
		   const timestruc_t *timeoutDatep)
{
    ActionState         returnState;
    int                 error = 0;

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));
    while (action->state < state && error != ETIME) {
	error = (timeoutDatep != NULL)
	  ? cond_timedwait(&action->stateCond, &action->mutex,
			   (timestruc_t *) timeoutDatep)
	  : cond_wait(&action->stateCond, &action->mutex);
	ASSERT(error == 0 || error == ETIME || error == EINTR);
    }
    returnState = action->state;
    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));

    return returnState;
}

ActionState
actionGetState(Action action)
{
    ActionState         returnState;

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));
    returnState = action->state;
    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));

    return returnState;
}

/*
 * Body methods
 */

void
actionDone(Action action)
{
    Boolean             doFree = FALSE;

    ASSERT(action->state == ACTION_STATE_ACTIVE);

    CHECK_IF_ERRNO(mutex_lock(&action->mutex));
    action->state = (action->type == ACTION_TYPE_RESULTS)
	? ACTION_STATE_DONE : ACTION_STATE_FREE;
    CHECK_IF_ERRNO(cond_broadcast(&action->stateCond));
    if (action->isFreed) {
	doFree = TRUE;
    }
    CHECK_IF_ERRNO(mutex_unlock(&action->mutex));

    if (doFree) {
	actionFree(action);
    }
}

void               *
actionGetParameterp(Action action)
{
    return action->parameterp;
}

static void
actionDoInitiate(Action action, Actor actor, const void *parameterp,
	ActionType type)
{
    ASSERT(action->state == ACTION_STATE_QUEUED);
    action->type = type;
    if (parameterp != NULL) {
	(void) memcpy(action->parameterp, parameterp, action->parameterSize);
    }
    actorPutAction(actor, action);
}
