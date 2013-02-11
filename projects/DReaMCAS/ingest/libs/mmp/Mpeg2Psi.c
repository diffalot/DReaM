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
 * $(@)Mpeg2Psi.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2Psi.c -- general Mpeg2 Psi parser.
 */

#pragma ident "@(#)Mpeg2Psi.c 1.1	96/09/18 SMI"

#include <stdlib.h>
#include <string.h>

#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Psi.h"

#define	MPEG2_PSI_MAX_CHUNK	1

struct _Mpeg2Psi {
    MmpParserObject    *tableIdToPop[MPEG2_PSI_TABLE_ID_STUFF];
    Boolean             doRecover;
    MmpParserObject     po;
    Boolean		isDirty;
};


static size_t       mpeg2PsiMaxChunk(void *instp);

static RetCode
mpeg2PsiParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg2PsiRecover(void *instp);

static RetCode
mpeg2PsiDefaultParse(void *instp, MmpContextObject *cop,
		     Pipe pipe);

static size_t       mpeg2PsiDefaultMaxChunk(void *instp);

static MmpParserObject mpeg2PsiDefaultParserObject = {
    mpeg2PsiDefaultMaxChunk,
    NULL,				   /* recognize */
    mpeg2PsiDefaultParse,
    NULL,				   /* recover */
    NULL				   /* instp */
};

static RetCodeTable mpeg2PsiErrorTable[] = {
    {MPEG2_PSI_ERROR_SL, NULL, "section length error"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Psi
mpeg2PsiNew(void)
{
    Mpeg2Psi            mpeg2Psi = NEW_ZEROED(struct _Mpeg2Psi, 1);

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2PSI_CLASSNAME,
					     mpeg2PsiErrorTable);
    }
    mpeg2Psi->doRecover = FALSE;

    mpeg2Psi->po.maxChunk = mpeg2PsiMaxChunk;
    mpeg2Psi->po.recognize = NULL;
    mpeg2Psi->po.parse = mpeg2PsiParse;
    mpeg2Psi->po.recover = mpeg2PsiRecover;
    mpeg2Psi->po.instp = mpeg2Psi;

    return mpeg2Psi;
}

void
mpeg2PsiSetTableParsers(Mpeg2Psi mpeg2Psi,
		    MmpParserObject *tableIdToPop[MPEG2_PSI_TABLE_ID_STUFF])
{
    mpeg2PsiRecover(mpeg2Psi);
    (void) memcpy(mpeg2Psi->tableIdToPop, tableIdToPop,
	          sizeof(mpeg2Psi->tableIdToPop));
}

MmpParserObject    *
mpeg2PsiParserObject(Mpeg2Psi mpeg2Psi)
{
    return &mpeg2Psi->po;
}

void
mpeg2PsiSetRecover(Mpeg2Psi mpeg2Psi, Boolean doRecover)
{
    mpeg2Psi->doRecover = doRecover;
}

Boolean
mpeg2PsiGetRecover(Mpeg2Psi mpeg2Psi)
{
    return mpeg2Psi->doRecover;
}

void
mpeg2PsiFree(Mpeg2Psi mpeg2Psi)
{
    mpeg2PsiRecover(mpeg2Psi);
    free(mpeg2Psi);
}

static size_t
mpeg2PsiMaxChunk(void *instp)
{
    Mpeg2Psi            mpeg2Psi = (Mpeg2Psi) instp;
    unsigned            tableId;
    size_t              maxChunk = MPEG2_PSI_MAX_CHUNK;

    for (tableId = 0; tableId < MPEG2_PSI_TABLE_ID_STUFF; tableId++) {
	MmpParserObject    *pop = mpeg2Psi->tableIdToPop[tableId];

	if (pop != NULL) {
	    size_t              chunk = MMP_PARSER_MAXCHUNK(pop);

	    if (chunk > maxChunk) {
		maxChunk = chunk;
	    }
	}
    }
    return maxChunk;
}

static RetCode
mpeg2PsiParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Psi            mpeg2Psi = (Mpeg2Psi) instp;
    RetCode             retCode = RETCODE_SUCCESS;

    mpeg2Psi->isDirty = TRUE;
    do {
	/*
	 * Find known table start
	 */
	pipeFindMark(pipe);
	do {
	    MmpParserObject    *pop;
	    unsigned            tableId;

	    /*
	     * If at start of transport packet and payload unit start is set,
	     * then get pointer field and skip
	     */
	    if (pipeIsAtMark(pipe)) {
		unsigned            pf = *PIPE_GET(pipe, u8);

		pipeSkip(pipe, pf);
	    }
	    /*
	     * If tableId is 0xff, then flush stuffing to end of transport
	     * packet.
	     */
	    tableId = *PIPE_PEEK(pipe, u8);
	    if (tableId == MPEG2_PSI_TABLE_ID_STUFF) {
		break;
	    }
	    pop = mpeg2Psi->tableIdToPop[tableId];
	    if (pop == NULL) {
		pop = &mpeg2PsiDefaultParserObject;
	    }
	    retCode = MMP_PARSER_PARSE(pop, cop, pipe);
	} while (retCode == RETCODE_SUCCESS);
    } while (retCode == RETCODE_SUCCESS || mpeg2Psi->doRecover);

    return retCode;
}

static void
mpeg2PsiRecover(void *instp)
{
    Mpeg2Psi            mpeg2Psi = (Mpeg2Psi) instp;
    unsigned            tableId;

    if (! mpeg2Psi->isDirty) {
	return;
    }
    mpeg2Psi->isDirty = FALSE;
    for (tableId = 0; tableId < MPEG2_PSI_TABLE_ID_STUFF; tableId++) {
	MMP_PARSER_RECOVER(mpeg2Psi->tableIdToPop[tableId]);
    }
}

/* ARGSUSED */
static size_t
mpeg2PsiDefaultMaxChunk(void *instp)
{
    return 0;
}

/* ARGSUSED */
static RetCode
mpeg2PsiDefaultParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    PipePosition        psiStart = pipeRelativePosition(pipe);
    Mpeg2PsiHeader      psiHeader = *PIPE_GET(pipe, Mpeg2PsiHeader);
    PipePosition        psiEnd;

    psiEnd = psiStart + MPEG2_PSI_HEADER_SECTION_LENGTH(&psiHeader)
      + sizeof(Mpeg2PsiHeader);
    /*
     * Blindly skip an unknown table type
     */
    if (pipeRelativePosition(pipe) > psiEnd) {
	return RETCODE_CONS(retCodeId, MPEG2_PSI_ERROR_SL);
    }
    pipeSkip(pipe, psiEnd - pipeRelativePosition(pipe));
    return RETCODE_SUCCESS;
}
