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
 * $(@)Actor.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/************************************************************************
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 ************************************************************************/

/************************************************************************
 * Actor.h
 *
 * A thread-based action executor.
 ************************************************************************/

#ifndef	_COBJS_ACTOR_H
#define	_COBJS_ACTOR_H

#pragma ident "@(#)Actor.h 1.2	99/03/22 SMI"

/************************************************************************
 * Includes
 ************************************************************************/

#include <time.h>

#include "cobjs/Types.h"
#include "cobjs/Foreign.h"

_FOREIGN_START

/************************************************************************
 * OBJECT Instance Types
 ************************************************************************/
typedef struct _Actor *Actor;
typedef struct _Action *Action;

/************************************************************************
 * OBJECT Public Types
 ************************************************************************/

typedef enum ActorType {
    ACTOR_TYPE_LWP,			   /* bound to an LWP */
    ACTOR_TYPE_THREAD			   /* coroutine */
} ActorType;

typedef enum ActionState {
    ACTION_STATE_QUEUED,		   /* queued with Actor */
    ACTION_STATE_ACTIVE,		   /* Actor processing */
    ACTION_STATE_DONE,			   /* Action completed */
    ACTION_STATE_FREE			   /* action available */
} ActionState;

typedef enum ActionType {
    ACTION_TYPE_RESULTS,		   /* action returns results */
    ACTION_TYPE_NO_RESULTS
} ActionType;

/***
 * ActorImpl -- implementation of actor
 *
 * Typical implementation looks like:
 *
 * void actorImpl(Actor actor, Object delegate) {
 *	while (actorIsRunning(actor)) {
 *		// NOTE: timeoutDate is a DATE!! Not a period.
 *		timestruc_t timeout = timeoutDate;
 *		Action action = actorGetAction(actor, &timeout);
 *		if (action != NULL) {
 *			void *paramp = actionGetParameterp(action);
 *			// do action processing based on parameters
 *			actionDone(action);
 *		} else if (actorIsRunning(actor)) {
 *			// do timeout processing
 *		}
 *	}
 * }
 */
typedef void        (*ActorImpl) (Actor actor, Object delegate);

/*
 * ActorWakeup -- poke actorImpl to go do an actorGetAction()
 * 
 * If non-null, actorWakeup function will be called whenever an action is queued
 * to the actor.  This function may do an appropriate operation to cause the
 * actorImpl to perform an actorGetAction() (e.g. a cond_signal())
 */
typedef void        (*ActorWakeup) (Object delegate);

/*
 * ActionFreeFunc -- free action parameters
 * 
 * If non-null, actionFreeFunc will be called during actionFree. This function
 * may free any storage referenced by the parameters of the action.
 */
typedef void        (*ActionFreeFunc) (void *parameterp);

/************************************************************************
 * OBJECT Class Interface
 ************************************************************************/

/*
 * actorNew -- create a new Actor.
 * 
 * impl is the body of the actor, it may call the implementation methods listed
 * below.
 * 
 * delegate is passed to the implementation.  It is assumed to reference
 * the object owning the actor.
 * 
 * implWakeup if non-null is invoked whenever action is queued to actor.
 * It may be used to signal the implementation to check its action queue via
 * actorGetAction().
 */
extern Actor
actorNew(ActorImpl impl, Object delegate, ActorWakeup implWakeup,
	 void *stackBase, size_t stackSize, ActorType type);

/*
 * actionNew -- create a new action.
 * 
 * parameterSize indicates the storage required for action parameters.
 * 
 * freeFunc, if non-null, will be called when the action is freed and may be
 * used to free any storage (or other assets) referenced by the parameters.
 */
extern Action       actionNew(size_t parameterSize, ActionFreeFunc freeFunc);

#define	NEW_ACTION(type, freeFunc) actionNew(sizeof(type), freeFunc)

/************************************************************************
 * OBJECT Instance Interface
 ************************************************************************/

/*
 * actorFree -- shutdown actor
 * 
 * actor will process all pending actions and then terminate
 */
extern void         actorFree(Actor actor);

/*
 * actionPerform -- queue Action and wait until done
 * 
 * parameters are passed value-result
 * 
 * equivalent to: actionInitiate() actionWaitTilDone()
 */
extern void         actionPerform(Action action, Actor actor, void *parameters);

/*
 * actionInitiate -- queue Action.
 *
 * If action is currently busy, will block until previous action is complete.
 */
extern void
actionInitiate(Action action, Actor actor,
	       const void *parameters, ActionType type);

/*
 * actionWaitTilDone -- wait for initiated action to complete
 */
extern void         actionWaitTilDone(Action action, void *parameters);

/*
 * actionWaitForState -- wait until action reaches requested state
 */
extern ActionState
actionWaitForState(Action action, ActionState state,
		   const timestruc_t *timeoutDatep);

/*
 * actionGetState -- get current state of action
 */
extern ActionState  actionGetState(Action action);

/*
 * actionFree -- free (or abandon) action
 * 
 * may be called will action is in progress, action will complete and then be
 * freed.
 */
extern void         actionFree(Action action);

/************************************************************************
 * OBJECT Implementation Interface
 *
 * These methods are to be used by the actor implementation.
 *
 ************************************************************************/

/*
 * actorIsRunning -- returns TRUE if implementation should continue
 * processing actions; returns FALSE if implementation should return.
 */
extern Boolean      actorIsRunning(Actor actor);

/*
 * actorGetAction -- get next action that has been queued for actor.
 * 
 * If timeoutDatep is NULL, blocks until an action is available. If timeoutDatep
 * points to a zero timestruct, polls for an action and returns NULL if no
 * action present. If timeoutDatep is a time of day, will return when an
 * action has been queued or when that time of day arrives.  Returns NULL if
 * timeout occured.
 * 
 * May return NULL if client called actorFree indicating that actor impl
 * should return.  This can be differentiated from a timeout by checking
 * actorIsRunning()
 * 
 * Implementation MUST check actorIsRunning() whenever a NULL action is
 * returned.  It MUST return if actorIsRunning() returns false.
 */
extern Action
actorGetAction(Actor actor,
	       const timestruc_t *timeoutDatep);

/*
 * actorIsActionPending -- return TRUE if action pending, FALSE if no action
 * pending
 */
extern Boolean      actorIsActionPending(Actor actor);

/*
 * actionGetParameterp -- get pointer to action parameters
 */
extern void        *actionGetParameterp(Action action);

/*
 * actionDone -- indicate action is completed.
 */
extern void         actionDone(Action action);

_FOREIGN_END

#endif					   /* _COBJS_ACTOR_H */
