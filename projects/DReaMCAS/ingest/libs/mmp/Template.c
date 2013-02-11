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
 * $(@)Template.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * MMP Parser Template
 */

#pragma ident "@(#)Template.c 1.1	98/10/22 SMI"

#include <string.h>
#include <stdlib.h>
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

/*
 * Header files describing API's you use to interface to the
 * parser system.
 */
#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

#include "mmp/Template.h"	/* Your include file */

struct _Template {
    TemplateCallBack    callBack;
    void               *token;
    MmpParserObject    *pesp;
    MmpParserObject     po;
    Boolean		isDirty;
    /*
     * Add any other instance variables needed.
     */
};

static size_t       templateMaxChunk(void *instp);
static RetCode      templateRecognize(void *instp, Pipe pipe);
static RetCode	    templateParse(void *instp, MmpContextObject *cop,
				  Pipe pipe);
static void         templateRecover(void *instp);

static RetCodeTable templateErrorTable[] = {
    /*
     * Replace these with appropriate error codes and messages.
     * The error codes are defined in Template.h
     */
    {TEMPLATE_ERROR_SYNTAX, NULL, "pack syntax error"},
    {TEMPLATE_ERROR_END_CODE, NULL, "missing end code"},
    {TEMPLATE_ERROR_HDR_LEN, NULL, "system header length error"},
    {0, NULL, NULL}
};

static RetCodeId    retCodeId;

Template
templateNew(void)
{
    Template            template;

    if (retCodeId == 0) {
	retCodeId = retCodeRegisterWithTable(TEMPLATE_CLASSNAME,
					     templateErrorTable);
    }
    template = NEW_ZEROED(struct _template, 1);

    template->po.maxChunk = templateMaxChunk;
    template->po.recognize = templateRecognize;
    template->po.parse = templateParse;
    template->po.recover = templateRecover;
    template->po.instp = template;

    /*
     * Initialize any other instance variables here.
     */

    return template;
}

MmpParserObject    *
templateParserObject(Template template)
{
    return &template->po;
}

void
templateSetCallBack(Template template, TemplateCallBack callBack, void *token)
{
    template->callBack = callBack;
    template->token = token;
}

void
templateFree(Template template)
{
    free(template);
}

static size_t
templateMaxChunk(void *instp)
{
    Template            template = (Template) instp;
    size_t		maxChunk;

    /* 
     * Determine max chunk here.
     */

    return maxChunk;
}

static RetCode
templateRecognize(void *instp, Pipe pipe)
{
    Boolean isRecognizable;

    /*
     * Peek at bits in pipe and return TRUE if they look like
     * something recognizable.
     */

    return isRecognizable; 
}

static RetCode
templateParse(void *instp, MmpContextObject *cop, Pipe pipe)
{
    Template            template = (Template) instp;
    TemplateInfo        info;
    TemplateInfo       *infop = &info;
    RetCode             retCode;

    template->isDirty = TRUE;
    do {
	(void) memset(infop, 0, sizeof(*infop));
	infop->position = pipePosition(pipe);

	infop->pack = *PIPE_GET(pipe, TemplatePack);

	/*
	 * Parse "unit" of stream
	 */

	if (template->callBack != NULL) {
	    retCode = (*template->callBack) (template->token, infop,
					     RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	/*
	 * If "unit" had payload, pass to appropriate
	 * sub-level parser else skip.
	 */
	if (template->pesp == NULL) {
	    templatePesSkip(template, pipe);
	} else {
	    retCode = MMP_PARSER_PARSE(template->pesp, NULL, pipe);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	scp = PIPE_PEEK(pipe, TemplateStartCode);
    } while (IS_MPEG1_SYS_START_CODE(scp, MPEG1_SYS_START_CODE_PACK));

    scp = PIPE_GET(pipe, TemplateStartCode);
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
    if (template->callBack != NULL) {
	(void) (*template->callBack) (template->token, infop, retCode);
    }
    return retCode;
}

static void
templateRecover(void *instp)
{
    Template            template = (Template) instp;

    if (! template->isDirty) {
	return;
    }
    template->isDirty = FALSE;
    MMP_PARSER_RECOVER(template->pesp);
}
