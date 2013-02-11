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
 * $(@)Mpeg1Sys.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:38 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG1 system stream parser
 */

#pragma ident "@(#)Mpeg1Sys.c 1.2	97/05/08 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mmp.h"
#include "mmp/Mpeg1Pes.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg1Sys.h"

#define	MPEG1_SYS_MAX_CHUNK	(sizeof(Mpeg1SysPackAndCode))

struct _Mpeg1Sys {
    Mpeg1SysCallBack    callBack;
    void               *token;
    MmpParserObject    *pesp;
    MmpParserObject     po;
    Boolean		isDirty;
};

typedef struct Mpeg1SysStartCode {
    BIT1(startCode31_24:8);
    BIT1(startCode23_16:8);
    BIT1(startCode15_8:8);
    BIT1(startCode7_0:8);
} Mpeg1SysStartCode;

#define	IS_MPEG1_SYS_START_CODE_PREFIX(p)				\
	( (  ((p)->startCode31_24 << 24)				\
	   | ((p)->startCode23_16 << 16)				\
	   | ((p)->startCode15_8 << 8)					\
	  ) == 0x100							\
	)

#define	IS_MPEG1_SYS_START_CODE(p, sc)					\
	( (  ((p)->startCode31_24 << 24)				\
	   | ((p)->startCode23_16 << 16)				\
	   | ((p)->startCode15_8 << 8)					\
	   |  (p)->startCode7_0						\
	  ) == (sc)							\
	)

typedef struct Mpeg1SysPackAndCode {
    Mpeg1SysPack        pack;
    Mpeg1SysStartCode   startCode2;
} Mpeg1SysPackAndCode;

static size_t       mpeg1SysMaxChunk(void *instp);
static RetCode      mpeg1SysRecognize(void *instp, Pipe pipe);

static RetCode
mpeg1SysParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg1SysRecover(void *instp);

static RetCode
mpeg1SysSystemHeader(Mpeg1Sys mpeg1Sys, Pipe pipe,
		     Mpeg1SysInfo *infop);

static void	    mpeg1SysPesSkip(Mpeg1Sys mpeg1Sys, Pipe pipe);

static const PipeBits MPEG1_SYS_STD_BUFFER_FLAG = {
    0x1,				   /* '1' */
    1					   /* 1 bits */
};

static RetCodeTable mpeg1SysErrorTable[] = {
    {MPEG1_SYS_ERROR_SYNTAX, NULL, "pack syntax error"},
    {MPEG1_SYS_ERROR_END_CODE, NULL, "missing end code"},
    {MPEG1_SYS_ERROR_SYSTEM_HDR_LEN, NULL, "system header length error"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg1Sys
mpeg1SysNew(void)
{
    Mpeg1Sys            mpeg1Sys;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG1SYS_CLASSNAME,
					     mpeg1SysErrorTable);
    }
    mpeg1Sys = NEW_ZEROED(struct _Mpeg1Sys, 1);

    mpeg1Sys->po.maxChunk = mpeg1SysMaxChunk;
    mpeg1Sys->po.recognize = mpeg1SysRecognize;
    mpeg1Sys->po.parse = mpeg1SysParse;
    mpeg1Sys->po.recover = mpeg1SysRecover;
    mpeg1Sys->po.instp = mpeg1Sys;

    return mpeg1Sys;
}

void
mpeg1SysSetPesParser(Mpeg1Sys mpeg1Sys, MmpParserObject *pesp)
{
    mpeg1SysRecover(mpeg1Sys);
    mpeg1Sys->pesp = pesp;
}

MmpParserObject    *
mpeg1SysParserObject(Mpeg1Sys mpeg1Sys)
{
    return &mpeg1Sys->po;
}

void
mpeg1SysSetCallBack(Mpeg1Sys mpeg1Sys, Mpeg1SysCallBack callBack, void *token)
{
    mpeg1Sys->callBack = callBack;
    mpeg1Sys->token = token;
}

void
mpeg1SysFree(Mpeg1Sys mpeg1Sys)
{
    free(mpeg1Sys);
}

static size_t
mpeg1SysMaxChunk(void *instp)
{
    Mpeg1Sys            mpeg1Sys = (Mpeg1Sys) instp;

    return MAX(MPEG1_SYS_MAX_CHUNK, MMP_PARSER_MAXCHUNK(mpeg1Sys->pesp));
}

/* ARGSUSED */
static RetCode
mpeg1SysRecognize(void *instp, Pipe pipe)
{
    Mpeg1SysPackAndCode *pacp = PIPE_PEEK(pipe, Mpeg1SysPackAndCode);

    return (MPEG1_SYS_PACK_START_CODE(&pacp->pack) == MPEG1_SYS_START_CODE_PACK
	    && pacp->pack.b0010 == 0x2
	    && IS_MPEG1_SYS_START_CODE_PREFIX(&pacp->startCode2))
      ? RETCODE_SUCCESS : RETCODE_FAILED;
}

/* ARGSUSED */
static RetCode
mpeg1SysParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg1Sys            mpeg1Sys = (Mpeg1Sys) instp;
    Mpeg1SysInfo        info;
    Mpeg1SysInfo       *infop = &info;
    RetCode             retCode;
    Mpeg1SysStartCode  *scp;

    mpeg1Sys->isDirty = TRUE;
    do {
	(void) memset(infop, 0, sizeof(*infop));
	infop->position = pipePosition(pipe);
	infop->pack = *PIPE_GET(pipe, Mpeg1SysPack);

	if (infop->pack.b0010 != 0x2) {
	    retCode = RETCODE_CONS(retCodeId, MPEG1_SYS_ERROR_SYNTAX);
	    goto error;
	}
	scp = PIPE_PEEK(pipe, Mpeg1SysStartCode);
	if (IS_MPEG1_SYS_START_CODE(scp, MPEG1_SYS_START_CODE_SYSTEM)) {
	    retCode = mpeg1SysSystemHeader(mpeg1Sys, pipe, infop);
	    if (retCode != RETCODE_SUCCESS) {
		goto error;
	    }
	}
	if (mpeg1Sys->callBack != NULL) {
	    retCode = (*mpeg1Sys->callBack) (mpeg1Sys->token, infop,
					     RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	if (mpeg1Sys->pesp == NULL) {
	    mpeg1SysPesSkip(mpeg1Sys, pipe);
	} else {
	    retCode = MMP_PARSER_PARSE(mpeg1Sys->pesp, NULL, pipe);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	scp = PIPE_PEEK(pipe, Mpeg1SysStartCode);
    } while (IS_MPEG1_SYS_START_CODE(scp, MPEG1_SYS_START_CODE_PACK));

    scp = PIPE_GET(pipe, Mpeg1SysStartCode);
    infop->pack.packStartCode31_24 = scp->startCode31_24;
    infop->pack.packStartCode23_16 = scp->startCode23_16;
    infop->pack.packStartCode15_8 = scp->startCode15_8;
    infop->pack.packStartCode7_0 = scp->startCode7_0;
    if (!IS_MPEG1_SYS_START_CODE(scp, MPEG1_SYS_START_CODE_END)) {
	retCode = RETCODE_CONS(retCodeId, MPEG1_SYS_ERROR_END_CODE);
	goto error;
    }
    return RETCODE_SUCCESS;

error:
    if (mpeg1Sys->callBack != NULL) {
	(void) (*mpeg1Sys->callBack) (mpeg1Sys->token, infop, retCode);
    }
    return retCode;
}

static void
mpeg1SysRecover(void *instp)
{
    Mpeg1Sys            mpeg1Sys = (Mpeg1Sys) instp;

    if (! mpeg1Sys->isDirty) {
	return;
    }
    mpeg1Sys->isDirty = FALSE;
    MMP_PARSER_RECOVER(mpeg1Sys->pesp);
}

/* ARGSUSED */
static RetCode
mpeg1SysSystemHeader(Mpeg1Sys mpeg1Sys, Pipe pipe, Mpeg1SysInfo *infop)
{
    PipePosition        shEnd = pipeRelativePosition(pipe);
    PipePosition        curPos;

    infop->hasSystemHeader = TRUE;
    infop->systemHeader = *PIPE_GET(pipe, Mpeg1SysSystemHeader);

    infop->nStdBuffer = Boolean(0);
    shEnd += MPEG1_SYS_SYSTEM_HEADER_LENGTH(&infop->systemHeader)
      + sizeof(Mpeg1SysStartCode) + 2;
    while (pipeIsNextBits(pipe, MPEG1_SYS_STD_BUFFER_FLAG)) {
	if (infop->nStdBuffer < MPEG1_SYS_MAX_STD_BUFFERS) {
        int nbuf = infop->nStdBuffer+1;
        infop->nStdBuffer = (Boolean)nbuf;
	    infop->stdBuffer[nbuf] = *PIPE_GET(pipe,
							 Mpeg1SysStdBuffer);
	} else {
	    PIPE_SKIP(pipe, Mpeg1SysStdBuffer);
	}
    }
    if ((curPos = pipeRelativePosition(pipe)) != shEnd) {
	if (curPos > shEnd) {
	    return RETCODE_CONS(retCodeId, MPEG1_SYS_ERROR_SYSTEM_HDR_LEN);
	}
	pipeSkip(pipe, shEnd - curPos);
    }
    return RETCODE_SUCCESS;
}

/* ARGSUSED */
static void
mpeg1SysPesSkip(Mpeg1Sys mpeg1Sys, Pipe pipe)
{
    Mpeg1PesPacket     *ppp;

    while ((ppp = PIPE_PEEK(pipe, Mpeg1PesPacket), TRUE)
	    && IS_MPEG1_PES_PACKET_START_CODE_PREFIX(ppp)
	    && MPEG1_PES_STREAM_ID_MIN <= ppp->streamId
	    && ppp->streamId <= MPEG1_PES_STREAM_ID_MAX) {
	pipeSkip(pipe, MPEG1_PES_PACKET_LENGTH(ppp) + sizeof(Mpeg1PesPacket));
    }
}
