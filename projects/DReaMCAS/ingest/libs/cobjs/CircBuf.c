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
 * $(@)CircBuf.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 */

/*
 * CircBuf.c -- Description of CircBuf
 *
 * This file generated from Tmplate version: 1.3 98/11/18
 */


#pragma ident "@(#)CircBuf.c 1.1	99/02/08 SMI"

#if	!defined(CIRCBUF_HEADER)
#define	CIRCBUF_BODY
#define	CIRCBUF_INLINE		extern
#include "cobjs/CircBuf.h"
#endif					   /* !defined(CIRCBUF_HEADER) */

#include <stdlib.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _CircBuf {
    size_t		in;
    size_t		out;
    size_t		size;
    char		*buf;
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void circBufInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * Get address and size of contiguous free "put" space in circular buffer
 */
CIRCBUF_INLINE size_t
circBufPutAddr(CircBuf circBuf, char **bufp)
{
    *bufp = &circBuf->buf[circBuf->in];
    return circBuf->in < circBuf->out
	? circBuf->out - circBuf->in - 1
	: circBuf->size - circBuf->in;
}

/*
 * Advance circular buffer put count
 */
CIRCBUF_INLINE void
circBufPutAdvance(CircBuf circBuf, size_t cnt)
{
    ASSERT(cnt <= circBufPutCount(circBuf));

    circBuf->in += cnt;
    if (circBuf->in >= circBuf->size) {
	circBuf->in -= circBuf->size;
    }
}

/*
 * Return number of characters in circular buffer than can be "put"
 */
CIRCBUF_INLINE size_t
circBufPutCount(CircBuf circBuf)
{
    return circBuf->in <= circBuf->out
	? circBuf->out - circBuf->in - 1
	: circBuf->size - circBuf->in + circBuf->out - 1;
}

/*
 * Get address and size of contiguous filled "get" space in circular buffer
 */
CIRCBUF_INLINE size_t
circBufGetAddr(CircBuf circBuf, char **bufp)
{
    *bufp = &circBuf->buf[circBuf->out];
    return circBuf->out <= circBuf->in
	? circBuf->in - circBuf->out
	: circBuf->size - circBuf->out;
}

/*
 * Advance circular buffer get count
 */
CIRCBUF_INLINE void
circBufGetAdvance(CircBuf circBuf, size_t cnt)
{
    ASSERT(cnt <= circBufGetCount(circBuf));

    circBuf->out += cnt;
    if (circBuf->out >= circBuf->size) {
	circBuf->out -= circBuf->size;
    }
}

/*
 * Return number of characters in circular buffer than can be "got"
 */
CIRCBUF_INLINE size_t
circBufGetCount(CircBuf circBuf)
{
    return circBuf->out <= circBuf->in
	? circBuf->in - circBuf->out
	: circBuf->size - circBuf->out + circBuf->in;
}

/*
 * Flush the contents of the circBuf;
 */
CIRCBUF_INLINE void
circBufFlush(CircBuf circBuf)
{
    circBuf->in = circBuf->out;
}

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(CIRCBUF_HEADER)

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
 * Class Methods
 *************************************************************************/

CircBuf
circBufNew(size_t size)
{
    CircBuf circBuf = NEW_ZEROED(struct _CircBuf, 1);

    circBuf->in = 0;
    circBuf->out = 0;
    circBuf->size = size;
    circBuf->buf = NEW(char, circBuf->size);

    return circBuf;
}

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

/*
 * CIRCBUF_INLINE void *circBufInline(CircBuf circBuf);
 */

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

/*************************************************************************
 * Instance Methods
 *************************************************************************/

/*
 * Move cnt bytes into circular buffer
 * Returns unput chars
 */
size_t
circBufPut(CircBuf circBuf, const char *buf, size_t cnt)
{
    char *cp;
    size_t contig;

#if	__lint
    contig = 0;
#endif	/* __lint */
    while (cnt != 0 && (contig = circBufPutAddr(circBuf, &cp)) != 0) {
	size_t len = MIN(cnt, contig);
	(void) memcpy(cp, buf, len);
	circBufPutAdvance(circBuf, len);
	buf += len;
	cnt -= len;
    }
    return cnt;
}

/*
 * Move cnt bytes from circular buffer to buf
 */
size_t
circBufGet(CircBuf circBuf, char *buf, size_t cnt)
{
    char *cp;
    size_t contig;

#if	__lint
    contig = 0;
#endif	/* __lint */
    while (cnt != 0 && (contig = circBufGetAddr(circBuf, &cp)) != 0) {
	size_t len = MIN(cnt, contig);
	(void) memcpy(buf, cp, len);
	circBufGetAdvance(circBuf, len);
	buf += len;
	cnt -= len;
    }
    return cnt;
}

void
circBufFree(CircBuf circBuf)
{
    free(circBuf->buf);
    free(circBuf);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 *
 * INLINE_PRIVATE void
 * circBufInlinePrivate(void)
 * {
 * }
 */

/*************************************************************************
 * Private Functions
 *************************************************************************/

#endif					   /* !defined(CIRCBUF_HEADER) */
