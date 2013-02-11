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
 * $(@)Mpeg2Pmt.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:38 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG2 program map table parser
 */

#pragma ident "@(#)Mpeg2Pmt.c 1.1	96/09/18 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/HashTable.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Pmt.h"

#define	MPEG2_PMT_MAX_CHUNK		256
#define	MPEG2_PMT_SIZE			10
#define	MPEG2_PMT_FACTOR		0.9

struct _Mpeg2Pmt {
    MmpParserObject    *streamTypeToPop[MPEG2_PMT_STREAM_TYPE_MAX + 1];
    Mpeg2PmtCallBack    callBack;
    void               *token;
    Boolean             isStrict:1;
    Boolean             shouldPmtStop:1;
    HashTable           pmtTable[2];
    MmpParserObject     po;
    Boolean		isDirty;
};

static size_t       mpeg2PmtMaxChunk(void *instp);

static RetCode
mpeg2PmtParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg2PmtRecover(void *instp);

static RetCode
mpeg2PmtActivate(Mpeg2Pmt mpeg2Pmt, MmpContextObject *cop,
		 Mpeg2PmtTsProgramMapSection * pmsp);

static void
mpeg2PmtDeactivate(Mpeg2Pmt mpeg2Pmt, unsigned pn,
		   Mpeg2PmtCurrentNextIndicator cni);

static RetCode      mpeg2PmtGetDesc(Pipe pipe, Mpeg2PmtDescriptor ***descppp);

static RetCode
mpeg2PmtGetStreamDesc(Pipe pipe,
		      Mpeg2PmtStream ***streamppp);

static void         mpeg2PmtDescListFree(Mpeg2PmtDescriptor *descp);
static void         mpeg2PmtStreamListFree(Mpeg2PmtStream *streamp);

static RetCodeTable mpeg2PmtErrorTable[] = {
    {MPEG2_PMT_ERROR_SSI, NULL, "section syntax indicator not set"},
    {MPEG2_PMT_ERROR_SN, NULL, "(last) section number != 0"},
    {MPEG2_PMT_ERROR_PIL_LEN, NULL, "program info length error"},
    {MPEG2_PMT_ERROR_LEN, NULL, "pmt length error"},
    {MPEG2_PMT_ERROR_CRC, NULL, "crc error"},
    {MPEG2_PMT_ERROR_STOP, NULL, "stop on pmt found"},
    {MPEG2_PMT_ERROR_PID_USE, NULL, "pid use conflict"},
    {MPEG2_PMT_ERROR_EOF, NULL, "unexpected eof"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Pmt
mpeg2PmtNew(void)
{
    Mpeg2Pmt            mpeg2Pmt;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2PMT_CLASSNAME,
					     mpeg2PmtErrorTable);
    }
    mpeg2Pmt = NEW_ZEROED(struct _Mpeg2Pmt, 1);

    mpeg2Pmt->pmtTable[0] = hashTableIntNewWithSizeAndFactor(MPEG2_PMT_SIZE,
							  MPEG2_PMT_FACTOR);
    mpeg2Pmt->pmtTable[1] = hashTableIntNewWithSizeAndFactor(MPEG2_PMT_SIZE,
							  MPEG2_PMT_FACTOR);

    mpeg2Pmt->po.maxChunk = mpeg2PmtMaxChunk;
    mpeg2Pmt->po.recognize = NULL;
    mpeg2Pmt->po.parse = mpeg2PmtParse;
    mpeg2Pmt->po.recover = mpeg2PmtRecover;
    mpeg2Pmt->po.instp = mpeg2Pmt;

    return mpeg2Pmt;
}

void
mpeg2PmtSetPesParsers(Mpeg2Pmt mpeg2Pmt,
	    MmpParserObject *streamTypeToPop[MPEG2_PMT_STREAM_TYPE_MAX + 1])
{
    mpeg2PmtRecover(mpeg2Pmt);
    (void) memcpy(mpeg2Pmt->streamTypeToPop, streamTypeToPop,
	          sizeof(mpeg2Pmt->streamTypeToPop));
}

MmpParserObject    *
mpeg2PmtParserObject(Mpeg2Pmt mpeg2Pmt)
{
    return &mpeg2Pmt->po;
}

void
mpeg2PmtSetCallBack(Mpeg2Pmt mpeg2Pmt, Mpeg2PmtCallBack callBack, void *token)
{
    mpeg2Pmt->callBack = callBack;
    mpeg2Pmt->token = token;
}

void
mpeg2PmtSetStrict(Mpeg2Pmt mpeg2Pmt, Boolean isStrict)
{
    mpeg2Pmt->isStrict = isStrict;
}

Boolean
mpeg2PmtGetStrict(Mpeg2Pmt mpeg2Pmt)
{
    return mpeg2Pmt->isStrict;
}

void
mpeg2PmtSetPmtStop(Mpeg2Pmt mpeg2Pmt, Boolean shouldPmtStop)
{
    mpeg2Pmt->shouldPmtStop = shouldPmtStop;
}

Boolean
mpeg2PmtGetPmtStop(Mpeg2Pmt mpeg2Pmt)
{
    return mpeg2Pmt->shouldPmtStop;
}

Mpeg2PmtStatus
mpeg2PmtStatus(Mpeg2Pmt mpeg2Pmt)
{
    Mpeg2PmtStatus      pmtStatus;

    pmtStatus.pmtTable[0] = mpeg2Pmt->pmtTable[0];
    pmtStatus.pmtTable[1] = mpeg2Pmt->pmtTable[1];
    return pmtStatus;
}

void
mpeg2PmtFree(Mpeg2Pmt mpeg2Pmt)
{
    int                 cni;

    mpeg2PmtRecover(mpeg2Pmt);
    for (cni = 0; cni < NELEM(mpeg2Pmt->pmtTable); cni++) {
	ASSERT(hashTableUsed(mpeg2Pmt->pmtTable[cni]) == 0);
	hashTableFree(mpeg2Pmt->pmtTable[cni]);
	mpeg2Pmt->pmtTable[cni] = NULL;
    }
    free(mpeg2Pmt);
}

/* ARGSUSED */
static size_t
mpeg2PmtMaxChunk(void *instp)
{
    return MPEG2_PMT_MAX_CHUNK;
}

static RetCode
mpeg2PmtParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Pmt            mpeg2Pmt = (Mpeg2Pmt) instp;
    Mpeg2PmtInfo        info;
    Mpeg2PmtInfo       *infop = &info;
    Mpeg2PmtTsProgramMapSection *pmsp = &infop->tsProgramMapSection;
    PipePosition        pmtEnd;
    RetCode             retCode;
    Mpeg2PmtDescriptor **descListEndpp;
    Mpeg2PmtStream    **streamListEndpp;
    unsigned            pil;
    PipePosition        descEndPos;

    mpeg2Pmt->isDirty = TRUE;
    (void) memset(infop, 0, sizeof(*infop));

    infop->position = pipePosition(pipe);
    pmtEnd = pipeRelativePosition(pipe);
    pmsp->psiHeader = *PIPE_GET(pipe, Mpeg2PsiHeader);
    if (!pmsp->psiHeader.sectionSyntaxIndicator) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_SSI);
	goto error;
    }
    pmsp->psiExtension = *PIPE_GET(pipe, Mpeg2PsiExtension);

    if (pmsp->psiExtension.sectionNumber != 0
	    || pmsp->psiExtension.lastSectionNumber != 0) {
	if (mpeg2Pmt->isStrict) {
	    return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_SN);
	}
    }
    pmtEnd += MPEG2_PSI_HEADER_SECTION_LENGTH(&pmsp->psiHeader)
      + sizeof(Mpeg2PsiHeader);

    /*
     * NOTE: pmt has only a single section. Because of this, we don't mess
     * with next->current "promotion" (although we do maintain both current
     * and next tables).
     */
    pmsp->pmtHeader = *PIPE_GET(pipe, Mpeg2PmtHeader);
    pmsp->descList = NULL;
    pmsp->streamList = NULL;
    pil = MPEG2_PMT_HEADER_PROGRAM_INFO_LENGTH(&pmsp->pmtHeader);
    descEndPos = pipeRelativePosition(pipe) + pil;
    if (descEndPos > pmtEnd) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_PIL_LEN);
	goto error;
    }
    /*
     * Becareful to avoid pipe detected unexpected eof's, that would keep us
     * from freeing the streams and descriptors allocated here.
     */
    descListEndpp = &pmsp->descList;
    while (pipeRelativePosition(pipe) < descEndPos) {
	if ((retCode = mpeg2PmtGetDesc(pipe, &descListEndpp))
		!= RETCODE_SUCCESS) {
	    goto error;
	}
    }
    if (pipeRelativePosition(pipe) != descEndPos) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_PIL_LEN);
	goto error;
    }
    streamListEndpp = &pmsp->streamList;
    while (pipeRelativePosition(pipe) + 4 < pmtEnd) {
	if ((retCode = mpeg2PmtGetStreamDesc(pipe, &streamListEndpp))
		!= RETCODE_SUCCESS) {
	    goto error;
	}
    }
    if (pmtEnd - pipeRelativePosition(pipe) != 4) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_LEN);
	goto error;
    }
    if (!PIPE_IS_AVAIL(pipe, u32)) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_EOF);
	goto error;
    }
    /*
     * FIXME: Do this the normal way with 4 bytes?
     */
    pmsp->crc = pipeGetBits(pipe, 32);
    if (pipeCrcGet(pipe) != 0) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_CRC);
	goto error;
    }
    if (mpeg2Pmt->callBack != NULL) {
	retCode = (*mpeg2Pmt->callBack) (mpeg2Pmt->token, infop,
					 RETCODE_SUCCESS);
	if (retCode != RETCODE_SUCCESS) {
	    return retCode;
	}
    }
    return mpeg2PmtActivate(mpeg2Pmt, cop, pmsp);

error:
    if (mpeg2Pmt->callBack != NULL) {
	(void) (*mpeg2Pmt->callBack) (mpeg2Pmt->token, infop, retCode);
    }
    mpeg2PmtDescListFree(pmsp->descList);
    mpeg2PmtStreamListFree(pmsp->streamList);
    return retCode;
}

static void
mpeg2PmtRecover(void *instp)
{
    Mpeg2Pmt            mpeg2Pmt = (Mpeg2Pmt) instp;
    int                 cni;
    int                 pmtCount;
    HashItem           *pmtItems;
    int                 i;

    if (! mpeg2Pmt->isDirty) {
	return;
    }
    mpeg2Pmt->isDirty = FALSE;
    for (cni = 0; cni < NELEM(mpeg2Pmt->pmtTable); cni++) {
	pmtCount = hashTableUsed(mpeg2Pmt->pmtTable[cni]);
	pmtItems = hashTableDump(mpeg2Pmt->pmtTable[cni]);
	for (i = 0; i < pmtCount; i++) {
	    mpeg2PmtDeactivate(mpeg2Pmt, (int) pmtItems[i].key, (Mpeg2PmtCurrentNextIndicator)cni);
	}
	free(pmtItems);
    }
}

static RetCode
mpeg2PmtActivate(Mpeg2Pmt mpeg2Pmt, MmpContextObject *cop,
		 Mpeg2PmtTsProgramMapSection * pmsp)
{
    Mpeg2PmtStream     *streamp;
    Mpeg2PmtTsProgramMapSection *curPmsp;
    Mpeg2PmtCurrentNextIndicator cni;
    unsigned            pn;

    cni = Mpeg2PmtCurrentNextIndicator(pmsp->psiExtension.currentNextIndicator);
    pn = MPEG2_PMT_PROGRAM_NUMBER(&pmsp->psiExtension);

    curPmsp = (Mpeg2PmtTsProgramMapSection*)hashTableGet(mpeg2Pmt->pmtTable[cni], pn);

    if (curPmsp == NULL
	    || curPmsp->psiExtension.versionNumber
	    != pmsp->psiExtension.versionNumber) {
	mpeg2PmtDeactivate(mpeg2Pmt, pn, cni);
	(void) hashTablePut(mpeg2Pmt->pmtTable[cni], pn,
			DUP(Mpeg2PmtTsProgramMapSection, pmsp));
	if (mpeg2Pmt->shouldPmtStop) {
	    return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_STOP);
	}
	for (streamp = pmsp->streamList;
		streamp != NULL; streamp = streamp->next) {
	    unsigned            streamType = streamp->streamHeader.streamType;
	    unsigned            ePid = MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(
						    &streamp->streamHeader);
	    Pipe                pidPipe;

	    if (MMP_CONTEXT_PIDTOPIPE(cop, ePid) == NULL) {
		MmpParserObject    *pop = mpeg2Pmt->streamTypeToPop[streamType];
		RetCode             retCode;

		streamp->cop = cop;
		pidPipe = MMP_PARSER_NEWPIPE(pop, cop);
		retCode = MMP_CONTEXT_ADDPIPE(cop, ePid, pidPipe);
		ABORT_IF_FALSE(retCode == RETCODE_SUCCESS);
	    } else if (streamp->cop != cop) {
		return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_PID_USE);
	    }
	}
    } else {
	mpeg2PmtDescListFree(pmsp->descList);
	mpeg2PmtStreamListFree(pmsp->streamList);
    }
    return RETCODE_SUCCESS;
}

static void
mpeg2PmtDeactivate(Mpeg2Pmt mpeg2Pmt, unsigned pn,
		   Mpeg2PmtCurrentNextIndicator cni)
{
    Mpeg2PmtTsProgramMapSection *pmsp;
    Mpeg2PmtStream     *streamp;

    pmsp = (Mpeg2PmtTsProgramMapSection*) hashTableRemove(mpeg2Pmt->pmtTable[cni], pn);

    if (pmsp != NULL) {
	for (streamp = pmsp->streamList;
		streamp != NULL; streamp = streamp->next) {
	    unsigned            ePid = MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(
						    &streamp->streamHeader);

	    (void) MMP_CONTEXT_DELETEPIPE(streamp->cop, ePid);
	}
	mpeg2PmtDescListFree(pmsp->descList);
	mpeg2PmtStreamListFree(pmsp->streamList);
	free(pmsp);
    }
}

static RetCode
mpeg2PmtGetDesc(Pipe pipe, Mpeg2PmtDescriptor ***descppp)
{
    Mpeg2PmtDescriptor *newDescp;

    if (!pipeIsAvail(pipe, 2)) {
	return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_EOF);
    }
    newDescp = NEW_ZEROED(Mpeg2PmtDescriptor, 1);
    newDescp->descriptor.descriptorTag = *PIPE_GET(pipe, u8);
    newDescp->descriptor.descriptorLen = *PIPE_GET(pipe, u8);
    if (!pipeIsAvail(pipe, newDescp->descriptor.descriptorLen)) {
	free(newDescp);
	return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_EOF);
    }
    (void) memcpy(newDescp->descriptor.un.descriptor,
		  pipeGet(pipe, newDescp->descriptor.descriptorLen),
		  newDescp->descriptor.descriptorLen);

    **descppp = newDescp;
    *descppp = &newDescp->next;
    return RETCODE_SUCCESS;
}

static RetCode
mpeg2PmtGetStreamDesc(Pipe pipe, Mpeg2PmtStream ***streamppp)
{
    Mpeg2PmtStream     *newStrp;
    Mpeg2PmtDescriptor **descListEndpp;
    PipePosition        descEndPos;
    RetCode             retCode = RETCODE_SUCCESS;

    if (!PIPE_IS_AVAIL(pipe, Mpeg2PmtStreamHeader)) {
	return RETCODE_CONS(retCodeId, MPEG2_PMT_ERROR_EOF);
    }
    newStrp = NEW_ZEROED(Mpeg2PmtStream, 1);
    newStrp->streamHeader = *PIPE_GET(pipe, Mpeg2PmtStreamHeader);
    descEndPos = pipeRelativePosition(pipe)
      + MPEG2_PMT_STREAM_HEADER_ES_INFO_LENGTH(&newStrp->streamHeader);
    descListEndpp = &newStrp->descList;
    while (pipeRelativePosition(pipe) < descEndPos) {
	if ((retCode = mpeg2PmtGetDesc(pipe, &descListEndpp))
		!= RETCODE_SUCCESS) {
	    break;
	}
    }
    **streamppp = newStrp;
    *streamppp = &newStrp->next;
    return retCode;
}

static void
mpeg2PmtDescListFree(Mpeg2PmtDescriptor *descp)
{
    Mpeg2PmtDescriptor *nextDescp;

    for (; descp != NULL; descp = nextDescp) {
	nextDescp = descp->next;
	free(descp);
    }
}

static void
mpeg2PmtStreamListFree(Mpeg2PmtStream *streamp)
{
    Mpeg2PmtStream     *nextStreamp;

    for (; streamp != NULL; streamp = nextStreamp) {
	nextStreamp = streamp->next;
	mpeg2PmtDescListFree(streamp->descList);
	free(streamp);
    }
}
