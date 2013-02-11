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
 * $(@)Mpeg2Pes.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG2 PES parser
 */

#pragma ident "@(#)Mpeg2Pes.c 1.1	96/09/18 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Context.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Pes.h"

#define	MPEG2_PES_MAX_CHUNK	(256)	   /* FIXME: Put real value here */

struct _Mpeg2Pes {
    MmpParserObject    *streamIdToPop[MPEG2_PES_STREAM_ID_MAX + 1];
    Mpeg2PesCallBack    callBack;
    void               *token;
    MmpParserObject     po;
    Context		context;
    MmpContextObject   *cop;
    Boolean		isDirty;
    Boolean		doFlush;
};

typedef struct Mpeg2PesPacketAndFlag {
    Mpeg2PesPacket      pesPacket;
    BIT2(b10:2,
         ignored:6);
} Mpeg2PesPacketAndFlag;

static size_t       mpeg2PesMaxChunk(void *instp);
static RetCode      mpeg2PesRecognize(void *instp, Pipe pipe);

static RetCode
mpeg2PesParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg2PesRecover(void *instp);

static RetCodeTable mpeg2PesErrorTable[] = {
    {MPEG2_PES_ERROR_SYNTAX, NULL, "packet syntax error"},
    {MPEG2_PES_ERROR_PTS, NULL, "pts flag error"},
    {MPEG2_PES_ERROR_DTS, NULL, "dts flag error"},
    {MPEG2_PES_ERROR_HDR_LEN, NULL, "header length error"},
    {MPEG2_PES_ERROR_LEN, NULL, "packet length error"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Pes
mpeg2PesNew(void)
{
    Mpeg2Pes            mpeg2Pes;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2PES_CLASSNAME,
					     mpeg2PesErrorTable);
    }
    mpeg2Pes = NEW_ZEROED(struct _Mpeg2Pes, 1);

    mpeg2Pes->po.maxChunk = mpeg2PesMaxChunk;
    mpeg2Pes->po.recognize = mpeg2PesRecognize;
    mpeg2Pes->po.parse = mpeg2PesParse;
    mpeg2Pes->po.recover = mpeg2PesRecover;
    mpeg2Pes->po.instp = mpeg2Pes;

    mpeg2Pes->context = contextNew();
    mpeg2Pes->cop = contextMmpContextObject(mpeg2Pes->context);
    return mpeg2Pes;
}

void
mpeg2PesSetEsParsers(Mpeg2Pes mpeg2Pes,
		MmpParserObject *streamIdToPop[MPEG2_PES_STREAM_ID_MAX + 1])
{
    mpeg2PesRecover(mpeg2Pes);
    (void) memcpy(mpeg2Pes->streamIdToPop, streamIdToPop,
	sizeof(mpeg2Pes->streamIdToPop));
}

MmpParserObject    *
mpeg2PesParserObject(Mpeg2Pes mpeg2Pes)
{
    return &mpeg2Pes->po;
}

void
mpeg2PesSetFlush(Mpeg2Pes mpeg2Pes, Boolean doFlush)
{
    mpeg2Pes->doFlush = doFlush;
}

void
mpeg2PesSetCallBack(Mpeg2Pes mpeg2Pes, Mpeg2PesCallBack callBack, void *token)
{
    mpeg2Pes->callBack = callBack;
    mpeg2Pes->token = token;
}

void
mpeg2PesFree(Mpeg2Pes mpeg2Pes)
{
    mpeg2PesRecover(mpeg2Pes);
    contextFree(mpeg2Pes->context);
    free(mpeg2Pes);
}

/* ARGSUSED */
static size_t
mpeg2PesMaxChunk(void *instp)
{
    return MPEG2_PES_MAX_CHUNK;
}

/* ARGSUSED */
static RetCode
mpeg2PesRecognize(void *instp, Pipe pipe)
{
    Mpeg2PesPacketAndFlag *pfp = PIPE_PEEK(pipe, Mpeg2PesPacketAndFlag);
    RetCode             retCode = RETCODE_FAILED;

    if (IS_MPEG2_PES_PACKET_START_CODE_PREFIX(&pfp->pesPacket)
	    && MPEG2_PES_STREAM_ID_MIN <= pfp->pesPacket.streamId
	    && pfp->pesPacket.streamId >= MPEG2_PES_STREAM_ID_MAX) {
	switch (pfp->pesPacket.streamId) {
	case MPEG2_PES_STREAM_ID_PROGRAM_STREAM_MAP:
	case MPEG2_PES_STREAM_ID_PADDING_STREAM:
	case MPEG2_PES_STREAM_ID_PRIVATE_STREAM_2:
	case MPEG2_PES_STREAM_ID_ECM_STREAM:
	case MPEG2_PES_STREAM_ID_EMM_STREAM:
	case MPEG2_PES_STREAM_ID_PROGRAM_STREAM_DIRECTORY:
	    retCode = RETCODE_SUCCESS;
	    break;
	default:
	    if (pfp->b10 == 2) {
		retCode = RETCODE_SUCCESS;
	    }
	    break;
	}
    }
    return retCode;
}

static RetCode
mpeg2PesParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Pes            mpeg2Pes = (Mpeg2Pes) instp;
    Mpeg2PesInfo        info;
    Mpeg2PesInfo       *infop = &info;
    RetCode             retCode = RETCODE_SUCCESS;
    Mpeg2PesPacket     *ppp;
    unsigned	        pid;

    mpeg2Pes->isDirty = TRUE;

    pid = MMP_CONTEXT_PIPETOPID(cop, pipe);

    /*
     * If this is a transport stream, pmt that caused initiation of this
     * pes parser may occur anywhere relative to the start of the pes
     * packet -- so wait until we're at a mark which indicates the
     * start of a PES packet.
     */
    if (pid != MMP_PID_NULL) {
	pipeFindMark(pipe);
    }

    while ((ppp = PIPE_PEEK(pipe, Mpeg2PesPacket), TRUE)
	    && IS_MPEG2_PES_PACKET_START_CODE_PREFIX(ppp)
	    && MPEG2_PES_STREAM_ID_MIN <= ppp->streamId
	    && ppp->streamId <= MPEG2_PES_STREAM_ID_MAX) {
	PipePosition        packetEnd;
	unsigned            streamId;
	PipePosition        nStuffBytes;
	PipePosition        headerEnd;
	int                 payloadLen;
	unsigned            packetLength;
	Pipe                pidPipe;
        unsigned	    pipeId;
	Boolean		    isMarked;

	(void) memset(infop, 0, sizeof(*infop));

	isMarked = pipeIsAtMark(pipe);
	infop->position = pipePosition(pipe);
	packetEnd = pipeRelativePosition(pipe);
	infop->packet = *PIPE_GET(pipe, Mpeg2PesPacket);

	packetLength = MPEG2_PES_PACKET_LENGTH(&infop->packet);
	packetEnd = packetLength == 0 ? 0 : packetEnd + packetLength + 6;
	streamId = infop->packet.streamId;
	switch (streamId) {
	default:
	    infop->packetFlags = *PIPE_GET(pipe, Mpeg2PesPacketFlags);
	    if (infop->packetFlags.b10 != 0x2) {
		retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_SYNTAX);
		goto error;
	    }
	    headerEnd = pipeRelativePosition(pipe)
	      + infop->packetFlags.pesHeaderDataLength;
	    switch (infop->packetFlags.ptsDtsFlags) {
	    case 0x0:
		break;
	    case 0x2:
		infop->pts = *PIPE_GET(pipe, Mpeg2PesTs);
		if (infop->pts.tsType != 0x2) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_PTS);
		    goto error;
		}
		break;
	    case 0x3:
		infop->pts = *PIPE_GET(pipe, Mpeg2PesTs);
		if (infop->pts.tsType != 0x3) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_PTS);
		    goto error;
		}
		infop->dts = *PIPE_GET(pipe, Mpeg2PesTs);
		if (infop->dts.tsType != 0x1) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_DTS);
		    goto error;
		}
		break;
	    default:
		return RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_SYNTAX);
	    }
	    if (infop->packetFlags.escrFlag) {
		infop->escr = *PIPE_GET(pipe, Mpeg2PesEscr);
	    }
	    if (infop->packetFlags.esRateFlag) {
		infop->esRate = *PIPE_GET(pipe, Mpeg2PesEsRate);
	    }
	    if (infop->packetFlags.dsmTrickModeFlag) {
		infop->dsmTrickMode = *PIPE_GET(pipe, Mpeg2PesDsmTrickMode);
	    }
	    if (infop->packetFlags.additionalCopyInfoFlag) {
		infop->additionalCopyInfo = *PIPE_GET(pipe,
						Mpeg2PesAdditionalCopyInfo);
	    }
	    if (infop->packetFlags.pesCrcFlag) {
		infop->pesCrc = *PIPE_GET(pipe, Mpeg2PesPesCrc);
	    }
	    if (infop->packetFlags.pesExtensionFlag) {
		infop->pesExtension = *PIPE_GET(pipe, Mpeg2PesPesExtension);
		if (infop->pesExtension.pesPrivateDataFlag) {
		    infop->pesPrivateData = *PIPE_GET(pipe,
						      Mpeg2PesPrivateData);
		}
		if (infop->pesExtension.packHeaderFieldFlag) {
		    infop->packHeaderField = *PIPE_GET(pipe,
						   Mpeg2PesPackHeaderField);
		}
		if (infop->pesExtension.programPacketSequenceCounterFlag) {
		    infop->programPacketSequenceCounter = *PIPE_GET(pipe,
				      Mpeg2PesProgramPacketSequenceCounter);
		}
		if (infop->pesExtension.pStdBufferFlag) {
		    infop->pStdBuffer = *PIPE_GET(pipe, Mpeg2PesPStdBuffer);
		}
		if (infop->pesExtension.pesExtensionFlag2) {
		    unsigned            fieldLen;

		    infop->pesExtension2 = *PIPE_GET(pipe, Mpeg2PesExtension2);
		    fieldLen = infop->pesExtension2.pesExtensionFieldLength;
		    (void) memcpy(infop->pesExtension2Data,
				  pipeGet(pipe, fieldLen), fieldLen);
		}
	    }
	    nStuffBytes = headerEnd - pipeRelativePosition(pipe);
	    if (nStuffBytes < 0) {
		retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_HDR_LEN);
		goto error;
	    } else if (nStuffBytes > 0) {
	        retCode = pipeSkipStuffBytes(pipe, nStuffBytes,
					 MPEG2_PES_PACKET_STUFF_BYTE);
	        if (retCode != RETCODE_SUCCESS) {
		    goto error;
	        }
	    }
	    /* FALLTHRU */

	case MPEG2_PES_STREAM_ID_PROGRAM_STREAM_MAP:
	case MPEG2_PES_STREAM_ID_PRIVATE_STREAM_2:
	case MPEG2_PES_STREAM_ID_ECM_STREAM:
	case MPEG2_PES_STREAM_ID_EMM_STREAM:
	case MPEG2_PES_STREAM_ID_PROGRAM_STREAM_DIRECTORY:
	    if (packetLength != 0) {
		payloadLen = packetEnd - pipeRelativePosition(pipe);
		if (payloadLen < 0) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_LEN);
		    goto error;
		}
	    }
	    if (mpeg2Pes->callBack != NULL) {
		retCode = (*mpeg2Pes->callBack) (mpeg2Pes->token, infop,
						 RETCODE_SUCCESS);
	    }
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	    /*
	     * If incoming pipe has a pid, then this is a PES within
	     * a transport stream, streamId's may be duplicated between
	     * various programs.  Use pid rather than streamId to name
	     * ES pipe in this context.
	     */
            pipeId = (pid != MMP_PID_NULL) ? pid : streamId;
	    if ((pidPipe = MMP_CONTEXT_PIDTOPIPE(mpeg2Pes->cop, pipeId))
		== NULL) {
		MmpParserObject    *pop = mpeg2Pes->streamIdToPop[streamId];

		pidPipe = MMP_PARSER_NEWPIPE(pop, mpeg2Pes->cop);
		/*
		 * This can't fail...  we just checked that pipePipe == NULL
		 */
		retCode = MMP_CONTEXT_ADDPIPE(mpeg2Pes->cop, pipeId, pidPipe);
		ASSERT(retCode == RETCODE_SUCCESS);
	    }
	    if (packetLength == 0) {
		if (pidPipe == PIPE_NULL) {
		    pipeFindMark(pipe);
		} else
		    while (pipeIsAvailUnmarked(pipe, 1)) {
			retCode = pipeTransfer(pidPipe, pipe,
					       pipeAvailUnmarked(pipe),
					       isMarked, FALSE);
			isMarked = FALSE;
			if (retCode != RETCODE_SUCCESS) {
			    return retCode;
			}
		    }
	    } else {
		retCode = pipeTransfer(pidPipe, pipe, payloadLen, isMarked,
				       FALSE);
		if (retCode != RETCODE_SUCCESS) {
		    return retCode;
		}
	    }
	    if (mpeg2Pes->doFlush
		    && (retCode = pipeSync(pipe)) != RETCODE_SUCCESS) {
		return retCode;
	    }
	    break;

	case MPEG2_PES_STREAM_ID_PADDING_STREAM:
	    if (packetLength != 0) {
		payloadLen = packetEnd - pipeRelativePosition(pipe);
		if (payloadLen < 0) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_LEN);
		    goto error;
		}
	    }
	    if (mpeg2Pes->callBack != NULL) {
		retCode = (*mpeg2Pes->callBack) (mpeg2Pes->token, infop,
						 RETCODE_SUCCESS);
	    }
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	    if (packetLength == 0) {
		while (pipeIsAvailUnmarked(pipe, 1)) {
		    size_t              unmarkedLen = pipeAvailUnmarked(pipe);

		    retCode = pipeSkipStuffBytes(pipe, unmarkedLen,
					     MPEG2_PES_PACKET_PADDING_BYTE);
		    if (retCode != RETCODE_SUCCESS) {
			return retCode;
		    }
		}
	    } else {
		retCode = pipeSkipStuffBytes(pipe, payloadLen,
					 MPEG2_PES_PACKET_PADDING_BYTE);
		if (retCode != RETCODE_SUCCESS) {
		    return retCode;
		}
	    }
	    break;
	}
    }
    /*
     * If pid is PID_NULL, then this is within a Program Stream, might
     * have just come up on a Pack header, so let Ps handle error.
     * If is isn't PID_NULL, then this is a transport stream and this
     * pipe should only be carrying PES packets -- give an error.
     */
    return pid == MMP_PID_NULL
	? RETCODE_SUCCESS : RETCODE_CONS(retCodeId, MPEG2_PES_ERROR_SYNTAX);

error:
    if (mpeg2Pes->callBack != NULL) {
	retCode = (*mpeg2Pes->callBack) (mpeg2Pes->token, infop, retCode);
    }
    return retCode;
}

static void
mpeg2PesRecover(void *instp)
{
    Mpeg2Pes            mpeg2Pes = (Mpeg2Pes) instp;
    unsigned            streamId;

    if (! mpeg2Pes->isDirty) {
	return;
    }
    mpeg2Pes->isDirty = FALSE;
    MMP_CONTEXT_DELETEALL(mpeg2Pes->cop);
    for (streamId = MPEG2_PES_STREAM_ID_MIN;
	    streamId <= MPEG2_PES_STREAM_ID_MAX; streamId++) {
	MMP_PARSER_RECOVER(mpeg2Pes->streamIdToPop[streamId]);
    }
}
