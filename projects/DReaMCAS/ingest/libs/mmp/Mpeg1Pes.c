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
 * $(@)Mpeg1Pes.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG1 PES parser
 */

#pragma ident "@(#)Mpeg1Pes.c 1.1	96/09/18 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Context.h"
#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg1Pes.h"

#define	MPEG1_PES_MAX_CHUNK	(256)	   /* FIXME: figure out right value */

struct _Mpeg1Pes {
    MmpParserObject    *streamIdToPop[MPEG1_PES_STREAM_ID_MAX + 1];
    Mpeg1PesCallBack    callBack;
    void               *token;
    MmpParserObject     po;
    Context		context;
    MmpContextObject   *cop;
    Boolean		isDirty;
    Boolean		doFlush;
};

typedef struct Mpeg1PesPacketAndFlag {
    Mpeg1PesPacket      pesPacket;
                        BIT2(bflag:2,
			                         ignored:6);
} Mpeg1PesPacketAndFlag;

static size_t       mpeg1PesMaxChunk(void *instp);
static RetCode      mpeg1PesRecognize(void *instp, Pipe pipe);

static RetCode
mpeg1PesParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg1PesRecover(void *instp);

static const PipeBits MPEG1_PES_STD_BUFFER_FLAG = {
    0x1,				   /* '01' */
    2					   /* 2 bits */
};

static const PipeBits MPEG1_PES_PTS_FLAG = {
    0x2,				   /* '0010' */
    4					   /* 4 bits */
};

static const PipeBits MPEG1_PES_PTS_DTS_FLAG = {
    0x3,				   /* '0011' */
    4					   /* 4 bits */
};

static RetCodeTable mpeg1PesErrorTable[] = {
    {MPEG1_PES_ERROR_SYNTAX, NULL, "packet syntax"},
    {MPEG1_PES_ERROR_LEN, NULL, "packet length mismatch"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg1Pes
mpeg1PesNew(void)
{
    Mpeg1Pes            mpeg1Pes;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG1PES_CLASSNAME,
					     mpeg1PesErrorTable);
    }
    mpeg1Pes = NEW_ZEROED(struct _Mpeg1Pes, 1);

    mpeg1Pes->po.maxChunk = mpeg1PesMaxChunk;
    mpeg1Pes->po.recognize = mpeg1PesRecognize;
    mpeg1Pes->po.parse = mpeg1PesParse;
    mpeg1Pes->po.recover = mpeg1PesRecover;
    mpeg1Pes->po.instp = mpeg1Pes;

    mpeg1Pes->context = contextNew();
    mpeg1Pes->cop = contextMmpContextObject(mpeg1Pes->context);

    return mpeg1Pes;
}

void
mpeg1PesSetEsParsers(Mpeg1Pes mpeg1Pes,
		MmpParserObject *streamIdToPop[MPEG1_PES_STREAM_ID_MAX + 1])
{
    mpeg1PesRecover(mpeg1Pes);
    (void) memcpy(mpeg1Pes->streamIdToPop, streamIdToPop,
		  sizeof(mpeg1Pes->streamIdToPop));
}

MmpParserObject    *
mpeg1PesParserObject(Mpeg1Pes mpeg1Pes)
{
    return &mpeg1Pes->po;
}

void
mpeg1PesSetCallBack(Mpeg1Pes mpeg1Pes, Mpeg1PesCallBack callBack, void *token)
{
    mpeg1Pes->callBack = callBack;
    mpeg1Pes->token = token;
}

void
mpeg1PesSetFlush(Mpeg1Pes mpeg1Pes, Boolean doFlush)
{
    mpeg1Pes->doFlush = doFlush;
}

void
mpeg1PesFree(Mpeg1Pes mpeg1Pes)
{
    mpeg1PesRecover(mpeg1Pes);
    contextFree(mpeg1Pes->context);
    free(mpeg1Pes);
}

/* ARGSUSED */
static size_t
mpeg1PesMaxChunk(void *instp)
{
    return MPEG1_PES_MAX_CHUNK;
}

/* ARGSUSED */
static RetCode
mpeg1PesRecognize(void *instp, Pipe pipe)
{
    Mpeg1PesPacketAndFlag *pfp = PIPE_PEEK(pipe, Mpeg1PesPacketAndFlag);

    return (IS_MPEG1_PES_PACKET_START_CODE_PREFIX(&pfp->pesPacket)
	    && MPEG1_PES_STREAM_ID_MIN <= pfp->pesPacket.streamId
	    && pfp->pesPacket.streamId >= MPEG1_PES_STREAM_ID_MAX
	 && (pfp->pesPacket.streamId == MPEG1_PES_STREAM_ID_PRIVATE_STREAM_2
	     || pfp->bflag != 2))
      ? RETCODE_SUCCESS : RETCODE_FAILED;
}

/* ARGSUSED */
static RetCode
mpeg1PesParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg1Pes            mpeg1Pes = (Mpeg1Pes) instp;
    Mpeg1PesInfo        info;
    Mpeg1PesInfo       *infop = &info;
    RetCode             retCode;
    Mpeg1PesPacket     *ppp;

    mpeg1Pes->isDirty = TRUE;
    while ((ppp = PIPE_PEEK(pipe, Mpeg1PesPacket), TRUE)
	    && IS_MPEG1_PES_PACKET_START_CODE_PREFIX(ppp)
	    && MPEG1_PES_STREAM_ID_MIN <= ppp->streamId
	    && ppp->streamId <= MPEG1_PES_STREAM_ID_MAX) {
	PipePosition        pktEnd;
	PipePosition        payloadLen;
	Pipe                pidPipe;
	unsigned            streamId;

	(void) memset(infop, 0, sizeof(*infop));

	infop->position = pipePosition(pipe);
	pktEnd = pipeRelativePosition(pipe);
	infop->packet = *PIPE_GET(pipe, Mpeg1PesPacket);

	/*
	 * add 6 bytes since packet length starts after the 4 bytes of start
	 * code and 2 bytes of packet length
	 */
	pktEnd += MPEG1_PES_PACKET_LENGTH(&infop->packet) + 6;
	streamId = infop->packet.streamId;
	if (streamId != MPEG1_PES_STREAM_ID_PRIVATE_STREAM_2) {
	    while (*PIPE_PEEK(pipe, u8) == MPEG1_PES_STUFF_BYTE) {
		PIPE_SKIP(pipe, u8);
	    }
	    if (pipeIsNextBits(pipe, MPEG1_PES_STD_BUFFER_FLAG)) {
		infop->hasStdBuffer = TRUE;
		infop->stdBuffer = *PIPE_GET(pipe, Mpeg1PesStdBuffer);
	    }
	    if (pipeIsNextBits(pipe, MPEG1_PES_PTS_FLAG)) {
		infop->hasPresentationTimeStamp = TRUE;
		infop->presentationTimeStamp = *PIPE_GET(pipe,
							 Mpeg1PesTimeStamp);
	    } else if (pipeIsNextBits(pipe, MPEG1_PES_PTS_DTS_FLAG)) {
		infop->hasPresentationTimeStamp = TRUE;
		infop->hasDecodingTimeStamp = TRUE;
		infop->presentationTimeStamp = *PIPE_GET(pipe,
							 Mpeg1PesTimeStamp);
		infop->decodingTimeStamp = *PIPE_GET(pipe, Mpeg1PesTimeStamp);
	    } else if (*PIPE_GET(pipe, u8) !=0x0f) {
		retCode = RETCODE_CONS(retCodeId, MPEG1_PES_ERROR_SYNTAX);
		goto error;
	    }
	}
	if (mpeg1Pes->callBack != NULL) {
	    retCode = (*mpeg1Pes->callBack) (mpeg1Pes->token, infop,
					     RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	if ((pidPipe = MMP_CONTEXT_PIDTOPIPE(mpeg1Pes->cop, streamId))
	    == NULL) {
	    MmpParserObject    *pop = mpeg1Pes->streamIdToPop[streamId];

	    pidPipe = MMP_PARSER_NEWPIPE(pop, mpeg1Pes->cop);
	    /*
	     * This can't fail...  we just checked that pipePipe == NULL
	     */
	    retCode = MMP_CONTEXT_ADDPIPE(mpeg1Pes->cop, streamId, pidPipe);
	    ABORT_IF_FALSE(retCode == RETCODE_SUCCESS);
	}
	payloadLen = pktEnd - pipeRelativePosition(pipe);
	if (payloadLen < 0) {
	    retCode = RETCODE_CONS(retCodeId, MPEG1_PES_ERROR_LEN);
	    goto error;
	}
	if ((retCode = pipeTransfer(pidPipe, pipe, payloadLen, FALSE,
			mpeg1Pes->doFlush)) != RETCODE_SUCCESS) {
	    return retCode;
	}
    }
    return RETCODE_SUCCESS;

error:
    if (mpeg1Pes->callBack != NULL) {
	(void) (*mpeg1Pes->callBack) (mpeg1Pes->token, infop, retCode);
    }
    return retCode;
}

static void
mpeg1PesRecover(void *instp)
{
    Mpeg1Pes            mpeg1Pes = (Mpeg1Pes) instp;
    unsigned            streamId;

    if (! mpeg1Pes->isDirty) {
	return;
    }
    mpeg1Pes->isDirty = FALSE;
    MMP_CONTEXT_DELETEALL(mpeg1Pes->cop);

    for (streamId = MPEG1_PES_STREAM_ID_MIN;
	    streamId <= MPEG1_PES_STREAM_ID_MAX; streamId++) {
	MMP_PARSER_RECOVER(mpeg1Pes->streamIdToPop[streamId]);
    }
}
