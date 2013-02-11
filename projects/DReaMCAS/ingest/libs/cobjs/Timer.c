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
 * $(@)Timer.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Timer.c -- Description.
 */

#pragma ident "@(#)Timer.c 1.1	98/10/22 SMI"

#if	!defined(TIMER_HEADER)
#define	TIMER_BODY
#define	TIMER_INLINE		extern
#include "cobjs/Timer.h"
#endif					   /* !defined(TIMER_HEADER) */

#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/Time.h"
#include "cobjs/StatMgr.h"

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _Timer {
    Time	epochStart;
    Boolean	isRunning;

    StatMgr	statMgr;
};

#if	!defined(TIMER_HEADER)

/*************************************************************************
 * Class Methods
 *************************************************************************/

Timer
timerNew(void)
{
    Timer timer = NEW_ZEROED(struct _Timer, 1);
    timer->statMgr = statMgrNew();
    return timer;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

/*
 * Clear timer
 */
void
timerClear(Timer timer)
{
    timer->epochStart = 0LL;
    timer->isRunning = FALSE;
    statMgrClear(timer->statMgr);
}

/*
 * Start the timer at its current position
 */
void
timerStart(Timer timer)
{
    if (! timer->isRunning) {
	timer->isRunning = TRUE;
	timer->epochStart = timeFromBoot();
    }
}

/*
 * Stop and immediately restart the timer and return the elapsed time
 */
double
timerSplit(Timer timer)
{
    Time newEpochStart;
    double delta;

    if (! timer->isRunning) {
	timerStart(timer);
	return 0.0;
    }

    newEpochStart = timeFromBoot();
    delta = timeToDouble(newEpochStart - timer->epochStart);
    statMgrRecord(timer->statMgr, delta);
    timer->epochStart = newEpochStart;
    return delta;
}

/*
 * Stop the timer and return the elapsed time
 */
double
timerStop(Timer timer)
{
    double delta;

    if (! timer->isRunning) {
	return 0.0;
    }
    delta = timerSplit(timer);
    timer->isRunning = FALSE;
    return delta;
}

/*
 * Return current timer value
 */
double
timerValue(Timer timer)
{
    if (! timer->isRunning) {
	return 0.0;
    }

    return timeToDouble(timeFromBoot() - timer->epochStart);
}

/*
 * Get complete timer statistics
 */
TimerStats
timerStats(Timer timer)
{
    return statMgrStats(timer->statMgr);
}

/*
 * Free the timer
 */
void
timerFree(Timer timer)
{
    statMgrFree(timer->statMgr);
    free(timer);
}

#endif					   /* !defined(TIMER_HEADER) */
