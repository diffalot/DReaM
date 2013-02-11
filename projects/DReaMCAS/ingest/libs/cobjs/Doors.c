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
 * $(@)Doors.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:33 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Doors.c -- Object wrapper for Doors RPC.
 */

/*
 * FIXME: Make sure client checks for need to munmap returned data.
 */

#pragma ident "@(#)Doors.c 1.9	99/10/27 SMI"

#if	!defined(DOORS_HEADER)
#define	DOORS_BODY
#define	DOORS_INLINE		extern
#include "cobjs/Doors.h"
#endif					   /* !defined(DOORS_HEADER) */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <door.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <synch.h>
#include <thread.h>
#include <unistd.h>

#include "cobjs/Actor.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*
 * WORKAROUND for libdoor bug.  libdoor should pass the door fd to
 * the thread_create func so it could store it somewhere the thread_init_func
 * could find it.  It doesn't, so we're forced to yield() until the
 * door_create thread can set it.
 */
#define	DOORS_BUG			    1
/*
 * WORKAROUND for libdoor bug.  There's no clean mechanism for shutting
 * down private door threads (ideally, doing a door_revoke() would cause
 * private threads to thr_exit() when they re-entered the private pool).
 * The first workaround was to do "self" door calls that were specially
 * detected in the server func as close shutdowns on the server thread.
 * This works "most" of the time, but occasionally the kernel seems to
 * block this self door_call even though a thread still exists.  This
 * seems to happen mostly when the thread init call occurs close to
 * the self door_call.  The hack here is to ensure that all thread_init's
 * (and the corresponding door_bind()) is done well before the self
 * door_call.
 */
#define	DOORS_BUG2			    1

#define	DOORS_DEFAULT_MAX_THREAD_COUNT	    5

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _Doors {
    Object		delegate;
    DoorsThreadInitFunc	threadInitFunc;
    DoorsServerFunc	serverFunc;
    DoorsNoClientFunc	noClientFunc;
    size_t		stackSize;
    int			maxThreadCount;

    u_int		attributes;
    int			fd;
    door_id_t		id;
    char		*file;

    mutex_t		lock;
#if DOORS_BUG
    Boolean		isInitialized;
    cond_t		initializedCond;
#endif /* DOORS_BUG */
#if DOORS_BUG2
    cond_t		pendingCond;
    Boolean		isPendingWait;
    int			pendingCreates;
#endif	/* DOORS_BUG2 */
    int			threadCount;
    Boolean		doingNoClient;
    Boolean		doingShutdown;
    Boolean		doTrace;
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void doorsInlinePrivate(void);
 */

typedef enum DoorsCommand {
    DOORS_COMMAND_NO_CLIENT
} DoorsCommand;

typedef struct DoorsParameters {
    DoorsCommand	command;
    Doors		doors;
} DoorsParameters;

typedef struct DoorBuffer {
    void	       *body;
    size_t		size;
} DoorBuffer;

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

DOORS_INLINE door_id_t
doorsId(Doors doors)
{
    return doors->id;
}

DOORS_INLINE int
doorsFd(Doors doors)
{
    return doors->fd;
}

DOORS_INLINE int
doorsThreadCount(Doors doors)
{
    return doors->threadCount;
}

DOORS_INLINE DoorsResultData
doorsResultData(void *datap, size_t dataSize, door_desc_t *descp, size_t nDesc)
{
    DoorsResultData drd;

    drd.dataPtr = datap;
    drd.dataSize = dataSize;
    drd.descPtr = descp;
    drd.nDesc = nDesc;
    return drd;
}

DOORS_INLINE unsigned long
doorsIdKeyHash(const void *key, unsigned int *rehashp)
{
    door_id_t doorId = *(door_id_t *)key;
    *rehashp = (unsigned int) ((doorId * 1103515245L + 12345) >> 16);
    return ((unsigned long) (doorId * 123821)) >> 10;
}

DOORS_INLINE Boolean
doorsIdKeyIsEqual(const void *key1, const void *key2)
{
    return Boolean( *(door_id_t *)key1 == *(door_id_t *)key2 );
}

DOORS_INLINE int
doorsIdKeyCmp(const void *key1, const void *key2)
{
    return *(door_id_t *)key1 - *(door_id_t *)key2;
}

/* ARGSUSED1 */
DOORS_INLINE const void *
doorsIdKeyDup(const void *key, const void *value)
{
    door_id_t *newIdp = NEW(door_id_t, 1);
    *newIdp = *(door_id_t *)key;
    return newIdp;
}

DOORS_INLINE void
doorsIdKeyFree(void *key)
{
    free(key);
}

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(DOORS_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

#ifdef __cplusplus 
extern "C" {
#endif 

static void *doorsThreadInit(void *arg);
static void doorsServerFunc(void *cookie, /*const void*/ char *argp, size_t argSize,
	/*const*/ door_desc_t *descp, size_t nDesc);

/*************************************************************************
 * Private function prototypes
 *************************************************************************/
   
static void doorsThreadCreate(/*const*/ door_info_t *dip);
static void doorsCommandImpl(Actor actor, void *implArg);
static void doorBufferFree(void *arg);

#ifdef __cplusplus
}
#endif
/*************************************************************************
 * Private class data
 *************************************************************************/

static DoorsResultData	nullRetData = { NULL, 0, NULL, 0};
static Actor		commandActor;
static thread_key_t	resultBufferKey;
static thread_key_t	descBufferKey;
static Action		noClientAction;
static pid_t		doorsPid;

/*************************************************************************
 * Class Methods
 *************************************************************************/

void
doorsInit(void)
{
    doorsPid = getpid();
    (void) door_server_create(doorsThreadCreate);
    noClientAction = NEW_ACTION(DoorsParameters, NULL);
    commandActor = actorNew(doorsCommandImpl, NULL, NULL, NULL, 0,
	ACTOR_TYPE_LWP);
    ABORT_IF_ERRNO(thr_keycreate(&resultBufferKey, doorBufferFree));
    ABORT_IF_ERRNO(thr_keycreate(&descBufferKey, doorBufferFree));
}

void
doorsFini(void)
{
    actorFree(commandActor);
    actionFree(noClientAction);
}

Doors
doorsNew(Object delegate, DoorsThreadInitFunc threadInitFunc,
	 DoorsServerFunc serverFunc, DoorsNoClientFunc noClientFunc,
	 size_t stackSize, int maxThreadCount)
{
    Doors doors = NEW_ZEROED(struct _Doors, 1);
    door_info_t info;

    if (maxThreadCount == 0) {
	maxThreadCount = DOORS_DEFAULT_MAX_THREAD_COUNT;
    }

    if (commandActor == NULL) {
	ABORT("doorsInit not called");
    }
    doors->delegate = delegate;
    doors->threadInitFunc = threadInitFunc;
    doors->serverFunc = serverFunc;
    doors->noClientFunc = noClientFunc;
    doors->stackSize = stackSize;
    doors->maxThreadCount = maxThreadCount;

    /*
     * You MUST have private threads until door_revoke() can block
     * until no thread active on door.
     */
    doors->attributes = DOOR_PRIVATE;
    if (noClientFunc != NULL) {
	doors->attributes |= DOOR_UNREF;
    }

    CHECK_IF_ERRNO(mutex_init(&doors->lock, USYNC_THREAD, 0));
    doors->threadCount = 0;
    doors->doingShutdown = FALSE;
    doors->doingNoClient = FALSE;
    doors->doTrace = FALSE;
#if DOORS_BUG
    CHECK_IF_ERRNO(cond_init(&doors->initializedCond, USYNC_THREAD, 0));
    doors->isInitialized = FALSE;
#endif	/* DOORS_BUG */
#if DOORS_BUG2
    CHECK_IF_ERRNO(cond_init(&doors->pendingCond, USYNC_THREAD, 0));
    doors->pendingCreates = 0;
    doors->isPendingWait = FALSE;
#endif	/* DOORS_BUG2 */

    LOG_DBG(('r', "doorsNew(0x%llx, 0x%x)", doors->id, thr_self()));
    ABORT_ON_ERRNO(doors->fd = door_create(doorsServerFunc, doors,
	doors->attributes));
    ABORT_ON_ERRNO(door_info(doors->fd, &info));
    doors->id = info.di_uniquifier;
    LOG_DBG(('r', "doorsNew(0x%llx, 0x%x)", doors->id, thr_self()));
#if DOORS_BUG
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    doors->isInitialized = TRUE;
    CHECK_IF_ERRNO(cond_broadcast(&doors->initializedCond));
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
#endif	/* DOORS_BUG */
    return doors;
}

Doors
doorsNewWithFile(Object delegate, DoorsThreadInitFunc threadInitFunc,
	 DoorsServerFunc serverFunc, DoorsNoClientFunc noClientFunc,
	 size_t stackSize, int maxThreadCount, const char *file, uid_t uid,
	 gid_t gid, mode_t mode)
{
    Doors doors = doorsNew(delegate, threadInitFunc, serverFunc, noClientFunc,
	    stackSize, maxThreadCount);
    int fd;

    (void) unlink(file);
    if ((fd = open(file, O_RDWR | O_CREAT | O_NOCTTY, mode)) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "FATAL: open(%s)", file);
	abort();
    }
    ABORT_ON_ERRNO(fchmod(fd, mode));
    ABORT_ON_ERRNO(fchown(fd, uid, gid));
    (void) close(fd);

    if (fattach(doors->fd, file) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "FATAL: fattach(%s)", file);
	abort();
    }
    doors->file = strdup(file);
    return doors;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

void         *
doorsResultBuffer(size_t size, size_t count)
{
    DoorBuffer *dbp;

    ABORT_IF_ERRNO(thr_getspecific(resultBufferKey, (void **)&dbp));
    if (dbp == NULL) {
	dbp = NEW_ZEROED(DoorBuffer, 1);
	ABORT_IF_ERRNO(thr_setspecific(resultBufferKey, dbp));
    }
    if (dbp->size < count * size) {
	dbp->body = REALLOC(dbp->body, count, size);
	dbp->size = count * size;
    }
    return dbp->body;
}

door_desc_t *
doorsDescBuffer(size_t count)
{
    DoorBuffer *dbp;

    ABORT_IF_ERRNO(thr_getspecific(descBufferKey, (void **)&dbp));
    if (dbp == NULL) {
	dbp = NEW_ZEROED(DoorBuffer, 1);
	ABORT_IF_ERRNO(thr_setspecific(descBufferKey, dbp));
    }
    if (dbp->size < count * sizeof(door_desc_t)) {
	dbp->body = REALLOC(dbp->body, count, sizeof(door_desc_t));
	dbp->size = count * sizeof(door_desc_t);
    }
    return (door_desc_t *)dbp->body;
}

void
doorsNoClient(Doors doors)
{
    DoorsParameters p;
    Boolean doShutdown = FALSE;

    LOG_DBG(('r', "doorsNoClient(0x%llx, 0x%x)", doors->id,
		thr_self()));
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    if (! doors->doingNoClient) {
	doors->doingNoClient = TRUE;
	doShutdown = TRUE;
    }
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));

    if (doShutdown) {
	LOG_DBG(('r', "doorsNoClient(0x%llx, 0x%x) shutdown", doors->id,
		    thr_self()));
	p.command = DOORS_COMMAND_NO_CLIENT;
	p.doors = doors;
	actionInitiate(noClientAction, commandActor, &p,
		ACTION_TYPE_NO_RESULTS);
    }
}

void
doorsSetTrace(Doors doors, Boolean doTrace)
{
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    doors->doTrace = doTrace;
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
}

void
doorsFree(Doors doors)
{
    if (doors->file != NULL) {
	ABORT_ON_ERRNO(fdetach(doors->file));
	free(doors->file);
    }

    /*
     * Terminate threads in private pool
     */
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    LOG_DBG(('r', "doorsFree(0x%llx, 0x%x) called, count %d", doors->id,
		thr_self(), doors->threadCount));
    ASSERT(! doors->doingShutdown);
    doors->doingShutdown = TRUE;
#if	DOORS_BUG2
    doors->isPendingWait = TRUE;
    while (doors->pendingCreates > 0) {
	LOG_DBG(('r', "doorsFree(0x%llx, 0x%x) pending wait, pending %d",
		    doors->id, thr_self(), doors->pendingCreates));
	CHECK_IF_ERRNO(cond_wait(&doors->pendingCond, &doors->lock));
    }
#endif	/* DOORS_BUG2 */
    while (doors->threadCount > 0) {
	LOG_DBG(('r', "doorsFree(0x%llx, 0x%x), doorCall start count %d",
		    doors->id, thr_self(), doors->threadCount));
	CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
	/*
	 * Do door_calls on all threads, doorsServerFunc sees shutdown TRUE
	 * and if the door_call() was from this pid, does a thr_exit().
	 */
	(void)door_call(doors->fd, NULL);
	CHECK_IF_ERRNO(mutex_lock(&doors->lock));
	LOG_DBG(('r', "doorsFree(0x%llx, 0x%x), doorCall done count %d",
		    doors->id, thr_self(), doors->threadCount));
    }
    LOG_DBG(('r', "doorsFree(0x%llx, 0x%x), door revoke", doors->id,
		thr_self()));
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));

    ABORT_ON_ERRNO(door_revoke(doors->fd));
    LOG_DBG(('r', "doorsFree(0x%llx, 0x%x), door revoke done", doors->id,
		thr_self()));

#if DOORS_BUG
    CHECK_IF_ERRNO(cond_destroy(&doors->initializedCond));
#endif	/* DOORS_BUG */
#if DOORS_BUG2
    CHECK_IF_ERRNO(cond_destroy(&doors->pendingCond));
#endif	/* DOORS_BUG2 */
    CHECK_IF_ERRNO(mutex_destroy(&doors->lock));
    free(doors);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 *
 * INLINE_PRIVATE void
 * doorsInlinePrivate(void)
 * {
 * }
 */

static void *
doorsThreadInit(void *arg)
{
    Doors doors = (Doors) arg;

#if DOORS_BUG
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    while (! doors->isInitialized) {
	CHECK_IF_ERRNO(cond_wait(&doors->initializedCond, &doors->lock));
    }
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
#endif /* DOORS_BUG */

    LOG_DBG(('r', "doorsThreadInit(0x%llx, 0x%x)", doors->id, thr_self()));

    if (doors->threadInitFunc != NULL) {
	(*doors->threadInitFunc)(doors->delegate);
    }
    LOG_DBG(('r', "doorsThreadInit(0x%llx, 0x%x) door_bind", doors->id,
		thr_self()));
    ABORT_ON_ERRNO(door_bind(doors->fd));
    LOG_DBG(('r', "doorsThreadInit(0x%llx, 0x%x) door_bind done", doors->id,
		thr_self()));
    ABORT_IF_ERRNO(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
    LOG_DBG(('r', "doorsThreadInit(0x%llx, 0x%x) door_return", doors->id,
		thr_self()));
#if DOORS_BUG2
    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    ASSERT(doors->pendingCreates > 0);
    if (--doors->pendingCreates == 0 && doors->isPendingWait) {
	CHECK_IF_ERRNO(cond_broadcast(&doors->pendingCond));
    }
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
#endif	/* DOORS_BUG2 */
    ABORT_ON_ERRNO(door_return(NULL, 0, NULL, 0));
    return NULL;	/* for lint */
}

static void
doorsServerFunc(void *cookie, /*const void*/ char *argp, size_t argSize,
	/*const*/ door_desc_t *descp, size_t nDesc)
{
    Doors doors = (Doors) cookie;
    DoorsResultData retData;

    if (argp == DOOR_UNREF_DATA) {
	ASSERT(doors->noClientFunc != NULL);
	LOG_DBG(('r', "doorsServerFunc(0x%llx, 0x%x) UNREF DOOR",
		    doors->id, thr_self()));
	doorsNoClient(doors);
	retData = nullRetData;
    } else {
	door_cred_t cred;

	if (door_cred(&cred) != 0) {
	    LOG_DBG(('r', "doorsServerFunc(0x%llx, 0x%x) cred failed",
		    doors->id, thr_self()));
	    switch (errno) {
	    case EINVAL:
		/*
		 * This can happen if client is aborted between start
		 * of door call and here....  Just ignore the request
		 * (thr_exit??).
		 */
		retData = nullRetData;
		break;
	    default:
		ABORT_WITH_ERRNO(errno, "door_cred");
		break;
	    }
	} else {
	    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
	    if (doors->doingShutdown && cred.dc_pid == doorsPid) {
		/*
		 * If doing shutdown, request is from ourself, then
		 * terminate this thread.  See doorsFree().
		 */
		doors->threadCount -= 1;
		ASSERT(doors->threadCount >= 0);
		LOG_DBG(('r', "doorsServerFunc(0x%llx, 0x%x) thr_exit count %d",
			    doors->id, thr_self(), doors->threadCount));
		CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
		thr_exit(NULL);
	    }
	    LOG_DBGC((doors->doTrace, 'r',
			"doorsServerFunc(0x%llx, 0x%x) do request",
			doors->id, thr_self()));
	    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
	    retData = (*doors->serverFunc)(doors->delegate, doors, argp,
		    argSize, descp, nDesc, &cred);
	}
    }
    ABORT_ON_ERRNO(door_return((char *)retData.dataPtr, retData.dataSize,
	retData.descPtr, retData.nDesc));
}

/*************************************************************************
 * Private Functions
 *************************************************************************/

static void
doorsThreadCreate(/*const*/ door_info_t *dip)
{
    Doors doors = (Doors) dip->di_data;

    CHECK_IF_ERRNO(mutex_lock(&doors->lock));
    LOG_DBG(('r', "doorsThreadCreate(0x%llx, 0x%x)", doors->id, thr_self()));
    if (doors->threadCount >= doors->maxThreadCount || doors->doingShutdown) {
	LOG_DBG(('r', "doorsThreadCreate(0x%llx, 0x%x) rejected count %d",
		    doors->id, thr_self(), doors->threadCount));
	CHECK_IF_ERRNO(mutex_unlock(&doors->lock));
	return;
    }
    doors->threadCount += 1;
    LOG_DBG(('r', "doorsThreadCreate(0x%llx, 0x%x) accepted count %d",
		doors->id, thr_self(), doors->threadCount));
#if DOORS_BUG2
    doors->pendingCreates += 1;
#endif	/* DOORS_BUG2 */
    CHECK_IF_ERRNO(mutex_unlock(&doors->lock));

    ABORT_IF_ERRNO(thr_create(NULL, doors->stackSize, doorsThreadInit, doors,
	THR_BOUND | THR_DETACHED, NULL));
}

/* ARGSUSED1 */
static void
doorsCommandImpl(Actor actor, void *implArg)
{
    while (actorIsRunning(actor)) {
	Action action = actorGetAction(actor, NULL);
	if (action != NULL) {
	    DoorsParameters *p = (DoorsParameters *)actionGetParameterp(action);
	    Doors doors = p->doors;

	    switch (p->command) {
	    case DOORS_COMMAND_NO_CLIENT:
		/*
		 * We expect the noClientFunc to do a doorsFree
		 */
		(*doors->noClientFunc)(doors->delegate);
		actionDone(action);
		break;
	    default:
		ABORT("unknown doorsCommand");
	    }
	}
    }
}

static void
doorBufferFree(void *arg)
{
    DoorBuffer *dbp = (DoorBuffer *)arg;

    if (dbp->body != NULL) {
	free(dbp->body);
    }
    free(dbp);
}

#endif					   /* !defined(DOORS_HEADER) */
