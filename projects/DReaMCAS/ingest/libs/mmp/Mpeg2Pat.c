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
 * $(@)Mpeg2Pat.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * MPEG2 program association table parser
 */

#pragma ident "@(#)Mpeg2Pat.c 1.2	97/05/20 SMI"

#include <string.h>
#include <stdlib.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg2Pat.h"

#define	MPEG2_PAT_MAX_CHUNK		256

struct _Mpeg2Pat {
    MmpParserObject    *netPop;
    MmpParserObject    *psiPop;
    Mpeg2PatCallBack    callBack;
    void               *token;
    Mpeg2PatTable       pat[2];
    MmpParserObject     po;
    Boolean		isDirty;
};

static size_t       mpeg2PatMaxChunk(void *instp);

static RetCode
mpeg2PatParse(void *instp, MmpContextObject *cop,
	      Pipe pipe);

static void         mpeg2PatRecover(void *instp);

static RetCode
mpeg2PatActivate(Mpeg2Pat mpeg2Pat, MmpContextObject *cop,
		 Mpeg2PatTable *patp, unsigned sectionNumber,
		 Mpeg2PatSection *psp);

static RetCode
mpeg2PatPromote(Mpeg2Pat mpeg2Pat, MmpContextObject *cop);

static void         mpeg2PatDeactivate(Mpeg2PatTable *patp);

static RetCodeTable mpeg2PatErrorTable[] = {
    {MPEG2_PAT_ERROR_SSI, NULL, "section syntax indicator not set"},
    {MPEG2_PAT_ERROR_SN, NULL, "section number > last section number"},
    {MPEG2_PAT_ERROR_PID, NULL, "PAT found on non-zero pid"},
    {MPEG2_PAT_ERROR_LSN, NULL, "last section number changed within version"},
    {MPEG2_PAT_ERROR_SL, NULL, "pat section too long"},
    {MPEG2_PAT_ERROR_LEN, NULL, "pat section length error"},
    {MPEG2_PAT_ERROR_CRC, NULL, "crc error"},
    {MPEG2_PAT_ERROR_MISMATCH, NULL, "section changed without new version"},
    {MPEG2_PAT_ERROR_PID_USE, NULL, "pid usage conflict"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Mpeg2Pat
mpeg2PatNew(void)
{
    Mpeg2Pat            mpeg2Pat;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(MPEG2PAT_CLASSNAME,
					     mpeg2PatErrorTable);
    }
    mpeg2Pat = NEW_ZEROED(struct _Mpeg2Pat, 1);

    mpeg2Pat->pat[MPEG2_PAT_NEXT].psiExtension.currentNextIndicator
      = MPEG2_PAT_NEXT;
    mpeg2Pat->pat[MPEG2_PAT_CURRENT].psiExtension.currentNextIndicator
      = MPEG2_PAT_CURRENT;

    mpeg2Pat->po.maxChunk = mpeg2PatMaxChunk;
    mpeg2Pat->po.recognize = NULL;
    mpeg2Pat->po.parse = mpeg2PatParse;
    mpeg2Pat->po.recover = mpeg2PatRecover;
    mpeg2Pat->po.instp = mpeg2Pat;

    return mpeg2Pat;
}

void
mpeg2PatSetPsiParsers(Mpeg2Pat mpeg2Pat, MmpParserObject *netPop,
		      MmpParserObject *psiPop)
{
    mpeg2PatRecover(mpeg2Pat);
    mpeg2Pat->netPop = netPop;
    mpeg2Pat->psiPop = psiPop;
}


MmpParserObject    *
mpeg2PatParserObject(Mpeg2Pat mpeg2Pat)
{
    return &mpeg2Pat->po;
}

void
mpeg2PatSetCallBack(Mpeg2Pat mpeg2Pat, Mpeg2PatCallBack callBack, void *token)
{
    mpeg2Pat->callBack = callBack;
    mpeg2Pat->token = token;
}

Mpeg2PatStatus
mpeg2PatStatus(Mpeg2Pat mpeg2Pat)
{
    Mpeg2PatStatus      patStatus;

    patStatus.pat[0] = mpeg2Pat->pat[0];
    patStatus.pat[1] = mpeg2Pat->pat[1];
    return patStatus;
}

void
mpeg2PatFree(Mpeg2Pat mpeg2Pat)
{
    mpeg2PatRecover(mpeg2Pat);
    free(mpeg2Pat);
}

/* ARGSUSED */
static size_t
mpeg2PatMaxChunk(void *instp)
{
    return MPEG2_PAT_MAX_CHUNK;
}

static RetCode
mpeg2PatParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Mpeg2Pat            mpeg2Pat = (Mpeg2Pat) instp;
    Mpeg2PatInfo        info;
    Mpeg2PatInfo       *infop = &info;
    Mpeg2PatTable      *patp;
    PipePosition        patEnd;
    RetCode             retCode;

    mpeg2Pat->isDirty = TRUE;
    (void) memset(infop, 0, sizeof(*infop));

    infop->position = pipePosition(pipe);
    patEnd = pipeRelativePosition(pipe);
    infop->psiHeader = *PIPE_GET(pipe, Mpeg2PsiHeader);
    if (!infop->psiHeader.sectionSyntaxIndicator) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_SSI);
	goto error;
    }
    infop->psiExtension = *PIPE_GET(pipe, Mpeg2PsiExtension);

    if (infop->psiExtension.sectionNumber
	    > infop->psiExtension.lastSectionNumber) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_SN);
	goto error;
    }
    /*
     * PAT's can only come on pid 0
     */
    if (MMP_CONTEXT_PIPETOPID(cop, pipe) != MPEG2_PAT_PID) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_PID);
	goto error;
    }
    patEnd += MPEG2_PSI_HEADER_SECTION_LENGTH(&infop->psiHeader)
      + sizeof(Mpeg2PsiHeader);

    /*
     * If version number doesn't match current contents of pat, invalidate
     * that pat.  Then check if "next" pat vn matches, if so, promote it to
     * the current pat.
     */
    patp = &mpeg2Pat->pat[infop->psiExtension.currentNextIndicator];
    if (patp->isValid
	    && infop->psiExtension.versionNumber
	    != patp->psiExtension.versionNumber) {
	mpeg2PatDeactivate(patp);
    }
    if (!patp->isValid) {
	patp->psiExtension = infop->psiExtension;
	patp->isValid = TRUE;
    } else if (infop->psiExtension.lastSectionNumber
	       != patp->psiExtension.lastSectionNumber) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_LSN);
	goto error;
    }
    if (infop->psiExtension.currentNextIndicator == MPEG2_PAT_CURRENT
	    && mpeg2Pat->pat[MPEG2_PAT_NEXT].psiExtension.versionNumber
	    == infop->psiExtension.versionNumber
	    && mpeg2Pat->pat[MPEG2_PAT_NEXT].isValid) {
	if ((retCode = mpeg2PatPromote(mpeg2Pat, cop))
		!= RETCODE_SUCCESS) {
	    goto error;
	}
    }
    /*
     * Now, load new pat section and validate crc
     */
    infop->patSection.nPatEntries = 0;
    while (pipeRelativePosition(pipe) + 4 < patEnd) {
	if (infop->patSection.nPatEntries
		>= MPEG2_PAT_MAX_ENTRIES_PER_SECTION) {
	    return RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_SL);
	}
	infop->patSection.patEntries[infop->patSection.nPatEntries++]
	  = *PIPE_GET(pipe, Mpeg2PatEntry);
    }
    if (patEnd - pipeRelativePosition(pipe) != 4) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_LEN);
	goto error;
    }
    /*
     * FIXME: Do this the normal way with 4 bytes?
     */
    infop->crc = pipeGetBits(pipe, 32);
    if (pipeCrcGet(pipe) != 0) {
	retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_CRC);
	goto error;
    }
    if (mpeg2Pat->callBack != NULL) {
	retCode = (*mpeg2Pat->callBack) (mpeg2Pat->token, infop,
					 RETCODE_SUCCESS);
	if (retCode != RETCODE_SUCCESS) {
	    return retCode;
	}
    }
    return mpeg2PatActivate(mpeg2Pat, cop, patp,
		      infop->psiExtension.sectionNumber, &infop->patSection);

error:
    if (mpeg2Pat->callBack != NULL) {
	(void) (*mpeg2Pat->callBack) (mpeg2Pat->token, infop, retCode);
    }
    return retCode;
}

static void
mpeg2PatRecover(void *instp)
{
    Mpeg2Pat            mpeg2Pat = (Mpeg2Pat) instp;

    if (! mpeg2Pat->isDirty) {
	return;
    }
    mpeg2Pat->isDirty = FALSE;
    mpeg2PatDeactivate(&mpeg2Pat->pat[MPEG2_PAT_CURRENT]);
    mpeg2PatDeactivate(&mpeg2Pat->pat[MPEG2_PAT_NEXT]);

    MMP_PARSER_RECOVER(mpeg2Pat->netPop);
    MMP_PARSER_RECOVER(mpeg2Pat->psiPop);
}

static RetCode
mpeg2PatActivate(Mpeg2Pat mpeg2Pat, MmpContextObject *cop, Mpeg2PatTable *patp,
		 unsigned sectionNumber, Mpeg2PatSection *psp)
{
    RetCode             retCode = RETCODE_SUCCESS;
    Mpeg2PatSection    *curPsp = patp->patSectionsp[sectionNumber];
    unsigned            pex;

    if (curPsp == NULL) {
	curPsp = DUP(Mpeg2PatSection, psp);
	patp->patSectionsp[sectionNumber] = curPsp;
	if (patp->psiExtension.currentNextIndicator == MPEG2_PAT_CURRENT) {
	    for (pex = 0; pex < curPsp->nPatEntries; pex++) {
	        Mpeg2PatEntry      *pep = &curPsp->patEntries[pex];
		unsigned            pn = MPEG2_PAT_ENTRY_PROGRAM_NUMBER(pep);
		unsigned            pid = MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(pep);
		Pipe                pidPipe;
		MmpParserObject    *pop;

		switch (pn) {
		case MPEG2_PAT_PN_NETWORK_INFORMATION:
		    pop = mpeg2Pat->netPop;
		    break;
		default:
		    pop = mpeg2Pat->psiPop;
		    break;
		}

		curPsp->cops[pex] = cop;
		if ((pidPipe = MMP_CONTEXT_PIDTOPIPE(cop, pid)) == NULL) {
		    pidPipe = MMP_PARSER_NEWPIPE(pop, cop);
		    retCode = MMP_CONTEXT_ADDPIPE(cop, pid, pidPipe);
		    /*
		     * ADDPIPE can't fail, just checked...
		     */
		    ASSERT(retCode == RETCODE_SUCCESS);
		} else if (! MMP_PARSER_ISOWNPIPE(pop, pidPipe)) {
		    retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_PID_USE);
		    goto done;
		}
	    }
	}
    } else {
	if (curPsp->nPatEntries != psp->nPatEntries) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_MISMATCH);
	    goto done;
	}
	for (pex = 0; pex < psp->nPatEntries; pex++) {
	    Mpeg2PatEntry      *pep = &psp->patEntries[pex];
	    Mpeg2PatEntry      *curPep = &curPsp->patEntries[pex];
	    if (MPEG2_PAT_ENTRY_PROGRAM_NUMBER(pep)
		    != MPEG2_PAT_ENTRY_PROGRAM_NUMBER(curPep)
		    || MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(pep)
		    != MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(curPep)
		    || cop != curPsp->cops[pex]) {
		retCode = RETCODE_CONS(retCodeId, MPEG2_PAT_ERROR_MISMATCH);
		goto done;
	    }
	}
    }
done:
    return retCode;
}

static RetCode
mpeg2PatPromote(Mpeg2Pat mpeg2Pat, MmpContextObject *cop)
{
    RetCode             retCode = RETCODE_SUCCESS;
    Mpeg2PatTable      *nextPatp = &mpeg2Pat->pat[MPEG2_PAT_NEXT];
    Mpeg2PatTable      *curPatp = &mpeg2Pat->pat[MPEG2_PAT_CURRENT];
    unsigned            sn;

    ASSERT(nextPatp->psiExtension.versionNumber
	   == curPatp->psiExtension.versionNumber);

    for (sn = 0; sn <= nextPatp->psiExtension.lastSectionNumber; sn++) {
	Mpeg2PatSection    *psp = nextPatp->patSectionsp[sn];

	if (psp == NULL) {
	    continue;
	}
	if ((retCode = mpeg2PatActivate(mpeg2Pat, cop, curPatp, sn, psp))
		!= RETCODE_SUCCESS) {
	    return retCode;
	}
	free(psp);
	nextPatp->patSectionsp[sn] = NULL;
    }
    nextPatp->isValid = FALSE;
    return retCode;
}

static void
mpeg2PatDeactivate(Mpeg2PatTable *patp)
{
    unsigned            sn;

    if (patp->isValid) {
	for (sn = 0; sn <= patp->psiExtension.lastSectionNumber; sn++) {
	    Mpeg2PatSection    *psp = patp->patSectionsp[sn];

	    if (psp == NULL) {
		continue;
	    }
	    if (patp->psiExtension.currentNextIndicator == MPEG2_PAT_CURRENT) {
                unsigned            pex;
	        for (pex = 0; pex < psp->nPatEntries; pex++) {
	            Mpeg2PatEntry      *pep = &psp->patEntries[pex];
		    (void) MMP_CONTEXT_DELETEPIPE(psp->cops[pex],
				      MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(pep));
		}
	    }
	    free(psp);
	    patp->patSectionsp[sn] = NULL;
	}
	patp->isValid = FALSE;
    }
}
