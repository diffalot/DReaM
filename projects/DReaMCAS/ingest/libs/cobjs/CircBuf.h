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
 * $(@)CircBuf.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 */

/*
 * CircBuf.h -- Description of CircBuf
 *
 * This file generated from Tmplate version: 1.3 98/11/18
 */

#ifndef	_COBJS_CIRCBUF_H
#define	_COBJS_CIRCBUF_H

#pragma ident "@(#)CircBuf.h 1.1	99/02/08 SMI"

/***********************************************************************
 * Global Includes
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
 * Instance Types
 ***********************************************************************/

typedef struct _CircBuf *CircBuf;

/***********************************************************************
 * Local Includes
 ***********************************************************************/

_FOREIGN_START
/* local includes go here */
_FOREIGN_END

/***********************************************************************
 * Public Types
 ***********************************************************************/

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(CIRCBUF_BODY)
#define	CIRCBUF_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(CIRCBUF_BODY) */
#define	CIRCBUF_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(CIRCBUF_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern CircBuf     circBufNew(size_t size);

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

/*
 * Get address and size of contiguous free "put" space in circular buffer
 */
CIRCBUF_INLINE size_t circBufPutAddr(CircBuf circBuf, char **bufp);

/*
 * Advance circular buffer put count
 */
CIRCBUF_INLINE void circBufPutAdvance(CircBuf circBuf, size_t cnt);

/*
 * Return number of characters in circular buffer than can be "put"
 */
CIRCBUF_INLINE size_t circBufPutCount(CircBuf circBuf);

/*
 * Get address and size of contiguous filled "get" space in circular buffer
 */
CIRCBUF_INLINE size_t circBufGetAddr(CircBuf circBuf, char **bufp);

/*
 * Advance circular buffer get count
 */
CIRCBUF_INLINE void circBufGetAdvance(CircBuf circBuf, size_t cnt);

/*
 * Return number of characters in circular buffer than can be "got"
 */
CIRCBUF_INLINE size_t circBufGetCount(CircBuf circBuf);

/*
 * Flush the contents of the circBuf;
 */
CIRCBUF_INLINE void circBufFlush(CircBuf circBuf);

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

/*
 * Move cnt bytes into circular buffer
 * Returns count of chars that could not be put into circBuf because
 * circBuf filled.
 */
extern size_t circBufPut(CircBuf circBuf, const char *buf, size_t cnt);

/*
 * Move cnt bytes from circular buffer to buf
 * Returns count of chars that could not be gotten from circBuf because
 * circBuf emptied.
 */
extern size_t circBufGet(CircBuf circBuf, char *buf, size_t cnt);

/*
 * Free circBuf
 */
extern void circBufFree(CircBuf circBuf);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(CIRCBUF_BODY)
#define	CIRCBUF_HEADER
#include "cobjs/CircBuf.c"
#undef	CIRCBUF_HEADER
#endif		   /* defined(DO_INLINING) && !defined(CIRCBUF_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_CIRCBUF_H */
