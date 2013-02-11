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
 * $(@)Timer.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Timer.h -- Description
 */

#ifndef	_COBJS_TIMER_H
#define	_COBJS_TIMER_H

#pragma ident "@(#)Timer.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"
#include "cobjs/StatMgr.h"

_FOREIGN_START

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _Timer *Timer;

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef StatMgrStats TimerStats;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TIMER_BODY)
#define	TIMER_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(TIMER_BODY) */
#define	TIMER_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(TIMER_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern Timer     timerNew(void);

/***********************************************************************
 * Instance Interface
 ***********************************************************************/

/*
 * Clear the timer and all stats
 */
extern void timerClear(Timer timer);

/*
 * Start the timer at its current position
 */
extern void timerStart(Timer timer);

/*
 * Stop and restart the timer and return the elapsed time
 */
extern double timerSplit(Timer timer);

/*
 * Stop the timer and return the elapsed time
 */
extern double timerStop(Timer timer);

/*
 * Get the timer's current value
 */
extern double timerValue(Timer timer);

/*
 * Get complete timer statistics
 */
extern TimerStats timerStats(Timer timer);

/*
 * Free the timer
 */
extern void         timerFree(Timer timer);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TIMER_BODY)
#define	TIMER_HEADER
#include "cobjs/Timer.c"
#undef	TIMER_HEADER
#endif		   /* defined(DO_INLINING) && !defined(TIMER_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_TIMER_H */
