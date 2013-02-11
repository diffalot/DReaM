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
 * $(@)SchedCall.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/******************************************************************************
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 ******************************************************************************/

/******************************************************************************
 * SchedCall.h
 *
 * A SchedCall is an object which schedules callbacks in the future.
 ******************************************************************************/

#ifndef	_COBJS_SCHEDCALL_H
#define	_COBJS_SCHEDCALL_H

#pragma ident "@(#)SchedCall.h 1.2	98/12/18 SMI"

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Time.h"
#include "cobjs/Types.h"

_FOREIGN_START

/******************************************************************************
 * OBJECT SchedCall Instance Type
 ******************************************************************************/
typedef struct _SchedCall *SchedCall;

/******************************************************************************
 * OBJECT SchedCall Public Types
 ******************************************************************************/

/*
 * Prototype for routine called at future date
 */
typedef void        (*SchedCallHandler) (Object delegate, Time schedDate);

/*
 * A handle for a schedCall entry
 */
typedef void       *SchedCallTag;

/******************************************************************************
 * OBJECT SchedCall Class Interface
 ******************************************************************************/
/*
 * Initialize global SchedCall object.
 */
extern void
schedCallInit(Time accuracy, Boolean isRealtime, int priority);

/*
 * Shutdown global SchedCall object.
 */
extern void         schedCallFini(void);

/*
 * Creates a new schedCall object.  If accuracy is non-zero, all schedcall
 * times will be rounded up to units of accuracy seconds.
 */
extern SchedCall
schedCallNew(Time accuracy, Boolean isRealtime,
		int priority);

/******************************************************************************
 * OBJECT SchedCall Instance Interface
 ******************************************************************************/

/*
 * Schedules a future call of handler with arg at date. If sc == NULL, use
 * global schedCall object.
 */
extern SchedCallTag
schedCallAdd(SchedCall sc, Time date,
	     SchedCallHandler handler, Object delegate);


/*
 * Deletes scheduled future call.  Returns TRUE if tag was found; return
 * FALSE if not (and therefore the call has already occurred). If sc == NULL,
 * use global schedCall object.
 */
extern Boolean      schedCallRemove(SchedCall sc, SchedCallTag tag);

/*
 * Free a schedCall object.
 */
extern void         schedCallFree(SchedCall sc);

_FOREIGN_END

#endif					   /* _COBJS_SCHEDCALL_H */
