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
 * $(@)Thread.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Thread.h -- Thread support routines.
 */

#ifndef	_COBJS_THREAD_H
#define	_COBJS_THREAD_H

#pragma ident "@(#)Thread.h 1.2	99/09/07 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

/***********************************************************************
 * Public Types
 ***********************************************************************/

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(THREAD_BODY)
#define	THREAD_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(THREAD_BODY) */
#define	THREAD_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(THREAD_BODY) */

/***********************************************************************
 * Inline-able Routines
 ***********************************************************************/

/*
 * THREAD_INLINE void threadInline(Thread thread);
 */

/***********************************************************************
 * Non-inline-able Routine Interface
 ***********************************************************************/

extern void threadSetRtPriority(int priority);
extern void threadSetTsPriority(int priority);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(THREAD_BODY)
#define	THREAD_HEADER
#include "cobjs/Thread.c"
#undef	THREAD_HEADER
#endif		   /* defined(DO_INLINING) && !defined(THREAD_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_THREAD_H */
