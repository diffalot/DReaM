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
 * $(@)Mmp.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mmp.h
 * 
 * Mmp is the multi-media parser framework.
 */

#ifndef	_MMP_MMP_H
#define	_MMP_MMP_H

#pragma ident "@(#)Mmp.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * OBJECT Mmp Defines
 ***********************************************************************/
#define	MMP_MAXCHUNK_DEFAULT		256

#define	MMP_PID_NULL			0xffff

/***********************************************************************
 * Mmp Error Codes
 *
 * NOTE: Use retCodeFor(MMP_CLASSNAME, MmpError) to map to
 * RetCode.
 ***********************************************************************/

#define	MMP_CLASSNAME			"Mmp"

typedef enum MmpError {
    MMP_ERROR_UNREC_STREAM = 1,		   /* no parser recognizes stream */
    MMP_ERROR_NO_PARSER			   /* no parser */
} MmpError;

/***********************************************************************
 * OBJECT Mmp Instance Type
 ***********************************************************************/

typedef struct _Mmp *Mmp;

/***********************************************************************
 * OBJECT Interface Definitions
 ***********************************************************************/

/*
 * MmpContextObject Interface Definition
 */

typedef struct MmpContextObject {
    RetCode             (*addPipe) (void *instp, u16 pid, Pipe pipe);
    RetCode             (*deletePipe) (void *instp, u16 pid);
    Pipe                (*pidToPipe) (void *instp, u16 pid);
    u16                 (*pipeToPid) (void *instp, Pipe pipe);
    void		(*deleteAll) (void *instp);
    void               *instp;
} MmpContextObject;

#define	MMP_CONTEXT_ADDPIPE(cop, pid, pipe)				\
	((*(cop)->addPipe)((cop)->instp, pid, pipe))

#define	MMP_CONTEXT_DELETEPIPE(cop, pid)				\
	((*(cop)->deletePipe)((cop)->instp, pid))

#define	MMP_CONTEXT_PIDTOPIPE(cop, pid)					\
	((*(cop)->pidToPipe)((cop)->instp, pid))

#define	MMP_CONTEXT_PIPETOPID(cop, pipe)				\
	((cop) != NULL ? (*(cop)->pipeToPid)((cop)->instp, pipe) : -1)

#define	MMP_CONTEXT_DELETEALL(cop)					\
	((*(cop)->deleteAll)((cop)->instp))

/*
 * MmpParserObject Interface Definition
 */

typedef struct MmpParserObject {
    size_t              (*maxChunk) (void *instp);
    RetCode             (*recognize) (void *instp, Pipe pipe);
    RetCode             (*parse) (void *instp, MmpContextObject *cop,
				                      Pipe pipe);
    void                (*recover) (void *instp);
    void               *instp;
} MmpParserObject;

#define	MMP_PARSER_MAXCHUNK(pop)				\
	((pop) != NULL && ((pop)->maxChunk) != NULL		\
	    ? ((*(pop)->maxChunk)((pop)->instp)) : MMP_MAXCHUNK_DEFAULT)

#define	MMP_PARSER_RECOGNIZE(pop, pipe)				\
	((pop) != NULL && ((pop)->recognize) != NULL		\
	    ? (*(pop)->recognize)((pop)->instp, pipe) : RETCODE_FAILED)

#define	MMP_PARSER_PARSE(pop, cop, pipe)			\
	((pop) != NULL && ((pop)->parse) != NULL		\
	    ? (*(pop)->parse)((pop)->instp, cop, pipe) : RETCODE_FAILED)

#define	MMP_PARSER_NEWPIPE(pop, cop)				\
	((pop) != NULL && ((pop)->parse) != NULL		\
	    ? pipeNew(MMP_PARSER_MAXCHUNK(pop),			\
		(PipeParser)(pop)->parse,			\
		(PipeCleanup)(pop)->recover, (pop)->instp, cop)	\
	    : PIPE_NULL)

#define	MMP_PARSER_RECOVER(pop)					\
	BEGIN_STMT						\
	if ((pop) != NULL && ((pop)->recover) != NULL) {	\
	    (*(pop)->recover)((pop)->instp);			\
	}							\
	END_STMT

#define	MMP_PARSER_ISOWNPIPE(pop, pipe)				\
	((((pop) == NULL || (pop)->parse == NULL)		\
	  && ((pipe) == NULL || (pipe) == PIPE_NULL))		\
	 || (PipeParser)((pop)->parse) == pipeGetParser(pipe))

/***********************************************************************
 * OBJECT Mmp Class Interface
 ***********************************************************************/

/*
 * parserObjects is terminated by an entry with a null instp
 */
extern Mmp          mmpNew(void);

/***********************************************************************
 * OBJECT Mmp Instance Interface
 ***********************************************************************/

extern void         mmpSetParsers(Mmp mmp, MmpParserObject *pops[]);

/*
 * Set stream offset for next mmpPut(), used to initialize
 * pipePosition() base when starting parse at other than beginning
 * of stream "file".
 */
extern void	    mmpSetPosition(Mmp mmp, PipePosition putPosition);

typedef struct MmpInfo {
    int                 parserIndex;
    PipePosition        scanDistance;
    PipePosition        position;
} MmpInfo;

typedef RetCode     (*MmpCallBack) (void *callBackToken, MmpInfo *infop,
				                        RetCode retCode);

extern void
mmpSetCallBack(Mmp mmp, MmpCallBack callBack,
	       void *callBackToken);
extern void
mmpSetMaxScanDistance(Mmp mmp,
		      PipePosition maxScanDistance);

extern RetCode      mmpPut(Mmp mmp, u8 *buf, size_t len);
extern RetCode      mmpEof(Mmp mmp);
extern PipePosition mmpParsedBytes(Mmp mmp);
extern void         mmpRecover(Mmp mmp);
extern void         mmpFree(Mmp mmp);

_FOREIGN_END

#endif					   /* _MMP_MMP_H */
