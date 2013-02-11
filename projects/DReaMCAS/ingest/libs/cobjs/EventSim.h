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
 * $(@)EventSim.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * EventSim.h
 * 
 * EventSim provides a discrete event simulation environment.
 */

#ifndef	_COBJS_EVENTSIM_H
#define	_COBJS_EVENTSIM_H

#pragma ident "@(#)EventSim.h 1.2	99/01/28 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <limits.h>
#include <stddef.h>

#include "cobjs/Foreign.h"
#include "cobjs/Types.h"
#include "cobjs/Time.h"

_FOREIGN_START

/*************************************************************************
 * OBJECT EventSim defines
 *************************************************************************/

#define	EVENT_TIME_FOREVER		LLONG_MAX

/***********************************************************************
 * OBJECT EventSim Instance Type
 ***********************************************************************/
typedef struct _EventSim *EventSim;
typedef struct _EventActor *EventActor;

/***********************************************************************
 * OBJECT EventSim Public Types
 ***********************************************************************/

typedef void        (*EventBody) (EventActor ea, void *arg);

/***********************************************************************
 * OBJECT EventSim Class Interface
 ***********************************************************************/
extern EventSim     eventSimNew(void);

/***********************************************************************
 * OBJECT EventSim Instance Interface
 ***********************************************************************/

extern double       eventSimSetRealTimeFrac(EventSim es, double realTimeFrac);

extern Time    eventSimSetStopTime(EventSim es, Time stopTime);
extern Time    eventSimNow(EventSim es);

extern void         eventSimRun(EventSim es);
extern void         eventSimStep(EventSim es);
extern void         eventSimStop(EventSim es);
extern void         eventSimWaitStopped(EventSim es);
extern void         eventSimFree(EventSim es);

extern EventActor
eventActorNew(EventSim es, size_t stackSize,
	      EventBody body, void *bodyArg);

extern void         eventActorFree(EventActor ea);

extern Boolean      eventActorShouldRun(EventActor ea);
extern Time    eventActorNow(EventActor ea);

extern Time    eventActorDelay(EventActor ea, Time delayTime);
extern Time    eventActorWait(EventActor ea);
extern Time    eventActorTimedWait(EventActor ea, Time maxWaitTime);

extern void         eventActorStopSim(EventActor ea);

extern void         eventActorSignal(EventActor ea);

_FOREIGN_END

#endif					   /* _COBJS_EVENTSIM_H */
