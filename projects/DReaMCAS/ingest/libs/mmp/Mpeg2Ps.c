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
 * $(@)Mpeg2Ps.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG2 program stream parser
 */

#pragma ident "@(#)Mpeg2Ps.c 1.2	97/05/08 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mpeg2Pes.h"
#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Ps.h"

#define	MPEG2_PS_MAX_CHUNK	(sizeof(Mpeg2PsPackAndCode))

struct _Mpeg2Ps {
    Mpeg2PsCallBack     callBack;
    void               *token;
    MmpParserObject    *pesp;
    MmpParserObject     po;
    Boolean		isDirty;
};

static size_t       mpeg2PsMaxChunk(void *instp);
static RetCode      mpeg2PsRecognize(void *instp, Pipe pipe);
static RetCode      mpeg2PsParse(void *instp, MmpContextObject *cop, Pipe pipe);
static void         mpeg2PsRecover(void *instp);
static RetCode
mpeg2PsSystemHeader(Mpeg2Ps mpeg2Ps, Pipe pipe,
		    Mpeg2PsInfo *infop);
static void         mpeg2PsPesSkip(Mpeg2Ps mpeg2Ps, Pipe pipe);

typedef struct Mpeg2PsStartCode {
    BIT1(startCode31_24:8);
    BIT1(startCode23_16:8);
    BIT1(startCode15_8:8);
    BIT1(startCode7_0:8);
} Mpeg2PsStartCode;

#define	IS_MPEG2_PS_START_CODE_PREFIX(p)				\
	( (  ((p)->startCode31_24 << 24)				\
	   | ((p)->startCode23_16 << 16)				\
	   | ((p)->startCode15_8 << 8)					\
	  ) == 0x100							\
	)

#define	MPEG2_PS_START_CODE(p)						\
	(   ((p)->startCode31_24 << 24)					\
	  | ((p)->startCode23_16 << 16)					\
	  | ((p)->startCode15_8 << 8)					\
	  |  (p)->startCode7_0						\
	)

typedef struct Mpeg2PsPackAndCode {
    Mpeg2PsPackHeader   packHeader;
    Mpeg2PsStartCode    startCode;
} Mpeg2PsPackAndCode;

static const PipeBits MPEG2_PS_P_STD_BUFFER_FLAG = {
    0x1,				   /* '1' */
    1					   /* 1 bits */
};

static RetCodeTable mpeg2PsErrorTable[] = {
    {MPEG2_PS_ERROR_PACK_SYNTAX, NULL, "pack syntax error"},
    {MPEG2_PS_ERROR_END_CODE, NULL, "missing or invalid end code"},
    {MPEG2_PS_ERROR_SYSTEM_HDR_LEN, NULL, "system header length error"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Ps
mpeg2PsNew(void)
{
    Mpeg2Ps             mpeg2Ps;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2PS_CLASSNAME,
					     mpeg2PsErrorTable);
    }
    mpeg2Ps = NEW_ZEROED(struct _Mpeg2Ps, 1);

    mpeg2Ps->po.maxChunk = mpeg2PsMaxChunk;
    mpeg2Ps->po.recognize = mpeg2PsRecognize;
    mpeg2Ps->po.parse = mpeg2PsParse;
    mpeg2Ps->po.recover = mpeg2PsRecover;
    mpeg2Ps->po.instp = mpeg2Ps;

    return mpeg2Ps;
}

void
mpeg2PsSetPesParser(Mpeg2Ps mpeg2Ps, MmpParserObject *pesp)
{
    mpeg2PsRecover(mpeg2Ps);
    mpeg2Ps->pesp = pesp;
}

MmpParserObject    *
mpeg2PsParserObject(Mpeg2Ps mpeg2Ps)
{
    return &mpeg2Ps->po;
}

void
mpeg2PsSetCallBack(Mpeg2Ps mpeg2Ps, Mpeg2PsCallBack callBack, void *token)
{
    mpeg2Ps->callBack = callBack;
    mpeg2Ps->token = token;
}

void
mpeg2PsFree(Mpeg2Ps mpeg2Ps)
{
    mpeg2PsRecover(mpeg2Ps);
    free(mpeg2Ps);
}

static size_t
mpeg2PsMaxChunk(void *instp)
{
    Mpeg2Ps             mpeg2Ps = (Mpeg2Ps) instp;

    return MAX(MPEG2_PS_MAX_CHUNK, MMP_PARSER_MAXCHUNK(mpeg2Ps->pesp));
}

/* ARGSUSED */
static RetCode
mpeg2PsRecognize(void *instp, Pipe pipe)
{
    Mpeg2PsPackAndCode *pacp = PIPE_PEEK(pipe, Mpeg2PsPackAndCode);

    return (MPEG2_PS_PACK_HEADER_START_CODE(&pacp->packHeader)
	    == MPEG2_PS_START_CODE_PACK
	    && pacp->packHeader.b01 == 0x1
	    && IS_MPEG2_PS_START_CODE_PREFIX(&pacp->startCode)
	    ? RETCODE_SUCCESS : RETCODE_FAILED);
}

static RetCode
mpeg2PsParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Ps             mpeg2Ps = (Mpeg2Ps) instp;
    Mpeg2PsInfo         info;
    Mpeg2PsInfo        *infop = &info;
    RetCode             retCode;
    Mpeg2PsStartCode   *scp;

    mpeg2Ps->isDirty = TRUE;
    do {
	(void) memset(infop, 0, sizeof(*infop));

	infop->position = pipePosition(pipe);
	infop->packHeader = *PIPE_GET(pipe, Mpeg2PsPackHeader);

	if (infop->packHeader.b01 != 0x1) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_PS_ERROR_PACK_SYNTAX);
	    goto error;
	}
	if (infop->packHeader.packStuffingLength != 0) {
	    retCode = pipeSkipStuffBytes(pipe,
	      infop->packHeader.packStuffingLength, MPEG2_PS_STUFFING_BYTE);
	    if (retCode != RETCODE_SUCCESS) {
		goto error;
	    }
	}
	scp = PIPE_PEEK(pipe, Mpeg2PsStartCode);
	if (MPEG2_PS_START_CODE(scp) == MPEG2_PS_START_CODE_SYSTEM) {
	    retCode = mpeg2PsSystemHeader(mpeg2Ps, pipe, infop);
	    if (retCode != RETCODE_SUCCESS) {
		goto error;
	    }
	}
	if (mpeg2Ps->callBack != NULL) {
	    retCode = (*mpeg2Ps->callBack) (mpeg2Ps->token, infop,
					    RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	if (mpeg2Ps->pesp == NULL) {
	    mpeg2PsPesSkip(mpeg2Ps, pipe);
	} else {
	    retCode = MMP_PARSER_PARSE(mpeg2Ps->pesp, cop, pipe);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	scp = PIPE_PEEK(pipe, Mpeg2PsStartCode);
    } while (MPEG2_PS_START_CODE(scp) == MPEG2_PS_START_CODE_PACK);

    scp = PIPE_GET(pipe, Mpeg2PsStartCode);
    infop->packHeader.packStartCode31_24 = scp->startCode31_24;
    infop->packHeader.packStartCode23_16 = scp->startCode23_16;
    infop->packHeader.packStartCode15_8 = scp->startCode15_8;
    infop->packHeader.packStartCode7_0 = scp->startCode7_0;
    if (MPEG2_PS_START_CODE(scp) != MPEG2_PS_START_CODE_END) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PS_ERROR_END_CODE);
	goto error;
    }
    return RETCODE_SUCCESS;

error:
    if (mpeg2Ps->callBack != NULL) {
	(void) (*mpeg2Ps->callBack) (mpeg2Ps->token, infop, retCode);
    }
    return retCode;
}

static void
mpeg2PsRecover(void *instp)
{
    Mpeg2Ps             mpeg2Ps = (Mpeg2Ps) instp;

    if (! mpeg2Ps->isDirty) {
	return;
    }
    mpeg2Ps->isDirty = FALSE;
    MMP_PARSER_RECOVER(mpeg2Ps->pesp);
}

/* ARGSUSED */
static RetCode
mpeg2PsSystemHeader(Mpeg2Ps mpeg2Ps, Pipe pipe, Mpeg2PsInfo *infop)
{
    PipePosition        shEnd = pipeRelativePosition(pipe);
    PipePosition        curPos;

    infop->hasSystemHeader = TRUE;
    infop->systemHeader = *PIPE_GET(pipe, Mpeg2PsSystemHeader);

    infop->nPStdBuffer = 0;
    shEnd += MPEG2_PS_SYSTEM_HEADER_LENGTH(&infop->systemHeader)
      + sizeof(Mpeg2PsStartCode) + 2;
    while (pipeIsNextBits(pipe, MPEG2_PS_P_STD_BUFFER_FLAG)) {
	if (infop->nPStdBuffer < MPEG2_PS_MAX_P_STD_BUFFERS) {
	    infop->pStdBuffer[infop->nPStdBuffer++] = *PIPE_GET(pipe,
							 Mpeg2PsPStdBuffer);
	} else {
	    PIPE_SKIP(pipe, Mpeg2PsPStdBuffer);
	}
    }
    if ((curPos = pipeRelativePosition(pipe)) != shEnd) {
	if (curPos > shEnd) {
	    return RETCODE_CONS(retCodeId, MPEG2_PS_ERROR_SYSTEM_HDR_LEN);
	}
	pipeSkip(pipe, shEnd - curPos);
    }
    return RETCODE_SUCCESS;
}

/* ARGSUSED */
static void
mpeg2PsPesSkip(Mpeg2Ps mpeg2Ps, Pipe pipe)
{
    Mpeg2PesPacket     *ppp;

    while ((ppp = PIPE_PEEK(pipe, Mpeg2PesPacket), TRUE)
	    && IS_MPEG2_PES_PACKET_START_CODE_PREFIX(ppp)
	    && MPEG2_PES_STREAM_ID_MIN <= ppp->streamId
	    && ppp->streamId <= MPEG2_PES_STREAM_ID_MAX) {
	pipeSkip(pipe, MPEG2_PES_PACKET_LENGTH(ppp) + sizeof(Mpeg2PesPacket));
    }
}
