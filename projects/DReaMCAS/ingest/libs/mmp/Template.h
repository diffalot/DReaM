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
 * $(@)Template.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Template.h
 * 
 * Template.h is the interface to information gleaned by parsing template
 * streams.
 */

#ifndef	_MMP_TEMPLATE_H
#define	_MMP_TEMPLATE_H

#pragma ident "@(#)Template.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Template Specific Types
 ***********************************************************************/

/*
 * Put protocol specific data types here
 */

/***********************************************************************
 * OBJECT Template Parser CallBack Info Type
 ***********************************************************************/

typedef struct TemplateInfo {
    PipePosition        position;

    /*
     * Protocol specific info
     */

} TemplateInfo;

typedef RetCode     (*TemplateCallBack) (void *token, TemplateInfo *infop,
				                           RetCode retCode);

/***********************************************************************
 * Template Error Codes
 *
 * NOTE: Use retCodeFor(TEMPLATE_CLASSNAME, TemplateError) to map to
 * RetCode.
 ***********************************************************************/

#define	TEMPLATE_CLASSNAME	"Template"

typedef enum TemplateError {
    TEMPLATE_ERROR_SYNTAX = 1,		   /* pack syntax error */
    TEMPLATE_ERROR_END_CODE,		   /* missing end code */
    TEMPLATE_ERROR_SYSTEM_HDR_LEN	   /* system header length error */
} TemplateError;

/***********************************************************************
 * OBJECT Template Parser Instance Type
 ***********************************************************************/

typedef struct _Template *Template;

/***********************************************************************
 * OBJECT Template Parser Class Interface
 ***********************************************************************/

extern Template     templateNew(void);

/***********************************************************************
 * OBJECT Template Parser Instance Interface
 ***********************************************************************/

/*
 * Set PES parser
 */
extern void
templateSetPesParser(Template template,
		     MmpParserObject *pesp);

extern MmpParserObject *templateParserObject(Template template);

extern void
templateSetCallBack(Template template, TemplateCallBack callBack,
		    void *token);

extern void         templateFree(Template template);

_FOREIGN_END

#endif					   /* _MMP_TEMPLATE_H */
