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
 * $(@)Mmp.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:38 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mmp.c
 * 
 * Mmp is the multi-media parser framework.
 */

#pragma ident "@(#)Mmp.c 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Pipe.h"

#include "mmp/Mmp.h"

#define	MMP_MAX_CHUNK		1

/***********************************************************************
 * OBJECT Mmp Instance Type
 ***********************************************************************/

struct _Mmp {
    MmpParserObject   **pops;
    Pipe                pipe;
    PipePosition        maxScanDistance;
    MmpCallBack         callBack;
    void               *callBackToken;
    PipePosition	putPosition;
};

/***********************************************************************
 * OBJECT Mmp Private Prototypes
 ***********************************************************************/

static size_t       mmpMaxChunk(Mmp mmp);
static RetCode      mmpRecognize(Mmp mmp, Pipe pipe, int *popxp);
static RetCode      mmpParse(void *instp, MmpContextObject *cop, Pipe pipe);
static void	    mmpCleanup(Mmp mmp);

/***********************************************************************
 * OBJECT Mmp Private Data
 ***********************************************************************/

static RetCodeTable mmpErrorTable[] = {
    {MMP_ERROR_UNREC_STREAM, NULL, "unrecognized stream"},
    {MMP_ERROR_NO_PARSER, NULL, "no parser"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

/***********************************************************************
 * OBJECT Mmp Class Interface
 ***********************************************************************/

Mmp
mmpNew(void)
{
    Mmp                 mmp = NEW_ZEROED(struct _Mmp, 1);

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MMP_CLASSNAME,
					     mmpErrorTable);
    }
    mmp->putPosition = 0LL;

    return mmp;
}

/***********************************************************************
 * OBJECT Mmp Instance Interface
 ***********************************************************************/

void
mmpSetParsers(Mmp mmp, MmpParserObject *pops[])
{
    int                 nParsers = 0;
    MmpParserObject   **popp;
    size_t		maxChunk;

    if (mmp->pops != NULL) {
	for (popp = mmp->pops; *popp != NULL; popp++) {
	    MMP_PARSER_RECOVER(*popp);
	}
	free(mmp->pops);
    }
    if (mmp->pipe != NULL) {
	(void) pipeFree(mmp->pipe);
	mmp->pipe = NULL;
    }

    for (popp = pops; *popp != NULL; popp++) {
	nParsers++;
    }
    mmp->pops = (MmpParserObject**)MEMDUP(pops, (nParsers + 1) * sizeof(MmpParserObject *));

    if (nParsers > 0) {
        maxChunk = mmpMaxChunk(mmp);
        mmp->pipe = pipeNew(maxChunk, (PipeParser) mmpParse,
		(PipeCleanup) mmpCleanup, mmp, NULL);
    }
}

void
mmpSetPosition(Mmp mmp, PipePosition putPosition)
{
    mmp->putPosition = putPosition;
}

void
mmpSetCallBack(Mmp mmp, MmpCallBack callBack, void *callBackToken)
{
    mmp->callBack = callBack;
    mmp->callBackToken = callBackToken;
}

void
mmpSetMaxScanDistance(Mmp mmp, PipePosition maxScanDistance)
{
    mmp->maxScanDistance = maxScanDistance;
}

RetCode
mmpPut(Mmp mmp, u8 *buf, size_t len)
{
    RetCode retCode;

    if (mmp->pipe == NULL) {
	return RetCode(RETCODE_CONS(retCodeId, MMP_ERROR_NO_PARSER));
    }
    retCode = pipePut(mmp->pipe, buf, len, FALSE, mmp->putPosition);
    mmp->putPosition += len;
    return retCode;
}

RetCode
mmpEof(Mmp mmp)
{
    if (mmp->pipe == NULL) {
	return RetCode(RETCODE_CONS(retCodeId, MMP_ERROR_NO_PARSER));
    }
    return pipeEof(mmp->pipe);
}

PipePosition
mmpParsedBytes(Mmp mmp)
{
    return mmp->pipe == NULL ? 0 : pipeRelativePosition(mmp->pipe);
}

void
mmpRecover(Mmp mmp)
{
    if (mmp->pipe != NULL) {
	mmpCleanup(mmp);
	pipeRecover(mmp->pipe);
    }
}

void
mmpFree(Mmp mmp)
{
    if (mmp->pipe != NULL) {
        MmpParserObject   **popp;

	for (popp = mmp->pops; *popp != NULL; popp++) {
	    MMP_PARSER_RECOVER(*popp);
	}
	(void) pipeFree(mmp->pipe);
    }
    free(mmp->pops);
    free(mmp);
}

/***********************************************************************
 * OBJECT Mmp Private Methods
 ***********************************************************************/

static size_t
mmpMaxChunk(Mmp mmp)
{
    MmpParserObject   **popp;
    size_t              maxChunk = MMP_MAX_CHUNK;

    for (popp = mmp->pops; *popp != NULL; popp++) {
	size_t              chunk = MMP_PARSER_MAXCHUNK(*popp);

	if (chunk > maxChunk) {
	    maxChunk = chunk;
	}
    }
    return maxChunk;
}

static RetCode
mmpRecognize(Mmp mmp, Pipe pipe, int *popxp)
{
    MmpParserObject   **popp;
    RetCode             retCode;

    for (popp = mmp->pops; *popp != NULL; popp++) {
	retCode = MMP_PARSER_RECOGNIZE(*popp, pipe);
	if (retCode == RETCODE_SUCCESS) {
	    goto done;
	}
    }
    retCode = RETCODE_CONS(retCodeId, MMP_ERROR_UNREC_STREAM);
done:
    if (popxp != NULL) {
	*popxp = popp - mmp->pops;
    }
    return retCode;
}

static RetCode
mmpParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mmp                 mmp = (Mmp) instp;
    RetCode             retCode;
    MmpInfo             info;
    MmpInfo            *infop = &info;
    PipePosition        scanDistance = 0;
    int                 popx;

    do {
	retCode = mmpRecognize(mmp, pipe, &popx);
	if (retCode == RETCODE_SUCCESS) {
	    break;
	}
	pipeSkip(mmp->pipe, 1);
    } while (++scanDistance < mmp->maxScanDistance);

    if (retCode == RETCODE_SUCCESS) {
	if (mmp->callBack != NULL) {
	    infop->parserIndex = popx;
	    infop->scanDistance = scanDistance;
	    infop->position = pipePosition(pipe);
	    retCode = (*mmp->callBack) (mmp->callBackToken, infop, retCode);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	retCode = MMP_PARSER_PARSE(mmp->pops[popx], cop, pipe);
    }
    return retCode;
}

static void
mmpCleanup(Mmp mmp)
{
    if (mmp->pipe != NULL) {
        MmpParserObject   **popp;

	for (popp = mmp->pops; *popp != NULL; popp++) {
	    MMP_PARSER_RECOVER(*popp);
	}
    }
}
