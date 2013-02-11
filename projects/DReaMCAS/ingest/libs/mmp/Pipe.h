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
 * $(@)Pipe.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
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
 * Pipe.h -- Description of Pipe
 *
 * This file generated from Tmplate version: 1.4 99/10/27
 * 
 * A Pipe is a object that passes data from a master thread to a slave thread
 * utilizing co-routines.  The master "puts" data to the Pipe and then gives
 * up control to the slave while the slave processes the passed data, when
 * the slave has consumed the data control returns to the master.  The slave
 * accesses data by requesting pointers to a contiguous chunk of data, the
 * max size of that chunk is specified at the time the Pipe in created.  Pipe
 * correctly handles collecting chunks when they break across master puts.
 * The pointer returned by Pipe to a slave and the data it points at is only
 * valid until the next call to Pipe, therefore, the slave should copy the
 * pointed to data if it will be needed beyond the duration of the current
 * co-routine tenure.
 */

#ifndef	_MMP_PIPE_H
#define	_MMP_PIPE_H

#pragma ident "@(#)Pipe.h 1.6	99/12/10 SMI"

/***********************************************************************
 * Global Includes
 ***********************************************************************/

#include <setjmp.h>
#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

#define	PIPE_NULL		((Pipe) 1)

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _Pipe *Pipe;

/***********************************************************************
 * RetCode Support
 * NOTE: Use retCodeFor(PIPE_CLASSNAME, PipeError) to map to
 * RetCode.
 ***********************************************************************/

#define	PIPE_CLASSNAME	    "Pipe"

/*
 * Pipe Error Codes
 */
typedef enum PipeError {
    PIPE_ERROR_EOF = 1, 		   /* unexpected eof */
    PIPE_ERROR_STUFF_BYTE,		   /* invalid stuff byte */
    PIPE_ERROR_EXTRANEOUS_DATA		   /* extraneous data at stream end */
} PipeError;

/***********************************************************************
 * Local Includes
 ***********************************************************************/

_FOREIGN_START
/* local includes go here */
_FOREIGN_END

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef RetCode     (*PipeParser) (void *instp, void *cop, Pipe pipe);
typedef void        (*PipeCleanup) (void *instp);

/*
 * Stream offset.  Must be signed.
 */
typedef long long PipePosition;

/*
 * Pipe bit string spec
 */
typedef struct PipeBits {
    u32                 bitPattern;
    size_t              nBits;
} PipeBits;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PIPE_BODY)
#define	PIPE_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(PIPE_BODY) */
#define	PIPE_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(PIPE_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern void pipeInit(void);

extern Pipe
pipeNew(size_t maxChunk, PipeParser pipeParser, PipeCleanup pipeCleanup,
	void *instp, void *cop);

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

/*
 * Slave calls
 */

/*
 * Get contiguous bytes from pipe
 */
PIPE_INLINE u8          *pipeGet(Pipe pipe, size_t chunk);

#define	PIPE_GET(pipe, type)					\
	((type *)pipeGet(pipe, sizeof(type)))

/*
 * Peek at contiguous bytes from pipe
 */
PIPE_INLINE u8          *pipePeek(Pipe pipe, size_t chunk);

#define	PIPE_PEEK(pipe, type)					\
	((type *)pipePeek(pipe, sizeof(type)))

/*
 * Skip bytes
 */
PIPE_INLINE void         pipeSkip(Pipe pipe, size_t chunk);

#define	PIPE_SKIP(pipe, type)					\
	(pipeSkip(pipe, sizeof(type)))

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

/*
 * Master calls
 */

/*
 * Puts len bytes for buf into stream and gives control to thread.
 */
extern RetCode
pipePut(Pipe pipe, void *buf, size_t len,
	Boolean isMarked, PipePosition putPosition);

/*
 * Transfer len bytes from src pipe to dst pipe
 */
extern RetCode
pipeTransfer(Pipe dstPipe, Pipe srcPipe, size_t len,
	     Boolean isMarked, Boolean doFlush);

/*
 * Mark that the pipe will receive no more data.
 */
extern RetCode      pipeEof(Pipe pipe);

/*
 * Clear any errors on pipe and restart parser
 */
extern void         pipeRecover(Pipe pipe);

/*
 * Record pid that this pipe is associated with.
 */
extern void         pipeSetPid(Pipe pipe, u16 pid);

/*
 * Force all data placed onto child pipes to be parsed.
 * NOTE: This is called on the parent pipe!
 */
extern RetCode	    pipeSync(Pipe pipe);

/*
 * Free the pipe.
 * 
 * Returns retCode from thread.
 */
extern RetCode      pipeFree(Pipe pipe);

/*
 * Slave calls
 */

/*
 * Verify and skip stuff bytes
 */
extern RetCode
pipeSkipStuffBytes(Pipe pipe, size_t nStuffBytes,
		   u8 stuffByte);

/*
 * Get nBits from pipe
 */
extern u32          pipeGetBits(Pipe pipe, size_t nBits);

/*
 * Peek at nBits from pipe
 */
extern u32          pipePeekBits(Pipe pipe, size_t nBits);

/*
 * Peek and verify n bytes from pipe
 */
extern Boolean      pipeIsNextBits(Pipe pipe, PipeBits pipeBits);

/*
 * Skip nbits on pipe
 */
extern void         pipeSkipBits(Pipe pipe, size_t nBits);

/*
 * Peek at non-byte aligned block of bytes
 */
extern void         pipePeekByteBlock(Pipe pipe, u8 *buf, size_t nBytes);

/*
 * Get non-byte aligned block of bytes
 */
extern void         pipeGetByteBlock(Pipe pipe, u8 *buf, size_t nBytes);

/*
 * Force pipe to byte boundary
 */
extern void         pipeByteAlign(Pipe pipe);

/*
 * Skip until pipe is at mark
 */
extern void         pipeFindMark(Pipe pipe);

/*
 * Return TRUE if pipe is at mark
 */
extern Boolean      pipeIsAtMark(Pipe Pipe);

/*
 * Returns true if chunk can be got/peeked without hitting eof
 */
extern Boolean      pipeIsAvail(Pipe pipe, size_t chunk);

#define	PIPE_IS_AVAIL(costream, type)				\
	pipeIsAvail(costream, sizeof(type))

/*
 * Returns true if chunk can be got/peeked without hitting mark or eof
 */
extern Boolean      pipeIsAvailUnmarked(Pipe pipe, size_t chunk);

#define	PIPE_IS_AVAIL_UNMARKED(costream, type)				\
	pipeIsAvailUnmarked(costream, sizeof(type))

/*
 * Return the current number of unmarked bytes
 */
extern size_t       pipeAvailUnmarked(Pipe pipe);


/*
 * Return the byte position in slave relative terms.
 */
extern PipePosition pipeRelativePosition(Pipe pipe);

/*
 * Return the byte position in global terms
 */
extern PipePosition pipePosition(Pipe pipe);

/*
 * Clear crc accumulator.
 */
extern void         pipeCrcClear(Pipe pipe);

/*
 * Get accumulated crc since last crc clear.
 */
extern u32          pipeCrcGet(Pipe pipe);

/*
 * Get pid that pipe is carrying
 */
extern u16          pipeGetPid(Pipe pipe);

/*
 * Return pipe parser func
 */
extern PipeParser pipeGetParser(Pipe pipe);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PIPE_BODY)
#define	PIPE_HEADER
#include "Pipe.c"
#undef	PIPE_HEADER
#endif		   /* defined(DO_INLINING) && !defined(PIPE_BODY) */

_FOREIGN_END

#endif					   /* _MMP_PIPE_H */
