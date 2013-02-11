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
 * $(@)Mpeg2Pat.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * OBJECT Mpeg2Pat.h -- Interface to mpeg2 program association table parser.
 */
#ifndef	_MMP_MPEG2PAT_H
#define	_MMP_MPEG2PAT_H

#pragma ident "@(#)Mpeg2Pat.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Mpeg2Psi.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Mpeg2 Program Association Table Types
 ***********************************************************************/

#define	MPEG2_PAT_TABLE_ID			0
#define	MPEG2_PAT_PID				0
#define	MPEG2_PAT_PN_NETWORK_INFORMATION	0

typedef struct Mpeg2PatEntry {
    BIT1(programNumber15_8:8);
    BIT1(programNumber7_0:8);
    BIT2(reserved:3,
	 programMapPid12_8:5);
    BIT1(programMapPid7_0:8);
} Mpeg2PatEntry;

#define	MPEG2_PAT_ENTRY_PROGRAM_NUMBER(p)			\
	(							\
	    ((p)->programNumber15_8 << 8)			\
	  |  (p)->programNumber7_0				\
	)

#define	MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(p)			\
	(							\
	    ((p)->programMapPid12_8 << 8)			\
	  |  (p)->programMapPid7_0				\
	)

#define	MPEG2_PAT_MAX_ENTRIES_PER_SECTION	(1024 / sizeof(Mpeg2PatEntry))

typedef struct Mpeg2PatSection {
    u16                 nPatEntries;
    Mpeg2PatEntry       patEntries[MPEG2_PAT_MAX_ENTRIES_PER_SECTION];
    MmpContextObject	*cops[MPEG2_PAT_MAX_ENTRIES_PER_SECTION];
} Mpeg2PatSection;

#define	MPEG2_PAT_MAX_SECTIONS		255

typedef struct Mpeg2PatTable {
    Boolean             isValid;
    Mpeg2PsiExtension   psiExtension;
    Mpeg2PatSection    *patSectionsp[MPEG2_PAT_MAX_SECTIONS];
} Mpeg2PatTable;

typedef enum Mpeg2PatCurrentNextIndicator {
    MPEG2_PAT_NEXT = 0,
    MPEG2_PAT_CURRENT = 1
} Mpeg2PatCurrentNextIndicator;

/***********************************************************************
 * Mpeg2 PAT Callback Info Types
 ***********************************************************************/

typedef struct Mpeg2PatInfo {
    PipePosition        position;

    Mpeg2PsiHeader      psiHeader;
    Mpeg2PsiExtension   psiExtension;
    Mpeg2PatSection     patSection;
    u32                 crc;
} Mpeg2PatInfo;

typedef RetCode     (*Mpeg2PatCallBack) (void *token, Mpeg2PatInfo *infop,
				                           RetCode retCode);

#define	MPEG2_PAT_TRANSPORT_STREAM_ID(p)			\
	MPEG2_PSI_EXTENSION_TABLE_ID_EXTENSION(p)

/***********************************************************************
 * Mpeg2 PAT Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2PAT_CLASSNAME, Mpeg2PatError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2PAT_CLASSNAME		"Mpeg2Pat"

typedef enum Mpeg2PatError {
    MPEG2_PAT_ERROR_SSI = 1,		   /* section syntax indicator not
					    * set */
    MPEG2_PAT_ERROR_SN,			   /* section number > last section
					    * number */
    MPEG2_PAT_ERROR_PID,		   /* PAT found on non-zero pid */
    MPEG2_PAT_ERROR_LSN,		   /* last section number changed
					    * within version */
    MPEG2_PAT_ERROR_SL,			   /* pat section too long */
    MPEG2_PAT_ERROR_LEN,		   /* pat section length error */
    MPEG2_PAT_ERROR_CRC,		   /* crc error */
    MPEG2_PAT_ERROR_MISMATCH,		   /* section changed without new
					    * version */
    MPEG2_PAT_ERROR_PID_USE		   /* pid usage conflict */
} Mpeg2PatError;

/***********************************************************************
 * Mpeg2 PAT Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Pat *Mpeg2Pat;

/***********************************************************************
 * Mpeg2 PAT Class Interface
 ***********************************************************************/

extern Mpeg2Pat     mpeg2PatNew(void);

/***********************************************************************
 * Mpeg2 PAT Instance Interface
 ***********************************************************************/

extern void
mpeg2PatSetPsiParsers(Mpeg2Pat mpeg2Pat, MmpParserObject *netPop,
		      MmpParserObject *psiPop);

extern MmpParserObject *mpeg2PatParserObject(Mpeg2Pat mpeg2Pat);

extern void
mpeg2PatSetCallBack(Mpeg2Pat mpeg2Pat, Mpeg2PatCallBack callBack,
		    void *token);

extern void         mpeg2PatFree(Mpeg2Pat mpeg2Pat);

typedef struct Mpeg2PatStatus {
    /*
     * pat[0] is "next" pat. pat[1] is "current" pat.
     */
    Mpeg2PatTable       pat[2];
} Mpeg2PatStatus;

extern Mpeg2PatStatus mpeg2PatStatus(Mpeg2Pat mpeg2Pat);

_FOREIGN_END

#endif					   /* _MMP_MPEG2PAT_H */
