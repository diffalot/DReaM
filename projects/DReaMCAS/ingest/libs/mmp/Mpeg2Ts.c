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
 * $(@)Mpeg2Ts.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:39 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG2 transport stream parser
 */

#pragma ident "@(#)Mpeg2Ts.c 1.1	96/09/18 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Context.h"
#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Ts.h"

#define	MPEG2_TS_MAX_CHUNK	(MPEG2_TS_PKT_SIZE + 1)

struct _Mpeg2Ts {
    Mpeg2TsCallBack     callBack;
    void               *token;
    MmpParserObject    *psip;		   /* PSI parser */
    MmpParserObject     po;
    Context		context;
    MmpContextObject   *cop;
    Boolean		isDirty;
    Boolean		doFlush;
};

static size_t       mpeg2TsMaxChunk(void *instp);
static RetCode      mpeg2TsRecognize(void *instp, Pipe pipe);
static RetCode      mpeg2TsParse(void *instp, MmpContextObject *cop, Pipe pipe);
static void         mpeg2TsRecover(void *instp);

static RetCode
mpeg2TsAdaptationField(Mpeg2Ts mpeg2Ts, Pipe pipe,
		       Mpeg2TsInfo *infop);

static RetCode
mpeg2TsAdaptationFieldExtension(Mpeg2Ts mpeg2Ts, Pipe pipe,
				Mpeg2TsInfo *infop);

static RetCodeTable mpeg2TsErrorTable[] = {
    {MPEG2_TS_ERROR_SYNC, NULL, "bad sync byte"},
    {MPEG2_TS_ERROR_TEI, NULL, "transport error indicator set"},
    {MPEG2_TS_ERROR_AFC, NULL, "invalid adaptation field code"},
    {MPEG2_TS_ERROR_LEN, NULL, "packet length error"},
    {MPEG2_TS_ERROR_AFL, NULL, "invalid adaptation field length"},
    {MPEG2_TS_ERROR_AFEL, NULL, "invalid adaptation field ext length"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Ts
mpeg2TsNew(void)
{
    Mpeg2Ts             mpeg2Ts;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2TS_CLASSNAME,
					     mpeg2TsErrorTable);
    }
    mpeg2Ts = NEW_ZEROED(struct _Mpeg2Ts, 1);

    mpeg2Ts->po.maxChunk = mpeg2TsMaxChunk;
    mpeg2Ts->po.recognize = mpeg2TsRecognize;
    mpeg2Ts->po.parse = mpeg2TsParse;
    mpeg2Ts->po.recover = mpeg2TsRecover;
    mpeg2Ts->po.instp = mpeg2Ts;

    mpeg2Ts->context = contextNew();
    mpeg2Ts->cop = contextMmpContextObject(mpeg2Ts->context);

    return mpeg2Ts;
}

void
mpeg2TsSetPsiParser(Mpeg2Ts mpeg2Ts, MmpParserObject *psip)
{
    mpeg2TsRecover(mpeg2Ts);
    mpeg2Ts->psip = psip;
}

MmpParserObject    *
mpeg2TsParserObject(Mpeg2Ts mpeg2Ts)
{
    return &mpeg2Ts->po;
}

void
mpeg2TsSetCallBack(Mpeg2Ts mpeg2Ts, Mpeg2TsCallBack callBack, void *token)
{
    mpeg2Ts->callBack = callBack;
    mpeg2Ts->token = token;
}

void
mpeg2TsSetFlush(Mpeg2Ts mpeg2Ts, Boolean doFlush)
{
    mpeg2Ts->doFlush = doFlush;
}

void
mpeg2TsFree(Mpeg2Ts mpeg2Ts)
{
    mpeg2TsRecover(mpeg2Ts);
    contextFree(mpeg2Ts->context);
    free(mpeg2Ts);
}

/* ARGSUSED */
static size_t
mpeg2TsMaxChunk(void *instp)
{
    return MPEG2_TS_MAX_CHUNK;
}

/* ARGSUSED */
static RetCode
mpeg2TsRecognize(void *instp, Pipe pipe)
{
    u8                 *p;

    p = pipePeek(pipe, MPEG2_TS_PKT_SIZE + 1);
    return (p[0] == MPEG2_TS_SYNC_BYTE
	    && p[MPEG2_TS_PKT_SIZE] == MPEG2_TS_SYNC_BYTE)
      ? RETCODE_SUCCESS : RETCODE_FAILED;
}

/* ARGSUSED */
static RetCode
mpeg2TsParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Ts             mpeg2Ts = (Mpeg2Ts) instp;
    Mpeg2TsInfo         info;
    Mpeg2TsInfo        *infop = &info;
    RetCode             retCode;
    unsigned		lastPid = MPEG2_TS_NULL_PID;

    mpeg2Ts->isDirty = TRUE;
    MMP_CONTEXT_ADDPIPE(mpeg2Ts->cop, MPEG2_TS_PAT_PID,
			MMP_PARSER_NEWPIPE(mpeg2Ts->psip, mpeg2Ts->cop));

    MMP_CONTEXT_ADDPIPE(mpeg2Ts->cop, MPEG2_TS_CAT_PID,
			MMP_PARSER_NEWPIPE(mpeg2Ts->psip, mpeg2Ts->cop));

    while (pipeIsAvail(pipe, 1)) {
	PipePosition        tpEnd;
	unsigned            pid;
	Pipe                pidPipe;

	(void) memset(infop, 0, sizeof(*infop));

	infop->position = pipePosition(pipe);
	tpEnd = infop->position + MPEG2_TS_PKT_SIZE;
	
	/* FIXME: added for CA injection by Yongfang Liang */
	if (mpeg2Ts->callBack != NULL)
	{
        /* a fake return id is constructed */
        #define MPEG2_TS_GATHER_TS 7
	    (*mpeg2Ts->callBack)((void *)mpeg2Ts, (Mpeg2TsInfo *)pipe, 
	                          RETCODE_CONS(retCodeId, MPEG2_TS_GATHER_TS));
        #undef MPEG2_TS_GATHER_TS
	}
	/* ---> */

	infop->transportPacket = *PIPE_GET(pipe, Mpeg2TsTransportPacket);
	if (infop->transportPacket.syncByte != MPEG2_TS_SYNC_BYTE) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_SYNC);
	    goto error;
	}
	pid = MPEG2_TS_TRANSPORT_PACKET_PID(&infop->transportPacket);
	if (pid == MPEG2_TS_NULL_PID) {
	    infop->payloadLen = tpEnd - pipePosition(pipe);
	    goto skip;
	}
	if (infop->transportPacket.transportErrorIndicator) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_TEI);
	    goto error;
	}
	switch (infop->transportPacket.adaptationFieldControl) {
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_RESERVED:
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_AFC);
	    goto error;
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY:
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD:
	    infop->adaptationFieldLength = *PIPE_PEEK(pipe, u8);
	    if (infop->adaptationFieldLength == 0) {
		PIPE_SKIP(pipe, u8);
	    } else if ((retCode = mpeg2TsAdaptationField(mpeg2Ts, pipe, infop))
		       != RETCODE_SUCCESS) {
		goto error;
	    }
	    break;
	}
	infop->payloadLen = tpEnd - pipePosition(pipe);
	if (infop->payloadLen < 0
		|| (infop->payloadLen > 0
		    && infop->transportPacket.adaptationFieldControl
		    == MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY)) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_LEN);
	    goto error;
	}
skip:
	if (pid != lastPid && pid != MPEG2_TS_NULL_PID) {
	    /*
	     * Switching pid. Force child pipe to parse accumulated 
	     * TS payloads because that might be a PSI table that
	     * will create a new pipe (the one for this new payload).
	     */
	    retCode = pipeSync(pipe);
	    if (retCode != RETCODE_SUCCESS) {
		goto done;
	    }
	    lastPid = pid;
	}
	if (mpeg2Ts->callBack != NULL) {
	    retCode = (*mpeg2Ts->callBack) (mpeg2Ts->token, infop,
					    RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	if (pid == MPEG2_TS_NULL_PID) {
	    infop->transportPacket.adaptationFieldControl
		= MPEG2_TS_ADAPTATION_FIELD_CONTROL_PAYLOAD_ONLY;
	}
	switch (infop->transportPacket.adaptationFieldControl) {
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD:
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_PAYLOAD_ONLY:
	    pidPipe = MMP_CONTEXT_PIDTOPIPE(mpeg2Ts->cop, pid);
	    if (pidPipe == NULL) {
		pidPipe = PIPE_NULL;
	    }
	    retCode = pipeTransfer(pidPipe, pipe, infop->payloadLen,
			  Boolean(infop->transportPacket.payloadUnitStartIndicator),
			  mpeg2Ts->doFlush);
	    if (retCode != RETCODE_SUCCESS) {
		goto done;
	    }
	    break;
	}
    }
    retCode = RETCODE_SUCCESS;
    goto done;
error:
    if (mpeg2Ts->callBack != NULL) {
	(void) (*mpeg2Ts->callBack) (mpeg2Ts->token, infop, retCode);
    }
done:
    /*
     * PAT and CAT pipes (along with any other created pipes)
     * are deleted when mpeg2TsRecover is called from pipeThreadWrapper.
     */
    return retCode;
}

static void
mpeg2TsRecover(void *instp)
{
    Mpeg2Ts             mpeg2Ts = (Mpeg2Ts) instp;

    if (! mpeg2Ts->isDirty) {
	return;
    }
    mpeg2Ts->isDirty = FALSE;
    MMP_CONTEXT_DELETEALL(mpeg2Ts->cop);
    MMP_PARSER_RECOVER(mpeg2Ts->psip);
}

static RetCode
mpeg2TsAdaptationField(Mpeg2Ts mpeg2Ts, Pipe pipe, Mpeg2TsInfo *infop)
{
    PipePosition        afStart = pipePosition(pipe);
    PipePosition        afEnd;
    PipePosition        stuffLen;

    infop->adaptationField = *PIPE_GET(pipe, Mpeg2TsAdaptationField);
    /*
     * add 1 to afStart because length is counted after adaptationFieldLength
     * byte
     */
    afEnd = afStart + 1 + infop->adaptationField.adaptationFieldLength;
    if (infop->adaptationField.pcrFlag) {
	infop->pcr = *PIPE_GET(pipe, Mpeg2TsProgramClockReference);
    }
    if (infop->adaptationField.opcrFlag) {
	infop->opcr = *PIPE_GET(pipe, Mpeg2TsProgramClockReference);
    }
    if (infop->adaptationField.splicingPointFlag) {
	infop->spliceCountdown = *PIPE_GET(pipe, u8);
    }
    if (infop->adaptationField.transportPrivateDataFlag) {
	unsigned            dLen = *PIPE_GET(pipe, u8);

	infop->transportPrivateDataLength = dLen;
	(void) memcpy(infop->privateDataBytes, pipeGet(pipe, dLen), dLen);
    }
    if (infop->adaptationField.adaptationFieldExtensionFlag) {
	RetCode             retCode;

	infop->adaptationFieldExtensionLength = *PIPE_PEEK(pipe, u8);
	if (infop->adaptationFieldExtensionLength == 0) {
	    PIPE_SKIP(pipe, u8);
	} else if ((retCode = mpeg2TsAdaptationFieldExtension(mpeg2Ts, pipe,
					       infop)) != RETCODE_SUCCESS) {
	    return retCode;
	}
    }
    stuffLen = afEnd - pipePosition(pipe);
    if (stuffLen < 0) {
	return RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_AFL);
    }
    return pipeSkipStuffBytes(pipe, stuffLen, MPEG2_TS_STUFF_BYTE);
}

/* ARGSUSED */
static RetCode
mpeg2TsAdaptationFieldExtension(Mpeg2Ts mpeg2Ts, Pipe pipe, Mpeg2TsInfo *infop)
{
    PipePosition        afeStart = pipePosition(pipe);
    PipePosition        afeEnd;
    PipePosition        rsvdLen;

    infop->adaptationFieldExtension = *PIPE_GET(pipe,
					   Mpeg2TsAdaptationFieldExtension);
    /*
     * add 1 to afeStart because length is counted after
     * adaptationFieldExtensionLength byte
     */
    afeEnd = afeStart + 1
      + infop->adaptationFieldExtension.adaptationFieldExtensionLength;
    if (infop->adaptationFieldExtension.ltwFlag) {
	infop->ltw = *PIPE_GET(pipe, Mpeg2TsLtw);
    }
    if (infop->adaptationFieldExtension.piecewiseRateFlag) {
	infop->piecewiseRate = *PIPE_GET(pipe, Mpeg2TsPiecewiseRate);
    }
    if (infop->adaptationFieldExtension.seamlessSpliceFlag) {
	infop->seamlessSplice = *PIPE_GET(pipe, Mpeg2TsSeamlessSplice);
    }
    rsvdLen = afeEnd - pipePosition(pipe);
    if (rsvdLen < 0) {
	return RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_AFEL);
    }
    return pipeSkipStuffBytes(pipe, rsvdLen, MPEG2_TS_STUFF_BYTE);
}
