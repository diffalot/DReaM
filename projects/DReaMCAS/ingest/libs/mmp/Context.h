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
 * $(@)Context.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Context.h
 * 
 * Context is a pid to pipe context
 */

#ifndef	_MMP_CONTEXT_H
#define	_MMP_CONTEXT_H

#pragma ident "@(#)Context.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Context Error Codes
 *
 * NOTE: Use retCodeFor(CONTEXT_CLASSNAME, ContextError) to map to
 * RetCode.
 ***********************************************************************/

#define	CONTEXT_CLASSNAME		"Context"

typedef enum ContextError {
    CONTEXT_ERROR_PID_IN_USE = 1,	    /* pid already in use */
    CONTEXT_ERROR_NO_PID		    /* reference to unknown pid */
} ContextError;

/***********************************************************************
 * OBJECT Context Instance Type
 ***********************************************************************/
typedef struct _Context *Context;

/***********************************************************************
 * OBJECT Context Class Interface
 ***********************************************************************/

extern Context      contextNew(void);

/***********************************************************************
 * OBJECT Context Instance Interface
 ***********************************************************************/

extern MmpContextObject *contextMmpContextObject(Context context);
extern RetCode      contextAddPipe(void *instp, u16 pid, Pipe pipe);
extern RetCode      contextDeletePipe(void *instp, u16 pid);
extern Pipe         contextPidToPipe(void *instp, u16 pid);
extern u16          contextPipeToPid(void *instp, Pipe pipe);
extern void	    contextDeleteAll(void *instp);
extern void         contextFree(Context context);

_FOREIGN_END

#endif					   /* _MMP_CONTEXT_H */
