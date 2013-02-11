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
 * $(@)Thread.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:35 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Thread.c -- Description.
 */

#pragma ident "@(#)Thread.c 1.5	99/12/10 SMI"

#if	!defined(THREAD_HEADER)
#define	THREAD_BODY
#define	THREAD_INLINE		extern
#include "cobjs/Thread.h"
#endif					   /* !defined(THREAD_HEADER) */

#include <stdlib.h>
#include <unistd.h>
#include <sys/priocntl.h>
#include <sys/resource.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

/*************************************************************************
 * Defines
 *************************************************************************/


/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void threadInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * THREAD_INLINE void
 * threadInlineRoutine(void)
 * {
 * }
 */

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(THREAD_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

/*************************************************************************
 * Private class data
 *************************************************************************/

/*************************************************************************
 * Non-inlinable Routines
 *************************************************************************/

void
threadSetRtPriority(int priority)
{
    pcinfo_t            pcinfo;
    pcparms_t           pcparms;
    rtparms_t          *rtparmsp;

    if (geteuid() != 0) {
	logLibPrint(cobjs_TEXT_DOMAIN,
		"WARNING: not super-user, cannot become realtime");
	return;
    }
    (void) memset(&pcinfo, 0, sizeof(pcinfo));
    (void) strcpy(pcinfo.pc_clname, NO_LOCALE("RT"));
    if (priocntl((idtype)0, 0, PC_GETCID, (caddr_t) & pcinfo) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "FATAL: priocntl()");
	exit(EXIT_FAILURE);
    }
    pcparms.pc_cid = pcinfo.pc_cid;
    rtparmsp = (rtparms_t *) pcparms.pc_clparms;
    /* LINTED */
    rtparmsp->rt_tqsecs = RT_TQDEF;
    rtparmsp->rt_tqnsecs = RT_TQDEF;
    rtparmsp->rt_pri = priority;

    if (priocntl(P_LWPID, P_MYID, PC_SETPARMS, (caddr_t) & pcparms) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "WARNING: priocntl()");
    }
}

void
threadSetTsPriority(int priority)
{
    pcinfo_t            pcinfo;
    pcparms_t           pcparms;
    tsparms_t          *tsparmsp;

    (void) memset(&pcinfo, 0, sizeof(pcinfo));
    (void) strcpy(pcinfo.pc_clname, NO_LOCALE("TS"));
    if (priocntl((idtype)0, 0, PC_GETCID, (caddr_t) & pcinfo) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "FATAL: priocntl()");
	exit(EXIT_FAILURE);
    }

    pcparms.pc_cid = pcinfo.pc_cid;

    tsparmsp = (tsparms_t *) pcparms.pc_clparms;
    tsparmsp->ts_uprilim = TS_NOCHANGE;
    tsparmsp->ts_upri = priority;

    if (priocntl(P_LWPID, P_MYID, PC_SETPARMS, (caddr_t) & pcparms) < 0) {
	logLibErrno(cobjs_TEXT_DOMAIN, "WARNING: priocntl()");
    }
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 * INLINE_PRIVATE void
 * threadInlinePrivate(void)
 * {
 * }
 * 
 * static void
 * threadPrivate(void)
 * {
 * }
 */

/*************************************************************************
 * Private Functions
 *************************************************************************/

/*
 * static void
 * threadFunc()
 * {
 * }
 */

#endif					   /* !defined(THREAD_HEADER) */
