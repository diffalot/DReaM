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
 * $(@)Pipe.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:39 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 */

/*
 * Pipe.c -- Description of Pipe
 *
 * This file generated from Tmplate version: 1.4 99/10/27
 * 
 * A Pipe is a object that passes data from a master thread to a slave thread in
 * a co-routine manner.  The master "puts" data to the Pipe and then gives up
 * control to the slave while the slave processes the passed data, when the
 * slave has consumed the data control returns to the master.  The slave
 * accesses data by requesting pointers to a contiguous chunk of data, the
 * max size of that chunk is specified at the time the Pipe in created.  Pipe
 * correctly handles collecting chunks when they break across master puts.
 * The pointer returned by Pipe to a slave and the data it points at is only
 * valid until the next call to Pipe, therefore, the slave should copy the
 * pointed to data before it's next call to get data.
 * 
 * The master indicates end of data to the slave by calling pipeEof(). A slave
 * read after pipeEof() will cause the slave to longjmp to an eof handler.
 * 
 * The slave should NEVER call thr_exit().
 */


#pragma ident "@(#)Pipe.c 1.8	99/12/10 SMI"

#if	!defined(PIPE_HEADER)
#define	PIPE_BODY
#define	PIPE_INLINE		extern
#include "mmp/Pipe.h"
#endif					   /* !defined(PIPE_HEADER) */

/***********************************************************************
 * Includes
 ***********************************************************************/

#ifdef	ASSERTS
#include <errno.h>
#endif	/* ASSERTS */
#include <setjmp.h>
#include <thread.h>
#include <synch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Crc32Table.h"

/*************************************************************************
 * Defines
 *************************************************************************/

#define	THR_BKG_BUG		0	/* job in bkg causes lwp starvation */

#define	PIPE_MAX_SEGMENTS	128
#define	N_BUF_POS		4	/* expect < 4 discontig segs residue */

/*************************************************************************
 * Instance Variables
 *************************************************************************/

typedef struct BufPosList {
    size_t		len;
    PipePosition	position;
} BufPosList;

typedef struct Buf {
    u8                 *basep;		/* base of buffer */
    size_t              len;		/* if alloc'ed, size of buf, else 0 */
    u8                 *outp;		/* current out pointer */
    size_t              avail;		/* chars in buffer */
    size_t              marked;		/* marked chars in buf */
    BufPosList		*bufPosList;	/* pos in orig stream */
    int			bplGet;		/* cur get entry in bufPosList */
    int			bplPut;		/* cur put entry in bufPosList */
    int			bplSize;	/* size of bufPosList */
} Buf;

typedef enum PipeThreadState {
    PIPETHREADSTATE_INIT,		/* hold til master init done */
    PIPETHREADSTATE_RUNNING,		/* parsing */
    PIPETHREADSTATE_ZOMBE		/* error, waiting to free */
} PipeThreadState;

struct _Pipe {
    Buf                 residueBuf;
    Buf                 masterBufs[PIPE_MAX_SEGMENTS];
    int			curSeg;
    int			nSegs;
    u32                 usedBits;

    PipePosition        position;	    /* pipe relative position */

    Boolean             doCrc;
    u32                 crcSum;

    int                *eofJmpBuf;
    RetCode             retCode;

    PipeParser          parser;
    PipeCleanup         cleanup;
    void               *instp;
    void               *cop;
    u16                 pid;

    Pipe                lastDstPipe;
    Boolean		isFlush;
    Boolean		isEof;

    PipeThreadState     threadState;
    sema_t              masterSema;
    sema_t              slaveSema;
    thread_t            threadId;
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void pipeInlinePrivate(void);
 */
static Buf         *pipeFillBuf(Pipe pipe, size_t size);
static void         pipeCrcSum(Pipe pipe, u8 *datap, int len);
static u8          *bufGet(Buf *bufp, size_t len);

#ifdef __cplusplus
extern "C" {
#endif

INLINE_PRIVATE Buf * pipeFillBuf2(Pipe pipe, size_t size);

#ifdef __cplusplus
}
#endif

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * PIPE_INLINE void *
 * pipeInlineRoutine(void)
 * {
 * }
 */

PIPE_INLINE u8 *
pipeGet(Pipe pipe, size_t size)
{
    u8                 *p;
    Buf		       *bufp;

    ASSERT(pipe->usedBits == 0);
    if ((bufp = pipeFillBuf(pipe, size)) == NULL) {
	longjmp(pipe->eofJmpBuf, 1);
    }
    pipe->position += size;
    p = bufGet(bufp, size);
    if (pipe->doCrc) {
	pipeCrcSum(pipe, p, size);
    }
    return p;
}

PIPE_INLINE u8 *
pipePeek(Pipe pipe, size_t size)
{
    Buf		       *bufp;

    ASSERT(pipe->usedBits == 0);
    if ((bufp = pipeFillBuf(pipe, size)) == NULL) {
	longjmp(pipe->eofJmpBuf, 1);
    }
    return bufp->outp;
}

PIPE_INLINE void
pipeSkip(Pipe pipe, size_t size)
{
    size_t              osize = size;
    size_t              rem;
    u8                 *p;

    ASSERT(pipe->usedBits == 0);
    while (size != 0) {
        Buf		       *bufp;

	if ((bufp = pipeFillBuf(pipe, 1)) == NULL) {
	    longjmp(pipe->eofJmpBuf, 1);
	}

	rem = MIN(size, bufp->avail);
	size -= rem;

	p = bufGet(bufp, rem);
	if (pipe->doCrc) {
	    pipeCrcSum(pipe, p, rem);
	}
    }
    pipe->position += osize;
}

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

static Buf         *
pipeFillBuf(Pipe pipe, size_t size)
{
    return pipe->isEof ? NULL
	: (pipe->residueBuf.avail != 0
	  || pipe->masterBufs[pipe->curSeg].avail < size)
		? pipeFillBuf2(pipe, size)
	: &pipe->masterBufs[pipe->curSeg];
}

#if	0
/**
 *            32   26   23   22   16   12   11   10   8   7   5   4   2
 * Calculate x  + x  + x  + x  + x  + x  + x  + x  + x + x + x + x + x + x + 1
 * 
 * 3 3 2 2  2 2 2 2  2 2 2 2  1 1 1 1  1 1 1 1  1 1 0 0  0 0 0 0  0 0 0 0
 * 1 0 9 8  7 6 5 4  3 2 1 0  9 8 7 6  5 4 3 2  1 0 9 8  7 6 5 4  3 2 1 0
 * -------  -------  -------  -------  -------  -------  -------  -------
 * 0 0 0 0  0 1 0 0  1 1 0 0  0 0 0 1  0 0 0 1  1 1 0 1  1 0 1 1  0 1 1 1
 *       0        4        c        1        1        d        b        7
 */

static void
pipeCrcSum(Pipe pipe, u8 *datap, int len)
{
    /*
     * Direct calculation code
     */
    while (len-- > 0) {
	int                 bit;
	u32                 input = *datap++ << 24;

	for (bit = 7; bit >= 0; bit -= 1) {
	    s32                 newZ0 = input ^ pipe->crcSum;

	    pipe->crcSum <<= 1;
	    if (newZ0 < 0) {
		pipe->crcSum ^= 0x04c11db7;
	    }
	    input <<= 1;
	}
    }
}
#endif	/* 0 */

static void
pipeCrcSum(Pipe pipe, u8 *datap, int len)
{
    /*
     * Table-based calculation code
     */
    while (len-- > 0) {
	pipe->crcSum = (pipe->crcSum >> 8)
	    ^ crc32Table[(pipe->crcSum ^ *datap++) & 0xff];
    }
}

static u8          *
bufGet(Buf *bufp, size_t size)
{
    u8                 *p = bufp->outp;

    ASSERT(size <= bufp->avail);
    if (size != 0) {
	bufp->outp += size;
	bufp->avail -= size;
	if (bufp->avail < bufp->marked) {
	    bufp->marked = 0;
	}
	while (size != 0) {
	    BufPosList *bplp = &bufp->bufPosList[bufp->bplGet];
	    size_t chunk = MIN(size, bplp->len);
	    ASSERT(bufp->bplGet < bufp->bplPut);
	    ASSERT(bplp->len > 0);
	    bplp->len -= chunk;
	    bplp->position += chunk;
	    if (bplp->len == 0) {
		bufp->bplGet += 1;
	    }
	    size -= chunk;
	}
    }
    return p;
}

#if	!defined(PIPE_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

static RetCode      pipePut2(Pipe pipe, void *buf, size_t len, Boolean isMarked,
			Boolean doFlush, PipePosition bufPosition);
static RetCode      pipeFlush(Pipe pipe, Boolean isForced);
static Boolean	    pipeIsPushBack(Pipe pipe);

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

static void        *pipeThreadWrapper(void *arg);

static Buf	    bufInit(u8 *buf, size_t len, int bplSize);
static void	    bufAssign(Buf *bufp, u8 *datap, size_t avail,
			Boolean isMarked, PipePosition bufPosition);
static void         bufTransfer(Buf *destBufp, Buf *srcBufp, size_t len);
static Boolean      bufIsAtMark(Buf *bufp);
static Boolean      bufIsEmpty(Buf *bufp);

/*************************************************************************
 * Private class data
 *************************************************************************/

static u32          bitMask[] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};

static RetCodeTable errorTable[] = {
    {PIPE_ERROR_EOF, NULL, "unexpected eof"},
    {PIPE_ERROR_STUFF_BYTE, NULL, "invalid stuff byte"},
    {PIPE_ERROR_EXTRANEOUS_DATA, NULL, "extraneous data at end of stream"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

/*************************************************************************
 * Class Methods
 *************************************************************************/

void
pipeInit(void)
{
    retCodeId = retCodeRegisterWithTable(PIPE_CLASSNAME, errorTable);
}

Pipe
pipeNew(size_t maxChunk, PipeParser parser, PipeCleanup cleanup,
	void *instp, void *cop)
{
    Pipe                newPipe = NEW_ZEROED(struct _Pipe, 1);
    int			i;

    if (retCodeId == 0) {
	pipeInit();
    }

    newPipe->curSeg = 0;
    newPipe->nSegs = 0;
    newPipe->residueBuf = bufInit(NEW(u8, maxChunk), maxChunk, N_BUF_POS);
    for (i = 0; i < NELEM(newPipe->masterBufs); i++) {
	newPipe->masterBufs[i] = bufInit(NULL, 0, 1);
    }
    /*
     * Slave code assumes that masterBufs[curSeg] is always valid,
     * so create an empty segment here to meet that expectation
     */
    bufAssign(&newPipe->masterBufs[newPipe->nSegs++], NULL, 0, FALSE, 0);
    newPipe->position = 0LL;
    newPipe->usedBits = 0;
    newPipe->doCrc = FALSE;
    newPipe->crcSum = 0xffffffff;
    newPipe->eofJmpBuf = NULL;
    newPipe->retCode = RETCODE_SUCCESS;
    newPipe->lastDstPipe = PIPE_NULL;
    newPipe->isFlush = FALSE;
    newPipe->isEof = FALSE;
    newPipe->parser = parser;
    newPipe->cleanup = cleanup;
    newPipe->instp = instp;
    newPipe->cop = cop;
    newPipe->pid = MMP_PID_NULL;
    newPipe->threadState = PIPETHREADSTATE_INIT;
    ABORT_IF_ERRNO(sema_init(&newPipe->masterSema, 0, USYNC_THREAD, NULL));
    ABORT_IF_ERRNO(sema_init(&newPipe->slaveSema, 0, USYNC_THREAD, NULL));
#if	THR_BKG_BUG != 0
    ABORT_IF_ERRNO(thr_create(NULL, 0, pipeThreadWrapper, newPipe, THR_NEW_LWP,
			  &newPipe->threadId));
#else					   /* THR_BKG_BUG */
    ABORT_IF_ERRNO(thr_create(NULL, 0, pipeThreadWrapper, newPipe, 0,
			  &newPipe->threadId));
#endif					   /* THR_BKG_BUG */
    return newPipe;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/


/*
 * Master-side calls
 */

/*
 * Free the pipe.
 * 
 * Returns retCode from thread.
 */
RetCode
pipeFree(Pipe pipe)
{
    RetCode             retCode;
    int			i;

    if (pipe == PIPE_NULL) {
	return RETCODE_SUCCESS;
    }
    retCode = pipeEof(pipe);
    ABORT_IF_ERRNO(thr_join(pipe->threadId, NULL, NULL));
    free(pipe->residueBuf.basep);
    free(pipe->residueBuf.bufPosList);
    for (i = 0; i < NELEM(pipe->masterBufs); i++) {
	free(pipe->masterBufs[i].bufPosList);
    }
    ABORT_IF_ERRNO(sema_destroy(&pipe->masterSema));
    ABORT_IF_ERRNO(sema_destroy(&pipe->slaveSema));
    free(pipe);
    return retCode;
}

/*
 * Puts len bytes for buf into stream and gives control to thread.
 * Should ONLY be called by original data source, not by a parser.
 */
RetCode
pipePut(Pipe pipe, void *buf, size_t len, Boolean isMarked,
	PipePosition putPosition)
{
    return pipePut2(pipe, buf, len, isMarked, TRUE, putPosition);
}

RetCode
pipeTransfer(Pipe dstPipe, Pipe srcPipe, size_t len,
	     Boolean isMarked, Boolean doFlush)
{
    RetCode retCode = RETCODE_SUCCESS;

    if (srcPipe->lastDstPipe != dstPipe
	&& (retCode = pipeFlush(srcPipe->lastDstPipe, TRUE))
		!= RETCODE_SUCCESS) {
	goto done;
    }
    if (dstPipe == PIPE_NULL) {
	pipeSkip(srcPipe, len);
	goto done;
    }
    srcPipe->lastDstPipe = dstPipe;
    while (len != 0) {
	Buf *bufp;
	size_t	chunk;
	PipePosition bufPosition;

	if ((bufp = pipeFillBuf(srcPipe, 1)) == NULL) {
	    longjmp(srcPipe->eofJmpBuf, 1);
	}
	chunk = MIN(len, bufp->avail);
	ASSERT(bufp->bplGet < bufp->bplPut);
	ASSERT(bufp->bufPosList[bufp->bplGet].len != 0);
	bufPosition = bufp->bufPosList[bufp->bplGet].position;
	if ((retCode = pipePut2(dstPipe, pipeGet(srcPipe, chunk), chunk,
			        isMarked, doFlush, bufPosition))
		!= RETCODE_SUCCESS) {
	    break;
	}
	isMarked = FALSE;
	len -= chunk;
    }
done:
    return retCode;
}

// This is a temporary function where the dst pipe is pipeNUll
// and hence the pipe skip goes into a file file 
RetCode
pipeTransfer2(Pipe dstPipe, Pipe srcPipe, size_t len,
	     Boolean isMarked, Boolean doFlush, int file)
{
    RetCode retCode = RETCODE_SUCCESS;
    u8 *p;
    while (len != 0) {
	Buf *bufp;
	size_t	chunk;
	PipePosition bufPosition;

	if ((bufp = pipeFillBuf(srcPipe, 1)) == NULL) {
	    longjmp(srcPipe->eofJmpBuf, 1);
	}
	chunk = MIN(len, bufp->avail);
	ASSERT(bufp->bplGet < bufp->bplPut);
	ASSERT(bufp->bufPosList[bufp->bplGet].len != 0);
	bufPosition = bufp->bufPosList[bufp->bplGet].position;
	

	p = pipeGet(srcPipe,chunk);
	fwrite(p,1,chunk, (FILE*)file);
/*	if ((retCode = pipePut2(dstPipe, pipeGet(srcPipe, chunk), chunk,
			        isMarked, doFlush, bufPosition))
		!= RETCODE_SUCCESS) {
	    break;
	}
*/
	isMarked = FALSE;
	len -= chunk;
    }
done:
    return retCode;
}

void
pipeSetPid(Pipe pipe, u16 pid)
{
    if (pipe != PIPE_NULL) {
	pipe->pid = pid;
    }
}

/*
 * Force all data placed onto child pipes to be parsed.
 * NOTE: This is called on the parent pipe!
 */
RetCode
pipeSync(Pipe pipe)
{
    return pipeFlush(pipe->lastDstPipe, TRUE);
}

RetCode
pipeEof(Pipe pipe)
{
    RetCode             retCode;

    if (pipe == PIPE_NULL) {
	return RETCODE_SUCCESS;
    }
    retCode = pipePut2(pipe, NULL, 0, FALSE, TRUE, 0);
    ASSERT(pipe->threadState == PIPETHREADSTATE_ZOMBE);
    return retCode;
}

void
pipeRecover(Pipe pipe)
{
    if (pipe == PIPE_NULL) {
	return;
    }
    (void) pipeEof(pipe);

    ABORT_IF_ERRNO(thr_join(pipe->threadId, NULL, NULL));

    pipeByteAlign(pipe);
    pipe->doCrc = FALSE;
    pipe->retCode = RETCODE_SUCCESS;
    pipe->threadState = PIPETHREADSTATE_INIT;
    ASSERT (sema_trywait(&pipe->masterSema) == EBUSY);
    ASSERT (sema_trywait(&pipe->slaveSema) == EBUSY);
#if	THR_BKG_BUG != 0
    ABORT_IF_ERRNO(thr_create(NULL, 0, pipeThreadWrapper, pipe, THR_BOUND,
			  &pipe->threadId));
#else					   /* THR_BKG_BUG */
    ABORT_IF_ERRNO(thr_create(NULL, 0, pipeThreadWrapper, pipe, 0,
			  &pipe->threadId));
#endif					   /* THR_BKG_BUG */
}

/*
 * Thread side calls
 */

RetCode
pipeSkipStuffBytes(Pipe pipe, size_t nStuffBytes, u8 stuffByte)
{
    size_t              osize = nStuffBytes;
    size_t              rem;
    u8                 *p;

    ASSERT(pipe->usedBits == 0);
    while (nStuffBytes != 0) {
        Buf		       *bufp;

	if ((bufp = pipeFillBuf(pipe, 1)) == NULL) {
	    longjmp(pipe->eofJmpBuf, 1);
	}

	rem = MIN(nStuffBytes, bufp->avail);
	nStuffBytes -= rem;

	p = bufGet(bufp, rem);
	if (pipe->doCrc) {
	    pipeCrcSum(pipe, p, rem);
	}

	while (rem != 0) {
	    if (*p++ != stuffByte) {
		return RETCODE_CONS(retCodeId, PIPE_ERROR_STUFF_BYTE);
	    }
	    rem -= 1;
	}
    }
    pipe->position += osize;
    return RETCODE_SUCCESS;
}

u32
pipeGetBits(Pipe pipe, size_t nBits)
{
    u32                 bits = pipePeekBits(pipe, nBits);

    pipeSkipBits(pipe, nBits);
    return bits;
}

u32
pipePeekBits(Pipe pipe, size_t nBits)
{
    size_t              gBits = pipe->usedBits + nBits;
    size_t              bytes = (gBits + 7) / 8;
    u8                 *p;
    u32                 buf;
    Buf                *bufp;

    ASSERT(nBits <= 32);
    if ((bufp = pipeFillBuf(pipe, bytes)) == NULL) {
	longjmp(pipe->eofJmpBuf, 1);
    }
    p = bufp->outp;
    if (gBits > 8) {
	buf = 0;
	do {
	    buf = (buf << 8) | *p++;
	    gBits -= 8;
	} while (gBits > 8);
	buf = (buf << gBits) | (*p >> (8 - gBits));
    } else {
	buf = *p >> (8 - gBits);
    }
    return buf & bitMask[nBits];
}

Boolean
pipeIsNextBits(Pipe pipe, PipeBits pipeBits)
{
    return Boolean(pipeBits.bitPattern == pipePeekBits(pipe, pipeBits.nBits));
}

void
pipeSkipBits(Pipe pipe, size_t nBits)
{
    pipe->usedBits += nBits;
    if (pipe->usedBits >= 8) {
	size_t usedBytes = pipe->usedBits / 8;
	size_t usedBits = pipe->usedBits & 7;

	pipe->usedBits = 0;
	pipeSkip(pipe, usedBytes);
	pipe->usedBits = usedBits;
    }
}

void
pipePeekByteBlock(Pipe pipe, u8 *buf, size_t nBytes)
{
    u8                 *p;
    Buf                *bufp;

    if (pipe->usedBits == 0) {
	p = pipePeek(pipe, nBytes);
	while (nBytes-- > 0) {
	    *buf++ = *p++;
	}
    } else {
	u8                  oldByte;

	if ((bufp = pipeFillBuf(pipe, nBytes + 1)) == NULL) {
	    longjmp(pipe->eofJmpBuf, 1);
	}
	p = bufp->outp;
	oldByte = *p++;
	while (nBytes-- > 0) {
	    u8                  newByte = *p++;

	    *buf++ = (oldByte << (pipe->usedBits))
	      | (newByte >> (8 - pipe->usedBits));
	    oldByte = newByte;
	}
    }
}

void
pipeGetByteBlock(Pipe pipe, u8 *buf, size_t nBytes)
{
    pipePeekByteBlock(pipe, buf, nBytes);
    pipeSkipBits(pipe, nBytes * 8);
}

void
pipeByteAlign(Pipe pipe)
{
    if (pipe->usedBits != 0) {
	pipe->usedBits = 0;
	pipeSkip(pipe, 1);
    }
}

void
pipeFindMark(Pipe pipe)
{
    ASSERT(pipe->usedBits == 0);
    while (!pipeIsAtMark(pipe)) {
	if (pipeFillBuf(pipe, 1) == NULL) {
	    longjmp(pipe->eofJmpBuf, 1);
	}
	pipeSkip(pipe, pipeAvailUnmarked(pipe));
    }
}

Boolean
pipeIsAtMark(Pipe pipe)
{
    ASSERT(pipe->usedBits == 0);
    return Boolean(bufIsAtMark(&pipe->residueBuf)
      || (bufIsEmpty(&pipe->residueBuf)
	  && bufIsAtMark(&pipe->masterBufs[pipe->curSeg])));
}

/*
 * Returns true if chunk can be got/peeked without hitting eof
 */
Boolean
pipeIsAvail(Pipe pipe, size_t size)
{
    ASSERT(pipe->usedBits == 0);
    return Boolean(pipeFillBuf(pipe, size) != NULL);
}

Boolean
pipeIsAvailUnmarked(Pipe pipe, size_t chunk)
{
    return Boolean(pipeIsAvail(pipe, chunk) && pipeAvailUnmarked(pipe) >= chunk);
}

size_t
pipeAvailUnmarked(Pipe pipe)
{
    size_t              unmarked;

    unmarked = pipe->residueBuf.avail - pipe->residueBuf.marked;
    if (pipe->residueBuf.marked == 0) {
	unmarked += pipe->masterBufs[pipe->curSeg].avail
	    - pipe->masterBufs[pipe->curSeg].marked;
    }
    return unmarked;
}

PipePosition
pipePosition(Pipe pipe)
{
    Buf *bufp;

    if ((bufp = pipeFillBuf(pipe, 1)) == NULL) {
	/*
	 * There is no position for a pipe at EOF
	 */
	longjmp(pipe->eofJmpBuf, 1);
    }
    return bufp->bufPosList[bufp->bplGet].position;
}

/*
 * Return the byte position in slave relative terms.
 */
PipePosition
pipeRelativePosition(Pipe pipe)
{
    ASSERT(pipe->usedBits == 0);
    return pipe->position;
}

/*
 * Clear crc accumulator.
 */
void
pipeCrcClear(Pipe pipe)
{
    ASSERT(pipe->usedBits == 0);
    pipe->doCrc = TRUE;
    pipe->crcSum = 0xffffffff;
}

/*
 * Get accumulated crc since last crc clear.
 */
u32
pipeCrcGet(Pipe pipe)
{
    ASSERT(pipe->usedBits == 0);
    pipe->doCrc = FALSE;
    return ~pipe->crcSum;
}

u16
pipeGetPid(Pipe pipe)
{
    return pipe->pid;
}

PipeParser
pipeGetParser(Pipe pipe)
{
    return pipe->parser;
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 *
 * INLINE_PRIVATE void
 * pipeInlinePrivate(void)
 * {
 * }
 */

static RetCode
pipePut2(Pipe pipe, void *buf, size_t len, Boolean isMarked,
	Boolean doFlush, PipePosition bufPosition)
{
    if (pipe == PIPE_NULL) {
	return RETCODE_SUCCESS;
    }
    switch (pipe->threadState) {
    case PIPETHREADSTATE_INIT:
	/*
	 * Thread is waiting in pipeThreadWrapper for master to complete
	 * instance initialization, start thread so it can run until it's
	 * first pipeFillBuf(). NOTE: It's illegal to detect an error before
	 * the first pipeFillBuf().
	 */
	(void) pipeFlush(pipe, FALSE);
	if (pipe->threadState == PIPETHREADSTATE_ZOMBE) {
	    break;
	}
	ASSERT(pipe->threadState == PIPETHREADSTATE_RUNNING);
	/* FALLTHRU */
    case PIPETHREADSTATE_RUNNING:
	bufAssign(&pipe->masterBufs[pipe->nSegs++], (unsigned char *)buf, len, isMarked,
		bufPosition);
	if (doFlush || pipe->nSegs >= PIPE_MAX_SEGMENTS) {
	    (void) pipeFlush(pipe, doFlush);
	}
	break;
    case PIPETHREADSTATE_ZOMBE:
	break;
    default:
	ABORT("Illegal threadState");
    }
    return pipe->retCode;
}

static RetCode
pipeFlush(Pipe pipe, Boolean isForced)
{
    if (pipe == PIPE_NULL) {
	return RETCODE_SUCCESS;
    }
    if (pipe->nSegs > 0 && pipe->threadState != PIPETHREADSTATE_ZOMBE) {
	pipe->isFlush = isForced;
        CHECK_IF_ERRNO(sema_post(&pipe->slaveSema));
        CHECK_IF_ERRNO(sema_wait(&pipe->masterSema));
	if (pipe->threadState != PIPETHREADSTATE_ZOMBE) {
	    ASSERT(pipe->curSeg == pipe->nSegs);
            pipe->nSegs = 0;
	}
    }
    return pipe->retCode;
}

INLINE_PRIVATE Buf         *
pipeFillBuf2(Pipe pipe, size_t size)
{
    Buf                *bufp;

    if (pipe->isEof) {
	return NULL;
    }

    ASSERT(size <= pipe->residueBuf.len);
    ASSERT(pipe->curSeg < pipe->nSegs);
    while (size > pipe->residueBuf.avail
	   + pipe->masterBufs[pipe->curSeg].avail) {
	if (pipe->masterBufs[pipe->curSeg].avail > 0) {
	    (void) pipeFlush(pipe->lastDstPipe, TRUE);
	    bufTransfer(&pipe->residueBuf, &pipe->masterBufs[pipe->curSeg],
			pipe->masterBufs[pipe->curSeg].avail);
	}
	pipe->curSeg += 1;
	if (pipe->curSeg >= pipe->nSegs) {
	    if (pipe->isFlush) {
		(void) pipeFlush(pipe->lastDstPipe, TRUE);
	    }
	    CHECK_IF_ERRNO(sema_post(&pipe->masterSema));
	    CHECK_IF_ERRNO(sema_wait(&pipe->slaveSema));
	    ASSERT(pipe->nSegs > 0);
	    pipe->curSeg = 0;
	}
	if (pipe->masterBufs[pipe->curSeg].avail == 0) {
	    pipe->curSeg += 1;
	    ASSERT(pipe->curSeg == pipe->nSegs);
	    pipe->isEof = TRUE;
	    return NULL;
	}
	ASSERT(pipe->masterBufs[pipe->curSeg].outp
	       == pipe->masterBufs[pipe->curSeg].basep);
    }

    ASSERT(pipe->residueBuf.avail + pipe->masterBufs[pipe->curSeg].avail
	   >= size);
    if (pipe->residueBuf.avail == 0 || pipeIsPushBack(pipe)) {
	bufp = &pipe->masterBufs[pipe->curSeg];
    } else {
	if (pipe->residueBuf.avail < size) {
	    size_t              rem = size - pipe->residueBuf.avail;

	    (void) pipeFlush(pipe->lastDstPipe, TRUE);
	    bufTransfer(&pipe->residueBuf,
			&pipe->masterBufs[pipe->curSeg], rem);
	}
	bufp = &pipe->residueBuf;
    }
    return bufp;
}

static Boolean
pipeIsPushBack(Pipe pipe)
{
    Buf *masterBufp = &pipe->masterBufs[pipe->curSeg];
    BufPosList *bplp;
    size_t residueAvail;
    size_t residueMarked;

    if (pipe->residueBuf.avail > masterBufp->outp - masterBufp->basep) {
	return FALSE;
    }
    residueAvail = pipe->residueBuf.avail;
    residueMarked = pipe->residueBuf.marked;
    (void) bufGet(&pipe->residueBuf, residueAvail);
    if (residueMarked != 0) {
	ASSERT(masterBufp->marked == 0);
	masterBufp->marked = masterBufp->avail + residueMarked;
    }
    masterBufp->outp -= residueAvail;
    masterBufp->avail += residueAvail;
    ASSERT(masterBufp->bplGet < masterBufp->bplPut);
    bplp = &masterBufp->bufPosList[masterBufp->bplGet];
    bplp->len += residueAvail;
    bplp->position -= residueAvail;
    return TRUE;
}

/*************************************************************************
 * Private Functions
 *************************************************************************/

static void        *
pipeThreadWrapper(void *arg)
{
    Pipe                pipe = (Pipe) arg;
    jmp_buf             jbuf;
    RetCode		retCode;

    CHECK_IF_ERRNO(sema_wait(&pipe->slaveSema));
    pipe->threadState = PIPETHREADSTATE_RUNNING;
    pipe->eofJmpBuf = jbuf;
    if (setjmp(jbuf) != 0) {
	pipe->retCode = RETCODE_CONS(retCodeId, PIPE_ERROR_EOF);
	goto done;
    }
    pipe->retCode = (*pipe->parser) (pipe->instp, pipe->cop, pipe);
    /*
     * Flush any pending sub-pipe data.  If there's an error, it takes
     * precedence -- the data was buffered and this error actually occurred
     * earlier in the stream.
     */
    if ((retCode = pipeFlush(pipe->lastDstPipe, TRUE)) != RETCODE_SUCCESS) {
	pipe->retCode = retCode;
    }
    if (pipe->retCode == RETCODE_SUCCESS
        && pipe->residueBuf.avail + pipe->masterBufs[pipe->curSeg].avail > 0) {
	pipe->retCode = RETCODE_CONS(retCodeId, PIPE_ERROR_EXTRANEOUS_DATA);
    }
done:
    /*
     * Call parser to cleanup any child pipes, etc.
     */
    if (pipe->cleanup != NULL) {
	(*pipe->cleanup) (pipe->instp);
    }
    pipe->threadState = PIPETHREADSTATE_ZOMBE;
    CHECK_IF_ERRNO(sema_post(&pipe->masterSema));
    return NULL;
}

static Buf
bufInit(u8 *buf, size_t len, int bplSize)
{
    Buf                 aBuf;

    aBuf.basep = aBuf.outp = buf;
    aBuf.len = len;
    aBuf.avail = 0;
    aBuf.marked = 0;
    aBuf.bufPosList = NEW_ZEROED(BufPosList, bplSize);
    aBuf.bplSize = bplSize;
    aBuf.bplGet = 0;
    aBuf.bplPut = 0;
    return aBuf;
}

static void
bufAssign(Buf *bufp, u8 *datap, size_t avail, Boolean isMarked,
	PipePosition bufPosition)
{
    ASSERT(bufp->len == 0);
    ASSERT(bufp->bplSize == 1);
    bufp->basep = bufp->outp = datap;
    bufp->avail = avail;
    bufp->marked = isMarked ? avail : 0;
    bufp->bufPosList[0].position = bufPosition;
    bufp->bufPosList[0].len = avail;
    bufp->bplGet = 0;
    bufp->bplPut = 1;
}

static void
bufTransfer(Buf *destBufp, Buf *srcBufp, size_t len)
{
    Boolean isSrcMarked = Boolean(srcBufp->marked != 0);
    PipePosition srcPosition = srcBufp->bufPosList[srcBufp->bplGet].position;
    BufPosList *bplp;

    /*
     * "Justify" data in buffer
     */
    if (destBufp->outp != destBufp->basep) {
	if (destBufp->avail != 0) {
	    (void) memmove(destBufp->basep, destBufp->outp, destBufp->avail);
	}
	destBufp->outp = destBufp->basep;
    }
    /*
     * Justify bufPosList
     */
    if (destBufp->bplGet > 0) {
	if (destBufp->bplPut > destBufp->bplGet) {
	    (void) memmove(destBufp->bufPosList,
		       &destBufp->bufPosList[destBufp->bplGet],
		       sizeof(destBufp->bufPosList[0])
		       * (destBufp->bplPut - destBufp->bplGet));
	}
	destBufp->bplPut -= destBufp->bplGet;
	destBufp->bplGet = 0;
    }
    /*
     * Move data from src to dest
     */
    ASSERT(destBufp->outp + destBufp->avail + len
	    <= destBufp->basep + destBufp->len);
    (void) memcpy(destBufp->outp + destBufp->avail, bufGet(srcBufp, len), len);
    destBufp->avail += len;
    if (destBufp->marked != 0 || isSrcMarked) {
	destBufp->marked += len;
    }
    /*
     * Update bufPosList
     */
    bplp = &destBufp->bufPosList[destBufp->bplPut - 1];
    if (destBufp->bplPut > destBufp->bplGet
	&& bplp->position + bplp->len == srcPosition) {
	bplp->len += len;
    } else {
	if (destBufp->bplPut >= destBufp->bplSize) {
	    destBufp->bplSize *= 2;
	    RENEW(BufPosList, destBufp->bufPosList, destBufp->bplSize);
	}
	bplp = &destBufp->bufPosList[destBufp->bplPut++];
	bplp->len = len;
	bplp->position = srcPosition;
    }
}

static Boolean
bufIsAtMark(Buf *bufp)
{
    return Boolean(bufp->marked != 0 && bufp->avail == bufp->marked);
}

static Boolean
bufIsEmpty(Buf *bufp)
{
    return Boolean(bufp->avail == 0);
}

#endif					   /* !defined(PIPE_HEADER) */
