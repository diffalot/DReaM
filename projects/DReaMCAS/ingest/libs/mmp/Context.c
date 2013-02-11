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
 * $(@)Context.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:37 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Context.c
 * 
 * Context.c is a pid to pipe map context
 */

#pragma ident "@(#)Context.c 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <stdlib.h>

#include "cobjs/HashTable.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Context.h"

/***********************************************************************
 * Defines
 ***********************************************************************/

#define	PIDPIPE_SIZE	16
#define	PIDPIPE_FACTOR	0.9

static RetCodeTable contextErrorTable[] = {
    {CONTEXT_ERROR_PID_IN_USE, NULL, "pid already in use"},
    {CONTEXT_ERROR_NO_PID, NULL, "reference to unknown pid"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

/***********************************************************************
 * OBJECT Context Instance Type
 ***********************************************************************/

struct _Context {
    HashTable           pidToPipeTable;
    MmpContextObject    co;
};

/***********************************************************************
 * OBJECT Context Class Interface
 ***********************************************************************/

Context
contextNew(void)
{
    Context                 context = NEW_ZEROED(struct _Context, 1);

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(CONTEXT_CLASSNAME,
					     contextErrorTable);
    }

    context->pidToPipeTable = hashTableIntNewWithSizeAndFactor(PIDPIPE_SIZE,
							   PIDPIPE_FACTOR);

    context->co.addPipe = contextAddPipe;
    context->co.deletePipe = contextDeletePipe;
    context->co.pidToPipe = contextPidToPipe;
    context->co.pipeToPid = contextPipeToPid;
    context->co.deleteAll = contextDeleteAll;
    context->co.instp = context;

    return context;
}

/***********************************************************************
 * OBJECT Context Instance Interface
 ***********************************************************************/

MmpContextObject *
contextMmpContextObject(Context context)
{
    return &context->co;
}

RetCode
contextAddPipe(void *instp, u16 pid, Pipe pipe)
{
    Context                 context = (Context) instp;

    if (hashTableIsMember(context->pidToPipeTable, pid)) {
	return RetCode(RETCODE_CONS(retCodeId, CONTEXT_ERROR_PID_IN_USE));
    }
    (void) hashTablePut(context->pidToPipeTable, pid, pipe);
    pipeSetPid(pipe, pid);
    return RETCODE_SUCCESS;
}

RetCode
contextDeletePipe(void *instp, u16 pid)
{
    Context             context = (Context) instp;
    Pipe                pipe;

    pipe = (Pipe)hashTableRemove(context->pidToPipeTable, pid);
    return RetCode((pipe == NULL) ? RETCODE_CONS(retCodeId, CONTEXT_ERROR_NO_PID)
			  : pipeFree(pipe));
}

Pipe
contextPidToPipe(void *instp, u16 pid)
{
    Context                 context = (Context) instp;

    return (Pipe)hashTableGet(context->pidToPipeTable, pid);
}

/* ARGSUSED */
u16
contextPipeToPid(void *instp, Pipe pipe)
{
    return pipeGetPid(pipe);
}

void
contextDeleteAll(void *instp)
{
    Context                 context = (Context) instp;
    HashItem               *pidPipeItems;
    HashItem               *itemp;

    pidPipeItems = hashTableDump(context->pidToPipeTable);
    for (itemp = pidPipeItems; itemp->value != NULL; itemp += 1) {
	(void) contextDeletePipe(context, (int) (itemp->key));
    }
    free(pidPipeItems);
}

void
contextFree(Context context)
{
    ASSERT(hashTableUsed(context->pidToPipeTable) == 0);
    /* contextDeleteAll(context); */
    hashTableFree(context->pidToPipeTable);
    free(context);
}
