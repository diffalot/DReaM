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
 * $(@)Delay.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Delay.c -- Description.
 */

#pragma ident "@(#)Delay.c 1.3	99/03/22 SMI"

#include <stdlib.h>
#include <synch.h>

#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Time.h"

#include "cobjs/Delay.h"

static mutex_t nullMutex;
static cond_t nullCond;

static void
delayInit(void)
{
    ABORT_IF_ERRNO(mutex_init(&nullMutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(cond_init(&nullCond, USYNC_THREAD, 0));
}

#pragma init (delayInit)

static void
delayFini(void)
{
    ABORT_IF_ERRNO(mutex_destroy(&nullMutex));
    ABORT_IF_ERRNO(cond_destroy(&nullCond));
}

#pragma fini (delayFini)

void
delayTil(Time dayTime)
{
    timestruc_t ts;
    int error;

#ifdef	__lint
    dayTime += DELAY_FOO;
#endif	/* __lint */
    ts = timeToTimestruc(dayTime);
    CHECK_IF_ERRNO(mutex_lock(&nullMutex));
    do {
	switch (error = cond_timedwait(&nullCond, &nullMutex, &ts)) {
	case 0:		/* libthread bug! */
	case EINTR:
	case ETIME:
	    break;
	default:
	    ABORT_WITH_ERRNO(error, "cond_timedwait");
	}
    } while (error != ETIME);
    CHECK_IF_ERRNO(mutex_unlock(&nullMutex));
}

void
delayFor(Time delayTime)
{
    delayTil(timeOfDay() + delayTime);
}

void
delaySecs(double delaySecs)
{
    delayTil(timeOfDay() + timeFromDouble(delaySecs));
}
